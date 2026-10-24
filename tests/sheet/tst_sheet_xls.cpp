// Tests of the reader of the old binary workbook format (BIFF8 and BIFF5): the sheets of a workbook, the cell records, the string table with its
// continuations, formula results, shared formulas, arrays and data tables, defined names, the styles, the layout of a sheet, BIFF5's own forms, what is refused,
// and what is not read. Every workbook is built in memory by xlshelpers.
#include "xlshelpers.h"

#include "io/cfb.h"
#include "sheet/calculation.h"
#include "sheet/xlscodepage.h"

#include <QFile>
#include <QTemporaryDir>

namespace {

// Writes over the stack below the caller's, so that a reference to something a finished call held there reads what it held no longer.
#if defined(__GNUC__)
__attribute__((noinline))
#endif
void scrubTheStack()
{
    volatile char junk[65536];
    for (size_t i = 0; i < sizeof(junk); ++i) junk[i] = char(0xA5);
}

} // namespace

class SheetXlsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The sheets: names (in the old form's own characters), states, a chart sheet, the 1904 date system, and a workbook that is read from a compound file or from the
    // bytes.
    void theSheetsOfAWorkbookAreListed()
    {
        Book b;
        b.globals(rec(0x0022, W().u16(1)));
        b.sheet(QStringLiteral("Data"), number(0, 0, 1.5));
        b.sheet(QStringLiteral("Hidden one"), QByteArray(), 1);
        b.sheet(QStringLiteral("Secret"), QByteArray(), 2);
        b.sheet(QStringLiteral("Chart"), QByteArray(), 0, 2);
        b.sheet(QStringLiteral("Café"), blank(0, 0));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        QVERIFY(wb.isBinary());
        QCOMPARE(wb.info().sheets.size(), 5);
        QCOMPARE(wb.info().sheets[0].name, QStringLiteral("Data"));
        QVERIFY(wb.info().sheets[0].state == SheetState::Visible);
        QVERIFY(wb.info().sheets[1].state == SheetState::Hidden);
        QVERIFY(wb.info().sheets[2].state == SheetState::VeryHidden);
        QVERIFY(wb.info().sheets[3].kind == SheetKind::Chartsheet);
        QCOMPARE(wb.info().sheets[4].name, QStringLiteral("Café"));
        QVERIFY(wb.info().dateSystem == DateSystem::Date1904);
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1.5"));
        QVERIFY(wb.readSheet(3, &s));   // a chart sheet has no cells
        QCOMPARE(s.cells.cellCount(), qint64(0));
        QVERIFY(!wb.readSheet(5, &s));
        QVERIFY(!wb.readSheet(-1, &s));
        // The bytes of the stream alone are not a file in a container, but a bare stream of the same records is read too.
        Workbook bare;
        QVERIFY2(bare.openBinary(b.stream(), &why), qPrintable(why));
        QCOMPARE(bare.info().sheets.size(), 5);
        // The default is the 1900 system, and the calculation mode and the active tab come from their records.
        Book c;
        c.globals(rec(0x000D, W().u16(0)) + rec(0x0011, W().u16(1)) + rec(0x003D, W().u16(0).u16(0).u16(0).u16(0).u16(0).u16(1).u16(0).u16(1).u16(600)));
        c.sheet(QStringLiteral("A"), QByteArray()).sheet(QStringLiteral("B"), QByteArray());
        Workbook cw;
        QVERIFY2(openBook(c, &cw, &why), qPrintable(why));
        QVERIFY(cw.info().dateSystem == DateSystem::Date1900);
        QVERIFY(cw.info().calcMode == CalcMode::Manual);
        QVERIFY(cw.info().iterate);
        QCOMPARE(cw.info().activeTab, 1);
    }

    // The cell records: numbers, the packed numbers (whole, hundredths, and floating) singly and in runs, strings of the table and in the cell, true and false, errors,
    // and blanks, which count only when they have a look of their own.
    void theCellRecordsAreRead()
    {
        Book b;
        QByteArray cells = number(0, 0, 3.25) + rk(0, 1, (7u << 2) | 2) + rk(0, 2, (1234u << 2) | 3) + rk(0, 3, 0x3FE00000u) + rk(0, 4, (quint32(-3) << 2) | 2);
        cells += mulrk(1, 1, {(1u << 2) | 2, (2u << 2) | 2, (3u << 2) | 2});
        cells += labelSst(2, 0, 1) + labelSst(2, 1, 0) + label(2, 2, QStringLiteral("plain"));
        cells += boolErr(3, 0, 1, false) + boolErr(3, 1, 0, false) + boolErr(3, 2, 0x07, true) + boolErr(3, 3, 0x2A, true) + boolErr(3, 4, 0x17, true) + boolErr(3, 5, 0x1D, true);
        cells += blank(4, 0, 16) + blank(4, 1, 15) + mulblank(5, 0, 3, 17) + number(6, 0, 1, 0);
        b.sheet(QStringLiteral("S"), cells);
        b.after(sst({QStringLiteral("alpha"), QStringLiteral("beta")}));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        auto at = [&](const QString &ref) { return shown(wb, s, ref); };
        QCOMPARE(at("A1"), QStringLiteral("n:3.25"));
        QCOMPARE(at("B1"), QStringLiteral("n:7"));
        QCOMPARE(at("C1"), QStringLiteral("n:12.34"));
        QCOMPARE(at("D1"), QStringLiteral("n:0.5"));
        QCOMPARE(at("E1"), QStringLiteral("n:-3"));
        QCOMPARE(at("B2"), QStringLiteral("n:1"));
        QCOMPARE(at("C2"), QStringLiteral("n:2"));
        QCOMPARE(at("D2"), QStringLiteral("n:3"));
        QCOMPARE(at("A3"), QStringLiteral("s:beta"));
        QCOMPARE(at("B3"), QStringLiteral("s:alpha"));
        QCOMPARE(at("C3"), QStringLiteral("i:plain"));
        QCOMPARE(at("A4"), QStringLiteral("b:1"));
        QCOMPARE(at("B4"), QStringLiteral("b:0"));
        QCOMPARE(at("C4"), QStringLiteral("e:#DIV/0!"));
        QCOMPARE(at("D4"), QStringLiteral("e:#N/A"));
        QCOMPARE(at("E4"), QStringLiteral("e:#REF!"));
        QCOMPARE(at("F4"), QStringLiteral("e:#NAME?"));
        // A blank cell is a cell only when its format is not the default; the default format is the 16th of the file and is style 0 here, the file's first is style 15.
        QCOMPARE(at("A5"), QStringLiteral("-"));
        QCOMPARE(int(sheettest::at(s, QStringLiteral("A5"))->style), 16);
        QCOMPARE(at("B5"), QStringLiteral("none"));
        QCOMPARE(at("A6"), QStringLiteral("-"));
        QCOMPARE(at("C6"), QStringLiteral("-"));
        QCOMPARE(int(sheettest::at(s, QStringLiteral("C6"))->style), 17);
        QCOMPARE(int(sheettest::at(s, QStringLiteral("A7"))->style), 15);
        QCOMPARE(int(sheettest::at(s, QStringLiteral("A1"))->style), 0);
    }

    // The string table is in records of at most a few thousand bytes: a string whose characters are cut by the end of a record goes on in the next with a byte of its own
    // saying how the rest is written. Every place a record can end is tried, with strings that take one byte a character, two, and runs of rich text.
    void aStringCutAnywhereIsOneString()
    {
        const QStringList strings = {QStringLiteral("short"), QStringLiteral("a longer string of plain letters, to be cut in the middle"),
                                     QString::fromUtf16(u"ΑΒΓΔΕΖΗ wide ΘΙΚ"), QString(), QStringLiteral("tail"),
                                     QString::fromUtf16(u"mixed éœ and 中文 text")};
        for (int cut = 9; cut < 160; ++cut) {
            Book b;
            b.sheet(QStringLiteral("S"), labelSst(0, 0, 1) + labelSst(1, 0, 2) + labelSst(2, 0, 5) + labelSst(3, 0, 4));
            b.after(sst(strings, cut, {0, 0, 0, 0, 0, 0}));
            Workbook wb;
            QString why;
            QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
            QVERIFY2(wb.sharedStrings().strings.size() == strings.size(), qPrintable(QStringLiteral("cut after %1 bytes").arg(cut)));
            for (int i = 0; i < strings.size(); ++i) QVERIFY2(wb.sharedStrings().strings[i] == strings[i], qPrintable(QStringLiteral("string %1 cut after %2 bytes").arg(i).arg(cut)));
        }
        // Rich text: the runs of formatting (four bytes each) are passed over, and the string is its characters.
        Book rich;
        rich.sheet(QStringLiteral("S"), labelSst(0, 0, 0));
        rich.after(sst({QStringLiteral("bold and plain"), QStringLiteral("next")}, 0, {3, 0}));
        Workbook wb;
        QVERIFY(openBook(rich, &wb));
        QCOMPARE(wb.sharedStrings().strings.value(0), QStringLiteral("bold and plain"));
        QCOMPARE(wb.sharedStrings().strings.value(1), QStringLiteral("next"));
    }

    // A formula cell has the result the file stored: a number, true or false, an error, a text that comes in the record after it (which may itself be continued), or the
    // empty text; a formula whose text record is missing has no result.
    void aFormulaKeepsItsStoredResult()
    {
        const QByteArray one = tInt(1) + tInt(2) + tOp(0x03);
        QByteArray cells = formulaNumber(0, 0, one, 3.5);
        cells += formulaSpecial(1, 0, one, 0) + stringRec(QStringLiteral("hello"));
        cells += formulaSpecial(2, 0, one, 1, 1) + formulaSpecial(3, 0, one, 2, 0x2A) + formulaSpecial(4, 0, one, 3);
        // a text of 300 characters whose record ends after 100 of them: the rest follows in a record that begins with the byte for one byte a character
        const QString longText(300, QLatin1Char('x'));
        cells += formulaSpecial(5, 0, one, 0);
        cells += rec(0x0207, W().u16(300).u8(0).raw(QByteArray(100, 'x'))) + rec(0x003C, W().u8(0).raw(QByteArray(200, 'x')));
        cells += formulaSpecial(6, 0, one, 0) + number(7, 0, 9);   // (no text record)
        // a text that is written in two bytes a character is continued as such
        cells += formulaSpecial(8, 0, one, 0);
        cells += rec(0x0207, W().u16(2).u8(1).u16(0x4E2D).u16(0x6587)) + formulaNumber(9, 0, one, 1);
        // a text in a LABEL record that goes on in a record after it
        cells += rec(0x0204, W().u16(10).u16(0).u16(15).u16(300).u8(0).raw(QByteArray(100, 'y'))) + rec(0x003C, W().u8(0).raw(QByteArray(200, 'y')));
        Book b;
        b.sheet(QStringLiteral("S"), cells);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        auto at = [&](const QString &ref) { return shown(wb, s, ref); };
        QCOMPARE(at("A11"), QStringLiteral("i:") + QString(300, QLatin1Char('y')));
        QCOMPARE(at("A1"), QStringLiteral("n:3.5"));
        QCOMPARE(sheettest::at(s, QStringLiteral("A1"))->hasFormula(), true);
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A1"))), QStringLiteral("1+2"));
        QCOMPARE(at("A2"), QStringLiteral("t:hello"));
        QCOMPARE(at("A3"), QStringLiteral("b:1"));
        QCOMPARE(at("A4"), QStringLiteral("e:#N/A"));
        QCOMPARE(at("A5"), QStringLiteral("t:"));
        QCOMPARE(at("A6"), QStringLiteral("t:") + longText);
        QCOMPARE(at("A7"), QStringLiteral("-"));
        QVERIFY(sheettest::at(s, QStringLiteral("A7"))->hasFormula());
        QVERIFY(!sheettest::at(s, QStringLiteral("A7"))->hasResult());
        QCOMPARE(at("A8"), QStringLiteral("n:9"));
        QCOMPARE(at("A9"), QString(QStringLiteral("t:")) + QChar(0x4E2D) + QChar(0x6587));
        QCOMPARE(at("A10"), QStringLiteral("n:1"));
        QCOMPARE(s.formulaCells, qint64(9));
        QCOMPARE(s.storedResults, qint64(8));
    }

    // A shared formula is kept once, written for its first cell, and each cell moves its references; one whose relative references leave the sheet at its first cell
    // is written out for each cell (the format wraps them round, as it counts only 256 columns).
    void sharedFormulasAreSharedOrWrittenOut()
    {
        QByteArray cells = number(0, 0, 1) + number(1, 0, 2) + number(2, 0, 3);
        // B1:B3 = A1*2, one formula for the three
        const QByteArray times2 = tRefN(0, -1) + tInt(2) + tOp(0x05);
        cells += formulaExp(0, 1, 0, 1, 2) + shrfmla(0, 2, 1, 1, times2) + formulaExp(1, 1, 0, 1, 4) + formulaExp(2, 1, 0, 1, 6);
        // C1:D1 = $A$1+A1... with a part absolute: ($A1 stays in column A and moves down)
        const QByteArray mixed = tRefN(0, 0, true, false) + tRef(5, 5, false, false) + tOp(0x03);
        cells += formulaExp(0, 2, 0, 2, 1) + shrfmla(0, 1, 2, 2, mixed) + formulaExp(1, 2, 0, 2, 2);
        // E1:G1 = the cell to the left: at E (column 4) it is D; at the first column it would be off the sheet, so a range that starts at column A wraps
        const QByteArray left = tRefN(0, -1);
        cells += formulaExp(5, 0, 5, 0, 0) + shrfmla(5, 5, 0, 2, left) + formulaExp(5, 1, 5, 0, 0) + formulaExp(5, 2, 5, 0, 0);
        cells += formulaExp(7, 4, 7, 4, 0) + shrfmla(7, 7, 4, 6, left) + formulaExp(7, 5, 7, 4, 0) + formulaExp(7, 6, 7, 4, 0);
        Book b;
        b.sheet(QStringLiteral("S"), cells);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
        auto f = [&](const QString &ref) {
            const Cell *c = sheettest::at(s, ref);
            return c ? s.formulaText(*c) : QStringLiteral("none");
        };
        QCOMPARE(f("B1"), QStringLiteral("A1*2"));
        QCOMPARE(f("B2"), QStringLiteral("A2*2"));
        QCOMPARE(f("B3"), QStringLiteral("A3*2"));
        QVERIFY(sheettest::at(s, QStringLiteral("B2"))->flags & SharedFormula);
        QCOMPARE(s.formulas[sheettest::at(s, QStringLiteral("B2"))->formula].text, s.formulas[sheettest::at(s, QStringLiteral("B1"))->formula].text);
        QCOMPARE(f("C1"), QStringLiteral("$A1+$F$6"));
        QCOMPARE(f("C2"), QStringLiteral("$A2+$F$6"));
        QCOMPARE(shown(wb, s, QStringLiteral("B3")), QStringLiteral("n:6"));
        // wrapped round: column A to the left is the last column of the old form (IV), and the cells after it count from there
        QCOMPARE(f("A6"), QStringLiteral("IV6"));
        QCOMPARE(f("B6"), QStringLiteral("A6"));
        QCOMPARE(f("C6"), QStringLiteral("B6"));
        QVERIFY(!(sheettest::at(s, QStringLiteral("A6"))->flags & SharedFormula));
        // not wrapped: shared as the others
        QCOMPARE(f("E8"), QStringLiteral("D8"));
        QCOMPARE(f("G8"), QStringLiteral("F8"));
        QVERIFY(sheettest::at(s, QStringLiteral("F8"))->flags & SharedFormula);
    }

    // The record of a shared formula follows the first cell that uses it, which need not be the first cell of the range it covers; the cells that share it name that cell.
    void aSharedFormulaIsNamedByTheCellItsRecordFollows()
    {
        QByteArray cells = number(1, 0, 1);
        // B2 and C2 have formulas of their own; D2 is the first to use the shared one, which covers B2:D3
        cells += formulaNumber(1, 1, tInt(1) + tInt(1) + tOp(0x03), 2) + formulaNumber(1, 2, tInt(2) + tInt(2) + tOp(0x03), 4);
        const QByteArray plusOne = tRefN(0, -1) + tInt(1) + tOp(0x03);
        cells += formulaExp(1, 3, 1, 3, 0) + shrfmla(1, 2, 1, 3, plusOne);
        cells += formulaExp(2, 1, 1, 3, 0) + formulaExp(2, 2, 1, 3, 0) + formulaExp(2, 3, 1, 3, 0);
        Book b;
        b.sheet(QStringLiteral("S"), cells);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(s.stats.unreadFormulas, qint64(0));
        auto f = [&](const QString &ref) { return s.formulaText(*sheettest::at(s, ref)); };
        QCOMPARE(f("B2"), QStringLiteral("1+1"));
        QCOMPARE(f("D2"), QStringLiteral("C2+1"));
        QCOMPARE(f("B3"), QStringLiteral("A3+1"));
        QCOMPARE(f("C3"), QStringLiteral("B3+1"));
        QCOMPARE(f("D3"), QStringLiteral("C3+1"));
    }

    // BIFF5 keeps the list of external sheets and names in each sheet, and names its own workbook's sheets with a number below 0.
    void biff5ListsExternalSheetsAndNamesInEachSheet()
    {
        Book b(5);
        QByteArray one = externCount5(1) + externSheet5(QByteArray("\x03Two", 4)) + externName5(QStringLiteral("MYFUNC"));
        one += formulaNumber(0, 0, tRef3d5(1, 0, 0, 4, 2) + tInt(2) + tOp(0x05), 0);                                        // =Two!C5*2 by the sheet's own list
        one += formulaNumber(1, 0, tRef3d5(-1, 1, 1, 0, 0, false, false) + tOp(0x13), 0);                                 // =-Two!$A$1, a sheet of this workbook named by number
        one += formulaNumber(2, 0, tNameX5(1, 1) + tInt(1) + tFuncVar(255, 2, 0x40), 0);                                    // =MYFUNC(1)
        b.sheet(QStringLiteral("One"), one).sheet(QStringLiteral("Two"), externCount5(1) + externSheet5(QByteArray("\x03One", 4)) + formulaNumber(0, 0, tRef3d5(1, 0, 0, 0, 0) + tInt(1) + tOp(0x03), 0));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(s.stats.unreadFormulas, qint64(0));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A1"))), QStringLiteral("Two!C5*2"));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A2"))), QStringLiteral("-Two!$A$1"));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A3"))), QStringLiteral("MYFUNC(1)"));
        QVERIFY(wb.readSheet(1, &s));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A1"))), QStringLiteral("One!A1+1"));   // (the list of the second sheet is its own)
    }

    // A formula that many cells share is read once, whatever the number of cells, and is read in work in proportion to its length: a chain of parentheses is no worse than a chain of
    // sums. The work is counted (in the sheet's statistics), not timed.
    void aSharedFormulaIsReadOnceForAllItsCells()
    {
        auto workOf = [&](int tokens, int rows, int baseCol, bool *whole = nullptr) {
            // the formula: the cell one column to the left (or, at column A, the last column: not shared), and `tokens` parentheses round it
            QByteArray rgce = tRefN(0, -1);
            for (int i = 0; i < tokens; ++i) rgce += tOp(0x15);
            QByteArray cells = formulaExp(0, baseCol, 0, baseCol, 0) + shrfmla(0, rows - 1, baseCol, baseCol, rgce);
            for (int r = 1; r < rows; ++r) cells += formulaExp(r, baseCol, 0, baseCol, 0);
            Book b;
            b.sheet(QStringLiteral("S"), cells);
            Workbook wb;
            QString why;
            if (!openBook(b, &wb, &why)) return qint64(-1);
            Sheet s;
            const bool ok = wb.readSheet(0, &s);
            if (whole) *whole = ok;
            return s.stats.formulaWork;
        };
        // shared: the work does not grow with the cells that share
        const qint64 few = workOf(500, 10, 2), many = workOf(500, 400, 2);
        QVERIFY2(few > 0 && many > 0, "work is counted");
        QVERIFY2(many < few + 3 * 400, qPrintable(QStringLiteral("%1 for 10 cells, %2 for 400").arg(few).arg(many)));
        QVERIFY(many < 5 * 500);
        // and grows with the length of the formula in proportion (twice the tokens, less than three times the work)
        const qint64 longer = workOf(1000, 10, 2);
        QVERIFY2(longer < 3 * few, qPrintable(QStringLiteral("%1 and %2").arg(few).arg(longer)));
        // not shared (the left of column A wraps): read once and written for each cell; the work of writing is counted for each
        const qint64 wrapped = workOf(500, 40, 0);
        QVERIFY(wrapped > 0);
        QVERIFY2(wrapped < 500 * 3 + 40 * 510 * 2, qPrintable(QString::number(wrapped)));
        QVERIFY2(wrapped > 40 * 500, qPrintable(QString::number(wrapped)));
    }

    // The work of formulas is bounded: a small file that makes a formula of thousands of pieces for each of tens of thousands of cells is stopped at the bound, with what was read
    // before it kept, and a long shared formula that need not be written for each cell is no trouble.
    void theWorkOfFormulasIsBounded()
    {
        QByteArray rgce = tRefN(0, -1);
        for (int i = 0; i < 2000; ++i) rgce += tOp(0x15);
        const int rows = 3000;
        auto book = [&](int baseCol) {
            QByteArray cells = formulaExp(0, baseCol, 0, baseCol, 0) + shrfmla(0, rows - 1, baseCol, baseCol, rgce);
            for (int r = 1; r < rows; ++r) cells += formulaExp(r, baseCol, 0, baseCol, 0);
            Book b;
            b.sheet(QStringLiteral("S"), cells);
            return b;
        };
        Limits limits;
        limits.formulaWork = 1000000;   // (6 million pieces would be written for the cells that cannot share the formula)
        Workbook wb;
        QString why;
        QVERIFY2(openBook(book(0), &wb, &why, limits), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY2(!s.error.isEmpty(), "the sheet says why");
        QVERIFY(s.stats.formulaWork <= limits.formulaWork + 4100);
        QVERIFY(s.cells.cellCount() > 0);
        // the same formula in a column it can be shared in: read once, in the bound
        QVERIFY2(openBook(book(2), &wb, &why, limits), qPrintable(why));
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY(s.error.isEmpty());
        QCOMPARE(s.cells.cellCount(), qint64(rows));
        // a formula of a thousand names in a chain of a thousand cells, none shared, each read by itself: the work is the sum of the tokens, in proportion to the bytes
        QByteArray each;
        for (int r = 0; r < 200; ++r) each += formulaNumber(r, 0, tInt(1) + tOp(0x13) + tOp(0x13) + tOp(0x13) + tOp(0x14), 0);
        Book c;
        c.sheet(QStringLiteral("S"), each);
        Workbook wc;
        QVERIFY(openBook(c, &wc, &why));
        QVERIFY(wc.readSheet(0, &s));
        QVERIFY2(s.stats.formulaWork < 200 * 30, qPrintable(QString::number(s.stats.formulaWork)));
    }

    // The read can be stopped (the progress function says no) while the cells of shared formulas are settled, after the records are read: that phase asks too.
    void theReadCanBeStoppedWhileSharedFormulasAreSettled()
    {
        // 20,000 cells that name one shared formula: the records are read in 4 asks (one for each 4,096 cells), and settling them asks again
        const int rows = 20000;
        QByteArray cells = formulaExp(0, 1, 0, 1, 0) + shrfmla(0, rows - 1, 1, 1, tRefN(0, -1) + tInt(1) + tOp(0x03));
        for (int r = 1; r < rows; ++r) cells += formulaExp(r, 1, 0, 1, 0);
        Book b;
        b.sheet(QStringLiteral("S"), cells);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        qint64 asked = 0, largest = 0;
        QVERIFY(wb.readSheet(0, &s, [&](qint64 cells) {
            ++asked;
            largest = std::max(largest, cells);
            return true;
        }));
        const qint64 asksForTheRecords = rows / 4096;
        QVERIFY2(asked > asksForTheRecords, qPrintable(QStringLiteral("asked %1 times").arg(asked)));
        // stopped at the first ask after the records
        qint64 calls = 0;
        QVERIFY(!wb.readSheet(0, &s, [&](qint64) { return ++calls <= asksForTheRecords; }));
        QVERIFY2(s.error.contains(QLatin1String("stopped")), qPrintable(s.error));
        QCOMPARE(calls, asksForTheRecords + 1);
    }

    // An array formula is at the first cell of its range (with the range); the other cells of the range hold their results as values. A data table is at its first cell
    // with the cells whose values it gave.
    void arraysAndDataTablesAreKept()
    {
        QByteArray cells = number(0, 2, 1) + number(0, 3, 2) + number(1, 2, 3) + number(1, 3, 4);
        const QByteArray times = tArea(0, 2, 1, 3) + tInt(2) + tOp(0x05);
        cells += formulaExp(2, 0, 2, 0, 2) + arrayRec(2, 3, 0, 1, times) + formulaExp(2, 1, 2, 0, 4) + formulaExp(3, 0, 2, 0, 6) + formulaExp(3, 1, 2, 0, 8);
        // a data table over B6:C7 with the row input in A1: its first cell holds the table token and the record follows it; the other cells hold the same token
        cells += formulaExp(5, 1, 5, 1, 10, 15, true) + tableRec(5, 6, 1, 2, 0, 0, 0) + formulaExp(5, 2, 5, 1, 11, 15, true) + formulaExp(6, 1, 5, 1, 12, 15, true) + formulaExp(6, 2, 5, 1, 13, 15, true);
        Book b;
        b.sheet(QStringLiteral("S"), cells);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        const Cell *a3 = sheettest::at(s, QStringLiteral("A3"));
        QVERIFY(a3 && a3->hasFormula());
        QVERIFY(a3->flags & ArrayFormula);
        QCOMPARE(s.formulaText(*a3), QStringLiteral("C1:D2*2"));
        QCOMPARE(rangeName(s.formulaRange(*a3)), QStringLiteral("A3:B4"));
        for (const QString &ref : {QStringLiteral("B3"), QStringLiteral("A4"), QStringLiteral("B4")}) {
            const Cell *c = sheettest::at(s, ref);
            QVERIFY(c && !c->hasFormula());
        }
        QCOMPARE(shown(wb, s, QStringLiteral("B4")), QStringLiteral("n:8"));
        const Cell *table = sheettest::at(s, QStringLiteral("B6"));
        QVERIFY(table && table->hasFormula());
        QVERIFY(table->flags & DataTable);
        QCOMPARE(s.formulaText(*table), QStringLiteral("TABLE(A1)"));
        QCOMPARE(rangeName(s.formulaRange(*table)), QStringLiteral("B6:C7"));
        QCOMPARE(shown(wb, s, QStringLiteral("B6")), QStringLiteral("n:10"));
        QVERIFY(!sheettest::at(s, QStringLiteral("C7"))->hasFormula());
        QCOMPARE(shown(wb, s, QStringLiteral("C7")), QStringLiteral("n:13"));
        QCOMPARE(s.formulaCells, qint64(2));
    }

    // Defined names: of the workbook and of a sheet, a built-in one (a print area), a hidden one, one that uses another, and a reference to a sheet of the workbook
    // through the list of external sheets.
    void definedNamesAreRead()
    {
        Book b;
        QByteArray after = supbookInternal(2) + externSheet({{0, 0, 0}, {0, 1, 1}, {0, 0, 1}});
        after += name(QStringLiteral("Rate"), tRef3d(0, 0, 0, false, false));                                  // 1
        after += name(QStringLiteral("Twice"), tName(1) + tInt(2) + tOp(0x05));                                // 2
        after += name(QStringLiteral("Local"), tArea3d(1, 1, 1, 3, 3, false, false), 2);                      // 3: of the second sheet
        after += name(QString(), tArea3d(0, 0, 0, 9, 4, false, false), 1, 6);                                  // 4: the print area of the first
        after += name(QStringLiteral("Quiet"), tRef3d(1, 5, 5, false, false), 0, -1, true);                   // 5
        after += name(QStringLiteral("Span"), tArea3d(2, 0, 0, 1, 1, false, false));                           // 6: through two sheets
        b.after(after);
        b.sheet(QStringLiteral("Sheet1"), QByteArray()).sheet(QStringLiteral("Sheet 2"), QByteArray());
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        const QVector<DefinedName> &names = wb.info().definedNames;
        QCOMPARE(names.size(), 6);
        QCOMPARE(names[0].name, QStringLiteral("Rate"));
        QCOMPARE(names[0].ref, QStringLiteral("Sheet1!$A$1"));
        QCOMPARE(names[0].localSheet, -1);
        QCOMPARE(names[1].ref, QStringLiteral("Rate*2"));
        QCOMPARE(names[2].name, QStringLiteral("Local"));
        QCOMPARE(names[2].ref, QStringLiteral("'Sheet 2'!$B$2:$D$4"));
        QCOMPARE(names[2].localSheet, 1);
        QCOMPARE(names[3].name, QStringLiteral("_xlnm.Print_Area"));
        QCOMPARE(names[3].ref, QStringLiteral("Sheet1!$A$1:$E$10"));
        QCOMPARE(names[3].localSheet, 0);
        QVERIFY(names[4].hidden);
        QVERIFY(!names[0].hidden);
        QCOMPARE(names[4].ref, QStringLiteral("'Sheet 2'!$F$6"));
        QCOMPARE(names[5].ref, QStringLiteral("'Sheet1:Sheet 2'!$A$1:$B$2"));
        // The old form has 256 columns: a name there may read as a cell in the new form (MU1 is a cell in column MU), and then it has an underscore in front of it, in the list of names
        // and in the formulas that use it, so that a formula that is shared by cells in a column does not move it as it moves a reference.
        Book d;
        d.after(name(QStringLiteral("MU1"), tRef(3, 7, false, false)) + name(QStringLiteral("R1C1"), tInt(1)) + name(QStringLiteral("Rate"), tInt(2)) + name(QStringLiteral("C"), tInt(3))
                + name(QStringLiteral("Row"), tInt(4)) + name(QStringLiteral("A1"), tInt(5)));
        d.sheet(QStringLiteral("S"), number(0, 0, 1) + formulaExp(0, 1, 0, 1, 0) + shrfmla(0, 1, 1, 1, tName(1) + tRefN(0, -1) + tOp(0x05)) + formulaExp(1, 1, 0, 1, 0));
        Workbook wd;
        QVERIFY2(openBook(d, &wd, &why), qPrintable(why));
        QCOMPARE(wd.info().definedNames.size(), 6);
        QCOMPARE(wd.info().definedNames[0].name, QStringLiteral("_MU1"));
        QCOMPARE(wd.info().definedNames[1].name, QStringLiteral("_R1C1"));
        QCOMPARE(wd.info().definedNames[2].name, QStringLiteral("Rate"));
        QCOMPARE(wd.info().definedNames[3].name, QStringLiteral("_C"));
        QCOMPARE(wd.info().definedNames[4].name, QStringLiteral("Row"));
        QCOMPARE(wd.info().definedNames[5].name, QStringLiteral("_A1"));
        Sheet sd;
        QVERIFY(wd.readSheet(0, &sd));
        QCOMPARE(sd.formulaText(*sheettest::at(sd, QStringLiteral("B1"))), QStringLiteral("_MU1*A1"));
        QCOMPARE(sd.formulaText(*sheettest::at(sd, QStringLiteral("B2"))), QStringLiteral("_MU1*A2"));   // (the name stays where it is: not MU2)
        // A name a formula uses is written as the name; one that belongs to another sheet with that sheet in front of it.
        Book c;
        c.after(name(QStringLiteral("Mine"), tRef(0, 0, false, false)) + name(QStringLiteral("Theirs"), tRef(1, 1, false, false), 2));
        c.sheet(QStringLiteral("One"), formulaNumber(0, 0, tName(1) + tName(2) + tOp(0x03), 0)).sheet(QStringLiteral("Two"), formulaNumber(0, 0, tName(2) + tName(1) + tOp(0x03), 0));
        Workbook wc;
        QVERIFY2(openBook(c, &wc, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wc.readSheet(0, &s));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A1"))), QStringLiteral("Mine+Two!Theirs"));
        QVERIFY(wc.readSheet(1, &s));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A1"))), QStringLiteral("Theirs+Mine"));
    }

    // The styles: fonts (the file has no font numbered 4, so the numbers above it are one higher than the fonts read), the number formats the file defines and the
    // built-in ones, alignment, borders, fills, and the palette; and the cell formats are in the file's order but for the first and the 16th, which exchange places.
    void stylesAreMapped()
    {
        Book b;
        b.noDefaultFormats();
        for (int i = 0; i < 5; ++i) b.globals(font(QStringLiteral("Arial"), 10));
        b.globals(font(QStringLiteral("Courier New"), 12.5, 700, true, true, 2, 10, 1));
        b.globals(format(164, QStringLiteral("0.000")) + format(165, QStringLiteral("#,##0 \"units\"")));
        // the 16 formats a workbook starts with: the first has format 9 and the 16th format 10, so that the exchange can be seen
        XfSpec first;
        first.style = true;
        first.format = 9;
        b.globals(xf(first));
        XfSpec styleXf;
        styleXf.style = true;
        for (int i = 1; i < 15; ++i) b.globals(xf(styleXf));
        XfSpec def;
        def.format = 10;
        b.globals(xf(def));
        XfSpec one;   // place 16: font 6 (the file's seventh), the file's own format, centered at the top, wrapped, turned, indented
        one.font = 6;
        one.format = 164;
        one.halign = 2;
        one.valign = 0;
        one.wrap = true;
        one.rotation = 90;
        one.indent = 2;
        one.shrink = true;
        one.borders[0] = 1;
        one.borders[2] = 6;
        one.borderColors[0] = 10;
        one.borderColors[2] = 12;
        one.pattern = 1;
        one.fore = 13;
        one.back = 65;
        b.globals(xf(one));
        XfSpec two;
        two.format = 14;
        two.font = 5;
        b.globals(xf(two));
        b.globals(palette({0x112233, 0xAABBCC}));
        b.sheet(QStringLiteral("S"), number(0, 0, 1, 15) + number(0, 1, 1, 0) + number(0, 2, 1, 16) + number(0, 3, 1, 17));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        const Styles &st = wb.styles();
        QCOMPARE(st.formats.size(), 18);
        QCOMPARE(int(st.numFmtId(0)), 10);    // the 16th of the file
        QCOMPARE(int(st.numFmtId(15)), 9);    // the first of the file
        QCOMPARE(int(st.numFmtId(1)), 0);
        QCOMPARE(st.formatCode(16), QStringLiteral("0.000"));
        QCOMPARE(st.formatCode(17), QStringLiteral("m/d/yyyy"));
        QCOMPARE(st.numFmts.value(165), QStringLiteral("#,##0 \"units\""));
        const FontSpec &f = st.font(st.format(16).font);
        QCOMPARE(f.name, QStringLiteral("Courier New"));
        QCOMPARE(f.size, 12.5);
        QVERIFY(f.bold && f.italic && f.strike);
        QVERIFY(f.underline == Underline::Double);
        QCOMPARE(int(f.vertAlign), 1);
        QCOMPARE(int(f.color.index), 10);
        QCOMPARE(st.font(st.format(17).font).name, QStringLiteral("Arial"));   // the file's sixth font (number 5) is the fifth that was read
        const CellFormat &cf = st.format(16);
        QVERIFY(cf.horizontal == HAlign::Center);
        QVERIFY(cf.vertical == VAlign::Top);
        QVERIFY(cf.wrap && cf.shrink);
        QCOMPARE(int(cf.rotation), 90);
        QCOMPARE(int(cf.indent), 2);
        const BorderSpec &bd = st.border(cf.border);
        QVERIFY(bd.left.style == BorderStyle::Thin);
        QVERIFY(bd.top.style == BorderStyle::Double);
        QVERIFY(bd.right.style == BorderStyle::None);
        QCOMPARE(int(bd.left.color.index), 10);
        QCOMPARE(int(bd.top.color.index), 12);
        // (the first two fills are the two every file starts with: none, and the pattern of gray 125)
        QVERIFY(st.fills.size() >= 3 && st.fills[0].pattern == PatternKind::None && st.fills[1].pattern == PatternKind::Gray125);
        const FillSpec &fill = st.fill(cf.fill);
        QVERIFY(fill.pattern == PatternKind::Solid);
        QCOMPARE(int(fill.fg.index), 13);
        QVERIFY(st.fill(st.format(17).fill).pattern == PatternKind::None);
        QVERIFY(st.border(st.format(17).border).left.style == BorderStyle::None);
        // the palette: the places from 8 are the file's, the others the usual
        ColorSpec c8;
        c8.kind = ColorSpec::Kind::Indexed;
        c8.index = 8;
        QCOMPARE(st.resolve(c8, 0), quint32(0x112233));
        c8.index = 9;
        QCOMPARE(st.resolve(c8, 0), quint32(0xAABBCC));
        c8.index = 10;
        QCOMPARE(st.resolve(c8, 0), Styles::defaultIndexed(10));
        c8.index = 2;
        QCOMPARE(st.resolve(c8, 0), quint32(0xFF0000));
        // cells carry the places as exchanged
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(int(sheettest::at(s, QStringLiteral("A1"))->style), 0);
        QCOMPARE(int(sheettest::at(s, QStringLiteral("B1"))->style), 15);
        QCOMPARE(int(sheettest::at(s, QStringLiteral("C1"))->style), 16);
    }

    // The named cell styles are kept (with the format that holds each one's look, and the style a cell format follows), and so is whether a cell is locked and whether a sheet is
    // protected: what a workbook saved as .xlsx needs to keep them.
    void namedStylesLockedCellsAndProtectedSheetsAreKept()
    {
        Book b;
        XfSpec comma;
        comma.style = true;
        comma.format = 4;
        XfSpec cell;
        cell.parent = 16;
        cell.format = 4;
        XfSpec heading;
        heading.style = true;
        heading.font = 1;
        XfSpec headed;
        headed.parent = 18;
        XfSpec open;
        open.locked = false;
        b.globals(xf(comma) + xf(cell) + xf(heading) + xf(headed) + xf(open));   // 16 to 20
        b.globals(styleBuiltin(0, 0) + styleBuiltin(16, 3) + styleNamed(18, QStringLiteral("Heading 4")));
        b.sheet(QStringLiteral("Locked"), protect(true) + number(0, 0, 1, 17)).sheet(QStringLiteral("Free"), number(0, 0, 1, 20));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        const Styles &st = wb.styles();
        QCOMPARE(st.namedStyles.size(), 3);
        QCOMPARE(st.namedStyles[0].name, QStringLiteral("Normal"));
        QCOMPARE(int(st.namedStyles[0].builtin), 0);
        QCOMPARE(int(st.namedStyles[0].format), 15);    // (the file's first cell format is the 16th place here)
        QCOMPARE(st.namedStyles[1].name, QStringLiteral("Comma"));
        QCOMPARE(int(st.namedStyles[1].builtin), 3);
        QCOMPARE(int(st.namedStyles[1].format), 16);
        QCOMPARE(st.namedStyles[2].name, QStringLiteral("Heading 4"));
        QCOMPARE(int(st.namedStyles[2].builtin), -1);
        QCOMPARE(int(st.namedStyles[2].format), 18);
        QVERIFY(st.format(16).isStyle && st.format(18).isStyle && st.format(15).isStyle);
        QVERIFY(!st.format(17).isStyle && !st.format(0).isStyle);
        QCOMPARE(st.format(17).parent, quint32(16));
        QCOMPARE(st.format(19).parent, quint32(18));
        QCOMPARE(st.format(0).parent, quint32(15));      // (the default cell format follows the file's first, which is at place 15)
        QCOMPARE(st.format(16).parent, CellFormat::kNoParent);
        QVERIFY(st.format(17).locked);
        QVERIFY(!st.format(20).locked);
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY(s.isProtected);
        QVERIFY(wb.readSheet(1, &s));
        QVERIFY(!s.isProtected);
        // BIFF5 keeps the name of a style in a byte of length
        Book five(5);
        five.globals(xf5(comma) + styleBuiltin(16, 4) + styleNamed(16, QStringLiteral("My style"), 5));
        five.sheet(QStringLiteral("S"), QByteArray());
        Workbook w5;
        QVERIFY2(openBook(five, &w5, &why), qPrintable(why));
        QCOMPARE(w5.styles().namedStyles.size(), 2);
        QCOMPARE(w5.styles().namedStyles[0].name, QStringLiteral("Currency"));
        QCOMPARE(w5.styles().namedStyles[1].name, QStringLiteral("My style"));
        QCOMPARE(int(w5.styles().namedStyles[1].builtin), -1);
    }

    // What the file says about the shape of a sheet: widths and hidden columns, row heights and hidden rows, the defaults, merged ranges, and how the sheet was left on
    // the screen (frozen panes, zoom, the cell it was left on).
    void theLayoutOfASheetIsRead()
    {
        QByteArray cells = dimensions(10, 5) + defColWidth(10) + defaultRowHeight(0, 255);
        cells += colinfo(1, 2, 3072, 0, 0x02) + colinfo(4, 4, 2560, 16, 0x01);
        cells += rowRec(3, 400, 0x40) + rowRec(5, 255, 0x20) + rowRec(6, 255, 0x80, 17) + rowRec(8, 255, 0) + rowRec(9, 8192, 0x40);
        cells += number(0, 0, 1);
        cells += mergedCells({{0, 0, 0, 2}, {3, 5, 1, 1}, {8, 8, 2, 2}});   // (the last is one cell, which merges nothing)
        cells += window2(0x02 | 0x04 | 0x08 | 0x10 | 0x200, 2, 1) + pane(2, 3, 5, 4, 0) + scl(3, 2) + selection(3, 1, 1) + selection(0, 7, 8);
        Book b;
        b.sheet(QStringLiteral("S"), cells);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY(s.error.isEmpty());
        QCOMPARE(s.dimension.valid, true);
        QCOMPARE(rangeName(s.dimension), QStringLiteral("A1:E10"));
        QCOMPARE(s.format.baseColWidth, 10);
        QCOMPARE(s.format.defaultRowHeight, 12.75);
        QCOMPARE(s.columns.size(), 2);
        QCOMPARE(s.columns[0].first, quint32(1));
        QCOMPARE(s.columns[0].last, quint32(2));
        QCOMPARE(s.columns[0].width, 12.0);
        QVERIFY(s.columns[0].customWidth);
        QVERIFY(!s.columns[0].hidden);
        QVERIFY(s.columns[1].hidden);
        QCOMPARE(s.columns[1].style, quint32(16));
        auto row = [&](int r) -> const RowSpec * {
            for (const RowSpec &spec : s.rowSpecs)
                if (spec.row == quint32(r)) return &spec;
            return nullptr;
        };
        QVERIFY(row(3) && row(3)->height == 20.0 && row(3)->customHeight && !row(3)->hidden);
        QVERIFY(row(5) && row(5)->hidden);
        QVERIFY(row(6) && row(6)->customFormat && row(6)->style == 17);
        QVERIFY(!row(8));   // a row as the default is no row of its own
        QVERIFY(row(9) && row(9)->height == 409.5);   // (no row is taller than the XML form allows, 409.5 points; the old form could say a little more)
        QCOMPARE(s.mergeCount, qint64(3));
        QCOMPARE(s.merges.size(), 2);
        QCOMPARE(rangeName(s.merges[0]), QStringLiteral("A1:C1"));
        QCOMPARE(rangeName(s.merges[1]), QStringLiteral("B4:B6"));
        QVERIFY(s.view.frozen);
        QCOMPARE(s.view.frozenCols, quint32(2));
        QCOMPARE(s.view.frozenRows, quint32(3));
        QCOMPARE(s.view.paneRow, quint32(5));
        QCOMPARE(s.view.paneCol, quint32(4));
        QCOMPARE(s.view.topRow, quint32(2));
        QCOMPARE(s.view.leftCol, quint32(1));
        QCOMPARE(s.view.zoom, 150);
        QVERIFY(s.view.showGridLines && s.view.showHeadings && s.view.showZeros && s.view.tabSelected);
        QVERIFY(!s.view.rightToLeft);
        QCOMPARE(s.view.activeRow, quint32(7));    // (the selection of the pane the sheet was left in)
        QCOMPARE(s.view.activeCol, quint32(8));
        // a split that is not frozen is not kept; no grid lines, no zero, from right to left
        Book c;
        c.sheet(QStringLiteral("S"), window2(0x04 | 0x40) + pane(100, 200, 0, 0, 3));
        Workbook wc;
        QVERIFY(openBook(c, &wc));
        QVERIFY(wc.readSheet(0, &s));
        QVERIFY(!s.view.frozen);
        QVERIFY(!s.view.showGridLines && !s.view.showZeros && s.view.rightToLeft);
        QCOMPARE(s.view.zoom, 100);
    }

    // BIFF5: text in the workbook's code page, 14-bit rows, the format records of its own, and sheets listed with their type in one word.
    void theFormOfBiff5IsRead()
    {
        Book b(5);
        b.codepage(1251);
        b.globals(format(164, QStringLiteral("0.0\"x\""), 5));
        QByteArray cells = dimensions(3, 3, 5);
        cells += rec(0x0204, W().u16(0).u16(0).u16(15).u16(6).u8(0xCF).u8(0xF0).u8(0xE8).u8(0xE2).u8(0xE5).u8(0xF2));   // a word of Cyrillic in code page 1251
        cells += label(1, 0, QStringLiteral("plain"), 15, 5);
        cells += formulaNumber(2, 0, tRef(16383, 3, true, true, 5) + tInt(1) + tOp(0x03), 7);
        cells += number(2, 1, 4.5, 16);
        b.sheet(QStringLiteral("Book1"), cells);
        b.sheet(QStringLiteral("Chart1"), QByteArray(), 0, 2);
        XfSpec fmt;
        fmt.format = 164;
        fmt.pattern = 1;
        fmt.fore = 22;
        fmt.borders[0] = 2;    // left medium
        fmt.borders[1] = 5;    // right thick
        fmt.borders[2] = 1;    // top thin
        fmt.borders[3] = 6;    // bottom double
        fmt.borderColors[0] = 10;
        fmt.borderColors[1] = 11;
        fmt.borderColors[2] = 12;
        fmt.borderColors[3] = 13;
        b.globals(xf5(fmt));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        QCOMPARE(wb.info().sheets.size(), 2);
        QVERIFY(wb.info().sheets[1].kind == SheetKind::Chartsheet);
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QString(QStringLiteral("i:")) + QString::fromUtf16(u"Привет"));
        QCOMPARE(shown(wb, s, QStringLiteral("A2")), QStringLiteral("i:plain"));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A3"))), QStringLiteral("D16384+1"));
        QCOMPARE(wb.styles().formatCode(16), QStringLiteral("0.0\"x\""));
        QCOMPARE(rangeName(s.dimension), QStringLiteral("A1:C3"));
        // the fill and the borders, where the form of BIFF5 keeps them (laid out from files written by the programs of the time)
        const CellFormat &cf = wb.styles().format(16);
        QVERIFY(wb.styles().fill(cf.fill).pattern == PatternKind::Solid);
        QCOMPARE(int(wb.styles().fill(cf.fill).fg.index), 22);
        const BorderSpec &bd = wb.styles().border(cf.border);
        QVERIFY(bd.left.style == BorderStyle::Medium && bd.right.style == BorderStyle::Thick && bd.top.style == BorderStyle::Thin && bd.bottom.style == BorderStyle::Double);
        QCOMPARE(int(bd.left.color.index), 10);
        QCOMPARE(int(bd.right.color.index), 11);
        QCOMPARE(int(bd.top.color.index), 12);
        QCOMPARE(int(bd.bottom.color.index), 13);
    }

    // BIFF3 and BIFF4 keep a workbook of one sheet as one stream with no container: the start of the stream is the start of the sheet, the formats are numbered by their order,
    // the fonts and cell formats have forms of their own, and a formula's header has no chain.
    void theFormsOfBiff3AndBiff4AreRead()
    {
        for (int version : {3, 4}) {
            const quint16 bofId = version == 3 ? 0x0209 : 0x0409;
            QByteArray stream = rec(bofId, W().u16(0).u16(0x10).u16(0x1234).u16(0x07C9));
            stream += rec(0x0042, W().u16(32769));
            // a font: the height, the flags, the color, and the name
            stream += rec(0x0231, W().u16(240).u16(0x01).u16(0x7FFF).str5(QStringLiteral("Courier"), 1));
            // the formats come in order from 0: General, 0, then the one the cell uses (BIFF4 has two words in front of the text)
            auto fmt = [&](const QString &code) { return version == 3 ? rec(0x001E, W().str5(code, 1)) : rec(0x041E, W().u16(0).str5(code, 1)); };
            stream += fmt(QStringLiteral("General")) + fmt(QStringLiteral("0")) + fmt(QStringLiteral("0.00 \\x"));
            // the cell formats: 12 bytes (the font, the format, the type, the alignment, the fill with colors of 5 bits, and the borders); the 16th is the default
            auto cellFormat = [&](int font, int format, bool style, int align, quint32 borders) {
                return rec(version == 3 ? 0x0243 : 0x0443, W().u8(font).u8(format).u16(style ? 0xFFF5 : 0x0001).u8(align).u8(0).u16(0x0040 | (7 << 6)).u32(borders));
            };
            for (int i = 0; i < 15; ++i) stream += cellFormat(0, 0, true, 0, 0);
            stream += cellFormat(0, 0, false, 0, 0);                                             // 15: the default
            stream += cellFormat(0, 2, false, 0x2A, 1 | (2 << 8) | (5u << 16) | (6u << 24));    // 16: the format 2, centered (2), wrapped, and the sides top thin, left medium, bottom thick, right double
            stream += rec(0x0200, W().u16(0).u16(4).u16(0).u16(3).u16(0));
            stream += rec(0x0204, W().u16(0).u16(0).u16(15).u16(5).raw("Hello"));
            stream += rec(0x0203, W().u16(0).u16(1).u16(16).f64(3.25));
            stream += rec(0x027E, W().u16(1).u16(0).u16(15).u32((12u << 2) | 2));
            stream += rec(0x0201, W().u16(1).u16(1).u16(16));
            // =A2*2 with the stored result 99: 18 bytes and then the tokens
            const QByteArray tokens = tRef(1, 0, true, true, 5) + tInt(2) + tOp(0x05);
            stream += rec(version == 3 ? 0x0206 : 0x0406, W().u16(2).u16(0).u16(15).f64(99).u16(0).u16(int(tokens.size())).raw(tokens));
            stream += rec(0x0208, W().u16(0).u16(0).u16(3).u16(300).u16(0).u16(0x100).u16(0x0F00 >> 8 << 8).u16(15));
            stream += rec(0x000A);
            Workbook wb;
            QString why;
            QVERIFY2(wb.openBinary(stream, &why), qPrintable(why));
            QVERIFY(wb.isBinary());
            QCOMPARE(wb.info().sheets.size(), 1);
            Sheet s;
            QVERIFY(wb.readSheet(0, &s));
            QVERIFY2(s.error.isEmpty(), qPrintable(s.error));
            QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("i:Hello"));
            QCOMPARE(shown(wb, s, QStringLiteral("B1")), QStringLiteral("n:3.25"));
            QCOMPARE(shown(wb, s, QStringLiteral("A2")), QStringLiteral("n:12"));
            QCOMPARE(shown(wb, s, QStringLiteral("A3")), QStringLiteral("n:99"));
            QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A3"))), QStringLiteral("A2*2"));
            QCOMPARE(int(sheettest::at(s, QStringLiteral("B1"))->style), 16);
            QCOMPARE(int(sheettest::at(s, QStringLiteral("A1"))->style), 0);
            QCOMPARE(wb.styles().formatCode(16), QStringLiteral("0.00 \\x"));
            const CellFormat &cf = wb.styles().format(16);
            QVERIFY(cf.horizontal == HAlign::Center);
            QVERIFY(cf.wrap);
            QVERIFY(cf.vertical == VAlign::Bottom);
            const BorderSpec &bd = wb.styles().border(cf.border);
            QVERIFY(bd.top.style == BorderStyle::Thin && bd.left.style == BorderStyle::Medium && bd.bottom.style == BorderStyle::Thick && bd.right.style == BorderStyle::Double);
            QCOMPARE(wb.styles().font(0).name, QStringLiteral("Courier"));
            QVERIFY(wb.styles().font(0).bold);
            QCOMPARE(wb.styles().font(0).size, 12.0);
            QCOMPARE(rangeName(s.dimension), QStringLiteral("A1:C4"));
        }
        // a stream of the form before BIFF3 is refused, and so is one that is not a worksheet
        Workbook wb;
        QString why;
        QVERIFY(!wb.openBinary(rec(0x0009, W().u16(0).u16(0x10)) + rec(0x000A), &why));
        QVERIFY(!wb.openBinary(rec(0x0409, W().u16(0).u16(0x20)) + rec(0x000A), &why));
    }

    // Text in a BIFF5 formula is in the workbook's code page, and the formula is turned into text after the workbook has been opened and the stack it was opened on is long gone:
    // the way to the code page must not be a reference to anything the opening held.
    void biff5TextInAFormulaIsInTheCodePageOfTheWorkbook()
    {
        Book b(5);
        b.codepage(1251);
        // ="Привет", the bytes in code page 1251
        const QByteArray tokens = W().u8(0x17).u8(6).raw(QByteArray("\xCF\xF0\xE8\xE2\xE5\xF2", 6)).bytes();
        b.sheet(QStringLiteral("S"), formulaNumber(0, 0, tokens, 0));
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        scrubTheStack();
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A1"))), QString(QStringLiteral("\"")) + QString::fromUtf16(u"\u041F\u0440\u0438\u0432\u0435\u0442") + QStringLiteral("\""));
    }

    // The lists of external sheets, supporting books, and external names are records that go on in the records after them when they are long, as the string table does.
    void listsOfExternalSheetsMayContinue()
    {
        // 2,000 entries of the list of sheets that formulas name, the first 8,222 bytes in the record and the rest in a CONTINUE record; entry 1,500 is the one that names the second sheet
        W data;
        data.u16(2000);
        for (int i = 0; i < 2000; ++i) data.u16(0).u16(i == 1500 ? 1 : 0).u16(i == 1500 ? 1 : 0);
        const QByteArray all = data.bytes();
        const QByteArray list = rec(0x0017, all.left(8220)) + rec(0x003C, all.mid(8220));
        QByteArray cells = formulaNumber(0, 0, tRef3d(1500, 0, 0), 0) + formulaNumber(1, 0, tRef3d(1999, 0, 0), 0) + formulaNumber(2, 0, tRef3d(0, 0, 0), 0);
        Book b;
        b.after(supbookInternal(2) + list);
        b.sheet(QStringLiteral("One"), cells).sheet(QStringLiteral("Two"), QByteArray());
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A1"))), QStringLiteral("Two!A1"));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A2"))), QStringLiteral("One!A1"));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A3"))), QStringLiteral("One!A1"));
        // a supporting book whose sheet names (and a name of it) go on in a record after
        QStringList names;
        for (int i = 0; i < 400; ++i) names << QStringLiteral("Sheet number %1").arg(i);
        W book;
        book.u16(int(names.size())).u16(9).u8(0).raw("other.xls");
        for (const QString &n : names) book.str8(n, 2);
        const QByteArray bookBytes = book.bytes();
        const QByteArray supbook = rec(0x01AE, bookBytes.left(3000)) + rec(0x003C, bookBytes.mid(3000));
        W twoLists;
        twoLists.u16(2).u16(0).u16(0).u16(0).u16(1).u16(399).u16(399);
        Book c;
        c.after(supbookInternal(1) + supbook + rec(0x0017, twoLists.bytes()));
        c.sheet(QStringLiteral("S"), formulaNumber(0, 0, tRef3d(1, 4, 4), 0));
        Workbook wc;
        QVERIFY2(openBook(c, &wc, &why), qPrintable(why));
        QVERIFY(wc.readSheet(0, &s));
        QCOMPARE(s.formulaText(*sheettest::at(s, QStringLiteral("A1"))), QStringLiteral("'[1]Sheet number 399'!E5"));
    }

    // A file larger than the limit is refused with the plain words other formats use, before any of it is read: from a path, and from bytes.
    void aFileLargerThanTheLimitIsRefusedBeforeItIsRead()
    {
        const QByteArray file = Book().sheet(QStringLiteral("S"), number(0, 0, 1)).file();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("big.xls"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(file);
        }
        Limits limits;
        limits.fileBytes = file.size();
        Workbook wb;
        QString why;
        QVERIFY2(wb.openFile(path, &why, limits), qPrintable(why));
        QVERIFY2(wb.openBinary(file, &why, limits), qPrintable(why));
        limits.fileBytes = file.size() - 1;
        QVERIFY(!wb.openFile(path, &why, limits));
        QVERIFY2(why.contains(QLatin1String("larger than")), qPrintable(why));
        QVERIFY(!wb.openBinary(file, &why, limits));
        QVERIFY2(why.contains(QLatin1String("larger than")), qPrintable(why));
        // a bare stream too, and a file of nothing but zeros that is longer than the limit (it is not read to find that out)
        const QByteArray stream = Book().sheet(QStringLiteral("S"), number(0, 0, 1)).stream();
        limits.fileBytes = stream.size() - 1;
        QVERIFY(!wb.openBinary(stream, &why, limits));
        QVERIFY2(why.contains(QLatin1String("larger than")), qPrintable(why));
        const QString zeros = dir.filePath(QStringLiteral("zeros.xls"));
        {
            QFile f(zeros);
            QVERIFY(f.open(QIODevice::WriteOnly));
            QVERIFY(f.resize(limits.fileBytes + 10));
        }
        QVERIFY(!wb.openFile(zeros, &why, limits));
        QVERIFY2(why.contains(QLatin1String("larger than")), qPrintable(why));
        // and the limit is what the apps open for every kind
        QCOMPARE(Limits().fileBytes, kMaxOpenFileBytes);
    }

    // A macro project in the file is told about the same way whether the file is opened from a path or from bytes, for each place the project may be kept.
    void macrosAreFoundWhicheverWayTheFileIsOpened()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QList<QStringList> kinds = {{}, {QStringLiteral("_VBA_PROJECT_CUR/VBA/dir")}, {QStringLiteral("_VBA_PROJECT_CUR/VBA/_VBA_PROJECT")}, {QStringLiteral("VBA/dir")},
                                          {QStringLiteral("VBA/_VBA_PROJECT")}, {QStringLiteral("Pictures/other")}};
        const QList<bool> expected = {false, true, true, true, true, false};
        for (int i = 0; i < kinds.size(); ++i) {
            Book b;
            b.sheet(QStringLiteral("S"), number(0, 0, 1));
            const QByteArray file = b.file(kinds[i]);
            const QString path = dir.filePath(QStringLiteral("m%1.xls").arg(i));
            {
                QFile f(path);
                QVERIFY(f.open(QIODevice::WriteOnly));
                f.write(file);
            }
            Workbook fromPath, fromBytes;
            QString why;
            QVERIFY2(fromPath.openFile(path, &why), qPrintable(why));
            QVERIFY2(fromBytes.openBinary(file, &why), qPrintable(why));
            QCOMPARE(fromPath.skippedFeatures().contains(QStringLiteral("macros")), expected[i]);
            QCOMPARE(fromBytes.skippedFeatures().contains(QStringLiteral("macros")), expected[i]);
        }
    }

    // A web page or an XML spreadsheet that has an .xls name is said to be what it is, in plain words, and that this program cannot open that kind; other text gets the general
    // refusal.
    void aWebPageOrAnXmlSpreadsheetNamedXlsSaysWhatItIs()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto open = [&](const QByteArray &bytes) {
            const QString path = dir.filePath(QStringLiteral("f.xls"));
            QFile f(path);
            f.open(QIODevice::WriteOnly | QIODevice::Truncate);
            f.write(bytes);
            f.close();
            Workbook wb;
            QString why;
            return wb.openFile(path, &why) ? QStringLiteral("opened") : why;
        };
        const QString page = open("<html><head><title>x</title></head><body><table><tr><td>1</td></tr></table></body></html>");
        QVERIFY2(page.contains(QLatin1String("web page")) && page.contains(QLatin1String("can't open")), qPrintable(page));
        QCOMPARE(open("\n  <!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"><HTML><BODY></BODY></HTML>"), page);
        QCOMPARE(open("\xEF\xBB\xBF<?xml version=\"1.0\"?><HTML xmlns=\"http://www.w3.org/TR/REC-html40\"><BODY/></HTML>"), page);
        const QString xml = open("<?xml version=\"1.0\"?>\n<?mso-application progid=\"Excel.Sheet\"?>\n<Workbook xmlns=\"urn:schemas-x:office:spreadsheet\"><Worksheet/></Workbook>");
        QVERIFY2(xml.contains(QLatin1String("XML spreadsheet")) && xml.contains(QLatin1String("can't open")), qPrintable(xml));
        QVERIFY(xml != page);
        for (const QString &word : {QStringLiteral("Excel"), QStringLiteral("Microsoft")}) QVERIFY(!xml.contains(word) && !page.contains(word));
        const QString other = open("AMTMIS 10 11 12");
        QVERIFY(!other.contains(QLatin1String("web page")) && !other.contains(QLatin1String("XML spreadsheet")));
        QVERIFY(other != QLatin1String("opened"));
    }

    // The double-byte code pages are read by the system's own converter (Windows', CoreFoundation's, or iconv), so that no platform depends on how Qt was built; Qt's, where Qt has
    // one, says the same. A sample of each is decoded here, on every platform the tests run on.
    void doubleBytePagesAreTheSystemsToConvert()
    {
        struct Sample {
            int page;
            QByteArray bytes;
            QString text;
        };
        const QList<Sample> samples = {
            {932, QByteArray::fromHex("93fa967b8ceab1"), QString::fromUtf16(u"\u65E5\u672C\u8A9E\uFF71")},
            {936, QByteArray::fromHex("d6d0cec4"), QString::fromUtf16(u"\u4E2D\u6587")},
            {949, QByteArray::fromHex("c7d1b1b9beee"), QString::fromUtf16(u"\uD55C\uAD6D\uC5B4")},
            {950, QByteArray::fromHex("a4a4a4e5"), QString::fromUtf16(u"\u4E2D\u6587")},
            {1361, QByteArray::fromHex("d0658a82b4e1"), QString::fromUtf16(u"\uD55C\uAD6D\uC5B4")},
        };
        for (const Sample &sample : samples) {
            QVERIFY2(xlsCanConvert(sample.page, CodePageBackend::System), qPrintable(QStringLiteral("the system converts page %1").arg(sample.page)));
            QVERIFY(xlsCanConvert(sample.page, CodePageBackend::Automatic));
            QCOMPARE(xlsBytesToText(sample.page, sample.bytes.constData(), sample.bytes.size(), CodePageBackend::System), sample.text);
            QCOMPARE(xlsBytesToText(sample.page, sample.bytes.constData(), sample.bytes.size()), sample.text);
            if (xlsCanConvert(sample.page, CodePageBackend::Qt)) QCOMPARE(xlsBytesToText(sample.page, sample.bytes.constData(), sample.bytes.size(), CodePageBackend::Qt), sample.text);
            // a character cut in two at the end does not take the rest with it, and a single-byte text is still ASCII
            const QString cut = xlsBytesToText(sample.page, sample.bytes.constData(), sample.bytes.size() - 1);
            QVERIFY(cut.size() >= 1);
            QCOMPARE(xlsBytesToText(sample.page, "abc", 3), QStringLiteral("abc"));
        }
        // a page that is not one of them is a table of the program's own
        QCOMPARE(xlsBytesToText(1251, "\xCF\xF0", 2), QString::fromUtf16(u"\u041F\u0440"));
        QVERIFY(!xlsCanConvert(1252, CodePageBackend::System));
    }

    // What cannot be opened is refused with a message: a workbook that asks for a password, one of the forms before BIFF3, a compound file that holds no workbook, and a file that
    // is not a workbook at all.
    void whatCannotBeOpenedIsRefusedInWords()
    {
        QString why;
        Workbook wb;
        Book locked;
        locked.globals(rec(0x002F, W().u16(1).u16(1).u16(0x1234).u16(0x5678)));
        locked.sheet(QStringLiteral("S"), number(0, 0, 1));
        QVERIFY(!openBook(locked, &wb, &why));
        QVERIFY2(why.contains(QLatin1String("encrypted")), qPrintable(why));
        QVERIFY(!wb.isBinary());
        // a bare stream that begins as the form before BIFF3 did
        const QByteArray old = rec(0x0009, W().u16(0x0200).u16(0x10)) + rec(0x0003, W().u16(0).u16(0).raw(QByteArray(3, '\0')).u16(1)) + rec(0x000A);
        QVERIFY(!wb.openBinary(old, &why));
        QVERIFY2(why.contains(QLatin1String("older")), qPrintable(why));
        // a compound file with other streams in it
        jp::cfb::File f;
        jp::cfb::Entry root;
        root.type = jp::cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        f.entries << root;
        jp::cfb::Entry other;
        other.name = QStringLiteral("WordDocument");
        other.data = QByteArray(5000, 'w');
        f.entries << other;
        f.entries[0].children << 1;
        QVERIFY(!wb.openBinary(jp::cfb::write(f), &why));
        QVERIFY2(!why.isEmpty(), "a message");
        // not a workbook at all
        QVERIFY(!wb.openBinary(QByteArray("<html><body>no</body></html>"), &why));
        QVERIFY(!why.isEmpty());
        QVERIFY(!wb.openBinary(QByteArray(), &why));
        // a workbook without sheets
        Book none;
        QVERIFY(!openBook(none, &wb, &why));
        QVERIFY(!why.isEmpty());
    }

    // What is not read is named, and does not get in the way: drawing objects, a chart inside a sheet (its own records, bracketed as a sheet is), comments, pivot tables,
    // and macros; the cells that come after them are read.
    void whatIsSkippedIsSaid()
    {
        const QByteArray bof = rec(0x0809, W().u16(0x0600).u16(0x0020).u16(1).u16(2).u32(0).u32(6));
        QByteArray cells = number(0, 0, 1) + rec(0x005D, QByteArray(20, '\0')) + rec(0x00EC, QByteArray(30, '\0'));
        // the chart: its records begin with a start of substream and end with its end, and may hold the records a sheet holds, which are not cells of this one
        cells += bof + number(5, 5, 99) + rec(0x0809, W().u16(0x0600).u16(0x0020).u16(1).u16(2).u32(0).u32(6)) + rec(0x000A) + rec(0x000A);
        cells += rec(0x001C, QByteArray(20, '\0')) + rec(0x00B0, QByteArray(20, '\0')) + number(1, 0, 2);
        Book b;
        b.sheet(QStringLiteral("S"), cells);
        Workbook wb;
        QString why;
        QVERIFY2(wb.openBinary(b.file({QStringLiteral("_VBA_PROJECT_CUR/VBA/dir")}), &why), qPrintable(why));
        const QStringList skipped = wb.skippedFeatures();
        QVERIFY(skipped.contains(QStringLiteral("charts")));
        QVERIFY(skipped.contains(QStringLiteral("pictures and other drawing objects")));
        QVERIFY(skipped.contains(QStringLiteral("comments")));
        QVERIFY(skipped.contains(QStringLiteral("pivot tables")));
        QVERIFY(skipped.contains(QStringLiteral("macros")));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY(s.error.isEmpty());
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(shown(wb, s, QStringLiteral("A2")), QStringLiteral("n:2"));
        QCOMPARE(shown(wb, s, QStringLiteral("F6")), QStringLiteral("none"));   // (the cell inside the chart is the chart's)
        // a workbook with none of them says nothing
        Book plain;
        plain.sheet(QStringLiteral("S"), number(0, 0, 1));
        Workbook pw;
        QVERIFY(openBook(plain, &pw));
        QVERIFY(pw.skippedFeatures().isEmpty());
    }

    // The calculation works an old workbook's formulas out as it does a new one's: from the constants, shared formulas and 3-D references included.
    void theCalculationWorksOldFormulasOut()
    {
        Book b;
        b.after(supbookInternal(2) + externSheet({{0, 1, 1}}));
        QByteArray one = number(0, 0, 2) + number(1, 0, 3);
        one += formulaNumber(2, 0, tArea(0, 0, 1, 0) + tFuncVar(4, 1, 0x40), 99);                                  // =SUM(A1:A2), stored wrong
        const QByteArray plusOne = tRefN(-1, 0) + tInt(1) + tOp(0x03);
        one += formulaExp(3, 0, 3, 0, 0) + shrfmla(3, 5, 0, 0, plusOne) + formulaExp(4, 0, 3, 0, 0) + formulaExp(5, 0, 3, 0, 0);   // =A3+1 and down
        one += formulaNumber(0, 2, tRef3d(0, 0, 0) + tInt(10) + tOp(0x05), 0);                                      // =Two!A1*10
        QByteArray two = number(0, 0, 4);
        b.sheet(QStringLiteral("One"), one).sheet(QStringLiteral("Two"), two);
        Workbook wb;
        QString why;
        QVERIFY2(openBook(b, &wb, &why), qPrintable(why));
        jp::office::sheet::Calculation calc(wb);
        QVERIFY(calc.load());
        calc.recalculate();
        QCOMPARE(calc.cell(0, 2, 0).debug(), QStringLiteral("n:5"));
        QCOMPARE(calc.cell(0, 3, 0).debug(), QStringLiteral("n:6"));
        QCOMPARE(calc.cell(0, 4, 0).debug(), QStringLiteral("n:7"));
        QCOMPARE(calc.cell(0, 5, 0).debug(), QStringLiteral("n:8"));
        QCOMPARE(calc.cell(0, 0, 2).debug(), QStringLiteral("n:40"));
        // an edit reaches the cells that read it
        QVERIFY(calc.setValue(0, 0, 0, jp::calc::Value::number(10)));
        calc.recalculate();
        QCOMPARE(calc.cell(0, 2, 0).debug(), QStringLiteral("n:13"));
        QCOMPARE(calc.cell(0, 5, 0).debug(), QStringLiteral("n:16"));
    }

    // A file is opened by what is in it, not by its name: a workbook of either kind opens from a path, and a file that is neither says why not.
    void aFileOpensByItsContents()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        Book b;
        b.sheet(QStringLiteral("Old"), number(0, 0, 8));
        const QString oldPath = dir.filePath(QStringLiteral("book.dat"));
        {
            QFile f(oldPath);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(b.file());
        }
        Workbook old;
        QString why;
        QVERIFY2(old.openFile(oldPath, &why), qPrintable(why));
        QVERIFY(old.isBinary());
        QCOMPARE(old.info().sheets[0].name, QStringLiteral("Old"));
        // an XML workbook still opens from a path
        const QString newPath = dir.filePath(QStringLiteral("book.xls"));   // (named as the old kind, and is the new)
        {
            QFile f(newPath);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))}));
        }
        Workbook fresh;
        QVERIFY2(fresh.openFile(newPath, &why), qPrintable(why));
        QVERIFY(!fresh.isBinary());
        Sheet s;
        QVERIFY(fresh.readSheet(0, &s));
        QCOMPARE(show(s, fresh.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:5"));
        // neither
        const QString junk = dir.filePath(QStringLiteral("junk.xls"));
        {
            QFile f(junk);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("this is text, not a workbook");
        }
        Workbook none;
        QVERIFY(!none.openFile(junk, &why));
        QVERIFY(!why.isEmpty());
        QVERIFY(!none.openFile(dir.filePath(QStringLiteral("missing.xls")), &why));
        QVERIFY(!why.isEmpty());
    }
};

JP_TEST_CLASS(SheetXlsTests)
#include "tst_sheet_xls.moc"
