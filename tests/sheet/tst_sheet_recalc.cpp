// Tests of the formula yardstick (sheet/recalc.h): each formula cell of a workbook is worked out once from the values the file stored, and compared with its own stored
// result. Every workbook here is built in memory, with the results written by hand from what each formula must give.
#include "sheethelpers.h"

#include "sheet/recalc.h"

#include <functional>

namespace {

struct Run {
    RecalcReport report;
    QVector<RecalcCell> cells;
    QString cellText(const QString &sheet, const QString &ref) const
    {
        for (const RecalcCell &c : cells)
            if (c.sheet == sheet && c.cell == ref) return QStringLiteral("%1|%2|%3|%4").arg(QChar(char(c.cls)), c.formula, c.stored, c.ours);
        return QStringLiteral("(not called)");
    }
};

bool runWorkbook(const QByteArray &zip, Run *out, const RecalcOptions &options = RecalcOptions())
{
    Package package;
    QString why;
    if (!package.readLazy(zip, &why)) return false;
    Workbook book;
    if (!book.open(package, &why)) return false;
    return recalcWorkbook(book, options, [&](const RecalcCell &c) { out->cells.push_back(c); }, &out->report, &why);
}

QString cell(const QString &ref, const QString &formula, const QString &value, const QString &type = QString())
{
    return QStringLiteral("<c r=\"%1\"%2><f>%3</f><v>%4</v></c>").arg(ref, type.isEmpty() ? QString() : QStringLiteral(" t=\"%1\"").arg(type), formula.toHtmlEscaped(), value.toHtmlEscaped());
}

QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">").arg(n) + cells + QStringLiteral("</row>"); }

QString plain(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }

} // namespace

class SheetRecalcTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void everyKindOfFormulaCellIsClassed()
    {
        const QString rows = QStringLiteral("<row r=\"1\">") + plain("A1", "1") + QStringLiteral("</row><row r=\"2\">") + plain("A2", "2")
            + QStringLiteral("</row><row r=\"3\">") + cell("A3", "SUM(A1:A2)", "3") + QStringLiteral("</row><row r=\"4\">") + cell("A4", "A3*2", "7")
            + QStringLiteral("</row><row r=\"5\">") + cell("A5", "FOO(1)", "#NAME?", "e") + QStringLiteral("</row><row r=\"6\">") + cell("A6", "NOW()", "45000")
            + QStringLiteral("</row><row r=\"7\">") + cell("A7", "[1]Sheet1!A1", "5") + QStringLiteral("</row><row r=\"8\">") + cell("A8", "Table1[Col]", "1")
            + QStringLiteral("</row><row r=\"9\">") + cell("A9", "1+", "0") + QStringLiteral("</row><row r=\"10\"><c r=\"A10\"><f>1+1</f></c></row><row r=\"11\">")
            + cell("A11", "\"a\"&\"b\"", "ab", "str") + QStringLiteral("</row><row r=\"12\">") + cell("A12", "1=1", "1", "b") + QStringLiteral("</row><row r=\"13\">")
            + cell("A13", "1/0", "#DIV/0!", "e") + QStringLiteral("</row><row r=\"14\">") + cell("A14", "OFFSET(A1,1,0)", "2") + QStringLiteral("</row><row r=\"15\">")
            + cell("A15", "IF(A1=1,\"x\",\"y\")", "x", "str") + QStringLiteral("</row>");
        Run run;
        QVERIFY(runWorkbook(buildXlsx({worksheet(rows)}), &run, [] {
            RecalcOptions o;
            o.keepMatches = 100;
            return o;
        }()));
        const RecalcReport &r = run.report;
        QCOMPARE(r.formulaCells, qint64(13));
        QCOMPARE(r.byClass.value('M'), qint64(5));      // A3, A11, A12, A13, and A15
        QCOMPARE(run.cellText("Sheet1", "A3"), QStringLiteral("M|SUM(A1:A2)|3|3"));
        QCOMPARE(run.cellText("Sheet1", "A4"), QStringLiteral("X|A3*2|7|6"));
        QCOMPARE(run.cellText("Sheet1", "A5").left(2), QStringLiteral("U|"));
        QCOMPARE(run.cellText("Sheet1", "A6").left(2), QStringLiteral("V|"));
        QCOMPARE(run.cellText("Sheet1", "A7").left(2), QStringLiteral("E|"));
        QCOMPARE(run.cellText("Sheet1", "A8").left(2), QStringLiteral("T|"));
        QCOMPARE(run.cellText("Sheet1", "A9").left(2), QStringLiteral("P|"));
        QCOMPARE(run.cellText("Sheet1", "A10").left(2), QStringLiteral("N|"));
        QCOMPARE(r.byClass.value('X'), qint64(1));
        QCOMPARE(r.byClass.value('U'), qint64(1));
        QCOMPARE(r.byClass.value('E'), qint64(1));
        QCOMPARE(r.byClass.value('T'), qint64(1));
        QCOMPARE(r.byClass.value('P'), qint64(1));
        QCOMPARE(r.byClass.value('N'), qint64(1));
        QCOMPARE(r.checked(), qint64(6));
        QCOMPARE(r.unsupportedFunctions.value("FOO"), qint64(1));
        // OFFSET depends on cells only: worked out, counted apart (and its match is told); NOW changes by itself and is not
        QCOMPARE(r.byClass.value('V'), qint64(2));
        QCOMPARE(r.volatileChecked, qint64(1));
        QCOMPARE(r.volatileMatched, qint64(1));
        QCOMPARE(r.functions.value("SUM").calls, qint64(1));
        QCOMPARE(r.functions.value("SUM").matchedCalls, qint64(1));
        QCOMPARE(r.functions.value("IF").matchedCells, qint64(1));
        QVERIFY(r.patterns.contains(QStringLiteral("number -> number: ")));
        QVERIFY(r.whole);
        QVERIFY(!r.manualCalc);
    }

    void sharedAndArrayFormulasAndOtherSheets()
    {
        const QString sheet1 = QStringLiteral("<row r=\"1\">") + plain("A1", "1") + QStringLiteral("<c r=\"B1\"><f t=\"shared\" ref=\"B1:B3\" si=\"0\">A1*10</f><v>10</v></c>")
            + QStringLiteral("<c r=\"C1\"><f t=\"array\" ref=\"C1:C2\">A1:A2*2</f><v>2</v></c></row><row r=\"2\">") + plain("A2", "2")
            + QStringLiteral("<c r=\"B2\"><f t=\"shared\" si=\"0\"/><v>20</v></c><c r=\"C2\"><v>4</v></c></row><row r=\"3\">") + plain("A3", "3")
            + QStringLiteral("<c r=\"B3\"><f t=\"shared\" si=\"0\"/><v>31</v></c></row>");
        const QString sheet2 = QStringLiteral("<row r=\"1\">") + cell("A1", "Sheet1!A2+1", "3") + QStringLiteral("</row><row r=\"2\">") + cell("A2", "'Sheet 1'!A2", "0")
            + QStringLiteral("</row><row r=\"3\">") + cell("A3", "SUM(Sheet1:Sheet2!A1)", "4") + QStringLiteral("</row>");
        Run run;
        RecalcOptions options;
        options.keepMatches = 100;
        QVERIFY(runWorkbook(buildXlsx({worksheet(sheet1), worksheet(sheet2)}), &run, options));
        QCOMPARE(run.cellText("Sheet1", "B1"), QStringLiteral("M|A1*10|10|10"));
        QCOMPARE(run.cellText("Sheet1", "B2"), QStringLiteral("M|A2*10|20|20"));          // a shared formula moved to its cell
        QCOMPARE(run.cellText("Sheet1", "B3"), QStringLiteral("X|A3*10|31|30"));
        QCOMPARE(run.cellText("Sheet1", "C1").left(2), QStringLiteral("M|"));            // the first cell of an array formula: its first element
        QCOMPARE(run.cellText("Sheet2", "A1"), QStringLiteral("M|Sheet1!A2+1|3|3"));
        QCOMPARE(run.cellText("Sheet2", "A2").left(2), QStringLiteral("X|"));            // 'Sheet 1' is no sheet of the workbook: #REF!
        QCOMPARE(run.cellText("Sheet2", "A3"), QStringLiteral("M|SUM(Sheet1:Sheet2!A1)|4|4"));   // 1 on the first sheet and 3 on the second
        QCOMPARE(run.report.formulaCells, qint64(7));
        QCOMPARE(run.report.byClass.value('X'), qint64(2));
    }

    void stateOfTheWorkbookIsTold()
    {
        Run manual;
        QVERIFY(runWorkbook(buildXlsx({worksheet(row(1, plain("A1", "1") + cell("B1", "A1+1", "2")))}, QByteArray(), QString(), QStringLiteral("calcMode=\"manual\"")), &manual));
        QVERIFY(manual.report.manualCalc);
        QCOMPARE(manual.report.checked(), qint64(1));
        // dates in the 1904 system
        Run d1904;
        RecalcOptions options;
        options.keepMatches = 10;
        QVERIFY(runWorkbook(buildXlsx({worksheet(row(1, cell("A1", "DATE(2024,3,15)", "43904") + cell("B1", "YEAR(A1)", "2024")))}, QByteArray(), QStringLiteral("date1904=\"1\"")), &d1904, options));
        QCOMPARE(d1904.report.byClass.value('M'), qint64(2));
        // defined names
        Run names;
        QVERIFY(runWorkbook(buildXlsx({worksheet(row(1, plain("A1", "4") + cell("B1", "Rate*100", "5") + cell("C1", "SUM(Block)", "4")))}, QByteArray(), QString(), QString(), QByteArray(), QStringList(),
                                      QStringLiteral("<definedName name=\"Rate\">0.05</definedName><definedName name=\"Block\">Sheet1!$A$1:$A$1</definedName>")),
                            &names, options));
        QCOMPARE(names.report.byClass.value('M'), qint64(2));
    }

    void whatIsLeftOfTheWorkIsKeptApart()
    {
        // a formula that takes more steps than the bound, and the time given
        QString rows;
        for (int i = 1; i <= 50; ++i) rows += QStringLiteral("<row r=\"%1\">").arg(i) + plain(QStringLiteral("A%1").arg(i), "1") + QStringLiteral("</row>");
        rows += QStringLiteral("<row r=\"60\">") + cell("A60", "SUM(A1:A50)", "50") + QStringLiteral("</row>");
        const QByteArray zip = buildXlsx({worksheet(rows)});
        Run limited;
        RecalcOptions tight;
        tight.workLimit = 20;
        QVERIFY(runWorkbook(zip, &limited, tight));
        QCOMPARE(limited.report.byClass.value('L'), qint64(1));
        QCOMPARE(limited.report.checked(), qint64(0));
        Run fine;
        QVERIFY(runWorkbook(zip, &fine));
        QCOMPARE(fine.report.byClass.value('M'), qint64(1));
        Run timed;
        RecalcOptions noTime;
        noTime.seconds = 1e-9;
        QVERIFY(runWorkbook(zip, &timed, noTime));
        QCOMPARE(timed.report.byClass.value('S'), qint64(1));
    }

    // The sheets of a workbook are held in memory together; past what they may take, the sheets not yet read are left out, and the report says so.
    void sheetsPastWhatAWorkbookMayTakeAreLeftOut()
    {
        const QByteArray zip = buildXlsx({worksheet(row(1, plain("A1", "1") + cell("B1", "A1+1", "2"))), worksheet(row(1, plain("A1", "1") + cell("B1", "A1+1", "2")))});
        Run all;
        QVERIFY(runWorkbook(zip, &all));
        QCOMPARE(all.report.formulaCells, qint64(2));
        QVERIFY(all.report.whole);
        Run cut;
        RecalcOptions small;
        small.workbookBytes = 1;
        QVERIFY(runWorkbook(zip, &cut, small));
        QCOMPARE(cut.report.formulaCells, qint64(1));    // the first sheet is read, and then the bound is passed
        QVERIFY(!cut.report.whole);
    }

    void theCellsOfAWorkbookAreReadFromTheStoredValuesInAnyKind()
    {
        // texts from the shared strings, true and false, errors, and an empty cell, read as values by the engine
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"s\"><v>0</v></c><c r=\"B1\" t=\"b\"><v>1</v></c><c r=\"C1\" t=\"e\"><v>#N/A</v></c><c r=\"D1\"/></row><row r=\"2\">")
            + cell("A2", "A1&\"!\"", "hi!", "str") + cell("B2", "IF(B1,\"yes\",\"no\")", "yes", "str") + cell("C2", "ISNA(C1)", "1", "b") + cell("D2", "ISBLANK(D1)", "1", "b")
            + cell("E2", "COUNTA(A1:D1)", "3") + QStringLiteral("</row>");
        Run run;
        RecalcOptions options;
        options.keepMatches = 100;
        QVERIFY(runWorkbook(buildXlsx({worksheet(rows)}, sharedStringsXml({QStringLiteral("<t>hi</t>")})), &run, options));
        QCOMPARE(run.report.byClass.value('M'), qint64(5));
        QCOMPARE(run.report.checked(), qint64(5));
    }
};

JP_TEST_CLASS(SheetRecalcTests)
#include "tst_sheet_recalc.moc"
