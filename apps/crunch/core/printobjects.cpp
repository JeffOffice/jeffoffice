#include "core/printobjects.h"

#include <algorithm>
#include <cmath>

namespace jp::crunch {

using jp::grid::Axis;
using jp::grid::LayerObject;
using jp::grid::ObjectBox;
using jp::grid::PageCut;
using jp::grid::RenderRequest;

PageObjectPainter::PageObjectPainter(std::vector<LayerObject> objects, const Axis &rows, const Axis &cols) : m_rows(rows), m_cols(cols)
{
    m_rows.setSkipped(0);
    m_cols.setSkipped(0);
    m_items.reserve(objects.size());
    for (LayerObject &o : objects) {
        Item item;
        item.box = jp::grid::anchorBox(o.anchor, m_rows, m_cols);
        item.object = std::move(o);
        m_items.push_back(std::move(item));
    }
}

std::vector<ObjectBox> PageObjectPainter::boxes() const
{
    std::vector<ObjectBox> out;
    for (const Item &i : m_items)
        if (i.object.thin ? !i.box.isNothing() : !i.box.isEmpty()) out.push_back(i.box);
    return out;
}

QImage PageObjectPainter::pictureOf(Item &item, const QSize &wanted, double zoom, const QRectF &inner)
{
    if (!item.image.isNull() && item.requested == wanted && item.zoom == zoom) return item.image;
    RenderRequest q;
    q.pixels = wanted;
    q.zoom = zoom;
    q.units = QSizeF(item.box.w, item.box.h);
    q.inner = inner;
    QImage image;
    if (item.object.prepare) {
        const std::function<QImage()> job = item.object.prepare(q);   // (a chart: what it needs is taken here, and the picture is made now)
        if (job) image = job();
    } else if (item.object.render) {
        image = item.object.render(q);
    }
    ++m_made;
    item.image = image;
    item.requested = wanted;
    item.zoom = zoom;
    return image;
}

void PageObjectPainter::trim()
{
    qint64 bytes = 0;
    for (const Item &i : m_items) bytes += i.image.isNull() ? 0 : i.image.sizeInBytes();
    if (bytes <= kMaxHeldBytes) return;
    std::vector<Item *> held;
    for (Item &i : m_items)
        if (!i.image.isNull()) held.push_back(&i);
    std::sort(held.begin(), held.end(), [](const Item *a, const Item *b) { return a->used < b->used; });
    for (Item *i : held) {
        if (bytes <= kMaxHeldBytes) break;
        bytes -= i->image.sizeInBytes();
        i->image = QImage();
    }
}

void PageObjectPainter::paint(QPainter *p, const PageCut &page, double zoom)
{
    if (m_items.empty() || page.row2 < page.row1 || page.col2 < page.col1) return;
    const qint64 bx = m_cols.position(page.col1), by = m_rows.position(page.row1);
    const qint64 bw = m_cols.position(page.col2 + 1) - bx, bh = m_rows.position(page.row2 + 1) - by;
    const qint64 tw = page.titleCol1 >= 0 ? m_cols.position(page.titleCol2 + 1) - m_cols.position(page.titleCol1) : 0;
    const qint64 th = page.titleRow1 >= 0 ? m_rows.position(page.titleRow2 + 1) - m_rows.position(page.titleRow1) : 0;
    const QRectF body(double(tw) * zoom, double(th) * zoom, double(bw) * zoom, double(bh) * zoom);
    ++m_tick;
    p->save();
    p->setClipRect(body);
    for (Item &item : m_items) {
        if (item.object.thin ? item.box.isNothing() : item.box.isEmpty()) continue;
        if (!item.settled) {
            item.settled = true;
            if (item.object.settle) item.object.settle(&item.object);
        }
        if (!item.object.render && !item.object.prepare) continue;   // (it has nothing to draw)
        const double m = std::max(0.0, item.object.margin);
        // does it reach this page's cells?
        if (item.box.x + item.box.w + m <= double(bx) || item.box.x - m >= double(bx + bw) || item.box.y + item.box.h + m <= double(by) || item.box.y - m >= double(by + bh)) continue;
        const QRectF box((item.box.x - double(bx) + double(tw)) * zoom, (item.box.y - double(by) + double(th)) * zoom, std::max(1.0, item.box.w * zoom), std::max(1.0, item.box.h * zoom));
        const int pad = m > 0 ? int(std::ceil(m * zoom)) : 0;
        const QRectF whole = box.adjusted(-pad, -pad, pad, pad);
        if (!whole.intersects(body)) continue;
        double w = std::ceil(whole.width()), h = std::ceil(whole.height());
        if (w * h > double(kMaxPixels)) {
            const double k = std::sqrt(double(kMaxPixels) / (w * h));
            w = std::floor(w * k);
            h = std::floor(h * k);
        }
        const QSize wanted(int(std::max(1.0, w)), int(std::max(1.0, h)));
        const double sx = double(wanted.width()) / whole.width(), sy = double(wanted.height()) / whole.height();
        const QRectF inner((box.x() - whole.x()) * sx, (box.y() - whole.y()) * sy, box.width() * sx, box.height() * sy);
        const QImage image = pictureOf(item, wanted, zoom, inner);
        item.used = m_tick;
        if (image.isNull()) continue;
        p->setRenderHint(QPainter::SmoothPixmapTransform, true);
        p->drawImage(whole, image);
    }
    p->restore();
    trim();
}

} // namespace jp::crunch
