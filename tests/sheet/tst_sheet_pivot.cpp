// Tests of the pivot table engine: a table worked out from its cache (labels, values, subtotals, grand totals, filters, the three layouts), read again from a
// range of cells (Refresh) and written, and the bounds a hostile file meets. The parts are built in memory.
#include "sheethelpers.h"

#include "sheet/pivot.h"

#include <QElapsedTimer>

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
        PivotTable placed = w.table;
        pivotApplyGrid(&placed, w.grid, 2, 4);
        const QByteArray table = writePivotTable(tableDefinition(true), placed, false);
        QVERIFY(!def.isEmpty() && !rec.isEmpty() && !table.isEmpty());
        QVERIFY2(def.contains("refreshedDate=\"45001.5\"") && def.contains("recordCount=\"6\""), def.constData());
        QVERIFY2(table.contains("E3:H7"), table.constData());
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

    // ---- the review: speed and bounds

    // A hundred thousand different items of one field are labeled in a time that grows with them (a scan of the item list for each label would take minutes).
    void manyDistinctRowItemsAreLabeledFast()
    {
        const int n = 100000;
        PivotCache c;
        PivotCacheField f0, f1;
        f0.name = QStringLiteral("R");
        f0.hasShared = true;
        f0.sharedCount = n;
        f1.name = QStringLiteral("Amount");
        PivotValue one;
        one.kind = PivotValue::Number;
        one.number = 1;
        f1.items.append(one);
        for (int i = 0; i < n; ++i) {
            PivotValue v;
            v.kind = PivotValue::Text;
            v.text = QStringLiteral("R%1").arg(i);
            f0.items.append(v);
            f0.records.append(i);
            f1.records.append(0);
        }
        c.fields = {f0, f1};
        c.recordCount = n;
        QString items;
        for (int i = n - 1; i >= 0; --i) items += QStringLiteral("<item x=\"%1\"/>").arg(i);
        const QByteArray xml = QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\" compact=\"0\"><location ref=\"A1:B2\" firstHeaderRow=\"1\" firstDataRow=\"1\" firstDataCol=\"1\"/>"
                                             "<pivotFields count=\"2\"><pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\"><items count=\"%2\">%3<item t=\"default\"/></items></pivotField><pivotField dataField=\"1\"/></pivotFields>"
                                             "<rowFields count=\"1\"><field x=\"0\"/></rowFields><dataFields count=\"1\"><dataField name=\"S\" fld=\"1\"/></dataFields></pivotTableDefinition>")
                                 .arg(QLatin1String(kNs)).arg(n + 1).arg(items).toUtf8();
        PivotTable t;
        QString why;
        QVERIFY2(readPivotTable(xml, &t, &why), qPrintable(why));
        PivotGrid g;
        QElapsedTimer timer;
        timer.start();
        QVERIFY2(computePivot(t, c, &g, &why), qPrintable(why));
        QVERIFY2(timer.elapsed() < 8000, qPrintable(QString::number(timer.elapsed())));
        QCOMPARE(g.rows, 2 + n + 1);
        QCOMPARE(text(g, 2, 0), QStringLiteral("R%1").arg(n - 1));
    }

    // The lists of fields are bounded and name a field once (the values once).
    void fieldListsAreBoundedAndNameAFieldOnce()
    {
        auto table = [&](const QString &lists) {
            return QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\"><location ref=\"A1:B2\"/><pivotFields count=\"3\"><pivotField/><pivotField/><pivotField/></pivotFields>%2</pivotTableDefinition>")
                .arg(QLatin1String(kNs), lists).toUtf8();
        };
        PivotTable t;
        QString why;
        QVERIFY2(readPivotTable(table(QStringLiteral("<rowFields count=\"2\"><field x=\"0\"/><field x=\"1\"/></rowFields>")), &t, &why), qPrintable(why));
        QVERIFY(!readPivotTable(table(QStringLiteral("<rowFields count=\"2\"><field x=\"0\"/><field x=\"0\"/></rowFields>")), &t, &why));
        QVERIFY(!readPivotTable(table(QStringLiteral("<rowFields count=\"1\"><field x=\"0\"/></rowFields><colFields count=\"1\"><field x=\"0\"/></colFields>")), &t, &why));
        QVERIFY(!readPivotTable(table(QStringLiteral("<rowFields count=\"1\"><field x=\"0\"/></rowFields><pageFields count=\"1\"><pageField fld=\"0\"/></pageFields>")), &t, &why));
        QVERIFY(!readPivotTable(table(QStringLiteral("<rowFields count=\"1\"><field x=\"-2\"/></rowFields><colFields count=\"1\"><field x=\"-2\"/></colFields>")), &t, &why));
        QString many;
        for (int i = 0; i < kPivotMaxFields + 5; ++i) many += QStringLiteral("<field x=\"%1\"/>").arg(i);
        QVERIFY(!readPivotTable(table(QStringLiteral("<rowFields count=\"1\">") + many + QStringLiteral("</rowFields>")), &t, &why));
        QString datas;
        for (int i = 0; i < 5000; ++i) datas += QStringLiteral("<dataField name=\"d%1\" fld=\"0\"/>").arg(i);
        QVERIFY(!readPivotTable(table(QStringLiteral("<dataFields count=\"5000\">") + datas + QStringLiteral("</dataFields>")), &t, &why));
    }

    // More different items than a sheet has rows are refused before the lines are built.
    void tooManyDistinctMembersAreRefusedEarly()
    {
        const int n = 1100000;
        PivotCache c;
        PivotCacheField f0, f1;
        f0.name = QStringLiteral("R");
        f1.name = QStringLiteral("A");
        PivotValue one;
        one.kind = PivotValue::Number;
        one.number = 1;
        f1.items.append(one);
        f0.items.reserve(n);
        for (int i = 0; i < n; ++i) {
            PivotValue v;
            v.kind = PivotValue::Number;
            v.number = i;
            f0.items.append(v);
            f0.records.append(i);
            f1.records.append(0);
        }
        c.fields = {f0, f1};
        c.recordCount = n;
        PivotTable t;
        QString why;
        QVERIFY2(readPivotTable(QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\"><location ref=\"A1:B2\"/><pivotFields count=\"2\"><pivotField axis=\"axisRow\"/><pivotField dataField=\"1\"/></pivotFields>"
                                                "<rowFields count=\"1\"><field x=\"0\"/></rowFields><dataFields count=\"1\"><dataField name=\"S\" fld=\"1\"/></dataFields></pivotTableDefinition>").arg(QLatin1String(kNs)).toUtf8(), &t, &why), qPrintable(why));
        PivotGrid g;
        QVERIFY(!computePivot(t, c, &g, &why));
        QVERIFY2(why.contains(QLatin1String("too many")), qPrintable(why));
    }

    // The items of all the fields of a table are bounded together.
    void theItemsOfAllFieldsAreBoundedTogether()
    {
        QString fields;
        for (int f = 0; f < 3; ++f) {
            QString items;
            for (int i = 0; i < 700000; ++i) items += QStringLiteral("<item x=\"%1\"/>").arg(i);
            fields += QStringLiteral("<pivotField axis=\"axisRow\"><items count=\"700000\">") + items + QStringLiteral("</items></pivotField>");
        }
        PivotTable t;
        QString why;
        QVERIFY(!readPivotTable(QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\"><location ref=\"A1:B2\"/><pivotFields count=\"3\">%2</pivotFields></pivotTableDefinition>").arg(QLatin1String(kNs), fields).toUtf8(), &t, &why));
        QVERIFY2(why.contains(QLatin1String("too many")), qPrintable(why));
    }

    // ---- the review: what is written

    // Dates are written as the schema's date and time, in the items and in the range of dates; a day the calendar does not have is written as a number.
    void datesAreWrittenAsDateTimes()
    {
        QByteArray def = cacheDefinition();
        def.replace("<s v=\"East\"/><s v=\"West\"/>", "<d v=\"2001-04-12T00:00:00\"/><d v=\"2002-01-31T12:00:00\"/><d v=\"1900-02-28T00:00:00\"/>").replace("<sharedItems count=\"2\"><d", "<sharedItems count=\"3\"><d");
        PivotCache c;
        QString why;
        QVERIFY2(readPivotCache(def, QByteArray(), DateSystem::Date1900, &c, &why), qPrintable(why));
        QCOMPARE(c.fields[0].items.size(), 3);
        c.fields[0].items[2].number = 60;   // (day 60 of the 1900 system is a day that never was)
        const QByteArray out = writePivotCacheDefinition(def, c, 0, 45000, DateSystem::Date1900);
        QVERIFY2(out.contains("minDate=\"2001-04-12T00:00:00\"") && out.contains("maxDate=\"2002-01-31T12:00:00\""), out.constData());
        QVERIFY2(out.contains("<d v=\"2001-04-12T00:00:00\""), out.constData());
        QVERIFY2(out.contains("<n v=\"60\""), out.constData());
        QVERIFY2(!out.contains("v=\"1900-01-01"), out.constData());
    }

    // A list with nothing in it is not written (the schema wants at least one child), after Clear All and after a filter that leaves nothing.
    void emptyListsAreNotWritten()
    {
        Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        PivotTable t = w.table;
        t.rowFields.clear();
        t.colFields.clear();
        t.dataFields.clear();
        pivotSyncRoles(&t);
        PivotGrid g;
        QVERIFY2(computePivot(t, w.cache, &g, &w.why), qPrintable(w.why));
        pivotApplyGrid(&t, g, 2, 4);
        QByteArray xml = writePivotTable(tableDefinition(true), t, true);
        QVERIFY2(!xml.contains("count=\"0\""), xml.constData());
        // a filter that hides every item: no lines, and no grand total
        t = w.table;
        t.rowGrandTotals = false;
        t.colGrandTotals = false;
        for (PivotTableItem &it : t.fields[0].items) it.hidden = it.t.isEmpty();
        QVERIFY2(computePivot(t, w.cache, &g, &w.why), qPrintable(w.why));
        pivotApplyGrid(&t, g, 2, 4);
        xml = writePivotTable(tableDefinition(true), t, false);
        QVERIFY2(!xml.contains("count=\"0\""), xml.constData());
    }

    // Each field keeps its own marks and children when the source gains or loses a column: a column gone takes its field's with it.
    void fieldsFollowTheirColumnsWhenTheSourceChanges()
    {
        QString xml = QString::fromUtf8(tableDefinition(true));
        xml.replace(QStringLiteral("<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\" showAll=\"0\">"),
                    QStringLiteral("<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\" showAll=\"0\" sortType=\"descending\"><autoSortScope><pivotArea dataOnly=\"0\" outline=\"0\" fieldPosition=\"0\"><references count=\"1\"><reference field=\"4294967294\" count=\"1\" selected=\"0\"><x v=\"0\"/></reference></references></pivotArea></autoSortScope>"));
        xml.replace(QStringLiteral("<pivotField axis=\"axisCol\" compact=\"0\" outline=\"0\" showAll=\"0\">"), QStringLiteral("<pivotField axis=\"axisCol\" compact=\"0\" outline=\"0\" showAll=\"0\" multipleItemSelectionAllowed=\"1\">"));
        Worked w = work(xml.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QVERIFY(w.table.fields[0].sortByData == 0);
        // the cells now have no Region column: Month, Amount
        PivotSource src;
        src.row1 = 0;
        src.col1 = 0;
        src.row2 = 2;
        src.col2 = 1;
        src.axisFields = {1};
        src.cell = [](quint32 r, quint32 c) {
            PivotValue v;
            if (r == 0) {
                v.kind = PivotValue::Text;
                v.text = c == 0 ? QStringLiteral("Month") : QStringLiteral("Amount");
            } else if (c == 0) {
                v.kind = PivotValue::Text;
                v.text = QStringLiteral("Jan");
            } else {
                v.kind = PivotValue::Number;
                v.number = r;
            }
            return v;
        };
        PivotCache fresh;
        QVERIFY2(buildPivotCache(src, w.cache, &fresh, &w.why), qPrintable(w.why));
        PivotTable t = w.table;
        matchPivotItems(&t, w.cache, fresh);
        PivotGrid g;
        QVERIFY2(computePivot(t, fresh, &g, &w.why), qPrintable(w.why));
        pivotApplyGrid(&t, g, 2, 4);
        const QByteArray out = writePivotTable(QByteArray(xml.toUtf8()), t, true);
        QVERIFY2(!out.contains("autoSortScope"), out.constData());
        QVERIFY2(out.count("multipleItemSelectionAllowed=\"1\"") == 1, out.constData());
        QVERIFY2(out.contains("<pivotFields count=\"2\">"), out.constData());
        // and read back, the Month field is the first and still has its mark
        PivotTable back;
        QVERIFY2(readPivotTable(out, &back, &w.why), qPrintable(w.why));
        QCOMPARE(back.fields.size(), 2);
        QCOMPARE(back.fields[0].axis, QStringLiteral("axisCol"));
    }

    // A control character in a name or a text is written as _xHHHH_, and a text that has _xHHHH_ in it as _x005F_xHHHH_; read back, both are what they were.
    void controlCharactersAreEscaped()
    {
        QByteArray def = cacheDefinition();
        PivotCache c;
        QString why;
        QVERIFY2(readPivotCache(def, cacheRecords(), DateSystem::Date1900, &c, &why), qPrintable(why));
        c.fields[0].name = QStringLiteral("a") + QChar(1) + QStringLiteral("b_x0041_");
        c.fields[0].items[0].text = QStringLiteral("x") + QChar(2) + QStringLiteral("y");
        c.fields[0].items[1].text = QStringLiteral("_x0041_");
        const QByteArray d = writePivotCacheDefinition(def, c, 6, 45000, DateSystem::Date1900);
        QVERIFY2(d.contains("_x0001_") && d.contains("_x0002_") && d.contains("_x005F_x0041_"), d.constData());
        PivotCache back;
        QVERIFY2(readPivotCache(d, writePivotCacheRecords(c, DateSystem::Date1900), DateSystem::Date1900, &back, &why), qPrintable(why));
        QCOMPARE(back.fields[0].name, c.fields[0].name);
        QCOMPARE(back.fields[0].items[0].text, c.fields[0].items[0].text);
        QCOMPARE(back.fields[0].items[1].text, QStringLiteral("_x0041_"));
        // a text in the records
        c.fields[2].hasShared = false;
        c.fields[0].hasShared = false;
        const QByteArray r = writePivotCacheRecords(c, DateSystem::Date1900);
        QVERIFY2(r.contains("_x0002_") && r.contains("_x005F_x0041_"), r.constData());
    }

    // The written parts are as the schema has them: the order of the elements, the counts, the places that are pointed to, and the marks of the shared items; and the check sees what is not.
    void writtenPartsAreAsTheSchemaHasThem()
    {
        const Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        PivotTable placed = w.table;
        pivotApplyGrid(&placed, w.grid, 2, 4);
        const QByteArray def = writePivotCacheDefinition(cacheDefinition(), w.cache, w.cache.recordCount, 45001.5, DateSystem::Date1900);
        const QByteArray rec = writePivotCacheRecords(w.cache, DateSystem::Date1900);
        const QByteArray table = writePivotTable(tableDefinition(true), placed, false);
        QString why;
        QVERIFY2(pivotCheckShape(table, def, rec, &why), qPrintable(why));
        auto bad = [&](QByteArray t, QByteArray d, QByteArray r, const char *from, const char *to) {
            QByteArray *which = t.contains(from) ? &t : d.contains(from) ? &d : &r;
            which->replace(from, to);
            return !pivotCheckShape(t, d, r, &why);
        };
        QVERIFY(bad(table, def, rec, "<rowItems count=\"3\">", "<rowItems count=\"4\">"));
        QVERIFY(bad(table, def, rec, "<rowFields", "<rowFieldz"));
        QVERIFY(bad(table, def, rec, "<x v=\"1\"/></i><i t=\"grand\">", "<x v=\"7\"/></i><i t=\"grand\">"));
        QVERIFY(bad(table, def, rec, "firstDataRow=\"2\"", "firstDataRow=\"9\""));
        QVERIFY(bad(table, def, rec, "<r><x v=\"0\"/>", "<r><x v=\"5\"/>"));
        QVERIFY(bad(table, def, rec, "<sharedItems count=\"2\">", "<sharedItems containsNumber=\"1\" count=\"2\">"));
        QVERIFY(bad(table, def, rec, "<sharedItems count=\"2\">", "<sharedItems containsMixedTypes=\"1\" count=\"2\">"));
        QVERIFY(bad(table, def, rec, "recordCount=\"6\"", "recordCount=\"7\""));
        // two element order: the cache fields before the source
        QByteArray swapped = def;
        const qsizetype a = swapped.indexOf("<cacheSource"), b = swapped.indexOf("</cacheSource>") + 14;
        const QByteArray src = swapped.mid(a, b - a);
        swapped.remove(a, b - a);
        swapped.insert(swapped.indexOf("</pivotCacheDefinition>"), src);
        QVERIFY(!pivotCheckShape(table, swapped, rec, &why));
    }

    // A sum is added in the order of the records, and what is left of two numbers that cancel is nothing (the original program shows 0, not 5.5e-17).
    void sumsAreAddedInRecordOrderAndCancelToZero()
    {
        const QByteArray rec = "<pivotCacheRecords xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"6\">"
                               "<r><x v=\"0\"/><x v=\"0\"/><n v=\"0.1\"/></r><r><x v=\"0\"/><x v=\"0\"/><n v=\"0.2\"/></r><r><x v=\"0\"/><x v=\"1\"/><n v=\"-0.3\"/></r>"
                               "<r><x v=\"1\"/><x v=\"0\"/><n v=\"1e16\"/></r><r><x v=\"1\"/><x v=\"1\"/><n v=\"1\"/></r><r><x v=\"1\"/><x v=\"1\"/><n v=\"-1e16\"/></r></pivotCacheRecords>";
        const Worked w = work(tableDefinition(false), cacheDefinition(), rec);
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(rowText(w.grid, 2), (QStringList{QStringLiteral("East"), QStringLiteral("0")}));   // 0.1 + 0.2 - 0.3
        QCOMPARE(w.grid.at(3, 1).number, (1e16 + 1.0) - 1e16);
    }

    // The line of the value's name: there in every form of a table with the classic grid, and a table with nothing on its sides has one line for it and one for "Total".
    void theCaptionLineAndABareTable()
    {
        QString xml = QString::fromUtf8(tableDefinition(false));
        xml.replace(QStringLiteral("<rowFields count=\"1\"><field x=\"0\"/></rowFields>"), QString());
        xml.replace(QStringLiteral("axis=\"axisRow\" "), QString());
        xml.replace(QStringLiteral("<items count=\"3\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items>"), QString());
        Worked w = work(xml.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.grid.rows, 2);
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Sum of Amount"), QString()}));
        QCOMPARE(rowText(w.grid, 1), (QStringList{QStringLiteral("Total"), QStringLiteral("49")}));
        QCOMPARE(w.grid.firstDataRow, 1);
        // one row field in the compact form: the caption line only with the classic grid
        QString compact = QString::fromUtf8(tableDefinition(false));
        compact.replace(QStringLiteral("compact=\"0\" outline=\"0\" showAll=\"0\""), QStringLiteral("showAll=\"0\""));
        compact.replace(QStringLiteral("compact=\"0\" compactData=\"0\""), QStringLiteral("compactData=\"0\""));
        w = work(compact.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Row Labels"), QStringLiteral("Sum of Amount")}));
        QCOMPARE(w.grid.firstDataRow, 1);
        QString classic = compact;
        classic.replace(QStringLiteral("<pivotTableDefinition "), QStringLiteral("<pivotTableDefinition gridDropZones=\"1\" "));
        w = work(classic.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Sum of Amount"), QString()}));
        QCOMPARE(rowText(w.grid, 1), (QStringList{QStringLiteral("Row Labels"), QStringLiteral("Total")}));
    }

    // Marks that do not agree with the items are what the original program will not open: a date among numbers, a blank among dates, shared strings that differ only in case.
    void theShapeCheckRefusesWhatTheOriginalRefuses()
    {
        const Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        PivotTable placed = w.table;
        pivotApplyGrid(&placed, w.grid, 2, 4);
        const QByteArray def = writePivotCacheDefinition(cacheDefinition(), w.cache, w.cache.recordCount, 45001.5, DateSystem::Date1900);
        const QByteArray rec = writePivotCacheRecords(w.cache, DateSystem::Date1900);
        const QByteArray table = writePivotTable(tableDefinition(true), placed, false);
        QString why;
        QVERIFY2(pivotCheckShape(table, def, rec, &why), qPrintable(why));
        QByteArray d = def;
        d.replace("<s v=\"West\"/>", "<s v=\"east\"/>");
        QVERIFY(!pivotCheckShape(table, d, rec, &why));
        d = def;
        d.replace("<s v=\"West\"/>", "<d v=\"2023-03-15T00:00:00\"/>");
        QVERIFY(!pivotCheckShape(table, d, rec, &why));   // a date item among the strings, with nothing saying so
        d = def;
        d.replace("containsInteger=\"1\"", "containsInteger=\"1\" containsDate=\"1\"");
        QVERIFY(!pivotCheckShape(table, d, rec, &why));
    }

    // A day the calendar has no date for (day 60 of the 1900 system, a negative day) is a number item in the definition, in the records, and in the marks of the field.
    void daysWithNoDateAreWrittenAsNumbers()
    {
        PivotCache c;
        PivotCacheField shared, inlined;
        shared.name = QStringLiteral("S");
        shared.hasShared = true;
        inlined.name = QStringLiteral("I");
        const double serials[] = {60, -5, 45000};
        for (int i = 0; i < 3; ++i) {
            PivotValue v;
            v.kind = PivotValue::Date;
            v.number = serials[i];
            shared.items.append(v);
            inlined.items.append(v);
            shared.records.append(i);
            inlined.records.append(i);
        }
        shared.sharedCount = 3;
        c.fields = {shared, inlined};
        c.recordCount = 3;
        const QByteArray original = "<pivotCacheDefinition xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><cacheSource type=\"worksheet\"><worksheetSource ref=\"A1:B4\" sheet=\"S\"/></cacheSource><cacheFields count=\"2\"/></pivotCacheDefinition>";
        const QByteArray def = writePivotCacheDefinition(original, c, 3, 45000, DateSystem::Date1900);
        QVERIFY2(def.contains("<n v=\"60\"/>") && def.contains("<n v=\"-5\"/>") && def.contains("<d v=\"2023-03-15T00:00:00\"/>"), def.constData());
        QVERIFY2(def.contains("containsMixedTypes=\"1\"") && !def.contains("minValue") && def.contains("minDate=\"2023-03-15T00:00:00\""), def.constData());
        const QByteArray rec = writePivotCacheRecords(c, DateSystem::Date1900);
        QVERIFY2(rec.contains("<n v=\"60\"/>") && rec.contains("<n v=\"-5\"/>") && rec.contains("<d v=\"2023-03-15T00:00:00\"/>"), rec.constData());
        QVERIFY(!rec.contains("1900-"));
    }
};

JP_TEST_CLASS(SheetPivotTests)
#include "tst_sheet_pivot.moc"
