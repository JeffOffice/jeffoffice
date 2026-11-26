#include "core/sheetslicers.h"

#include "core/slicerpaint.h"

#include "sheet/xlsxsave.h"

#include <QImage>
#include <QPainter>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

bool samePoint(const AnchorPoint &a, const AnchorPoint &b) { return a.col == b.col && a.row == b.row && std::llabs(a.colOff - b.colOff) <= kEmuPerPixel / 2 && std::llabs(a.rowOff - b.rowOff) <= kEmuPerPixel / 2; }
bool sameAnchor(const DrawingAnchor &a, const DrawingAnchor &b) { return samePoint(a.from, b.from) && samePoint(a.to, b.to); }

} // namespace

SheetSlicers::SheetSlicers(QObject *parent) : QObject(parent) {}

void SheetSlicers::open(const std::shared_ptr<Workbook> &book)
{
    m_book = book;
    m_sheets.clear();
    if (m_book) m_sheets.resize(size_t(std::max(0, int(m_book->info().sheets.size()))));
}

void SheetSlicers::setSources(FaceSource faces, NameSource names)
{
    m_faces = std::move(faces);
    m_names = std::move(names);
}

void SheetSlicers::list(int sheet, const QVector<DrawingSlicer> &found)
{
    if (sheet < 0) return;
    if (sheet >= int(m_sheets.size())) m_sheets.resize(size_t(sheet) + 1);
    Sheet &s = m_sheets[size_t(sheet)];
    if (s.listed) return;
    s.listed = true;
    int next = kSlicerIdBase;
    for (const DrawingSlicer &d : found) {
        auto item = std::make_shared<Item>();
        item->id = next++;
        item->name = d.name;
        item->baseAnchor = item->anchor = item->fileAnchor = d.anchor;
        item->ordinal = d.ordinal;
        item->hidden = d.hidden;
        item->prints = d.prints;
        s.items.push_back(item);
        recompute(sheet, *item);   // (the edits made before the sheet was listed)
    }
}

void SheetSlicers::recompute(int sheet, Item &item)
{
    static const std::vector<StructEdit> none;
    const std::vector<StructEdit> &all = m_edits ? m_edits() : none;
    const std::vector<StructEdit> from(all.begin() + std::min<size_t>(size_t(std::max(0, item.baseAt)), all.size()), all.end());
    const DrawingAnchor before = item.anchor;
    const bool droppedBefore = item.dropped;
    item.anchor = movedAnchor(item.baseAnchor, sheet, from);
    item.dropped = !anchorSurvives(item.baseAnchor, sheet, from);
    if (!sameAnchor(before, item.anchor) || droppedBefore != item.dropped) Q_EMIT changed(sheet, item.id);
}

void SheetSlicers::cellsMoved()
{
    for (size_t sheet = 0; sheet < m_sheets.size(); ++sheet)
        for (const std::shared_ptr<Item> &i : m_sheets[sheet].items) recompute(int(sheet), *i);
}

void SheetSlicers::saved()
{
    for (Sheet &s : m_sheets)
        for (const std::shared_ptr<Item> &i : s.items) {
            i->baseAnchor = i->fileAnchor = i->anchor;
            i->baseAt = 0;
            i->moved = false;
            i->ordinal = i->savedOrdinal;
            i->slicer = -2;   // (the slicers the file has are listed again: a place among them may have changed)
            if (i->deleted) i->inFile = false;
        }
}

const SheetSlicers::Item *SheetSlicers::find(int sheet, int id) const
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || !isSlicerId(id)) return nullptr;
    const std::vector<std::shared_ptr<Item>> &items = m_sheets[size_t(sheet)].items;
    const size_t at = size_t(id - kSlicerIdBase);   // (the ids run in order from the base)
    return at < items.size() && items[at]->id == id ? items[at].get() : nullptr;
}

int SheetSlicers::countOf(int sheet) const
{
    return sheet < 0 || size_t(sheet) >= m_sheets.size() ? 0 : int(m_sheets[size_t(sheet)].items.size());
}

int SheetSlicers::ordinalOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i ? i->ordinal : 0;
}

bool SheetSlicers::isGone(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return !i || i->dropped || i->hidden || i->deleted;
}

bool SheetSlicers::isDeleted(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i && i->deleted;
}

QString SheetSlicers::nameOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i ? i->name : QString();
}

int SheetSlicers::scrollRowOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i ? i->scrollRow : 0;
}

qint64 SheetSlicers::focusOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i && i->inside ? i->focus : -1;
}

int SheetSlicers::neighbor(int sheet, int from, bool backwards) const
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return -1;
    const std::vector<std::shared_ptr<Item>> &items = m_sheets[size_t(sheet)].items;
    int found = -1;
    for (size_t k = 0; k < items.size(); ++k) {
        const size_t at = backwards ? items.size() - 1 - k : k;
        const Item &i = *items[at];
        if (i.dropped || i.hidden || i.deleted) continue;
        if (from < 0) return i.id;
        if (found >= 0) return i.id;
        if (i.id == from) found = i.id;
    }
    return -1;
}

ObjectState SheetSlicers::stateOf(int sheet, int id, int epoch) const
{
    ObjectState st;
    const Item *i = find(sheet, id);
    if (!i) return st;
    st.placements.push_back(AnchorPlacement{i->baseAt, i->baseAnchor});
    st.deleted = i->deleted;
    st.epoch = epoch;
    return st;
}

ObjectState SheetSlicers::stateMoved(int sheet, int id, const DrawingAnchor &anchor, int epoch) const
{
    ObjectState st = stateOf(sheet, id, epoch);
    if (st.placements.empty()) return st;
    DrawingAnchor a = anchor;   // (as the layer gives it: a slicer that keeps its size when cells are resized is a place and a size, and is made two places when it is saved)
    a.editAs = find(sheet, id)->baseAnchor.editAs;
    st.placements.assign(1, AnchorPlacement{editCount(), a});
    return st;
}

void SheetSlicers::apply(int sheet, int id, const ObjectState &state, bool placed)
{
    Item *i = find(sheet, id);
    if (!i) return;
    if (placed && !state.placements.empty()) {
        i->baseAnchor = state.placements.back().anchor;
        i->baseAt = state.placements.back().at;
        i->moved = !sameAnchor(i->baseAnchor, i->fileAnchor) || i->baseAt != 0;
    }
    i->deleted = state.deleted;
    if (state.deleted) {
        i->inside = false;
        i->thumbHeld = false;
    }
    recompute(sheet, *i);
    Q_EMIT changed(sheet, id);
}

std::vector<int> SheetSlicers::removedOrdinals(int sheet) const
{
    std::vector<int> out;
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return out;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items)
        if (i->deleted && i->inFile) out.push_back(i->ordinal);
    return out;
}

void SheetSlicers::addToSave(int sheet, const QString &drawingPart, SaveChanges *changes, const std::vector<int> &removed, const std::function<DrawingAnchor(const DrawingAnchor &)> &twoCell)
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size() || drawingPart.isEmpty()) return;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items) {
        if (!i->inFile) continue;
        if (i->deleted) {
            changes->removedObjects[drawingPart].push_back(i->ordinal);
            changes->removedSlicers.emplace_back(sheet, i->name);
            continue;
        }
        i->savedOrdinal = i->ordinal - int(std::lower_bound(removed.begin(), removed.end(), i->ordinal) - removed.begin());
        if (i->moved && !i->dropped) {
            const DrawingAnchor a = twoCell ? twoCell(i->anchor) : i->anchor;
            if (a.kind == AnchorKind::TwoCell) changes->anchors[drawingPart][i->ordinal] = a;
        }
    }
}

DrawingAnchor SheetSlicers::anchorOf(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i ? i->anchor : DrawingAnchor();
}

void SheetSlicers::looksChanged(int sheet)
{
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items) Q_EMIT lookChanged(sheet, i->id);
}

int SheetSlicers::slicerOf(int sheet, Item &item)
{
    // (a slicer found is kept; "none" is asked again each time, as the workbook may not have been able to tell yet, and a click would then do nothing for good)
    if (item.slicer < 0) item.slicer = m_names ? m_names(item.name, sheet) : -1;
    return item.slicer;
}

bool SheetSlicers::faceOf(int sheet, Item &item, SlicerFace *face, const QSizeF &size, bool scrolls)
{
    ++m_looks;
    const int slicer = slicerOf(sheet, item);
    if (slicer < 0 || !m_faces) {
        *face = SlicerFace();
        face->unusable = SlicerWhy::Missing;
        return false;
    }
    // (only the buttons that show are asked for: the counts come first, then the window the frame has room for)
    auto window = [&](qint64 first) {
        SlicerFace counts;
        if (!m_faces(slicer, item.name, sheet, first, 0, &counts)) {
            *face = counts;
            return false;
        }
        const int room = size.width() >= 1 && size.height() >= 1 ? slicerGeometry(size, counts).capacity() : kMaxSlicerButtons;
        *face = counts;
        return m_faces(slicer, item.name, sheet, first, std::clamp(room, 1, kMaxSlicerButtons), face);
    };
    if (!scrolls) return window(0);
    // (the buttons from the row it is scrolled to; a row past the last that can be shown, or a different number of columns than was last known, asks again)
    int row = std::max(0, item.scrollRow);
    for (int pass = 0; pass < 2; ++pass) {
        if (!window(qint64(row) * item.columns)) return false;
        const bool columnsChanged = face->columns != item.columns;
        item.columns = face->columns;
        if (columnsChanged && pass == 0) continue;
        face->scrolls = true;
        face->first = qint64(row) * face->columns;
        if (size.width() >= 1 && size.height() >= 1) {
            const int most = slicerGeometry(size, *face).maxRow;
            if (row > most) {
                row = most;
                if (pass == 0) continue;
                face->first = qint64(row) * face->columns;
            }
        }
        break;
    }
    item.scrollRow = row;
    face->focus = item.inside ? item.focus : -1;
    return true;
}

void SheetSlicers::scrollTo(int sheet, Item &item, int row)
{
    row = std::max(0, row);
    if (row == item.scrollRow) return;
    item.scrollRow = row;
    Q_EMIT lookChanged(sheet, item.id);
}

bool SheetSlicers::press(int sheet, int id, const QPointF &at, const QSizeF &size, Qt::KeyboardModifiers keys)
{
    Item *found = find(sheet, id);
    if (!found) return false;
    Item &item = *found;
    item.size = size;
    SlicerFace face;
    if (!faceOf(sheet, item, &face, size, true)) return false;   // (a slicer that can't be used has no buttons: a press on it picks it)
    const SlicerGeometry g = slicerGeometry(size, face);
    const SlicerHit hit = slicerHit(g, face, at);
    const bool extend = keys.testFlag(Qt::ControlModifier);
    switch (hit.part) {
    case SlicerPart::Button: {
        const qint64 index = face.first + hit.index;
        if (item.inside) {
            item.focus = index;
            Q_EMIT lookChanged(sheet, id);
        }
        if (keys.testFlag(Qt::ShiftModifier) && item.anchorAt >= 0 && item.anchorAt != index) {
            const qint64 from = std::min(item.anchorAt, index), to = std::max(item.anchorAt, index);
            SlicerFace run;
            if (m_faces && m_faces(item.slicer, item.name, sheet, from, int(std::min<qint64>(to - from + 1, kMaxSlicerRange)), &run)) {
                QVector<int> places;
                for (const SlicerButton &b : run.buttons) places.append(b.place);
                Q_EMIT runClicked(item.slicer, places, extend);
                return true;
            }
        }
        item.anchorAt = index;
        Q_EMIT itemClicked(item.slicer, face.buttons[size_t(hit.index)].place, extend);
        return true;
    }
    case SlicerPart::More: Q_EMIT moreClicked(item.slicer); return true;
    case SlicerPart::Clear:
        if (!face.filtered) return false;
        Q_EMIT clearClicked(item.slicer);
        return true;
    case SlicerPart::Thumb:
        item.thumbHeld = true;
        item.thumbGrab = at.y() - g.thumb.top();
        return true;
    case SlicerPart::BarAbove: scrollTo(sheet, item, item.scrollRow - g.rows); return true;
    case SlicerPart::BarBelow: scrollTo(sheet, item, std::min(g.maxRow, item.scrollRow + g.rows)); return true;
    case SlicerPart::Header:
    case SlicerPart::None: break;
    }
    return false;
}

void SheetSlicers::dragged(int sheet, int id, const QPointF &at, const QSizeF &size, bool done)
{
    Item *found = find(sheet, id);
    if (!found || !found->thumbHeld) return;
    Item &item = *found;
    if (done) item.thumbHeld = false;
    SlicerFace face;
    if (!faceOf(sheet, item, &face, size, true)) return;
    const SlicerGeometry g = slicerGeometry(size, face);
    if (g.bar.isNull()) return;
    scrollTo(sheet, item, slicerRowAtThumb(g, at.y() - item.thumbGrab, 0));
}

bool SheetSlicers::wheeled(int sheet, int id, double steps, const QSizeF &size)
{
    Item *found = find(sheet, id);
    if (!found || steps == 0) return false;
    Item &item = *found;
    SlicerFace face;
    if (!faceOf(sheet, item, &face, size, true)) return false;
    const SlicerGeometry g = slicerGeometry(size, face);
    if (g.bar.isNull()) return false;   // (every button is there: the sheet scrolls)
    // (a notch is three rows; a smaller turn adds up until it is a row)
    item.wheelRest += steps * 3;
    const int whole = int(item.wheelRest);
    item.wheelRest -= whole;
    const int row = std::clamp(item.scrollRow - whole, 0, g.maxRow);
    if (row != item.scrollRow - whole) item.wheelRest = 0;   // (at an end: what is left of the turn is not kept)
    scrollTo(sheet, item, row);
    return true;
}

bool SheetSlicers::isInside(int sheet, int id) const
{
    const Item *i = find(sheet, id);
    return i && i->inside;
}

void SheetSlicers::enter(int sheet, int id)
{
    Item *i = find(sheet, id);
    if (!i || i->deleted || i->dropped || i->hidden) return;
    i->inside = true;
    if (i->focus < 0) i->focus = qint64(i->scrollRow) * i->columns;
    Q_EMIT lookChanged(sheet, id);
}

void SheetSlicers::leave(int sheet, int id)
{
    Item *i = find(sheet, id);
    if (!i || !i->inside) return;
    i->inside = false;
    Q_EMIT lookChanged(sheet, id);
}

bool SheetSlicers::key(int sheet, int id, QKeyEvent *event)
{
    Item *found = find(sheet, id);
    if (!found || found->deleted) return false;
    Item &item = *found;
    const int k = event->key();
    const Qt::KeyboardModifiers mods = event->modifiers();
    const bool enter = k == Qt::Key_Return || k == Qt::Key_Enter;
    if (k == Qt::Key_F6 && (mods == Qt::NoModifier || mods == Qt::ShiftModifier)) {
        Q_EMIT neighborWanted(mods == Qt::ShiftModifier);   // (on to the next slicer: the window does it, and the layer does not let go of this one first)
        return true;
    }
    if (!item.inside) {
        if (!enter || (mods & ~Qt::KeypadModifier)) return false;
        this->enter(sheet, id);
        return true;
    }
    if (k == Qt::Key_Escape) {
        leave(sheet, id);
        return true;
    }
    SlicerFace face;
    if (!faceOf(sheet, item, &face, item.size, true) || face.unusable != SlicerWhy::None) return false;
    const qint64 total = std::max<qint64>(face.itemCount, 1), columns = std::max(1, face.columns);
    const SlicerGeometry g = slicerGeometry(item.size, face);
    auto goTo = [&](qint64 index) {
        index = std::clamp<qint64>(index, 0, total - 1);
        item.focus = index;
        const int row = int(index / columns);
        if (row < item.scrollRow) item.scrollRow = row;
        else if (g.rows > 0 && row >= item.scrollRow + g.rows) item.scrollRow = row - g.rows + 1;
        Q_EMIT lookChanged(sheet, item.id);
        return true;
    };
    const qint64 at = std::max<qint64>(item.focus, 0);
    switch (k) {
    case Qt::Key_Left: return goTo(at - 1);
    case Qt::Key_Right: return goTo(at + 1);
    case Qt::Key_Up: return goTo(at - columns);
    case Qt::Key_Down: return goTo(at + columns);
    case Qt::Key_Home: return goTo(0);
    case Qt::Key_End: return goTo(total - 1);
    case Qt::Key_PageUp: return goTo(at - std::max(1, g.rows) * columns);
    case Qt::Key_PageDown: return goTo(at + std::max(1, g.rows) * columns);
    case Qt::Key_C:
        if (!(mods & Qt::AltModifier) || !(mods & Qt::ControlModifier)) return false;   // (Alt and a letter alone belong to the ribbon's key letters)
        if (!event->text().isEmpty() && event->text().front().isPrint() && event->text().front().toUpper() != QChar('C')) return false;   // (a keyboard where Ctrl and Alt together, AltGr, type a letter: that is typing, not the shortcut)
        if (face.filtered) Q_EMIT clearClicked(item.slicer);
        return true;
    case Qt::Key_Delete: return true;   // (the buttons have the keys: Delete is for a slicer picked as an object, after Esc)
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        SlicerFace one;
        if (!m_faces || !m_faces(item.slicer, item.name, sheet, at, 1, &one) || one.buttons.empty()) return true;
        const bool extend = mods.testFlag(Qt::ControlModifier);
        if (mods.testFlag(Qt::ShiftModifier) && item.anchorAt >= 0 && item.anchorAt != at) {
            const qint64 from = std::min(item.anchorAt, at), to = std::max(item.anchorAt, at);
            SlicerFace run;
            if (m_faces(item.slicer, item.name, sheet, from, int(std::min<qint64>(to - from + 1, kMaxSlicerRange)), &run)) {
                QVector<int> places;
                for (const SlicerButton &b : run.buttons) places.append(b.place);
                Q_EMIT runClicked(item.slicer, places, extend);
                return true;
            }
        }
        item.anchorAt = at;
        Q_EMIT itemClicked(item.slicer, one.buttons[0].place, extend);
        return true;
    }
    default: break;
    }
    return false;
}

jp::grid::LayerObject SheetSlicers::layerObject(int sheet, const std::shared_ptr<Item> &item, bool forPrint)
{
    jp::grid::LayerObject o;
    o.id = item->id;
    o.anchor = item->anchor;
    o.kind = ObjectKind::Other;
    o.name = item->name;
    o.hitInk = false;
    const std::weak_ptr<Item> weak = item;
    o.render = [this, sheet, weak, forPrint](const jp::grid::RenderRequest &q) -> QImage {
        const std::shared_ptr<Item> i = weak.lock();
        if (!i || q.units.width() < 1 || q.units.height() < 1) return QImage();
        SlicerFace face;
        if (!forPrint) i->size = q.units;
        if (!faceOf(sheet, *i, &face, q.units, !forPrint) && face.caption.isEmpty()) face.caption = i->name;
        face.printed = forPrint;
        QImage image(q.pixels, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter p(&image);
        p.translate(q.inner.topLeft());
        p.scale(q.inner.width() / q.units.width(), q.inner.height() / q.units.height());
        paintSlicer(&p, QRectF(QPointF(0, 0), q.units), face);
        return image;
    };
    if (forPrint) return o;
    auto at = [](const QPointF &fraction, const QSizeF &units) { return QPointF(fraction.x() * units.width(), fraction.y() * units.height()); };
    o.operateKeys = [this, sheet, weak, at](const QPointF &fraction, const QSizeF &units, Qt::KeyboardModifiers keys) {
        const std::shared_ptr<Item> i = weak.lock();
        return i && press(sheet, i->id, at(fraction, units), units, keys);
    };
    o.worksAt = [this, sheet, weak, at](const QPointF &fraction, const QSizeF &units) {
        const std::shared_ptr<Item> i = weak.lock();
        if (!i) return false;
        SlicerFace face;
        if (!faceOf(sheet, *i, &face, units, true) || face.unusable != SlicerWhy::None) return false;
        const SlicerPart part = slicerHit(slicerGeometry(units, face), face, at(fraction, units)).part;
        return part != SlicerPart::None && part != SlicerPart::Header && (part != SlicerPart::Clear || face.filtered);
    };
    o.wheel = [this, sheet, weak](const QPointF &, const QSizeF &units, double steps) {
        const std::shared_ptr<Item> i = weak.lock();
        return i && wheeled(sheet, i->id, steps, units);
    };
    o.dragWork = [this, sheet, weak, at](const QPointF &fraction, const QSizeF &units, bool done) {
        if (const std::shared_ptr<Item> i = weak.lock()) dragged(sheet, i->id, at(fraction, units), units, done);
    };
    o.keys = [this, sheet, weak](QKeyEvent *event) {
        const std::shared_ptr<Item> i = weak.lock();
        return i && key(sheet, i->id, event);
    };
    return o;
}

std::vector<jp::grid::LayerObject> SheetSlicers::layerObjects(int sheet, bool forPrint)
{
    std::vector<jp::grid::LayerObject> out;
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return out;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items) {
        if (i->dropped || i->hidden || (forPrint && !i->prints)) continue;
        out.push_back(layerObject(sheet, i, forPrint));
    }
    return out;
}

std::optional<jp::grid::LayerObject> SheetSlicers::layerObjectOf(int sheet, int id)
{
    const Item *found = find(sheet, id);
    if (!found || found->dropped || found->hidden) return std::nullopt;
    const std::vector<std::shared_ptr<Item>> &items = m_sheets[size_t(sheet)].items;
    return layerObject(sheet, items[size_t(id - kSlicerIdBase)]);
}

} // namespace jp::crunch
