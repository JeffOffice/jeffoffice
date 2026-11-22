// Tests of spills: a formula in one cell whose result is a block fills the cells below and beside it (sheet/calcspill.cpp), the spill range operator A1#, LET in a cell, and the file form
// of a formula that spills (an array formula with the cell metadata that marks a dynamic array). Every workbook is built in memory; every expected value is worked out by hand.
#include "sheethelpers.h"

#include "sheet/calculation.h"
#include "sheet/xlsxsave.h"

#include <memory>

namespace {

// Whether the tests of the index below run with it on (they are written to fail with it off: the cells the store holds are not the blocks').
constexpr bool kIndexOn = true;

QString constant(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }
QString rowXml(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

// A workbook read into memory, and a calculation over it.
struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool open(const QByteArray &zip)
    {
        QString why;
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        calc.reset(new Calculation(book));
        return calc->load() && settle();
    }
    bool settle() { return calc->recalculate().finished; }
    quint32 row(const QString &ref) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return r;
    }
    quint32 col(const QString &ref) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return c;
    }
    // What the cell holds as the engine says it: "n:1.5", "s:text", "e:#SPILL!", "-" for none.
    QString at(const QString &ref, int sheet = 0) const { return calc->cell(sheet, row(ref), col(ref)).debug(); }
    // A formula typed in a cell, as the window enters it: it spills when it can give a block.
    bool type(const QString &ref, const QString &formula, int sheet = 0)
    {
        const bool ok = calc->setFormula(sheet, row(ref), col(ref), formula, Calculation::Marking::Now, nullptr, Calculation::formulaCanSpill(formula));
        return ok && settle();
    }
    bool put(const QString &ref, const Value &v, int sheet = 0) { return calc->setValue(sheet, row(ref), col(ref), v) && settle(); }
    bool member(const QString &ref, int sheet = 0) const { return calc->isSpillMember(sheet, row(ref), col(ref)); }
    QString block(const QString &ref, int sheet = 0) const
    {
        const CellRange r = calc->spillBlock(sheet, row(ref), col(ref));
        return r.valid ? rangeName(r) : QString();
    }
};

QByteArray blank(const QString &rows = QString()) { return buildXlsx({worksheet(rows)}); }

// The metadata part a file has for dynamic arrays (cell metadata 1).
QByteArray dynamicMetadata() { return dynamicArrayMetadataXml(); }

QByteArray withMetadata(const QByteArray &xlsx) { return withParts(xlsx, {{QStringLiteral("xl/metadata.xml"), dynamicMetadata(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheetMetadata+xml"), -1, QString(), QString(), QString()}}); }

} // namespace

class SheetSpillTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A formula that can give a block is one that spills; one that gives a value is not made one, so what it does is what it did.
    void aFormulaThatCanGiveABlockIsOneThatSpills()
    {
        for (const char *f : {"_xlfn.SEQUENCE(3)", "_xlfn._xlws.SORT(A1:A9)", "_xlfn._xlws.FILTER(A1:A9,B1:B9>1)", "_xlfn.UNIQUE(A1:A9)", "A1:A3*2", "A1#", "SUM(A1#)", "{1,2,3}", "TRANSPOSE(A1:C1)", "IF(A1>1,_xlfn.SEQUENCE(2),0)", "-A1:A3"})
            QVERIFY2(Calculation::formulaCanSpill(QString::fromLatin1(f)), f);
        for (const char *f : {"SUM(A1:A3)", "A1+B1", "1+2", "SUM(A1:A3)*2", "VLOOKUP(A1,B1:C3,2,FALSE)", "A1", "INDEX(A1:A3,2)", "A1:A3"})
            QVERIFY2(!Calculation::formulaCanSpill(QString::fromLatin1(f)), f);
    }

    // The cells below fill with the values; they hold no formula of their own, and the cell after the block is empty.
    void aFormulaWhoseResultIsABlockFillsTheCellsBelowIt()
    {
        Book b;
        QVERIFY(b.open(blank()));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(3)")));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:2"));
        QCOMPARE(b.at("A3"), QStringLiteral("n:3"));
        QCOMPARE(b.at("A4"), QStringLiteral("-"));
        QCOMPARE(b.block("A1"), QStringLiteral("A1:A3"));
        QVERIFY(b.calc->isSpillAnchor(0, 0, 0));
        QVERIFY(b.member("A2") && b.member("A3") && !b.member("A1") && !b.member("A4"));
        const Calculation::CellContent filled = b.calc->content(0, b.row("A2"), b.col("A2"));
        QVERIFY(filled.spilled && !filled.isFormula && filled.value.isEmpty());
        QVERIFY(b.calc->content(0, 0, 0).dynamic && b.calc->content(0, 0, 0).isFormula);
        // beside it, too
        QVERIFY(b.type(QStringLiteral("C1"), QStringLiteral("_xlfn.SEQUENCE(2,3)")));
        QCOMPARE(b.at("E2"), QStringLiteral("n:6"));
        QCOMPARE(b.block("C1"), QStringLiteral("C1:E2"));
    }

    // The index of the blocks (sheet/spillindex.h) finds the block that holds a place, the blocks that meet a rectangle, and lets go of a block.
    void theSpillTableFindsMeetsAndLetsGo()
    {
        SpillTable t;
        QVERIFY(t.empty());
        QVERIFY(t.set(SpillRect{0, 0, 2, 0, 1}));
        QVERIFY(t.set(SpillRect{10, 2, 20000, 2, 2}));   // (a tall one, one column wide)
        QVERIFY(t.set(SpillRect{100, 5, 163, 9, 3}));
        QCOMPARE(t.size(), size_t(3));
        QVERIFY(t.at(2, 0) && t.at(2, 0)->formula == 1 && !t.at(3, 0) && !t.at(2, 1));
        QVERIFY(t.at(19000, 2) && t.at(19000, 2)->formula == 2);
        QVERIFY(t.at(163, 9) && t.at(163, 9)->formula == 3 && !t.at(164, 9));
        QVERIFY(t.meetsOther(1, 0, 40, 3, 1));   // (the tall one is in it)
        QVERIFY(!t.meetsOther(1, 0, 4, 1, 1));
        QVERIFY(t.meetsOther(1, 0, 4, 1, 99));
        int count = 0;
        t.forEachMeeting(0, 0, 200, 20, [&](const SpillRect &) { ++count; });
        QCOMPARE(count, 3);   // (a block that is in several buckets is met once)
        count = 0;
        t.forEachMeeting(0, 0, 200000, 1000000, [&](const SpillRect &) { ++count; return count < 2; });
        QCOMPARE(count, 2);   // (a walk that gives false stops)
        QVERIFY(t.set(SpillRect{100, 5, 100, 5, 3}));   // (the block of a formula is replaced)
        QVERIFY(!t.at(163, 9) && t.at(100, 5));
        t.erase(2);
        QVERIFY(!t.at(19000, 2) && t.size() == 2);
    }

    // `at` and `meetsOther` look at a few blocks however many there are (a bucket holds at most 256 disjoint ones, and these hold 8): 500 one-column blocks side by side in a header row, and 200
    // columns of 5,000 rows. A scan of the blocks of the rows (the first form) looks at 500 and at 200.
    void theSpillTableLooksAtFewBlocksWhenThereAreManyInARowOrAColumn()
    {
        SpillTable header;
        for (qint32 c = 0; c < 500; ++c) QVERIFY(header.set(SpillRect{1, quint32(c), 20, quint32(c), c}));   // (A2:A21, B2:B21, ...)
        SpillTable wide;
        for (qint32 c = 0; c < 200; ++c) QVERIFY(wide.set(SpillRect{1, quint32(c), 5000, quint32(c), c}));
        qint64 probes = 0, mostForOne = 0;
        for (const SpillTable *t : {&header, &wide}) {
            const quint32 cols = t == &header ? 500 : 200, last = t == &header ? 20 : 5000;
            for (quint32 c = 0; c < cols; ++c)
                for (const quint32 r : {quint32(1), quint32(last / 2), last}) {
                    qint64 one = 0;
                    const SpillRect *found = t->at(r, c, &one);
                    QVERIFY(found && found->formula == qint32(c));
                    probes += one;
                    mostForOne = std::max(mostForOne, one);
                }
        }
        QVERIFY2(mostForOne <= 8, qPrintable(QString::number(mostForOne)));   // (8 columns a bucket)
        QCOMPARE(header.at(0, 3), static_cast<const SpillRect *>(nullptr));
        // a block's own place is met by a walk over a rectangle on it, and one beside it is not; each counts what it looked at
        qint64 meet = 0;
        QVERIFY(header.meetsOther(10, 250, 10, 250, -1, &meet));
        QVERIFY(meet <= 8);
        meet = 0;
        QVERIFY(!header.meetsOther(22, 0, 30, 499, -1, &meet));
        QVERIFY(meet <= 500);   // (the one bucket row of rows 22 to 30 holds all 500 blocks: they are looked at once each, not once each for every place)
        meet = 0;
        QVERIFY(!wide.meetsOther(1, 200, 5000, 260, -1, &meet));
        QCOMPARE(meet, qint64(0));
        QVERIFY(probes > 0);
    }

    // A block of more than kMaxBuckets buckets is refused (the largest a spill can be, 200,000 cells in a column, is under it).
    void theSpillTableRefusesABlockOfTooManyBuckets()
    {
        SpillTable t;
        QVERIFY(t.set(SpillRect{0, 0, quint32(kMaxSpillCells) - 1, 0, 1}));
        QVERIFY(!t.set(SpillRect{0, 5, 1000000, 40, 2}));   // (about 15,600 by 5 buckets)
        QVERIFY(!t.ofFormula(2));
        QVERIFY(!t.set(SpillRect{4, 0, 3, 0, 3}));
        QCOMPARE(t.size(), size_t(1));
        // (a formula whose new block is refused has no block: the old one was let go of)
        QVERIFY(t.set(SpillRect{0, 1, 5, 1, 2}));
        QVERIFY(!t.set(SpillRect{0, 5, 1000000, 40, 2}));
        QVERIFY(!t.ofFormula(2) && t.size() == 1);
    }

    // A store with a table of blocks answers for the places of the blocks: find, findById, forEachInRange, and forEach give them in order, as members of the formula; a walk that gives false stops; a copy
    // of the store keeps the blocks it was made with. A store with no table (every store, unless the index is on) answers for the cells it holds.
    void aStoreWithBlocksAnswersForTheirPlacesInOrder()
    {
        MutationGuard guard;
        CellStore plain;
        Cell a;
        a.type = CellType::Number;
        a.number = 5;
        plain.set(0, 0, a);
        plain.set(0, 3, a);
        QVERIFY(!plain.spills());
        CellStore cells = plain;
        cells.enableSpills(&guard);
        QVERIFY(cells.spills());
        QVERIFY(!cells.find(1, 0));   // (nothing in the table yet)
        cells.spills()->mine().set(SpillRect{0, 0, 2, 0, 7});   // A1:A3: its first cell is a real one
        cells.spills()->mine().set(SpillRect{0, 2, 1, 3, 8});   // C1:D2: D1 holds a number of its own
        cells.spills()->noteChanged();
        const std::optional<Cell> member = cells.find(2, 0);
        QVERIFY(member && (member->flags & SpillMember) && (member->flags & ArrayMember) && member->formula == 7 && member->type == CellType::Empty);
        QVERIFY(cells.findById(member->rowId, member->colId) && cells.findById(member->rowId, member->colId)->formula == 7);
        QVERIFY(!cells.find(3, 0) && !cells.find(2, 1));
        QVERIFY(cells.find(0, 3) && cells.find(0, 3)->type == CellType::Number && !(cells.find(0, 3)->flags & SpillMember));   // (a cell that holds something is itself)
        std::vector<QString> seen;
        cells.forEachInRange(0, 0, 5, 5, [&](quint32 row, quint32 col, const Cell &c) {
            seen.push_back(QStringLiteral("%1:%2:%3").arg(row).arg(col).arg((c.flags & SpillMember) ? c.formula : -1));
            return true;
        });
        const std::vector<QString> want = {QStringLiteral("0:0:-1"), QStringLiteral("0:2:8"), QStringLiteral("0:3:-1"), QStringLiteral("1:0:7"), QStringLiteral("1:2:8"), QStringLiteral("1:3:8"), QStringLiteral("2:0:7")};
        // (A1 holds a number of its own here, D1 too; the others are the blocks')
        QCOMPARE(seen, want);
        std::vector<QString> all;
        cells.forEach([&](quint32 row, quint32 col, const Cell &) { all.push_back(QStringLiteral("%1:%2").arg(row).arg(col)); });
        QCOMPARE(all.size(), want.size());
        int calls = 0;
        cells.forEachInRange(0, 0, 5, 5, [&](quint32, quint32, const Cell &) { return ++calls < 3; });
        QCOMPARE(calls, 3);
        const CellStore copy = cells;
        cells.spills()->mine().erase(7);
        cells.spills()->noteChanged();
        QVERIFY(!cells.find(2, 0));
        QVERIFY(copy.find(2, 0) && (copy.find(2, 0)->flags & SpillMember));   // (the copy has the block the store had)
        QCOMPARE(plain.cellCount(), qint64(2));
    }

    // With the index off, which is how it is unless it is switched on, a spill makes the cells of its block in the store, as it always did.
    void theIndexIsOffByDefaultAndASpillMakesCells()
    {
        if (qEnvironmentVariableIntValue("JO_SPILL_INDEX") != 0) QSKIP("the process runs with the index on");
        QVERIFY(!spillIndexDefault());
        Book b;
        QVERIFY(b.open(blank()));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(3)")));
        QVERIFY(!b.calc->sheet(0).cells.spills());
        QCOMPARE(b.calc->sheet(0).cells.cellCount(), qint64(3));
    }

    // A lookup that was cached over cells that a spill grows into reads them as they are now (the spill epoch: a change of the blocks moves the count of writes of the store).
    void aCachedLookupSeesCellsASpillGrowsInto()
    {
        SpillIndexSwitch on(kIndexOn);
        QString rows;
        for (int n = 11; n <= 500; ++n) rows += rowXml(n, constant(QStringLiteral("B%1").arg(n), QString::number(n)));
        Book b;
        QVERIFY(b.open(blank(rows)));
        QVERIFY(b.type(QStringLiteral("D1"), QStringLiteral("MATCH(10,B1:B500,0)")));
        QCOMPARE(b.at("D1"), QStringLiteral("e:#N/A"));   // (B1:B10 are empty: the lookup read the 490 numbers and cached them)
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(10,2)")));   // A1:B10: B5 is 10
        QCOMPARE(b.at("B5"), QStringLiteral("n:10"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:5"));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(4,2)")));   // it shrinks: B5 is empty again
        QCOMPARE(b.at("D1"), QStringLiteral("e:#N/A"));
    }

    // A loaded spill, and the blocks table made again from the ranges, agree: the same blocks for a spill that is worked out, and none for one that is kept apart (its cells are the file's).
    void aLoadedSpillAndTheRebuiltTableAgreeOnKeptFormulas()
    {
        SpillIndexSwitch on(kIndexOn);
        const QString rows = rowXml(1, QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:A3\">_xlfn.SEQUENCE(3)</f><v>1</v></c><c r=\"C1\" cm=\"1\"><f t=\"array\" ref=\"C1:C3\">_xlfn.NOSUCHFUNCTIONX(3)</f><v>1</v></c>"))
            + rowXml(2, constant("A2", "2") + constant("C2", "2")) + rowXml(3, constant("A3", "3") + constant("C3", "3"));
        const QByteArray file = withMetadata(blank(rows));
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(file, &why) && b.book.open(b.package, &why));
        CalcOptions options;
        options.keepApart = true;
        b.calc.reset(new Calculation(b.book, options));
        QVERIFY(b.calc->load());
        const SpillPublisher *pub = b.calc->sheet(0).cells.spills();
        QVERIFY(pub);
        auto blocks = [&] {
            std::vector<QString> out;
            pub->mine().forEach([&](const SpillRect &r) { out.push_back(QStringLiteral("%1:%2:%3:%4").arg(r.row1).arg(r.col1).arg(r.row2).arg(r.col2)); });
            std::sort(out.begin(), out.end());
            return out;
        };
        const std::vector<QString> loaded = blocks();
        QCOMPARE(loaded, std::vector<QString>{QStringLiteral("0:0:2:0")});   // (A1:A3 only: C1:C3 is kept apart)
        b.calc->rebuildSpillTableForTest(0);
        QCOMPARE(blocks(), loaded);
        QVERIFY(b.settle());
        QCOMPARE(blocks(), loaded);
    }

    // The block grows and shrinks as the result does, and what reads the cells follows it.
    void theBlockGrowsAndShrinksWithTheResult()
    {
        Book b;
        QVERIFY(b.open(blank(rowXml(1, constant("B1", "3")))));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(B1)")));
        QVERIFY(b.type(QStringLiteral("D1"), QStringLiteral("SUM(A1:A9)")));
        QVERIFY(b.type(QStringLiteral("D2"), QStringLiteral("A5+100")));
        QCOMPARE(b.at("D1"), QStringLiteral("n:6"));
        QCOMPARE(b.at("D2"), QStringLiteral("n:100"));
        QVERIFY(b.put(QStringLiteral("B1"), Value::number(5)));
        QCOMPARE(b.block("A1"), QStringLiteral("A1:A5"));
        QCOMPARE(b.at("A5"), QStringLiteral("n:5"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:15"));
        QCOMPARE(b.at("D2"), QStringLiteral("n:105"));
        QVERIFY(b.put(QStringLiteral("B1"), Value::number(2)));
        QCOMPARE(b.block("A1"), QStringLiteral("A1:A2"));
        QCOMPARE(b.at("A3"), QStringLiteral("-"));
        QVERIFY(!b.member("A3") && !b.member("A5") && b.member("A2"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:3"));
        QCOMPARE(b.at("D2"), QStringLiteral("n:100"));
        // a result of one value is a formula like any other
        QVERIFY(b.put(QStringLiteral("B1"), Value::number(1)));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1"));
        QVERIFY(!b.member("A2"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:1"));
    }

    // A cell in the way gives #SPILL! and nothing is filled; clearing it fills the cells.
    void aCellInTheWayGivesSpillAndClearingItFillsTheCells()
    {
        Book b;
        QVERIFY(b.open(blank(rowXml(2, constant("A2", "9")))));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(3)")));
        QCOMPARE(b.at("A1"), QStringLiteral("e:#SPILL!"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:9"));
        QCOMPARE(b.at("A3"), QStringLiteral("-"));
        QVERIFY(!b.member("A3"));
        QVERIFY(b.put(QStringLiteral("A2"), Value()));
        QCOMPARE(b.at("A1"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:2"));
        QCOMPARE(b.at("A3"), QStringLiteral("n:3"));
        QVERIFY(b.member("A2") && b.member("A3"));
        // a formula in the way is in the way too, and so is another formula's spill
        QVERIFY(b.type(QStringLiteral("C3"), QStringLiteral("1+1")));
        QVERIFY(b.type(QStringLiteral("C1"), QStringLiteral("_xlfn.SEQUENCE(3)")));
        QCOMPARE(b.at("C1"), QStringLiteral("e:#SPILL!"));
        QVERIFY(b.type(QStringLiteral("A2"), QStringLiteral("_xlfn.SEQUENCE(2)")));   // over the cell A1's block filled: A1 is blocked now
        QCOMPARE(b.at("A1"), QStringLiteral("e:#SPILL!"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:1"));
    }

    // A cell the person types in, inside the block, is in the way: the formula's cell shows #SPILL!, and the other cells are let go of.
    void typingIntoASpilledCellMakesTheFormulaSpill()
    {
        Book b;
        QVERIFY(b.open(blank()));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(4)")));
        QVERIFY(b.put(QStringLiteral("A3"), Value::number(77)));
        QCOMPARE(b.at("A1"), QStringLiteral("e:#SPILL!"));
        QCOMPARE(b.at("A2"), QStringLiteral("-"));
        QCOMPARE(b.at("A3"), QStringLiteral("n:77"));
        QCOMPARE(b.at("A4"), QStringLiteral("-"));
        QVERIFY(!b.member("A2") && !b.member("A3") && !b.member("A4"));
        // and clearing what was typed lets it fill again
        QVERIFY(b.put(QStringLiteral("A3"), Value()));
        QCOMPARE(b.at("A4"), QStringLiteral("n:4"));
    }

    // A1# is the whole block of A1's formula, in a SUM, in a formula that follows its size, and from another sheet.
    void theSpillRangeOperatorIsTheWholeBlock()
    {
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rowXml(1, constant("B1", "3"))), worksheet(QString())})));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(B1)")));
        QVERIFY(b.type(QStringLiteral("C1"), QStringLiteral("SUM(A1#)")));
        QVERIFY(b.type(QStringLiteral("C2"), QStringLiteral("ROWS(A1#)")));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("SUM(Sheet1!A1#)"), 1));
        QCOMPARE(b.at("C1"), QStringLiteral("n:6"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:3"));
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:6"));
        QVERIFY(b.put(QStringLiteral("B1"), Value::number(4)));
        QCOMPARE(b.at("C1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:4"));
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:10"));
        // a cell that holds no formula that spills has no block
        QVERIFY(b.type(QStringLiteral("D1"), QStringLiteral("SUM(B1#)")));
        QCOMPARE(b.at("D1"), QStringLiteral("e:#REF!"));
        // and a spill that is blocked has none
        QVERIFY(b.put(QStringLiteral("A3"), Value::number(5)));
        QCOMPARE(b.at("C1"), QStringLiteral("e:#REF!"));
    }

    // A block that is larger than the sheet, or than the cap, is #SPILL!: no cell is filled, nothing is made of the size.
    void aBlockPastTheSheetOrTheCapIsSpill()
    {
        Book b;
        QVERIFY(b.open(blank()));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(%1)").arg(kMaxSpillCells + 1)));
        QCOMPARE(b.at("A1"), QStringLiteral("e:#SPILL!"));
        QCOMPARE(b.at("A2"), QStringLiteral("-"));
        QVERIFY(b.type(QStringLiteral("B1"), QStringLiteral("_xlfn.SEQUENCE(1,16385)")));   // past the last column
        QCOMPARE(b.at("B1"), QStringLiteral("e:#SPILL!"));
        QVERIFY(b.type(QStringLiteral("C1048575"), QStringLiteral("_xlfn.SEQUENCE(3)")));   // past the last row
        QCOMPARE(b.at("C1048575"), QStringLiteral("e:#SPILL!"));
        QVERIFY(b.type(QStringLiteral("D1048575"), QStringLiteral("_xlfn.SEQUENCE(2)")));   // the last two rows fit
        QCOMPARE(b.at("D1048576"), QStringLiteral("n:2"));
        // a block the cap allows
        QVERIFY(b.type(QStringLiteral("F1"), QStringLiteral("_xlfn.SEQUENCE(20000)")));
        QCOMPARE(b.at("F20000"), QStringLiteral("n:20000"));
        QCOMPARE(b.block("F1"), QStringLiteral("F1:F20000"));
        QVERIFY(b.type(QStringLiteral("F1"), QStringLiteral("_xlfn.SEQUENCE(1)")));   // and it is let go of whole
        QCOMPARE(b.at("F2"), QStringLiteral("-"));
        QCOMPARE(b.at("F20000"), QStringLiteral("-"));
    }

    // Undo and Redo put back what the cell held, and the cells it filled follow.
    void undoAndRedoOfASpillPutBackTheCellsItFilled()
    {
        Book b;
        QVERIFY(b.open(blank()));
        const Calculation::CellContent empty = b.calc->content(0, 0, 0);
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(3)")));
        const Calculation::CellContent typed = b.calc->content(0, 0, 0);
        QVERIFY(typed.dynamic);
        QVERIFY(b.calc->put(0, 0, 0, empty) && b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("-"));
        QCOMPARE(b.at("A2"), QStringLiteral("-"));
        QVERIFY(!b.member("A2"));
        QVERIFY(b.calc->put(0, 0, 0, typed) && b.settle());
        QCOMPARE(b.at("A3"), QStringLiteral("n:3"));
        // typing over a filled cell is undone by putting its content back: nothing of its own
        const Calculation::CellContent filled = b.calc->content(0, 1, 0);
        QVERIFY(b.put(QStringLiteral("A2"), Value::number(5)));
        QCOMPARE(b.at("A1"), QStringLiteral("e:#SPILL!"));
        QVERIFY(b.calc->put(0, 1, 0, filled) && b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:2"));
    }

    // LET in a cell: names that are good in the formula alone, a later value that uses an earlier name, up to 126 pairs, and a block from the calculation.
    void letInACell()
    {
        Book b;
        QVERIFY(b.open(blank(rowXml(1, constant("B1", "4")))));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.LET(_xlpm.a,B1,_xlpm.b,_xlpm.a*3,_xlpm.a+_xlpm.b)")));
        QCOMPARE(b.at("A1"), QStringLiteral("n:16"));
        QVERIFY(b.type(QStringLiteral("A2"), QStringLiteral("_xlfn.LET(_xlpm.n,B1,_xlfn.SEQUENCE(_xlpm.n))")));
        QCOMPARE(b.at("A2"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A5"), QStringLiteral("n:4"));
        QCOMPARE(b.block("A2"), QStringLiteral("A2:A5"));
        // the names are not the sheet's
        QVERIFY(b.type(QStringLiteral("C1"), QStringLiteral("_xlfn.LET(a,1,a)+a")));
        QCOMPARE(b.at("C1"), QStringLiteral("e:#NAME?"));
        // 126 pairs
        QString f = QStringLiteral("_xlfn.LET(");
        for (int i = 1; i <= 126; ++i) {
            // (plain steps: the two kinds of string in one ?: crashed here on macOS)
            const QString value = i == 1 ? QString::number(1) : QStringLiteral("_xlpm.v") + QString::number(i - 1) + QStringLiteral("+1");
            f += QStringLiteral("_xlpm.v") + QString::number(i) + QLatin1Char(',') + value + QLatin1Char(',');
        }
        f += QStringLiteral("_xlpm.v126)");
        QVERIFY(b.type(QStringLiteral("D1"), f));
        QCOMPARE(b.at("D1"), QStringLiteral("n:126"));
        // a name that is no name is no formula
        QVERIFY(!b.calc->setFormula(0, 0, 4, QStringLiteral("_xlfn.LET(1,2,3)")));
    }

    // An array formula of the older kind (Ctrl+Shift+Enter) is as it was: the cells it covers are not edited alone, its range is its own, and a block larger than the range is cut to it.
    void aLegacyArrayFormulaIsAsItWas()
    {
        const QString rows = rowXml(1, constant("A1", "1") + QStringLiteral("<c r=\"C1\"><f t=\"array\" ref=\"C1:C2\">_xlfn.SEQUENCE(5)</f><v>1</v></c>")) + rowXml(2, constant("A2", "2") + constant("C2", "2"));
        Book b;
        QVERIFY(b.open(blank(rows)));
        QVERIFY(!b.calc->isSpillAnchor(0, 0, 2));
        QVERIFY(!b.member("C2"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:1"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:2"));
        QCOMPARE(b.at("C3"), QStringLiteral("-"));   // (cut to its range)
        QVERIFY(!b.calc->setValue(0, b.row("C2"), b.col("C2"), Value::number(7)));
        QVERIFY(!b.calc->content(0, b.row("C2"), b.col("C2")).editable);
        QVERIFY(!b.calc->content(0, 0, 2).dynamic);
        QCOMPARE(b.calc->content(0, 0, 2).arrayRange.row2, 1u);
    }

    // A file's dynamic array: the anchor's array formula with the cell metadata that marks it, and the cached values of the cells it filled, read as saved.
    void aDynamicArrayOfAFileIsASpillShownAsSaved()
    {
        const QString rows = rowXml(1, QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:A3\">_xlfn.SEQUENCE(3)</f><v>1</v></c>")) + rowXml(2, constant("A2", "2")) + rowXml(3, constant("A3", "3"));
        const QByteArray file = withMetadata(blank(rows));
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(file, &why) && b.book.open(b.package, &why));
        QCOMPARE(b.book.dynamicCellMetadata().size(), 1);
        b.calc.reset(new Calculation(b.book));
        QVERIFY(b.calc->load());
        // before anything is worked out, the cells show what the file saved
        QCOMPARE(b.calc->shown(0, 1, 0).debug(), QStringLiteral("n:2"));
        QCOMPARE(b.calc->shown(0, 2, 0).debug(), QStringLiteral("n:3"));
        QVERIFY(b.calc->isSpillAnchor(0, 0, 0));
        QVERIFY(b.member("A2") && b.member("A3"));
        QCOMPARE(b.block("A1"), QStringLiteral("A1:A3"));
        QVERIFY(b.settle());
        QCOMPARE(b.at("A3"), QStringLiteral("n:3"));
        QCOMPARE(b.block("A1"), QStringLiteral("A1:A3"));
        // the same array formula with no metadata is the older kind
        Book old;
        QVERIFY(old.open(blank(rows)));
        QVERIFY(!old.calc->isSpillAnchor(0, 0, 0));
        QVERIFY(!old.member("A2"));
    }

    // A formula that spills is saved as an array formula over its block with `cm`, and a metadata part that says it is a dynamic array; the file is read again as a spill.
    void aSpillIsSavedAsADynamicArrayAndReadBack()
    {
        Package package;
        QString why;
        QVERIFY(package.readLazy(blank(rowXml(1, constant("B1", "2"))), &why));
        Workbook wb;
        QVERIFY(wb.open(package, &why));
        SaveChanges changes;
        auto cell = [](quint32 row, quint16 col) {
            SavedCell c;
            c.row = row;
            c.col = col;
            return c;
        };
        SavedCell anchor = cell(0, 0);
        anchor.isFormula = true;
        anchor.dynamic = true;
        anchor.formula = QStringLiteral("_xlfn.LET(_xlpm.n,B1,_xlfn.SEQUENCE(_xlpm.n))");
        anchor.arrayRef = QStringLiteral("A1:A2");
        anchor.value.kind = SavedValue::Kind::Number;
        anchor.value.number = 1;
        SavedCell second = cell(1, 0);
        second.value.kind = SavedValue::Kind::Number;
        second.value.number = 2;
        changes.sheets[0].cells = {anchor, second};
        QByteArray bytes;
        QVERIFY2(saveXlsx(wb, changes, &bytes, &why), qPrintable(why));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(bytes, parts));
        const QString sheet = QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QVERIFY2(sheet.contains(QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:A2\">_xlfn.LET(_xlpm.n,B1,_xlfn.SEQUENCE(_xlpm.n))</f><v>1</v></c>")), qPrintable(sheet));
        QVERIFY2(sheet.contains(QStringLiteral("<c r=\"A2\"><v>2</v></c>")), qPrintable(sheet));
        const QString metadata = QString::fromUtf8(parts.value(QStringLiteral("xl/metadata.xml")));
        QVERIFY2(metadata.contains(QStringLiteral("XLDAPR")) && metadata.contains(QStringLiteral("fDynamic=\"1\"")) && metadata.contains(QStringLiteral("<cellMetadata count=\"1\">")), qPrintable(metadata));
        QVERIFY(QString::fromUtf8(parts.value(QStringLiteral("[Content_Types].xml"))).contains(QStringLiteral("/xl/metadata.xml")));
        QVERIFY(QString::fromUtf8(parts.value(QStringLiteral("xl/_rels/workbook.xml.rels"))).contains(QStringLiteral("sheetMetadata")));
        // read again: a spill, with the block, the names, and the values as saved
        Book again;
        QVERIFY(again.open(bytes));
        QVERIFY(again.calc->isSpillAnchor(0, 0, 0));
        QCOMPARE(again.block("A1"), QStringLiteral("A1:A2"));
        QCOMPARE(again.at("A2"), QStringLiteral("n:2"));
        QVERIFY(again.member("A2"));
        QCOMPARE(again.calc->content(0, 0, 0).formula, anchor.formula);
        // and a file that has the metadata already keeps its number for it, and no second part is made
        const QByteArray other = withMetadata(blank());
        Package p2;
        QVERIFY(p2.readLazy(other, &why));
        Workbook w2;
        QVERIFY(w2.open(p2, &why));
        QByteArray bytes2;
        QVERIFY(saveXlsx(w2, changes, &bytes2, &why));
        QMap<QString, QByteArray> parts2;
        QVERIFY(jp::readZip(bytes2, parts2));
        QVERIFY(QString::fromUtf8(parts2.value(QStringLiteral("xl/worksheets/sheet1.xml"))).contains(QStringLiteral("<c r=\"A1\" cm=\"1\">")));
    }
};

JP_TEST_CLASS(SheetSpillTests)
#include "tst_sheet_spill.moc"
