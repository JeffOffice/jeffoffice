// Tests of formulas: the references of a shared formula moved to each cell, array formulas, stored results, and the functions a formula calls.
#include "sheethelpers.h"

namespace {

struct Shift {
    const char *formula;
    int rows, cols;
    const char *expected;
};

QString formulaAt(const Sheet &sheet, const QString &ref)
{
    const Cell *c = at(sheet, ref);
    return c ? sheet.formulaText(*c) : QStringLiteral("(no cell)");
}

} // namespace

class SheetFormulaTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- moving references

    void referencesMoveAsTheirDollarsSay()
    {
        const Shift cases[] = {
            {"A1", 1, 1, "B2"},
            {"$A$1", 5, 5, "$A$1"},
            {"$A1+A$1", 2, 3, "$A3+D$1"},
            {"a1", 1, 1, "B2"},
            {"SUM(A1:B2)", 1, 1, "SUM(B2:C3)"},
            {"SUM($A$1:B2)", 1, 1, "SUM($A$1:C3)"},
            {"A$1:B$2", 1, 1, "B$1:C$2"},
            {"A:A", 0, 2, "C:C"},
            {"$A:B", 3, 1, "$A:C"},
            {"1:3", 2, 0, "3:5"},
            {"$1:$3", 2, 5, "$1:$3"},
            {"A1:A10 B1:B10", 0, 1, "B1:B10 C1:C10"},
            {"Sheet2!A1", 1, 0, "Sheet2!A2"},
            {"'Sheet 2'!A1:B2", 1, 1, "'Sheet 2'!B2:C3"},
            {"'It''s'!$A1", 2, 2, "'It''s'!$A3"},
            {"Sheet1:Sheet3!A1", 1, 1, "Sheet1:Sheet3!B2"},
            {"Sheet2!1:2", 1, 0, "Sheet2!2:3"},
            {"[1]Sheet1!A1", 1, 0, "[1]Sheet1!A2"},
            {"'[1]Sheet 1'!A1", 1, 0, "'[1]Sheet 1'!A2"},
            {"Sheet.1!A1+Caf\xc3\xa9!B2", 1, 1, "Sheet.1!B2+Caf\xc3\xa9!C3"},
            // not references: text, function and defined names, numbers, errors
            {"\"A1\"&A1&\"\"\"B2\"", 1, 1, "\"A1\"&B2&\"\"\"B2\""},
            {"LOG10(A1)+ATAN2(A1,B1)", 1, 1, "LOG10(B2)+ATAN2(B2,C2)"},
            {"1E5+A1+1.5E+3+.5", 1, 1, "1E5+B2+1.5E+3+.5"},
            {"#REF!+A1+#N/A+#DIV/0!", 1, 1, "#REF!+B2+#N/A+#DIV/0!"},
            {"tax_rate*A1+name.1", 1, 1, "tax_rate*B2+name.1"},
            {"Table1[Col1]+A1", 1, 1, "Table1[Col1]+B2"},
            {"Table1[[#This Row],[A1]]+A1", 1, 1, "Table1[[#This Row],[A1]]+B2"},
            {"IF(A1=\"x\",TRUE,FALSE)", 1, 0, "IF(A2=\"x\",TRUE,FALSE)"},
            {"R1C1+A1", 1, 1, "R1C1+B2"},
            {"A1#+A1", 1, 1, "B2#+B2"},
            {"-A1%", 1, 1, "-B2%"},
            {"SUM(A1,,B1)", 0, 1, "SUM(B1,,C1)"},
            {"_xlfn.IFS(A1>0,1)", 1, 0, "_xlfn.IFS(A2>0,1)"},
            {"@A1", 1, 0, "@A2"},
            {"A1:INDEX(B:B,3)", 1, 1, "B2:INDEX(C:C,3)"},
            // off the grid
            {"A1", -1, 0, "#REF!"},
            {"A1", 0, -1, "#REF!"},
            {"XFD1", 0, 1, "#REF!"},
            {"A1048576", 1, 0, "#REF!"},
            {"A1:B2", -1, 0, "#REF!"},
            {"1:1", -1, 0, "#REF!"},
            {"A:A", 0, -1, "#REF!"},
            {"B2+A1", -1, -1, "A1+#REF!"},
            // negative moves, and none
            {"C3", -2, -2, "A1"},
            {"A1+B2", 0, 0, "A1+B2"},
        };
        for (const Shift &c : cases) {
            const QString in = QString::fromUtf8(c.formula);
            QCOMPARE(shiftFormula(in, c.rows, c.cols), QString::fromUtf8(c.expected));
        }
    }

    void theFunctionsAFormulaCallsAreListed()
    {
        QCOMPARE(formulaFunctions(QStringLiteral("SUM(A1:A3)+IF(B1>0,MAX(1,2),LOG10(100))")), (QStringList{"SUM", "IF", "MAX", "LOG10"}));
        QCOMPARE(formulaFunctions(QStringLiteral("sum(a1)+Sum(b1)")), (QStringList{"SUM", "SUM"}));
        QCOMPARE(formulaFunctions(QStringLiteral("_xlfn.CONCAT(A1,\"SUM(\")&_xlfn._xlws.FILTER(A1:A3,B1:B3)")), (QStringList{"CONCAT", "FILTER"}));
        QCOMPARE(formulaFunctions(QStringLiteral("Table1[[#This Row],[SUM(x)]]+'My (Sheet)'!A1+A1")), QStringList());
        QCOMPARE(formulaFunctions(QStringLiteral("STDEV.S(A1:A9)+Sheet1!B1")), (QStringList{"STDEV.S"}));
        QCOMPARE(formulaFunctions(QStringLiteral("A1+B1*2")), QStringList());
        QCOMPARE(formulaFunctions(QStringLiteral("TRUE+FALSE+TRUE()")), (QStringList{"TRUE"}));
        QCOMPARE(formulaFunctions(QString()), QStringList());
    }

    // ---- formulas in a sheet

    // The first cell of a shared formula has the text; each other cell has it as it would be written for that cell.
    void sharedFormulasAreExpandedForEachCell()
    {
        const QByteArray xml = worksheet(
            "<row r=\"2\"><c r=\"A2\"><v>1</v></c><c r=\"C2\"><f t=\"shared\" ref=\"C2:C4\" si=\"0\">A2*$B$1+B2+Sheet2!A2+$A2</f><v>10</v></c>"
            "<c r=\"E2\"><f t=\"shared\" ref=\"E2:G2\" si=\"1\">A2+$A$2+A$2</f><v>11</v></c><c r=\"F2\"><f t=\"shared\" si=\"1\"/><v>12</v></c><c r=\"G2\"><f t=\"shared\" si=\"1\"/><v>13</v></c></row>"
            "<row r=\"3\"><c r=\"C3\"><f t=\"shared\" si=\"0\"/><v>20</v></c></row>"
            "<row r=\"4\"><c r=\"C4\"><f t=\"shared\" si=\"0\"/></c><c r=\"E4\"><f>SUM(A1:A3)</f><v>5</v></c></row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        QCOMPARE(formulaAt(sheet, "C2"), QStringLiteral("A2*$B$1+B2+Sheet2!A2+$A2"));
        QCOMPARE(formulaAt(sheet, "C3"), QStringLiteral("A3*$B$1+B3+Sheet2!A3+$A3"));
        QCOMPARE(formulaAt(sheet, "C4"), QStringLiteral("A4*$B$1+B4+Sheet2!A4+$A4"));
        QCOMPARE(formulaAt(sheet, "E2"), QStringLiteral("A2+$A$2+A$2"));
        QCOMPARE(formulaAt(sheet, "F2"), QStringLiteral("B2+$A$2+B$2"));
        QCOMPARE(formulaAt(sheet, "G2"), QStringLiteral("C2+$A$2+C$2"));
        QCOMPARE(formulaAt(sheet, "E4"), QStringLiteral("SUM(A1:A3)"));
        QCOMPARE(formulaAt(sheet, "A2"), QString());
        QVERIFY(at(sheet, "C2")->flags & SharedFormula);
        QVERIFY(at(sheet, "C3")->flags & SharedFormula);
        QVERIFY(!(at(sheet, "E4")->flags & SharedFormula));
        QVERIFY(at(sheet, "E4")->hasFormula());
        QVERIFY(!at(sheet, "A2")->hasFormula());
        // Seven formula cells, six with a result; the shared formulas are two, one text each.
        QCOMPARE(sheet.formulaCells, qint64(7));
        QCOMPARE(sheet.storedResults, qint64(6));
        QCOMPARE(sheet.stats.sharedFormulas, qint64(2));
        QCOMPARE(sheet.formulaTexts.size(), 3);
        QCOMPARE(sheet.stats.largestSharedRange, qint64(3));
        // The sum of the characters of every formula as asked for: a shared one once for each of its cells.
        QCOMPARE(sheet.stats.expandedFormulaChars, qint64(3 * 24 + 3 * 11 + 10));
    }

    // A shared formula may be a cell above or to the left of the first one that has it; and a group may be numbered any way (far apart, out of order).
    void sharedFormulasNeedNoParticularOrderOrNumbers()
    {
        const QByteArray xml = worksheet(
            "<row r=\"1\"><c r=\"B1\"><f t=\"shared\" si=\"4000000000\"/></c></row>"
            "<row r=\"5\"><c r=\"B5\"><f t=\"shared\" ref=\"B1:B5\" si=\"4000000000\">A5+1</f></c></row>"
            "<row r=\"2\"><c r=\"B2\"><f t=\"shared\" si=\"7\"/></c></row>"
            "<row r=\"3\"><c r=\"B3\"><f t=\"shared\" ref=\"B3:B4\" si=\"7\">A3*2</f></c><c r=\"C3\"><f t=\"shared\" si=\"99\"/></c></row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        QCOMPARE(formulaAt(sheet, "B5"), QStringLiteral("A5+1"));
        // B1 comes before its first cell in the file but is above it in the sheet, so the formula is as written for B5, moved four rows up.
        QCOMPARE(formulaAt(sheet, "B1"), QStringLiteral("A1+1"));
        QCOMPARE(formulaAt(sheet, "B3"), QStringLiteral("A3*2"));
        QCOMPARE(formulaAt(sheet, "B2"), QStringLiteral("A2*2"));
        // A cell of a group nothing ever gave a formula to has none to show, but it is a formula cell.
        QVERIFY(at(sheet, "C3")->hasFormula());
        QCOMPARE(formulaAt(sheet, "C3"), QString());
    }

    void arrayFormulasAreMarkedWithTheirRange()
    {
        const QByteArray xml = worksheet(
            "<row r=\"1\"><c r=\"A1\"><f t=\"array\" ref=\"A1:B2\">SUM(C1:C3*D1:D3)</f><v>5</v></c><c r=\"B1\"><v>6</v></c></row>"
            "<row r=\"2\"><c r=\"A2\"><v>7</v></c><c r=\"B2\"><v>8</v></c><c r=\"D2\"><f t=\"array\" ref=\"D2\" aca=\"1\">TRANSPOSE(A1:B1)</f><v>1</v></c></row>"
            "<row r=\"3\"><c r=\"A3\"><f t=\"dataTable\" ref=\"B3:D3\" dt2D=\"1\" dtr=\"1\" r1=\"A1\" r2=\"B1\"/><v>9</v></c></row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        const Cell *a1 = at(sheet, "A1");
        QVERIFY(a1 && (a1->flags & ArrayFormula) && a1->hasFormula());
        QCOMPARE(sheet.formulaText(*a1), QStringLiteral("SUM(C1:C3*D1:D3)"));
        QVERIFY(sheet.formulaRange(*a1).valid);
        QCOMPARE(rangeName(sheet.formulaRange(*a1)), QStringLiteral("A1:B2"));
        // The other cells of the array hold their values, and no formula.
        QVERIFY(!at(sheet, "B1")->hasFormula());
        QVERIFY(!sheet.formulaRange(*at(sheet, "B1")).valid);
        QCOMPARE(rangeName(sheet.formulaRange(*at(sheet, "D2"))), QStringLiteral("D2"));
        QVERIFY(at(sheet, "D2")->flags & ArrayFormula);
        QVERIFY(at(sheet, "A3")->flags & DataTable);
        QVERIFY(!(at(sheet, "A3")->flags & ArrayFormula));
        QCOMPARE(rangeName(sheet.formulaRange(*at(sheet, "A3"))), QStringLiteral("B3:D3"));
        QCOMPARE(sheet.formulaCells, qint64(3));
        QCOMPARE(sheet.storedResults, qint64(3));
    }

    void formulaTextIsReadWithItsEntitiesAndWhitespaceAsWritten()
    {
        const QByteArray xml = worksheet("<row r=\"1\"><c r=\"A1\" t=\"str\"><f>IF(B1&lt;&gt;&quot;&amp;&quot;, &quot; a  b &quot;,CONCAT(&quot;x&quot;))</f><v>x</v></c>"
                                         "<c r=\"B1\"><f ca=\"1\">NOW()</f><v>45000.5</v></c><c r=\"C1\"><f>  </f></c></row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        QCOMPARE(formulaAt(sheet, "A1"), QStringLiteral("IF(B1<>\"&\", \" a  b \",CONCAT(\"x\"))"));
        QCOMPARE(formulaAt(sheet, "B1"), QStringLiteral("NOW()"));
        SharedStrings none;
        QCOMPARE(show(sheet, none, "A1"), QStringLiteral("t:x"));
        QCOMPARE(show(sheet, none, "B1"), QStringLiteral("n:45000.5"));
        // A formula element with nothing in it, and not a shared formula's, makes the cell a formula cell all the same.
        QVERIFY(at(sheet, "C1")->hasFormula());
    }
};

JP_TEST_CLASS(SheetFormulaTests)
#include "tst_sheet_formulas.moc"
