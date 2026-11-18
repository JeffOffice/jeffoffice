#include "core/sheetembeds.h"

#include "app/objects.h"
#include "office/package.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QImageReader>
#include <QPainter>

#include <algorithm>
#include <atomic>

namespace jp::crunch {

using namespace jp::office::sheet;
namespace ole = jp::office::ole;

namespace {

// A sheet's part larger than this is not looked through for objects (a stranger's file that names an object and has a sheet of this size is shown without them).
qint64 g_scanLimit = SheetEmbeds::kScanBytes;
std::atomic<qint64> g_previewReads{0};

constexpr double kEmuPerUnit = 9525;   // the grid's unit is a pixel at 100%

bool samePoint(const AnchorPoint &a, const AnchorPoint &b) { return a.col == b.col && a.row == b.row && a.colOff == b.colOff && a.rowOff == b.rowOff; }
bool sameAnchor(const DrawingAnchor &a, const DrawingAnchor &b) { return samePoint(a.from, b.from) && samePoint(a.to, b.to); }

bool hasObjectRelationship(const jp::office::Package &package, const QString &part)
{
    for (const jp::office::Relationship &r : package.relationships(part))
        if (!r.external && (r.type.endsWith(QLatin1String("/oleObject")) || r.type.endsWith(QLatin1String("/package")))) return true;
    return false;
}

} // namespace

SheetEmbeds::SheetEmbeds(QObject *parent) : QObject(parent) {}

qint64 SheetEmbeds::previewReads() { return g_previewReads.load(std::memory_order_relaxed); }
qint64 SheetEmbeds::scanLimit() { return g_scanLimit; }
void SheetEmbeds::setScanLimit(qint64 bytes) { g_scanLimit = bytes > 0 ? bytes : kScanBytes; }

QImage SheetEmbeds::previewImage(const QByteArray &png)
{
    if (!png.startsWith("\x89PNG")) return QImage();
    QBuffer buffer;
    buffer.setData(png);
    if (!buffer.open(QIODevice::ReadOnly)) return QImage();
    QImageReader reader(&buffer, "PNG");
    // (the size is in the header: a picture that would unpack past the cap is never unpacked)
    const QSize size = reader.size();
    if (!size.isValid() || qint64(size.width()) * size.height() * 4 > kMaxPreviewBytes) return QImage();
    reader.setAllocationLimit(int(kMaxPreviewBytes >> 20));
    g_previewReads.fetch_add(1, std::memory_order_relaxed);
    return reader.read();
}

void SheetEmbeds::open(const std::shared_ptr<Workbook> &book)
{
    m_book = book;
    m_sheets.clear();
    m_read = OleReadState();
    if (!m_book) return;
    m_sheets.resize(size_t(std::max(0, int(m_book->info().sheets.size()))));
    listAll();   // (every sheet, once, in order: the budget is the workbook's, and nothing asks for a sheet to be read after the window is up)
}

int SheetEmbeds::editCount() const { return m_edits ? int(m_edits().size()) : 0; }

SheetEmbeds::Sheet &SheetEmbeds::sheetState(int sheet)
{
    if (sheet >= int(m_sheets.size())) m_sheets.resize(size_t(sheet) + 1);
    Sheet &s = m_sheets[size_t(sheet)];
    if (!s.listed) list(sheet, s);
    return s;
}

void SheetEmbeds::list(int sheet, Sheet &s)
{
    s.listed = true;
    if (!m_book || sheet < 0 || sheet >= int(m_book->info().sheets.size())) return;
    const SheetEntry &se = m_book->info().sheets[sheet];
    // the objects an OpenDocument file held: the file the saves start from has none of them (they are written as objects of the sheet, as the ones put in are), so each is "not in the file"
    if (const auto &held = m_book->heldObjects(); held && se.kind == SheetKind::Worksheet && !se.deleted && !se.isNew && size_t(sheet) < held->size()) {
        int n = 0;
        for (const HeldObject &h : (*held)[size_t(sheet)]) {
            auto item = std::make_shared<Item>();
            item->id = s.nextId++;
            item->object = std::make_shared<const ole::EmbeddedObject>(h.object);
            item->baseAnchor = item->anchor = item->fileAnchor = h.anchor;
            item->name = tr("Object %1").arg(++n);
            s.items.push_back(item);
        }
        return;
    }
    if (se.kind != SheetKind::Worksheet || se.deleted || se.isNew || se.part.isEmpty()) return;
    const jp::office::Package &package = m_book->objectPackage();
    // (a sheet that names no object is not read through: that is most of them)
    if (!package.hasPart(se.part) || !hasObjectRelationship(package, se.part)) return;
    if (package.partSize(se.part) > g_scanLimit) {
        s.tooBig = true;   // (its objects are the file's: nothing here changes them)
        return;
    }
    std::vector<SheetOleObject> found;
    if (!readSheetOleObjects(package, se.part, &m_read, &found)) return;
    int n = 0;
    for (const SheetOleObject &o : found) {
        auto item = std::make_shared<Item>();
        item->id = s.nextId++;
        item->object = std::make_shared<const ole::EmbeddedObject>(o.object);
        item->fileObject = item->object;
        item->baseAnchor = item->anchor = item->fileAnchor = o.anchor;
        item->fileIndex = o.fileIndex;
        item->objectRel = o.objectRel;
        item->partName = o.object.origin;
        item->inFile = true;
        item->name = tr("Object %1").arg(++n);
        s.items.push_back(item);
        recompute(sheet, *item);   // (the edits made before the sheet was listed)
    }
}

void SheetEmbeds::recompute(int sheet, Item &item)
{
    static const std::vector<StructEdit> none;
    const DrawingAnchor before = item.anchor;
    const bool droppedBefore = item.dropped;
    // (an object of another program follows the rows and columns as ours does: the file is given its place)
    const std::vector<StructEdit> &all = m_edits ? m_edits() : none;
    const std::vector<StructEdit> from(all.begin() + std::min<size_t>(size_t(std::max(0, item.baseAt)), all.size()), all.end());
    item.anchor = movedAnchor(item.baseAnchor, sheet, from);
    item.dropped = !anchorSurvives(item.baseAnchor, sheet, from);
    if (!sameAnchor(before, item.anchor) || droppedBefore != item.dropped) Q_EMIT changed(sheet, item.id);
}

void SheetEmbeds::cellsMoved()
{
    for (size_t sheet = 0; sheet < m_sheets.size(); ++sheet)
        if (m_sheets[sheet].listed)
            for (const std::shared_ptr<Item> &i : m_sheets[sheet].items) recompute(int(sheet), *i);
}

const SheetEmbeds::Item *SheetEmbeds::find(int sheet, int id) const
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || !isEmbedId(id)) return nullptr;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items)
        if (i->id == id) return i.get();
    return nullptr;
}

SheetEmbeds::Item *SheetEmbeds::find(int sheet, int id) { return const_cast<Item *>(static_cast<const SheetEmbeds *>(this)->find(sheet, id)); }

bool SheetEmbeds::isDeleted(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i && i->deleted;
}

bool SheetEmbeds::isGone(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i && (i->deleted || i->dropped);
}

std::vector<int> SheetEmbeds::idsOf(int sheet)
{
    std::vector<int> ids;
    if (!m_book || sheet < 0 || sheet >= int(m_book->info().sheets.size())) return ids;
    for (const std::shared_ptr<Item> &i : sheetState(sheet).items)
        if (!i->deleted && !i->dropped) ids.push_back(i->id);
    return ids;
}

int SheetEmbeds::countOf(int sheet) { return int(idsOf(sheet).size()); }

std::shared_ptr<const ole::EmbeddedObject> SheetEmbeds::objectOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i ? i->object : nullptr;
}

DrawingAnchor SheetEmbeds::anchorOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i ? i->anchor : DrawingAnchor();
}

QString SheetEmbeds::nameOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i ? i->name : QString();
}

void SheetEmbeds::sizeOf(int sheet, int id, qint64 *widthEmu, qint64 *heightEmu) const
{
    const Item *i = find(sheet, id);
    if (!i || !i->object) return;
    *widthEmu = i->object->widthEmu;
    *heightEmu = i->object->heightEmu;
    double w = 0, h = 0;
    if (m_box && m_box(sheet, id, &w, &h) && w >= 1 && h >= 1) {
        *widthEmu = std::clamp<qint64>(qint64(w * kEmuPerUnit), 1, ole::kMaxEmu);
        *heightEmu = std::clamp<qint64>(qint64(h * kEmuPerUnit), 1, ole::kMaxEmu);
    }
}

bool SheetEmbeds::canMove(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i && i->object && !i->deleted && !i->dropped;
}

bool SheetEmbeds::holdsObjectsOf(int sheet) const { return sheet < 0 || size_t(sheet) >= m_sheets.size() || !m_sheets[size_t(sheet)].tooBig; }

// ---- the layer

jp::grid::LayerObject SheetEmbeds::layerObject(int, const std::shared_ptr<Item> &item)
{
    jp::grid::LayerObject o;
    o.id = item->id;
    o.anchor = item->anchor;
    o.kind = ObjectKind::Picture;
    o.name = item->name;
    o.hitInk = false;
    const std::weak_ptr<Item> weak = item;
    // (on the window's thread the picture is unpacked, once, and the words that say what the object is are taken; the job that draws holds copies of them alone, so that print and PDF, which
    // may draw on another thread, touch no object the window changes)
    o.prepare = [weak](const jp::grid::RenderRequest &q) -> std::function<QImage()> {
        const std::shared_ptr<Item> i = weak.lock();
        if (!i || !i->object || q.pixels.isEmpty() || q.inner.isEmpty()) return nullptr;
        if (!i->pictureRead) {
            i->picture = previewImage(i->object->previewPng);
            i->pictureRead = true;
        }
        const QImage picture = i->picture;
        const QString words = picture.isNull() ? objects::kindWords(*i->object) : QString();
        const QSize pixels = q.pixels;
        const QRectF inner = q.inner;
        return [picture, words, pixels, inner]() -> QImage {
            QImage image(pixels, QImage::Format_ARGB32_Premultiplied);
            if (image.isNull()) return QImage();
            image.fill(Qt::transparent);
            QPainter p(&image);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            if (!picture.isNull()) {
                p.drawImage(inner, picture);
            } else {
                // (no picture it can draw: a box that says what the object is)
                p.fillRect(inner, QColor(244, 244, 244));
                p.setPen(QColor(150, 150, 150));
                p.drawRect(inner.adjusted(0.5, 0.5, -0.5, -0.5));
                p.setPen(QColor(80, 80, 80));
                p.drawText(inner.adjusted(4, 4, -4, -4), Qt::AlignCenter | Qt::TextWordWrap, words);
            }
            return image;
        };
    };
    return o;
}

std::vector<jp::grid::LayerObject> SheetEmbeds::layerObjects(int sheet)
{
    std::vector<jp::grid::LayerObject> out;
    if (!m_book || sheet < 0 || sheet >= int(m_book->info().sheets.size())) return out;
    for (const std::shared_ptr<Item> &i : sheetState(sheet).items) {
        if (i->deleted || i->dropped) continue;
        out.push_back(layerObject(sheet, i));
    }
    return out;
}

std::optional<jp::grid::LayerObject> SheetEmbeds::layerObjectOf(int sheet, int id)
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return std::nullopt;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items)
        if (i->id == id) {
            if (i->deleted || i->dropped) return std::nullopt;
            return layerObject(sheet, i);
        }
    return std::nullopt;
}

// ---- steps

ObjectState SheetEmbeds::stateOf(int sheet, int id, int epoch) const
{
    ObjectState st;
    const Item *i = find(sheet, id);
    if (!i) return st;
    auto e = std::make_shared<EmbedState>();
    e->object = i->object;
    e->anchor = i->baseAnchor;
    e->at = i->baseAt;
    st.embed = e;
    st.deleted = i->deleted;
    st.madeHere = true;   // (the whole object is held, so a delete that was saved can be undone: it is written again from what is held)
    st.epoch = epoch;
    return st;
}

ObjectState SheetEmbeds::stateMoved(int sheet, int id, const DrawingAnchor &anchor, int epoch, const QSizeF &unitsNow) const
{
    ObjectState st = stateOf(sheet, id, epoch);
    if (!st.embed) return st;
    auto e = std::make_shared<EmbedState>(*st.embed);
    e->anchor = anchor;
    e->anchor.kind = AnchorKind::TwoCell;
    e->at = editCount();
    // an object of ours that is given another size has its picture drawn again at it (the one it has would be stretched); one of another program has only its own
    const ole::EmbeddedObject &o = *e->object;
    if (o.isOurs && o.kind() != ole::Kind::Other && unitsNow.width() >= 1 && unitsNow.height() >= 1) {
        const qint64 w = std::clamp<qint64>(qint64(unitsNow.width() * kEmuPerUnit), 1, ole::kMaxEmu), h = std::clamp<qint64>(qint64(unitsNow.height() * kEmuPerUnit), 1, ole::kMaxEmu);
        const bool changedSize = std::abs(double(w) - double(o.widthEmu)) > 0.02 * double(o.widthEmu) || std::abs(double(h) - double(o.heightEmu)) > 0.02 * double(o.heightEmu);
        if (changedSize && ole::pictureSizeIsSane(w, h)) {
            const ole::PictureFn draw = objects::pictureFor(o.kind());
            const QByteArray png = draw ? ole::pngOf(draw(o.partBytes, w, h, ole::kPictureScale)) : QByteArray();
            int live = 0;
            qint64 bytes = 0;
            totals(&live, &bytes);
            if (!png.isEmpty() && bytes - o.previewPng.size() + png.size() <= ole::limits().totalBytes) {
                auto sized = std::make_shared<ole::EmbeddedObject>(o);
                sized->previewPng = png;
                sized->widthEmu = w;
                sized->heightEmu = h;
                e->object = sized;
            }
        }
    }
    st.embed = e;
    return st;
}

ObjectState SheetEmbeds::stateChanged(int sheet, int id, const ole::EmbeddedObject &object, int epoch) const
{
    ObjectState st = stateOf(sheet, id, epoch);
    if (!st.embed) return st;
    auto e = std::make_shared<EmbedState>(*st.embed);
    e->object = std::make_shared<const ole::EmbeddedObject>(object);
    st.embed = e;
    return st;
}

void SheetEmbeds::apply(int sheet, int id, const EmbedState &state, bool deleted, bool placed)
{
    Item *i = find(sheet, id);
    if (!i) return;
    if (state.object) {
        if (state.object != i->object) i->pictureRead = false;
        i->object = state.object;
    }
    if (placed) {
        i->baseAnchor = state.anchor;
        i->baseAt = state.at;
    }
    i->deleted = deleted;
    // (a package no file has, taken away: the steps of Undo that name it hold it, and when the stack lets them go it goes)
    if (deleted && !i->inFile) {
        i->object.reset();
        i->picture = QImage();
        i->pictureRead = false;
    }
    recompute(sheet, *i);
    Q_EMIT changed(sheet, id);
}

int SheetEmbeds::make(int sheet, const ole::EmbeddedObject &object, const DrawingAnchor &anchor, QString *why)
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return -1;
    };
    if (!m_book || sheet < 0 || sheet >= int(m_book->info().sheets.size()) || m_book->info().sheets[sheet].kind != SheetKind::Worksheet)
        return refuse(tr("Objects can only be put on a sheet of cells."));
    if (!object.isOurs || object.partBytes.isEmpty() || object.previewPng.isEmpty()) return refuse(tr("This can't be put on a sheet as an object."));
    sheetState(sheet);
    if (m_sheets[size_t(sheet)].tooBig) return refuse(tr("This sheet is too large to put objects on."));
    // (the workbook's objects, all of them: the count, and the bytes of the packages and pictures, are held to the object core's caps, which the save holds the file to as well)
    int live = 0;
    qint64 bytes = 0;
    totals(&live, &bytes);
    if (live >= ole::limits().objects) return refuse(tr("A workbook can't have more than %1 objects.").arg(ole::limits().objects));
    if (bytes + object.partBytes.size() + object.previewPng.size() > ole::limits().totalBytes) return refuse(tr("The objects of this workbook would come to more than a file can hold."));
    Sheet &s = sheetState(sheet);
    auto item = std::make_shared<Item>();
    item->id = s.nextId++;
    item->object = std::make_shared<const ole::EmbeddedObject>(object);
    item->baseAnchor = item->anchor = anchor;
    item->baseAnchor.kind = item->anchor.kind = AnchorKind::TwoCell;
    item->baseAt = editCount();
    item->deleted = true;   // (there, but taken away, until a step gives it the state with `deleted` false)
    item->name = tr("Object %1").arg(int(s.items.size()) + 1);
    s.items.push_back(item);
    return item->id;
}

void SheetEmbeds::discard(int sheet, int id)
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return;
    auto &items = m_sheets[size_t(sheet)].items;
    for (size_t k = 0; k < items.size(); ++k)
        if (items[k]->id == id && !items[k]->inFile) {
            items.erase(items.begin() + qsizetype(k));
            return;
        }
}

std::optional<ole::EmbeddedObject> SheetEmbeds::copyOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    if (!i || !i->object) return std::nullopt;
    ole::EmbeddedObject copy = *i->object;
    copy.origin.clear();
    return copy;
}

void SheetEmbeds::listAll()
{
    if (!m_book) return;
    for (int i = 0; i < int(m_book->info().sheets.size()); ++i) sheetState(i);
}

// What the objects not taken away come to: how many, and the bytes of their packages and pictures.
void SheetEmbeds::totals(int *live, qint64 *bytes) const
{
    *live = 0;
    *bytes = 0;
    for (const Sheet &s : m_sheets)
        for (const std::shared_ptr<Item> &i : s.items)
            if (!i->deleted && !i->dropped && i->object) {
                ++*live;
                *bytes += i->object->partBytes.size() + i->object->previewPng.size() + i->object->otherPicture.size();
            }
}

int SheetEmbeds::liveCount() const
{
    int live = 0;
    qint64 bytes = 0;
    totals(&live, &bytes);
    return live;
}

void SheetEmbeds::sheetsChanged()
{
    if (!m_book) return;
    const int count = int(m_book->info().sheets.size());
    m_sheets.resize(size_t(std::max(0, count)));   // (a sheet added and then taken away by Undo has none)
    for (int sheet = 0; sheet < count; ++sheet) {
        Sheet &s = m_sheets[size_t(sheet)];
        if (s.listed) continue;
        const int source = m_copySource ? m_copySource(sheet) : -1;
        if (source >= 0 && source < count && source != sheet && m_book->info().sheets[sheet].isNew) copyFrom(sheet, source);
        else sheetState(sheet);
    }
}

// The objects of a sheet that was copied: the copy's own, made from the source's as they are now (a copy that would take the workbook past the caps has the objects that fit).
void SheetEmbeds::copyFrom(int sheet, int source)
{
    sheetState(source);
    Sheet &from = m_sheets[size_t(source)];
    Sheet &s = m_sheets[size_t(sheet)];
    s.listed = true;
    int live = 0;
    qint64 bytes = 0;
    totals(&live, &bytes);
    int n = 0;
    for (const std::shared_ptr<Item> &i : std::vector<std::shared_ptr<Item>>(from.items)) {
        if (i->deleted || i->dropped || !i->object) continue;
        const qint64 size = i->object->partBytes.size() + i->object->previewPng.size() + i->object->otherPicture.size();
        if (live >= ole::limits().objects || bytes + size > ole::limits().totalBytes) continue;
        ++live;
        bytes += size;
        auto item = std::make_shared<Item>();
        item->id = s.nextId++;
        ole::EmbeddedObject copy = *i->object;
        copy.origin.clear();
        item->object = std::make_shared<const ole::EmbeddedObject>(std::move(copy));
        item->baseAnchor = item->anchor = item->fileAnchor = i->anchor;
        item->baseAt = editCount();
        item->name = tr("Object %1").arg(++n);
        s.items.push_back(item);
    }
}

// ---- saving

void SheetEmbeds::addToSave(SaveChanges *changes)
{
    if (!m_book) return;
    for (size_t sheet = 0; sheet < m_sheets.size() && sheet < size_t(m_book->info().sheets.size()); ++sheet) {
        const Sheet &s = m_sheets[sheet];
        const SheetEntry &se = m_book->info().sheets[int(sheet)];
        if (!s.listed || se.deleted || se.kind != SheetKind::Worksheet) continue;
        OleObjectEdit edit;
        for (const std::shared_ptr<Item> &i : s.items) {
            const bool live = !i->deleted && !i->dropped;
            const bool ours = i->object && i->object->isOurs;
            if (i->inFile) {
                if (!live) {
                    edit.remove.insert(i->fileIndex);   // (an object of the file that was taken away: its element, its part, and its picture go)
                    continue;
                }
                // (moved or sized, or its rows and columns moved: only the place changes, in the element the file has; the object's part and picture, and what names them, are as they were)
                if (!sameAnchor(i->anchor, i->fileAnchor)) edit.reanchor[i->fileIndex] = i->anchor;
                if (ours && i->object != i->fileObject) edit.replace[i->fileIndex] = *i->object;   // (changed by the app that made it, or drawn again at another size)
            } else if (live && i->object) {
                // (new, or back from a delete that was saved: an object of another program is written as the parts it was read from)
                edit.add.push_back({*i->object, i->anchor});
            }
        }
        if (!edit.empty()) changes->oleObjects[int(sheet)] = std::move(edit);
    }
}

void SheetEmbeds::saved()
{
    if (!m_book) return;
    for (size_t sheet = 0; sheet < m_sheets.size() && sheet < size_t(m_book->info().sheets.size()); ++sheet) {
        Sheet &s = m_sheets[sheet];
        if (!s.listed || s.tooBig) continue;
        // what the file holds now, read from it (its places are the file's own)
        std::vector<SheetOleObject> found;
        OleReadState fresh;
        const SheetEntry &se = m_book->info().sheets[int(sheet)];
        if (!se.part.isEmpty() && !se.isNew) readSheetOleObjects(m_book->objectPackage(), se.part, &fresh, &found);
        std::vector<bool> taken(found.size(), false);
        std::vector<Item *> live;
        for (const std::shared_ptr<Item> &i : s.items) {
            if (i->deleted || i->dropped) {
                // (what the file now lacks stays away: a cell deleted is an object deleted; its bytes are held by the steps that can give it back)
                if (i->dropped) i->deleted = true;
                i->dropped = false;
                i->inFile = false;
                i->fileIndex = -1;
                i->objectRel.clear();
                i->partName.clear();
                i->fileObject.reset();
                i->object.reset();
                i->picture = QImage();
                i->pictureRead = false;
                continue;
            }
            live.push_back(i.get());
        }
        // each object is found by what the file names it by: an object the save did not touch by its relationship and part, the others (written anew, or given other bytes) by their bytes and place
        std::vector<int> at(live.size(), -1);
        for (size_t k = 0; k < live.size(); ++k) {
            const Item &i = *live[k];
            if (!i.inFile || i.object != i.fileObject || i.objectRel.isEmpty()) continue;
            for (size_t f = 0; f < found.size(); ++f)
                if (!taken[f] && found[f].objectRel == i.objectRel && found[f].object.origin.compare(i.partName, Qt::CaseInsensitive) == 0) {
                    at[k] = int(f);
                    taken[f] = true;
                    break;
                }
        }
        for (size_t k = 0; k < live.size(); ++k) {
            if (at[k] >= 0) continue;
            const Item &i = *live[k];
            for (size_t f = 0; f < found.size(); ++f)
                if (!taken[f] && found[f].object.partBytes == i.object->partBytes && sameAnchor(found[f].anchor, i.anchor)) {
                    at[k] = int(f);
                    taken[f] = true;
                    break;
                }
        }
        for (size_t k = 0; k < live.size(); ++k) {
            Item &i = *live[k];
            if (at[k] >= 0) {
                const SheetOleObject &f = found[size_t(at[k])];
                i.inFile = true;
                i.fileIndex = f.fileIndex;
                i.objectRel = f.objectRel;
                i.partName = f.object.origin;
            } else {
                i.inFile = false;   // (not read back: it is written again by the next save rather than lost)
                i.fileIndex = -1;
                i.objectRel.clear();
                i.partName.clear();
            }
            i.fileObject = i.object;
            i.baseAnchor = i.fileAnchor = i.anchor;
            i.baseAt = 0;
        }
    }
}

} // namespace jp::crunch
