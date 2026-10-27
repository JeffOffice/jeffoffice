// Tests of rows, columns, and cells inserted and deleted in a calculation (Calculation::applyEdit): the cells move, every formula that reads cells that moved is written again with its
// references following them (on every sheet, and in the defined names), a reference to deleted cells is #REF!, what depends is worked out again, and Undo puts everything back. Counted
// work only: what moved, and what was written again. Every workbook is built in memory.
#include "sheethelpers.h"

#include "sheet/calculation.h"
#include "sheet/tables.h"

#include <QRandomGenerator>

#include <memory>

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
        const QByteArray zip = buildXlsx(parts, QByteArray(), QString(), QString(), QByteArray(), QStringList(), names);
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.keepApart = true;
        options.workOutWhatHasNoResult = true;
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
        auto held = [&] { return QStringLiteral("%1/%2/%3").arg(b.calc->formulaTextsInUse(0)).arg(b.calc->formulaEntriesInUse(0)).arg(b.calc->programsHeld(0)); };
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

private:
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

JP_TEST_CLASS(SheetStructureTests)
#include "tst_sheet_structure.moc"
