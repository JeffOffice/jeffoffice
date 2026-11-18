#include "core/sheetembeds.h"

#include "app/objects.h"
#include "office/package.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QImageReader>
#include <QPainter>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;
namespace ole = jp::office::ole;

namespace {

// A sheet's part larger than this is not looked through for objects (a stranger's file that names an object and has a sheet of this size is shown without them).
constexpr qint64 kMaxScanBytes = qint64(512) << 20;

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
    return reader.read();
}

void SheetEmbeds::open(const std::shared_ptr<Workbook> &book)
{
    m_book = book;
    m_sheets.clear();
    m_read = OleReadState();
    if (m_book) m_sheets.resize(size_t(std::max(0, int(m_book->info().sheets.size()))));
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
    if (se.kind != SheetKind::Worksheet || se.deleted || se.isNew || se.part.isEmpty()) return;
    const jp::office::Package &package = m_book->objectPackage();
    // (a sheet that names no object is not read through: that is most of them)
    if (!package.hasPart(se.part) || package.partSize(se.part) > kMaxScanBytes || !hasObjectRelationship(package, se.part)) return;
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
    if (item.object && item.object->isOurs) {
        // (an object of another program stays where the file put it: saving leaves its place as the file has it)
        const std::vector<StructEdit> &all = m_edits ? m_edits() : none;
        const std::vector<StructEdit> from(all.begin() + std::min<size_t>(size_t(std::max(0, item.baseAt)), all.size()), all.end());
        item.anchor = movedAnchor(item.baseAnchor, sheet, from);
        item.dropped = !anchorSurvives(item.baseAnchor, sheet, from);
    } else {
        item.anchor = item.baseAnchor;
    }
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
    return i && i->object && i->object->isOurs;
}

// ---- the layer

jp::grid::LayerObject SheetEmbeds::layerObject(int, const std::shared_ptr<Item> &item)
{
    jp::grid::LayerObject o;
    o.id = item->id;
    o.anchor = item->anchor;
    o.kind = ObjectKind::Picture;
    o.name = item->name;
    o.fixed = !(item->object && item->object->isOurs);
    o.hitInk = false;
    const std::weak_ptr<Item> weak = item;
    o.render = [weak](const jp::grid::RenderRequest &q) -> QImage {
        const std::shared_ptr<Item> i = weak.lock();
        if (!i || !i->object || q.pixels.isEmpty() || q.inner.isEmpty()) return QImage();
        QImage image(q.pixels, QImage::Format_ARGB32_Premultiplied);
        if (image.isNull()) return QImage();
        image.fill(Qt::transparent);
        QPainter p(&image);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        const QImage picture = previewImage(i->object->previewPng);
        if (!picture.isNull()) {
            p.drawImage(q.inner, picture);
        } else {
            // (no picture it can draw: a box that says what the object is)
            p.fillRect(q.inner, QColor(244, 244, 244));
            p.setPen(QColor(150, 150, 150));
            p.drawRect(q.inner.adjusted(0.5, 0.5, -0.5, -0.5));
            p.setPen(QColor(80, 80, 80));
            p.drawText(q.inner.adjusted(4, 4, -4, -4), Qt::AlignCenter | Qt::TextWordWrap, objects::kindWords(*i->object));
        }
        return image;
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
    st.epoch = epoch;
    return st;
}

ObjectState SheetEmbeds::stateMoved(int sheet, int id, const DrawingAnchor &anchor, int epoch) const
{
    ObjectState st = stateOf(sheet, id, epoch);
    if (!st.embed) return st;
    auto e = std::make_shared<EmbedState>(*st.embed);
    e->anchor = anchor;
    e->anchor.kind = AnchorKind::TwoCell;
    e->at = editCount();
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
    if (state.object) i->object = state.object;
    if (placed) {
        i->baseAnchor = state.anchor;
        i->baseAt = state.at;
    }
    i->deleted = deleted;
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
    // (the workbook's objects, all of them: the count, and the bytes of the packages and pictures, are held to the object core's caps, which the save holds the file to as well)
    listAll();
    int live = 0;
    qint64 bytes = 0;
    for (const Sheet &s : m_sheets)
        for (const std::shared_ptr<Item> &i : s.items)
            if (!i->deleted && !i->dropped && i->object) {
                ++live;
                bytes += i->object->partBytes.size() + i->object->previewPng.size();
            }
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

int SheetEmbeds::liveCount()
{
    listAll();
    int n = 0;
    for (const Sheet &s : m_sheets)
        for (const std::shared_ptr<Item> &i : s.items)
            if (!i->deleted && !i->dropped) ++n;
    return n;
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
                } else if (ours && !sameAnchor(i->anchor, i->fileAnchor)) {
                    edit.remove.insert(i->fileIndex);   // (moved: written again where it is now)
                    edit.add.push_back({*i->object, i->anchor});
                } else if (ours && i->object != i->fileObject) {
                    edit.replace[i->fileIndex] = *i->object;   // (changed by the app that made it: its place stays)
                }
            } else if (live && ours) {
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
        if (!s.listed) continue;
        // what the file holds now: the objects that stayed, in the order they were, then those written anew, in the order the save wrote them
        std::vector<Item *> inFileNow, added;
        for (const std::shared_ptr<Item> &i : s.items) {
            const bool live = !i->deleted && !i->dropped;
            if (!live) {
                // (what the file now lacks stays away: a cell deleted is an object deleted)
                if (i->dropped) i->deleted = true;
                i->dropped = false;
                i->inFile = false;
                i->fileIndex = -1;
                continue;
            }
            const bool ours = i->object && i->object->isOurs;
            if (i->inFile && !(ours && !sameAnchor(i->anchor, i->fileAnchor))) inFileNow.push_back(i.get());
            else if (ours) added.push_back(i.get());
        }
        std::stable_sort(inFileNow.begin(), inFileNow.end(), [](const Item *a, const Item *b) { return a->fileIndex < b->fileIndex; });
        std::vector<Item *> order = inFileNow;
        order.insert(order.end(), added.begin(), added.end());
        // (read from the file as it is now, so that the places are the file's own)
        std::vector<SheetOleObject> found;
        OleReadState fresh;
        const SheetEntry &se = m_book->info().sheets[int(sheet)];
        if (!se.part.isEmpty() && !se.isNew) readSheetOleObjects(m_book->objectPackage(), se.part, &fresh, &found);
        for (size_t k = 0; k < order.size(); ++k) {
            Item &i = *order[k];
            if (k < found.size()) {
                i.inFile = true;
                i.fileIndex = found[k].fileIndex;
            } else {
                i.inFile = false;   // (not read back: it is written again by the next save rather than lost)
                i.fileIndex = -1;
            }
            i.fileObject = i.object;
            i.baseAnchor = i.fileAnchor = i.anchor;
            i.baseAt = 0;
        }
    }
}

} // namespace jp::crunch
