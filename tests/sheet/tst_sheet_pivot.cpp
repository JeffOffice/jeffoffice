// Tests of the pivot table engine: a table worked out from its cache (labels, values, subtotals, grand totals, filters, the three layouts), read again from a
// range of cells (Refresh) and written, and the bounds a hostile file meets. The parts are built in memory.
#include "sheethelpers.h"

#include "sheet/pivot.h"
#include "sheet/xlsxsave.h"

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
        // (the classic grid has the line of the value's name over the header in the tabular form; a table made now has one line)
        QString classic = xml;
        classic.replace(QStringLiteral("<pivotTableDefinition "), QStringLiteral("<pivotTableDefinition gridDropZones=\"1\" "));
        Worked w = work(classic.toUtf8());
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
        const QByteArray xml = QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\" compact=\"0\" gridDropZones=\"1\"><location ref=\"A1:B2\" firstHeaderRow=\"1\" firstDataRow=\"1\" firstDataCol=\"1\"/>"
                                             "<pivotFields count=\"2\"><pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\"><items count=\"%2\">%3<item t=\"default\"/></items></pivotField><pivotField dataField=\"1\"/></pivotFields>"
                                             "<rowFields count=\"1\"><field x=\"0\"/></rowFields><dataFields count=\"1\"><dataField name=\"S\" fld=\"1\"/></dataFields></pivotTableDefinition>")
                                 .arg(QLatin1String(kNs)).arg(n + 1).arg(items).toUtf8();
        PivotTable t;
        QString why;
        QVERIFY2(readPivotTable(xml, &t, &why), qPrintable(why));
        PivotGrid g;
        QVERIFY2(computePivot(t, c, &g, &why), qPrintable(why));
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
        const Worked w = work(tableDefinition(false, QStringLiteral("gridDropZones=\"1\"")), cacheDefinition(), rec);
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
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Sum of Amount"), QStringLiteral("Total")}));
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

    // A total is the sum of the sums of its groups (each added in the order of the records), as the original program has it: 1e16 and 1 in one group, and -1e16 in the other.
    void aTotalIsTheSumOfTheSumsOfItsGroups()
    {
        const QByteArray rec = "<pivotCacheRecords xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"3\">"
                               "<r><x v=\"0\"/><x v=\"0\"/><n v=\"1e16\"/></r><r><x v=\"1\"/><x v=\"0\"/><n v=\"-1e16\"/></r><r><x v=\"0\"/><x v=\"0\"/><n v=\"1\"/></r></pivotCacheRecords>";
        QByteArray def = cacheDefinition();
        def.replace("recordCount=\"6\"", "recordCount=\"3\"");
        const Worked w = work(tableDefinition(false, QStringLiteral("gridDropZones=\"1\"")), def, rec);
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.grid.at(2, 1).number, 1e16);   // East: 1e16 + 1 (the 1 is lost)
        QCOMPARE(w.grid.at(3, 1).number, -1e16);
        QCOMPARE(w.grid.at(4, 1).number, 0.0);    // a flat sum in the order of the records would be 1
    }

    // The classic table with the values across: the caption of the values over them in the compact form too; the format "none" has no line of captions; the last row field has no blank line after its items.
    void theCaptionsOfTheValuesAcrossAndTheFormatNone()
    {
        // two values across, no column field, one row field: Region down the side
        QString xml = QString::fromUtf8(tableDefinition(false));
        xml.replace(QStringLiteral("<dataFields count=\"1\"><dataField name=\"Sum of Amount\" fld=\"2\" baseField=\"0\" baseItem=\"0\"/></dataFields>"),
                    QStringLiteral("<dataFields count=\"2\"><dataField name=\"A\" fld=\"2\" baseField=\"0\" baseItem=\"0\"/><dataField name=\"B\" fld=\"2\" subtotal=\"max\" baseField=\"0\" baseItem=\"0\"/></dataFields>"));
        xml.replace(QStringLiteral("<rowFields count=\"1\"><field x=\"0\"/></rowFields>"), QStringLiteral("<rowFields count=\"1\"><field x=\"0\"/></rowFields><colFields count=\"1\"><field x=\"-2\"/></colFields>"));
        QString classic = xml;
        classic.replace(QStringLiteral("compact=\"0\" outline=\"0\" showAll=\"0\""), QStringLiteral("showAll=\"0\""));
        classic.replace(QStringLiteral("compact=\"0\" compactData=\"0\""), QStringLiteral("compactData=\"0\" gridDropZones=\"1\""));
        Worked w = work(classic.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(rowText(w.grid, 0), (QStringList{QString(), QStringLiteral("Values"), QString()}));
        QCOMPARE(rowText(w.grid, 1), (QStringList{QStringLiteral("Row Labels"), QStringLiteral("A"), QStringLiteral("B")}));
        QCOMPARE(w.grid.firstDataRow, 2);
        // the same with no classic grid (a table made now): the compact form has the caption of the values over them as well
        QString modern = classic;
        modern.replace(QStringLiteral(" gridDropZones=\"1\""), QString());
        w = work(modern.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(rowText(w.grid, 0), (QStringList{QString(), QStringLiteral("Values"), QString()}));
        QCOMPARE(rowText(w.grid, 1), (QStringList{QStringLiteral("Row Labels"), QStringLiteral("A"), QStringLiteral("B")}));
        QCOMPARE(w.grid.firstDataRow, 2);
        // the format none: the one line of captions
        QString none = xml;
        none.replace(QStringLiteral("<pivotTableDefinition "), QStringLiteral("<pivotTableDefinition autoFormatId=\"4097\" gridDropZones=\"1\" "));
        w = work(none.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(rowText(w.grid, 0), (QStringList{QStringLiteral("Region"), QStringLiteral("A"), QStringLiteral("B")}));
        QCOMPARE(w.grid.firstDataRow, 1);
        QCOMPARE(w.grid.firstHeaderRow, 0);
        // a blank line after each item, asked of the only row field: none
        QString blank = xml;
        blank.replace(QStringLiteral("<pivotField axis=\"axisRow\" "), QStringLiteral("<pivotField axis=\"axisRow\" insertBlankRow=\"1\" "));
        w = work(blank.toUtf8());
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(rowText(w.grid, 3).value(0), QStringLiteral("West"));
    }

    // What the original program will not open: a text of more than 255 characters among the shared items; a selection of the view outside the lines of the table.
    void aLongSharedItemAndAStaleSelectionAreRefused()
    {
        const Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        PivotTable placed = w.table;
        pivotApplyGrid(&placed, w.grid, 2, 4);
        const QByteArray def = writePivotCacheDefinition(cacheDefinition(), w.cache, w.cache.recordCount, 45001.5, DateSystem::Date1900);
        const QByteArray rec = writePivotCacheRecords(w.cache, DateSystem::Date1900);
        const QByteArray table = writePivotTable(tableDefinition(true), placed, false);
        QString why;
        QByteArray d = def;
        d.replace("<s v=\"West\"/>", "<s v=\"" + QByteArray(256, 'w') + "\"/>");
        QVERIFY(!pivotCheckShape(table, d, rec, &why));
        d = def;
        d.replace("<s v=\"West\"/>", "<s v=\"" + QByteArray(255, 'w') + "\"/>");
        QVERIFY2(pivotCheckShape(table, d, rec, &why), qPrintable(why));
        const int lines = QString::fromUtf8(table).count(QStringLiteral("<i")) ;
        Q_UNUSED(lines)
        QVERIFY2(pivotCheckSelection("<pivotSelection axis=\"axisRow\" start=\"1\" max=\"3\"/>", table, &why), qPrintable(why));
        QVERIFY(!pivotCheckSelection("<pivotSelection axis=\"axisRow\" start=\"1\" max=\"9\"/>", table, &why));
        QVERIFY(!pivotCheckSelection("<pivotSelection axis=\"axisRow\" start=\"9\" max=\"9\"/>", table, &why));
        QVERIFY(pivotCheckSelection("<pivotSelection axis=\"axisCol\" start=\"9\" max=\"99\"/>", table, &why));   // (columns: files of the original program have them outside)
    }

    // Hostile parts: a field that lists a million subtotal lines gets one of each function, and more fields on an axis than the work allows are refused.
    void subtotalListsAndLevelsAreBounded()
    {
        PivotCache c;
        const int n = 66;
        c.recordCount = 1;
        for (int f = 0; f < n; ++f) {
            PivotCacheField cf;
            cf.name = QStringLiteral("F%1").arg(f);
            cf.hasShared = true;
            cf.sharedCount = 1;
            PivotValue v;
            v.kind = PivotValue::Text;
            v.text = QStringLiteral("a");
            cf.items.append(v);
            cf.records.append(0);
            c.fields.append(cf);
        }
        auto tableOf = [&](int rows, const QString &subtotals) {
            QString fields, rowList;
            for (int f = 0; f < n; ++f) {
                fields += f < rows ? QStringLiteral("<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\"><items count=\"2\"><item x=\"0\"/><item t=\"default\"/></items></pivotField>")
                                   : QStringLiteral("<pivotField dataField=\"1\"/>");
                if (f < rows) rowList += QStringLiteral("<field x=\"%1\"/>").arg(f);
            }
            if (!subtotals.isEmpty()) fields.replace(QStringLiteral("<item t=\"default\"/>"), subtotals);
            return QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\" compact=\"0\"><location ref=\"A1:B2\" firstHeaderRow=\"1\" firstDataRow=\"1\" firstDataCol=\"1\"/>"
                                  "<pivotFields count=\"%2\">%3</pivotFields><rowFields count=\"%4\">%5</rowFields><dataFields count=\"1\"><dataField name=\"S\" fld=\"%6\" subtotal=\"count\"/></dataFields></pivotTableDefinition>")
                .arg(QLatin1String(kNs)).arg(n).arg(fields).arg(rows).arg(rowList).arg(n - 1).toUtf8();
        };
        PivotTable t;
        PivotGrid g;
        QString why;
        // 65 fields on the rows: refused
        QVERIFY2(readPivotTable(tableOf(65, QString()), &t, &why), qPrintable(why));
        QVERIFY(!computePivot(t, c, &g, &why));
        QVERIFY2(why.contains(QLatin1String("axis")), qPrintable(why));
        // the same with 3: worked out, and a list of 100,000 subtotal lines of one function is one line
        QString many = QStringLiteral("<item t=\"sum\"/>");
        QString list;
        for (int i = 0; i < 100000; ++i) list += many;
        QVERIFY2(readPivotTable(tableOf(3, list), &t, &why), qPrintable(why));
        QVERIFY2(computePivot(t, c, &g, &why), qPrintable(why));
        QVERIFY2(g.rows < 20, qPrintable(QString::number(g.rows)));
    }

    // Subtotals of nested fields: each group has its line after it, the inner group's before the outer's.
    void nestedGroupsHaveTheirSubtotals()
    {
        PivotCache c;
        c.recordCount = 4;
        auto text = [](const char *s) {
            PivotValue v;
            v.kind = PivotValue::Text;
            v.text = QString::fromLatin1(s);
            return v;
        };
        auto fieldOf = [&](const char *name, std::initializer_list<const char *> items, std::initializer_list<int> recs) {
            PivotCacheField f;
            f.name = QString::fromLatin1(name);
            f.hasShared = true;
            for (const char *i : items) f.items.append(text(i));
            f.sharedCount = int(f.items.size());
            for (int r : recs) f.records.append(r);
            return f;
        };
        c.fields.append(fieldOf("A", {"x", "y"}, {0, 0, 0, 1}));
        c.fields.append(fieldOf("B", {"p", "q"}, {0, 0, 1, 0}));
        c.fields.append(fieldOf("C", {"m", "n"}, {0, 1, 0, 0}));
        PivotCacheField amount;
        amount.name = QStringLiteral("N");
        const double nums[] = {1, 2, 4, 8};
        for (int i = 0; i < 4; ++i) {
            PivotValue v;
            v.kind = PivotValue::Number;
            v.number = nums[i];
            amount.items.append(v);
            amount.records.append(i);
        }
        c.fields.append(amount);
        QString fields;
        for (int f = 0; f < 3; ++f)
            fields += QStringLiteral("<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\"><items count=\"%1\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField>").arg(3);
        const QByteArray xml = QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\" compact=\"0\"><location ref=\"A1:B2\" firstHeaderRow=\"1\" firstDataRow=\"1\" firstDataCol=\"3\"/>"
                                             "<pivotFields count=\"4\">%2<pivotField dataField=\"1\"/></pivotFields><rowFields count=\"3\"><field x=\"0\"/><field x=\"1\"/><field x=\"2\"/></rowFields>"
                                             "<dataFields count=\"1\"><dataField name=\"S\" fld=\"3\"/></dataFields></pivotTableDefinition>").arg(QLatin1String(kNs), fields).toUtf8();
        PivotTable t;
        PivotGrid g;
        QString why;
        QVERIFY2(readPivotTable(xml, &t, &why), qPrintable(why));
        QVERIFY2(computePivot(t, c, &g, &why), qPrintable(why));
        // a table made now, in the tabular form, with one value and nothing across: one line of names, the value's name last (no line of captions, no \"Total\")
        QCOMPARE(rowText(g, 0), (QStringList{QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("S")}));
        QCOMPARE(g.firstDataRow, 1);
        QStringList lines;
        for (int r = g.firstDataRow; r < g.rows; ++r) lines << rowText(g, r).join(QLatin1Char('|'));
        QCOMPARE(lines, (QStringList{QStringLiteral("x|p|m|1"), QStringLiteral("||n|2"), QStringLiteral("|p Total||3"), QStringLiteral("|q|m|4"), QStringLiteral("|q Total||4"), QStringLiteral("x Total|||7"),
                                     QStringLiteral("y|p|m|8"), QStringLiteral("|p Total||8"), QStringLiteral("y Total|||8"), QStringLiteral("Grand Total|||15")}));
        // GETPIVOTDATA over the same table: a leaf, the subtotals of the two outer fields, and the grand total; items listed in another order than the cache has them; an item that is hidden
        auto pair = [&](const char *field, const char *item) {
            PivotValue v;
            v.kind = PivotValue::Text;
            v.text = QString::fromLatin1(item);
            return std::make_pair(QString::fromLatin1(field), v);
        };
        auto lookupOf = [&](const PivotTable &table) {
            auto grid = std::make_shared<PivotGrid>();
            QString problem;
            return computePivot(table, c, grid.get(), &problem) ? std::make_shared<PivotLookup>(std::make_shared<PivotTable>(table), std::make_shared<PivotCache>(c), grid) : std::shared_ptr<PivotLookup>();
        };
        PivotCell cell;
        auto lookup = lookupOf(t);
        QVERIFY(lookup);
        QVERIFY(lookup->find(QStringLiteral("S"), {pair("A", "x"), pair("B", "p"), pair("C", "n")}, &cell));
        QCOMPARE(cell.number, 2.0);
        QVERIFY(lookup->find(QStringLiteral("S"), {pair("A", "x"), pair("B", "p")}, &cell));
        QCOMPARE(cell.number, 3.0);
        QVERIFY(lookup->find(QStringLiteral("S"), {pair("A", "x")}, &cell));
        QCOMPARE(cell.number, 7.0);
        QVERIFY(lookup->find(QStringLiteral("S"), {}, &cell));
        QCOMPARE(cell.number, 15.0);
        QVERIFY(!lookup->find(QStringLiteral("S"), {pair("A", "y"), pair("B", "q")}, &cell));   // (there is no such group)
        QVERIFY(!lookup->find(QStringLiteral("S"), {pair("B", "p")}, &cell));                   // (B without A: not a group the rows give)
        PivotTable reordered = t;
        std::swap(reordered.fields[1].items[0], reordered.fields[1].items[1]);   // q before p
        lookup = lookupOf(reordered);
        QVERIFY(lookup);
        QVERIFY(lookup->find(QStringLiteral("S"), {pair("A", "x"), pair("B", "p")}, &cell));
        QCOMPARE(cell.number, 3.0);
        QVERIFY(lookup->find(QStringLiteral("S"), {pair("A", "x"), pair("B", "q")}, &cell));
        QCOMPARE(cell.number, 4.0);
        PivotTable hidden = t;
        hidden.fields[1].items[0].hidden = true;   // p
        lookup = lookupOf(hidden);
        QVERIFY(lookup);
        QVERIFY(!lookup->find(QStringLiteral("S"), {pair("A", "x"), pair("B", "p")}, &cell));
        QVERIFY(lookup->find(QStringLiteral("S"), {pair("A", "x"), pair("B", "q")}, &cell));
        QCOMPARE(cell.number, 4.0);
        QVERIFY(lookup->find(QStringLiteral("S"), {pair("A", "x")}, &cell));
        QCOMPARE(cell.number, 4.0);   // (the total leaves out what is hidden)
    }

    // A value renamed and given another function in one step keeps the sort by it; a filter on a field with no list of items is kept when the data is read again.
    void aRenamedValueKeepsItsSortAndAFilterWithNoItemsIsKept()
    {
        const Worked w = work(tableDefinition(false, QString(), QString(), QString(), QStringLiteral("<pageFields count=\"1\"><pageField fld=\"1\" item=\"1\" hier=\"-1\"/></pageFields>")));
        QVERIFY2(w.ok, qPrintable(w.why));
        QCOMPARE(w.table.pageFields.size(), 1);
        QCOMPARE(w.table.pageFields[0].item, 1);
        PivotTable fresh = w.table;
        PivotCache next = w.cache;
        next.remap.resize(next.fields.size());
        next.fromField.clear();
        for (int f = 0; f < next.fields.size(); ++f) {
            next.fromField.append(f);
            for (int i = 0; i < next.fields[f].items.size(); ++i) next.remap[f].append(i);
        }
        matchPivotItems(&fresh, w.cache, next);
        QCOMPARE(fresh.pageFields.size(), 1);
        QCOMPARE(fresh.pageFields[0].item, 1);
        PivotTable old = w.table, now = w.table;
        old.fields[0].sortByData = 0;
        now.fields[0].sortByData = 0;
        old.dataFields[0].name = QStringLiteral("Sum of Amount");
        now.dataFields[0].name = QStringLiteral("Mean");
        now.dataFields[0].subtotal = QStringLiteral("average");
        pivotRemapDataFields(old, &now);
        QCOMPARE(now.fields[0].sortByData, 0);
    }

    // GETPIVOTDATA: a value of the table by its data field and the items of its lines; the totals when fewer fields are named; nothing for what the table does not show; and the arguments of a cell.
    void aValueOfTheTableIsFoundByItsItemsAndItsArgumentsAreMade()
    {
        const Worked w = work(tableDefinition(true));
        QVERIFY2(w.ok, qPrintable(w.why));
        auto text = [](const char *s) {
            PivotValue v;
            v.kind = PivotValue::Text;
            v.text = QString::fromLatin1(s);
            return v;
        };
        PivotCell cell;
        QVERIFY(pivotGetData(w.table, w.cache, w.grid, QStringLiteral("Sum of Amount"), {{QStringLiteral("Region"), text("East")}, {QStringLiteral("Month"), text("Jan")}}, &cell));
        QCOMPARE(cell.number, 15.0);
        // in the other order of the fields, in other case, and by the name of the field the value adds up
        QVERIFY(pivotGetData(w.table, w.cache, w.grid, QStringLiteral("amount"), {{QStringLiteral("month"), text("feb")}, {QStringLiteral("REGION"), text("west")}}, &cell));
        QCOMPARE(cell.number, 7.0);
        // the total of a region, of a month, and of all
        QVERIFY(pivotGetData(w.table, w.cache, w.grid, QStringLiteral("Sum of Amount"), {{QStringLiteral("Region"), text("East")}}, &cell));
        QCOMPARE(cell.number, 35.0);
        QVERIFY(pivotGetData(w.table, w.cache, w.grid, QStringLiteral("Sum of Amount"), {}, &cell));
        QCOMPARE(cell.number, 49.0);
        // what is not shown: an item that is not there, a field that is not in the table, a month named without its region (not a group the rows give), another value
        QVERIFY(!pivotGetData(w.table, w.cache, w.grid, QStringLiteral("Sum of Amount"), {{QStringLiteral("Region"), text("North")}}, &cell));
        QVERIFY(!pivotGetData(w.table, w.cache, w.grid, QStringLiteral("Sum of Amount"), {{QStringLiteral("Color"), text("Red")}}, &cell));
        QVERIFY(!pivotGetData(w.table, w.cache, w.grid, QStringLiteral("Sum of Price"), {}, &cell));
        // the arguments of the cell East, Jan; of the grand total; of the total of West
        QString data;
        std::vector<std::pair<QString, PivotValue>> items;
        QVERIFY(pivotGetDataArguments(w.table, w.cache, w.grid, w.grid.firstDataRow, w.grid.firstDataCol, &data, &items));
        QCOMPARE(data, QStringLiteral("Sum of Amount"));
        QCOMPARE(int(items.size()), 2);
        QCOMPARE(items[0].first, QStringLiteral("Region"));
        QCOMPARE(items[0].second.text, QStringLiteral("East"));
        QCOMPARE(items[1].first, QStringLiteral("Month"));
        QCOMPARE(items[1].second.text, QStringLiteral("Jan"));
        QVERIFY(pivotGetDataArguments(w.table, w.cache, w.grid, w.grid.rows - 1, w.grid.cols - 1, &data, &items));
        QVERIFY(items.empty());
        QVERIFY(pivotGetDataArguments(w.table, w.cache, w.grid, w.grid.firstDataRow + 1, w.grid.cols - 1, &data, &items));
        QCOMPARE(int(items.size()), 1);
        QCOMPARE(items[0].second.text, QStringLiteral("West"));
        // a label is not a value
        QVERIFY(!pivotGetDataArguments(w.table, w.cache, w.grid, 0, 0, &data, &items));
    }

    // The list of pivot caches in a workbook that writes its elements with a prefix, and binds the relationships to another prefix: the new elements follow the file's own.
    void newPivotCachesTakeTheFilesPrefixes()
    {
        const QByteArray main = "http://schemas.openxmlformats.org/spreadsheetml/2006/main", rel = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
        QByteArray out;
        // a list there already, with prefixes
        const QByteArray prefixed = "<x:workbook xmlns:x=\"" + main + "\" xmlns:rel=\"" + rel + "\"><x:sheets/><x:calcPr/><x:pivotCaches><x:pivotCache cacheId=\"0\" rel:id=\"rId1\"/></x:pivotCaches></x:workbook>";
        QVERIFY(addPivotCaches(prefixed, {{5, QStringLiteral("rIdNew")}}, &out));
        QVERIFY2(out.contains("<x:pivotCache cacheId=\"5\" rel:id=\"rIdNew\"/></x:pivotCaches>"), out.constData());
        // no list, the relationships bound to "r": made after the calculation settings, before what follows
        const QByteArray plain = "<workbook xmlns=\"" + main + "\" xmlns:r=\"" + rel + "\"><sheets/><calcPr/><extLst/></workbook>";
        QVERIFY(addPivotCaches(plain, {{2, QStringLiteral("rIdA")}, {3, QStringLiteral("rIdB")}}, &out));
        QVERIFY2(out.contains("<calcPr/><pivotCaches><pivotCache cacheId=\"2\" r:id=\"rIdA\"/><pivotCache cacheId=\"3\" r:id=\"rIdB\"/></pivotCaches><extLst/>"), out.constData());
        // no list, and the relationships bound to nothing: declared where they are used
        const QByteArray unbound = "<workbook xmlns=\"" + main + "\"><sheets/><calcPr/></workbook>";
        QVERIFY(addPivotCaches(unbound, {{1, QStringLiteral("rIdZ")}}, &out));
        QVERIFY2(out.contains("<pivotCache cacheId=\"1\" r:id=\"rIdZ\" xmlns:r=\"" + rel + "\"/></pivotCaches></workbook>"), out.constData());
        // and a prefix that is not "r", with no list
        const QByteArray other = "<workbook xmlns=\"" + main + "\" xmlns:q=\"" + rel + "\"><sheets/><calcPr/></workbook>";
        QVERIFY(addPivotCaches(other, {{1, QStringLiteral("rIdY")}}, &out));
        QVERIFY2(out.contains("q:id=\"rIdY\""), out.constData());
    }

    // GETPIVOTDATA with items that are dates, numbers, accents, and text that reads as a number or a date; and the time many asks take on a table of ten thousand lines.
    void itemsAreMatchedByWhatTheySay()
    {
        PivotCache c;
        c.recordCount = 3;
        auto field = [](const char *name, QVector<PivotValue> items, QVector<qint32> recs) {
            PivotCacheField f;
            f.name = QString::fromLatin1(name);
            f.hasShared = true;
            f.items = items;
            f.sharedCount = int(items.size());
            f.records = recs;
            return f;
        };
        auto date = [](double d) {
            PivotValue v;
            v.kind = PivotValue::Date;
            v.number = d;
            return v;
        };
        auto number = [](double d) {
            PivotValue v;
            v.kind = PivotValue::Number;
            v.number = d;
            return v;
        };
        auto text = [](const QString &t) {
            PivotValue v;
            v.kind = PivotValue::Text;
            v.text = t;
            return v;
        };
        c.fields.append(field("Day", {date(36906), date(36907), date(36908)}, {0, 1, 2}));          // 2001-01-15, -16, -17
        c.fields.append(field("Rate", {number(0.3), number(1.5), number(2)}, {0, 1, 2}));
        c.fields.append(field("Name", {text(QStringLiteral("Zoë")), text(QStringLiteral("Zoe")), text(QStringLiteral("2001"))}, {0, 1, 2}));
        PivotCacheField amount;
        amount.name = QStringLiteral("Amount");
        for (int i = 0; i < 3; ++i) {
            amount.items.append(number(10 * (i + 1)));
            amount.records.append(i);
        }
        c.fields.append(amount);
        auto tableFor = [&](int rowField) {
            QString fields;
            for (int f = 0; f < 3; ++f)
                fields += f == rowField ? QStringLiteral("<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\"><items count=\"4\"><item x=\"2\"/><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField>")
                                       : QStringLiteral("<pivotField/>");
            const QByteArray xml = QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\" compact=\"0\"><location ref=\"A1:B2\" firstHeaderRow=\"1\" firstDataRow=\"1\" firstDataCol=\"1\"/>"
                                                 "<pivotFields count=\"4\">%2<pivotField dataField=\"1\"/></pivotFields><rowFields count=\"1\"><field x=\"%3\"/></rowFields>"
                                                 "<dataFields count=\"1\"><dataField name=\"S\" fld=\"3\"/></dataFields></pivotTableDefinition>").arg(QLatin1String(kNs), fields).arg(rowField).toUtf8();
            auto t = std::make_shared<PivotTable>();
            QString why;
            if (!readPivotTable(xml, t.get(), &why)) return std::shared_ptr<PivotLookup>();
            auto g = std::make_shared<PivotGrid>();
            if (!computePivot(*t, c, g.get(), &why)) return std::shared_ptr<PivotLookup>();
            return std::make_shared<PivotLookup>(t, std::make_shared<PivotCache>(c), g);
        };
        PivotCell cell;
        // dates: the serial number, a date written as text (with the reading the engine gives it), and a text that is not that date
        auto days = tableFor(0);
        QVERIFY(days);
        QVERIFY(days->find(QStringLiteral("S"), {{QStringLiteral("Day"), number(36907)}}, &cell));   // (the items are listed in another order than the cache has them: x = 2, 0, 1)
        QCOMPARE(cell.number, 20.0);
        PivotValue written = text(QStringLiteral("1/17/2001"));
        written.reading = true;
        written.number = 36908;
        QVERIFY(days->find(QStringLiteral("S"), {{QStringLiteral("Day"), written}}, &cell));
        QCOMPARE(cell.number, 30.0);
        QVERIFY(!days->find(QStringLiteral("S"), {{QStringLiteral("Day"), text(QStringLiteral("1/17/2001"))}}, &cell));   // (no reading: it is only a text)
        // numbers: the same to 15 digits is the same (0.1 + 0.2 is 0.3), and a text that reads as the number is that item
        auto rates = tableFor(1);
        QVERIFY(rates);
        QVERIFY(rates->find(QStringLiteral("S"), {{QStringLiteral("Rate"), number(0.1 + 0.2)}}, &cell));
        QCOMPARE(cell.number, 10.0);
        PivotValue one = text(QStringLiteral("1.5"));
        one.reading = true;
        one.number = 1.5;
        QVERIFY(rates->find(QStringLiteral("S"), {{QStringLiteral("Rate"), one}}, &cell));
        QCOMPARE(cell.number, 20.0);
        // texts: without regard to case, with regard to an accent; a number is the text of that number
        auto names = tableFor(2);
        QVERIFY(names);
        QVERIFY(names->find(QStringLiteral("S"), {{QStringLiteral("Name"), text(QStringLiteral("ZOË"))}}, &cell));
        QCOMPARE(cell.number, 10.0);
        QVERIFY(names->find(QStringLiteral("S"), {{QStringLiteral("Name"), text(QStringLiteral("zoe"))}}, &cell));
        QCOMPARE(cell.number, 20.0);
        QVERIFY(names->find(QStringLiteral("S"), {{QStringLiteral("Name"), number(2001)}}, &cell));
        QCOMPARE(cell.number, 30.0);
    }

    // Ten thousand asks over a table of ten thousand lines (the time is told, not held to a number: the check is that each ask is a few lookups, so the count of lines is not in it).
    void manyAsksOverAnEnormousTableAreCheap()
    {
        const int n = 10000;
        PivotCache c;
        c.recordCount = n;
        PivotCacheField id, amount;
        id.name = QStringLiteral("Id");
        id.hasShared = true;
        amount.name = QStringLiteral("Amount");
        for (int i = 0; i < n; ++i) {
            PivotValue v;
            v.kind = PivotValue::Text;
            v.text = QStringLiteral("item %1").arg(i);
            id.items.append(v);
            id.records.append(i);
            PivotValue a;
            a.kind = PivotValue::Number;
            a.number = i;
            amount.items.append(a);
            amount.records.append(i);
        }
        id.sharedCount = n;
        c.fields = {id, amount};
        const QByteArray xml = QStringLiteral("<pivotTableDefinition xmlns=\"%1\" name=\"t\" cacheId=\"1\" compact=\"0\"><location ref=\"A1:B2\" firstHeaderRow=\"1\" firstDataRow=\"1\" firstDataCol=\"1\"/>"
                                             "<pivotFields count=\"2\"><pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\"/><pivotField dataField=\"1\"/></pivotFields><rowFields count=\"1\"><field x=\"0\"/></rowFields>"
                                             "<dataFields count=\"1\"><dataField name=\"S\" fld=\"1\"/></dataFields></pivotTableDefinition>").arg(QLatin1String(kNs)).toUtf8();
        auto t = std::make_shared<PivotTable>();
        QString why;
        QVERIFY2(readPivotTable(xml, t.get(), &why), qPrintable(why));
        auto g = std::make_shared<PivotGrid>();
        QVERIFY2(computePivot(*t, c, g.get(), &why), qPrintable(why));
        QElapsedTimer timer;
        timer.start();
        PivotLookup lookup(t, std::make_shared<PivotCache>(c), g);
        const qint64 made = timer.elapsed();
        PivotCell cell;
        for (int i = 0; i < n; ++i) {
            PivotValue v;
            v.kind = PivotValue::Text;
            v.text = QStringLiteral("item %1").arg(n - 1 - i);
            QVERIFY(lookup.find(QStringLiteral("S"), {{QStringLiteral("Id"), v}}, &cell));
            QCOMPARE(cell.number, double(n - 1 - i));
        }
        qInfo("GETPIVOTDATA: lookup made in %lld ms, %d asks in %lld ms", made, n, timer.elapsed() - made);
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
