// Tests of the cells a worksheet is read into: the store, every type of cell, the shared strings, and the shape the part declares.
#include "sheethelpers.h"

#include <random>

namespace {

PlacedCell makeCell(quint32 row, quint32 col, double number = 0)
{
    PlacedCell c;
    c.row = row;
    c.col = quint16(col);
    c.cell.type = CellType::Number;
    c.cell.number = number;
    return c;
}

SharedStrings stringsOf(const QByteArray &xml, QString *error = nullptr)
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    SharedStrings out;
    QString why;
    const bool ok = readSharedStrings(&buffer, Limits(), &out, &why);
    if (error) *error = why;
    else if (!ok) qWarning("shared strings: %s", qPrintable(why));
    return out;
}

} // namespace

class SheetCellTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- addresses

    void cellReferencesAreReadAndWritten()
    {
        quint32 row = 99, col = 99;
        QVERIFY(parseCellRef(QStringLiteral("A1"), &row, &col));
        QCOMPARE(row, 0u);
        QCOMPARE(col, 0u);
        QVERIFY(parseCellRef(QStringLiteral("$b$3"), &row, &col));
        QCOMPARE(row, 2u);
        QCOMPARE(col, 1u);
        QVERIFY(parseCellRef(QStringLiteral("XFD1048576"), &row, &col));
        QCOMPARE(row, 1048575u);
        QCOMPARE(col, 16383u);
        for (const char *bad : {"", "A", "1", "A0", "XFE1", "A1048577", "AAAA1", "A1B", "A-1", "A99999999999999999999", " A1"})
            QVERIFY2(!parseCellRef(QString::fromLatin1(bad), &row, &col), bad);
        QCOMPARE(columnName(0), QStringLiteral("A"));
        QCOMPARE(columnName(25), QStringLiteral("Z"));
        QCOMPARE(columnName(26), QStringLiteral("AA"));
        QCOMPARE(columnName(701), QStringLiteral("ZZ"));
        QCOMPARE(columnName(702), QStringLiteral("AAA"));
        QCOMPARE(columnName(16383), QStringLiteral("XFD"));
        QVERIFY(columnName(16384).isEmpty());
        QCOMPARE(cellName(0, 0), QStringLiteral("A1"));
        QCOMPARE(cellName(1048575, 16383), QStringLiteral("XFD1048576"));
        CellRange r;
        QVERIFY(parseRangeRef(QStringLiteral("B2:D5"), &r));
        QVERIFY(r.valid && r.row1 == 1 && r.col1 == 1 && r.row2 == 4 && r.col2 == 3);
        QCOMPARE(r.cellCount(), qint64(12));
        QVERIFY(parseRangeRef(QStringLiteral("D5:B2"), &r));
        QVERIFY(r.row1 == 1 && r.col1 == 1 && r.row2 == 4 && r.col2 == 3);
        QVERIFY(parseRangeRef(QStringLiteral("C7"), &r));
        QVERIFY(r.valid && r.row1 == r.row2 && r.col1 == r.col2 && r.row1 == 6 && r.col1 == 2);
        QCOMPARE(rangeName(r), QStringLiteral("C7"));
        for (const char *bad : {"", "A1:", ":A1", "A1:B", "A1:B2:C3", "A1:XFE1"}) {
            QVERIFY2(!parseRangeRef(QString::fromLatin1(bad), &r), bad);
            QVERIFY(!r.valid);
        }
    }

    // ---- the store

    // Cells added in any order are in order of row and then column once finished; one added twice for a place is there once, the later one.
    void theStoreKeepsCellsInOrderAndNoneTwice()
    {
        CellStore store;
        store.add(makeCell(200, 3, 1));
        store.add(makeCell(0, 5, 2));
        store.add(makeCell(0, 1, 3));
        store.add(makeCell(63, 0, 4));
        store.add(makeCell(64, 0, 5));
        store.add(makeCell(0, 1, 6));   // a second cell for A2... the place (0, 1)
        store.add(makeCell(1048575, 16383, 7));
        QCOMPARE(store.cellCount(), qint64(7));
        store.finish();
        QVector<QPair<quint32, quint32>> seen;
        QVector<double> numbers;
        store.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            seen.append({cellRow, cellCol});
            numbers.append(c.number);
        });
        const QVector<QPair<quint32, quint32>> expected = {{0, 1}, {0, 5}, {63, 0}, {64, 0}, {200, 3}, {1048575, 16383}};
        QCOMPARE(seen, expected);
        QCOMPARE(numbers, (QVector<double>{6, 2, 4, 5, 1, 7}));
        QCOMPARE(store.cellCount(), qint64(6));
        QCOMPARE(store.blockCount(), 4);
        const Cell *found = store.find(0, 1);
        QVERIFY(found);
        QCOMPARE(found->number, 6.0);
        QVERIFY(store.find(200, 3));
        QVERIFY(!store.find(200, 4));
        QVERIFY(!store.find(2, 2));
        QVERIFY(!store.find(5000, 0));
        QVERIFY(store.find(1048575, 16383));
        QVERIFY(store.memoryBytes() >= 6 * 24);
        // Cells that come in order need no sorting, and a copy is a store of its own.
        CellStore ordered;
        for (quint32 r = 0; r < 1000; ++r)
            for (quint32 c = 0; c < 3; ++c) ordered.add(makeCell(r, c, r * 3 + c));
        ordered.finish();
        CellStore copy = ordered;
        copy.add(makeCell(5000, 0, 1));
        copy.finish();
        QCOMPARE(ordered.cellCount(), qint64(3000));
        QCOMPARE(copy.cellCount(), qint64(3001));
        QVERIFY(!ordered.find(5000, 0));
        QCOMPARE(ordered.find(999, 2)->number, 2999.0);
    }

    // A rectangle of a sheet is visited cell by cell in order, and only its own cells: the same as asking every cell whether it is inside.
    void aRectangleOfTheStoreVisitsExactlyItsCells()
    {
        std::mt19937 rng(20261023);
        CellStore store;
        for (int i = 0; i < 6000; ++i) store.add(makeCell(rng() % 400, rng() % 40, i));
        // a few whole rows, so that rows with many cells meet rows with few
        for (quint32 col = 0; col < 40; ++col) store.add(makeCell(100, col, 1));
        store.finish();
        for (int trial = 0; trial < 3000; ++trial) {
            quint32 r1 = rng() % 420, r2 = rng() % 420, c1 = rng() % 45, c2 = rng() % 45;
            if (r1 > r2) std::swap(r1, r2);
            if (c1 > c2) std::swap(c1, c2);
            QVector<QPair<quint32, quint32>> expected, got;
            store.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
                if (cellRow >= r1 && cellRow <= r2 && cellCol >= c1 && cellCol <= c2) expected.append({cellRow, cellCol});
            });
            store.forEachInRange(r1, c1, r2, c2, [&](quint32 cellRow, quint32 cellCol, const Cell &c) {
                got.append({cellRow, cellCol});
                return true;
            });
            QVERIFY2(got == expected, qPrintable(QStringLiteral("rows %1-%2, columns %3-%4: %5 cells, expected %6").arg(r1).arg(r2).arg(c1).arg(c2).arg(got.size()).arg(expected.size())));
        }
        // and it stops when asked
        int seen = 0;
        store.forEachInRange(0, 0, 399, 39, [&](quint32, quint32, const Cell &) { return ++seen < 5; });
        QCOMPARE(seen, 5);
    }

    // A narrow range of a wide sheet costs what it holds, not what the rows hold: column A of rows that have 16,384 cells each steps over the cells of the column and no
    // others, and finds the first cell of each row by halving. What is counted is what the walk did (see CellStore::RangeWalk), not a count kept apart from it.
    void aNarrowRangeOfAWideSheetDoesNotStepOverTheRestOfEachRow()
    {
        CellStore store;
        for (quint32 r = 0; r < 64; ++r)
            for (quint32 c = 0; c < 16384; ++c) store.add(makeCell(r, c, r * 16384 + c));
        store.finish();
        CellStore::RangeWalk walk;
        int seen = 0;
        double last = -1;
        store.forEachInRange(0, 0, 63, 0, [&](quint32, quint32, const Cell &c) {
            ++seen;
            last = c.number;
            return true;
        }, &walk);
        QCOMPARE(seen, 64);
        QCOMPARE(last, 63.0 * 16384);
        const QString said = QStringLiteral("stepped over %1 cells and made %2 comparisons for 64 cells").arg(walk.stepped).arg(walk.compared);
        QVERIFY2(walk.stepped == 64 && walk.compared <= 64 * 22, qPrintable(said));
        // a column in the middle, and a block of columns
        CellStore::RangeWalk middle;
        seen = 0;
        store.forEachInRange(0, 8000, 63, 8003, [&](quint32, quint32, const Cell &) { return ++seen > 0; }, &middle);
        QCOMPARE(seen, 64 * 4);
        const QString saidMiddle = QStringLiteral("stepped over %1 cells and made %2 comparisons for 256 cells").arg(middle.stepped).arg(middle.compared);
        QVERIFY2(middle.stepped == 64 * 4 && middle.compared <= 64 * 22, qPrintable(saidMiddle));
        // the last column, and a range that holds nothing
        CellStore::RangeWalk edge, none;
        seen = 0;
        store.forEachInRange(0, 16383, 63, 16383, [&](quint32, quint32, const Cell &) { return ++seen > 0; }, &edge);
        QCOMPARE(seen, 64);
        QVERIFY2(edge.stepped == 64 && edge.compared <= 64 * 22, qPrintable(QStringLiteral("%1 stepped, %2 compared").arg(edge.stepped).arg(edge.compared)));
        store.forEachInRange(100, 0, 163, 5, [&](quint32, quint32, const Cell &) { return true; }, &none);
        QVERIFY2(none.stepped == 0 && none.compared == 0, qPrintable(QStringLiteral("%1 stepped, %2 compared").arg(none.stepped).arg(none.compared)));
    }

    // Cells put in and taken out of a finished store, in any order, leave it what a map of places to cells would be: find, the walk over a rectangle, and the count agree.
    void cellsSetAndErasedKeepTheStoreInOrder()
    {
        std::mt19937 rng(20261024);
        CellStore store;
        std::map<std::pair<quint32, quint32>, double> want;
        for (int i = 0; i < 3000; ++i) {
            const quint32 r = rng() % 300, c = rng() % 30;
            store.add(makeCell(r, c, i));
            want[{r, c}] = i;
        }
        store.finish();
        for (int i = 0; i < 6000; ++i) {
            const quint32 r = rng() % 340, c = rng() % 34;
            if (rng() % 3 == 0) {
                QCOMPARE(store.erase(r, c), want.erase({r, c}) == 1);
            } else {
                const Cell *put = store.set(makeCell(r, c, 100000 + i));
                QVERIFY(put && put == store.find(r, c) && put->number == 100000 + i);
                want[{r, c}] = 100000 + i;
            }
        }
        QCOMPARE(store.cellCount(), qint64(want.size()));
        for (quint32 r = 0; r < 340; ++r)
            for (quint32 c = 0; c < 34; ++c) {
                const Cell *found = store.find(r, c);
                const auto it = want.find({r, c});
                QVERIFY2((found != nullptr) == (it != want.end()), qPrintable(QStringLiteral("%1,%2").arg(r).arg(c)));
                if (found) QCOMPARE(found->number, it->second);
            }
        // the walk over the store is in order and holds every cell once
        std::vector<std::pair<quint32, quint32>> walked;
        store.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &cell) { walked.push_back({cellRow, cellCol}); });
        QVERIFY(std::is_sorted(walked.begin(), walked.end()));
        QCOMPARE(qint64(walked.size()), qint64(want.size()));
        // a rectangle of it
        QVector<QPair<quint32, quint32>> got, expected;
        store.forEachInRange(50, 3, 250, 20, [&](quint32 cellRow, quint32 cellCol, const Cell &cell) {
            got.append({cellRow, cellCol});
            return true;
        });
        for (const auto &entry : want)
            if (entry.first.first >= 50 && entry.first.first <= 250 && entry.first.second >= 3 && entry.first.second <= 20) expected.append({entry.first.first, entry.first.second});
        QVERIFY(got == expected);
        // a cell is changed where it is
        Cell *edit = store.findMutable(want.begin()->first.first, want.begin()->first.second);
        QVERIFY(edit);
        edit->number = -5;
        QCOMPARE(store.find(want.begin()->first.first, want.begin()->first.second)->number, -5.0);
        QVERIFY(!store.findMutable(100000, 3));
        QVERIFY(!store.erase(100000, 3));
    }

    void rowsAndColumnsEditedMoveNoCellAndTheWalksGiveTheirPlaces()
    {
        lineEditsAgainstAModel(120, 24, 1500, 220, 20261027, 0, 0);
    }

    // The same with a sheet of thousands of rows and columns of hundreds, so that the orders have many runs (the walks, ranges, and slices go along many), and at the end of the sheet, where
    // inserts push cells off it and deletes are cut to it.
    void manyRunsOfRowsAndColumnsAreWalkedInOrderOfPlace()
    {
        lineEditsAgainstAModel(3000, 200, 9000, 700, 20261028, 0, 0);
    }

    void linesAtTheEndOfTheSheetPushCellsOffAndUndoBringsThemBack()
    {
        lineEditsAgainstAModel(40, 12, 300, 260, 20261029, 1048576 - 40, 16384 - 12);
    }

    // A walk across five or more runs of rows (and of columns) visits the cells in the order of their places, and a row that is deleted is never reached, though its cells are held until
    // Undo (the count leaves them out).
    void aWalkAcrossManyRunsVisitsCellsInOrderOfPlace()
    {
        CellStore store;
        for (quint32 r = 0; r < 40; ++r)
            for (quint32 c = 0; c < 6; ++c) store.add(makeCell(r, c, r * 100 + c));
        store.finish();
        QCOMPARE(store.cellCount(), qint64(240));
        // the cells are known by their numbers: a row's number is 100 times the row it was
        auto rowOf = [](const Cell &c) { return int(c.number) / 100; };
        auto colOf = [](const Cell &c) { return int(c.number) % 100; };
        CellStore::LineEdits e1, e2, e3, e4, e5;
        QCOMPARE(store.insertLines(true, 30, 2, &e1), jp::calc::LineStatus::Ok);    // (two blank rows at 30, 31)
        QCOMPARE(store.deleteLines(true, 20, 3, &e2), jp::calc::LineStatus::Ok);    // (rows 20 to 22 gone)
        QCOMPARE(store.insertLines(false, 2, 1, &e3), jp::calc::LineStatus::Ok);    // (a blank column at 2)
        QCOMPARE(store.deleteLines(true, 5, 1, &e4), jp::calc::LineStatus::Ok);     // (row 5 gone)
        QCOMPARE(store.deleteLines(false, 4, 1, &e5), jp::calc::LineStatus::Ok);    // (what was column 3, now at 4, gone)
        QVERIFY(store.rows().runCount() >= 5);
        QVERIFY(store.cols().runCount() >= 3);
        // 40 rows minus 4 deleted, of 6 columns minus one
        std::vector<std::pair<quint32, quint32>> places;
        std::vector<int> rowsSeen;
        store.forEach([&](quint32 r, quint32 c, const Cell &cell) {
            places.push_back({r, c});
            rowsSeen.push_back(rowOf(cell));
            QVERIFY(colOf(cell) != 3);
            QVERIFY(rowOf(cell) < 20 || rowOf(cell) > 22);
            QVERIFY(rowOf(cell) != 5);
        });
        QCOMPARE(qint64(places.size()), qint64(36 * 5));
        QCOMPARE(store.cellCount(), qint64(36 * 5));
        QVERIFY(std::is_sorted(places.begin(), places.end()));
        QVERIFY(std::adjacent_find(places.begin(), places.end()) == places.end());
        QCOMPARE(store.deadCount(), qint64(4 * 6 + 36));   // (the cells of 4 rows, and of the column in the rows left; the column's cells in the 4 rows are counted once)
        // the cell that was row 6, column 1 is at row 5, column 1 now; and the one that was row 29 is at 29 - 3 - 1 = 25
        QVERIFY(store.find(5, 1) && rowOf(*store.find(5, 1)) == 6 && colOf(*store.find(5, 1)) == 1);
        QVERIFY(store.find(25, 0) && rowOf(*store.find(25, 0)) == 29);
        QVERIFY(store.find(25, 4) && colOf(*store.find(25, 4)) == 4);   // (the blank column is at 2, so what was column 3 is gone and 4 is at 4)
        QVERIFY(!store.find(25, 2));
        // a rectangle across the runs gives the same cells as the whole walk does for it
        std::vector<std::pair<quint32, quint32>> inRange;
        store.forEachInRange(3, 1, 30, 4, [&](quint32 r, quint32 c, const Cell &) {
            inRange.push_back({r, c});
            return true;
        });
        std::vector<std::pair<quint32, quint32>> expected;
        for (const auto &p : places)
            if (p.first >= 3 && p.first <= 30 && p.second >= 1 && p.second <= 4) expected.push_back(p);
        QVERIFY(inRange == expected && !inRange.empty());
        // everything undone: the 240 cells, where they were
        QCOMPARE(store.undoLines(false, e5), jp::calc::LineStatus::Ok);
        QCOMPARE(store.undoLines(true, e4), jp::calc::LineStatus::Ok);
        QCOMPARE(store.undoLines(false, e3), jp::calc::LineStatus::Ok);
        QCOMPARE(store.undoLines(true, e2), jp::calc::LineStatus::Ok);
        QCOMPARE(store.undoLines(true, e1), jp::calc::LineStatus::Ok);
        QCOMPARE(store.cellCount(), qint64(240));
        QCOMPARE(store.deadCount(), qint64(0));
        int seen = 0;
        store.forEach([&](quint32 r, quint32 c, const Cell &cell) {
            QVERIFY(rowOf(cell) == int(r) && colOf(cell) == int(c));
            ++seen;
        });
        QCOMPARE(seen, 240);
        // an Undo that is not the last one made is refused, and changes nothing
        CellStore::LineEdits a, b;
        QCOMPARE(store.insertLines(true, 1, 1, &a), jp::calc::LineStatus::Ok);
        QCOMPARE(store.insertLines(true, 1, 1, &b), jp::calc::LineStatus::Ok);
        QCOMPARE(store.undoLines(true, a), jp::calc::LineStatus::StaleStep);
        QCOMPARE(store.cellCount(), qint64(240));
    }

    // Lines inserted so near the end of the sheet that they do not all fit push everything from there on off the sheet (the cells of those rows stay, dead, until Undo), and a delete that
    // reaches past the end is cut to it; either way the sheet is as long as it was, and Undo gives back what was.
    void linesNearTheEndOfTheSheetAreCutToIt()
    {
        CellStore store;
        store.add(makeCell(10, 0, 1));
        store.add(makeCell(1048574, 0, 2));
        store.add(makeCell(1048575, 3, 3));
        store.finish();
        const quint32 rows = store.rows().size(), cols = store.cols().size();
        QCOMPARE(rows, 1048576u);
        CellStore::LineEdits insert;
        QCOMPARE(store.insertLines(true, 1048574, 5, &insert), jp::calc::LineStatus::Ok);
        QCOMPARE(store.rows().size(), rows);
        QVERIFY(store.find(10, 0) && !store.find(1048574, 0) && !store.find(1048575, 3));
        QCOMPARE(store.cellCount(), qint64(1));
        QVERIFY(store.set(makeCell(1048575, 3, 9)));   // (the rows at the end are there to be written to)
        QVERIFY(store.erase(1048575, 3));
        QCOMPARE(store.undoLines(true, insert), jp::calc::LineStatus::Ok);
        QCOMPARE(store.rows().size(), rows);
        QVERIFY(store.find(1048574, 0) && store.find(1048575, 3));
        QCOMPARE(store.cellCount(), qint64(3));
        // a delete of more rows than there are left, and of the last column
        CellStore::LineEdits erase, column;
        QCOMPARE(store.deleteLines(true, 1048570, 100, &erase), jp::calc::LineStatus::Ok);
        QCOMPARE(store.rows().size(), rows);
        QCOMPARE(store.cellCount(), qint64(1));
        QVERIFY(store.set(makeCell(1048575, 0, 4)));
        QVERIFY(store.erase(1048575, 0));
        QCOMPARE(store.undoLines(true, erase), jp::calc::LineStatus::Ok);
        QCOMPARE(store.cellCount(), qint64(3));
        QCOMPARE(store.deleteLines(false, 3, 1, &column), jp::calc::LineStatus::Ok);
        QCOMPARE(store.cols().size(), cols);
        QCOMPARE(store.cellCount(), qint64(2));
        QCOMPARE(store.redoLines(false, column), jp::calc::LineStatus::StaleStep);   // (it is done)
        QCOMPARE(store.undoLines(false, column), jp::calc::LineStatus::Ok);
        QCOMPARE(store.cellCount(), qint64(3));
        QCOMPARE(store.undoLines(false, column), jp::calc::LineStatus::StaleStep);   // (and it is undone)
        QVERIFY(store.find(1048575, 3));
    }

    // A walk from the end in slices, with rows or columns inserted or deleted between two slices (the print layout walks a sheet a slice at a time while it is edited): the cursor sees that the order
    // changed and starts again from the end, saying so (`restarted`), so that what the walk gave until then (a used range) can be let go of; what it gives after is exactly what a fresh walk gives,
    // for an edit above the place it had got to, at it, or below it, in a store that was edited before or not, and in a store the order of which is one run again.
    void aSlicedWalkFromTheEndStartsAgainWhenTheOrderIsEditedBetweenSlices()
    {
        auto build = [] {
            auto store = std::make_unique<CellStore>();
            for (quint32 r = 0; r < 300; ++r)
                for (quint32 c = 0; c < 3; ++c) store->add(makeCell(r, c, r * 10 + c));
            store->finish();
            return store;
        };
        auto walkAcross = [&](CellStore &store, const std::function<void()> &between, bool expectRestart, const char *what) {
            CellStore::RowTailCursor cursor;
            qint64 steps = 0;
            int firstCount = 0;
            store.forEachRowTailSome(&cursor, 40, &steps, [&](quint32, quint32, const Cell &) {
                ++firstCount;
                return false;
            });
            QVERIFY2(firstCount > 0 && !cursor.restarted, what);
            between();
            std::vector<std::pair<quint32, quint32>> rest, fresh;
            quint32 lastRow = 0, lastCol = 0;
            while (!store.forEachRowTailSome(&cursor, steps + 25, &steps, [&](quint32 r, quint32 c, const Cell &) {
                rest.push_back({r, c});
                lastRow = std::max(lastRow, r);
                lastCol = std::max(lastCol, c);
                return false;
            })) {}
            store.forEachRowTail([&](quint32 r, quint32 c, const Cell &) {
                fresh.push_back({r, c});
                return false;
            });
            QVERIFY2(cursor.restarted == expectRestart, what);
            if (!expectRestart) {
                // (it goes on: what is left of the walk, after the cells of the first slice)
                QVERIFY2(rest.size() + size_t(firstCount) == fresh.size() && std::equal(rest.begin(), rest.end(), fresh.begin() + firstCount), what);
                return;
            }
            QVERIFY2(rest == fresh && !fresh.empty(), qPrintable(QStringLiteral("%1: %2 cells after the first slice, a fresh walk gives %3").arg(QLatin1String(what)).arg(rest.size()).arg(fresh.size())));
            quint32 freshCol = 0;
            for (const auto &p : fresh) freshCol = std::max(freshCol, p.second);
            QVERIFY2(lastRow == fresh.front().first && lastCol == freshCol, what);   // (the used range of what was walked after)
        };
        for (const quint32 at : {5u, 150u, 290u, 299u}) {
            auto rowsInserted = build();
            walkAcross(*rowsInserted, [&] { QCOMPARE(rowsInserted->insertLines(true, at, 2, nullptr), jp::calc::LineStatus::Ok); }, true, "rows inserted");
            auto rowsDeleted = build();
            walkAcross(*rowsDeleted, [&] { QCOMPARE(rowsDeleted->deleteLines(true, at, 2, nullptr), jp::calc::LineStatus::Ok); }, true, "rows deleted");
        }
        auto columns = build();
        walkAcross(*columns, [&] { QCOMPARE(columns->insertLines(false, 1, 2, nullptr), jp::calc::LineStatus::Ok); }, true, "columns inserted");
        // a store edited before the walk, and again between the slices
        for (const bool insert : {true, false}) {
            auto store = build();
            QCOMPARE(store->insertLines(true, 100, 3, nullptr), jp::calc::LineStatus::Ok);
            QCOMPARE(store->deleteLines(true, 50, 2, nullptr), jp::calc::LineStatus::Ok);
            walkAcross(*store, [&] {
                if (insert) QCOMPARE(store->insertLines(true, 20, 1, nullptr), jp::calc::LineStatus::Ok);
                else QCOMPARE(store->deleteLines(true, 20, 1, nullptr), jp::calc::LineStatus::Ok);
            }, true, "edited before, and again");
        }
        {
            // the order is one run again between the slices (the step that made the edit is let go of)
            auto store = build();
            CellStore::LineEdits edit;
            QCOMPARE(store->insertLines(true, 295, 1, &edit), jp::calc::LineStatus::Ok);
            QVERIFY(!store->rows().isIdentity());
            walkAcross(*store, [&] {
                QCOMPARE(store->undoLines(true, edit), jp::calc::LineStatus::Ok);
                store->forgetLines(true, &edit);
                QVERIFY(store->rows().isIdentity());
            }, true, "edited, then back to one run");
        }
        {
            // nothing edited between the slices: the walk goes on, and does not say it started again
            auto store = build();
            walkAcross(*store, [] {}, false, "not edited");
        }
    }

    // Walks over a range of a store whose rows and columns were edited count the runs they go along and the searches they make, so that a limit on the work of a walk bounds it, and a range
    // across many runs of columns steps along each row once (it does not search for each run in each row).
    void aRangeWalkOfAnEditedStoreCountsItsRunsAndItsSearches()
    {
        CellStore store;
        for (quint32 r = 0; r < 200; ++r) store.add(makeCell(r, 0, r));
        store.finish();
        for (quint32 k = 0; k < 40; ++k) QCOMPARE(store.insertLines(true, 3 + k * 4, 1, nullptr), jp::calc::LineStatus::Ok);
        QVERIFY(store.rows().runCount() >= 40);
        // a range that holds nothing, across every run of rows: the runs are counted
        CellStore::RangeWalk none;
        store.forEachInRange(0, 5, 239, 6, [&](quint32, quint32, const Cell &) { return true; }, &none);
        QVERIFY2(none.stepped + none.compared >= 40, qPrintable(QStringLiteral("%1 stepped, %2 compared").arg(none.stepped).arg(none.compared)));
        // and a range of rows that have no cell at all, across 40 runs: nothing is stepped over and nothing is searched for but the runs themselves, which are counted as steps
        CellStore sparse;
        for (quint32 r = 0; r < 4; ++r) sparse.add(makeCell(r, 0, r));
        sparse.finish();
        for (quint32 k = 0; k < 40; ++k) QCOMPARE(sparse.insertLines(true, 10 + k * 2, 1, nullptr), jp::calc::LineStatus::Ok);
        CellStore::RangeWalk empty;
        sparse.forEachInRange(5, 0, 150, 3, [&](quint32, quint32, const Cell &) { return true; }, &empty);
        QVERIFY2(empty.stepped >= 40, qPrintable(QStringLiteral("%1 stepped, %2 compared").arg(empty.stepped).arg(empty.compared)));
        // a range with cells: what it holds is counted too
        CellStore::RangeWalk some;
        int seen = 0;
        store.forEachInRange(0, 0, 239, 0, [&](quint32, quint32, const Cell &) { return ++seen > 0; }, &some);
        QCOMPARE(seen, 200);
        QVERIFY2(some.stepped >= 200 && some.stepped + some.compared < 200 * 30, qPrintable(QStringLiteral("%1 stepped, %2 compared").arg(some.stepped).arg(some.compared)));
        // many runs of columns, none in the order of its ids: each row is gone along once
        CellStore wide;
        for (quint32 r = 0; r < 100; ++r)
            for (quint32 c = 0; c < 120; ++c) wide.add(makeCell(r, c, r * 1000 + c));
        wide.finish();
        for (quint32 k = 0; k < 40; ++k) QCOMPARE(wide.insertLines(false, 2 + k * 2, 1, nullptr), jp::calc::LineStatus::Ok);
        QVERIFY(wide.cols().runCount() >= 40);
        CellStore::RangeWalk whole;
        int cells = 0;
        quint32 lastRow = 0, lastCol = 0;
        bool ordered = true;
        wide.forEachInRange(0, 0, 99, 200, [&](quint32 r, quint32 c, const Cell &) {
            ordered = ordered && (cells == 0 || r > lastRow || (r == lastRow && c > lastCol));
            lastRow = r;
            lastCol = c;
            ++cells;
            return true;
        }, &whole);
        QCOMPARE(cells, 100 * 120);
        QVERIFY(ordered);
        QVERIFY2(whole.compared < 100 * 40, qPrintable(QStringLiteral("%1 stepped, %2 compared for %3 cells").arg(whole.stepped).arg(whole.compared).arg(cells)));
    }

    // Cells shifted down or right in a store whose rows and columns were edited go where the places say, and cells of lines that are deleted stay where they are.
    void cellsShiftedInAnEditedStoreAreByPlace()
    {
        CellStore store;
        std::map<std::pair<quint32, quint32>, double> model;
        for (quint32 r = 0; r < 60; ++r)
            for (quint32 c = 0; c < 6; ++c) {
                store.add(makeCell(r, c, r * 10 + c));
                model[{r, c}] = r * 10 + c;
            }
        store.finish();
        QCOMPARE(store.insertLines(true, 5, 2, nullptr), jp::calc::LineStatus::Ok);
        QCOMPARE(store.deleteLines(true, 20, 3, nullptr), jp::calc::LineStatus::Ok);
        QCOMPARE(store.insertLines(false, 3, 1, nullptr), jp::calc::LineStatus::Ok);
        // the model of those: places of the cells that are left
        std::map<std::pair<quint32, quint32>, double> placed;
        store.forEach([&](quint32 r, quint32 c, const Cell &cell) { placed[{r, c}] = cell.number; });
        QCOMPARE(qint64(placed.size()), qint64(57 * 6));
        // the cells of columns 1 to 4 from row 30 on move down by 2
        store.reshuffle(30, [](quint32 r, quint32 c, const Cell &) { return r >= 30 && c >= 1 && c <= 4; }, [](PlacedCell *p) { p->row += 2; return true; }, [](const PlacedCell &, const PlacedCell &) {}, nullptr);
        std::map<std::pair<quint32, quint32>, double> want;
        for (const auto &e : placed) want[{e.first.first >= 30 && e.first.second >= 1 && e.first.second <= 4 ? e.first.first + 2 : e.first.first, e.first.second}] = e.second;
        std::map<std::pair<quint32, quint32>, double> got;
        store.forEach([&](quint32 r, quint32 c, const Cell &cell) { got[{r, c}] = cell.number; });
        QVERIFY(got == want);
        QCOMPARE(store.deadCount(), qint64(3 * 6));   // (the cells of the rows that were deleted)
    }

    // The step that made an edit of rows or columns is let go of: the cells that only it could bring back go with it, and the order is one run again when nothing else is held of it; what is left
    // is the live cells, and the count.
    void forgettingTheStepOfAnEditLetsGoOfTheCellsOfItsDeadLines()
    {
        CellStore store;
        for (quint32 r = 0; r < 100; ++r)
            for (quint32 c = 0; c < 5; ++c) store.add(makeCell(r, c, r * 10 + c));
        store.finish();
        CellStore::LineEdits del, col, ins;
        QCOMPARE(store.deleteLines(true, 10, 4, &del), jp::calc::LineStatus::Ok);
        QCOMPARE(store.deleteLines(false, 1, 1, &col), jp::calc::LineStatus::Ok);
        QCOMPARE(store.insertLines(true, 50, 2, &ins), jp::calc::LineStatus::Ok);
        QCOMPARE(store.cellCount(), qint64(96 * 4));
        QCOMPARE(store.deadCount(), qint64(4 * 5 + 96));
        const qint64 held = store.memoryBytes();
        // the columns' step is let go of while the rows' is still held: the column's cells go (those of the deleted rows were counted with the rows')
        store.forgetLines(false, &col);
        QCOMPARE(store.cellCount(), qint64(96 * 4));
        QCOMPARE(store.deadCount(), qint64(4 * 5 - 4));     // (the 4 rows' cells that are in the column went with it)
        QVERIFY(store.memoryBytes() <= held);
        QCOMPARE(store.undoLines(true, ins), jp::calc::LineStatus::Ok);
        QCOMPARE(store.undoLines(true, del), jp::calc::LineStatus::Ok);
        QCOMPARE(store.cellCount(), qint64(100 * 4));
        QCOMPARE(store.deadCount(), qint64(0));
        // the rows' steps are let go of (the insert undone, the delete undone): nothing is held of either, and the order is one run
        store.forgetLines(true, &ins);
        store.forgetLines(true, &del);
        QVERIFY(store.rows().isIdentity());
        QCOMPARE(store.cellCount(), qint64(100 * 4));
        // a delete that is let go of while it is done: its cells go, and the rows stay deleted
        CellStore::LineEdits again;
        QCOMPARE(store.deleteLines(true, 0, 10, &again), jp::calc::LineStatus::Ok);
        QCOMPARE(store.deadCount(), qint64(10 * 4));
        store.forgetLines(true, &again);
        QCOMPARE(store.deadCount(), qint64(0));
        QCOMPARE(store.cellCount(), qint64(90 * 4));
        QCOMPARE(store.undoLines(true, again), jp::calc::LineStatus::StaleStep);   // (it is empty)
        // an edit of another store's is not let go of in this
        CellStore other;
        other.add(makeCell(0, 0, 1));
        other.finish();
        CellStore::LineEdits foreign;
        QCOMPARE(other.deleteLines(true, 0, 1, &foreign), jp::calc::LineStatus::Ok);
        QCOMPARE(store.undoLines(true, foreign), jp::calc::LineStatus::StaleStep);
        const qint64 before = store.cellCount();
        store.forgetLines(true, &foreign);
        QCOMPARE(store.cellCount(), before);
    }

    // The order put back to one run: the cells have the ids of their places, the dead lines' cells are gone, the count is right, and an Undo of an edit made before is refused.
    void anOrderPutBackToOneRunKeepsTheLiveCells()
    {
        std::mt19937 rng(20261030);
        for (const bool rows : {true, false}) {
            CellStore store;
            std::map<std::pair<quint32, quint32>, double> model;
            for (int i = 0; i < 2000; ++i) {
                const quint32 r = rng() % 150, c = rng() % 30;
                store.add(makeCell(r, c, i));
                model[{r, c}] = i;
            }
            store.finish();
            CellStore::LineEdits last;
            for (int k = 0; k < 30; ++k) {
                const quint32 at = rng() % (rows ? 150 : 30), count = 1 + rng() % 3;
                if (rng() % 2) QCOMPARE(store.insertLines(rows, at, count, &last), jp::calc::LineStatus::Ok);
                else QCOMPARE(store.deleteLines(rows, at, count, &last), jp::calc::LineStatus::Ok);
            }
            std::vector<std::pair<std::pair<quint32, quint32>, double>> before;
            store.forEach([&](quint32 r, quint32 c, const Cell &cell) { before.push_back({{r, c}, cell.number}); });
            QVERIFY(!(rows ? store.rows().isIdentity() : store.cols().isIdentity()));
            QCOMPARE(store.rebaseLines(rows), jp::calc::LineStatus::Ok);
            QVERIFY(rows ? store.rows().isIdentity() : store.cols().isIdentity());
            std::vector<std::pair<std::pair<quint32, quint32>, double>> after;
            store.forEach([&](quint32 r, quint32 c, const Cell &cell) { after.push_back({{r, c}, cell.number}); });
            QVERIFY(before == after);
            QCOMPARE(store.cellCount(), qint64(before.size()));
            QCOMPARE(store.deadCount(), qint64(0));
            QCOMPARE(store.undoLines(rows, last), jp::calc::LineStatus::StaleStep);
            // and it goes on as before
            QCOMPARE(store.insertLines(rows, 3, 2, nullptr), jp::calc::LineStatus::Ok);
            QCOMPARE(store.cellCount(), qint64(before.size()));
        }
    }

    // A long session of edits of columns, with Undo and Redo, new edits that drop what was undone, and a budget of ids that is small (as the real one is, after a very long session): no edit is refused, the
    // order is put back to one run when it is near what it may hold, the cells held do not grow, and what is walked is always what a model of the places says.
    void tenThousandEditsOfColumnsAreNeverRefusedForWantOfIds()
    {
        std::mt19937 rng(20261031);
        CellStore store;
        jp::calc::LineLimits cols = jp::calc::LineLimits::cols();
        cols.idLimit = cols.firstNew + 400;
        cols.maxRuns = 600;
        cols.rebaseRuns = 40;
        store.setLineLimits(jp::calc::LineLimits::rows(), cols);
        using Model = std::map<std::pair<quint32, quint32>, double>;
        Model model;
        for (quint32 r = 0; r < 40; ++r)
            for (quint32 c = 0; c < 12; ++c) {
                store.add(makeCell(r, c, r * 100 + c));
                model[{r, c}] = r * 100 + c;
            }
        store.finish();
        struct Rec {
            CellStore::LineEdits edits;
            Model before, after;
        };
        std::vector<Rec> done, undone;
        auto apply = [](Model *m, bool insert, quint32 at, quint32 count) {
            Model out;
            for (const auto &e : *m) {
                auto key = e.first;
                if (insert) {
                    if (key.second >= at) key.second += count;
                    if (key.second >= 16384) continue;
                } else {
                    if (key.second >= at && key.second < at + count) continue;
                    if (key.second >= at + count) key.second -= count;
                }
                out[key] = e.second;
            }
            *m = out;
        };
        auto same = [&] {
            Model got;
            store.forEach([&](quint32 r, quint32 c, const Cell &cell) { got[{r, c}] = cell.number; });
            return got == model && store.cellCount() == qint64(model.size());
        };
        int compactions = 0, undos = 0, redos = 0, edits = 0;
        qint64 mostHeld = 0;
        auto forgetAll = [&] {
            while (!undone.empty()) {
                store.forgetLines(false, &undone.back().edits);
                undone.pop_back();
            }
            while (!done.empty()) {
                store.forgetLines(false, &done.back().edits);
                done.pop_back();
            }
        };
        for (int i = 0; i < 10000; ++i) {
            const int roll = int(rng() % 10);
            if (roll < 2 && !done.empty()) {
                Rec r = std::move(done.back());
                done.pop_back();
                QCOMPARE(store.undoLines(false, r.edits), jp::calc::LineStatus::Ok);
                model = r.before;
                undone.push_back(std::move(r));
                ++undos;
            } else if (roll < 3 && !undone.empty()) {
                Rec r = std::move(undone.back());
                undone.pop_back();
                QCOMPARE(store.redoLines(false, r.edits), jp::calc::LineStatus::Ok);
                model = r.after;
                done.push_back(std::move(r));
                ++redos;
            } else {
                // a new edit: what was undone is dropped (let go of, newest first), and the order is compacted first if it is near its limits
                while (!undone.empty()) {
                    store.forgetLines(false, &undone.back().edits);
                    undone.pop_back();
                }
                const bool insert = rng() % 2 == 0;
                const quint32 at = rng() % 14, count = 1 + rng() % 2;
                if (store.needsCompaction(false, count)) {
                    forgetAll();
                    QCOMPARE(store.rebaseLines(false), jp::calc::LineStatus::Ok);
                    ++compactions;
                }
                Rec r;
                r.before = model;
                const jp::calc::LineStatus st = insert ? store.insertLines(false, at, count, &r.edits) : store.deleteLines(false, at, count, &r.edits);
                QVERIFY2(st == jp::calc::LineStatus::Ok, qPrintable(QStringLiteral("edit %1 was refused: %2").arg(i).arg(jp::calc::lineStatusText(st))));
                apply(&model, insert, at, count);
                r.after = model;
                done.push_back(std::move(r));
                ++edits;
                if (done.size() > 60) {
                    // (the history is limited: the oldest step is let go of)
                    store.forgetLines(false, &done.front().edits);
                    done.erase(done.begin());
                }
            }
            if (i % 97 == 0) QVERIFY2(same(), qPrintable(QStringLiteral("after %1 edits").arg(i)));
            mostHeld = std::max(mostHeld, store.cellCount() + store.deadCount());
        }
        QVERIFY(same());
        QVERIFY(compactions > 10 && undos > 100 && redos > 50 && edits > 5000);
        // nothing grows: the cells held (live and dead) are at most the live ones and what the steps held at the most can bring back
        QVERIFY2(mostHeld <= 40 * 12 + 60 * 40 * 2, qPrintable(QString::number(mostHeld)));
        forgetAll();
        QCOMPARE(store.deadCount(), qint64(0));
        QVERIFY(same());
    }

    // A store that was moved from is empty and has nothing of the one it was moved to: cells added to either go where they should.
    void aStoreMovedFromAndTheOneItWentToAreSeparate()
    {
        CellStore a;
        for (quint32 r = 0; r < 10; ++r) a.add(makeCell(r, 0));   // not finished: the place of the last cell added is remembered
        CellStore b(std::move(a));
        a.add(makeCell(1, 5));
        b.add(makeCell(2, 5));
        a.finish();
        b.finish();
        QCOMPARE(a.cellCount(), qint64(1));
        QCOMPARE(b.cellCount(), qint64(11));
        QVERIFY(a.find(1, 5) && !a.find(2, 5) && !a.find(2, 0));
        QVERIFY(b.find(2, 5) && !b.find(1, 5) && b.find(2, 0));
        // And moved by assignment.
        CellStore c;
        for (quint32 r = 0; r < 10; ++r) c.add(makeCell(r, 0));
        CellStore d;
        d.add(makeCell(70, 1));
        d = std::move(c);
        c.add(makeCell(3, 7));
        d.add(makeCell(4, 7));
        c.finish();
        d.finish();
        QCOMPARE(c.cellCount(), qint64(1));
        QCOMPARE(d.cellCount(), qint64(11));
        QVERIFY(c.find(3, 7) && !c.find(4, 7) && !c.find(70, 1));
        QVERIFY(d.find(4, 7) && !d.find(3, 7) && !d.find(70, 1));
        // A store moved into itself, or from one that is empty, is as it was.
        CellStore e;
        CellStore f(std::move(e));
        f.add(makeCell(0, 0));
        f.finish();
        QCOMPARE(f.cellCount(), qint64(1));
        QCOMPARE(e.cellCount(), qint64(0));
    }

    // ---- the types of cell

    void everyTypeOfCellIsRead()
    {
        const QByteArray sst = sharedStringsXml({QStringLiteral("<t>alpha</t>"), QStringLiteral("<t>beta</t>")});
        SharedStrings shared = stringsOf(sst);
        QCOMPARE(shared.strings.size(), 2);
        const QByteArray xml = worksheet(
            "<row r=\"1\">"
            "<c r=\"A1\"><v>1.5</v></c>"
            "<c r=\"B1\" t=\"n\"><v>-2e3</v></c>"
            "<c r=\"C1\" t=\"s\"><v>1</v></c>"
            "<c r=\"D1\" t=\"inlineStr\"><is><t>inline</t></is></c>"
            "<c r=\"E1\" t=\"b\"><v>1</v></c>"
            "<c r=\"F1\" t=\"b\"><v>0</v></c>"
            "<c r=\"G1\" t=\"e\"><v>#DIV/0!</v></c>"
            "<c r=\"H1\" t=\"str\"><f>A1&amp;\"x\"</f><v>1.5x</v></c>"
            "<c r=\"I1\" t=\"d\"><v>2024-03-15</v></c>"
            "<c r=\"J1\" s=\"4\"/>"
            "<c r=\"K1\" t=\"e\"><v>#N/A</v></c>"
            "<c r=\"L1\" t=\"e\"><v>#NAME?</v></c>"
            "<c r=\"M1\" t=\"e\"><v>#SPILL!</v></c>"
            "<c r=\"N1\" t=\"e\"><v>#WHAT?</v></c>"
            "<c r=\"O1\"><v>1E-3</v></c>"
            "<c r=\"P1\" t=\"s\"><v>0</v></c>"
            "</row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        QVERIFY(sheet.error.isEmpty());
        QCOMPARE(sheet.cells.cellCount(), qint64(16));
        QCOMPARE(show(sheet, shared, "A1"), QStringLiteral("n:1.5"));
        QCOMPARE(show(sheet, shared, "B1"), QStringLiteral("n:-2000"));
        QCOMPARE(show(sheet, shared, "C1"), QStringLiteral("s:beta"));
        QCOMPARE(show(sheet, shared, "D1"), QStringLiteral("i:inline"));
        QCOMPARE(show(sheet, shared, "E1"), QStringLiteral("b:1"));
        QCOMPARE(show(sheet, shared, "F1"), QStringLiteral("b:0"));
        QCOMPARE(show(sheet, shared, "G1"), QStringLiteral("e:#DIV/0!"));
        QCOMPARE(show(sheet, shared, "H1"), QStringLiteral("t:1.5x"));
        QCOMPARE(show(sheet, shared, "I1"), QStringLiteral("d:45366"));
        QCOMPARE(show(sheet, shared, "J1"), QStringLiteral("-"));
        QCOMPARE(at(sheet, "J1")->style, 4u);
        QCOMPARE(show(sheet, shared, "K1"), QStringLiteral("e:#N/A"));
        QCOMPARE(show(sheet, shared, "L1"), QStringLiteral("e:#NAME?"));
        QCOMPARE(show(sheet, shared, "M1"), QStringLiteral("e:#SPILL!"));
        QCOMPARE(at(sheet, "N1")->type, CellType::Error);
        QCOMPARE(CellError(int(at(sheet, "N1")->number)), CellError::Unknown);
        QCOMPARE(show(sheet, shared, "O1"), QStringLiteral("n:0.001"));
        QCOMPARE(show(sheet, shared, "P1"), QStringLiteral("s:alpha"));
        QCOMPARE(sheet.formulaCells, qint64(1));
        QCOMPARE(sheet.storedResults, qint64(1));
        // The error texts are those of the file, and both ways.
        bool known = false;
        for (const char *text : {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A", "#GETTING_DATA", "#SPILL!", "#CALC!"}) {
            const CellError e = errorFromText(QString::fromLatin1(text), &known);
            QVERIFY2(known, text);
            QCOMPARE(errorText(e), QString::fromLatin1(text));
        }
        errorFromText(QStringLiteral("#NOPE!"), &known);
        QVERIFY(!known);
    }

    // A cell's address is optional in a file: a cell follows the one before it, and a row the one before it.
    void cellsAndRowsWithoutAddressesFollowOn()
    {
        const QByteArray xml = worksheet("<row><c><v>1</v></c><c><v>2</v></c><c r=\"E1\"><v>3</v></c><c><v>4</v></c></row>"
                                         "<row r=\"5\"><c><v>5</v></c></row><row><c><v>6</v></c><c r=\"C6\"><v>7</v></c></row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        SharedStrings none;
        QCOMPARE(show(sheet, none, "A1"), QStringLiteral("n:1"));
        QCOMPARE(show(sheet, none, "B1"), QStringLiteral("n:2"));
        QCOMPARE(show(sheet, none, "E1"), QStringLiteral("n:3"));
        QCOMPARE(show(sheet, none, "F1"), QStringLiteral("n:4"));
        QCOMPARE(show(sheet, none, "A5"), QStringLiteral("n:5"));
        QCOMPARE(show(sheet, none, "A6"), QStringLiteral("n:6"));
        QCOMPARE(show(sheet, none, "C6"), QStringLiteral("n:7"));
        QCOMPARE(sheet.cells.cellCount(), qint64(7));
    }

    // Rows and cells out of order, and a place given twice, are put right: in order, the later cell standing.
    void rowsAndCellsOutOfOrderArePutRight()
    {
        const QByteArray xml = worksheet("<row r=\"3\"><c r=\"B3\"><v>1</v></c><c r=\"A3\"><v>2</v></c></row><row r=\"1\"><c r=\"A1\"><v>3</v></c></row>"
                                         "<row r=\"1\"><c r=\"A1\"><v>4</v></c></row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        QCOMPARE(sheet.cells.cellCount(), qint64(3));
        QVector<QString> order;
        sheet.cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) { order.append(cellName(cellRow, cellCol) + ":" + QString::number(c.number)); });
        QCOMPARE(order, (QVector<QString>{"A1:4", "A3:2", "B3:1"}));
    }

    // A number that is no number, or not a finite one, or a date that is none, is the error #VALUE!; a string with no place in the shared strings, a cell with no value.
    void valuesThatAreNotValuesAreNotNumbers()
    {
        const QByteArray xml = worksheet("<row r=\"1\"><c r=\"A1\"><v>nan</v></c><c r=\"B1\"><v>inf</v></c><c r=\"C1\"><v>abc</v></c><c r=\"D1\"><v></v></c>"
                                         "<c r=\"E1\"><v>1e999</v></c><c r=\"F1\" t=\"b\"><v>true</v></c><c r=\"G1\" t=\"s\"><v>x</v></c><c r=\"H1\" t=\"d\"><v>not a date</v></c>"
                                         "<c r=\"I1\"><f>1+1</f></c><c r=\"J1\" t=\"str\"><f>\"\"</f><v></v></c></row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        SharedStrings none;
        for (const char *ref : {"A1", "B1", "C1", "E1", "H1"}) {
            QVERIFY2(at(sheet, ref) != nullptr, ref);
            QCOMPARE(show(sheet, none, ref), QStringLiteral("e:#VALUE!"));
        }
        QCOMPARE(show(sheet, none, "D1"), QStringLiteral("-"));
        QCOMPARE(show(sheet, none, "F1"), QStringLiteral("b:1"));
        QCOMPARE(show(sheet, none, "G1"), QStringLiteral("-"));
        // A formula with no result stored has the cell empty; one whose result is the empty text has it.
        QCOMPARE(show(sheet, none, "I1"), QStringLiteral("-"));
        QCOMPARE(sheet.formulaCells, qint64(2));
        QCOMPARE(show(sheet, none, "J1"), QStringLiteral("t:"));
        QCOMPARE(sheet.storedResults, qint64(1));
    }

    // ---- shared strings

    void sharedStringsAreReadWithRichRunsAsPlainText()
    {
        QString why;
        const SharedStrings s = stringsOf(sharedStringsXml({
            QStringLiteral("<t>plain</t>"),
            QStringLiteral("<r><rPr><b/><sz val=\"11\"/></rPr><t>bold </t></r><r><t>and </t></r><r><rPr><i/></rPr><t xml:space=\"preserve\">italic</t></r>"),
            QStringLiteral("<t xml:space=\"preserve\">  spaced  </t>"),
            QStringLiteral("<t>phonetic</t><rPh sb=\"0\" eb=\"1\"><t>ignored</t></rPh><phoneticPr fontId=\"1\"/>"),
            QStringLiteral("<r><t>run</t></r><rPh sb=\"0\" eb=\"1\"><t>ruby</t></rPh>"),
            QStringLiteral(""),
            QStringLiteral("<t/>"),
            QStringLiteral("<t>a&amp;b &lt;c&gt; &#x4e2d;</t>"),
            QStringLiteral("<t>line_x000D__x000A_two_x005F_x0041_ _x00zz_ _x0041_</t>"),
            QStringLiteral("<t>café \U0001F600</t>"),
        }), &why);
        QVERIFY2(why.isEmpty(), qPrintable(why));
        QCOMPARE(s.strings.size(), 10);
        QCOMPARE(s.at(0), QStringLiteral("plain"));
        QCOMPARE(s.at(1), QStringLiteral("bold and italic"));
        QCOMPARE(s.at(2), QStringLiteral("  spaced  "));
        QCOMPARE(s.at(3), QStringLiteral("phonetic"));
        QCOMPARE(s.at(4), QStringLiteral("run"));
        QCOMPARE(s.at(5), QString());
        QCOMPARE(s.at(6), QString());
        QCOMPARE(s.at(7), QStringLiteral("a&b <c> 中"));
        // _xHHHH_ is a character written as its code; _x005F_ is an underscore, so what it escapes is not read again.
        QCOMPARE(s.at(8), QStringLiteral("line\r\ntwo_x0041_ _x00zz_ A"));
        QCOMPARE(s.at(9), QStringLiteral("café \U0001F600"));
        QCOMPARE(s.at(10), QString());
        QCOMPARE(s.at(-1), QString());
        qint64 chars = 0;
        for (const QString &str : s.strings) chars += str.size();
        QCOMPARE(s.chars, chars);
    }

    // An inline string reads as a shared one does (runs, no phonetic text), and the escapes are decoded.
    void inlineStringsAreReadLikeSharedOnes()
    {
        const QByteArray xml = worksheet("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><r><t>ab</t></r><r><rPr><b/></rPr><t>cd</t></r><rPh><t>no</t></rPh></is></c>"
                                         "<c r=\"B1\" t=\"inlineStr\"><is><t>x_x000A_y</t></is></c><c r=\"C1\" t=\"inlineStr\"><is/></c>"
                                         "<c r=\"D1\" t=\"inlineStr\"/></row>");
        Sheet sheet;
        QVERIFY2(readSheetXml(xml, &sheet), qPrintable(sheet.error));
        SharedStrings none;
        QCOMPARE(show(sheet, none, "A1"), QStringLiteral("i:abcd"));
        QCOMPARE(show(sheet, none, "B1"), QStringLiteral("i:x\ny"));
        QCOMPARE(show(sheet, none, "C1"), QStringLiteral("i:"));
        QCOMPARE(show(sheet, none, "D1"), QStringLiteral("-"));
    }

    // ---- the shape a part declares

    void theDimensionAndMergedCellsAreRead()
    {
        Sheet sheet;
        QVERIFY2(readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>",
                                        "<mergeCells count=\"99\"><mergeCell ref=\"A1:B2\"/><mergeCell ref=\"C3:D4\"/><mergeCell ref=\"E5:F6\"/></mergeCells>",
                                        "<dimension ref=\"B2:D20\"/>"), &sheet), qPrintable(sheet.error));
        QVERIFY(sheet.dimension.valid);
        QCOMPARE(rangeName(sheet.dimension), QStringLiteral("B2:D20"));
        QCOMPARE(sheet.mergeCount, qint64(3));   // counted, not taken from the count it says
        Sheet single;
        QVERIFY(readSheetXml(worksheet("", QString(), "<dimension ref=\"C7\"/>"), &single));
        QVERIFY(single.dimension.valid);
        QCOMPARE(rangeName(single.dimension), QStringLiteral("C7"));
        QCOMPARE(single.mergeCount, qint64(0));
        Sheet none;
        QVERIFY(readSheetXml(worksheet(""), &none));
        QVERIFY(!none.dimension.valid);
        Sheet wrong;
        QVERIFY(readSheetXml(worksheet("", QString(), "<dimension ref=\"nonsense\"/>"), &wrong));
        QVERIFY(!wrong.dimension.valid);
        // Other things in a sheet (columns, views, conditional formats with formulas of their own) are passed over: no cell comes of them.
        Sheet other;
        QVERIFY2(readSheetXml(worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>",
                                        "<conditionalFormatting sqref=\"A1\"><cfRule type=\"expression\"><formula>A1&gt;1</formula></cfRule></conditionalFormatting>"
                                        "<drawing r:id=\"rId1\"/>", "<sheetViews><sheetView/></sheetViews><cols><col min=\"1\" max=\"1\" width=\"9\"/></cols>"), &other),
                 qPrintable(other.error));
        QCOMPARE(other.cells.cellCount(), qint64(1));
        QCOMPARE(other.formulaCells, qint64(0));
    }

    // Strict files use other namespaces, and prefixes may be anything: what matters is the name.
    void theNamespaceFormAndPrefixDontMatter()
    {
        const QByteArray strict = "<x:worksheet xmlns:x=\"http://purl.oclc.org/ooxml/spreadsheetml/main\"><x:sheetData><x:row r=\"2\"><x:c r=\"B2\" t=\"d\"><x:v>2024-03-15T12:00:00</x:v></x:c>"
                                  "<x:c r=\"C2\"><x:f>B2+1</x:f><x:v>3</x:v></x:c></x:row></x:sheetData></x:worksheet>";
        Sheet sheet;
        QVERIFY2(readSheetXml(strict, &sheet), qPrintable(sheet.error));
        SharedStrings none;
        QCOMPARE(show(sheet, none, "B2"), QStringLiteral("d:45366.5"));
        QCOMPARE(show(sheet, none, "C2"), QStringLiteral("n:3"));
        QCOMPARE(sheet.formulaText(*at(sheet, "C2")), QStringLiteral("B2+1"));
        // A part that is not a sheet at all is refused.
        Sheet notSheet;
        QVERIFY(!readSheetXml("<workbook><sheetData><row><c><v>1</v></c></row></sheetData></workbook>", &notSheet));
        QVERIFY(!notSheet.error.isEmpty());
        QCOMPARE(notSheet.cells.cellCount(), qint64(0));
    }

private:
    // Rows and columns inserted and deleted, and Undo and Redo of them, move no cell: the store is what a map of places to cells would be after each (find, the walks over all of it, over a
    // rectangle, and from the end, in slices too, the count, and the cells held that are in lines that are deleted), the same cell is at its new place, and a deleted row or column comes back
    // with its cells. The rows are `rows` from `rowBase`, the columns `cols` from `colBase` (the end of the sheet, to push cells off it), and `cellCount` cells are put in.
    void lineEditsAgainstAModel(quint32 rows, quint32 cols, int cellCount, int steps, quint32 seed, quint32 rowBase, quint32 colBase)
    {
        struct Model {
            std::map<std::pair<quint32, quint32>, double> cells;
            void insert(bool rows, quint32 at, quint32 count)
            {
                std::map<std::pair<quint32, quint32>, double> out;
                for (const auto &e : cells) {
                    auto key = e.first;
                    quint32 &line = rows ? key.first : key.second;
                    if (line >= at) line += count;
                    if (line >= (rows ? 1048576u : 16384u)) continue;   // (pushed off the end)
                    out[key] = e.second;
                }
                cells = out;
            }
            void erase(bool rows, quint32 at, quint32 count)
            {
                std::map<std::pair<quint32, quint32>, double> out;
                for (const auto &e : cells) {
                    auto key = e.first;
                    quint32 &line = rows ? key.first : key.second;
                    if (line >= at && line < at + count) continue;
                    if (line >= at + count) line -= count;
                    out[key] = e.second;
                }
                cells = out;
            }
        };
        std::mt19937 rng(seed);
        const quint32 rowCap = 1048576, colCap = 16384;
        CellStore store;
        Model model;
        for (int i = 0; i < cellCount; ++i) {
            const quint32 r = rowBase + rng() % rows, c = colBase + rng() % cols;
            store.add(makeCell(r, c, i));
            model.cells[{r, c}] = i;
        }
        store.finish();
        const Model start = model;
        // one cell is followed: where it is held never changes
        const Cell *tracked = store.find(model.cells.begin()->first.first, model.cells.begin()->first.second);
        QVERIFY(tracked);
        std::pair<quint32, quint32> trackedAt = model.cells.begin()->first;
        bool trackedLive = true;
        struct Done {
            bool rows, insert;
            CellStore::LineEdits edits;
            Model before, after;
            std::pair<quint32, quint32> trackedBefore, trackedAfter;
            bool liveBefore, liveAfter;
        };
        std::vector<Done> done, undone;
        auto check = [&](const char *what) {
            const QString where = QStringLiteral("%1 (after %2 steps)").arg(QLatin1String(what)).arg(done.size());
            QVERIFY2(store.cellCount() == qint64(model.cells.size()), qPrintable(where + QStringLiteral(": %1 cells, expected %2").arg(store.cellCount()).arg(model.cells.size())));
            // (every cell put in is held still, those of deleted lines as dead cells)
            QVERIFY2(store.deadCount() == qint64(start.cells.size()) - qint64(model.cells.size()), qPrintable(where + QStringLiteral(": %1 dead cells, expected %2").arg(store.deadCount()).arg(qint64(start.cells.size()) - qint64(model.cells.size()))));
            std::vector<std::pair<quint32, quint32>> walked, want;
            store.forEach([&](quint32 r, quint32 c, const Cell &cell) {
                walked.push_back({r, c});
                const auto it = model.cells.find({r, c});
                if (it == model.cells.end() || it->second != cell.number) walked.push_back({~0u, ~0u});
            });
            for (const auto &e : model.cells) want.push_back(e.first);
            QVERIFY2(walked == want, qPrintable(where + QStringLiteral(": the walk over all of it")));
            for (int trial = 0; trial < 12; ++trial) {
                quint32 r1 = rowBase + rng() % (rows + 20), r2 = rowBase + rng() % (rows + 20), c1 = colBase + rng() % (cols + 6), c2 = colBase + rng() % (cols + 6);
                r1 = std::min(r1, rowCap - 1);
                r2 = std::min(r2, rowCap - 1);
                c1 = std::min(c1, colCap - 1);
                c2 = std::min(c2, colCap - 1);
                if (r1 > r2) std::swap(r1, r2);
                if (c1 > c2) std::swap(c1, c2);
                std::vector<std::pair<quint32, quint32>> got, expected;
                store.forEachInRange(r1, c1, r2, c2, [&](quint32 r, quint32 c, const Cell &) {
                    got.push_back({r, c});
                    return true;
                });
                for (const auto &e : model.cells)
                    if (e.first.first >= r1 && e.first.first <= r2 && e.first.second >= c1 && e.first.second <= c2) expected.push_back(e.first);
                QVERIFY2(got == expected, qPrintable(where + QStringLiteral(": rows %1-%2, columns %3-%4").arg(r1).arg(r2).arg(c1).arg(c2)));
            }
            // from the end: every cell of each row, last first; whole, and in slices of a few steps
            std::vector<std::pair<quint32, quint32>> tail, expectedTail(want.rbegin(), want.rend());
            store.forEachRowTail([&](quint32 r, quint32 c, const Cell &) {
                tail.push_back({r, c});
                return false;
            });
            QVERIFY2(tail == expectedTail, qPrintable(where + QStringLiteral(": the walk from the end")));
            std::vector<std::pair<quint32, quint32>> sliced;
            CellStore::RowTailCursor cursor;
            qint64 steps = 0;
            int slices = 0;
            while (!store.forEachRowTailSome(&cursor, steps + 9, &steps, [&](quint32 r, quint32 c, const Cell &) {
                sliced.push_back({r, c});
                return false;
            }))
                ++slices;
            QVERIFY2(sliced == expectedTail && (slices > 1 || expectedTail.size() < 30), qPrintable(where + QStringLiteral(": the walk from the end in slices: %1 cells, expected %2, %3 slices").arg(sliced.size()).arg(expectedTail.size()).arg(slices)));
            for (int probe = 0; probe < 40; ++probe) {
                const quint32 r = std::min<quint32>(rowBase + rng() % (rows + 20), rowCap - 1), c = std::min<quint32>(colBase + rng() % (cols + 6), colCap - 1);
                const Cell *found = store.find(r, c);
                const auto it = model.cells.find({r, c});
                QVERIFY2((found != nullptr) == (it != model.cells.end()) && (!found || found->number == it->second), qPrintable(where + QStringLiteral(": find %1,%2").arg(r).arg(c)));
            }
            // the cell that was followed is the very cell, if it is not in a deleted row or column
            if (trackedLive) QVERIFY2(store.find(trackedAt.first, trackedAt.second) == tracked, qPrintable(where + QStringLiteral(": the followed cell moved")));
        };
        check("at the start");
        for (int step = 0; step < steps; ++step) {
            const int roll = int(rng() % 10);
            if (roll >= 7 && !done.empty()) {
                // Undo
                Done d = std::move(done.back());
                done.pop_back();
                QCOMPARE(store.undoLines(d.rows, d.edits), jp::calc::LineStatus::Ok);
                model = d.before;
                trackedAt = d.trackedBefore;
                trackedLive = d.liveBefore;
                undone.push_back(std::move(d));
                check("undo");
            } else if (roll == 6 && !undone.empty()) {
                // Redo
                Done d = std::move(undone.back());
                undone.pop_back();
                QCOMPARE(store.redoLines(d.rows, d.edits), jp::calc::LineStatus::Ok);
                model = d.after;
                trackedAt = d.trackedAfter;
                trackedLive = d.liveAfter;
                done.push_back(std::move(d));
                check("redo");
            } else {
                undone.clear();
                Done d;
                d.rows = rng() % 2 == 0;
                d.insert = rng() % 2 == 0;
                const quint32 at = std::min<quint32>((d.rows ? rowBase : colBase) + rng() % ((d.rows ? rows : cols) + 10), (d.rows ? rowCap : colCap) - 1), count = 1 + rng() % 4;
                d.before = model;
                d.trackedBefore = trackedAt;
                d.liveBefore = trackedLive;
                if (d.insert) {
                    QCOMPARE(store.insertLines(d.rows, at, count, &d.edits), jp::calc::LineStatus::Ok);
                    model.insert(d.rows, at, count);
                    quint32 &line = d.rows ? trackedAt.first : trackedAt.second;
                    if (line >= at) line += count;
                    if (line >= (d.rows ? rowCap : colCap)) trackedLive = false;   // (pushed off the end)
                } else {
                    QCOMPARE(store.deleteLines(d.rows, at, count, &d.edits), jp::calc::LineStatus::Ok);
                    model.erase(d.rows, at, count);
                    quint32 &line = d.rows ? trackedAt.first : trackedAt.second;
                    if (trackedLive && line >= at && line < at + count) trackedLive = false;
                    else if (line >= at + count) line -= count;
                }
                d.after = model;
                d.trackedAfter = trackedAt;
                d.liveAfter = trackedLive;
                done.push_back(std::move(d));
                check(done.back().insert ? "an insert" : "a delete");
            }
        }
        // after all that, every edit undone gives the cells that were at the start, in the places they were
        while (!done.empty()) {
            Done d = std::move(done.back());
            done.pop_back();
            QCOMPARE(store.undoLines(d.rows, d.edits), jp::calc::LineStatus::Ok);
            model = d.before;
            trackedAt = d.trackedBefore;
            trackedLive = d.liveBefore;
        }
        QVERIFY(trackedLive);
        check("everything undone");
        QVERIFY(model.cells == start.cells);
        QCOMPARE(store.deadCount(), qint64(0));
    }


};

JP_TEST_CLASS(SheetCellTests)
#include "tst_sheet_cells.moc"
