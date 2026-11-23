// Tests of spills: a formula in one cell whose result is a block fills the cells below and beside it (sheet/calcspill.cpp), the spill range operator A1#, LET in a cell, and the file form
// of a formula that spills (an array formula with the cell metadata that marks a dynamic array). Every workbook is built in memory; every expected value is worked out by hand.
#include "sheethelpers.h"

#include "sheet/calculation.h"
#include "sheet/xlsxsave.h"

#include "core/workthread.h"

#include <QThread>

#include <memory>

namespace {

QString constant(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }
QString rowXml(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }
// The name of a column from 0: A to Z, then AA, AB, and so on.
QString columnName(int c) { return c < 26 ? QString(QChar('A' + c)) : QString(QChar('A' + c / 26 - 1)) + QChar('A' + c % 26); }

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

// The blocks of a sheet's table, as "row1:col1:row2:col2" in order (places counted from 0).
std::vector<QString> blocksOf(const Calculation &calc, int sheet)
{
    std::vector<QString> out;
    if (const SpillPublisher *pub = calc.sheet(sheet).cells.spills())
        pub->mine().forEach([&](const SpillRect &r) { out.push_back(QStringLiteral("%1:%2:%3:%4").arg(r.row1).arg(r.col1).arg(r.row2).arg(r.col2)); });
    std::sort(out.begin(), out.end());
    return out;
}

// How many cells the store holds (none made from a block).
qint64 realCount(const CellStore &cells)
{
    qint64 n = 0;
    cells.forEachReal([&](quint32, quint32, const Cell &) { ++n; });
    return n;
}

// The cell the store holds at a place (none made from a block).
std::optional<Cell> realAt(const CellStore &cells, quint32 row, quint32 col)
{
    std::optional<Cell> out;
    cells.forEachInRangeReal(row, col, row, col, [&](quint32, quint32, const Cell &x) {
        out = x;
        return false;
    });
    return out;
}

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

    // A lookup whose cells were read into an index (the lookup cache), and a spill that then grows into those cells: the lookup reads the cells as they are now. The spill changes no cell of the store
    // (its block is a rectangle of the table), so only the count of the store's writes, moved by a change of the blocks, tells the cache its index is stale.
    void aCachedLookupSeesCellsASpillGrowsInto()
    {
        SpillIndexSwitch on(true);
        QString rows;
        for (int n = 11; n <= 500; ++n) rows += rowXml(n, constant(QStringLiteral("B%1").arg(n), QString::number(n)));
        Book b;
        QVERIFY(b.open(blank(rows)));
        for (int k = 0; k < 6; ++k) QVERIFY(b.type(QStringLiteral("D%1").arg(k + 1), QStringLiteral("MATCH(%1,B1:B500,0)").arg(480 + k)));
        QVERIFY(b.type(QStringLiteral("D7"), QStringLiteral("MATCH(10,B1:B500,0)")));
        QCOMPARE(b.at("D7"), QStringLiteral("e:#N/A"));   // (B1:B10 are empty)
        // the searches have cost what an index costs: B1:B500 is held as an index now, and answers
        QVERIFY2(b.calc->lookupStats().builds >= 1, qPrintable(QString::number(b.calc->lookupStats().builds)));
        QVERIFY(b.type(QStringLiteral("D8"), QStringLiteral("MATCH(11,B1:B500,0)")));
        QVERIFY2(b.calc->lookupStats().hits >= 1, qPrintable(QString::number(b.calc->lookupStats().hits)));
        QCOMPARE(b.at("D8"), QStringLiteral("n:11"));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(10,2)")));   // A1:B10: B5 is 10
        QCOMPARE(b.at("B5"), QStringLiteral("n:10"));
        QCOMPARE(b.at("D7"), QStringLiteral("n:5"));
        QCOMPARE(b.at("D8"), QStringLiteral("n:11"));
        QVERIFY2(b.calc->lookupStats().dropped >= 1, qPrintable(QString::number(b.calc->lookupStats().dropped)));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(4,2)")));   // it shrinks: B5 is empty again
        QCOMPARE(b.at("D7"), QStringLiteral("e:#N/A"));
    }

    // A spill that a copied formula is put over (a paste or a fill onto its first cell) fills no block: the formula is let go of, and so is its block (put does not clear the cell first).
    void aFormulaCopiedOverASpillLeavesNoBlock()
    {
        SpillIndexSwitch on(true);
        Book b;
        QVERIFY(b.open(blank()));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(3)")));
        QVERIFY(b.type(QStringLiteral("E5"), QStringLiteral("1+2")));
        QCOMPARE(blocksOf(*b.calc, 0), std::vector<QString>{QStringLiteral("0:0:2:0")});
        Calculation::CellContent c = b.calc->content(0, b.row("E5"), b.col("E5"));
        QVERIFY(c.hasHeld);
        c.copied = true;
        QVERIFY(b.calc->put(0, b.row("A1"), b.col("A1"), c, Calculation::Marking::Now));
        QVERIFY(b.settle());
        QCOMPARE(b.at("A1"), QStringLiteral("n:3"));
        QVERIFY(blocksOf(*b.calc, 0).empty());
        QVERIFY(!b.member("A2") && !b.member("A3"));
        QCOMPARE(b.at("A2"), QStringLiteral("-"));
    }

    // A sheet copied with a formula that spills has its block as soon as it is made (its table starts empty, and the formulas of the copy are not worked out before the table is read).
    void aCopyOfASheetHasTheBlocksOfItsSpills()
    {
        SpillIndexSwitch on(true);
        Book b;
        QVERIFY(b.open(blank()));
        QVERIFY(b.type(QStringLiteral("B2"), QStringLiteral("_xlfn.SEQUENCE(3)")));
        const int made = b.calc->addSheet(QStringLiteral("Copy"), 0);
        QVERIFY(made > 0);
        QCOMPARE(blocksOf(*b.calc, made), std::vector<QString>{QStringLiteral("1:1:3:1")});
        QVERIFY(b.member("B4", made));
        QCOMPARE(realCount(b.calc->sheet(made).cells), qint64(1));   // (the head only: no cell is made for the block)
        QCOMPARE(blocksOf(*b.calc, 0), std::vector<QString>{QStringLiteral("1:1:3:1")});
    }

    // A formula that spills, in a cell an edit takes away (a shift of cells over it, or whole rows or columns), leaves no block, whatever is worked out after.
    void aSpillInACellThatAnEditTakesAwayLeavesNoBlock()
    {
        SpillIndexSwitch on(true);
        const StructEdit edits[] = {StructEdit::deleteCellsUp(0, CellRange{0, 0, 0, 0, true}), StructEdit::deleteCellsLeft(0, CellRange{0, 0, 0, 0, true}), StructEdit::deleteRows(0, 0, 1),
                                    StructEdit::deleteColumns(0, 0, 1)};
        for (const StructEdit &edit : edits) {
            Book b;
            QVERIFY(b.open(blank()));
            QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(2,2)")));
            QCOMPARE(blocksOf(*b.calc, 0), std::vector<QString>{QStringLiteral("0:0:1:1")});
            Calculation::EditRecord rec;
            QVERIFY(b.calc->applyEdit(edit, &rec));
            QVERIFY(b.settle());
            for (const QString &blk : blocksOf(*b.calc, 0)) QVERIFY2(false, qPrintable(blk));
            QVERIFY(!b.calc->isSpillAnchor(0, 0, 0));
        }
    }

    // A loaded spill and the table made again by an edit of rows agree on kept formulas: a spill that is worked out has its block, one that is kept apart (its cells are the file's) has none, before the
    // edit and after it.
    void aLoadedSpillAndTheTableAfterAnEditAgreeOnKeptFormulas()
    {
        SpillIndexSwitch on(true);
        // A1:A3 is worked out; A5:A7 is a spill that changes by itself (RAND), which a calculation that keeps such formulas apart leaves with the cells the file stored
        const QString rows = rowXml(1, QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:A3\">_xlfn.SEQUENCE(3)</f><v>1</v></c>")) + rowXml(2, constant("A2", "2")) + rowXml(3, constant("A3", "3"))
            + rowXml(5, QStringLiteral("<c r=\"A5\" cm=\"1\"><f t=\"array\" ref=\"A5:A7\">_xlfn.SEQUENCE(3)+0*RAND()</f><v>1</v></c>")) + rowXml(6, constant("A6", "2")) + rowXml(7, constant("A7", "3"));
        const QByteArray file = withMetadata(blank(rows));
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(file, &why) && b.book.open(b.package, &why));
        CalcOptions options;
        options.keepApart = true;
        b.calc.reset(new Calculation(b.book, options));
        QVERIFY(b.calc->load());
        QVERIFY(b.settle());
        QVERIFY(b.calc->isSpillAnchor(0, 4, 0));   // (a spill that is kept apart is one still)
        QCOMPARE(blocksOf(*b.calc, 0), std::vector<QString>{QStringLiteral("0:0:2:0")});   // (A1:A3 only: A5:A7 is kept apart)
        Calculation::EditRecord rec;
        QVERIFY(b.calc->applyEdit(StructEdit::insertRows(0, 0, 1), &rec));   // (a row above: the table is made again from the ranges)
        QVERIFY(b.settle());
        QCOMPARE(blocksOf(*b.calc, 0), std::vector<QString>{QStringLiteral("1:0:3:0")});
        QCOMPARE(b.at("A7"), QStringLiteral("n:2"));   // (the file's cells, as they were)
        QCOMPARE(b.calc->sheet(0).cells.cellCount(), qint64(1 + 3 + 0 + 2) );
    }

    // A stored workbook with a spill whose stored range is over the size a spill can have: no block is made for the range, no cell is made for the thousands of places in it (the cells of the store are
    // the ones the file stored), the formula is worked out for what it gives, and the values the file stored in the range are the cells' own or the block's, none lost.
    void aStoredSpillOverTheSizeCapMakesNoBlockAndLosesNoValue()
    {
        const qint64 over = kMaxSpillCells + 1;
        const QString rows = rowXml(1, QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:A%1\">_xlfn.SEQUENCE(3)</f><v>1</v></c>").arg(over)) + rowXml(2, constant("A2", "2")) + rowXml(3, constant("A3", "3"))
            + rowXml(4, constant("A4", "44")) + rowXml(9, constant("B9", "99"));
        QStringList seen[2];   // (what each cell reads as, with the index off and then on: the same)
        for (int idx = 0; idx < 2; ++idx) {
            SpillIndexSwitch sw(idx != 0);
            Book b;
            QString why;
            QVERIFY(b.package.readLazy(withMetadata(blank(rows)), &why) && b.book.open(b.package, &why));
            b.calc.reset(new Calculation(b.book));
            QVERIFY(b.calc->load());
            if (idx) QVERIFY(blocksOf(*b.calc, 0).empty());   // (not a block: over the cap)
            for (const char *ref : {"A1", "A2", "A3", "A4", "B9"}) seen[idx] << b.at(ref);
            QVERIFY(b.settle());
            for (const char *ref : {"A1", "A2", "A3", "A4", "B9"}) seen[idx] << b.at(ref);
            QCOMPARE(b.at("A1"), QStringLiteral("n:1"));
            QCOMPARE(b.at("A3"), QStringLiteral("n:3"));
            QCOMPARE(b.at("B9"), QStringLiteral("n:99"));   // (the file's own cell, outside the range)
            // a formula that gives more than a spill can have is blocked, as ever, and no block or cell is made for it
            const qint64 cellsBefore = b.calc->sheet(0).cells.cellCount();
            QVERIFY(b.type(QStringLiteral("C1"), QStringLiteral("_xlfn.SEQUENCE(%1)").arg(over)));
            QCOMPARE(b.at("C1"), QStringLiteral("e:#SPILL!"));
            QVERIFY2(b.calc->sheet(0).cells.cellCount() <= cellsBefore + 1, qPrintable(QString::number(b.calc->sheet(0).cells.cellCount())));
            if (idx) QCOMPARE(blocksOf(*b.calc, 0).size(), size_t(1));
        }
        QCOMPARE(seen[1], seen[0]);
    }

    // Stored ranges that overlap: the first of two that meet is the block, the other has none, and every value the file stored in either is read afterwards, as a cell's own or as a block's.
    void storedSpillRangesThatOverlapMakeBlocksThatDoNotAndLoseNoValue()
    {
        const QString rows = rowXml(1, QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:A5\">_xlfn.SEQUENCE(5)</f><v>1</v></c><c r=\"C1\" cm=\"1\"><f t=\"array\" ref=\"C1:C4\">_xlfn.SEQUENCE(4)</f><v>1</v></c>"))
            + rowXml(3, constant("A3", "3")) + rowXml(4, constant("A4", "4")) + rowXml(5, constant("A5", "5"))
            + rowXml(7, constant("A7", "77")) + rowXml(8, QStringLiteral("<c r=\"A8\" cm=\"1\"><f t=\"array\" ref=\"A4:A8\">_xlfn.SEQUENCE(5,1,10)</f><v>10</v></c>"));
        QStringList seen[2];   // (what each cell reads as, with the index off and then on: the same)
        for (int idx = 0; idx < 2; ++idx) {
            SpillIndexSwitch sw(idx != 0);
            Book b;
            QString why;
            QVERIFY(b.package.readLazy(withMetadata(blank(rows)), &why) && b.book.open(b.package, &why));
            b.calc.reset(new Calculation(b.book));
            QVERIFY(b.calc->load());
            auto disjoint = [&] {
                std::vector<SpillRect> all;
                if (const SpillPublisher *pub = b.calc->sheet(0).cells.spills()) pub->mine().forEach([&](const SpillRect &r) { all.push_back(r); });
                for (size_t i = 0; i < all.size(); ++i)
                    for (size_t j = i + 1; j < all.size(); ++j)
                        if (all[i].meets(all[j].row1, all[j].col1, all[j].row2, all[j].col2)) return false;
                return true;
            };
            QVERIFY(disjoint());
            if (idx) QVERIFY(!blocksOf(*b.calc, 0).empty());
            for (int row = 1; row <= 8; ++row) seen[idx] << b.at(QStringLiteral("A%1").arg(row).toUtf8().constData()) << b.at(QStringLiteral("C%1").arg(row).toUtf8().constData());
            QVERIFY(b.settle());
            QVERIFY(disjoint());
            for (int row = 1; row <= 8; ++row) seen[idx] << b.at(QStringLiteral("A%1").arg(row).toUtf8().constData()) << b.at(QStringLiteral("C%1").arg(row).toUtf8().constData());
        }
        QCOMPARE(seen[1], seen[0]);
    }

    // Thousands of small spills, as a file has them: every one has its block (a 1 by 2 block in each row), a place is found by looking at a few blocks however many there are (counted, not timed),
    // and the values a file stored in the blocks are read as they were.
    void thousandsOfStoredSmallSpillsAreFoundByAFewLooks()
    {
        SpillIndexSwitch on(true);
        const int count = 3000;
        QString rows;
        for (int n = 1; n <= count; ++n)
            rows += rowXml(n, QStringLiteral("<c r=\"A%1\" cm=\"1\"><f t=\"array\" ref=\"A%1:B%1\">_xlfn.SEQUENCE(1,2,%1)</f><v>%1</v></c>").arg(n) + constant(QStringLiteral("B%1").arg(n), QString::number(n + 1)));
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(withMetadata(blank(rows)), &why) && b.book.open(b.package, &why));
        b.calc.reset(new Calculation(b.book));
        QVERIFY(b.calc->load());
        QCOMPARE(blocksOf(*b.calc, 0).size(), size_t(count));
        QVERIFY(b.settle());
        const SpillTable &t = b.calc->sheet(0).cells.spills()->mine();
        QCOMPARE(t.size(), size_t(count));
        qint64 mostForOne = 0, total = 0;
        for (quint32 r = 0; r < quint32(count); ++r)
            for (quint32 c = 0; c < 2; ++c) {
                qint64 probes = 0;
                const SpillRect *found = t.at(r, c, &probes);
                QVERIFY(found && found->row1 == r && found->col2 == 1);
                mostForOne = std::max(mostForOne, probes);
                total += probes;
            }
        QVERIFY2(mostForOne <= SpillTable::kBlockRows, qPrintable(QString::number(mostForOne)));   // (a bucket holds the blocks of its 64 rows: a scan of the table would look at 3,000)
        QVERIFY2(total <= qint64(SpillTable::kBlockRows) * 2 * count, qPrintable(QString::number(total)));
        QCOMPARE(b.at("B1"), QStringLiteral("n:2"));
        QCOMPARE(b.at(QStringLiteral("B%1").arg(count).toUtf8().constData()), QStringLiteral("n:%1").arg(count + 1));
        QCOMPARE(b.at(QStringLiteral("A%1").arg(count).toUtf8().constData()), QStringLiteral("n:%1").arg(count));
        qint64 seen = 0;
        b.calc->sheet(0).cells.forEach([&](quint32, quint32, const Cell &) { ++seen; });
        QCOMPARE(seen, qint64(2 * count));   // (a walk of the sheet gives each place of a block once)
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

    // A formula with a range that is not a spill's (here a data table, whose members are cells the store holds; a legacy array formula is the same to the code, but edits refuse it before) that is let go
    // of tells the window nothing of its members: it filled no block.
    void aFormulaWithARangeThatIsNoSpillLetGoOfTellsNothingOfItsMembers()
    {
        SpillIndexSwitch on(true);
        const QString rows = rowXml(1, constant("A1", "5") + QStringLiteral("<c r=\"F1\"><f t=\"dataTable\" ref=\"F1:F6\" r1=\"A1\"/><v>5</v></c>"))
            + rowXml(2, constant("F2", "2")) + rowXml(3, constant("F3", "3")) + rowXml(4, constant("F4", "4")) + rowXml(5, constant("F5", "5")) + rowXml(6, constant("F6", "6"));
        Book b;
        QVERIFY(b.open(blank(rows)));
        QVERIFY(b.calc->spillBlock(0, 0, 5).valid == false);   // (not a spill)
        qint64 gone = 0;
        b.calc->setResultSink([&](int, quint32, quint32, const Value &v) { if (v.isEmpty()) ++gone; });
        QVERIFY(b.calc->setValue(0, b.row("F1"), b.col("F1"), Value::number(9)));   // (the formula of its first cell is let go of)
        QVERIFY(b.settle());
        QCOMPARE(gone, qint64(0));
        QCOMPARE(b.at("F4"), QStringLiteral("n:4"));   // (its members are the cells they were)
    }

    // A spill's block let go of tells the window that its cells but the formula's own are empty, once each; a stored one over the size a spill can have (it has no block, and the cells it covers are the
    // file's) is not walked over: the work is the cells of the block the table held, never the area of a range.
    void aBlockLetGoOfTellsItsCellsAndARangeWithNoBlockTellsNone()
    {
        SpillIndexSwitch on(true);
        qint64 b0walked = 0;
        {
            Book b;
            QVERIFY(b.open(blank()));
            QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(5)")));
            qint64 gone = 0;
            b.calc->setResultSink([&](int, quint32, quint32, const Value &v) { if (v.isEmpty()) ++gone; });
            QVERIFY(b.put(QStringLiteral("A1"), Value::number(1)));
            QCOMPARE(gone, qint64(4));
            b0walked = b.calc->goneCellsWalked();
        }
        QCOMPARE(b0walked, qint64(5));   // (the cells of the block, the first of them skipped)
        const qint64 over = kMaxSpillCells + 1;
        const QString rows = rowXml(1, QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:A%1\">_xlfn.SEQUENCE(3)</f><v>1</v></c>").arg(over)) + rowXml(2, constant("A2", "2")) + rowXml(3, constant("A3", "3"));
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(withMetadata(blank(rows)), &why) && b.book.open(b.package, &why));
        b.calc.reset(new Calculation(b.book));
        b.calc->setResultSink([](int, quint32, quint32, const Value &) {});   // (the window is told, if there is one)
        QVERIFY(b.calc->load());
        QVERIFY(b.settle());   // (the stored range shrinks to the three cells the formula gives)
        QVERIFY(b.put(QStringLiteral("A1"), Value::number(1)));   // (and the formula is let go of)
        QVERIFY2(b.calc->goneCellsWalked() <= 10, qPrintable(QString::number(b.calc->goneCellsWalked())));   // (the range of 200,001 cells is not walked: it had no block)
    }

    // A store that is given another takes its blocks with the cells: the table of the one assigned follows the table of the one it is made from (none, if it has none).
    void aStoreAssignedFromAnotherTakesItsBlocks()
    {
        MutationGuard guard;
        CellStore from;
        from.enableSpills(&guard);
        from.spills()->mine().set(SpillRect{0, 0, 2, 0, 1});
        from.spills()->noteChanged();
        CellStore into;
        into.enableSpills(&guard);
        into.spills()->mine().set(SpillRect{10, 5, 12, 5, 2});
        into.spills()->noteChanged();
        QVERIFY(into.find(11, 5));
        into = from;
        QVERIFY(into.find(2, 0) && (into.find(2, 0)->flags & SpillMember));
        QVERIFY(!into.find(11, 5));
        CellStore frozen = from;   // (a copy has the blocks as they were, held apart)
        CellStore other;
        other.enableSpills(&guard);
        other.spills()->mine().set(SpillRect{20, 1, 22, 1, 3});
        other.spills()->noteChanged();
        frozen = other;
        QVERIFY(frozen.find(21, 1) && !frozen.find(2, 0));
        const CellStore none;
        into = none;   // (a store with no blocks: the one assigned has none)
        QVERIFY(!into.find(2, 0));
        frozen = none;
        QVERIFY(!frozen.find(21, 1));
    }

    // The blocks of a table are at most so many buckets in all (the table is copied each time it is published): a block that would pass the total is refused and has no block, and its cells, which the file
    // stored, are read as they were; a block let go of makes room.
    void theSpillTableHasATotalCapOnBuckets()
    {
        SpillTable t;
        const quint32 rows = quint32(kMaxSpillCells);   // 3,125 buckets: one column of the most a spill can be
        int accepted = 0;
        for (qint32 f = 0; f < 400 && t.set(SpillRect{0, quint32(f), rows - 1, quint32(f), f}); ++f) ++accepted;
        QVERIFY(accepted >= 1 && accepted < 400);
        QVERIFY(qint64(accepted) * 3125 <= (1 << 18));
        QVERIFY(!t.ofFormula(accepted));   // (the one refused has no block)
        QCOMPARE(t.size(), size_t(accepted));
        t.erase(0);
        QVERIFY(t.set(SpillRect{0, quint32(accepted), rows - 1, quint32(accepted), accepted}));   // (room for one)
        // the same by a file: many stored spills of the most a block can be
        const int columns = 150;
        QString rowsXml, second;
        for (int c = 0; c < columns; ++c) {
            const QString name = columnName(c);
            rowsXml += QStringLiteral("<c r=\"%1%2\" cm=\"1\"><f t=\"array\" ref=\"%1%2:%1%3\">_xlfn.SEQUENCE(3)</f><v>1</v></c>").arg(name, QStringLiteral("1")).arg(kMaxSpillCells);
            second += constant(name + QStringLiteral("2"), QStringLiteral("77"));
        }
        SpillIndexSwitch on(true);
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(withMetadata(blank(rowXml(1, rowsXml) + rowXml(2, second))), &why) && b.book.open(b.package, &why));
        b.calc.reset(new Calculation(b.book));
        QVERIFY(b.calc->load());
        const size_t blocks = blocksOf(*b.calc, 0).size();
        QVERIFY(blocks >= 1 && blocks < size_t(columns));
        QVERIFY(qint64(blocks) * 3125 <= (1 << 18));
        for (int c = 0; c < columns; ++c) {
            const QString name = columnName(c);
            const std::optional<Cell> stored = b.calc->sheet(0).cells.find(1, b.col(name + QStringLiteral("2")));   // (a refused block keeps what the file stored in it)
            QVERIFY(stored && stored->type == CellType::Number && stored->number == 77);
        }
    }

    // A copy of a table (what a publication is) shares its shards: a change copies only the shards it touches, which hold a few of the blocks, however many there are (counted in entries copied, not timed).
    // Fails when a change copies the whole table (kShards of 1: the change copies the 100,000 blocks).
    void aPublicationCopiesWhatChangedNotTheTable()
    {
        SpillTable t;
        const qint32 n = 100000;
        for (qint32 f = 0; f < n; ++f) QVERIFY(t.set(SpillRect{quint32(f), 0, quint32(f), 1, f}));
        QCOMPARE(t.size(), size_t(n));
        QCOMPARE(t.copiedEntries(), qint64(0));   // (no copy of it was shared: nothing to copy)
        const SpillTable published = t;           // (a publication)
        t.set(SpillRect{5, 0, 9, 3, 5});          // (a block grows and moves)
        t.erase(77);
        t.set(SpillRect{quint32(n) + 4, 2, quint32(n) + 5, 3, n + 1});
        QVERIFY2(t.copiedEntries() < 4 * n / int(SpillTable::kShards) * 8 && t.copiedEntries() > 0, qPrintable(QString::number(t.copiedEntries())));   // (a few shards of about n / kShards entries)
        QVERIFY(t.copiedEntries() < n / 8);
        // the copy that was shared is what it was
        QVERIFY(published.ofFormula(77) && published.at(77, 1));
        QCOMPARE(published.ofFormula(5)->row2, quint32(5));
        QCOMPARE(published.size(), size_t(n));
        QVERIFY(!t.ofFormula(77) && !t.at(77, 1));
        QCOMPARE(t.ofFormula(5)->row2, quint32(9));
        QCOMPARE(t.size(), size_t(n));
        // a second change to a shard already copied copies nothing more
        const qint64 before = t.copiedEntries();
        t.set(SpillRect{5, 0, 9, 4, 5});
        QCOMPARE(t.copiedEntries(), before);
        // finding and meeting go through the shards
        QVERIFY(t.at(99999, 1) && t.at(99999, 1)->formula == 99999);
        QVERIFY(t.meetsOther(8, 4, 8, 4, 6) && !t.meetsOther(8, 5, 8, 5, 6));
        qint32 seen = 0;
        t.forEachMeeting(0, 0, 200, 7, [&](const SpillRect &) { ++seen; });
        QCOMPARE(seen, qint32(200));   // (rows 0 to 200: 5 was joined by 6 to 9 inside its block, 77 is gone)
    }

    // A table that was copied (what the others read) is what it was when the worker changes its own: blocks put, grown, and taken away leave the copy's blocks and places as they were. Fails when a change is
    // made in a shard that the copy shares (the copy loses the block taken away, has the block that grew, and finds the new one).
    void aPublishedCopyIsNotChangedByLaterChanges()
    {
        SpillTable t;
        for (qint32 f = 0; f < 40; ++f) QVERIFY(t.set(SpillRect{quint32(f * 3), 0, quint32(f * 3 + 1), 2, f}));
        const SpillTable published = t;
        t.erase(3);
        QVERIFY(t.set(SpillRect{15, 0, 19, 4, 5}));   // (grown, and over where 6 was: moved out of 6's rows below)
        QVERIFY(t.set(SpillRect{500, 0, 501, 1, 77}));
        QVERIFY(published.ofFormula(3) && published.at(9, 0) && published.at(9, 0)->formula == 3);
        QCOMPARE(published.ofFormula(5)->row2, quint32(16));
        QVERIFY(!published.at(18, 4));
        QVERIFY(!published.ofFormula(77) && !published.at(500, 0));
        QCOMPARE(published.size(), size_t(40));
        QVERIFY(!t.ofFormula(3) && !t.at(9, 0) && t.at(18, 4) && t.at(500, 0));
        QCOMPARE(t.size(), size_t(40));
    }

    // A place outside the rectangle that holds every block is answered with no block looked at, and a table of up to kSmall blocks answers from its short list: the answers are those of a search of all the
    // blocks, through every size the table passes (4 to 5 blocks and back, a block changed, one taken away). Fails when the short list is not made again at 5 to 4 (a block taken away is still found).
    void aSmallTableAndTheOuterRectangleAnswerAsTheBucketsDo()
    {
        SpillTable t;
        std::vector<SpillRect> blocks;
        auto brute = [&](quint32 row, quint32 col) -> const SpillRect * {
            for (const SpillRect &r : blocks)
                if (r.contains(row, col)) return &r;
            return nullptr;
        };
        auto check = [&](const char *what) {
            qint64 outsideProbes = 0;
            for (quint32 row = 0; row < 40; row += 1)
                for (quint32 col = 0; col < 40; ++col) {
                    const SpillRect *want = brute(row, col), *got = t.at(row, col);
                    if ((want == nullptr) != (got == nullptr) || (want && want->formula != got->formula)) {
                        QFAIL(qPrintable(QStringLiteral("%1: place %2,%3").arg(QString::fromLatin1(what)).arg(row).arg(col)));
                    }
                }
            t.at(100, 100, &outsideProbes);   // (past every block)
            QCOMPARE(outsideProbes, qint64(0));
            QCOMPARE(t.size(), blocks.size());
        };
        QVERIFY(!t.at(0, 0));
        qint64 none = 0;
        QVERIFY(!t.at(3, 3, &none) && none == 0);   // (an empty table looks at nothing)
        for (qint32 f = 0; f < 6; ++f) {
            const SpillRect r{quint32(f * 5), quint32(f * 6), quint32(f * 5 + 3), quint32(f * 6 + 4), f};
            QVERIFY(t.set(r));
            blocks.push_back(r);
            check("grown");
        }
        for (qint32 f : {5, 0, 3}) {   // (6 to 5 to 4 to 3: the short list is made again where it begins to hold them all)
            t.erase(f);
            blocks.erase(std::remove_if(blocks.begin(), blocks.end(), [&](const SpillRect &r) { return r.formula == f; }), blocks.end());
            check("shrunk");
        }
        const SpillRect moved{20, 0, 22, 2, 1};   // (a block changed in place)
        QVERIFY(t.set(moved));
        for (SpillRect &r : blocks)
            if (r.formula == 1) r = moved;
        check("changed");
        QVERIFY(t.set(SpillRect{30, 30, 31, 31, 9}));
        blocks.push_back(SpillRect{30, 30, 31, 31, 9});
        QVERIFY(t.set(SpillRect{35, 0, 35, 0, 10}));
        blocks.push_back(SpillRect{35, 0, 35, 0, 10});
        check("grown again");
        t.erase(9);
        blocks.erase(std::remove_if(blocks.begin(), blocks.end(), [&](const SpillRect &r) { return r.formula == 9; }), blocks.end());
        check("shrunk again");
    }

    // The window's thread reads the blocks of the last batch it took, and the worker's slices wait until a batch comes: a copy made at the end of a slice is staged under a number, and adopt(seq) of a later
    // batch makes it the one that is read. Fails when stage() makes the copy the one the others read at once (the reader sees the second block before its batch).
    void blocksAreStagedByTheWorkerAndAdoptedByTheWindow()
    {
        MutationGuard guard;
        SpillPublisher pub(&guard);
        CellStore store;
        const quintptr me = quintptr(QThread::currentThreadId());
        guard.setWorker(me + 1);   // (a worker that is another thread: this one is the window's)
        QVERIFY(!pub.view());
        bool staged = false;
        jp::WorkThread worker([&] {
            guard.setWorker(quintptr(QThread::currentThreadId()));
            pub.mine().set(SpillRect{0, 0, 2, 0, 0});
            pub.noteChanged();
            pub.stage(1);
            pub.mine().set(SpillRect{0, 5, 3, 5, 1});
            pub.noteChanged();
            pub.stage(2);
            staged = true;
        });
        worker.join();
        QVERIFY(staged);
        QCOMPARE(pub.stagedCount(), qint64(2));
        QVERIFY(!pub.view());   // (nothing is adopted: the window reads what it read)
        pub.adopt(0);
        QVERIFY(!pub.view());
        pub.adopt(1);
        QVERIFY(pub.view() && pub.view().table->at(1, 0) && !pub.view().table->at(1, 5));
        QCOMPARE(pub.stagedCount(), qint64(1));
        pub.adopt(7);   // (a batch that knows later slices)
        QVERIFY(pub.view().table->at(1, 0) && pub.view().table->at(1, 5));
        QCOMPARE(pub.stagedCount(), qint64(0));
        // only so many are kept apart: a window that falls behind adopts the oldest that is left
        guard.setWorker(0);
        SpillPublisher slow(&guard);
        guard.setWorker(me + 1);
        for (qint32 i = 0; i < 20; ++i) {
            slow.mine().set(SpillRect{quint32(i), 0, quint32(i), 1, i});
            slow.noteChanged();
            slow.stage(i + 1);
        }
        QCOMPARE(slow.stagedCount(), qint64(SpillPublisher::kMaxStaged));
        slow.adopt(3);   // (those up to 3 were let go of: nothing older than the kept ones is adopted)
        QVERIFY(!slow.view());
        slow.adopt(13);
        QVERIFY(slow.view().table->at(12, 0) && !slow.view().table->at(13, 0));
    }

    // A copy of the store made by a thread that has the worker held back (a snapshot for a save) freezes the blocks the worker has made, which the sheet's ranges agree with, though the window has not yet taken
    // the slice that made them; a copy made by any other thread freezes what it reads. Fails when the copy freezes the blocks the window reads whatever the thread (the block of the slice not yet taken is lost).
    void aCopyMadeUnderAHoldFreezesTheWorkersBlocks()
    {
        MutationGuard guard;
        CellStore store;
        store.enableSpills(&guard);
        SpillPublisher *pub = store.spills();
        QVERIFY(pub);
        const quintptr me = quintptr(QThread::currentThreadId());
        guard.setWorker(me + 1);   // (a worker that is another thread: this one is the window's)
        pub->mine().set(SpillRect{0, 0, 2, 0, 0});
        pub->noteChanged();
        pub->stage(1);
        pub->adopt(1);   // (the first slice was taken)
        pub->mine().set(SpillRect{0, 0, 5, 0, 0});   // (the next grew the block, and is not taken yet)
        pub->noteChanged();
        pub->stage(2);
        QVERIFY(store.spillView().table->at(2, 0) && !store.spillView().table->at(4, 0));
        {
            const CellStore other(store);   // (this thread does not hold the worker back: it freezes what it reads)
            QVERIFY(other.find(2, 0) && !other.find(4, 0));
        }
        guard.enter();
        const CellStore held(store);
        QVERIFY(held.find(5, 0));   // (the worker's block)
        QVERIFY(held.find(5, 0)->flags & SpillMember);
        QVERIFY(!held.find(6, 0));
        guard.leave();
        guard.setWorker(0);
    }

    // A file's spill with a stored value for each cell: the value is kept as the cell's own at load (nothing is rewritten when the formula is first worked out), the block is a rectangle, and the sheet's
    // count of cells and the extent that print and Ctrl+End use take the block in. Fails when the count leaves the blocks out (cellCount) or the extent does not reach the block (spillExtent).
    void aLoadedBlockKeepsItsStoredValuesAndIsCountedAndReached()
    {
        SpillIndexSwitch on(true);
        const QString rows = rowXml(1, QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:D1\">_xlfn.SEQUENCE(1,4)</f><v>1</v></c>") + constant("B1", "2") + constant("C1", "3") + constant("D1", "4"));
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(withMetadata(blank(rows)), &why) && b.book.open(b.package, &why));
        b.calc.reset(new Calculation(b.book));
        QVERIFY(b.calc->load());
        const CellStore &cells = b.calc->sheet(0).cells;
        for (quint32 col = 1; col <= 3; ++col) {
            const std::optional<Cell> real = realAt(cells, 0, col);   // (the stored value, still the cell's own)
            QVERIFY(real && real->type == CellType::Number && (real->flags & SpillMember));
            QCOMPARE(real->number, double(col + 1));
        }
        QCOMPARE(cells.cellCount(), qint64(4));   // (the formula's cell, and the three stored)
        QVERIFY(b.settle());
        QCOMPARE(QStringList({b.at("A1"), b.at("B1"), b.at("C1"), b.at("D1")}).join(QLatin1Char(' ')), QStringLiteral("n:1 n:2 n:3 n:4"));
        QCOMPARE(cells.cellCount(), qint64(4));
        // a block that has cells of its own for some places counts each place once
        b.put("H5", Value::number(1));
        QVERIFY(b.type(QStringLiteral("A3"), QStringLiteral("_xlfn.SEQUENCE(40,3)")));
        QCOMPARE(cells.cellCount(), qint64(125));   // (6 cells of its own: A1:D1, A3, H5; the places of A3:C42 that hold none are 119)
        quint32 rowsReached = 0, colsReached = 0;
        cells.spillExtent(&rowsReached, &colsReached);
        QCOMPARE(rowsReached, quint32(42));
        QCOMPARE(colsReached, quint32(4));
    }

    // The formula of a loaded spill goes (its cell is given a value) by the worker's own thread: the window reads the store beside it, so the cells the block filled are told empty and left in the store; a thread
    // that has the worker held back takes them. Fails when letGoOfSpill takes them from the store itself (they are gone before the hold, and none waits).
    void aLoadedSpillWhoseFormulaGoesIsLetGoOfWithTheWorkerHeld()
    {
        SpillIndexSwitch on(true);
        const QString rows = rowXml(1, QStringLiteral("<c r=\"A1\" cm=\"1\"><f t=\"array\" ref=\"A1:D1\">_xlfn.SEQUENCE(1,4)</f><v>1</v></c>") + constant("B1", "2") + constant("C1", "3") + constant("D1", "4"));
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(withMetadata(blank(rows)), &why) && b.book.open(b.package, &why));
        b.calc.reset(new Calculation(b.book));
        QVERIFY(b.calc->load());
        QVERIFY(b.settle());
        const CellStore &cells = b.calc->sheet(0).cells;
        MutationGuard &guard = b.calc->mutationGuard();
        const quintptr me = quintptr(QThread::currentThreadId());
        guard.setWorker(me + 1);   // (a worker that is another thread: this one is the window's)
        bool done = false;
        jp::WorkThread worker([&] {
            guard.setWorker(quintptr(QThread::currentThreadId()));
            done = b.calc->setValue(0, 0, 0, Value::number(7));
        });
        worker.join();
        QVERIFY(done);
        QVERIFY(b.calc->hasStrays());
        QCOMPARE(b.calc->straysReleased(), qint64(0));
        for (quint32 col = 1; col <= 3; ++col) QVERIFY(realAt(cells, 0, col));   // (still in the store: the window may be reading it)
        guard.enter();
        b.calc->releaseStrays();
        guard.leave();
        QVERIFY(!b.calc->hasStrays());
        QCOMPARE(b.calc->straysReleased(), qint64(3));
        for (quint32 col = 1; col <= 3; ++col) QVERIFY(!realAt(cells, 0, col));
        guard.setWorker(0);
    }

    // 99,999 cells of a loaded spill wait to be taken when another formula's block is asked for over them: each cell is told from one in the way by one look at the set of waiting places, whatever their number (the
    // work counted is one for each such cell, not the cells times the waiting ones). Fails when the cells are looked for in the list (the count is then the sum of the list's places looked at, about 5 billion).
    void manyWaitingCellsAreToldFromOnesInTheWayByOneLookEach()
    {
        SpillIndexSwitch on(true);
        const int n = 100000;
        QString rows = rowXml(1, QStringLiteral("<c r=\"B1\" cm=\"1\"><f t=\"array\" ref=\"B1:B%1\">_xlfn.SEQUENCE(%1)</f><v>1</v></c>").arg(n));
        for (int i = 2; i <= n; ++i) rows += rowXml(i, constant(QStringLiteral("B%1").arg(i), QString::number(i)));
        Book b;
        QString why;
        QVERIFY(b.package.readLazy(withMetadata(blank(rows)), &why) && b.book.open(b.package, &why));
        b.calc.reset(new Calculation(b.book));
        QVERIFY(b.calc->load());
        // (not worked out yet: the cells the file stored are the cells of the store)
        MutationGuard &guard = b.calc->mutationGuard();
        const quintptr me = quintptr(QThread::currentThreadId());
        guard.setWorker(me + 1);
        bool done = false;
        jp::WorkThread worker([&] {
            guard.setWorker(quintptr(QThread::currentThreadId()));
            // (B1 gives one cell: the 99,999 below wait to be taken; then A3 asks for A3:B100000, over them)
            done = b.calc->setFormula(0, 0, 1, QStringLiteral("_xlfn.SEQUENCE(1)"), Calculation::Marking::Now, nullptr, true) && b.calc->recalculate().finished;
            done = done && b.calc->setFormula(0, 2, 0, QStringLiteral("_xlfn.SEQUENCE(99998,2)"), Calculation::Marking::Now, nullptr, true) && b.calc->recalculate().finished;
        });
        worker.join();
        QVERIFY(done);
        QCOMPARE(b.calc->spillBlock(0, 2, 0).valid ? rangeName(b.calc->spillBlock(0, 2, 0)) : QString(), QStringLiteral("A3:B100000"));
        QVERIFY(b.calc->hasStrays());
        QVERIFY(b.calc->strayChecks() >= n - 3);
        QVERIFY(b.calc->strayChecks() <= n + 3);
        guard.enter();
        b.calc->releaseStrays();
        guard.leave();
        guard.setWorker(0);
    }

    // The table made again after an edit and the table a load makes are made of the same ranges: the one each formula's cell names. A range that is in the sheet's list and that no formula names (one that was
    // not let go of, or is there twice) is no block.
    void theTableMadeAgainAgreesWithALoadOnAStaleRange()
    {
        SpillIndexSwitch on(true);
        Book b;
        QVERIFY(b.open(blank()));
        QVERIFY(b.type(QStringLiteral("A1"), QStringLiteral("_xlfn.SEQUENCE(3)")));
        QCOMPARE(blocksOf(*b.calc, 0), std::vector<QString>{QStringLiteral("0:0:2:0")});
        Sheet &s = const_cast<Sheet &>(b.calc->sheet(0));   // (a range of the list that no formula names: the same anchor, larger)
        s.ranges.append(CellRange{0, 0, 4, 0, true});
        Calculation::EditRecord rec;
        QVERIFY(b.calc->applyEdit(StructEdit::insertRows(0, 0, 1), &rec));
        QVERIFY(b.settle());
        QCOMPARE(blocksOf(*b.calc, 0), std::vector<QString>{QStringLiteral("1:0:3:0")});
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
