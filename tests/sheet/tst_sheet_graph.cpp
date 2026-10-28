// Tests of the dependency graph (calc/graph.h): what reads a cell, found from single cells and from blocks kept as blocks, checked against asking every formula; a formula
// that tells again what it reads; blocks that are shared; the limit on what the graph holds; and that a million-row column is one entry, not a million.
#include "calchelpers.h"

#include "calc/graph.h"
#include "calc/idframe.h"
#include "calc/linemap.h"
#include "calc/refs.h"

#include <QRandomGenerator>

#include <algorithm>
#include <set>

using namespace jp::calc;

namespace {

CellArea cellArea(int sheet, quint32 row, quint32 col)
{
    CellArea a;
    a.sheet1 = a.sheet2 = sheet;
    a.row1 = a.row2 = row;
    a.col1 = a.col2 = col;
    return a;
}

CellArea blockArea(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2)
{
    CellArea a = cellArea(sheet, row1, col1);
    a.row2 = row2;
    a.col2 = col2;
    return a;
}

bool covers(const CellArea &a, int sheet, quint32 row, quint32 col) { return sheet >= a.sheet1 && sheet <= a.sheet2 && row >= a.row1 && row <= a.row2 && col >= a.col1 && col <= a.col2; }

bool meets(const CellArea &a, const CellArea &b)
{
    return a.sheet1 <= b.sheet2 && b.sheet1 <= a.sheet2 && a.row1 <= b.row2 && b.row1 <= a.row2 && a.col1 <= b.col2 && b.col1 <= a.col2;
}

std::set<quint32> asSet(const std::vector<quint32> &v) { return std::set<quint32>(v.begin(), v.end()); }

} // namespace

class SheetGraphTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // What reads a cell, from a thousand formulas of single cells, columns, blocks, whole columns and rows, and cells of several sheets, is what asking each of them gives; and
    // so is what reads a block, and what reads a cell after formulas have told again what they read.
    void whatReadsACellIsWhatEachFormulaReadsAgainstAskingEach()
    {
        QRandomGenerator rng(20261024);
        DependencyGraph graph;
        const int nodes = 1500;
        std::vector<std::vector<CellArea>> reads(static_cast<size_t>(nodes));
        auto randomArea = [&]() {
            CellArea a;
            const int kind = int(rng.bounded(10));
            a.sheet1 = a.sheet2 = int(rng.bounded(3));
            a.row1 = rng.bounded(300);
            a.col1 = rng.bounded(40);
            a.row2 = a.row1;
            a.col2 = a.col1;
            if (kind >= 4 && kind < 7) {
                a.row2 = a.row1 + rng.bounded(60);
                a.col2 = a.col1 + rng.bounded(3);
            } else if (kind == 7) {
                a.row1 = 0;
                a.row2 = 1048575;   // a whole column
                a.col2 = a.col1;
            } else if (kind == 8) {
                a.col1 = 0;
                a.col2 = 16383;     // a whole row
                a.row2 = a.row1;
            } else if (kind == 9) {
                a.sheet2 = std::min(2, a.sheet1 + 1 + int(rng.bounded(2)));   // through several sheets
                a.row2 = a.row1 + rng.bounded(5);
                a.col2 = a.col1 + rng.bounded(5);
            } else if (kind == 3) {
                a.row2 = a.row1 + rng.bounded(200);   // a block of many columns
                a.col2 = std::min<quint32>(16383, a.col1 + 9 + rng.bounded(rng.bounded(5) ? 60 : 16000));
            }
            return a;
        };
        auto tell = [&](int node) {
            std::vector<CellArea> areas;
            const int n = 1 + int(rng.bounded(4));
            for (int i = 0; i < n; ++i) areas.push_back(randomArea());
            QVERIFY(graph.setPrecedents(quint32(node), areas));
            reads[size_t(node)] = areas;
        };
        for (int node = 0; node < nodes; ++node) tell(node);
        // some tell again what they read, some read nothing now
        for (int i = 0; i < 400; ++i) tell(int(rng.bounded(quint32(nodes))));
        for (int i = 0; i < 100; ++i) {
            const int node = int(rng.bounded(quint32(nodes)));
            graph.clearPrecedents(quint32(node));
            reads[size_t(node)].clear();
        }
        auto check = [&]() {
            for (int trial = 0; trial < 600; ++trial) {
                const int sheet = int(rng.bounded(3));
                const quint32 row = rng.bounded(rng.bounded(4) ? 320 : 1048576), col = rng.bounded(rng.bounded(4) ? 45 : 16384);
                std::vector<quint32> got;
                graph.dependentsOfCell(sheet, row, col, &got);
                std::set<quint32> expected;
                for (int n = 0; n < nodes; ++n)
                    for (const CellArea &a : reads[size_t(n)])
                        if (covers(a, sheet, row, col)) expected.insert(quint32(n));
                QVERIFY2(asSet(got) == expected, qPrintable(QStringLiteral("sheet %1 cell %2,%3: %4 formulas, expected %5").arg(sheet).arg(row).arg(col).arg(asSet(got).size()).arg(expected.size())));
            }
            for (int trial = 0; trial < 300; ++trial) {
                CellArea q = randomArea();
                if (rng.bounded(3) == 0) {
                    q.row2 = q.row1 + rng.bounded(2000);
                    q.col2 = std::min<quint32>(16383, q.col1 + rng.bounded(100));
                }
                std::vector<quint32> got;
                graph.dependentsOfArea(q, &got);
                std::set<quint32> expected;
                for (int n = 0; n < nodes; ++n)
                    for (const CellArea &a : reads[size_t(n)])
                        if (meets(a, q)) expected.insert(quint32(n));
                QVERIFY2(asSet(got) == expected, qPrintable(QStringLiteral("block: %1 formulas, expected %2").arg(asSet(got).size()).arg(expected.size())));
            }
        };
        check();
        QVERIFY(!QTest::currentTestFailed());
        // after the graph lets go of what was dropped, and after more is told
        graph.compact();
        check();
        QVERIFY(!QTest::currentTestFailed());
        for (int i = 0; i < 300; ++i) tell(int(rng.bounded(quint32(nodes))));
        check();
    }

    // A column of a million rows read by two hundred thousand formulas, and a running total of ten thousand blocks, each from the top to its own row, are entries as many as
    // the reads, not the cells of the blocks.
    void blocksAreNotExpandedToTheirCells()
    {
        DependencyGraph graph;
        for (quint32 n = 0; n < 200000; ++n) QVERIFY(graph.setPrecedents(n, {blockArea(0, 0, 0, 1048575, 0)}));
        QVERIFY2(graph.entryCount() <= 200000 + 5, qPrintable(QString::number(graph.entryCount())));
        for (quint32 n = 0; n < 10000; ++n) QVERIFY(graph.setPrecedents(300000 + n, {blockArea(0, 0, 1, n, 1)}));
        QVERIFY2(graph.entryCount() <= 200000 + 10000 * 2 + 10, qPrintable(QString::number(graph.entryCount())));
        QVERIFY2(graph.memoryBytes() < 80 * 1000 * 1000, qPrintable(QString::number(graph.memoryBytes())));
        std::vector<quint32> got;
        graph.dependentsOfCell(0, 123456, 0, &got);
        QCOMPARE(got.size(), size_t(200000));
        got.clear();
        graph.dependentsOfCell(0, 4999, 1, &got);
        QCOMPARE(got.size(), size_t(10000 - 4999));   // the running totals of rows 4999 down
        got.clear();
        graph.dependentsOfCell(0, 5000, 2, &got);
        QVERIFY(got.empty());
    }

    // A formula that tells again what it reads is not found by what it read before, at once for however many cells that was.
    void whatAFormulaReadBeforeIsForgottenAtOnce()
    {
        DependencyGraph graph;
        std::vector<CellArea> many;
        for (quint32 i = 0; i < 5000; ++i) many.push_back(cellArea(0, i, 3));
        QVERIFY(graph.setPrecedents(7, many));
        QVERIFY(graph.setPrecedents(8, {blockArea(0, 0, 3, 5000, 3)}));
        std::vector<quint32> got;
        graph.dependentsOfCell(0, 100, 3, &got);
        QCOMPARE(asSet(got), (std::set<quint32>{7, 8}));
        QVERIFY(graph.setPrecedents(7, {cellArea(0, 9, 9)}));
        got.clear();
        graph.dependentsOfCell(0, 100, 3, &got);
        QCOMPARE(asSet(got), (std::set<quint32>{8}));
        got.clear();
        graph.dependentsOfCell(0, 9, 9, &got);
        QCOMPARE(asSet(got), (std::set<quint32>{7}));
        graph.clearPrecedents(8);
        got.clear();
        graph.dependentsOfCell(0, 100, 3, &got);
        QVERIFY(got.empty());
        graph.compact();
        QVERIFY(graph.entryCount() <= 3);
    }

    // The graph holds no more bytes than it is allowed: past it a formula is told no, tells nothing, and what it told before is gone; what is dropped makes room.
    void theGraphHoldsNoMoreThanItMay()
    {
        const qint64 budget = 20000;
        DependencyGraph graph(budget);
        int refused = 0;
        for (quint32 n = 0; n < 600; ++n)
            if (!graph.setPrecedents(n, {cellArea(0, n, 0), cellArea(0, n, 1)})) ++refused;
        QVERIFY2(refused > 0 && graph.memoryBytes() <= budget, qPrintable(QStringLiteral("%1 refused, %2 bytes held of %3").arg(refused).arg(graph.memoryBytes()).arg(budget)));
        // dropped entries are let go of to make room
        for (quint32 n = 0; n < 600; ++n) graph.clearPrecedents(n);
        QVERIFY(graph.setPrecedents(700, {cellArea(0, 1, 1), cellArea(0, 2, 2)}));
        std::vector<quint32> got;
        graph.dependentsOfCell(0, 1, 1, &got);
        QCOMPARE(asSet(got), (std::set<quint32>{700}));
        // a formula refused is told nothing: what it told before is gone
        DependencyGraph small(4000);
        QVERIFY(small.setPrecedents(1, {cellArea(0, 0, 0), cellArea(0, 1, 0)}));
        std::vector<CellArea> more;
        for (quint32 r = 5; r < 200; ++r) more.push_back(cellArea(0, r, 0));
        QVERIFY(!small.setPrecedents(1, more));
        got.clear();
        small.dependentsOfCell(0, 0, 0, &got);
        QVERIFY(got.empty());
        QVERIFY(small.memoryBytes() <= 4000);
    }

    // A query for the formulas that read a cell looks at a number of items that grows with the logarithm of what the graph holds, at every size (counted, not timed): not
    // with the cells told since the graph last put its cells in order.
    void aPointQueryLooksAtFewItemsAtEverySize()
    {
        DependencyGraph graph;
        const quint32 n = 600000;
        qint64 worst = 0;
        quint32 worstAt = 0;
        for (quint32 node = 0; node < n; ++node) {
            QVERIFY(graph.setPrecedents(node, {cellArea(0, (node * 7919u) % 1000003u, node % 16000)}));
            if (node % 15000 != 14999) continue;
            for (quint32 q = 0; q < 10; ++q) {
                const quint32 k = (node * 31u + q * 977u) % (node + 1);
                std::vector<quint32> got;
                const qint64 before = graph.queryWork();
                graph.dependentsOfCell(0, (k * 7919u) % 1000003u, k % 16000, &got);
                QVERIFY2(got.size() == 1 && got[0] == k, qPrintable(QString::number(k)));
                if (graph.queryWork() - before > worst) {
                    worst = graph.queryWork() - before;
                    worstAt = node;
                }
            }
        }
        QVERIFY2(worst <= 3000, qPrintable(QStringLiteral("a query looked at %1 items with %2 formulas told").arg(worst).arg(worstAt)));
    }

    // A graph that is full says no at once, and goes on saying it: it does not put itself in order again for every formula it refuses (counted).
    void aFullGraphRefusesWithoutGoingOverItsWorkAgain()
    {
        DependencyGraph graph(200000);
        quint32 node = 0;
        while (node < 1000000 && graph.setPrecedents(node, {cellArea(0, node, 0), cellArea(0, node, 1), cellArea(0, node, 2)})) ++node;
        QVERIFY2(node >= 500 && node < 1000000, qPrintable(QString::number(node)));
        const qint64 before = graph.compactions();
        int refused = 0;
        for (quint32 k = 0; k < 2000; ++k)
            if (!graph.setPrecedents(node + 1 + k, {cellArea(0, node + k, 5)})) ++refused;
        QVERIFY2(refused >= 1990, qPrintable(QString::number(refused)));
        QVERIFY2(graph.compactions() - before <= 2, qPrintable(QStringLiteral("%1 compactions for 2000 refusals").arg(graph.compactions() - before)));
        // what was dropped makes room again
        for (quint32 k = 0; k < node / 2; ++k) graph.clearPrecedents(k);
        QVERIFY(graph.setPrecedents(node + 5000, {cellArea(0, 9, 9)}));
    }

    // What a formula told before is dropped when it tells again: a thousand formulas that tell the same block and a cell that moves, sixty times, hold what they read now, and a
    // query at the head of the block looks at those that read it, not at what they read before.
    void whatAFormulaToldBeforeDoesNotPileUp()
    {
        DependencyGraph graph;
        const quint32 formulas = 1000;
        for (quint32 round = 0; round < 60; ++round)
            for (quint32 node = 0; node < formulas; ++node)
                QVERIFY(graph.setPrecedents(node, {blockArea(0, 0, 0, 19999, 0), cellArea(0, node, 3), cellArea(0, 1000 + (node + round) % 5000, 5)}));
        QVERIFY2(graph.entryCount() <= 2 * 3 * formulas + 100, qPrintable(QStringLiteral("%1 entries for %2 reads").arg(graph.entryCount()).arg(3 * formulas)));
        QVERIFY2(graph.memoryBytes() <= 2 * 1000 * 1000, qPrintable(QString::number(graph.memoryBytes())));
        std::vector<quint32> got;
        const qint64 before = graph.queryWork();
        graph.dependentsOfCell(0, 0, 0, &got);
        QCOMPARE(got.size(), size_t(formulas));
        QVERIFY2(graph.queryWork() - before <= 4 * formulas + 200, qPrintable(QStringLiteral("looked at %1 items").arg(graph.queryWork() - before)));
        // a cell that read where it was, and now reads elsewhere, is not found where it was
        got.clear();
        graph.dependentsOfCell(0, 1000 + (5 + 0) % 5000, 5, &got);   // (the cell node 5 read in round 0; node 5 read 1000 + (5 + 59) % 5000 last)
        QVERIFY(std::find(got.begin(), got.end(), quint32(5)) == got.end());
    }

    // The graph holds no more bytes than it is given, whatever it is told, and takes time in proportion to what it holds (counted): narrow blocks of eight columns are
    // refused when they would take it past its bytes.
    void theGraphHoldsNoMoreBytesThanItMayAndBuildsInProportion()
    {
        const qint64 budget = qint64(24) << 20;
        DependencyGraph graph(budget);
        int refused = 0;
        quint32 told = 0;
        const qint64 work0 = graph.buildWork();
        for (quint32 n = 0; n < 300000 && refused < 100; ++n) {
            const quint32 col = n % 8000, row = (n * 13u) % 900000u;
            if (graph.setPrecedents(n, {blockArea(0, row, col, row + 1 + n % 40, col + 7)})) ++told;
            else ++refused;
        }
        QVERIFY2(refused > 0, qPrintable(QStringLiteral("it never said no: %1 formulas, %2 MB held of %3").arg(told).arg(graph.memoryBytes() >> 20).arg(budget >> 20)));
        QVERIFY2(graph.memoryBytes() <= budget, qPrintable(QStringLiteral("%1 bytes held of %2").arg(graph.memoryBytes()).arg(budget)));
        QVERIFY2(told > 10000, qPrintable(QString::number(told)));
        QVERIFY2(graph.buildWork() - work0 <= 80 * qint64(told), qPrintable(QStringLiteral("%1 steps to hold %2 formulas").arg(graph.buildWork() - work0).arg(told)));
    }

    // A formula that names more places than the graph could ever hold (a block through a thousand sheets, with columns that make many nodes of the tree on each) is refused
    // before its places are listed, and the graph is as it was.
    void aFormulaThatNamesMoreThanTheGraphCouldHoldIsRefusedAtOnce()
    {
        DependencyGraph graph(100000);
        QVERIFY(graph.setPrecedents(1, {cellArea(0, 5, 5)}));
        CellArea through = blockArea(0, 0, 1, 1000, 16000);
        through.sheet2 = 1023;
        std::vector<CellArea> many(300, through);
        QVERIFY(!graph.setPrecedents(2, many));
        QVERIFY(graph.memoryBytes() <= 100000);
        std::vector<quint32> got;
        graph.dependentsOfCell(0, 5, 5, &got);
        QCOMPARE(asSet(got), (std::set<quint32>{1}));
        got.clear();
        graph.dependentsOfCell(1000, 5, 5, &got);
        QVERIFY(got.empty());
    }

    // The list of places a formula's blocks go to, which the graph makes while it works, is counted in the bytes it takes (and let go of when it is a large one): a formula whose
    // places fit in the bytes but not with their list is refused, and the same in a graph with room for both is held without the list.
    void theListOfPlacesOfAFormulaIsCountedInTheBytes()
    {
        std::vector<CellArea> cells;
        for (quint32 i = 0; i < 200000; ++i) cells.push_back(cellArea(0, i, 3));
        DependencyGraph tight(6 << 20);   // the 200,000 places take about 4.8 MB, and their list 3.2 MB
        QVERIFY(!tight.setPrecedents(1, cells));
        QVERIFY(tight.memoryBytes() <= (6 << 20));
        DependencyGraph roomy(16 << 20);
        QVERIFY(roomy.setPrecedents(1, cells));
        QVERIFY2(roomy.memoryBytes() <= 200000 * 24 + (1 << 20), qPrintable(QString::number(roomy.memoryBytes())));
        QVERIFY(roomy.memoryBytes() <= (16 << 20));
        std::vector<quint32> got;
        roomy.dependentsOfCell(0, 123456, 3, &got);
        QCOMPARE(got.size(), size_t(1));
    }

    // Blocks that are wide are found by their columns as well as their rows: twenty thousand blocks of twelve columns that share their rows are not all looked at for a cell.
    void wideBlocksAreFoundByTheirColumnsToo()
    {
        DependencyGraph graph;
        const quint32 n = 20000;
        std::vector<CellArea> block(n);
        for (quint32 i = 0; i < n; ++i) {
            block[i] = blockArea(0, 0, i % 6000, 1000000 - i, i % 6000 + 11);
            QVERIFY(graph.setPrecedents(i, {block[i]}));
        }
        qint64 worst = 0;
        size_t most = 0;
        for (quint32 q = 0; q < 200; ++q) {
            const quint32 row = 500 + q, col = (q * 31) % 6000 + 5;
            std::vector<quint32> got;
            const qint64 before = graph.queryWork();
            graph.dependentsOfCell(0, row, col, &got);
            worst = std::max(worst, graph.queryWork() - before);
            most = std::max(most, got.size());
            std::set<quint32> expected;
            for (quint32 i = 0; i < n; ++i)
                if (covers(block[i], 0, row, col)) expected.insert(i);
            QVERIFY2(asSet(got) == expected, qPrintable(QString::number(q)));
        }
        QVERIFY2(worst <= 400 + 60 * qint64(most), qPrintable(QStringLiteral("a query looked at %1 items to find at most %2 formulas").arg(worst).arg(most)));
    }

    // The blocks a formula's references name, for the cell it is in (a shared formula moves its relative references; names of other sheets are bound).
    // A cell that very many formulas read is one query of that many: with a flag that says stop, the query ends at its next check (every 4,096 formulas found) with some of the answer and says
    // so, so that a caller that must stop soon does not wait for the whole of it; with no flag, or one that is not set, the answer is whole.
    void aQueryCanBeStoppedWhereItIs()
    {
        const int n = 20000;
        DependencyGraph g;
        for (int i = 0; i < n; ++i) QVERIFY(g.setPrecedents(quint32(i), {cellArea(0, 5, 5)}));
        std::vector<quint32> all;
        QVERIFY(g.dependentsOfCell(0, 5, 5, &all));
        QCOMPARE(int(all.size()), n);
        std::atomic<bool> stop{false};
        std::vector<quint32> whole;
        QVERIFY(g.dependentsOfCell(0, 5, 5, &whole, &stop));
        QCOMPARE(int(whole.size()), n);
        stop = true;
        std::vector<quint32> part;
        const qint64 before = g.queryWork();
        QVERIFY(!g.dependentsOfCell(0, 5, 5, &part, &stop));
        QVERIFY2(part.size() >= 1 && part.size() <= 4096, qPrintable(QString::number(part.size())));
        QVERIFY2(g.queryWork() - before < 4 * 4096, qPrintable(QString::number(g.queryWork() - before)));
        // a block too
        std::vector<quint32> blockPart;
        QVERIFY(!g.dependentsOfArea(blockArea(0, 0, 0, 10, 10), &blockPart, &stop));
        QVERIFY(blockPart.size() <= 4096);
        stop = false;
        std::vector<quint32> blockAll;
        QVERIFY(g.dependentsOfArea(blockArea(0, 0, 0, 10, 10), &blockAll, &stop));
        QCOMPARE(int(blockAll.size()), n);
    }

    void theReferencesOfAProgramAreItsBlocks()
    {
        Grid g;
        g.addSheet("Sheet2");
        ParseOptions options;
        const ParseResult p = parseFormula("A1+$B$2+SUM(C3:D4)+Sheet2!E5+SUM(Sheet1:Sheet2!F6)+Missing!G7+A:A+1:1", options);
        QVERIFY(p.ok());
        const std::vector<qint32> bound = bindSheetNames(*p.program, g);
        std::vector<CellArea> areas;
        resolveReferences(*p.program, bound, g.sheetCount(), 0, 2, 3, &areas);   // moved two rows down and three columns right
        std::sort(areas.begin(), areas.end());
        std::vector<CellArea> want = {cellArea(0, 2, 3), cellArea(0, 1, 1), blockArea(0, 4, 5, 5, 6), cellArea(1, 6, 7), blockArea(0, 0, 3, 1048575, 3), blockArea(0, 2, 0, 2, 16383)};
        CellArea three = cellArea(0, 7, 8);
        three.sheet2 = 1;
        want.push_back(three);
        std::sort(want.begin(), want.end());
        QCOMPARE(areas.size(), want.size());
        for (size_t i = 0; i < areas.size(); ++i) QVERIFY2(areas[i] == want[i], qPrintable(QString::number(i)));
        const ProgramTraits plain = traitsOf(*p.program);
        QVERIFY(!plain.usesNames && !plain.dynamic && !plain.changesByItself);
        QVERIFY(traitsOf(*parseFormula("Total*2", options).program).usesNames);
        QVERIFY(traitsOf(*parseFormula("INDIRECT(\"A1\")", options).program).dynamic);
        QVERIFY(traitsOf(*parseFormula("A1+OFFSET(A1,1,1)", options).program).readsWhereItIsTold());
        QVERIFY(traitsOf(*parseFormula("NOW()+RAND()", options).program).changesByItself);
        QVERIFY(!traitsOf(*parseFormula("NOW()+RAND()", options).program).readsWhereItIsTold());
        QVERIFY(traitsOf(*parseFormula("SUM(A1:INDEX(C:C,3))", options).program).readsWhereItIsTold());   // a block made of references that a function gives
        QVERIFY(traitsOf(*parseFormula("SUMIF(A1:A3,\">0\",B1)", options).program).readsWhereItIsTold());   // a sum range of the shape of the range
        QVERIFY(!traitsOf(*parseFormula("SUM(A1:C3)+SUMIFS(B1:B3,A1:A3,\">0\")", options).program).readsWhereItIsTold());
        // a formula that is its own row or column (ROW() and COLUMN() with nothing in the brackets) changes when an edit moves its cell; ROW(A1) reads the cell it names
        QVERIFY(traitsOf(*parseFormula("ROW()+1", options).program).observesPlace);
        QVERIFY(traitsOf(*parseFormula("IF(A1>0,COLUMN(),0)", options).program).readsWhereItIsTold());
        QVERIFY(!traitsOf(*parseFormula("ROW(A1)+COLUMN(B2)+SUM(ROW(A1:A3))", options).program).observesPlace);
    }

    // A formula that reads where its values say tells, as it is worked out, what it read: through INDIRECT, OFFSET, and a defined name, as well as by its references.
    void theEvaluationTellsWhatItRead()
    {
        Grid g;
        g.n("A1", 3).n("B3", 30).n("C5", 5);
        g.name("Far", "Sheet1!$C$5:$C$6");
        auto touched = [&](const QString &formula) {
            ParseOptions options;
            options.row = 9;
            options.col = 9;
            const ParseResult p = parseFormula(formula, options);
            Q_ASSERT(p.ok());
            std::vector<CellArea> areas;
            EvalContext ctx;
            ctx.source = &g;
            ctx.row = 9;
            ctx.col = 9;
            ctx.now = 45366.5;
            ctx.touched = &areas;
            Evaluator evaluator;
            evaluator.evaluate(*p.program, ctx);
            std::sort(areas.begin(), areas.end());
            areas.erase(std::unique(areas.begin(), areas.end()), areas.end());
            return areas;
        };
        QCOMPARE(touched("A1+1"), (std::vector<CellArea>{cellArea(0, 0, 0)}));
        QCOMPARE(touched("INDIRECT(\"B\"&A1)"), (std::vector<CellArea>{cellArea(0, 0, 0), cellArea(0, 2, 1)}));
        QCOMPARE(touched("OFFSET(A1,2,1)"), (std::vector<CellArea>{cellArea(0, 0, 0), cellArea(0, 2, 1)}));
        QCOMPARE(touched("SUM(Far)"), (std::vector<CellArea>{blockArea(0, 4, 2, 5, 2)}));
        QCOMPARE(touched("5+1"), (std::vector<CellArea>{}));
    }
    // Rows and columns inserted and deleted one after another, with the graph told nothing: it holds the ids of the lines, and an edit changes none. After every edit, what reads a cell (or a
    // block) is what asking each formula gives, if the formulas were told what they read as ids (a block that ends in an inserted line is told as the original line at that end); and
    // for a change in an inserted line, which has an id no interval has, it is a superset (the original lines either side of it are asked). A query that names no inserted line, of
    // formulas that were told no inserted line, is answered exactly. Inserted lines and deleted lines are in rows and in columns, over many edits.
    void idsKeepWhatEveryFormulaReadsThroughInsertsAndDeletes()
    {
        QRandomGenerator rng(20261027);
        struct Axis {
            LineMapCore map;
            quint32 last, maxId;
            std::vector<quint32> ids;   // the live ids in the order of their places
            Axis(const LineLimits &l, quint32 lastPlace, quint32 largest) : map(l, l.capacity), last(lastPlace), maxId(largest) { refresh(); }
            void refresh()
            {
                std::vector<IdRun> runs;
                map.walk(0, map.size() - 1, &runs);
                ids.clear();
                for (const IdRun &r : runs)
                    for (uint32_t i = 0; i < r.count; ++i) ids.push_back(r.first + i);
            }
            bool original(quint32 place) const { return place < ids.size() && ids[place] < map.limits().firstNew; }
        };
        Axis rows(LineLimits::rows(), 1048575, 0xFFFFFF), cols(LineLimits::cols(), 16383, 0xFFFF);
        const int nodes = 600;
        struct Reading {
            std::vector<CellArea> places;   // what each formula reads now, in places, as the edits so far have made it (the sheet is 0)
            bool inexact = false;           // told with an end in an inserted line, or read lines that are deleted (the graph still holds their ids)
        };
        std::vector<Reading> reads(static_cast<size_t>(nodes));
        DependencyGraph graph;
        auto randomArea = [&]() {
            CellArea a;
            const int kind = int(rng.bounded(10));
            a.sheet1 = a.sheet2 = 0;
            a.row1 = rng.bounded(60);
            a.col1 = rng.bounded(14);
            a.row2 = a.row1;
            a.col2 = a.col1;
            if (kind >= 3 && kind < 6) {
                a.row2 = a.row1 + rng.bounded(30);
                a.col2 = a.col1 + rng.bounded(4);
            } else if (kind == 6) {
                a.row1 = 0;
                a.row2 = 1048575;   // a whole column
                a.col2 = a.col1;
            } else if (kind == 7) {
                a.col1 = 0;
                a.col2 = 16383;     // a whole row
                a.row2 = a.row1;
            } else if (kind == 8) {
                a.row2 = 1048575;   // to the end of the sheet
            }
            return a;
        };
        auto tell = [&](int node) {
            Reading r;
            const int n = 1 + int(rng.bounded(3));
            std::vector<CellArea> told;
            for (int i = 0; i < n; ++i) {
                const CellArea a = randomArea();
                r.places.push_back(a);
                r.inexact = r.inexact || !rows.original(a.row1) || !rows.original(a.row2) || !cols.original(a.col1) || !cols.original(a.col2);
            }
            for (const CellArea &a : r.places) {
                CellArea t = a;
                const IdSpan rr = idSpan(rows.map, a.row1, a.row2, rows.last, rows.maxId, true), cc = idSpan(cols.map, a.col1, a.col2, cols.last, cols.maxId, true);
                t.row1 = rr.lo;
                t.row2 = rr.hi;
                t.col1 = cc.lo;
                t.col2 = cc.hi;
                told.push_back(t);
            }
            QVERIFY(graph.setPrecedents(quint32(node), told));
            reads[size_t(node)] = r;
        };
        for (int node = 0; node < nodes; ++node) tell(node);
        // what an edit of lines does to a block of places: the rule a person expects of references
        auto moved = [&](quint32 lo, quint32 hi, quint32 first, qint64 delta, quint32 last, quint32 *nlo, quint32 *nhi) {
            const quint32 count = quint32(delta < 0 ? -delta : delta), end = first + count;
            *nlo = lo;
            *nhi = hi;
            if (hi < first) return true;
            const bool toTheEnd = hi >= last;
            if (delta > 0) {
                const quint64 l = lo >= first ? quint64(lo) + count : quint64(lo);
                if (l > last) return false;
                quint64 h = quint64(hi) + count;
                if (toTheEnd || h > last) h = last;
                *nlo = quint32(l);
                *nhi = quint32(h);
                return true;
            }
            if (lo >= first && hi < end) return false;
            *nlo = lo < first ? lo : (lo < end ? first : lo - count);
            *nhi = toTheEnd ? last : (hi < end ? first - 1 : hi - count);
            return true;
        };
        int insertedCells = 0, exactChecks = 0, inexactChecks = 0;
        for (int round = 0; round < 60; ++round) {
            const bool onRows = rng.bounded(2) == 0;
            Axis &axis = onRows ? rows : cols;
            const quint32 size = axis.map.size();
            const quint32 at = rng.bounded(8) == 0 ? 0 : (rng.bounded(10) == 0 ? size - 1 - rng.bounded(3) : rng.bounded(onRows ? 70 : 16));
            const quint32 count = 1 + rng.bounded(3);
            const bool insert = rng.bounded(5) < 3;
            LineEdit edit;
            qint64 delta;
            if (insert) {
                QCOMPARE(axis.map.insert(at, count, &edit), LineStatus::Ok);
                delta = count;
            } else {
                QCOMPARE(axis.map.erase(at, count, &edit), LineStatus::Ok);
                LineEdit refill;
                QCOMPARE(axis.map.insert(axis.map.size(), count, &refill), LineStatus::Ok);   // (the sheet is as long as it was: lines are added at its end)
                delta = -qint64(count);
            }
            axis.refresh();
            for (Reading &r : reads) {
                std::vector<CellArea> now;
                for (const CellArea &a : r.places) {
                    CellArea m = a;
                    bool keep;
                    if (onRows) keep = moved(a.row1, a.row2, at, delta, rows.last, &m.row1, &m.row2);
                    else keep = moved(a.col1, a.col2, at, delta, cols.last, &m.col1, &m.col2);
                    if (keep) now.push_back(m);
                    else r.inexact = true;   // (what it read is gone, and the graph keeps the ids: a block asked about that spans the dead lines still names it, until it is told what it reads again)
                }
                r.places = std::move(now);
            }
            // some formulas are told again what they read, now that lines have been inserted among the places (and some read nothing)
            for (int i = 0; i < 40; ++i) tell(int(rng.bounded(quint32(nodes))));
            for (int i = 0; i < 4; ++i) {
                const int node = int(rng.bounded(quint32(nodes)));
                graph.clearPrecedents(quint32(node));
                reads[size_t(node)] = Reading();
            }
            auto covers = [&](const CellArea &a, quint32 r1, quint32 c1, quint32 r2, quint32 c2) { return a.row1 <= r2 && r1 <= a.row2 && a.col1 <= c2 && c1 <= a.col2; };
            for (int trial = 0; trial < 400; ++trial) {
                quint32 r1 = rng.bounded(rng.bounded(4) ? 80 : 1048576), c1 = rng.bounded(rng.bounded(4) ? 20 : 16384);
                quint32 r2 = r1, c2 = c1;
                if (rng.bounded(4) == 0) {
                    r2 = std::min<quint32>(1048575, r1 + rng.bounded(40));
                    c2 = std::min<quint32>(16383, c1 + rng.bounded(5));
                }
                bool named = true;   // (the query names only lines that were not inserted)
                for (quint32 r = r1; r <= std::min(r2, r1 + 80); ++r) named = named && rows.original(r);
                named = named && rows.original(r2);
                for (quint32 c = c1; c <= c2; ++c) named = named && cols.original(c);
                if (!named) ++insertedCells;
                CellArea q;
                q.sheet1 = q.sheet2 = 0;
                q.row1 = r1;
                q.row2 = r2;
                q.col1 = c1;
                q.col2 = c2;
                const IdSpan rr = idSpan(rows.map, r1, r2, rows.last, rows.maxId, false), cc = idSpan(cols.map, c1, c2, cols.last, cols.maxId, false);
                CellArea ids = q;
                ids.row1 = rr.lo;
                ids.row2 = rr.hi;
                ids.col1 = cc.lo;
                ids.col2 = cc.hi;
                std::vector<quint32> got;
                graph.dependentsOfArea(ids, &got);
                std::set<quint32> expected, inexact;
                for (int n = 0; n < nodes; ++n) {
                    for (const CellArea &a : reads[size_t(n)].places)
                        if (covers(a, r1, c1, r2, c2)) expected.insert(quint32(n));
                    if (reads[size_t(n)].inexact) inexact.insert(quint32(n));
                }
                const std::set<quint32> have = asSet(got);
                for (const quint32 n : expected)
                    QVERIFY2(have.count(n), qPrintable(QStringLiteral("round %1: formula %2 reads %3:%4 (%5 rows by %6 columns) and the graph does not say so (rows %7 columns %8, %9 edit at %10 by %11)")
                                                           .arg(round).arg(n).arg(r1).arg(c1).arg(r2 - r1 + 1).arg(c2 - c1 + 1).arg(named).arg(ids.row1).arg(onRows ? "rows" : "columns").arg(at).arg(delta)));
                if (named) {
                    for (const quint32 n : have)
                    {
                        QString what;
                        for (const CellArea &a : reads[size_t(n)].places) what += QStringLiteral(" [%1..%2 x %3..%4]").arg(a.row1).arg(a.row2).arg(a.col1).arg(a.col2);
                        QVERIFY2(expected.count(n) || inexact.count(n), qPrintable(QStringLiteral("round %1 (%2 at %3 by %4): the graph says formula %5 reads %6:%7 (ids rows %8..%9 columns %10..%11), which it does not (it reads%12), and it was told no inserted line").arg(round).arg(onRows ? "rows" : "columns").arg(at).arg(delta).arg(n).arg(r1).arg(c1).arg(ids.row1).arg(ids.row2).arg(ids.col1).arg(ids.col2).arg(what)));
                    }
                    ++exactChecks;
                } else {
                    ++inexactChecks;
                }
            }
        }
        QVERIFY(exactChecks > 3000);
        QVERIFY(inexactChecks > 500);
        QVERIFY(insertedCells > 500);
    }

    // A change in an inserted line reaches a formula that reads a range around it: the line has an id no interval holds, so it is asked as the original lines either side, and the
    // formulas whose blocks hold both of them are the ones that hold the gap. Blocks that end at the gap, or begin after it, are asked about it by the cell query and not by the gap.
    void aChangeInAnInsertedLineReachesTheRangeAroundIt()
    {
        RowMap rows(LineLimits::rows(), 1048576);
        ColMap cols(LineLimits::cols(), 16384);
        DependencyGraph graph;
        auto told = [&](quint32 node, quint32 r1, quint32 r2, quint32 col) {
            CellArea a = cellArea(0, r1, col);
            a.row2 = r2;
            QVERIFY(graph.setPrecedents(node, {idArea(rows, cols, a, true)}));
        };
        told(1, 0, 9, 0);    // A1:A10, which the row is put into
        told(2, 0, 4, 0);    // A1:A5, which ends above it
        told(3, 5, 9, 0);    // A6:A10, which begins at the row it is put before
        told(4, 5, 5, 0);    // A6
        told(5, 0, 1048575, 0);   // the whole column
        told(6, 0, 9, 3);    // D1:D10, another column
        RowMap::Edit edit;
        QCOMPARE(rows.insert(RowPos(5), 2, &edit), LineStatus::Ok);   // two rows between A5 and A6
        // a cell in the first inserted row (place 5), asked as the original rows on either side
        auto ask = [&](quint32 place, quint32 col) {
            std::vector<quint32> got;
            CellArea q = cellArea(0, place, col);
            graph.dependentsOfArea(idArea(rows, cols, q, false), &got);
            return asSet(got);
        };
        QCOMPARE(ask(5, 0), (std::set<quint32>{1, 2, 3, 4, 5}));   // (a superset: the blocks that end at A5 or begin at A6 are asked as well)
        QCOMPARE(ask(6, 0), (std::set<quint32>{1, 2, 3, 4, 5}));
        QCOMPARE(ask(5, 3), (std::set<quint32>{6}));
        // the formulas that hold the gap are those whose blocks contain the row either side of it
        const IdGap gap = idGap(rows.core(), 5 + 2);
        QVERIFY(gap.holdable());
        std::vector<quint32> holding;
        graph.dependentsHoldingRows(0, gap.before, gap.after, &holding);
        QCOMPARE(asSet(holding), (std::set<quint32>{1, 5, 6}));   // (any column: D1:D10 has the gap in it too)
        // a cell of an original row is asked as itself
        QCOMPARE(ask(4, 0), (std::set<quint32>{1, 2, 5}));
        QCOMPARE(ask(7, 0), (std::set<quint32>{1, 3, 4, 5}));
        // a gap at the top, or at the end, is inside no block
        QCOMPARE(rows.insert(RowPos(0), 1, &edit), LineStatus::Ok);
        const IdGap top = idGap(rows.core(), 1);
        QVERIFY(!top.holdable());
        holding.clear();
        graph.dependentsHoldingRows(0, top.before, top.after, &holding);
        QVERIFY(holding.empty());
    }

    // The traits that say what an edit of whole lines can change in a formula: whether a call can notice an empty row or column put inside a block it reads (blank-insensitive: every block goes
    // to a function on the allowlist, and nowhere else; the default is sensitive), and whether a call can see where cells are, at any depth.
    void theTraitsSayWhichFormulasNoticeAnEmptyLineAndWhichSeeWhereCellsAre()
    {
        ParseOptions options;
        auto insensitive = [&](const char *text) {
            const ParseResult p = parseFormula(QString::fromLatin1(text), options);
            Q_ASSERT(p.ok());
            return traitsOf(*p.program).blankInsensitive;
        };
        auto sees = [&](const char *text) {
            const ParseResult p = parseFormula(QString::fromLatin1(text), options);
            Q_ASSERT(p.ok());
            return traitsOf(*p.program).observesPosition;
        };
        for (const char *text : {"SUM(A1:E1)", "SUM(A1:A5,C1:C5)+MAX(B1:B3)*2", "IF(A1>0,SUM(B1:B10),0)", "IFERROR(SUM(A1:A5),0)", "IFNA(AVERAGE(A1:A5),MIN(B1:B2))", "CHOOSE(2,SUM(A1:A3),MAX(B1:B3))",
                                 "SUM(IF(A1>0,A1:A5,B1:B5))", "LARGE(A1:A9,2)", "SUBTOTAL(9,A1:A5)", "SUM(A1:A5)/COUNT(A1:A5)", "A1+1", "5", "SUM(A:A)", "SUM(1:1)", "COUNTA(A1:A5)&\"x\"",
                                 "IF(SUM(A1:A5)>1,IF(A1>0,MAX(B1:B5),MIN(B1:B5)),SUM(C1:C5))", "SUM(Sheet2!A1:A5)"})
            QVERIFY2(insensitive(text), text);
        // sensitive: a function that counts or places, a block under an operator or as the answer, a block given where it is taken by its place, a name, a table, a function nobody classified
        for (const char *text : {"COUNTBLANK(A1:A5)", "ROWS(A1:A5)", "COLUMNS(A1:E1)", "INDEX(A1:A5,2)", "MATCH(3,A1:A5,0)", "VLOOKUP(2,A1:B5,2,FALSE)", "SUMPRODUCT(A1:A5,B1:B5)", "SUMIF(A1:A5,\">0\")",
                                 "SUMIFS(B1:B3,A1:A3,\">0\")", "COUNTIF(A1:A5,\"\")", "SUM(A1:A5*2)", "SUM(A1:A5)+A1:A5", "IF(A1>0,A1:A5,0)", "SUM(A1:A5)&COUNTBLANK(B1:B5)", "Total", "SUM(Total)",
                                 "SUM(A1:INDEX(C:C,3))", "LARGE(A1:A9,B1:B3)", "OFFSET(A1,1,1)", "SUM(A1:A5,(C1:C5,D1:D5))", "SUM(IF(A1>0,A1:A5,B1:B5))*COUNTBLANK(A1:A3)", "SUM(-A1:A5)",
                                 "CHOOSE(2,SUM(A1:A3),COUNTBLANK(B1:B3))", "IFERROR(COUNTBLANK(A1:A5),0)", "UNKNOWNFUNCTION(A1:A5)", "SUM(TRANSPOSE(A1:A5))"})
            QVERIFY2(!insensitive(text), text);
        for (const char *text : {"ROW(B5)", "COLUMN()", "SUMPRODUCT(ROW(A1:A10))", "IF(A1>0,ADDRESS(1,1),0)", "CELL(\"row\",A1)", "OFFSET(A1,1,1)", "INDIRECT(\"A1\")", "SUM(IF(A1>0,COLUMN(B2:D2),0))", "ROW()"})
            QVERIFY2(sees(text), text);
        for (const char *text : {"A1+1", "SUM(A1:A5)", "ROWS(A1:A5)", "INDEX(A1:A5,2)", "COUNTBLANK(A1:A5)", "MATCH(3,A1:A5,0)", "5", "SUMPRODUCT(A1:A5,B1:B5)", "IF(A1>0,SUM(B1:B5),0)"})
            QVERIFY2(!sees(text), text);
        // a block taken for one value of it, the one in the row or the column of the cell the formula is in (implicit intersection), sees where that cell is
        for (const char *text : {"Sheet2!A3:C3", "A1:A5*2", "IF(A1:A5>1,1,0)", "LARGE(A1:A9,B1:B3)", "A1:A5", "SUM(A1:A5*2)", "ABS(A1:A5)", "-A1:A5", "CHOOSE(2,1,A1:A5)"})
            QVERIFY2(sees(text), text);
        // a name stands for what it stands for: one that has ROW() in it is seen through, and a formula that uses a name is never blank-insensitive
        Grid g;
        g.name("MyRow", "ROW(Sheet1!$A$5)");
        g.name("Plain", "Sheet1!$A$1:$A$5");
        const ParseResult p = parseFormula(QStringLiteral("MyRow+1"), options);
        QVERIFY(p.ok());
        const ProgramTraits t = traitsFollowingNames(*p.program, bindSheetNames(*p.program, g), 0, g);
        QVERIFY(t.observesPosition);
        QVERIFY(!t.blankInsensitive);
        const ParseResult q = parseFormula(QStringLiteral("SUM(Plain)"), options);
        const ProgramTraits u = traitsFollowingNames(*q.program, bindSheetNames(*q.program, g), 0, g);
        QVERIFY(!u.blankInsensitive);   // (and what a name stands for may be a block that is taken for one value: the formula is looked at, which is more and not less)
    }

    // The allowlist is the functions that skip the empty cells they are given and do not count where they are: each is shown to give the same value for a block with an empty row or column put
    // inside it, over random cells (numbers, empty cells, text, logical values); and the same test shows COUNTBLANK and ROWS (not on the list) can tell.
    void theFunctionsOnTheBlankInsensitiveListSkipAnEmptyLinePutInsideTheirBlocks()
    {
        QStringList listed;
        for (int i = 0; i < functionCount(); ++i)
            if (blankInsensitiveFunction(functionAt(i).name)) listed << QString::fromLatin1(functionAt(i).name);
        listed.sort();
        QCOMPARE(listed, (QStringList{"AVERAGE", "AVERAGEA", "COUNT", "COUNTA", "LARGE", "MAX", "MAXA", "MEDIAN", "MIN", "MINA", "PRODUCT", "SMALL", "STDEV", "STDEV.P", "STDEV.S", "STDEVP", "SUBTOTAL", "SUM",
                                      "SUMSQ", "VAR", "VAR.P", "VAR.S", "VARP"}));
        QRandomGenerator rng(20261028);
        auto fill = [&](Grid &g, const QStringList &cells, const std::vector<int> &kinds, const std::vector<double> &numbers) {
            for (size_t i = 0; i < cells.size(); ++i) {
                switch (kinds[i]) {
                case 0:
                case 1:
                case 2: g.n(cells[int(i)], numbers[i]); break;
                case 3: g.s(cells[int(i)], QStringLiteral("x")); break;
                case 4: g.b(cells[int(i)], numbers[i] > 25); break;
                default: break;   // (empty)
                }
            }
        };
        auto cellName = [](bool down, int line, int offset) { return down ? QStringLiteral("%1%2").arg(QChar(char('A' + offset))).arg(line + 1) : QStringLiteral("%1%2").arg(QChar(char('A' + line))).arg(offset + 1); };
        int compared = 0, countBlankDiffers = 0, rowsDiffers = 0;
        for (int trial = 0; trial < 300; ++trial) {
            const bool down = rng.bounded(2) == 0;
            const int n = 6, at = 1 + int(rng.bounded(n - 1));   // (the empty line goes between line at - 1 and line at)
            std::vector<int> kinds;
            std::vector<double> numbers;
            for (int k = 0; k < 2 * n; ++k) {
                kinds.push_back(int(rng.bounded(6)));
                numbers.push_back(double(1 + rng.bounded(50)));
            }
            QStringList before, after, beforeOther, afterOther;
            for (int line = 0; line < n; ++line) {
                before << cellName(down, line, 0);
                beforeOther << cellName(down, line, 1);
                after << cellName(down, line < at ? line : line + 1, 0);
                afterOther << cellName(down, line < at ? line : line + 1, 1);
            }
            Grid g1, g2;
            std::vector<int> kinds1(kinds.begin(), kinds.begin() + n), kinds2(kinds.begin() + n, kinds.end());
            std::vector<double> numbers1(numbers.begin(), numbers.begin() + n), numbers2(numbers.begin() + n, numbers.end());
            fill(g1, before, kinds1, numbers1);
            fill(g1, beforeOther, kinds2, numbers2);
            fill(g2, after, kinds1, numbers1);
            fill(g2, afterOther, kinds2, numbers2);
            const QString block1 = down ? QStringLiteral("A1:A%1").arg(n) : QStringLiteral("A1:%1").arg(cellName(false, n - 1, 0)), block2 = down ? QStringLiteral("A1:A%1").arg(n + 1) : QStringLiteral("A1:%1").arg(cellName(false, n, 0));
            const QString other1 = down ? QStringLiteral("B1:B%1").arg(n) : QStringLiteral("A2:%1").arg(cellName(false, n - 1, 1)), other2 = down ? QStringLiteral("B1:B%1").arg(n + 1) : QStringLiteral("A2:%1").arg(cellName(false, n, 1));
            QStringList calls1, calls2;
            for (const QString &name : listed) {
                if (name == QLatin1String("SUBTOTAL")) {
                    for (const int k : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111}) {
                        calls1 << QStringLiteral("SUBTOTAL(%1,%2)").arg(k).arg(block1);
                        calls2 << QStringLiteral("SUBTOTAL(%1,%2)").arg(k).arg(block2);
                    }
                } else if (name == QLatin1String("LARGE") || name == QLatin1String("SMALL")) {
                    calls1 << QStringLiteral("%1(%2,2)").arg(name, block1);
                    calls2 << QStringLiteral("%1(%2,2)").arg(name, block2);
                } else {
                    calls1 << QStringLiteral("%1(%2)").arg(name, block1) << QStringLiteral("%1(%2,%3)").arg(name, block1, other1);
                    calls2 << QStringLiteral("%1(%2)").arg(name, block2) << QStringLiteral("%1(%2,%3)").arg(name, block2, other2);
                }
            }
            for (int i = 0; i < calls1.size(); ++i) {
                QCOMPARE(ev(calls2[i], g2), ev(calls1[i], g1));
                ++compared;
            }
            if (ev(QStringLiteral("COUNTBLANK(%1)").arg(block2), g2) != ev(QStringLiteral("COUNTBLANK(%1)").arg(block1), g1)) ++countBlankDiffers;
            if (ev(QStringLiteral("%1(%2)").arg(down ? "ROWS" : "COLUMNS", block2), g2) != ev(QStringLiteral("%1(%2)").arg(down ? "ROWS" : "COLUMNS", block1), g1)) ++rowsDiffers;
        }
        QVERIFY(compared > 15000);
        QCOMPARE(countBlankDiffers, 300);   // (a function that is not on the list can tell)
        QCOMPARE(rowsDiffers, 300);
    }

    // A formula can be marked as it is told, and a query can leave the marked out: asking for the others answers what the others are, whether one in ten or all but one is marked, and a formula
    // that is told again with another mark has it from then on.
    void aQueryCanLeaveOutMarkedFormulas()
    {
        DependencyGraph graph;
        std::set<quint32> others;
        for (quint32 n = 0; n < 1000; ++n) {
            QVERIFY(graph.setPrecedents(n, {blockArea(0, 0, 2, 100, 5)}, n % 10 != 0));
            if (n % 10 == 0) others.insert(n);
        }
        std::vector<quint32> got;
        QVERIFY(graph.dependentsOfArea(blockArea(0, 3, 4, 4, 6), &got, nullptr, true));
        QCOMPARE(asSet(got), others);
        got.clear();
        QVERIFY(graph.dependentsOfCell(0, 50, 5, &got, nullptr, true));
        QCOMPARE(asSet(got), others);
        got.clear();
        QVERIFY(graph.dependentsHoldingRows(0, 10, 20, &got, nullptr, true));
        QCOMPARE(asSet(got), others);
        got.clear();
        QVERIFY(graph.dependentsOfCell(0, 50, 5, &got));
        QCOMPARE(got.size(), size_t(1000));   // (the marked are found when the query does not leave them out)
        std::set<quint32> seen;
        QVERIFY(graph.forEachDependent(blockArea(0, 3, 4, 4, 6), [&](quint32 n) { return seen.insert(n).second; }, true));
        QCOMPARE(seen, others);
        // told again with another mark (the areas are the same: it is not taken for what it told before)
        QVERIFY(graph.setPrecedents(0, {blockArea(0, 0, 2, 100, 5)}, true));
        QVERIFY(graph.setPrecedents(1, {blockArea(0, 0, 2, 100, 5)}, false));
        others.erase(0);
        others.insert(1);
        got.clear();
        QVERIFY(graph.dependentsOfCell(0, 50, 5, &got, nullptr, true));
        QCOMPARE(asSet(got), others);
        // dropped formulas are in neither
        graph.clearPrecedents(1);
        graph.clearPrecedents(10);
        others.erase(1);
        others.erase(10);
        got.clear();
        QVERIFY(graph.dependentsOfCell(0, 50, 5, &got, nullptr, true));
        QCOMPARE(asSet(got), others);
    }

    // The column tree has a leaf for each of the 65,536 column ids: a block of the columns that inserted columns were given is found, and a whole row is one node.
    void theColumnTreeHoldsTheIdsOfInsertedColumns()
    {
        DependencyGraph graph;
        QVERIFY(graph.setPrecedents(1, {blockArea(0, 5, 16384, 6, 16390)}));    // columns with ids past the grid's
        QVERIFY(graph.setPrecedents(2, {blockArea(0, 5, 0, 5, 65535)}));         // a whole row
        QVERIFY(graph.setPrecedents(3, {cellArea(0, 5, 65535)}));
        std::vector<quint32> got;
        graph.dependentsOfCell(0, 5, 16387, &got);
        QCOMPARE(asSet(got), (std::set<quint32>{1, 2}));
        got.clear();
        graph.dependentsOfCell(0, 5, 65535, &got);
        QCOMPARE(asSet(got), (std::set<quint32>{2, 3}));
        got.clear();
        graph.dependentsOfCell(0, 5, 70000, &got);
        QVERIFY(got.empty());
        QVERIFY2(graph.entryCount() <= 8, qPrintable(QString::number(graph.entryCount())));
    }

    // Asking one at a time ends where the visitor says: whether anything reads a block costs the first one found, not all of them.
    void aVisitorEndsTheSearchAtTheFirstFormula()
    {
        DependencyGraph graph;
        for (quint32 n = 0; n < 5000; ++n) QVERIFY(graph.setPrecedents(n, {blockArea(0, 0, 0, 100, 0)}));
        qint64 before = graph.queryWork();
        int seen = 0;
        QVERIFY(!graph.forEachDependent(blockArea(0, 3, 0, 4, 0), [&](quint32) { return ++seen < 1; }));
        QCOMPARE(seen, 1);
        QVERIFY2(graph.queryWork() - before < 100, qPrintable(QString::number(graph.queryWork() - before)));
        seen = 0;
        before = graph.queryWork();
        QVERIFY(graph.forEachDependent(blockArea(0, 3, 0, 4, 0), [&](quint32) { return ++seen > 0; }));
        QCOMPARE(seen, 5000);
    }

};

JP_TEST_CLASS(SheetGraphTests)
#include "tst_sheet_graph.moc"
