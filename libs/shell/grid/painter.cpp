#include "grid/painter.h"

#include "sheet/cellref.h"

#include <QGlyphRun>
#include <QImage>
#include <QLinearGradient>
#include <QPixmap>
#include <QPolygonF>
#include <QRawFont>

#include <algorithm>
#include <cmath>
#include <memory>

namespace jp::grid {

using jp::office::sheet::CellType;
using jp::office::sheet::columnName;

namespace {

constexpr int kMaxScanSteps = 8192;     // rows or columns stepped over to find those that have size (rows too small to show have none)

// How heavy a line is where two cells' lines meet: the heavier is drawn.
int weight(BorderStyle s)
{
    switch (s) {
    case BorderStyle::None: return 0;
    case BorderStyle::Hair: return 1;
    case BorderStyle::Dotted: return 2;
    case BorderStyle::DashDotDot: return 3;
    case BorderStyle::DashDot: return 4;
    case BorderStyle::Dashed: return 5;
    case BorderStyle::Thin: return 6;
    case BorderStyle::MediumDashDotDot: return 7;
    case BorderStyle::SlantDashDot: return 8;
    case BorderStyle::MediumDashDot: return 9;
    case BorderStyle::MediumDashed: return 10;
    case BorderStyle::Medium: return 11;
    case BorderStyle::Double: return 12;
    case BorderStyle::Thick: return 13;
    }
    return 0;
}

const LineLook kNoLine;

const LineLook &heavier(const LineLook *a, const LineLook *b)
{
    const LineLook &x = a ? *a : kNoLine, &y = b ? *b : kNoLine;
    return weight(y.style) > weight(x.style) ? y : x;
}

// A pattern's 8 by 8 tile: which pixels are the foreground.
bool inPattern(PatternKind kind, int x, int y)
{
    switch (kind) {
    case PatternKind::MediumGray: return ((x + y) & 1) == 0;
    case PatternKind::DarkGray: return !((x & 1) && (y & 1));
    case PatternKind::LightGray: return (x & 1) == 0 && (y & 1) == 0;
    case PatternKind::Gray125: return (x & 3) == 0 && (y & 1) == 0;
    case PatternKind::Gray0625: return (x & 3) == 0 && (y & 3) == 0;
    case PatternKind::DarkHorizontal: return (y & 3) < 2;
    case PatternKind::DarkVertical: return (x & 3) < 2;
    case PatternKind::DarkDown: return ((x - y + 8) & 3) < 2;
    case PatternKind::DarkUp: return ((x + y) & 3) < 2;
    case PatternKind::DarkGrid: return (x & 3) < 2 || (y & 3) < 2;
    case PatternKind::DarkTrellis: return ((x + y) & 3) < 2 || ((x - y + 8) & 3) < 2;
    case PatternKind::LightHorizontal: return (y & 3) == 0;
    case PatternKind::LightVertical: return (x & 3) == 0;
    case PatternKind::LightDown: return ((x - y + 8) & 3) == 0;
    case PatternKind::LightUp: return ((x + y) & 3) == 0;
    case PatternKind::LightGrid: return (x & 3) == 0 || (y & 3) == 0;
    case PatternKind::LightTrellis: return ((x + y) & 3) == 0 || ((x - y + 8) & 3) == 0;
    case PatternKind::None:
    case PatternKind::Solid: return true;
    }
    return true;
}

} // namespace

// ---- text as glyphs
// A cell's text is a line of letters in one font, which a spreadsheet draws without kerning or ligatures; the glyphs of the letters of Latin script (up to U+00FF) are
// looked up once in the font, and a line of them is put together and drawn as a run of glyphs, without the shaping that the layout of text does for every script (a text of
// any other letters takes that way). The width of a line is the sum of its glyphs' advances, the same in the meter and in the drawing.
class GridPainter::GlyphFace {
public:
    explicit GlyphFace(const QFont &font) : m_font(font), m_raw(QRawFont::fromFont(font)), m_metrics(font)
    {
        for (int i = 0; i < 256; ++i) m_known[i] = false;
    }
    const QFont &font() const { return m_font; }
    const QRawFont &raw() const { return m_raw; }
    double ascent() const { return m_metrics.ascent(); }
    double descent() const { return m_metrics.descent(); }
    double lineSpacing() const { return m_metrics.lineSpacing(); }
    double underlinePos() const { return m_metrics.underlinePos(); }
    double lineWidth() const { return m_metrics.lineWidth(); }
    // The glyphs and where each begins (from 0), and the width of the line; false if a letter of the line is not one of the simple ones.
    bool layout(const QString &text, QList<quint32> *glyphs, QList<QPointF> *positions, double *width) const
    {
        glyphs->reserve(text.size());
        positions->reserve(text.size());
        double x = 0;
        for (const QChar c : text) {
            const ushort u = c.unicode();
            if (u < 0x20 || u > 0xFF || (u >= 0x7F && u < 0xA0)) return false;
            if (!m_known[u]) {
                const QList<quint32> g = m_raw.glyphIndexesForString(QString(c));
                if (g.size() != 1) return false;
                m_glyph[u] = g.first();
                m_advance[u] = m_raw.advancesForGlyphIndexes(g, QRawFont::UseDesignMetrics).first().x();
                m_known[u] = true;
            }
            glyphs->append(m_glyph[u]);
            positions->append(QPointF(x, 0));
            x += m_advance[u];
        }
        *width = x;
        return true;
    }
    // The width of a text: from the glyphs when they are simple, else from the layout.
    double width(const QString &text) const
    {
        double w = 0;
        double sum = 0;
        for (const QChar c : text) {
            const ushort u = c.unicode();
            if (u < 0x20 || u > 0xFF || (u >= 0x7F && u < 0xA0)) return m_metrics.horizontalAdvance(text);
            if (!m_known[u]) {
                const QList<quint32> g = m_raw.glyphIndexesForString(QString(c));
                if (g.size() != 1) return m_metrics.horizontalAdvance(text);
                m_glyph[u] = g.first();
                m_advance[u] = m_raw.advancesForGlyphIndexes(g, QRawFont::UseDesignMetrics).first().x();
                m_known[u] = true;
            }
            sum += m_advance[u];
        }
        w = sum;
        return w;
    }

private:
    QFont m_font;
    QRawFont m_raw;
    QFontMetricsF m_metrics;
    mutable quint32 m_glyph[256];
    mutable double m_advance[256];
    mutable bool m_known[256];
};

// ---- measuring text, for the model

class GridPainter::QtMeter : public TextMeter {
public:
    double width(const QString &text, const CellLook &look) const override
    {
        if (text.isEmpty()) return 0;
        return face(look)->width(text);
    }
    double lineHeight(const CellLook &look) const override { return face(look)->lineSpacing(); }
    void reset() { m_faces.clear(); }

private:
    const GridPainter::GlyphFace *face(const CellLook &look) const
    {
        const size_t id = look.fontId;
        if (id >= m_faces.size()) m_faces.resize(id + 1);
        std::unique_ptr<GridPainter::GlyphFace> &e = m_faces[id];
        if (!e) {
            QFont f(look.family.isEmpty() ? QFont().family() : look.family);
            f.setPixelSize(std::max(1, int(std::lround(look.pointSize * 96.0 / 72.0 * (look.vertAlign ? 0.65 : 1.0)))));
            f.setBold(look.bold);
            f.setItalic(look.italic);
            f.setKerning(false);
            f.setHintingPreference(QFont::PreferNoHinting);
            e = std::make_unique<GridPainter::GlyphFace>(f);
        }
        return e.get();
    }
    mutable std::vector<std::unique_ptr<GridPainter::GlyphFace>> m_faces;
};

GridPainter::GridPainter(GridSheet *sheet) : m_sheet(sheet), m_meter(new QtMeter)
{
    m_sheet->setMeter(m_meter);
}

GridPainter::~GridPainter() { delete m_meter; }

const TextMeter &GridPainter::meter() const { return *m_meter; }

void GridPainter::resetFonts()
{
    m_fontMade.assign(m_fontMade.size(), false);
    m_fontZoom = 0;
    m_meter->reset();
    m_sheet->invalidateTexts();
}

const QFont &GridPainter::fontFor(const CellLook &look)
{
    const size_t id = look.fontId;
    if (id >= m_fonts.size()) {
        m_fonts.resize(id + 1);
        m_fontMade.resize(id + 1, false);
    }
    if (!m_fontMade[id]) {
        QFont f(look.family.isEmpty() ? QFont().family() : look.family);
        f.setPixelSize(std::max(1, int(std::lround(look.pointSize * 96.0 / 72.0 * m_sheet->layout().zoom() * (look.vertAlign ? 0.65 : 1.0)))));
        f.setBold(look.bold);
        f.setItalic(look.italic);
        f.setStrikeOut(look.strike);
        f.setUnderline(look.underline == Underline::Single);   // (the other kinds are drawn as lines of their own: see drawText)
        f.setKerning(false);
        f.setStyleStrategy(QFont::PreferAntialias);
        f.setHintingPreference(QFont::PreferNoHinting);
        m_fonts[id] = f;
        m_fontMade[id] = true;
        if (id < m_faces.size()) m_faces[id].reset();
    }
    return m_fonts[id];
}

QBrush GridPainter::patternBrush(PatternKind kind, QRgb fg, QRgb bg)
{
    const quint64 key = (quint64(kind) << 56) ^ (quint64(fg & 0xFFFFFF) << 28) ^ quint64(bg & 0xFFFFFF);
    const auto it = m_patterns.constFind(key);
    if (it != m_patterns.constEnd()) return it.value();
    QImage tile(8, 8, QImage::Format_RGB32);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) tile.setPixel(x, y, inPattern(kind, x, y) ? (fg | 0xFF000000u) : (bg | 0xFF000000u));
    if (m_patterns.size() > 512) m_patterns.clear();
    const QBrush brush(QPixmap::fromImage(tile));
    m_patterns.insert(key, brush);
    return brush;
}

// ---- drawing a cell's parts

void GridPainter::drawFill(QPainter *p, const QRect &rect, const CellLook &look, const PaintOptions &)
{
    if (look.pattern == PatternKind::None || m_mono) return;
    ++m_stats.fills;
    if (look.pattern == PatternKind::Solid) {
        p->fillRect(rect, QColor::fromRgba(look.fg));
        return;
    }
    // (the pattern begins at the cell's corner, so it is the same wherever the cell is on the screen)
    p->setBrushOrigin(rect.topLeft());
    p->fillRect(rect, patternBrush(look.pattern, look.fg, look.bg));
}

// A border on the edge that begins at (x, y) and goes `length` pixels, down (vertical) or right. A line's own pixels lie on the side of the edge before it (the last pixel of
// the cell above or to the left, where a gridline is), thicker ones reaching over it.
void GridPainter::drawEdge(QPainter *p, const LineLook &line, int x, int y, int length, bool vertical, const QRect &clip)
{
    if (line.style == BorderStyle::None || length <= 0) return;
    ++m_stats.borderSegments;
    const QColor color = m_mono ? QColor(Qt::black) : QColor::fromRgba(line.color);
    int thickness = 1;
    int first = -1;      // where its first pixel is, from the edge
    QList<qreal> dashes;
    bool two = false;
    switch (line.style) {
    case BorderStyle::Thin: break;
    case BorderStyle::Hair: dashes = {1, 1}; break;
    case BorderStyle::Dotted: dashes = {1, 1}; break;
    case BorderStyle::Dashed: dashes = {3, 1}; break;
    case BorderStyle::DashDot: dashes = {3, 1, 1, 1}; break;
    case BorderStyle::DashDotDot: dashes = {3, 1, 1, 1, 1, 1}; break;
    case BorderStyle::Medium: thickness = 2; break;
    case BorderStyle::MediumDashed: thickness = 2; dashes = {3, 1}; break;
    case BorderStyle::MediumDashDot: thickness = 2; dashes = {3, 1, 1, 1}; break;
    case BorderStyle::MediumDashDotDot: thickness = 2; dashes = {3, 1, 1, 1, 1, 1}; break;
    case BorderStyle::SlantDashDot: thickness = 2; dashes = {3, 1, 1, 1}; break;
    case BorderStyle::Thick: thickness = 3; first = -2; break;
    case BorderStyle::Double: two = true; thickness = 1; break;
    case BorderStyle::None: return;
    }
    // Only the part of the edge that is in the clip is drawn: along the edge, from `lo` to `hi` (the edge may be as long as a range of a million rows), and across it, the edge's own pixels.
    const int along = vertical ? y : x, alongLo = vertical ? clip.top() : clip.left(), alongHi = vertical ? clip.bottom() + 1 : clip.right() + 1;
    const int across = vertical ? x : y, acrossLo = vertical ? clip.left() : clip.top(), acrossHi = vertical ? clip.right() + 1 : clip.bottom() + 1;
    const qint64 lo = std::max<qint64>(0, qint64(alongLo) - along), hi = std::min<qint64>(length, qint64(alongHi) - along);
    if (lo >= hi) return;
    const int reach = two ? 3 : std::abs(first) + thickness;   // (the pixels of the line lie within this of the edge)
    if (across + 2 < acrossLo - reach || across - reach > acrossHi) return;
    const int from = int(lo), size = int(hi - lo);
    auto rect = [&](int offset, int width) { return vertical ? QRect(x + offset, y + from, width, size) : QRect(x + from, y + offset, size, width); };
    if (two) {
        p->fillRect(rect(-2, 1), color);
        p->fillRect(rect(0, 1), color);
        return;
    }
    if (thickness == 2) first = -1;
    if (dashes.isEmpty()) {
        p->fillRect(rect(first, thickness), color);
        return;
    }
    // a dashed line: dashes of the pattern's lengths along the edge, from the whole pattern that the part in the clip begins in
    int period = 0;
    for (qreal d : dashes) period += int(d);
    if (m_strokedDashes) {
        // (one stroke: the pen's pattern is in widths of the pen, and begins at the stroke's start, so it is moved on by what the clip cut off)
        QPen pen(color, thickness, Qt::CustomDashLine, Qt::FlatCap);
        QList<qreal> pattern;
        for (qreal d : dashes) pattern.append(d / thickness);
        pen.setDashPattern(pattern);
        pen.setDashOffset(double(lo % period) / thickness);
        ++m_stats.dashSteps;
        p->save();
        p->setPen(pen);
        const double across0 = (vertical ? x : y) + first + thickness / 2.0;
        if (vertical) p->drawLine(QPointF(across0, y + from), QPointF(across0, y + from + size));
        else p->drawLine(QPointF(x + from, across0), QPointF(x + from + size, across0));
        p->restore();
        return;
    }
    qint64 at = (lo / period) * period;
    size_t step = 0;
    while (at < hi) {
        ++m_stats.dashSteps;
        const int run = int(dashes[int(step % size_t(dashes.size()))]);
        if (step % 2 == 0) {
            const qint64 begin = std::max(at, lo), end = std::min(at + run, hi);
            if (end > begin) p->fillRect(vertical ? QRect(x + first, y + int(begin), thickness, int(end - begin)) : QRect(x + int(begin), y + first, int(end - begin), thickness), color);
        }
        at += run;
        ++step;
    }
}

void GridPainter::drawBar(QPainter *p, const QRect &cell, const CondBar &bar, bool gridlines)
{
    const QRect inner = cell.adjusted(2, 2, -2 - (gridlines ? 1 : 0), -2 - (gridlines ? 1 : 0));
    if (inner.width() < 2 || inner.height() < 2) return;
    const int x0 = inner.left() + int(std::lround(bar.from * inner.width())), x1 = inner.left() + int(std::lround(bar.to * inner.width()));
    const QRect r(std::min(x0, x1), inner.top(), std::max(1, std::abs(x1 - x0)), inner.height());
    ++m_stats.bars;
    if (bar.gradient) {
        // solid at the end that is at the axis, and lighter at the far end
        QLinearGradient g(bar.negative ? r.right() + 1 : r.left(), 0, bar.negative ? r.left() : r.right() + 1, 0);
        g.setColorAt(0, QColor::fromRgba(bar.fill));
        g.setColorAt(1, QColor::fromRgba(bar.fillEnd));
        p->fillRect(r, g);
    } else {
        p->fillRect(r, QColor::fromRgba(bar.fill));
    }
    if (bar.hasBorder) {
        p->setPen(QColor::fromRgba(bar.border));
        p->setBrush(Qt::NoBrush);
        p->drawRect(r.adjusted(0, 0, -1, -1));
    }
    if (bar.hasAxis) {
        const int ax = inner.left() + int(std::lround(bar.axisAt * inner.width()));
        p->fillRect(QRect(ax, inner.top() - 1, 1, inner.height() + 2), QColor::fromRgba(bar.axis));
    }
}

// An icon of a set, in a square of `size` pixels at the left of the cell and in the middle of its height.
void GridPainter::drawIcon(QPainter *p, const QRect &cell, const QString &set, int index, int count, int size)
{
    ++m_stats.icons;
    const QRectF box(cell.left() + 2, cell.top() + (cell.height() - size) / 2.0, size, size);
    const QColor green(0x2E, 0xA0, 0x4F), yellow(0xE8, 0xB4, 0x1C), red(0xD6, 0x3E, 0x3E), gray(0x76, 0x76, 0x76), black(0x20, 0x20, 0x20), gold(0xE3, 0xA8, 0x0E);
    const bool gray3 = set.contains(QLatin1String("Gray"));
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setPen(Qt::NoPen);
    if (m_sheet->rightToLeft()) {
        // (the grid is drawn mirrored; the icon is not a thing that reads, so it is turned back)
        p->translate(box.center().x(), 0);
        p->scale(-1, 1);
        p->translate(-box.center().x(), 0);
    }
    const double cx = box.center().x(), cy = box.center().y(), h = box.width() / 2.0 - 0.5;
    // three-color sets run from the lowest (red) up (green); the four- and five-icon ones through yellow between
    auto colorAt = [&]() -> QColor {
        if (set.startsWith(QLatin1String("4RedToBlack"))) return index == 0 ? black : index == 1 ? QColor(0x70, 0x70, 0x70) : index == 2 ? QColor(0xE9, 0x8A, 0x8A) : red;
        if (set.startsWith(QLatin1String("4TrafficLights"))) return index == 0 ? black : index == 1 ? red : index == 2 ? yellow : green;
        if (gray3) return gray;
        if (count == 3) return index == 0 ? red : index == 1 ? yellow : green;
        if (count == 4) return index == 0 ? red : index == 3 ? green : yellow;
        return index == 0 ? red : index == count - 1 ? green : yellow;
    };
    const QColor c = colorAt();
    p->setBrush(c);
    if (set.contains(QLatin1String("Arrows"))) {
        // an arrow turned by where in the set it is: down, down and across, across, up and across, up
        const double turn = count == 3 ? (index - 1) * 90.0 : count == 4 ? (index - 1.5) * 60.0 : (index - 2) * 45.0;
        p->translate(cx, cy);
        p->rotate(-turn);   // 0 across to the right, positive up
        QPolygonF arrow;
        arrow << QPointF(h, 0) << QPointF(0, -h * 0.9) << QPointF(0, -h * 0.4) << QPointF(-h, -h * 0.4) << QPointF(-h, h * 0.4) << QPointF(0, h * 0.4) << QPointF(0, h * 0.9);
        p->drawPolygon(arrow);
    } else if (set.contains(QLatin1String("Triangles"))) {
        QPolygonF t;
        if (index == 1) t << QPointF(cx - h, cy - h * 0.2) << QPointF(cx + h, cy - h * 0.2) << QPointF(cx + h, cy + h * 0.2) << QPointF(cx - h, cy + h * 0.2);
        else if (index == 2) t << QPointF(cx, cy - h) << QPointF(cx + h, cy + h) << QPointF(cx - h, cy + h);
        else t << QPointF(cx - h, cy - h) << QPointF(cx + h, cy - h) << QPointF(cx, cy + h);
        p->drawPolygon(t);
    } else if (set.contains(QLatin1String("Flags"))) {
        p->setBrush(black);
        p->drawRect(QRectF(box.left() + 1, box.top() + 1, 1.5, box.height() - 2));
        p->setBrush(c);
        QPolygonF f;
        f << QPointF(box.left() + 2.5, box.top() + 1) << QPointF(box.right() - 1, box.top() + 3.5) << QPointF(box.left() + 2.5, box.top() + 7);
        p->drawPolygon(f);
    } else if (set.contains(QLatin1String("Signs"))) {
        if (index == 0) {
            QPolygonF d;
            d << QPointF(cx, cy - h) << QPointF(cx + h, cy) << QPointF(cx, cy + h) << QPointF(cx - h, cy);
            p->drawPolygon(d);
        } else if (index == 1) {
            QPolygonF t;
            t << QPointF(cx, cy - h) << QPointF(cx + h, cy + h) << QPointF(cx - h, cy + h);
            p->drawPolygon(t);
        } else {
            p->drawEllipse(QPointF(cx, cy), h, h);
        }
    } else if (set.contains(QLatin1String("Symbols"))) {
        const bool bare = set.contains(QLatin1String("Symbols2"));
        if (!bare) p->drawEllipse(QPointF(cx, cy), h, h);
        QPen pen(bare ? c : QColor(Qt::white), std::max(1.2, h * 0.28), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        const double a = h * 0.5;
        if (index == 0) {
            p->drawLine(QPointF(cx - a, cy - a), QPointF(cx + a, cy + a));
            p->drawLine(QPointF(cx + a, cy - a), QPointF(cx - a, cy + a));
        } else if (index == 1) {
            p->drawLine(QPointF(cx, cy - a), QPointF(cx, cy + a * 0.2));
            p->drawPoint(QPointF(cx, cy + a * 0.9));
        } else {
            QPolygonF tick;
            tick << QPointF(cx - a, cy) << QPointF(cx - a * 0.2, cy + a * 0.8) << QPointF(cx + a, cy - a * 0.7);
            p->drawPolyline(tick);
        }
    } else if (set.contains(QLatin1String("Stars"))) {
        auto star = [&](bool full, bool half) {
            QPolygonF st;
            for (int k = 0; k < 10; ++k) {
                const double angle = -3.14159265358979 / 2 + k * 3.14159265358979 / 5, radius = k % 2 ? h * 0.45 : h;
                st << QPointF(cx + radius * std::cos(angle), cy + radius * std::sin(angle));
            }
            p->setPen(QPen(gold, 1));
            p->setBrush(Qt::NoBrush);
            p->drawPolygon(st);
            p->setPen(Qt::NoPen);
            p->setBrush(gold);
            if (full) p->drawPolygon(st);
            else if (half) {
                p->save();
                p->setClipRect(QRectF(box.left(), box.top(), box.width() / 2.0, box.height()));
                p->drawPolygon(st);
                p->restore();
            }
        };
        star(index == 2, index == 1);
    } else if (set.contains(QLatin1String("Rating"))) {
        // bars of a signal that stand taller: the first `index + 1` are filled
        const double bw = box.width() / (count * 2.0);
        for (int k = 0; k < count; ++k) {
            const double top = box.bottom() - box.height() * (k + 1) / count;
            p->setBrush(k <= index ? black : QColor(0xC8, 0xC8, 0xC8));
            p->drawRect(QRectF(box.left() + k * 2 * bw, top, bw * 1.4, box.bottom() - top));
        }
    } else if (set.contains(QLatin1String("Quarters"))) {
        const QRectF circle(cx - h, cy - h, 2 * h, 2 * h);
        p->setBrush(Qt::white);
        p->setPen(QPen(black, 1));
        p->drawEllipse(circle);
        p->setPen(Qt::NoPen);
        p->setBrush(black);
        // (the lowest is empty and the highest full)
        if (index == count - 1) p->drawEllipse(circle);
        else if (index > 0) p->drawPie(circle, 90 * 16, -index * 90 * 16);
    } else if (set.contains(QLatin1String("Boxes"))) {
        const QRectF square(cx - h, cy - h, 2 * h, 2 * h);
        p->setBrush(Qt::white);
        p->setPen(QPen(gray, 1));
        p->drawRect(square);
        p->setPen(Qt::NoPen);
        p->setBrush(gray);
        const double part = double(index) / double(count - 1);
        p->drawRect(QRectF(square.left(), square.bottom() - square.height() * part, square.width(), square.height() * part));
    } else {
        // traffic lights (and any set that is not known): a disc, on a dark rounded square for the second kind
        if (set.contains(QLatin1String("TrafficLights2"))) {
            p->setBrush(QColor(0x40, 0x40, 0x40));
            p->drawRoundedRect(box, 3, 3);
            p->setBrush(c);
            p->drawEllipse(QPointF(cx, cy), h * 0.65, h * 0.65);
        } else {
            p->drawEllipse(QPointF(cx, cy), h, h);
        }
    }
    p->restore();
}

// The lines of an underline that is not the font's own: double, and the accounting kinds (which run the width of the cell). `x` and `width` are where the text is, `baseline` its baseline,
// and `lineWidth` and `position` the font's; the lines are kept inside the cell's bottom.
static void drawUnderlineLines(QPainter *p, Underline kind, const QRect &cell, const QRect &r, double x, double width, double baseline, double lineWidth, double position, bool gridlines, const QColor &ink)
{
    const bool accounting = kind == Underline::SingleAccounting || kind == Underline::DoubleAccounting;
    const bool twice = kind == Underline::Double || kind == Underline::DoubleAccounting;
    const int t = std::max(1, int(std::lround(lineWidth)));
    int y = int(std::lround(baseline)) + std::max(1, int(std::lround(position)));
    const int last = cell.bottom() - (gridlines ? 1 : 0);
    const int lowest = y + (twice ? t + 1 + t : t) - 1;
    if (lowest > last) y -= lowest - last;   // (a line the room under the text does not have is drawn higher, never past the cell)
    const int left = accounting ? r.left() : int(std::lround(x)), w = accounting ? r.width() : int(std::lround(width));
    p->fillRect(QRect(left, y, w, t), ink);
    if (twice) p->fillRect(QRect(left, y + t + 1, w, t), ink);
}

void GridPainter::drawText(QPainter *p, const QRect &cell, const QRect &box, const QRect &area, const CellLook &look, const Shown &sh, const PaintOptions &o)
{
    const GridLayout &L = m_sheet->layout();
    if (m_sheet->rightToLeft() && !m_drawingMirrored) {
        // (the grid is drawn mirrored for a sheet that reads right to left; the text is drawn as it reads, in the places the screen has: the rectangles are mirrored, and the left and the right
        // of its alignment, which the grid keeps in its own places, are the screen's again)
        const int width = L.viewport().width();
        auto screen = [width](const QRect &r) { return QRect(width - r.right() - 1, r.top(), r.width(), r.height()); };
        Shown onScreen = sh;
        onScreen.align = sh.align == HAlign::Left ? HAlign::Right : sh.align == HAlign::Right ? HAlign::Left : sh.align;
        p->save();
        p->resetTransform();
        m_drawingMirrored = true;
        drawText(p, screen(cell), screen(box), screen(area), look, onScreen, o);
        m_drawingMirrored = false;
        p->restore();
        return;
    }
    const double z = L.zoom();
    QFont font = fontFor(look);
    if (font.pixelSize() < kMinFontPixels) return;
    const int pad = std::max(1, int(std::lround(kCellPadding * z))), indent = int(std::lround(look.indent * 9 * z));
    const bool left = sh.align != HAlign::Right && sh.align != HAlign::Center && sh.align != HAlign::CenterContinuous, right = sh.align == HAlign::Right;
    // (the icon is at the left edge of the grid's cell, so on the screen's right in a sheet that reads right to left)
    QRect r = box.adjusted(pad + (left ? indent + (m_drawingMirrored ? 0 : m_iconInset) : 0), 1, -(pad + (right ? indent + (m_drawingMirrored ? m_iconInset : 0) : (m_drawingMirrored ? m_iconInset : 0))), -1);
    if (r.width() <= 0 || r.height() <= 0) return;
    int flags = left ? Qt::AlignLeft : right ? Qt::AlignRight : Qt::AlignHCenter;
    flags |= look.vertical == VAlign::Top || look.vertical == VAlign::Justify || look.vertical == VAlign::Distributed ? Qt::AlignTop : look.vertical == VAlign::Center ? Qt::AlignVCenter : Qt::AlignBottom;
    QString text = sh.text;
    double deviceWidth = sh.width * z;
    bool clip = deviceWidth > r.width() || r.height() < font.pixelSize() + 2;
    if (look.shrink && !look.wrap && deviceWidth > r.width() && deviceWidth > 0) {
        font.setPixelSize(std::max(1, int(std::floor(font.pixelSize() * r.width() / deviceWidth))));
        if (font.pixelSize() < kMinFontPixels) return;
        clip = true;
    }
    if (look.wrap) {
        flags |= Qt::TextWordWrap;
        if (look.horizontal == HAlign::Justify || look.horizontal == HAlign::Distributed) flags = (flags & ~Qt::AlignHorizontal_Mask) | Qt::AlignJustify;
        clip = true;
    } else {
        flags |= Qt::TextSingleLine;
    }
    if (look.readingOrder == 2) flags |= Qt::TextForceRightToLeft;
    // "*x" in a number's format: the text is spread with x to fill the cell
    if (sh.kind == Kind::Number && sh.fillAt >= 0 && sh.fillAt <= text.size() && !sh.hashes) {
        const double charWidth = std::max(1.0, m_meter->width(QString(sh.fillChar), look) * z);
        const int n = int((r.width() - deviceWidth) / charWidth);
        if (n > 0) text = text.left(sh.fillAt) + QString(n, sh.fillChar) + text.mid(sh.fillAt);
    }
    // fill alignment: the text is repeated to fill the width of the cell
    if (look.horizontal == HAlign::Fill && !look.wrap && !sh.hashes && deviceWidth > 0 && !text.isEmpty() && look.rotation == 0) {
        const int n = std::min(2000, int(r.width() / deviceWidth));
        if (n > 1) {
            text = text.repeated(n);
            deviceWidth *= n;
            clip = true;
        }
    }
    ++m_stats.textRuns;
    m_stats.textChars += text.size();
    if (sh.hashes) ++m_stats.hashed;
    const QRgb ink = m_mono ? 0xFF000000u : sh.color ? sh.color : look.textColor;
    const bool otherUnderline = look.underline == Underline::Double || look.underline == Underline::SingleAccounting || look.underline == Underline::DoubleAccounting;
    // A line of text with nothing special (not turned, not wrapped, not shrunk) is drawn from its baseline as a run of glyphs, where no layout of a box is needed.
    if (look.rotation == 0 && !look.wrap && look.readingOrder != 2 && font.pixelSize() == fontFor(look).pixelSize()) {
        const size_t id = look.fontId;
        if (id >= m_faces.size()) m_faces.resize(id + 1);
        if (!m_faces[id]) m_faces[id] = std::make_unique<GlyphFace>(font);
        const GlyphFace &face = *m_faces[id];
        QList<quint32> glyphs;
        QList<QPointF> positions;
        double w = 0;
        if (face.raw().isValid() && face.layout(text, &glyphs, &positions, &w) && !glyphs.isEmpty()) {
            const double x = left ? r.left() : right ? r.right() + 1 - w : r.left() + (r.width() - w) / 2;
            const double ascent = face.ascent(), descent = face.descent();
            const double y = (flags & Qt::AlignTop) ? r.top() + ascent : (flags & Qt::AlignVCenter) ? r.top() + (r.height() - (ascent + descent)) / 2 + ascent : r.bottom() + 1 - descent;
            QGlyphRun run;
            run.setRawFont(face.raw());
            run.setGlyphIndexes(glyphs);
            run.setPositions(positions);
            run.setUnderline(look.underline == Underline::Single);
            run.setStrikeOut(look.strike);
            if (clip) p->setClipRect(box & area);
            p->setPen(QColor::fromRgba(ink));
            p->drawGlyphRun(QPointF(std::round(x), std::round(y)), run);
            if (otherUnderline) {
                p->setClipRect(box & area);
                drawUnderlineLines(p, look.underline, cell, r, std::round(x), w, std::round(y), face.lineWidth(), face.underlinePos(), o.gridlines, QColor::fromRgba(ink));
                if (!clip) p->setClipRect(area);
            }
            if (clip) p->setClipRect(area);
            return;
        }
    }
    p->setFont(font);
    p->setPen(QColor::fromRgba(ink));
    const QFontMetricsF fm(font);
    if (look.rotation != 0 && look.rotation != 255) {
        // turned: the box the text turns in (as wide and as tall as the turned text is) is put in the cell where its alignment says, and the text is turned about the middle of it
        const int turn = look.rotation <= 90 ? look.rotation : -(look.rotation - 90);
        const double rad = turn * 3.14159265358979323846 / 180.0;
        const double tw = fm.horizontalAdvance(text), th = fm.height();
        const double bw = std::abs(tw * std::cos(rad)) + th * std::abs(std::sin(rad)), bh = std::abs(tw * std::sin(rad)) + th * std::abs(std::cos(rad));
        const double bx = sh.align == HAlign::Right ? r.right() + 1 - bw : sh.align == HAlign::Center || sh.align == HAlign::CenterContinuous ? r.left() + (r.width() - bw) / 2 : r.left();
        const double by = look.vertical == VAlign::Top || look.vertical == VAlign::Justify || look.vertical == VAlign::Distributed ? r.top() : look.vertical == VAlign::Center ? r.top() + (r.height() - bh) / 2
                                                                                                                                                  : r.bottom() + 1 - bh;
        p->save();
        p->setClipRect(cell & area);
        p->translate(QPointF(bx + bw / 2, by + bh / 2));
        p->rotate(-turn);
        p->drawText(QRectF(-tw / 2 - 2, -th / 2, tw + 4, th), Qt::AlignCenter | Qt::TextSingleLine | Qt::TextDontClip, text);
        if (otherUnderline) drawUnderlineLines(p, look.underline, QRect(-100000, -100000, 200000, 200000), QRect(int(-tw / 2), 0, int(tw), 1), -tw / 2, tw, -th / 2 + fm.ascent(), fm.lineWidth(), fm.underlinePos(), false, QColor::fromRgba(ink));
        p->restore();
        return;
    }
    if (look.rotation == 255) {
        // upright, one letter under another: the block of them is put where the alignment says
        QStringList letters;
        for (QChar c : text) letters << QString(c);
        double widest = 0;
        for (const QString &l : std::as_const(letters)) widest = std::max(widest, fm.horizontalAdvance(l));
        const double bw = widest, bh = fm.lineSpacing() * double(letters.size());
        const double bx = sh.align == HAlign::Right ? r.right() + 1 - bw : sh.align == HAlign::Center || sh.align == HAlign::CenterContinuous ? r.left() + (r.width() - bw) / 2 : r.left();
        const double by = look.vertical == VAlign::Top || look.vertical == VAlign::Justify || look.vertical == VAlign::Distributed ? r.top() : look.vertical == VAlign::Center ? r.top() + (r.height() - bh) / 2
                                                                                                                                                  : r.bottom() + 1 - bh;
        p->setClipRect(cell & area);
        p->drawText(QRectF(bx, by, bw + 1, bh), Qt::AlignHCenter | Qt::AlignTop | Qt::TextDontClip, letters.join(QLatin1Char('\n')));
        p->setClipRect(area);
        return;
    }
    if (clip) p->setClipRect(box & area);
    if (otherUnderline && look.wrap) {
        QFont underlined = font;
        underlined.setUnderline(true);   // (a wrapped text with one of the other underlines has the font's: its lines are not each measured)
        p->setFont(underlined);
    }
    p->drawText(r, flags | (clip ? 0 : Qt::TextDontClip), text);
    if (otherUnderline && !look.wrap) {
        const double w = fm.horizontalAdvance(text), h = fm.height();
        const double x = left ? r.left() : right ? r.right() + 1 - w : r.left() + (r.width() - w) / 2;
        const double top = (flags & Qt::AlignTop) ? r.top() : (flags & Qt::AlignVCenter) ? r.top() + (r.height() - h) / 2 : r.bottom() + 1 - h;
        p->setClipRect(box & area);
        drawUnderlineLines(p, look.underline, cell, r, x, w, top + fm.ascent(), fm.lineWidth(), fm.underlinePos(), o.gridlines, QColor::fromRgba(ink));
    }
    if (clip || otherUnderline) p->setClipRect(area);
}

// ---- the parts of the grid

struct GridPainter::PaneWork {
    std::vector<Span> rows, cols;
    std::vector<const Cell *> cells;           // rows by columns, those beside the ones drawn included
    std::vector<const CellLook *> looks;       // the same, null for the plain look
    std::vector<qint32> cond;                  // the same, what the rules of conditional formatting do to the cell: a place in `condCells` + 1 (0 for nothing; empty when the sheet has no rules)
    std::vector<CondCell> condCells;           // what the rules did, held here by value (what the rules keep may be let go of while the paint goes on)
    std::vector<char> covered;                 // under a merged range
    std::vector<const Cell *> leftCandidate, rightCandidate;   // by row: the nearest cell with something in it outside the columns drawn, on each side
    std::vector<qint64> leftCandidateCol, rightCandidateCol;   // (and the column each is in)
    size_t C = 0;
};

void GridPainter::paintPane(QPainter *p, Pane which, const QRect &clip, const PaintOptions &o)
{
    const GridLayout &L = m_sheet->layout();
    const PaneView pv = L.pane(which);
    const QRect area = pv.rect & clip;
    if (area.isEmpty()) return;
    ++m_stats.panes;
    p->setClipRect(area);
    p->fillRect(area, o.backdrop);
    if (pv.isEmpty()) return;
    const bool frozenRows = which == Pane::Corner || which == Pane::Top, frozenCols = which == Pane::Corner || which == Pane::Left;
    const Axis &rowsAxis = L.rows, &colsAxis = L.cols;
    PaneWork w;
    // ---- the rows and columns drawn: those that have pixels and are in the clip, with the one beside each end for the borders they share
    auto collect = [&](const Axis &axis, qint64 first, qint64 last, bool frozen, bool vertical, int lo, int hi, int most, std::vector<Span> *out) {
        int steps = 0;
        for (qint64 i = first; i <= last && i < axis.count(); i = axis.nextVisible(i)) {
            if (++steps > kMaxScanSteps) {
                m_stats.truncated = true;
                break;
            }
            const int a = vertical ? L.yEdge(i, frozen) : L.xEdge(i, frozen), b = vertical ? L.yEdge(i + 1, frozen) : L.xEdge(i + 1, frozen);
            if (b <= lo) continue;
            if (a > hi) break;
            if (b <= a) continue;
            if (int(out->size()) >= most) {
                m_stats.truncated = true;
                break;
            }
            out->push_back({i, a, b - a, false});
        }
    };
    collect(colsAxis, pv.firstCol, pv.lastCol, frozenCols, false, area.left(), area.right(), kMaxCols, &w.cols);
    const int colsCount = int(std::max<size_t>(1, w.cols.size()));
    collect(rowsAxis, pv.firstRow, pv.lastRow, frozenRows, true, area.top(), area.bottom(), int(std::min<qint64>(kMaxRows, std::max<qint64>(1, kMaxCells / colsCount))), &w.rows);
    if (w.rows.empty() || w.cols.empty()) return;
    m_stats.rowsVisited += qint64(w.rows.size());
    m_stats.colsVisited += qint64(w.cols.size());
    const Span firstRow = w.rows.front(), lastRow = w.rows.back(), firstCol = w.cols.front(), lastCol = w.cols.back();
    auto ring = [](qint64 index) { return Span{index, 0, 0, true}; };
    const qint64 rowBefore = rowsAxis.previousVisible(firstRow.index), rowAfter = rowsAxis.nextVisible(lastRow.index);
    const qint64 colBefore = colsAxis.previousVisible(firstCol.index), colAfter = colsAxis.nextVisible(lastCol.index);
    if (rowBefore >= 0) w.rows.insert(w.rows.begin(), ring(rowBefore));
    if (rowAfter < rowsAxis.count()) w.rows.push_back(ring(rowAfter));
    if (colBefore >= 0) w.cols.insert(w.cols.begin(), ring(colBefore));
    if (colAfter < colsAxis.count()) w.cols.push_back(ring(colAfter));
    const size_t R = w.rows.size(), C = w.cols.size();
    w.C = C;
    const size_t r0 = rowBefore >= 0 ? 1 : 0, r1 = R - (rowAfter < rowsAxis.count() ? 1 : 0);   // the rows drawn: [r0, r1)
    const size_t c0 = colBefore >= 0 ? 1 : 0, c1 = C - (colAfter < colsAxis.count() ? 1 : 0);
    // ---- the paper under the cells, and the gridlines on it
    p->fillRect(QRect(firstCol.start, firstRow.start, lastCol.start + lastCol.size - firstCol.start, lastRow.start + lastRow.size - firstRow.start), o.paper);
    if (o.gridlines) {
        const int x0 = firstCol.start, x1 = lastCol.start + lastCol.size, y0 = firstRow.start, y1 = lastRow.start + lastRow.size;
        for (size_t i = r0; i < r1; ++i) {
            p->fillRect(QRect(x0, w.rows[i].start + w.rows[i].size - 1, x1 - x0, 1), o.gridColor);
            ++m_stats.gridLines;
        }
        for (size_t j = c0; j < c1; ++j) {
            p->fillRect(QRect(w.cols[j].start + w.cols[j].size - 1, y0, 1, y1 - y0), o.gridColor);
            ++m_stats.gridLines;
        }
    }
    // ---- the cells: found a row at a time, with those on either side that text may spill from
    const bool scrollingCols = !frozenCols;
    w.cells.assign(R * C, nullptr);
    w.leftCandidate.assign(R, nullptr);
    w.rightCandidate.assign(R, nullptr);
    w.leftCandidateCol.assign(R, 0);
    w.rightCandidateCol.assign(R, 0);
    const qint64 marginLeft = scrollingCols ? kMaxSpill : 0, marginRight = scrollingCols ? kMaxSpill : 0;
    const bool wide = m_sheet->hasImplicitStyles();
    for (size_t i = 0; i < R; ++i) {
        const qint64 row = w.rows[i].index;
        const bool inMargin = scrollingCols && m_stats.cellsLooked < kMaxCells * 3;
        const qint64 lo = inMargin ? std::max<qint64>(0, w.cols.front().index - marginLeft) : w.cols.front().index;
        const qint64 hi = inMargin ? std::min<qint64>(colsAxis.count() - 1, w.cols.back().index + marginRight) : w.cols.back().index;
        const qint64 leftEdge = w.cols[c0].index, rightEdge = w.cols[c1 - 1].index;
        m_sheet->data().sheet->cells.forEachInRange(quint32(row), quint32(lo), quint32(row), quint32(hi), [&](quint32, quint32 cellCol, const Cell &cell) {
            ++m_stats.cellsLooked;
            const qint64 col = cellCol;
            const auto it = std::lower_bound(w.cols.begin(), w.cols.end(), col, [](const Span &s, qint64 v) { return s.index < v; });
            if (it != w.cols.end() && it->index == col) w.cells[i * C + size_t(it - w.cols.begin())] = &cell;
            const bool holds = cell.type != CellType::Empty || cell.hasFormula();
            if (holds && scrollingCols) {
                if (col < leftEdge && !colsAxis.isHidden(col)) {                                                  // (the last one met is the nearest)
                    w.leftCandidate[i] = &cell;
                    w.leftCandidateCol[i] = col;
                } else if (col > rightEdge && !w.rightCandidate[i] && !colsAxis.isHidden(col)) {
                    w.rightCandidate[i] = &cell;
                    w.rightCandidateCol[i] = col;
                }
            }
            return true;
        });
    }
    // ---- merged ranges that cross what is drawn, and the cells under them
    std::vector<int> merges;
    if (L.merges.count() > 0) {
        qint64 crossed = 0;
        if (!L.merges.crossing(quint32(firstRow.index), quint32(firstCol.index), quint32(lastRow.index), quint32(lastCol.index), kMaxMerges, &merges, &crossed)) m_stats.truncated = true;
        if (crossed > qint64(merges.size())) m_stats.mergesLeftOut += crossed - qint64(merges.size());   // (counted, and the paint says it was cut short)
        w.covered.assign(R * C, 0);
        for (int id : merges) {
            const jp::office::sheet::CellRange &m = L.merges.range(id);
            const auto rb = std::lower_bound(w.rows.begin() + std::ptrdiff_t(r0), w.rows.begin() + std::ptrdiff_t(r1), qint64(m.row1), [](const Span &s, qint64 v) { return s.index < v; });
            const auto cb = std::lower_bound(w.cols.begin() + std::ptrdiff_t(c0), w.cols.begin() + std::ptrdiff_t(c1), qint64(m.col1), [](const Span &s, qint64 v) { return s.index < v; });
            for (auto ri = rb; ri != w.rows.begin() + std::ptrdiff_t(r1) && ri->index <= m.row2; ++ri)
                for (auto ci = cb; ci != w.cols.begin() + std::ptrdiff_t(c1) && ci->index <= m.col2; ++ci) {
                    if (m_stats.mergeMarks >= kMaxMergeMarks) {
                        m_stats.truncated = true;
                        goto marked;
                    }
                    ++m_stats.mergeMarks;
                    w.covered[size_t(ri - w.rows.begin()) * C + size_t(ci - w.cols.begin())] = 1;
                }
        }
    }
marked:
    // ---- the looks, by cell (a cell with the first format, and no cell, has the plain look unless its row or column has a format)
    w.looks.assign(R * C, nullptr);
    bool anyFill = false, anyBorder = false, anyBar = false, anyIcon = false;
    const LookTable &looks = m_sheet->looks();
    const bool ruled = m_sheet->hasConditionalFormats();
    const bool skinned = m_sheet->hasTableSkins();
    if (ruled) w.cond.assign(R * C, 0);
    for (size_t i = 0; i < R; ++i)
        for (size_t j = 0; j < C; ++j) {
            const Cell *c = w.cells[i * C + j];
            quint32 xf = c ? c->style : 0;
            if (!c && wide && !w.rows[i].ring && !w.cols[j].ring) xf = m_sheet->styleOf(quint32(w.rows[i].index), quint32(w.cols[j].index), nullptr);
            if (ruled && !w.rows[i].ring && !w.cols[j].ring && m_stats.condCells < kMaxCondCells) {
                // what the rules do to the cell (asked of the cells drawn only, and for a bounded number of them in a paint)
                const quint32 row = quint32(w.rows[i].index), col = quint32(w.cols[j].index);
                if (m_sheet->conditional().covers(row, col)) {
                    ++m_stats.condCells;
                    const CondCell cc = m_sheet->conditional().at(row, col);
                    if (cc.any) {
                        w.condCells.push_back(cc);
                        w.cond[i * C + j] = qint32(w.condCells.size());
                        anyBar = anyBar || cc.hasBar;
                        anyIcon = anyIcon || cc.icon >= 0;
                        anyFill = anyFill || cc.hasScale;
                        if (cc.format >= 0) {
                            const CellLook &look = m_sheet->lookOf(row, col, xf);
                            w.looks[i * C + j] = &look;
                            anyFill = anyFill || look.hasFill();
                            anyBorder = anyBorder || look.hasBorder();
                            continue;
                        }
                    }
                }
            } else if (ruled && m_stats.condCells >= kMaxCondCells) {
                m_stats.truncated = true;
                m_sheet->conditional().reportTruncated();
            }
            if (skinned && !w.rows[i].ring && !w.cols[j].ring && m_sheet->inTableSkin(quint32(w.rows[i].index), quint32(w.cols[j].index))) {
                // (a cell of a table: the look its style gives it, under what the cell has of its own)
                const CellLook &look = m_sheet->lookOf(quint32(w.rows[i].index), quint32(w.cols[j].index), xf);
                w.looks[i * C + j] = &look;
                anyFill = anyFill || look.hasFill();
                anyBorder = anyBorder || look.hasBorder();
                continue;
            }
            if (xf == 0) continue;
            const CellLook &look = looks.look(xf);
            w.looks[i * C + j] = &look;
            anyFill = anyFill || look.hasFill();
            anyBorder = anyBorder || look.hasBorder();
        }
    // (what a cell shows of the rules besides its look: a color scale's fill, a data bar, an icon)
    auto condOf = [&](size_t i, size_t j) -> const CondCell * { return w.cond.empty() || w.cond[i * C + j] == 0 ? nullptr : &w.condCells[size_t(w.cond[i * C + j] - 1)]; };   // (the cells are all taken above: nothing is added to them from here)
    auto iconOf = [&](const CondCell &cc, const CellLook &look, const Span &rs, int *size, QString *name, int *count) {
        *name = m_sheet->conditional().iconSetName(cc.iconSet);
        *count = iconSetCount(*name);
        const int font = fontFor(look).pixelSize();
        *size = std::max(6, std::min(rs.size - 4, font + 3));
    };
    // ---- each row: fills, borders, text
    qint64 textBudget = kMaxTextRuns - m_stats.textRuns;
    // (a font too small to read is not drawn, so a cell in one has no text made for it)
    auto tooSmall = [&](const CellLook &look) { return fontFor(look).pixelSize() < kMinFontPixels; };
    auto spillOfCell = [&](qint64 cellCol, const Cell &cell, const CellLook &look, const Shown &sh, qint64 row) {
        const double width = colsAxis.size(cellCol);
        const bool centerContinuous = sh.align == HAlign::CenterContinuous;
        return spillOf(sh, look, width, [&](int step) -> double {
            const qint64 col = cellCol + step;
            if (col < 0 || col >= colsAxis.count()) return -1;
            if ((col < L.frozenCols()) != (cellCol < L.frozenCols())) return -1;   // (not across the edge of the frozen columns)
            const double w2 = colsAxis.size(col);
            if (w2 == 0) return 0;
            if (++m_stats.spillProbes > kMaxSpillProbes) return -1;
            const Cell *other = m_sheet->cellAt(quint32(row), quint32(col));
            if (other && (other->type != CellType::Empty || other->hasFormula())) return -1;
            if (L.merges.count() > 0 && L.merges.at(quint32(row), quint32(col)) >= 0) return -1;
            if (centerContinuous) {
                const quint32 xf = other ? other->style : m_sheet->styleOf(quint32(row), quint32(col), nullptr);
                if (looks.look(xf).horizontal != HAlign::CenterContinuous) return -1;
            }
            return w2;
        });
    };
    for (size_t i = r0; i < r1; ++i) {
        const Span &rs = w.rows[i];
        // fills (a color scale's color is the fill; data bars are drawn on it)
        if (anyFill || anyBar)
            for (size_t j = c0; j < c1; ++j) {
                if (!w.covered.empty() && w.covered[i * C + j]) continue;
                const CellLook *look = w.looks[i * C + j];
                const CondCell *cc = condOf(i, j);
                const QRect cellRect(w.cols[j].start, rs.start, w.cols[j].size, rs.size);
                if (cc && cc->hasScale) {
                    p->fillRect(cellRect, QColor::fromRgba(cc->scale));
                    ++m_stats.fills;
                } else if (look && look->hasFill()) {
                    drawFill(p, cellRect, *look, o);
                }
                if (cc && cc->hasBar) drawBar(p, cellRect, cc->bar, o.gridlines);
            }
        // borders: the lines between the cells of this row, and above it
        if (anyBorder) {
            for (size_t k = c0; k <= c1; ++k) {
                if (!w.covered.empty() && ((k > 0 && k - 1 >= c0 && w.covered[i * C + k - 1] && k < C && w.covered[i * C + k]) )) continue;   // inside a merged range
                const CellLook *a = k > 0 ? w.looks[i * C + k - 1] : nullptr, *b = k < C ? w.looks[i * C + k] : nullptr;
                const LineLook &line = heavier(a ? &a->right : nullptr, b ? &b->left : nullptr);
                const int x = k < c1 ? w.cols[k].start : w.cols[c1 - 1].start + w.cols[c1 - 1].size;
                drawEdge(p, line, x, rs.start, rs.size, true, area);
            }
            for (size_t j = c0; j < c1; ++j) {
                if (!w.covered.empty() && w.covered[i * C + j] && i > 0 && w.covered[(i - 1) * C + j]) continue;
                const CellLook *above = i > 0 ? w.looks[(i - 1) * C + j] : nullptr, *here = w.looks[i * C + j];
                drawEdge(p, heavier(above ? &above->bottom : nullptr, here ? &here->top : nullptr), w.cols[j].start, rs.start, w.cols[j].size, false, area);
                if (here && here->diagonal.style != BorderStyle::None && (here->diagonalUp || here->diagonalDown)) {
                    p->save();
                    p->setClipRect(QRect(w.cols[j].start, rs.start, w.cols[j].size, rs.size) & area);
                    p->setPen(m_mono ? QColor(Qt::black) : QColor::fromRgba(here->diagonal.color));
                    if (here->diagonalUp) p->drawLine(w.cols[j].start, rs.start + rs.size - 1, w.cols[j].start + w.cols[j].size - 1, rs.start);
                    if (here->diagonalDown) p->drawLine(w.cols[j].start, rs.start, w.cols[j].start + w.cols[j].size - 1, rs.start + rs.size - 1);
                    p->restore();
                    ++m_stats.borderSegments;
                }
            }
        }
        // the text of this row; then that of the cell beside the columns drawn whose text runs into them
        for (size_t j = c0; j < c1 && textBudget > 0; ++j) {
            const Cell *cell = w.cells[i * C + j];
            if (!cell || (!w.covered.empty() && w.covered[i * C + j])) continue;
            if (cell->type == CellType::Empty && !cell->hasFormula()) continue;
            const CondCell *cc = condOf(i, j);
            const CellLook &look = w.looks[i * C + j] ? *w.looks[i * C + j] : looks.look(cell->style);
            if (tooSmall(look)) continue;   // (before the text is made, and its neighbors looked at: nothing of it would be drawn)
            int iconSize = 0, iconCount = 0;
            QString iconName;
            if (cc && cc->icon >= 0) iconOf(*cc, look, rs, &iconSize, &iconName, &iconCount);
            const bool icon = iconSize > 0 && iconCount > 0 && w.cols[j].size >= iconSize + 4;
            if (icon) drawIcon(p, QRect(w.cols[j].start, rs.start, w.cols[j].size, rs.size), iconName, std::min(cc->icon, iconCount - 1), iconCount, iconSize);
            if (cc && cc->hideValue && (cc->hasBar || icon)) continue;   // (the rule says to show the bar or the icon and not the value)
            const Shown sh = m_sheet->shown(quint32(rs.index), quint32(w.cols[j].index), *cell);
            if (sh.kind == Kind::Empty && !sh.hashes) continue;
            m_iconInset = icon ? iconSize + 4 : 0;
            const Spill sp = spillOfCell(w.cols[j].index, *cell, look, sh, rs.index);
            QRect box(w.cols[j].start, rs.start, w.cols[j].size, rs.size);
            if (o.gridlines && sp.left + sp.right > 0) {
                // the gridlines between the cells the text runs over are under it: they are not drawn (those of cells with a fill are covered already)
                auto plain = [&](size_t k) { return !w.looks[i * C + k] || !w.looks[i * C + k]->hasFill(); };
                for (size_t k = j; k + 1 < c1 && sp.right > 0 && w.cols[k + 1].index <= w.cols[j].index + sp.right; ++k)
                    if (plain(k) && plain(k + 1)) p->fillRect(QRect(w.cols[k].start + w.cols[k].size - 1, rs.start, 1, rs.size - 1), o.paper);
                for (size_t k = j; k > c0 && sp.left > 0 && w.cols[k - 1].index >= w.cols[j].index - sp.left; --k)
                    if (plain(k) && plain(k - 1)) p->fillRect(QRect(w.cols[k - 1].start + w.cols[k - 1].size - 1, rs.start, 1, rs.size - 1), o.paper);
            }
            if (sp.left + sp.right > 0) {
                const int x0 = L.xEdge(w.cols[j].index - sp.left, frozenCols), x1 = L.xEdge(w.cols[j].index + sp.right + 1, frozenCols);
                box = QRect(x0, rs.start, x1 - x0, rs.size);
            }
            drawText(p, QRect(w.cols[j].start, rs.start, w.cols[j].size, rs.size), box, area, look, sh, o);
            m_iconInset = 0;
            --textBudget;
        }
        if (textBudget <= 0 && !m_stats.truncated) m_stats.truncated = true;
        for (int side = 0; side < 2; ++side) {
            const Cell *cand = side == 0 ? w.leftCandidate[i] : w.rightCandidate[i];
            if (!cand || textBudget <= 0) continue;
            const qint64 candCol = side == 0 ? w.leftCandidateCol[i] : w.rightCandidateCol[i];
            const CellLook &look = m_sheet->lookOf(quint32(rs.index), quint32(candCol), cand->style);
            if (tooSmall(look)) continue;
            const Shown sh = m_sheet->shown(quint32(rs.index), quint32(candCol), *cand);
            if (sh.kind != Kind::Text || sh.hashes) continue;
            const Spill sp = spillOfCell(candCol, *cand, look, sh, rs.index);
            if (sp.left + sp.right == 0) continue;
            const int cx0 = L.xEdge(candCol, frozenCols), cx1 = L.xEdge(candCol + 1, frozenCols);
            const int x0 = L.xEdge(candCol - sp.left, frozenCols), x1 = L.xEdge(candCol + sp.right + 1, frozenCols);
            const QRect box(x0, rs.start, x1 - x0, rs.size);
            if (!box.intersects(area)) continue;
            drawText(p, QRect(cx0, rs.start, cx1 - cx0, rs.size), box, area, look, sh, o);
            --textBudget;
        }
    }
    // the edge below the last row
    if (anyBorder && r1 > r0)
        for (size_t j = c0; j < c1; ++j) {
            const CellLook *here = w.looks[(r1 - 1) * C + j], *below = r1 < R ? w.looks[r1 * C + j] : nullptr;
            const Span &rs = w.rows[r1 - 1];
            drawEdge(p, heavier(here ? &here->bottom : nullptr, below ? &below->top : nullptr), w.cols[j].start, rs.start + rs.size, w.cols[j].size, false, area);
        }
    // ---- merged ranges, each as one cell: its fill (over the gridlines inside it, but not those that bound it), its borders from the cells at its corners, and its first cell's text
    // (merged ranges do not overlap in a sheet that is whole, so what they cover is the screen at the most; a file that stacks a hundred thousand of them over it is drawn as far as
    // eight screens of fill, and the rest are counted as left out)
    qint64 mergeArea = 8 * qint64(area.width()) * area.height();
    for (size_t at = 0; at < merges.size(); ++at) {
        const int id = merges[at];
        const jp::office::sheet::CellRange &m = L.merges.range(id);
        const QRect rect = L.rangeRectIn(which, m.row1, m.col1, m.row2, m.col2);
        if (rect.isEmpty() || !rect.intersects(area)) continue;
        const QRect shown = rect & area;
        mergeArea -= qint64(shown.width()) * shown.height();
        if (mergeArea < 0) {
            m_stats.mergesLeftOut += qint64(merges.size() - at);
            m_stats.truncated = true;
            break;
        }
        ++m_stats.mergesDrawn;
        const Cell *anchor = m_sheet->cellAt(m.row1, m.col1);
        const quint32 xf = anchor ? anchor->style : m_sheet->styleOf(m.row1, m.col1, nullptr);
        CondCell anchorCondValue;
        const CondCell *anchorCond = nullptr;
        if (ruled && m_sheet->conditional().covers(m.row1, m.col1) && m_stats.condCells < kMaxCondCells) {
            anchorCondValue = m_sheet->conditional().at(m.row1, m.col1);
            ++m_stats.condCells;
            if (anchorCondValue.any) anchorCond = &anchorCondValue;
        }
        const CellLook &look = m_sheet->lookOf(m.row1, m.col1, xf);
        if (anchorCond && anchorCond->hasScale) p->fillRect(rect.adjusted(0, 0, o.gridlines ? -1 : 0, o.gridlines ? -1 : 0), QColor::fromRgba(anchorCond->scale));
        else if (look.hasFill()) drawFill(p, rect, look, o);
        else p->fillRect(rect.adjusted(0, 0, o.gridlines ? -1 : 0, o.gridlines ? -1 : 0), o.paper);
        if (anchorCond && anchorCond->hasBar) drawBar(p, rect, anchorCond->bar, o.gridlines);
        if (look.hasBorder() || xf == 0) {
            auto lookOf = [&](quint32 r, quint32 c) -> const CellLook * {
                const Cell *x = m_sheet->cellAt(r, c);
                return x && x->style ? &looks.look(x->style) : nullptr;
            };
            const CellLook *tl = lookOf(m.row1, m.col1), *tr = lookOf(m.row1, m.col2), *bl = lookOf(m.row2, m.col1), *br = lookOf(m.row2, m.col2);
            drawEdge(p, heavier(tl ? &tl->left : nullptr, bl ? &bl->left : nullptr), rect.left(), rect.top(), rect.height(), true, area);
            drawEdge(p, heavier(tr ? &tr->right : nullptr, br ? &br->right : nullptr), rect.right() + 1, rect.top(), rect.height(), true, area);
            drawEdge(p, heavier(tl ? &tl->top : nullptr, tr ? &tr->top : nullptr), rect.left(), rect.top(), rect.width(), false, area);
            drawEdge(p, heavier(bl ? &bl->bottom : nullptr, br ? &br->bottom : nullptr), rect.left(), rect.bottom() + 1, rect.width(), false, area);
        }
        if (anchor && textBudget > 0 && !tooSmall(look)) {
            int iconSize = 0, iconCount = 0;
            QString iconName;
            if (anchorCond && anchorCond->icon >= 0) {
                // (the icon is sized from the height of the first row of the range)
                Span first;
                first.size = std::max(1, std::min(rect.height(), int(std::lround(rowsAxis.size(m.row1) * L.zoom()))));
                iconOf(*anchorCond, look, first, &iconSize, &iconName, &iconCount);
            }
            const bool icon = iconSize > 0 && iconCount > 0 && rect.width() >= iconSize + 4;
            if (icon) drawIcon(p, rect, iconName, std::min(anchorCond->icon, iconCount - 1), iconCount, iconSize);
            const Shown sh = m_sheet->shown(m.row1, m.col1, *anchor);
            if ((sh.kind != Kind::Empty || sh.hashes) && !(anchorCond && anchorCond->hideValue && (anchorCond->hasBar || icon))) {
                m_iconInset = icon ? iconSize + 4 : 0;
                drawText(p, rect, rect, area, look, sh, o);
                m_iconInset = 0;
                --textBudget;
            }
        }
    }
    if (o.copyRange) paintDashedRange(p, which, area, *o.copyRange, o.accent, o.copyPhase, false);
    paintSelection(p, which, area, o);
    if (o.fillPreview) paintDashedRange(p, which, area, *o.fillPreview, QColor(0x44, 0x44, 0x44), 0, false);
    p->setClipping(false);
}

void GridPainter::paintSelection(QPainter *p, Pane which, const QRect &area, const PaintOptions &o)
{
    if (!o.selection) return;
    const GridLayout &L = m_sheet->layout();
    const Selection &s = *o.selection;
    const QRect range = L.rangeRectIn(which, s.range.row1, s.range.col1, s.range.row2, s.range.col2);
    if (range.isEmpty() || !range.intersects(area)) return;
    QRect active = L.rangeRectIn(which, s.activeRow, s.activeCol, s.activeRow, s.activeCol);
    const int merge = L.merges.at(quint32(s.activeRow), quint32(s.activeCol));
    if (merge >= 0) {
        const jp::office::sheet::CellRange &m = L.merges.range(merge);
        active = L.rangeRectIn(which, m.row1, m.col1, m.row2, m.col2);
    }
    p->setClipRect(area);
    if (!(range == active)) {
        QColor tint = o.accent;
        tint.setAlpha(40);
        p->setClipRegion(QRegion(range & area).subtracted(active));
        p->fillRect(range, tint);
        p->setClipRect(area);
    }
    // the border, two pixels, inside the range's edge
    p->fillRect(QRect(range.left(), range.top(), range.width(), 2), o.accent);
    p->fillRect(QRect(range.left(), range.bottom() - 1, range.width(), 2), o.accent);
    p->fillRect(QRect(range.left(), range.top(), 2, range.height()), o.accent);
    p->fillRect(QRect(range.right() - 1, range.top(), 2, range.height()), o.accent);
    // the fill handle: a square on the corner of the range, drawn in the part that has that corner
    if (o.fillHandle && s.whole == Selection::Whole::None) {
        const QRect corner = L.rangeRectIn(which, s.range.row2, s.range.col2, s.range.row2, s.range.col2);
        if (!corner.isEmpty() && corner.intersects(area)) {
            const QRect square(corner.right() - 3, corner.bottom() - 3, 7, 7);
            p->fillRect(square, QColor(Qt::white));
            p->fillRect(square.adjusted(1, 1, -1, -1), o.accent);
        }
    }
}

// A dashed border round a range of cells, in a pane: the cells that were copied (the dashes move as `phase` does), or where a fill would go.
void GridPainter::paintDashedRange(QPainter *p, Pane which, const QRect &area, const GridRange &r, const QColor &color, int phase, bool outside)
{
    const GridLayout &L = m_sheet->layout();
    const QRect range = L.rangeRectIn(which, r.row1, r.col1, r.row2, r.col2);
    if (range.isEmpty() || !range.intersects(area)) return;
    QPen pen(color, 1, Qt::CustomDashLine);
    pen.setDashPattern({4, 3});
    pen.setDashOffset(phase);
    p->save();
    p->setClipRect(area);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    QRect line = range.adjusted(0, 0, -1, -1);
    if (outside) line.adjust(-1, -1, 1, 1);
    p->drawRect(line);
    QPen white(QColor(Qt::white), 1, Qt::CustomDashLine);
    white.setDashPattern({3, 4});
    white.setDashOffset(phase + 4);
    p->setPen(white);
    p->drawRect(line);
    p->restore();
}

void GridPainter::drawHeaderText(QPainter *p, const QRect &cell, const QString &text)
{
    if (!m_sheet->rightToLeft()) {
        p->drawText(cell, Qt::AlignCenter | Qt::TextSingleLine, text);
        return;
    }
    // (the text of a mirrored sheet is drawn as it reads, in the cell's place on the screen)
    const int width = m_sheet->layout().viewport().width();
    p->save();
    p->resetTransform();
    p->drawText(QRect(width - cell.right() - 1, cell.top(), cell.width(), cell.height()), Qt::AlignCenter | Qt::TextSingleLine, text);
    p->restore();
}

void GridPainter::paintHeaders(QPainter *p, const QRect &clip, const PaintOptions &o)
{
    const GridLayout &L = m_sheet->layout();
    const int hw = L.headerWidth(), hh = L.headerHeight();
    if (hw == 0 || hh == 0) return;
    QFont font = o.headerFont;
    if (font.pixelSize() <= 0) font.setPixelSize(11);
    p->setFont(font);
    const Selection *sel = o.selection;
    auto colSelected = [&](qint64 c) { return sel && c >= sel->range.col1 && c <= sel->range.col2; };
    auto rowSelected = [&](qint64 r) { return sel && r >= sel->range.row1 && r <= sel->range.row2; };
    // the corner
    const QRect corner(0, 0, hw, hh);
    if (corner.intersects(clip)) {
        p->setClipRect(corner & clip);
        p->fillRect(corner, o.headerBackground);
        p->fillRect(QRect(hw - 1, 0, 1, hh), o.headerLine);
        p->fillRect(QRect(0, hh - 1, hw, 1), o.headerLine);
        ++m_stats.headersDrawn;
    }
    for (Pane which : {Pane::Left, Pane::Body}) {
        // the letters: along the top, over the columns of this part
        const PaneView pv = L.pane(which);
        const bool frozen = which == Pane::Left;
        const QRect band(pv.rect.left(), 0, pv.rect.width(), hh);
        if (band.width() > 0 && band.intersects(clip)) {
            const QRect area = band & clip;
            p->setClipRect(area);
            p->fillRect(area, o.headerBackground);
            int steps = 0, drawn = 0;
            for (qint64 c = pv.firstCol; !pv.isEmpty() && c <= pv.lastCol && c < L.cols.count() && drawn < kMaxCols && ++steps < kMaxScanSteps; c = L.cols.nextVisible(c)) {
                const int a = L.xEdge(c, frozen), b = L.xEdge(c + 1, frozen);
                if (b <= area.left()) continue;
                if (a > area.right()) break;
                if (b <= a) continue;
                const QRect cell(a, 0, b - a, hh);
                p->fillRect(cell, colSelected(c) ? o.headerSelected : o.headerBackground);
                p->fillRect(QRect(b - 1, 0, 1, hh), o.headerLine);
                p->fillRect(QRect(a, hh - 1, b - a, 1), o.headerLine);
                if (cell.width() >= 12) {
                    p->setPen(o.headerText);
                    drawHeaderText(p, cell, columnName(quint32(c)));
                }
                ++m_stats.headersDrawn;
                ++drawn;
            }
        }
    }
    for (Pane which : {Pane::Top, Pane::Body}) {
        // the numbers: down the side, beside the rows of this part
        const PaneView pv = L.pane(which);
        const bool frozen = which == Pane::Top;
        const QRect band(0, pv.rect.top(), hw, pv.rect.height());
        if (band.height() > 0 && band.intersects(clip)) {
            const QRect area = band & clip;
            p->setClipRect(area);
            p->fillRect(area, o.headerBackground);
            int steps = 0, drawn = 0;
            for (qint64 r = pv.firstRow; !pv.isEmpty() && r <= pv.lastRow && r < L.rows.count() && drawn < kMaxRows && ++steps < kMaxScanSteps; r = L.rows.nextVisible(r)) {
                const int a = L.yEdge(r, frozen), b = L.yEdge(r + 1, frozen);
                if (b <= area.top()) continue;
                if (a > area.bottom()) break;
                if (b <= a) continue;
                const QRect cell(0, a, hw, b - a);
                p->fillRect(cell, rowSelected(r) ? o.headerSelected : o.headerBackground);
                p->fillRect(QRect(hw - 1, a, 1, b - a), o.headerLine);
                p->fillRect(QRect(0, b - 1, hw, 1), o.headerLine);
                if (cell.height() >= 10) {
                    p->setPen(o.headerText);
                    drawHeaderText(p, cell, QString::number(r + 1));
                }
                ++m_stats.headersDrawn;
                ++drawn;
            }
        }
    }
    p->setClipping(false);
}

void GridPainter::paint(QPainter *p, const QRect &clip, const PaintOptions &o)
{
    m_stats = PaintStats();
    m_mono = o.monochrome;
    m_strokedDashes = o.strokedDashes;
    m_sheet->conditional().beginPaint();   // (the steps this paint may spend on the rules' formulas)
    struct EndOfPaint {   // (what the rules kept of the cells, if the paint filled it, is let go of when the paint is over, not in the middle of it)
        const CondFormats &rules;
        ~EndOfPaint() { rules.endPaint(); }
    } endOfPaint{m_sheet->conditional()};
    if (!m_sheet->data().sheet) {
        p->fillRect(clip, o.backdrop);   // (no sheet to show)
        return;
    }
    const qint64 textsBefore = m_sheet->textsMade(), looksBefore = m_sheet->looks().made();
    m_sheet->looks().trim();
    const double zoom = m_sheet->layout().zoom();
    if (zoom != m_fontZoom) {
        m_fontMade.assign(m_fontMade.size(), false);
        m_fontZoom = zoom;
    }
    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);
    p->setRenderHint(QPainter::SmoothPixmapTransform, false);
    // (a sheet that reads right to left is drawn as the layout has it and mirrored: column A is at the right edge; the text is drawn as it reads, see drawText)
    if (m_sheet->rightToLeft()) {
        p->translate(m_sheet->layout().viewport().width(), 0);
        p->scale(-1, 1);
    }
    for (Pane which : {Pane::Body, Pane::Left, Pane::Top, Pane::Corner}) paintPane(p, which, clip, o);
    if (o.headers) paintHeaders(p, clip, o);
    if (!o.dropDown.isEmpty() && o.dropDown.intersects(clip)) {
        // a small button with an arrow
        p->setPen(QPen(o.headerLine, 1));
        p->setBrush(o.headerBackground);
        p->drawRect(o.dropDown.adjusted(0, 0, -1, -1));
        p->setPen(Qt::NoPen);
        p->setBrush(o.headerText);
        const QPoint c = o.dropDown.center();
        const QPointF triangle[3] = {QPointF(c.x() - 4, c.y() - 2), QPointF(c.x() + 4, c.y() - 2), QPointF(c.x(), c.y() + 3)};
        p->setRenderHint(QPainter::Antialiasing, true);
        p->drawPolygon(triangle, 3);
        p->setRenderHint(QPainter::Antialiasing, false);
    }
    if (o.resizeGuide >= 0) {
        // the border of a row or column being dragged: a dashed line across the cells
        QPen pen(o.accent, 1, Qt::DashLine);
        p->setPen(pen);
        const QRect all = clip;
        if (o.resizeGuideColumn) p->drawLine(o.resizeGuide, all.top(), o.resizeGuide, all.bottom());
        else p->drawLine(all.left(), o.resizeGuide, all.right(), o.resizeGuide);
    }
    p->restore();
    m_stats.textsMade = m_sheet->textsMade() - textsBefore;
    m_stats.looksMade = m_sheet->looks().made() - looksBefore;
}

} // namespace jp::grid
