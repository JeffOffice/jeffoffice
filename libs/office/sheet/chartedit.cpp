#include "sheet/chartedit.h"

#include <QCoreApplication>

#include <algorithm>

namespace jp::office::sheet {

using jp::office::Chart;
using jp::office::ChartAxis;
using jp::office::ChartPlot;
using jp::office::ChartSeries;
using jp::office::ChartType;

namespace {

QString tr(const char *text) { return QCoreApplication::translate("Chart", text); }

bool isPie(const Chart &c) { return !c.plots.isEmpty() && (c.plots.first().type == ChartType::Pie || c.plots.first().type == ChartType::Doughnut); }

int totalSeries(const Chart &c)
{
    int n = 0;
    for (const ChartPlot &p : c.plots) n += int(p.series.size());
    return n;
}

} // namespace

const ChartAxis *primaryAxis(const Chart &chart, bool horizontal)
{
    if (chart.plots.isEmpty()) return nullptr;
    const ChartPlot &p = chart.plots.first();
    for (const qint64 id : {p.axisIds[0], p.axisIds[1]}) {
        const ChartAxis *a = chart.axis(id);
        if (!a) continue;
        const bool across = a->position == QLatin1String("b") || a->position == QLatin1String("t");
        if (across == horizontal) return a;
    }
    return nullptr;
}

ChartAxis *primaryAxis(Chart *chart, bool horizontal) { return const_cast<ChartAxis *>(primaryAxis(*chart, horizontal)); }

ChartElements elementsOf(const Chart &chart)
{
    ChartElements e;
    e.title = chart.title.present;
    e.titleOverlay = chart.title.present && chart.title.overlay;
    for (const bool horizontal : {true, false}) {
        const ChartAxis *a = primaryAxis(chart, horizontal);
        bool &has = horizontal ? e.hasHorizontal : e.hasVertical;
        has = a != nullptr;
        if (!a) continue;
        (horizontal ? e.axisHorizontal : e.axisVertical) = !a->deleted;
        (horizontal ? e.axisTitleHorizontal : e.axisTitleVertical) = a->title.present;
    }
    // a line across the chart belongs to the axis that runs up it
    if (const ChartAxis *v = primaryAxis(chart, false)) {
        e.gridHorizontal = v->majorGridlines;
        e.minorGridHorizontal = v->minorGridlines;
    }
    if (const ChartAxis *h = primaryAxis(chart, true)) {
        e.gridVertical = h->majorGridlines;
        e.minorGridVertical = h->minorGridlines;
    }
    e.legend = chart.legend.shown;
    e.legendPosition = chart.legend.shown ? chart.legend.position : QString();
    for (const ChartPlot &p : chart.plots)
        for (const ChartSeries &s : p.series)
            if (s.labels.shown()) {
                e.labels = true;
                e.labelPosition = s.labels.position;
            }
    return e;
}

void setChartTitle(Chart *chart, TitleMode mode)
{
    ChartTitle &t = chart->title;
    if (mode == TitleMode::None) {
        t = ChartTitle();
        return;
    }
    if (!t.present) {
        t = ChartTitle();
        t.present = true;
        if (totalSeries(*chart) == 1 && chart->type() != ChartType::Scatter) t.automatic = true;
        t.text = tr("Chart Title");
    }
    t.overlay = mode == TitleMode::Overlay;
}

void setAxisShown(Chart *chart, bool horizontal, bool shown)
{
    if (ChartAxis *a = primaryAxis(chart, horizontal)) a->deleted = !shown;
}

void setAxisTitle(Chart *chart, bool horizontal, bool shown)
{
    ChartAxis *a = primaryAxis(chart, horizontal);
    if (!a) return;
    if (!shown) {
        a->title = ChartTitle();
        return;
    }
    if (a->title.present) return;
    a->title = ChartTitle();
    a->title.present = true;
    a->title.text = tr("Axis Title");
}

void setGridlines(Chart *chart, bool horizontal, bool minor, bool shown)
{
    // the lines across the chart are those of the axis that runs up it
    ChartAxis *a = primaryAxis(chart, !horizontal);
    if (!a) return;
    (minor ? a->minorGridlines : a->majorGridlines) = shown;
}

void setLegend(Chart *chart, const QString &position)
{
    if (position.isEmpty()) {
        chart->legend.shown = false;
        return;
    }
    chart->legend.shown = true;
    chart->legend.position = position;
    chart->legend.overlay = false;
    chart->legend.manual = false;
}

QStringList labelPositionsFor(const Chart &chart)
{
    if (chart.plots.isEmpty()) return {};
    const ChartPlot &p = chart.plots.first();
    switch (p.type) {
    case ChartType::Bar:
    case ChartType::Other:
        return p.grouping == ChartGrouping::Clustered ? QStringList{"ctr", "inEnd", "inBase", "outEnd"} : QStringList{"ctr", "inEnd", "inBase"};
    case ChartType::Line:
    case ChartType::Scatter: return {"ctr", "l", "r", "t", "b"};
    case ChartType::Pie: return {"ctr", "inEnd", "outEnd", "bestFit"};
    case ChartType::Doughnut:
    case ChartType::Area: return {};
    }
    return {};
}

void setDataLabels(Chart *chart, bool shown, const QString &position)
{
    const bool pie = isPie(*chart);
    for (ChartPlot &p : chart->plots)
        for (ChartSeries &s : p.series) {
            if (!shown) {
                s.labels = ChartDataLabels();
                continue;
            }
            s.labels.value = !pie;
            s.labels.percent = pie;
            s.labels.position = position;
        }
}

// ---- colors and looks

namespace {

struct Scheme {
    const char *name;
    QList<QRgb> colors;   // empty: the theme's own
};

const QList<Scheme> &schemes()
{
    static const QList<Scheme> list = {
        {QT_TRANSLATE_NOOP("Chart", "Colorful"), {}},
        {QT_TRANSLATE_NOOP("Chart", "Blues"), {0x1F4E79, 0x2E75B6, 0x5B9BD5, 0x9DC3E6, 0xBDD7EE, 0xDEEBF7}},
        {QT_TRANSLATE_NOOP("Chart", "Greens"), {0x375623, 0x548235, 0x70AD47, 0xA9D18E, 0xC5E0B4, 0xE2F0D9}},
        {QT_TRANSLATE_NOOP("Chart", "Oranges"), {0x843C0C, 0xC55A11, 0xED7D31, 0xF4B183, 0xF8CBAD, 0xFBE5D6}},
        {QT_TRANSLATE_NOOP("Chart", "Grays"), {0x262626, 0x595959, 0x7F7F7F, 0xA5A5A5, 0xC9C9C9, 0xE7E7E7}},
        {QT_TRANSLATE_NOOP("Chart", "Warm"), {0xA5002E, 0xC00000, 0xED7D31, 0xFFC000, 0xF4B183, 0xFFE699}},
        {QT_TRANSLATE_NOOP("Chart", "Cool"), {0x264478, 0x2E75B6, 0x00B0A0, 0x7030A0, 0x5B9BD5, 0x70AD47}},
        {QT_TRANSLATE_NOOP("Chart", "Vivid"), {0xE6194B, 0x3CB44B, 0x4363D8, 0xF58231, 0x911EB4, 0x46C7C7, 0xF032E6, 0xBCF60C}},
    };
    return list;
}

} // namespace

int chartColorSchemeCount() { return int(schemes().size()); }

QString chartColorSchemeName(int index)
{
    return index >= 0 && index < schemes().size() ? QCoreApplication::translate("Chart", schemes()[index].name) : QString();
}

QColor chartSchemeColor(const Chart &chart, int scheme, int n)
{
    if (scheme < 0 || scheme >= schemes().size()) return QColor();
    const Scheme &s = schemes()[scheme];
    if (s.colors.isEmpty()) return chart.palette.isEmpty() ? QColor(0x44, 0x72, 0xC4) : chart.palette.value(n % chart.palette.size());
    return QColor(s.colors[n % s.colors.size()]);
}

void applyColorScheme(Chart *chart, int scheme)
{
    int n = 0;
    for (ChartPlot &p : chart->plots)
        for (ChartSeries &s : p.series) {
            s.color = chartSchemeColor(*chart, scheme, n);
            s.colorFromFile = true;
            s.noFill = false;
            for (auto it = s.pointColors.begin(); it != s.pointColors.end(); ++it) it.value() = chartSchemeColor(*chart, scheme, it.key());
            if (s.marker.fill.isValid()) s.marker.fill = s.color;
            ++n;
        }
}

namespace {

struct LookName {
    const char *name;
};
const char *const kStyleNames[] = {QT_TRANSLATE_NOOP("Chart", "Plain"), QT_TRANSLATE_NOOP("Chart", "Clean"), QT_TRANSLATE_NOOP("Chart", "Dark"), QT_TRANSLATE_NOOP("Chart", "Soft")};
const char *const kLayoutNames[] = {QT_TRANSLATE_NOOP("Chart", "Title and legend below"), QT_TRANSLATE_NOOP("Chart", "Values on the marks"), QT_TRANSLATE_NOOP("Chart", "Legend at the side, axes named"),
                                    QT_TRANSLATE_NOOP("Chart", "Marks only")};

void gridColor(Chart *c, const QColor &color, bool on)
{
    for (ChartAxis &a : c->axes) {
        if (a.id != c->plots.value(0).axisIds[0] && a.id != c->plots.value(0).axisIds[1]) continue;
        a.gridline = ChartLine();
        if (color.isValid()) {
            a.gridline.set = true;
            a.gridline.color = color;
            a.gridline.width = 0.75;
        }
        if (!on) a.majorGridlines = a.minorGridlines = false;
    }
}

} // namespace

int chartStyleCount() { return 4; }

QString chartStyleName(int index) { return index >= 0 && index < 4 ? QCoreApplication::translate("Chart", kStyleNames[index]) : QString(); }

void applyChartStyle(Chart *c, int style)
{
    const QColor border(0xD9, 0xD9, 0xD9);
    c->background = QColor(Qt::white);
    c->border = ChartLine();
    c->plotFill = QColor();
    c->plotBorder = ChartLine();
    c->font.color = QColor();
    c->textColor = QColor(Qt::black);
    switch (style) {
    case 0:
        c->border.set = true;
        c->border.color = border;
        c->border.width = 0.75;
        gridColor(c, QColor(0xD9, 0xD9, 0xD9), true);
        // the axis that runs up the chart has the lines across it, as a plain chart does
        if (ChartAxis *v = primaryAxis(c, false)) v->majorGridlines = true;
        break;
    case 1:
        c->border.set = true;
        c->border.none = true;
        gridColor(c, QColor(), false);
        break;
    case 2:
        c->background = QColor(0x26, 0x26, 0x26);
        c->border.set = true;
        c->border.none = true;
        c->textColor = QColor(Qt::white);
        c->font.color = QColor(0xF2, 0xF2, 0xF2);
        gridColor(c, QColor(0x59, 0x59, 0x59), true);
        break;
    case 3:
        c->border.set = true;
        c->border.none = true;
        c->plotFill = QColor(0xF2, 0xF2, 0xF2);
        gridColor(c, QColor(Qt::white), true);
        break;
    default: break;
    }
}

int chartLayoutCount() { return 4; }

QString chartLayoutName(int index) { return index >= 0 && index < 4 ? QCoreApplication::translate("Chart", kLayoutNames[index]) : QString(); }

void applyChartLayout(Chart *c, int layout)
{
    const bool pie = isPie(*c);
    switch (layout) {
    case 0:
        setChartTitle(c, TitleMode::Above);
        setLegend(c, QStringLiteral("b"));
        setDataLabels(c, false);
        setGridlines(c, true, false, true);
        break;
    case 1:
        setChartTitle(c, TitleMode::Above);
        setLegend(c, QString());
        setDataLabels(c, true, pie ? QStringLiteral("outEnd") : QString());
        setGridlines(c, true, false, false);
        setGridlines(c, false, false, false);
        break;
    case 2:
        setChartTitle(c, TitleMode::Above);
        setLegend(c, QStringLiteral("r"));
        setAxisTitle(c, true, true);
        setAxisTitle(c, false, true);
        break;
    case 3:
        setChartTitle(c, TitleMode::None);
        setLegend(c, QString());
        setGridlines(c, true, false, false);
        setGridlines(c, false, false, false);
        setDataLabels(c, false);
        break;
    default: break;
    }
}

// ---- kinds and series

QVector<ChartSeries> seriesOf(const Chart &chart)
{
    QVector<ChartSeries> out;
    for (const ChartPlot &p : chart.plots) out += p.series;
    return out;
}

int seriesCount(const Chart &chart) { return totalSeries(chart); }

QVector<SeriesKind> seriesKindsOf(const Chart &chart)
{
    QVector<SeriesKind> out;
    const qint64 first = chart.plots.isEmpty() ? 0 : chart.plots.first().axisIds[0];
    for (const ChartPlot &p : chart.plots) {
        const SeriesKind k{kindOfPlot(p), p.axisIds[0] != first};
        for (int i = 0; i < p.series.size(); ++i) out << k;
    }
    return out;
}

Chart withSeriesKinds(const Chart &chart, const QVector<SeriesKind> &kinds, const jp::office::Theme &theme)
{
    return withSeries(chart, seriesOf(chart), kinds, theme);
}

Chart withSeries(const Chart &chart, const QVector<ChartSeries> &series, const QVector<SeriesKind> &kindsIn, const jp::office::Theme &theme)
{
    QVector<SeriesKind> kinds = kindsIn;
    const QVector<SeriesKind> before = seriesKindsOf(chart);
    // series beyond the kinds given are drawn as the last was
    while (kinds.size() < series.size()) kinds << (kinds.isEmpty() ? (before.isEmpty() ? SeriesKind() : before.last()) : kinds.last());
    kinds.resize(series.size());
    Chart c = chartFromSeries(series, kinds, theme, &chart);
    if (!c.plots.isEmpty() && (c.plots[0].type == ChartType::Pie || c.plots[0].type == ChartType::Doughnut) && !c.plots[0].series.isEmpty()) {
        ChartSeries &s = c.plots[0].series[0];
        if (s.pointColors.isEmpty()) {
            const int slices = std::min<int>(256, std::max<int>(s.values.size(), s.categories.size()));
            for (int k = 0; k < slices; ++k) s.pointColors.insert(k, c.palette.isEmpty() ? QColor(0x44, 0x72, 0xC4) : c.palette.value(k % c.palette.size()));
        }
    } else {
        for (ChartPlot &p : c.plots)
            for (ChartSeries &s : p.series) s.pointColors.clear();   // (slices' colors mean nothing to a series drawn another way)
    }
    return c;
}

Chart withKind(const Chart &chart, ChartKind kind, const jp::office::Theme &theme)
{
    const int n = totalSeries(chart);
    QVector<SeriesKind> kinds;
    const bool combo = kind == ChartKind::Combo && n >= 2;
    for (int i = 0; i < n; ++i) kinds << (combo && i == n - 1 ? SeriesKind{ChartKind::LineMarkers, true} : SeriesKind{kind == ChartKind::Combo ? ChartKind::Column : kind, false});
    return withSeriesKinds(chart, kinds, theme);
}


namespace {

// A reference of one area on one sheet: the sheet's name and the cells.
bool parseOneArea(const QString &ref, QString *sheet, CellRange *range)
{
    const int bang = int(ref.lastIndexOf(QLatin1Char('!')));
    if (bang <= 0 || ref.contains(QLatin1Char(',')) || ref.contains(QLatin1Char('('))) return false;
    QString name = ref.left(bang);
    if (name.size() >= 2 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
    CellRange r;
    if (!parseRangeRef(QStringView(ref).mid(bang + 1), &r) || !r.valid) return false;
    *sheet = name;
    *range = r;
    return true;
}

} // namespace

QString chartDataRange(const Chart &chart)
{
    QString sheet;
    CellRange hull;
    for (const ChartSeries &s : seriesOf(chart))
        for (const QString &ref : {s.nameRef, s.categoryRef, s.valueRef, s.xRef}) {
            if (ref.isEmpty()) continue;
            QString name;
            CellRange r;
            if (!parseOneArea(ref, &name, &r)) return QString();
            if (hull.valid) {
                if (name.compare(sheet, Qt::CaseInsensitive) != 0) return QString();
                hull.row1 = std::min(hull.row1, r.row1);
                hull.col1 = std::min(hull.col1, r.col1);
                hull.row2 = std::max(hull.row2, r.row2);
                hull.col2 = std::max(hull.col2, r.col2);
            } else {
                sheet = name;
                hull = r;
            }
        }
    return hull.valid ? chartRef(sheet, hull) : QString();
}

bool seriesRunDownColumns(const Chart &chart)
{
    const QVector<ChartSeries> all = seriesOf(chart);
    if (all.isEmpty()) return false;
    QString sheet;
    CellRange r;
    if (!parseOneArea(all.first().valueRef, &sheet, &r)) return false;
    return r.col1 == r.col2;
}

Chart switchedRowsColumns(const Chart &chart, const jp::office::Theme &theme, bool *ok)
{
    *ok = false;
    const QVector<ChartSeries> all = seriesOf(chart);
    if (all.isEmpty()) return Chart();
    QString sheet;
    CellRange hull;
    bool columns = true, rows = true, names = true, cats = false;
    auto take = [&](const QString &ref) {
        QString name;
        CellRange r;
        if (!parseOneArea(ref, &name, &r)) return false;
        if (hull.valid) {
            if (name.compare(sheet, Qt::CaseInsensitive) != 0) return false;
            hull.row1 = std::min(hull.row1, r.row1);
            hull.col1 = std::min(hull.col1, r.col1);
            hull.row2 = std::max(hull.row2, r.row2);
            hull.col2 = std::max(hull.col2, r.col2);
        } else {
            sheet = name;
            hull = r;
        }
        return true;
    };
    for (const ChartSeries &s : all) {
        QString name;
        CellRange v;
        if (!parseOneArea(s.valueRef, &name, &v)) return Chart();
        columns = columns && v.col1 == v.col2;
        rows = rows && v.row1 == v.row2;
        if (!take(s.valueRef)) return Chart();
        if (s.nameRef.isEmpty()) names = false;
        else if (!take(s.nameRef)) return Chart();
        const QString &catRef = chart.plots.isEmpty() || chart.plots.first().type != ChartType::Scatter ? s.categoryRef : s.xRef;
        if (!catRef.isEmpty()) {
            cats = true;
            if (!take(catRef)) return Chart();
        }
    }
    if (!columns && !rows) return Chart();
    const bool downColumns = columns;   // (one cell each is a column)
    const bool scatter = !chart.plots.isEmpty() && chart.plots.first().type == ChartType::Scatter;
    const ChartKind kind = seriesKindsOf(chart).value(0).kind;
    ChartKind planKind = scatter ? ChartKind::Scatter : kind;
    const bool headerRow = downColumns ? names : cats, headerColumn = downColumns ? cats : names;
    const ChartPlan plan = planChartFrom(sheet, hull, headerRow, headerColumn, downColumns, planKind);
    if (plan.series.isEmpty()) return Chart();
    // the new plan is for series that run the other way: rows if they ran down columns
    QVector<ChartSeries> fresh;
    QVector<SeriesKind> kinds;
    for (int i = 0; i < plan.series.size(); ++i) {
        ChartSeries s;
        s.name = QCoreApplication::translate("Chart", "Series%1").arg(i + 1);
        s.nameRef = plan.series[i].nameRef;
        s.categoryRef = plan.series[i].categoryRef;
        s.valueRef = plan.series[i].valueRef;
        s.xRef = plan.series[i].xRef;
        fresh << s;
        kinds << SeriesKind{kind == ChartKind::Combo ? ChartKind::Column : kind, false};
    }
    *ok = true;
    return chartFromSeries(fresh, kinds, theme, &chart);
}

} // namespace jp::office::sheet
