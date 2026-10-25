// Tests of tables as a person makes and changes them through the edit session (core/edittables.cpp): making one from cells, turning it back, naming it, the row of names and the row of totals,
// resizing, the table growing by a cell typed next to it, a formula typed in an empty column filling it, structured references in formulas, one step of Undo each, and what a save leaves of
// the file. The workbook is built here.
#include "app/crunchwindow.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"
#include "grid/gridview.h"
#include "savedok.h"
#include "sheet/structref.h"
#include "sheet/tablestyle.h"
#include "app/ribbon.h"
#include "app/filterpopup.h"
#include "app/formulahelper.h"
#include "core/formulaspot.h"
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QTreeWidget>

#include <QMenu>
#include <QSignalSpy>
#include <QToolButton>
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
    const bool started = e.win.w.save();
    // (a save that has nothing to write is over when the call returns)
    return started && (!saved.isEmpty() ? saved.last().value(1).toBool() : savedOk(saved));
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

// The window with the popups that stand over it (the list of the help, a menu) drawn where they are.
void saveWithPopups(EditWindow &e, const QString &name, std::initializer_list<QWidget *> popups)
{
    if (qgetenv("JC_SHOT_DIR").isEmpty()) return;
    QCoreApplication::processEvents();
    QPixmap shot = e.win.w.grab();
    QPainter p(&shot);
    for (QWidget *w : popups)
        if (w && w->isVisible()) p.drawPixmap(e.win.w.mapFromGlobal(w->pos()), w->grab());
    p.end();
    QDir().mkpath(QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR")));
    shot.save(QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR")) + QLatin1Char('/') + name + QStringLiteral(".png"));
}

bool rowHidden(EditWindow &e, quint32 row)
{
    for (const RowSpec &r : e.win.w.session()->calculation()->rowSpecsIn(0, row, row))
        if (r.row == row) return r.hidden;
    return false;
}

bool tabShown(CrunchWindow &w)
{
    Ribbon *r = w.ribbon();
    for (int i = 0; i < r->tabCount(); ++i)
        if (r->tabName(i) == QLatin1String("Table")) return r->tabVisible(i);
    return false;
}

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
        marks.style = QStringLiteral("TableStyleDark1");
        marks.firstColumn = true;
        marks.rowStripes = false;
        marks.columnStripes = true;
        marks.filterButtons = false;
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        QVERIFY(s->setTableMarks(0, marks).ok);
        QCOMPARE(u->count(), before + 1);
        const Table &t = s->workbook()->tables().first();
        QCOMPARE(t.styleName, QStringLiteral("TableStyleDark1"));
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

    // Rows and columns inserted inside a table: the table has more of them, new rows get the calculated columns' formulas, new columns are named in the row of names; deleted columns go from
    // the table; each is undone, and a column that is gone leaves #REF! in the formulas that named it.
    void rowsAndColumnsInsideATableChangeIt()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 1, 3, QStringLiteral("=[@Q1]*5")).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=SUM(Table1[Q1])")).ok);
        QVERIFY(e.settle());
        QUndoStack *u = e.win.w.undoStack();
        QVERIFY(s->insertRows(0, 2, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
        QCOMPARE(editOf(e, QStringLiteral("D3")), QStringLiteral("=Table1[[#This Row],[Q1]]*5"));
        QCOMPARE(valueOf(e, QStringLiteral("D3")), QStringLiteral("n:0"));
        u->undo();   // the formulas
        u->undo();   // the row
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 3);
        QVERIFY(s->insertColumns(0, 1, 1).ok);
        QVERIFY(e.settle());
        const Table &t = s->workbook()->tables().first();
        QCOMPARE(t.columns, (QStringList{QStringLiteral("Region"), QStringLiteral("Column5"), QStringLiteral("Q1"), QStringLiteral("Q2"), QStringLiteral("Column4")}));
        QCOMPARE(int(t.col2), 4);
        QCOMPARE(valueOf(e, QStringLiteral("B1")), QStringLiteral("s:Column5"));
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        QCOMPARE(valueOf(e, QStringLiteral("E3")), QStringLiteral("n:10"));
        // a column deleted: the one the formula names
        QVERIFY(s->deleteColumns(0, 2, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().columns.size(), 4);
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("e:#REF!"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().columns.size(), 5);
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        // all of the columns can't be deleted
        QVERIFY(!s->deleteColumns(0, 0, 5).ok);
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QCOMPARE(back.book.tables().first().columns.size(), 5);
        QCOMPARE(back.book.tables().first().columns.at(1), QStringLiteral("Column5"));
    }

    // Insert > Table makes a table of the block round the cursor and draws it in its look: the header filled, the rows striped, a button on each name; the Table tab shows while the cursor
    // is in it; the look is chosen from the gallery; the table is not drawn where it is not.
    void insertTableDrawsTheTableInItsLook()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        QString asked;
        w.setTableMakeAsker([&](QString *where, bool *headers) {
            asked = *where;
            *headers = true;
            return true;
        });
        e.select(QStringLiteral("B3"));
        w.act(QStringLiteral("insert.table"))->trigger();
        QCOMPARE(asked, QStringLiteral("A1:C4"));
        QCOMPARE(w.session()->workbook()->tables().size(), 1);
        QCOMPARE(int(e.view()->headerButtonCount()), 3);
        auto colorAt = [&](const QString &ref) {
            quint32 r = 0, c = 0;
            parseCellRef(ref, &r, &c);
            const QRect cell = e.view()->cellRect(r, c);
            const QImage img = e.view()->viewport()->grab().toImage();
            const qreal ratio = img.devicePixelRatio();
            return img.pixelColor(int((cell.left() + 3) * ratio), int((cell.bottom() - 3) * ratio));
        };
        QTest::qWait(20);
        const QColor header = colorAt(QStringLiteral("A1")), stripe = colorAt(QStringLiteral("A2")), plain = colorAt(QStringLiteral("A3")), outside = colorAt(QStringLiteral("F2"));
        QVERIFY(header != outside);                     // (a filled header)
        QVERIFY(header.blue() > header.red());          // (the first accent is blue)
        QVERIFY(stripe != plain);                       // (the rows are striped)
        QVERIFY(plain == outside || plain.lightness() > 240);
        QCOMPARE(outside.name(), QColor(Qt::white).name());
        saveShot(&w, QStringLiteral("table-styled"));
        // the Table tab shows for a cell in the table and not for one outside
        QVERIFY(tabShown(w));
        e.select(QStringLiteral("F2"));
        QVERIFY(!tabShown(w));
        e.select(QStringLiteral("B3"));
        QVERIFY(w.act(QStringLiteral("table.stripeRows"))->isChecked());
        QVERIFY(w.act(QStringLiteral("table.header"))->isChecked());
        QVERIFY(!w.act(QStringLiteral("table.totals"))->isChecked());
        // marks from the tab
        w.act(QStringLiteral("table.stripeRows"))->trigger();
        QVERIFY(!w.session()->workbook()->tables().first().showRowStripes);
        QTest::qWait(20);
        QCOMPARE(colorAt(QStringLiteral("A2")), colorAt(QStringLiteral("A3")));
        w.act(QStringLiteral("table.stripeRows"))->trigger();
        // the gallery: a look chosen by its button
        QWidget *picker = w.makeTableLooksPicker(nullptr);
        picker->resize(picker->sizeHint());
        picker->show();
        auto *button = picker->findChild<QToolButton *>(QStringLiteral("tableLook.TableStyleDark1"));
        QVERIFY(button);
        QCOMPARE(picker->findChildren<QToolButton *>().size(), 60);
        saveShot(picker, QStringLiteral("table-looks-gallery"));
        button->click();
        QCOMPARE(w.session()->workbook()->tables().first().styleName, QStringLiteral("TableStyleDark1"));
        delete picker;
        QTest::qWait(20);
        QVERIFY(colorAt(QStringLiteral("A3")).lightness() < 140);   // (a dark look)
        // the name from the tab, and back to cells
        auto *box = w.findChild<QLineEdit *>(QStringLiteral("table.name"));
        QVERIFY(box);
        QCOMPARE(box->text(), QStringLiteral("Table1"));
        box->setText(QStringLiteral("Sales"));
        box->setModified(true);
        box->editingFinished();
        QCOMPARE(w.session()->workbook()->tables().first().name, QStringLiteral("Sales"));
        w.act(QStringLiteral("table.toRange"))->trigger();
        QVERIFY(w.session()->workbook()->tables().isEmpty());
        QCOMPARE(int(e.view()->headerButtonCount()), 0);
        QVERIFY(!tabShown(w));
        // undone, the table is back, drawn, and its buttons; the cells in a table can't be made one again
        w.undoStack()->undo();
        QCOMPARE(int(e.view()->headerButtonCount()), 3);
        e.select(QStringLiteral("B3"));
        const int steps = w.undoStack()->count();
        w.act(QStringLiteral("insert.table"))->trigger();
        QCOMPARE(w.undoStack()->count(), steps);
        QVERIFY(e.win.told.last().contains(QStringLiteral("in a table already")));
    }

    // The totals row: a button on each of its cells chooses what the column adds up; a filter from the buttons on the names hides rows of the table only, and the totals leave out the rows it
    // hides; sorting from the same button puts the table's rows in order; the filter is undone, saved, and kept in the table's part.
    void theTotalsButtonsAndTheFilterButtonsOfATable()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        w.setTableMakeAsker([](QString *, bool *headers) {
            *headers = true;
            return true;
        });
        e.select(QStringLiteral("B3"));
        w.act(QStringLiteral("insert.table"))->trigger();
        w.act(QStringLiteral("table.totals"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.session()->workbook()->tables().first().totalsRows, 1);
        QCOMPARE(int(e.view()->headerButtonCount()), 6);          // (three on the names, three on the totals)
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("n:60"));
        // the button of a total: the menu lists the functions with the one in use checked
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->headerButtonRect(4, 1).center());
        QMenu *menu = w.findChild<QMenu *>(QStringLiteral("totalsMenu"));
        QVERIFY(menu);
        QStringList texts;
        QAction *average = nullptr;
        for (QAction *a : menu->actions()) {
            texts << a->text();
            if (a->text() == QLatin1String("Average")) average = a;
            if (a->isChecked()) QCOMPARE(a->text(), QStringLiteral("None"));   // (the first column of the sum is the last: this one has none)
        }
        QCOMPARE(texts.size(), 10);
        QVERIFY(average);
        saveWithPopups(e, QStringLiteral("table-totals-menu"), {menu});
        average->trigger();
        delete menu;
        QVERIFY(e.settle());
        QCOMPARE(editOf(e, QStringLiteral("B5")), QStringLiteral("=SUBTOTAL(101,Table1[Q1])"));
        QCOMPARE(valueOf(e, QStringLiteral("B5")), QStringLiteral("n:2"));
        // a filter from the button on a name
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->headerButtonRect(0, 0).center());
        FilterPopup *popup = w.filterPopup();
        QVERIFY(popup);
        QTRY_VERIFY(popup->listed());
        QTreeWidgetItem *all = popup->tree()->topLevelItem(0);
        QCOMPARE(all->childCount(), 3);
        all->setCheckState(0, Qt::Unchecked);
        all->child(0)->setCheckState(0, Qt::Checked);   // East
        all->child(2)->setCheckState(0, Qt::Checked);   // West
        saveWithPopups(e, QStringLiteral("table-filter-dropdown"), {popup});
        popup->findChild<QPushButton *>(QStringLiteral("filterOk"))->click();
        QTRY_VERIFY(!w.isFiltering());
        QVERIFY(e.settle());
        QVERIFY(!rowHidden(e, 1) && !rowHidden(e, 2) && rowHidden(e, 3));       // (North, the third row of data, is hidden)
        QVERIFY(!rowHidden(e, 4) && !rowHidden(e, 0));                          // (the totals and the names stay)
        QVERIFY(w.session()->rowFilteredOut(0, 3));                             // (a row the filter hides, as a sheet's filter does)
        QCOMPARE(valueOf(e, QStringLiteral("B5")), QStringLiteral("n:1.5"));    // (East 1 and West 2 are shown: their average)
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("n:30"));     // (the sum of 10 and 20)
        QVERIFY(w.session()->tableFilter(0).active());
        QCOMPARE(w.session()->workbook()->tables().first().filter.columns.size(), 1);
        QVERIFY(!w.session()->calculation()->autoFilterOf(0).valid());          // (the sheet has no filter)
        saveShot(&w, QStringLiteral("table-filtered"));
        // sorted from the button: the rows that show are put in order (Z to A)
        w.undoStack()->count();
        const int steps = w.undoStack()->count();
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->headerButtonRect(0, 0).center());
        popup = w.filterPopup();
        QVERIFY(popup);
        QTRY_VERIFY(popup->listed());
        Q_EMIT popup->sortRequested(true);
        QTRY_VERIFY(w.undoStack()->count() > steps);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("s:West"));
        QCOMPARE(valueOf(e, QStringLiteral("A3")), QStringLiteral("s:East"));
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("s:North"));   // (the row the filter hides stays where it is)
        QVERIFY(rowHidden(e, 3));
        // saved: the part keeps the filter, the file has the rows hidden, and it opens as it was
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QCOMPARE(back.book.tables().first().filter.columns.size(), 1);
        QCOMPARE(back.book.tables().first().filter.columns[0].values.size(), 2);
        Sheet sheet;
        QVERIFY(back.book.readSheet(0, &sheet));
        bool hidden = false;
        for (const RowSpec &r : sheet.rowSpecs)
            if (r.row == 3) hidden = r.hidden;
        QVERIFY(hidden);
        QVERIFY(!sheet.autoFilter.valid());
        // a filtered table keeps its buttons: taking them off is refused (the rows it hides would stay hidden)
        w.act(QStringLiteral("table.buttons"))->trigger();
        QVERIFY(w.session()->workbook()->tables().first().filterButtons);
        QVERIFY(e.win.told.last().contains(QStringLiteral("Clear its filter first")));
        QVERIFY(w.act(QStringLiteral("table.buttons"))->isChecked());
        // cleared: the rows are shown again, and the totals count them
        w.act(QStringLiteral("filter.clear"))->trigger();
        QTRY_VERIFY(!w.isFiltering());
        QVERIFY(e.settle());
        QVERIFY(!rowHidden(e, 3));
        QVERIFY(!w.session()->tableFilter(0).active());
        // the buttons off: the table has none (and a filtered table cannot lose them)
        w.act(QStringLiteral("table.buttons"))->trigger();
        QCOMPARE(int(e.view()->headerButtonCount()), 3);
    }

    // What is typed inside the brackets of a table is helped: after the name and "[" the table's columns and the special items are listed, a few letters narrow the list, Tab puts the column
    // in with its closing bracket, and a name that needs the double brackets is given them; in a cell of the table, [@ lists the columns too.
    void theColumnsOfATableAreOfferedInsideItsBrackets()
    {
        crunch::FormulaSpot spot = crunch::spotAt(QStringLiteral("=SUM(Sales[Am"), 13, QLatin1Char(','));
        QVERIFY(spot.inText && spot.inColumns);
        QCOMPARE(spot.table, QStringLiteral("Sales"));
        QCOMPARE(spot.columnWord, QStringLiteral("Am"));
        QCOMPARE(spot.columnStart, 11);
        spot = crunch::spotAt(QStringLiteral("=Sales[[#Headers],[Re"), 21, QLatin1Char(','));
        QVERIFY(spot.inColumns);
        QCOMPARE(spot.columnWord, QStringLiteral("Re"));
        spot = crunch::spotAt(QStringLiteral("=[@Q"), 4, QLatin1Char(','));
        QVERIFY(spot.inColumns);
        QVERIFY(spot.table.isEmpty());
        QCOMPARE(spot.columnWord, QStringLiteral("Q"));
        QVERIFY(!crunch::spotAt(QStringLiteral("=Sales[[#Headers],"), 18, QLatin1Char(',')).inColumns);   // (a comma between the parts: nothing typed yet)
        QVERIFY(!crunch::spotAt(QStringLiteral("=\"Sales[Am"), 10, QLatin1Char(',')).inColumns);           // (in a text)
        QVERIFY(crunch::spotAt(QStringLiteral("=1+"), 3, QLatin1Char(',')).canPoint);
        QVERIFY(!crunch::spotAt(QStringLiteral("=1+A"), 4, QLatin1Char(',')).canPoint);
        QVERIFY(!crunch::spotAt(QStringLiteral("=1"), 2, QLatin1Char(',')).canPoint);
        crunch::CompletionIndex index;
        index.setTableColumns({{QStringLiteral("Sales"), {QStringLiteral("Region"), QStringLiteral("Amount"), QStringLiteral("Unit #")}}});
        QCOMPARE(int(index.columnMatches(QStringLiteral("sales"), QString()).size()), 3 + 5);
        const auto am = index.columnMatches(QStringLiteral("Sales"), QStringLiteral("am"));
        QCOMPARE(int(am.size()), 1);
        QCOMPARE(am[0].text, QStringLiteral("Amount"));
        QCOMPARE(int(index.columnMatches(QStringLiteral("Sales"), QStringLiteral("#h")).size()), 1);
        QVERIFY(index.columnMatches(QStringLiteral("Nothing"), QString()).empty());
        // in the window
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(w.session()->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(w.session()->renameTable(0, QStringLiteral("Sales")).ok);
        QVERIFY(w.session()->type(0, 0, 2, QStringLiteral("Q 2.5")).ok);
        QVERIFY(e.settle());
        FormulaHelper *h = w.formulaHelper();
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=SUM(Sales["));
        QVERIFY(h->isListOpen());
        QCOMPARE(h->offered().mid(0, 3), (QStringList{QStringLiteral("Region"), QStringLiteral("Q1"), QStringLiteral("Q 2.5")}));
        QVERIFY(h->offered().contains(QStringLiteral("#Totals")));
        saveWithPopups(e, QStringLiteral("table-autocomplete"), {h->list()->window(), static_cast<QWidget *>(h->hint())});
        QTest::keyClicks(w.cellEditor(), QStringLiteral("q"));
        QCOMPARE(h->offered(), (QStringList{QStringLiteral("Q1"), QStringLiteral("Q 2.5")}));
        QTest::keyClick(w.cellEditor(), Qt::Key_Down);
        QTest::keyClick(w.cellEditor(), Qt::Key_Tab);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(Sales[[Q 2.5]]"));
        QVERIFY(!h->isListOpen());
        QTest::keyClicks(w.cellEditor(), QStringLiteral(")"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("F1")), QStringLiteral("n:60"));
        // a column that is a plain word: the closing bracket goes after it, whatever follows
        e.select(QStringLiteral("F2"));
        e.type(QStringLiteral("=Sales[Reg"));
        QCOMPARE(h->offered(), QStringList{QStringLiteral("Region")});
        QTest::keyClick(w.cellEditor(), Qt::Key_Tab);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Sales[Region]"));
        e.key(Qt::Key_Escape);
        // inside the table, [@ lists the table's own columns
        e.select(QStringLiteral("B3"));
        e.type(QStringLiteral("=[@"));
        QVERIFY(h->isListOpen());
        QCOMPARE(h->offered().mid(0, 2), (QStringList{QStringLiteral("Region"), QStringLiteral("Q1")}));
        e.key(Qt::Key_Escape);
        e.key(Qt::Key_Escape);
        // a renamed column is what is offered next
        QVERIFY(w.session()->type(0, 0, 1, QStringLiteral("First")).ok);
        e.select(QStringLiteral("F3"));
        e.type(QStringLiteral("=Sales[Fir"));
        QCOMPARE(h->offered(), QStringList{QStringLiteral("First")});
        e.key(Qt::Key_Escape);
        e.key(Qt::Key_Escape);
    }

    // While a formula is typed, a click on a cell of a table writes a reference by the table's names: the cell of its own row, a name, a total; a cell of another row is its address; a cell
    // that is in no table is not pointed at (the click ends the entry as before).
    void aClickOnATableCellWhileTypingAFormulaWritesAStructuredReference()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(w.session()->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(w.session()->setTableTotalsRow(0, true).ok);
        QVERIFY(e.settle());
        auto click = [&](const QString &ref) {
            quint32 r = 0, c = 0;
            parseCellRef(ref, &r, &c);
            const QRect cell = e.view()->cellRect(r, c);
            QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(cell.left() + 6, cell.top() + 4));
        };
        // beside the table, in its row 3: the cell of the row is Table1[@Q1]
        e.select(QStringLiteral("F3"));
        e.type(QStringLiteral("="));
        click(QStringLiteral("B3"));
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Table1[@Q1]"));
        QTest::keyClicks(w.cellEditor(), QStringLiteral("+"));
        click(QStringLiteral("C1"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Table1[@Q1]+Table1[[#Headers],[Q2]]"));
        QTest::keyClicks(w.cellEditor(), QStringLiteral("&"));
        click(QStringLiteral("B5"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Table1[@Q1]+Table1[[#Headers],[Q2]]&Table1[[#Totals],[Q1]]"));
        QTest::keyClicks(w.cellEditor(), QStringLiteral("&"));
        click(QStringLiteral("A4"));    // (another row of the table: its address)
        QVERIFY(w.cellEditor()->text().endsWith(QStringLiteral("&A4")));
        e.key(Qt::Key_Escape);
        // in the table: [@Q1]
        e.select(QStringLiteral("C3"));
        e.type(QStringLiteral("=2*"));
        click(QStringLiteral("B3"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=2*[@Q1]"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(w.session()->calculation()->content(0, 2, 2).formula, QStringLiteral("2*Table1[[#This Row],[Q1]]"));
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("n:4"));
        // where no reference may go (after a name, a number), or on a cell that is in no table: the click ends the entry
        e.select(QStringLiteral("F4"));
        e.type(QStringLiteral("=1"));
        click(QStringLiteral("B3"));
        QVERIFY(!w.isEditing());
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("F4")), QStringLiteral("n:1"));
    }

    // A sheet that has tables is copied with them: the copy's tables have names, ids, and parts of their own, the formulas of the copy name them, and a formula of another sheet is not
    // changed; the saved file has every table once with its own id; Undo takes the copies away.
    void aCopyOfASheetHasCopiesOfItsTables()
    {
        const QByteArray two = buildXlsx({salesRows(), worksheet(row(1, formula(QStringLiteral("A1"), QStringLiteral("SUM(Sales[Q1])"))))}, QByteArray(), QString(), QString(), QByteArray(), QStringList{QStringLiteral("Data"), QStringLiteral("Other")});
        EditWindow e;
        QVERIFY(e.open(two));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->renameTable(0, QStringLiteral("Sales")).ok);
        QVERIFY(s->type(0, 1, 3, QStringLiteral("=[@Q1]*10")).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=SUM(Sales[Q1])")).ok);
        QVERIFY(s->setTableTotalsRow(0, true).ok);
        QVERIFY(e.settle());
        QUndoStack *u = e.win.w.undoStack();
        int made = -1;
        const auto copied = s->copySheet(0, -1, QString(), &made);
        QVERIFY2(copied.ok, qPrintable(copied.problem));
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 2);
        const Table &c = s->workbook()->tables().last();
        QCOMPARE(c.name, QStringLiteral("Sales2"));
        QCOMPARE(c.sheet, made);
        QVERIFY(c.id != s->workbook()->tables().first().id && c.part != s->workbook()->tables().first().part);
        QCOMPARE(c.info[3].calcFormula, QStringLiteral("Sales2[[#This Row],[Q1]]*10"));
        QCOMPARE(c.styleName, s->workbook()->tables().first().styleName);
        QCOMPARE(editOf(e, QStringLiteral("A8"), made), QStringLiteral("=SUM(Sales2[Q1])"));
        QCOMPARE(editOf(e, QStringLiteral("A8"), 0), QStringLiteral("=SUM(Sales[Q1])"));
        QCOMPARE(editOf(e, QStringLiteral("A1"), 1), QStringLiteral("=SUM(Sales[Q1])"));
        QCOMPARE(valueOf(e, QStringLiteral("A8"), made), QStringLiteral("n:6"));
        QCOMPARE(valueOf(e, QStringLiteral("D3"), made), QStringLiteral("n:20"));
        // the copy's table is its own: a value changed there is not changed in the first
        QVERIFY(s->type(made, 1, 1, QStringLiteral("100")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8"), made), QStringLiteral("n:105"));
        QCOMPARE(valueOf(e, QStringLiteral("A8"), 0), QStringLiteral("n:6"));
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QCOMPARE(back.book.tables().size(), 2);
        QCOMPARE(back.book.tables()[1].name, QStringLiteral("Sales2"));
        QCOMPARE(back.book.tables()[1].sheet, made);
        QVERIFY(back.book.tables()[0].id != back.book.tables()[1].id);
        QCOMPARE(back.book.tables()[1].info[3].calcFormula, QStringLiteral("Sales2[[#This Row],[Q1]]*10"));
        QCOMPARE(back.formula(made, QStringLiteral("A8")), QStringLiteral("SUM(Sales2[Q1])"));
        // and again, it is another copy with another name
        QVERIFY(s->copySheet(0, -1, QString()).ok);
        QCOMPARE(s->workbook()->tables().last().name, QStringLiteral("Sales3"));
        QCOMPARE(s->workbook()->tables().size(), 3);
        u->undo();   // the second copy
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 2);
        QCOMPARE(s->workbook()->tables().last().name, QStringLiteral("Sales2"));
        // saved with the copy taken away, the file has the first copy and the table once
        QVERIFY(saveNow(e));
        Saved again;
        QVERIFY(again.open(e.path));
        QCOMPARE(again.book.tables().size(), 2);
    }
};

JP_TEST_CLASS(CrunchTablesTests)
#include "tst_crunch_tables.moc"
