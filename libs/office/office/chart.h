#pragma once
// Charts (DrawingML Charts, ECMA-376 Part 1, chapter 21): the chart parts
// that presentations, documents, and workbooks hold. A chart part is read
// into a model of what it draws, from the values the file stores in each
// series (the cached ones: the file's own numbers are not looked up in an
// embedded workbook), and drawn as SVG so it scales crisply.
//
// Where the file says nothing about how something looks, it looks as the
// chart's style (c:style, 2 when it has none) makes it: the colorful
// style's gray lines and black text, series in the theme's accent colors.

#include "office/drawingml_color.h"
#include "office/theme.h"

#include <QByteArray>
#include <QColor>
#include <QDomElement>
#include <QMap>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::office {

// Whether the graphic frame (p:graphicFrame) holds a chart: its
// a:graphicData names the chart's URI.
bool isChartFrame(const QDomElement &graphicFrame);
// The relationship (r:id) of the frame's c:chart, which names the chart
// part; empty if the frame has none.
QString chartFrameRelId(const QDomElement &graphicFrame);

enum class ChartType {
    Bar,        // bars and columns (see ChartPlot::horizontal)
    Line,
    Pie,
    Doughnut,
    Area,
    Scatter,
    Other       // a kind not drawn yet (radar, stock, surface): drawn as columns of its values
};

enum class ChartGrouping { Clustered, Stacked, PercentStacked, Standard };

// What a text says about its look; a value left at its default is "not said".
struct ChartFont {
    double size = 0;                // points; 0: the chart's default for it
    int bold = -1, italic = -1;     // 1 or 0 when said
    QColor color;                   // invalid: the chart's text color
    QString family;                 // empty: the theme's body font
};

struct ChartLine {
    bool set = false;               // the file says how the line looks
    bool none = false;              // no line
    QColor color;
    double width = 0;               // points; 0: the usual
    QString dash;                   // the preset dash name ("dash", "sysDot"...); empty: solid
};

struct ChartTitle {
    bool present = false;
    QString text;                   // paragraphs separated by newlines; empty with `present`: the automatic text
    ChartFont font;
    bool overlay = false;           // drawn over the plot, not above it
    bool hasRotation = false;       // the file turns it (degrees clockwise); an axis title's usual turn is not written here
    double rotation = 0;
    QString ref;                    // the cells the text is read from (c:strRef's c:f), else empty
    bool automatic = false;         // a chart title with no text of its own, named for its series
    // Where the file puts the title (c:layout's c:manualLayout, from the chart's edges): its top left corner, as fractions of the chart.
    bool manual = false;
    double x = 0, y = 0;
};

struct ChartMarker {
    enum Symbol { Auto, None, Circle, Dash, Diamond, Dot, Plus, Square, Star, Triangle, X };
    Symbol symbol = Auto;
    double size = 5;                // points
    QColor fill, line;              // invalid: the series' color
    bool noFill = false, noLine = false;
};

struct ChartDataLabels {
    bool value = false, category = false, series = false, percent = false;
    QString position;               // c:dLblPos: "outEnd", "ctr", "inEnd", "inBase", "bestFit", "l", "r", "t", "b"; empty: the usual for the chart
    QString format;                 // a number format; empty: the values' own
    ChartFont font;
    QString separator;              // empty: ", "
    bool shown() const { return value || category || series || percent; }
};

struct ChartSeries {
    QString name;
    // What the file stores for each point; a point it stores none for is a
    // blank: an empty category, NaN for a value.
    QStringList categories;
    QVector<double> values;
    QVector<double> xValues;        // a scatter's, and a bubble's: its c:xVal
    QString format;                 // the number format of the values ("General")
    // The series' own color: the fill of its bars, area, and slices, the
    // stroke of its line. The file's, else the theme's accent for its place
    // in the chart (the accents in order, then each again darker and lighter).
    QColor color;
    bool colorFromFile = false;
    bool noFill = false;            // the file says no fill
    ChartLine outline;              // the border of bars, areas, and slices; the stroke of a line
    QMap<int, QColor> pointColors;  // a point (or a slice) of another color, by its place
    // Where a chart in a workbook reads the series' data (the c:f of its name, its categories, and its values, or of a scatter's x and y values); empty when
    // the file gives a literal or says nothing. The name, categories, and values above are what the file stored; a program that has the cells puts theirs in.
    QString nameRef, categoryRef, valueRef, xRef;
    bool categoryRefNumeric = false;    // the categories are numbers (c:numRef): `categories` shows them through the category axis's number format when that is not linked to the source, else the source's
    QVector<double> categoryNumbers;    // and these are the numbers (NaN for a blank); empty for text categories
    QString categoryFormat;             // the number format of the source (c:formatCode of the cache)
    ChartMarker marker;
    bool smooth = false;
    ChartDataLabels labels;
    double explosion = 0;           // a pie's slice pulled out, percent of the radius
};

// A kind of drawing in the plot area (c:barChart, c:lineChart, ...); a chart
// may have several (columns with a line over them).
struct ChartPlot {
    ChartType type = ChartType::Bar;
    QString element;                // the file's name for it ("barChart"), for what is drawn as Other
    ChartGrouping grouping = ChartGrouping::Clustered;
    bool horizontal = false;        // bars along the horizontal axis (c:barDir "bar"); else columns
    int gapWidth = 150;             // between bars, percent of a bar
    int overlap = 0;                // of bars in a group, percent of a bar
    bool varyColors = false;        // each point, not each series, takes a color
    bool markers = true;            // a line chart's markers (c:marker)
    int firstSliceAngle = 0;        // pie: degrees clockwise from twelve o'clock
    int holeSize = 50;              // doughnut: percent of the diameter
    bool lines = true;              // scatter: the points are joined (else markers only)
    qint64 axisIds[2] = {0, 0};     // the category (or horizontal) and the value axes it is drawn against
    QVector<ChartSeries> series;
};

struct ChartAxis {
    enum Kind { Category, Value, Date, Series };
    Kind kind = Category;
    qint64 id = 0, crossId = 0;
    QString position = QStringLiteral("b");   // "b", "t", "l", "r"
    bool deleted = false;
    bool reversed = false;          // c:orientation "maxMin"
    bool autoMin = true, autoMax = true;
    double min = 0, max = 0;        // when not automatic
    double majorUnit = 0, minorUnit = 0;     // 0: automatic
    bool majorGridlines = false, minorGridlines = false;
    ChartLine gridline;
    QString format = QStringLiteral("General");   // the labels' number format
    bool formatLinked = true;       // the labels take the values' own format
    ChartTitle title;
    QString labelPosition = QStringLiteral("nextTo");   // "nextTo", "low", "high", or "none"
    QString majorTick = QStringLiteral("out"), minorTick = QStringLiteral("none");   // "out", "in", "cross", "none"
    ChartLine line;
    ChartFont font;                 // of its labels
    double labelRotation = 0;       // degrees, when `hasRotation`
    bool hasRotation = false;
    QString crosses = QStringLiteral("autoZero");   // where the other axis crosses it: "autoZero", "min", "max"
    bool hasCrossesAt = false;
    double crossesAt = 0;
    int labelSkip = 0;              // every nth category label; 0: as many as fit
    bool between = true;            // a value axis: the categories sit between ticks (else on them)
};

struct ChartLegend {
    bool shown = false;
    QString position = QStringLiteral("r");   // "r", "l", "t", "b", "tr"
    bool overlay = false;
    ChartFont font;
    // Where the file puts the legend (c:layout's c:manualLayout), as fractions of the chart: its top left corner (`edge`), or how far it is moved from where its position puts it
    // (not `edge`); `w` and `h` are its size, 0 if the file does not give one (the entries' own).
    bool manual = false, edge = true;
    double x = 0, y = 0, w = 0, h = 0;
    QColor fill;                    // the legend's own fill; invalid for none
    ChartLine border;               // and its border
};

struct Chart {
    QVector<ChartPlot> plots;
    QVector<ChartAxis> axes;
    ChartTitle title;
    ChartLegend legend;
    int style = 2;                  // c:style, 1 to 48
    // What it is set in and drawn on, from the theme and the color map.
    QVector<QColor> palette;        // the colors series (and slices) take when the file gives none, in order
    QString fontFamily;
    ChartFont font;                 // c:txPr of the chart: its text, unless a part says otherwise
    QColor textColor, backgroundColor;
    QColor background;              // the chart area's fill; invalid for none
    ChartLine border;               // the chart area's border
    QColor plotFill;                // the plot area's fill; invalid for none
    ChartLine plotBorder;
    // The plot area's place when the file fixes it, as fractions of the chart.
    bool plotManual = false, plotInner = true;
    double plotX = 0, plotY = 0, plotW = 0, plotH = 0;
    QString blanksAs = QStringLiteral("gap");   // c:dispBlanksAs: "gap", "zero", or "span"
    bool plotVisibleOnly = true;                // c:plotVisOnly: the cells in hidden rows and columns are left out

    // The first plot's kind; Bar for a chart with none.
    ChartType type() const { return plots.isEmpty() ? ChartType::Bar : plots.first().type; }
    // The axis with the id, or null.
    const ChartAxis *axis(qint64 id) const;
};

// Reads a chart part (c:chartSpace). `theme` and `colors` say what the
// theme's colors and the slide's color names stand for. False, with a
// message, if the XML isn't a chart: a chart that has nothing to draw
// (no series) is still one.
bool readChart(const QByteArray &chartPartXml, const Theme &theme, const ColorMap &colors, Chart *out, QString *error = nullptr);

// How many characters of number format codes were parsed on this thread since it began (a code past what a spreadsheet allows is not parsed): a count for tests of cost.
qint64 chartFormatCharsReadForTest();

// The largest a chart is drawn on a side, in points: the longest side a
// slide has (56 inches). A frame that asks for more is drawn this large.
constexpr double kChartMaxSide = 4032;
// The most of the text a chart keeps for a label, name, or title; real ones
// are a few dozen characters, and a longer text can't be read as a label.
constexpr int kChartMaxLabel = 256;

// The chart as SVG, `size` points across and high (at most kChartMaxSide):
// shapes, and the text as text in the chart's fonts. A chart with more to
// draw than a picture can show (tens of thousands of points) is drawn
// coarser, from every nth point, and leaves out the labels that crowd it;
// `coarser` is told whether it was.
QByteArray chartSvg(const Chart &chart, const QSizeF &size, bool *coarser = nullptr);
// The same, drawn as a test wants to see it: gridlines and the marks of axes
// drawn one by one while the chart has this many of them (more are one path;
// the usual is 300), a line of many points drawn in paths of this many
// segments (the usual is 32; it is drawn so only if it has more than 256
// points), and markers drawn one by one while the chart has this many (more
// are one path for a series; the usual is 1,500). -1 (0 for the segments)
// leaves the usual. The picture is the same however they are set.
QByteArray chartSvgTuned(const Chart &chart, const QSizeF &size, int separateLines, int segmentsPerPath, int separateMarkers = -1);

// A number as a number format code shows it ("0.0%", "#,##0", "$#,##0.00;($#,##0.00)",
// "mmm-yy"...); "General", or no code, shows it as short as it can be.
QString formatChartNumber(double value, const QString &formatCode);

} // namespace jp::office
