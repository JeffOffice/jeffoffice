#include "sheet/chartops.h"

#include "office/chart_write.h"

#include <QCoreApplication>
#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

// (The words here keep the context "jp::write::Pictures" they had when only the word processor made charts, so their translations stay.)
namespace {

ChartKind kindOfForm(ChartForm form)
{
    using K = ChartKind;
    switch (form) {
    case ChartForm::Column: return K::Column;
    case ChartForm::ColumnStacked: return K::ColumnStacked;
    case ChartForm::ColumnPercent: return K::ColumnPercent;
    case ChartForm::Bar: return K::Bar;
    case ChartForm::BarStacked: return K::BarStacked;
    case ChartForm::Line: return K::Line;
    case ChartForm::LineMarkers: return K::LineMarkers;
    case ChartForm::Pie: return K::Pie;
    case ChartForm::Doughnut: return K::Doughnut;
    case ChartForm::Area: return K::Area;
    case ChartForm::AreaStacked: return K::AreaStacked;
    case ChartForm::Scatter: return K::Scatter;
    }
    return K::Column;
}

ChartForm formOfKind(ChartKind kind)
{
    using K = ChartKind;
    switch (kind) {
    case K::ColumnStacked: return ChartForm::ColumnStacked;
    case K::ColumnPercent: return ChartForm::ColumnPercent;
    case K::Bar:
    case K::BarPercent: return ChartForm::Bar;
    case K::BarStacked: return ChartForm::BarStacked;
    case K::Line:
    case K::LineStacked: return ChartForm::Line;
    case K::LineMarkers: return ChartForm::LineMarkers;
    case K::Pie: return ChartForm::Pie;
    case K::Doughnut: return ChartForm::Doughnut;
    case K::Area: return ChartForm::Area;
    case K::AreaStacked: return ChartForm::AreaStacked;
    case K::Scatter:
    case K::ScatterLines:
    case K::ScatterSmooth: return ChartForm::Scatter;
    default: return ChartForm::Column;
    }
}

// The series of a chart from the data: each its name, the categories, and its numbers. A scatter's categories are its x numbers (or 1, 2, 3 for words).
QVector<ChartSeries> seriesOfData(const ChartData &data, bool scatter)
{
    QVector<ChartSeries> out;
    QVector<double> xs;
    if (scatter) {
        for (int i = 0; i < data.categories.size(); ++i) {
            bool ok = false;
            const double v = data.categories[i].toDouble(&ok);
            xs << (ok && std::isfinite(v) ? v : double(i + 1));
        }
    }
    for (int s = 0; s < data.values.size(); ++s) {
        ChartSeries series;
        series.name = data.seriesNames.value(s, QCoreApplication::translate("jp::write::Pictures", "Series %1").arg(s + 1));
        series.categories = data.categories;
        series.values = data.values[s];
        series.xValues = xs;
        out << series;
    }
    return out;
}

} // namespace

const QVector<ChartForm> &chartForms()
{
    static const QVector<ChartForm> forms = {ChartForm::Column, ChartForm::ColumnStacked, ChartForm::ColumnPercent, ChartForm::Bar, ChartForm::BarStacked, ChartForm::Line, ChartForm::LineMarkers,
                                             ChartForm::Pie, ChartForm::Doughnut, ChartForm::Area, ChartForm::AreaStacked, ChartForm::Scatter};
    return forms;
}

ChartData sampleChartData()
{
    ChartData d;
    d.categories = {QCoreApplication::translate("jp::write::Pictures", "Category 1"), QCoreApplication::translate("jp::write::Pictures", "Category 2"), QCoreApplication::translate("jp::write::Pictures", "Category 3")};
    d.seriesNames = {QCoreApplication::translate("jp::write::Pictures", "Series 1"), QCoreApplication::translate("jp::write::Pictures", "Series 2")};
    d.values = {{4.3, 2.5, 3.5}, {2.4, 4.4, 1.8}};
    return d;
}

bool chartDataIsUsable(const ChartData &data)
{
    if (data.categories.isEmpty() || data.values.isEmpty() || data.categories.size() > kMostChartCategories || data.values.size() > kMostChartSeries) return false;
    for (const QVector<double> &v : data.values) {
        if (v.size() != data.categories.size()) return false;
        for (double x : v)
            if (!std::isfinite(x) || std::abs(x) > 1e15) return false;
    }
    return true;
}

QString cleanChartText(const QString &text, int limit)
{
    QString clean;
    clean.reserve(int(std::min<qsizetype>(text.size(), limit)));
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar ch = text[i];
        const ushort u = ch.unicode();
        if (ch.isHighSurrogate()) {
            if (i + 1 < text.size() && text[i + 1].isLowSurrogate()) {
                if (clean.size() + 2 > limit) break;
                clean += ch;
                clean += text[i + 1];
                ++i;
            }
            continue;
        }
        if (ch.isLowSurrogate() || u == 0xFFFE || u == 0xFFFF) continue;
        if (clean.size() >= limit) break;
        clean += u < 0x20 && u != 0x09 ? QChar(QLatin1Char(' ')) : ch;
    }
    return clean;
}

ChartAxis *chartAxisOf(Chart &chart, ChartAxisKind which)
{
    ChartAxis *category = nullptr;
    for (ChartAxis &a : chart.axes)
        if (!category && (a.kind == ChartAxis::Category || a.kind == ChartAxis::Date)) category = &a;
    for (ChartAxis &a : chart.axes)
        if (!category && (a.position == QLatin1String("b") || a.position == QLatin1String("t"))) category = &a;   // (a scatter's x axis)
    if (which == ChartAxisKind::Category) return category;
    for (ChartAxis &a : chart.axes)
        if (&a != category && a.kind == ChartAxis::Value) return &a;
    return nullptr;
}

QVector<ChartSeries> chartAllSeries(const Chart &chart)
{
    QVector<ChartSeries> out;
    for (const ChartPlot &p : chart.plots) out << p.series;
    return out;
}

Chart chartOfData(ChartForm form, const ChartData &data, const Theme &theme)
{
    const ChartKind kind = kindOfForm(form);
    const QVector<ChartSeries> series = seriesOfData(data, form == ChartForm::Scatter);
    const QVector<SeriesKind> kinds(series.size(), SeriesKind{kind, false});
    return chartFromSeries(series, kinds, theme);
}

ChartInfo chartInfoOf(const Chart &made, const QByteArray &part)
{
    ChartInfo info;
    Chart chart = made;
    info.valid = true;
    info.form = formOfKind(kindOfChart(chart));
    info.titleShown = chart.title.present;
    info.title = chart.title.text;
    info.legendShown = chart.legend.shown;
    info.legendPosition = chart.legend.position;
    info.style = chart.style;
    if (ChartAxis *a = chartAxisOf(chart, ChartAxisKind::Category)) {
        info.hasAxes = true;
        info.categoryAxisShown = !a->deleted;
        info.categoryGridlines = a->majorGridlines;
        info.categoryAxisTitle = a->title.present ? a->title.text : QString();
    }
    if (ChartAxis *a = chartAxisOf(chart, ChartAxisKind::Value)) {
        info.hasAxes = true;
        info.valueAxisShown = !a->deleted;
        info.valueGridlines = a->majorGridlines;
        info.valueMinorGridlines = a->minorGridlines;
        info.valueAxisTitle = a->title.present ? a->title.text : QString();
    }
    const QVector<ChartSeries> series = chartAllSeries(chart);
    info.dataEditable = !series.isEmpty() && chartIsFullyModeled(part);
    for (const ChartSeries &s : series) {
        info.dataLabels = info.dataLabels || s.labels.shown();
        if (!s.valueRef.isEmpty() || !s.categoryRef.isEmpty() || !s.nameRef.isEmpty()) info.dataEditable = false;
    }
    if (!series.isEmpty()) {
        info.data.categories = series.first().categories;
        // (A scatter's x numbers are its categories.)
        if (info.data.categories.isEmpty())
            for (double x : series.first().xValues) info.data.categories << QString::number(x, 'g', 15);
    }
    for (const ChartSeries &s : series) {
        info.data.seriesNames << s.name;
        info.data.values << s.values;
    }
    return info;
}

bool chartSetForm(Chart &chart, ChartForm form, const Theme &theme)
{
    if (formOfKind(kindOfChart(chart)) == form && chart.plots.size() == 1) return false;
    QVector<ChartSeries> series = chartAllSeries(chart);
    if (series.isEmpty()) return false;
    // (A scatter's x numbers are the categories of a chart of another kind.)
    for (ChartSeries &s : series)
        if (s.categories.isEmpty())
            for (double x : s.xValues) s.categories << QString::number(x, 'g', 15);
    // (A scatter's points are placed by x numbers: the categories, read as numbers, or 1, 2, 3.)
    if (form == ChartForm::Scatter)
        for (ChartSeries &s : series)
            if (s.xValues.isEmpty())
                for (int i = 0; i < s.values.size(); ++i) {
                    bool ok = false;
                    const double v = s.categories.value(i).toDouble(&ok);
                    s.xValues << (ok && std::isfinite(v) ? v : double(i + 1));
                }
    const QVector<SeriesKind> kinds(series.size(), SeriesKind{kindOfForm(form), false});
    const Chart like = chart;
    chart = chartFromSeries(series, kinds, theme, &like);
    return true;
}

bool chartSetTitle(Chart &chart, bool shown, const QString &text)
{
    const QString clean = cleanChartText(text);
    const bool same = chart.title.present == shown && (!shown || (chart.title.text == clean && !chart.title.automatic));
    if (same) return false;
    chart.title.present = shown;
    if (shown) {
        chart.title.text = clean.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "Chart Title") : clean;
        chart.title.automatic = false;
        chart.title.ref.clear();
    }
    return true;
}

bool chartSetLegend(Chart &chart, const QString &legendPosition)
{
    if (!legendPosition.isEmpty() && !QStringList{QStringLiteral("r"), QStringLiteral("l"), QStringLiteral("t"), QStringLiteral("b")}.contains(legendPosition)) return false;
    const bool shown = !legendPosition.isEmpty();
    if (chart.legend.shown == shown && (!shown || chart.legend.position == legendPosition)) return false;
    chart.legend.shown = shown;
    if (shown) {
        chart.legend.position = legendPosition;
        chart.legend.manual = false;
    }
    return true;
}

bool chartSetDataLabels(Chart &chart, bool shown)
{
    bool now = false;
    for (const ChartPlot &p : chart.plots)
        for (const ChartSeries &s : p.series) now = now || s.labels.shown();
    if (now == shown) return false;
    for (ChartPlot &p : chart.plots)
        for (ChartSeries &s : p.series) {
            s.labels.value = shown;
            if (!shown) s.labels.category = s.labels.series = s.labels.percent = s.labels.legendKey = false;
        }
    return true;
}

bool chartSetAxis(Chart &chart, ChartAxisKind which, bool shown)
{
    ChartAxis *a = chartAxisOf(chart, which);
    if (!a || a->deleted == !shown) return false;
    a->deleted = !shown;
    return true;
}

bool chartSetGridlines(Chart &chart, ChartAxisKind which, bool major, bool minor)
{
    ChartAxis *a = chartAxisOf(chart, which);
    if (!a || (a->majorGridlines == major && a->minorGridlines == minor)) return false;
    a->majorGridlines = major;
    a->minorGridlines = minor;
    return true;
}

bool chartSetAxisTitle(Chart &chart, ChartAxisKind which, const QString &text)
{
    const QString clean = cleanChartText(text);
    ChartAxis *a = chartAxisOf(chart, which);
    if (!a || (a->title.present == !clean.isEmpty() && a->title.text == clean)) return false;
    a->title.present = !clean.isEmpty();
    a->title.text = clean;
    return true;
}

bool chartSetStyle(Chart &chart, int style)
{
    if (style < 1 || style > 48 || chart.style == style) return false;
    chart.style = style;
    return true;
}

bool chartSetData(Chart &chart, const ChartData &data, const Theme &theme)
{
    if (!chartDataIsUsable(data)) return false;
    const QVector<ChartSeries> old = chartAllSeries(chart);
    if (old.isEmpty()) return false;
    for (const ChartSeries &s : old)
        if (!s.valueRef.isEmpty() || !s.categoryRef.isEmpty() || !s.nameRef.isEmpty()) return false;
    const ChartKind kind = kindOfChart(chart);
    QVector<ChartSeries> series = seriesOfData(data, kind == ChartKind::Scatter || kind == ChartKind::ScatterLines || kind == ChartKind::ScatterSmooth);
    // The look of the series that were there stays with them (their colors, lines, and markers).
    for (int i = 0; i < series.size() && i < old.size(); ++i) {
        ChartSeries made = old[i];
        made.name = series[i].name;
        made.categories = series[i].categories;
        made.values = series[i].values;
        made.xValues = series[i].xValues;
        made.categoryNumbers.clear();
        made.categoryRefNumeric = false;
        series[i] = made;
    }
    QVector<SeriesKind> kinds;
    for (const ChartPlot &p : chart.plots)
        for (int i = 0; i < p.series.size(); ++i) kinds << SeriesKind{kindOfPlot(p), false};
    while (kinds.size() < series.size()) kinds << SeriesKind{kinds.isEmpty() ? ChartKind::Column : kinds.last().kind, false};
    kinds.resize(series.size());
    const Chart like = chart;
    chart = chartFromSeries(series, kinds, theme, &like);
    return true;
}

} // namespace jp::office::sheet
