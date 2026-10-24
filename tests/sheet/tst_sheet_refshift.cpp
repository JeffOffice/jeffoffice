// Tests of what happens to references when cells move (sheet/refshift.h): rows and columns inserted and deleted, cells shifted, a block cut and pasted, and a formula copied. Every
// expected text is written from the rules a person knows (a reference follows its cells, a range grows when a row goes in it and loses the rows deleted from it, a reference to deleted
// cells is #REF!), never from what the code printed.
#include "sheethelpers.h"

#include "sheet/refshift.h"

#include "calc/formula.h"

#include <QRandomGenerator>

namespace {

const QStringList kNames{QStringLiteral("Sheet1"), QStringLiteral("Sheet2"), QStringLiteral("My Sheet")};

struct Case {
    const char *before;
    const char *after;
};

QString adjusted(const char *formula, int own, const StructEdit &e, bool *changed = nullptr)
{
    return adjustFormula(QString::fromUtf8(formula), own, kNames, e, changed);
}

#define CHECK_FORMULAS(cases, own, edit)                                                                                                       \
    for (const Case &c : cases) {                                                                                                              \
        bool changed = false;                                                                                                                  \
        const QString got = adjusted(c.before, own, edit, &changed);                                                                           \
        QVERIFY2(got == QString::fromUtf8(c.after), qPrintable(QString::fromUtf8(c.before) + QStringLiteral(" became ") + got + QStringLiteral(", expected ") + QString::fromUtf8(c.after))); \
        QCOMPARE(changed, got != QString::fromUtf8(c.before));                                                                                 \
    }

} // namespace

class SheetRefShiftTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A row inserted at row 5: what is at or below it moves down, a range with the row inside it grows, a range above it and whole columns stay, a reference to another sheet stays.
    void insertingRowsMovesAndGrowsReferences()
    {
        static const Case cases[] = {
            {"A1+A5", "A1+A6"},
            {"A4+A6", "A4+A7"},
            {"SUM(A1:A10)", "SUM(A1:A11)"},
            {"SUM(A5:A10)", "SUM(A6:A11)"},
            {"SUM(A1:A4)", "SUM(A1:A4)"},
            {"SUM(A4:A5)", "SUM(A4:A6)"},
            {"$A$5+A$5+$A5", "$A$6+A$6+$A6"},
            {"B2:D4", "B2:D4"},
            {"Sheet2!A5+Sheet1!A5", "Sheet2!A5+Sheet1!A6"},
            {"'My Sheet'!B7", "'My Sheet'!B7"},
            {"SUM(A:A)+SUM(A:C)", "SUM(A:A)+SUM(A:C)"},
            {"SUM(5:6)+SUM(3:5)+SUM(1:3)", "SUM(6:7)+SUM(3:6)+SUM(1:3)"},
            {"\"A5\"&A5&LOG10(A5)", "\"A5\"&A6&LOG10(A6)"},
            {"SUM(Sheet1:Sheet2!A5)", "SUM(Sheet1:Sheet2!A6)"},
            {"IF(A5>1,A6,B9)", "IF(A6>1,A7,B10)"},
            {"A5:A5", "A6:A6"},
            {"1+2", "1+2"},
            {"#REF!+A5", "#REF!+A6"},
        };
        CHECK_FORMULAS(cases, 0, StructEdit::insertRows(0, 4, 1));
        // two rows
        static const Case two[] = {{"A5", "A7"}, {"A1:A5", "A1:A7"}, {"A4:A4", "A4:A4"}};
        CHECK_FORMULAS(two, 0, StructEdit::insertRows(0, 4, 2));
        // a formula on another sheet that names the sheet
        static const Case other[] = {{"Sheet1!A5+A5", "Sheet1!A6+A5"}, {"Sheet2!A5", "Sheet2!A5"}};
        CHECK_FORMULAS(other, 1, StructEdit::insertRows(0, 4, 1));
        // the sheet's own name in quotes, and a name with a quote in it
        const QStringList quoted{QStringLiteral("It's"), QStringLiteral("B")};
        QCOMPARE(adjustFormula(QStringLiteral("'It''s'!A5"), 1, quoted, StructEdit::insertRows(0, 4, 1)), QStringLiteral("'It''s'!A6"));
        // rows pushed off the edge
        QCOMPARE(adjusted("A1048576", 0, StructEdit::insertRows(0, 0, 1)), QStringLiteral("#REF!"));
        QCOMPARE(adjusted("A1:A1048576", 0, StructEdit::insertRows(0, 4, 3)), QStringLiteral("A1:A1048576"));
    }

    // Rows deleted: the references below move up, a range loses the rows in it, and a reference to deleted cells is #REF! (with its sheet's name before it).
    void deletingRowsClosesUpAndMakesRef()
    {
        static const Case cases[] = {
            {"A3", "#REF!"},
            {"Sheet1!A4", "Sheet1!#REF!"},
            {"A2+A5", "A2+A3"},
            {"A1:A10", "A1:A8"},
            {"A2:A3", "A2"},
            {"A3:A4", "#REF!"},
            {"A3:A6", "A3:A4"},
            {"A4:A6", "A3:A4"},
            {"A1:A2", "A1:A2"},
            {"SUM(A3:A4,B1,B4)", "SUM(#REF!,B1,#REF!)"},
            {"SUM(A:A)", "SUM(A:A)"},
            {"SUM(3:4)+SUM(2:6)", "SUM(#REF!)+SUM(2:4)"},
            {"Sheet2!A4", "Sheet2!A4"},
            {"$A$5", "$A$3"},
            {"B3:C3", "#REF!"},
        };
        CHECK_FORMULAS(cases, 0, StructEdit::deleteRows(0, 2, 2));
        // the other kind: columns
        static const Case columns[] = {
            {"C1", "#REF!"},
            {"B1+E1", "B1+C1"},
            {"A1:J1", "A1:H1"},
            {"SUM(C:D)", "SUM(#REF!)"},
            {"SUM(A:F)", "SUM(A:D)"},
            {"SUM(2:5)", "SUM(2:5)"},
            {"$E$1", "$C$1"},
        };
        CHECK_FORMULAS(columns, 0, StructEdit::deleteColumns(0, 2, 2));
        static const Case ins[] = {{"B1", "B1"}, {"C1", "D1"}, {"A1:C1", "A1:D1"}, {"SUM(B:C)", "SUM(B:D)"}, {"SUM(1:3)", "SUM(1:3)"}, {"$C$3", "$D$3"}};
        CHECK_FORMULAS(ins, 0, StructEdit::insertColumns(0, 2, 1));
    }

    // Cells inserted and deleted with the cells after them shifted: only a reference that lies within the columns (rows) of the block is changed.
    void shiftingCellsChangesWhatLiesInTheirColumns()
    {
        CellRange block;
        parseRangeRef(QStringLiteral("B2:C3"), &block);
        static const Case down[] = {
            {"B5", "B7"},
            {"C2", "C4"},
            {"A5", "A5"},
            {"D5", "D5"},
            {"A5:C9", "A5:C9"},
            {"B1:C9", "B1:C11"},
            {"B5:B6", "B7:B8"},
            {"B1", "B1"},
            {"SUM(B:C)", "SUM(B:C)"},
        };
        CHECK_FORMULAS(down, 0, StructEdit::insertCellsDown(0, block));
        static const Case up[] = {
            {"B5", "B3"},
            {"B2", "#REF!"},
            {"C3", "#REF!"},
            {"B1:C9", "B1:C7"},
            {"A2:A9", "A2:A9"},
            {"B4:C4", "B2:C2"},
        };
        CHECK_FORMULAS(up, 0, StructEdit::deleteCellsUp(0, block));
        static const Case right[] = {{"B2", "D2"}, {"E3", "G3"}, {"A2", "A2"}, {"B4", "B4"}, {"A1:J3", "A1:J3"}, {"B2:F3", "D2:H3"}, {"A2:D3", "A2:F3"}};
        CHECK_FORMULAS(right, 0, StructEdit::insertCellsRight(0, block));
        static const Case left[] = {{"E2", "C2"}, {"B2", "#REF!"}, {"F2:H3", "D2:F3"}, {"F4", "F4"}};
        CHECK_FORMULAS(left, 0, StructEdit::deleteCellsLeft(0, block));
    }

    // A block cut and pasted: a reference whole inside it goes with it (to another sheet too, which then names that sheet); one that is only partly in it, or outside, stays.
    void movingABlockTakesTheReferencesToItsCells()
    {
        CellRange block;
        parseRangeRef(QStringLiteral("A1:B2"), &block);
        static const Case cases[] = {
            {"A1", "D5"},
            {"B2+A1", "E6+D5"},
            {"A1:B2", "D5:E6"},
            {"A1:A5", "A1:A5"},
            {"C3", "C3"},
            {"$A$1", "$D$5"},
            {"Sheet2!A1", "Sheet2!A1"},
            {"Sheet1!A2", "Sheet1!D6"},
        };
        CHECK_FORMULAS(cases, 0, StructEdit::move(0, block, 0, 4, 3));
        // to another sheet
        static const Case across[] = {{"A1", "Sheet2!D5"}, {"Sheet1!B2", "Sheet2!E6"}, {"C3", "C3"}};
        CHECK_FORMULAS(across, 0, StructEdit::move(0, block, 1, 4, 3));
        // and the same from a formula on the sheet it goes to
        QCOMPARE(adjusted("Sheet1!A1+A1", 1, StructEdit::move(0, block, 1, 4, 3)), QStringLiteral("D5+A1"));
        // the sheet's name in apostrophes
        QCOMPARE(adjusted("A1", 0, StructEdit::move(0, block, 2, 0, 0)), QStringLiteral("'My Sheet'!A1"));
        // a move onto itself changes nothing
        QCOMPARE(adjusted("A1", 0, StructEdit::move(0, block, 0, 0, 0)), QStringLiteral("A1"));
    }

    // A formula copied to another cell: only the parts without "$" move; whole columns and whole rows keep the other kind; what goes off the grid is #REF!.
    void copyingAFormulaMovesItsRelativeParts()
    {
        QCOMPARE(offsetFormula(QStringLiteral("A1+$B$2+C$3+$D4"), 2, 1), QStringLiteral("B3+$B$2+D$3+$D6"));
        QCOMPARE(offsetFormula(QStringLiteral("SUM(A1:B2)*SUM($A1:B$2)"), 1, 1), QStringLiteral("SUM(B2:C3)*SUM($A2:C$2)"));
        QCOMPARE(offsetFormula(QStringLiteral("SUM(A:A)+SUM(1:2)"), 3, 2), QStringLiteral("SUM(C:C)+SUM(4:5)"));
        QCOMPARE(offsetFormula(QStringLiteral("Sheet2!A1+'My Sheet'!B2"), 1, 1), QStringLiteral("Sheet2!B2+'My Sheet'!C3"));
        QCOMPARE(offsetFormula(QStringLiteral("A1"), -1, 0), QStringLiteral("#REF!"));
        QCOMPARE(offsetFormula(QStringLiteral("A1+B2"), 0, -1), QStringLiteral("#REF!+A2"));
        QCOMPARE(offsetFormula(QStringLiteral("XFD1"), 0, 1), QStringLiteral("#REF!"));
        QCOMPARE(offsetFormula(QStringLiteral("A1048576"), 1, 0), QStringLiteral("#REF!"));
        QCOMPARE(offsetFormula(QStringLiteral("\"A1\"&A1"), 1, 0), QStringLiteral("\"A1\"&A2"));
        QCOMPARE(offsetFormula(QStringLiteral("1+2"), 5, 5), QStringLiteral("1+2"));
        QCOMPARE(offsetFormula(QStringLiteral("A1"), 0, 0), QStringLiteral("A1"));
        bool changed = true;
        QCOMPARE(offsetFormula(QStringLiteral("$A$1"), 3, 3, &changed), QStringLiteral("$A$1"));
        QVERIFY(!changed);
        // the edit form of it goes the same way
        QCOMPARE(adjusted("A1+B2", 0, StructEdit::offset(1, 1)), QStringLiteral("B2+C3"));
    }

    // Ranges (merged cells, validations, conditional formats, a table's cells) follow the same rules, and a list of them as a file writes it.
    void rangesAndTheirListsFollowTheCells()
    {
        CellRange r;
        parseRangeRef(QStringLiteral("B2:D4"), &r);
        CellRange x = r;
        QVERIFY(adjustRange(StructEdit::insertRows(0, 2, 1), 0, &x));
        QCOMPARE(cellName(x.row1, x.col1) + QLatin1Char(':') + cellName(x.row2, x.col2), QStringLiteral("B2:D5"));       // (a row inside it: it grows)
        x = r;
        QVERIFY(adjustRange(StructEdit::insertRows(0, 1, 1), 0, &x));
        QCOMPARE(cellName(x.row1, x.col1) + QLatin1Char(':') + cellName(x.row2, x.col2), QStringLiteral("B3:D5"));       // (a row above it: it moves)
        x = r;
        QVERIFY(!adjustRange(StructEdit::deleteRows(0, 1, 3), 0, &x));                                                      // (all its rows deleted)
        x = r;
        QVERIFY(adjustRange(StructEdit::deleteColumns(0, 0, 1), 0, &x));
        QCOMPARE(cellName(x.row1, x.col1) + QLatin1Char(':') + cellName(x.row2, x.col2), QStringLiteral("A2:C4"));
        x = r;
        QVERIFY(adjustRange(StructEdit::insertRows(1, 0, 5), 0, &x));                                                       // (another sheet's rows)
        QCOMPARE(cellName(x.row1, x.col1), QStringLiteral("B2"));
        QCOMPARE(adjustSqref(QStringLiteral("A1:B2 D4 F6:F9"), StructEdit::insertRows(0, 3, 2), 0), QStringLiteral("A1:B2 D6 F8:F11"));
        QCOMPARE(adjustSqref(QStringLiteral("A1:B2 D4 F6:F9"), StructEdit::deleteRows(0, 3, 1), 0), QStringLiteral("A1:B2 F5:F8"));
        QCOMPARE(adjustSqref(QStringLiteral("D4"), StructEdit::deleteRows(0, 3, 1), 0), QString());
        // the inverse of an edit puts the references back (what was not deleted)
        for (const StructEdit &e : {StructEdit::insertRows(0, 4, 3), StructEdit::insertColumns(0, 1, 2), StructEdit::deleteRows(0, 20, 2)}) {
            const QString before = QStringLiteral("SUM(A1:C10)+B3+$D$4+Sheet1!E6:F30");
            QString mid = adjustFormula(before, 0, kNames, e);
            QString back = adjustFormula(mid, 0, kNames, e.inverse());
            if (e.kind == StructEdit::Kind::DeleteRows) continue;   // (what is deleted is not given back by the references)
            QCOMPARE(back, before);
        }
    }

    // The common case is made without reading the formula as one (for sheets of many formulas): it must give what the full reading gives, for every kind of text, including those it
    // leaves to the full reading.
    void theFastWayForRowsGivesWhatTheFullReadingGives()
    {
        static const QStringList pieces = {"A1", "$A$1", "B$12:C34", "LOG10(A1)", "ATAN2(1,2)", "1E5", "2.5", "A1:A5", "R1C1", "Sheet2!A1", "'My Sheet'!B2", "\"A1\"", "name", "Tax2023", "TRUE",
                                           "3:5", "A:A", "$A:$C", "a1", "XFD1048576", "ZZZ9", "ABCD1", "1", "B2", "C10", "D100", "$E7", "F$8", "AA3", "AB20:AC30", "IF(A5>1,B6,C7)", "Table1[Col]",
                                           "#REF!", "Sheet1!B5", "{1,2}", "\xC3\xA9" "t1", "SUM(A1:A9)", "MAX(B2:B40,C3)", "(A7+B8)*2", "-A12", "A1%", "SUM(Sheet2!A1:A3)", "10", "7"};
        static const QStringList joins = {"+", "-", "*", "/", "&", ">", "=", "<>", ","};
        QRandomGenerator rng(20260);
        int fastDecided = 0;
        for (int round = 0; round < 8000; ++round) {
            QString f;
            const int count = 1 + int(rng.bounded(5));
            for (int k = 0; k < count; ++k) f += (k ? joins[int(rng.bounded(joins.size()))] : QString()) + QString::fromUtf8(pieces[int(rng.bounded(pieces.size()))].toUtf8());
            if (rng.bounded(4) == 0) f = QStringLiteral("SUM(") + f + QLatin1Char(')');
            StructEdit e = rng.bounded(2) ? StructEdit::insertRows(rng.bounded(3) == 0 ? 1 : 0, quint32(rng.bounded(40)), 1 + quint32(rng.bounded(3)))
                                          : StructEdit::deleteRows(rng.bounded(3) == 0 ? 1 : 0, quint32(rng.bounded(40)), 1 + quint32(rng.bounded(3)));
            const int own = int(rng.bounded(2));
            if (!jp::calc::parseFormula(f).ok()) continue;   // (a text that is no formula is left as it is by the full reading, and not meant to be)
            bool changedFast = false, changedFull = false;
            setFastRowEdits(true);
            const QString fast = adjustFormula(f, own, kNames, e, &changedFast);
            setFastRowEdits(false);
            const QString full = adjustFormula(f, own, kNames, e, &changedFull);
            setFastRowEdits(true);
            QVERIFY2(fast == full && changedFast == changedFull, qPrintable(QStringLiteral("%1 -> %2 against %3").arg(f, fast, full)));
            if (fast != f) ++fastDecided;
        }
        QVERIFY(fastDecided > 500);
    }
};

JP_TEST_CLASS(SheetRefShiftTests)
#include "tst_sheet_refshift.moc"
