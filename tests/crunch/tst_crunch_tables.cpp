// Tests of tables as a person makes and changes them through the edit session (core/edittables.cpp): making one from cells, turning it back, naming it, the row of names and the row of totals,
// resizing, the table growing by a cell typed next to it, a formula typed in an empty column filling it, structured references in formulas, one step of Undo each, and what a save leaves of
// the file. The workbook is built here.
#include "app/crunchwindow.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "savedok.h"
#include "sheet/structref.h"
#include "sheet/tablestyle.h"

#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

QString text(const QString &ref, const QString &t) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t.toHtmlEscaped()); }
QString num(const QString &ref, double v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref).arg(v); }
QString formula(const QString &ref, const QString &f) { return QStringLiteral("<c r=\"%1\"><f>%2</f><v>0</v></c>").arg(ref, f.toHtmlEscaped()); }
QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

//        A        B     C
//   1  Region    Q1    Q2
//   2  East      1     10
//   3  West      2     20
//   4  North     3     30
//   6  (a formula under: =SUM(B2:B4))
QByteArray salesRows()
{
    return worksheet(row(1, text(QStringLiteral("A1"), QStringLiteral("Region")) + text(QStringLiteral("B1"), QStringLiteral("Q1")) + text(QStringLiteral("C1"), QStringLiteral("Q2")))
                     + row(2, text(QStringLiteral("A2"), QStringLiteral("East")) + num(QStringLiteral("B2"), 1) + num(QStringLiteral("C2"), 10))
                     + row(3, text(QStringLiteral("A3"), QStringLiteral("West")) + num(QStringLiteral("B3"), 2) + num(QStringLiteral("C3"), 20))
                     + row(4, text(QStringLiteral("A4"), QStringLiteral("North")) + num(QStringLiteral("B4"), 3) + num(QStringLiteral("C4"), 30))
                     + row(6, formula(QStringLiteral("B6"), QStringLiteral("SUM(B2:B4)"))));
}

CellRange rangeText(const QString &a1)
{
    CellRange r;
    parseRangeRef(a1, &r);
    return r;
}

quint32 rowOf(const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return r;
}

quint32 colOf(const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return c;
}

QString valueOf(EditWindow &e, const QString &ref, int sheet = 0) { return e.win.w.session()->valueAt(sheet, rowOf(ref), colOf(ref)).debug(); }
QString editOf(EditWindow &e, const QString &ref, int sheet = 0) { return e.win.w.session()->editText(sheet, rowOf(ref), colOf(ref)); }

bool saveNow(EditWindow &e)
{
    if (!e.settle()) return false;
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    return e.win.w.save() && savedOk(saved);
}

struct Saved {
    Package package;
    Workbook book;
    bool open(const QString &path)
    {
        QString why;
        return package.readLazyFile(path, &why) && book.open(package, &why);
    }
    QString formula(int sheet, const QString &ref) const
    {
        Sheet s;
        if (!book.readSheet(sheet, &s)) return QStringLiteral("unreadable");
        const Cell *c = at(s, ref);
        return c ? s.formulaText(*c) : QStringLiteral("-");
    }
};

const QLocale kUs(QLocale::English, QLocale::UnitedStates);

} // namespace

class CrunchTablesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The cells become a table with a name and a style; a header cell that is not text becomes text, an empty one is named, and a repeat is numbered. One step; Undo takes the table and the
    // headers back; the saved file has the table and opens with it.
    void aRangeBecomesATableInOneStep()
    {
        EditWindow e;
        const QByteArray book = buildXlsx({worksheet(row(1, text(QStringLiteral("A1"), QStringLiteral("Item")) + num(QStringLiteral("B1"), 2024) + text(QStringLiteral("D1"), QStringLiteral("Item")))
                                                     + row(2, text(QStringLiteral("A2"), QStringLiteral("Pen")) + num(QStringLiteral("B2"), 3) + num(QStringLiteral("C2"), 4) + num(QStringLiteral("D2"), 5)))});
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        int made = -1;
        const auto out = s->makeTable(0, rangeText(QStringLiteral("A1:D2")), true, QStringLiteral("TableStyleLight9"), &made);
        QVERIFY2(out.ok, qPrintable(out.problem));
        QCOMPARE(u->count(), before + 1);
        QCOMPARE(made, 0);
        const Table &t = s->workbook()->tables().first();
        QCOMPARE(t.name, QStringLiteral("Table1"));
        QCOMPARE(t.styleName, QStringLiteral("TableStyleLight9"));
        QCOMPARE(t.columns, (QStringList{QStringLiteral("Item"), QStringLiteral("2024"), QStringLiteral("Column3"), QStringLiteral("Item2")}));
        QCOMPARE(int(t.row2), 1);
        QCOMPARE(int(t.col2), 3);
        QVERIFY(t.filterButtons && t.showRowStripes);
        QCOMPARE(valueOf(e, QStringLiteral("B1")), QStringLiteral("s:2024"));
        QCOMPARE(valueOf(e, QStringLiteral("C1")), QStringLiteral("s:Column3"));
        QCOMPARE(valueOf(e, QStringLiteral("D1")), QStringLiteral("s:Item2"));
        QCOMPARE(s->tableAt(0, 1, 2), 0);
        QCOMPARE(s->tableAt(0, 2, 2), -1);
        // saved, and opened again
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QCOMPARE(back.book.tables().size(), 1);
        QCOMPARE(back.book.tables().first().columns, t.columns);
        QCOMPARE(back.book.tables().first().styleName, QStringLiteral("TableStyleLight9"));
        // Undo takes it all back
        u->undo();
        QVERIFY(e.settle());
        QVERIFY(s->workbook()->tables().isEmpty());
        QCOMPARE(valueOf(e, QStringLiteral("B1")), QStringLiteral("n:2024"));
        QCOMPARE(valueOf(e, QStringLiteral("C1")), QStringLiteral("-"));
        u->redo();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 1);
        QCOMPARE(valueOf(e, QStringLiteral("C1")), QStringLiteral("s:Column3"));
        // undone after the save: the next save takes the table out of the file again
        u->undo();
        QVERIFY(saveNow(e));
        Saved gone;
        QVERIFY(gone.open(e.path));
        QVERIFY(gone.book.tables().isEmpty());
        QVERIFY(!gone.package.hasPart(QStringLiteral("/xl/tables/table1.xml")));
    }

    // Cells that cannot be a table are refused with the reason: another table's, merged, an array, a protected sheet; and the name is unique.
    void cellsThatCannotBeATableAreRefused()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        const int count = e.win.w.undoStack()->count();
        const auto overlap = s->makeTable(0, rangeText(QStringLiteral("C3:D6")), true, QString());
        QVERIFY(!overlap.ok && overlap.problem.contains(QStringLiteral("another table")));
        QVERIFY(!s->makeTable(0, CellRange(), true, QString()).ok);
        QVERIFY(!s->makeTable(5, rangeText(QStringLiteral("A8:B9")), true, QString()).ok);
        QCOMPARE(e.win.w.undoStack()->count(), count);
        // the second table is Table2, whatever its place
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("E1:F3")), false, QString()).ok);
        QCOMPARE(s->workbook()->tables().last().name, QStringLiteral("Table2"));
        QCOMPARE(s->workbook()->tables().last().headerRows, 0);
        QCOMPARE(s->workbook()->tables().last().columns, (QStringList{QStringLiteral("Column1"), QStringLiteral("Column2")}));
        // a name that is a table's, a defined name's, or not a name is refused
        QVERIFY(!s->tableNameProblem(QStringLiteral("table1"), 1).isEmpty());
        QVERIFY(s->tableNameProblem(QStringLiteral("table1"), 0).isEmpty());
        QVERIFY(!s->tableNameProblem(QStringLiteral("A1"), 1).isEmpty());
        QVERIFY(!s->tableNameProblem(QStringLiteral("two words"), 1).isEmpty());
    }

    // A table renamed: every formula that names it follows, in one step, and the saved file has the new name in the part and in the formulas.
    void renamingATableRenamesItInEveryFormula()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=SUM(Table1[Q1])+SUM(Table1[[#Headers],[Q2]:[Q2]])*0")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        QVERIFY(!s->renameTable(0, QStringLiteral("not ok")).ok);
        QVERIFY(s->renameTable(0, QStringLiteral("Sales")).ok);
        QCOMPARE(u->count(), before + 1);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().name, QStringLiteral("Sales"));
        QCOMPARE(editOf(e, QStringLiteral("A8")), QStringLiteral("=SUM(Sales[Q1])+SUM(Sales[[#Headers],[Q2]:[Q2]])*0"));
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QCOMPARE(back.book.tables().first().name, QStringLiteral("Sales"));
        QCOMPARE(back.formula(0, QStringLiteral("A8")), QStringLiteral("SUM(Sales[Q1])+SUM(Sales[[#Headers],[Q2]:[Q2]])*0"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().name, QStringLiteral("Table1"));
        QCOMPARE(editOf(e, QStringLiteral("A8")), QStringLiteral("=SUM(Table1[Q1])+SUM(Table1[[#Headers],[Q2]:[Q2]])*0"));
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
    }

    // Turned back into cells, the formulas that named the table name its cells, the cells keep what they hold, and Undo brings the table and the formulas back.
    void aTableTurnedBackIntoCellsLeavesFormulasOnCells()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=SUM(Table1[Q2])")).ok);
        QVERIFY(s->type(0, 1, 3, QStringLiteral("=[@Q1]*100")).ok);   // (a formula typed in the column beside: the table grows, and the column is filled)
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().columns.size(), 4);
        QCOMPARE(valueOf(e, QStringLiteral("D3")), QStringLiteral("n:200"));
        QVERIFY(s->tableToRange(0).ok);
        QVERIFY(e.settle());
        QVERIFY(s->workbook()->tables().isEmpty());
        QCOMPARE(editOf(e, QStringLiteral("A8")), QStringLiteral("=SUM($C$2:$C$4)"));
        QCOMPARE(editOf(e, QStringLiteral("D3")), QStringLiteral("=B3*100"));
        QCOMPARE(valueOf(e, QStringLiteral("D3")), QStringLiteral("n:200"));
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:60"));
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QVERIFY(back.book.tables().isEmpty());
        QCOMPARE(back.formula(0, QStringLiteral("A8")), QStringLiteral("SUM($C$2:$C$4)"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 1);
        QCOMPARE(editOf(e, QStringLiteral("A8")), QStringLiteral("=SUM(Table1[Q2])"));
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:60"));
    }

    // The row of totals: on puts a label and a sum, a function chosen for a column is written as the subtotal of its column, a custom formula is kept, off clears the row; and the numbers
    // are right.
    void theTotalsRowShowsTheFunctionOfEachColumn()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        const auto onUnder = s->setTableTotalsRow(0, true);
        QVERIFY(onUnder.ok);
        QVERIFY(e.settle());
        const Table &t = s->workbook()->tables().first();
        QCOMPARE(t.totalsRows, 1);
        QCOMPARE(int(t.row2), 4);
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("s:Total"));
        QCOMPARE(editOf(e, QStringLiteral("C5")), QStringLiteral("=SUBTOTAL(109,Table1[Q2])"));
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("n:60"));
        QVERIFY(s->setTableTotal(0, 1, QStringLiteral("average")).ok);
        QVERIFY(s->setTableTotal(0, 0, QStringLiteral("countNums")).ok);
        QVERIFY(e.settle());
        QCOMPARE(editOf(e, QStringLiteral("B5")), QStringLiteral("=SUBTOTAL(101,Table1[Q1])"));
        QCOMPARE(valueOf(e, QStringLiteral("B5")), QStringLiteral("n:2"));
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("n:0"));
        QVERIFY(s->setTableTotal(0, 2, QStringLiteral("custom"), QStringLiteral("MAX(Table1[Q2])-MIN(Table1[Q2])")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("n:20"));
        QVERIFY(!s->setTableTotal(0, 2, QStringLiteral("nonsense")).ok);
        QVERIFY(!s->setTableTotal(0, 9, QStringLiteral("sum")).ok);
        // the saved file keeps the functions
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        const Table &r = back.book.tables().first();
        QCOMPARE(r.totalsRows, 1);
        QCOMPARE(r.info[1].totalsFunction, QStringLiteral("average"));
        QCOMPARE(r.info[0].totalsFunction, QStringLiteral("countNums"));
        QCOMPARE(r.info[2].totalsFunction, QStringLiteral("custom"));
        QCOMPARE(r.info[2].totalsFormula, QStringLiteral("MAX(Table1[Q2])-MIN(Table1[Q2])"));
        // off clears the row and the part's totals; a function with none is refused
        QVERIFY(s->setTableTotalsRow(0, false).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().totalsRows, 0);
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("-"));
        QVERIFY(!s->setTableTotal(0, 1, QStringLiteral("sum")).ok);
        // a table with something under it can't have a row of totals
        QVERIFY(s->type(0, 4, 1, QStringLiteral("5")).ok);   // (but it grows by typing under it instead)
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
    }

    // Typing in the row under a table, or the column beside it, makes it grow; Undo takes the growth away first and the entry after; a row that grows gets the columns' formulas.
    void aTableGrowsWhenAnEntryIsMadeNextToIt()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 1, 3, QStringLiteral("=[@Q1]+[@Q2]")).ok);   // (a new column that is calculated)
        QVERIFY(e.settle());
        Table t = s->workbook()->tables().first();
        QCOMPARE(t.columns.size(), 4);
        QCOMPARE(t.columns.last(), QStringLiteral("Column4"));
        QCOMPARE(valueOf(e, QStringLiteral("D1")), QStringLiteral("s:Column4"));
        QCOMPARE(t.info[3].calcFormula, QStringLiteral("Table1[[#This Row],[Q1]]+Table1[[#This Row],[Q2]]"));
        QCOMPARE(valueOf(e, QStringLiteral("D2")), QStringLiteral("n:11"));
        QCOMPARE(valueOf(e, QStringLiteral("D4")), QStringLiteral("n:33"));
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        QVERIFY(s->type(0, 4, 0, QStringLiteral("South")).ok);
        QVERIFY(e.settle());
        QCOMPARE(u->count(), before + 2);   // (the entry, then the growth)
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
        QCOMPARE(editOf(e, QStringLiteral("D5")), QStringLiteral("=Table1[[#This Row],[Q1]]+Table1[[#This Row],[Q2]]"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 3);
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("s:South"));   // (the entry stays)
        QCOMPARE(valueOf(e, QStringLiteral("D5")), QStringLiteral("-"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("-"));
        u->redo();
        u->redo();
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
        // a cell two rows under, or with a table's totals row under, grows nothing
        QVERIFY(s->type(0, 6, 0, QStringLiteral("far")).ok);
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
        QCOMPARE(s->growthByTyping(0, 5, 1).table, 0);
        QCOMPARE(s->growthByTyping(0, 6, 1).table, -1);
        QCOMPARE(s->growthByTyping(0, 0, 4).column, true);
        QCOMPARE(s->growthByTyping(1, 5, 1).table, -1);
    }

    // A formula typed in a cell of an empty column fills the column, and can be refused with one Undo that leaves the cell as typed; a formula typed over a calculated column changes the column.
    void aFormulaTypedInAnEmptyColumnFillsIt()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 2, 2, QStringLiteral("=[@Q1]*2")).ok);    // (over the Q2 cells: they are not empty, so only this cell has it)
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().info[2].calcFormula, QString());
        QVERIFY(!s->lastEntryMadeCalculatedColumn());
        QVERIFY(s->type(0, 1, 3, QStringLiteral("=B2+C2")).ok);
        QVERIFY(e.settle());
        QVERIFY(s->lastEntryMadeCalculatedColumn());
        QCOMPARE(editOf(e, QStringLiteral("D3")), QStringLiteral("=B3+C3"));
        QCOMPARE(editOf(e, QStringLiteral("D4")), QStringLiteral("=B4+C4"));
        QCOMPARE(s->workbook()->tables().first().info[3].calcFormula, QStringLiteral("B2+C2"));
        // the choice not to: the cell stays as typed
        QVERIFY(s->undoCalculatedColumn().ok);
        QVERIFY(e.settle());
        QCOMPARE(editOf(e, QStringLiteral("D2")), QStringLiteral("=B2+C2"));
        QCOMPARE(editOf(e, QStringLiteral("D3")), QString());
        QCOMPARE(s->workbook()->tables().first().info[3].calcFormula, QString());
        QVERIFY(!s->undoCalculatedColumn().ok);
        // a column whose cells hold the formula is changed by a new formula typed in one of them
        QVERIFY(s->type(0, 2, 3, QStringLiteral("=B3-C3")).ok);   // (D3 empty and D2 not: an exception, nothing is filled)
        QCOMPARE(s->workbook()->tables().first().info[3].calcFormula, QString());
        QVERIFY(s->type(0, 1, 3, QStringLiteral("")).ok);
        QVERIFY(s->type(0, 2, 3, QStringLiteral("")).ok);
        QVERIFY(s->type(0, 1, 3, QStringLiteral("=C2*3")).ok);
        QVERIFY(e.settle());
        QCOMPARE(editOf(e, QStringLiteral("D4")), QStringLiteral("=C4*3"));
        QVERIFY(s->type(0, 3, 3, QStringLiteral("=C4*4")).ok);
        QVERIFY(e.settle());
        QCOMPARE(editOf(e, QStringLiteral("D2")), QStringLiteral("=C2*4"));
        QCOMPARE(s->workbook()->tables().first().info[3].calcFormula, QStringLiteral("C2*4"));
        // saved and read again, the column's formula is in the part
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QCOMPARE(back.book.tables().first().info[3].calcFormula, QStringLiteral("C2*4"));
        QCOMPARE(back.formula(0, QStringLiteral("D3")), QStringLiteral("C3*4"));
    }

    // The table changed to other cells: columns named and rows filled, the row of totals moved, a size that is not allowed refused; the part follows.
    void aTableIsResized()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 1, 3, QStringLiteral("=B2*2")).ok);
        QVERIFY(e.settle());
        QVERIFY(s->setTableTotalsRow(0, true).ok);
        QVERIFY(e.settle());
        // longer: the new rows are data, they get the column's formula, and the totals go to the last row
        QVERIFY(s->resizeTable(0, rangeText(QStringLiteral("A1:D7"))).ok);
        QVERIFY(e.settle());
        const Table &t = s->workbook()->tables().first();
        QCOMPARE(int(t.row2), 6);
        QCOMPARE(t.totalsRows, 1);
        QCOMPARE(valueOf(e, QStringLiteral("A7")), QStringLiteral("s:Total"));
        QCOMPARE(editOf(e, QStringLiteral("D5")), QStringLiteral("=B5*2"));
        QCOMPARE(editOf(e, QStringLiteral("D6")), QStringLiteral("=B6*2"));
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("-"));   // (the old totals row is a row of data now, empty but for the formula)
        QCOMPARE(editOf(e, QStringLiteral("D7")), QStringLiteral("=SUBTOTAL(109,Table1[Column4])"));
        // wider: a column is named from the cell above it, or Column5
        QVERIFY(s->type(0, 0, 4, QStringLiteral("Notes")).ok);   // (a table with a row of totals does not grow by itself)
        QCOMPARE(s->workbook()->tables().first().columns.last(), QStringLiteral("Column4"));
        QVERIFY(s->resizeTable(0, rangeText(QStringLiteral("A1:F7"))).ok);
        QCOMPARE(s->workbook()->tables().first().columns.last(), QStringLiteral("Column6"));
        QCOMPARE(s->workbook()->tables().first().columns.at(4), QStringLiteral("Notes"));   // (named from the cell above)
        QCOMPARE(valueOf(e, QStringLiteral("F1")), QStringLiteral("s:Column6"));
        // not allowed: another first cell, too few rows, a size past the sheet
        QVERIFY(!s->resizeTable(0, rangeText(QStringLiteral("A2:F7"))).ok);
        QVERIFY(!s->resizeTable(0, rangeText(QStringLiteral("A1:F2"))).ok);
        // shorter and narrower: the columns go from the right, the cells stay where they are
        QVERIFY(s->resizeTable(0, rangeText(QStringLiteral("A1:C4"))).ok);
        QVERIFY(e.settle());
        const Table &small = s->workbook()->tables().first();
        QCOMPARE(small.columns, (QStringList{QStringLiteral("Region"), QStringLiteral("Q1"), QStringLiteral("Q2")}));
        QCOMPARE(int(small.row2), 3);
        QCOMPARE(small.totalsRows, 1);
        QCOMPARE(valueOf(e, QStringLiteral("D2")), QStringLiteral("n:2"));
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QCOMPARE(int(back.book.tables().first().row2), 3);
        QCOMPARE(back.book.tables().first().columns.size(), 3);
    }

    // The row of names off and on: the names' cells are cleared and come back; the style's marks and the buttons are one step.
    void theRowOfNamesAndTheMarksAreSteps()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({row(1, text(QStringLiteral("A1"), QStringLiteral("x"))) .isEmpty() ? salesRows() : salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        crunch::EditSession::TableMarks marks;
        marks.style = QStringLiteral("TableStyleDark3");
        marks.firstColumn = true;
        marks.rowStripes = false;
        marks.columnStripes = true;
        marks.filterButtons = false;
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        QVERIFY(s->setTableMarks(0, marks).ok);
        QCOMPARE(u->count(), before + 1);
        const Table &t = s->workbook()->tables().first();
        QCOMPARE(t.styleName, QStringLiteral("TableStyleDark3"));
        QVERIFY(t.showFirstColumn && !t.showRowStripes && t.showColumnStripes && !t.filterButtons);
        QVERIFY(s->setTableMarks(0, marks).ok);
        QCOMPARE(u->count(), before + 1);   // (nothing changed: no step)
        u->undo();
        QVERIFY(s->workbook()->tables().first().showRowStripes);
        u->redo();
        QVERIFY(s->setTableHeaderRow(0, false).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().headerRows, 0);
        QCOMPARE(int(s->workbook()->tables().first().row1), 1);
        QCOMPARE(valueOf(e, QStringLiteral("B1")), QStringLiteral("-"));
        QCOMPARE(s->workbook()->tables().first().columns.first(), QStringLiteral("Region"));   // (the names are kept for the formulas)
        QVERIFY(s->setTableHeaderRow(0, true).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("B1")), QStringLiteral("s:Q1"));
        QCOMPARE(int(s->workbook()->tables().first().row1), 0);
        // on with something in the row above: refused
        QVERIFY(s->setTableHeaderRow(0, false).ok);
        QVERIFY(s->type(0, 0, 0, QStringLiteral("title")).ok);
        const auto refused = s->setTableHeaderRow(0, true);
        QVERIFY(!refused.ok && !refused.problem.isEmpty());
    }

    // A formula with a table's name worked out in the window, and the references to a column that is renamed in its header follow.
    void structuredReferencesWorkInFormulasAndFollowRenamedColumns()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=SUM(Table1[Q1])")).ok);
        QVERIFY(s->type(0, 7, 1, QStringLiteral("=SUM(Table1[[Q1]:[Q2]])")).ok);
        QVERIFY(s->type(0, 7, 2, QStringLiteral("=ROWS(Table1[#Data])&\"/\"&COLUMNS(Table1[#All])&\"/\"&INDEX(Table1[Region],2)")).ok);
        QVERIFY(s->type(0, 7, 3, QStringLiteral("=Table1[Q1]")).ok);   // (a column typed alone in a cell: its first cell)
        QVERIFY(s->type(0, 7, 4, QStringLiteral("=SUM(Nope[Q1])")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        QCOMPARE(valueOf(e, QStringLiteral("B8")), QStringLiteral("n:66"));
        QCOMPARE(valueOf(e, QStringLiteral("C8")), QStringLiteral("s:3/3/West"));
        QCOMPARE(valueOf(e, QStringLiteral("E8")), QStringLiteral("e:#REF!"));
        // a value changed in the column: what reads it follows
        QVERIFY(s->type(0, 1, 1, QStringLiteral("100")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:105"));
        // the column renamed in its header
        QVERIFY(s->type(0, 0, 1, QStringLiteral("First")).ok);
        QVERIFY(e.settle());
        QCOMPARE(editOf(e, QStringLiteral("A8")), QStringLiteral("=SUM(Table1[First])"));
        QCOMPARE(editOf(e, QStringLiteral("B8")), QStringLiteral("=SUM(Table1[[First]:[Q2]])"));
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:105"));
        // and a table in the other direction: a name that was a column's before is not, now
        QVERIFY(s->type(0, 7, 5, QStringLiteral("=SUM(Table1[Q1])")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("F8")), QStringLiteral("e:#REF!"));
    }

    // Formulas typed inside the table with the short form are kept as the file writes them, and shown as they were typed.
    void theShortFormOfARowReferenceIsKeptAsTheFileWritesIt()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=Table1[@Q2]+1")).ok);   // (outside: the long form is not needed, the table is named)
        QVERIFY(s->type(0, 3, 3, QStringLiteral("=[@Q1]+[@[Q2]]")).ok);   // beside the table, in its row: the table grows and the column is filled
        QVERIFY(e.settle());
        const Calculation *calc = s->calculation();
        QCOMPARE(calc->content(0, 3, 3).formula, QStringLiteral("Table1[[#This Row],[Q1]]+Table1[[#This Row],[Q2]]"));
        QCOMPARE(valueOf(e, QStringLiteral("D4")), QStringLiteral("n:33"));
        QCOMPARE(valueOf(e, QStringLiteral("D2")), QStringLiteral("n:11"));
    }
};

JP_TEST_CLASS(CrunchTablesTests)
#include "tst_crunch_tables.moc"
