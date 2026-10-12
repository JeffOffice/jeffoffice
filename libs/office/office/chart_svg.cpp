#include "office/chart.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QSvgGenerator>
#include <algorithm>
#include <cmath>
#include <limits>

namespace jp::office {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

QString say(const char *text) { return QCoreApplication::translate("Chart", text); }

// The color a fraction of the way to white, in linear light: what the chart styles do to the text color for the lines of a chart.
QColor tinted(const QColor &c, double keep)
{
    auto linear = [](double v) { return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
    auto gamma = [](double v) { return v <= 0.0031308 ? v * 12.92 : 1.055 * std::pow(v, 1 / 2.4) - 0.055; };
    auto one = [&](int channel) { return int(std::lround(std::clamp(gamma(linear(channel / 255.0) * keep + (1 - keep)), 0.0, 1.0) * 255)); };
    return QColor(one(c.red()), one(c.green()), one(c.blue()), c.alpha());
}

// The smallest 1, 2, or 5 times a power of ten that is not below `target`.
double niceUnit(double target)
{
    if (!(target > 0) || !std::isfinite(target)) return 1;
    const double base = std::pow(10.0, std::floor(std::log10(target)));
    for (double m : {1.0, 2.0, 5.0})
        if (m * base >= target * (1 - 1e-9)) return m * base;
    return 10 * base;
}

// Where an axis begins and ends and how often it is marked.
struct Scale {
    double min = 0, max = 1, major = 0.2;
};

// An axis scaled to its data the way charts are: the baseline is zero unless
// the values are all far from it, there is a little room above the largest,
// and the marks are at nice steps, no more than ten to the axis.
Scale scaleOf(const ChartAxis &axis, double dataLow, double dataHigh, bool percent)
{
    if (!(dataHigh >= dataLow)) dataLow = 0, dataHigh = 1;
    double low = dataLow, high = dataHigh;
    if (percent) {
        low = 0;
        high = 1;
    } else {
        if (low >= 0 && high >= 0) {
            if (high == low || low < high * 5.0 / 6.0) low = 0;
        } else if (low <= 0 && high <= 0) {
            if (high == low || high > low * 5.0 / 6.0) high = 0;
        }
        double span = high - low;
        if (span <= 0) {
            high = low + 1;
            span = 1;
        }
        const double top = high > 0 ? high + 0.05 * span : high, bottom = low < 0 ? low - 0.05 * span : low;
        high = top;
        low = bottom;
    }
    const double from = axis.autoMin ? low : axis.min;
    double to = axis.autoMax ? high : axis.max;
    if (!(to > from)) to = from + 1;
    double unit = axis.majorUnit > 0 ? axis.majorUnit : niceUnit((to - from) / 10);
    while ((to - from) / unit > 500) unit *= 2;
    Scale s;
    s.major = unit;
    s.min = axis.autoMin ? std::floor(low / unit + 1e-9) * unit : axis.min;
    s.max = axis.autoMax ? std::ceil(high / unit - 1e-9) * unit : axis.max;
    if (!(s.max > s.min)) s.max = s.min + unit;
    return s;
}

QVector<qreal> dashPattern(const QString &name)
{
    if (name == QLatin1String("sysDot")) return {1, 1};
    if (name == QLatin1String("sysDash")) return {3, 1};
    if (name == QLatin1String("dot")) return {1, 3};
    if (name == QLatin1String("dash")) return {4, 3};
    if (name == QLatin1String("lgDash")) return {8, 3};
    if (name == QLatin1String("dashDot")) return {4, 3, 1, 3};
    if (name == QLatin1String("sysDashDot")) return {3, 1, 1, 1};
    if (name == QLatin1String("lgDashDot")) return {8, 3, 1, 3};
    if (name == QLatin1String("sysDashDotDot")) return {3, 1, 1, 1, 1, 1};
    if (name == QLatin1String("lgDashDotDot")) return {8, 3, 1, 3, 1, 3};
    return {};
}

// What a series' points stand at in the chart: for each category, where its piece begins and ends.
struct Seg {
    double from = 0, to = kNaN;   // `to` is NaN for a blank
};

struct Entry {
    QString name;
    QColor color;
    bool line = false;            // its key is a line (with its marker) rather than a square
    ChartMarker::Symbol marker = ChartMarker::None;
    QColor markerFill;
    double width = 2.25;
};

class ChartPainter {
public:
    ChartPainter(const Chart &chart, QPainter *p, QPaintDevice *device, const QSizeF &size) : m_chart(chart), m_p(p), m_dev(device), m_size(size) {}
    void paint();

private:
    // ---- text
    QFont fontOf(const ChartFont &own, double size, bool bold) const;
    QColor colorOf(const ChartFont &own) const;
    double widthOf(const QFont &f, const QString &text) const;
    double heightOf(const QFont &f, int lines = 1) const;
    // Draws the text so that the anchor falls at `ha` across it (0: its left, 1: its right) and `va` down it, the text turned `angle` degrees about the anchor.
    void drawText(const QString &text, const QFont &font, const QColor &color, const QPointF &anchor, double ha, double va, double angle = 0);
    QPen penFor(const ChartLine &line, const QColor &fallback, double fallbackWidth) const;

    // ---- the pieces of a chart
    QVector<Entry> legendEntries() const;
    QSizeF legendSize(const QVector<Entry> &entries, double available, QVector<int> *rowBreaks) const;
    void drawLegend(const QRectF &box, const QVector<Entry> &entries, const QVector<int> &rowBreaks);
    void drawKey(const Entry &e, const QRectF &key);
    void drawMarker(const QPointF &c, ChartMarker::Symbol symbol, double size, const QColor &fill, const QColor &line, bool noFill, bool noLine);
    void paintPie(const QRectF &area);
    void paintAxes(const QRectF &area);
    void drawLabels(const ChartPlot &plot, const ChartSeries &s, int index, QPointF at, double total, double ha, double va, bool inside, const QColor &under);
    QString labelText(const ChartPlot &plot, const ChartSeries &s, int i, double total, const ChartDataLabels &l) const;
    QColor pointColor(const ChartPlot &plot, const ChartSeries &s, int i) const;
    void titleBlock(const ChartTitle &t, double defaultSize, QFont *font, QColor *color) const;

    // ---- the plot with axes
    struct View {
        const ChartPlot *plot = nullptr;
        ChartType type = ChartType::Bar;
        bool secondary = false;
        QVector<QVector<Seg>> segs;       // by series, then category
    };
    void stack(View *v) const;
    double catUnit(int i) const { return m_between ? i + 0.5 : i; }
    // Positions along the category axis (0 at the first category's side) and the value axis (from its minimum) in the plot, as the page's x and y.
    QPointF at(double unit, double value, const Scale &scale) const;
    double valueAlong(double value, const Scale &scale) const;
    double catAlong(double unit) const;
    QRectF rectOf(double u0, double u1, double v0, double v1, const Scale &scale) const;
    void drawBars(const View &v, const Scale &scale);
    void drawLines(const View &v, const Scale &scale);
    void drawAreas(const View &v, const Scale &scale);
    void drawScatter(const View &v, const Scale &xs, const Scale &ys);
    QStringList categoryLabels() const;
    QString valueText(double v, const ChartAxis &axis, const ChartSeries *first, bool percent) const;
    struct LabelFit {
        double rotation = 0;
        int skip = 1;
        int lines = 1;
        double extent = 0;   // how far the labels reach from the axis, across it
    };
    LabelFit fitLabels(const QStringList &labels, const QFont &font, double slot, const ChartAxis &axis, bool alongX) const;
    QStringList wrapped(const QString &text, const QFont &font, double width) const;

    const Chart &m_chart;
    QPainter *m_p;
    QPaintDevice *m_dev;
    QSizeF m_size;

    // The plot with axes, once laid out.
    QRectF m_inner;
    bool m_swap = false;                  // categories run up the page (bars across)
    bool m_between = true;
    int m_n = 0;                          // categories
    double m_slot = 1;                    // a category's length along its axis
    bool m_catReversed = false, m_valReversed = false;
    QVector<View> m_views;
    const ChartAxis *m_cat = nullptr, *m_val = nullptr, *m_val2 = nullptr;
    ChartAxis m_defaultCat, m_defaultVal;
    Scale m_scale, m_scale2;
    Scale m_xScale;                       // a scatter's horizontal scale
    bool m_percent = false;
    double m_baseline = 0;
};

// ---- text and lines

QFont ChartPainter::fontOf(const ChartFont &own, double size, bool bold) const
{
    const QString family = !own.family.isEmpty() ? own.family : !m_chart.font.family.isEmpty() ? m_chart.font.family : m_chart.fontFamily;
    QFont f(family);
    f.setPointSizeF(own.size > 0 ? own.size : m_chart.font.size > 0 ? m_chart.font.size : size);
    f.setBold(own.bold >= 0 ? own.bold == 1 : m_chart.font.bold >= 0 ? m_chart.font.bold == 1 : bold);
    f.setItalic(own.italic >= 0 ? own.italic == 1 : m_chart.font.italic == 1);
    return f;
}

QColor ChartPainter::colorOf(const ChartFont &own) const
{
    if (own.color.isValid()) return own.color;
    if (m_chart.font.color.isValid()) return m_chart.font.color;
    return m_chart.textColor.isValid() ? m_chart.textColor : QColor(Qt::black);
}

double ChartPainter::widthOf(const QFont &f, const QString &text) const
{
    return QFontMetricsF(f, m_dev).horizontalAdvance(text);
}

double ChartPainter::heightOf(const QFont &f, int lines) const
{
    return QFontMetricsF(f, m_dev).height() * lines;
}

void ChartPainter::drawText(const QString &text, const QFont &font, const QColor &color, const QPointF &anchor, double ha, double va, double angle)
{
    const QFontMetricsF fm(font, m_dev);
    const QStringList lines = text.split(QLatin1Char('\n'));
    const double total = fm.height() * lines.size();
    m_p->save();
    m_p->translate(anchor);
    if (angle != 0) m_p->rotate(angle);
    m_p->setFont(font);
    m_p->setPen(color);
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].isEmpty()) continue;
        // A line of several is centered or flush as the text is.
        const double w = fm.horizontalAdvance(lines[i]);
        m_p->drawText(QPointF(-w * ha, -total * va + i * fm.height() + fm.ascent()), lines[i]);
    }
    m_p->restore();
}

// The pen for a line the file may or may not say anything about; a "none" line has no pen.
QPen ChartPainter::penFor(const ChartLine &line, const QColor &fallback, double fallbackWidth) const
{
    if (line.none) return Qt::NoPen;
    const QColor c = line.color.isValid() ? line.color : fallback;
    QPen pen(c, line.width > 0 ? line.width : fallbackWidth, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    if (const QVector<qreal> dashes = dashPattern(line.dash); !dashes.isEmpty()) {
        pen.setDashPattern(dashes);
        pen.setCapStyle(Qt::FlatCap);
    }
    return pen;
}

void ChartPainter::titleBlock(const ChartTitle &t, double defaultSize, QFont *font, QColor *color) const
{
    *font = fontOf(t.font, defaultSize, true);
    *color = colorOf(t.font);
}

// ---- keys and markers

QColor ChartPainter::pointColor(const ChartPlot &plot, const ChartSeries &s, int i) const
{
    if (s.pointColors.contains(i)) return s.pointColors.value(i);
    // Slices take the palette's colors in turn; so do the bars of one series that varies its colors, if the file gave it none.
    const bool slices = plot.type == ChartType::Pie || plot.type == ChartType::Doughnut;
    if ((slices || (plot.varyColors && !s.colorFromFile && plot.series.size() == 1)) && !m_chart.palette.isEmpty()) return m_chart.palette[i % m_chart.palette.size()];
    return s.color;
}

void ChartPainter::drawMarker(const QPointF &c, ChartMarker::Symbol symbol, double size, const QColor &fill, const QColor &line, bool noFill, bool noLine)
{
    if (symbol == ChartMarker::None) return;
    const double r = size / 2;
    m_p->save();
    const QPen stroke = noLine ? QPen(Qt::NoPen) : QPen(line, 0.75, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    m_p->setPen(stroke);
    m_p->setBrush(noFill ? QBrush(Qt::NoBrush) : QBrush(fill));
    switch (symbol) {
    case ChartMarker::Circle:
        m_p->drawEllipse(c, r, r);
        break;
    case ChartMarker::Dot:
        m_p->drawEllipse(c, r * 0.6, r * 0.6);
        break;
    case ChartMarker::Square:
        m_p->drawRect(QRectF(c.x() - r, c.y() - r, size, size));
        break;
    case ChartMarker::Diamond:
        m_p->drawPolygon(QPolygonF({QPointF(c.x(), c.y() - r * 1.2), QPointF(c.x() + r * 1.2, c.y()), QPointF(c.x(), c.y() + r * 1.2), QPointF(c.x() - r * 1.2, c.y())}));
        break;
    case ChartMarker::Triangle:
        m_p->drawPolygon(QPolygonF({QPointF(c.x(), c.y() - r * 1.1), QPointF(c.x() + r * 1.1, c.y() + r * 0.9), QPointF(c.x() - r * 1.1, c.y() + r * 0.9)}));
        break;
    case ChartMarker::X:
    case ChartMarker::Plus:
    case ChartMarker::Star:
    case ChartMarker::Dash: {
        // Marks drawn with strokes alone take the fill's color when the line has none.
        m_p->setPen(QPen(noLine ? fill : line, 1.0, Qt::SolidLine, Qt::FlatCap));
        if (symbol == ChartMarker::Dash) {
            m_p->drawLine(QPointF(c.x() - r, c.y()), QPointF(c.x() + r, c.y()));
        } else {
            if (symbol != ChartMarker::Plus) {
                m_p->drawLine(QPointF(c.x() - r, c.y() - r), QPointF(c.x() + r, c.y() + r));
                m_p->drawLine(QPointF(c.x() - r, c.y() + r), QPointF(c.x() + r, c.y() - r));
            }
            if (symbol != ChartMarker::X) {
                m_p->drawLine(QPointF(c.x() - r, c.y()), QPointF(c.x() + r, c.y()));
                m_p->drawLine(QPointF(c.x(), c.y() - r), QPointF(c.x(), c.y() + r));
            }
        }
        break;
    }
    default:
        break;
    }
    m_p->restore();
}

// ---- the legend

QVector<Entry> ChartPainter::legendEntries() const
{
    QVector<Entry> out;
    static const ChartMarker::Symbol autoSymbols[] = {ChartMarker::Diamond, ChartMarker::Square, ChartMarker::Triangle, ChartMarker::X,
                                                      ChartMarker::Star,    ChartMarker::Circle, ChartMarker::Plus,     ChartMarker::Dash};
    int index = 0;
    for (const ChartPlot &plot : m_chart.plots) {
        const bool slices = plot.type == ChartType::Pie || plot.type == ChartType::Doughnut;
        if ((slices || (plot.varyColors && plot.series.size() == 1 && plot.type == ChartType::Bar)) && !plot.series.isEmpty()) {
            // One entry for each category, in its slice's color.
            const ChartSeries &s = plot.series[0];
            const int n = int(std::max(s.categories.size(), s.values.size()));
            for (int i = 0; i < n; ++i) {
                Entry e;
                e.name = i < s.categories.size() && !s.categories[i].isEmpty() ? s.categories[i] : QString::number(i + 1);
                e.color = pointColor(plot, s, i);
                out << e;
            }
            continue;
        }
        for (const ChartSeries &s : plot.series) {
            Entry e;
            e.name = s.name;
            e.color = s.color;
            if (plot.type == ChartType::Line || plot.type == ChartType::Scatter) {
                e.line = !s.outline.none && (plot.type == ChartType::Line || plot.lines);
                e.width = s.outline.width > 0 ? s.outline.width : 2.25;
                ChartMarker::Symbol symbol = s.marker.symbol;
                if (symbol == ChartMarker::Auto) symbol = plot.markers || plot.type == ChartType::Scatter ? autoSymbols[index % 8] : ChartMarker::None;
                e.marker = symbol;
                e.markerFill = s.marker.fill.isValid() ? s.marker.fill : s.color;
            }
            out << e;
            ++index;
        }
    }
    return out;
}

// The size of the legend, and where its rows break when it runs along the top or bottom.
QSizeF ChartPainter::legendSize(const QVector<Entry> &entries, double available, QVector<int> *rowBreaks) const
{
    const QFont font = fontOf(m_chart.legend.font, 10, false);
    const double keyW = 7, gap = 4, between = 12, pad = 5, row = heightOf(font) + 1;
    const bool across = m_chart.legend.position == QLatin1String("t") || m_chart.legend.position == QLatin1String("b");
    double widest = 0, rowWidth = 0, widthSum = 0;
    int rows = 1;
    rowBreaks->clear();
    for (int i = 0; i < entries.size(); ++i) {
        const double w = (entries[i].line ? 20 : keyW) + gap + widthOf(font, entries[i].name);
        widest = std::max(widest, w);
        if (across) {
            if (rowWidth > 0 && rowWidth + between + w > available) {
                rowBreaks->append(i);
                ++rows;
                widthSum = std::max(widthSum, rowWidth);
                rowWidth = 0;
            }
            rowWidth += (rowWidth > 0 ? between : 0) + w;
        }
    }
    if (across) return QSizeF(std::max(widthSum, rowWidth) + 2 * pad, rows * row + 2 * pad - 2);
    return QSizeF(widest + 2 * pad, entries.size() * row + 2 * pad - 2);
}

void ChartPainter::drawKey(const Entry &e, const QRectF &key)
{
    if (e.line) {
        m_p->setPen(QPen(e.color, std::min(e.width, 3.0), Qt::SolidLine, Qt::FlatCap));
        m_p->drawLine(QPointF(key.left(), key.center().y()), QPointF(key.right(), key.center().y()));
        drawMarker(key.center(), e.marker, std::min(7.0, key.height()), e.markerFill, e.color, false, false);
    } else if (e.marker != ChartMarker::None) {
        drawMarker(key.center(), e.marker, std::min(7.0, key.height()), e.markerFill, e.color, false, false);
    } else {
        m_p->fillRect(key, e.color);
    }
}

void ChartPainter::drawLegend(const QRectF &box, const QVector<Entry> &entries, const QVector<int> &rowBreaks)
{
    const QFont font = fontOf(m_chart.legend.font, 10, false);
    const QColor color = colorOf(m_chart.legend.font);
    const double keyW = 7, gap = 4, between = 12, pad = 5, row = heightOf(font) + 1;
    const bool across = m_chart.legend.position == QLatin1String("t") || m_chart.legend.position == QLatin1String("b");
    m_p->save();
    m_p->setPen(Qt::NoPen);
    if (!across) {
        double y = box.top() + pad - 1;
        for (const Entry &e : entries) {
            const double kw = e.line ? 20 : keyW;
            drawKey(e, QRectF(box.left() + pad, y + row / 2 - keyW / 2, kw, keyW));
            drawText(e.name, font, color, QPointF(box.left() + pad + kw + gap, y + row / 2), 0, 0.5);
            y += row;
        }
    } else {
        int from = 0;
        double y = box.top() + pad - 1;
        QVector<int> ends = rowBreaks;
        ends.append(int(entries.size()));
        for (int end : ends) {
            double w = 0;
            for (int i = from; i < end; ++i) w += (i > from ? between : 0) + (entries[i].line ? 20 : keyW) + gap + widthOf(font, entries[i].name);
            double x = box.center().x() - w / 2;
            for (int i = from; i < end; ++i) {
                const double kw = entries[i].line ? 20 : keyW;
                drawKey(entries[i], QRectF(x, y + row / 2 - keyW / 2, kw, keyW));
                drawText(entries[i].name, font, color, QPointF(x + kw + gap, y + row / 2), 0, 0.5);
                x += kw + gap + widthOf(font, entries[i].name) + between;
            }
            y += row;
            from = end;
        }
    }
    m_p->restore();
}

// ---- data labels

QString ChartPainter::labelText(const ChartPlot &plot, const ChartSeries &s, int i, double total, const ChartDataLabels &l) const
{
    QStringList parts;
    if (l.series) parts << s.name;
    if (l.category) {
        if (plot.type == ChartType::Scatter && i < s.xValues.size()) parts << formatChartNumber(s.xValues[i], QString());
        else if (i < s.categories.size() && !s.categories[i].isEmpty()) parts << s.categories[i];
        else parts << QString::number(i + 1);
    }
    const double v = i < s.values.size() ? s.values[i] : kNaN;
    if (l.value && !std::isnan(v)) parts << formatChartNumber(v, l.format.isEmpty() ? s.format : l.format);
    if (l.percent && total > 0 && !std::isnan(v)) parts << formatChartNumber(v / total, l.format.contains(QLatin1Char('%')) ? l.format : QStringLiteral("0%"));
    return parts.join(l.separator.isEmpty() ? QStringLiteral(", ") : l.separator);
}

// Writes the label of one point, near `at`: `ha` and `va` say where the label sits against it.
void ChartPainter::drawLabels(const ChartPlot &plot, const ChartSeries &s, int index, QPointF at, double total, double ha, double va, bool inside, const QColor &under)
{
    const QString text = labelText(plot, s, index, total, s.labels);
    if (text.isEmpty()) return;
    const QFont font = fontOf(s.labels.font, 10, false);
    QColor color = colorOf(s.labels.font);
    // Text on a dark fill is light, as long as the file didn't choose its color.
    if (inside && under.isValid() && !s.labels.font.color.isValid() && under.lightnessF() < 0.45) color = Qt::white;
    drawText(text, font, color, at, ha, va);
}

// ---- pies

void ChartPainter::paintPie(const QRectF &area)
{
    const ChartPlot &plot = m_chart.plots.first();
    if (plot.series.isEmpty()) return;
    const bool ring = plot.type == ChartType::Doughnut;
    const int rings = ring ? int(plot.series.size()) : 1;
    const ChartSeries &first = plot.series[0];
    // Slices pulled out make the pie smaller, so they stay inside; labels outside need room too.
    double maxExplosion = 0;
    for (const ChartSeries &s : plot.series) maxExplosion = std::max(maxExplosion, s.explosion);
    bool outside = false;
    for (const ChartSeries &s : plot.series)
        if (s.labels.shown() && (s.labels.position == QLatin1String("outEnd") || s.labels.position == QLatin1String("bestFit"))) outside = true;
    const QFont labelFont = fontOf(first.labels.font, 10, false);
    const double room = outside ? heightOf(labelFont) * 1.6 : 0;
    const double side = std::min(area.width(), area.height()) - 2 * room;
    if (side <= 4) return;
    const double radius = side / 2 / (1 + maxExplosion / 100.0) * 0.96;
    const QPointF center = area.center();
    for (int r = 0; r < rings; ++r) {
        const ChartSeries &s = plot.series[r];
        double total = 0;
        for (double v : s.values)
            if (std::isfinite(v) && v > 0) total += v;
        if (total <= 0) continue;
        const double outer = ring ? radius - r * (radius * (1 - plot.holeSize / 100.0) / rings) : radius;
        const double inner = ring ? outer - radius * (1 - plot.holeSize / 100.0) / rings : 0;
        double angle = plot.firstSliceAngle;   // degrees clockwise from twelve o'clock
        for (int i = 0; i < s.values.size(); ++i) {
            const double v = s.values[i];
            if (!std::isfinite(v) || v <= 0) continue;
            const double sweep = v / total * 360;
            const double mid = (angle + sweep / 2) * kPi / 180;
            const QPointF dir(std::sin(mid), -std::cos(mid));
            const double pulled = radius * s.explosion / 100.0;
            const QPointF c = center + dir * pulled;
            // Qt's angles run counterclockwise from three o'clock, in sixteenths of a degree.
            QPainterPath path;
            const double start = 90 - angle;
            path.moveTo(c + QPointF(inner * std::cos(start * kPi / 180), -inner * std::sin(start * kPi / 180)));
            path.arcTo(QRectF(c.x() - outer, c.y() - outer, 2 * outer, 2 * outer), start, -sweep);
            if (inner > 0) path.arcTo(QRectF(c.x() - inner, c.y() - inner, 2 * inner, 2 * inner), start - sweep, sweep);
            else path.lineTo(c);
            path.closeSubpath();
            const QColor fill = pointColor(plot, s, i);
            m_p->setPen(s.outline.set ? penFor(s.outline, m_chart.backgroundColor, 0.75) : QPen(Qt::NoPen));
            m_p->setBrush(s.noFill ? QBrush(Qt::NoBrush) : QBrush(fill));
            m_p->drawPath(path);
            if (s.labels.shown()) {
                const QString pos = s.labels.position;
                const bool out = pos == QLatin1String("outEnd") || ((pos.isEmpty() || pos == QLatin1String("bestFit")) && sweep < 20);
                const double reach = out ? outer + 3 : pos == QLatin1String("inEnd") ? outer * 0.82 : pos == QLatin1String("inBase") ? inner + (outer - inner) * 0.2
                                     : ring || pos == QLatin1String("ctr") ? (inner + outer) / 2 * (ring ? 1 : 0.9) : outer * 0.7;
                // Outside, the label stands away from the pie on the side it is on; inside, it is centered on its place.
                drawLabels(plot, s, i, c + dir * reach, total, out ? std::clamp(0.5 - 1.7 * dir.x(), 0.0, 1.0) : 0.5, out ? std::clamp(0.5 - 1.7 * dir.y(), 0.0, 1.0) : 0.5, !out, fill);
            }
            angle += sweep;
        }
    }
}

// ---- stacking

void ChartPainter::stack(View *v) const
{
    const ChartPlot &plot = *v->plot;
    const bool stacked = plot.grouping == ChartGrouping::Stacked || plot.grouping == ChartGrouping::PercentStacked;
    const bool percent = plot.grouping == ChartGrouping::PercentStacked;
    v->segs.clear();
    v->segs.resize(plot.series.size());
    QVector<double> positive(m_n, 0), negative(m_n, 0), total(m_n, 0);
    if (percent)
        for (const ChartSeries &s : plot.series)
            for (int i = 0; i < m_n && i < s.values.size(); ++i)
                if (std::isfinite(s.values[i])) total[i] += std::fabs(s.values[i]);
    for (int k = 0; k < plot.series.size(); ++k) {
        const ChartSeries &s = plot.series[k];
        v->segs[k].resize(m_n);
        for (int i = 0; i < m_n; ++i) {
            double value = i < s.values.size() ? s.values[i] : kNaN;
            if (!std::isfinite(value)) {
                // A blank adds nothing to a stack, and stands where the stack is.
                v->segs[k][i].from = stacked ? positive[i] : 0;
                continue;
            }
            if (percent) value = total[i] > 0 ? value / total[i] : 0;
            if (!stacked) {
                v->segs[k][i] = {0, value};
            } else if (value >= 0) {
                v->segs[k][i] = {positive[i], positive[i] + value};
                positive[i] += value;
            } else {
                v->segs[k][i] = {negative[i], negative[i] + value};
                negative[i] += value;
            }
        }
    }
}

// ---- mapping to the page

// The page position along the value axis of a value: from the plot's bottom (or left, when categories run up the page).
double ChartPainter::valueAlong(double value, const Scale &scale) const
{
    double f = (value - scale.min) / (scale.max - scale.min);
    if (m_valReversed) f = 1 - f;
    f = std::clamp(f, 0.0, 1.0);
    return m_swap ? m_inner.left() + f * m_inner.width() : m_inner.bottom() - f * m_inner.height();
}

// The page position along the category axis of a place in category units.
double ChartPainter::catAlong(double unit) const
{
    const double u = m_catReversed ? (m_between ? m_n : std::max(1, m_n - 1)) - unit : unit;
    return m_swap ? m_inner.bottom() - u * m_slot : m_inner.left() + u * m_slot;
}

QPointF ChartPainter::at(double unit, double value, const Scale &scale) const
{
    const double c = catAlong(unit), v = valueAlong(value, scale);
    return m_swap ? QPointF(v, c) : QPointF(c, v);
}

QRectF ChartPainter::rectOf(double u0, double u1, double v0, double v1, const Scale &scale) const
{
    const QPointF a = at(u0, v0, scale), b = at(u1, v1, scale);
    return QRectF(a, b).normalized();
}

// ---- the kinds of plot

void ChartPainter::drawBars(const View &v, const Scale &scale)
{
    const ChartPlot &plot = *v.plot;
    const bool stacked = plot.grouping == ChartGrouping::Stacked || plot.grouping == ChartGrouping::PercentStacked;
    const int groupSize = stacked ? 1 : int(plot.series.size());
    const double overlap = stacked ? 1.0 : plot.overlap / 100.0, gap = plot.gapWidth / 100.0;
    const double inGroup = groupSize - (groupSize - 1) * overlap;
    const double bar = 1.0 / (inGroup + gap);
    const double leftOff = (1.0 - bar * inGroup) / 2;
    m_p->save();
    for (int k = 0; k < plot.series.size(); ++k) {
        const ChartSeries &s = plot.series[k];
        const int slot = stacked ? 0 : k;
        for (int i = 0; i < m_n; ++i) {
            const Seg &seg = v.segs[k][i];
            if (!std::isfinite(seg.to)) continue;
            const double u0 = i + leftOff + slot * bar * (1 - overlap), u1 = u0 + bar;
            // Clustered bars stand on the line the category axis crosses at; stacked ones where their stack is.
            const double from = stacked ? seg.from : std::clamp(m_baseline, scale.min, scale.max), to = seg.to;
            // The unit of a category's place on the axis is its index: bars are placed from its edge.
            const QRectF r = rectOf(u0, u1, from, to, scale);
            if (r.width() <= 0 && r.height() <= 0) continue;
            const QColor fill = pointColor(plot, s, i);
            m_p->setBrush(s.noFill ? QBrush(Qt::NoBrush) : QBrush(fill));
            m_p->setPen(s.outline.set && !s.outline.none ? penFor(s.outline, fill, 0.75) : QPen(Qt::NoPen));
            m_p->drawRect(r);
            if (s.labels.shown()) {
                QString pos = s.labels.position;
                if (pos.isEmpty()) pos = stacked ? QStringLiteral("ctr") : QStringLiteral("outEnd");
                const bool up = to >= from;   // the bar's end is on the far side from the baseline
                QPointF p;
                double ha = 0.5, va = 0.5;
                bool inside = pos != QLatin1String("outEnd");
                if (!m_swap) {
                    p.setX(r.center().x());
                    if (pos == QLatin1String("ctr")) p.setY(r.center().y());
                    else if (pos == QLatin1String("inBase")) p = QPointF(r.center().x(), up ? r.bottom() - 2 : r.top() + 2), va = up ? 1 : 0;
                    else if (pos == QLatin1String("inEnd")) p = QPointF(r.center().x(), up ? r.top() + 2 : r.bottom() - 2), va = up ? 0 : 1;
                    else p = QPointF(r.center().x(), up ? r.top() - 2 : r.bottom() + 2), va = up ? 1 : 0;
                } else {
                    if (pos == QLatin1String("ctr")) p = r.center();
                    else if (pos == QLatin1String("inBase")) p = QPointF(up ? r.left() + 2 : r.right() - 2, r.center().y()), ha = up ? 0 : 1;
                    else if (pos == QLatin1String("inEnd")) p = QPointF(up ? r.right() - 2 : r.left() + 2, r.center().y()), ha = up ? 1 : 0;
                    else p = QPointF(up ? r.right() + 2 : r.left() - 2, r.center().y()), ha = up ? 0 : 1;
                }
                drawLabels(plot, s, i, p, 0, ha, va, inside, fill);
            }
        }
    }
    m_p->restore();
}

// A series' points in order as a path through them: broken at blanks (or not, as the chart says), smoothed when it asks.
static QPainterPath throughPoints(const QVector<QPointF> &pts, const QVector<bool> &present, bool smooth, const QString &blanks)
{
    QPainterPath path;
    QVector<QPointF> run;
    auto flush = [&]() {
        if (run.size() == 1) {
            path.moveTo(run[0]);
            path.lineTo(run[0]);
        } else if (run.size() > 1) {
            path.moveTo(run[0]);
            if (!smooth) {
                for (int i = 1; i < run.size(); ++i) path.lineTo(run[i]);
            } else {
                // Catmull-Rom through the points, as cubic curves.
                for (int i = 0; i + 1 < run.size(); ++i) {
                    const QPointF p0 = run[std::max(0, i - 1)], p1 = run[i], p2 = run[i + 1], p3 = run[std::min<int>(int(run.size()) - 1, i + 2)];
                    path.cubicTo(p1 + (p2 - p0) / 6.0, p2 - (p3 - p1) / 6.0, p2);
                }
            }
        }
        run.clear();
    };
    for (int i = 0; i < pts.size(); ++i) {
        if (present[i]) run << pts[i];
        else if (blanks != QLatin1String("span")) flush();
    }
    flush();
    return path;
}

void ChartPainter::drawLines(const View &v, const Scale &scale)
{
    const ChartPlot &plot = *v.plot;
    static const ChartMarker::Symbol autoSymbols[] = {ChartMarker::Diamond, ChartMarker::Square, ChartMarker::Triangle, ChartMarker::X,
                                                      ChartMarker::Star,    ChartMarker::Circle, ChartMarker::Plus,     ChartMarker::Dash};
    const bool stacked = plot.grouping == ChartGrouping::Stacked || plot.grouping == ChartGrouping::PercentStacked;
    m_p->save();
    for (int k = 0; k < plot.series.size(); ++k) {
        const ChartSeries &s = plot.series[k];
        QVector<QPointF> pts(m_n);
        QVector<bool> present(m_n, false);
        for (int i = 0; i < m_n; ++i) {
            double value = v.segs[k][i].to;
            const bool blank = !std::isfinite(value);
            if (blank && m_chart.blanksAs == QLatin1String("zero")) value = stacked ? v.segs[k][i].from : 0;
            present[i] = std::isfinite(value);
            pts[i] = present[i] ? at(catUnit(i), value, scale) : QPointF();
        }
        if (!s.outline.none) {
            m_p->setPen(penFor(s.outline, s.color, 2.25));
            m_p->setPen([&] {
                QPen pen = penFor(s.outline, s.color, 2.25);
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);
                return pen;
            }());
            m_p->setBrush(Qt::NoBrush);
            m_p->drawPath(throughPoints(pts, present, s.smooth, m_chart.blanksAs));
        }
        ChartMarker::Symbol symbol = s.marker.symbol;
        if (symbol == ChartMarker::Auto) symbol = plot.markers ? autoSymbols[k % 8] : ChartMarker::None;
        for (int i = 0; i < m_n; ++i) {
            if (!present[i] || !std::isfinite(v.segs[k][i].to)) continue;
            const QColor fill = s.marker.fill.isValid() ? s.marker.fill : s.color;
            drawMarker(pts[i], symbol, s.marker.size, fill, s.marker.line.isValid() ? s.marker.line : fill, s.marker.noFill, s.marker.noLine);
        }
        if (s.labels.shown()) {
            for (int i = 0; i < m_n; ++i) {
                if (!present[i] || !std::isfinite(v.segs[k][i].to)) continue;
                const QString pos = s.labels.position.isEmpty() ? QStringLiteral("r") : s.labels.position;
                QPointF p = pts[i];
                double ha = 0, va = 0.5;
                if (pos == QLatin1String("t")) p += QPointF(0, -s.marker.size / 2 - 2), ha = 0.5, va = 1;
                else if (pos == QLatin1String("b")) p += QPointF(0, s.marker.size / 2 + 2), ha = 0.5, va = 0;
                else if (pos == QLatin1String("l")) p += QPointF(-s.marker.size / 2 - 2, 0), ha = 1;
                else if (pos == QLatin1String("ctr")) ha = 0.5;
                else p += QPointF(s.marker.size / 2 + 2, 0);
                drawLabels(plot, s, i, p, 0, ha, va, pos == QLatin1String("ctr"), QColor());
            }
        }
    }
    m_p->restore();
}

void ChartPainter::drawAreas(const View &v, const Scale &scale)
{
    const ChartPlot &plot = *v.plot;
    const bool stacked = plot.grouping == ChartGrouping::Stacked || plot.grouping == ChartGrouping::PercentStacked;
    const double base = std::clamp(m_baseline, scale.min, scale.max);
    m_p->save();
    for (int k = 0; k < plot.series.size(); ++k) {
        const ChartSeries &s = plot.series[k];
        // Runs of points with values: each is one shape; a blank ends a run (a gap), is a zero, or is bridged, as the chart says.
        QVector<QVector<int>> runs;
        QVector<int> run;
        for (int i = 0; i < m_n; ++i) {
            const bool blank = !std::isfinite(v.segs[k][i].to);
            if (!blank || m_chart.blanksAs == QLatin1String("zero")) run << i;
            else if (m_chart.blanksAs != QLatin1String("span")) {
                if (!run.isEmpty()) runs << run;
                run.clear();
            }
        }
        if (!run.isEmpty()) runs << run;
        m_p->setBrush(s.noFill ? QBrush(Qt::NoBrush) : QBrush(s.color));
        m_p->setPen(s.outline.set && !s.outline.none ? penFor(s.outline, s.color, 0.75) : QPen(Qt::NoPen));
        for (const QVector<int> &r : runs) {
            QPolygonF shape;
            for (int i : r) {
                const double top = std::isfinite(v.segs[k][i].to) ? v.segs[k][i].to : (stacked ? v.segs[k][i].from : 0);
                shape << at(catUnit(i), top, scale);
            }
            for (int j = int(r.size()) - 1; j >= 0; --j) {
                const int i = r[j];
                shape << at(catUnit(i), stacked ? v.segs[k][i].from : base, scale);
            }
            m_p->drawPolygon(shape);
        }
    }
    m_p->restore();
}

void ChartPainter::drawScatter(const View &v, const Scale &xs, const Scale &ys)
{
    const ChartPlot &plot = *v.plot;
    static const ChartMarker::Symbol autoSymbols[] = {ChartMarker::Diamond, ChartMarker::Square, ChartMarker::Triangle, ChartMarker::X,
                                                      ChartMarker::Star,    ChartMarker::Circle, ChartMarker::Plus,     ChartMarker::Dash};
    m_p->save();
    for (int k = 0; k < plot.series.size(); ++k) {
        const ChartSeries &s = plot.series[k];
        const int n = int(std::min(s.values.size(), s.xValues.size()));
        QVector<QPointF> pts(n);
        QVector<bool> present(n, false);
        for (int i = 0; i < n; ++i) {
            present[i] = std::isfinite(s.values[i]) && std::isfinite(s.xValues[i]);
            if (!present[i]) continue;
            const double fx = (s.xValues[i] - xs.min) / (xs.max - xs.min), fy = (s.values[i] - ys.min) / (ys.max - ys.min);
            pts[i] = QPointF(m_inner.left() + (m_catReversed ? 1 - fx : fx) * m_inner.width(), m_inner.bottom() - (m_valReversed ? 1 - fy : fy) * m_inner.height());
        }
        if (plot.lines && !s.outline.none) {
            QPen pen = penFor(s.outline, s.color, 2.25);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            m_p->setPen(pen);
            m_p->setBrush(Qt::NoBrush);
            m_p->drawPath(throughPoints(pts, present, s.smooth, QStringLiteral("gap")));
        }
        ChartMarker::Symbol symbol = s.marker.symbol;
        if (symbol == ChartMarker::Auto) symbol = autoSymbols[k % 8];
        if (plot.element == QLatin1String("bubbleChart") && s.marker.symbol == ChartMarker::Auto) symbol = ChartMarker::Circle;
        for (int i = 0; i < n; ++i) {
            if (!present[i]) continue;
            const QColor fill = s.marker.fill.isValid() ? s.marker.fill : s.color;
            drawMarker(pts[i], symbol, plot.element == QLatin1String("bubbleChart") ? std::max(s.marker.size, 10.0) : s.marker.size, fill, s.marker.line.isValid() ? s.marker.line : fill,
                       s.marker.noFill, s.marker.noLine);
            if (s.labels.shown()) {
                const QString pos = s.labels.position.isEmpty() ? QStringLiteral("r") : s.labels.position;
                QPointF p = pts[i];
                double ha = 0, va = 0.5;
                if (pos == QLatin1String("t")) p += QPointF(0, -s.marker.size / 2 - 2), ha = 0.5, va = 1;
                else if (pos == QLatin1String("b")) p += QPointF(0, s.marker.size / 2 + 2), ha = 0.5, va = 0;
                else if (pos == QLatin1String("l")) p += QPointF(-s.marker.size / 2 - 2, 0), ha = 1;
                else if (pos == QLatin1String("ctr")) ha = 0.5;
                else p += QPointF(s.marker.size / 2 + 2, 0);
                drawLabels(plot, s, i, p, 0, ha, va, false, QColor());
            }
        }
    }
    m_p->restore();
}

// ---- axes

QStringList ChartPainter::categoryLabels() const
{
    QStringList labels;
    for (const ChartPlot &plot : m_chart.plots)
        for (const ChartSeries &s : plot.series)
            if (labels.isEmpty() && !s.categories.isEmpty()) labels = s.categories;
    // A chart that names no categories numbers them.
    while (labels.size() < m_n) labels << QString::number(labels.size() + 1);
    return labels;
}

QString ChartPainter::valueText(double v, const ChartAxis &axis, const ChartSeries *first, bool percent) const
{
    if (std::fabs(v) < 1e-12) v = 0;
    if (!axis.formatLinked) return formatChartNumber(v, axis.format);
    if (percent) return formatChartNumber(v, QStringLiteral("0%"));
    return formatChartNumber(v, first ? first->format : QString());
}

// A label broken at spaces into lines no wider than `width`, when its words each fit.
QStringList ChartPainter::wrapped(const QString &text, const QFont &font, double width) const
{
    QStringList words = text.split(QLatin1Char(' '), Qt::SkipEmptyParts), lines;
    QString line;
    for (const QString &w : std::as_const(words)) {
        if (widthOf(font, w) > width) return {text};
        const QString tryLine = line.isEmpty() ? w : line + QLatin1Char(' ') + w;
        if (!line.isEmpty() && widthOf(font, tryLine) > width) {
            lines << line;
            line = w;
        } else {
            line = tryLine;
        }
    }
    if (!line.isEmpty()) lines << line;
    return lines.isEmpty() ? QStringList{text} : lines;
}

// How the labels of a category axis go: level, or turned a half right angle, and every how-manyth is shown, from how much room each category has.
ChartPainter::LabelFit ChartPainter::fitLabels(const QStringList &labels, const QFont &font, double slot, const ChartAxis &axis, bool alongX) const
{
    LabelFit fit;
    if (labels.isEmpty() || axis.labelPosition == QLatin1String("none") || axis.deleted) return fit;
    const double lineH = heightOf(font);
    double widest = 0;
    for (const QString &l : labels) widest = std::max(widest, widthOf(font, l));
    fit.skip = axis.labelSkip > 0 ? axis.labelSkip : 1;
    if (!alongX) {
        // Up the page: the labels are level, one to a category; every nth if they crowd.
        if (axis.labelSkip <= 0 && lineH > slot) fit.skip = int(std::ceil(lineH * 1.1 / slot));
        fit.extent = widest;
        return fit;
    }
    const double room = slot * fit.skip - 4;
    if (axis.hasRotation) {
        fit.rotation = axis.labelRotation;
    } else if (widest > room) {
        // Words may go on a second and third line, if each fits as it is.
        int most = 1;
        bool fits = true;
        for (const QString &l : labels) {
            const QStringList lines = wrapped(l, font, room);
            if (lines.size() > 3 || (lines.size() == 1 && widthOf(font, l) > room)) fits = false;
            most = std::max(most, int(lines.size()));
        }
        if (fits && most > 1) {
            fit.lines = most;
        } else {
            fit.rotation = -45;
            if (axis.labelSkip <= 0) fit.skip = std::max(1, int(std::ceil(lineH / 0.7071 / slot - 1e-6)));
        }
    }
    const double angle = std::fabs(fit.rotation) * kPi / 180;
    fit.extent = widest * std::sin(angle) + lineH * fit.lines * std::cos(angle);
    return fit;
}

void ChartPainter::paintAxes(const QRectF &outer)
{
    const Chart &c = m_chart;
    if (c.plots.isEmpty()) return;
    // The axes the first plot is drawn against; a plot on another value axis has its own scale, marked at the far side.
    const ChartPlot &first = c.plots.first();
    const ChartAxis *a0 = c.axis(first.axisIds[0]), *a1 = c.axis(first.axisIds[1]);
    const bool scatter = first.type == ChartType::Scatter;
    m_defaultCat = ChartAxis();
    m_defaultVal = ChartAxis();
    m_defaultVal.kind = ChartAxis::Value;
    m_defaultVal.position = QStringLiteral("l");
    // The category (or horizontal) axis and the value axis, whichever order the file lists them in.
    if (a0 && a1 && a0->kind == ChartAxis::Value && a1->kind != ChartAxis::Value && !scatter) std::swap(a0, a1);
    m_cat = a0 ? a0 : (scatter ? &m_defaultVal : &m_defaultCat);
    m_val = a1 ? a1 : &m_defaultVal;
    m_val2 = nullptr;
    m_views.clear();
    bool anyBar = false;
    for (const ChartPlot &plot : c.plots) {
        View v;
        v.plot = &plot;
        v.type = plot.type == ChartType::Other ? ChartType::Bar : plot.type;
        const ChartAxis *own = c.axis(plot.axisIds[1]);
        if (own && own->kind != ChartAxis::Value) own = c.axis(plot.axisIds[0]);
        if (plot.type != ChartType::Scatter && own && own != m_val && own->kind == ChartAxis::Value) {
            v.secondary = true;
            m_val2 = own;
        }
        anyBar = anyBar || v.type == ChartType::Bar;
        m_views << v;
    }
    m_swap = first.type == ChartType::Bar && first.horizontal;
    m_between = anyBar || m_val->between;
    m_catReversed = m_cat->reversed;
    m_valReversed = m_val->reversed;
    m_n = 0;
    for (const ChartPlot &plot : c.plots)
        for (const ChartSeries &s : plot.series) m_n = std::max<int>(m_n, int(std::max(s.values.size(), s.categories.size())));
    for (View &v : m_views) stack(&v);
    m_percent = first.grouping == ChartGrouping::PercentStacked;

    // ---- the scales, from the data of the plots on each value axis
    auto rangeOf = [&](bool secondary, double *lo, double *hi) {
        *lo = std::numeric_limits<double>::infinity();
        *hi = -std::numeric_limits<double>::infinity();
        for (const View &v : m_views) {
            if (v.secondary != secondary || v.type == ChartType::Scatter) continue;
            const bool stacked = v.plot->grouping == ChartGrouping::Stacked || v.plot->grouping == ChartGrouping::PercentStacked;
            for (const QVector<Seg> &row : v.segs)
                for (const Seg &seg : row) {
                    if (!std::isfinite(seg.to)) continue;
                    *lo = std::min(*lo, seg.to);
                    *hi = std::max(*hi, seg.to);
                    if (stacked) {
                        *lo = std::min(*lo, seg.from);
                        *hi = std::max(*hi, seg.from);
                    }
                }
        }
    };
    double lo = 0, hi = 0;
    if (scatter) {
        double xlo = std::numeric_limits<double>::infinity(), xhi = -xlo, ylo = xlo, yhi = -xlo;
        for (const View &v : m_views)
            for (const ChartSeries &s : v.plot->series)
                for (int i = 0; v.type == ChartType::Scatter && i < s.values.size() && i < s.xValues.size(); ++i) {
                    if (!std::isfinite(s.values[i]) || !std::isfinite(s.xValues[i])) continue;
                    xlo = std::min(xlo, s.xValues[i]);
                    xhi = std::max(xhi, s.xValues[i]);
                    ylo = std::min(ylo, s.values[i]);
                    yhi = std::max(yhi, s.values[i]);
                }
        m_xScale = scaleOf(*m_cat, xlo, xhi, false);
        m_scale = scaleOf(*m_val, ylo, yhi, false);
    } else {
        rangeOf(false, &lo, &hi);
        m_scale = scaleOf(*m_val, lo, hi, m_percent);
        if (m_val2) {
            rangeOf(true, &lo, &hi);
            m_scale2 = scaleOf(*m_val2, lo, hi, false);
        }
    }
    // Where the category axis crosses the value axis.
    const double crossBase = m_cat->hasCrossesAt ? m_cat->crossesAt : (m_cat->crosses == QLatin1String("min") ? m_scale.min : m_cat->crosses == QLatin1String("max") ? m_scale.max : 0.0);
    m_baseline = std::clamp(crossBase, m_scale.min, m_scale.max);

    // ---- what stands around the plot: the marks' labels, the axes' titles
    const ChartSeries *firstSeries = first.series.isEmpty() ? nullptr : &first.series.first();
    const QFont catFont = fontOf(m_cat->font, 10, false), valFont = fontOf(m_val->font, 10, false);
    const double tickLen = 4;
    auto outTick = [&](const ChartAxis &a) { return a.majorTick == QLatin1String("out") || a.majorTick == QLatin1String("cross"); };
    auto inTick = [&](const ChartAxis &a) { return a.majorTick == QLatin1String("in") || a.majorTick == QLatin1String("cross"); };
    const bool valLabels = !m_val->deleted && m_val->labelPosition != QLatin1String("none");
    const bool catLabels = !m_cat->deleted && m_cat->labelPosition != QLatin1String("none");
    auto ticksOf = [](const Scale &scale) {
        QVector<double> out;
        for (double t = scale.min; t <= scale.max + scale.major * 1e-6 && out.size() < 600; t += scale.major) out << t;
        return out;
    };
    const QVector<double> valueTicks = ticksOf(m_scale), xTicks = scatter ? ticksOf(m_xScale) : QVector<double>(), secondTicks = m_val2 ? ticksOf(m_scale2) : QVector<double>();
    QStringList valueLabels, xLabels, secondLabels;
    for (double t : valueTicks) valueLabels << valueText(t, *m_val, firstSeries, m_percent);
    for (double t : xTicks) xLabels << valueText(t, *m_cat, firstSeries, false);
    const QFont secondFont = m_val2 ? fontOf(m_val2->font, 10, false) : valFont;
    if (m_val2) {
        const ChartSeries *second = nullptr;
        for (const View &v : m_views)
            if (v.secondary && !v.plot->series.isEmpty()) second = &v.plot->series.first();
        for (double t : secondTicks) secondLabels << valueText(t, *m_val2, second, false);
    }
    auto widest = [&](const QStringList &labels, const QFont &font) {
        double w = 0;
        for (const QString &l : labels) w = std::max(w, widthOf(font, l));
        return w;
    };
    auto titleOf = [&](const ChartAxis *a, QFont *font, QColor *color) {
        if (!a || !a->title.present || a->title.text.isEmpty()) return 0.0;
        titleBlock(a->title, 10, font, color);
        return heightOf(*font, int(a->title.text.count(QLatin1Char('\n')) + 1)) + 4;
    };
    QFont catTitleFont, valTitleFont, secondTitleFont;
    QColor catTitleColor, valTitleColor, secondTitleColor;
    const double catTitleH = titleOf(m_cat, &catTitleFont, &catTitleColor), valTitleH = titleOf(m_val, &valTitleFont, &valTitleColor);
    const double secondTitleH = titleOf(m_val2, &secondTitleFont, &secondTitleColor);
    const double valLineH = heightOf(valFont), catLineH = heightOf(catFont);
    const QStringList cats = categoryLabels();

    // ---- the plot's place: what the file fixes, else what is left of `outer` after the labels and titles
    const bool manual = c.plotManual;
    const QRectF given = manual ? QRectF(c.plotX * m_size.width(), c.plotY * m_size.height(), c.plotW * m_size.width(), c.plotH * m_size.height()) : outer;
    const bool fixed = manual && c.plotInner;
    LabelFit fit;
    double leftSpace = 0, rightSpace = 0, topSpace = 0, bottomSpace = 0;
    auto categoriesAcross = [&]() { return std::max(1.0, m_between ? double(m_n) : double(std::max(1, m_n - 1))); };
    for (int pass = 0; pass < 2; ++pass) {
        // The first pass guesses at the plot's width for the labels' fit; the second has the width the first left.
        leftSpace = rightSpace = topSpace = bottomSpace = 0;
        if (scatter) {
            leftSpace = (valLabels ? widest(valueLabels, valFont) + 5 : 0) + (outTick(*m_val) ? tickLen : 0) + valTitleH;
            bottomSpace = (catLabels ? catLineH + 4 : 0) + (outTick(*m_cat) ? tickLen : 0) + catTitleH;
            rightSpace = xLabels.isEmpty() ? 0 : widthOf(catFont, xLabels.last()) / 2;
            topSpace = valLabels ? std::max(0.0, valLineH / 2 - 1.5) : 0;
        } else if (m_swap) {
            // Categories up the left side; the values along the bottom.
            const double slot = (given.height() - bottomSpace) / std::max(1, m_n);
            fit = fitLabels(cats, catFont, slot, *m_cat, false);
            leftSpace = (catLabels ? fit.extent + 5 : 0) + (outTick(*m_cat) ? tickLen : 0) + catTitleH;
            bottomSpace = (valLabels ? valLineH + 4 : 0) + (outTick(*m_val) ? tickLen : 0) + valTitleH;
            rightSpace = valueLabels.isEmpty() ? 0 : widthOf(valFont, valueLabels.last()) / 2;
        } else {
            leftSpace = (valLabels ? widest(valueLabels, valFont) + 3.5 : 0) + (outTick(*m_val) ? tickLen : 0) + valTitleH;
            if (m_val2 && !m_val2->deleted) rightSpace = widest(secondLabels, secondFont) + 5 + tickLen + secondTitleH;
            const double width = (fixed ? given.width() : given.width() - leftSpace - rightSpace);
            const double slot = width / categoriesAcross();
            fit = fitLabels(cats, catFont, slot, *m_cat, true);
            bottomSpace = (catLabels ? fit.extent + 2 : 0) + (outTick(*m_cat) ? tickLen : 0) + catTitleH;
            if (!cats.isEmpty()) {
                // The last label is given room as if it were level and centered under its category; a first one that is turned reaches left of its category.
                const double half = m_between ? slot / 2 : 0;   // how far in from the edge the first category's label is centered
                rightSpace = std::max(rightSpace, widthOf(catFont, cats.last()) / 2 + 3);
                if (fit.rotation != 0) leftSpace = std::max(leftSpace, widthOf(catFont, cats.first()) * std::cos(std::fabs(fit.rotation) * kPi / 180) - half + 2);
                else leftSpace = std::max(leftSpace, widthOf(catFont, cats.first()) / 2 - half);
            }
            topSpace = valLabels ? std::max(0.0, valLineH / 2 - 1.5) : 0;
            if (valLabels && !catLabels) bottomSpace = std::max(bottomSpace, valLineH / 2);
        }
        m_inner = fixed ? given : QRectF(given.left() + leftSpace, given.top() + topSpace, given.width() - leftSpace - rightSpace, given.height() - topSpace - bottomSpace);
        if (m_inner.width() < 10 || m_inner.height() < 10) return;
        m_slot = (m_swap ? m_inner.height() : m_inner.width()) / categoriesAcross();
        if (fixed) break;
    }

    // ---- the plot area's own fill and border
    m_p->save();
    if (c.plotFill.isValid()) m_p->fillRect(m_inner, c.plotFill);
    const QColor classic = tinted(c.textColor.isValid() ? c.textColor : QColor(Qt::black), 0.75);
    if (c.plotBorder.set && !c.plotBorder.none) {
        m_p->setPen(penFor(c.plotBorder, classic, 0.75));
        m_p->setBrush(Qt::NoBrush);
        m_p->drawRect(m_inner);
    }

    // ---- gridlines
    auto linesAcross = [&](const ChartAxis &a, const QVector<double> &at) {
        const QPen pen = penFor(a.gridline, classic, 0.75);
        if (!a.majorGridlines || pen.style() == Qt::NoPen) return;
        m_p->setPen(pen);
        for (double pos : at) {
            if (m_swap == (&a == m_val)) m_p->drawLine(QPointF(pos, m_inner.top()), QPointF(pos, m_inner.bottom()));
            else m_p->drawLine(QPointF(m_inner.left(), pos), QPointF(m_inner.right(), pos));
        }
    };
    auto onPage = [&](const QVector<double> &ticks, const Scale &scale, bool horizontal) {
        // A scatter's axes both run by value; a chart with categories has them at their units.
        QVector<double> out;
        for (double t : ticks) {
            if (scatter && horizontal) {
                const double f = (t - scale.min) / (scale.max - scale.min);
                out << m_inner.left() + (m_catReversed ? 1 - f : f) * m_inner.width();
            } else {
                out << valueAlong(t, scale);
            }
        }
        return out;
    };
    if (scatter) {
        linesAcross(*m_val, onPage(valueTicks, m_scale, false));
        linesAcross(*m_cat, onPage(xTicks, m_xScale, true));
    } else {
        linesAcross(*m_val, onPage(valueTicks, m_scale, false));
        QVector<double> boundaries;
        for (int i = 0; i <= (m_between ? m_n : std::max(0, m_n - 1)); ++i) boundaries << catAlong(i);
        linesAcross(*m_cat, boundaries);
    }
    m_p->restore();

    // ---- the plots
    for (const View &v : m_views) {
        const Scale &scale = v.secondary ? m_scale2 : m_scale;
        switch (v.type) {
        case ChartType::Bar: drawBars(v, scale); break;
        case ChartType::Line: drawLines(v, scale); break;
        case ChartType::Area: drawAreas(v, scale); break;
        case ChartType::Scatter: drawScatter(v, m_xScale, m_scale); break;
        default: break;
        }
    }

    // ---- the axes: line, marks, labels
    m_p->save();
    // One axis line from `a` to `b`, with its marks at `along` (positions on the line) that stand out to `side` (a unit vector across it).
    auto axisLine = [&](const ChartAxis &a, QPointF from, QPointF to, const QVector<double> &along, bool horizontal, QPointF side) {
        if (a.deleted) return;
        const QPen pen = penFor(a.line, classic, 0.75);
        if (pen.style() == Qt::NoPen) return;
        m_p->setPen(pen);
        m_p->drawLine(from, to);
        const double out = outTick(a) ? tickLen : 0, in = inTick(a) ? tickLen : 0;
        if (out == 0 && in == 0) return;
        for (double pos : along) {
            const QPointF at = horizontal ? QPointF(pos, from.y()) : QPointF(from.x(), pos);
            m_p->drawLine(at - side * in, at + side * out);
        }
    };
    if (scatter) {
        axisLine(*m_cat, QPointF(m_inner.left(), m_inner.bottom()), QPointF(m_inner.right(), m_inner.bottom()), onPage(xTicks, m_xScale, true), true, QPointF(0, 1));
        axisLine(*m_val, QPointF(m_inner.left(), m_inner.top()), QPointF(m_inner.left(), m_inner.bottom()), onPage(valueTicks, m_scale, false), false, QPointF(-1, 0));
        if (catLabels) {
            const QVector<double> xs = onPage(xTicks, m_xScale, true);
            for (int i = 0; i < xs.size(); ++i)
                drawText(xLabels[i], catFont, colorOf(m_cat->font), QPointF(xs[i], m_inner.bottom() + 4 + (outTick(*m_cat) ? tickLen : 0)), 0.5, 0);
        }
        if (valLabels)
            for (int i = 0; i < valueTicks.size(); ++i)
                drawText(valueLabels[i], valFont, colorOf(m_val->font), QPointF(m_inner.left() - 4 - (outTick(*m_val) ? tickLen : 0), valueAlong(valueTicks[i], m_scale)), 1.0, 0.5);
    } else if (!m_swap) {
        // The category axis runs along the baseline.
        const double y = valueAlong(m_baseline, m_scale);
        QVector<double> marks;
        for (int i = 0; i <= (m_between ? m_n : std::max(0, m_n - 1)); ++i) marks << catAlong(i);
        axisLine(*m_cat, QPointF(m_inner.left(), y), QPointF(m_inner.right(), y), marks, true, QPointF(0, 1));
        if (catLabels) {
            const bool high = m_cat->labelPosition == QLatin1String("high"), low = m_cat->labelPosition == QLatin1String("low");
            const double edge = high ? m_inner.top() : low ? m_inner.bottom() : y;
            const double labelY = high ? edge - 2 - (outTick(*m_cat) ? tickLen : 0) : edge + 2 + (outTick(*m_cat) ? tickLen : 0);
            const QColor color = colorOf(m_cat->font);
            for (int i = 0; i < m_n && i < cats.size(); i += fit.skip) {
                const double x = catAlong(catUnit(i));
                if (fit.rotation != 0) {
                    drawText(cats[i], catFont, color, QPointF(x + 2, labelY + catLineH * 0.35), 1.0, 0.5, fit.rotation);
                } else {
                    const QStringList lines = fit.lines > 1 ? wrapped(cats[i], catFont, m_slot * fit.skip - 4) : QStringList{cats[i]};
                    drawText(lines.join(QLatin1Char('\n')), catFont, color, QPointF(x, labelY), 0.5, high ? 1 : 0);
                }
            }
        }
        // The value axis at the left, and a second one at the right.
        for (int which = 0; which < (m_val2 ? 2 : 1); ++which) {
            const ChartAxis &a = which == 0 ? *m_val : *m_val2;
            const Scale &scale = which == 0 ? m_scale : m_scale2;
            const bool right = which == 1 || a.crosses == QLatin1String("max") || a.position == QLatin1String("r");
            const double x = right ? m_inner.right() : m_inner.left();
            const QVector<double> &ticks = which == 0 ? valueTicks : secondTicks;
            QVector<double> along;
            for (double t : ticks) along << valueAlong(t, scale);
            axisLine(a, QPointF(x, m_inner.top()), QPointF(x, m_inner.bottom()), along, false, QPointF(right ? 1 : -1, 0));
            if (a.deleted || a.labelPosition == QLatin1String("none")) continue;
            const bool onLeft = a.labelPosition == QLatin1String("low") ? true : a.labelPosition == QLatin1String("high") ? false : !right;
            const double gap = 3.5 + (outTick(a) ? tickLen : 0);
            const QStringList &labels = which == 0 ? valueLabels : secondLabels;
            for (int i = 0; i < ticks.size(); ++i)
                drawText(labels[i], which == 0 ? valFont : secondFont, colorOf(a.font), QPointF((onLeft ? m_inner.left() - gap : m_inner.right() + gap), along[i]), onLeft ? 1.0 : 0.0, 0.5);
        }
    } else {
        // Bars across: the categories run up the page beside the baseline, the values along the bottom.
        const double x = valueAlong(m_baseline, m_scale);
        QVector<double> marks;
        for (int i = 0; i <= m_n; ++i) marks << catAlong(i);
        axisLine(*m_cat, QPointF(x, m_inner.top()), QPointF(x, m_inner.bottom()), marks, false, QPointF(-1, 0));
        if (catLabels) {
            const bool right = m_cat->labelPosition == QLatin1String("high");
            const double edge = m_cat->labelPosition == QLatin1String("nextTo") ? x : right ? m_inner.right() : m_inner.left();
            const double labelX = right ? edge + 4 + (outTick(*m_cat) ? tickLen : 0) : edge - 4 - (outTick(*m_cat) ? tickLen : 0);
            for (int i = 0; i < m_n && i < cats.size(); i += fit.skip)
                drawText(cats[i], catFont, colorOf(m_cat->font), QPointF(labelX, catAlong(catUnit(i))), right ? 0.0 : 1.0, 0.5);
        }
        const double y = m_val->crosses == QLatin1String("max") ? m_inner.top() : m_inner.bottom();
        QVector<double> along;
        for (double t : valueTicks) along << valueAlong(t, m_scale);
        axisLine(*m_val, QPointF(m_inner.left(), y), QPointF(m_inner.right(), y), along, true, QPointF(0, 1));
        if (valLabels)
            for (int i = 0; i < valueTicks.size(); ++i)
                drawText(valueLabels[i], valFont, colorOf(m_val->font), QPointF(along[i], y + 4 + (outTick(*m_val) ? tickLen : 0)), 0.5, 0);
    }
    m_p->restore();

    // ---- axis titles
    auto turn = [](const ChartTitle &t, double usual) { return t.hasRotation ? t.rotation : usual; };
    if (catTitleH > 0) {
        if (m_swap) drawText(m_cat->title.text, catTitleFont, catTitleColor, QPointF(given.left() + catTitleH / 2 - 2, m_inner.center().y()), 0.5, 0.5, turn(m_cat->title, -90));
        else drawText(m_cat->title.text, catTitleFont, catTitleColor, QPointF(m_inner.center().x(), given.bottom() - 1), 0.5, 1.0, turn(m_cat->title, 0));
    }
    if (valTitleH > 0) {
        if (m_swap) drawText(m_val->title.text, valTitleFont, valTitleColor, QPointF(m_inner.center().x(), given.bottom() - 1), 0.5, 1.0, turn(m_val->title, 0));
        else drawText(m_val->title.text, valTitleFont, valTitleColor, QPointF(given.left() + valTitleH / 2 - 2, m_inner.center().y()), 0.5, 0.5, turn(m_val->title, -90));
    }
    if (secondTitleH > 0 && !m_swap)
        drawText(m_val2->title.text, secondTitleFont, secondTitleColor, QPointF(given.right() - secondTitleH / 2 + 2, m_inner.center().y()), 0.5, 0.5, turn(m_val2->title, 90));
}

// ---- the whole chart

void ChartPainter::paint()
{
    const Chart &c = m_chart;
    m_p->setRenderHint(QPainter::Antialiasing);
    m_p->setRenderHint(QPainter::TextAntialiasing);
    const QRectF full(QPointF(0, 0), m_size);
    if (c.background.isValid()) m_p->fillRect(full, c.background);
    if (c.border.set && !c.border.none) {
        m_p->setPen(penFor(c.border, tinted(c.textColor, 0.75), 0.75));
        m_p->setBrush(Qt::NoBrush);
        m_p->drawRect(full.adjusted(0.4, 0.4, -0.4, -0.4));
    }
    const double margin = std::clamp(std::min(m_size.width(), m_size.height()) * 0.018, 3.0, 7.0);
    QRectF area = full.adjusted(margin, margin, -margin, -margin);

    // The title across the top.
    if (c.title.present && !c.title.text.isEmpty()) {
        QFont font;
        QColor color;
        titleBlock(c.title, 18, &font, &color);
        const double h = heightOf(font, int(c.title.text.count(QLatin1Char('\n')) + 1));
        drawText(c.title.text, font, color, QPointF(area.center().x(), area.top()), 0.5, 0, c.title.hasRotation ? c.title.rotation : 0);
        if (!c.title.overlay) area.setTop(area.top() + h + margin * 0.6);
    }

    // The legend at its side.
    if (c.legend.shown) {
        const QVector<Entry> entries = legendEntries();
        if (!entries.isEmpty()) {
            QVector<int> breaks;
            const QSizeF size = legendSize(entries, area.width(), &breaks);
            const QString pos = c.legend.position;
            QRectF box;
            if (pos == QLatin1String("l")) box = QRectF(area.left(), area.center().y() - size.height() / 2, size.width(), size.height());
            else if (pos == QLatin1String("t")) box = QRectF(area.center().x() - size.width() / 2, area.top(), size.width(), size.height());
            else if (pos == QLatin1String("b")) box = QRectF(area.center().x() - size.width() / 2, area.bottom() - size.height(), size.width(), size.height());
            else if (pos == QLatin1String("tr")) box = QRectF(area.right() - size.width(), area.top(), size.width(), size.height());
            else box = QRectF(area.right() - size.width(), area.center().y() - size.height() / 2, size.width(), size.height());
            drawLegend(box, entries, breaks);
            if (!c.legend.overlay) {
                const double gap = margin * 0.6;
                if (pos == QLatin1String("l")) area.setLeft(box.right() + gap);
                else if (pos == QLatin1String("t")) area.setTop(box.bottom() + gap);
                else if (pos == QLatin1String("b")) area.setBottom(box.top() - gap);
                else area.setRight(box.left() - gap);
            }
        }
    }
    if (area.width() < 10 || area.height() < 10) return;
    if (c.plots.isEmpty()) return;
    const ChartType type = c.plots.first().type;
    if (type == ChartType::Pie || type == ChartType::Doughnut) paintPie(c.plotManual ? QRectF(c.plotX * m_size.width(), c.plotY * m_size.height(), c.plotW * m_size.width(), c.plotH * m_size.height()) : area);
    else paintAxes(area);
}

} // namespace

QByteArray chartSvg(const Chart &chart, const QSizeF &size)
{
    const QSizeF s(std::max(1.0, size.width()), std::max(1.0, size.height()));
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    // SVG 1.1, with the resolution a point to the unit: font sizes in the file are then the chart's own.
    QSvgGenerator gen(QSvgGenerator::SvgVersion::Svg11);
    gen.setOutputDevice(&buf);
    gen.setResolution(72);
    gen.setSize(QSize(int(std::ceil(s.width())), int(std::ceil(s.height()))));
    gen.setViewBox(QRectF(QPointF(0, 0), s));
    gen.setTitle(say("Chart"));
    gen.setDescription(QString());
    {
        QPainter p(&gen);
        ChartPainter painter(chart, &p, &gen, s);
        painter.paint();
    }
    return buf.data();
}

} // namespace jp::office
