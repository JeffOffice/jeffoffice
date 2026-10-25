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

// What a picture of a chart holds. QSvgRenderer reads a picture in time that grows with the square of its tags (a shape is three of them): a
// chart of 20,000 columns took minutes, and two thousand shapes take a second. Filling is slow too, for a line that crosses the plot back and
// forth, and for one path of many shapes (a path of columns took a thousand times what the columns drawn one by one took). So a chart with
// more than these allow is drawn coarser: from every nth point, the same n for all its series, with the labels that crowd it left out; and
// the picture is drawn again, coarser, if it still has more than kMaxTags. The real charts here (five, in the reference decks) have one series
// of 63 points; a chart with a dozen series of a few hundred points, or one long series of readings (a couple of thousand), is a big one, and
// it has some dozens of labels. These are several times that, or ten, and where a picture has more points than they allow its columns are
// narrower than a pixel and its lines are bands, so what is left out hardly shows.
constexpr int kMaxDrawnPoints = 2000;      // of a series
constexpr int kMaxDrawnTotal = 10000;      // of all the series of a chart
constexpr int kMaxDrawnPieces = 1500;      // columns and the shapes of areas, each of its own
constexpr int kMaxDrawnMarkers = 6000;     // markers on points, drawn one by one up to kMaxSeparateMarkers of them, past that as one path for a series
constexpr int kMaxDrawnSlices = 500;       // of a pie's series; a doughnut's rings together may have twice that
constexpr int kMaxSeparateLines = 300;     // gridlines and the marks of axes drawn one by one; past that, one path of them
constexpr int kMaxSeparateMarkers = 1500;
constexpr int kMaxDataLabels = 500;        // labels on the points
constexpr int kMaxLegendEntries = 150;
constexpr int kMaxAxisLabels = 250;        // along a category axis
constexpr int kMaxTicks = 100;             // along a value axis
constexpr int kMaxTags = 6000;             // of a finished picture
constexpr qsizetype kMaxBytes = 8 << 20;   // and its size
constexpr int kMaxLevel = 5;               // times a picture over the limit is drawn again, each time with the limits halved
constexpr int kLongRun = 256;              // points of a line past which it is drawn in paths of kPathSegments segments (the same picture, drawn faster)
constexpr int kPathSegments = 32;
constexpr double kStrokeLength = 150;      // the longest all the lines of a plot are drawn with, in times its width and height together; the rest is left out
constexpr int kMaxFamily = 100;            // characters of a font family (a name is at most 32 in a font file)
constexpr int kMaxText = 1000;             // of any text drawn (a label is kChartMaxLabel)
constexpr double kMaxMagnitude = 1e150;    // the largest number a scale is made of: its span, and the sums of a stack, stay numbers


// A text as a picture can write it: at most `most` characters, and none that XML has no place for (control characters, half a surrogate pair).
QString writable(const QString &text, int most)
{
    QString out;
    out.reserve(int(std::min<qsizetype>(text.size(), most)));
    for (qsizetype i = 0; i < text.size() && out.size() < most; ++i) {
        const QChar c = text[i];
        const ushort u = c.unicode();
        if (c.isHighSurrogate() && i + 1 < text.size() && text[i + 1].isLowSurrogate()) {
            out += c;
            out += text[++i];
        } else if (u == '\n' || u == '\t' || (u >= 0x20 && u < 0x7F) || (u >= 0xA0 && !c.isSurrogate() && u < 0xFFFE)) {
            out += c;
        }
    }
    return out;
}

// A font family as a picture can write it in an attribute (an SVG writer puts it in as it is): none of the characters that would end the
// attribute or the tag, and no longer than a name is.
QString safeFamily(const QString &name)
{
    QString out = writable(name, kMaxFamily);
    out.remove(QLatin1Char('"')).remove(QLatin1Char('\'')).remove(QLatin1Char('<')).remove(QLatin1Char('>')).remove(QLatin1Char('&'));
    return out.trimmed();
}

// How much of a chart is drawn, and in how many pieces. Each level past the first halves what the one before allowed.
struct Limits {
    int points = kMaxDrawnPoints, total = kMaxDrawnTotal, pieces = kMaxDrawnPieces, markers = kMaxDrawnMarkers, slices = kMaxDrawnSlices;
    int separateLines = kMaxSeparateLines, separateMarkers = kMaxSeparateMarkers, dataLabels = kMaxDataLabels, legend = kMaxLegendEntries, axisLabels = kMaxAxisLabels;
    int segmentsPerPath = kPathSegments;
};

Limits limitsAt(int level)
{
    Limits l;
    l.points >>= level;
    l.total >>= level;
    l.pieces >>= level;
    l.markers >>= level;
    l.slices >>= level;
    l.separateLines >>= level;
    l.separateMarkers >>= level;
    l.dataLabels >>= level;
    l.legend >>= level;
    l.axisLabels >>= level;
    return l;
}

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
    // A number past what a scale is made of is no number to it: a file's axis of 1e308 is the automatic one.
    auto usable = [](double v) { return std::isfinite(v) && std::fabs(v) <= kMaxMagnitude; };
    if (!(dataHigh >= dataLow) || !usable(dataLow) || !usable(dataHigh)) dataLow = 0, dataHigh = 1;
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
    const bool fixedMin = !axis.autoMin && usable(axis.min), fixedMax = !axis.autoMax && usable(axis.max);
    const double from = fixedMin ? axis.min : low;
    double to = fixedMax ? axis.max : high;
    if (!(to > from)) to = from + 1;
    double unit = axis.majorUnit > 0 && usable(axis.majorUnit) ? axis.majorUnit : niceUnit((to - from) / 10);
    while ((to - from) / unit > kMaxTicks) unit *= 2;
    Scale s;
    s.major = unit;
    s.min = fixedMin ? axis.min : std::floor(low / unit + 1e-9) * unit;
    s.max = fixedMax ? axis.max : std::ceil(high / unit - 1e-9) * unit;
    if (!(s.max > s.min)) s.max = s.min + unit;
    // What is left of a scale that has no span (numbers too large for their steps) is the unit one.
    if (!usable(s.min) || !usable(s.max) || !(s.max > s.min) || !std::isfinite(s.max - s.min) || !(unit > 0) || !std::isfinite(unit)) return Scale();
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
    ChartPainter(const Chart &chart, QPainter *p, QPaintDevice *device, const QSizeF &size, const Limits &limits)
        : m_chart(chart), m_p(p), m_dev(device), m_size(size), m_limits(limits), m_separateLeft(limits.separateLines), m_markersLeft(limits.separateMarkers), m_labelsLeft(limits.dataLabels) {}
    void paint();
    // Whether anything was left out for want of room in the picture (labels, entries of the legend, labels along an axis).
    bool leftOut() const { return m_leftOut; }

private:
    // ---- text
    QFont fontOf(const ChartFont &own, double size, bool bold) const;
    QColor colorOf(const ChartFont &own) const;
    double widthOf(const QFont &f, const QString &text) const;
    double heightOf(const QFont &f, int lines = 1) const;
    // Draws the text so that the anchor falls at `ha` across it (0: its left, 1: its right) and `va` down it, the text turned `angle` degrees about the anchor.
    void drawText(const QString &text, const QFont &font, const QColor &color, const QPointF &anchor, double ha, double va, double angle = 0);
    QPen penFor(const ChartLine &line, const QColor &fallback, double fallbackWidth) const;
    QColor lineColor() const;

    // ---- the pieces of a chart
    QVector<Entry> legendEntries() const;
    QFont legendFont() const;
    QSizeF legendSize(const QVector<Entry> &entries, double available, QVector<int> *rowBreaks) const;
    void drawLegend(const QRectF &box, const QVector<Entry> &entries, const QVector<int> &rowBreaks);
    void drawKey(const Entry &e, const QRectF &key);
    void drawMarker(const QPointF &c, ChartMarker::Symbol symbol, double size, const QColor &fill, const QColor &line, bool noFill, bool noLine);
    // A marker's shape added to a path (so that the markers of a series can be drawn as one); true if it is made of strokes, not of a fill.
    static bool addMarker(QPainterPath *path, const QPointF &c, ChartMarker::Symbol symbol, double size);
    // The markers of a series, one by one while the chart has few, else as one path: the same look, the markers over one another in the order of their points
    // as far as their fill and outline go.
    void drawMarkers(const QVector<QPointF> &at, const QVector<bool> &marked, ChartMarker::Symbol symbol, double size, const QColor &fill, const QColor &line, bool noFill, bool noLine);
    void strokeThrough(const QVector<QPointF> &pts, const QVector<bool> &present, bool smooth, const QString &blanks);
    // Whether `lines` marks of one kind (gridlines, the marks of an axis) are drawn one by one: so while the chart has few. If not, they are one path.
    bool drawSeparately(int lines);
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
    Limits m_limits;
    int m_separateLeft, m_markersLeft, m_labelsLeft;   // lines and markers that may still be drawn one by one, and labels on points that may still be drawn
    double m_strokeShare = 0;             // the length a series' line may have, in points
    mutable bool m_leftOut = false;

    // What a plot the file fixes may take of the chart: the chart less the room a legend at a side takes (the whole chart when it has none or lays over the plot).
    QRectF m_room;
    // The plot the file fixes, as a rectangle of the chart, kept out of the legend's room.
    QRectF manualPlot() const;
    // A title of an axis: at `where` (its middle across, `vertical` of its height down from it), turned `usual` degrees unless the file turns it, or at the place its own layout gives it.
    void drawAxisTitle(const ChartTitle &t, const QFont &font, const QColor &color, const QPointF &where, double vertical, double usual);

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
    const QString asked = !own.family.isEmpty() ? own.family : !m_chart.font.family.isEmpty() ? m_chart.font.family : m_chart.fontFamily;
    QString family = safeFamily(asked);
    if (family.isEmpty()) family = safeFamily(m_chart.fontFamily);
    QFont f(family);
    f.setPointSizeF(std::clamp(own.size > 0 ? own.size : m_chart.font.size > 0 ? m_chart.font.size : size, 1.0, 400.0));
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
    const QStringList lines = writable(text, kMaxText).split(QLatin1Char('\n'));
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

// The gray of the lines (gridlines, axes, borders) a chart's style draws: the text's color a quarter of the way to the background.
QColor ChartPainter::lineColor() const
{
    const QColor text = m_chart.textColor.isValid() ? m_chart.textColor : QColor(Qt::black);
    if (text.lightnessF() <= 0.5) return tinted(text, 0.75);
    const QColor back = m_chart.backgroundColor.isValid() ? m_chart.backgroundColor : QColor(Qt::black);
    auto mix = [](int a, int b) { return int(std::lround(a * 0.55 + b * 0.45)); };
    return QColor(mix(text.red(), back.red()), mix(text.green(), back.green()), mix(text.blue(), back.blue()));
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

bool ChartPainter::addMarker(QPainterPath *path, const QPointF &c, ChartMarker::Symbol symbol, double size)
{
    const double r = size / 2;
    switch (symbol) {
    case ChartMarker::Circle:
        path->addEllipse(c, r, r);
        return false;
    case ChartMarker::Dot:
        path->addEllipse(c, r * 0.6, r * 0.6);
        return false;
    case ChartMarker::Square:
        path->addRect(QRectF(c.x() - r, c.y() - r, size, size));
        return false;
    case ChartMarker::Diamond:
        path->addPolygon(QPolygonF({QPointF(c.x(), c.y() - r * 1.2), QPointF(c.x() + r * 1.2, c.y()), QPointF(c.x(), c.y() + r * 1.2), QPointF(c.x() - r * 1.2, c.y())}));
        path->closeSubpath();
        return false;
    case ChartMarker::Triangle:
        path->addPolygon(QPolygonF({QPointF(c.x(), c.y() - r * 1.1), QPointF(c.x() + r * 1.1, c.y() + r * 0.9), QPointF(c.x() - r * 1.1, c.y() + r * 0.9)}));
        path->closeSubpath();
        return false;
    case ChartMarker::Dash:
        path->moveTo(c.x() - r, c.y());
        path->lineTo(c.x() + r, c.y());
        return true;
    case ChartMarker::X:
    case ChartMarker::Plus:
    case ChartMarker::Star:
        if (symbol != ChartMarker::Plus) {
            path->moveTo(c.x() - r, c.y() - r);
            path->lineTo(c.x() + r, c.y() + r);
            path->moveTo(c.x() - r, c.y() + r);
            path->lineTo(c.x() + r, c.y() - r);
        }
        if (symbol != ChartMarker::X) {
            path->moveTo(c.x() - r, c.y());
            path->lineTo(c.x() + r, c.y());
            path->moveTo(c.x(), c.y() - r);
            path->lineTo(c.x(), c.y() + r);
        }
        return true;
    default:
        return false;
    }
}

void ChartPainter::drawMarkers(const QVector<QPointF> &at, const QVector<bool> &marked, ChartMarker::Symbol symbol, double size, const QColor &fill, const QColor &line, bool noFill, bool noLine)
{
    if (symbol == ChartMarker::None) return;
    int pieces = 0;
    for (const bool m : marked) pieces += m;
    if (pieces <= m_markersLeft) {
        m_markersLeft -= pieces;
        for (int i = 0; i < at.size(); ++i)
            if (marked[i]) drawMarker(at[i], symbol, size, fill, line, noFill, noLine);
        return;
    }
    // So many markers are one path: drawn one by one they are three tags each, and a thousand of them are what QSvgRenderer takes a second to read.
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    bool strokes = false;
    for (int i = 0; i < at.size(); ++i)
        if (marked[i]) strokes = addMarker(&path, at[i], symbol, size);
    m_p->save();
    if (strokes) {
        // Marks drawn with strokes alone take the fill's color when the line has none.
        m_p->setPen(QPen(noLine ? fill : line, 1.0, Qt::SolidLine, Qt::FlatCap));
        m_p->setBrush(Qt::NoBrush);
    } else {
        m_p->setPen(noLine ? QPen(Qt::NoPen) : QPen(line, 0.75, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        m_p->setBrush(noFill ? QBrush(Qt::NoBrush) : QBrush(fill));
    }
    m_p->drawPath(path);
    m_p->restore();
}

bool ChartPainter::drawSeparately(int lines)
{
    if (lines > m_separateLeft) return false;
    m_separateLeft -= lines;
    return true;
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
            m_leftOut = m_leftOut || n > m_limits.legend;
            for (int i = 0; i < n && out.size() < m_limits.legend; ++i) {
                Entry e;
                e.name = i < s.categories.size() && !s.categories[i].isEmpty() ? writable(s.categories[i], kChartMaxLabel) : QString::number(i + 1);
                e.color = pointColor(plot, s, i);
                out << e;
            }
            continue;
        }
        for (const ChartSeries &s : plot.series) {
            if (out.size() >= m_limits.legend) {
                m_leftOut = true;
                break;
            }
            Entry e;
            e.name = writable(s.name, kChartMaxLabel);
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

// The font the legend's names are measured in and drawn in (one place for both, so that a legend is as wide as what is drawn in it).
QFont ChartPainter::legendFont() const { return fontOf(m_chart.legend.font, 10, false); }

// The size of the legend, and where its rows break when it runs along the top or bottom.
QSizeF ChartPainter::legendSize(const QVector<Entry> &entries, double available, QVector<int> *rowBreaks) const
{
    const QFont font = legendFont();
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
    const QFont font = legendFont();
    const QColor color = colorOf(m_chart.legend.font);
    const double keyW = 7, gap = 4, between = 12, pad = 5, row = heightOf(font) + 1;
    const bool across = m_chart.legend.position == QLatin1String("t") || m_chart.legend.position == QLatin1String("b");
    m_p->save();
    // (the legend's own fill and line, when the file gives them: what lies under it does not show through)
    if (m_chart.legend.fill.isValid()) m_p->fillRect(box, m_chart.legend.fill);
    if (m_chart.legend.border.set && !m_chart.legend.border.none) {
        m_p->setPen(penFor(m_chart.legend.border, lineColor(), 0.75));
        m_p->setBrush(Qt::NoBrush);
        m_p->drawRect(box);
    }
    m_p->setPen(Qt::NoPen);
    if (!across) {
        double y = box.top() + pad - 1;
        for (const Entry &e : entries) {
            // A legend longer than the chart is cut off by its picture; the entries beyond it are not drawn.
            if (y + row >= 0 && y <= m_size.height()) {
                const double kw = e.line ? 20 : keyW;
                drawKey(e, QRectF(box.left() + pad, y + row / 2 - keyW / 2, kw, keyW));
                drawText(e.name, font, color, QPointF(box.left() + pad + kw + gap, y + row / 2), 0, 0.5);
            }
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
                const double next = x + kw + gap + widthOf(font, entries[i].name) + between;
                if (y + row >= 0 && y <= m_size.height() && next >= 0 && x <= m_size.width()) {
                    drawKey(entries[i], QRectF(x, y + row / 2 - keyW / 2, kw, keyW));
                    drawText(entries[i].name, font, color, QPointF(x + kw + gap, y + row / 2), 0, 0.5);
                }
                x = next;
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
    if (l.series) parts << writable(s.name, kChartMaxLabel);
    if (l.category) {
        if (plot.type == ChartType::Scatter && i < s.xValues.size()) parts << formatChartNumber(s.xValues[i], QString());
        else if (i < s.categories.size() && !s.categories[i].isEmpty()) parts << writable(s.categories[i], kChartMaxLabel);
        else parts << QString::number(i + 1);
    }
    const double v = i < s.values.size() ? s.values[i] : kNaN;
    if (l.value && !std::isnan(v)) parts << formatChartNumber(v, l.format.isEmpty() ? s.format : l.format);
    if (l.percent && total > 0 && !std::isnan(v)) parts << formatChartNumber(std::fabs(v) / total, l.format.contains(QLatin1Char('%')) ? l.format : QStringLiteral("0%"));
    return parts.join(l.separator.isEmpty() ? QStringLiteral(", ") : writable(l.separator, kChartMaxLabel));
}

// Writes the label of one point, near `at`: `ha` and `va` say where the label sits against it.
void ChartPainter::drawLabels(const ChartPlot &plot, const ChartSeries &s, int index, QPointF at, double total, double ha, double va, bool inside, const QColor &under)
{
    // Labels on points that crowd one another past reading are not drawn, nor are those past the chart's budget of them.
    if (m_labelsLeft <= 0) {
        m_leftOut = true;
        return;
    }
    const QString text = labelText(plot, s, index, total, s.labels);
    if (text.isEmpty()) return;
    --m_labelsLeft;
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
        // (a value below nothing is a slice of its size, as the common programs draw it)
        double total = 0;
        for (double v : s.values)
            if (std::isfinite(v)) total += std::fabs(v);
        if (total <= 0) continue;
        const double outer = ring ? radius - r * (radius * (1 - plot.holeSize / 100.0) / rings) : radius;
        const double inner = ring ? outer - radius * (1 - plot.holeSize / 100.0) / rings : 0;
        double angle = plot.firstSliceAngle;   // degrees clockwise from twelve o'clock
        for (int i = 0; i < s.values.size(); ++i) {
            const double v = std::fabs(s.values[i]);
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
    if (!std::isfinite(f)) f = 0;
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

// The runs of points a line goes through, in order: a blank ends a run (a gap), or is bridged ("span"), as the chart says.
static QVector<QVector<QPointF>> runsOf(const QVector<QPointF> &pts, const QVector<bool> &present, const QString &blanks)
{
    QVector<QVector<QPointF>> runs;
    QVector<QPointF> run;
    for (int i = 0; i < pts.size(); ++i) {
        if (present[i]) {
            run << pts[i];
        } else if (blanks != QLatin1String("span") && !run.isEmpty()) {
            runs << run;
            run.clear();
        }
    }
    if (!run.isEmpty()) runs << run;
    return runs;
}

// A run of points added to a path: a lone point is a dot, the others a line through them, smoothed when the chart asks.
static void addRun(QPainterPath *path, const QVector<QPointF> &run, bool smooth)
{
    if (run.size() == 1) {
        path->moveTo(run[0]);
        path->lineTo(run[0]);
    } else if (run.size() > 1) {
        path->moveTo(run[0]);
        if (!smooth) {
            for (int i = 1; i < run.size(); ++i) path->lineTo(run[i]);
        } else {
            // Catmull-Rom through the points, as cubic curves.
            for (int i = 0; i + 1 < run.size(); ++i) {
                const QPointF p0 = run[std::max(0, i - 1)], p1 = run[i], p2 = run[i + 1], p3 = run[std::min<int>(int(run.size()) - 1, i + 2)];
                path->cubicTo(p1 + (p2 - p0) / 6.0, p2 - (p3 - p1) / 6.0, p2);
            }
        }
    }
}

// A series' points in order as a line through them with the pen set: broken at blanks (or not, as the chart says), smoothed when it asks.
// A line that is too long to fill in reasonable time (one that crosses the plot back and forth thousands of times takes seconds) is drawn from
// fewer of its points; and one of many points is drawn in paths of some segments each, which comes out the same but fills in a fraction of
// the time (a polyline is filled as one polygon of as many crossings as the line has).
void ChartPainter::strokeThrough(const QVector<QPointF> &pts, const QVector<bool> &present, bool smooth, const QString &blanks)
{
    QVector<QVector<QPointF>> runs = runsOf(pts, present, blanks);
    double length = 0;
    for (const QVector<QPointF> &run : std::as_const(runs))
        for (int i = 1; i < run.size(); ++i) length += std::hypot(run[i].x() - run[i - 1].x(), run[i].y() - run[i - 1].y());
    const double most = m_strokeShare;
    if (length > most && most > 0) {
        const int stride = int(std::min(std::ceil(length / most), 1e6));
        for (QVector<QPointF> &run : runs) {
            QVector<QPointF> kept;
            for (int i = 0; i < run.size(); ++i)
                if (i % stride == 0 || i + 1 == run.size()) kept << run[i];
            run = kept;
        }
    }
    bool longRun = false;
    for (const QVector<QPointF> &run : std::as_const(runs)) longRun = longRun || run.size() > kLongRun;
    QPainterPath whole;
    for (const QVector<QPointF> &run : std::as_const(runs)) {
        if (!longRun || smooth || run.size() <= kLongRun) {
            addRun(&whole, run, smooth);
            continue;
        }
        const int segments = std::max(1, m_limits.segmentsPerPath);
        for (int from = 0; from + 1 < run.size(); from += segments) {
            QPainterPath piece;
            piece.moveTo(run[from]);
            for (int i = from + 1; i <= std::min<int>(from + segments, int(run.size()) - 1); ++i) piece.lineTo(run[i]);
            m_p->drawPath(piece);
        }
    }
    if (!longRun || !whole.isEmpty()) m_p->drawPath(whole);
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
            strokeThrough(pts, present, s.smooth, m_chart.blanksAs);
        }
        ChartMarker::Symbol symbol = s.marker.symbol;
        if (symbol == ChartMarker::Auto) symbol = plot.markers ? autoSymbols[k % 8] : ChartMarker::None;
        QVector<bool> marked(m_n, false);
        for (int i = 0; i < m_n; ++i) marked[i] = present[i] && std::isfinite(v.segs[k][i].to);
        const QColor markFill = s.marker.fill.isValid() ? s.marker.fill : s.color;
        drawMarkers(pts, marked, symbol, s.marker.size, markFill, s.marker.line.isValid() ? s.marker.line : markFill, s.marker.noFill, s.marker.noLine);
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
            // A point far off its axes (a fixed axis that leaves the data out) is not drawn, nor is one that has no place.
            if (!(std::fabs(fx) <= 1000 && std::fabs(fy) <= 1000)) {
                present[i] = false;
                continue;
            }
            pts[i] = QPointF(m_inner.left() + (m_catReversed ? 1 - fx : fx) * m_inner.width(), m_inner.bottom() - (m_valReversed ? 1 - fy : fy) * m_inner.height());
        }
        if (plot.lines && !s.outline.none) {
            QPen pen = penFor(s.outline, s.color, 2.25);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            m_p->setPen(pen);
            m_p->setBrush(Qt::NoBrush);
            strokeThrough(pts, present, s.smooth, QStringLiteral("gap"));
        }
        ChartMarker::Symbol symbol = s.marker.symbol;
        if (symbol == ChartMarker::Auto) symbol = autoSymbols[k % 8];
        if (plot.element == QLatin1String("bubbleChart") && s.marker.symbol == ChartMarker::Auto) symbol = ChartMarker::Circle;
        const double markSize = plot.element == QLatin1String("bubbleChart") ? std::max(s.marker.size, 10.0) : s.marker.size;
        const QColor markFill = s.marker.fill.isValid() ? s.marker.fill : s.color;
        const QColor markLine = s.marker.line.isValid() ? s.marker.line : markFill;
        int pieces = 0;
        for (int i = 0; i < n; ++i) pieces += present[i];
        // The markers of a series that has many are one path, drawn before the labels; else each is drawn with its label.
        const bool together = symbol != ChartMarker::None && pieces > m_markersLeft;
        if (together) drawMarkers(pts, present, symbol, markSize, markFill, markLine, s.marker.noFill, s.marker.noLine);
        else if (symbol != ChartMarker::None) m_markersLeft -= pieces;
        for (int i = 0; i < n; ++i) {
            if (!present[i]) continue;
            if (!together) drawMarker(pts[i], symbol, markSize, markFill, markLine, s.marker.noFill, s.marker.noLine);
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
    for (QString &label : labels) label = writable(label, kChartMaxLabel);
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
    // Past a few hundred, labels are not read one by one, whatever the file says about every nth.
    const int most = int(std::ceil(double(labels.size()) / std::max(1, m_limits.axisLabels)));
    if (!alongX) {
        // Up the page: the labels are level, one to a category; every nth if they crowd.
        if (axis.labelSkip <= 0 && lineH > slot) fit.skip = int(std::ceil(lineH * 1.1 / slot));
        m_leftOut = m_leftOut || most > fit.skip;
        fit.skip = std::max(fit.skip, most);
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
    m_leftOut = m_leftOut || most > fit.skip;
    fit.skip = std::max(fit.skip, most);
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
        for (double t = scale.min; t <= scale.max + scale.major * 1e-6 && out.size() < kMaxTicks + 2; t += scale.major) out << t;
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
    const QRectF given = manual ? manualPlot() : outer;
    const bool fixed = manual && c.plotInner;
    // (the titles of the axes stand at the edge of what the plot and its labels may take: for a plot whose inner place is fixed, that is the chart's, not the plot's)
    const QRectF frame = fixed ? outer : given;
    LabelFit fit;
    const bool valueAxisAtTop = (m_val->crosses == QLatin1String("max")) != m_catReversed;
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
            // The value axis is along the bottom, or along the top when it crosses at the last category (or the categories are turned round).
            const double valueSpace = (valLabels ? valLineH + 4 : 0) + (outTick(*m_val) ? tickLen : 0) + valTitleH;
            (valueAxisAtTop ? topSpace : bottomSpace) = valueSpace;
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

    // The lines of the plot share what is drawn of them.
    int lines = 0;
    for (const View &v : m_views)
        if (v.type == ChartType::Line || v.type == ChartType::Scatter) lines += int(v.plot->series.size());
    m_strokeShare = kStrokeLength * (m_inner.width() + m_inner.height()) / std::max(1, lines);

    // ---- the plot area's own fill and border
    m_p->save();
    if (c.plotFill.isValid()) m_p->fillRect(m_inner, c.plotFill);
    const QColor classic = lineColor();
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
        if (drawSeparately(int(at.size()))) {
            for (double pos : at) {
                if (m_swap == (&a == m_val)) m_p->drawLine(QPointF(pos, m_inner.top()), QPointF(pos, m_inner.bottom()));
                else m_p->drawLine(QPointF(m_inner.left(), pos), QPointF(m_inner.right(), pos));
            }
        } else {
            QPainterPath lines;
            for (double pos : at) {
                if (m_swap == (&a == m_val)) {
                    lines.moveTo(pos, m_inner.top());
                    lines.lineTo(pos, m_inner.bottom());
                } else {
                    lines.moveTo(m_inner.left(), pos);
                    lines.lineTo(m_inner.right(), pos);
                }
            }
            m_p->setBrush(Qt::NoBrush);
            m_p->drawPath(lines);
        }
    };
    auto onPage = [&](const QVector<double> &ticks, const Scale &scale, bool horizontal) {
        // A scatter's axes both run by value; a chart with categories has them at their units.
        QVector<double> out;
        for (double t : ticks) {
            if (scatter && horizontal) {
                double f = (t - scale.min) / (scale.max - scale.min);
                if (!std::isfinite(f)) f = 0;
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
        const bool separate = drawSeparately(int(along.size()));
        QPainterPath marks;
        for (double pos : along) {
            const QPointF at = horizontal ? QPointF(pos, from.y()) : QPointF(from.x(), pos);
            if (separate) {
                m_p->drawLine(at - side * in, at + side * out);
            } else {
                marks.moveTo(at - side * in);
                marks.lineTo(at + side * out);
            }
        }
        if (!separate) {
            m_p->setBrush(Qt::NoBrush);
            m_p->drawPath(marks);
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
            // The value axis stands at the first category (its far end if it crosses at the last), and turning the categories round swaps the ends.
            const bool right = which == 1 || ((a.crosses == QLatin1String("max")) != m_catReversed);
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
        const double y = valueAxisAtTop ? m_inner.top() : m_inner.bottom();
        const double away = valueAxisAtTop ? -1 : 1;
        QVector<double> along;
        for (double t : valueTicks) along << valueAlong(t, m_scale);
        axisLine(*m_val, QPointF(m_inner.left(), y), QPointF(m_inner.right(), y), along, true, QPointF(0, away));
        if (valLabels)
            for (int i = 0; i < valueTicks.size(); ++i)
                drawText(valueLabels[i], valFont, colorOf(m_val->font), QPointF(along[i], y + away * (4 + (outTick(*m_val) ? tickLen : 0))), 0.5, valueAxisAtTop ? 1 : 0);
    }
    m_p->restore();

    // ---- axis titles
    if (catTitleH > 0) {
        if (m_swap) drawAxisTitle(m_cat->title, catTitleFont, catTitleColor, QPointF(frame.left() + catTitleH / 2 - 2, m_inner.center().y()), 0.5, -90);
        else drawAxisTitle(m_cat->title, catTitleFont, catTitleColor, QPointF(m_inner.center().x(), frame.bottom() - 1), 1.0, 0);
    }
    if (valTitleH > 0) {
        if (m_swap) drawAxisTitle(m_val->title, valTitleFont, valTitleColor, QPointF(m_inner.center().x(), valueAxisAtTop ? frame.top() + 1 : frame.bottom() - 1), valueAxisAtTop ? 0.0 : 1.0, 0);
        else drawAxisTitle(m_val->title, valTitleFont, valTitleColor, QPointF(frame.left() + valTitleH / 2 - 2, m_inner.center().y()), 0.5, -90);
    }
    if (secondTitleH > 0 && !m_swap) drawAxisTitle(m_val2->title, secondTitleFont, secondTitleColor, QPointF(frame.right() - secondTitleH / 2 + 2, m_inner.center().y()), 0.5, 90);
}

// ---- the whole chart

QRectF ChartPainter::manualPlot() const
{
    const Chart &c = m_chart;
    const QRectF file(c.plotX * m_size.width(), c.plotY * m_size.height(), c.plotW * m_size.width(), c.plotH * m_size.height());
    const QRectF kept = file.intersected(m_room);
    return kept.width() >= 10 && kept.height() >= 10 ? kept : file;
}

void ChartPainter::drawAxisTitle(const ChartTitle &t, const QFont &font, const QColor &color, const QPointF &where, double vertical, double usual)
{
    const double angle = t.hasRotation ? t.rotation : usual;
    if (!t.manual) {
        drawText(t.text, font, color, where, 0.5, vertical, angle);
        return;
    }
    // the file puts the title's box (as it is turned) at a corner of its own: the text is centered in that box
    double w = 0;
    const QStringList lines = writable(t.text, kMaxText).split(QLatin1Char('\n'));
    for (const QString &line : lines) w = std::max(w, widthOf(font, line));
    const double h = heightOf(font, int(lines.size()));
    const double rad = angle * kPi / 180, cs = std::fabs(std::cos(rad)), sn = std::fabs(std::sin(rad));
    const double boxW = w * cs + h * sn, boxH = w * sn + h * cs;
    drawText(t.text, font, color, QPointF(t.x * m_size.width() + boxW / 2, t.y * m_size.height() + boxH / 2), 0.5, 0.5, angle);
}

void ChartPainter::paint()
{
    const Chart &c = m_chart;
    m_p->setRenderHint(QPainter::Antialiasing);
    m_p->setRenderHint(QPainter::TextAntialiasing);
    const QRectF full(QPointF(0, 0), m_size);
    if (c.background.isValid()) m_p->fillRect(full, c.background);
    if (c.border.set && !c.border.none) {
        m_p->setPen(penFor(c.border, lineColor(), 0.75));
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
        // (across the top, or where the file puts its corner; the room it would take is kept either way, as the plot is laid out for it)
        drawAxisTitle(c.title, font, color, QPointF(area.center().x(), area.top()), 0.0, 0);
        if (!c.title.overlay) area.setTop(area.top() + h + margin * 0.6);
    }

    // The legend: where the file puts it, else at its side (and the plot has what is left, unless the legend lies over it). It is drawn after the plot, so a column does not cover it.
    QVector<Entry> legend;
    QVector<int> legendBreaks;
    QRectF legendBox;
    m_room = full.adjusted(margin, margin, -margin, -margin);   // (the title does not take room from a plot the file fixes: the file put it where it is)
    if (c.legend.shown) {
        legend = legendEntries();
        if (!legend.isEmpty()) {
            const double pad = 5;
            const bool sized = c.legend.manual && c.legend.w > 0 && c.legend.h > 0;
            const QSizeF size = legendSize(legend, sized ? c.legend.w * m_size.width() - 2 * pad : area.width(), &legendBreaks);
            const QString pos = c.legend.position;
            QRectF box;
            if (pos == QLatin1String("l")) box = QRectF(area.left(), area.center().y() - size.height() / 2, size.width(), size.height());
            else if (pos == QLatin1String("t")) box = QRectF(area.center().x() - size.width() / 2, area.top(), size.width(), size.height());
            else if (pos == QLatin1String("b")) box = QRectF(area.center().x() - size.width() / 2, area.bottom() - size.height(), size.width(), size.height());
            else if (pos == QLatin1String("tr")) box = QRectF(area.right() - size.width(), area.top(), size.width(), size.height());
            else box = QRectF(area.right() - size.width(), area.center().y() - size.height() / 2, size.width(), size.height());
            const bool placed = c.legend.manual;
            if (placed) {
                // the file's place: its corner (or how far it is moved from its usual place), and its size if it says
                const QPointF corner = c.legend.edge ? QPointF(c.legend.x * m_size.width(), c.legend.y * m_size.height())
                                                     : box.topLeft() + QPointF(c.legend.x * m_size.width(), c.legend.y * m_size.height());
                box = QRectF(corner, sized ? QSizeF(c.legend.w * m_size.width(), c.legend.h * m_size.height()) : box.size());
            }
            legendBox = box;
            if (!c.legend.overlay && !placed) {
                const double gap = margin * 0.6;
                if (pos == QLatin1String("l")) area.setLeft(box.right() + gap);
                else if (pos == QLatin1String("t")) area.setTop(box.bottom() + gap);
                else if (pos == QLatin1String("b")) area.setBottom(box.top() - gap);
                else area.setRight(box.left() - gap);
                // (and a plot the file fixes keeps out of it as well)
                if (pos == QLatin1String("l")) m_room.setLeft(area.left());
                else if (pos == QLatin1String("t")) m_room.setTop(area.top());
                else if (pos == QLatin1String("b")) m_room.setBottom(area.bottom());
                else m_room.setRight(area.right());
            }
        }
    }
    if (area.width() >= 10 && area.height() >= 10 && !c.plots.isEmpty()) {
        const ChartType type = c.plots.first().type;
        if (type == ChartType::Pie || type == ChartType::Doughnut) paintPie(c.plotManual ? manualPlot() : area);
        else paintAxes(area);
    }
    if (!legend.isEmpty()) drawLegend(legendBox, legend, legendBreaks);
}

} // namespace

namespace {

// Every `stride`th of a list, from the first.
template <class T>
T everyNth(const T &list, int stride)
{
    if (stride <= 1) return list;
    T out;
    for (qsizetype i = 0; i < list.size(); i += stride) out << list[i];
    return out;
}

// The shapes a series is drawn in, each of its own (a column for each of a bar's values, a shape for each run of values of an area), and
// the markers on its points (of a line or a scatter that has them).
struct Pieces {
    qsizetype shapes = 0, markers = 0;
};

Pieces piecesOf(const ChartPlot &plot, const ChartSeries &s, const QString &blanks)
{
    auto finite = [](const QVector<double> &v) {
        qsizetype n = 0;
        for (const double x : v) n += std::isfinite(x);
        return n;
    };
    Pieces out;
    switch (plot.type) {
    case ChartType::Line: {
        const ChartMarker::Symbol symbol = s.marker.symbol;
        if (symbol != ChartMarker::None && (symbol != ChartMarker::Auto || plot.markers)) out.markers = finite(s.values);
        break;
    }
    case ChartType::Scatter:
        if (s.marker.symbol != ChartMarker::None)
            for (qsizetype i = 0; i < std::min(s.values.size(), s.xValues.size()); ++i) out.markers += std::isfinite(s.values[i]) && std::isfinite(s.xValues[i]);
        break;
    case ChartType::Area: {
        if (blanks == QLatin1String("zero") || blanks == QLatin1String("span")) {
            out.shapes = 1;
            break;
        }
        bool inside = false;
        for (const double x : s.values) {
            if (std::isfinite(x) && !inside) ++out.shapes;
            inside = std::isfinite(x);
        }
        break;
    }
    case ChartType::Pie:
    case ChartType::Doughnut:
        break;
    default:
        out.shapes = finite(s.values);
    }
    return out;
}

// The chart as it is drawn: every nth point of each series when the chart has more than a picture can show (the same n for all of them, so
// that their points still line up), and numbers no scale is made of as blanks. `thinned` is told whether points were left out.
Chart drawnChart(const Chart &chart, const Limits &limits, bool *thinned)
{
    qsizetype most = 0, total = 0, shapes = 0, markers = 0;
    bool unusable = false;
    for (const ChartPlot &plot : chart.plots)
        for (const ChartSeries &s : plot.series) {
            const qsizetype n = std::max({s.values.size(), s.categories.size(), s.xValues.size()});
            most = std::max(most, n);
            total += n;
            const Pieces pieces = piecesOf(plot, s, chart.blanksAs);
            shapes += pieces.shapes;
            markers += pieces.markers;
            for (const double v : s.values) unusable = unusable || (std::isfinite(v) && std::fabs(v) > kMaxMagnitude);
            for (const double v : s.xValues) unusable = unusable || (std::isfinite(v) && std::fabs(v) > kMaxMagnitude);
        }
    const bool slices = !chart.plots.isEmpty() && (chart.plots.first().type == ChartType::Pie || chart.plots.first().type == ChartType::Doughnut);
    auto times = [](qsizetype n, int limit) { return (n + std::max(1, limit) - 1) / std::max(1, limit); };
    const int stride = int(std::max<qsizetype>({1, times(most, slices ? limits.slices : limits.points), times(total, slices ? 2 * limits.slices : limits.total), times(shapes, limits.pieces), times(markers, limits.markers)}));
    *thinned = stride > 1;
    if (stride == 1 && !unusable) return chart;
    auto usable = [](const QVector<double> &numbers) {
        QVector<double> out = numbers;
        for (double &v : out)
            if (std::isfinite(v) && std::fabs(v) > kMaxMagnitude) v = kNaN;
        return out;
    };
    Chart out = chart;
    for (ChartPlot &plot : out.plots)
        for (ChartSeries &s : plot.series) {
            s.values = usable(everyNth(s.values, stride));
            s.xValues = usable(everyNth(s.xValues, stride));
            s.categories = everyNth(s.categories, stride);
            if (stride > 1) {
                QMap<int, QColor> colors;
                for (auto it = s.pointColors.cbegin(); it != s.pointColors.cend(); ++it)
                    if (it.key() % stride == 0) colors.insert(it.key() / stride, it.value());
                s.pointColors = colors;
            }
        }
    return out;
}

// A chart drawn as SVG once, within `limits`.
QByteArray drawOnce(const Chart &chart, const QSizeF &s, const Limits &limits, bool *leftOut)
{
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    // SVG 1.1, with the resolution a point to the unit: font sizes in the file are then the chart's own.
    QSvgGenerator gen(QSvgGenerator::SvgVersion::Svg11);
    gen.setOutputDevice(&buf);
    gen.setResolution(72);
    gen.setSize(QSize(int(std::ceil(s.width())), int(std::ceil(s.height()))));
    gen.setViewBox(QRectF(QPointF(0, 0), s));
    gen.setTitle(QCoreApplication::translate("Chart", "Chart"));
    gen.setDescription(QString());
    {
        QPainter p(&gen);
        ChartPainter painter(chart, &p, &gen, s, limits);
        painter.paint();
        *leftOut = painter.leftOut();
    }
    return buf.data();
}

// A size a picture can have: at least a point, and at most the longest side a slide has; a size that is no number is a point.
QSizeF pictureSize(const QSizeF &size)
{
    auto side = [](double v) { return std::isnan(v) ? 1.0 : std::clamp(v, 1.0, kChartMaxSide); };
    return QSizeF(side(size.width()), side(size.height()));
}

} // namespace

static thread_local qint64 g_svgsMade = 0;
qint64 chartSvgsMadeForTest() { return g_svgsMade; }

QByteArray chartSvg(const Chart &chart, const QSizeF &size, bool *coarser)
{
    ++g_svgsMade;
    const QSizeF s = pictureSize(size);
    // A picture that still has more tags than its reader takes in is drawn again from fewer points and in fewer pieces.
    bool thinned = false, leftOut = false;
    for (int level = 0;; ++level) {
        const Limits limits = limitsAt(level);
        const Chart drawn = drawnChart(chart, limits, &thinned);
        const QByteArray svg = drawOnce(drawn, s, limits, &leftOut);
        if (level >= kMaxLevel || (svg.count('<') <= kMaxTags && svg.size() <= kMaxBytes)) {
            if (coarser) *coarser = thinned || leftOut || level > 0;
            return svg;
        }
    }
}

QByteArray chartSvgTuned(const Chart &chart, const QSizeF &size, int separateLines, int segmentsPerPath, int separateMarkers)
{
    Limits limits = limitsAt(0);
    if (separateLines >= 0) limits.separateLines = separateLines;
    if (separateMarkers >= 0) limits.separateMarkers = separateMarkers;
    if (segmentsPerPath > 0) limits.segmentsPerPath = segmentsPerPath;
    bool thinned = false, leftOut = false;
    return drawOnce(drawnChart(chart, limits, &thinned), pictureSize(size), limits, &leftOut);
}

} // namespace jp::office
