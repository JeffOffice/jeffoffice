// Tests of the grid's rows and columns (libs/shell/grid/axis.h): where each begins and which holds a place, with sizes of their own and hidden ones, at a million rows.
#include <QtTest>

#include "grid/axis.h"

#include <cstdlib>

#include "../testrunner.h"

using jp::grid::Axis;

class GridAxisTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // With no sizes of their own, row n begins at n times the default.
    void rowsOfOneSizeBeginWhereTheirNumberSays()
    {
        Axis a(1000, 20);
        QCOMPARE(a.position(0), qint64(0));
        QCOMPARE(a.position(7), qint64(140));
        QCOMPARE(a.total(), qint64(20000));
        QCOMPARE(a.size(999), 20);
        QCOMPARE(a.indexAt(0), qint64(0));
        QCOMPARE(a.indexAt(19), qint64(0));
        QCOMPARE(a.indexAt(20), qint64(1));
        QCOMPARE(a.indexAt(19999), qint64(999));
        QCOMPARE(a.indexAt(20000), qint64(-1));
        QCOMPARE(a.indexAt(-1), qint64(-1));
        QCOMPARE(a.nextVisible(5), qint64(6));
        QCOMPARE(a.nextVisible(999), qint64(1000));
        QCOMPARE(a.previousVisible(0), qint64(-1));
        QCOMPARE(a.firstVisible(), qint64(0));
        QCOMPARE(a.lastVisible(), qint64(999));
    }

    // A row of its own size moves every row after it; a hidden row has no size, and the rows after it begin where it did.
    void sizesOfTheirOwnAndHiddenRowsMoveTheRowsAfterThem()
    {
        Axis a(100, 20);
        a.setSize(3, 30);
        a.setSize(5, 0);
        a.setSize(6, 0);
        a.setSize(7, 0);
        a.setSize(10, 25);
        QCOMPARE(a.position(3), qint64(60));
        QCOMPARE(a.position(4), qint64(90));
        QCOMPARE(a.position(5), qint64(110));
        QCOMPARE(a.position(8), qint64(110));
        QCOMPARE(a.position(9), qint64(130));
        QCOMPARE(a.position(10), qint64(150));
        QCOMPARE(a.position(11), qint64(175));
        // 100 rows of 20, 10 more for row 3, 3 hidden, 5 more for row 10
        QCOMPARE(a.total(), qint64(2000 + 10 - 60 + 5));
        QCOMPARE(a.size(3), 30);
        QCOMPARE(a.size(6), 0);
        QVERIFY(a.isHidden(6) && !a.isHidden(4));
        // the row at a place is the one that holds it, and never a hidden one
        QCOMPARE(a.indexAt(60), qint64(3));
        QCOMPARE(a.indexAt(89), qint64(3));
        QCOMPARE(a.indexAt(90), qint64(4));
        QCOMPARE(a.indexAt(109), qint64(4));
        QCOMPARE(a.indexAt(110), qint64(8));
        QCOMPARE(a.indexAt(149), qint64(9));
        QCOMPARE(a.indexAt(150), qint64(10));
        QCOMPARE(a.indexAt(174), qint64(10));
        QCOMPARE(a.indexAt(175), qint64(11));
        QCOMPARE(a.nextVisible(4), qint64(8));
        QCOMPARE(a.nextVisible(2), qint64(3));
        QCOMPARE(a.previousVisible(8), qint64(4));
        QCOMPARE(a.previousVisible(5), qint64(4));
        // every row that is shown is found at each place it covers, and at no place of another
        for (qint64 i = 0; i < 100; ++i) {
            if (a.isHidden(i)) continue;
            QCOMPARE(a.indexAt(a.position(i)), i);
            QCOMPARE(a.indexAt(a.position(i) + a.size(i) - 1), i);
        }
        // a size of the default is no size of its own
        a.setSize(3, 20);
        QCOMPARE(a.position(4), qint64(80));
        QCOMPARE(a.placeCount(), 4);
        // and a row hidden or sized past the most is brought in
        a.setSize(50, 1000000);
        QCOMPARE(a.size(50), Axis::kMaxSize);
        a.setSize(51, -4);
        QVERIFY(a.isHidden(51));
    }

    // Rows hidden at the first and the last place of the sheet leave the first and last shown rows where they are.
    void hiddenRowsAtTheEdgesAreSteppedOver()
    {
        Axis a(10, 10);
        a.setSize(0, 0);
        a.setSize(1, 0);
        a.setSize(9, 0);
        QCOMPARE(a.firstVisible(), qint64(2));
        QCOMPARE(a.lastVisible(), qint64(8));
        QCOMPARE(a.position(2), qint64(0));
        QCOMPARE(a.indexAt(0), qint64(2));
        QCOMPARE(a.total(), qint64(70));
        QCOMPARE(a.indexAt(69), qint64(8));
        QCOMPARE(a.indexAt(70), qint64(-1));
        Axis none(5, 10);
        for (int i = 0; i < 5; ++i) none.setSize(i, 0);
        QCOMPARE(none.total(), qint64(0));
        QCOMPARE(none.indexAt(0), qint64(-1));
        QCOMPARE(none.firstVisible(), qint64(5));
        QCOMPARE(none.lastVisible(), qint64(-1));
    }

    // A million rows with a hundred thousand of their own: a lookup takes about as many steps as the logarithm of that, and a run of a hundred thousand hidden rows is
    // stepped over in one.
    void aLookupCostsTheLogarithmOfTheDifferingRowsAtAMillionRows()
    {
        const qint64 count = 1048576;
        std::vector<Axis::Place> places;
        for (qint64 i = 0; i < count; i += 10)
            if (i < 500001 || i >= 600001) places.push_back({i, int(10 + (i / 10) % 50)});
        // a hidden run of a hundred thousand rows
        for (qint64 i = 500001; i < 600001; ++i) places.push_back({i, 0});
        std::sort(places.begin(), places.end(), [](const Axis::Place &x, const Axis::Place &y) { return x.index < y.index; });
        places.erase(std::unique(places.begin(), places.end(), [](const Axis::Place &x, const Axis::Place &y) { return x.index == y.index; }), places.end());
        Axis a(count, 20);
        a.assign(places);
        QVERIFY(a.placeCount() > 100000);
        // (the first lookup builds the sums)
        a.position(1000);
        const qint64 before = a.probes();
        const qint64 p = a.position(1000000);
        QVERIFY2(a.probes() - before <= 40, qPrintable(QString::number(a.probes() - before)));
        const qint64 before2 = a.probes();
        QCOMPARE(a.indexAt(p), qint64(1000000));
        QVERIFY2(a.probes() - before2 <= 40, qPrintable(QString::number(a.probes() - before2)));
        const qint64 before3 = a.probes();
        QCOMPARE(a.nextVisible(500000), qint64(600001));
        QVERIFY2(a.probes() - before3 <= 40, qPrintable(QString::number(a.probes() - before3)));
        // the answers agree with adding the rows up one by one, at a few hundred rows
        qint64 sum = 0, row = 0;
        for (qint64 target : {0, 1, 17, 999, 5000, 123456, 500000, 500001, 600000, 600001, 1000000, 1048575}) {
            while (row < target) sum += a.size(row++);
            QCOMPARE(a.position(target), sum);
        }
        QCOMPARE(a.total(), a.position(count));
    }

    // A run of half a million hidden rows at the end of the sheet is stepped over backward in one step too: the last shown row (which every scroll asks for) costs a few steps.
    void aHiddenRunAtTheEndIsSteppedOverBackwardInOneStep()
    {
        const qint64 count = 1048576;
        Axis a(count, 20);
        std::vector<Axis::Place> places;
        for (qint64 i = 500000; i < count; ++i) places.push_back({i, 0});
        a.assign(places);
        a.total();
        const qint64 before = a.probes();
        QCOMPARE(a.lastVisible(), qint64(499999));
        QVERIFY2(a.probes() - before < 50, qPrintable(QString::number(a.probes() - before)));
        const qint64 before2 = a.probes();
        QCOMPARE(a.previousVisible(count - 10), qint64(499999));
        QCOMPARE(a.previousVisible(500000), qint64(499999));
        QCOMPARE(a.previousVisible(499999), qint64(499998));
        QVERIFY2(a.probes() - before2 < 100, qPrintable(QString::number(a.probes() - before2)));
        // and a run that begins at the first row
        Axis b(1000, 20);
        std::vector<Axis::Place> front;
        for (qint64 i = 0; i < 700; ++i) front.push_back({i, 0});
        b.assign(front);
        QCOMPARE(b.previousVisible(700), qint64(-1));
        QCOMPARE(b.previousVisible(701), qint64(700));
        QCOMPARE(b.firstVisible(), qint64(700));
    }

    // Edits and changes of the default keep every answer right.
    void editsAndANewDefaultKeepTheAnswersRight()
    {
        Axis a(50, 20);
        a.setSize(10, 40);
        QCOMPARE(a.position(11), qint64(240));
        a.setDefaultSize(30);
        QCOMPARE(a.position(11), qint64(10 * 30 + 40));
        QCOMPARE(a.indexAt(339), qint64(10));
        QCOMPARE(a.indexAt(340), qint64(11));
        a.setSize(10, 30);   // as the default now
        QCOMPARE(a.placeCount(), 0);
        a.setCount(20);
        QCOMPARE(a.total(), qint64(600));
        a.setSize(25, 5);    // past the count: no row is changed
        QCOMPARE(a.total(), qint64(600));
        // random sizes agree with a sum
        std::srand(7);
        Axis b(300, 17);
        std::vector<int> sizes(300, 17);
        for (int k = 0; k < 200; ++k) {
            const int i = std::rand() % 300, s = std::rand() % 4 == 0 ? 0 : std::rand() % 60;
            b.setSize(i, s);
            sizes[size_t(i)] = s;
        }
        qint64 sum = 0;
        for (int i = 0; i < 300; ++i) {
            QCOMPARE(b.position(i), sum);
            QCOMPARE(b.size(i), sizes[size_t(i)]);
            sum += sizes[size_t(i)];
        }
        QCOMPARE(b.total(), sum);
        for (qint64 pos = 0; pos < sum; pos += 3) {
            const qint64 i = b.indexAt(pos);
            QVERIFY(i >= 0 && !b.isHidden(i) && b.position(i) <= pos && pos < b.position(i) + b.size(i));
        }
    }

    // A copy of an axis is the same sizes without a copy of the list of them (a million rows with their own sizes are forty megabytes), until one of the two is changed.
    void aCopyOfAnAxisSharesItsPlacesUntilItChanges()
    {
        Axis a(1048576, 20);
        std::vector<Axis::Place> places;
        for (qint64 i = 0; i < 1048576; i += 2) places.push_back({i, 33});
        a.assign(places);
        a.total();
        Axis b = a;
        QVERIFY(b.sharesStorageWith(a));
        QCOMPARE(b.position(1000), a.position(1000));
        b.setSize(10, 50);
        QVERIFY(!b.sharesStorageWith(a));
        QCOMPARE(a.size(10), 33);
        QCOMPARE(b.size(10), 50);
        QCOMPARE(b.position(11), a.position(11) + 17);
        // a copy of an axis that was changed and not yet asked anything is the same as the axis
        Axis c(1000, 20);
        c.setSize(5, 7);
        Axis d = c;
        QCOMPARE(d.position(6), c.position(6));
        QCOMPARE(d.position(6), qint64(5 * 20 + 7));
    }

    // The first rows can be skipped (a view scrolled down and then frozen has the rows it scrolled past out of sight): they have no size, no search finds them, and the rows after them
    // begin where the first of them would have.
    void aSkippedPrefixIsNotThereToSearchForOrWalkOver()
    {
        Axis a(100, 20);
        a.setSize(12, 0);
        a.setSize(30, 50);
        a.setSkipped(10);
        QCOMPARE(a.skipped(), qint64(10));
        QCOMPARE(a.size(5), 0);
        QVERIFY(a.isHidden(9));
        QCOMPARE(a.position(10), qint64(0));
        QCOMPARE(a.position(11), qint64(20));
        QCOMPARE(a.position(13), qint64(40));   // (12 is hidden)
        QCOMPARE(a.position(31), a.position(30) + 50);
        QCOMPARE(a.total(), a.position(100));
        QCOMPARE(a.indexAt(0), qint64(10));
        QCOMPARE(a.indexAt(20), qint64(11));
        QCOMPARE(a.indexAt(40), qint64(13));
        QCOMPARE(a.indexAt(a.total() - 1), qint64(99));
        QCOMPARE(a.indexAt(a.total()), qint64(-1));
        QCOMPARE(a.nextVisible(-1), qint64(10));
        QCOMPARE(a.nextVisible(3), qint64(10));
        QCOMPARE(a.firstVisible(), qint64(10));
        QCOMPARE(a.previousVisible(10), qint64(-1));
        QCOMPARE(a.previousVisible(11), qint64(10));
        QCOMPARE(a.previousVisible(13), qint64(11));
        QCOMPARE(a.previousVisible(14), qint64(13));
        QCOMPARE(a.lastVisible(), qint64(99));
        // skipping up to and over a hidden row: the first shown is after it
        a.setSkipped(12);
        QCOMPARE(a.firstVisible(), qint64(13));
        QCOMPARE(a.previousVisible(13), qint64(-1));
        // none skipped: as it was
        a.setSkipped(0);
        QCOMPARE(a.position(11), qint64(220));
        QCOMPARE(a.firstVisible(), qint64(0));
        // a copy keeps what is skipped
        a.setSkipped(5);
        Axis b = a;
        QCOMPARE(b.skipped(), qint64(5));
        QCOMPARE(b.position(6), qint64(20));
    }
};

JP_TEST_CLASS(GridAxisTests)
#include "tst_grid_axis.moc"
