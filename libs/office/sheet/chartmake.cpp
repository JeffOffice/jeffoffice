#include "sheet/chartmake.h"

#include <QCoreApplication>
#include <QRegularExpression>

#include <algorithm>

namespace jp::office::sheet {

using jp::office::Chart;
using jp::office::ChartAxis;
using jp::office::ChartGrouping;
using jp::office::ChartMarker;
using jp::office::ChartPlot;
using jp::office::ChartSeries;
using jp::office::ChartType;

namespace {

enum class Kind : quint8 { Empty, Number, Date, Text };

Kind kindAt(const ChartCells &cells, int sheet, quint32 row, quint32 col)
{
    const jp::calc::Value v = cells.value(sheet, row, col);
    if (v.isEmpty()) return Kind::Empty;
    if (v.isNumber()) {
        // a number shown with date marks is a date: its text is not its digits
        const QString t = cells.text(sheet, row, col);
        if (t.contains(QLatin1Char('/')) || t.contains(QLatin1Char(':')) || (t.contains(QLatin1Char('-')) && !t.startsWith(QLatin1Char('-')) && !t.endsWith(QLatin1Char('-')))) return Kind::Date;
        for (const QChar ch : t)
            if (ch.isLetter() && ch != QLatin1Char('E') && ch != QLatin1Char('e')) return Kind::Date;   // (a month's name)
        return Kind::Number;
    }
    if (v.isText() && v.text().isEmpty()) return Kind::Empty;
    return v.isBool() || v.isError() ? Kind::Number : Kind::Text;
}

CellRange rangeOf(quint32 r1, quint32 c1, quint32 r2, quint32 c2)
{
    CellRange r;
    r.row1 = r1;
    r.col1 = c1;
    r.row2 = r2;
    r.col2 = c2;
    r.valid = true;
    return r;
}

bool rowHasContent(const ChartCells &cells, int sheet, quint32 row, quint32 c1, quint32 c2, qint64 *budget)
{
    for (quint32 c = c1; c <= c2; ++c) {
        if (--*budget < 0) return false;
        if (!cells.value(sheet, row, c).isEmpty()) return true;
    }
    return false;
}
bool columnHasContent(const ChartCells &cells, int sheet, quint32 col, quint32 r1, quint32 r2, qint64 *budget)
{
    for (quint32 r = r1; r <= r2; ++r) {
        if (--*budget < 0) return false;
        if (!cells.value(sheet, r, col).isEmpty()) return true;
    }
    return false;
}

// The block of cells with something in them around the cell, as far as a budget of cells looked at allows.
CellRange regionAround(const ChartCells &cells, int sheet, quint32 row, quint32 col, quint32 rows, quint32 cols)
{
    quint32 r1 = row, r2 = row, c1 = col, c2 = col;
    qint64 budget = 600000;
    bool grew = true;
    while (grew && budget > 0) {
        grew = false;
        if (r1 > 0 && rowHasContent(cells, sheet, r1 - 1, c1 > 0 ? c1 - 1 : c1, std::min(c2 + 1, cols ? cols - 1 : c2), &budget)) {
            --r1;
            grew = true;
        }
        if (r2 + 1 < rows && rowHasContent(cells, sheet, r2 + 1, c1 > 0 ? c1 - 1 : c1, std::min(c2 + 1, cols ? cols - 1 : c2), &budget)) {
            ++r2;
            grew = true;
        }
        if (c1 > 0 && columnHasContent(cells, sheet, c1 - 1, r1 > 0 ? r1 - 1 : r1, std::min(r2 + 1, rows ? rows - 1 : r2), &budget)) {
            --c1;
            grew = true;
        }
        if (c2 + 1 < cols && columnHasContent(cells, sheet, c2 + 1, r1 > 0 ? r1 - 1 : r1, std::min(r2 + 1, rows ? rows - 1 : r2), &budget)) {
            ++c2;
            grew = true;
        }
    }
    return rangeOf(r1, c1, r2, c2);
}

bool plainOneCell(const CellRange &r) { return r.row1 == r.row2 && r.col1 == r.col2; }

} // namespace

QString chartRef(const QString &sheetName, const CellRange &range)
{
    static const QRegularExpression plain(QStringLiteral("^[A-Za-z_\\\\][A-Za-z0-9_.]*$")), cellLike(QStringLiteral("^(?:[A-Za-z]{1,3}[0-9]+|[Rr][0-9]*[Cc][0-9]*|[Rr]|[Cc])$"));
    QString name = sheetName;
    if (!plain.match(name).hasMatch() || cellLike.match(name).hasMatch()) name = QLatin1Char('\'') + name.replace(QLatin1Char('\''), QStringLiteral("''")) + QLatin1Char('\'');
    auto cell = [](quint32 r, quint32 c) { return QStringLiteral("$%1$%2").arg(columnName(c)).arg(r + 1); };
    return name + QLatin1Char('!') + (plainOneCell(range) ? cell(range.row1, range.col1) : cell(range.row1, range.col1) + QLatin1Char(':') + cell(range.row2, range.col2));
}

ChartPlan planChartFrom(const QString &sheetName, const CellRange &block, bool headerRow, bool headerColumn, bool inRows, ChartKind kind)
{
    ChartPlan plan;
    plan.seriesInRows = inRows;
    if (!block.valid) return plan;
    const bool scatter = kind == ChartKind::Scatter || kind == ChartKind::ScatterLines;
    const quint32 r1 = block.row1 + (headerRow ? 1 : 0), c1 = block.col1 + (headerColumn ? 1 : 0);
    if (r1 > block.row2 || c1 > block.col2) return plan;
    plan.headerRow = headerRow;
    plan.headerColumn = headerColumn;
    quint32 r2 = block.row2, c2 = block.col2;
    // the points of a series: the rows (series in columns) or the columns (series in rows) of the data, at most what a chart reads
    if (!inRows) {
        if (qint64(r2) - r1 + 1 > kMaxChartSeriesCells) {
            r2 = r1 + kMaxChartSeriesCells - 1;
            plan.truncated = true;
        }
        plan.points = int(r2 - r1 + 1);
    } else {
        if (qint64(c2) - c1 + 1 > kMaxChartSeriesCells) {
            c2 = c1 + kMaxChartSeriesCells - 1;
            plan.truncated = true;
        }
        plan.points = int(c2 - c1 + 1);
    }
    int count = inRows ? int(r2 - r1 + 1) : int(c2 - c1 + 1);
    int most = std::min(kMaxMadeSeries, std::max(1, kMaxChartCells / std::max(1, plan.points)));
    if (kind == ChartKind::Pie || kind == ChartKind::Doughnut) most = 1;
    if (count > most) {
        count = most;
        if (kind != ChartKind::Pie && kind != ChartKind::Doughnut) plan.truncated = true;
        if (inRows) r2 = r1 + quint32(count) - 1;
        else c2 = c1 + quint32(count) - 1;
    }
    plan.data = rangeOf(r1, c1, r2, c2);
    for (int i = 0; i < count; ++i) {
        ChartSeriesRefs s;
        if (!inRows) {
            const quint32 c = c1 + quint32(i);
            if (headerRow) s.nameRef = chartRef(sheetName, rangeOf(block.row1, c, block.row1, c));
            s.valueRef = chartRef(sheetName, rangeOf(r1, c, r2, c));
            if (headerColumn) (scatter ? s.xRef : s.categoryRef) = chartRef(sheetName, rangeOf(r1, block.col1, r2, block.col1));
        } else {
            const quint32 r = r1 + quint32(i);
            if (headerColumn) s.nameRef = chartRef(sheetName, rangeOf(r, block.col1, r, block.col1));
            s.valueRef = chartRef(sheetName, rangeOf(r, c1, r, c2));
            if (headerRow) (scatter ? s.xRef : s.categoryRef) = chartRef(sheetName, rangeOf(block.row1, c1, block.row1, c2));
        }
        plan.series.push_back(s);
    }
    return plan;
}

ChartPlan planChart(const ChartCells &cells, int sheet, const QString &sheetName, const CellRange &selection, ChartKind kind, int inRowsAsked)
{
    ChartPlan none;
    if (!selection.valid) return none;
    quint32 rows = 0, cols = 0;
    cells.extent(sheet, &rows, &cols);
    if (rows == 0 || cols == 0) return none;
    CellRange r = selection;
    r.row2 = std::min(r.row2, rows - 1);
    r.col2 = std::min(r.col2, cols - 1);
    if (r.row1 > r.row2 || r.col1 > r.col2) return none;
    bool truncated = false;
    if (plainOneCell(r)) {
        r = regionAround(cells, sheet, r.row1, r.col1, rows, cols);
    } else if (r.cellCount() > 4000000) {
        // a selection too large to look through cell by cell: its first part
        r.row2 = std::min<quint32>(r.row2, r.row1 + 20000);
        truncated = true;
    }
    // leave out the empty rows and columns at the far end (a whole column goes as far as the cells do), and at the near end
    qint64 budget = 2000000;
    while (r.row2 > r.row1 && !rowHasContent(cells, sheet, r.row2, r.col1, r.col2, &budget) && budget > 0) --r.row2;
    while (r.col2 > r.col1 && !columnHasContent(cells, sheet, r.col2, r.row1, r.row2, &budget) && budget > 0) --r.col2;
    while (r.row1 < r.row2 && !rowHasContent(cells, sheet, r.row1, r.col1, r.col2, &budget) && budget > 0) ++r.row1;
    while (r.col1 < r.col2 && !columnHasContent(cells, sheet, r.col1, r.row1, r.row2, &budget) && budget > 0) ++r.col1;
    const quint32 n = r.row2 - r.row1 + 1, m = r.col2 - r.col1 + 1;
    const bool scatter = kind == ChartKind::Scatter || kind == ChartKind::ScatterLines;

    // what the first row and column look like (only the cells that decide are looked at)
    auto kindOfCell = [&](quint32 row, quint32 col) { return kindAt(cells, sheet, row, col); };
    const bool topLeftEmpty = kindOfCell(r.row1, r.col1) == Kind::Empty;
    bool headerRow = false, headerColumn = false;
    if (n >= 2 && m >= 2 && topLeftEmpty) {
        headerRow = headerColumn = true;
    } else {
        if (n >= 2) {
            // the first row is the names when it holds words only and the rows below it have numbers
            quint32 words = 0, numbers = 0;
            const quint32 from = r.col1;
            for (quint32 c = from; c <= r.col2 && c < from + 512; ++c) {
                const Kind k = kindOfCell(r.row1, c);
                if (k == Kind::Text) ++words;
                else if (k == Kind::Number || k == Kind::Date) ++numbers;
            }
            bool below = false;
            for (quint32 c = r.col1; c <= r.col2 && c < r.col1 + 512 && !below; ++c)
                for (quint32 row = r.row1 + 1; row <= r.row2 && row < r.row1 + 64; ++row)
                    if (kindOfCell(row, c) == Kind::Number) {
                        below = true;
                        break;
                    }
            headerRow = words > 0 && numbers == 0 && below;
        }
        if (m >= 2 || scatter) {
            // the first column is the categories when it holds words or dates only
            quint32 words = 0, dates = 0, numbers = 0;
            for (quint32 row = r.row1 + (headerRow ? 1 : 0); row <= r.row2 && row < r.row1 + 4096; ++row) {
                const Kind k = kindOfCell(row, r.col1);
                if (k == Kind::Text) ++words;
                else if (k == Kind::Date) ++dates;
                else if (k == Kind::Number) ++numbers;
            }
            headerColumn = words + dates > 0 && numbers == 0;
            if (scatter && m >= 2) headerColumn = true;
        }
        // one row of words and numbers ("Total 4 5 6"): the words are the series' name
        if (n == 1 && m >= 2) headerRow = false;
    }
    if (n == 1 && m == 1) headerRow = headerColumn = false;
    // which way the series run
    const quint32 dataRows = n - (headerRow ? 1 : 0), dataCols = m - (headerColumn ? 1 : 0);
    bool inRows = inRowsAsked >= 0 ? inRowsAsked == 1 : dataCols > dataRows;
    if (scatter) inRows = inRowsAsked == 1;
    ChartPlan plan = planChartFrom(sheetName, r, headerRow, headerColumn, inRows, kind);
    plan.truncated = plan.truncated || truncated;
    return plan;
}

namespace {

ChartAxis axisOf(ChartAxis::Kind kind, qint64 id, qint64 cross, const QString &position)
{
    ChartAxis a;
    a.kind = kind;
    a.id = id;
    a.crossId = cross;
    a.position = position;
    return a;
}

}

Chart makeChart(ChartKind kind, const ChartPlan &plan, const jp::office::Theme &theme, int slices)
{
    Chart c = jp::office::blankChart(theme);
    c.background = QColor(Qt::white);
    c.border.set = true;
    c.border.color = QColor(0xD9, 0xD9, 0xD9);
    c.border.width = 0.75;
    c.style = 2;
    const int count = int(plan.series.size());
    auto colorOf = [&](int i) { return c.palette.isEmpty() ? QColor(0x44, 0x72, 0xC4) : c.palette.value(i % c.palette.size()); };
    auto seriesOf = [&](int i) {
        ChartSeries s;
        s.name = QCoreApplication::translate("Chart", "Series%1").arg(i + 1);
        s.nameRef = plan.series[i].nameRef;
        s.categoryRef = plan.series[i].categoryRef;
        s.valueRef = plan.series[i].valueRef;
        s.xRef = plan.series[i].xRef;
        s.color = colorOf(i);
        s.colorFromFile = true;
        return s;
    };
    const qint64 catId = 1001, valId = 1002, cat2Id = 1003, val2Id = 1004;
    ChartPlot p;
    p.axisIds[0] = catId;
    p.axisIds[1] = valId;
    bool horizontal = false, axes = true, combo = false;
    switch (kind) {
    case ChartKind::Column: p.type = ChartType::Bar; break;
    case ChartKind::ColumnStacked: p.type = ChartType::Bar; p.grouping = ChartGrouping::Stacked; p.overlap = 100; break;
    case ChartKind::ColumnPercent: p.type = ChartType::Bar; p.grouping = ChartGrouping::PercentStacked; p.overlap = 100; break;
    case ChartKind::Bar: p.type = ChartType::Bar; horizontal = true; break;
    case ChartKind::BarStacked: p.type = ChartType::Bar; p.grouping = ChartGrouping::Stacked; p.overlap = 100; horizontal = true; break;
    case ChartKind::BarPercent: p.type = ChartType::Bar; p.grouping = ChartGrouping::PercentStacked; p.overlap = 100; horizontal = true; break;
    case ChartKind::Line: p.type = ChartType::Line; p.grouping = ChartGrouping::Standard; p.markers = false; break;
    case ChartKind::LineMarkers: p.type = ChartType::Line; p.grouping = ChartGrouping::Standard; p.markers = true; break;
    case ChartKind::LineStacked: p.type = ChartType::Line; p.grouping = ChartGrouping::Stacked; p.markers = false; break;
    case ChartKind::Area: p.type = ChartType::Area; p.grouping = ChartGrouping::Standard; break;
    case ChartKind::AreaStacked: p.type = ChartType::Area; p.grouping = ChartGrouping::Stacked; break;
    case ChartKind::Pie: p.type = ChartType::Pie; p.grouping = ChartGrouping::Standard; p.varyColors = true; axes = false; break;
    case ChartKind::Doughnut: p.type = ChartType::Doughnut; p.grouping = ChartGrouping::Standard; p.varyColors = true; axes = false; break;
    case ChartKind::Scatter: p.type = ChartType::Scatter; p.grouping = ChartGrouping::Standard; p.lines = false; break;
    case ChartKind::ScatterLines: p.type = ChartType::Scatter; p.grouping = ChartGrouping::Standard; p.lines = true; break;
    case ChartKind::Combo: p.type = ChartType::Bar; combo = count >= 2; break;
    }
    p.horizontal = horizontal;
    if (!axes) p.axisIds[0] = p.axisIds[1] = 0;
    ChartPlot lines;
    const int barSeries = combo ? count - 1 : count;
    for (int i = 0; i < barSeries; ++i) {
        ChartSeries s = seriesOf(i);
        if (p.type == ChartType::Line && !p.markers) s.marker.symbol = ChartMarker::None;
        if (p.type == ChartType::Pie || p.type == ChartType::Doughnut) {
            // each slice its own color
            for (int k = 0; k < std::min(256, slices); ++k) s.pointColors.insert(k, colorOf(k));
        }
        p.series << s;
    }
    c.plots << p;
    if (combo) {
        lines.type = ChartType::Line;
        lines.grouping = ChartGrouping::Standard;
        lines.markers = true;
        lines.axisIds[0] = cat2Id;
        lines.axisIds[1] = val2Id;
        lines.series << seriesOf(count - 1);
        c.plots << lines;
    }
    if (axes) {
        const bool scatter = p.type == ChartType::Scatter;
        ChartAxis cat = axisOf(scatter ? ChartAxis::Value : ChartAxis::Category, catId, valId, horizontal ? QStringLiteral("l") : QStringLiteral("b"));
        ChartAxis val = axisOf(ChartAxis::Value, valId, catId, horizontal ? QStringLiteral("b") : QStringLiteral("l"));
        val.majorGridlines = true;
        if (scatter) cat.between = false;
        if (p.grouping == ChartGrouping::PercentStacked) {
            val.format = QStringLiteral("0%");
            val.formatLinked = false;
        }
        c.axes << cat << val;
        if (combo) {
            ChartAxis cat2 = axisOf(ChartAxis::Category, cat2Id, val2Id, QStringLiteral("b"));
            cat2.deleted = true;
            ChartAxis val2 = axisOf(ChartAxis::Value, val2Id, cat2Id, QStringLiteral("r"));
            val2.crosses = QStringLiteral("max");
            c.axes << cat2 << val2;
        }
    }
    if (count == 1 && p.type != ChartType::Scatter) {
        c.title.present = true;
        c.title.automatic = true;
        c.title.text = QCoreApplication::translate("Chart", "Chart Title");
    }
    c.legend.shown = count > 1 || p.type == ChartType::Pie || p.type == ChartType::Doughnut;
    c.legend.position = (p.type == ChartType::Pie || p.type == ChartType::Doughnut) ? QStringLiteral("r") : QStringLiteral("b");
    return c;
}

ChartKind kindOfChart(const Chart &chart)
{
    if (chart.plots.isEmpty()) return ChartKind::Column;
    if (chart.plots.size() > 1) return ChartKind::Combo;
    const ChartPlot &p = chart.plots.first();
    const bool stacked = p.grouping == ChartGrouping::Stacked, percent = p.grouping == ChartGrouping::PercentStacked;
    switch (p.type) {
    case ChartType::Bar:
    case ChartType::Other:
        if (p.horizontal) return percent ? ChartKind::BarPercent : stacked ? ChartKind::BarStacked : ChartKind::Bar;
        return percent ? ChartKind::ColumnPercent : stacked ? ChartKind::ColumnStacked : ChartKind::Column;
    case ChartType::Line: return stacked ? ChartKind::LineStacked : p.markers ? ChartKind::LineMarkers : ChartKind::Line;
    case ChartType::Area: return stacked || percent ? ChartKind::AreaStacked : ChartKind::Area;
    case ChartType::Pie: return ChartKind::Pie;
    case ChartType::Doughnut: return ChartKind::Doughnut;
    case ChartType::Scatter: return p.lines ? ChartKind::ScatterLines : ChartKind::Scatter;
    }
    return ChartKind::Column;
}

} // namespace jp::office::sheet
