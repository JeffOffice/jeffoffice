// Tests of the model of a table (sheet/tables.h, tablestyle.h, structref.h): the part read into it and written from it (a part that is not changed is not touched; one that is keeps what
// the program has no model of), a table made here and saved with its part, relationship, and type, a table taken out, structured references read into cells and worked out by the engine,
// the looks of the built-in styles, and the names a table may have. Every workbook is built in memory.
#include "sheethelpers.h"

#include "io/zipindex.h"
#include "sheet/calculation.h"
#include "sheet/structref.h"
#include "sheet/tables.h"
#include "sheet/tablestyle.h"
#include "sheet/xlsxsave.h"

namespace {

const char *const kNs = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

QString inlineCell(const QString &ref, const QString &text) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, text.toHtmlEscaped()); }
QString numberCell(const QString &ref, double v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref).arg(v); }
QString formulaCell(const QString &ref, const QString &f) { return QStringLiteral("<c r=\"%1\"><f>%2</f></c>").arg(ref, f.toHtmlEscaped()); }

// A table part with a header row (A1:C1), four data rows, and a row of totals under them (A1:C6); extra things the program keeps no model of in it.
QString salesPart()
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" xmlns:x14=\"urn:x14\" id=\"7\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C6\" totalsRowCount=\"1\" insertRow=\"0\">"
                          "<autoFilter ref=\"A1:C5\"><filterColumn colId=\"1\"><filters><filter val=\"2\"/></filters></filterColumn></autoFilter>"
                          "<tableColumns count=\"3\"><tableColumn id=\"1\" name=\"Region\" totalsRowLabel=\"Total\" dataDxfId=\"4\"/>"
                          "<tableColumn id=\"2\" name=\"Q1\" totalsRowFunction=\"sum\"/>"
                          "<tableColumn id=\"3\" name=\"Double\" totalsRowFunction=\"custom\"><calculatedColumnFormula>Sales[[#This Row],[Q1]]*2</calculatedColumnFormula><totalsRowFormula>SUM(Sales[Double])</totalsRowFormula></tableColumn>"
                          "</tableColumns><tableStyleInfo name=\"TableStyleMedium2\" showFirstColumn=\"1\" showLastColumn=\"0\" showRowStripes=\"1\" showColumnStripes=\"0\"/>"
                          "<extLst><ext uri=\"{x}\"><x14:note/></ext><ext uri=\"{y}\"><x14:table altTextSummary=\"s\"/></ext></extLst></table>")
        .arg(QLatin1String(kNs));
}

QByteArray salesBook(const QString &part = salesPart())
{
    QString rows = QStringLiteral("<row r=\"1\">%1%2%3</row>").arg(inlineCell(QStringLiteral("A1"), QStringLiteral("Region")), inlineCell(QStringLiteral("B1"), QStringLiteral("Q1")), inlineCell(QStringLiteral("C1"), QStringLiteral("Double")));
    const char *regions[] = {"East", "West", "North", "South"};
    for (int i = 0; i < 4; ++i)
        rows += QStringLiteral("<row r=\"%1\">%2%3%4</row>").arg(i + 2).arg(inlineCell(QStringLiteral("A%1").arg(i + 2), QLatin1String(regions[i])), numberCell(QStringLiteral("B%1").arg(i + 2), i + 1),
                                                                      formulaCell(QStringLiteral("C%1").arg(i + 2), QStringLiteral("Sales[[#This Row],[Q1]]*2")));
    rows += QStringLiteral("<row r=\"6\">%1%2%3</row>").arg(inlineCell(QStringLiteral("A6"), QStringLiteral("Total")), formulaCell(QStringLiteral("B6"), QStringLiteral("SUBTOTAL(109,Sales[Q1])")),
                                                            formulaCell(QStringLiteral("C6"), QStringLiteral("SUM(Sales[Double])")));
    rows += QStringLiteral("<row r=\"8\">%1</row>").arg(formulaCell(QStringLiteral("A8"), QStringLiteral("SUM(Sales[Q1])+COUNTA(Sales[[#Headers],[Region]:[Double]])+ROWS(Sales[#All])")));
    rows += QStringLiteral("<row r=\"9\">%1</row>").arg(formulaCell(QStringLiteral("A9"), QStringLiteral("ROWS(Sales[#All])")));
    return withTables(buildXlsx({worksheet(rows)}), {part});
}

struct Open {
    Package p;
    Workbook b;
    bool ok(const QByteArray &zip)
    {
        QString why;
        return p.readLazy(zip, &why) && b.open(p, &why);
    }
};

CellRange rangeOf(const QString &a1)
{
    CellRange r;
    parseRangeRef(a1, &r);
    return r;
}

} // namespace

class SheetTableModelTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The part is read into the model: id, style and marks, the filter, and what each column says of its totals and its formula.
    void thePartIsReadIntoTheModel()
    {
        Open o;
        QVERIFY(o.ok(salesBook()));
        QCOMPARE(o.b.tables().size(), 1);
        const Table &t = o.b.tables().first();
        QCOMPARE(t.id, 7);
        QCOMPARE(t.styleName, QStringLiteral("TableStyleMedium2"));
        QVERIFY(t.showFirstColumn && !t.showLastColumn && t.showRowStripes && !t.showColumnStripes);
        QVERIFY(t.filterButtons);
        QCOMPARE(t.filter.columns.size(), 1);
        QCOMPARE(int(t.filter.columns[0].column), 1);
        QCOMPARE(t.filter.columns[0].values, QStringList{QStringLiteral("2")});
        QCOMPARE(t.info.size(), 3);
        QCOMPARE(t.info[0].totalsLabel, QStringLiteral("Total"));
        QCOMPARE(t.info[1].totalsFunction, QStringLiteral("sum"));
        QCOMPARE(t.info[2].calcFormula, QStringLiteral("Sales[[#This Row],[Q1]]*2"));
        QCOMPARE(t.info[2].totalsFormula, QStringLiteral("SUM(Sales[Double])"));
        QCOMPARE(int(t.dataFirst()), 1);
        QCOMPARE(int(t.dataLast()), 4);
    }

    // A part nobody changed is not written, however many other things are saved; one that changed keeps the elements and attributes the program has no model of.
    void anUntouchedPartIsNotWrittenAndAChangedOneKeepsWhatItDoesNotKnow()
    {
        Open o;
        QVERIFY(o.ok(salesBook()));
        QVERIFY(o.b.changedTableParts().empty());
        Table t = o.b.tables().first();
        t.styleName = QStringLiteral("TableStyleLight9");
        t.showColumnStripes = true;
        t.info[1].totalsFunction = QStringLiteral("average");
        o.b.setTableAt(0, t);
        const std::map<QString, QByteArray> changed = o.b.changedTableParts();
        QCOMPARE(int(changed.size()), 1);
        const QString xml = QString::fromUtf8(changed.at(QStringLiteral("/xl/tables/table1.xml")));
        QVERIFY2(xml.contains(QStringLiteral("name=\"TableStyleLight9\"")) && xml.contains(QStringLiteral("showColumnStripes=\"1\"")) && xml.contains(QStringLiteral("totalsRowFunction=\"average\"")), qPrintable(xml));
        QVERIFY2(xml.contains(QStringLiteral("<x14:note/>")) && xml.contains(QStringLiteral("insertRow=\"0\"")) && xml.contains(QStringLiteral("dataDxfId=\"4\"")) && xml.contains(QStringLiteral("xmlns:x14=\"urn:x14\"")), qPrintable(xml));
        QVERIFY2(xml.contains(QStringLiteral("<filter val=\"2\"/>")) && xml.contains(QStringLiteral("<totalsRowFormula>SUM(Sales[Double])</totalsRowFormula>")), qPrintable(xml));
        // what is written reads back as the model
        Table back;
        back.columns = t.columns;
        back.name = t.name;
        QVERIFY(readTableModel(changed.begin()->second, &back));
        QCOMPARE(back.styleName, t.styleName);
        QVERIFY(back.showColumnStripes);
        QCOMPARE(back.info[1].totalsFunction, QStringLiteral("average"));
        QCOMPARE(back.info[2].calcFormula, t.info[2].calcFormula);
        // put back, it is the file's again
        o.b.setTableAt(0, o.b.tables().first());
        Table same = t;
        same.styleName = QStringLiteral("TableStyleMedium2");
        same.showColumnStripes = false;
        same.info[1].totalsFunction = QStringLiteral("sum");
        o.b.setTableAt(0, same);
        QVERIFY(o.b.changedTableParts().empty());
    }

    // A column made here (no place in the old part), a column taken out, a totals row on, and the filter's conditions: the part is the model.
    void columnsAndTotalsAreWrittenFromTheModel()
    {
        Open o;
        QVERIFY(o.ok(salesBook()));
        Table t = o.b.tables().first();
        t.columns.removeAt(1);
        t.info.removeAt(1);
        t.columns.append(QStringLiteral("Notes"));
        TableColumnInfo n;
        n.id = 8;
        n.totalsFunction = QStringLiteral("count");
        t.info.append(n);
        t.col2 = 2;
        t.filter.columns.clear();
        o.b.setTableAt(0, t);
        const QString xml = QString::fromUtf8(o.b.changedTableParts().begin()->second);
        QVERIFY2(xml.contains(QStringLiteral("<tableColumns count=\"3\">")) && !xml.contains(QStringLiteral("name=\"Q1\"")) && xml.contains(QStringLiteral("name=\"Notes\"")), qPrintable(xml));
        QVERIFY2(xml.contains(QStringLiteral("id=\"8\"")) && xml.contains(QStringLiteral("totalsRowFunction=\"count\"")) && !xml.contains(QStringLiteral("<filterColumn")), qPrintable(xml));
        QVERIFY2(xml.contains(QStringLiteral("ref=\"A1:C6\"")) && xml.contains(QStringLiteral("<autoFilter ref=\"A1:C5\"/>")), qPrintable(xml));
        // the order the format has: filter, columns, style, extensions
        QVERIFY(xml.indexOf(QStringLiteral("<autoFilter")) < xml.indexOf(QStringLiteral("<tableColumns")) && xml.indexOf(QStringLiteral("<tableColumns")) < xml.indexOf(QStringLiteral("<tableStyleInfo"))
                && xml.indexOf(QStringLiteral("<tableStyleInfo")) < xml.indexOf(QStringLiteral("<extLst")));
    }

    // The formula a total holds follows the function chosen: the number is the kind that leaves out hidden rows, the name has its marks escaped.
    void aTotalIsASubtotalOfItsColumn()
    {
        Table t;
        t.name = QStringLiteral("Orders");
        t.columns = {QStringLiteral("Price"), QStringLiteral("Unit #"), QStringLiteral("Cost, net")};
        t.info.resize(3);
        t.info[0].totalsFunction = QStringLiteral("sum");
        t.info[1].totalsFunction = QStringLiteral("max");
        t.info[2].totalsFunction = QStringLiteral("stdDev");
        QCOMPARE(totalsFormulaOf(t, 0), QStringLiteral("SUBTOTAL(109,Orders[Price])"));
        QCOMPARE(totalsFormulaOf(t, 1), QStringLiteral("SUBTOTAL(104,Orders[[Unit '#]])"));
        QCOMPARE(totalsFormulaOf(t, 2), QStringLiteral("SUBTOTAL(107,Orders[[Cost, net]])"));
        t.info[0].totalsFunction = QStringLiteral("custom");
        t.info[0].totalsFormula = QStringLiteral("MEDIAN(Orders[Price])");
        QCOMPARE(totalsFormulaOf(t, 0), QStringLiteral("MEDIAN(Orders[Price])"));
        t.info[0].totalsFunction.clear();
        QVERIFY(totalsFormulaOf(t, 0).isEmpty());
        // every function offered has its number among the subtotals that leave out hidden rows
        for (const TotalsFunction &f : totalsFunctions()) QVERIFY(f.number >= 101 && f.number <= 111);
        QCOMPARE(int(totalsFunctions().size()), 8);
    }

    // A table made here is saved with its part, a relationship from its sheet, its type, and the sheet's tableParts, and reads back as it was made. A part nothing touched stays byte for byte.
    void aTableMadeHereIsSavedAndReadBack()
    {
        const QByteArray original = buildXlsx({worksheet(QStringLiteral("<row r=\"1\">%1%2</row><row r=\"2\">%3%4</row>")
                                                             .arg(inlineCell(QStringLiteral("A1"), QStringLiteral("Item")), inlineCell(QStringLiteral("B1"), QStringLiteral("Cost")), inlineCell(QStringLiteral("A2"), QStringLiteral("Pen")),
                                                                  numberCell(QStringLiteral("B2"), 3)))});
        Open o;
        QVERIFY(o.ok(original));
        QVERIFY(o.b.tables().isEmpty());
        Table t;
        t.part = o.b.freshTablePart();
        t.id = o.b.freshTableId();
        t.sheet = 0;
        t.name = QStringLiteral("Items");
        t.row1 = 0;
        t.col1 = 0;
        t.row2 = 1;
        t.col2 = 1;
        t.columns = {QStringLiteral("Item"), QStringLiteral("Cost")};
        t.info.resize(2);
        t.info[0].id = 1;
        t.info[1].id = 2;
        t.styleName = QStringLiteral("TableStyleMedium9");
        QCOMPARE(t.part, QStringLiteral("/xl/tables/table1.xml"));
        QCOMPARE(t.id, 1);
        o.b.insertTable(-1, t);
        SaveChanges changes;
        for (const Workbook::TableAdd &a : o.b.addedTables()) changes.addedTables.push_back({a.sheet, a.part, a.xml});
        QCOMPARE(int(changes.addedTables.size()), 1);
        QByteArray out;
        QString why;
        QVERIFY2(saveXlsx(o.b, changes, &out, &why), qPrintable(why));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(out, files));
        QVERIFY(files.contains(QStringLiteral("xl/tables/table1.xml")));
        QVERIFY2(files.value(QStringLiteral("[Content_Types].xml")).contains("/xl/tables/table1.xml"), "the type");
        QVERIFY2(files.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).contains("../tables/table1.xml"), "the relationship");
        QVERIFY2(files.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<tableParts count=\"1\"><tablePart"), files.value(QStringLiteral("xl/worksheets/sheet1.xml")).constData());
        Open back;
        QVERIFY(back.ok(out));
        QCOMPARE(back.b.tables().size(), 1);
        const Table &r = back.b.tables().first();
        QCOMPARE(r.name, QStringLiteral("Items"));
        QCOMPARE(r.columns, t.columns);
        QCOMPARE(r.styleName, QStringLiteral("TableStyleMedium9"));
        QCOMPARE(rangeName(rangeOf(QStringLiteral("A1:B2"))), QStringLiteral("A1:B2"));
        QCOMPARE(int(r.row2), 1);
        // a table made in the workbook that has the sheet's relationships already gets the next id, and the first stays as it was
        Table second = t;
        second.part = back.b.freshTablePart();
        second.id = back.b.freshTableId();
        second.name = QStringLiteral("More");
        QCOMPARE(second.part, QStringLiteral("/xl/tables/table2.xml"));
        QCOMPARE(second.id, 2);
        back.b.insertTable(-1, second);
        SaveChanges again;
        for (const Workbook::TableAdd &a : back.b.addedTables()) again.addedTables.push_back({a.sheet, a.part, a.xml});
        QByteArray out2;
        QVERIFY2(saveXlsx(back.b, again, &out2, &why), qPrintable(why));
        QMap<QString, QByteArray> files2;
        QVERIFY(jp::readZip(out2, files2));
        QCOMPARE(files2.value(QStringLiteral("xl/tables/table1.xml")), files.value(QStringLiteral("xl/tables/table1.xml")));
        QVERIFY(files2.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<tableParts count=\"2\">"));
        Open third;
        QVERIFY(third.ok(out2));
        QCOMPARE(third.b.tables().size(), 2);
    }

    // A table taken out loses its part, its type, the sheet's link, and its element; the other table of the sheet stays.
    void aTableTakenOutLeavesNothingBehind()
    {
        const QString second = QString(salesPart()).replace(QStringLiteral("id=\"7\" name=\"Sales\" displayName=\"Sales\""), QStringLiteral("id=\"8\" name=\"Other\" displayName=\"Other\""));
        Open o;
        QVERIFY(o.ok(salesBook()));
        QByteArray two = withTables(buildXlsx({worksheet(QStringLiteral("<row r=\"1\">%1</row>").arg(inlineCell(QStringLiteral("A1"), QStringLiteral("Region"))))}), {salesPart(), second});
        Open w;
        QVERIFY(w.ok(two));
        QCOMPARE(w.b.tables().size(), 2);
        w.b.eraseTable(0);
        SaveChanges changes;
        for (const Workbook::TableGone &g : w.b.removedTables()) changes.removedTables.push_back({g.sheet, g.part});
        QCOMPARE(int(changes.removedTables.size()), 1);
        QByteArray out;
        QString why;
        QVERIFY2(saveXlsx(w.b, changes, &out, &why), qPrintable(why));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(out, files));
        QVERIFY(!files.contains(QStringLiteral("xl/tables/table1.xml")) && files.contains(QStringLiteral("xl/tables/table2.xml")));
        QVERIFY(!files.value(QStringLiteral("[Content_Types].xml")).contains("/xl/tables/table1.xml") && files.value(QStringLiteral("[Content_Types].xml")).contains("/xl/tables/table2.xml"));
        QVERIFY(!files.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).contains("table1.xml") && files.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).contains("table2.xml"));
        QVERIFY2(files.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<tableParts count=\"1\">"), files.value(QStringLiteral("xl/worksheets/sheet1.xml")).constData());
        Open back;
        QVERIFY(back.ok(out));
        QCOMPARE(back.b.tables().size(), 1);
        QCOMPARE(back.b.tables().first().name, QStringLiteral("Other"));
        // and the last one taken out takes the element
        back.b.eraseTable(0);
        SaveChanges last;
        for (const Workbook::TableGone &g : back.b.removedTables()) last.removedTables.push_back({g.sheet, g.part});
        QByteArray out2;
        QVERIFY2(saveXlsx(back.b, last, &out2, &why), qPrintable(why));
        QMap<QString, QByteArray> files2;
        QVERIFY(jp::readZip(out2, files2));
        QVERIFY2(!files2.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("tableParts"), files2.value(QStringLiteral("xl/worksheets/sheet1.xml")).constData());
    }

    // Structured references are read into the cells they stand for.
    void structuredReferencesStandForCells()
    {
        Open o;
        QVERIFY(o.ok(salesBook()));
        const Table &t = o.b.tables().first();
        auto area = [&](const QString &inside, qint64 row = -1) {
            CellRange r;
            const StructResult res = structuredArea(t, inside, row, &r);
            return res == StructResult::Ok ? rangeName(r) : (res == StructResult::Value ? QStringLiteral("#VALUE!") : QStringLiteral("#REF!"));
        };
        QCOMPARE(area(QStringLiteral("Q1")), QStringLiteral("B2:B5"));
        QCOMPARE(area(QStringLiteral("q1")), QStringLiteral("B2:B5"));
        QCOMPARE(area(QStringLiteral("[Region]:[Double]")), QStringLiteral("A2:C5"));
        QCOMPARE(area(QStringLiteral("[Q1]:[Region]")), QStringLiteral("A2:B5"));
        QCOMPARE(area(QStringLiteral("#All")), QStringLiteral("A1:C6"));
        QCOMPARE(area(QStringLiteral("#Data")), QStringLiteral("A2:C5"));
        QCOMPARE(area(QStringLiteral("#Headers")), QStringLiteral("A1:C1"));
        QCOMPARE(area(QStringLiteral("#Totals")), QStringLiteral("A6:C6"));
        QCOMPARE(area(QStringLiteral("[#Headers],[Q1]")), QStringLiteral("B1"));
        QCOMPARE(area(QStringLiteral("[#Totals],[Q1]")), QStringLiteral("B6"));
        QCOMPARE(area(QStringLiteral("[#Headers],[#Data]")), QStringLiteral("A1:C5"));
        QCOMPARE(area(QStringLiteral("[#Data],[#Totals],[Q1]")), QStringLiteral("B2:B6"));
        QCOMPARE(area(QStringLiteral("[#Headers],[#Totals]")), QStringLiteral("#REF!"));
        QCOMPARE(area(QStringLiteral("Nothing")), QStringLiteral("#REF!"));
        QCOMPARE(area(QStringLiteral("@Q1"), 2), QStringLiteral("B3"));
        QCOMPARE(area(QStringLiteral("@"), 3), QStringLiteral("A4:C4"));
        QCOMPARE(area(QStringLiteral("@[Q1]"), 2), QStringLiteral("B3"));
        QCOMPARE(area(QStringLiteral("[#This Row],[Q1]"), 4), QStringLiteral("B5"));
        QCOMPARE(area(QStringLiteral("[#This Row],[Q1]:[Double]"), 4), QStringLiteral("B5:C5"));
        QCOMPARE(area(QStringLiteral("@Q1"), 7), QStringLiteral("#VALUE!"));    // (a row outside the data)
        QCOMPARE(area(QStringLiteral("@Q1")), QStringLiteral("#VALUE!"));
        // a table with no row of totals, or no header row, has no such block
        Table plain = t;
        plain.totalsRows = 0;
        plain.row2 = 5;
        CellRange r;
        QCOMPARE(int(structuredArea(plain, QStringLiteral("#Totals"), -1, &r)), int(StructResult::Ref));
        plain.headerRows = 0;
        QCOMPARE(int(structuredArea(plain, QStringLiteral("#Headers"), -1, &r)), int(StructResult::Ref));
        QVERIFY(structuredArea(plain, QStringLiteral("Q1"), -1, &r) == StructResult::Ok && r.row1 == 0);
        // the text of a reference is split, and typed back
        QString table, inside;
        QVERIFY(splitStructuredRef(QStringLiteral("Sales[[#Headers],[Q1]]"), &table, &inside));
        QCOMPARE(table, QStringLiteral("Sales"));
        QCOMPARE(inside, QStringLiteral("[#Headers],[Q1]"));
        QVERIFY(splitStructuredRef(QStringLiteral("[@Q1]"), &table, &inside) && table.isEmpty() && inside == QLatin1String("@Q1"));
        QVERIFY(!splitStructuredRef(QStringLiteral("Sales"), &table, &inside));
        QCOMPARE(structuredColumnRef(QStringLiteral("T"), QStringLiteral("A b")), QStringLiteral("T[A b]"));
        QCOMPARE(structuredColumnRef(QStringLiteral("T"), QStringLiteral("Unit #")), QStringLiteral("T[[Unit '#]]"));
        QCOMPARE(structuredRowRef(QStringLiteral("T"), QStringLiteral("Q1"), true), QStringLiteral("[@Q1]"));
        QCOMPARE(structuredRowRef(QStringLiteral("T"), QStringLiteral("Q1"), false), QStringLiteral("T[[#This Row],[Q1]]"));
        QCOMPARE(structuredSpecialRef(QStringLiteral("T"), QStringLiteral("#Totals")), QStringLiteral("T[#Totals]"));
    }

    // The short forms become the file's, and back; a table renamed or turned into cells changes the formulas that name it, not texts and not other tables.
    void formulasFollowTheTableAndItsShortForms()
    {
        QCOMPARE(fileFormOfStructuredRefs(QStringLiteral("[@Q1]*2+[@[Unit '#]]"), QStringLiteral("Sales")), QStringLiteral("Sales[[#This Row],[Q1]]*2+Sales[[#This Row],[Unit '#]]"));
        QCOMPARE(fileFormOfStructuredRefs(QStringLiteral("Other[@Q1]+\"[@Q1]\""), QString()), QStringLiteral("Other[[#This Row],[Q1]]+\"[@Q1]\""));
        QCOMPARE(fileFormOfStructuredRefs(QStringLiteral("[@Q1]"), QString()), QStringLiteral("[@Q1]"));
        QCOMPARE(fileFormOfStructuredRefs(QStringLiteral("[@]"), QStringLiteral("T")), QStringLiteral("T[#This Row]"));
        QCOMPARE(fileFormOfStructuredRefs(QStringLiteral("[@[A]:[B]]"), QStringLiteral("T")), QStringLiteral("T[[#This Row],[A]:[B]]"));
        QCOMPARE(typedFormOfStructuredRefs(QStringLiteral("Sales[[#This Row],[Q1]]*2"), QStringLiteral("Sales")), QStringLiteral("[@Q1]*2"));
        QCOMPARE(typedFormOfStructuredRefs(QStringLiteral("Sales[[#This Row],[Q1]]*2"), QStringLiteral("Other")), QStringLiteral("Sales[[#This Row],[Q1]]*2"));
        QCOMPARE(typedFormOfStructuredRefs(QStringLiteral("Sales[[#This Row],[Q1]:[Double]]"), QStringLiteral("Sales")), QStringLiteral("Sales[[#This Row],[Q1]:[Double]]"));
        QCOMPARE(renameTableInFormula(QStringLiteral("SUM(sales[Q1])+Sales[[#Totals],[Q1]]+\"Sales[Q1]\"+Other[Q1]"), QStringLiteral("Sales"), QStringLiteral("Orders")),
                 QStringLiteral("SUM(Orders[Q1])+Orders[[#Totals],[Q1]]+\"Sales[Q1]\"+Other[Q1]"));
        Open o;
        QVERIFY(o.ok(salesBook()));
        const Table &t = o.b.tables().first();
        QCOMPARE(structuredRefsToCells(QStringLiteral("SUM(Sales[Q1])+Sales[[#This Row],[Q1]]"), t, QStringLiteral("Sheet1"), QStringLiteral("Sheet1"), 2, true), QStringLiteral("SUM($B$2:$B$5)+B3"));
        QCOMPARE(structuredRefsToCells(QStringLiteral("Sales[#All]"), t, QStringLiteral("Sheet1"), QStringLiteral("Sheet2"), -1, false), QStringLiteral("Sheet1!$A$1:$C$6"));
        QCOMPARE(structuredRefsToCells(QStringLiteral("Sales[[#This Row],[Q1]]"), t, QStringLiteral("Sheet1"), QStringLiteral("Sheet1"), -1, false), QStringLiteral("#REF!"));
        QCOMPARE(structuredRefsToCells(QStringLiteral("Other[Q1]+Sales[Gone]"), t, QStringLiteral("Sheet1"), QStringLiteral("Sheet1"), -1, false), QStringLiteral("Other[Q1]+#REF!"));
    }

    // The engine works structured references out: a column, a row's own cell, the totals, the headers, and a count of the whole; and again after the table grows or is renamed.
    void theEngineWorksOutStructuredReferences()
    {
        Open o;
        QVERIFY(o.ok(salesBook()));
        CalcOptions options;
        options.keepApart = true;
        options.workOutTables = true;
        options.workOutWhatHasNoResult = true;
        Calculation calc(o.b, options);
        QVERIFY(calc.load());
        calc.recalculate();
        auto at = [&](const QString &ref) {
            quint32 r = 0, c = 0;
            parseCellRef(ref, &r, &c);
            return calc.cell(0, r, c).debug();
        };
        QCOMPARE(at(QStringLiteral("C3")), QStringLiteral("n:4"));          // the second row's Q1 (2) doubled
        QCOMPARE(at(QStringLiteral("B6")), QStringLiteral("n:10"));
        QCOMPARE(at(QStringLiteral("C6")), QStringLiteral("n:20"));
        QCOMPARE(at(QStringLiteral("A8")), QStringLiteral("n:%1").arg(10 + 3 + 6));
        // a value in the column changes what reads the column
        quint32 r = 0, c = 0;
        parseCellRef(QStringLiteral("B2"), &r, &c);
        QVERIFY(calc.setValue(0, r, c, Value::number(11)));
        calc.recalculate();
        QCOMPARE(at(QStringLiteral("B6")), QStringLiteral("n:20"));
        QCOMPARE(at(QStringLiteral("C2")), QStringLiteral("n:22"));
        // the table grows by a row (the totals row moves down): what names the table is worked out again
        Table grown = o.b.tables().first();
        grown.row2 += 1;
        o.b.setTableAt(0, grown);
        calc.tablesChanged();
        calc.recalculate();
        QCOMPARE(at(QStringLiteral("A9")), QStringLiteral("n:7"));
        // a name that is no table's is a reference error, not the stored value
        QVERIFY(calc.setFormula(0, 9, 0, QStringLiteral("SUM(Nothing[Q1])")));
        calc.recalculate();
        QCOMPARE(at(QStringLiteral("A10")), QStringLiteral("e:#REF!"));
    }

    // The yardstick's way (tables not worked out) keeps what the file stored for them, as before.
    void withoutTablesTheStoredValueIsKept()
    {
        Open o;
        QVERIFY(o.ok(salesBook()));
        CalcOptions options;
        options.keepApart = true;
        Calculation calc(o.b, options);
        QVERIFY(calc.load());
        calc.recalculate();
        quint32 r = 0, c = 0;
        parseCellRef(QStringLiteral("B6"), &r, &c);
        QVERIFY(calc.isTainted(0, r, c));
    }

    // A part is written as it was: a name that holds the file's own mark for a line break (_x000a_), a function a column names although the table has no row of totals, and attributes the
    // program knows nothing of; the table's range and filter follow a row added.
    void aChangedPartKeepsWhatItNeverHeld()
    {
        const QString part = QStringLiteral("<table xmlns=\"%1\" id=\"3\" name=\"Budget\" displayName=\"Budget\" ref=\"A1:B4\"><autoFilter ref=\"A1:B4\"/><tableColumns count=\"2\">"
                                           "<tableColumn id=\"1\" name=\"Hand_x000a_Bars\" totalsRowLabel=\"Total\"/><tableColumn id=\"2\" name=\"Amount &amp; more\" totalsRowFunction=\"sum\" totalsRowDxfId=\"2\"/></tableColumns>"
                                           "<tableStyleInfo name=\"My &quot;own&quot; style\" showRowStripes=\"1\"/></table>")
                                 .arg(QLatin1String(kNs));
        Open o;
        QVERIFY(o.ok(withTables(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"/>"))}), {part})));
        QCOMPARE(o.b.tables().size(), 1);
        Table t = o.b.tables().first();
        QCOMPARE(t.columns.first(), QStringLiteral("Hand_x000a_Bars"));
        t.row2 += 1;
        o.b.setTableAt(0, t);
        const QString xml = QString::fromUtf8(o.b.changedTableParts().begin()->second);
        QVERIFY2(xml.contains(QStringLiteral("name=\"Hand_x000a_Bars\"")) && xml.contains(QStringLiteral("name=\"Amount &amp; more\"")), qPrintable(xml));
        QVERIFY2(xml.contains(QStringLiteral("totalsRowFunction=\"sum\"")) && xml.contains(QStringLiteral("totalsRowLabel=\"Total\"")) && xml.contains(QStringLiteral("totalsRowDxfId=\"2\"")), qPrintable(xml));
        QVERIFY2(xml.contains(QStringLiteral("ref=\"A1:B5\"")) && xml.contains(QStringLiteral("<autoFilter ref=\"A1:B5\"/>")) && !xml.contains(QStringLiteral("totalsRowShown")), qPrintable(xml));
        QVERIFY2(xml.contains(QStringLiteral("name=\"My &quot;own&quot; style\"")), qPrintable(xml));
        // a real line break in a name is written as the character reference an attribute keeps
        t.columns[0] = QStringLiteral("Two\nlines");
        o.b.setTableAt(0, t);
        const QByteArray bytes = o.b.changedTableParts().begin()->second;
        QVERIFY(bytes.contains("name=\"Two&#10;lines\""));
        Table back;
        back.columns = t.columns;
        QVERIFY(readTableModel(bytes, &back));
        QCOMPARE(back.info.size(), 2);
    }

    // Names: what a table may be called, and the name a new table is given.
    void aTableNameIsChecked()
    {
        QVERIFY(validTableName(QStringLiteral("Sales_2024")));
        QVERIFY(validTableName(QStringLiteral("_x")));
        QVERIFY(validTableName(QStringLiteral("Ünïcode.Name")));
        QString why;
        QVERIFY(!validTableName(QString(), &why) && !why.isEmpty());
        QVERIFY(!validTableName(QStringLiteral("1Table"), &why));
        QVERIFY(!validTableName(QStringLiteral("My Table"), &why));
        QVERIFY(!validTableName(QStringLiteral("A1"), &why));
        QVERIFY(!validTableName(QStringLiteral("XFD1048576"), &why));
        QVERIFY(!validTableName(QStringLiteral("R2C3"), &why));
        QVERIFY(!validTableName(QStringLiteral("r"), &why));
        QVERIFY(!validTableName(QStringLiteral("C"), &why));
        QVERIFY(!validTableName(QString(256, QLatin1Char('a')), &why));
        QVERIFY(validTableName(QString(255, QLatin1Char('a'))));
        QVERIFY(validTableName(QStringLiteral("ABCD1")));   // (four letters is no cell)
        QCOMPARE(freshTableName({}), QStringLiteral("Table1"));
        QCOMPARE(freshTableName({QStringLiteral("table1"), QStringLiteral("Table2")}), QStringLiteral("Table3"));
    }

    // The built-in styles: every name of the gallery is a style; the parts a cell is in follow the marks; the look puts the parts over one another.
    void aStyleSaysWhatEachPartLooksLike()
    {
        const QStringList names = builtinTableStyleNames();
        QCOMPARE(names.size(), 21 + 28 + 11);
        for (const QString &n : names) QVERIFY2(builtinTableStyle(n) != nullptr, qPrintable(n));
        QVERIFY(builtinTableStyle(QStringLiteral("tablestylemedium2")) != nullptr);
        QVERIFY(builtinTableStyle(QStringLiteral("Mine")) == nullptr);
        int cells = 0;
        for (const TableStyleGroup &g : tableStyleGroups()) cells += g.names.size();
        QCOMPARE(cells, names.size());
        Table t;
        t.name = QStringLiteral("T");
        t.row1 = 2;
        t.row2 = 7;
        t.col1 = 1;
        t.col2 = 3;
        t.headerRows = 1;
        t.totalsRows = 1;
        t.showRowStripes = true;
        t.showFirstColumn = true;
        TablePart parts[16];
        auto has = [&](quint32 row, quint32 col, TablePart p) {
            const int n = tablePartsAt(t, row, col, parts);
            return std::find(parts, parts + n, p) != parts + n;
        };
        QCOMPARE(tablePartsAt(t, 1, 1, parts), 0);          // (above the table)
        QCOMPARE(tablePartsAt(t, 2, 4, parts), 0);          // (beside it)
        QVERIFY(has(2, 2, TablePart::HeaderRow) && !has(2, 2, TablePart::FirstRowStripe) && !has(2, 2, TablePart::FirstColumn));
        QVERIFY(has(2, 1, TablePart::FirstHeaderCell) && has(2, 1, TablePart::FirstColumn));
        QVERIFY(has(3, 2, TablePart::FirstRowStripe) && !has(3, 2, TablePart::SecondRowStripe));
        QVERIFY(has(4, 2, TablePart::SecondRowStripe));
        QVERIFY(has(7, 2, TablePart::TotalRow) && !has(7, 2, TablePart::FirstRowStripe) && !has(7, 2, TablePart::SecondRowStripe));
        QVERIFY(has(7, 3, TablePart::LastTotalCell) && !has(7, 3, TablePart::LastColumn));
        t.showLastColumn = true;
        QVERIFY(has(4, 3, TablePart::LastColumn));
        // a medium style (Medium 2): a filled header with light text, stripes in a pale color
        const TableStyle &st = *builtinTableStyle(QStringLiteral("TableStyleMedium2"));
        quint32 headerKey = 0, bodyKey = 0, plainKey = 0;
        const DifferentialFormat header = tableLookAt(st, t, 2, 2, &headerKey);
        QVERIFY(header.hasFill && header.bold.value_or(false) && header.fontColor.isSet());
        const DifferentialFormat stripe = tableLookAt(st, t, 3, 2, &bodyKey);
        const DifferentialFormat band2 = tableLookAt(st, t, 4, 2, &plainKey);
        QVERIFY(stripe.hasFill && stripe.fill != header.fill);
        QVERIFY(bodyKey != headerKey && bodyKey != plainKey);
        QVERIFY(!band2.hasFill || band2.fill != stripe.fill);
        // marks off: nothing of the stripes
        t.showRowStripes = false;
        t.showFirstColumn = false;
        t.showLastColumn = false;
        quint32 key = 0;
        const DifferentialFormat flat = tableLookAt(st, t, 3, 2, &key);
        QVERIFY(!(flat.hasFill && flat.fill == stripe.fill));
        // a cell outside the table is not styled
        QVERIFY(tableLookAt(st, t, 0, 0, &key).isEmpty() && key == 0);
        // the merge of two differential formats: what the second sets wins, the sides one by one
        DifferentialFormat a, b;
        a.bold = true;
        a.hasBorder = true;
        a.border.top.style = BorderStyle::Thin;
        b.bold = false;
        b.hasBorder = true;
        b.border.bottom.style = BorderStyle::Medium;
        const DifferentialFormat m = mergedDifferential(a, b);
        QVERIFY(m.bold.has_value() && !*m.bold && m.border.top.style == BorderStyle::Thin && m.border.bottom.style == BorderStyle::Medium);
    }

    // A workbook can have too many tables, or one with too many columns: the reader keeps to its bounds.
    void hostileTablePartsAreBounded()
    {
        QString cols;
        for (int i = 0; i < 20000; ++i) cols += QStringLiteral("<tableColumn id=\"%1\" name=\"C%1\"/>").arg(i + 1);
        const QString part = QStringLiteral("<table xmlns=\"%1\" id=\"1\" name=\"Big\" displayName=\"Big\" ref=\"A1:XFD5\"><tableColumns count=\"20000\">%2</tableColumns></table>").arg(QLatin1String(kNs), cols);
        Open o;
        QVERIFY(o.ok(withTables(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"/>"))}), {part})));
        QCOMPARE(o.b.tables().size(), 1);
        QCOMPARE(o.b.tables().first().columns.size(), 16384);
        QCOMPARE(o.b.tables().first().info.size(), 16384);
        // a part whose columns are not as many as the cells it covers is left out (the file is not valid)
        Open narrow;
        QVERIFY(narrow.ok(withTables(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"/>"))}), {part.left(part.indexOf(QStringLiteral("ref=\"A1:XFD5\""))) + QStringLiteral("ref=\"A1:C5\"") + part.mid(part.indexOf(QStringLiteral("ref=\"A1:XFD5\"")) + 14)})));
        QVERIFY(narrow.b.tables().isEmpty());
        // a part that is not XML, or not a table's, is left out
        Open junk;
        QVERIFY(junk.ok(withTables(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"/>"))}), {QStringLiteral("not xml at all"), QStringLiteral("<other/>")})));
        QVERIFY(junk.b.tables().isEmpty());
    }
};

JP_TEST_CLASS(SheetTableModelTests)
#include "tst_sheet_tablemodel.moc"
