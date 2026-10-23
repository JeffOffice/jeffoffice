// Tests of where the grid puts things (libs/shell/grid/layout.h and merges.h): which merged range holds a cell and which cross a block, the cell under a point with
// merged ranges, frozen rows and columns, scrolling, and zoom, and the rows and columns each part of the grid shows. No window: the layout is numbers.
#include <QtTest>

#include "grid/layout.h"
#include "grid/merges.h"

#include "../testrunner.h"

using namespace jp::grid;

namespace {

CellRange range(quint32 r1, quint32 c1, quint32 r2, quint32 c2)
{
    CellRange r;
    r.row1 = r1;
    r.col1 = c1;
    r.row2 = r2;
    r.col2 = c2;
    r.valid = true;
    return r;
}

// A grid of 1,000 rows of 20 and 100 columns of 64, headers 40 wide and 20 high, in a window of 680 by 420 (a body of 640 by 400).
GridLayout plain()
{
    GridLayout g;
    g.rows.setCount(1000);
    g.cols.setCount(100);
    g.rows.setDefaultSize(20);
    g.cols.setDefaultSize(64);
    g.setHeaders(true, 40, 20);
    g.setViewport(QSize(680, 420));
    return g;
}

} // namespace

class GridMergeTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The merge a cell is in, and the cells that are in none.
    void aCellIsFoundInTheMergeThatHoldsIt()
    {
        MergeIndex m;
        m.assign({range(5, 2, 7, 4), range(0, 0, 1, 1), range(5, 6, 5, 8), range(10, 0, 10, 0) /* one cell: no merge */});
        QCOMPARE(m.count(), 3);
        const int a = m.at(6, 3);
        QVERIFY(a >= 0);
        QVERIFY(m.range(a) == range(5, 2, 7, 4));
        QVERIFY(m.at(0, 0) >= 0 && m.at(1, 1) >= 0);
        QVERIFY(m.range(m.at(1, 0)) == range(0, 0, 1, 1));
        QCOMPARE(m.at(2, 0), -1);
        QCOMPARE(m.at(5, 5), -1);
        QVERIFY(m.at(5, 7) >= 0);
        QCOMPARE(m.at(10, 0), -1);
        QCOMPARE(m.at(8, 3), -1);
        // a merge of a hundred thousand rows is found like any other, and does not slow the others' searches
        m.assign({range(0, 0, 99999, 2), range(5, 5, 6, 6), range(2000, 4, 2001, 5)});
        QVERIFY(m.at(50000, 1) >= 0);
        QVERIFY(m.range(m.at(50000, 1)) == range(0, 0, 99999, 2));
        QVERIFY(m.range(m.at(5, 6)) == range(5, 5, 6, 6));
        QCOMPARE(m.at(50000, 3), -1);
    }

    // The merges that cross a block of the screen, in order, up to a limit.
    void theMergesThatCrossABlockAreFound()
    {
        MergeIndex m;
        m.assign({range(2, 0, 3, 1), range(10, 5, 12, 6), range(30, 0, 30, 4), range(11, 2, 11, 3)});
        std::vector<int> found;
        QVERIFY(m.crossing(0, 0, 11, 5, 10, &found));
        QCOMPARE(int(found.size()), 3);   // the first, the second (rows 10 to 12 reach in), and the fourth
        QVERIFY(m.range(found[0]) == range(2, 0, 3, 1));
        found.clear();
        QVERIFY(m.crossing(0, 0, 100, 0, 10, &found));
        QCOMPARE(int(found.size()), 2);   // only those that reach the first column
        found.clear();
        QVERIFY(!m.crossing(0, 0, 100, 10, 2, &found));   // four cross, two were asked for
        QCOMPARE(int(found.size()), 2);
        found.clear();
        QVERIFY(m.crossing(13, 0, 29, 9, 5, &found));
        QVERIFY(found.empty());
    }

    // A hundred thousand merges, one for each fifth row: a search for the merge of a cell, and for those on a screen, looks at the merges near, not at them all.
    void aSearchAmongAHundredThousandMergesLooksOnlyAtThoseNear()
    {
        QVector<CellRange> all;
        for (quint32 i = 0; i < 100000; ++i) all.append(range(i * 5, 1, i * 5 + 1, 2));
        MergeIndex m;
        m.assign(all);
        QCOMPARE(m.count(), 100000);
        const qint64 before = m.examined();
        QVERIFY(m.at(250000, 1) >= 0);
        QVERIFY(m.at(250003, 1) < 0);
        QVERIFY2(m.examined() - before < 2000, qPrintable(QString::number(m.examined() - before)));
        const qint64 before2 = m.examined();
        std::vector<int> found;
        QVERIFY(m.crossing(250000, 0, 250040, 5, 100, &found));
        QCOMPARE(int(found.size()), 9);
        QVERIFY2(m.examined() - before2 < 2000, qPrintable(QString::number(m.examined() - before2)));
        // the same hundred thousand all in one place (made so that every search would look at all of them) are looked at to a bound
        QVector<CellRange> stacked;
        for (quint32 i = 0; i < 100000; ++i) stacked.append(range(100, 0, 105 + i % 900, 1 + i / 900));
        MergeIndex s;
        s.assign(stacked);
        QVERIFY(s.count() > 90000);
        const qint64 before3 = s.examined();
        s.at(110, 50);
        QVERIFY2(s.examined() - before3 <= MergeIndex::kMaxScan, qPrintable(QString::number(s.examined() - before3)));
        const qint64 before4 = s.examined();
        std::vector<int> some;
        s.crossing(100, 0, 900, 199, 20, &some);
        QVERIFY(s.examined() - before4 <= MergeIndex::kMaxScan);
        QCOMPARE(int(some.size()), 20);
        // hundreds of tall merges: only the first kMaxTall are kept
        QVector<CellRange> tall;
        for (quint32 i = 0; i < 1000; ++i) tall.append(range(0, i % 100, 5000 + i, i % 100 + 1));
        MergeIndex t;
        t.assign(tall);
        QVERIFY(t.count() <= MergeIndex::kMaxTall + 100);
    }
};

class GridLayoutTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A point in the body is the cell under it, at the sizes the rows and columns have, hidden ones passed.
    void aPointInTheBodyIsTheCellUnderIt()
    {
        GridLayout g = plain();
        Hit h = g.hit(QPoint(40 + 10, 20 + 10));
        QVERIFY(h.kind == Hit::Kind::Cell && h.pane == Pane::Body);
        QCOMPARE(h.row, qint64(0));
        QCOMPARE(h.col, qint64(0));
        h = g.hit(QPoint(40 + 64, 20 + 20));
        QCOMPARE(h.row, qint64(1));
        QCOMPARE(h.col, qint64(1));
        h = g.hit(QPoint(40 + 63, 20 + 19));
        QCOMPARE(h.row, qint64(0));
        QCOMPARE(h.col, qint64(0));
        // row 0 is 30 high, row 1 is hidden, column 0 is 100 wide
        g.rows.setSize(0, 30);
        g.rows.setSize(1, 0);
        g.cols.setSize(0, 100);
        h = g.hit(QPoint(40 + 99, 20 + 29));
        QCOMPARE(h.row, qint64(0));
        QCOMPARE(h.col, qint64(0));
        h = g.hit(QPoint(40 + 100, 20 + 30));
        QCOMPARE(h.row, qint64(2));
        QCOMPARE(h.col, qint64(1));
        QCOMPARE(g.cellRect(2, 1), QRect(140, 50, 64, 20));
        QCOMPARE(g.cellRect(0, 0), QRect(40, 20, 100, 30));
        QVERIFY(g.cellRect(1, 0).isEmpty());   // (a hidden row has no rectangle)
    }

    // The headers and the corner, and the points past the last row and column, which are no cell.
    void headersTheCornerAndTheEdgesOfTheSheet()
    {
        GridLayout g = plain();
        Hit h = g.hit(QPoint(10, 5));
        QVERIFY(h.kind == Hit::Kind::Corner);
        h = g.hit(QPoint(40 + 64 * 3 + 2, 5));
        QVERIFY(h.kind == Hit::Kind::ColumnHeader);
        QCOMPARE(h.col, qint64(3));
        h = g.hit(QPoint(5, 20 + 20 * 7));
        QVERIFY(h.kind == Hit::Kind::RowHeader);
        QCOMPARE(h.row, qint64(7));
        // a sheet of 3 rows and 2 columns in a larger window
        g.rows.setCount(3);
        g.cols.setCount(2);
        QVERIFY(g.hit(QPoint(40 + 64 * 2, 40)).kind == Hit::Kind::None);
        QVERIFY(g.hit(QPoint(60, 20 + 60)).kind == Hit::Kind::None);
        QVERIFY(g.hit(QPoint(40 + 127, 20 + 59)).kind == Hit::Kind::Cell);
        QVERIFY(g.hit(QPoint(-3, 100)).kind == Hit::Kind::None);
        // no headers: the cells begin at the corner
        GridLayout bare = plain();
        bare.setHeaders(false, 40, 20);
        QCOMPARE(bare.hit(QPoint(0, 0)).kind, Hit::Kind::Cell);
        QCOMPARE(bare.cellRect(0, 0), QRect(0, 0, 64, 20));
    }

    // Scrolling and zoom move what is under a point: the grid's own units are scrolled, and the screen shows them zoomed, with no gap or overlap between cells.
    void scrollAndZoomMoveWhatIsUnderAPoint()
    {
        GridLayout g = plain();
        g.setScroll(0, 1000);
        QCOMPARE(g.scrollY(), qint64(1000));
        Hit h = g.hit(QPoint(41, 21));
        QCOMPARE(h.row, qint64(50));
        g.setScroll(640, 1000);
        QCOMPARE(g.hit(QPoint(41, 21)).col, qint64(10));
        // zoom 150%: rows of 30, columns of 96
        g.setZoom(1.5);
        QCOMPARE(g.cellRect(50, 10), QRect(40, 20, 96, 30));
        QCOMPARE(g.hit(QPoint(40 + 95, 20 + 29)).row, qint64(50));
        QCOMPARE(g.hit(QPoint(40 + 96, 20 + 30)).row, qint64(51));
        // zoom 75%: rows of 15 and columns of 48, and a size that does not divide leaves no gap
        g.setZoom(0.75);
        g.setScroll(0, 0);
        g.rows.setSize(0, 25);   // 18.75 on the screen
        for (qint64 r = 0; r < 40; ++r) {
            const QRect a = g.cellRect(r, 0), b = g.cellRect(r + 1, 0);
            QCOMPARE(a.bottom() + 1, b.top());
        }
        for (qint64 c = 0; c < 40; ++c) QCOMPARE(g.cellRect(0, c).right() + 1, g.cellRect(0, c + 1).left());
        // a zoom outside what the grid zooms to is brought in
        g.setZoom(100);
        QCOMPARE(g.zoom(), GridLayout::kMaxZoom);
        g.setZoom(0.0001);
        QCOMPARE(g.zoom(), GridLayout::kMinZoom);
    }

    // A merged range is hit as its first cell, from any cell of it, and its rectangle is the whole.
    void aMergedRangeIsHitAsOneCell()
    {
        GridLayout g = plain();
        g.merges.assign({range(1, 1, 2, 2)});
        Hit h = g.hit(QPoint(40 + 64 * 2 + 5, 20 + 20 * 2 + 5));   // the cell C3 (row 2, column 2)
        QVERIFY(h.kind == Hit::Kind::Cell);
        QCOMPARE(h.row, qint64(1));
        QCOMPARE(h.col, qint64(1));
        QVERIFY(h.merge >= 0);
        QCOMPARE(g.cellRect(1, 1), QRect(40 + 64, 20 + 20, 128, 40));
        QCOMPARE(g.cellRect(2, 2), QRect(40 + 64, 20 + 20, 128, 40));   // (any cell of it)
        QCOMPARE(g.rangeRect(1, 1, 2, 2), QRect(40 + 64, 20 + 20, 128, 40));
        h = g.hit(QPoint(40 + 64 * 3 + 5, 20 + 20 * 2 + 5));   // next to it
        QCOMPARE(h.col, qint64(3));
        QCOMPARE(h.merge, -1);
        // a merge with a hidden row inside is as tall as its shown rows
        g.rows.setSize(2, 0);
        QCOMPARE(g.cellRect(1, 1), QRect(40 + 64, 20 + 20, 128, 20));
    }

    // Frozen rows and columns stay where they are while the rest scrolls: each part of the window is the cells of its own rows and columns.
    void frozenRowsAndColumnsHoldTheirPlaceWhileTheRestScrolls()
    {
        GridLayout g = plain();
        g.setFrozen(2, 1);
        g.setScroll(640, 400);   // ten columns and twenty rows on
        // the corner: rows 0 and 1, column 0
        Hit h = g.hit(QPoint(40 + 10, 20 + 10));
        QCOMPARE(h.pane, Pane::Corner);
        QCOMPARE(h.row, qint64(0));
        QCOMPARE(h.col, qint64(0));
        // the top part: the frozen rows over the columns that scrolled (the first is 1 + 10)
        h = g.hit(QPoint(40 + 64 + 5, 20 + 10));
        QCOMPARE(h.pane, Pane::Top);
        QCOMPARE(h.row, qint64(0));
        QCOMPARE(h.col, qint64(11));
        // the left part: the frozen column beside the rows that scrolled (2 + 20)
        h = g.hit(QPoint(40 + 10, 20 + 40 + 5));
        QCOMPARE(h.pane, Pane::Left);
        QCOMPARE(h.row, qint64(22));
        QCOMPARE(h.col, qint64(0));
        // the body
        h = g.hit(QPoint(40 + 64 + 5, 20 + 40 + 5));
        QCOMPARE(h.pane, Pane::Body);
        QCOMPARE(h.row, qint64(22));
        QCOMPARE(h.col, qint64(11));
        // where cells are: a frozen row stays at the top, a frozen column at the left
        QCOMPARE(g.cellRect(0, 20).top(), 20);
        QCOMPARE(g.cellRect(30, 0).left(), 40);
        QCOMPARE(g.cellRect(22, 11), QRect(40 + 64, 20 + 40, 64, 20));
        // the headers of the scrolled rows and columns name those, and of the frozen ones the frozen
        QCOMPARE(g.hit(QPoint(40 + 64 + 5, 5)).col, qint64(11));
        QCOMPARE(g.hit(QPoint(40 + 5, 5)).col, qint64(0));
        QCOMPARE(g.hit(QPoint(5, 20 + 40 + 5)).row, qint64(22));
        QCOMPARE(g.hit(QPoint(5, 20 + 5)).row, qint64(0));
        // the four parts share the window with none left over or twice over
        const PaneView c = g.pane(Pane::Corner), t = g.pane(Pane::Top), l = g.pane(Pane::Left), b = g.pane(Pane::Body);
        QCOMPARE(c.rect, QRect(40, 20, 64, 40));
        QCOMPARE(t.rect, QRect(40 + 64, 20, 640 - 64, 40));
        QCOMPARE(l.rect, QRect(40, 20 + 40, 64, 400 - 40));
        QCOMPARE(b.rect, QRect(40 + 64, 20 + 40, 640 - 64, 400 - 40));
        QCOMPARE(c.firstRow, qint64(0));
        QCOMPARE(c.lastRow, qint64(1));
        QCOMPARE(t.firstCol, qint64(11));
        QCOMPARE(l.firstRow, qint64(22));
        QCOMPARE(b.firstRow, qint64(22));
        QCOMPARE(b.firstCol, qint64(11));
        QCOMPARE(b.lastRow, qint64(22 + 17));    // 360 pixels of 20 (the last whole)
        QCOMPARE(b.lastCol, qint64(11 + 8));     // 576 pixels of 64: nine columns
    }

    // A merged range that is across the edge of the frozen part is hit as itself from either side of the edge.
    void aMergeAcrossTheFrozenEdgeIsHitAsItselfFromBothSides()
    {
        GridLayout g = plain();
        g.merges.assign({range(0, 0, 0, 3)});
        g.setFrozen(0, 1);
        g.setScroll(0, 0);
        const Hit left = g.hit(QPoint(40 + 10, 20 + 5));
        const Hit right = g.hit(QPoint(40 + 64 * 2 + 5, 20 + 5));
        QCOMPARE(left.row, qint64(0));
        QCOMPARE(left.col, qint64(0));
        QCOMPARE(right.col, qint64(0));
        QVERIFY(left.merge >= 0 && left.merge == right.merge);
        // scrolled so that the merge's first columns are gone: still the merge, still its first cell
        g.setScroll(64 * 2, 0);
        const Hit scrolled = g.hit(QPoint(40 + 64 + 5, 20 + 5));
        QCOMPARE(scrolled.merge, left.merge);
        QCOMPARE(scrolled.col, qint64(0));
    }

    // A merged range across the edge of the frozen part is drawn by each part as its own coordinates put it; the edges of rows and columns are the same either way up to it.
    void aRangeIsPlacedAsEachPartOfTheGridDrawsIt()
    {
        GridLayout g = plain();
        g.setFrozen(0, 1);
        g.setScroll(128, 0);
        // columns 0 to 3 from the frozen part: from its left to four columns on; from the part that scrolls: the same four, 192 to the left of where it begins
        const QRect inFrozen = g.rangeRectIn(Pane::Left, 0, 0, 0, 3);
        QCOMPARE(inFrozen, QRect(40, 20, 256, 20));
        const QRect inScrolling = g.rangeRectIn(Pane::Body, 0, 0, 0, 3);
        QCOMPARE(inScrolling, QRect(40 + 64 - 192, 20, 256, 20));
        QCOMPARE(g.rangeRectIn(Pane::Corner, 0, 0, 0, 3), g.rangeRectIn(Pane::Left, 0, 0, 0, 3));
        QCOMPARE(g.rangeRectIn(Pane::Top, 0, 0, 0, 3), g.rangeRectIn(Pane::Body, 0, 0, 0, 3));
        // edges: the end of a column is the start of the next, and a hidden one between has no width
        QCOMPARE(g.xEdge(1, true), 40 + 64);
        QCOMPARE(g.xEdge(1, false), 40 + 64 + (64 - 192));
        g.cols.setSize(5, 0);
        QCOMPARE(g.xEdge(5, false), g.xEdge(6, false));
        QCOMPARE(g.yEdge(3, false), 20 + 60);
        // rows
        g.setFrozen(2, 0);
        g.setScroll(0, 100);
        QCOMPARE(g.rangeRectIn(Pane::Top, 0, 0, 3, 0).height(), 80);
        QCOMPARE(g.rangeRectIn(Pane::Body, 0, 0, 3, 0), QRect(40, 20 + 40 + (0 - 140), 64, 80));
        QCOMPARE(g.yEdge(2, true), 20 + 40);
    }

    // A file that holds half a million rows in place (more than any window shows) costs the rows the window shows: the parts are worked out with a few steps, not by walking the frozen rows.
    void aHugeFrozenPartIsWorkedOutInAFewSteps()
    {
        GridLayout g;
        g.setHeaders(true, 40, 20);
        g.setViewport(QSize(680, 420));
        g.setFrozen(500000, 3);
        const qint64 before = g.rows.probes();
        const PaneView top = g.pane(Pane::Top), corner = g.pane(Pane::Corner), body = g.pane(Pane::Body);
        QVERIFY2(g.rows.probes() - before < 400, qPrintable(QString::number(g.rows.probes() - before)));
        QCOMPARE(top.firstRow, qint64(0));
        QCOMPARE(top.lastRow, qint64(19));                        // (400 pixels of 20: the window is all frozen rows)
        QCOMPARE(corner.lastRow, qint64(19));
        QVERIFY(body.isEmpty() || body.rect.height() <= 0);       // (nothing is left for the part that scrolls)
        QCOMPARE(g.hit(QPoint(40 + 10, 20 + 25)).row, qint64(1));
        // the same with a hundred thousand of them hidden at the start
        for (qint64 r = 0; r < 100000; ++r) g.rows.setSize(r, 0);
        const qint64 before2 = g.rows.probes();
        const PaneView hidden = g.pane(Pane::Top);
        QVERIFY2(g.rows.probes() - before2 < 400, qPrintable(QString::number(g.rows.probes() - before2)));
        QCOMPARE(hidden.firstRow, qint64(100000));
        QCOMPARE(hidden.lastRow, qint64(100019));
    }

    // The scroll is kept where the last row can be the first, and a cell is scrolled into view by the least that shows it.
    void scrollIsKeptInRangeAndACellIsBroughtIntoView()
    {
        GridLayout g = plain();
        g.setScroll(-50, -50);
        QCOMPARE(g.scrollX(), qint64(0));
        QCOMPARE(g.scrollY(), qint64(0));
        g.setScroll(1000000, 1000000);
        QCOMPARE(g.scrollY(), g.maxScrollY());
        QCOMPARE(g.scrollX(), g.maxScrollX());
        QCOMPARE(g.maxScrollY(), qint64(999 * 20));    // the last row at the top
        QCOMPARE(g.maxScrollX(), qint64(99 * 64));
        g.setScroll(0, 0);
        QVERIFY(!g.ensureVisible(3, 3));
        QVERIFY(g.ensureVisible(100, 0));      // row 100 is below the 20 rows the body shows
        QCOMPARE(g.cellRect(100, 0).bottom() + 1, 20 + 400);
        QVERIFY(g.ensureVisible(2, 0));        // above the part now shown: its top is the top
        QCOMPARE(g.cellRect(2, 0).top(), 20);
        QVERIFY(g.ensureVisible(2, 50));
        QCOMPARE(g.cellRect(2, 50).right() + 1, 40 + 640);
        // frozen cells are always in view
        g.setFrozen(3, 3);
        g.setScroll(1000, 1000);
        QVERIFY(!g.ensureVisible(1, 1));
        // a column wider than the body has its start in view
        g.setFrozen(0, 0);
        g.cols.setSize(60, 2000);
        QVERIFY(g.ensureVisible(0, 60));
        QCOMPARE(g.cellRect(0, 60).left(), 40);
    }

    // The block of rows and columns the body shows, hidden rows left out, whatever the zoom; a sheet of a million rows scrolled to its end shows what is left.
    void thePartsShowTheRowsAndColumnsInView()
    {
        GridLayout g = plain();
        g.rows.setSize(1, 0);
        g.rows.setSize(2, 0);
        PaneView b = g.pane(Pane::Body);
        QCOMPARE(b.firstRow, qint64(0));
        QCOMPARE(b.lastRow, qint64(21));    // 20 rows of 20 fill 400 pixels; two hidden ones are not rows
        QCOMPARE(b.firstCol, qint64(0));
        QCOMPARE(b.lastCol, qint64(9));
        g.setZoom(0.5);
        b = g.pane(Pane::Body);
        QCOMPARE(b.lastRow, qint64(39 + 2));
        QCOMPARE(b.lastCol, qint64(19));
        // a million rows scrolled to the end
        GridLayout big;
        big.setHeaders(true, 40, 20);
        big.setViewport(QSize(680, 420));
        big.setScroll(0, 1048576LL * 20);
        b = big.pane(Pane::Body);
        QCOMPARE(b.firstRow, qint64(1048575));
        QCOMPARE(b.lastRow, qint64(1048575));
        // a window too small to hold the frozen part leaves the body empty
        GridLayout small = plain();
        small.setFrozen(100, 0);
        QVERIFY(small.pane(Pane::Body).isEmpty() || small.pane(Pane::Body).rect.height() <= 0);
        QCOMPARE(small.pane(Pane::Top).rect.height(), 400);
    }
};

JP_TEST_CLASS(GridMergeTests)
JP_TEST_CLASS(GridLayoutTests)
#include "tst_grid_layout.moc"
