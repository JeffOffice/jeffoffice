// Tests of selecting in a sheet (libs/shell/grid/selection.h and stats.h): clicks, drags, and shift-clicks, whole rows and columns, the arrow keys over hidden rows and merged
// ranges, Ctrl and an arrow to the edge of the data, Home and End, the page keys, Tab and Enter, the numbers the status bar shows, and the walks that do their work a little at a
// time. The sheets are small ones made here; the hostile ones are a million rows.
#include <QtTest>

#include "grid/gridsheet.h"
#include "grid/selection.h"
#include "grid/stats.h"
#include "grid/textwalk.h"

#include <functional>
#include <set>

#include "../localepin.h"
#include "../testrunner.h"

using namespace jp::grid;
using jp::office::sheet::Cell;
using jp::office::sheet::CellRange;
using jp::office::sheet::CellType;
using jp::office::sheet::Sheet;

namespace {

CellRange mergeOf(quint32 r1, quint32 c1, quint32 r2, quint32 c2)
{
    CellRange r;
    r.row1 = r1;
    r.col1 = c1;
    r.row2 = r2;
    r.col2 = c2;
    r.valid = true;
    return r;
}

// Cells that hold something are those a predicate says; the data reaches the extent given.
class Cells : public NavSource {
public:
    std::function<bool(qint64, qint64)> has = [](qint64, qint64) { return false; };
    qint64 rows = 0, cols = 0;
    bool hasValue(qint64 row, qint64 col) const override
    {
        ++looks;
        return has(row, col);
    }
    void usedExtent(qint64 *r, qint64 *c) const override
    {
        *r = rows;
        *c = cols;
    }
    mutable qint64 looks = 0;
    void set(const std::set<std::pair<qint64, qint64>> &cells)
    {
        has = [cells](qint64 r, qint64 c) { return cells.count({r, c}) > 0; };
    }
};

// A grid of 100 rows and 50 columns.
GridLayout small()
{
    GridLayout g;
    g.rows.setCount(100);
    g.cols.setCount(50);
    g.setViewport(QSize(680, 420));
    return g;
}

Selection at(const Navigator &nav, qint64 row, qint64 col) { return nav.click(row, col); }

GridRange rangeOf(qint64 r1, qint64 c1, qint64 r2, qint64 c2)
{
    GridRange r;
    r.row1 = r1;
    r.col1 = c1;
    r.row2 = r2;
    r.col2 = c2;
    return r;
}

// Where Ctrl and an arrow go from a selection, walked to the end.
std::pair<qint64, qint64> jumpFrom(const Navigator &nav, const Selection &s, int dRow, int dCol)
{
    JumpWalk w = nav.jump(s, dRow, dCol);
    while (!w.step(0)) {
    }
    return {w.row(), w.col()};
}

} // namespace

class GridSelectionTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A click selects a cell, or the whole of the merged range it is in; a drag or a shift-click selects from the first cell to the one reached, with merged ranges kept whole.
    void clicksAndDragsSelectCellsKeepingMergedRangesWhole()
    {
        GridLayout g = small();
        g.merges.assign({mergeOf(4, 5, 5, 6)});
        Cells cells;
        Navigator nav(&g, &cells);
        Selection s = nav.click(2, 2);
        QCOMPARE(s.activeRow, qint64(2));
        QCOMPARE(s.activeCol, qint64(2));
        QVERIFY(s.isOneCell() && s.whole == Selection::Whole::None);
        QVERIFY(s.range == rangeOf(2, 2, 2, 2));
        // a click in a merged range lands on its first cell and selects it all
        s = nav.click(5, 6);
        QCOMPARE(s.activeRow, qint64(4));
        QCOMPARE(s.activeCol, qint64(5));
        QVERIFY(s.range == rangeOf(4, 5, 5, 6));
        // a drag from a cell: the range grows from the first cell, whichever way it goes
        s = nav.extendTo(nav.click(2, 2), 5, 3);
        QVERIFY(s.range == rangeOf(2, 2, 5, 3));
        QCOMPARE(s.activeRow, qint64(2));
        s = nav.extendTo(s, 0, 0);
        QVERIFY(s.range == rangeOf(0, 0, 2, 2));
        QCOMPARE(s.activeRow, qint64(2));
        QCOMPARE(s.activeCol, qint64(2));
        // a drag over a merged range takes all of it
        s = nav.extendTo(nav.click(3, 3), 4, 5);
        QVERIFY(s.range == rangeOf(3, 3, 5, 6));
        // and a drag that begins in one takes all of it too
        s = nav.extendTo(nav.click(4, 5), 8, 8);
        QVERIFY(s.range == rangeOf(4, 5, 8, 8));
        s = nav.extendTo(nav.click(5, 6), 1, 1);
        QVERIFY(s.range == rangeOf(1, 1, 5, 6));
    }

    // The arrow keys move one cell, over hidden rows and columns; a merged range is one cell wide and tall to the cursor; at the edge of the sheet it stays.
    void arrowKeysMoveOverHiddenRowsAndWholeMergedRanges()
    {
        GridLayout g = small();
        g.rows.setSize(3, 0);
        g.cols.setSize(2, 0);
        g.merges.assign({mergeOf(4, 5, 5, 6)});
        Cells cells;
        Navigator nav(&g, &cells);
        Selection s = nav.click(2, 0);
        s = nav.move(s, 1, 0, false);
        QCOMPARE(s.activeRow, qint64(4));   // (row 3 is hidden)
        s = nav.move(s, 0, 1, false);
        QCOMPARE(s.activeCol, qint64(1));
        s = nav.move(s, 0, 1, false);
        QCOMPARE(s.activeCol, qint64(3));   // (column 2 is hidden)
        s = nav.move(s, 0, -1, false);
        QCOMPARE(s.activeCol, qint64(1));
        s = nav.move(nav.click(4, 4), 0, 1, false);
        QCOMPARE(s.activeCol, qint64(5));
        QVERIFY(s.range == rangeOf(4, 5, 5, 6));
        s = nav.move(s, 0, 1, false);
        QCOMPARE(s.activeCol, qint64(7));   // (past the merged range)
        s = nav.move(nav.click(4, 7), 0, -1, false);
        QCOMPARE(s.activeCol, qint64(5));   // (onto its first cell)
        s = nav.move(s, 0, -1, false);
        QCOMPARE(s.activeCol, qint64(4));
        s = nav.move(nav.click(4, 5), 1, 0, false);
        QCOMPARE(s.activeRow, qint64(6));
        s = nav.move(nav.click(6, 5), -1, 0, false);
        QCOMPARE(s.activeRow, qint64(4));
        // the edges
        s = nav.move(nav.click(0, 0), -1, 0, false);
        QCOMPARE(s.activeRow, qint64(0));
        s = nav.move(nav.click(0, 0), 0, -1, false);
        QCOMPARE(s.activeCol, qint64(0));
        s = nav.move(nav.click(99, 49), 1, 0, false);
        QCOMPARE(s.activeRow, qint64(99));
        s = nav.move(nav.click(99, 49), 0, 1, false);
        QCOMPARE(s.activeCol, qint64(49));
        // a row hidden at the first place: the first shown row is the edge
        g.rows.setSize(0, 0);
        s = nav.move(nav.click(1, 0), -1, 0, false);
        QCOMPARE(s.activeRow, qint64(1));
    }

    // With Shift, the corner that is not the active cell moves, and the range is the active cell to it (merged ranges whole); the active cell stays.
    void shiftWithAnArrowExtendsTheSelectionFromItsActiveCell()
    {
        GridLayout g = small();
        g.rows.setSize(3, 0);
        g.merges.assign({mergeOf(4, 5, 5, 6)});
        Cells cells;
        Navigator nav(&g, &cells);
        Selection s = nav.click(2, 0);
        s = nav.move(s, 1, 0, true);
        QVERIFY(s.range == rangeOf(2, 0, 4, 0));
        QCOMPARE(s.activeRow, qint64(2));
        QCOMPARE(s.headRow, qint64(4));
        s = nav.move(s, 0, 1, true);
        QVERIFY(s.range == rangeOf(2, 0, 4, 1));
        s = nav.move(s, -1, 0, true);
        QVERIFY(s.range == rangeOf(2, 0, 2, 1));
        // taking in a merged range at the corner takes all of it
        s = nav.click(2, 0);
        for (int i = 0; i < 5; ++i) s = nav.move(s, 0, 1, true);
        QCOMPARE(s.headCol, qint64(5));
        QVERIFY(s.range == rangeOf(2, 0, 2, 5));
        s = nav.move(s, 1, 0, true);   // the head is at row 4 now, among the merged cells: the range takes them all
        QVERIFY(s.range == rangeOf(2, 0, 5, 6));
        // at the edge it stays
        s = nav.move(nav.click(0, 0), -1, 0, true);
        QVERIFY(s.range == rangeOf(0, 0, 0, 0));
    }

    // Ctrl and an arrow go to the end of the run of cells that hold something, or, from the end or from an empty cell, to the next that does, or to the edge of the sheet.
    void ctrlAndAnArrowGoToTheEdgeOfTheData()
    {
        GridLayout g = small();
        Cells cells;
        std::set<std::pair<qint64, qint64>> held;
        for (qint64 r = 0; r <= 4; ++r) held.insert({r, 0});
        held.insert({8, 0});
        held.insert({9, 0});
        for (qint64 c : {0, 1, 2, 3, 4, 10}) held.insert({0, c});
        cells.set(held);
        Navigator nav(&g, &cells);
        auto down = [&](qint64 r, qint64 c) { return jumpFrom(nav, at(nav, r, c), 1, 0); };
        auto up = [&](qint64 r, qint64 c) { return jumpFrom(nav, at(nav, r, c), -1, 0); };
        QCOMPARE(down(0, 0).first, qint64(4));    // to the end of the run
        QCOMPARE(down(4, 0).first, qint64(8));    // from the end of a run: the next cell that holds something
        QCOMPARE(down(8, 0).first, qint64(9));
        QCOMPARE(down(9, 0).first, qint64(99));   // none: the edge
        QCOMPARE(down(99, 0).first, qint64(99));
        QCOMPARE(down(6, 3).first, qint64(99));   // from an empty cell in an empty column
        QCOMPARE(down(6, 0).first, qint64(8));    // from an empty cell: the next that holds something
        QCOMPARE(up(99, 0).first, qint64(9));
        QCOMPARE(up(9, 0).first, qint64(8));
        QCOMPARE(up(8, 0).first, qint64(4));
        QCOMPARE(up(4, 0).first, qint64(0));
        QCOMPARE(up(0, 0).first, qint64(0));
        QCOMPARE(jumpFrom(nav, at(nav, 0, 0), 0, 1).second, qint64(4));
        QCOMPARE(jumpFrom(nav, at(nav, 0, 4), 0, 1).second, qint64(10));
        QCOMPARE(jumpFrom(nav, at(nav, 0, 10), 0, 1).second, qint64(49));
        QCOMPARE(jumpFrom(nav, at(nav, 0, 10), 0, -1).second, qint64(4));
        QCOMPARE(jumpFrom(nav, at(nav, 0, 0), 0, -1).second, qint64(0));
        // hidden rows are not stopped at, and the run goes through them; one at the end of the run is not where it ends
        g.rows.setSize(2, 0);
        QCOMPARE(down(0, 0).first, qint64(4));
        g.rows.setSize(4, 0);
        QCOMPARE(down(0, 0).first, qint64(3));
        g.rows.setSize(99, 0);
        QCOMPARE(down(9, 0).first, qint64(98));   // (the last shown row is the edge)
        // arriving: the cursor there, or the range from the active cell to there
        Selection s = nav.arrive(at(nav, 0, 0), 3, 0, false);
        QCOMPARE(s.activeRow, qint64(3));
        QVERIFY(s.range == rangeOf(3, 0, 3, 0));
        s = nav.arrive(at(nav, 1, 0), 8, 0, true);
        QCOMPARE(s.activeRow, qint64(1));
        QVERIFY(s.range == rangeOf(1, 0, 8, 0));
        // a merged range is one cell: the walk starts from its edge in the direction
        GridLayout m = small();
        m.merges.assign({mergeOf(1, 0, 3, 0)});
        Navigator nav2(&m, &cells);
        QCOMPARE(jumpFrom(nav2, nav2.click(1, 0), 1, 0).first, qint64(4));
    }

    // Home goes to the first shown column of the row; Ctrl+Home to the first cell that scrolls (A1 with nothing frozen); Ctrl+End to the last cell the data reaches.
    void homeEndAndTheCornersOfTheData()
    {
        GridLayout g = small();
        Cells cells;
        cells.rows = 30;
        cells.cols = 8;
        Navigator nav(&g, &cells);
        Selection s = nav.home(nav.click(5, 7), false, false);
        QCOMPARE(s.activeRow, qint64(5));
        QCOMPARE(s.activeCol, qint64(0));
        s = nav.home(nav.click(5, 7), true, false);
        QCOMPARE(s.activeRow, qint64(0));
        QCOMPARE(s.activeCol, qint64(0));
        s = nav.end(nav.click(5, 1), false);
        QCOMPARE(s.activeRow, qint64(29));
        QCOMPARE(s.activeCol, qint64(7));
        // extended: from the active cell
        s = nav.end(nav.click(5, 1), true);
        QCOMPARE(s.activeRow, qint64(5));
        QVERIFY(s.range == rangeOf(5, 1, 29, 7));
        s = nav.home(nav.click(5, 4), true, true);
        QVERIFY(s.range == rangeOf(0, 0, 5, 4));
        QCOMPARE(s.activeRow, qint64(5));
        // hidden at the edges
        g.rows.setSize(29, 0);
        g.cols.setSize(0, 0);
        s = nav.end(nav.click(0, 1), false);
        QCOMPARE(s.activeRow, qint64(28));
        s = nav.home(nav.click(5, 4), false, false);
        QCOMPARE(s.activeCol, qint64(1));
        // frozen rows and columns: the first cell that scrolls
        g.cols.setSize(0, 64);
        g.setFrozen(2, 1);
        s = nav.home(nav.click(50, 4), true, false);
        QCOMPARE(s.activeRow, qint64(2));
        QCOMPARE(s.activeCol, qint64(1));
        // a sheet with no data: A1
        Cells none;
        Navigator nav2(&g, &none);
        g.setFrozen(0, 0);
        s = nav2.end(nav2.click(5, 5), false);
        QCOMPARE(s.activeRow, qint64(0));
        QCOMPARE(s.activeCol, qint64(0));
    }

    // Page Down and Up move the cursor by as many shown rows as the screen holds, in the same column, and stop at the first and last row.
    void pageKeysMoveByAScreenOfShownRows()
    {
        GridLayout g = small();
        g.rows.setSize(8, 0);
        g.rows.setSize(9, 0);
        Cells cells;
        Navigator nav(&g, &cells);
        Selection s = nav.page(nav.click(5, 7), 1, 10, false);
        QCOMPARE(s.activeRow, qint64(17));   // (ten shown rows on: two are hidden)
        QCOMPARE(s.activeCol, qint64(7));
        s = nav.page(s, -1, 10, false);
        QCOMPARE(s.activeRow, qint64(5));
        s = nav.page(nav.click(3, 0), -1, 10, false);
        QCOMPARE(s.activeRow, qint64(0));
        s = nav.page(nav.click(95, 0), 1, 10, false);
        QCOMPARE(s.activeRow, qint64(99));
        s = nav.page(nav.click(5, 7), 1, 10, true);
        QCOMPARE(s.activeRow, qint64(5));
        QVERIFY(s.range == rangeOf(5, 7, 17, 7));
    }

    // Tab goes to the next cell on the right and Enter to the next below (Shift the other way); with a range selected they go through the cells of the range and round again.
    void tabAndEnterGoThroughTheSelectionOrTheSheet()
    {
        GridLayout g = small();
        g.cols.setSize(5, 0);
        g.rows.setSize(4, 0);
        Cells cells;
        Navigator nav(&g, &cells);
        Selection s = nav.tab(nav.click(2, 3), false);
        QCOMPARE(s.activeCol, qint64(4));
        s = nav.tab(s, false);
        QCOMPARE(s.activeCol, qint64(6));
        s = nav.tab(s, true);
        QCOMPARE(s.activeCol, qint64(4));
        s = nav.enter(nav.click(3, 4), false);
        QCOMPARE(s.activeRow, qint64(5));
        s = nav.enter(s, true);
        QCOMPARE(s.activeRow, qint64(3));
        QCOMPARE(nav.tab(nav.click(2, 49), false).activeCol, qint64(49));
        QCOMPARE(nav.enter(nav.click(99, 2), false).activeRow, qint64(99));
        // through a range of two rows and two columns
        Selection r = nav.extendTo(nav.click(2, 3), 3, 4);
        QVERIFY(r.range == rangeOf(2, 3, 3, 4));
        auto cell = [](const Selection &x) { return std::make_pair(x.activeRow, x.activeCol); };
        std::vector<std::pair<qint64, qint64>> order = {{2, 4}, {3, 3}, {3, 4}, {2, 3}};
        Selection t = r;
        for (const auto &want : order) {
            t = nav.tab(t, false);
            QCOMPARE(cell(t), want);
            QVERIFY(t.range == r.range);
        }
        std::vector<std::pair<qint64, qint64>> back = {{3, 4}, {3, 3}, {2, 4}, {2, 3}};
        for (const auto &want : back) {
            t = nav.tab(t, true);
            QCOMPARE(cell(t), want);
        }
        order = {{3, 3}, {2, 4}, {3, 4}, {2, 3}};
        for (const auto &want : order) {
            t = nav.enter(t, false);
            QCOMPARE(cell(t), want);
            QVERIFY(t.range == r.range);
        }
        t = nav.enter(t, true);
        QCOMPARE(cell(t), std::make_pair(qint64(3), qint64(4)));
    }

    // Whole rows and whole columns: the headers' clicks and drags, Shift+Space and Ctrl+Space, and the whole sheet.
    void wholeRowsAndColumnsAreSelected()
    {
        GridLayout g = small();
        Cells cells;
        Navigator nav(&g, &cells);
        Selection s = nav.selectRows(3, 5);
        QVERIFY(s.whole == Selection::Whole::Rows);
        QVERIFY(s.range == rangeOf(3, 0, 5, 49));
        QCOMPARE(s.activeRow, qint64(3));
        QCOMPARE(s.activeCol, qint64(0));
        s = nav.selectRows(5, 3);   // (dragged upward)
        QVERIFY(s.range == rangeOf(3, 0, 5, 49));
        QCOMPARE(s.activeRow, qint64(5));
        s = nav.selectColumns(2, 4);
        QVERIFY(s.whole == Selection::Whole::Columns);
        QVERIFY(s.range == rangeOf(0, 2, 99, 4));
        QCOMPARE(s.activeRow, qint64(0));
        QCOMPARE(s.activeCol, qint64(2));
        // a drag or a shift-click on a header: from where it began
        s = nav.extendRows(nav.selectRows(3, 3), 8);
        QVERIFY(s.range == rangeOf(3, 0, 8, 49));
        s = nav.extendRows(s, 1);
        QVERIFY(s.range == rangeOf(1, 0, 3, 49));
        s = nav.extendColumns(nav.selectColumns(6, 6), 2);
        QVERIFY(s.range == rangeOf(0, 2, 99, 6));
        // Shift+Space and Ctrl+Space take the rows and columns of the selection; the active cell stays
        Selection part = nav.extendTo(nav.click(1, 4), 3, 6);
        s = nav.selectWholeRows(part);
        QVERIFY(s.whole == Selection::Whole::Rows && s.range == rangeOf(1, 0, 3, 49));
        QCOMPARE(s.activeRow, qint64(1));
        QCOMPARE(s.activeCol, qint64(4));
        s = nav.selectWholeColumns(part);
        QVERIFY(s.whole == Selection::Whole::Columns && s.range == rangeOf(0, 4, 99, 6));
        // the whole sheet
        s = nav.selectAll(part);
        QVERIFY(s.whole == Selection::Whole::All && s.range == rangeOf(0, 0, 99, 49));
        QCOMPARE(s.activeRow, qint64(1));
        // a merged range at the edge of a whole-row selection widens it
        g.merges.assign({mergeOf(2, 3, 4, 4)});
        QVERIFY(nav.selectRows(3, 3).range == rangeOf(2, 0, 4, 49) || nav.selectRows(3, 3).range == rangeOf(3, 0, 3, 49));
    }

    // The walk for the edge of the data in a sheet of a million rows does the work it is given at a time, and finds the same place as a walk all at once.
    void aWalkOverAMillionRowsIsDoneInSlices()
    {
        GridLayout g;
        g.setViewport(QSize(680, 420));
        Cells empty;
        Navigator nav(&g, &empty);
        JumpWalk w = nav.jump(nav.click(0, 0), 1, 0);
        int slices = 0;
        qint64 most = 0;
        while (true) {
            const qint64 before = w.looks();
            const bool done = w.step(50000);
            most = std::max(most, w.looks() - before);
            ++slices;
            if (done) break;
            QVERIFY(slices < 1000);
        }
        QCOMPARE(w.row(), qint64(1048575));
        QVERIFY2(slices >= 20, qPrintable(QString::number(slices)));
        QVERIFY2(most <= 50000 + 8, qPrintable(QString::number(most)));
        // a run of a million cells that hold something
        Cells dense;
        dense.has = [](qint64, qint64 c) { return c == 0; };
        Navigator nav2(&g, &dense);
        JumpWalk w2 = nav2.jump(nav2.click(0, 0), 1, 0);
        int slices2 = 0;
        while (!w2.step(50000)) ++slices2;
        QCOMPARE(w2.row(), qint64(1048575));
        QVERIFY(slices2 >= 20);
        // and one the budget of none does at once
        JumpWalk w3 = nav2.jump(nav2.click(5, 0), 1, 0);
        QVERIFY(w3.step(0));
        QCOMPARE(w3.row(), qint64(1048575));
        // hidden rows cost nothing to step over: a hundred thousand of them are one step
        GridLayout h;
        h.setViewport(QSize(680, 420));
        for (qint64 r = 10; r < 100010; ++r) h.rows.setSize(r, 0);
        Navigator nav3(&h, &dense);
        const qint64 before = dense.looks;
        JumpWalk w4 = nav3.jump(nav3.click(0, 0), 1, 0);
        QVERIFY(w4.step(0));
        QVERIFY2(dense.looks - before > 900000, "(the cells after the hidden ones are looked at)");
        QCOMPARE(w4.row(), qint64(1048575));
    }
};

class GridStatsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The cells that hold something, the numbers among them, and their sum, average, smallest, and largest; hidden rows and columns are left out.
    void aRangeIsCountedAndAddedUp()
    {
        const PinnedLocale english;
        Sheet sheet;
        auto num = [&](quint32 r, quint32 c, double v) {
            jp::office::sheet::PlacedCell x;
            x.row = r;
            x.col = quint16(c);
            x.cell.type = CellType::Number;
            x.cell.number = v;
            sheet.cells.add(x);
        };
        num(0, 0, 1);
        num(1, 0, 2);
        num(2, 0, 3);
        num(3, 0, 100);   // (in a hidden row)
        num(0, 3, 1000);  // (in a hidden column)
        jp::office::sheet::PlacedCell text;
        text.row = 0;
        text.col = 1;
        text.cell.type = CellType::InlineString;
        text.cell.number = 0;
        sheet.texts.append(QStringLiteral("x"));
        sheet.cells.add(text);
        jp::office::sheet::PlacedCell flag;
        flag.row = 1;
        flag.col = 1;
        flag.cell.type = CellType::Bool;
        flag.cell.number = 1;
        sheet.cells.add(flag);
        jp::office::sheet::PlacedCell err;
        err.row = 2;
        err.col = 1;
        err.cell.type = CellType::Error;
        err.cell.number = 1;
        sheet.cells.add(err);
        jp::office::sheet::PlacedCell blank;   // (a cell with a format and nothing in it)
        blank.row = 4;
        blank.col = 1;
        blank.cell.style = 2;
        sheet.cells.add(blank);
        sheet.cells.finish();
        jp::office::sheet::Styles styles;
        styles.fonts = {jp::office::sheet::FontSpec()};
        styles.formats = {jp::office::sheet::CellFormat()};
        GridData d;
        d.styles = &styles;
        d.sheet = &sheet;
        GridSheet g;
        g.setData(d);
        g.layout().rows.setSize(3, 0);
        g.layout().cols.setSize(3, 0);
        const Stats s = computeStats(g, rangeOf(0, 0, 4, 3));
        QCOMPARE(s.count, qint64(6));
        QCOMPARE(s.numbers, qint64(3));
        QCOMPARE(s.sum, 6.0);
        QCOMPARE(s.min, 1.0);
        QCOMPARE(s.max, 3.0);
        QCOMPARE(s.average(), 2.0);
        // a range of nothing
        const Stats none = computeStats(g, rangeOf(10, 10, 20, 20));
        QCOMPARE(none.count, qint64(0));
        QVERIFY(!none.hasNumbers());
        QCOMPARE(none.average(), 0.0);
        // the words
        QCOMPARE(statsText(s), QStringLiteral("Average: 2    Count: 6    Sum: 6"));
        Stats two;
        two.count = 2;
        QCOMPARE(statsText(two), QStringLiteral("Count: 2"));
        Stats one;
        one.count = 1;
        QCOMPARE(statsText(one), QString());
        Stats only;
        only.count = 1;
        only.numbers = 1;
        only.sum = only.min = only.max = 3.5;
        QCOMPARE(statsText(only), QStringLiteral("Average: 3.5    Count: 1    Sum: 3.5"));
        QCOMPARE(statsText(Stats()), QString());
    }

    // The sum and the average in the status line have at most ten significant digits (14.58333333333 would be a number nobody reads), in the person's language.
    void statsAreShownToTenDigitsInTheLocale()
    {
        Stats s;
        s.count = 12;
        s.numbers = 12;
        s.sum = 175.0;   // (the average is 14.583333333333334)
        s.min = 1;
        s.max = 30;
        {
            PinnedLocale pin(QLocale(QLocale::English, QLocale::UnitedStates));
            QCOMPARE(statsText(s), QStringLiteral("Average: 14.58333333    Count: 12    Sum: 175"));
        }
        {
            PinnedLocale pin(QLocale(QLocale::German, QLocale::Germany));
            const QString text = statsText(s);
            QVERIFY2(text.contains(QStringLiteral("14,58333333")) && !text.contains(QStringLiteral("14,583333333")), qPrintable(text));
        }
    }

    // A whole column of a million rows is added up a few thousand rows at a time, and comes to what adding it all at once does.
    void aWholeColumnIsAddedUpInSlices()
    {
        Sheet sheet;
        double sum = 0;
        qint64 n = 0;
        for (quint32 r = 0; r < 1048576; r += 5) {
            jp::office::sheet::PlacedCell x;
            x.row = r;
            x.col = 0;
            x.cell.type = CellType::Number;
            x.cell.number = r % 97;
            sheet.cells.add(x);
            sum += x.cell.number;
            ++n;
        }
        sheet.cells.finish();
        jp::office::sheet::Styles styles;
        styles.fonts = {jp::office::sheet::FontSpec()};
        styles.formats = {jp::office::sheet::CellFormat()};
        GridData d;
        d.styles = &styles;
        d.sheet = &sheet;
        GridSheet g;
        g.setData(d);
        StatsWalk w(&g, rangeOf(0, 0, 1048575, 0));
        int slices = 0;
        qint64 most = 0;
        while (true) {
            const qint64 before = w.rowsLooked();
            const bool done = w.step(10000);
            most = std::max(most, w.rowsLooked() - before);
            ++slices;
            if (done) break;
        }
        QVERIFY2(slices >= 50, qPrintable(QString::number(slices)));
        QVERIFY2(most <= 10000 + StatsWalk::kWorkPerSlice, qPrintable(QString::number(most)));
        QCOMPARE(w.stats().numbers, n);
        QCOMPARE(w.stats().sum, sum);
        QCOMPARE(w.rowsLooked(), qint64(1048576));
        // all at once
        const Stats all = computeStats(g, rangeOf(0, 0, 1048575, 0));
        QCOMPARE(all.numbers, n);
        QCOMPARE(all.sum, sum);
        QCOMPARE(all.min, 0.0);
    }

    // A walk does its work in slices of the cells it looks at, not of rows (a row may be sixteen thousand cells): a slice of a thousand looks at about a thousand cells and a row
    // more, and the columns of the range that are hidden are worked out once.
    void aSliceIsBoundedByCellsAndNotRows()
    {
        Sheet sheet;
        for (quint32 r = 0; r < 200; ++r)
            for (quint32 c = 0; c < 2000; ++c) {
                jp::office::sheet::PlacedCell x;
                x.row = r;
                x.col = quint16(c);
                x.cell.type = CellType::Number;
                x.cell.number = 1;
                sheet.cells.add(x);
            }
        sheet.cells.finish();
        sheet.extentRows = 200;
        sheet.extentCols = 2000;
        jp::office::sheet::Styles styles;
        styles.fonts = {jp::office::sheet::FontSpec()};
        styles.formats = {jp::office::sheet::CellFormat()};
        GridData d;
        d.styles = &styles;
        d.sheet = &sheet;
        GridSheet g;
        g.setData(d);
        g.layout().cols.setSize(7, 0);
        StatsWalk w(&g, rangeOf(0, 0, 199, 1999));
        qint64 most = 0, slices = 0;
        for (;;) {
            const qint64 before = w.cellsLooked();
            const bool done = w.step(1000);
            most = std::max(most, w.cellsLooked() - before);
            ++slices;
            if (done) break;
        }
        QVERIFY2(most <= 1000 + 2000, qPrintable(QStringLiteral("a slice looked at %1 cells").arg(most)));
        QVERIFY2(slices >= 100, qPrintable(QString::number(slices)));
        QCOMPARE(w.stats().numbers, qint64(200 * 1999));   // (one column hidden)
        QCOMPARE(w.stats().sum, 200.0 * 1999);
        QCOMPARE(w.hiddenBuilds(), qint64(1));
    }

    // The numbers of a selection are written as the user's locale writes them, to ten digits: a sum of twelve billion is written out and grouped, not in the scientific way.
    void sumsAreWrittenAsNumbersAreInTheUsersLocale()
    {
        const QLocale saved = QLocale();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        Stats s;
        s.count = 2;
        s.numbers = 2;
        s.sum = 12345678901.0;
        QCOMPARE(statsText(s), QStringLiteral("Average: 6,172,839,451    Count: 2    Sum: 12,345,678,901"));
        s.count = 1234567;
        s.numbers = 4;
        s.sum = 0.1 + 0.2;
        QCOMPARE(statsText(s), QStringLiteral("Average: 0.075    Count: 1,234,567    Sum: 0.3"));
        s.count = s.numbers = 3;
        s.sum = -4500.25;
        QCOMPARE(statsText(s), QStringLiteral("Average: -1,500.083333    Count: 3    Sum: -4,500.25"));
        QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));
        s.count = s.numbers = 2;
        s.sum = 1234.5;
        QCOMPARE(statsText(s), QStringLiteral("Average: 617,25    Count: 2    Sum: 1.234,5"));
        QLocale::setDefault(saved);
    }
};

class GridTextWalkTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The cells of a block as text: tabs between cells, a line to a row, what each cell shows; a text with a tab, a line break, or a quote is quoted; hidden rows and columns are left out.
    void aBlockIsTextWithTabsAndLines()
    {
        Sheet sheet;
        auto num = [&](quint32 r, quint32 c, double v) {
            jp::office::sheet::PlacedCell x;
            x.row = r;
            x.col = quint16(c);
            x.cell.type = CellType::Number;
            x.cell.number = v;
            sheet.cells.add(x);
        };
        auto text = [&](quint32 r, quint32 c, const QString &t) {
            jp::office::sheet::PlacedCell x;
            x.row = r;
            x.col = quint16(c);
            x.cell.type = CellType::InlineString;
            x.cell.number = sheet.texts.size();
            sheet.texts.append(t);
            sheet.cells.add(x);
        };
        num(0, 0, 1);
        num(0, 2, 3);
        text(1, 1, QStringLiteral("a\tb \"q\"\nline"));
        num(2, 0, 7);   // (in a hidden row)
        num(3, 3, 9);   // (in a hidden column)
        num(3, 1, 4);
        sheet.cells.finish();
        sheet.extentRows = 4;
        sheet.extentCols = 4;
        jp::office::sheet::Styles styles;
        styles.fonts = {jp::office::sheet::FontSpec()};
        styles.formats = {jp::office::sheet::CellFormat()};
        GridData d;
        d.styles = &styles;
        d.sheet = &sheet;
        GridSheet g;
        g.setData(d);
        g.layout().rows.setSize(2, 0);
        g.layout().cols.setSize(3, 0);
        TextWalk w(&g, GridRange{0, 0, 3, 3});
        QVERIFY(!w.tooMany());
        QVERIFY(w.step(0));
        QCOMPARE(w.text(), QStringLiteral("1\t\t3\n\t\"a\tb \"\"q\"\"\nline\"\t\n\t4\t\n"));
        // a block with nothing in it
        TextWalk none(&g, GridRange{10, 10, 12, 12});
        QVERIFY(none.step(0));
        QCOMPARE(none.text(), QStringLiteral("\t\t\n\t\t\n\t\t\n"));
    }

    // More than a million cells are not made into text; a column of a million rows is made a few thousand cells at a time, into the same text as all at once.
    void aLongBlockIsMadeInSlices()
    {
        Sheet sheet;
        for (quint32 r = 0; r < 200000; ++r) {
            jp::office::sheet::PlacedCell x;
            x.row = r;
            x.col = 0;
            x.cell.type = CellType::Number;
            x.cell.number = r;
            sheet.cells.add(x);
        }
        sheet.cells.finish();
        sheet.extentRows = 200000;
        sheet.extentCols = 1;
        jp::office::sheet::Styles styles;
        styles.fonts = {jp::office::sheet::FontSpec()};
        styles.formats = {jp::office::sheet::CellFormat()};
        GridData d;
        d.styles = &styles;
        d.sheet = &sheet;
        GridSheet g;
        g.setData(d);
        TextWalk sliced(&g, GridRange{0, 0, 199999, 0});
        int slices = 0;
        qint64 most = 0;
        while (true) {
            const qint64 before = sliced.looked();
            const bool done = sliced.step(5000);
            most = std::max(most, sliced.looked() - before);
            ++slices;
            if (done) break;
        }
        QVERIFY2(slices >= 30, qPrintable(QString::number(slices)));
        QVERIFY2(most <= 5000 + 8, qPrintable(QString::number(most)));
        TextWalk whole(&g, GridRange{0, 0, 199999, 0});
        QVERIFY(whole.step(0));
        QCOMPARE(sliced.text(), whole.text());
        QVERIFY(whole.text().startsWith(QStringLiteral("0\n1\n2\n")));
        QVERIFY(whole.text().endsWith(QStringLiteral("\n199999\n")));
        // a wide row is sliced inside the row
        TextWalk wide(&g, GridRange{0, 0, 2, 5000});
        int wideSlices = 0;
        while (!wide.step(1000)) ++wideSlices;
        QVERIFY(wideSlices >= 5);
        // too many
        TextWalk huge(&g, GridRange{0, 0, 1048575, 16383});
        QVERIFY(huge.tooMany());
        QVERIFY(huge.step(0));
        QVERIFY(huge.text().isEmpty());
    }

    // A copy of cells whose texts are very long stops at a bound of characters (and says so), where it would have gone on to an allocation no program survives; and each slice is of
    // about the work it was given, a long text counting for its length.
    void aCopyOfVeryLongTextsStopsAtABound()
    {
        Sheet sheet;
        sheet.texts.append(QString(100000, QLatin1Char('x')));
        for (quint32 r = 0; r < 1000; ++r) {
            jp::office::sheet::PlacedCell x;
            x.row = r;
            x.col = 0;
            x.cell.type = CellType::InlineString;
            x.cell.number = 0;   // (every cell has the same text: 100 million characters in all)
            sheet.cells.add(x);
        }
        sheet.cells.finish();
        sheet.extentRows = 1000;
        sheet.extentCols = 1;
        jp::office::sheet::Styles styles;
        styles.fonts = {jp::office::sheet::FontSpec()};
        styles.formats = {jp::office::sheet::CellFormat()};
        GridData d;
        d.styles = &styles;
        d.sheet = &sheet;
        GridSheet g;
        g.setData(d);
        TextWalk w(&g, GridRange{0, 0, 999, 0});
        QVERIFY(!w.tooMany());
        qint64 most = 0;
        for (;;) {
            const qint64 before = w.looked();
            const bool done = w.step(20000);
            most = std::max(most, w.looked() - before);
            if (done) break;
        }
        QVERIFY(w.tooLarge());
        QVERIFY2(w.chars() <= TextWalk::kMaxChars + 100002, qPrintable(QString::number(w.chars())));
        QVERIFY(w.text().size() <= TextWalk::kMaxChars + 100002);
        QVERIFY2(most <= 20000 + 8 + 100000 / 64 + 100, qPrintable(QStringLiteral("a slice did %1 of work").arg(most)));
        // a copy of what fits is whole
        TextWalk small(&g, GridRange{0, 0, 9, 0});
        QVERIFY(small.step(0));
        QVERIFY(!small.tooLarge());
        QCOMPARE(small.text().size(), qint64(10 * 100001));
    }
};

JP_TEST_CLASS(GridSelectionTests)
JP_TEST_CLASS(GridTextWalkTests)
JP_TEST_CLASS(GridStatsTests)
#include "tst_grid_select.moc"
