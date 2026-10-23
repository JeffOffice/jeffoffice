// Tests of the dependency graph (calc/graph.h): what reads a cell, found from single cells and from blocks kept as blocks, checked against asking every formula; a formula
// that tells again what it reads; blocks that are shared; the limit on what the graph holds; and that a million-row column is one entry, not a million.
#include "calchelpers.h"

#include "calc/graph.h"
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
        QVERIFY2(refused > 0 && graph.memoryBytes() <= budget + 4096, qPrintable(QStringLiteral("%1 refused, %2 bytes held of %3").arg(refused).arg(graph.memoryBytes()).arg(budget)));
        // dropped entries are let go of to make room
        for (quint32 n = 0; n < 600; ++n) graph.clearPrecedents(n);
        QVERIFY(graph.setPrecedents(700, {cellArea(0, 1, 1), cellArea(0, 2, 2)}));
        std::vector<quint32> got;
        graph.dependentsOfCell(0, 1, 1, &got);
        QCOMPARE(asSet(got), (std::set<quint32>{700}));
        // a formula refused is told nothing: what it told before is gone
        DependencyGraph small(1000);
        QVERIFY(small.setPrecedents(1, {cellArea(0, 0, 0), cellArea(0, 1, 0)}));
        std::vector<CellArea> more;
        for (quint32 r = 5; r < 200; ++r) more.push_back(cellArea(0, r, 0));
        QVERIFY(!small.setPrecedents(1, more));
        got.clear();
        small.dependentsOfCell(0, 0, 0, &got);
        QVERIFY(got.empty());
        QVERIFY(small.memoryBytes() <= 1000 + 4096);
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
        QVERIFY2(graph.memoryBytes() <= budget + budget / 4, qPrintable(QStringLiteral("%1 MB held of %2").arg(graph.memoryBytes() >> 20).arg(budget >> 20)));
        QVERIFY2(told > 10000, qPrintable(QString::number(told)));
        QVERIFY2(graph.buildWork() - work0 <= 80 * qint64(told), qPrintable(QStringLiteral("%1 steps to hold %2 formulas").arg(graph.buildWork() - work0).arg(told)));
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
};

JP_TEST_CLASS(SheetGraphTests)
#include "tst_sheet_graph.moc"
