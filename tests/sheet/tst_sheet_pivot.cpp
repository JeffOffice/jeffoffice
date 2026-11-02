// Tests of the pivot table engine: a table worked out from its cache (labels, values, subtotals, grand totals, filters, the three layouts), read again from a
// range of cells (Refresh) and written, and the bounds a hostile file meets. The parts are built in memory.
#include "sheethelpers.h"

#include "sheet/pivot.h"

namespace {

const char *const kNs = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

// Region, Month, Amount: six records.
//   East Jan 10, East Jan 5, East Feb 20, West Jan 7, West Feb 3, West Feb 4
QByteArray cacheDefinition()
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotCacheDefinition xmlns=\"%1\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                          "r:id=\"rId1\" refreshedBy=\"x\" refreshedDate=\"45000\" recordCount=\"6\"><cacheSource type=\"worksheet\"><worksheetSource ref=\"A1:C7\" sheet=\"Data\"/></cacheSource>"
                          "<cacheFields count=\"3\"><cacheField name=\"Region\" numFmtId=\"0\"><sharedItems count=\"2\"><s v=\"East\"/><s v=\"West\"/></sharedItems></cacheField>"
                          "<cacheField name=\"Month\" numFmtId=\"0\"><sharedItems count=\"2\"><s v=\"Jan\"/><s v=\"Feb\"/></sharedItems></cacheField>"
                          "<cacheField name=\"Amount\" numFmtId=\"0\"><sharedItems containsSemiMixedTypes=\"0\" containsString=\"0\" containsNumber=\"1\" containsInteger=\"1\" minValue=\"3\" maxValue=\"20\"/></cacheField>"
                          "</cacheFields></pivotCacheDefinition>").arg(QLatin1String(kNs)).toUtf8();
}

QByteArray cacheRecords()
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotCacheRecords xmlns=\"%1\" count=\"6\">"
                          "<r><x v=\"0\"/><x v=\"0\"/><n v=\"10\"/></r><r><x v=\"0\"/><x v=\"0\"/><n v=\"5\"/></r><r><x v=\"0\"/><x v=\"1\"/><n v=\"20\"/></r>"
                          "<r><x v=\"1\"/><x v=\"0\"/><n v=\"7\"/></r><r><x v=\"1\"/><x v=\"1\"/><n v=\"3\"/></r><r><x v=\"1\"/><x v=\"1\"/><n v=\"4\"/></r></pivotCacheRecords>").arg(QLatin1String(kNs)).toUtf8();
}

// The table: Region down the side, Month across (when `columns`), the sum of Amount; `attrs` are the table's, `region` the pivotField of Region.
QByteArray tableDefinition(bool columns, const QString &attrs = QString(), const QString &region = QString(), const QString &dataFieldAttrs = QString(), const QString &pageFields = QString())
{
    const QString regionField = region.isEmpty() ? QStringLiteral("<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\" showAll=\"0\"><items count=\"3\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField>") : region;
    const QString monthField = columns ? QStringLiteral("<pivotField axis=\"axisCol\" compact=\"0\" outline=\"0\" showAll=\"0\"><items count=\"3\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField>")
                                       : QStringLiteral("<pivotField compact=\"0\" outline=\"0\" showAll=\"0\"/>");
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotTableDefinition xmlns=\"%1\" name=\"PivotTable1\" cacheId=\"1\" dataCaption=\"Values\" compact=\"0\" compactData=\"0\" %2>"
                          "<location ref=\"E3:G8\" firstHeaderRow=\"1\" firstDataRow=\"2\" firstDataCol=\"1\"/><pivotFields count=\"3\">%3%4<pivotField dataField=\"1\" compact=\"0\" outline=\"0\" showAll=\"0\"/></pivotFields>"
                          "<rowFields count=\"1\"><field x=\"0\"/></rowFields>%5%6<dataFields count=\"1\"><dataField name=\"Sum of Amount\" fld=\"2\" %7/></dataFields></pivotTableDefinition>")
        .arg(QLatin1String(kNs), attrs, regionField, monthField, columns ? QStringLiteral("<colFields count=\"1\"><field x=\"1\"/></colFields>") : QString(), pageFields, dataFieldAttrs.isEmpty() ? QStringLiteral("baseField=\"0\" baseItem=\"0\"") : dataFieldAttrs)
        .toUtf8();
}

struct Worked {
    PivotTable table;
    PivotCache cache;
    PivotGrid grid;
    QString why;
    bool ok = false;
};

Worked work(const QByteArray &tableXml, const QByteArray &def = cacheDefinition(), const QByteArray &rec = cacheRecords())
{
    Worked w;
    if (!readPivotCache(def, rec, DateSystem::Date1900, &w.cache, &w.why)) return w;
    if (!readPivotTable(tableXml, &w.table, &w.why)) return w;
    w.ok = computePivot(w.table, w.cache, &w.grid, &w.why);
    return w;
}

QString text(const PivotGrid &g, int r, int c)
{
    const PivotCell &cell = g.at(r, c);
    switch (cell.kind) {
    case PivotCell::Empty: return QString();
    case PivotCell::Number: return QString::number(cell.number, 'g', 15);
    default: return cell.text;
    }
}

QStringList rowText(const PivotGrid &g, int r)
{
    QStringList out;
    for (int c = 0; c < g.cols; ++c) out << text(g, r, c);
    return out;
}

} // namespace

class SheetPivotTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A table with the months across: a label for each region and month, the sum in each cell, a total for each region, and a grand total.
    void aTableIsWorkedOutFromItsCache()
    {
        const Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.grid.rows, 5);
        QCOMPARE(w.grid.cols, 4);
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Sum of Amount"), QStringLiteral("Month"), QString(), QString()}));
        QCOMPARE(rowText(w.grid, 1), (QStringList{QStringLiteral("Region"), QStringLiteral("Jan"), QStringLiteral("Feb"), QStringLiteral("Grand Total")}));
        QCOMPARE(rowText(w.grid, 2), (QStringList{QStringLiteral("East"), QStringLiteral("15"), QStringLiteral("20"), QStringLiteral("35")}));
        QCOMPARE(rowText(w.grid, 3), (QStringList{QStringLiteral("West"), QStringLiteral("7"), QStringLiteral("7"), QStringLiteral("14")}));
        QCOMPARE(rowText(w.grid, 4), (QStringList{QStringLiteral("Grand Total"), QStringLiteral("22"), QStringLiteral("27"), QStringLiteral("49")}));
        QCOMPARE(w.grid.firstDataRow, 2);
        QCOMPARE(w.grid.firstDataCol, 1);
    }

    // The lines of the axes are made as a file has them: the places of the items, and a line for the grand total.
    void theLinesOfTheAxesAreWorkedOut()
    {
        const Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.grid.rowItems.size(), 3);
        QCOMPARE(w.grid.rowItems[0].x, (QVector<int>{0}));
        QCOMPARE(w.grid.rowItems[1].x, (QVector<int>{1}));
        QCOMPARE(w.grid.rowItems[2].t, QStringLiteral("grand"));
        QCOMPARE(w.grid.colItems.size(), 3);
        QCOMPARE(w.grid.colItems[2].t, QStringLiteral("grand"));
    }

    // Grand totals can be turned off for either side; a row field with no more fields under it has no subtotal.
    void grandTotalsFollowTheirSwitches()
    {
        Worked w = work(tableDefinition(true, QStringLiteral("rowGrandTotals=\"0\"")));
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.grid.rows, 4);   // (no row of grand totals)
        QCOMPARE(w.grid.cols, 4);
        w = work(tableDefinition(true, QStringLiteral("colGrandTotals=\"0\"")));
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.grid.rows, 5);
        QCOMPARE(w.grid.cols, 3);   // (no column of them)
    }

    // The function of the values: count, average, max, min, product, and the deviations and variances.
    void theValuesAreTotaledByTheFunctionTheFieldAsks()
    {
        auto grand = [&](const QString &fn) {
            const Worked w = work(tableDefinition(false, QString(), QString(), QStringLiteral("subtotal=\"%1\" baseField=\"0\" baseItem=\"0\"").arg(fn)));
            if (!w.ok) return QStringLiteral("ERR ") + w.why;
            return text(w.grid, w.grid.rows - 1, w.grid.cols - 1);
        };
        QCOMPARE(grand(QStringLiteral("sum")), QStringLiteral("49"));
        QCOMPARE(grand(QStringLiteral("count")), QStringLiteral("6"));
        QCOMPARE(grand(QStringLiteral("countNums")), QStringLiteral("6"));
        QCOMPARE(grand(QStringLiteral("max")), QStringLiteral("20"));
        QCOMPARE(grand(QStringLiteral("min")), QStringLiteral("3"));
        QCOMPARE(grand(QStringLiteral("product")), QStringLiteral("84000"));
        const double mean = 49.0 / 6.0;
        double sq = 0;
        for (double v : {10., 5., 20., 7., 3., 4.}) sq += (v - mean) * (v - mean);
        QVERIFY(qAbs(grand(QStringLiteral("average")).toDouble() - mean) < 1e-12);
        QVERIFY(qAbs(grand(QStringLiteral("var")).toDouble() - sq / 5) < 1e-10);
        QVERIFY(qAbs(grand(QStringLiteral("varp")).toDouble() - sq / 6) < 1e-10);
        QVERIFY(qAbs(grand(QStringLiteral("stdDev")).toDouble() - std::sqrt(sq / 5)) < 1e-10);
        QVERIFY(qAbs(grand(QStringLiteral("stdDevp")).toDouble() - std::sqrt(sq / 6)) < 1e-10);
    }

    // A hidden item is left out of the table and of its totals; a filter on a page field keeps the records of one item.
    void hiddenItemsAndFiltersLeaveRecordsOut()
    {
        const QString west = QStringLiteral("<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\" showAll=\"0\"><items count=\"3\"><item x=\"0\"/><item h=\"1\" x=\"1\"/><item t=\"default\"/></items></pivotField>");
        Worked w = work(tableDefinition(true, QString(), west));
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.grid.rows, 4);
        QCOMPARE(rowText(w.grid, 3), (QStringList{QStringLiteral("Grand Total"), QStringLiteral("15"), QStringLiteral("20"), QStringLiteral("35")}));
        // Month as a filter, on Jan
        const QString page = QStringLiteral("<pageFields count=\"1\"><pageField fld=\"1\" item=\"0\" hier=\"-1\"/></pageFields>");
        QString xml = QString::fromUtf8(tableDefinition(false, QString(), QString(), QString(), page));
        xml.replace(QStringLiteral("<pivotField compact=\"0\" outline=\"0\" showAll=\"0\"/><pivotField dataField"), QStringLiteral("<pivotField axis=\"axisPage\" compact=\"0\" outline=\"0\" showAll=\"0\"><items count=\"3\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField><pivotField dataField"));
        w = work(xml.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(text(w.grid, w.grid.rows - 1, w.grid.cols - 1), QStringLiteral("22"));
    }

    // The outline form puts a field's item on a line of its own, the compact form puts every field in one column; each has a caption the file names.
    void theThreeLayoutsPutTheLabelsWhereTheyBelong()
    {
        QString xml = QString::fromUtf8(tableDefinition(false));
        // two row fields: Region, then Month
        xml.replace(QStringLiteral("<rowFields count=\"1\"><field x=\"0\"/></rowFields>"), QStringLiteral("<rowFields count=\"2\"><field x=\"0\"/><field x=\"1\"/></rowFields>"));
        xml.replace(QStringLiteral("<pivotField compact=\"0\" outline=\"0\" showAll=\"0\"/><pivotField dataField"), QStringLiteral("<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\" showAll=\"0\"><items count=\"3\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField><pivotField dataField"));
        Worked w = work(xml.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        // tabular: the region on its first line, "East Total" after its months, "Grand Total" at the end
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Sum of Amount"), QString(), QString()}));
        QCOMPARE(rowText(w.grid, 1), (QStringList{QStringLiteral("Region"), QStringLiteral("Month"), QStringLiteral("Total")}));
        QCOMPARE(rowText(w.grid, 2), (QStringList{QStringLiteral("East"), QStringLiteral("Jan"), QStringLiteral("15")}));
        QCOMPARE(rowText(w.grid, 3), (QStringList{QString(), QStringLiteral("Feb"), QStringLiteral("20")}));
        QCOMPARE(rowText(w.grid, 4), (QStringList{QStringLiteral("East Total"), QString(), QStringLiteral("35")}));
        QCOMPARE(rowText(w.grid, 8), (QStringList{QStringLiteral("Grand Total"), QString(), QStringLiteral("49")}));
        // outline: each region on a line of its own with its total (subtotals on top)
        QString outline = xml;
        outline.replace(QStringLiteral("compact=\"0\" outline=\"0\" showAll=\"0\" "), QStringLiteral("compact=\"0\" showAll=\"0\" "));
        outline.replace(QStringLiteral("axis=\"axisRow\" compact=\"0\" outline=\"0\""), QStringLiteral("axis=\"axisRow\" compact=\"0\""));
        w = work(outline.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Region"), QStringLiteral("Month"), QStringLiteral("Sum of Amount")}));
        QCOMPARE(rowText(w.grid, 1), (QStringList{QStringLiteral("East"), QString(), QStringLiteral("35")}));
        QCOMPARE(rowText(w.grid, 2), (QStringList{QString(), QStringLiteral("Jan"), QStringLiteral("15")}));
        // compact: one column of labels
        QString compact = xml;
        compact.replace(QStringLiteral("compact=\"0\" outline=\"0\" showAll=\"0\""), QStringLiteral("showAll=\"0\""));
        compact.replace(QStringLiteral("compact=\"0\" compactData=\"0\""), QStringLiteral("compactData=\"0\""));
        w = work(compact.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.grid.cols, 2);
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Row Labels"), QStringLiteral("Sum of Amount")}));
        QCOMPARE(rowText(w.grid, 1), (QStringList{QStringLiteral("East"), QStringLiteral("35")}));
        QCOMPARE(rowText(w.grid, 2), (QStringList{QStringLiteral("Jan"), QStringLiteral("15")}));
    }

    // A source cell that is empty is "(blank)" among the items, and nothing as a value; items that are text are as they are.
    void blanksAreNamedBlank()
    {
        QByteArray def = cacheDefinition();
        def.replace("<s v=\"West\"/>", "<m/>").replace("<sharedItems count=\"2\"><s v=\"East\"/>", "<sharedItems containsBlank=\"1\" count=\"2\"><s v=\"East\"/>");
        const Worked w = work(tableDefinition(true), def);
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(text(w.grid, 3, 0), QStringLiteral("(blank)"));
    }

    // Refresh: the cache is read from the cells again, a new item is added after the others, the items that went are taken out, and the table keeps its layout.
    void refreshReadsTheCellsAgain()
    {
        Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        // the cells now: East gone, North added, Mar added
        const QVector<QStringList> rows = {{QStringLiteral("Region"), QStringLiteral("Month"), QStringLiteral("Amount")},
                                           {QStringLiteral("West"), QStringLiteral("Jan"), QStringLiteral("7")},
                                           {QStringLiteral("West"), QStringLiteral("Mar"), QStringLiteral("1")},
                                           {QStringLiteral("North"), QStringLiteral("Feb"), QStringLiteral("2")}};
        PivotSource src;
        src.row1 = 0;
        src.col1 = 0;
        src.row2 = 3;
        src.col2 = 2;
        src.axisFields = {0, 1};
        src.cell = [&](quint32 r, quint32 c) {
            PivotValue v;
            const QString s = rows[int(r)][int(c)];
            bool ok = false;
            const double d = s.toDouble(&ok);
            if (ok) {
                v.kind = PivotValue::Number;
                v.number = d;
            } else {
                v.kind = PivotValue::Text;
                v.text = s;
            }
            return v;
        };
        PivotCache fresh;
        QVERIFY2(buildPivotCache(src, w.cache, &fresh, &w.why), qPrintable(w.why));
        QCOMPARE(fresh.recordCount, qint64(3));
        PivotTable table = w.table;
        matchPivotItems(&table, w.cache, fresh);
        PivotGrid grid;
        QVERIFY2(computePivot(table, fresh, &grid, &w.why), qPrintable(w.why));
        // (the previous order stays: West was second, now the first that is left, North after it; Jan was first, Feb second, Mar after them)
        QCOMPARE(rowText(grid, 1), (QStringList{QStringLiteral("Region"), QStringLiteral("Jan"), QStringLiteral("Feb"), QStringLiteral("Mar"), QStringLiteral("Grand Total")}));
        QCOMPARE(rowText(grid, 2), (QStringList{QStringLiteral("West"), QStringLiteral("7"), QString(), QStringLiteral("1"), QStringLiteral("8")}));
        QCOMPARE(rowText(grid, 3), (QStringList{QStringLiteral("North"), QString(), QStringLiteral("2"), QString(), QStringLiteral("2")}));
        QCOMPARE(rowText(grid, 4), (QStringList{QStringLiteral("Grand Total"), QStringLiteral("7"), QStringLiteral("2"), QStringLiteral("1"), QStringLiteral("10")}));
    }

    // What is written is well formed, and reads back to a table that shows the same cells (the cache, its records, and the definition agree).
    void whatIsWrittenReadsBack()
    {
        const Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        const QByteArray def = writePivotCacheDefinition(cacheDefinition(), w.cache, w.cache.recordCount, 45001.5, DateSystem::Date1900);
        const QByteArray rec = writePivotCacheRecords(w.cache, DateSystem::Date1900);
        const QByteArray table = writePivotTable(tableDefinition(true), w.table, w.grid, QStringLiteral("E3:H8"), false);
        QVERIFY(!def.isEmpty() && !rec.isEmpty() && !table.isEmpty());
        QVERIFY2(def.contains("refreshedDate=\"45001.5\"") && def.contains("recordCount=\"6\""), def.constData());
        QVERIFY2(table.contains("E3:H8"), table.constData());
        const Worked again = work(table, def, rec);
        QVERIFY2(again.ok, qPrintable(again.why));
        QCOMPARE(again.grid.rows, w.grid.rows);
        for (int i = 0; i < w.grid.cells.size(); ++i) QVERIFY(w.grid.cells[i] == again.grid.cells[i]);
    }

    // A cache that is not as it should be, or too big, is refused with a reason and never loops or allocates without bound.
    void hostilePartsAreRefused()
    {
        PivotCache cache;
        QString why;
        QVERIFY(!readPivotCache("<pivotCacheDefinition", QByteArray(), DateSystem::Date1900, &cache, &why));
        // more values in a record than fields
        QByteArray rec = cacheRecords();
        rec.replace("<r><x v=\"0\"/><x v=\"0\"/><n v=\"10\"/></r>", "<r><x v=\"0\"/><x v=\"0\"/><n v=\"10\"/><n v=\"1\"/></r>");
        QVERIFY(!readPivotCache(cacheDefinition(), rec, DateSystem::Date1900, &cache, &why));
        // a record that names a shared item that is not there
        rec = cacheRecords();
        rec.replace("<x v=\"1\"/><x v=\"1\"/><n v=\"4\"/>", "<x v=\"9\"/><x v=\"1\"/><n v=\"4\"/>");
        QVERIFY(!readPivotCache(cacheDefinition(), rec, DateSystem::Date1900, &cache, &why));
        // too many fields
        QByteArray def = "<pivotCacheDefinition xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><cacheFields>";
        for (int i = 0; i < kPivotMaxFields + 1; ++i) def += "<cacheField name=\"f\"/>";
        def += "</cacheFields></pivotCacheDefinition>";
        QVERIFY(!readPivotCache(def, QByteArray(), DateSystem::Date1900, &cache, &why));
        // a table that names a field the cache does not have, or has an axis of values twice
        Worked w = work(tableDefinition(true));
        PivotTable bad = w.table;
        bad.rowFields = {7};
        PivotGrid grid;
        QVERIFY(!computePivot(bad, w.cache, &grid, &why));
        bad = w.table;
        bad.rowFields = {-2};
        bad.colFields = {-2};
        QVERIFY(!computePivot(bad, w.cache, &grid, &why));
        // a source with too many columns is not read
        PivotSource src;
        src.row1 = 0;
        src.col1 = 0;
        src.row2 = 1;
        src.col2 = kPivotMaxFields + 5;
        src.cell = [](quint32, quint32) { return PivotValue(); };
        PivotCache fresh;
        QVERIFY(!buildPivotCache(src, w.cache, &fresh, &why));
    }
};

JP_TEST_CLASS(SheetPivotTests)
#include "tst_sheet_pivot.moc"
