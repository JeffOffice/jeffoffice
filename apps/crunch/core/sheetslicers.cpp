#include "core/sheetslicers.h"

#include "core/slicerpaint.h"

#include <QImage>
#include <QPainter>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

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
        item->baseAnchor = item->anchor = d.anchor;
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
    const std::vector<StructEdit> &edits = m_edits ? m_edits() : none;
    const DrawingAnchor before = item.anchor;
    const bool droppedBefore = item.dropped;
    item.anchor = movedAnchor(item.baseAnchor, sheet, edits);
    item.dropped = !anchorSurvives(item.baseAnchor, sheet, edits);
    const bool moved = item.anchor.from.col != before.from.col || item.anchor.from.row != before.from.row || item.anchor.to.col != before.to.col || item.anchor.to.row != before.to.row;
    if (moved || droppedBefore != item.dropped) Q_EMIT changed(sheet, item.id);
}

void SheetSlicers::cellsMoved()
{
    for (size_t sheet = 0; sheet < m_sheets.size(); ++sheet)
        for (const std::shared_ptr<Item> &i : m_sheets[sheet].items) recompute(int(sheet), *i);
}

void SheetSlicers::saved()
{
    for (Sheet &s : m_sheets)
        for (const std::shared_ptr<Item> &i : s.items) i->baseAnchor = i->anchor;
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
    return !i || i->dropped || i->hidden;
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
    if (item.slicer == -2) item.slicer = m_names ? m_names(item.name, sheet) : -1;
    return item.slicer;
}

bool SheetSlicers::faceOf(int sheet, Item &item, SlicerFace *face)
{
    ++m_looks;
    const int slicer = slicerOf(sheet, item);
    if (slicer < 0 || !m_faces) {
        *face = SlicerFace();
        face->unusable = SlicerWhy::Missing;
        return false;
    }
    return m_faces(slicer, item.name, sheet, 0, kMaxSlicerButtons, face);
}

bool SheetSlicers::press(int sheet, int id, const QPointF &at, const QSizeF &size, bool extend)
{
    Item *found = find(sheet, id);
    if (!found) return false;
    Item &item = *found;
    SlicerFace face;
    if (!faceOf(sheet, item, &face)) return false;   // (a slicer that can't be used does nothing with a click, and does not take it from the cell under it)
    const SlicerGeometry g = slicerGeometry(size, face);
    const SlicerHit hit = slicerHit(g, face, at);
    if (hit.part == SlicerPart::Button) {
        Q_EMIT itemClicked(item.slicer, face.buttons[size_t(hit.index)].place, extend);
    } else if (hit.part == SlicerPart::More) {
        Q_EMIT moreClicked(item.slicer);
    } else if (hit.part == SlicerPart::Clear && face.filtered) {
        Q_EMIT clearClicked(item.slicer);
    }
    return true;
}

jp::grid::LayerObject SheetSlicers::layerObject(int sheet, const std::shared_ptr<Item> &item)
{
    jp::grid::LayerObject o;
    o.id = item->id;
    o.anchor = item->anchor;
    o.kind = ObjectKind::Other;
    o.name = item->name;
    o.fixed = true;
    o.hitInk = false;
    const std::weak_ptr<Item> weak = item;
    o.render = [this, sheet, weak](const jp::grid::RenderRequest &q) -> QImage {
        const std::shared_ptr<Item> i = weak.lock();
        if (!i || q.units.width() < 1 || q.units.height() < 1) return QImage();
        SlicerFace face;
        if (!faceOf(sheet, *i, &face) && face.caption.isEmpty()) face.caption = i->name;
        QImage image(q.pixels, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter p(&image);
        p.translate(q.inner.topLeft());
        p.scale(q.inner.width() / q.units.width(), q.inner.height() / q.units.height());
        paintSlicer(&p, QRectF(QPointF(0, 0), q.units), face);
        return image;
    };
    // (every click on a slicer is taken by it: it is not picked, so there is nothing yet to move or delete)
    o.operate = [](const QPointF &, const QSizeF &) { return true; };
    o.operateKeys = [this, sheet, weak](const QPointF &fraction, const QSizeF &units, Qt::KeyboardModifiers keys) {
        const std::shared_ptr<Item> i = weak.lock();
        if (!i) return false;
        return press(sheet, i->id, QPointF(fraction.x() * units.width(), fraction.y() * units.height()), units, keys.testFlag(Qt::ControlModifier));
    };
    return o;
}

std::vector<jp::grid::LayerObject> SheetSlicers::layerObjects(int sheet, bool forPrint)
{
    std::vector<jp::grid::LayerObject> out;
    if (sheet < 0 || size_t(sheet) >= m_sheets.size()) return out;
    for (const std::shared_ptr<Item> &i : m_sheets[size_t(sheet)].items) {
        if (i->dropped || i->hidden || (forPrint && !i->prints)) continue;
        out.push_back(layerObject(sheet, i));
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
