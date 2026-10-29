// Tests of rows, columns, and cells inserted and deleted in a calculation (Calculation::applyEdit): the cells move, every formula that reads cells that moved is written again with its
// references following them (on every sheet, and in the defined names), a reference to deleted cells is #REF!, what depends is worked out again, and Undo puts everything back. Counted
// work only: what moved, and what was written again. Every workbook is built in memory.
#include "sheethelpers.h"

#include "calc/formula.h"
#include "calc/functions.h"
#include "sheet/calculation.h"
#include "sheet/formulaframe.h"
#include "sheet/refshift.h"
#include "sheet/tables.h"

#include <QRandomGenerator>

#include <memory>
#include <numeric>

namespace {

// A sheet's cells from "A1=1" (a number), "B1=text", "C1==SUM(A1:A5)" (a formula), in any order.
QByteArray sheetXml(const QStringList &specs)
{
    struct Item {
        quint32 row, col;
        QString xml;
    };
    QVector<Item> items;
    for (const QString &spec : specs) {
        const int eq = int(spec.indexOf(QLatin1Char('=')));
        const QString ref = spec.left(eq), value = spec.mid(eq + 1);
        quint32 row = 0, col = 0;
        parseCellRef(ref, &row, &col);
        QString xml;
        if (value.startsWith(QLatin1Char('='))) {
            xml = QStringLiteral("<c r=\"%1\"><f>%2</f><v>0</v></c>").arg(ref, value.mid(1).toHtmlEscaped());
        } else if (value.startsWith(QLatin1Char('{'))) {   // ("D5={SUM(A1:A3*B1:B3)": a formula of one cell, entered as an array)
            xml = QStringLiteral("<c r=\"%1\"><f t=\"array\" ref=\"%1\">%2</f><v>0</v></c>").arg(ref, value.mid(1).toHtmlEscaped());
        } else {
            bool number = false;
            value.toDouble(&number);
            xml = number ? QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value)
                         : QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, value.toHtmlEscaped());
        }
        items.append({row, col, xml});
    }
    std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    QString rows;
    for (int i = 0; i < items.size();) {
        const quint32 row = items[i].row;
        rows += QStringLiteral("<row r=\"%1\">").arg(row + 1);
        for (; i < items.size() && items[i].row == row; ++i) rows += items[i].xml;
        rows += QStringLiteral("</row>");
    }
    return worksheet(rows);
}

struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool idFrame = false;   // the formulas are held in the id frame (CalcOptions::idFrame)
    bool forceOff = false;  // held in places whatever the process says (JC_ID_FRAME): for the books a test compares with ones in ids
    QStringList tableParts;   // the XML of the tables of the first sheet (see withTables)
    // `after` is what follows the cells in the first sheet (merged ranges), `names` the inside of definedNames.
    bool open(const QVector<QStringList> &sheets, const QString &after = QString(), const QString &names = QString())
    {
        QVector<QByteArray> parts;
        for (int i = 0; i < sheets.size(); ++i) {
            QByteArray xml = sheetXml(sheets[i]);
            if (i == 0 && !after.isEmpty()) xml.replace("</worksheet>", (after + QStringLiteral("</worksheet>")).toUtf8());
            parts << xml;
        }
        QString why;
        QByteArray zip = buildXlsx(parts, QByteArray(), QString(), QString(), QByteArray(), QStringList(), names);
        if (!tableParts.isEmpty()) zip = withTables(zip, tableParts);
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.keepApart = true;
        options.workOutWhatHasNoResult = true;
        options.workOutTables = !tableParts.isEmpty();
        options.idFrame = idFrame;
        options.idFrameOff = forceOff;
        calc.reset(new Calculation(book, options));
        if (!calc->load()) return false;
        finish();
        return true;
    }
    // The first sheet as the XML of its part.
    bool openXml(const QByteArray &sheet) { return openXmls({sheet}); }
    bool openXmls(const QVector<QByteArray> &sheets)
    {
        QString why;
        const QByteArray zip = buildXlsx(sheets);
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.keepApart = true;
        options.workOutWhatHasNoResult = true;
        options.idFrame = idFrame;
        options.idFrameOff = forceOff;
        calc.reset(new Calculation(book, options));
        if (!calc->load()) return false;
        finish();
        return true;
    }
    static quint32 row(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return r;
    }
    static quint32 col(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return c;
    }
    QString at(const QString &ref, int sheet = 0) const { return calc->cell(sheet, row(ref), col(ref)).debug(); }
    QString formula(const QString &ref, int sheet = 0) const { return calc->content(sheet, row(ref), col(ref)).formula; }
    bool has(const QString &ref, int sheet = 0) const { return calc->content(sheet, row(ref), col(ref)).exists; }
    void finish() { while (!calc->recalculate().finished) {} }
    // As the session does an edit of the structure: the tables of the workbook follow the cells, and the formulas that name a table read it where it is now.
    bool applyWithTables(const StructEdit &e, Calculation::EditRecord *rec, std::vector<TablePlace> *places)
    {
        const bool ok = calc->applyEdit(e, rec);
        if (ok) {
            *places = book.moveTables(e);
            if (!places->empty()) calc->tablesMovedByEdit();
        }
        finish();
        return ok;
    }
    bool undoWithTables(const Calculation::EditRecord &rec, const std::vector<TablePlace> &places)
    {
        const bool ok = calc->undoEdit(rec);
        if (ok) {
            book.putTablesBack(places);
            if (!places.empty()) calc->tablesMovedByEdit();
        }
        finish();
        return ok;
    }
    QStringList tablePlaces() const
    {
        QStringList out;
        for (const Table &t : book.tables())
            out << QStringLiteral("%1 sheet %2 %3,%4:%5,%6 totals %7 columns %8").arg(t.name).arg(t.sheet).arg(t.row1).arg(t.col1).arg(t.row2).arg(t.col2).arg(t.totalsRows).arg(t.columns.join(QLatin1Char('|')));
        return out;
    }
    bool apply(const StructEdit &e, Calculation::EditRecord *rec)
    {
        Calculation::EditRecord local;
        const bool ok = calc->applyEdit(e, rec ? rec : &local);
        finish();
        return ok;
    }
};

} // namespace

class SheetStructureTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A row inserted: the cells at and below it move down, formulas that read them (on any sheet) follow, a range with the row inside grows, and what depends has its value again.
    void insertingARowMovesTheCellsAndTheReferences()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(A1:A5)", "B2==A5*2", "B3==A2+1", "C1==$A$4+A$3", "D1==SUM(A:A)"}, {"A1==Sheet1!A5+Sheet1!B1", "A2==SUM(Sheet1!A1:A3)"}}));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 1), &rec));                 // a row at row 3
        QCOMPARE(b.at("A1"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:2"));
        QVERIFY(!b.has("A3"));
        QCOMPARE(b.at("A4"), QStringLiteral("n:3"));
        QCOMPARE(b.at("A6"), QStringLiteral("n:5"));
        QCOMPARE(b.formula("B1"), QStringLiteral("SUM(A1:A6)"));                // (the range grew)
        QCOMPARE(b.formula("B2"), QStringLiteral("A6*2"));
        QCOMPARE(b.formula("B4"), QStringLiteral("A2+1"));                       // (B3 moved down to B4 and still reads A2)
        QCOMPARE(b.formula("C1"), QStringLiteral("$A$5+A$4"));
        QCOMPARE(b.formula("D1"), QStringLiteral("SUM(A:A)"));
        QCOMPARE(b.formula("A1", 1), QStringLiteral("Sheet1!A6+Sheet1!B1"));    // (another sheet's formula)
        QCOMPARE(b.formula("A2", 1), QStringLiteral("SUM(Sheet1!A1:A4)"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:10"));
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:20"));
        QCOMPARE(b.at("A2", 1), QStringLiteral("n:6"));                         // (1 + 2 + the empty row + 3)
        // the graph follows: a change to a cell that moved is read by what reads it
        QVERIFY(b.calc->setValue(0, 5, 0, Value::number(50)));                   // A6
        b.finish();
        QCOMPARE(b.at("B2"), QStringLiteral("n:100"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:60"));
        // a value put in the row that was made is read by the range that grew over it
        QVERIFY(b.calc->setValue(0, 2, 0, Value::number(1000)));                 // A3
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:1060"));
        // counted: no cell moved (the rows are held by id, and the edit is a change to their order)
        QCOMPARE(rec.cellsMoved, qint64(0));
    }

    // Rows and columns inserted and deleted move no cell: a cell is held by the id of its row and of its column, and the edit changes the order of those. A cell of a row that is deleted is held
    // while the step can be undone and is the very cell that comes back; the count leaves it out; and the edit done again after Undo brings back the rows it made, so that edits undone and
    // done again use no ids up.
    void rowsAndColumnsInsertedAndDeletedMoveNoCellAndUndoBringsTheSameCellsBack()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "B1==SUM(A1:A4)", "B3==A3*2", "C2=text", "C4=more", "D3==B3+1"}}));
        const Sheet &s = b.calc->sheet(0);
        const qint64 cells = s.cells.cellCount();
        QCOMPARE(cells, qint64(9));
        const Cell *a3 = s.cells.find(2, 0);
        const Cell *b3 = s.cells.find(2, 1);
        const Cell *d3 = s.cells.find(2, 3);
        QVERIFY(a3 && b3 && d3);
        const qint64 movedBefore = b.calc->editStats().cellsMoved;
        // a row above and a column before: every cell is where it was held, with a new place
        Calculation::EditRecord rows, cols;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 2), &rows));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), &cols));
        QCOMPARE(rows.cellsMoved, qint64(0));
        QCOMPARE(cols.cellsMoved, qint64(0));
        QCOMPARE(b.calc->editStats().cellsMoved, movedBefore);
        QVERIFY(s.cells.find(4, 1) == a3 && s.cells.find(4, 2) == b3 && s.cells.find(4, 4) == d3);
        QCOMPARE(b.formula("C5", 0), QStringLiteral("B5*2"));                      // (the formula's node was told where it is)
        QCOMPARE(b.at("C5"), QStringLiteral("n:6"));
        QCOMPARE(b.at("E5"), QStringLiteral("n:7"));
        QCOMPARE(s.cells.cellCount(), cells);
        // rows deleted: the cells are held and left out of the count and of every walk
        Calculation::EditRecord del;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 4, 2), &del));                   // the rows of A3 and A4 (now 5 and 6)
        QCOMPARE(del.cellsMoved, qint64(0));
        QCOMPARE(s.cells.cellCount(), qint64(4));
        QCOMPARE(s.cells.deadCount(), qint64(5));
        qint64 walked = 0;
        s.cells.forEach([&](quint32 row, quint32, const Cell &) {
            ++walked;
            QVERIFY(row != 4 && row != 5);
        });
        QCOMPARE(walked, qint64(4));
        QVERIFY(!s.cells.find(4, 1));
        const quint32 nextRowId = s.rows().nextId().value();
        // Undo gives back the same cells, and the count; the edit done again with the same record brings back the same rows, and Undo of it, and so on, using no ids
        QVERIFY(b.calc->undoEdit(del));
        b.finish();
        QCOMPARE(s.cells.cellCount(), cells);
        QCOMPARE(s.cells.deadCount(), qint64(0));
        QVERIFY(s.cells.find(4, 1) == a3 && s.cells.find(4, 2) == b3 && s.cells.find(4, 4) == d3);
        QCOMPARE(b.at("C5"), QStringLiteral("n:6"));
        QCOMPARE(b.formula("E5"), QStringLiteral("C5+1"));
        for (int i = 0; i < 5; ++i) {
            QVERIFY(b.apply(StructEdit::deleteRows(0, 4, 2), &del));
            QCOMPARE(s.cells.cellCount(), qint64(4));
            QVERIFY(b.calc->undoEdit(del));
            b.finish();
            QCOMPARE(s.cells.cellCount(), cells);
        }
        QCOMPARE(s.rows().nextId().value(), nextRowId);
        QVERIFY(s.cells.find(4, 1) == a3);
        // a column deleted, and an insert undone and done again
        Calculation::EditRecord col;
        QVERIFY(b.apply(StructEdit::deleteColumns(0, 1, 1), &col));
        QCOMPARE(s.cells.cellCount(), qint64(cells - 4));
        QVERIFY(b.calc->undoEdit(col));
        b.finish();
        QCOMPARE(s.cells.cellCount(), cells);
        QVERIFY(s.cells.find(4, 1) == a3 && s.cells.find(4, 4) == d3);
        const quint32 nextColId = s.cols().nextId().value();
        for (int i = 0; i < 4; ++i) {
            QVERIFY(b.calc->undoEdit(cols));
            QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), &cols));
        }
        b.finish();
        QCOMPARE(s.cols().nextId().value(), nextColId);
        QVERIFY(s.cells.find(4, 1) == a3 && s.cells.find(4, 4) == d3);
        QCOMPARE(b.at("E5"), QStringLiteral("n:7"));
    }

    // Undo, and an edit done again after Undo, take no ids and no runs, so the limits of the order never refuse them (an edit that is new may be refused at the limit, until the order is compacted).
    void undoAndRedoNeedNoIdsSoTheLimitsNeverRefuseThem()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "B1==SUM(A1:A3)"}}));
        jp::calc::LineLimits cols = jp::calc::LineLimits::cols();
        cols.idLimit = cols.firstNew + 10;
        b.calc->setLineLimitsForTest(0, jp::calc::LineLimits::rows(), cols);
        std::vector<Calculation::EditRecord> recs(10);
        int made = 0;
        while (made < 10 && b.apply(StructEdit::insertColumns(0, 0, 1), &recs[size_t(made)])) ++made;
        QVERIFY2(made >= 3 && made < 10, qPrintable(QString::number(made)));
        const quint32 next = b.calc->sheet(0).cols().nextId().value();
        Calculation::EditRecord fresh;
        QVERIFY(!b.apply(StructEdit::insertColumns(0, 0, 1), &fresh));    // (a new edit is refused: the ids are used up)
        for (int i = made - 1; i >= 0; --i) {
            QVERIFY2(b.calc->undoEdit(recs[size_t(i)]), qPrintable(QString::number(i)));
            b.finish();
        }
        QCOMPARE(b.at("B1"), QStringLiteral("n:6"));
        for (int i = 0; i < made; ++i) QVERIFY2(b.apply(StructEdit::insertColumns(0, 0, 1), &recs[size_t(i)]), qPrintable(QString::number(i)));
        QCOMPARE(b.calc->sheet(0).cols().nextId().value(), next);
        QCOMPARE(b.at(QStringLiteral("%1%2").arg(QLatin1Char(char('A' + made + 1))).arg(1)), QStringLiteral("n:6"));
    }

    // An Undo or a Redo that the order does not take (the edit is not the last made to it) is refused, and makes no other rows or columns in their place.
    void aRedoThatTheOrderDoesNotTakeIsRefusedAndMakesNoNewIds()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "B1==SUM(A1:A3)"}}));
        Calculation::EditRecord a, other;
        QVERIFY(b.apply(StructEdit::insertRows(0, 1, 1), &a));
        QVERIFY(b.calc->undoEdit(a));
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 1), &other));
        const quint32 next = b.calc->sheet(0).rows().nextId().value();
        const qint64 cells = b.calc->sheet(0).cells.cellCount();
        QVERIFY(!b.apply(StructEdit::insertRows(0, 1, 1), &a));      // (a is not the last edit made: its Redo is refused, not made again with new rows)
        QCOMPARE(b.calc->sheet(0).rows().nextId().value(), next);
        QCOMPARE(b.calc->sheet(0).cells.cellCount(), cells);
        QCOMPARE(b.at("A2"), QStringLiteral("n:2"));
        // and the Undo of an edit that is not the last made
        Calculation::EditRecord first, second;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &first));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &second));
        QVERIFY(!b.calc->undoEdit(first));
        QCOMPARE(b.calc->sheet(0).rows().nextId().value(), next + 2);
    }

    // A record that is let go of releases what it held: the copy of the cells its edit deleted, and the ids and dead cells of its rows or columns. That is so for a step that is dropped, and for a
    // Redo the order does not take (the record is let go of, not left holding them).
    void aRecordThatIsLetGoOfReleasesItsGoneCopyAndItsLines()
    {
        {
            Book b;
            QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "B1==SUM(A1:A3)"}}));
            Calculation::EditRecord del;
            QVERIFY(b.apply(StructEdit::deleteRows(0, 0, 2), &del));
            QVERIFY(!del.gone.empty());
            QCOMPARE(b.calc->sheet(0).cells.deadCount(), qint64(del.gone.size()));
            b.calc->forgetLines(&del);
            QVERIFY(del.gone.empty() && del.gone.capacity() == 0);
            QCOMPARE(b.calc->sheet(0).cells.deadCount(), qint64(0));
        }
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "B1==SUM(A1:A3)"}}));
        Calculation::EditRecord a, other;
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), &a));
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(9)));     // (a cell in the column that is made)
        QVERIFY(b.calc->undoEdit(a));                             // (it dies with the column: held, dead)
        QCOMPARE(b.calc->sheet(0).cells.deadCount(), qint64(1));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 3, 1), &other));
        QVERIFY(!b.apply(StructEdit::insertColumns(0, 0, 1), &a));   // (not the last edit made: the Redo is refused)
        QCOMPARE(b.calc->sheet(0).cells.deadCount(), qint64(0));    // (and what the record held is let go of)
        QCOMPARE(b.calc->sheet(0).cols().core().deadCount(), size_t(1));   // (only the records of the other edit's lines are left: its refill)
    }

    // A row pushed off the end of the sheet is what Undo gives back: a formula in the last row is not pushed off (the edit is refused, in words), and a cell with a format only is, kept as a deleted
    // cell's is and put back.
    void aRowPushedOffTheEndOfTheSheetComesBackWithUndo()
    {
        {
            Book b;
            QVERIFY(b.open({{"A1=1", "A1048576==A1*2"}}));
            QCOMPARE(b.at("A1048576"), QStringLiteral("n:2"));
            QString why;
            QVERIFY(!b.calc->canApply(StructEdit::insertRows(0, 1, 1), &why));
            QVERIFY2(why.contains(QLatin1String("push")), qPrintable(why));
            Calculation::EditRecord rec;
            QVERIFY(!b.apply(StructEdit::insertRows(0, 1, 1), &rec));
            QCOMPARE(b.formula("A1048576"), QStringLiteral("A1*2"));
        }
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2"}}));
        QVERIFY(b.calc->setStyle(0, 1048575, 0, 1));
        QVERIFY(b.calc->setStyle(0, 5, 16383, 2));
        Calculation::EditRecord rows, cols;
        QVERIFY(b.apply(StructEdit::insertRows(0, 1, 1), &rows));
        QCOMPARE(rows.gone.size(), size_t(1));                       // (the cell that was pushed off)
        QVERIFY(!b.calc->content(0, 1048575, 0).exists || b.calc->content(0, 1048575, 0).style == 0);
        QVERIFY(b.apply(StructEdit::insertColumns(0, 1, 1), &cols));
        QCOMPARE(cols.gone.size(), size_t(1));
        QVERIFY(b.calc->undoEdit(cols));
        QVERIFY(b.calc->undoEdit(rows));
        b.finish();
        QCOMPARE(b.calc->content(0, 1048575, 0).style, quint32(1));
        QCOMPARE(b.calc->content(0, 5, 16383).style, quint32(2));
    }

    // A long session of edits of columns, with Undo, Redo, and new edits that drop what was undone, the way the session does it: every record that is dropped is let go of, the order is compacted (and
    // every record let go of) when it is near its limits, no edit is refused, and the sheet is always the sheet that a calculation with no limits gives for the same edits.
    void aLongSessionOfEditsOfColumnsNeverRunsOutAndStaysRight()
    {
        QStringList specs;
        for (int r = 1; r <= 6; ++r)
            for (int c = 0; c < 8; ++c) specs << QStringLiteral("%1%2=%3").arg(QLatin1Char(char('A' + c))).arg(r).arg(r * 10 + c);
        for (int r = 1; r <= 6; ++r) specs << QStringLiteral("I%1==SUM(A%1:H%1)").arg(r);
        Book limited, plain;
        QVERIFY(limited.open({specs}));
        QVERIFY(plain.open({specs}));
        jp::calc::LineLimits cols = jp::calc::LineLimits::cols();
        cols.idLimit = cols.firstNew + 300;
        cols.rebaseRuns = 30;
        cols.maxRuns = 500;
        limited.calc->setLineLimitsForTest(0, jp::calc::LineLimits::rows(), cols);
        struct Pair {
            Calculation::EditRecord a, b;
        };
        std::vector<Pair> done, undone;
        auto snapshot = [](Book &book) {
            QStringList out;
            for (quint32 r = 0; r < 7; ++r)
                for (quint32 c = 0; c < 24; ++c) {
                    const Calculation::CellContent content = book.calc->content(0, r, c);
                    if (content.exists) out << QStringLiteral("%1,%2 %3 %4").arg(r).arg(c).arg(content.formula, book.calc->cell(0, r, c).debug());
                }
            return out.join(QLatin1Char('\n'));
        };
        std::mt19937 rng(20261101);
        QStringList log;
        int compactions = 0, undos = 0, redos = 0, edits = 0;
        qint64 mostDead = 0;
        for (int i = 0; i < 3000; ++i) {
            const int roll = int(rng() % 10);
            log << QStringLiteral("%1 roll %2 state %3 done %4 undone %5").arg(i).arg(roll).arg(limited.calc->sheet(0).cols().state()).arg(done.size()).arg(undone.size());
            if (log.size() > 12) log.removeFirst();
            if (roll < 2 && !done.empty()) {
                Pair p = std::move(done.back());
                done.pop_back();
                QVERIFY(limited.calc->undoEdit(p.a));
                QVERIFY(plain.calc->undoEdit(p.b));
                undone.push_back(std::move(p));
                ++undos;
            } else if (roll < 3 && !undone.empty()) {
                Pair p = std::move(undone.back());
                undone.pop_back();
                QVERIFY2(limited.apply(p.a.edit, &p.a), qPrintable(log.join(QLatin1Char('\n'))));
                QVERIFY(plain.apply(p.b.edit, &p.b));
                done.push_back(std::move(p));
                ++redos;
            } else {
                while (!undone.empty()) {
                    limited.calc->forgetLines(&undone.back().a);
                    undone.pop_back();
                }
                const quint32 at = rng() % 10, count = 1 + rng() % 2;
                const StructEdit e = rng() % 2 ? StructEdit::insertColumns(0, at, count) : StructEdit::deleteColumns(0, at, count);
                if (limited.calc->linesNeedCompaction(e)) {
                    for (Pair &p : done) limited.calc->forgetLines(&p.a);
                    done.clear();
                    QVERIFY(limited.calc->compactLines(0, false));
                    ++compactions;
                }
                Pair p;
                const bool ok = limited.apply(e, &p.a);
                const bool okPlain = plain.apply(e, &p.b);
                QVERIFY2(ok == okPlain, qPrintable(QStringLiteral("edit %1: %2 with limits, %3 without").arg(i).arg(ok).arg(okPlain)));
                if (ok) {
                    done.push_back(std::move(p));
                    ++edits;
                }
                while (done.size() > 40) {
                    limited.calc->forgetLines(&done.front().a);
                    done.erase(done.begin());
                }
            }
            if (i % 50 == 0) QCOMPARE(snapshot(limited), snapshot(plain));
            mostDead = std::max(mostDead, limited.calc->sheet(0).cells.deadCount());
        }
        QCOMPARE(snapshot(limited), snapshot(plain));
        QVERIFY2(compactions > 5 && undos > 100 && redos > 30 && edits > 1500, qPrintable(QStringLiteral("%1 %2 %3 %4").arg(compactions).arg(undos).arg(redos).arg(edits)));
        // (what is held of lines that are dead is what the steps that are still held could bring back, and no more: 40 steps of 2 columns of 7 rows, and the 12 formulas that read them)
        QVERIFY2(mostDead <= 40 * 2 * 7 + 100, qPrintable(QString::number(mostDead)));
        QVERIFY(limited.calc->sheet(0).cols().nextId().value() <= cols.idLimit);
    }

    // Cells put in another place after rows and columns were inserted and deleted (a block cut and pasted, cells shifted down and to the right, a sheet copied) are where the places say, and
    // the sheet copied is the same cells in the same places, apart from the original.
    void cellsMovedAndSheetsCopiedAfterStructuralEditsAreByPlace()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==A1*10", "B3==A3*10", "B5==A5*10", "C2=x", "C4=y"}}));
        Calculation::EditRecord r1, r2, r3, r4;
        QVERIFY(b.apply(StructEdit::insertRows(0, 1, 1), &r1));       // A2 blank; the values are in A1, A3 to A6
        QVERIFY(b.apply(StructEdit::deleteRows(0, 3, 1), &r2));       // the 3 (now at row 4) goes: 1, blank, 2, 4, 5
        QVERIFY(b.apply(StructEdit::insertColumns(0, 1, 1), &r3));    // B blank
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &r4));       // a row at the top
        QCOMPARE(b.at("A2"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A4"), QStringLiteral("n:2"));
        QCOMPARE(b.at("A5"), QStringLiteral("n:4"));
        QCOMPARE(b.at("A6"), QStringLiteral("n:5"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:10"));
        QCOMPARE(b.at("C6"), QStringLiteral("n:50"));
        // the sheet copied is these cells in these places, and separate from the original
        Sheet copy = b.calc->sheet(0);
        QCOMPARE(copy.cells.cellCount(), b.calc->sheet(0).cells.cellCount());
        QStringList original, copied;
        b.calc->sheet(0).cells.forEach([&](quint32 row, quint32 col, const Cell &c) { original << QStringLiteral("%1,%2:%3:%4").arg(row).arg(col).arg(c.number).arg(int(c.type)); });
        copy.cells.forEach([&](quint32 row, quint32 col, const Cell &c) { copied << QStringLiteral("%1,%2:%3:%4").arg(row).arg(col).arg(c.number).arg(int(c.type)); });
        QCOMPARE(copied, original);
        Cell extra;
        extra.type = CellType::Number;
        extra.number = 77;
        copy.cells.set(3, 0, extra);
        QCOMPARE(b.at("A4"), QStringLiteral("n:2"));
        QCOMPARE(copy.cells.find(3, 0)->number, 77.0);
        QVERIFY(copy.cells.find(2, 0) == nullptr && b.calc->sheet(0).cells.find(2, 0) == nullptr);
        QVERIFY(copy.cells.find(5, 0) != b.calc->sheet(0).cells.find(5, 0));
        // a block cut and pasted: A5:A6 (4 and 5) to E2, formulas that read the block follow it
        Calculation::EditRecord cut;
        CellRange block;
        block.valid = true;
        block.row1 = 4;
        block.row2 = 5;
        block.col1 = block.col2 = 0;
        QVERIFY(b.apply(StructEdit::move(0, block, 0, 1, 4), &cut));
        QCOMPARE(b.at("E2"), QStringLiteral("n:4"));
        QCOMPARE(b.at("E3"), QStringLiteral("n:5"));
        QVERIFY(!b.has("A5"));
        QVERIFY(!b.has("A6"));
        QCOMPARE(b.formula("C6"), QStringLiteral("E3*10"));
        QCOMPARE(b.at("C6"), QStringLiteral("n:50"));
        // cells shifted down in column A (the 1 and the 2 move), and Undo of both
        Calculation::EditRecord down;
        CellRange cellsDown;
        cellsDown.valid = true;
        cellsDown.row1 = 1;
        cellsDown.row2 = 1;
        cellsDown.col1 = cellsDown.col2 = 0;
        QVERIFY(b.apply(StructEdit::insertCellsDown(0, cellsDown), &down));
        QCOMPARE(b.at("A3"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A5"), QStringLiteral("n:2"));
        QVERIFY(b.calc->undoEdit(down));
        b.finish();
        QCOMPARE(b.at("A2"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A4"), QStringLiteral("n:2"));
        QVERIFY(b.calc->undoEdit(cut));
        b.finish();
        QCOMPARE(b.at("A5"), QStringLiteral("n:4"));
        QCOMPARE(b.at("A6"), QStringLiteral("n:5"));
        QVERIFY(!b.has("E2"));
    }

    // Rows deleted: the cells below move up, a range loses the rows deleted from it, a reference to a deleted cell is #REF!, and Undo puts everything back as it was (the cells, the
    // formulas, and the values).
    void deletingRowsMakesRefAndUndoPutsItBack()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(A1:A5)", "B2==A3*2", "B3==A5*2", "B4==A2+1", "C1==B2+1"}}));
        auto snapshot = [&] {
            QStringList out;
            for (int r = 1; r <= 8; ++r)
                for (const char *c : {"A", "B", "C"}) {
                    const QString ref = QString::fromLatin1(c) + QString::number(r);
                    out << ref + QLatin1Char(':') + (b.has(ref) ? b.at(ref) + QLatin1Char(' ') + b.formula(ref) : QStringLiteral("-"));
                }
            return out.join(QLatin1Char('\n'));
        };
        const QString before = snapshot();
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 2, 2), &rec));                 // rows 3 and 4
        QCOMPARE(b.formula("B1"), QStringLiteral("SUM(A1:A3)"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:8"));                             // 1 + 2 + 5
        QCOMPARE(b.at("A3"), QStringLiteral("n:5"));
        QCOMPARE(b.formula("B3"), QString());                                    // (B3 and B4 were in the rows that went)
        QCOMPARE(b.formula("B2"), QStringLiteral("#REF!*2"));
        QCOMPARE(b.at("B2"), QStringLiteral("e:#REF!"));
        QCOMPARE(b.at("C1"), QStringLiteral("e:#REF!"));                         // what reads it has the error too
        QCOMPARE(rec.gone.size(), size_t(4));                                    // A3, B3, A4, B4 (B3 has a formula)
        // Undo: all as it was
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        QCOMPARE(snapshot(), before);
        // and the edit again
        Calculation::EditRecord again;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 2, 2), &again));
        QCOMPARE(b.formula("B1"), QStringLiteral("SUM(A1:A3)"));
        QVERIFY(b.calc->undoEdit(again));
        b.finish();
        QCOMPARE(snapshot(), before);
    }

    // Columns inserted and deleted, the same way; and cells shifted down, up, right, and left (only the references that lie in the columns (rows) of the block follow).
    void columnsAndShiftedCellsFollowTheSameRules()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "B1=2", "C1=3", "D1=4", "A2=5", "B2=6", "C2=7", "D2=8", "E1==SUM(A1:D1)", "E2==C2+B1", "F1==SUM(A:B)"}}));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertColumns(0, 1, 1), &rec));              // a column at B
        QCOMPARE(b.formula("F1"), QStringLiteral("SUM(A1:E1)"));                // (E moved to F)
        QCOMPARE(b.formula("F2"), QStringLiteral("D2+C1"));
        QCOMPARE(b.formula("G1"), QStringLiteral("SUM(A:C)"));
        QCOMPARE(b.at("F1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:2"));
        QVERIFY(!b.has("B1"));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        QCOMPARE(b.formula("E1"), QStringLiteral("SUM(A1:D1)"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:10"));
        Calculation::EditRecord del;
        QVERIFY(b.apply(StructEdit::deleteColumns(0, 1, 2), &del));              // columns B and C
        QCOMPARE(b.formula("C1"), QStringLiteral("SUM(A1:B1)"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:5"));                             // 1 + 4
        QCOMPARE(b.formula("C2"), QStringLiteral("#REF!+#REF!"));
        QCOMPARE(b.formula("D1"), QStringLiteral("SUM(A:A)"));
        QVERIFY(b.calc->undoEdit(del));
        b.finish();
        QCOMPARE(b.at("E2"), QStringLiteral("n:9"));
        // cells shifted down in B1:C1 (the cells below them in B and C move; the rest do not)
        CellRange block;
        parseRangeRef(QStringLiteral("B1:C1"), &block);
        Calculation::EditRecord down;
        QVERIFY(b.apply(StructEdit::insertCellsDown(0, block), &down));
        QCOMPARE(b.at("B2"), QStringLiteral("n:2"));
        QCOMPARE(b.at("B3"), QStringLiteral("n:6"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:5"));                             // (column A did not move)
        QCOMPARE(b.formula("E2"), QStringLiteral("C3+B2"));                      // (E2 stays, in a column the block is not in: what it reads in B and C moved)
        QCOMPARE(b.at("E2"), QStringLiteral("n:9"));
        QCOMPARE(b.formula("E1"), QStringLiteral("SUM(A1:D1)"));                // (a range across other columns is as it was)
        QVERIFY(b.calc->undoEdit(down));
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:2"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:6"));
    }

    // The defined names follow the cells they name, and Undo gives them back.
    void definedNamesFollowTheCells()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(Total)", "B2==Last*2"}}, QString(),
                       QStringLiteral("<definedName name=\"Total\">Sheet1!$A$1:$A$5</definedName><definedName name=\"Last\">Sheet1!$A$5</definedName>")));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:10"));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 2), &rec));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Total")), QStringLiteral("Sheet1!$A$1:$A$7"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Last")), QStringLiteral("Sheet1!$A$7"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:10"));
        QVERIFY(b.calc->setValue(0, 6, 0, Value::number(50)));                   // A7, what both names stand for part of
        b.finish();
        QCOMPARE(b.at("B2"), QStringLiteral("n:100"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:60"));
        QVERIFY(b.calc->setValue(0, 6, 0, Value::number(5)));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Total")), QStringLiteral("Sheet1!$A$1:$A$5"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        // a row deleted that a name stands for gives #REF!
        Calculation::EditRecord del;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 4, 1), &del));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Last")), QStringLiteral("Sheet1!#REF!"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Total")), QStringLiteral("Sheet1!$A$1:$A$4"));
        QCOMPARE(b.at("B2"), QStringLiteral("e:#REF!"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:10"));
    }

    // Formulas that share a text: the cells that move keep their formulas, the references among them follow the cells, and the group is split where the edit is in it.
    void sharedFormulasKeepTheirMeaning()
    {
        QString rows;
        for (int r = 1; r <= 5; ++r) {
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c>").arg(r);
            rows += r == 1 ? QStringLiteral("<c r=\"B1\"><f t=\"shared\" ref=\"B1:B5\" si=\"0\">A1*2+$A$1</f><v>3</v></c>")
                           : QStringLiteral("<c r=\"B%1\"><f t=\"shared\" si=\"0\"/><v>0</v></c>").arg(r);
            rows += QStringLiteral("</row>");
        }
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QCOMPARE(b.at("B3"), QStringLiteral("n:7"));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 1), &rec));
        const QStringList wanted{QStringLiteral("A1*2+$A$1"), QStringLiteral("A2*2+$A$1"), QString(), QStringLiteral("A4*2+$A$1"), QStringLiteral("A5*2+$A$1"), QStringLiteral("A6*2+$A$1")};
        for (int r = 1; r <= 6; ++r) QCOMPARE(b.formula(QStringLiteral("B%1").arg(r)), wanted[r - 1]);
        QCOMPARE(b.at("B6"), QStringLiteral("n:11"));
        QCOMPARE(b.at("B4"), QStringLiteral("n:7"));
        // a formula's own row moves it; deleting the first row (the master's) leaves the others as they were
        Calculation::EditRecord del;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 0, 1), &del));
        QCOMPARE(b.formula("B1"), QStringLiteral("A1*2+#REF!"));
        QCOMPARE(b.formula("B3"), QStringLiteral("A3*2+#REF!"));
        QVERIFY(b.calc->undoEdit(del));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        for (int r = 1; r <= 5; ++r) QCOMPARE(b.formula(QStringLiteral("B%1").arg(r)), QStringLiteral("A%1*2+$A$1").arg(r));
        QCOMPARE(b.at("B5"), QStringLiteral("n:11"));
    }

    // An edit that would cut through an array formula, or push cells off the sheet, is refused (and nothing changes); one that moves the array whole is made, and its range follows.
    void arraysAndTheEdgeOfTheSheetRefuseWhatCannotBeDone()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">A1:A3*2</f><v>2</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><v>4</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"C3\"><v>6</v></c></row>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QString why;
        QVERIFY(!b.calc->canApply(StructEdit::insertRows(0, 1, 1), &why));
        QVERIFY2(why.contains(QLatin1String("array")), qPrintable(why));
        QVERIFY(!b.calc->canApply(StructEdit::deleteRows(0, 1, 1), &why));
        QVERIFY(!b.calc->canApply(StructEdit::deleteRows(0, 0, 3), &why));
        Calculation::EditRecord none;
        QVERIFY(!b.calc->applyEdit(StructEdit::insertRows(0, 2, 1), &none));
        QCOMPARE(b.at("A3"), QStringLiteral("n:3"));
        QVERIFY(b.calc->canApply(StructEdit::insertRows(0, 3, 2), &why));          // below the array
        QVERIFY(b.calc->canApply(StructEdit::insertColumns(0, 0, 1), &why));       // left of it: it moves whole
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 2), &rec));                   // above it, and everything: moves whole
        QCOMPARE(b.at("C3"), QStringLiteral("n:2"));
        QCOMPARE(b.at("C5"), QStringLiteral("n:6"));
        QCOMPARE(b.calc->sheet(0).ranges.first().row1, quint32(2));
        QVERIFY(b.calc->setValue(0, 2, 0, Value::number(10)));                      // A3 (was A1)
        b.finish();
        QCOMPARE(b.at("C3"), QStringLiteral("n:20"));
        // cells at the end of the sheet that hold something are not pushed off it
        Book e;
        QVERIFY(e.open({{"A1=1", "A1048576=9"}}));
        QVERIFY(!e.calc->canApply(StructEdit::insertRows(0, 0, 1), &why));
        QVERIFY2(why.contains(QLatin1String("edge")), qPrintable(why));
        QVERIFY(e.calc->canApply(StructEdit::insertRows(0, 0, 1).inverse(), &why));
        // a protected sheet refuses
        Book p;
        QVERIFY(p.openXml(worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QStringLiteral("<sheetProtection sheet=\"1\"/>"))));
        QVERIFY(!p.calc->canApply(StructEdit::insertRows(0, 0, 1), &why));
        QVERIFY2(why.contains(QLatin1String("protected")), qPrintable(why));
    }

    // A formula kept apart for a function the engine lacks has the result the file stored until something it reads changes: then it has #NAME?, and so it has when a cell it read is
    // deleted (its text has #REF! in it, and nothing it reads changes any more, so it is given at once); Undo gives back its text.
    void aFormulaKeptApartForItsFunctionKeepsItsResultUnlessACellItReadsIsDeleted()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row><row r=\"3\"><c r=\"C3\"><f>_xll.NOPE(A2)</f><v>7</v></c></row>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QCOMPARE(b.at("C3"), QStringLiteral("n:7"));
        Calculation::EditRecord up, gone;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &up));   // the cell it reads is A3 now (it moved, which is a change of what it reads: #NAME?)
        QCOMPARE(b.formula("C4"), QStringLiteral("_xll.NOPE(A3)"));
        QCOMPARE(b.at("C4"), QStringLiteral("e:#NAME?"));
        QVERIFY(b.calc->undoEdit(up));
        b.finish();
        QCOMPARE(b.formula("C3"), QStringLiteral("_xll.NOPE(A2)"));
        QVERIFY(b.apply(StructEdit::deleteRows(0, 1, 1), &gone));   // A2 is deleted
        QCOMPARE(b.formula("C2"), QStringLiteral("_xll.NOPE(#REF!)"));
        QCOMPARE(b.at("C2"), QStringLiteral("e:#NAME?"));
        QVERIFY(b.calc->undoEdit(gone));
        b.finish();
        QCOMPARE(b.formula("C3"), QStringLiteral("_xll.NOPE(A2)"));
    }

    // The formulas of a rule of validation are written for the first cell of its range: when that cell is deleted, the first cell left is the one they are written for, so the references
    // the formula has that are not fixed follow it (the first row deleted does not make ISNUMBER($A2) read a cell that is gone).
    void aRulesRelativeReferencesAreWrittenForItsNewFirstCell()
    {
        QStringList cells;
        for (int r = 1; r <= 10; ++r) cells << QStringLiteral("A%1=%1").arg(r);
        const QString rule = QStringLiteral("<dataValidations count=\"1\"><dataValidation type=\"custom\" allowBlank=\"1\" showErrorMessage=\"1\" sqref=\"B2:B10\"><formula1>ISNUMBER($A2)</formula1></dataValidation></dataValidations>");
        {
            Book b;
            QVERIFY(b.open({cells}, rule));
            QVERIFY(b.apply(StructEdit::deleteRows(0, 1, 1), nullptr));   // the first row of the range
            const Validation &v = b.calc->sheet(0).validations.first();
            QCOMPARE(rangeName(v.ranges.first()), QStringLiteral("B2:B9"));
            QCOMPARE(v.formula1, QStringLiteral("ISNUMBER($A2)"));
            QVERIFY(b.calc->entryAllowed(0, 1, 1, v, jp::calc::Value::number(7), QStringLiteral("7")));
        }
        {
            Book b;
            QVERIFY(b.open({cells}, rule));
            QVERIFY(b.apply(StructEdit::deleteRows(0, 0, 3), nullptr));   // two rows above it and its first
            QCOMPARE(b.calc->sheet(0).validations.first().formula1, QStringLiteral("ISNUMBER($A1)"));
            QCOMPARE(rangeName(b.calc->sheet(0).validations.first().ranges.first()), QStringLiteral("B1:B7"));
        }
        {
            Book b;
            QVERIFY(b.open({cells}, rule));
            QVERIFY(b.apply(StructEdit::insertRows(0, 1, 2), nullptr));   // above its first row: it moves down with the cells it reads
            QCOMPARE(b.calc->sheet(0).validations.first().formula1, QStringLiteral("ISNUMBER($A4)"));
            QCOMPARE(rangeName(b.calc->sheet(0).validations.first().ranges.first()), QStringLiteral("B4:B12"));
        }
        {
            // a column deleted that the formula reads from the left of the range; and the first column of a rule on two columns, with a formula that reads the cell it is written for
            Book b;
            QVERIFY(b.open({cells}, QStringLiteral("<dataValidations count=\"1\"><dataValidation type=\"custom\" sqref=\"C2:D5\"><formula1>C2&gt;5</formula1></dataValidation></dataValidations>")));
            QVERIFY(b.apply(StructEdit::deleteColumns(0, 2, 1), nullptr));   // C, the first column of the range
            QCOMPARE(b.calc->sheet(0).validations.first().formula1, QStringLiteral("C2>5"));   // (it reads its own cell, as it did: D is C now)
            QCOMPARE(rangeName(b.calc->sheet(0).validations.first().ranges.first()), QStringLiteral("C2:C5"));
            QVERIFY(b.apply(StructEdit::deleteColumns(0, 0, 1), nullptr));   // A, which is left of the range
            QCOMPARE(b.calc->sheet(0).validations.first().formula1, QStringLiteral("B2>5"));
        }
        {
            Book b;
            QVERIFY(b.open({cells}, QStringLiteral("<dataValidations count=\"1\"><dataValidation type=\"custom\" sqref=\"C2:D5\"><formula1>A2&gt;B2</formula1></dataValidation></dataValidations>")));
            QVERIFY(b.apply(StructEdit::deleteColumns(0, 0, 1), nullptr));   // A, which the formula reads
            QCOMPARE(b.calc->sheet(0).validations.first().formula1, QStringLiteral("#REF!>A2"));
        }
    }

    // A one-cell array formula whose row is deleted is put back by Undo as an array formula (a plain formula would not be one): the range the file writes, the cell not editable alone, and no
    // range left over where the cell was.
    void aOneCellArrayFormulaDeletedAndUndoneIsAnArrayAgain()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "B1=4", "B2=5", "B3=6", "D5={SUM(A1:A3*B1:B3)", "D6=9"}}));
        QCOMPARE(b.at("D5"), QStringLiteral("n:32"));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 4, 1), &rec));
        // D5 holds what was in D6, a number that is not an array's; the one-cell range is gone with its cell
        QCOMPARE(b.at("D5"), QStringLiteral("n:9"));
        QVERIFY(!b.calc->content(0, 4, 3).arrayRange.valid);
        int live = 0;
        for (const CellRange &r : b.calc->sheet(0).ranges) live += r.valid ? 1 : 0;
        QCOMPARE(live, 0);
        QVERIFY(b.calc->setValue(0, 4, 3, Value::number(11)));   // (the cell is an ordinary one: it can be changed)
        QVERIFY(b.calc->setValue(0, 4, 3, Value::number(9)));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        const Calculation::CellContent c = b.calc->content(0, 4, 3);
        QVERIFY(c.isFormula);
        QVERIFY2(c.arrayRange.valid, "the array is a plain formula again");
        QCOMPARE(rangeName(c.arrayRange), QStringLiteral("D5"));
        QVERIFY(!c.editable);
        QCOMPARE(b.at("D5"), QStringLiteral("n:32"));
        const Cell *cell = b.calc->sheet(0).cells.find(4, 3);
        QVERIFY(cell && (cell->flags & ArrayFormula));
        QCOMPARE(b.at("D6"), QStringLiteral("n:9"));
    }

    // A whole array formula is cut and pasted whole (a one-cell array too): its cells, its formula, and its results go where the cells go, what reads it follows, and Undo (and Redo) gives it
    // back. Part of a bigger array is not cut; an array written over whole (one cell) is replaced, and part of a bigger one is not.
    void aWholeArrayCanBeCutAndPartOfOneCannot()
    {
        const QString rows = QStringLiteral(
            "<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">A1:A3*2</f><v>2</v></c><c r=\"E1\"><f t=\"array\" ref=\"E1\">SUM(A1:A3)</f><v>6</v></c><c r=\"G1\"><v>5</v></c><c r=\"H1\"><f>SUM(C1:C3)</f><v>12</v></c></row>"
            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><v>4</v></c></row>"
            "<row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"C3\"><v>6</v></c></row>");
        Book b;
        QVERIFY(b.openXmls({worksheet(rows), worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>7</v></c></row>"))}));
        QCOMPARE(b.at("C2"), QStringLiteral("n:4"));
        auto block = [](const QString &first, const QString &last) {
            CellRange r;
            r.valid = true;
            r.row1 = Book::row(first);
            r.col1 = Book::col(first);
            r.row2 = Book::row(last);
            r.col2 = Book::col(last);
            return r;
        };
        auto liveRanges = [&](int sheet) {
            QStringList out;
            for (const CellRange &r : b.calc->sheet(sheet).ranges)
                if (r.valid) out << rangeName(r);
            out.sort();
            return out;
        };
        QCOMPARE(liveRanges(0), (QStringList{"C1:C3", "E1"}));
        QString why;
        // part of an array is not cut
        QVERIFY(!b.calc->canApply(StructEdit::move(0, block("C1", "C2"), 0, 9, 9), &why));
        QVERIFY2(why.contains(QLatin1String("array")), qPrintable(why));
        QVERIFY(!b.calc->canApply(StructEdit::move(0, block("C2", "D3"), 0, 9, 9), &why));
        // and not written over in part
        QVERIFY(!b.calc->canApply(StructEdit::move(0, block("G1", "G1"), 0, 1, 2), &why));   // G1 onto C2
        QVERIFY2(why.contains(QLatin1String("array")), qPrintable(why));
        // the whole of it is
        QVERIFY(b.calc->canApply(StructEdit::move(0, block("B1", "D3"), 0, 9, 9), &why));
        Calculation::EditRecord first;
        QVERIFY(b.apply(StructEdit::move(0, block("C1", "C3"), 0, 4, 5), &first));   // C1:C3 to F5:F7
        QVERIFY(!b.has("C1") && !b.has("C2") && !b.has("C3"));
        Calculation::CellContent master = b.calc->content(0, 4, 5);
        QVERIFY(master.isFormula && master.arrayRange.valid);
        QCOMPARE(rangeName(master.arrayRange), QStringLiteral("F5:F7"));
        QCOMPARE(master.formula, QStringLiteral("A1:A3*2"));
        QVERIFY(!b.calc->content(0, 5, 5).editable && !b.calc->content(0, 6, 5).editable);   // (the members are the array's)
        QCOMPARE(b.at("F5"), QStringLiteral("n:2"));
        QCOMPARE(b.at("F6"), QStringLiteral("n:4"));
        QCOMPARE(b.at("F7"), QStringLiteral("n:6"));
        QCOMPARE(b.formula("H1"), QStringLiteral("SUM(F5:F7)"));
        QCOMPARE(b.at("H1"), QStringLiteral("n:12"));
        QCOMPARE(liveRanges(0), (QStringList{"E1", "F5:F7"}));
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(10)));   // A1 changed: the array at its new place is worked out again
        b.finish();
        QCOMPARE(b.at("F5"), QStringLiteral("n:20"));
        QCOMPARE(b.at("F6"), QStringLiteral("n:4"));
        QCOMPARE(b.at("H1"), QStringLiteral("n:30"));
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(1)));
        b.finish();
        QVERIFY(b.calc->undoEdit(first));
        b.finish();
        QVERIFY(b.has("C1") && b.has("C3") && !b.has("F5") && !b.has("F6") && !b.has("F7"));
        QCOMPARE(liveRanges(0), (QStringList{"C1:C3", "E1"}));
        master = b.calc->content(0, 0, 2);
        QVERIFY(master.isFormula && master.arrayRange.valid && !b.calc->content(0, 1, 2).editable);
        QCOMPARE(rangeName(master.arrayRange), QStringLiteral("C1:C3"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:4"));
        QCOMPARE(b.formula("H1"), QStringLiteral("SUM(C1:C3)"));
        QVERIFY(b.calc->setValue(0, 1, 0, Value::number(20)));   // A2 changed: the array at its old place is worked out again
        b.finish();
        QCOMPARE(b.at("C2"), QStringLiteral("n:40"));
        QVERIFY(b.calc->setValue(0, 1, 0, Value::number(2)));
        b.finish();
        // Redo
        Calculation::EditRecord again;
        QVERIFY(b.apply(StructEdit::move(0, block("C1", "C3"), 0, 4, 5), &again));
        QCOMPARE(liveRanges(0), (QStringList{"E1", "F5:F7"}));
        QCOMPARE(b.at("F7"), QStringLiteral("n:6"));
        QVERIFY(b.calc->undoEdit(again));
        b.finish();
        // an array moved down over itself (C1:C3 to C2:C4)
        QVERIFY(b.calc->canApply(StructEdit::move(0, block("C1", "C3"), 0, 1, 2), &why));
        Calculation::EditRecord slide;
        QVERIFY(b.apply(StructEdit::move(0, block("C1", "C3"), 0, 1, 2), &slide));
        QVERIFY(!b.has("C1"));
        QCOMPARE(liveRanges(0), (QStringList{"C2:C4", "E1"}));
        QCOMPARE(b.at("C4"), QStringLiteral("n:6"));
        QVERIFY(b.calc->undoEdit(slide));
        b.finish();
        QCOMPARE(liveRanges(0), (QStringList{"C1:C3", "E1"}));
        // a one-cell array
        Calculation::EditRecord one;
        QVERIFY(b.apply(StructEdit::move(0, block("E1", "E1"), 0, 3, 8), &one));   // E1 to I4
        master = b.calc->content(0, 3, 8);
        QVERIFY(master.isFormula && master.arrayRange.valid);
        QCOMPARE(rangeName(master.arrayRange), QStringLiteral("I4"));
        QVERIFY(!b.has("E1"));
        QCOMPARE(b.at("I4"), QStringLiteral("n:6"));
        QVERIFY(b.calc->undoEdit(one));
        b.finish();
        QVERIFY(b.calc->content(0, 0, 4).arrayRange.valid && !b.has("I4"));
        // to another sheet: what the formula reads stays where it is
        Calculation::EditRecord across;
        QVERIFY(b.apply(StructEdit::move(0, block("C1", "C3"), 1, 1, 1), &across));   // to Sheet2!B2:B4
        QCOMPARE(b.formula("B2", 1), QStringLiteral("Sheet1!A1:A3*2"));
        QCOMPARE(b.at("B3", 1), QStringLiteral("n:4"));
        QCOMPARE(liveRanges(1), (QStringList{"B2:B4"}));
        QCOMPARE(liveRanges(0), (QStringList{"E1"}));
        QCOMPARE(b.formula("H1"), QStringLiteral("SUM(Sheet2!B2:B4)"));
        QVERIFY(b.calc->undoEdit(across));
        b.finish();
        QCOMPARE(liveRanges(1), QStringList());
        QCOMPARE(liveRanges(0), (QStringList{"C1:C3", "E1"}));
        QVERIFY(!b.has("B2", 1));
        // an array of one cell is written over whole
        Calculation::EditRecord over;
        QVERIFY(b.apply(StructEdit::move(0, block("G1", "G1"), 0, 0, 4), &over));   // G1 onto E1
        QCOMPARE(b.at("E1"), QStringLiteral("n:5"));
        QVERIFY(b.calc->content(0, 0, 4).editable);
        QCOMPARE(liveRanges(0), (QStringList{"C1:C3"}));
        QVERIFY(b.calc->undoEdit(over));
        b.finish();
        QCOMPARE(liveRanges(0), (QStringList{"C1:C3", "E1"}));
        QCOMPARE(b.at("E1"), QStringLiteral("n:6"));
        QVERIFY(!b.calc->content(0, 0, 4).editable);
    }

    // The rules of conditional formatting follow the cells in the model as they do in the file: their ranges (a range that is all deleted takes its rule with it), and their formulas, written
    // for the first cell of the range; Undo gives them back.
    void conditionalFormatsFollowTheEditsAndComeBackWithUndo()
    {
        QString rows;
        for (int r = 1; r <= 12; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        const QString after = QStringLiteral("<conditionalFormatting sqref=\"B3:B6\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;$D$8</formula></cfRule></conditionalFormatting>"
                                             "<conditionalFormatting sqref=\"C1:C2\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"2\" operator=\"greaterThan\"><formula>$D$9</formula></cfRule></conditionalFormatting>"
                                             "<conditionalFormatting sqref=\"E10\"><cfRule type=\"colorScale\" priority=\"3\"><colorScale><cfvo type=\"min\"/><cfvo type=\"max\"/><color rgb=\"FFFFFFFF\"/><color rgb=\"FF63BE7B\"/></colorScale></cfRule></conditionalFormatting>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows, after)));
        auto rules = [&] {
            QStringList out;
            for (const CondRule &c : b.calc->sheet(0).conditionalFormats) {
                QStringList ranges;
                for (const CellRange &r : c.ranges) ranges << rangeName(r);
                out << ranges.join(QLatin1Char(' ')) + QLatin1Char('|') + c.formula1;
            }
            return out;
        };
        QCOMPARE(rules(), (QStringList{"B3:B6|$A3>$D$8", "C1:C2|$D$9", "E10|"}));
        Calculation::EditRecord first;
        QVERIFY(b.apply(StructEdit::insertRows(0, 1, 2), &first));                        // two rows above row 2
        QCOMPARE(rules(), (QStringList{"B5:B8|$A5>$D$10", "C1:C4|$D$11", "E12|"}));       // (the second range grew: the rows went in it)
        Calculation::EditRecord second;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 0, 4), &second));                       // the first four rows: all of C1:C4
        QCOMPARE(rules(), (QStringList{"B1:B4|$A1>$D$6", "E8|"}));
        Calculation::EditRecord third;
        QVERIFY(b.apply(StructEdit::deleteColumns(0, 1, 1), &third));                     // column B: all of the first rule's cells
        QCOMPARE(rules(), (QStringList{"E8|"}.mid(0, 0) + QStringList{"D8|"}));
        QVERIFY(b.calc->undoEdit(third));
        QVERIFY(b.calc->undoEdit(second));
        QCOMPARE(rules(), (QStringList{"B5:B8|$A5>$D$10", "C1:C4|$D$11", "E12|"}));
        QVERIFY(b.calc->undoEdit(first));
        QCOMPARE(rules(), (QStringList{"B3:B6|$A3>$D$8", "C1:C2|$D$9", "E10|"}));
    }

    // A cut to another sheet: a reference to cells that stay names the sheet the formula came from; one to cells that go with it follows them.
    void aCutToAnotherSheetKeepsTheCellsItReads()
    {
        Book b;
        QVERIFY(b.open({{"A1==B2*2+C1", "B2=7", "C1=1"}, {"B2=100", "C1=1000"}}));
        QCOMPARE(b.at("A1"), QStringLiteral("n:15"));
        CellRange block;
        block.valid = true;
        block.row1 = block.row2 = 0;
        block.col1 = block.col2 = 0;
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::move(0, block, 1, 0, 0), &rec));   // A1 alone, to Sheet2
        QCOMPARE(b.formula("A1", 1), QStringLiteral("Sheet1!B2*2+Sheet1!C1"));
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:15"));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        QCOMPARE(b.formula("A1"), QStringLiteral("B2*2+C1"));
        // the cells it reads go with it: B2 and C1 are in the block, so the formula reads them where they are now
        block.row1 = 0;
        block.row2 = 1;
        block.col1 = 0;
        block.col2 = 2;
        QVERIFY(b.apply(StructEdit::move(0, block, 1, 4, 4), &rec));   // A1:C2, to Sheet2!E5
        QCOMPARE(b.formula("E5", 1), QStringLiteral("F6*2+G5"));
        QCOMPARE(b.at("E5", 1), QStringLiteral("n:15"));
    }

    // A cut pasted over cells that other formulas read makes their references #REF!, as the programs people know do; a range that only partly covers the cells written over stays as it is.
    void aCutOverCellsThatAreReadMakesTheirReferencesGone()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "B1=2", "C1==B1+10", "D1==SUM(B1:B3)", "B3=5", "D2==A1+1"}}));
        CellRange a1;
        a1.valid = true;
        a1.row1 = a1.row2 = 0;
        a1.col1 = a1.col2 = 0;
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::move(0, a1, 0, 0, 1), &rec));   // A1 onto B1
        QCOMPARE(b.formula("C1"), QStringLiteral("#REF!+10"));
        QCOMPARE(b.at("C1"), QStringLiteral("e:#REF!"));
        QCOMPARE(b.formula("D1"), QStringLiteral("SUM(B1:B3)"));   // (a range only partly covered)
        QCOMPARE(b.formula("D2"), QStringLiteral("B1+1"));         // (the cell that moved is followed)
        QCOMPARE(b.at("D2"), QStringLiteral("n:2"));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        QCOMPARE(b.formula("C1"), QStringLiteral("B1+10"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:12"));
        QCOMPARE(b.formula("D2"), QStringLiteral("A1+1"));
        // the block and the cells written over overlap: what is read in both goes with the block
        CellRange two;
        two.valid = true;
        two.row1 = two.row2 = 0;
        two.col1 = 0;
        two.col2 = 1;
        QVERIFY(b.apply(StructEdit::move(0, two, 0, 0, 1), &rec));   // A1:B1 onto B1:C1
        QCOMPARE(b.formula("D2"), QStringLiteral("B1+1"));   // A1 is now B1
        QCOMPARE(b.at("C1"), QStringLiteral("n:2"));         // (C1 was written over by the cell that was B1)
    }

    // Part of a merged range is not cut or written over (the whole merged cell, or none of it).
    void aCutThroughMergedCellsIsRefused()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "C5=2"}}, QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A1:B1\"/></mergeCells>")));
        CellRange block;
        block.valid = true;
        block.row1 = block.row2 = 0;
        block.col1 = block.col2 = 0;   // A1, the first cell of A1:B1
        QString why;
        QVERIFY(!b.calc->canApply(StructEdit::move(0, block, 0, 6, 6), &why));
        QVERIFY2(why.contains(QLatin1String("merged")), qPrintable(why));
        block.col2 = 1;   // the whole merged cell
        QVERIFY(b.calc->canApply(StructEdit::move(0, block, 0, 6, 6), &why));
        // written over only in part
        CellRange c5;
        c5.valid = true;
        c5.row1 = c5.row2 = 4;
        c5.col1 = c5.col2 = 2;
        QVERIFY(!b.calc->canApply(StructEdit::move(0, c5, 0, 0, 1), &why));   // C5 onto B1
        QVERIFY2(why.contains(QLatin1String("merged")), qPrintable(why));
        QVERIFY(b.calc->canApply(StructEdit::move(0, c5, 0, 8, 8), &why));
    }

    // A reference through several sheets (Sheet1:Sheet3!A5) names the same cell on every sheet: rows inserted or deleted on one sheet of them do not change it, as in the programs people
    // know; one to a single sheet follows the edit.
    void aReferenceThroughSeveralSheetsIsLeftWhenOneOfThemIsEdited()
    {
        const QStringList names = {QStringLiteral("Sheet1"), QStringLiteral("Sheet2"), QStringLiteral("Sheet3")};
        bool changed = true;
        QCOMPARE(adjustFormula(QStringLiteral("SUM(Sheet1:Sheet3!A5)"), 0, names, StructEdit::insertRows(1, 2, 3), &changed), QStringLiteral("SUM(Sheet1:Sheet3!A5)"));
        QVERIFY(!changed);
        QCOMPARE(adjustFormula(QStringLiteral("SUM(Sheet1:Sheet3!A5)"), 0, names, StructEdit::deleteRows(0, 4, 1)), QStringLiteral("SUM(Sheet1:Sheet3!A5)"));
        QCOMPARE(adjustFormula(QStringLiteral("SUM(Sheet2!A5)+Sheet1:Sheet3!A5"), 0, names, StructEdit::insertRows(1, 2, 3)), QStringLiteral("SUM(Sheet2!A8)+Sheet1:Sheet3!A5"));
        // the same through the cells of a workbook
        Book b;
        QVERIFY(b.open({{"A1==SUM(Sheet1:Sheet3!A5)", "A5=1"}, {"A5=10"}, {"A5=100"}}));
        QCOMPARE(b.at("A1"), QStringLiteral("n:111"));
        QVERIFY(b.apply(StructEdit::insertRows(1, 2, 3), nullptr));
        QCOMPARE(b.formula("A1"), QStringLiteral("SUM(Sheet1:Sheet3!A5)"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:101"));   // (Sheet2's A5 is the cell that moved down)
    }

    // A table with a row of totals: the totals row stays the last row, and deleting its sheet row takes the totals away (the table keeps its data); a table is not left with no data row.
    void aTablesTotalsRowStaysLastAndGoesWhenItsRowIsDeleted()
    {
        Table t;
        t.sheet = 0;
        t.row1 = 2;
        t.col1 = 1;
        t.row2 = 7;
        t.col2 = 3;   // B3:D8, header on row 3, data on rows 4 to 7, totals on row 8
        t.headerRows = 1;
        t.totalsRows = 1;
        auto fate = [&](const StructEdit &e, QString *place = nullptr, int *totals = nullptr) {
            CellRange where;
            QString why;
            int tr = -1;
            const TableFate f = tableAfter(e, t, &where, &why, &tr);
            if (place) *place = f == TableFate::Refused ? why : rangeName(where);
            if (totals) *totals = tr;
            return f;
        };
        QString place;
        int totals = -1;
        QCOMPARE(fate(StructEdit::insertRows(0, 7, 3), &place, &totals), TableFate::Moved);   // at the totals row: the new rows go above it
        QCOMPARE(place, QStringLiteral("B3:D11"));
        QCOMPARE(totals, 1);
        QCOMPARE(fate(StructEdit::insertRows(0, 8, 3)), TableFate::Same);                       // below the totals
        QCOMPARE(fate(StructEdit::deleteRows(0, 7, 1), &place, &totals), TableFate::Moved);     // the totals row: the table keeps its data
        QCOMPARE(place, QStringLiteral("B3:D7"));
        QCOMPARE(totals, 0);
        QCOMPARE(fate(StructEdit::deleteRows(0, 6, 2), &place, &totals), TableFate::Moved);     // the last data row and the totals
        QCOMPARE(place, QStringLiteral("B3:D6"));
        QCOMPARE(totals, 0);
        QCOMPARE(fate(StructEdit::deleteRows(0, 6, 1), &place, &totals), TableFate::Moved);     // the last data row: the totals row is still last
        QCOMPARE(place, QStringLiteral("B3:D7"));
        QCOMPARE(totals, 1);
        QCOMPARE(fate(StructEdit::deleteRows(0, 0, 1), &place, &totals), TableFate::Moved);     // above it
        QCOMPARE(place, QStringLiteral("B2:D7"));
        QCOMPARE(totals, 1);
        QCOMPARE(fate(StructEdit::deleteRows(0, 3, 4), &place), TableFate::Refused);            // all the data rows: the header and the totals would be all that is left
        QVERIFY2(place.contains(QStringLiteral("data")), qPrintable(place));
        QCOMPARE(fate(StructEdit::deleteRows(0, 3, 5)), TableFate::Refused);
        t.totalsRows = 0;   // without totals: the same rule for the data rows
        t.row2 = 6;
        QCOMPARE(fate(StructEdit::deleteRows(0, 3, 4)), TableFate::Refused);
        QCOMPARE(fate(StructEdit::deleteRows(0, 4, 3), &place), TableFate::Moved);              // one data row is left
        QCOMPARE(place, QStringLiteral("B3:D4"));
    }

    // The formulas of a shared family stay a family through edits (one text and one program for the whole family, however many rows move, and what an edit splits by the references it
    // changes is a few texts), and what edits make and Undo makes again does not pile up: twenty rounds of an edit and its Undo hold what the first did.
    void aSharedFamilyStaysSharedAndEditsDoNotPileUpTextsAndFormulas()
    {
        const int n = 300;
        QString rows;
        for (int r = 1; r <= n; ++r) {
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c>").arg(r);
            if (r == 1) rows += QStringLiteral("<c r=\"B1\"><f t=\"shared\" ref=\"B1:B%1\" si=\"0\">A1*2</f><v>2</v></c>").arg(n);
            else rows += QStringLiteral("<c r=\"B%1\"><f t=\"shared\" si=\"0\"/><v>%2</v></c>").arg(r).arg(r * 2);
            rows += QStringLiteral("<c r=\"C%1\"><f>SUM(A1:A%1)</f><v>0</v></c></row>").arg(r);
        }
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QCOMPARE(b.at("B300"), QStringLiteral("n:600"));
        const qint64 texts0 = b.calc->formulaTextsInUse(0), entries0 = b.calc->formulaEntriesInUse(0);
        QCOMPARE(entries0, qint64(2 * n));
        // rows inserted above all of it: the family is one text still (the old one is let go of), and its cells read the cells that moved
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 3), &rec));
        QCOMPARE(b.formula("B4"), QStringLiteral("A4*2"));
        QCOMPARE(b.formula("B303"), QStringLiteral("A303*2"));
        QCOMPARE(b.at("B303"), QStringLiteral("n:600"));
        QCOMPARE(b.calc->formulaEntriesInUse(0), entries0);
        QCOMPARE(rec.formulasRewritten, qint64(2 * n));
        QVERIFY2(b.calc->formulaTextsInUse(0) <= texts0 + 1 + n, qPrintable(QString::number(b.calc->formulaTextsInUse(0))));   // (the sums, each its own text, are written one by one; the family is one)
        const qint64 familyPrograms = b.calc->programsHeld(0);
        QVERIFY2(familyPrograms <= 1 + n + 1, qPrintable(QString::number(familyPrograms)));
        // a row inserted in the middle of the family: the cells above it are as they were, those below read cells that moved; two texts
        QVERIFY(b.apply(StructEdit::insertRows(0, 150, 1), nullptr));
        QCOMPARE(b.formula("B100"), QStringLiteral("A100*2"));
        QCOMPARE(b.formula("B200"), QStringLiteral("A200*2"));
        QCOMPARE(b.at("B200"), QStringLiteral("n:392"));   // (it is the cell that was in row 196, and reads that row: 196 * 2)
        // rows deleted from the middle: the cells that read them are #REF!, the others follow their cells
        QVERIFY(b.apply(StructEdit::deleteRows(0, 20, 5), nullptr));
        QCOMPARE(b.formula("B30"), QStringLiteral("A30*2"));
        // twenty rounds of an edit and its Undo, at the top and in the middle: what is held does not grow
        // (programs are made when a formula is worked out, and a formula that only moved is not: how many are held is between none and one for each text, and the texts and formulas are what is counted)
        auto held = [&] { return QStringLiteral("%1/%2").arg(b.calc->formulaTextsInUse(0)).arg(b.calc->formulaEntriesInUse(0)); };
        Calculation::EditRecord one;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &one));
        QVERIFY(b.calc->undoEdit(one));
        b.finish();
        const QString afterFirst = held();
        for (int round = 0; round < 20; ++round) {
            Calculation::EditRecord top, middle, del;
            QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1 + round % 3), &top));
            QVERIFY(b.apply(StructEdit::insertRows(0, 100, 2), &middle));
            QVERIFY(b.apply(StructEdit::deleteRows(0, 40, 3), &del));
            QVERIFY(b.calc->undoEdit(del));
            QVERIFY(b.calc->undoEdit(middle));
            QVERIFY(b.calc->undoEdit(top));
            b.finish();
        }
        QCOMPARE(held(), afterFirst);
        QVERIFY(b.calc->programsHeld(0) <= b.calc->formulaTextsInUse(0) * 2);
        QCOMPARE(b.formula("B30"), QStringLiteral("A30*2"));
        QCOMPARE(b.at("B30"), QStringLiteral("n:64"));   // (the cell that was in row 32: three rows came in above it and five went)
    }

    // What is found of a formula's text is kept by the text's place, and a place is used again for another text when the first is let go of: a formula that was a SUBTOTAL and is written
    // over with a SUM is a SUM to the SUBTOTAL that reads it (it was counted as a SUBTOTAL, and left out, by the place's old text).
    void aPlaceUsedAgainForAnotherTextForgetsWhatWasFoundOfTheFirst()
    {
        Book b;
        QVERIFY(b.open({{"B1=1", "B2=2", "A2=5", "C1==SUBTOTAL(9,A1:A3)"}}));
        QVERIFY(b.calc->setFormula(0, 0, 0, QStringLiteral("SUBTOTAL(9,B1:B2)")));
        b.finish();
        QCOMPARE(b.at("C1"), QStringLiteral("n:5"));   // (A1 is a SUBTOTAL: left out of the other's)
        QVERIFY(b.calc->setFormula(0, 0, 0, QStringLiteral("SUM(B1:B2)")));   // the place of the first text is for this one
        b.finish();
        QCOMPARE(b.at("A1"), QStringLiteral("n:3"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:8"));   // (and A1 is a SUM now: counted)
    }

    // Undo of an edit that wrote thousands of formulas again, each with a text of its own, puts what waits to be worked out right once, not at each formula: the work grows with the
    // formulas, not with the formulas and what has piled up (it was the square of them: forty-five seconds for a hundred thousand).
    void undoOfManyFormulasPutsWhatWaitsRightOnce()
    {
        const int n = 4000;
        QString rows;
        for (int r = 1; r <= n; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2+%1</f><v>0</v></c></row>").arg(r);
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &rec));
        QCOMPARE(rec.formulasRewritten, qint64(n));
        const qint64 before = b.calc->abandonWork();
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        const qint64 work = b.calc->abandonWork() - before;
        QVERIFY2(work < qint64(n) * 40, qPrintable(QStringLiteral("%1 entries put right to undo %2 formulas").arg(work).arg(n)));
        QCOMPARE(b.formula("B1"), QStringLiteral("A1*2+1"));
        QCOMPARE(b.at("B4000"), QStringLiteral("n:12000"));
    }

    // Formulas that are typed, written over, and cleared do not leave their texts and places behind.
    void typedFormulasDoNotPileUp()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2"}}));
        const qint64 texts = b.calc->formulaTextsInUse(0), entries = b.calc->formulaEntriesInUse(0);
        for (int i = 0; i < 200; ++i) {
            QVERIFY(b.calc->setFormula(0, 0, 2, QStringLiteral("A1+%1").arg(i % 7)));
            QVERIFY(b.calc->setFormula(0, 1, 2, QStringLiteral("A2+%1").arg(i % 5)));
            if (i % 3 == 0) QVERIFY(b.calc->setValue(0, 0, 2, Value::number(1)));
            b.finish();
        }
        QVERIFY(b.calc->setValue(0, 0, 2, Value::number(1)));
        QVERIFY(b.calc->setValue(0, 1, 2, Value::number(1)));
        b.finish();
        QVERIFY2(b.calc->formulaTextsInUse(0) <= texts + 1, qPrintable(QString::number(b.calc->formulaTextsInUse(0))));
        QVERIFY2(b.calc->formulaEntriesInUse(0) <= entries + 1, qPrintable(QString::number(b.calc->formulaEntriesInUse(0))));
        QVERIFY(b.calc->programsHeld(0) <= 2);
    }

    // Formulas over a block of several sheets, typed, written over, and cleared, are not kept in the list of them once their formula is gone (it does not wait for the next edit of rows).
    void typedThreeDFormulasDoNotPileUp()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2"}, {"A1=5"}}));
        QCOMPARE(b.calc->threeDFormulasHeld(), qint64(0));
        for (int i = 0; i < 100; ++i) {
            QVERIFY(b.calc->setFormula(0, 0, 2, QStringLiteral("SUM(Sheet1:Sheet2!A1)+%1").arg(i)));
            QVERIFY(b.calc->setFormula(0, 1, 2, QStringLiteral("SUM(Sheet1:Sheet2!A2)+%1").arg(i)));
            if (i % 2 == 0) QVERIFY(b.calc->setFormula(0, 1, 2, QStringLiteral("A2+%1").arg(i)));   // (a formula over one sheet: not one of them any more)
            b.finish();
        }
        QCOMPARE(b.calc->threeDFormulasHeld(), qint64(2));   // (the last of the first, and the last of the second, which was not written over on that round)
        QVERIFY(b.calc->setValue(0, 0, 2, Value::number(1)));
        b.finish();
        QCOMPARE(b.calc->threeDFormulasHeld(), qint64(1));
        QVERIFY(b.calc->setValue(0, 1, 2, Value::number(1)));
        b.finish();
        QCOMPARE(b.calc->threeDFormulasHeld(), qint64(0));
    }

    // A defined name with references that are not fixed keeps its offsets when rows or columns are inserted (it is read from the cell it is used in), and the fixed parts follow the cells.
    void aRelativeDefinedNameKeepsItsOffsets()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3"}}, QString(),
                       QStringLiteral("<definedName name=\"Rel\">Sheet1!A2</definedName><definedName name=\"Abs\">Sheet1!$A$2</definedName><definedName name=\"ColFixed\">Sheet1!$A2</definedName>")));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 2), nullptr));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Rel")), QStringLiteral("Sheet1!A2"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Abs")), QStringLiteral("Sheet1!$A$4"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("ColFixed")), QStringLiteral("Sheet1!$A2"));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), nullptr));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Rel")), QStringLiteral("Sheet1!A2"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Abs")), QStringLiteral("Sheet1!$B$4"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("ColFixed")), QStringLiteral("Sheet1!$B2"));
    }

    // A name's reference with one end fixed ($ on its row): that end follows the cells like a cell reference, and the other end, which is read from the cell the name is used in, keeps its offset.
    void aNameWithOneFixedEndMovesOnlyThatEnd()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4"}}, QString(),
                       QStringLiteral("<definedName name=\"First\">Sheet1!A$1:A3</definedName><definedName name=\"Second\">Sheet1!A1:A$3</definedName><definedName name=\"Both\">Sheet1!A$1:A$3</definedName>")));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), nullptr));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("First")), QStringLiteral("Sheet1!A$2:A3"));    // (the fixed end moved down with its cell; the other did not)
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Second")), QStringLiteral("Sheet1!A1:A$4"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Both")), QStringLiteral("Sheet1!A$2:A$4"));
        QVERIFY(b.apply(StructEdit::insertRows(0, 3, 1), nullptr));                                        // a row in the middle
        QCOMPARE(b.calc->definedName(0, QStringLiteral("First")), QStringLiteral("Sheet1!A$2:A3"));    // (its fixed end is above the new row)
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Second")), QStringLiteral("Sheet1!A1:A$5"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Both")), QStringLiteral("Sheet1!A$2:A$5"));
    }

    // A data table reads its input cells: deleting one is refused (the file cannot say it is gone), and a row or column inserted or deleted elsewhere moves the table's reference with the cell.
    void aDataTablesInputCellIsNotDeleted()
    {
        const QString rows = QStringLiteral("<row r=\"2\"><c r=\"B2\"><v>1</v></c><c r=\"D2\"><f t=\"dataTable\" ref=\"D2:D3\" r1=\"B2\"/><v>5</v></c></row><row r=\"3\"><c r=\"D3\"><v>6</v></c></row>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QString why;
        QVERIFY(!b.calc->canApply(StructEdit::deleteRows(0, 1, 1), &why));   // row 2 holds B2
        QVERIFY2(why.contains(QLatin1String("data table")), qPrintable(why));
        QVERIFY(!b.calc->canApply(StructEdit::deleteColumns(0, 0, 3), &why));   // columns A to C hold it
        Calculation::EditRecord none;
        QVERIFY(!b.calc->applyEdit(StructEdit::deleteColumns(0, 1, 1), &none));
        QCOMPARE(b.calc->content(0, 1, 1).value.number(), 1.0);              // (nothing changed)
        // a row above the cell, and a column beside it, are fine: the table's text follows the cell
        QVERIFY(b.calc->canApply(StructEdit::deleteRows(0, 0, 1), &why));
        QVERIFY(b.calc->canApply(StructEdit::deleteColumns(0, 0, 1), &why));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 2), &rec));
        QCOMPARE(b.calc->content(0, 3, 3).formula, QStringLiteral("TABLE(,B4)"));
    }

    // Where a table is after rows, columns, and cells are inserted and deleted around it: it moves, grows, and shrinks with its rows, and is not cut through.
    void aTableMovesGrowsShrinksOrRefusesTheEdit()
    {
        Table t;
        t.sheet = 0;
        t.row1 = 2;
        t.col1 = 1;
        t.row2 = 6;
        t.col2 = 3;   // B3:D7
        t.headerRows = 1;
        auto fate = [&](const StructEdit &e, QString *place = nullptr) {
            CellRange where;
            QString why;
            const TableFate f = tableAfter(e, t, &where, &why);
            if (place) *place = f == TableFate::Refused ? why : rangeName(where);
            return f;
        };
        QString place;
        QCOMPARE(fate(StructEdit::insertRows(0, 0, 2), &place), TableFate::Moved);
        QCOMPARE(place, QStringLiteral("B5:D9"));
        QCOMPARE(fate(StructEdit::insertRows(0, 2, 1), &place), TableFate::Moved);   // above the header: it moves
        QCOMPARE(place, QStringLiteral("B4:D8"));
        QCOMPARE(fate(StructEdit::insertRows(0, 4, 3), &place), TableFate::Moved);   // inside: it grows
        QCOMPARE(place, QStringLiteral("B3:D10"));
        QCOMPARE(fate(StructEdit::insertRows(0, 7, 3)), TableFate::Same);            // below it
        QCOMPARE(fate(StructEdit::insertRows(1, 0, 3)), TableFate::Same);            // on another sheet
        QCOMPARE(fate(StructEdit::deleteRows(0, 0, 2), &place), TableFate::Moved);
        QCOMPARE(place, QStringLiteral("B1:D5"));
        QCOMPARE(fate(StructEdit::deleteRows(0, 4, 2), &place), TableFate::Moved);   // two data rows
        QCOMPARE(place, QStringLiteral("B3:D5"));
        QCOMPARE(fate(StructEdit::deleteRows(0, 6, 3), &place), TableFate::Moved);   // the last row, and below
        QCOMPARE(place, QStringLiteral("B3:D6"));
        QCOMPARE(fate(StructEdit::deleteRows(0, 2, 1), &place), TableFate::Refused);   // the header
        QVERIFY(place.contains(QStringLiteral("names")));
        QCOMPARE(fate(StructEdit::deleteRows(0, 1, 9)), TableFate::Refused);         // all of it
        QCOMPARE(fate(StructEdit::insertColumns(0, 0, 2), &place), TableFate::Moved);
        QCOMPARE(place, QStringLiteral("D3:F7"));
        QCOMPARE(fate(StructEdit::insertColumns(0, 4, 2)), TableFate::Same);
        QCOMPARE(fate(StructEdit::insertColumns(0, 2, 1), &place), TableFate::Moved);   // inside: it has a column more
        QCOMPARE(place, QStringLiteral("B3:E7"));
        QCOMPARE(fate(StructEdit::deleteColumns(0, 3, 1), &place), TableFate::Moved);   // its last column
        QCOMPARE(place, QStringLiteral("B3:C7"));
        QCOMPARE(fate(StructEdit::deleteColumns(0, 0, 3), &place), TableFate::Moved);   // a column left of it, and the first two of its three
        QCOMPARE(place, QStringLiteral("A3:A7"));
        QCOMPARE(fate(StructEdit::deleteColumns(0, 1, 3)), TableFate::Refused);      // all of its columns
        QCOMPARE(fate(StructEdit::deleteColumns(0, 0, 1), &place), TableFate::Moved);
        QCOMPARE(place, QStringLiteral("A3:C7"));
        CellRange block;
        block.valid = true;
        block.row1 = 0;
        block.row2 = 1;
        block.col1 = 1;
        block.col2 = 3;
        QCOMPARE(fate(StructEdit::insertCellsDown(0, block), &place), TableFate::Moved);   // all its columns: it moves whole
        QCOMPARE(place, QStringLiteral("B5:D9"));
        block.col2 = 2;
        QCOMPARE(fate(StructEdit::insertCellsDown(0, block)), TableFate::Refused);         // only some of its columns
        block.row1 = 8;
        block.row2 = 9;
        QCOMPARE(fate(StructEdit::insertCellsDown(0, block)), TableFate::Same);            // below it
        block.row1 = 2;
        block.row2 = 6;
        block.col1 = 4;
        block.col2 = 5;
        QCOMPARE(fate(StructEdit::deleteCellsLeft(0, block)), TableFate::Same);            // right of it: its cells do not move
        block.col1 = 0;
        block.col2 = 0;
        QCOMPARE(fate(StructEdit::deleteCellsLeft(0, block), &place), TableFate::Moved);   // left of it, in all its rows
        QCOMPARE(place, QStringLiteral("A3:C7"));
        block.row2 = 3;
        QCOMPARE(fate(StructEdit::deleteCellsLeft(0, block)), TableFate::Refused);         // in some of its rows
    }

    // The cost is what moves and what reads it: an insert in the middle of a tall sheet moves the cells below it and writes again only the formulas that read them, and the search for
    // those looks at little (counted).
    void theCostIsWhatMovesAndWhatReadsIt()
    {
        QString rows;
        const int n = 100000;
        rows.reserve(n * 60);
        for (int r = 1; r <= n; ++r) {
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c>").arg(r);
            if (r == 1) rows += QStringLiteral("<c r=\"B1\"><f>SUM(A1:A100)</f><v>0</v></c><c r=\"C1\"><f>A99999*2</f><v>0</v></c>");
            if (r == 2) rows += QStringLiteral("<c r=\"B2\"><f>A50000*2</f><v>0</v></c>");
            rows += QStringLiteral("</row>");
        }
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        const qint64 queried = b.calc->graph().queryWork();
        // an edit at the end of the sheet moves nothing and refers to nothing: it looks at next to nothing, though the sheet has a hundred thousand formulas and cells
        {
            const qint64 work = b.calc->editWork();
            Calculation::EditRecord end;
            QVERIFY(b.apply(StructEdit::insertRows(0, 100500, 1), &end));
            QCOMPARE(end.cellsMoved, qint64(0));
            QCOMPARE(end.formulasRewritten, qint64(0));
            QVERIFY2(b.calc->editWork() - work < 100, qPrintable(QString::number(b.calc->editWork() - work)));
            QVERIFY(b.calc->graph().queryWork() - queried < 2000);
            QVERIFY(b.apply(StructEdit::insertRows(0, 100500, 1).inverse(), &end));
        }
        const qint64 workBefore = b.calc->editWork();
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 50000, 1), &rec));               // a row at 50,001
        QVERIFY2(b.calc->editWork() - workBefore < 100, qPrintable(QString::number(b.calc->editWork() - workBefore)));   // (the one formula: no cell moves)
        QCOMPARE(rec.cellsMoved, qint64(0));
        QCOMPARE(rec.formulasRewritten, qint64(1));                                 // (C1 reads A99999: B1 reads rows 1 to 100, and B2 reads A50000, which is above)
        QVERIFY2(b.calc->graph().queryWork() - queried < 5000, qPrintable(QString::number(b.calc->graph().queryWork() - queried)));
        QCOMPARE(b.formula("C1"), QStringLiteral("A100000*2"));
        QCOMPARE(b.formula("B2"), QStringLiteral("A50000*2"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:199998"));
        // an insert at the top moves everything, and rewrites the three formulas
        Calculation::EditRecord top;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &top));
        QCOMPARE(top.cellsMoved, qint64(0));                                        // (no cell moves, though every cell has a new place)
        QCOMPARE(top.formulasRewritten, qint64(3));
        QCOMPARE(b.formula("B2"), QStringLiteral("SUM(A2:A101)"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:%1").arg(5050));
    }

    // What the edits do is what a calculation made again from the cells would give: after each of many random edits (rows, columns, and cells, inserted and deleted, on two sheets)
    // every value is what a whole new calculation of the same cells gives (the graph has not gone stale), and Undo of them all gives back the cells and formulas that were.
    void randomEditsKeepTheGraphRightAndUndoesBack() { runRandomEdits(qEnvironmentVariableIsSet("STRUCT_SEEDS") ? quint32(qEnvironmentVariableIntValue("STRUCT_SEEDS")) : 24, false, true); }

    // The same, with formulas that read through two sheets, by position, and everything in a column, and with numbers changed after each edit: what is changed reaches what reads it, so
    // the graph is right after the edit (not only the values), whether the graph was told at once or by each formula.
    void randomEditsAndValuesChangedAfterThemKeepTheGraphRight() { runRandomEdits(qEnvironmentVariableIsSet("STRUCT_SEEDS") ? quint32(qEnvironmentVariableIntValue("STRUCT_SEEDS")) : 80, true, false); }

    void randomEditsAndValuesChangedAfterThemKeepTheGraphRightWithEachFormulaTellingIt()
    {
        // (the setting is put back whatever happens: a failed check returns from the test at once, and the tests after it would run without the graph shifts)
        struct GraphShifts {
            GraphShifts() { Calculation::setGraphShifts(false); }
            ~GraphShifts() { Calculation::setGraphShifts(true); }
        } shifts;
        runRandomEdits(qEnvironmentVariableIsSet("STRUCT_SEEDS") ? quint32(qEnvironmentVariableIntValue("STRUCT_SEEDS")) : 40, true, false);
    }

    // ---- the graph by id, and what the worker is asked after an edit of whole rows or columns (docs/jeffcrunch-rowids.md, 4.6). A formula is worked out again only if the edit could change its
    // value: counted as the formulas worked out (Calculation::evaluations), never timed.

    // Formulas that can see where cells are (ROW and COLUMN, with an argument or without, at any depth) are worked out again when a place they saw changes; a formula that only reads
    // cells (A1+1, a sum) is not, whatever the edit moved.
    void formulasThatSeeWhereCellsAreAreWorkedOutAgainWhenTheirPlacesChangeAndOthersAreNot()
    {
        Book b;
        QStringList cells;
        for (int r = 1; r <= 10; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r);
        cells << "C1==ROW(B5)" << "C2==COLUMN()" << "C3==SUMPRODUCT(ROW(A1:A10))" << "C4==A1+1" << "C5==ROW()" << "C6==SUM(A1:A10)" << "C7==IF(A1>0,ROW(A3),0)";
        QVERIFY(b.open({cells}));
        QCOMPARE(b.at("C1"), QStringLiteral("n:5"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:3"));
        QCOMPARE(b.at("C3"), QStringLiteral("n:55"));
        QCOMPARE(b.at("C4"), QStringLiteral("n:2"));
        QCOMPARE(b.at("C5"), QStringLiteral("n:5"));
        QCOMPARE(b.at("C7"), QStringLiteral("n:3"));
        // a row put in at the top: everything below moves, and the five that see a place are worked out again (the sum and A1+1 read the same cells and are not)
        qint64 before = b.calc->evaluations();
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), nullptr));
        QCOMPARE(b.calc->evaluations() - before, qint64(5));
        QCOMPARE(b.at("C2"), QStringLiteral("n:6"));    // (ROW(B6))
        QCOMPARE(b.at("C3"), QStringLiteral("n:3"));
        QCOMPARE(b.at("C4"), QStringLiteral("n:65"));   // (SUMPRODUCT(ROW(A2:A11)))
        QCOMPARE(b.at("C5"), QStringLiteral("n:2"));    // (A2+1)
        QCOMPARE(b.at("C6"), QStringLiteral("n:6"));    // (ROW() in its new row)
        QCOMPARE(b.at("C7"), QStringLiteral("n:55"));
        QCOMPARE(b.at("C8"), QStringLiteral("n:4"));
        // a row put in past everything moves nothing that is read or seen: nothing is asked
        before = b.calc->evaluations();
        const quint32 epoch = b.calc->structEpoch(0);
        QVERIFY(b.apply(StructEdit::insertRows(0, 200, 1), nullptr));
        QCOMPARE(b.calc->evaluations() - before, qint64(0));
        QCOMPARE(b.calc->structEpoch(0), epoch);
        // a column put in at the left: the same five see a place that moved (their column), and the others do not
        before = b.calc->evaluations();
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), nullptr));
        QCOMPARE(b.calc->evaluations() - before, qint64(5));
        QCOMPARE(b.at("D3"), QStringLiteral("n:4"));    // (COLUMN())
        QCOMPARE(b.at("D5"), QStringLiteral("n:2"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // ---- review of round 4

    // A function the engine lacks may read the text of a formula or the place of a cell (FORMULATEXT, ISFORMULA): a formula that uses one is not kept as it was when the formula it reads is written
    // again or moves, but has #NAME? as before; one whose inputs are untouched keeps its result.
    void anUnknownFunctionThatReadsTheTextOrPlaceOfACellIsNotKeptWhenThatChanges()
    {
        const QStringList cells{"A10=5", "C5==A10", "D1==FORMULATEXT(C5)", "D2==ISFORMULA(C5)"};
        QString why;
        {
            Book b;
            QVERIFY(b.open({cells}));
            QCOMPARE(b.at("D1"), QStringLiteral("n:0"));
            QVERIFY(b.apply(StructEdit::insertRows(0, 2, 1), nullptr));   // C5 moves to C6, and the formulas that read it do not
            QCOMPARE(b.at("D1"), QStringLiteral("e:#NAME?"));
            QCOMPARE(b.at("D2"), QStringLiteral("e:#NAME?"));
        }
        {
            Book b;
            QVERIFY(b.open({cells}));
            QVERIFY(b.apply(StructEdit::insertRows(0, 7, 1), nullptr));   // C5 stays and its text is A11 now
            QCOMPARE(b.formula("C5"), QStringLiteral("A11"));
            QCOMPARE(b.at("D1"), QStringLiteral("e:#NAME?"));
            QCOMPARE(b.at("D2"), QStringLiteral("e:#NAME?"));
        }
        {
            Book b;
            QVERIFY(b.open({cells}));
            QVERIFY(b.apply(StructEdit::insertRows(0, 40, 1), nullptr));   // past everything: nothing it reads changed
            QCOMPARE(b.at("D1"), QStringLiteral("n:0"));
            QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), nullptr));   // the cell moves right
            QCOMPARE(b.at("E1"), QStringLiteral("e:#NAME?"));
        }
    }

    // Lines an insert pushes off the end of the sheet are asked of the formulas that can notice an empty line and read a block that reaches them, though no cell was in them: the block is cut, and
    // COUNTBLANK and ROWS have another value. Rows and columns.
    void linesPushedOffTheEndOfTheSheetAreAskedOfTheFormulasThatCanNoticeThem()
    {
        QString why;
        {
            Book b;
            QVERIFY(b.open({{"C1==ROWS(A1048570:A1048576)", "C2==COUNTBLANK(A1048570:A1048576)", "C3==SUM(A:A)", "C4==COUNTBLANK(A1048560:A1048576)"}}));
            const qint64 before = b.calc->evaluations();
            const QString wasC4 = b.at("C4");
            QVERIFY(b.apply(StructEdit::insertRows(0, 0, 3), nullptr));
            QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
            QVERIFY(b.at("C4") != wasC4);   // (the block is cut at the end by the rows pushed off, and has another number of empty cells)
            QVERIFY(b.calc->evaluations() - before >= 3);
        }
        {
            Book b;
            QVERIFY(b.open({{"A1==COLUMNS(XFA1:XFD1)", "A2==COUNTBLANK(XFA2:XFD2)", "A3==SUM(1:1)"}}));
            const qint64 before = b.calc->evaluations();
            QVERIFY(b.apply(StructEdit::insertColumns(0, 3, 2), nullptr));
            QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
            QVERIFY(b.calc->evaluations() - before >= 2);
        }
    }

    // What is waiting for the worker when the lines of a sheet are renumbered (a change queued for later, a question) is answered first, with the ids it has: the formula on another sheet that read
    // the cell has its value.
    void aRebaseWithAChangeQueuedAndAQuestionWaitingLeavesTheFormulasOnOtherSheetsRight()
    {
        Book b;
        QVERIFY(b.open({{"A5=1", "C1==COUNTBLANK(A1:A9)"}, {"A1==Sheet1!A5*2", "B1==Sheet1!C1"}}));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 3), nullptr));          // A5 is A8 now, with an id that is not its place
        QVERIFY(b.calc->setValue(0, 7, 0, Value::number(21), Calculation::Marking::Later));   // queued for the worker, which has not got to it
        Calculation::EditRecord rec;
        QVERIFY(b.calc->applyEdit(StructEdit::insertRows(0, 5, 1), &rec));  // a question is waiting too (COUNTBLANK holds the gap)
        std::vector<jp::calc::LineMapCore::Renumber> table;
        QVERIFY(b.calc->compactLines(0, true, &table));
        QVERIFY(!table.empty());
        b.finish();
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:42"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // A formula kept apart after it was worked out (a circle) has graph entries, and a rebase tells them again with the others.
    void aCircleKeptApartAfterItWasWorkedOutKeepsItsEntriesThroughARebase()
    {
        Book b;
        QVERIFY(b.open({{"A1==B1+1", "B1==A1+1", "D1=3", "E1==D1*2"}}));
        QVERIFY(b.calc->keptClass(0, 0, 0) != 0);   // (kept apart after it was worked out)
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 3), nullptr));
        auto readers = [&](quint32 row, quint32 col) {
            CellArea a;
            a.sheet1 = a.sheet2 = 0;
            a.row1 = a.row2 = row;
            a.col1 = a.col2 = col;
            std::vector<quint32> out;
            b.calc->readersOf(a, &out);
            return out.size();
        };
        const size_t before = readers(3, 1);   // what reads B1 (now B4): A1's formula
        QVERIFY(before >= 1);
        std::vector<jp::calc::LineMapCore::Renumber> table;
        QVERIFY(b.calc->compactLines(0, true, &table));
        QVERIFY(!table.empty());
        b.finish();
        QCOMPARE(readers(3, 1), before);
    }

    // A 3-D formula is made again only by an edit of a sheet its blocks span.
    void aThreeDFormulaIsMadeAgainOnlyByAnEditOfASheetItSpans()
    {
        Book b;
        QVERIFY(b.open({{"A1==SUM(Sheet1:Sheet3!B2)", "B2=1"}, {"B2=2"}, {"B2=3"}, {"B2=4"}}));
        QCOMPARE(b.at("A1"), QStringLiteral("n:6"));
        qint64 remade = b.calc->editStats().nodesRemade;
        QVERIFY(b.apply(StructEdit::insertColumns(3, 0, 1), nullptr));
        QCOMPARE(b.calc->editStats().nodesRemade - remade, qint64(0));
        QVERIFY(b.apply(StructEdit::insertColumns(1, 5, 1), nullptr));
        QCOMPARE(b.calc->editStats().nodesRemade - remade, qint64(1));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // The places formulas saw are looked at only when an edit could have moved one: a row put in past every place any formula kept looks at none.
    void anEditPastEveryPlaceFormulasSawLooksAtNoneOfThem()
    {
        QStringList cells;
        for (int r = 1; r <= 200; ++r) cells << QStringLiteral("B%1==ROW()-1").arg(r);
        Book b;
        QVERIFY(b.open({cells}));
        QCOMPARE(b.calc->ownOnlyCount(), qint64(200));   // (they see their own cells and nothing else: no place is kept for them)
        QCOMPARE(b.calc->observerCount(), qint64(0));
        const qint64 looked = b.calc->observersLooked(), evaluated = b.calc->evaluations();
        QVERIFY(b.apply(StructEdit::insertRows(0, 5000, 1), nullptr));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 9, 1), nullptr));   // (columns: none of them is at or past the tenth either)
        QCOMPARE(b.calc->observersLooked(), looked);
        QCOMPARE(b.calc->evaluations(), evaluated);
        QVERIFY(b.apply(StructEdit::insertRows(0, 100, 1), nullptr));   // inside them: they are looked at, and the lower ones are worked out again
        QVERIFY(b.calc->observersLooked() > looked);
        QCOMPARE(b.calc->evaluations() - evaluated, qint64(100));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // The largest place a formula saw moves on with the lines put in before it: formulas pushed down by a hundred rows are still looked at by an edit that is past where they were.
    void aFormulaPushedDownIsStillLookedAtByALaterEditAboveIt()
    {
        Book b;
        QVERIFY(b.open({{"B1==ROW()", "B2==ROW()", "B3==ROW()"}}));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 100), nullptr));
        const qint64 evaluated = b.calc->evaluations();
        QVERIFY(b.apply(StructEdit::insertRows(0, 50, 1), nullptr));   // above them, where they were not when they were told
        QCOMPARE(b.calc->evaluations() - evaluated, qint64(3));
        QCOMPARE(b.at("B102"), QStringLiteral("n:102"));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 30), nullptr));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 10, 1), nullptr));
        QCOMPARE(b.at("AG102"), QStringLiteral("n:102"));   // (ROW() and not COLUMN(): worked out again for being in a column that moved, which is more and not less)
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // An array over cells, and what reads it, are right after rows and columns are put in and taken out around and through its neighbors, and after Undo of all of it.
    void anArrayFormulaAndItsReadersAreRightAfterEditsAroundItAndTheirUndo()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><f t=\"array\" ref=\"C2:C4\">A2:A4*2</f><v>4</v></c><c r=\"E1\"><f>SUM(C2:C4)</f><v>18</v></c></row>"
                                            "<row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"C3\"><v>6</v></c></row><row r=\"4\"><c r=\"A4\"><v>4</v></c><c r=\"C4\"><v>8</v></c></row>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QCOMPARE(b.at("E1"), QStringLiteral("n:18"));
        QString why;
        Calculation::EditRecord one, two, three, four;
        Calculation::EditRecord refused;
        QVERIFY(!b.apply(StructEdit::deleteRows(0, 1, 3), &refused));   // (the rows it is in: an array is not cut or deleted, and nothing changes)
        QCOMPARE(b.at("E1"), QStringLiteral("n:18"));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &one));        // above it
        QCOMPARE(b.at("C3"), QStringLiteral("n:4"));
        QCOMPARE(b.at("C5"), QStringLiteral("n:8"));
        QCOMPARE(b.at("E2"), QStringLiteral("n:18"));
        QVERIFY(b.apply(StructEdit::deleteRows(0, 8, 2), &two));        // below it
        QVERIFY(b.apply(StructEdit::insertColumns(0, 1, 1), &three));   // between A and the array
        QCOMPARE(b.at("D5"), QStringLiteral("n:8"));
        QCOMPARE(b.at("F2"), QStringLiteral("n:18"));
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        QVERIFY(b.calc->setValue(0, 2, 0, Value::number(10)));          // A3, which the first element reads
        b.finish();
        QCOMPARE(b.at("D3"), QStringLiteral("n:20"));
        QCOMPARE(b.at("F2"), QStringLiteral("n:34"));
        QVERIFY(b.calc->setValue(0, 2, 0, Value::number(2)));
        QVERIFY(b.calc->setValue(0, 4, 0, Value::number(7)));           // A5, which the last element reads
        b.finish();
        QCOMPARE(b.at("D5"), QStringLiteral("n:14"));
        QCOMPARE(b.at("F2"), QStringLiteral("n:24"));
        QVERIFY(b.calc->setValue(0, 4, 0, Value::number(4)));
        b.finish();
        QVERIFY(b.apply(StructEdit::deleteColumns(0, 5, 1), &four));    // an empty column
        QVERIFY(b.calc->undoEdit(four));
        QVERIFY(b.calc->undoEdit(three));
        QVERIFY(b.calc->undoEdit(two));
        QVERIFY(b.calc->undoEdit(one));
        b.finish();
        QCOMPARE(b.at("C2"), QStringLiteral("n:4"));
        QCOMPARE(b.at("C4"), QStringLiteral("n:8"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:18"));
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // A rule of validation and a rule of conditional formatting that read cells that moved give the answers for the cells they are for: after a row and a column are put in and taken out, and after Undo.
    void rulesOfValidationAndFormattingGiveTheRightAnswersAfterEditsAndUndo()
    {
        QString rows;
        for (int r = 1; r <= 12; ++r)
            rows += r == 5 ? QStringLiteral("<row r=\"5\"><c r=\"A5\" t=\"inlineStr\"><is><t>x</t></is></c></row>") : QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c>%2</row>").arg(r).arg(r == 8 ? QStringLiteral("<c r=\"D8\"><v>5</v></c>") : QString());
        const QString after = QStringLiteral("<conditionalFormatting sqref=\"B3:B10\"><cfRule type=\"expression\" dxfId=\"0\" priority=\"1\"><formula>$A3&gt;$D$8</formula></cfRule></conditionalFormatting>"
                                             "<dataValidations count=\"1\"><dataValidation type=\"custom\" allowBlank=\"1\" showErrorMessage=\"1\" sqref=\"C2:C10\"><formula1>ISNUMBER($A2)</formula1></dataValidation></dataValidations>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows, after)));
        // the answers, for every cell of the ranges, against the cells the rules read (found by what they hold)
        auto check = [&](const QString &step) {
            const CondRule &cf = b.calc->sheet(0).conditionalFormats.first();
            const CellRange first = cf.ranges.first();
            quint32 dRow = 0, dCol = 0;
            b.calc->sheet(0).cells.forEach([&](quint32 r, quint32 c, const Cell &cell) {
                if (cell.type == CellType::Number && cell.number == 5 && !cell.hasFormula() && c > 0) {
                    dRow = r;
                    dCol = c;
                }
            });
            for (quint32 row = first.row1; row <= first.row2; ++row) {
                ParseOptions po;
                const ParseResult p = parseFormula(cf.formula1, po);
                QVERIFY2(p.ok(), qPrintable(step));
                jp::calc::EvalContext ctx;
                ctx.source = b.calc.get();
                ctx.sheet = 0;
                ctx.row = row;
                ctx.col = first.col1;
                ctx.shiftRows = int(row) - int(first.row1);
                ctx.shiftCols = 0;
                ctx.now = 45366.5;
                jp::calc::Evaluator ev;
                const Value got = ev.evaluate(*p.program, ctx);
                const Value a = b.calc->cell(0, row, 0);
                const Value d = b.calc->cell(0, dRow, dCol);
                const bool expected = a.isText() || (a.isNumber() && d.isNumber() && a.number() > d.number());   // (a text is more than a number)
                QVERIFY2(got.isBool() && got.boolean() == expected, qPrintable(QStringLiteral("%1: row %2 gives %3").arg(step).arg(row + 1).arg(got.debug())));
            }
            const Validation &dv = b.calc->sheet(0).validations.first();
            const CellRange range = dv.ranges.first();
            for (quint32 row = range.row1; row <= range.row2; ++row) {
                const Validation *rule = b.calc->validationAt(0, row, range.col1);
                QVERIFY2(rule, qPrintable(step));
                const bool numeric = b.calc->cell(0, row, 0).isNumber();
                QVERIFY2(b.calc->entryAllowed(0, row, range.col1, *rule, Value::number(1), QStringLiteral("1")) == numeric, qPrintable(QStringLiteral("%1: validation at row %2").arg(step).arg(row + 1)));
            }
        };
        check(QStringLiteral("opened"));
        Calculation::EditRecord one, two, three, four;
        QVERIFY(b.apply(StructEdit::insertRows(0, 4, 2), &one));
        check(QStringLiteral("two rows in"));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 1, 1), &two));
        check(QStringLiteral("a column in"));
        QVERIFY(b.apply(StructEdit::deleteRows(0, 8, 1), &three));
        check(QStringLiteral("a row out"));
        QVERIFY(b.apply(StructEdit::deleteColumns(0, 2, 1), &four));   // the column the validation is for: its range is gone
        QVERIFY(b.calc->undoEdit(four));
        QVERIFY(b.calc->undoEdit(three));
        check(QStringLiteral("two undone"));
        QVERIFY(b.calc->undoEdit(two));
        QVERIFY(b.calc->undoEdit(one));
        check(QStringLiteral("all undone"));
    }

    // A block taken for one value of it by the cell the formula is in (a row of another sheet, picked by the formula's column) picks another when the cell moves and the block does not: the formula is
    // worked out again, though it reads cells that did not move. (Found by working a workbook out again after edits.)
    void aBlockTakenForOneValueByTheCellItIsInIsWorkedOutAgainWhenOnlyTheCellMoves()
    {
        Book b;
        QVERIFY(b.open({{"A3==Sheet2!A3:C3", "B1==A3"}, {"A3=11", "B3=22", "C3=33"}}));
        QCOMPARE(b.at("A3"), QStringLiteral("n:11"));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 2), nullptr));   // on the first sheet: the formula is in column C now, and the block is where it was
        QCOMPARE(b.at("C3"), QStringLiteral("n:33"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:33"));
        QVERIFY(b.apply(StructEdit::deleteColumns(0, 0, 1), nullptr));
        QCOMPARE(b.at("B3"), QStringLiteral("n:22"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        // the same on the sheet itself: the block and the cell move together, and the formula is worked out again for no gain, and is right
        Book c;
        QVERIFY(c.open({{"A1=1", "B1=2", "C1=3", "B2==A1:C1"}}));
        QCOMPARE(c.at("B2"), QStringLiteral("n:2"));
        QVERIFY(c.apply(StructEdit::insertColumns(0, 0, 1), nullptr));
        QCOMPARE(c.at("C2"), QStringLiteral("n:2"));
        QVERIFY2(sameAsAWholeNewCalculation(c, &why), qPrintable(why));
    }

    // A column put inside a thousand row sums asks for none of them (a sum skips the empty cells it is given and does not count where they are); the same range under COUNTBLANK, and a sum of
    // a block under an operator, are asked for all. None of them is made again in the graph: the sums read the same lines.
    void aColumnInsertedInsideRowSumsAsksForNoneOfThemAndCountBlankAsksForAll()
    {
        const int n = 1000;
        Book b;
        QStringList cells;
        for (int r = 1; r <= n; ++r) {
            for (int c = 0; c < 5; ++c) cells << QStringLiteral("%1%2=%3").arg(QChar(char('A' + c))).arg(r).arg(r + c);
            cells << QStringLiteral("G%1==SUM(A%1:E%1)").arg(r) << QStringLiteral("H%1==COUNTBLANK(A%1:E%1)").arg(r) << QStringLiteral("I%1==IF(A%1>0,AVERAGE(A%1:E%1),0)").arg(r)
                  << QStringLiteral("J%1==SUM(A%1:E%1*2)").arg(r);
        }
        QVERIFY(b.open({cells}));
        QCOMPARE(b.at("G5"), QStringLiteral("n:35"));
        QCOMPARE(b.at("H5"), QStringLiteral("n:0"));
        const qint64 before = b.calc->evaluations(), remade = b.calc->editStats().nodesRemade;
        QVERIFY(b.apply(StructEdit::insertColumns(0, 2, 1), nullptr));   // a column between B and C
        QCOMPARE(b.calc->evaluations() - before, qint64(2 * n));         // (the COUNTBLANKs and the sums of an array: not the sums, not the average)
        QCOMPARE(b.calc->editStats().nodesRemade, remade);
        QCOMPARE(b.formula("H5"), QStringLiteral("SUM(A5:F5)"));
        QCOMPARE(b.at("H5"), QStringLiteral("n:35"));
        QCOMPARE(b.at("I5"), QStringLiteral("n:1"));                     // (COUNTBLANK counts the new empty cell)
        QCOMPARE(b.at("J5"), QStringLiteral("n:7"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        // and the same for a row put into a column of sums, which the sums are not asked about either
        Book c;
        QStringList column;
        for (int r = 1; r <= n; ++r) column << QStringLiteral("A%1=%2").arg(r).arg(r);
        for (int k = 0; k < n; ++k) column << QStringLiteral("C%1==SUM(A1:A%2)").arg(k + 1).arg(n) << QStringLiteral("D%1==COUNTBLANK(A1:A%2)").arg(k + 1).arg(n);
        QVERIFY(c.open({column}));
        const qint64 beforeRows = c.calc->evaluations();
        QVERIFY(c.apply(StructEdit::insertRows(0, n / 2, 1), nullptr));
        QCOMPARE(c.calc->evaluations() - beforeRows, qint64(n));
        QVERIFY2(sameAsAWholeNewCalculation(c, &why), qPrintable(why));
    }

    // A row put inside a range: the formulas whose blocks hold the gap and can notice an empty row are worked out again (COUNTBLANK, ROWS, INDEX, MATCH here), and no other: not the sum of the
    // same range, not the formulas whose ranges end above the gap or begin below it, not a single cell.
    void aRowInsertedInsideARangeAsksForTheSensitiveFormulasThatHoldItAndNoOthers()
    {
        Book b;
        QStringList cells;
        for (int r = 1; r <= 10; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r * 10);
        cells << "C1==SUM(A1:A10)" << "C2==COUNTBLANK(A1:A10)" << "C3==ROWS(A1:A10)" << "C4==INDEX(A1:A10,5)" << "C5==MATCH(50,A1:A10,0)" << "C6==SUM(A1:A4)" << "C7==COUNTBLANK(A6:A10)"
              << "C8==A7*2" << "C9==COUNTBLANK(A1:A4)";
        QVERIFY(b.open({cells}));
        const qint64 before = b.calc->evaluations();
        QVERIFY(b.apply(StructEdit::insertRows(0, 4, 1), nullptr));   // between A4 and A5
        QCOMPARE(b.calc->evaluations() - before, qint64(4));
        QCOMPARE(b.at("C1"), QStringLiteral("n:550"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:1"));                   // (COUNTBLANK counts the new row)
        QCOMPARE(b.at("C3"), QStringLiteral("n:11"));
        QCOMPARE(b.at("C6"), QStringLiteral("n:6"));                   // (MATCH: the fifth is the sixth now; the formula moved down with the row it is in)
        QCOMPARE(b.at("C7"), QStringLiteral("n:100"));                 // (SUM(A1:A4), which ends above the new row)
        QCOMPARE(b.at("C8"), QStringLiteral("n:0"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // A change in a cell of an inserted row or column reaches the formulas that read a range around it (the row has an id no interval holds: it is asked as the lines either side), and the
    // formulas whose ranges do not take it in are not changed by it.
    void aValueTypedInAnInsertedLineReachesTheFormulasThatReadARangeAroundIt()
    {
        Book b;
        QStringList cells;
        for (int r = 1; r <= 10; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("D%1=%2").arg(r).arg(r);
        cells << "C1==SUM(A1:A10)" << "C2==SUM(A1:A5)" << "C3==SUM(A6:A10)" << "C4==AVERAGE(A1:A10)" << "C5==MAX(A1:A10)" << "C6==SUM(A:A)" << "E1==SUM(D1:D10)";
        QVERIFY(b.open({cells}));
        QVERIFY(b.apply(StructEdit::insertRows(0, 5, 1), nullptr));    // between A5 and A6
        QVERIFY(b.calc->setValue(0, 5, 0, Value::number(100)));
        b.finish();
        // a formula in the inserted row is read by the sum of the column around it: when what it reads changes, the sum has its value
        QVERIFY(b.calc->setFormula(0, 5, 3, QStringLiteral("A1*2")));
        b.finish();
        QCOMPARE(b.at("E1"), QStringLiteral("n:57"));
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(7)));
        b.finish();
        QCOMPARE(b.at("E1"), QStringLiteral("n:69"));
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(1)));
        b.finish();
        QCOMPARE(b.at("C1"), QStringLiteral("n:155"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:15"));                  // (ends above the new row)
        QCOMPARE(b.at("C3"), QStringLiteral("n:40"));                  // (begins below it)
        QCOMPARE(b.at("C5"), QStringLiteral("n:100"));
        QCOMPARE(b.at("C7"), QStringLiteral("n:155"));                 // (SUM(A:A), which moved down with the row it is in)
        // at the top, the range shifts down and does not take the new row in; a value typed there changes none of them
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), nullptr));
        QVERIFY(b.calc->setValue(0, 0, 0, Value::number(1000)));
        b.finish();
        QCOMPARE(b.at("C3"), QStringLiteral("n:15"));
        QCOMPARE(b.at("C8"), QStringLiteral("n:1155"));                // (the whole column does)
        // a column put inside a row of numbers, then a number typed in it
        Book c;
        QVERIFY(c.open({{"A1=1", "B1=2", "C1=3", "D1=4", "F1==SUM(A1:D1)", "F2==MAX(A1:D1)", "F3==SUM(A1:B1)", "F4==COUNT(A1:D1)"}}));
        QVERIFY(c.apply(StructEdit::insertColumns(0, 2, 1), nullptr));   // between B and C
        QVERIFY(c.calc->setValue(0, 0, 2, Value::number(50)));
        c.finish();
        QCOMPARE(c.at("G1"), QStringLiteral("n:60"));
        QCOMPARE(c.at("G2"), QStringLiteral("n:50"));
        QCOMPARE(c.at("G3"), QStringLiteral("n:3"));
        QCOMPARE(c.at("G4"), QStringLiteral("n:5"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        QVERIFY2(sameAsAWholeNewCalculation(c, &why), qPrintable(why));
    }

    // Rows deleted: the formulas that read them from outside are worked out again (and told the graph again what they read), the formulas in the rows are not worked out, and the formulas that
    // read other cells are not.
    void aRowDeletedAsksForTheReadersOfItFromOutsideAndNotTheFormulasInIt()
    {
        Book b;
        QStringList cells;
        for (int r = 1; r <= 12; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r);
        cells << "B1==SUM(A1:A10)" << "B2==SUM(A1:A3)" << "B3==A12+1" << "B4==A5*2" << "C5==A5+1" << "D1==SUM(A6:A10)" << "D2==COUNTBLANK(A6:A12)";
        QVERIFY(b.open({cells}));
        const qint64 before = b.calc->evaluations();
        QVERIFY(b.apply(StructEdit::deleteRows(0, 4, 1), nullptr));   // row 5
        QCOMPARE(b.calc->evaluations() - before, qint64(2));          // (B1, and B4, which is #REF! now)
        QCOMPARE(b.at("B1"), QStringLiteral("n:50"));
        QCOMPARE(b.at("B4"), QStringLiteral("e:#REF!"));
        QCOMPARE(b.formula("B1"), QStringLiteral("SUM(A1:A9)"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        // what B1 reads now is told to the graph: a number it still reads reaches it, and the row it read no more does not
        QVERIFY(b.calc->setValue(0, 8, 0, Value::number(1000)));
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:1040"));
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        // and a range that loses its last rows does not read the rows that come after
        Book c;
        QVERIFY(c.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(A1:A4)"}}));
        QVERIFY(c.apply(StructEdit::deleteRows(0, 2, 2), nullptr));   // rows 3 and 4: the range is A1:A2
        QCOMPARE(c.formula("B1"), QStringLiteral("SUM(A1:A2)"));
        QCOMPARE(c.at("B1"), QStringLiteral("n:3"));
        const qint64 evaluated = c.calc->evaluations();
        QVERIFY(c.calc->setValue(0, 2, 0, Value::number(100)));         // (A3, which was A5: not read)
        c.finish();
        QCOMPARE(c.at("B1"), QStringLiteral("n:3"));
        QCOMPARE(c.calc->evaluations(), evaluated);                          // (and nothing reads it: the graph was told again what B1 reads)
        QVERIFY(c.calc->setValue(0, 1, 0, Value::number(10)));
        c.finish();
        QCOMPARE(c.at("B1"), QStringLiteral("n:11"));
    }

    // OFFSET, INDIRECT, INDEX, MATCH, a lookup, and a name that has ROW() in it are each worked out again when the edit could change them, and are what a whole new calculation gives.
    void formulasThatReadByPlaceAreWorkedOutAgainWhenAnEditCouldChangeThem()
    {
        Book b;
        QStringList cells;
        for (int r = 1; r <= 10; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r * 10) << QStringLiteral("B%1=%2").arg(r).arg(r);
        cells << "D1==INDIRECT(\"A5\")" << "D2==OFFSET(A1,4,0)" << "D3==INDEX(A1:A10,5)" << "D4==MATCH(50,A1:A10,0)" << "D5==VLOOKUP(50,A1:B10,2,FALSE)" << "D6==MyRow" << "D7==ROW(A5)+MyRow";
        QVERIFY(b.open({cells}, QString(), QStringLiteral("<definedName name=\"MyRow\">ROW(Sheet1!$A$5)</definedName>")));
        QCOMPARE(b.at("D1"), QStringLiteral("n:50"));
        QCOMPARE(b.at("D6"), QStringLiteral("n:5"));
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 1), nullptr));   // above A3
        QCOMPARE(b.at("D1"), QStringLiteral("n:40"));                  // (INDIRECT("A5") reads the fifth row, which holds what the fourth did)
        QCOMPARE(b.at("D2"), QStringLiteral("n:40"));                  // (OFFSET(A1,4,0), the fifth row)
        QCOMPARE(b.at("D4"), QStringLiteral("n:40"));                  // (INDEX(A1:A11,5); the formulas from the third row down moved down)
        QCOMPARE(b.at("D5"), QStringLiteral("n:6"));
        QCOMPARE(b.at("D6"), QStringLiteral("n:5"));
        QCOMPARE(b.at("D7"), QStringLiteral("n:6"));                   // (the name stands for ROW(Sheet1!$A$6) now)
        QCOMPARE(b.at("D8"), QStringLiteral("n:12"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        QVERIFY(b.apply(StructEdit::deleteRows(0, 0, 2), nullptr));
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), nullptr));
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // After Undo and Redo of edits of rows and columns, and after the lines are renumbered (a rebase), the graph is right: a number changed afterwards reaches what reads it, which is what a
    // whole new calculation gives.
    void theGraphIsRightAfterUndoRedoAndARebaseOfRowAndColumnEdits()
    {
        Book b;
        QStringList cells;
        for (int r = 1; r <= 8; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1=%2").arg(r).arg(r * 2);
        cells << "D1==SUM(A1:A8)" << "D2==A4*2" << "D3==COUNTBLANK(A1:A8)" << "D4==SUM(A:A)" << "D5==INDEX(A1:A8,4)" << "D6==SUM(A2:B6)" << "D7==SUMPRODUCT(A1:A8,B1:B8)" << "E1==D1+D2";
        QVERIFY(b.open({cells}));
        QString why;
        // a number in a cell that is read, changed after each step: the cell is found by what it holds (A1 held 1 and is the first number in the first column)
        int bump = 1;
        auto changeAndCheck = [&](const QString &step) {
            // (every number the sheet holds is changed in turn, and what reads it must have its new value)
            struct Number {
                quint32 row, col;
                double was;
            };
            std::vector<Number> numbers;
            b.calc->sheet(0).cells.forEach([&](quint32 r, quint32 c, const Cell &cell) {
                if (!cell.hasFormula() && cell.type == CellType::Number) numbers.push_back({r, c, cell.number});
            });
            QVERIFY2(!numbers.empty(), qPrintable(step));
            for (const Number &n : numbers) {
                QVERIFY(b.calc->setValue(0, n.row, n.col, Value::number(n.was + 100 * bump++)));
                b.finish();
                QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(step + QStringLiteral(": ") + why));
                QVERIFY(b.calc->setValue(0, n.row, n.col, Value::number(n.was)));
                b.finish();
            }
        };
        const std::vector<StructEdit> edits{StructEdit::insertRows(0, 3, 2), StructEdit::deleteRows(0, 1, 1), StructEdit::insertColumns(0, 0, 1), StructEdit::deleteRows(0, 5, 2), StructEdit::insertRows(0, 0, 1),
                                            StructEdit::deleteColumns(0, 1, 1)};
        std::vector<Calculation::EditRecord> records(edits.size());
        for (size_t i = 0; i < edits.size(); ++i) {
            QVERIFY(b.apply(edits[i], &records[i]));
            QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(QStringLiteral("edit %1: %2").arg(i).arg(why)));
            changeAndCheck(QStringLiteral("after edit %1").arg(i));
        }
        for (size_t i = edits.size(); i-- > 0;) {
            QVERIFY(b.calc->undoEdit(records[i], nullptr));
            b.finish();
            QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(QStringLiteral("undo %1: %2").arg(i).arg(why)));
            changeAndCheck(QStringLiteral("after undo %1").arg(i));
        }
        QCOMPARE(b.formula("D1"), QStringLiteral("SUM(A1:A8)"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:36"));
        for (size_t i = 0; i < edits.size(); ++i) {
            QVERIFY(b.apply(edits[i], &records[i]));
            QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(QStringLiteral("redo %1: %2").arg(i).arg(why)));
            changeAndCheck(QStringLiteral("after redo %1").arg(i));
        }
        // the lines are put back to one run: every id is renumbered, and the graph is told again what each formula reads
        std::vector<jp::calc::LineMapCore::Renumber> table;
        QVERIFY(b.calc->compactLines(0, true, &table));
        QVERIFY(!table.empty());
        b.finish();
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        changeAndCheck(QStringLiteral("after the rows were renumbered"));
        QVERIFY(b.calc->compactLines(0, false, &table));
        b.finish();
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        changeAndCheck(QStringLiteral("after the columns were renumbered"));
        // and edits go on after it
        Calculation::EditRecord after;
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 1), &after));
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
        changeAndCheck(QStringLiteral("after an edit after the renumbering"));
    }

    // After the lines of a sheet are renumbered (a rebase puts every id at its place), the graph is told again what each formula reads: a number typed afterwards reaches the formulas that read it, in
    // rows and in columns, though the ids the graph held before would not say so.
    void theGraphIsToldAgainWhenTheLinesAreRenumbered()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B7==SUM(A1:A5)", "C7==A5*2", "D7==SUM(A:A)", "A9=10", "E9==SUM(A9:D9)"}}));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 3), nullptr));       // the ids of the lines are the old places, and the lines are three places down
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 2), nullptr));    // (the same for columns)
        std::vector<jp::calc::LineMapCore::Renumber> table;
        QVERIFY(b.calc->compactLines(0, true, &table));
        QVERIFY(!table.empty());
        QVERIFY(b.calc->compactLines(0, false, &table));
        QVERIFY(!table.empty());
        b.finish();
        QCOMPARE(b.formula("D10"), QStringLiteral("SUM(C4:C8)"));
        QVERIFY(b.calc->setValue(0, 7, 2, Value::number(100)));            // C8, which held 5
        b.finish();
        QCOMPARE(b.at("D10"), QStringLiteral("n:110"));
        QCOMPARE(b.at("E10"), QStringLiteral("n:200"));
        QCOMPARE(b.at("F10"), QStringLiteral("n:120"));
        QVERIFY(b.calc->setValue(0, 11, 2, Value::number(1000)));          // C12, which held 10
        b.finish();
        QCOMPARE(b.at("G12"), QStringLiteral("n:1000"));
        QString why;
        QVERIFY2(sameAsAWholeNewCalculation(b, &why), qPrintable(why));
    }

    // The epoch of a sheet grows only for an edit that is not rigid for some formula (one that asks something of the worker), so that a result worked out before a rigid edit is still good; a
    // formula the questions mark to be worked out again is stamped with the epoch (while a value is in flight), and a result older than the stamp is let go of.
    void theEpochGrowsOnlyForAnEditThatIsNotRigidAndTheFormulasAskedAreStamped()
    {
        auto bookOf = [](Book &b) {
            QStringList cells;
            for (int r = 1; r <= 10; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r);
            cells << "B1==SUM(A1:A10)" << "B2==COUNTBLANK(A1:A10)" << "B3==A3+1" << "B4==SUM(A1:A3)";
            return b.open({cells});
        };
        {
            Book b;
            QVERIFY(bookOf(b));
            const quint32 e0 = b.calc->structEpoch(0);
            QVERIFY(b.apply(StructEdit::insertRows(0, 200, 1), nullptr));   // past everything
            QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), nullptr));     // above everything: the ranges shift and take nothing in
            QVERIFY(b.apply(StructEdit::insertColumns(0, 5, 1), nullptr));  // past the columns that are read
            QVERIFY(b.apply(StructEdit::deleteRows(0, 100, 1), nullptr));   // an empty row nothing reads
            QCOMPARE(b.calc->structEpoch(0), e0);
            QVERIFY(b.apply(StructEdit::insertRows(0, 5, 1), nullptr));     // inside A1:A10: COUNTBLANK holds the gap
            QCOMPARE(b.calc->structEpoch(0), e0 + 1);
            QVERIFY(b.apply(StructEdit::deleteRows(0, 3, 1), nullptr));     // a row the sum reads
            QCOMPARE(b.calc->structEpoch(0), e0 + 2);
        }
        {
            // a sheet with a formula that sees a place: an edit that moves the place is not rigid, one that does not is
            Book b;
            QVERIFY(b.open({{"A1=1", "B1==ROW(A5)", "B2==SUM(A1:A3)"}}));
            const quint32 e0 = b.calc->structEpoch(0);
            QVERIFY(b.apply(StructEdit::insertRows(0, 100, 1), nullptr));
            QCOMPARE(b.calc->structEpoch(0), e0);
            QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), nullptr));
            QCOMPARE(b.calc->structEpoch(0), e0 + 1);
        }
        // stamps: with a value in flight, the formulas asked are stamped and a result older than the stamp is discarded; the others are kept
        Book b;
        QVERIFY(bookOf(b));
        const quint32 before = b.calc->structEpoch(0);
        b.calc->valuesTaken(-1);   // (a value told and not taken: one is on its way)
        QVERIFY(b.calc->resultsInFlight());
        QVERIFY(b.apply(StructEdit::insertRows(0, 5, 1), nullptr));   // COUNTBLANK (B2) holds the gap; the sum (B1) does not mind
        const quint32 now = b.calc->structEpoch(0);
        QCOMPARE(now, before + 1);
        const CellStore &store = b.calc->sheet(0).cells;
        const quint32 col = store.colIdAt(1);
        QVERIFY(b.calc->staleResult(0, store.rowIdAt(1), col, before));    // B2
        QVERIFY(!b.calc->staleResult(0, store.rowIdAt(1), col, now));      // (a result worked out after the edit is good)
        QVERIFY(!b.calc->staleResult(0, store.rowIdAt(0), col, before));   // B1, the sum
        QVERIFY(!b.calc->staleResult(0, store.rowIdAt(3), col, before));   // B4... (moved by one, not asked: rigid)
        b.calc->pruneStamps({now});
        QCOMPARE(b.calc->stampCount(), qint64(0));
    }

    // The questions are counted: an edit asks the worker once, of the sets it needs, and a rigid edit asks nothing.
    void anEditOfWholeLinesAsksTheWorkerOnceAndARigidEditAsksNothing()
    {
        Book b;
        QStringList cells;
        for (int r = 1; r <= 100; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1==A%1*2").arg(r);
        cells << "D1==SUM(A:A)" << "D2==COUNT(A5:A1048576)";   // (read to the end of the sheet: the row an insert pushes off its end is not one they hold anything of)
        QVERIFY(b.open({cells}));
        const qint64 asked = b.calc->questionsAnswered(), marked = b.calc->formulasAsked();
        QVERIFY(b.apply(StructEdit::insertRows(0, 10, 1), nullptr));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 1, 1), nullptr));
        QVERIFY(b.apply(StructEdit::deleteColumns(0, 4, 1), nullptr));
        QCOMPARE(b.calc->questionsAnswered(), asked);   // (nothing here can notice the edits)
        QCOMPARE(b.calc->formulasAsked(), marked);
        QCOMPARE(b.calc->editStats().nodesRemade, qint64(0));
        // a row deleted that a formula reads from outside is one question, and the formula is the one it names
        QVERIFY(b.apply(StructEdit::deleteRows(0, 20, 1), nullptr));
        QVERIFY(b.calc->questionsAnswered() == asked);   // (B21 read A21 and went with it: no one reads a cell of the row from outside)
        Book c;
        QVERIFY(c.open({{"A1=1", "A2=2", "A3=3", "B1==SUM(A1:A3)", "B2==A2*2"}}));
        const qint64 askedC = c.calc->questionsAnswered();
        QVERIFY(c.apply(StructEdit::deleteRows(0, 1, 1), nullptr));
        QCOMPARE(c.calc->questionsAnswered(), askedC + 1);
        QCOMPARE(c.calc->formulasAsked(), qint64(1));   // (B1, which read the row; B2 was in it)
    }

private:
    // Every formula, in the order of the sheets and cells, with its value: what a whole new calculation of the same cells must leave as it is.
    static QStringList formulaValues(Book &b)
    {
        QStringList out;
        for (int sheet = 0; sheet < b.calc->sheetCount(); ++sheet)
            b.calc->sheet(sheet).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
                if (c.hasFormula()) out << QStringLiteral("%1!%2,%3 %4").arg(sheet).arg(row).arg(col).arg(b.calc->cell(sheet, row, col).debug());
            });
        return out;
    }
    static bool sameAsAWholeNewCalculation(Book &b, QString *why)
    {
        const QStringList now = formulaValues(b);
        b.calc->invalidateAll();
        b.finish();
        const QStringList again = formulaValues(b);
        if (now == again) return true;
        for (int i = 0; i < std::min(now.size(), again.size()); ++i)
            if (now[i] != again[i]) {
                *why = QStringLiteral("a formula was left stale: %1 against %2 in a whole new calculation").arg(now[i], again[i]);
                return false;
            }
        *why = QStringLiteral("%1 formulas against %2").arg(now.size()).arg(again.size());
        return false;
    }

    void runRandomEdits(quint32 seeds, bool valuesToo, bool undoThem)
    {
        const quint32 only = quint32(qEnvironmentVariableIntValue("STRUCT_SEED"));   // (one seed, and what it does)
        for (quint32 seed = only ? only : 1; seed <= (only ? only : seeds); ++seed) {
            QRandomGenerator rng(seed);
            QStringList first, second;
            for (int r = 1; r <= 12; ++r)
                for (int c = 0; c < 6; ++c) {
                    const QString ref = QString(QLatin1Char(char('A' + c))) + QString::number(r);
                    if (rng.bounded(5) == 0) continue;
                    if (c < 3 || rng.bounded(3) == 0) first << ref + QLatin1Char('=') + QString::number(rng.bounded(100));
                    else {
                        const int kind = rng.bounded(10);
                        const int r1 = 1 + int(rng.bounded(12)), r2 = 1 + int(rng.bounded(12));
                        const QString a = QString(QLatin1Char(char('A' + rng.bounded(3)))) + QString::number(std::min(r1, r2)), z = QString(QLatin1Char(char('A' + rng.bounded(3)))) + QString::number(std::max(r1, r2));
                        const QString formula = kind == 0 ? QStringLiteral("SUM(%1:%2)").arg(a, z) : kind == 1 ? QStringLiteral("%1*2+Sheet2!A1").arg(a) : kind == 2 ? QStringLiteral("MAX(%1,%2)").arg(a, z)
                                              : kind == 3 ? QStringLiteral("SUM(A:A)+%1").arg(a) : kind == 5 ? QStringLiteral("SUM(Sheet1:Sheet2!A%1)+%2").arg(1 + int(rng.bounded(6))).arg(a)
                                              : kind == 6 ? QStringLiteral("INDEX(A1:A12,%1)+%2").arg(1 + int(rng.bounded(8))).arg(a)
                                              : kind == 7 ? QStringLiteral("INDIRECT(\"A%1\")+%2").arg(1 + int(rng.bounded(8))).arg(a) : kind == 8 ? QStringLiteral("OFFSET(A1,%1,0)+%2").arg(int(rng.bounded(8))).arg(a)
                                              : QStringLiteral("Sheet2!B%1+$%2").arg(1 + int(rng.bounded(6))).arg(a);
                        first << ref + QStringLiteral("==") + formula;
                    }
                }
            for (int r = 1; r <= 6; ++r) {
                second << QStringLiteral("A%1=%2").arg(r).arg(r * 3);
                second << QStringLiteral("B%1==SUM(Sheet1!A1:A%2)+Sheet1!C%1").arg(r).arg(r + 3);
            }
            Book b;
            QVERIFY2(b.open({first, second}), qPrintable(QString::number(seed)));
            auto snapshot = [&](bool values) {
                QStringList out;
                for (int sheet = 0; sheet < 2; ++sheet)
                    b.calc->sheet(sheet).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
                        const auto content = b.calc->content(sheet, row, col);
                        out << QStringLiteral("%1!%2,%3 %4 %5").arg(sheet).arg(row).arg(col).arg(content.isFormula ? QLatin1Char('=') + content.formula : content.value.debug(), values ? b.calc->cell(sheet, row, col).debug() : QString());
                    });
                return out.join(QLatin1Char('\n'));
            };
            const QString cellsAtStart = snapshot(false);
            std::vector<Calculation::EditRecord> done;
            for (int step = 0; step < 14; ++step) {
                const quint32 at = quint32(rng.bounded(10)), count = 1 + quint32(rng.bounded(2));
                CellRange block;
                block.valid = true;
                block.row1 = at;
                block.row2 = at + count - 1;
                block.col1 = quint32(rng.bounded(4));
                block.col2 = block.col1 + quint32(rng.bounded(2));
                const int sheet = rng.bounded(4) == 0 ? 1 : 0;
                StructEdit e;
                switch (rng.bounded(8)) {
                case 0: e = StructEdit::insertRows(sheet, at, count); break;
                case 1: e = StructEdit::deleteRows(sheet, at, count); break;
                case 2: e = StructEdit::insertColumns(sheet, at % 6, count); break;
                case 3: e = StructEdit::deleteColumns(sheet, at % 6, count); break;
                case 4: e = StructEdit::insertCellsDown(sheet, block); break;
                case 5: e = StructEdit::deleteCellsUp(sheet, block); break;
                case 6: e = StructEdit::insertCellsRight(sheet, block); break;
                default: e = StructEdit::deleteCellsLeft(sheet, block); break;
                }
                Calculation::EditRecord rec;
                if (!b.calc->canApply(e, nullptr)) continue;
                if (only) qInfo() << "step" << step << "kind" << int(e.kind) << "sheet" << e.sheet << "first" << e.first << "count" << e.count << "block" << e.row1 << e.col1 << e.row2 << e.col2;
                QVERIFY(b.apply(e, &rec));
                done.push_back(rec);
                // every value is what a calculation made again from these cells gives
                const QString incremental = snapshot(true);
                b.calc->invalidateAll();
                b.finish();
                QCOMPARE(snapshot(true), incremental);
                // and what is changed afterwards reaches what reads it (the graph is right after the edit, not only the values)
                for (int k = 0; valuesToo && k < 3; ++k) {
                    const int vs = int(rng.bounded(2));
                    const quint32 vr = rng.bounded(14), vc = rng.bounded(3);
                    if (only) qInfo() << "  set" << vs << vr << vc;
                    QVERIFY(b.calc->setValue(vs, vr, vc, Value::number(rng.bounded(100))));
                    b.finish();
                    const QString afterValue = snapshot(true);
                    b.calc->invalidateAll();
                    b.finish();
                    const QString whole = snapshot(true);
                    if (whole != afterValue) {
                        const QStringList x = afterValue.split(QLatin1Char('\n')), y = whole.split(QLatin1Char('\n'));
                        QStringList differ;
                        for (int i = 0; i < std::min(x.size(), y.size()); ++i)
                            if (x[i] != y[i]) differ << x[i] + QStringLiteral("  ==>  ") + y[i];
                        QFAIL(qPrintable(QStringLiteral("seed %1 step %2: after the edit and a value changed, %3 cells differ from a whole calculation: %4").arg(seed).arg(step).arg(differ.size()).arg(differ.mid(0, 4).join(QStringLiteral(" | ")))));
                    }
                }
            }
            if (!undoThem) continue;
            for (size_t i = done.size(); i-- > 0;) {
                QVERIFY(b.calc->undoEdit(done[i]));
                b.finish();
            }
            QCOMPARE(snapshot(false), cellsAtStart);
            // and what is changed after the undoes reaches what reads it (Undo left the graph right too)
            for (int k = 0; k < 6; ++k) {
                QVERIFY(b.calc->setValue(int(rng.bounded(2)), rng.bounded(14), rng.bounded(3), Value::number(rng.bounded(100))));
                b.finish();
                const QString afterValue = snapshot(true);
                b.calc->invalidateAll();
                b.finish();
                QVERIFY2(snapshot(true) == afterValue, qPrintable(QStringLiteral("seed %1: after the undoes and a value changed, the values are not those of a whole calculation").arg(seed)));
            }
        }
    }
};

// Round 5a: formulas held in the id frame (docs/jeffcrunch-rowids.md, 4.4): the text a workbook holds names rows and columns by their ids, so an edit of whole rows or columns writes no text again
// for a formula that reads no line that dies. What a person sees (the values, the text of a formula, Find) is as it is with the formulas held in places.
class SheetIdFrameTests : public QObject {
    Q_OBJECT

    // Every formula of every sheet as a person reads it, by place, with its value, in order.
    static QStringList state(const Book &b)
    {
        QStringList out;
        b.calc->forEachFormulaText([&](int sheet, quint32 row, quint32 col, const QString &text) {
            // (what a circle gives depends on where the work began, and a cell that reads one: the text only)
            const bool tainted = b.calc->isCircular(sheet, row, col) || b.calc->isTainted(sheet, row, col);
            out << QStringLiteral("%1!%2,%3 =%4 -> %5").arg(sheet).arg(row).arg(col).arg(text, tainted ? QStringLiteral("?") : b.calc->cell(sheet, row, col).debug());
        });
        out.sort();
        return out;
    }
    static QStringList data()
    {
        return {"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==A1*2", "B2==SUM(A1:A5)", "B3==A$5+$A2", "B4==SUM(A:A)", "C1==SUM(B1:B3)"};
    }

private Q_SLOTS:
    // The parser reads rows to 2^24 and columns to CRXP when it is told to, and takes only the grid's otherwise (a word that is not a cell there is a name).
    void wideLimitsReadReferencesPastTheGrid()
    {
        jp::calc::ParseOptions wide;
        wide.limits = jp::calc::ParseLimits::ids();
        const jp::calc::ParseResult w = jp::calc::parseFormula(QStringLiteral("A2097153+CRXP1+$B$16777216+C:C+SUM(A5:A2097200)"), wide);
        QVERIFY(w.ok());
        QCOMPARE(w.program->refs.size(), size_t(5));
        QCOMPARE(w.program->refs[0].row1, quint32(2097152));
        QCOMPARE(w.program->refs[1].col1, quint32(65535));
        QCOMPARE(w.program->refs[2].row1, quint32(16777215));
        QCOMPARE(w.program->refs[4].row2, quint32(2097199));
        QCOMPARE(jp::calc::columnLetters(65535), QStringLiteral("CRXP"));
        const jp::calc::ParseResult g = jp::calc::parseFormula(QStringLiteral("A2097153+CRXP1"), jp::calc::ParseOptions());
        QVERIFY(g.ok());
        QCOMPARE(g.program->refs.size(), size_t(0));   // (two names)
        QVERIFY(nameLooksWide(QStringLiteral("ABCD1")));
        QVERIFY(!nameLooksWide(QStringLiteral("XFD1")));
        QVERIFY(!nameLooksWide(QStringLiteral("Total")));
        std::vector<jp::calc::ReferenceAt> refs;
        QVERIFY(jp::calc::findReferences(QStringLiteral("A2097153:B5+CRXP9"), wide, &refs));
        QCOMPARE(refs.size(), size_t(2));
        QCOMPARE(jp::calc::referenceCellsText(refs[1].ref), QStringLiteral("CRXP9"));
        QCOMPARE(offsetFormula(QStringLiteral("A2097153+B$1"), 2, 1, nullptr, jp::calc::ParseLimits::ids()), QStringLiteral("B2097155+C$1"));
    }

    // A row inserted at the top of a sheet writes no text again: no formula is looked at, rewritten, or made again, and the counts are the same at two sizes of the sheet. With the formulas in
    // places every formula that reads the rows below is.
    void anInsertWritesNoTextAgainAtAnySize()
    {
        qint64 visited[2] = {0, 0};
        for (const int mode : {0, 1}) {
            QList<qint64> perSize;
            for (const int rows : {1000, 8000}) {
                Book b;
                b.idFrame = mode == 1;
                b.forceOff = mode == 0;
                QStringList cells;
                for (int r = 1; r <= rows; ++r) {
                    cells << QStringLiteral("A%1=%2").arg(r).arg(r);
                    cells << QStringLiteral("B%1==A%1*2+A$1").arg(r);
                    cells << QStringLiteral("C%1==SUM(A1:A%1)").arg(r);
                }
                cells << QStringLiteral("D1==SUM(A:A)");
                QVERIFY(b.open({cells}));
                QCOMPARE(b.calc->idFrame(), mode == 1);
                const Calculation::EditStats before = b.calc->editStats();
                QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), nullptr));
                const Calculation::EditStats d = b.calc->editStats().since(before);
                perSize << d.formulasVisited;
                QCOMPARE(b.formula("B2"), QStringLiteral("A2*2+A$2"));
                QCOMPARE(b.formula("C3"), QStringLiteral("SUM(A2:A3)"));
                QCOMPARE(b.formula("D2"), QStringLiteral("SUM(A:A)"));
                if (mode == 1) {
                    QCOMPARE(d.formulasRewritten, qint64(0));
                    QCOMPARE(d.textsMade, qint64(0));
                    QCOMPARE(d.eagerFormulas, qint64(0));
                    QCOMPARE(d.nodesRemade, qint64(0));
                    QCOMPARE(b.calc->positionalFormulas(), qint64(0));
                }
            }
            if (mode == 1) QCOMPARE(perSize[1], perSize[0]);
            visited[mode] = perSize[1];
        }
        QVERIFY(visited[0] >= 8000);
        QVERIFY(visited[1] <= 1);   // (the one that reads every row, SUM(A:A), is found among the readers of the rows that fall off the end; its text is left as it is)
    }

    // A formula typed into a row that was inserted, one that reads such a row, a totals row under data that grew, and a computed column inserted and filled down: each is held by ids (none is held in
    // places), reads what it names after more edits, and is shown where it is.
    void formulasOfInsertedRowsAndColumnsAreHeldById()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) {
            QVERIFY(b->open({{"A1=1", "A2=2", "A3=3", "A4=4", "B6==SUM(A1:A4)"}}));
            QVERIFY(b->apply(StructEdit::insertRows(0, 2, 1), nullptr));                       // a row at row 3: the data has a gap
            QVERIFY(b->calc->setFormula(0, 2, 0, QStringLiteral("A2*10")));                    // typed into it (A3)
            QVERIFY(b->calc->setFormula(0, 0, 2, QStringLiteral("A3+1")));                     // reads it (C1)
            b->finish();
            QCOMPARE(b->at("A3"), QStringLiteral("n:20"));
            QCOMPARE(b->at("C1"), QStringLiteral("n:21"));
            QCOMPARE(b->at("B7"), QStringLiteral("n:30"));                                     // (the total grew with the row)
            QVERIFY(b->apply(StructEdit::insertRows(0, 0, 2), nullptr));
            QVERIFY(b->apply(StructEdit::insertColumns(0, 1, 1), nullptr));                    // a column B
            for (int r = 2; r <= 7; ++r) QVERIFY(b->calc->setFormula(0, quint32(r), 1, QStringLiteral("A%1*2").arg(r + 1)));   // a computed column, filled down
            QVERIFY(b->apply(StructEdit::insertRows(0, 4, 1), nullptr));
            b->finish();
        }
        QCOMPARE(state(on), state(off));
        QCOMPARE(on.calc->positionalFormulas(), qint64(0));
        QVERIFY(!on.calc->sheet(0).wideTexts.isEmpty());   // (the formula that reads an inserted row has an id past the grid in it)
        QVERIFY(off.calc->sheet(0).wideTexts.isEmpty());
        QCOMPARE(on.formula("D3"), QStringLiteral("A6+1"));   // (what the person reads is places)
        // Undo and Redo of the edits with ids
        for (Book *b : {&off, &on}) {
            std::vector<Calculation::EditRecord> recs(3);
            QVERIFY(b->apply(StructEdit::insertRows(0, 1, 2), &recs[0]));
            QVERIFY(b->apply(StructEdit::deleteColumns(0, 0, 1), &recs[1]));
            QVERIFY(b->apply(StructEdit::deleteRows(0, 3, 1), &recs[2]));
            QStringList afterEdits = state(*b);
            for (int i = 2; i >= 0; --i) {
                QVERIFY(b->calc->undoEdit(recs[size_t(i)]));
                b->finish();
            }
            QVERIFY(state(*b) != afterEdits);
            for (int i = 0; i < 3; ++i) {
                Calculation::EditRecord again = recs[size_t(i)];
                QVERIFY(b->calc->applyEdit(recs[size_t(i)].edit, &again));
                b->finish();
            }
            QCOMPARE(state(*b), afterEdits);
        }
        QCOMPARE(state(on), state(off));
    }

    // A formula that reads a row that is deleted is not written again: a single cell there is #REF!, and an end of a range that dies is the line next to where it was (a start the one that followed, an
    // end the one that preceded), by the order. The text a person reads is what it was when the text was written again, and Undo brings the same rows back.
    void aDeleteWritesNoTextAgainAndAnEndThatDiesIsTheLineNextToIt()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) QVERIFY(b->open({data()}));
        const QStringList before = state(on);
        QCOMPARE(before, state(off));
        Calculation::EditRecord rec[2];
        const Calculation::EditStats s0 = on.calc->editStats();
        QVERIFY(off.apply(StructEdit::deleteRows(0, 4, 1), &rec[0]));   // row 5: A$5 in B3 and the end of the range in B2
        QVERIFY(on.apply(StructEdit::deleteRows(0, 4, 1), &rec[1]));
        const Calculation::EditStats d = on.calc->editStats().since(s0);
        QCOMPARE(on.formula("B3"), QStringLiteral("#REF!+$A2"));
        QCOMPARE(on.formula("B2"), QStringLiteral("SUM(A1:A4)"));
        QCOMPARE(on.at("B3"), QStringLiteral("e:#REF!"));
        QCOMPARE(on.at("B2"), QStringLiteral("n:10"));
        QCOMPARE(state(on), state(off));
        QCOMPARE(d.eagerFormulas, qint64(0));
        QCOMPARE(d.formulasRewritten, qint64(0));
        QCOMPARE(d.textsMade, qint64(0));
        QCOMPARE(d.nodesRemade, qint64(0));
        QVERIFY(on.calc->undoEdit(rec[1]));
        QVERIFY(off.calc->undoEdit(rec[0]));
        on.finish();
        off.finish();
        QCOMPARE(state(on), before);
        QCOMPARE(state(off), before);
        // a row inside a range dies: the same ids, no text
        const Calculation::EditStats s1 = on.calc->editStats();
        QVERIFY(on.apply(StructEdit::deleteRows(0, 2, 1), &rec[1]));
        QVERIFY(off.apply(StructEdit::deleteRows(0, 2, 1), &rec[0]));
        QCOMPARE(on.formula("B1"), QStringLiteral("A1*2"));
        QCOMPARE(on.formula("B2"), QStringLiteral("SUM(A1:A4)"));
        QCOMPARE(on.calc->editStats().since(s1).eagerFormulas, qint64(0));
        QCOMPARE(state(on), state(off));
    }

    // A start that dies is the line that followed it, an end the line that preceded it; a range whose lines are all gone is #REF!; and Undo brings the same lines back and every reference reads them
    // again. A line put in next to the dead ones afterwards is outside the range (docs/jeffcrunch-rowids.md, 4.4). Checked against the formulas in places after every step.
    void aDeadStartAndEndAndARangeWhoseRowsAreAllGone()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        const QStringList data = {"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "A6=6"};
        const QStringList reads = {"A1==SUM(Sheet1!A2:A5)", "A2==Sheet1!A3+Sheet1!A$4", "A3==SUM(Sheet1!A$3:A4)", "A4==COUNT(Sheet1!A3:A3)", "A5==SUM(Sheet1!A:A)", "A6==SUM(Sheet1!$A$2:$A$3)+ROWS(Sheet1!A2:A5)"};
        for (Book *b : {&off, &on}) QVERIFY(b->open({data, reads}));
        std::vector<Calculation::EditRecord> recs[2];
        auto same = [&]() { QCOMPARE(state(on), state(off)); };
        auto text = [&](const char *cell, const QString &want) { QCOMPARE(on.formula(QString::fromLatin1(cell), 1), want); QCOMPARE(off.formula(QString::fromLatin1(cell), 1), want); };
        auto both = [&](const StructEdit &e) {
            for (Book *b : {&off, &on}) {
                recs[b == &on].emplace_back();
                QVERIFY(b->apply(e, &recs[b == &on].back()));
            }
            same();
        };
        auto undo = [&]() {
            for (Book *b : {&off, &on}) {
                QVERIFY(b->calc->undoEdit(recs[b == &on].back()));
                recs[b == &on].pop_back();
                b->finish();
            }
            same();
        };
        auto redo = [&](const StructEdit &e) {
            for (Book *b : {&off, &on}) {
                recs[b == &on].emplace_back();
                QVERIFY(b->calc->applyEdit(e, &recs[b == &on].back()));
                b->finish();
            }
            same();
        };
        const Calculation::EditStats s0 = on.calc->editStats();
        // the named sequence: delete a (the start), delete b (the start goes on to c), Undo (b is the start), Redo, Undo twice, do both again, put a row between the live neighbors of the dead ones
        both(StructEdit::deleteRows(0, 1, 1));
        text("A1", QStringLiteral("SUM(Sheet1!A2:A4)"));
        text("A6", QStringLiteral("SUM(Sheet1!$A$2)+ROWS(Sheet1!A2:A4)"));   // (A$2:A$3 lost its start and closed up to the one cell)
        both(StructEdit::deleteRows(0, 1, 1));
        text("A1", QStringLiteral("SUM(Sheet1!A2:A3)"));
        text("A6", QStringLiteral("SUM(Sheet1!#REF!)+ROWS(Sheet1!A2:A3)"));
        undo();
        text("A1", QStringLiteral("SUM(Sheet1!A2:A4)"));
        redo(StructEdit::deleteRows(0, 1, 1));
        text("A1", QStringLiteral("SUM(Sheet1!A2:A3)"));
        undo();
        undo();
        text("A1", QStringLiteral("SUM(Sheet1!A2:A5)"));
        QCOMPARE(on.at("A1", 1), QStringLiteral("n:14"));
        redo(StructEdit::deleteRows(0, 1, 1));
        redo(StructEdit::deleteRows(0, 1, 1));
        both(StructEdit::insertRows(0, 1, 1));   // (between the live neighbors of the dead rows: before the start)
        text("A1", QStringLiteral("SUM(Sheet1!A3:A4)"));
        both(StructEdit::insertRows(0, 4, 1));   // (after the end: outside as well)
        text("A1", QStringLiteral("SUM(Sheet1!A3:A4)"));
        undo();
        undo();
        undo();
        undo();
        text("A1", QStringLiteral("SUM(Sheet1!A2:A5)"));
        // every row of a range gone
        both(StructEdit::deleteRows(0, 1, 4));
        text("A1", QStringLiteral("SUM(Sheet1!#REF!)"));
        QCOMPARE(on.at("A1", 1), QStringLiteral("e:#REF!"));
        QCOMPARE(on.at("A6", 1), QStringLiteral("e:#REF!"));
        undo();
        QCOMPARE(on.at("A1", 1), QStringLiteral("n:14"));
        // a range that closes up to one cell is shown as one cell
        both(StructEdit::deleteRows(0, 3, 1));
        text("A1", QStringLiteral("SUM(Sheet1!A2:A4)"));
        both(StructEdit::deleteRows(0, 2, 1));
        text("A1", QStringLiteral("SUM(Sheet1!A2:A3)"));
        text("A4", QStringLiteral("COUNT(Sheet1!#REF!)"));   // (its one cell went with the second of those rows)
        // the work: no formula's text was written again or made again for any of it
        const Calculation::EditStats d = on.calc->editStats().since(s0);
        QCOMPARE(d.formulasRewritten, qint64(0));
        QCOMPARE(d.textsMade, qint64(0));
        QCOMPARE(d.eagerFormulas, qint64(0));
        QCOMPARE(d.nodesRemade, qint64(0));
    }

    // The order of the ids is renumbered by a rebase (compaction): the texts are written again with the new ids and every formula reads and shows what it did, and a family of cells that shared one
    // text (a shared formula read from a file) is kept where its ids are in one run and split where they are not.
    void aRebaseKeepsTheFormulas()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        QStringList cells;
        for (int r = 1; r <= 12; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1==A%1+A$1").arg(r);
        cells << QStringLiteral("C1==SUM(A:A)") << QStringLiteral("D1==Sheet2!A3+B2");
        for (Book *b : {&off, &on}) {
            QVERIFY(b->open({cells, {"A1=10", "A2=20", "A3=30", "B1==Sheet1!B5+A3"}}));
            QVERIFY(b->apply(StructEdit::insertRows(0, 3, 2), nullptr));
            QVERIFY(b->calc->setFormula(0, 3, 4, QStringLiteral("A4+A6")));
            QVERIFY(b->apply(StructEdit::deleteRows(0, 8, 1), nullptr));
            QVERIFY(b->apply(StructEdit::insertColumns(0, 1, 1), nullptr));
            QVERIFY(b->apply(StructEdit::insertRows(1, 1, 1), nullptr));
            b->finish();
        }
        const QStringList before = state(on);
        QCOMPARE(before, state(off));
        std::vector<jp::calc::LineMapCore::Renumber> table;
        QVERIFY(on.calc->compactLines(0, true, &table));
        QVERIFY(!table.empty());
        QVERIFY(on.calc->compactLines(0, false, &table));
        QVERIFY(on.calc->compactLines(1, true, &table));
        on.finish();
        QCOMPARE(state(on), before);
        QVERIFY(on.calc->sheet(0).wideTexts.isEmpty());   // (the ids are the places again, and no text has one past the grid)
        QVERIFY(on.apply(StructEdit::insertRows(0, 0, 1), nullptr));
        QVERIFY(off.apply(StructEdit::insertRows(0, 0, 1), nullptr));
        QCOMPARE(state(on), state(off));
    }

    // A reference through several sheets stays in places (and is written again by an edit as before ids), and so does a text that has a name that could be a cell past XFD in it; both are counted.
    void textsThatCannotBeHeldInIdsStayInPlaces()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) {
            QVERIFY(b->open({{"A1=1", "A2=2", "A3=3"}, {"A1=10", "A2=20"}}, QString(), QStringLiteral("<definedName name=\"ABCD1\">Sheet1!$A$2</definedName>")));
            QVERIFY(b->apply(StructEdit::insertRows(0, 0, 1), nullptr));
            QVERIFY(b->calc->setFormula(1, 5, 0, QStringLiteral("SUM(Sheet1:Sheet2!A2)")));
            QVERIFY(b->calc->setFormula(1, 6, 0, QStringLiteral("ABCD1*2+A1")));
            QVERIFY(b->apply(StructEdit::insertRows(0, 0, 1), nullptr));
            QVERIFY(b->apply(StructEdit::deleteRows(1, 0, 1), nullptr));
            b->finish();
        }
        QCOMPARE(state(on), state(off));
        QCOMPARE(on.calc->positionalFormulas(), qint64(2));
        QCOMPARE(off.calc->positionalFormulas(), qint64(0));
    }

    // A formula kept apart for a function the engine lacks has #NAME? when the text it is shown as is written again (it can't be said what the function makes of it): with the formulas in ids the
    // stored text is not written again, but the shown one changes all the same.
    void anUnknownFunctionHasANameErrorWhenWhatItShowsChanges()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) {
            QVERIFY(b->open({{"A1=1", "A2=2", "A3=3", "B1==NOSUCHFN(A2)", "C1==A2+1"}}));
            const QString first = b->at("B1");
            QVERIFY(b->apply(StructEdit::insertRows(0, 6, 1), nullptr));   // below everything it reads: its text is as it was
            QCOMPARE(b->at("B1"), first);
            QVERIFY(b->apply(StructEdit::insertRows(0, 0, 1), nullptr));   // above: it is shown as NOSUCHFN(A3)
            QCOMPARE(b->formula("B2"), QStringLiteral("NOSUCHFN(A3)"));
            QCOMPARE(b->at("B2"), QStringLiteral("e:#NAME?"));
            QVERIFY(b->apply(StructEdit::insertColumns(0, 5, 1), nullptr));   // to the right of it: unchanged
            QCOMPARE(b->at("B2"), QStringLiteral("e:#NAME?"));
        }
        QCOMPARE(state(on), state(off));
        // what it reads is another formula whose text is written again, though neither moves: the row is put between A1 and A2, and C1 reads A2
        for (const bool far : {false, true}) {
            QString was[2];
            for (Book *b : {&off, &on}) {
                QVERIFY(b->open({{"A1=1", "A2=2", "B1==NOSUCHFN(C1)", "C1==A2+1"}}));
                was[b == &on] = b->at("B1");
                QVERIFY(b->apply(StructEdit::insertRows(0, far ? 30 : 1, 1), nullptr));
                QCOMPARE(b->formula("C1"), far ? QStringLiteral("A2+1") : QStringLiteral("A3+1"));
                if (!far) QCOMPARE(b->at("B1"), QStringLiteral("e:#NAME?"));
                else QCOMPARE(b->at("B1"), was[b == &on]);
            }
            QCOMPARE(was[1], was[0]);
        }
    }

    // A whole column (SUM(A:A)) reads every row: the rows put in at the top, after the data, and far past it as well, and a whole row every column. They are flags of the reference, not ids, so no formula
    // is written again or made again for any of the inserts, and every value is what it is with the formulas in places.
    void aWholeColumnAndAWholeRowReadWhatIsPutInAtTheTopAtTheEndAndPastIt()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) QVERIFY(b->open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B3=10", "C3=20", "D1==SUM(A:A)", "D2==SUM(3:3)", "E1==COUNTA(A:A)+ROWS(A:A)*0"}}));
        QCOMPARE(on.at("D1"), QStringLiteral("n:15"));
        QCOMPARE(on.at("D2"), QStringLiteral("n:33"));
        const Calculation::EditStats s0 = on.calc->editStats();
        auto both = [&](const StructEdit &e) {
            for (Book *b : {&off, &on}) QVERIFY(b->apply(e, nullptr));
            QCOMPARE(state(on), state(off));
        };
        auto type = [&](quint32 row, quint32 col, double v) {
            for (Book *b : {&off, &on}) {
                QVERIFY(b->calc->setValue(0, row, col, jp::calc::Value::number(v)));
                b->finish();
            }
            QCOMPARE(state(on), state(off));
        };
        both(StructEdit::insertRows(0, 0, 1));              // at the top: the formulas are in rows 2 to 4 now
        type(0, 0, 100);
        QCOMPARE(on.at("D2"), QStringLiteral("n:115"));
        both(StructEdit::insertRows(0, 6, 1));              // after the last row of data (row 6)
        type(6, 0, 1000);
        QCOMPARE(on.at("D2"), QStringLiteral("n:1115"));
        both(StructEdit::insertRows(0, 500, 1));            // past it
        type(500, 0, 10000);
        QCOMPARE(on.at("D2"), QStringLiteral("n:11115"));
        QCOMPARE(on.formula("D2"), QStringLiteral("SUM(A:A)"));
        QCOMPARE(on.formula("D3"), QStringLiteral("SUM(4:4)"));   // (the whole row followed its cells)
        QCOMPARE(on.at("E2"), QStringLiteral("n:8"));              // (COUNTA(A:A): the three numbers typed in the rows put in and the five of the data)
        QCOMPARE(on.at("D3"), QStringLiteral("n:33"));
        // columns at the first column, after the data, and past it, read by the whole row
        both(StructEdit::insertColumns(0, 0, 1));
        type(3, 0, 100);                                    // (row 4 of the sheet, which row 3 became)
        QCOMPARE(on.at("E3"), QStringLiteral("n:133"));
        both(StructEdit::insertColumns(0, 20, 1));
        type(3, 20, 1000);
        QCOMPARE(on.at("E3"), QStringLiteral("n:1133"));
        const Calculation::EditStats d = on.calc->editStats().since(s0);
        QCOMPARE(d.formulasRewritten, qint64(0));
        QCOMPARE(d.eagerFormulas, qint64(0));
        QCOMPARE(d.nodesRemade, qint64(0));
        QCOMPARE(d.textsMade, qint64(0));
        // a whole column loses the column that dies (an end of the reference dies: nothing is written again for it), the others are the same, and Undo brings it back
        std::vector<Calculation::EditRecord> recs(2);
        const Calculation::EditStats s1 = on.calc->editStats();
        for (size_t m = 0; m < 2; ++m) {
            Book &b = m ? on : off;
            QVERIFY(b.apply(StructEdit::deleteColumns(0, 1, 1), &recs[m]));   // the column of A (now B)
        }
        QCOMPARE(on.calc->editStats().since(s1).eagerFormulas, qint64(0));
        QCOMPARE(on.calc->editStats().since(s1).formulasRewritten, qint64(0));
        QCOMPARE(state(on), state(off));
        QCOMPARE(on.at("D2"), QStringLiteral("e:#REF!"));   // (SUM(A:A) lost its column: it is in the column before now)
        for (size_t m = 0; m < 2; ++m) {
            Book &b = m ? on : off;
            QVERIFY(b.calc->undoEdit(recs[m]));
            b.finish();
        }
        QCOMPARE(state(on), state(off));
        QCOMPARE(on.at("E3"), QStringLiteral("n:1133"));
    }

    // A reference through several sheets (Sheet1:Sheet3!A2) names the same place on every sheet, and the sheets have each an order of their own, so it is held in places: an edit of a sheet it spans
    // makes it again, one of another sheet does not, and with the formulas in places or in ids every value and every text is the same.
    void aReferenceThroughSeveralSheetsIsTheSameInBothModes()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        const QStringList s1 = {"A1=1", "A2=2", "A3=3", "B1==SUM(Sheet1:Sheet3!A2)", "B2==SUM(Sheet2:Sheet3!A1:A3)+A1"};
        const QStringList s2 = {"A1=10", "A2=20", "A3=30"};
        const QStringList s3 = {"A1=100", "A2=200", "A3=300"};
        const QStringList s4 = {"A1=5", "A2=6", "B1==Sheet3!A2+Sheet4!A1"};
        for (Book *b : {&off, &on}) QVERIFY(b->open({s1, s2, s3, s4}));
        QCOMPARE(on.at("B1"), QStringLiteral("n:222"));
        QCOMPARE(on.calc->positionalFormulas(), qint64(2));
        QCOMPARE(off.calc->positionalFormulas(), qint64(0));
        std::vector<Calculation::EditRecord> recs[2];
        auto both = [&](const StructEdit &e) {
            for (Book *b : {&off, &on}) {
                recs[b == &on].emplace_back();
                QVERIFY(b->apply(e, &recs[b == &on].back()));
            }
            QCOMPARE(state(on), state(off));
        };
        const qint64 remade0 = on.calc->editStats().nodesRemade;
        both(StructEdit::insertRows(1, 0, 1));      // Sheet2, which the reference spans: A2 of it is the old A1
        QCOMPARE(on.at("B1"), QStringLiteral("n:212"));
        QCOMPARE(on.formula("B1"), QStringLiteral("SUM(Sheet1:Sheet3!A2)"));
        both(StructEdit::deleteRows(2, 1, 1));      // Sheet3
        both(StructEdit::insertColumns(0, 0, 1));   // Sheet1: its own cells move; the reference reads A2 of every sheet, which is not where Sheet1's A2 went
        both(StructEdit::deleteRows(1, 0, 2));
        QVERIFY(on.calc->editStats().nodesRemade - remade0 >= 4);   // (made again: each of the two that spans the edited sheet, as before ids)
        // an edit of a sheet that no reference spans leaves them alone
        const qint64 remade1 = on.calc->editStats().nodesRemade;
        both(StructEdit::insertRows(3, 0, 2));
        QCOMPARE(on.calc->editStats().nodesRemade, remade1);
        while (!recs[1].empty()) {
            for (Book *b : {&off, &on}) {
                QVERIFY(b->calc->undoEdit(recs[b == &on].back()));
                recs[b == &on].pop_back();
                b->finish();
            }
            QCOMPARE(state(on), state(off));
        }
        QCOMPARE(on.at("B1"), QStringLiteral("n:222"));
    }

    // Defined names are held in ids like formulas: an edit of whole rows or columns writes no name again and makes no formula that uses one again (counted by the formulas, 4.14), the text a person
    // reads for a name (Names, Find) is in places, a single cell or a range whose lines die is #REF! or the lines next to them, Undo and a rebase keep every name, and the values and texts are those of the
    // workbook with the names in places. A name with a relative reference (a place relative to the cell that uses it) stays in places and is written again as before ids.
    void definedNamesAreHeldByIdAndFollowEveryEdit()
    {
        const QString names = QStringLiteral("<definedName name=\"Data\">Sheet1!$A$1:$A$10</definedName><definedName name=\"Last\">Sheet1!$A$10</definedName><definedName name=\"Total\">Sheet1!$A$3</definedName>"
                                             "<definedName name=\"Cols\">Sheet1!$A$2:$C$2</definedName><definedName name=\"Other\">Sheet2!$B$1</definedName>");
        QStringList cells = {"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "A6=6", "A7=7", "A8=8", "A9=9", "A10=10", "B2=20", "C2=30", "B1==Last*2", "B3==Total+1", "B4==SUM(Cols)+Other", "B5==SUM(Data)+Sheet2!A2"};
        for (int r = 1; r <= 400; ++r) cells << QStringLiteral("D%1==SUM(Data)+A%1").arg(r + 11);
        const QStringList other = {"A1=100", "A2=200", "B1=7"};
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) QVERIFY(b->open({cells, other}, QString(), names));
        QCOMPARE(on.at("B5"), QStringLiteral("n:255"));
        auto textsOf = [&](Book &b) {
            QStringList out;
            for (const char *name : {"Data", "Last", "Total", "Cols", "Other"}) {
                QString t;
                b.calc->nameText(-1, QString::fromLatin1(name), &t);
                out << QStringLiteral("%1=%2").arg(QString::fromLatin1(name), t);
            }
            return out;
        };
        QCOMPARE(textsOf(on), textsOf(off));
        std::vector<Calculation::EditRecord> recs[2];
        const Calculation::EditStats s0 = on.calc->editStats();
        auto both = [&](const StructEdit &e) {
            for (Book *b : {&off, &on}) {
                recs[b == &on].emplace_back();
                QVERIFY(b->apply(e, &recs[b == &on].back()));
            }
            QCOMPARE(state(on), state(off));
            QCOMPARE(textsOf(on), textsOf(off));
        };
        // edits that touch the cells of a name, an end of it, and the whole of it; none writes a name or makes a formula that uses one again
        both(StructEdit::insertRows(0, 0, 1));
        QCOMPARE(textsOf(on)[0], QStringLiteral("Data=Sheet1!$A$2:$A$11"));
        both(StructEdit::insertRows(0, 6, 2));      // inside Data
        QCOMPARE(textsOf(on)[0], QStringLiteral("Data=Sheet1!$A$2:$A$13"));
        both(StructEdit::insertColumns(0, 1, 1));   // inside Cols
        QCOMPARE(textsOf(on)[3], QStringLiteral("Cols=Sheet1!$A$3:$D$3"));
        both(StructEdit::deleteRows(0, 12, 1));     // the last row of Data and the cell Last is
        QCOMPARE(textsOf(on)[0], QStringLiteral("Data=Sheet1!$A$2:$A$12"));
        QCOMPARE(textsOf(on)[1], QStringLiteral("Last=Sheet1!#REF!"));
        both(StructEdit::deleteRows(0, 3, 1));      // the cell Total is
        QCOMPARE(textsOf(on)[2], QStringLiteral("Total=Sheet1!#REF!"));
        const Calculation::EditStats d = on.calc->editStats().since(s0);
        QCOMPARE(d.namesRewritten, qint64(0));
        QCOMPARE(d.namesRegisteredAgain, qint64(0));
        QCOMPARE(d.nodesRemade, qint64(0));
        QCOMPARE(d.formulasRewritten, qint64(0));
        QVERIFY(on.calc->nameCount() == 5);
        // Undo brings every name and what it stands for back, and Redo takes it away again
        for (int i = 0; i < 5; ++i) {
            for (Book *b : {&off, &on}) {
                QVERIFY(b->calc->undoEdit(recs[b == &on].back()));
                recs[b == &on].pop_back();
                b->finish();
            }
            QCOMPARE(state(on), state(off));
            QCOMPARE(textsOf(on), textsOf(off));
        }
        QCOMPARE(textsOf(on)[0], QStringLiteral("Data=Sheet1!$A$1:$A$10"));
        QCOMPARE(on.at("B5"), QStringLiteral("n:255"));
        // a name typed in places after edits is held by ids, and a rebase renumbers it with the rest
        both(StructEdit::insertRows(0, 0, 2));
        both(StructEdit::deleteRows(0, 4, 1));
        for (Book *b : {&off, &on}) {
            b->calc->setNameText(-1, QStringLiteral("Later"), QStringLiteral("Sheet1!$A$5:$A$6"));
            QVERIFY(b->calc->setFormula(0, 20, 5, QStringLiteral("SUM(Later)+SUM(Data)")));
            b->finish();
        }
        QCOMPARE(state(on), state(off));
        std::vector<jp::calc::LineMapCore::Renumber> table;
        QVERIFY(on.calc->compactLines(0, true, &table));
        QVERIFY(on.calc->compactLines(0, false, &table));
        on.finish();
        QCOMPARE(state(on), state(off));
        QCOMPARE(textsOf(on), textsOf(off));
        QString later[2];
        on.calc->nameText(-1, QStringLiteral("Later"), &later[1]);
        off.calc->nameText(-1, QStringLiteral("Later"), &later[0]);
        QCOMPARE(later[1], later[0]);
        both(StructEdit::insertRows(0, 0, 1));
    }

    // A name that is relative to the cell that uses it (a row that is not absolute), a name through several sheets, and a word of a formula that could be a cell past XFD are held in places, as before ids:
    // the first is written again by an edit of columns, and the values and texts are those of the workbook with every name in places.
    void namesThatIdsCannotHoldStayInPlaces()
    {
        const QString names = QStringLiteral("<definedName name=\"Rel\">Sheet1!A2</definedName><definedName name=\"Mixed\">Sheet1!$A2</definedName><definedName name=\"Span\">SUM(Sheet1:Sheet2!$A$1)</definedName>"
                                             "<definedName name=\"ABCD1\">Sheet1!$A$3</definedName><definedName name=\"Abs\">Sheet1!$A$4</definedName>");
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) QVERIFY(b->open({{"A1=1", "A2=2", "A3=3", "A4=4", "C1==Rel*10", "C2==Span+ABCD1", "C3==Abs*2", "C4==Mixed*3"}, {"A1=5"}}, QString(), names));
        const Calculation::EditStats s0 = on.calc->editStats();
        for (Book *b : {&off, &on}) {
            QVERIFY(b->apply(StructEdit::insertColumns(0, 0, 1), nullptr));
            QVERIFY(b->apply(StructEdit::insertRows(0, 0, 1), nullptr));
            QVERIFY(b->apply(StructEdit::deleteRows(0, 2, 1), nullptr));
            QVERIFY(b->apply(StructEdit::deleteColumns(0, 3, 1), nullptr));
        }
        QCOMPARE(state(on), state(off));
        for (const char *name : {"Rel", "Mixed", "Span", "ABCD1", "Abs"}) {
            QString a, b;
            on.calc->nameText(-1, QString::fromLatin1(name), &a);
            off.calc->nameText(-1, QString::fromLatin1(name), &b);
            QCOMPARE(a, b);
        }
        const Calculation::EditStats d = on.calc->editStats().since(s0);
        QVERIFY(d.namesRewritten >= 1);   // (Mixed is written again by the column put in; Abs, in ids, is not)
        QVERIFY(d.namesRewritten < off.calc->editStats().namesRewritten);
    }

    // A line put inside a block, and taken away again by Undo, changes what a formula that counts or picks lines reads (INDEX, COLUMNS): it is worked out again when the line dies, as it is when it comes.
    void anInsertedLineThatIsUndoneAsksTheFormulasThatCanNoticeIt()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) QVERIFY(b->open({{"A1=1", "B1=2", "C1=3", "A2=4", "B2=5", "C2=6", "E1==INDEX(A1:C2,1,2)", "E2==COLUMNS(A1:C2)", "E3==SUM(A1:C2)"}}));
        std::vector<Calculation::EditRecord> recs[2];
        auto both = [&](const StructEdit &e) {
            for (Book *b : {&off, &on}) {
                recs[b == &on].emplace_back();
                QVERIFY(b->apply(e, &recs[b == &on].back()));
            }
            QCOMPARE(state(on), state(off));
        };
        auto undo = [&]() {
            for (Book *b : {&off, &on}) {
                QVERIFY(b->calc->undoEdit(recs[b == &on].back()));
                recs[b == &on].pop_back();
                b->finish();
            }
            QCOMPARE(state(on), state(off));
        };
        both(StructEdit::insertColumns(0, 0, 2));
        both(StructEdit::deleteColumns(0, 3, 1));    // the second column of the block
        both(StructEdit::insertColumns(0, 3, 1));    // one put between what is left of it
        QCOMPARE(on.at("G1"), QStringLiteral("n:0"));
        undo();
        QCOMPARE(on.at("F1"), QStringLiteral("n:3"));
        QCOMPARE(on.at("F2"), QStringLiteral("n:2"));
        undo();
        undo();
        QCOMPARE(on.at("E1"), QStringLiteral("n:2"));
    }

    // A formula that is deleted, or written again by cells that shift, after a line it reads died: Undo of both, one after the other, gives it back reading that line (the text a person read had the line next
    // to it, or #REF!); with the formulas in places the record of the first step has the old text.
    void aFormulaWrittenOrDeletedAfterItsLineDiedReadsTheLineAgainWhenBothAreUndone()
    {
        for (const int variant : {0, 1}) {
            Book on, off;
            on.idFrame = true;
            off.forceOff = true;
            for (Book *b : {&off, &on}) QVERIFY(b->open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "C6==SUM(A2:A4)+A2", "D1==SUM(A1:A5)", "D2==A3*2"}}));
            const QStringList start = state(on);
            QCOMPARE(start, state(off));
            std::vector<Calculation::EditRecord> recs[2];
            CellRange block;
            block.valid = true;
            block.row1 = 2;
            block.row2 = 3;
            block.col1 = 0;
            block.col2 = 0;
            const QVector<StructEdit> edits = {StructEdit::deleteRows(0, 1, 1),   // the first row C6 reads
                                               variant == 0 ? StructEdit::deleteRows(0, 4, 1)   // and the row C6 is in
                                                            : StructEdit::insertCellsDown(0, block)};   // or cells that C6 reads are shifted: its text is written again
            for (const StructEdit &e : edits)
                for (Book *b : {&off, &on}) {
                    recs[b == &on].emplace_back();
                    QVERIFY(b->apply(e, &recs[b == &on].back()));
                }
            QCOMPARE(state(on), state(off));
            for (int i = 0; i < 2; ++i) {
                for (Book *b : {&off, &on}) {
                    QVERIFY(b->calc->undoEdit(recs[b == &on].back()));
                    recs[b == &on].pop_back();
                    b->finish();
                }
                QCOMPARE(state(on), state(off));
            }
            QCOMPARE(state(on), start);
            QCOMPARE(on.formula("C6"), QStringLiteral("SUM(A2:A4)+A2"));
        }
    }

    // A range whose start died and now resolves to a line that was inserted: when that line goes, the range has other lines (COLUMNS(A1:C1) over a column put in at B, then column A deleted twice). The
    // question after an edit named only the formulas whose block holds the lines either side of the gap, and a dead end is not in any block. Both modes, and the same as a calculation from nothing.
    void aRangeWhoseEndDiedAndNowResolvesToAnInsertedLineIsAskedWhenThatLineGoes()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) QVERIFY(b->open({{"A1=1", "B1=2", "C1=3", "D1=4", "E1==COLUMNS(A1:C1)", "E2==SUM(A1:C1)", "E3==INDEX(A1:C1,1,2)"}}));
        std::vector<Calculation::EditRecord> recs[2];
        auto both = [&](const StructEdit &e) {
            for (Book *b : {&off, &on}) {
                recs[b == &on].emplace_back();
                QVERIFY(b->apply(e, &recs[b == &on].back()));
            }
            QCOMPARE(state(on), state(off));
        };
        both(StructEdit::insertColumns(0, 1, 1));
        QCOMPARE(on.at("F1"), QStringLiteral("n:4"));
        both(StructEdit::deleteColumns(0, 0, 1));
        QCOMPARE(on.at("E1"), QStringLiteral("n:3"));
        both(StructEdit::deleteColumns(0, 0, 1));
        QCOMPARE(on.at("D1"), QStringLiteral("n:2"));
        QCOMPARE(off.at("D1"), QStringLiteral("n:2"));
        QCOMPARE(on.at("D3"), QStringLiteral("n:3"));
        for (Book *b : {&off, &on}) {
            b->calc->invalidateAll();
            b->finish();
        }
        QCOMPARE(on.at("D1"), QStringLiteral("n:2"));
        // and Undo of the second delete, which brings the line back
        for (Book *b : {&off, &on}) {
            QVERIFY(b->calc->undoEdit(recs[b == &on].back()));
            b->finish();
        }
        QCOMPARE(state(on), state(off));
        QCOMPARE(on.at("E1"), QStringLiteral("n:3"));
    }

    // A text in places that is written again by an edit as a text that ids can hold (a range written back to front is put in order) is held in ids from then on: its node tells the graph again and is in the
    // list of the formulas that have an inserted line at an end, so that the line going is asked of it.
    void aFormulaThatIsHeldInIdsOnceItIsWrittenAgainIsAskedWhenAnEndGoes()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        QStringList s1;
        for (int r = 1; r <= 16; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r);
        QStringList s2{"B1=2", "C1=3"};
        for (int r = 1; r <= 12; ++r) s2 << QStringLiteral("A%1=%2").arg(r).arg(r);
        for (Book *b : {&off, &on}) QVERIFY(b->open({s1, s2}));
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::insertColumns(1, 0, 2), nullptr));
        for (Book *b : {&off, &on}) {
            QVERIFY(b->calc->setFormula(0, 7, 9, QStringLiteral("SUM(Sheet2!A12:A1)")));   // (back to front: held in places)
            b->finish();
        }
        QCOMPARE(on.calc->positionalFormulas(), qint64(1));
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::insertColumns(1, 0, 2), nullptr));
        QCOMPARE(on.calc->positionalFormulas(), qint64(0));   // (written in order, it is held in ids)
        QCOMPARE(off.at("J8"), QStringLiteral("n:0"));
        QCOMPARE(on.at("J8"), QStringLiteral("n:0"));
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::deleteColumns(1, 2, 2), nullptr));
        // (the shown value, before anything is worked out again: both ends were the same column put in, and it is gone)
        QCOMPARE(off.at("J8"), QStringLiteral("e:#REF!"));
        QCOMPARE(on.at("J8"), QStringLiteral("e:#REF!"));
        QCOMPARE(state(on), state(off));
        for (Book *b : {&off, &on}) {
            b->calc->invalidateAll();
            b->finish();
        }
        QCOMPARE(state(on), state(off));
    }

    // A rebase of the rows keeps the order of the ends of a reference on the columns, as the text names them (a range from a column that was inserted to one before it): else a dead end is read the wrong way.
    void aRebaseOfOneKindOfLineKeepsTheOrderOfTheEndsOnTheOther()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        QStringList s1;
        for (int r = 1; r <= 8; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1=%2").arg(r).arg(r * 10);
        for (Book *b : {&off, &on}) QVERIFY(b->open({s1}));
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::insertColumns(0, 1, 1), nullptr));   // a column between A and B
        for (Book *b : {&off, &on}) {
            QVERIFY(b->calc->setFormula(0, 9, 0, QStringLiteral("SUM(C1:B3)")));   // written back to front, from a column of the file's to the inserted one
            b->finish();
        }
        std::vector<jp::calc::LineMapCore::Renumber> table;
        QVERIFY(on.calc->compactLines(0, true, &table));   // a rebase of the rows
        on.finish();
        QCOMPARE(state(on), state(off));
        QCOMPARE(on.formula("A10"), off.formula("A10"));   // (the columns are still named as they were written)
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::deleteColumns(0, 1, 1), nullptr));   // the column that was put in goes
        QCOMPARE(state(on), state(off));
        QCOMPARE(on.formula("A10"), off.formula("A10"));
    }

    // A formula kept apart (a function the engine lacks) is put back by Undo as it was held, a line that had died before still being that line.
    void aFormulaKeptApartIsPutBackReadingTheLineThatHadDied()
    {
        for (const int variant : {0, 1}) {
            Book on, off;
            on.idFrame = true;
            off.forceOff = true;
            QStringList cells = {"A5=1", "B5=2", "C5=3", "A6=4", "B6=5", "C6=6", "A7=7", "B7=8", "C7=9"};
            cells << (variant == 0 ? "H1==NOSUCHFN(A5:C7)" : "H1==H1+SUM(A5:C7)");
            for (Book *b : {&off, &on}) QVERIFY(b->open({cells}));
            const QStringList start = state(on);
            std::vector<Calculation::EditRecord> recs[2];
            for (const StructEdit &e : {StructEdit::deleteColumns(0, 0, 1), StructEdit::insertColumns(0, 0, 1)})
                for (Book *b : {&off, &on}) {
                    recs[b == &on].emplace_back();
                    QVERIFY(b->apply(e, &recs[b == &on].back()));
                }
            QCOMPARE(state(on), state(off));
            for (int i = 0; i < 2; ++i)
                for (Book *b : {&off, &on}) {
                    QVERIFY(b->calc->undoEdit(recs[b == &on].back()));
                    recs[b == &on].pop_back();
                    b->finish();
                }
            QCOMPARE(state(on), state(off));
            QCOMPARE(on.formula("H1"), off.formula("H1"));
            QVERIFY2(on.formula("H1").contains(QLatin1String("A5:C7")), qPrintable(on.formula("H1")));
        }
    }

    // The formula the review found (a block of cells put in below inside the columns a range reads, after a column it starts on was deleted): Undo of the cells and then of the column delete shows the range
    // from the column that came back, as before.
    void undoOfAnEditPutsBackTheDeadEndOfAFormulaItRewrote()
    {
      for (const char *text : {"E2==SUM(A2:C4)", "E2==NOSUCHFN(A2:C4)", "E2==SUM(A2:C4)+E2"}) {   // (a formula that is worked out, one the engine lacks, and a circle: the last two are kept apart)
        QStringList s1;
        for (int r = 1; r <= 16; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r);
        s1 << QString::fromLatin1(text);
        Book off, on;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) QVERIFY(b->open({s1, {"A1=1"}}));
        std::vector<Calculation::EditRecord> recs[2];
        auto both = [&](const StructEdit &e) {
            for (Book *b : {&off, &on}) {
                recs[b == &on].emplace_back();
                QVERIFY(b->apply(e, &recs[b == &on].back()));
            }
            QCOMPARE(state(on), state(off));
        };
        auto undo = [&]() {
            for (Book *b : {&off, &on}) {
                QVERIFY(b->calc->undoEdit(recs[b == &on].back()));
                recs[b == &on].pop_back();
                b->finish();
            }
            QCOMPARE(state(on), state(off));
        };
        both(StructEdit::insertRows(0, 0, 3));
        both(StructEdit::deleteColumns(0, 0, 1));
        CellRange blk;
        blk.valid = true;
        blk.row1 = 6;
        blk.row2 = 9;
        blk.col1 = 0;
        blk.col2 = 1;
        both(StructEdit::insertCellsDown(0, blk));
        undo();
        undo();
        QCOMPARE(on.formula("E5"), off.formula("E5"));
        QVERIFY2(on.formula("E5").contains(QLatin1String("A5:C7")), qPrintable(on.formula("E5")));
      }
    }

    // The edits the differential test found that flipped the ends of a reference on the other axis at a rebase (seeds 167 and 213 of the first 600): the first rebases the rows of a book whose columns are
    // named back to front by a name and a formula, the second a name.
    void aRebaseKeepsTheOrderOfTheEndsOfWhatItDoesNotRenumber()
    {
        auto mk = [](int kind, int sheet, quint32 at, quint32 count, const char *typed, quint32 row, quint32 col, const char *nm, const char *nameTxt, quint32 bcol2) {
            Op o;
            o.kind = kind;
            o.sheet = sheet;
            o.at = at;
            o.count = count;
            o.typed = QString::fromLatin1(typed);
            o.row = row;
            o.col = col;
            o.nm = QString::fromLatin1(nm);
            o.nameTxt = QString::fromLatin1(nameTxt);
            o.bcol2 = bcol2;
            return o;
        };
        int at = -1;
        const std::vector<Op> first = {
        mk(4, 0, 0, 2, "SUM(Sheet1!A4:A9)+N1", 4, 7, "N3", "Sheet1!$B$1:$B$4", 0),
        mk(1, 0, 1, 4, "SUM(Sheet1!A5:A11)+N1", 5, 9, "N1", "Sheet1!$A$5:$A$12", 1),
        mk(9, 1, 7, 2, "SUM(Sheet1!A3:A13)", 3, 9, "N2", "Sheet1!$B$1:$B$4", 0),
        mk(3, 0, 1, 2, "SUM(Sheet1!A2:A6)+N1", 11, 7, "N2", "Sheet1!$B$1:$B$4", 1),
        mk(16, 0, 2, 1, "SUM(Sheet1!A6:A6)", 5, 7, "N3", "Sheet1!$A$3:$A$9", 0)
        };
        QCOMPARE(runOps(first, &at), QString());
        const std::vector<Op> second = {
        mk(5, 0, 0, 1, "SUM(Sheet2!A10:A2)", 11, 9, "N4", "Sheet1!$A$5:$A$12", 1),
        mk(0, 0, 4, 2, "SUM(Sheet1!A1:A2)", 7, 9, "N2", "Sheet2!$A$1:$A$3", 1),
        mk(2, 0, 0, 3, "SUM(Sheet1!A1:A10)+N1", 9, 7, "N3", "Sheet1!$A$3:$A$9", 1),
        mk(11, 0, 7, 2, "SUM(Sheet1!A1:A8)", 4, 8, "N3", "Sheet1!$A$3:$A$9", 0),
        mk(16, 0, 0, 2, "SUM(Sheet1!A10:A13)", 10, 8, "N1", "Sheet1!$A$5:$A$12", 0),
        mk(3, 0, 1, 4, "SUM(Sheet2!A4:A12)", 7, 7, "N2", "Sheet1!$A$3:$A$9", 1)
        };
        QCOMPARE(runOps(second, &at), QString());
    }

    // A name edited after a line it names was deleted: Undo of the edit puts the name back as it was held, and Undo of the delete then brings the line back to it.
    void undoOfANameEditAfterADeleteKeepsTheNameForTheUndoOfTheDelete()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        QStringList s1;
        for (int r = 1; r <= 12; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1=%2").arg(r).arg(r * 2);
        s1 << "D1==SUM(N1)";
        for (Book *b : {&off, &on}) {
            QVERIFY(b->open({s1}));
            b->calc->setNameText(-1, QStringLiteral("N1"), QStringLiteral("Sheet1!$A$3:$A$9"));
            b->finish();
        }
        Calculation::EditRecord rec[2];
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::deleteColumns(0, 0, 1), &rec[b == &on]));
        QString was[2], now[2];
        bool has[2];
        for (Book *b : {&off, &on}) {
            const int m = b == &on;
            was[m] = b->calc->nameTagged(-1, QStringLiteral("N1"), &has[m]);
            b->calc->setNameText(-1, QStringLiteral("N1"), QStringLiteral("Sheet1!$B$1:$B$3"));   // redefined
            b->finish();
            b->calc->setNameTagged(-1, QStringLiteral("N1"), has[m], was[m]);   // Undo
            b->finish();
            QVERIFY(b->calc->undoEdit(rec[m]));   // Undo of the delete
            b->finish();
            b->calc->nameText(-1, QStringLiteral("N1"), &now[m]);
        }
        QCOMPARE(now[1], QStringLiteral("Sheet1!$A$3:$A$9"));
        QCOMPARE(now[0], now[1]);
        QCOMPARE(state(on), state(off));
    }

    // A name's record (the text Undo puts back) is held in ids with the renumber epochs of the sheets: after a rebase it is the text in places, held anew. For an edit that shifts cells, deletes cells, and a move.
    void aNameRestoredByUndoAfterARebaseIsHeldAnew()
    {
        for (int variant = 0; variant < 3; ++variant) {
            Book off, on;
            on.idFrame = true;
            off.forceOff = true;
            QStringList s1;
            for (int r = 1; r <= 16; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r);
            s1 << "B4==SUM(N1)" << "B5==N2*2";
            QStringList out;
            for (Book *b : {&off, &on}) {
                QVERIFY(b->open({s1, {"A1=1"}}));
                b->calc->setNameText(-1, QStringLiteral("N1"), QStringLiteral("Sheet1!$A$3:$A$9"));
                b->calc->setNameText(-1, QStringLiteral("N2"), QStringLiteral("Sheet1!$A$6"));
                b->finish();
                QVERIFY(b->apply(StructEdit::deleteRows(0, 0, 1), nullptr));
                CellRange blk;
                blk.valid = true;
                blk.row1 = 1;
                blk.row2 = 2;
                blk.col1 = 0;
                blk.col2 = 0;
                Calculation::EditRecord rec;
                if (variant == 0) QVERIFY(b->apply(StructEdit::insertCellsDown(0, blk), &rec));
                else if (variant == 1) QVERIFY(b->apply(StructEdit::deleteCellsUp(0, blk), &rec));
                else QVERIFY(b->apply(StructEdit::move(0, blk, 0, 0, 3), &rec));
                if (b == &on) {
                    std::vector<jp::calc::LineMapCore::Renumber> table;
                    QVERIFY(on.calc->compactLines(0, true, &table));
                    on.finish();
                }
                QVERIFY(b->calc->undoEdit(rec));
                b->finish();
                QString t1, t2;
                b->calc->nameText(-1, QStringLiteral("N1"), &t1);
                b->calc->nameText(-1, QStringLiteral("N2"), &t2);
                out << (QStringLiteral("variant %1 ").arg(variant) + t1 + QStringLiteral(" / ") + t2 + QStringLiteral(" B4=") + b->at("B4") + QStringLiteral(" B5=") + b->at("B5"));
            }
            QVERIFY2(out[0] == out[1], qPrintable(out.join(QLatin1Char('\n'))));
        }
    }

    // 20,000 formulas that end on one line that was inserted: dropping them all (their column goes) looks at each entry of the list once, not at the whole list for each.
    void formulasThatEndOnOneInsertedLineAreLetGoOfInLinearWork()
    {
        Book on;
        on.idFrame = true;
        QStringList s1;
        for (int r = 1; r <= 8; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r);
        QVERIFY(on.open({s1}));
        QVERIFY(on.apply(StructEdit::insertRows(0, 4, 1), nullptr));
        const int n = 20000;
        for (int i = 0; i < n; ++i) on.calc->setFormula(0, quint32(i), 3, QStringLiteral("SUM($A$1:$A$5)"));
        on.finish();
        const qint64 before = on.calc->insertedEndsWork();
        QVERIFY(before >= n);
        QVERIFY(on.apply(StructEdit::deleteColumns(0, 3, 1), nullptr));
        QVERIFY(on.calc->insertedEndsWork() - before <= 4 * qint64(n));
        QVERIFY(on.apply(StructEdit::deleteRows(0, 4, 1), nullptr));   // and the line itself going, with nothing left to ask
        QCOMPARE(on.calc->insertedEndsWork() - before <= 5 * qint64(n), true);
    }

    // The lines put in at the front of the sheet are the start of a reference that has no original line before it, and a reference whose end died stays as wide as it was while the line it reads
    // is one that was put in: an insert inside either is asked of them, in places as in ids (found by the differential test, seeds 487 and 803 of the first 1,000).
    void aReferenceWithAnInsertedEndIsAskedWhenLinesAreAddedInsideIt()
    {
        auto op = [](int kind, quint32 at, quint32 count, quint32 bcol2 = 0, int sheet = 0) {
            Op o;
            o.kind = kind;
            o.sheet = sheet;
            o.at = at;
            o.count = count;
            o.bcol2 = bcol2;
            o.nm = QStringLiteral("N1");
            o.nameTxt = QStringLiteral("Sheet1!$B$1:$B$4");
            return o;
        };
        int at = -1;
        // columns: put in at C, A and B deleted, put in at C again (COLUMNS(A1:C1) reads from the first inserted column)
        const std::vector<Op> columns = {op(4, 8, 3), op(5, 0, 3), op(16, 6, 1), op(4, 2, 1)};
        QCOMPARE(runOps(columns, &at), QString());
        // rows: lines put in inside a range, some before and the end of the range deleted, then a line put in inside what is left
        const std::vector<Op> rows = {op(1, 8, 3), op(16, 1, 1), op(3, 3, 3), op(3, 6, 4), op(1, 5, 1)};
        QCOMPARE(runOps(rows, &at), QString());
        // lines put in at the front (and cells shifted up under them), then a line put in among them: the references that start on a line put in at the front have the gap inside
        // (the sheet's first original line is the end of the gap)
        const std::vector<Op> front = {op(0, 0, 3), op(7, 0, 4, 1), op(0, 3, 4)};
        QCOMPARE(runOps(front, &at), QString());
        const std::vector<Op> front2 = {op(0, 0, 4), op(7, 0, 4, 1), op(0, 2, 3)};
        QCOMPARE(runOps(front2, &at), QString());
        // the same after many edits: lines deleted between the neighbours of the gap, and the start of the reference one of those
        // a column put in at the top, a size typed that starts on it, and the column deleted: the range starts on the next column (found by the model check's wide option)
        Op typed = op(8, 0, 1);
        typed.typed = QStringLiteral("COLUMNS(A1:C1)");
        typed.row = 0;
        typed.col = 10;
        const std::vector<Op> top = {op(4, 0, 2), typed, op(5, 0, 2)};
        QCOMPARE(runOps(top, &at), QString());
        Op typedRows = op(8, 0, 1);
        typedRows.typed = QStringLiteral("ROWS(A1:A3)");
        typedRows.row = 0;
        typedRows.col = 10;
        const std::vector<Op> topRows = {op(0, 0, 1), typedRows, op(2, 0, 1)};
        QCOMPARE(runOps(topRows, &at), QString());
        const std::vector<Op> many = {op(1, 0, 3), op(6, 3, 2, 1), op(4, 6, 1, 1), op(0, 1, 4), op(0, 11, 1, 1), op(1, 8, 4, 1), op(0, 12, 3), op(3, 12, 2, 1), op(3, 1, 3, 1), op(3, 10, 3), op(10, 13, 4, 0, 1), op(7, 1, 4, 1), op(0, 11, 3, 1)};
        QCOMPARE(runOps(many, &at), QString());
    }

    // A formula typed over a cell whose formula read lines that were deleted, and Undo of the typing then of the delete: the cell has the formula as it was held, so that the lines come back to it (the
    // session keeps a cell's content for Undo, with the formula as it is held).
    void undoOfTypingOverAFormulaWithADeadEndGivesItBackItsLines()
    {
        auto op = [](int kind, int sheet, quint32 at, quint32 count, const char *typed = "", quint32 row = 0, quint32 col = 7) {
            Op o;
            o.kind = kind;
            o.sheet = sheet;
            o.at = at;
            o.count = count;
            o.typed = QString::fromLatin1(typed);
            o.row = row;
            o.col = col;
            o.nm = QStringLiteral("N1");
            o.nameTxt = QStringLiteral("Sheet1!$A$3:$A$9");
            return o;
        };
        int at = -1;
        const std::vector<Op> ops = {op(8, 1, 11, 2, "SUM(Sheet1!A9:A13)", 1, 8), op(2, 0, 11, 2), op(9, 1, 1, 2, "SUM(Sheet2!A8:A9)", 1, 8), op(12, 1, 11, 3), op(12, 0, 9, 1)};
        QCOMPARE(runOps(ops, &at), QString());
    }

    // Real edits of a workbook with a table, in places and in ids, one after the other with the tables following the cells as the session does it: rows and columns put in and taken out inside the table, at
    // its edges, and around it; cells shifted in its columns; Undo and Redo of them; and a rebase. The formulas that name the table (its columns, a cell of its row, its totals, its whole data) are on its sheet
    // and on another, and everything they show and the table's place must be the same in both. Without totals and with.
    void tablesGrowAndShrinkTheSameInPlacesAndIds()
    {
        for (const bool totals : {false, true}) {
            const QString nsUrl = QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main");
            const QString part = totals
                ? QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C7\" totalsRowCount=\"1\"><autoFilter ref=\"A1:C6\"/>"
                                 "<tableColumns count=\"3\"><tableColumn id=\"1\" name=\"Region\" totalsRowLabel=\"Total\"/><tableColumn id=\"2\" name=\"Q1\" totalsRowFunction=\"sum\"/>"
                                 "<tableColumn id=\"3\" name=\"Twice\"><calculatedColumnFormula>Sales[[#This Row],[Q1]]*2</calculatedColumnFormula></tableColumn></tableColumns></table>").arg(nsUrl)
                : QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C6\"><autoFilter ref=\"A1:C6\"/>"
                                 "<tableColumns count=\"3\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/>"
                                 "<tableColumn id=\"3\" name=\"Twice\"><calculatedColumnFormula>Sales[[#This Row],[Q1]]*2</calculatedColumnFormula></tableColumn></tableColumns></table>").arg(nsUrl);
            QStringList s1 = {"A1=Region", "B1=Q1", "C1=Twice"};
            for (int r = 2; r <= (totals ? 6 : 6); ++r)
                s1 << QStringLiteral("A%1=R%1").arg(r) << QStringLiteral("B%1=%2").arg(r).arg(r * 3) << QStringLiteral("C%1==Sales[[#This Row],[Q1]]*2").arg(r);
            if (totals) s1 << "A7=Total" << "B7==SUBTOTAL(109,Sales[Q1])";
            s1 << "E1==SUM(Sales[Q1])" << "E2==ROWS(Sales[Region])" << "E3==SUM(Sales[Twice])+COUNTA(Sales[Region])" << "E4==INDEX(Sales[Q1],2)" << "F1==SUM(B2:B4)" << "F2==SUM(A1:C3)";
            if (totals) s1 << "E5==SUM(Sales[[#Totals],[Q1]])" << "E6==ROWS(Sales[#All])";
            const QStringList s2 = {"A1==SUM(Sheet1!Sales[Q1])", "A2==ROWS(Sales)", "A3==SUM(Sheet1!B2:B4)", "A4==SUM(Sales[[#Headers],[Q1]])"};
            for (quint32 seed = 0; seed < 30; ++seed) {
                Book off, on;
                on.idFrame = true;
                off.forceOff = true;
                for (Book *b : {&off, &on}) {
                    b->tableParts = {part};
                    QVERIFY(b->open({s1, s2}));
                }
                QCOMPARE(state(on), state(off));
                QCOMPARE(on.at("E1"), QStringLiteral("n:%1").arg(totals ? 60 : 60));
                struct Done {
                    Calculation::EditRecord rec;
                    std::vector<TablePlace> places;
                };
                std::vector<Done> done[2], undone[2];
                QRandomGenerator rng(seed * 104729u + 7u);
                for (int step = 0; step < 14; ++step) {
                    const quint32 kind = rng.bounded(9u);
                    const quint32 at = rng.bounded(9u);
                    const quint32 count = 1 + rng.bounded(2u);
                    const QString what = QStringLiteral("totals %1 seed %2 step %3 kind %4 at %5 count %6").arg(totals).arg(seed).arg(step).arg(kind).arg(at).arg(count);
                    CellRange blk;
                    blk.valid = true;
                    blk.row1 = at;
                    blk.row2 = at + count - 1;
                    blk.col1 = 0;
                    blk.col2 = 1 + rng.bounded(2u);
                    bool ok[2] = {true, true};
                    for (int m = 0; m < 2; ++m) {
                        Book &b = m ? on : off;
                        Done d;
                        switch (kind) {
                        case 0: case 1: ok[m] = b.applyWithTables(StructEdit::insertRows(0, at, count), &d.rec, &d.places); break;
                        case 2: ok[m] = b.applyWithTables(StructEdit::deleteRows(0, at, count), &d.rec, &d.places); break;
                        case 3: ok[m] = b.applyWithTables(StructEdit::insertColumns(0, at % 5, count), &d.rec, &d.places); break;
                        case 4: ok[m] = b.applyWithTables(StructEdit::deleteColumns(0, at % 5, count), &d.rec, &d.places); break;
                        case 5: ok[m] = b.applyWithTables(StructEdit::insertCellsDown(0, blk), &d.rec, &d.places); break;
                        case 6:
                            if (!done[m].empty()) {
                                ok[m] = b.undoWithTables(done[m].back().rec, done[m].back().places);
                                undone[m].push_back(std::move(done[m].back()));
                                done[m].pop_back();
                            }
                            continue;
                        case 7:
                            if (!undone[m].empty()) {
                                Done again = std::move(undone[m].back());
                                undone[m].pop_back();
                                ok[m] = b.applyWithTables(again.rec.edit, &again.rec, &again.places);
                                done[m].push_back(std::move(again));
                            }
                            continue;
                        default:
                            if (m == 1) {
                                QVERIFY2(on.calc->compactLines(0, at % 2 != 0), qPrintable(what));
                                on.finish();
                            }
                            done[m].clear();
                            undone[m].clear();
                            continue;
                        }
                        if (ok[m]) {
                            done[m].push_back(std::move(d));
                            undone[m].clear();
                        }
                    }
                    QVERIFY2(ok[0] == ok[1], qPrintable(what));
                    QVERIFY2(state(on) == state(off), qPrintable(what + QStringLiteral("\nplaces: ") + state(off).join(QLatin1Char('\n')) + QStringLiteral("\nids: ") + state(on).join(QLatin1Char('\n'))));
                    QVERIFY2(on.tablePlaces() == off.tablePlaces(), qPrintable(what + QStringLiteral(": ") + off.tablePlaces().join(QLatin1Char(';')) + QStringLiteral(" vs ") + on.tablePlaces().join(QLatin1Char(';'))));
                }
                // and worked out from nothing, the same
                for (Book *b : {&off, &on}) {
                    b->calc->invalidateAll();
                    b->finish();
                }
                QVERIFY2(state(on) == state(off), qPrintable(QStringLiteral("totals %1 seed %2 at the end").arg(totals).arg(seed)));
            }
        }
    }

    // A word of a formula that could be read as a cell past XFD is what keeps a text in places; a function whose name looks like one (ATAN2) is none, and neither is any other of the functions there are.
    void noFunctionNameKeepsATextInPlaces()
    {
        QVERIFY(nameLooksWide(QStringLiteral("ATAN2")));
        QVERIFY(Calculation::textHasWideLookingWord(QStringLiteral("A1+ABCD1")));
        QVERIFY(!Calculation::textHasWideLookingWord(QStringLiteral("ATAN2(A1,2)")));
        QVERIFY(!Calculation::textHasWideLookingWord(QStringLiteral("\"ABCD1\"&Sheet2!A1")));
        for (int i = 0; i < jp::calc::functionCount(); ++i) {
            const QString name = QString::fromLatin1(jp::calc::functionAt(i).name);
            QVERIFY2(!Calculation::textHasWideLookingWord(name + QStringLiteral("(A1)")), qPrintable(name));
        }
        Book b;
        b.idFrame = true;
        QVERIFY(b.open({{"A1=1", "A2=2"}}));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), nullptr));
        QVERIFY(b.calc->setFormula(0, 5, 1, QStringLiteral("ATAN2(A2,A3)")));
        QCOMPARE(b.calc->positionalFormulas(), qint64(0));
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), nullptr));
        QCOMPARE(b.formula("B7"), QStringLiteral("ATAN2(A3,A4)"));
    }

    // A range written back to front is shown as written until an edit writes it again, and then in order: in the id frame as before it. Typed, loaded from a file, and after Undo.
    void aRangeWrittenBackToFrontIsShownAsBefore()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        QVector<StructEdit> edits = {StructEdit::insertRows(0, 9, 1), StructEdit::insertRows(0, 0, 1), StructEdit::insertRows(0, 5, 2), StructEdit::deleteRows(0, 20, 1), StructEdit::insertColumns(0, 0, 1), StructEdit::insertColumns(0, 5, 1)};
        QStringList seen;
        for (Book *b : {&off, &on}) {
            QVERIFY(b->open({{"A1=1", "B3=1", "B4=2", "C1==SUM(B9:B3)", "C2==SUM($C$9:A3)"}}));
            QVERIFY(b->calc->setFormula(0, 0, 4, QStringLiteral("SUM(B9:B3)+Sheet1!A9")));
            QCOMPARE(b->formula("C1"), QStringLiteral("SUM(B9:B3)"));
            QStringList lines;
            std::vector<Calculation::EditRecord> recs(size_t(edits.size()));
            for (int i = 0; i < edits.size(); ++i) {
                QVERIFY(b->apply(edits[i], &recs[size_t(i)]));
                if (i == 0) QCOMPARE(b->formula("C1"), QStringLiteral("SUM(B9:B3)"));    // (no corner moved: as written)
                if (i == 1) QCOMPARE(b->formula("C2"), QStringLiteral("SUM(B10:B4)"));   // (both did: as written, which is how the rewrite of a text in places leaves the order of a range)
                lines << state(*b);
                lines << QStringLiteral("--");
            }
            for (int i = edits.size() - 1; i >= 0; --i) {
                QVERIFY(b->calc->undoEdit(recs[size_t(i)]));
                b->finish();
                lines << state(*b);
                lines << QStringLiteral("--");
            }
            seen << lines.join(QLatin1Char('\n'));
        }
        QCOMPARE(seen[1], seen[0]);
        QVERIFY(seen[0].contains(QStringLiteral("SUM(B9:B3)")));   // (as written, before the edits and after Undo)
    }

    // A name that could be read as a cell past XFD, while a formula in ids has the id of an inserted column in it (written XFE1): renaming a name leaves that reference alone, and the other name's uses follow.
    void renamingANameDoesNotTouchAnIdInAText()
    {
        Book b;
        b.idFrame = true;
        QVERIFY(b.open({{"A1=1", "A2=2", "B2=5"}}));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), nullptr));   // a column with an id past the grid, now A
        QVERIFY(b.calc->setFormula(0, 0, 3, QStringLiteral("A1+Total")));   // reads cell A1 of it: the text has XFE1 in it
        b.calc->setNameText(-1, QStringLiteral("Total"), QStringLiteral("Sheet1!$C$2"));
        b.calc->setNameText(-1, QStringLiteral("XFE1"), QStringLiteral("Sheet1!$C$1"));
        QVERIFY(!b.calc->sheet(0).wideTexts.isEmpty());
        b.finish();
        QCOMPARE(b.formula("D1"), QStringLiteral("A1+Total"));
        Calculation::SheetRewrite rec;
        b.calc->renameName(-1, QStringLiteral("XFE1"), QStringLiteral("Zed"), &rec);   // (a name of the file that looks like an address of a column the grid has not)
        QCOMPARE(b.formula("D1"), QStringLiteral("A1+Total"));
        b.calc->renameName(-1, QStringLiteral("Total"), QStringLiteral("Grand"), &rec);
        b.finish();
        QCOMPARE(b.formula("D1"), QStringLiteral("A1+Grand"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:5"));
        QVERIFY(b.apply(StructEdit::insertColumns(0, 0, 1), nullptr));
        QCOMPARE(b.formula("E1"), QStringLiteral("B1+Grand"));
    }

    // The key a text of the class Wide or Positional is shared by says its class and nothing of the text's own characters can say another: no two different (text, class) have the same key (the texts of
    // the class Plain have a map of their own, by their characters).
    void theKeyATextIsSharedByIsUnambiguous()
    {
        const QStringList texts = {QString(), QStringLiteral("A1"), QStringLiteral("0|A1"), QStringLiteral("1|A1"), QStringLiteral("2|A1"), QString(QChar(1)) + QStringLiteral("A1"), QString(QChar(2)) + QStringLiteral("A1"),
                                   QString(QChar(1)) + QString(QChar(2)) + QStringLiteral("A1"), QStringLiteral("|"), QStringLiteral("0|")};
        const Calculation::TextClass classes[2] = {Calculation::TextClass::Wide, Calculation::TextClass::Positional};
        QSet<QString> keys;
        int n = 0;
        for (const QString &t : texts)
            for (const Calculation::TextClass c : classes) {
                keys.insert(Calculation::internKey(t, c));
                ++n;
            }
        QCOMPARE(keys.size(), n);
    }

    // Two workbooks, one with the formulas in places and one in ids, are given the same 200 random edits (rows and columns inserted and deleted on two sheets, formulas of every kind of reference typed,
    // Undo and Redo), and after each the text and the value of every formula are the same.
    void twoHundredRandomEditsGiveTheSameFormulasInBothModes()
    {
        const QStringList sheet1 = {"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "A6=6", "B1==A1+1", "B2==SUM(A1:A4)", "B3==$A$3*2", "B4==SUM(A:A)", "C1==SUM(1:2)", "C2==Sheet2!A2+Sheet2!B1", "C3==SUM(B2:B5)", "D1==A6-A1"};
        const QStringList sheet2 = {"A1=7", "A2=8", "A3=9", "B1==Sheet1!A3+A1", "B2==SUM(Sheet1!A1:A5)", "B3==SUM(A:A)"};
        for (const quint32 seed : {7u, 21u, 1234u}) {
            Book off, on;
            on.idFrame = true;
            off.forceOff = true;
            for (Book *b : {&off, &on}) QVERIFY(b->open({sheet1, sheet2}));
            QRandomGenerator rng(seed);
            QStringList history;
            std::vector<Calculation::EditRecord> done[2];
            std::vector<Calculation::EditRecord> undone[2];
            for (int step = 0; step < 200; ++step) {
                const int kind = int(rng.bounded(12));
                const int sheet = int(rng.bounded(2));
                const quint32 at = rng.bounded(10), count = 1 + rng.bounded(2);
                const quint32 first = 1 + rng.bounded(6), second = first + 1 + rng.bounded(4);
                const QString plusCell = rng.bounded(2) ? QStringLiteral("+B%1").arg(1 + rng.bounded(7)) : QString();
                const QString plusSheet = rng.bounded(2) ? QStringLiteral("+Sheet%1!$A$%2").arg(1 + rng.bounded(2)).arg(1 + rng.bounded(6)) : QString();
                const QString plusColumn = rng.bounded(3) == 0 ? QStringLiteral("+SUM(B:B)") : QString();
                // (the ranges here are written front to back; one written back to front, and the names that look like cells, are the test below's)
                const QString typed = QStringLiteral("SUM(A%1:A%2)").arg(first).arg(second) + plusCell + plusSheet + plusColumn;
                const quint32 row = rng.bounded(12), col = 7 + rng.bounded(3);   // (typed away from the columns read, so that no formula reads itself: what a circle gives depends on where the work began)
                const double number = double(rng.bounded(9));
                history << QStringLiteral("%1: kind %2 sheet %3 at %4 count %5 row %6 col %7 typed %8").arg(step).arg(kind).arg(sheet).arg(at).arg(count).arg(row).arg(col).arg(typed);
                bool ok[2];
                for (int m = 0; m < 2; ++m) {
                    Book &b = m ? on : off;
                    Calculation::EditRecord rec;
                    ok[m] = true;
                    switch (kind) {
                    case 0: case 1: ok[m] = b.apply(StructEdit::insertRows(sheet, at, count), &rec); break;
                    case 2: ok[m] = b.apply(StructEdit::deleteRows(sheet, at, count), &rec); break;
                    case 3: case 4: ok[m] = b.apply(StructEdit::insertColumns(sheet, at % 6, count), &rec); break;
                    case 5: ok[m] = b.apply(StructEdit::deleteColumns(sheet, at % 6, count), &rec); break;
                    case 6: case 7: case 8: ok[m] = b.calc->setFormula(sheet, row, col, typed); b.finish(); rec.edit.kind = StructEdit::Kind::Offset; break;
                    case 9: if (!done[m].empty() && done[m].back().edit.kind != StructEdit::Kind::Offset) { ok[m] = b.calc->undoEdit(done[m].back()); b.finish(); undone[m].push_back(done[m].back()); done[m].pop_back(); } rec.edit.kind = StructEdit::Kind::Offset; break;
                    case 10: if (!undone[m].empty()) { Calculation::EditRecord again = undone[m].back(); ok[m] = b.calc->applyEdit(again.edit, &again); b.finish(); done[m].push_back(again); undone[m].pop_back(); } rec.edit.kind = StructEdit::Kind::Offset; break;
                    default: b.calc->setValue(sheet, row, col, jp::calc::Value::number(number)); b.finish(); rec.edit.kind = StructEdit::Kind::Offset; break;
                    }
                    if (kind < 6 && ok[m]) { done[m].push_back(rec); undone[m].clear(); }
                    // (a cell written outside the steps that are undone is no step: what was done before it can't be undone, as the window's history has it)
                    if (kind >= 6 && kind != 9 && kind != 10) { done[m].clear(); undone[m].clear(); }
                }
                QCOMPARE(ok[1], ok[0]);
                const QStringList a = state(off), c = state(on);
                QString why;
                for (const QString &x : a)
                    if (!c.contains(x)) why += QStringLiteral(" [places: ") + x + QLatin1Char(']');
                for (const QString &x : c)
                    if (!a.contains(x)) why += QStringLiteral(" [ids: ") + x + QLatin1Char(']');
                QVERIFY2(a == c, qPrintable(QStringLiteral("seed %1, step %2 (kind %3): the formulas differ:%4").arg(seed).arg(step).arg(kind).arg(history.mid(std::max(0, int(history.size()) - 40)).join(QLatin1Char('\n')) + QLatin1Char('\n') + why.left(600) + QStringLiteral(" [sheet %1 at %2 count %3 row %4 col %5 typed %6]").arg(sheet).arg(at).arg(count).arg(row).arg(col).arg(typed))));
            }
        }
    }
    // ---- the differential fuzz: the same edits, typed formulas, names, Undo and Redo, and rebases on a book with the formulas in places and one in ids; the text, the value, and the names must be equal
public:
    struct Act {
        int kind = 0;   // 0 structure, 1 formula, 2 name
        Calculation::EditRecord rec;
        int sheet = 0; quint32 row = 0, col = 0;
        QString before, after; bool hadBefore = false;
        Calculation::CellContent contentBefore, contentAfter;   // (a typed formula: as the session keeps it for Undo, with the formula as it was held)
        QString name;
        struct Ch { int sheet; quint32 row, col; Calculation::CellContent before, after; };
        std::vector<Ch> cells;   // kind 3: cells given other contents (a sort, a fill, a paste), as the session's step holds them
    };
    // The values of the cells that are not formulas, and the formats: a sort, a fill, a paste, and a move change these, which the formulas' state does not show.
    static QStringList valuesOf(const Book &b)
    {
        QStringList out;
        for (int sheet = 0; sheet < 2; ++sheet)
            for (quint32 r = 0; r < 44; ++r)
                for (quint32 c = 0; c < 12; ++c) {
                    const Calculation::CellContent cc = b.calc->content(sheet, r, c);
                    if (!cc.exists || cc.isFormula) continue;
                    out << QStringLiteral("v%1!%2,%3 -> %4 style %5").arg(sheet).arg(r).arg(c).arg(b.calc->cell(sheet, r, c).debug()).arg(cc.style);
                }
        return out;
    }
    // The cells of a sort as the session makes them (apps/crunch/core/editsort.cpp): the contents of the rows in the order of the keys in column 0, a formula written as if copied to its new row.
    static std::vector<Act::Ch> sortChanges(const Book &b, int sheet, quint32 r1, quint32 r2, quint32 c2, bool descending)
    {
        const size_t n = size_t(r2 - r1) + 1;
        std::vector<double> key(n);
        std::vector<char> has(n);
        for (size_t i = 0; i < n; ++i) {
            const QString shown = b.calc->cell(sheet, r1 + quint32(i), 0).debug();
            // (a number typed there, not a formula's: a column deleted can bring a formula into the key column, and one that reads itself has the value of whichever way the work went)
            has[i] = shown.startsWith(QLatin1String("n:")) && !b.calc->content(sheet, r1 + quint32(i), 0).isFormula ? 1 : 0;
            key[i] = has[i] ? shown.mid(2).toDouble() : 0;
        }
        std::vector<size_t> order(n);
        std::iota(order.begin(), order.end(), size_t(0));
        std::stable_sort(order.begin(), order.end(), [&](size_t x, size_t y) {
            if (has[x] != has[y]) return has[x] != 0;   // (blanks last)
            return descending ? key[x] > key[y] : key[x] < key[y];
        });
        std::vector<Act::Ch> out;
        for (size_t i = 0; i < n; ++i) {
            const size_t from = order[i];
            if (from == i) continue;
            for (quint32 c = 0; c <= c2; ++c) {
                Act::Ch ch;
                ch.sheet = sheet;
                ch.row = r1 + quint32(i);
                ch.col = c;
                ch.before = b.calc->content(sheet, ch.row, c);
                const Calculation::CellContent src = b.calc->content(sheet, r1 + quint32(from), c);
                ch.after = src;
                if (src.exists && src.isFormula) {
                    ch.after.formula = offsetFormula(src.formula, qint32(qint64(i) - qint64(from)), 0);
                    ch.after.hasStored = false;
                    ch.after.stored = jp::calc::Value();
                    ch.after.copied = src.hasHeld;
                }
                out.push_back(std::move(ch));
            }
        }
        return out;
    }
    // A block copied to another place (a paste, or a fill when it is one cell and the target is a strip): the formulas are written for the cell they land in.
    static std::vector<Act::Ch> copyChanges(const Book &b, int sheet, quint32 r1, quint32 c1, quint32 rows, quint32 cols, int toSheet, quint32 tr, quint32 tc, quint32 tileRows, quint32 tileCols)
    {
        std::vector<Act::Ch> out;
        for (quint32 tr_ = 0; tr_ < tileRows; ++tr_)
            for (quint32 tc_ = 0; tc_ < tileCols; ++tc_)
                for (quint32 dr = 0; dr < rows; ++dr)
                    for (quint32 dc = 0; dc < cols; ++dc) {
                        Act::Ch ch;
                        ch.sheet = toSheet;
                        ch.row = tr + tr_ * rows + dr;
                        ch.col = tc + tc_ * cols + dc;
                        if (ch.row >= 200 || ch.col >= 12) continue;
                        ch.before = b.calc->content(toSheet, ch.row, ch.col);
                        ch.after = b.calc->content(sheet, r1 + dr, c1 + dc);
                        if (ch.after.exists && ch.after.isFormula) {
                            ch.after.formula = offsetFormula(ch.after.formula, qint32(qint64(ch.row) - qint64(r1 + dr)), qint32(qint64(ch.col) - qint64(c1 + dc)));
                            ch.after.hasStored = false;
                            ch.after.stored = jp::calc::Value();
                            ch.after.copied = ch.after.hasHeld;
                        }
                        if ((ch.before.exists && !ch.before.editable) || (ch.after.exists && !ch.after.editable)) return {};   // (an array or a data table is refused)
                        out.push_back(std::move(ch));
                    }
        return out;
    }
    static void putCells(Book &b, const std::vector<Act::Ch> &cells, bool after)
    {
        for (size_t k = 0; k < cells.size(); ++k) {
            const Act::Ch &ch = cells[after ? k : cells.size() - 1 - k];
            b.calc->put(ch.sheet, ch.row, ch.col, after ? ch.after : ch.before);
        }
        b.finish();
    }
    static QStringList namesOf(const Book &b)
    {
        QStringList out;
        for (const char *n : {"N1", "N2", "N3", "N4"}) {
            QString t;
            const bool has = b.calc->nameText(-1, QString::fromLatin1(n), &t);
            out << QStringLiteral("%1=%2:%3").arg(QString::fromLatin1(n), has ? QStringLiteral("y") : QStringLiteral("n"), t);
        }
        return out;
    }

public:
    struct Op {
        int kind = 0, sheet = 0;
        quint32 at = 0, count = 1, row = 0, col = 7, bcol2 = 0;
        quint32 h = 1, w = 1, tr = 0, tc = 0, tiles = 1;   // (kinds 20 and up: the block's rows and columns, where it goes, and how many times a fill or paste repeats it)
        int toSheet = 0;
        bool desc = false;
        QString typed, nm, nameTxt;
        QString str() const { return QStringLiteral("kind %1 sheet %2 at %3 count %4 typed %5 row %6 col %7 name %8=%9 bcol2 %10 h %11 w %12 to %13:%14,%15 tiles %16 desc %17").arg(kind).arg(sheet).arg(at).arg(count).arg(typed).arg(row).arg(col).arg(nm, nameTxt).arg(bcol2).arg(h).arg(w).arg(toSheet).arg(tr).arg(tc).arg(tiles).arg(desc); }
    };
    // runs the ops on a places book and an ids book; returns the report of the first divergence ("" if none) and the step
    static QString runOps(const std::vector<Op> &ops, int *failStep, bool oneWay = false)
    {
        QStringList s1, s2;
        for (int r = 1; r <= 16; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r);
        s1 << "B1==SUM(A1:A10)" << "B2==SUM(A3:A5)" << "B3==A2" << "B4==SUM(N1)" << "B5==N2*2" << "B6==INDEX(N1,2)" << "B7==SUM(N3)" << "B8==ROWS(A1:A10)" << "B9==COUNT(A8:A12)" << "B10==SUM(A:A)" << "B11==SUM(N4)";
        s1 << "C1==COLUMNS(A1:C1)" << "C2==SUM(A2:C4)";
        s2 << "A1==SUM(Sheet1!A1:A10)" << "A2==SUM(Sheet1!N1)" << "A3==INDEX(Sheet1!A1:A10,3)" << "A4=5" << "A5=6" << "A6=7";
        Book off, on;
        on.idFrame = true;
        off.forceOff = true;
        for (Book *b : {&off, &on}) {
            if (!b->open({s1, s2}, QString(), QString())) return QStringLiteral("open");
            b->calc->setNameText(-1, QStringLiteral("N1"), QStringLiteral("Sheet1!$A$3:$A$9"));
            b->calc->setNameText(-1, QStringLiteral("N2"), QStringLiteral("Sheet1!$A$2"));
            b->calc->setNameText(-1, QStringLiteral("N3"), QStringLiteral("Sheet1!A2:A4"));
            b->calc->setNameText(-1, QStringLiteral("N4"), QStringLiteral("Sheet2!$A$4:$A$6"));
            b->finish();
        }
        std::vector<Act> done[2], undone[2];
        for (size_t step = 0; step < ops.size(); ++step) {
            const Op &o = ops[step];
            CellRange blk;
            blk.valid = true; blk.row1 = o.at; blk.row2 = o.at + o.count - 1; blk.col1 = 0; blk.col2 = o.bcol2;
            bool ok[2] = {true, true};
            for (int m = 0; m < 2; ++m) {
                Book &b = m ? on : off;
                Act act;
                act.sheet = o.sheet; act.row = o.row; act.col = o.col; act.name = o.nm;
                int pushed = 0;
                switch (o.kind) {
                case 0: case 1: ok[m] = b.apply(StructEdit::insertRows(o.sheet, o.at, o.count), &act.rec); pushed = ok[m]; break;
                case 2: case 3: ok[m] = b.apply(StructEdit::deleteRows(o.sheet, o.at, o.count), &act.rec); pushed = ok[m]; break;
                case 4: ok[m] = b.apply(StructEdit::insertColumns(o.sheet, o.at % 6, 1 + o.count % 2), &act.rec); pushed = ok[m]; break;
                case 5: ok[m] = b.apply(StructEdit::deleteColumns(o.sheet, o.at % 6, 1 + o.count % 2), &act.rec); pushed = ok[m]; break;
                case 6: ok[m] = b.apply(StructEdit::insertCellsDown(o.sheet, blk), &act.rec); pushed = ok[m]; break;
                case 7: ok[m] = b.apply(StructEdit::deleteCellsUp(o.sheet, blk), &act.rec); pushed = ok[m]; break;
                case 8: case 9: {
                    act.kind = 1;
                    const Calculation::CellContent cc = b.calc->content(o.sheet, o.row, o.col);
                    act.hadBefore = cc.exists; act.before = cc.formula; act.after = o.typed;
                    act.contentBefore = cc;
                    act.contentAfter.exists = true;
                    act.contentAfter.isFormula = true;
                    act.contentAfter.formula = o.typed;
                    ok[m] = b.calc->setFormula(o.sheet, o.row, o.col, o.typed); b.finish(); pushed = 1; break;
                }
                case 10: case 11: {
                    act.kind = 2;
                    QString t;
                    act.hadBefore = b.calc->nameText(-1, o.nm, &t);
                    { bool h = false; act.before = b.calc->nameTagged(-1, o.nm, &h); }   // (as the session keeps it: held, with the epochs)
                    b.calc->setNameText(-1, o.nm, o.nameTxt); b.finish();
                    { bool h = false; act.after = b.calc->nameTagged(-1, o.nm, &h); }
                    pushed = 1; break;
                }
                case 20: {   // a sort of rows at..at+count in the columns up to bcol2 by the first column
                    act.kind = 3;
                    act.cells = sortChanges(b, o.sheet, o.at, o.at + o.count, o.bcol2 + 1, o.desc);
                    putCells(b, act.cells, true);
                    pushed = !act.cells.empty();
                    break;
                }
                case 21: {   // a fill: the cell at (row, col) down or right, o.tiles cells
                    act.kind = 3;
                    act.cells = copyChanges(b, o.sheet, o.row, o.col, 1, 1, o.sheet, o.desc ? o.row : o.row + 1, o.desc ? o.col + 1 : o.col, o.desc ? 1 : o.tiles, o.desc ? o.tiles : 1);
                    ok[m] = !act.cells.empty();
                    if (ok[m]) putCells(b, act.cells, true);
                    pushed = ok[m];
                    break;
                }
                case 22: {   // a paste of the block h by w at (row, col) to (tr, tc) on toSheet, o.tiles times down
                    act.kind = 3;
                    act.cells = copyChanges(b, o.sheet, o.row, o.col % 6, o.h, o.w, o.toSheet, o.tr, o.tc, o.tiles, 1);
                    ok[m] = !act.cells.empty();
                    if (ok[m]) putCells(b, act.cells, true);
                    pushed = ok[m];
                    break;
                }
                case 23: {   // cut and paste (a move) of the block h by w at (row, col) to (tr, tc) on toSheet
                    CellRange blk2;
                    blk2.valid = true; blk2.row1 = o.row; blk2.row2 = o.row + o.h - 1; blk2.col1 = o.col % 6; blk2.col2 = o.col % 6 + o.w - 1;
                    ok[m] = b.apply(StructEdit::move(o.sheet, blk2, o.toSheet, o.tr, o.tc), &act.rec);
                    pushed = ok[m];
                    break;
                }
                case 24: case 25: {   // cells shifted right or left
                    CellRange blk2;
                    blk2.valid = true; blk2.row1 = o.at; blk2.row2 = o.at + o.count - 1; blk2.col1 = 1 + o.bcol2; blk2.col2 = 1 + o.bcol2 + o.w - 1;
                    ok[m] = o.kind == 24 ? b.apply(StructEdit::insertCellsRight(o.sheet, blk2), &act.rec) : b.apply(StructEdit::deleteCellsLeft(o.sheet, blk2), &act.rec);
                    pushed = ok[m];
                    break;
                }
                case 12: case 13: {
                    if (!done[m].empty()) {
                        Act &top = done[m].back();
                        if (top.kind == 3) putCells(b, top.cells, false);
                        else if (top.kind == 0) ok[m] = b.calc->undoEdit(top.rec);
                        else if (top.kind == 1) { if (top.hadBefore) b.calc->put(top.sheet, top.row, top.col, top.contentBefore); else b.calc->clearCell(top.sheet, top.row, top.col); }
                        else b.calc->setNameTagged(-1, top.name, top.hadBefore, top.before);
                        b.finish();
                        undone[m].push_back(std::move(top)); done[m].pop_back();
                    }
                    break;
                }
                case 14: case 15: {
                    if (!undone[m].empty()) {
                        Act top = std::move(undone[m].back()); undone[m].pop_back();
                        if (top.kind == 3) putCells(b, top.cells, true);
                        else if (top.kind == 0) { Calculation::EditRecord again = top.rec; ok[m] = b.calc->applyEdit(again.edit, &again); top.rec = again; }
                        else if (top.kind == 1) b.calc->put(top.sheet, top.row, top.col, top.contentAfter);
                        else b.calc->setNameTagged(-1, top.name, true, top.after);
                        b.finish();
                        done[m].push_back(std::move(top));
                    }
                    break;
                }
                default: {
                    if (m == 1) { b.calc->compactLines(o.sheet, o.at % 2 != 0); b.finish(); }
                    done[m].clear(); undone[m].clear();
                    break;
                }
                }
                if (pushed && ok[m]) { done[m].push_back(std::move(act)); undone[m].clear(); }
            }
            if (qEnvironmentVariableIntValue("JO_FUZZ_FULL")) { off.calc->invalidateAll(); off.finish(); on.calc->invalidateAll(); on.finish(); }
            QStringList a = state(off), c = state(on);
            a << valuesOf(off); c << valuesOf(on);
            if (qEnvironmentVariableIsSet("JO_FUZZ_DUMP")) { static QFile rawDump(qEnvironmentVariable("JO_FUZZ_DUMP")); if (!rawDump.isOpen()) rawDump.open(QIODevice::WriteOnly | QIODevice::Truncate); rawDump.write(QStringLiteral("STEP %1 ok%2\n%3\n").arg(step).arg(ok[0]).arg(a.join(QLatin1Char('\n'))).toUtf8()); rawDump.flush(); }
            {
                auto split = [](const QString &x, QString *k, QString *v) { const int i = int(x.lastIndexOf(QStringLiteral(" -> "))); *k = x.left(i); *v = x.mid(i + 4); };
                QHash<QString, QString> av, cv;
                for (const QString &x : a) { QString k, v; split(x, &k, &v); av.insert(k, v); }
                for (const QString &x : c) { QString k, v; split(x, &k, &v); cv.insert(k, v); }
                for (QString &x : a) { QString k, v; split(x, &k, &v); if (cv.value(k) == QStringLiteral("?") || v == QStringLiteral("?")) x = k + QStringLiteral(" -> ?"); }
                for (QString &x : c) { QString k, v; split(x, &k, &v); if (av.value(k) == QStringLiteral("?") || v == QStringLiteral("?")) x = k + QStringLiteral(" -> ?"); }
            }
            a << namesOf(off); c << namesOf(on);
            if (ok[0] != ok[1] || a != c) {
                QString why;
                for (const QString &x : a) if (!c.contains(x)) why += QStringLiteral("\n  [places] ") + x;
                for (const QString &x : c) if (!a.contains(x)) why += QStringLiteral("\n  [ids]    ") + x;
                *failStep = int(step);
                return QStringLiteral("ok %1/%2%3").arg(ok[0]).arg(ok[1]).arg(why.left(1200));
            }
            // What both books show is what a whole new calculation of the same cells shows (a stale value in both would not tell in the comparison): at the end, and every JO_FUZZ_TRUTH steps
            const int every = qEnvironmentVariableIntValue("JO_FUZZ_TRUTH");
            if (step + 1 == ops.size() || (every > 0 && int(step) % every == every - 1)) {
                for (int m = 0; m < 2; ++m) {
                    Book &b = m ? on : off;
                    const QStringList now = state(b);
                    b.calc->invalidateAll();
                    b.finish();
                    const QStringList fresh = state(b);
                    if (now != fresh) {
                        QString why;
                        for (const QString &x : now) if (!fresh.contains(x)) why += QStringLiteral("\n  [%1 incremental] ").arg(m ? "ids" : "places") + x;
                        for (const QString &x : fresh) if (!now.contains(x)) why += QStringLiteral("\n  [%1 whole calculation] ").arg(m ? "ids" : "places") + x;
                        *failStep = int(step);
                        return QStringLiteral("stale%1").arg(why.left(1200));
                    }
                }
            }
        }
        (void)oneWay;
        return QString();
    }
    static std::vector<Op> makeOps(int seed, int n, bool compact)
    {
        QRandomGenerator rng(quint32(seed) * 7919u + 13u);
        QRandomGenerator moves(quint32(seed) * 104729u + 7u);
        std::vector<Op> ops;
        for (int step = 0; step < n; ++step) {
            Op o;
            o.kind = int(rng.bounded(compact ? 17 : 16));
            if (qEnvironmentVariableIntValue("JO_FUZZ_NOUNDO") && o.kind >= 12 && o.kind <= 15) o.kind = 3;
            if (qEnvironmentVariableIntValue("JO_FUZZ_NONAMES") && (o.kind == 10 || o.kind == 11)) o.kind = 2;
            o.sheet = int(rng.bounded(2));
            o.at = rng.bounded(14);
            o.count = 1 + rng.bounded(4);
            quint32 ra = 1 + rng.bounded(14), c2 = 1 + rng.bounded(14);
            const bool back = rng.bounded(5) == 0 && !qEnvironmentVariableIntValue("JO_FUZZ_NOBACK");
            if (!back && c2 < ra) std::swap(ra, c2);
            const int sh = 1 + int(rng.bounded(2));
            o.typed = QStringLiteral("SUM(Sheet%1!A%2:A%3)").arg(sh).arg(ra).arg(c2) + (rng.bounded(3) == 0 ? QStringLiteral("+N1") : QString());
            o.row = rng.bounded(14);
            o.col = 7 + rng.bounded(3);
            o.nm = QStringLiteral("N%1").arg(1 + rng.bounded(4));
            const QStringList texts = {"Sheet1!$A$3:$A$9", "Sheet1!$A$2", "Sheet1!$A$5:$A$12", "Sheet2!$A$1:$A$3", "Sheet1!A2:A4", "Sheet1!$B$1:$B$4"};
            o.nameTxt = texts[int(rng.bounded(quint32(texts.size())))];
            o.bcol2 = rng.bounded(2);
            // Sorts, fills, pastes, cuts, and cells shifted sideways (JO_FUZZ_NOMOVES leaves them out): drawn from a generator of their own, so that the steps above are the seed's as they were.
            if (!qEnvironmentVariableIntValue("JO_FUZZ_NOMOVES") && moves.bounded(3) == 0) {
                o.kind = 20 + int(moves.bounded(6));   // 20 sort, 21 fill, 22 paste, 23 cut, 24 and 25 cells shifted right and left
                o.sheet = int(moves.bounded(2));
                o.toSheet = int(moves.bounded(2));
                o.at = moves.bounded(12);
                o.count = 1 + moves.bounded(6);
                o.row = moves.bounded(14);
                o.col = moves.bounded(10);
                o.h = 1 + moves.bounded(3);
                o.w = 1 + moves.bounded(2);
                o.tr = moves.bounded(30);
                o.tc = 1 + moves.bounded(9);   // (never in the first column, the sort's key: a formula there would read itself, and what a circle gives depends on where the work began)
                o.tiles = 1 + moves.bounded(4);
                o.desc = moves.bounded(2) != 0;
                o.bcol2 = moves.bounded(3);
            }
            ops.push_back(o);
        }
        return ops;
    }
private Q_SLOTS:
    // The same edits to a book in places and a book in ids: 24 seeds of 60 steps (JO_FUZZ_SEEDS, JO_FUZZ_STEPS, JO_FUZZ_FIRST for the long run; JO_FUZZ_NOBACK, JO_FUZZ_NONAMES, JO_FUZZ_FULL to narrow it down).
    void differentialFuzzOfPlacesAndIds()
    {
        const int seeds = qEnvironmentVariableIntValue("JO_FUZZ_SEEDS") ? qEnvironmentVariableIntValue("JO_FUZZ_SEEDS") : 24;
        const int first = qEnvironmentVariableIntValue("JO_FUZZ_FIRST");
        const bool compact = qEnvironmentVariableIsSet("JO_FUZZ_COMPACT") ? qEnvironmentVariableIntValue("JO_FUZZ_COMPACT") != 0 : true;   // (rebases are in the steps)
        const int steps = qEnvironmentVariableIntValue("JO_FUZZ_STEPS") ? qEnvironmentVariableIntValue("JO_FUZZ_STEPS") : 60;
        int fails = 0;
        QString report;
        for (int seed = first; seed < first + seeds && fails < 4; ++seed) {
            std::vector<Op> ops = makeOps(seed, steps, compact);
            int at = -1;
            QString why = runOps(ops, &at);
            if (why.isEmpty()) continue;
            ++fails;
            ops.resize(size_t(at + 1));
            // delta-debug: drop chunks while some divergence remains
            for (size_t chunk = ops.size() / 2; chunk >= 1;) {
                bool shrunk = false;
                for (size_t i = 0; i + chunk <= ops.size();) {
                    std::vector<Op> t = ops;
                    t.erase(t.begin() + qint64(i), t.begin() + qint64(i + chunk));
                    int a2 = -1;
                    const QString w2 = runOps(t, &a2);
                    if (!w2.isEmpty()) { t.resize(size_t(a2 + 1)); ops = t; why = w2; shrunk = true; }
                    else i += chunk;
                }
                if (!shrunk) chunk /= 2;
                if (chunk == 0) break;
            }
            report += QStringLiteral("\nSEED %1 minimal (%2 ops):\n").arg(seed).arg(ops.size());
            for (const Op &o : ops) report += QStringLiteral("  ") + o.str() + QLatin1Char('\n');
            report += why;
        }
        QVERIFY2(fails == 0, qPrintable(report));
    }


    // A formula filled down, sorted, or pasted in the id frame is held as the text of the cell it came from with its distance changed, when every line it reads is the line the text would give: no text is read or
    // made, and the cells share one text. Where a run of rows ends between the cell and what it reads, the text goes the long way. Either way the cells are what the book in places has.
    void aFilledColumnIsHeldAsOneTextWhereTheRowsAreInOneRun()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        QStringList s1;
        for (int r = 1; r <= 40; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r % 7) << QStringLiteral("B%1=%2").arg(r).arg(r % 5);
        s1 << "D1==A1+A3*B2-$A$1";
        for (Book *b : {&off, &on}) QVERIFY(b->open({s1}));
        // rows put in and taken out first, so that the ids are in runs
        for (Book *b : {&off, &on}) {
            QVERIFY(b->apply(StructEdit::insertRows(0, 10, 2), nullptr));
            QVERIFY(b->apply(StructEdit::deleteRows(0, 25, 1), nullptr));
        }
        const qint64 sharedTextsBefore = on.calc->sheet(0).formulaTexts.size();
        const Calculation::EditStats before = on.calc->editStats();
        const qint64 rebasedBefore = on.calc->formulasRebased();
        std::vector<Act::Ch> cells[2];
        for (int m = 0; m < 2; ++m) {
            Book &b = m ? on : off;
            cells[m] = copyChanges(b, 0, 0, 3, 1, 1, 0, 1, 3, 30, 1);   // D1 down 30 rows
            QCOMPARE(cells[m].size(), size_t(30));
            putCells(b, cells[m], true);
        }
        const Calculation::EditStats did = on.calc->editStats().since(before);
        QCOMPARE(state(on), state(off));
        // a run of rows ends at the rows put in and at the one taken out: some of the 30 copies are rebased and the others were turned into ids from their text
        QVERIFY2((on.calc->formulasRebased() - rebasedBefore) > 0 && (on.calc->formulasRebased() - rebasedBefore) < 30, qPrintable(QString::number((on.calc->formulasRebased() - rebasedBefore))));
        QCOMPARE((on.calc->formulasRebased() - rebasedBefore) + did.textsToIds, qint64(30));
        // each text is held once for all its cells: the copies that were rebased added no text
        QVERIFY2(on.calc->sheet(0).formulaTexts.size() - sharedTextsBefore <= 1 + (30 - (on.calc->formulasRebased() - rebasedBefore)), qPrintable(QString::number(on.calc->sheet(0).formulaTexts.size() - sharedTextsBefore)));
        for (Book *b : {&off, &on}) {
            b->calc->invalidateAll();
            b->finish();
        }
        QCOMPARE(state(on), state(off));
        // Undo puts the cells back as they were
        for (Book *b : {&off, &on}) putCells(*b, b == &on ? cells[1] : cells[0], false);
        QCOMPARE(state(on), state(off));
    }

    // With the rows in one run (nothing put in or taken out) every copy is rebased: no text is turned into ids, and the 100 copies share one text.
    void aFillDownInOneRunReadsAndMakesNoText()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        QStringList s1;
        for (int r = 1; r <= 120; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(r % 7);
        s1 << "D1==A1+A3-$A$1+SUM(A1:A2)";
        for (Book *b : {&off, &on}) QVERIFY(b->open({s1}));
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::insertRows(0, 0, 1), nullptr));   // (not the file's order any more: the ids are 1 run and one more)
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::deleteRows(0, 0, 1), nullptr));
        const qint64 textsBefore = on.calc->sheet(0).formulaTexts.size();
        const Calculation::EditStats before = on.calc->editStats();
        const qint64 rebasedBefore = on.calc->formulasRebased();
        std::vector<Act::Ch> cells[2];
        for (int m = 0; m < 2; ++m) {
            Book &b = m ? on : off;
            cells[m] = copyChanges(b, 0, 0, 3, 1, 1, 0, 1, 3, 100, 1);
            putCells(b, cells[m], true);
        }
        const Calculation::EditStats did = on.calc->editStats().since(before);
        QCOMPARE((on.calc->formulasRebased() - rebasedBefore), qint64(100));
        QCOMPARE(did.textsToIds, qint64(0));
        QVERIFY2(on.calc->sheet(0).formulaTexts.size() <= textsBefore + 1, qPrintable(QString::number(on.calc->sheet(0).formulaTexts.size())));   // (the text the file read is not found by its characters: one more, and the 100 copies share it)
        QCOMPARE(state(on), state(off));
        // a sort of the same cells is rebased as well, and shows what the book in places shows
        const std::vector<Act::Ch> sorted[2] = {sortChanges(off, 0, 0, 100, 3, true), sortChanges(on, 0, 0, 100, 3, true)};
        QVERIFY(!sorted[1].empty());
        const qint64 rebasedBeforeSort = on.calc->formulasRebased();
        putCells(off, sorted[0], true);
        putCells(on, sorted[1], true);
        QVERIFY(on.calc->formulasRebased() > rebasedBeforeSort);
        QCOMPARE(state(on), state(off));
    }


    // A formula sorted into a row that was put in (its id is past the grid's) reads the row it should, and is shown as the book in places shows it: the text of the grid's numbers is held in the wide class
    // once the distance takes it past the grid.
    void aFormulaSortedIntoARowPutInIsHeldWithTheWideLimits()
    {
        Book on, off;
        on.idFrame = true;
        off.forceOff = true;
        QStringList s1;
        for (int r = 1; r <= 8; ++r) s1 << QStringLiteral("A%1=%2").arg(r).arg(9 - r) << QStringLiteral("B%1==A%1*2+A$1").arg(r);
        for (Book *b : {&off, &on}) QVERIFY(b->open({s1}));
        for (Book *b : {&off, &on}) QVERIFY(b->apply(StructEdit::insertRows(0, 0, 3), nullptr));   // three rows with ids past the grid, above the data
        for (Book *b : {&off, &on}) {
            QVERIFY(b->calc->setValue(0, 0, 0, jp::calc::Value::number(100)));   // (the first of them is the smallest key... or the largest)
            b->finish();
        }
        const qint64 rebasedBeforeWide = on.calc->formulasRebased();
        const std::vector<Act::Ch> changes[2] = {sortChanges(off, 0, 0, 10, 1, false), sortChanges(on, 0, 0, 10, 1, false)};
        QVERIFY(!changes[0].empty());
        putCells(off, changes[0], true);
        putCells(on, changes[1], true);
        QVERIFY(on.calc->formulasRebased() > rebasedBeforeWide);
        QCOMPARE(state(on), state(off));
        QCOMPARE(on.formula("B1"), off.formula("B1"));
        QCOMPARE(on.formula("B2"), off.formula("B2"));
        QCOMPARE(on.at("B3"), off.at("B3"));
        for (Book *b : {&off, &on}) {
            b->calc->invalidateAll();
            b->finish();
        }
        QCOMPARE(state(on), state(off));
    }

};

JP_TEST_CLASS(SheetStructureTests)
JP_TEST_CLASS(SheetIdFrameTests)
#include "tst_sheet_structure.moc"
