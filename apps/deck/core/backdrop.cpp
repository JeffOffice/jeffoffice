#include "core/backdrop.h"

#include "core/color.h"
#include "render/shapes.h"

#include <QPainterPath>
#include <algorithm>

namespace jp::deck {

namespace {

// The most objects looked through for all the points asked of one page (a hundred thousand objects times as many
// points would take minutes); past it, a point's backdrop is "not known" and the caller leaves things as they are.
constexpr qint64 kMaxLayerTests = 3000000;
// A fill this clear lets what is under it show through; clearer than this paints nothing.
constexpr double kSeeThrough = 0.1, kInvisible = 0.97;
// The colors kept of a backdrop that several clear fills lie over (their combinations would grow without end).
constexpr int kMaxColors = 8;

void keepColor(QVector<QColor> *colors, const QColor &c)
{
    for (const QColor &have : *colors)
        if (have.rgb() == c.rgb()) return;
    if (colors->size() < kMaxColors) colors->push_back(QColor(c.rgb()));
}

} // namespace

bool Backdrop::paints(const Fill &f)
{
    if (f.type == Fill::NoFill || f.transparency >= kInvisible) return false;
    return f.type != Fill::Solid || !f.color.isNone();
}

QVector<QColor> Backdrop::colorsOf(const Fill &f, bool *picture) const
{
    QVector<QColor> out;
    auto add = [&](const ColorRef &c) {
        if (c.kind() == ColorRef::None) return;
        const QColor v = c.resolve(m_d.colors);
        if (v.isValid()) keepColor(&out, v);
    };
    switch (f.type) {
    case Fill::Solid: add(f.color); break;
    case Fill::Gradient:
        if (f.stops.isEmpty()) {
            add(f.color);
            add(f.color2);
        } else {
            for (const GradientStop &s : f.stops) add(s.color);
        }
        break;
    case Fill::Pattern:
        add(f.color);
        add(f.color2);
        break;
    case Fill::Picture:
    case Fill::Texture: *picture = true; break;
    default: break;
    }
    return out;
}

Backdrop::Backdrop(const Document &d, const PageBase &page, const MasterPage *base) : m_d(d)
{
    // The masters under the page, bottom first, and how far down the page lets them show.
    QVector<const MasterPage *> chain;
    for (const MasterPage *m : d.masterChain(base)) chain.push_back(m);
    // The background: the page's own, else the nearest master's that has one.
    m_background = page.background;
    for (int i = int(chain.size()) - 1; i >= 0 && m_background.type == Fill::NoFill; --i) m_background = chain[i]->background;

    auto layer = [&](const ItemPtr &it) {
        Layer l;
        l.item = it.get();
        l.inverse = it->transform().inverted();
        l.box = it->bounds();
        l.frame = QRectF(QPointF(0, 0), it->rect.size());
        const bool picture = it->type() == ItemType::Picture && !static_cast<const PictureItem &>(*it).imageId.isEmpty();
        if (picture || it->type() == ItemType::Table || paints(it->fill)) m_layers.push_back(l);
    };
    const bool isMaster = base == &page;
    const int own = isMaster ? int(chain.size()) - 1 : int(chain.size());   // a master page's own objects are drawn last, in full
    if (page.showMasterItems && !chain.isEmpty()) {
        int lowest = own - 1;
        while (lowest > 0 && chain[lowest]->showMasterItems) --lowest;
        for (int i = std::max(lowest, 0); i < own; ++i)
            walkItems(chain[i]->items, [&](const ItemPtr &it) {
                if (!it->role.startsWith(QLatin1String("ph:"))) layer(it);
            });
    }
    walkItems(page.items, [&](const ItemPtr &it) {
        m_below.insert(it.get(), int(m_layers.size()));
        layer(it);
    });
}

bool Backdrop::covers(const Layer &layer, const QPointF &p, QPointF *local) const
{
    if (!layer.box.contains(p)) return false;
    *local = layer.inverse.map(p);
    if (!layer.frame.contains(*local)) return false;
    if (layer.item->type() == ItemType::Shape) {
        const auto &s = static_cast<const ShapeItem &>(*layer.item);
        if (!s.customPath.isEmpty()) return s.customPath.contains(*local);
        if (s.shape != QLatin1String("rect") && !s.shape.isEmpty()) return shapePath(s.shape, layer.frame.size(), s.adj).contains(*local);
    }
    return true;
}

const Fill *Backdrop::fillAt(const Layer &layer, const QPointF &local) const
{
    if (layer.item->type() != ItemType::Table) return &layer.item->fill;
    // The cell under the point: its own fill, else the table's.
    const auto &t = static_cast<const TableItem &>(*layer.item);
    if (t.rows > 0 && t.cols > 0 && t.colW.size() >= t.cols && t.rowH.size() >= t.rows && t.cells.size() >= t.rows * t.cols) {
        int r = 0, c = 0;
        double x = 0, y = 0;
        while (c < t.cols - 1 && local.x() >= x + t.colW[c]) x += t.colW[c++];
        while (r < t.rows - 1 && local.y() >= y + t.rowH[r]) y += t.rowH[r++];
        const TableCell &cell = t.cell(r, c);
        if (!cell.covered && paints(cell.fill)) return &cell.fill;
    }
    return paints(t.fill) ? &t.fill : nullptr;
}

Behind Backdrop::at(const Item *above, const QPointF &p, const Fill *own) const
{
    struct Paint {
        QVector<QColor> colors;
        double clear;
    };
    Behind out;
    QVector<Paint> over;   // clear paints seen so far, top first
    bool opaque = false;
    auto lay = [&](const Fill &f) {
        bool picture = false;
        Paint paint{colorsOf(f, &picture), f.transparency};
        if (picture) {
            out.picture = true;
            opaque = f.transparency < 0.5;
            return;
        }
        if (paint.colors.isEmpty()) return;
        over.push_back(paint);
        opaque = paint.clear < kSeeThrough;
    };
    if (own && paints(*own)) lay(*own);
    int top = int(m_layers.size());
    if (above) {
        const auto found = m_below.constFind(above);
        top = found == m_below.cend() ? top : found.value();
    }
    for (int i = top - 1; i >= 0 && !opaque && !out.picture; --i) {
        if (++m_work > kMaxLayerTests) {
            out.known = false;
            return out;
        }
        const Layer &l = m_layers[i];
        QPointF local;
        if (!covers(l, p, &local)) continue;
        if (l.item->type() == ItemType::Picture) {
            out.picture = true;
            break;
        }
        if (const Fill *f = fillAt(l, local)) lay(*f);
    }
    if (!opaque && !out.picture) {
        // The page's background at the bottom (white where it has none, as a slide is drawn).
        if (paints(m_background)) lay(m_background);
        if (!opaque && !out.picture) {
            over.push_back({{QColor(Qt::white)}, 0.0});
            opaque = true;
        }
    }
    if (out.picture) return out;   // a picture shows through whatever clear paint lies over it: its colors aren't told
    // Lay the clear paints over the one that stopped the search (the last), bottom up.
    out.colors = over.isEmpty() ? QVector<QColor>() : over.last().colors;
    for (int i = int(over.size()) - 2; i >= 0; --i) {
        QVector<QColor> mixed;
        for (const QColor &b : std::as_const(out.colors))
            for (const QColor &c : std::as_const(over[i].colors)) keepColor(&mixed, mix(b, c, 1 - over[i].clear));
        out.colors = mixed;
    }
    return out;
}

} // namespace jp::deck
