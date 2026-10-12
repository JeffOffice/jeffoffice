// Charts of an open document presentation (odpimport_p.h). A chart is an
// embedded object: a part of its own with the numbers in a table and the
// plot's settings in styles. The picture the file keeps of it is the
// producer's own metafile, which can't be shown here, so the chart is drawn
// from its numbers by the shared chart drawing (office/chart.h), as a picture.

#include "io/odpimport_p.h"

#include "office/chart.h"

#include <QRegularExpression>
#include <cmath>
#include <limits>

namespace jp::deck::odp {

namespace {

constexpr int kMaxColumns = 256, kMaxRows = 10000, kMaxSeries = 64;

// The cells of the chart's own table: what each says as text, and as a number (NaN for none).
struct LocalTable {
    QVector<QStringList> text;
    QVector<QVector<double>> number;
    int rows() const { return int(text.size()); }
    int cols() const { return text.isEmpty() ? 0 : int(text[0].size()); }
    QString textAt(int r, int c) const { return r >= 0 && r < text.size() && c >= 0 && c < text[r].size() ? text[r][c] : QString(); }
    double numberAt(int r, int c) const { return r >= 0 && r < number.size() && c >= 0 && c < number[r].size() ? number[r][c] : std::numeric_limits<double>::quiet_NaN(); }
};

LocalTable readTable(const QDomElement &table)
{
    LocalTable t;
    struct Walk {
        static void rows(const QDomElement &parent, LocalTable *t, int depth)
        {
            if (depth > 4) return;
            for (QDomElement c = parent.firstChildElement(); !c.isNull() && t->text.size() < kMaxRows; c = c.nextSiblingElement()) {
                const QString tag = tagOf(c);
                if (tag == QLatin1String("table:table-header-rows") || tag == QLatin1String("table:table-rows") || tag == QLatin1String("table:table-row-group")) {
                    rows(c, t, depth + 1);
                } else if (tag == QLatin1String("table:table-row")) {
                    QStringList texts;
                    QVector<double> numbers;
                    for (QDomElement cell = c.firstChildElement(); !cell.isNull() && texts.size() < kMaxColumns; cell = cell.nextSiblingElement()) {
                        const QString ct = tagOf(cell);
                        if (ct != QLatin1String("table:table-cell") && ct != QLatin1String("table:covered-table-cell")) continue;
                        const Attrs a(cell);
                        QString text;
                        for (const QDomElement &p : kids(cell, "text:p")) text += (text.isEmpty() ? QString() : QStringLiteral("\n")) + p.text();
                        double v = std::numeric_limits<double>::quiet_NaN();
                        if (a.get("office:value-type") == QLatin1String("float") || a.get("office:value-type") == QLatin1String("percentage") || a.get("office:value-type") == QLatin1String("currency"))
                            parseNumber(a.get("office:value"), &v);
                        const int repeat = int(std::clamp(lengthOr(a.get("table:number-columns-repeated", QStringLiteral("1")), 1), 1.0, double(kMaxColumns)));
                        for (int i = 0; i < repeat && texts.size() < kMaxColumns; ++i) {
                            texts << text;
                            numbers << v;
                        }
                    }
                    const int repeat = int(std::clamp(lengthOr(at(c, "table:number-rows-repeated", QStringLiteral("1")), 1), 1.0, 64.0));
                    for (int i = 0; i < repeat && t->text.size() < kMaxRows; ++i) {
                        t->text.append(texts);
                        t->number.append(numbers);
                    }
                }
            }
        }
    };
    Walk::rows(table, &t, 0);
    return t;
}

// The cells a range address names, "local-table.$B$2:.$B$64": the first and last column and row, counted from 0.
bool cellRange(const QString &address, int *c0, int *r0, int *c1, int *r1)
{
    static const QRegularExpression cell(QStringLiteral("\\$?([A-Z]{1,3})\\$?([0-9]{1,6})"));
    auto it = cell.globalMatch(address);
    if (!it.hasNext()) return false;
    auto column = [](const QString &letters) {
        int v = 0;
        for (const QChar ch : letters) v = v * 26 + (ch.unicode() - 'A' + 1);
        return v - 1;
    };
    const QRegularExpressionMatch first = it.next();
    *c0 = *c1 = column(first.captured(1));
    *r0 = *r1 = first.captured(2).toInt() - 1;
    if (it.hasNext()) {
        const QRegularExpressionMatch last = it.next();
        *c1 = column(last.captured(1));
        *r1 = last.captured(2).toInt() - 1;
    }
    return *c0 >= 0 && *r0 >= 0 && *c1 >= *c0 && *r1 >= *r0;
}

} // namespace

QString Reader::chartImage(const QDomElement &object, const QSizeF &size)
{
    const QString part = office::resolvePartName(QStringLiteral("/"), at(object, "xlink:href"));
    if (part.isEmpty() || size.width() < 1 || size.height() < 1) return QString();
    QDomDocument dom;
    QString why;
    if (!parse(part + QStringLiteral("/content.xml"), &dom, &why)) return QString();
    const QDomElement chartEl = kid(kid(kid(dom.documentElement(), "office:body"), "office:chart"), "chart:chart");
    if (chartEl.isNull()) return QString();

    StyleSheet styles;
    styles.read(kid(dom.documentElement(), "office:automatic-styles"), true);
    const Attrs ca(chartEl);
    const QString cls = ca.get("chart:class");
    const QDomElement plotArea = kid(chartEl, "chart:plot-area");
    const Bag plotBag = styles.resolve(QStringLiteral("chart"), at(plotArea, "chart:style-name"));
    const LocalTable table = readTable(kid(chartEl, "table:table"));
    if (table.rows() == 0 || table.cols() == 0) return QString();

    office::Chart chart;
    chart.palette = {QColor(0x00, 0x45, 0x86), QColor(0xFF, 0x42, 0x0E), QColor(0xFF, 0xD3, 0x20), QColor(0x57, 0x9D, 0x1C), QColor(0x7E, 0x00, 0x21),
                     QColor(0x83, 0xCA, 0xFF), QColor(0x31, 0x40, 0x04), QColor(0xAE, 0xCF, 0x00), QColor(0x4B, 0x1F, 0x6F), QColor(0xFF, 0x95, 0x0E)};
    chart.textColor = QColor(Qt::black);
    chart.background = QColor();
    chart.backgroundColor = QColor(Qt::white);

    // The kind of plot, and the way its bars are grouped.
    auto kindOf = [](const QString &c) {
        if (c == QLatin1String("chart:bar")) return office::ChartType::Bar;
        if (c == QLatin1String("chart:line")) return office::ChartType::Line;
        if (c == QLatin1String("chart:area")) return office::ChartType::Area;
        if (c == QLatin1String("chart:circle")) return office::ChartType::Pie;
        if (c == QLatin1String("chart:ring")) return office::ChartType::Doughnut;
        if (c == QLatin1String("chart:scatter")) return office::ChartType::Scatter;
        return office::ChartType::Other;
    };
    const bool stacked = plotBag.get(Graphic, "chart:stacked") == QLatin1String("true"), percent = plotBag.get(Graphic, "chart:percentage") == QLatin1String("true");
    const bool horizontal = plotBag.get(Graphic, "chart:vertical") == QLatin1String("true");

    // The axes: the categories and the values, each with its title and gridlines.
    office::ChartAxis categoryAxis, valueAxis;
    categoryAxis.kind = office::ChartAxis::Category;
    categoryAxis.id = 1;
    categoryAxis.crossId = 2;
    categoryAxis.position = horizontal ? QStringLiteral("l") : QStringLiteral("b");
    valueAxis.kind = office::ChartAxis::Value;
    valueAxis.id = 2;
    valueAxis.crossId = 1;
    valueAxis.position = horizontal ? QStringLiteral("b") : QStringLiteral("l");
    QString categoriesAddress;
    for (const QDomElement &axis : kids(plotArea, "chart:axis")) {
        const Attrs a(axis);
        const Bag b = styles.resolve(QStringLiteral("chart"), a.get("chart:style-name"));
        office::ChartAxis &target = a.get("chart:dimension") == QLatin1String("y") ? valueAxis : categoryAxis;
        target.majorGridlines = !kids(axis, "chart:grid").isEmpty() && std::any_of(kids(axis, "chart:grid").cbegin(), kids(axis, "chart:grid").cend(), [](const QDomElement &g) { return at(g, "chart:class") == QLatin1String("major"); });
        if (const QColor line = parseColor(b.get(Graphic, "svg:stroke-color")); line.isValid()) {
            target.line.set = true;
            target.line.color = line;
            target.gridline.set = true;
            target.gridline.color = QColor(0xB3, 0xB3, 0xB3);
        }
        if (b.get(Graphic, "chart:display-label") == QLatin1String("false") || b.get(Graphic, "chart:visible") == QLatin1String("false")) target.labelPosition = QStringLiteral("none");
        double v = 0;
        if (parseNumber(b.get(Graphic, "chart:minimum"), &v)) { target.autoMin = false; target.min = v; }
        if (parseNumber(b.get(Graphic, "chart:maximum"), &v)) { target.autoMax = false; target.max = v; }
        if (parseNumber(b.get(Graphic, "chart:interval-major"), &v) && v > 0) target.majorUnit = v;
        if (b.get(Graphic, "chart:reverse-direction") == QLatin1String("true")) target.reversed = true;
        if (double size = 0; parseLength(b.get(Text, "fo:font-size"), &size) && size > 0) target.font.size = size;
        if (const QString family = b.get(Text, "fo:font-family"); !family.isEmpty() && chart.fontFamily.isEmpty()) chart.fontFamily = family.section(QLatin1Char(','), 0, 0).remove(QLatin1Char('\'')).remove(QLatin1Char('"')).trimmed();
        if (const QDomElement title = kid(axis, "chart:title"); !title.isNull()) {
            target.title.present = true;
            for (const QDomElement &p : kids(title, "text:p")) target.title.text += (target.title.text.isEmpty() ? QString() : QStringLiteral("\n")) + p.text();
        }
        if (const QDomElement cat = kid(axis, "chart:categories"); !cat.isNull() && a.get("chart:dimension") != QLatin1String("y")) categoriesAddress = at(cat, "table:cell-range-address");
    }
    // The categories: the cells the x axis names, else the first column below its heading.
    QStringList categories;
    {
        int c0, r0, c1, r1;
        if (cellRange(categoriesAddress, &c0, &r0, &c1, &r1))
            for (int r = r0; r <= r1; ++r) categories << table.textAt(r, c0);
    }

    // The series, grouped into one plot for each kind of drawing the file asks for.
    QVector<office::ChartPlot> plots;
    int seriesCount = 0;
    for (const QDomElement &s : kids(plotArea, "chart:series")) {
        if (++seriesCount > kMaxSeries) break;
        const Attrs a(s);
        int c0, r0, c1, r1;
        if (!cellRange(a.get("chart:values-cell-range-address"), &c0, &r0, &c1, &r1)) continue;
        office::ChartSeries series;
        series.categories = categories;
        for (int r = r0; r <= r1 && r - r0 < kMaxRows; ++r) series.values.append(table.numberAt(r, c0));
        if (int lc, lr, lc1, lr1; cellRange(a.get("chart:label-cell-address"), &lc, &lr, &lc1, &lr1)) series.name = table.textAt(lr, lc);
        if (series.name.isEmpty()) series.name = QStringLiteral("Series %1").arg(seriesCount);
        if (series.categories.size() < series.values.size()) {
            // No categories named: the first column of the table, or numbers.
            series.categories.clear();
            for (int r = r0; r <= r1; ++r) series.categories << (table.cols() > 1 && c0 > 0 ? table.textAt(r, 0) : QString::number(r - r0 + 1));
        }
        const Bag b = styles.resolve(QStringLiteral("chart"), a.get("chart:style-name"));
        const QString seriesClass = a.get("chart:class", cls);
        const office::ChartType type = kindOf(seriesClass);
        QColor color = parseColor(b.get(Graphic, type == office::ChartType::Line || type == office::ChartType::Scatter ? "svg:stroke-color" : "draw:fill-color"));
        if (!color.isValid()) color = parseColor(b.get(Graphic, "draw:fill-color"));
        if (color.isValid()) {
            series.color = color;
            series.colorFromFile = true;
        }
        if (b.get(Graphic, "draw:fill") == QLatin1String("none") && type != office::ChartType::Line) series.noFill = true;
        if (b.get(Graphic, "draw:stroke") == QLatin1String("none")) {
            series.outline.set = true;
            series.outline.none = true;
        }
        if (type == office::ChartType::Line && !b.has(Graphic, "svg:stroke-width")) series.outline.width = 2.25;
        if (double w = 0; parseLength(b.get(Graphic, "svg:stroke-width"), &w) && w > 0) {
            series.outline.set = true;
            series.outline.width = std::max(w, 0.75);
            if (!series.outline.color.isValid()) series.outline.color = parseColor(b.get(Graphic, "svg:stroke-color"));
        }
        if (b.get(Graphic, "chart:symbol-type") == QLatin1String("none")) series.marker.symbol = office::ChartMarker::None;
        if (b.get(Graphic, "chart:data-label-number") == QLatin1String("value") || b.get(Graphic, "chart:data-label-number") == QLatin1String("percentage")) {
            series.labels.value = b.get(Graphic, "chart:data-label-number") == QLatin1String("value");
            series.labels.percent = !series.labels.value;
        }
        // Points of another color (the slices of a pie).
        int point = 0;
        for (const QDomElement &dp : kids(s, "chart:data-point")) {
            const Attrs d(dp);
            const Bag pb = styles.resolve(QStringLiteral("chart"), d.get("chart:style-name"));
            const int repeat = int(std::clamp(lengthOr(d.get("chart:repeated", QStringLiteral("1")), 1), 1.0, 1000.0));
            const QColor pc = parseColor(pb.get(Graphic, "draw:fill-color"));
            for (int i = 0; i < repeat; ++i, ++point)
                if (pc.isValid() && point < kMaxRows) series.pointColors.insert(point, pc);
        }
        // The plot it goes in.
        int index = -1;
        for (int i = 0; i < plots.size(); ++i)
            if (plots[i].element == seriesClass) index = i;
        if (index < 0) {
            office::ChartPlot plot;
            plot.type = type;
            plot.element = seriesClass;
            plot.grouping = percent ? office::ChartGrouping::PercentStacked : stacked ? office::ChartGrouping::Stacked : office::ChartGrouping::Clustered;
            if (type == office::ChartType::Area && !stacked && !percent) plot.grouping = office::ChartGrouping::Standard;
            plot.horizontal = horizontal && type == office::ChartType::Bar;
            plot.axisIds[0] = 1;
            plot.axisIds[1] = 2;
            plot.varyColors = type == office::ChartType::Pie || type == office::ChartType::Doughnut;
            plot.lines = type != office::ChartType::Scatter || plotBag.get(Graphic, "chart:lines") != QLatin1String("false");
            plots.append(plot);
            index = int(plots.size()) - 1;
        }
        plots[index].series.append(series);
    }
    if (plots.isEmpty()) return QString();
    // A scatter's x values: the first series' column before its own, or the numbers 1, 2, 3.
    for (office::ChartPlot &plot : plots)
        if (plot.type == office::ChartType::Scatter)
            for (office::ChartSeries &s : plot.series)
                for (int i = 0; i < s.values.size(); ++i) s.xValues.append(i + 1);
    chart.plots = plots;
    const bool pieLike = plots[0].type == office::ChartType::Pie || plots[0].type == office::ChartType::Doughnut;
    if (!pieLike) chart.axes = {categoryAxis, valueAxis};
    if (plots[0].type == office::ChartType::Scatter && !pieLike) {
        chart.axes[0].kind = office::ChartAxis::Value;
        chart.axes[0].between = false;
    }

    // The title, the legend, and the areas behind the plot.
    if (const QDomElement title = kid(chartEl, "chart:title"); !title.isNull()) {
        chart.title.present = true;
        for (const QDomElement &p : kids(title, "text:p")) chart.title.text += (chart.title.text.isEmpty() ? QString() : QStringLiteral("\n")) + p.text();
    }
    if (const QDomElement legend = kid(chartEl, "chart:legend"); !legend.isNull()) {
        chart.legend.shown = true;
        const QString position = at(legend, "chart:legend-position", QStringLiteral("end"));
        chart.legend.position = position == QLatin1String("start") ? QStringLiteral("l") : position == QLatin1String("top") ? QStringLiteral("t") : position == QLatin1String("bottom") ? QStringLiteral("b") : QStringLiteral("r");
    }
    {
        const Bag area = styles.resolve(QStringLiteral("chart"), ca.get("chart:style-name"));
        if (area.get(Graphic, "draw:fill") == QLatin1String("solid")) chart.background = parseColor(area.get(Graphic, "draw:fill-color"));
        const Bag wall = styles.resolve(QStringLiteral("chart"), at(kid(plotArea, "chart:wall"), "chart:style-name"));
        if (wall.get(Graphic, "draw:fill") == QLatin1String("solid")) chart.plotFill = parseColor(wall.get(Graphic, "draw:fill-color"));
    }
    const QByteArray svg = office::chartSvg(chart, size);
    return svg.isEmpty() ? QString() : m_doc.addImage(svg, QStringLiteral("svg"));
}

} // namespace jp::deck::odp
