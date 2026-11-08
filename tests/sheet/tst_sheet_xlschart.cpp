// Tests of the charts the reader of the old binary workbook format finds: the records of a chart's substream (an embedded chart, under a drawing object of a worksheet, or the whole
// of a chart sheet) are read into the same chart model the XML form fills, and a workbook saved as the XML form holds them as real charts. Every workbook is built here from records.
#include "xlshelpers.h"

#include "office/chart.h"
#include "office/package.h"
#include "office/theme.h"
#include "sheet/drawing.h"
#include "sheet/oldsave.h"
#include "sheet/xlsxsave.h"
#include "io/zip.h"

#include <QXmlStreamReader>

using jp::office::Chart;
using jp::office::ChartGrouping;
using jp::office::ChartType;

namespace {

QByteArray chartBof() { return rec(0x0809, W().u16(0x0600).u16(0x0020).u16(1).u16(2).u32(0).u32(6)); }
QByteArray begin() { return rec(0x1033); }
QByteArray end() { return rec(0x1034); }
QByteArray zeros(int n) { return QByteArray(n, '\0'); }

// An AI record: the series' name (0), values (1), categories (2), or bubble sizes (3), as a literal (1) or a reference (2) by its tokens.
QByteArray ai(int id, int rt, const QByteArray &rgce = QByteArray()) { return rec(0x1051, W().u8(id).u8(rt).u16(0).u16(0).u16(rgce.size()).raw(rgce)); }
QByteArray refAi(int id, int row1, int col1, int row2, int col2) { return ai(id, 2, tArea3d(0, row1, col1, row2, col2, false, false)); }
QByteArray seriesText(const QString &s) { return rec(0x100D, W().u16(0).str8(s)); }

// A series: its name from a cell (or a text), its values and categories from areas of the sheet "Data", and the records of its blocks.
QByteArray series(int group, const QByteArray &name, int valCol, const QByteArray &extra = QByteArray(), bool categories = true, int points = 3)
{
    QByteArray s = rec(0x1003, W().u16(1).u16(1).u16(points).u16(points).u16(0).u16(0)) + begin();
    s += name;
    s += refAi(1, 1, valCol, points, valCol);
    s += categories ? refAi(2, 1, 0, points, 0) : ai(2, 1);
    s += ai(3, 1) + extra + rec(0x1045, W().u16(group)) + end();
    return s;
}
QByteArray named(int row, int col) { return refAi(0, row, col, row, col); }
QByteArray showValues() { return rec(0x1006, W().u16(0xFFFF).u16(0).u16(0).u16(0)) + begin() + rec(0x100C, W().u16(1)) + end(); }
QByteArray seriesColor(int rgb) { return rec(0x1006, W().u16(0xFFFF).u16(0).u16(0).u16(0)) + begin() + rec(0x100A, W().u32(rgb).u32(0xFFFFFF).u16(1).u16(0).u16(0).u16(0)) + end(); }

// A group of series drawn the same way: ChartFormat and the record of its kind.
QByteArray group(const QByteArray &kind, bool varied = false, int order = 0)
{
    return rec(0x1014, zeros(16) + W().u16(varied ? 1 : 0).u16(order).bytes()) + begin() + kind + rec(0x1022, zeros(8)) + end();
}
QByteArray bar(int grbit = 0, int overlap = 0, int gap = 150) { return rec(0x1017, W().u16(overlap).u16(gap).u16(grbit)); }
QByteArray lineKind(int grbit = 0) { return rec(0x1018, W().u16(grbit)); }
QByteArray pie(int start = 0, int donut = 0) { return rec(0x1019, W().u16(start).u16(donut).u16(0)); }
QByteArray areaKind(int grbit = 0) { return rec(0x101A, W().u16(grbit)); }
QByteArray scatterKind(int grbit = 0) { return rec(0x101B, W().u16(100).u16(1).u16(grbit)); }

QByteArray axis(int type, const QByteArray &inside = QByteArray()) { return rec(0x101D, W().u16(type).raw(zeros(16))) + begin() + inside + end(); }
QByteArray axisParent(int iax, const QByteArray &inside) { return rec(0x1041, W().u16(iax).raw(zeros(16))) + begin() + rec(0x104F, W().u16(2).u16(2).raw(zeros(16))) + inside + end(); }
QByteArray valueRange(double lo, double hi, double major, int grbit) { return rec(0x101F, W().f64(lo).f64(hi).f64(major).f64(0).f64(0).u16(grbit)); }
QByteArray textBlock(int link, const QString &text)
{
    QByteArray t = rec(0x1025, zeros(32)) + begin() + rec(0x104F, W().u16(2).u16(2).raw(zeros(16))) + rec(0x1026, W().u16(5)) + ai(0, 1);
    if (!text.isNull()) t += seriesText(text);
    return t + rec(0x1027, W().u16(link).u16(0).u16(0)) + end();
}
QByteArray legend(int type) { return rec(0x1015, W().raw(zeros(16)).u8(type).u8(1).u16(0x0F)) + begin() + rec(0x104F, W().u16(2).u16(2).raw(zeros(16))) + end(); }

// The records of a chart, from its first record to its last (without the start and the end of the substream).
QByteArray chartBody(const QByteArray &series, const QByteArray &parents, const QByteArray &texts = QByteArray(), const QByteArray &legendRecord = QByteArray())
{
    return rec(0x1001, W().u16(0)) + rec(0x1002, zeros(16)) + begin() + series + rec(0x1044, W().u16(0).u16(0)) + parents + texts + legendRecord + end();
}
QByteArray columnParents(const QByteArray &g, const QByteArray &catAxis = QByteArray(), const QByteArray &valAxis = QByteArray())
{
    return axisParent(0, axis(0, catAxis) + axis(1, valAxis) + g);
}

// The drawing object a chart is under: a drawing record with the anchor (columns and rows, with the parts of cells in 1024ths and 256ths), and the object record of a chart.
QByteArray anchorRecord(int col1, int dx1, int row1, int dy1, int col2, int dx2, int row2, int dy2)
{
    const QByteArray anchor = W().u16(2).u16(col1).u16(dx1).u16(row1).u16(dy1).u16(col2).u16(dx2).u16(row2).u16(dy2);
    const QByteArray sp = QByteArray(W().u32(0x400).u32(0xA00)) ;   // (a shape atom: id and flags)
    QByteArray atoms = W().u16(0x0A).u16(0xF00A).u32(sp.size()).raw(sp) + W().u16(0x0000).u16(0xF010).u32(anchor.size()).raw(anchor);
    return rec(0x00EC, W().u16(0x000F).u16(0xF004).u32(atoms.size()).raw(atoms));
}
QByteArray objRecord(int type) { return rec(0x005D, W().u16(0x15).u16(0x12).u16(type).u16(1).u16(0x6011).u32(0).u32(0).u32(0).u16(0).u16(0)); }

// The sheet "Data": a header row, three categories, and two columns of numbers; the chart is under it. The list of sheets for the references is the one of the book.
Book bookWith(const QByteArray &chartRecords, int sheetType = 0, bool embedded = true, const QString &sheetName = QStringLiteral("Data"), const QByteArray &anchor = anchorRecord(3, 512, 1, 128, 9, 0, 16, 0),
              const QByteArray &columns = QByteArray())
{
    Book b;
    b.after(supbookInternal(1) + externSheet({{0, 0, 0}}));
    QByteArray cells = dimensions(4, 3) + columns + label(0, 1, QStringLiteral("Sales")) + label(0, 2, QStringLiteral("Costs"));
    for (int r = 1; r <= 3; ++r) cells += label(r, 0, QStringLiteral("c%1").arg(r)) + number(r, 1, r * 10) + number(r, 2, r * 3);
    if (embedded) cells += anchor + objRecord(5) + chartBof() + chartRecords + rec(0x000A);
    b.sheet(sheetName, cells);
    if (!embedded) b.sheet(QStringLiteral("Chart1"), chartRecords, 0, sheetType);
    return b;
}

bool readCharts(const Book &b, Workbook *wb, int sheet, QVector<Chart> *out, QString *why = nullptr, int *count = nullptr)
{
    if (!openBook(b, wb, why)) return false;
    Sheet s;
    if (!wb->readSheet(sheet, &s)) return false;
    if (count) *count = int(s.importedCharts.size());
    for (const auto &c : s.importedCharts) {
        Chart chart;
        QString error;
        if (!jp::office::readChart(c.part, jp::office::defaultTheme(), jp::office::ColorMap(), &chart, &error)) {
            if (why) *why = error;
            return false;
        }
        out->append(chart);
    }
    return true;
}

} // namespace

class SheetXlsChartTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The usual chart: clustered columns, two series from cells, a title, titles of both axes, a legend at the bottom, labels on the first series, and the place the drawing gives it.
    void anEmbeddedColumnChartIsRead()
    {
        const QByteArray s = series(0, named(0, 1), 1, showValues() + seriesColor(0x00FF00)) + series(0, named(0, 2), 2);
        const QByteArray body = chartBody(s, columnParents(group(bar())), textBlock(1, QStringLiteral("Results")) + textBlock(3, QStringLiteral("Quarter")) + textBlock(2, QStringLiteral("Money")), legend(0));
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        QVERIFY2(readCharts(bookWith(body), &wb, 0, &charts, &why), qPrintable(why));
        QCOMPARE(charts.size(), 1);
        const Chart &c = charts[0];
        QCOMPARE(c.plots.size(), 1);
        QVERIFY(c.plots[0].type == ChartType::Bar);
        QVERIFY(!c.plots[0].horizontal);
        QVERIFY(c.plots[0].grouping == ChartGrouping::Clustered);
        QCOMPARE(c.plots[0].series.size(), 2);
        QCOMPARE(c.plots[0].series[0].nameRef, QStringLiteral("Data!$B$1"));
        QCOMPARE(c.plots[0].series[0].valueRef, QStringLiteral("Data!$B$2:$B$4"));
        QCOMPARE(c.plots[0].series[0].categoryRef, QStringLiteral("Data!$A$2:$A$4"));
        QCOMPARE(c.plots[0].series[1].valueRef, QStringLiteral("Data!$C$2:$C$4"));
        QVERIFY(c.plots[0].series[0].labels.value);
        QVERIFY(!c.plots[0].series[1].labels.shown());
        QCOMPARE(c.plots[0].series[0].color, QColor(0x00, 0xFF, 0x00));
        QCOMPARE(c.title.text, QStringLiteral("Results"));
        QVERIFY(c.legend.shown);
        QCOMPARE(c.legend.position, QStringLiteral("b"));
        const jp::office::ChartAxis *cat = c.axis(c.plots[0].axisIds[0]), *val = c.axis(c.plots[0].axisIds[1]);
        QVERIFY(cat && val);
        QCOMPARE(cat->title.text, QStringLiteral("Quarter"));
        QCOMPARE(val->title.text, QStringLiteral("Money"));
        // the place: columns and rows from the drawing, the parts of cells in EMU
        Sheet sh;
        QVERIFY(wb.readSheet(0, &sh));
        QCOMPARE(sh.importedCharts.size(), 1);
        QCOMPARE(int(sh.importedCharts[0].col1), 3);
        QCOMPARE(int(sh.importedCharts[0].row1), 1);
        QCOMPARE(int(sh.importedCharts[0].col2), 9);
        QCOMPARE(int(sh.importedCharts[0].row2), 16);
        QCOMPARE(int(sh.importedCharts[0].dx1), 512);
        QCOMPARE(int(sh.importedCharts[0].dy1), 128);
    }

    // The kinds: columns, bars, stacked and whole-stacked ones, lines, areas, pies, doughnuts, and scatters.
    void theKindsOfChartAreTold_data()
    {
        QTest::addColumn<int>("kind");   // 0 column 1 bar 2 stacked 3 percent 4 line 5 stacked line 6 area 7 stacked area 8 pie 9 doughnut 10 scatter
        QTest::newRow("column") << 0;
        QTest::newRow("bar") << 1;
        QTest::newRow("stacked") << 2;
        QTest::newRow("percent") << 3;
        QTest::newRow("line") << 4;
        QTest::newRow("stacked line") << 5;
        QTest::newRow("area") << 6;
        QTest::newRow("stacked area") << 7;
        QTest::newRow("pie") << 8;
        QTest::newRow("doughnut") << 9;
        QTest::newRow("scatter") << 10;
    }
    void theKindsOfChartAreTold()
    {
        QFETCH(int, kind);
        QByteArray g;
        switch (kind) {
        case 0: g = group(bar()); break;
        case 1: g = group(bar(1)); break;
        case 2: g = group(bar(2, 100)); break;
        case 3: g = group(bar(6, 100)); break;
        case 4: g = group(lineKind()); break;
        case 5: g = group(lineKind(1)); break;
        case 6: g = group(areaKind()); break;
        case 7: g = group(areaKind(1)); break;
        case 8: g = group(pie(90), true); break;
        case 9: g = group(pie(0, 60), true); break;
        default: g = group(scatterKind()); break;
        }
        const bool pieLike = kind == 8 || kind == 9;
        const QByteArray parents = pieLike ? axisParent(0, g) : columnParents(g);
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        QVERIFY2(readCharts(bookWith(chartBody(series(0, named(0, 1), 1), parents)), &wb, 0, &charts, &why), qPrintable(why));
        QCOMPARE(charts.size(), 1);
        const jp::office::ChartPlot &p = charts[0].plots.value(0);
        QCOMPARE(p.series.size(), 1);
        switch (kind) {
        case 0: QVERIFY(p.type == ChartType::Bar && !p.horizontal && p.grouping == ChartGrouping::Clustered); break;
        case 1: QVERIFY(p.type == ChartType::Bar && p.horizontal); break;
        case 2: QVERIFY(p.type == ChartType::Bar && p.grouping == ChartGrouping::Stacked && p.overlap == 100); break;
        case 3: QVERIFY(p.type == ChartType::Bar && p.grouping == ChartGrouping::PercentStacked); break;
        case 4: QVERIFY(p.type == ChartType::Line && p.grouping == ChartGrouping::Standard); break;
        case 5: QVERIFY(p.type == ChartType::Line && p.grouping == ChartGrouping::Stacked); break;
        case 6: QVERIFY(p.type == ChartType::Area && p.grouping == ChartGrouping::Standard); break;
        case 7: QVERIFY(p.type == ChartType::Area && p.grouping == ChartGrouping::Stacked); break;
        case 8: QVERIFY(p.type == ChartType::Pie && p.firstSliceAngle == 90 && p.varyColors); break;
        case 9: QVERIFY(p.type == ChartType::Doughnut && p.holeSize == 60); break;
        default: QVERIFY(p.type == ChartType::Scatter); QCOMPARE(p.series[0].xRef, QStringLiteral("Data!$A$2:$A$4")); QCOMPARE(p.series[0].valueRef, QStringLiteral("Data!$B$2:$B$4")); break;
        }
    }

    // A chart sheet holds the records of a chart itself; it is read as a chart too.
    void aChartSheetIsRead()
    {
        const QByteArray body = chartBody(series(0, named(0, 1), 1), columnParents(group(lineKind())));
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        QVERIFY2(readCharts(bookWith(body, 2, false), &wb, 1, &charts, &why), qPrintable(why));
        QCOMPARE(charts.size(), 1);
        QVERIFY(charts[0].type() == ChartType::Line);
        QCOMPARE(charts[0].plots[0].series[0].valueRef, QStringLiteral("Data!$B$2:$B$4"));
        // a chart sheet's chart fills a page of its sheet
        Sheet s;
        QVERIFY(wb.readSheet(1, &s));
        QVERIFY(s.importedCharts[0].absolute);   // (no cells: it fills a page of its own)
    }

    // Values given in the chart itself (an array in the series' formula) are literal values.
    void literalValuesAreRead()
    {
        const QByteArray lit = tArray();
        const QByteArray values = rec(0x1051, W().u8(1).u8(2).u16(0).u16(0).u16(lit.size()).raw(lit).raw(arrayValues({{1.0, 2.5, 4.0}})));
        QByteArray s = rec(0x1003, W().u16(1).u16(1).u16(3).u16(3).u16(0).u16(0)) + begin() + ai(0, 1) + seriesText(QStringLiteral("Own")) + values + ai(2, 1) + ai(3, 1) + rec(0x1045, W().u16(0)) + end();
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        QVERIFY2(readCharts(bookWith(chartBody(s, columnParents(group(bar())))), &wb, 0, &charts, &why), qPrintable(why));
        QCOMPARE(charts.size(), 1);
        const jp::office::ChartSeries &cs = charts[0].plots[0].series[0];
        QCOMPARE(cs.name, QStringLiteral("Own"));
        QVERIFY(cs.valueRef.isEmpty());
        QCOMPARE(cs.values.size(), 3);
        QCOMPARE(cs.values[1], 2.5);
    }

    // The scale of the value axis and its gridlines.
    void theValueAxisScaleIsRead()
    {
        const QByteArray valAxis = valueRange(5, 50, 10, 0x00) + rec(0x1021, W().u16(1)) + rec(0x1007, zeros(10));
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        QVERIFY2(readCharts(bookWith(chartBody(series(0, named(0, 1), 1), columnParents(group(bar()), QByteArray(), valAxis))), &wb, 0, &charts, &why), qPrintable(why));
        QCOMPARE(charts.size(), 1);
        const jp::office::ChartAxis *val = charts[0].axis(charts[0].plots[0].axisIds[1]);
        QVERIFY(val);
        QVERIFY(!val->autoMin && !val->autoMax);
        QCOMPARE(val->min, 5.0);
        QCOMPARE(val->max, 50.0);
        QCOMPARE(val->majorUnit, 10.0);
        QVERIFY(val->majorGridlines);
    }

    // A chart of a kind the model does not draw (radar, bubbles) is left out, and said to be.
    void whatTheModelCannotHoldIsSkipped()
    {
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        int count = -1;
        const QByteArray radar = rec(0x103E, W().u16(0).u16(0));
        QVERIFY2(readCharts(bookWith(chartBody(series(0, named(0, 1), 1), columnParents(group(radar)))), &wb, 0, &charts, &why, &count), qPrintable(why));
        QCOMPARE(count, 0);
        QVERIFY(wb.skippedFeatures().contains(QStringLiteral("charts")));
        // bubbles are a scatter that says so
        Workbook wb2;
        QVector<Chart> none;
        QVERIFY2(readCharts(bookWith(chartBody(series(0, named(0, 1), 1), columnParents(group(scatterKind(1))))), &wb2, 0, &none, &why, &count), qPrintable(why));
        QCOMPARE(count, 0);
        // a chart that is read whole is not said to be skipped
        Workbook wb3;
        QVector<Chart> one;
        QVERIFY2(readCharts(bookWith(chartBody(series(0, named(0, 1), 1), columnParents(group(bar())))), &wb3, 0, &one, &why, &count), qPrintable(why));
        QCOMPARE(count, 1);
        QVERIFY(!wb3.skippedFeatures().contains(QStringLiteral("charts")));
    }

    // Saved as the XML form, the chart is a chart part under a drawing of its sheet, with the references and the place.
    void aSavedWorkbookHoldsTheChart()
    {
        const QByteArray s = series(0, named(0, 1), 1) + series(0, named(0, 2), 2);
        const QByteArray body = chartBody(s, columnParents(group(bar())), textBlock(1, QStringLiteral("Results")), legend(3));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(bookWith(body), &wb, &why), qPrintable(why));
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        Package pkg;
        QVERIFY2(pkg.readLazy(bytes, &why), qPrintable(why));
        QVERIFY(pkg.hasPart(QStringLiteral("/xl/charts/chart1.xml")));
        Workbook back;
        QVERIFY2(back.open(pkg, &why), qPrintable(why));
        const QString drawingPart = drawingPartOf(pkg, back.info().sheets[0].part);
        QVERIFY(!drawingPart.isEmpty());
        SheetDrawing d;
        QVERIFY2(readDrawing(pkg, drawingPart, &d, &why), qPrintable(why));
        QCOMPARE(d.objects.size(), 1);
        QVERIFY(d.objects[0].kind == jp::office::sheet::ObjectKind::Chart);
        QCOMPARE(d.objects[0].anchor.from.col, quint32(3));
        QCOMPARE(d.objects[0].anchor.to.row, quint32(16));
        Chart chart;
        QVERIFY2(jp::office::readChart(pkg.part(d.objects[0].chartPart), jp::office::defaultTheme(), jp::office::ColorMap(), &chart, &why), qPrintable(why));
        QCOMPARE(chart.plots[0].series.size(), 2);
        QCOMPARE(chart.plots[0].series[1].valueRef, QStringLiteral("Data!$C$2:$C$4"));
        QCOMPARE(chart.title.text, QStringLiteral("Results"));
        QCOMPARE(chart.legend.position, QStringLiteral("r"));
        // converted and saved again by the writer of the older form's file, the same parts come out
        QByteArray again;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &again, &why), qPrintable(why));
        QMap<QString, QByteArray> one, two;
        QVERIFY(jp::readZip(bytes, one));
        QVERIFY(jp::readZip(again, two));
        QCOMPARE(one.value(QStringLiteral("xl/charts/chart1.xml")), two.value(QStringLiteral("xl/charts/chart1.xml")));
        QVERIFY(!one.value(QStringLiteral("xl/charts/chart1.xml")).isEmpty());
        QCOMPARE(one.value(QStringLiteral("xl/drawings/drawing1.xml")), two.value(QStringLiteral("xl/drawings/drawing1.xml")));
    }

    // The content types stay the first entry of the file, and a workbook with no chart has the entries it always had.
    void theEntriesOfTheFileKeepTheirOrder()
    {
        Book plain;
        plain.sheet(QStringLiteral("S"), number(0, 0, 1));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(plain, &wb, &why), qPrintable(why));
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, SaveChanges(), &bytes, &why), qPrintable(why));
        QStringList names;
        for (const jp::ZipScanEntry &e : jp::scanZip(bytes)) names << e.name;
        const QStringList expected = {QStringLiteral("[Content_Types].xml"), QStringLiteral("_rels/.rels"), QStringLiteral("xl/_rels/workbook.xml.rels"), QStringLiteral("xl/workbook.xml"),
                                      QStringLiteral("xl/styles.xml"), QStringLiteral("xl/worksheets/sheet1.xml"), QStringLiteral("xl/sharedStrings.xml")};
        QCOMPARE(names, expected);
        // with a chart the first entry is the same, and the types name the chart and its drawing
        Workbook withChart;
        QVERIFY2(openBook(bookWith(chartBody(series(0, named(0, 1), 1), columnParents(group(bar())))), &withChart, &why), qPrintable(why));
        QVERIFY2(saveXlsx(withChart, SaveChanges(), &bytes, &why), qPrintable(why));
        const QVector<jp::ZipScanEntry> entries = jp::scanZip(bytes);
        QCOMPARE(entries.first().name, QStringLiteral("[Content_Types].xml"));
        QVERIFY(entries.first().data.contains("/xl/charts/chart1.xml"));
        QVERIFY(entries.first().data.contains("/xl/drawings/drawing1.xml"));
    }

    // An anchor far off the grid, and one whose end is before its start, are written inside the grid and in order; the parts of cells use the sheet's own column widths.
    void anchorsAreKeptToTheGrid()
    {
        const QByteArray body = chartBody(series(0, named(0, 1), 1), columnParents(group(bar())));
        auto place = [&](const QByteArray &anchor, const QByteArray &columns = QByteArray()) {
            Workbook wb;
            QString why;
            SheetDrawing d;
            Package pkg;
            Workbook back;
            QByteArray bytes;
            if (!openBook(bookWith(body, 0, true, QStringLiteral("Data"), anchor, columns), &wb, &why) || !saveXlsx(wb, SaveChanges(), &bytes, &why) || !pkg.readLazy(bytes, &why) || !back.open(pkg, &why) ||
                !readDrawing(pkg, drawingPartOf(pkg, back.info().sheets[0].part), &d, &why) || d.objects.size() != 1)
                return DrawingAnchor();
            return d.objects[0].anchor;
        };
        DrawingAnchor far = place(anchorRecord(60000, 100, 65000, 10, 100, 900, 5, 0));
        QCOMPARE(far.from.col, quint32(16383));
        QVERIFY(far.to.col >= far.from.col);
        QVERIFY(far.to.row >= far.from.row);
        DrawingAnchor backwards = place(anchorRecord(4, 800, 4, 200, 4, 100, 4, 50));
        QCOMPARE(backwards.to.col, backwards.from.col);
        QVERIFY(backwards.to.colOff >= backwards.from.colOff);
        QVERIFY(backwards.to.rowOff >= backwards.from.rowOff);
        // a wide column makes the same part of a cell a larger distance
        const DrawingAnchor narrow = place(anchorRecord(1, 512, 1, 0, 3, 0, 5, 0));
        const DrawingAnchor wide = place(anchorRecord(1, 512, 1, 0, 3, 0, 5, 0), colinfo(1, 1, 9000));
        QVERIFY(narrow.from.colOff > 0);
        QVERIFY(wide.from.colOff > narrow.from.colOff);
    }

    // A copy in an older form or in the other open format still says the charts are not kept.
    void copiesSayChartsAreNotKept()
    {
        Workbook wb;
        QString why;
        QVERIFY2(openBook(bookWith(chartBody(series(0, named(0, 1), 1), columnParents(group(bar())))), &wb, &why), qPrintable(why));
        QVERIFY(featuresNotInCopy(wb, CopyKind::Xls).contains(QStringLiteral("charts")));
        QVERIFY(featuresNotInCopy(wb, CopyKind::Ods).contains(QStringLiteral("charts")));
        QByteArray out;
        CopyReport report;
        QVERIFY2(saveXls(wb, &out, &report, &why), qPrintable(why));
        QVERIFY(report.notKept.contains(QStringLiteral("charts")));
    }

    // A whole column or row is not a single cell; a sheet's name with a space and an apostrophe is quoted as the XML form quotes it; a title or axis title read from a cell keeps the link.
    void referencesAndLinkedTitles()
    {
        const QByteArray column = rec(0x1003, W().u16(1).u16(1).u16(3).u16(3).u16(0).u16(0)) + begin() + named(0, 1) + ai(1, 2, tArea3d(0, 0, 1, 65535, 1, false, false)) + ai(2, 1) + ai(3, 1) +
                                  rec(0x1045, W().u16(0)) + end();
        const QByteArray linked = rec(0x1025, zeros(32)) + begin() + ai(0, 2, tRef3d(0, 0, 0, false, false)) + rec(0x1027, W().u16(1).u16(0).u16(0)) + end() + rec(0x1025, zeros(32)) + begin() +
                                  ai(0, 2, tRef3d(0, 0, 2, false, false)) + rec(0x1027, W().u16(3).u16(0).u16(0)) + end();
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        QVERIFY2(readCharts(bookWith(chartBody(column, columnParents(group(bar())), linked), 0, true, QStringLiteral("O'Brien data")), &wb, 0, &charts, &why), qPrintable(why));
        QCOMPARE(charts.size(), 1);
        const jp::office::ChartSeries &cs = charts[0].plots[0].series[0];
        QCOMPARE(cs.nameRef, QStringLiteral("'O''Brien data'!$B$1"));
        QVERIFY(cs.valueRef.startsWith(QStringLiteral("'O''Brien data'!$B")));
        QVERIFY(cs.valueRef.contains(QLatin1Char(':')));   // (not collapsed to the one cell $B)
        QCOMPARE(charts[0].title.ref, QStringLiteral("'O''Brien data'!$A$1"));
        QVERIFY(charts[0].title.present);
        QVERIFY(!charts[0].title.automatic);
        const jp::office::ChartAxis *cat = charts[0].axis(charts[0].plots[0].axisIds[0]);
        QVERIFY(cat);
        QCOMPARE(cat->title.ref, QStringLiteral("'O''Brien data'!$C$1"));
    }

    // A second group on the secondary axes keeps its own pair of axes.
    void aSecondaryAxisIsRead()
    {
        const QByteArray s = series(0, named(0, 1), 1) + series(1, named(0, 2), 2);
        const QByteArray parents = axisParent(0, axis(0) + axis(1) + group(bar(), false, 0)) + axisParent(1, axis(0) + axis(1) + group(lineKind(), false, 1));
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        QVERIFY2(readCharts(bookWith(chartBody(s, parents)), &wb, 0, &charts, &why), qPrintable(why));
        QCOMPARE(charts.size(), 1);
        QCOMPARE(charts[0].plots.size(), 2);
        QVERIFY(charts[0].plots[0].type == ChartType::Bar);
        QVERIFY(charts[0].plots[1].type == ChartType::Line);
        QCOMPARE(charts[0].axes.size(), 4);
        QCOMPARE(charts[0].plots[1].axisIds[1], qint64(1004));
        const jp::office::ChartAxis *cat2 = charts[0].axis(charts[0].plots[1].axisIds[0]), *val2 = charts[0].axis(charts[0].plots[1].axisIds[1]);
        QVERIFY(cat2 && val2);
        QVERIFY(cat2->deleted);
        QCOMPARE(val2->position, QStringLiteral("r"));
    }

    // A series of a group that is not there is left out (not put in the first group), and the chart is said to be partly read; the points of a series are bounded over all its records.
    void limitsHoldForSeriesAndPoints()
    {
        const QByteArray s = series(0, named(0, 1), 1) + series(70, named(0, 2), 2);
        Workbook wb;
        QString why;
        QVector<Chart> charts;
        QVERIFY2(readCharts(bookWith(chartBody(s, columnParents(group(bar())))), &wb, 0, &charts, &why), qPrintable(why));
        QCOMPARE(charts.size(), 1);
        QCOMPARE(charts[0].plots[0].series.size(), 1);
        QVERIFY(wb.skippedFeatures().contains(QStringLiteral("parts of charts")));
        // values in several records of one series
        QByteArray rows;
        QList<QVariantList> items;
        QVariantList row;
        for (int i = 0; i < 4000; ++i) row << double(i);
        items << row;
        QByteArray many = rec(0x1003, W().u16(1).u16(1).u16(3).u16(3).u16(0).u16(0)) + begin() + ai(0, 1);
        for (int k = 0; k < 5; ++k) many += rec(0x1051, W().u8(1).u8(2).u16(0).u16(0).u16(tArray().size()).raw(tArray()).raw(arrayValues(items)));
        many += ai(2, 1) + ai(3, 1) + rec(0x1045, W().u16(0)) + end();
        Workbook wb2;
        QVector<Chart> bounded;
        QVERIFY2(readCharts(bookWith(chartBody(many, columnParents(group(bar())))), &wb2, 0, &bounded, &why), qPrintable(why));
        QCOMPARE(bounded.size(), 1);
        QVERIFY(bounded[0].plots[0].series[0].values.size() <= 10000);
    }

    // Files made to hurt: a chart cut short, counts that are far past the bytes there are, a record longer than the file, and more series than a chart may have. Each is refused with a reason
    // or opened with its cells (when the damage is not to them), its charts and series within the limits, and a saved copy that opens again.
    void hostileChartsDoNotHurt()
    {
        // the place of the first record with the number, found by walking the records
        auto recordAt = [](const QByteArray &stream, quint16 id) -> qsizetype {
            qsizetype pos = 0;
            while (pos + 4 <= stream.size()) {
                const quint16 here = quint16(uchar(stream[pos]) | (uchar(stream[pos + 1]) << 8));
                const qsizetype len = uchar(stream[pos + 2]) | (uchar(stream[pos + 3]) << 8);
                if (here == id) return pos;
                pos += 4 + len;
            }
            return -1;
        };
        // empty if the file is handled as it should be, else what went wrong
        auto check = [](const QByteArray &file, bool cellsIntact) -> QString {
            Workbook wb;
            QString why;
            if (!wb.openBinary(file, &why)) return why.isEmpty() ? QStringLiteral("refused with no reason") : QString();
            int charts = 0;
            for (int i = 0; i < wb.info().sheets.size(); ++i) {
                Sheet s;
                wb.readSheet(i, &s);
                if (i == 0 && cellsIntact && shown(wb, s, QStringLiteral("B2")) != QStringLiteral("n:10")) return QStringLiteral("cells are not intact");
                if (s.importedCharts.size() > 1000) return QStringLiteral("too many charts");
                for (const auto &c : s.importedCharts) {
                    Chart chart;
                    QString error;
                    if (!jp::office::readChart(c.part, jp::office::defaultTheme(), jp::office::ColorMap(), &chart, &error)) return QStringLiteral("a chart part that does not read: ") + error;
                    int count = 0;
                    for (const auto &p : chart.plots) count += int(p.series.size());
                    if (count > 256) return QStringLiteral("more series than a chart may have");
                    for (const auto &p : chart.plots)
                        for (const auto &cs : p.series)
                            if (cs.values.size() > 10000) return QStringLiteral("more points than a series may have");
                }
                charts += int(s.importedCharts.size());
            }
            QByteArray out;
            if (!saveXlsx(wb, SaveChanges(), &out, &why)) return why.isEmpty() ? QStringLiteral("a save that failed with no reason") : QString();
            Package pkg;
            Workbook back;
            if (!pkg.readLazy(out, &why) || !back.open(pkg, &why)) return QStringLiteral("a saved file that does not open: ") + why;
            if (back.info().sheets.size() != wb.info().sheets.size()) return QStringLiteral("a saved file with other sheets");
            return QString();
        };
        const QByteArray s = series(0, named(0, 1), 1) + series(0, named(0, 2), 2);
        const QByteArray whole = bookWith(chartBody(s, columnParents(group(bar())), textBlock(1, QStringLiteral("T")), legend(0))).stream();
        QVERIFY2(check(whole, true).isEmpty(), qPrintable(check(whole, true)));
        // cut at every length from where the chart begins: opens or is refused, never hangs or crashes
        const qsizetype from = std::max<qsizetype>(0, recordAt(whole, 0x1002) - 40);
        QVERIFY(recordAt(whole, 0x1002) > 0);
        for (qsizetype n = from; n < whole.size(); n += 3) {
            const QString problem = check(whole.left(n), false);
            QVERIFY2(problem.isEmpty(), qPrintable(QStringLiteral("cut at %1: %2").arg(n).arg(problem)));
        }
        // a series list that claims 65,535 series in a few bytes
        const QByteArray listed = group(bar() + rec(0x1016, W().u16(0xFFFF).u16(1).u16(2)));
        QString problem = check(bookWith(chartBody(s, columnParents(listed))).file(), true);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        // a series that claims 65,535 points
        const QByteArray claims = rec(0x1003, W().u16(1).u16(1).u16(0xFFFF).u16(0xFFFF).u16(0).u16(0)) + begin() + ai(1, 2, tArea3d(0, 0, 0, 65535, 255, false, false)) + end();
        problem = check(bookWith(chartBody(claims, columnParents(group(bar())))).file(), true);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        // 65,535 series records: the chart keeps what a chart may hold
        QByteArray lots;
        for (int i = 0; i < 65535; ++i) lots += series(0, named(0, 1), 1);
        const Book lotsBook = bookWith(chartBody(lots, columnParents(group(bar()))));
        problem = check(lotsBook.file(), true);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        {
            Workbook wb;
            QVector<Chart> read;
            QString why;
            QVERIFY2(readCharts(lotsBook, &wb, 0, &read, &why), qPrintable(why));
            QCOMPARE(read.size(), 1);
            QCOMPARE(read[0].plots[0].series.size(), 256);
            QVERIFY(wb.skippedFeatures().contains(QStringLiteral("parts of charts")));
        }
        // a record whose length goes past the end of the file (the legend's)
        QByteArray stream = bookWith(chartBody(s, columnParents(group(bar())), QByteArray(), legend(0))).stream();
        const qsizetype at = recordAt(stream, 0x1015);
        QVERIFY(at > 0);
        stream[at + 2] = char(0xFF);
        stream[at + 3] = char(0xFF);
        problem = check(stream, false);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        // blocks that begin and never end, and end and never began
        QByteArray open = rec(0x1003, W().u16(1).u16(1).u16(1).u16(1).u16(0).u16(0));
        for (int i = 0; i < 500; ++i) open += begin();
        problem = check(bookWith(chartBody(open, columnParents(group(bar())))).file(), true);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        QByteArray closed;
        for (int i = 0; i < 500; ++i) closed += end();
        problem = check(bookWith(chartBody(closed, columnParents(group(bar())))).file(), true);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        // a reference with a count of tokens past the record
        const QByteArray lie = rec(0x1051, W().u8(1).u8(2).u16(0).u16(0).u16(0xFFFF).raw(zeros(5)));
        problem = check(bookWith(chartBody(rec(0x1003, W().u16(1).u16(1).u16(1).u16(1).u16(0).u16(0)) + begin() + lie + end(), columnParents(group(bar())))).file(), true);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
    }
};

JP_TEST_CLASS(SheetXlsChartTests)
#include "tst_sheet_xlschart.moc"
