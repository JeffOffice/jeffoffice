// Tests of the turning of a formula's tokens, as the old binary workbook format stores them, back into the text the engine reads: every kind of operand and operator, the
// functions with and without a count, the layout hints that are passed over, references to sheets, books, and names, array constants, relative references of a shared formula,
// and what is refused. The tokens are built here, byte by byte.
#include "xlshelpers.h"

#include "sheet/xlsformula.h"

namespace {

XlsContext context(int biff = 8)
{
    XlsContext cx;
    cx.biff = biff;
    cx.sheetNames = {QStringLiteral("Sheet1"), QStringLiteral("My Sheet"), QStringLiteral("A1"), QStringLiteral("It's"), QStringLiteral("2020"), QStringLiteral("Data")};
    XlsSupBook self;
    self.kind = XlsSupBook::Kind::Internal;
    XlsSupBook other;
    other.kind = XlsSupBook::Kind::External;
    other.url = QStringLiteral("other.xls");
    other.sheets = {QStringLiteral("Prices"), QStringLiteral("Old Prices")};
    other.names = {QStringLiteral("Rate")};
    other.ordinal = 1;
    XlsSupBook addin;
    addin.kind = XlsSupBook::Kind::AddIn;
    addin.names = {QStringLiteral("EUROCONVERT")};
    cx.supbooks = {self, other, addin};
    // the list of sheets that tokens name: 0 Sheet1, 1 My Sheet, 2 Sheet1:My Sheet, 3 a deleted sheet, 4 A1, 5 It's, 6 2020, 7 Prices of the other book, 8 its two sheets, 9 the other book itself,
    // 10 the add-ins
    cx.xti = {{0, 0, 0}, {0, 1, 1}, {0, 0, 1}, {0, -1, -1}, {0, 2, 2}, {0, 3, 3}, {0, 4, 4}, {1, 0, 0}, {1, 0, 1}, {1, -2, -2}, {2, -2, -2}};
    XlsNameInfo rate;
    rate.text = QStringLiteral("Rate");
    XlsNameInfo local;
    local.text = QStringLiteral("Local");
    local.localSheet = 1;
    cx.names = {rate, local};
    cx.bytesToText = [](const char *p, int n) { return QString::fromLatin1(p, n); };
    return cx;
}

QString text(const QByteArray &rgce, const QByteArray &extra = QByteArray(), const XlsFormulaOptions &options = XlsFormulaOptions(), int biff = 8)
{
    const XlsFormulaResult r = xlsFormulaText(context(biff), rgce, extra, options);
    return r.ok() ? r.text : QStringLiteral("!") + r.why;
}

bool refused(const QByteArray &rgce, const QByteArray &extra = QByteArray(), int biff = 8) { return !xlsFormulaText(context(biff), rgce, extra, XlsFormulaOptions()).ok(); }

} // namespace

class SheetXlsFormulaTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Each operator, joined to its two operands; the layout of the text is that of the tokens (a parenthesis is a token of its own, so none is added).
    void operatorsJoinTheirOperands()
    {
        struct Op {
            int ptg;
            const char *sign;
        };
        const Op ops[] = {{0x03, "+"}, {0x04, "-"}, {0x05, "*"}, {0x06, "/"}, {0x07, "^"}, {0x08, "&"}, {0x09, "<"}, {0x0A, "<="}, {0x0B, "="}, {0x0C, ">="}, {0x0D, ">"}, {0x0E, "<>"},
                          {0x0F, " "}, {0x10, ","}, {0x11, ":"}};
        for (const Op &op : ops) QCOMPARE(text(tRef(0, 0) + tRef(1, 1) + tOp(op.ptg)), QStringLiteral("A1%1B2").arg(QLatin1String(op.sign)));
        QCOMPARE(text(tInt(1) + tInt(2) + tOp(0x03) + tInt(3) + tOp(0x05)), QStringLiteral("1+2*3"));   // (as the tokens order them: the sum, then the product with 3)
        QCOMPARE(text(tInt(1) + tInt(2) + tOp(0x03) + tOp(0x15) + tInt(3) + tOp(0x05)), QStringLiteral("(1+2)*3"));
        QCOMPARE(text(tRef(0, 0) + tOp(0x13)), QStringLiteral("-A1"));
        QCOMPARE(text(tRef(0, 0) + tOp(0x12)), QStringLiteral("+A1"));
        QCOMPARE(text(tInt(5) + tOp(0x14)), QStringLiteral("5%"));
        QCOMPARE(text(tRef(0, 0) + tRef(1, 0) + tOp(0x10) + tOp(0x15)), QStringLiteral("(A1,A2)"));
    }

    // The constants: whole numbers, floating ones in the shortest form that reads back the same, texts with their quotes doubled, true and false, errors, and a missing argument.
    void constantsAreWrittenAsTheFormulaLanguageHasThem()
    {
        QCOMPARE(text(tInt(0)), QStringLiteral("0"));
        QCOMPARE(text(tInt(65535)), QStringLiteral("65535"));
        QCOMPARE(text(tNum(0.1)), QStringLiteral("0.1"));
        QCOMPARE(text(tNum(1.5e20)), QStringLiteral("1.5E+20"));
        QCOMPARE(text(tNum(-2.5e-7)), QStringLiteral("-2.5E-07"));
        QCOMPARE(text(tNum(1.0 / 3.0)), QStringLiteral("0.3333333333333333"));   // (the fewest digits, from 15, that read back as the same number)
        QCOMPARE(text(tNum(123456789012345678.0)), QStringLiteral("1.2345678901234568E+17"));
        QCOMPARE(text(tStr(QStringLiteral("say \"hi\""))), QStringLiteral("\"say \"\"hi\"\"\""));
        QCOMPARE(text(tStr(QString())), QStringLiteral("\"\""));
        QCOMPARE(text(tStr(QStringLiteral("abc"), 5), QByteArray(), XlsFormulaOptions(), 5), QStringLiteral("\"abc\""));
        QCOMPARE(text(tBool(true)), QStringLiteral("TRUE"));
        QCOMPARE(text(tBool(false)), QStringLiteral("FALSE"));
        const QList<QPair<int, QString>> errors = {{0x00, "#NULL!"}, {0x07, "#DIV/0!"}, {0x0F, "#VALUE!"}, {0x17, "#REF!"}, {0x1D, "#NAME?"}, {0x24, "#NUM!"}, {0x2A, "#N/A"}};
        for (const auto &e : errors) QCOMPARE(text(tErr(e.first)), e.second);
        QCOMPARE(text(tInt(1) + tMissing() + tInt(2) + tFuncVar(4, 3)), QStringLiteral("SUM(1,,2)"));
        QCOMPARE(text(tMissing() + tFuncVar(4, 1)), QStringLiteral("SUM()"));
    }

    // Functions: those with the count their definition fixes, those with a count of their own, one the format calls through a name (an add-in), and the single-argument sum
    // that the program writes as a hint.
    void functionsAreCalledByNumber()
    {
        QCOMPARE(text(tFunc(19)), QStringLiteral("PI()"));                                           // no arguments
        QCOMPARE(text(tRef(0, 0) + tFunc(24)), QStringLiteral("ABS(A1)"));                            // one
        QCOMPARE(text(tRef(0, 0) + tInt(2) + tFunc(27)), QStringLiteral("ROUND(A1,2)"));              // two
        QCOMPARE(text(tInt(1) + tInt(2) + tInt(3) + tFunc(65)), QStringLiteral("DATE(1,2,3)"));       // three
        QCOMPARE(text(tInt(1) + tInt(2) + tInt(3) + tInt(4) + tFunc(119)), QStringLiteral("REPLACE(1,2,3,4)"));
        QCOMPARE(text(tInt(1) + tInt(2) + tInt(3) + tFuncVar(4, 3)), QStringLiteral("SUM(1,2,3)"));
        QCOMPARE(text(tFunc(34)), QStringLiteral("TRUE()"));
        QCOMPARE(text(tArea(0, 0, 2, 0) + tAttrSum()), QStringLiteral("SUM(A1:A3)"));
        QCOMPARE(text(tRef(0, 0) + tRef(1, 0) + tFuncVar(228, 2)), QStringLiteral("SUMPRODUCT(A1,A2)"));
        QCOMPARE(text(tRef(0, 0) + tInt(1) + tInt(2) + tFuncVar(100, 3)), QStringLiteral("CHOOSE(A1,1,2)"));
        // a name called with arguments: the name is the first of them
        QCOMPARE(text(tNameX(10, 1) + tInt(100) + tStr(QStringLiteral("USD")) + tFuncVar(255, 3, 0x40)), QStringLiteral("EUROCONVERT(100,\"USD\")"));
        QCOMPARE(text(tName(1) + tInt(1) + tFuncVar(255, 2, 0x40)), QStringLiteral("Rate(1)"));
        // a function that is not in the list, a call with more arguments than there are operands, and a command function (of macro sheets)
        QVERIFY(refused(tInt(1) + tFunc(0x7FFF)));
        QVERIFY(refused(tInt(1) + tFuncVar(4, 2)));
        QVERIFY(refused(tInt(1) + tFuncVar(0x8004, 1)));
        QVERIFY(refused(tInt(1) + tFunc(1)));   // (IF always has a count)
    }

    // The jumps and hints that help the original program evaluate IF and CHOOSE are passed over, and so are spaces.
    void layoutHintsArePassedOver()
    {
        // IF(A1>0,"yes","no"): A1, 0, >, the jump past the first branch, "yes", the jump to the end, "no", the end of the jump, IF with three
        const QByteArray ifs = tRef(0, 0) + tInt(0) + tOp(0x0D) + tAttrIf(10) + tStr(QStringLiteral("yes")) + tAttrGoto(7) + tStr(QStringLiteral("no")) + tAttrGoto(3) + tFuncVar(1, 3);
        QCOMPARE(text(ifs), QStringLiteral("IF(A1>0,\"yes\",\"no\")"));
        QVERIFY(refused(tAttrSpace() + tInt(1) + tAttrSpace(1, 2) + tOp(0x03)));   // (a sum of one operand is none)
        QCOMPARE(text(tInt(1) + tAttrSpace(0, 3) + tInt(2) + tAttrSpace(2, 1) + tOp(0x03)), QStringLiteral("1+2"));
        QCOMPARE(text(tRef(0, 0) + tAttrChoose({6, 6, 6}) + tInt(1) + tAttrGoto(2) + tInt(2) + tAttrGoto(0) + tFuncVar(100, 3)), QStringLiteral("CHOOSE(A1,1,2)"));
        // a choice that claims more offsets than the formula has bytes
        QVERIFY(refused(W().u8(0x19).u8(0x04).u16(60000).raw(QByteArray(4, '\0'))));
        // the hint for a volatile formula
        QCOMPARE(text(W().u8(0x19).u8(0x01).u16(0) + tFunc(74)), QStringLiteral("NOW()"));
    }

    // Cells and areas, with a dollar where the part does not move: all four forms of a cell, the whole column and the whole row, and an area of one cell, which stays as it was written.
    void referencesCarryTheirDollars()
    {
        QCOMPARE(text(tRef(0, 0, true, true)), QStringLiteral("A1"));
        QCOMPARE(text(tRef(0, 0, false, false)), QStringLiteral("$A$1"));
        QCOMPARE(text(tRef(4, 27, false, true)), QStringLiteral("AB$5"));
        QCOMPARE(text(tRef(4, 27, true, false)), QStringLiteral("$AB5"));
        QCOMPARE(text(tRef(65535, 255)), QStringLiteral("IV65536"));
        QCOMPARE(text(tArea(0, 0, 9, 3, false, false)), QStringLiteral("$A$1:$D$10"));
        QCOMPARE(text(tArea(2, 2, 2, 2)), QStringLiteral("C3:C3"));
        QCOMPARE(text(tArea(0, 0, 65535, 0, false, false)), QStringLiteral("$A:$A"));
        QCOMPARE(text(tArea(0, 1, 65535, 3, true, true)), QStringLiteral("B:D"));
        QCOMPARE(text(tArea(4, 0, 6, 255, false, false)), QStringLiteral("$5:$7"));
        // the value class and the array class are references too
        QCOMPARE(text(tRef(0, 0, true, true, 8, 0x40)), QStringLiteral("A1"));
        QCOMPARE(text(tArea(0, 0, 1, 1, true, true, 8, 0x60)), QStringLiteral("A1:B2"));
        // a cell that is gone
        QCOMPARE(text(W().u8(0x2A).u16(0).u16(0)), QStringLiteral("#REF!"));
        QCOMPARE(text(W().u8(0x2B).u16(0).u16(0).u16(0).u16(0)), QStringLiteral("#REF!"));
        // Excel 5: the rows are 14 bits and the columns a byte
        QCOMPARE(text(tRef(16383, 255, true, true, 5), QByteArray(), XlsFormulaOptions(), 5), QStringLiteral("IV16384"));
        QCOMPARE(text(tArea(0, 0, 16383, 2, false, true, 5), QByteArray(), XlsFormulaOptions(), 5), QStringLiteral("A:C"));
        QCOMPARE(text(tArea(1, 1, 3, 3, false, false, 5), QByteArray(), XlsFormulaOptions(), 5), QStringLiteral("$B$2:$D$4"));
    }

    // References that name a sheet: in this workbook (quoted when the name needs it: a space, an apostrophe, a name that looks like a cell, one that begins with a digit), through
    // several sheets, in another workbook (numbered as the XML form numbers them), and to a sheet that is gone.
    void sheetsAreNamedWithTheQuotesTheyNeed()
    {
        QCOMPARE(text(tRef3d(0, 0, 0)), QStringLiteral("Sheet1!A1"));
        QCOMPARE(text(tRef3d(1, 1, 2, false, false)), QStringLiteral("'My Sheet'!$C$2"));
        QCOMPARE(text(tArea3d(2, 0, 0, 1, 1)), QStringLiteral("'Sheet1:My Sheet'!A1:B2"));
        QCOMPARE(text(tRef3d(3, 0, 0)), QStringLiteral("#REF!"));
        QCOMPARE(text(tArea3d(3, 0, 0, 1, 1)), QStringLiteral("#REF!"));
        QCOMPARE(text(tRef3d(4, 0, 0)), QStringLiteral("'A1'!A1"));
        QCOMPARE(text(tRef3d(5, 0, 0)), QStringLiteral("'It''s'!A1"));
        QCOMPARE(text(tRef3d(6, 0, 0)), QStringLiteral("'2020'!A1"));
        QCOMPARE(text(tRef3d(7, 3, 1)), QStringLiteral("[1]Prices!B4"));
        QCOMPARE(text(tArea3d(8, 0, 0, 1, 1)), QStringLiteral("'[1]Prices:Old Prices'!A1:B2"));
        QCOMPARE(text(tRef3d(99, 0, 0)), QStringLiteral("#REF!"));   // (a place in the list that is not there)
        // names: of the workbook, of a sheet other than the one the formula is in, and of that sheet itself; and a name of the other workbook
        QCOMPARE(text(tName(1)), QStringLiteral("Rate"));
        QCOMPARE(text(tName(2)), QStringLiteral("'My Sheet'!Local"));
        XlsFormulaOptions inThatSheet;
        inThatSheet.sheet = 1;
        QCOMPARE(text(tName(2), QByteArray(), inThatSheet), QStringLiteral("Local"));
        QCOMPARE(text(tNameX(9, 1)), QStringLiteral("[1]!Rate"));
        QCOMPARE(text(tName(0)), QStringLiteral("!A formula has a token this program does not read."));
        QVERIFY(refused(tName(3)));
        // Excel 5: a sheet by the number of a record of its list, or by a number below 0 for the workbook's own
        XlsContext five = context(5);
        five.externSheet5 = {QByteArray(1, char(3)) + "Data", QByteArray(1, char(1)) + "other.xls" + QByteArray(1, char(3)) + "Prices"};
        XlsFormulaOptions none;
        QCOMPARE(xlsFormulaText(five, tRef3d5(1, 0, 0, 0, 0), QByteArray(), none).text, QStringLiteral("Data!A1"));
        QCOMPARE(xlsFormulaText(five, tRef3d5(2, 0, 0, 1, 1), QByteArray(), none).text, QStringLiteral("[1]Prices!B2"));
        QCOMPARE(xlsFormulaText(five, tRef3d5(-1, 1, 1, 2, 2), QByteArray(), none).text, QStringLiteral("'My Sheet'!C3"));
        QVERIFY(!xlsFormulaText(five, tRef3d5(9, 0, 0, 0, 0), QByteArray(), none).ok() || xlsFormulaText(five, tRef3d5(9, 0, 0, 0, 0), QByteArray(), none).text == QStringLiteral("#REF!"));
    }

    // Array constants follow the formula, in the order of the tokens that stand for them: numbers, texts, true and false, errors, and cells with nothing in them; rows by
    // semicolons and columns by commas.
    void arrayConstantsComeAfterTheFormula()
    {
        const QByteArray values = arrayValues({{1.0, QStringLiteral("two")}, {true, 0x2Au}});
        QCOMPARE(text(tArray(), values), QStringLiteral("{1,\"two\";TRUE,#N/A}"));
        // two arrays, with a token between them that is not one: the second is the second of the values
        const QByteArray two = arrayValues({{1.0}}) + arrayValues({{2.0, 3.0}});
        QCOMPARE(text(tArray() + tArray() + tOp(0x03), two), QStringLiteral("{1}+{2,3}"));
        // the cells a memory token remembers come first
        const QByteArray remembered = W().u16(1).raw(QByteArray(8, '\0')) + arrayValues({{5.0}});
        QCOMPARE(text(W().u8(0x26).raw(QByteArray(4, '\0')).u16(0) + tArray(), remembered), QStringLiteral("{5}"));
        QCOMPARE(text(tArray(), arrayValues({{QVariant()}})), QStringLiteral("{\"\"}"));
        // Excel 5 writes the count of columns in a byte (0 for 256) and the texts with a byte of length
        QCOMPARE(text(tArray(), arrayValues({{1.0, QStringLiteral("x")}}, 5), XlsFormulaOptions(), 5), QStringLiteral("{1,\"x\"}"));
        // the values are missing, or claim more cells than there are bytes
        QVERIFY(refused(tArray(), QByteArray()));
        QVERIFY(refused(tArray(), W().u8(255).u16(65535).raw(QByteArray(20, '\0'))));
        QVERIFY(refused(tArray(), W().u8(0).u16(0).u8(0x77).raw(QByteArray(8, '\0'))));   // (a kind of value that is none)
    }

    // The tokens of a memory area or a reference-valued function only say that what follows is a reference: the tokens that follow are the formula.
    void memoryTokensArePassedOver()
    {
        QCOMPARE(text(tMemFunc(11) + tRef(0, 0) + tRef(5, 0) + tOp(0x11)), QStringLiteral("A1:A6"));
        QCOMPARE(text(W().u8(0x29 + 0x20).u16(0) + tRef(0, 0)), QStringLiteral("A1"));
        QCOMPARE(text(W().u8(0x27).raw(QByteArray(4, '\0')).u16(0) + tErr(0x17)), QStringLiteral("#REF!"));
        QCOMPARE(text(W().u8(0x28).raw(QByteArray(4, '\0')).u16(0) + tRef(1, 1)), QStringLiteral("B2"));
        QCOMPARE(text(W().u8(0x2E).u16(0) + tRef(1, 1)), QStringLiteral("B2"));
    }

    // In a shared formula and in a name a relative part is a distance from the cell the formula is written for, counted round the edge: the text is written for that cell, and
    // `outside` says when a part left the old form's sheet, so that the formula cannot be shared by cells that count it differently.
    void relativePartsAreDistancesFromTheBase()
    {
        XlsFormulaOptions shared;
        shared.relative = true;
        shared.baseRow = 4;
        shared.baseCol = 2;
        XlsFormulaResult r = xlsFormulaText(context(), tRefN(-1, 1) + tRefN(2, -2, false, true) + tOp(0x03), QByteArray(), shared);
        QVERIFY(r.ok());
        QCOMPARE(r.text, QStringLiteral("D4+A$3"));   // (the second: the row is absolute (2, so row 3), the column is 2 - 2 = 0)
        QVERIFY(!r.outside);
        r = xlsFormulaText(context(), tAreaN(0, -1, 3, 1), QByteArray(), shared);
        QCOMPARE(r.text, QStringLiteral("B5:D8"));
        // the left of column A is the last column of the old form
        shared.baseCol = 0;
        r = xlsFormulaText(context(), tRefN(0, -1), QByteArray(), shared);
        QCOMPARE(r.text, QStringLiteral("IV5"));
        QVERIFY(r.outside);
        shared.baseRow = 0;
        shared.baseCol = 0;
        r = xlsFormulaText(context(), tRefN(-1, 0), QByteArray(), shared);
        QCOMPARE(r.text, QStringLiteral("A65536"));
        QVERIFY(r.outside);
        // a name counts round the edge of the grid of the XML form, which is wider
        XlsFormulaOptions name;
        name.relative = true;
        name.name = true;
        r = xlsFormulaText(context(), tRef3d(0, 0, 0xFFFE, true, true).left(1) + QByteArray(), QByteArray(), name);
        QVERIFY(!r.ok());   // (a token cut short)
        r = xlsFormulaText(context(), tRefN(0, -2), QByteArray(), name);
        QCOMPARE(r.text, QStringLiteral("XFC1"));
        // Excel 5 counts rows in 14 bits
        XlsFormulaOptions five;
        five.relative = true;
        five.baseRow = 3;
        r = xlsFormulaText(context(5), W().u8(0x2C).u16(0x8000 | 0x4000 | (0x4000 - 5)).u8(0), QByteArray(), five);
        QCOMPARE(r.text, QStringLiteral("A16383"));   // (3 - 5 = -2, round the 16384 rows)
        QVERIFY(r.outside);
    }

    // A formula that is damaged is refused, not guessed at: tokens cut short, a stack that does not end in one value, an operator with too few operands, a token that is none,
    // a cell that other cells share, and a formula longer than the bound.
    void whatIsDamagedIsRefused()
    {
        QVERIFY(refused(QByteArray()));
        QVERIFY(refused(tInt(1) + tInt(2)));
        QVERIFY(refused(tOp(0x03)));
        QVERIFY(refused(tInt(1) + tOp(0x03)));
        QVERIFY(refused(tNum(1.5).left(5)));
        QVERIFY(refused(tRef(0, 0).left(3)));
        QVERIFY(refused(tStr(QStringLiteral("abc")).left(3)));
        QVERIFY(refused(W().u8(0x80)));
        QVERIFY(refused(W().u8(0x18).u8(0x19).raw(QByteArray(12, '\0'))));
        QVERIFY(refused(W().u8(0x01).u16(0).u16(0)));    // (the cell of a shared formula)
        QVERIFY(refused(W().u8(0x02).u16(0).u16(0)));
        QVERIFY(refused(tInt(1) + tOp(0x15) + tOp(0x15) + tOp(0x15) + tOp(0x03)));
        XlsFormulaOptions small;
        small.maxChars = 20;
        QVERIFY(!xlsFormulaText(context(), tStr(QString(100, QLatin1Char('x'))), QByteArray(), small).ok());
        QByteArray many = tInt(1);
        for (int i = 0; i < 20000; ++i) many += tInt(1) + tOp(0x03);
        // (a sum of twenty thousand terms is read: 8 bytes become 2 characters, and the text is 40,001 characters, which the default bound of 100,000 allows)
        const XlsFormulaResult r = xlsFormulaText(context(), many, QByteArray(), XlsFormulaOptions());
        QVERIFY(r.ok());
        QCOMPARE(r.text.size(), 40001);
        // a thousand parentheses are a thousand tokens, not a thousand levels of anything
        QByteArray nest = tInt(1);
        for (int i = 0; i < 5000; ++i) nest += tOp(0x15);
        QCOMPARE(text(nest).size(), 5001 + 5000);
    }
};

JP_TEST_CLASS(SheetXlsFormulaTests)
#include "tst_sheet_xlsformula.moc"
