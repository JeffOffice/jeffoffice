// Tests of tables as a person makes and changes them through the edit session (core/edittables.cpp): making one from cells, turning it back, naming it, the row of names and the row of totals,
// resizing, the table growing by a cell typed next to it, a formula typed in an empty column filling it, structured references in formulas, one step of Undo each, and what a save leaves of
// the file. The workbook is built here.
#include "sheet/datasource.h"
#include "app/crunchwindow.h"
#include "app/crunchinfo.h"
#include "app/connectionspane.h"
#include "app/getdata.h"
#include "app/theme.h"
#include "sheet/oldsave.h"
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
#include "app/removeduplicatesdialog.h"
#include "app/filterpopup.h"
#include "app/formulahelper.h"
#include "core/formulaspot.h"
#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QPointer>
#include <QProgressDialog>
#include <QTimer>
#include <atomic>
#include <memory>
#include <thread>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QScrollArea>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QTableWidget>
#include <QTreeWidget>

#include <QMenu>
#include <functional>
#include <QRegularExpression>
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
        const std::optional<Cell> c = at(s, ref);
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

QByteArray repeatedRows(int pairs)
{
    QString rows = row(1, text(QStringLiteral("A1"), QStringLiteral("K")) + text(QStringLiteral("B1"), QStringLiteral("V")));
    for (int i = 0; i < pairs * 2; ++i) rows += row(i + 2, num(QStringLiteral("A%1").arg(i + 2), i / 2) + num(QStringLiteral("B%1").arg(i + 2), i));
    return worksheet(rows);
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

    // Remove Duplicates in a table: a row that repeats an earlier one (in the columns chosen, in any case) is taken out, the rows under it move up, the table gets shorter, and the totals row goes
    // with its last row. One step of Undo; a table with rows of formulas of its own is refused.
    void duplicateRowsAreRemovedFromATable()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 4, 0, QStringLiteral("east")).ok);
        QVERIFY(s->type(0, 4, 1, QStringLiteral("1")).ok);
        QVERIFY(s->type(0, 4, 2, QStringLiteral("10")).ok);
        QVERIFY(s->type(0, 5, 0, QStringLiteral("West")).ok);
        QVERIFY(s->type(0, 5, 1, QStringLiteral("9")).ok);
        QVERIFY(s->type(0, 5, 2, QStringLiteral("90")).ok);
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 5);
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        int removed = -1;
        // on the first column alone: the second "East" and the second "West" are repeats
        auto o = s->removeTableDuplicates(0, {0}, &removed);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QCOMPARE(removed, 2);
        QVERIFY(e.settle());
        QCOMPARE(u->count(), before + 1);
        QCOMPARE(int(s->workbook()->tables().first().row2), 3);
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("s:North"));
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("-"));
        QCOMPARE(valueOf(e, QStringLiteral("B6")), QStringLiteral("-"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 5);
        QCOMPARE(valueOf(e, QStringLiteral("A6")), QStringLiteral("s:West"));
        QCOMPARE(valueOf(e, QStringLiteral("B6")), QStringLiteral("n:9"));
        // on all the columns: only the row that repeats whole goes
        o = s->removeTableDuplicates(0, {0, 1, 2}, &removed);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QCOMPARE(removed, 1);
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("s:West"));
        QCOMPARE(valueOf(e, QStringLiteral("B5")), QStringLiteral("n:9"));
        // nothing repeats now
        o = s->removeTableDuplicates(0, {0, 1, 2}, &removed);
        QVERIFY(o.ok);
        QCOMPARE(removed, 0);
        // no column chosen: refused
        QVERIFY(!s->removeTableDuplicates(0, {}, &removed).ok);
    }

    // What points at the cells of a table follows Remove Duplicates as it follows a delete: a formula under the table keeps its cells (the cells under a table stay where they are), a range that
    // ends in the removed row is shorter, a formula on another sheet follows too, and a reference to a removed row is #REF!. One step of Undo puts it all back.
    void removingDuplicatesAdjustsTheFormulasThatPointAtTheRows()
    {
        EditWindow e;
        const QByteArray other = worksheet(row(1, formula(QStringLiteral("A1"), QStringLiteral("Sheet1!B5")) + formula(QStringLiteral("B1"), QStringLiteral("Sheet1!B6")) + formula(QStringLiteral("C1"), QStringLiteral("Sheet1!B4"))
                                                 + formula(QStringLiteral("D1"), QStringLiteral("SUM(Sheet1!B2:B5)"))));
        QVERIFY(e.open(buildXlsx({salesRows(), other})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 4, 0, QStringLiteral("West")).ok);   // (row 5: a repeat of row 3)
        QVERIFY(s->type(0, 4, 1, QStringLiteral("2")).ok);
        QVERIFY(s->type(0, 4, 2, QStringLiteral("20")).ok);
        QVERIFY(s->type(0, 6, 1, QStringLiteral("=SUM(B2:B5)")).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=B5")).ok);
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        int removed = 0;
        QVERIFY(s->removeTableDuplicates(0, {0, 1, 2}, &removed).ok);
        QCOMPARE(removed, 1);
        QVERIFY(e.settle());
        QCOMPARE(u->count(), before + 1);
        QCOMPARE(int(s->workbook()->tables().first().row2), 3);
        QCOMPARE(editOf(e, QStringLiteral("B6")), QStringLiteral("=SUM(B2:B4)"));   // (under the table: still there, and still reads the same rows)
        QCOMPARE(editOf(e, QStringLiteral("B7")), QStringLiteral("=SUM(B2:B4)"));   // (the range ended in the removed row)
        QCOMPARE(editOf(e, QStringLiteral("A8")), QStringLiteral("=#REF!"));
        QCOMPARE(editOf(e, QStringLiteral("A1"), 1), QStringLiteral("=Sheet1!#REF!"));
        QCOMPARE(editOf(e, QStringLiteral("B1"), 1), QStringLiteral("=Sheet1!B6"));   // (a cell under the table is where it was)
        QCOMPARE(editOf(e, QStringLiteral("C1"), 1), QStringLiteral("=Sheet1!B4"));
        QCOMPARE(editOf(e, QStringLiteral("D1"), 1), QStringLiteral("=SUM(Sheet1!B2:B4)"));
        QCOMPARE(valueOf(e, QStringLiteral("B7")), QStringLiteral("n:6"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
        QCOMPARE(editOf(e, QStringLiteral("B7")), QStringLiteral("=SUM(B2:B5)"));
        QCOMPARE(editOf(e, QStringLiteral("A8")), QStringLiteral("=B5"));
        QCOMPARE(editOf(e, QStringLiteral("A1"), 1), QStringLiteral("=Sheet1!B5"));
        QCOMPARE(editOf(e, QStringLiteral("D1"), 1), QStringLiteral("=SUM(Sheet1!B2:B5)"));
    }

    // Remove Duplicates for a plain range (the selection less its header row): the same engine as a table's, so the cells under the range stay, what points at them follows, and Undo is one step.
    void duplicateRowsAreRemovedFromAPlainRange()
    {
        EditWindow e;
        const QByteArray sheet = worksheet(row(1, text(QStringLiteral("A1"), QStringLiteral("K")) + text(QStringLiteral("B1"), QStringLiteral("V")))
                                           + row(2, text(QStringLiteral("A2"), QStringLiteral("a")) + num(QStringLiteral("B2"), 1)) + row(3, text(QStringLiteral("A3"), QStringLiteral("b")) + num(QStringLiteral("B3"), 2))
                                           + row(4, text(QStringLiteral("A4"), QStringLiteral("A")) + num(QStringLiteral("B4"), 3)) + row(5, text(QStringLiteral("A5"), QStringLiteral("c")) + num(QStringLiteral("B5"), 4))
                                           + row(6, text(QStringLiteral("A6"), QStringLiteral("b")) + num(QStringLiteral("B6"), 5))
                                           + row(7, text(QStringLiteral("A7"), QStringLiteral("under")) + formula(QStringLiteral("B7"), QStringLiteral("SUM(B2:B6)"))) + row(8, formula(QStringLiteral("B8"), QStringLiteral("B6"))));
        QVERIFY(e.open(buildXlsx({sheet})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QUndoStack *u = e.win.w.undoStack();
        const int before = u->count();
        int removed = -1;
        // the first column says what repeats ("A" repeats "a" in any case; the second "b" repeats the first)
        const auto o = s->removeDuplicates(0, rangeText(QStringLiteral("A1:B6")), true, {0}, &removed);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QCOMPARE(removed, 2);
        QVERIFY(e.settle());
        QCOMPARE(u->count(), before + 1);
        QCOMPARE(valueOf(e, QStringLiteral("A1")), QStringLiteral("s:K"));
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("s:a"));
        QCOMPARE(valueOf(e, QStringLiteral("A3")), QStringLiteral("s:b"));
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("s:c"));
        QCOMPARE(valueOf(e, QStringLiteral("B4")), QStringLiteral("n:4"));
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("-"));
        QCOMPARE(valueOf(e, QStringLiteral("A6")), QStringLiteral("-"));
        QCOMPARE(valueOf(e, QStringLiteral("A7")), QStringLiteral("s:under"));   // (under the range: where it was)
        QCOMPARE(editOf(e, QStringLiteral("B7")), QStringLiteral("=SUM(B2:B4)"));
        QCOMPARE(editOf(e, QStringLiteral("B8")), QStringLiteral("=#REF!"));
        u->undo();
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("s:A"));
        QCOMPARE(editOf(e, QStringLiteral("B7")), QStringLiteral("=SUM(B2:B6)"));
        // no header: the first row is data too; a range of one row, or no column chosen, is refused
        QVERIFY(!s->removeDuplicates(0, rangeText(QStringLiteral("A2:B2")), false, {0}, &removed).ok);
        QVERIFY(!s->removeDuplicates(0, rangeText(QStringLiteral("A1:B6")), true, {}, &removed).ok);
        QVERIFY(!s->removeDuplicates(0, rangeText(QStringLiteral("A1:B6")), true, {5}, &removed).ok);
    }

    // No two keytips on a tab of the ribbon are the same (a tab's, its groups', and its commands' are pressed one after the other), and no two tabs have the same: the keys of Table, Pivot Table,
    // and the other new commands do not clash.
    void theKeytipsOfEveryTabAreUnique()
    {
        QFile file(QStringLiteral(JC_SOURCE_DIR "/apps/crunch/resources/ribbon.json"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
        std::function<void(const QJsonValue &, QStringList *)> walk = [&](const QJsonValue &v, QStringList *out) {
            if (v.isObject()) {
                const QJsonObject o = v.toObject();
                if (o.contains(QStringLiteral("keytip"))) *out << o.value(QStringLiteral("keytip")).toString();
                for (auto it = o.begin(); it != o.end(); ++it) walk(it.value(), out);
            } else if (v.isArray()) {
                for (const QJsonValue &x : v.toArray()) walk(x, out);
            }
        };
        QStringList tabKeys;
        const QJsonArray tabs = root.value(QStringLiteral("tabs")).toArray();
        QVERIFY(tabs.size() > 8);
        QSet<QString> seenNew;
        for (const QJsonValue &t : tabs) {
            const QJsonObject tab = t.toObject();
            tabKeys << tab.value(QStringLiteral("keytip")).toString();
            QStringList keys;
            walk(tab.value(QStringLiteral("groups")), &keys);
            QStringList seen;
            for (const QString &k : keys) {
                QVERIFY2(!seen.contains(k), qPrintable(tab.value(QStringLiteral("name")).toString() + QStringLiteral(": ") + k));
                seen << k;
            }
            for (const QString &k : {QStringLiteral("ZU"), QStringLiteral("TZ"), QStringLiteral("TD"), QStringLiteral("PM"), QStringLiteral("DD"), QStringLiteral("DU")})
                if (seen.contains(k)) seenNew.insert(k);
        }
        QCOMPARE(int(seenNew.size()), 6);   // (all six are on a tab)
        QStringList uniqueTabs = tabKeys;
        uniqueTabs.removeDuplicates();
        QCOMPARE(int(uniqueTabs.size()), int(tabKeys.size()));
    }

    // The Remove Duplicates box has a scroll area for many columns, and an & in a column's name is shown as an & (not a shortcut).
    void theRemoveDuplicatesBoxScrollsAndShowsAmpersands()
    {
        QStringList names{QStringLiteral("Q&A"), QStringLiteral("B")};
        for (int i = 0; i < 60; ++i) names << QStringLiteral("Column%1").arg(i);
        RemoveDuplicatesDialog d(names, nullptr);
        QVERIFY(d.findChild<QScrollArea *>());
        const QList<QCheckBox *> boxes = d.findChildren<QCheckBox *>();
        QCOMPARE(int(boxes.size()), int(names.size()));
        QCOMPARE(boxes.first()->text(), QStringLiteral("Q&&A"));
        QVERIFY(QKeySequence::mnemonic(boxes.first()->text()).isEmpty());
        QCOMPARE(int(d.chosen().size()), int(names.size()));
    }

    // So many separate repeats that deleting them one run at a time would take minutes are refused, with nothing changed.
    void aVeryBrokenUpSetOfRepeatsIsRefused()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({repeatedRows(crunch::kMaxDuplicateRuns + 20)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const int before = e.win.w.undoStack()->count();
        int removed = -1;
        QElapsedTimer clock;
        clock.start();
        const auto o = s->removeDuplicates(0, rangeText(QStringLiteral("A1:B%1").arg((crunch::kMaxDuplicateRuns + 20) * 2 + 1)), true, {0}, &removed);
        QVERIFY(!o.ok);
        QVERIFY(clock.elapsed() < 5000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")));
        QCOMPARE(e.win.w.undoStack()->count(), before);
        QCOMPARE(valueOf(e, QStringLiteral("A3")), QStringLiteral("n:0"));
        // fewer runs go through
        QVERIFY(s->removeDuplicates(0, rangeText(QStringLiteral("A1:B21")), true, {0}, &removed).ok);
        QCOMPARE(removed, 10);
    }

    // A protected sheet refuses the removal when a cell it would move is locked; with the cells unlocked it goes through. A sheet that can't be edited is refused too.
    void removingDuplicatesRespectsProtection()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({repeatedRows(5)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        jp::office::sheet::SheetProtection p;
        p.enabled = true;
        QVERIFY(s->setSheetProtection(0, p, QStringLiteral("Protect Sheet")).ok);
        int removed = -1;
        const int before = e.win.w.undoStack()->count();
        const auto o = s->removeDuplicates(0, rangeText(QStringLiteral("A1:B11")), true, {0}, &removed);
        QVERIFY(!o.ok);
        QVERIFY(!o.problem.isEmpty());
        QCOMPARE(e.win.w.undoStack()->count(), before);
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
        // a formula that names a column the table has is a total; one that names a column it has not is refused (the file would hold a total that is #REF!, which the other program will not open)
        const auto lost = s->setTableTotal(0, 2, QStringLiteral("custom"), QStringLiteral("SUBTOTAL(109,[Q3])*2"));
        QVERIFY(!lost.ok);
        QVERIFY(!lost.problem.isEmpty());
        QVERIFY(!s->setTableTotal(0, 2, QStringLiteral("custom"), QStringLiteral("SUM(Table1[Nope])")).ok);
        QVERIFY(s->setTableTotal(0, 2, QStringLiteral("custom"), QStringLiteral("SUBTOTAL(109,[Q2])*2")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("n:120"));
        QVERIFY(s->setTableTotal(0, 2, QStringLiteral("custom"), QStringLiteral("MAX(Table1[Q2])-MIN(Table1[Q2])")).ok);
        QVERIFY(e.settle());
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

    // New Table Style: a style of the workbook's own is made from the looks of some parts (one step, taken out again by Undo), used by a table, joins the gallery, and is saved in the file's
    // `tableStyles` after its `dxfs`, with its looks, and read back.
    void aNewTableStyleIsMadeUsedAndSaved()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()}, QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>")))));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVector<jp::office::sheet::Workbook::TableStylePart> parts(2);
        parts[0].part = TablePart::HeaderRow;
        parts[0].format.hasFill = true;
        parts[0].format.fill.pattern = jp::office::sheet::PatternKind::Solid;
        parts[0].format.fill.bg = ColorSpec::fromRgb(0x2E6E8E);
        parts[0].format.fontColor = ColorSpec::fromRgb(0xFFFFFF);
        parts[0].format.bold = true;
        parts[1].part = TablePart::FirstRowStripe;
        parts[1].format.hasFill = true;
        parts[1].format.fill.pattern = jp::office::sheet::PatternKind::Solid;
        parts[1].format.fill.bg = ColorSpec::fromRgb(0xE6EEF3);
        QUndoStack *u = e.win.w.undoStack();
        const int steps = u->count();
        QVERIFY(!s->defineTableStyle(QString(), parts).ok);
        QVERIFY(!s->defineTableStyle(QStringLiteral("TableStyleDark1"), parts).ok);
        QCOMPARE(u->count(), steps);
        // made, and taken out again by Undo, which also takes out the looks it added
        const qsizetype dxfs = e.win.w.session()->workbook()->styles().dxfs.size();
        QVERIFY(s->defineTableStyle(QStringLiteral("Mine"), parts).ok);
        QCOMPARE(u->count(), steps + 1);
        QVERIFY(!s->defineTableStyle(QStringLiteral("mine"), parts).ok);
        QCOMPARE(e.win.w.session()->workbook()->styles().tableStyles.size(), 1);
        u->undo();
        QVERIFY(e.win.w.session()->workbook()->styles().tableStyles.isEmpty());
        QCOMPARE(e.win.w.session()->workbook()->styles().dxfs.size(), dxfs);
        // (the gallery has it only while the style is there)
        {
            QWidget *none = e.win.w.makeTableLooksPicker(nullptr);
            QVERIFY(!none->findChild<QToolButton *>(QStringLiteral("tableLook.Mine")));
            delete none;
        }
        u->redo();
        QCOMPARE(e.win.w.session()->workbook()->styles().tableStyles.size(), 1);
        // the table uses it, and the gallery lists it
        crunch::EditSession::TableMarks marks;
        marks.style = QStringLiteral("Mine");
        marks.rowStripes = true;
        QVERIFY(s->setTableMarks(0, marks).ok);
        QVERIFY(e.settle());
        QWidget *picker = e.win.w.makeTableLooksPicker(nullptr);
        QVERIFY(picker->findChild<QToolButton *>(QStringLiteral("tableLook.Mine")));
        delete picker;
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        const jp::office::sheet::Styles &styles = back.book.styles();
        QCOMPARE(styles.tableStyles.size(), 1);
        QCOMPARE(styles.tableStyles[0].name, QStringLiteral("Mine"));
        QCOMPARE(styles.tableStyles[0].parts.size(), 2);
        const qint32 header = styles.tableStyles[0].parts[0].dxf;
        QVERIFY(header >= 0 && header < styles.dxfs.size());
        QVERIFY(styles.dxfs[header].bold && *styles.dxfs[header].bold);
        QCOMPARE(styles.dxfs[header].fill.bg.rgb, quint32(0x2E6E8E));
        QCOMPARE(styles.dxfs[header].fontColor.rgb, quint32(0xFFFFFF));
        QCOMPARE(back.book.tables().first().styleName, QStringLiteral("Mine"));
        const QByteArray xml = back.book.package().part(back.book.stylesPart());
        QVERIFY(xml.indexOf("<dxfs") >= 0 && xml.indexOf("<dxfs") < xml.indexOf("<tableStyles"));
        QVERIFY(xml.contains("name=\"Mine\"") && xml.contains("type=\"headerRow\""));
    }

    // The question New Table Style asks is answered with a name and looks: the style is made and put on the table the cursor is in, as two steps; no looks, or a taken name, makes nothing.
    void newTableStyleFromTheWindow()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()}, QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>")))));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        e.select(QStringLiteral("B3"));
        QString told;
        e.win.w.setNotifier([&told](const QString &, const QString &text) { told = text; });
        e.win.w.setTableStyleAsker([](QString *name, QVector<jp::office::sheet::Workbook::TableStylePart> *parts) {
            *name = QStringLiteral("Ocean");
            jp::office::sheet::Workbook::TableStylePart p;
            p.part = TablePart::TotalRow;
            p.format.bold = true;
            parts->append(p);
            return true;
        });
        QUndoStack *u = e.win.w.undoStack();
        const int steps = u->count();
        e.win.w.newTableStyle();
        QCOMPARE(u->count(), steps + 2);
        QCOMPARE(s->workbook()->tables().first().styleName, QStringLiteral("Ocean"));
        QVERIFY(told.isEmpty());
        e.win.w.newTableStyle();   // (the name is taken)
        QVERIFY(!told.isEmpty());
        QCOMPARE(u->count(), steps + 2);
        e.win.w.setTableStyleAsker([](QString *name, QVector<jp::office::sheet::Workbook::TableStylePart> *) {
            *name = QStringLiteral("Empty");
            return true;
        });
        told.clear();
        e.win.w.newTableStyle();
        QVERIFY(!told.isEmpty());
        QCOMPARE(u->count(), steps + 2);
    }

    // The looks of a table style keep strike-through and underline: formats that differ in only one of them are two, and the saved file holds both and reads them back.
    void tableStyleLooksKeepStrikeAndUnderline()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()}, QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>")))));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVector<jp::office::sheet::Workbook::TableStylePart> parts(3);
        parts[0].part = TablePart::HeaderRow;
        parts[0].format.bold = true;
        parts[1].part = TablePart::TotalRow;
        parts[1].format.bold = true;
        parts[1].format.strike = true;
        parts[2].part = TablePart::FirstColumn;
        parts[2].format.bold = true;
        parts[2].format.underline = jp::office::sheet::Underline::Double;
        QVERIFY(s->defineTableStyle(QStringLiteral("Lines"), parts).ok);
        const auto &made = s->workbook()->styles().tableStyles.last();
        QCOMPARE(made.parts.size(), 3);
        QVERIFY(made.parts[0].dxf != made.parts[1].dxf && made.parts[0].dxf != made.parts[2].dxf && made.parts[1].dxf != made.parts[2].dxf);
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        const jp::office::sheet::Styles &styles = back.book.styles();
        QCOMPARE(styles.tableStyles.size(), 1);
        const auto &st = styles.tableStyles[0];
        QVERIFY(styles.dxfs[st.parts[1].dxf].strike && *styles.dxfs[st.parts[1].dxf].strike);
        QVERIFY(!styles.dxfs[st.parts[0].dxf].strike && !styles.dxfs[st.parts[0].dxf].underline);
        QVERIFY(styles.dxfs[st.parts[2].dxf].underline && *styles.dxfs[st.parts[2].dxf].underline == jp::office::sheet::Underline::Double);
    }

    // New Table Style through its own dialog: the name typed, a fill chosen in the color dialog, the stripes left out, bold left on; OK makes the style and puts it on the table.
    void newTableStyleThroughItsDialog()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()}, QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>")))));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        e.select(QStringLiteral("B3"));
        bool dialogFound = false;
        QTimer::singleShot(0, &e.win.w, [&] {
            auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dialog || dialog->objectName() != QLatin1String("newTableStyleDialog")) return;
            dialogFound = true;
            dialog->findChild<QLineEdit *>(QStringLiteral("tableStyleName"))->setText(QStringLiteral("Through Dialog"));
            dialog->findChild<QCheckBox *>(QStringLiteral("tableStyleRow.stripe"))->setChecked(false);
            dialog->findChild<QCheckBox *>(QStringLiteral("tableStyleRow.total"))->setChecked(false);
            QTimer::singleShot(0, dialog, [] {
                if (auto *colors = qobject_cast<QColorDialog *>(QApplication::activeModalWidget())) {
                    colors->setCurrentColor(QColor(0x112233));
                    colors->accept();
                }
            });
            dialog->findChild<QToolButton *>(QStringLiteral("tableStyleFill.header"))->click();
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        e.win.w.newTableStyle();
        QVERIFY(dialogFound);
        QCOMPARE(s->workbook()->tables().first().styleName, QStringLiteral("Through Dialog"));
        const jp::office::sheet::Styles &styles = s->workbook()->styles();
        QCOMPARE(styles.tableStyles.size(), 1);
        const auto &parts = styles.tableStyles[0].parts;
        QCOMPARE(parts.size(), 1);   // (the header row: the stripes and the totals were left out, the first column was never on)
        QCOMPARE(int(parts[0].part), int(TablePart::HeaderRow));
        const jp::office::sheet::DifferentialFormat &look = styles.dxfs[parts[0].dxf];
        QCOMPARE(look.fill.bg.rgb, quint32(0x112233));
        QVERIFY(look.bold && *look.bold);
        QCOMPARE(look.fontColor.rgb, quint32(0xFFFFFF));
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

    // A table grows when a row is put in it, and the formulas that name its columns have the cells it has: after a row is put in, a number typed in it, a row is taken out, a column put in before the table,
    // and all of it undone.
    void aStructuredReferenceFollowsATableThatGrowsAndShrinks()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=SUM(Table1[Q1])")).ok);
        QVERIFY(s->type(0, 7, 1, QStringLiteral("=ROWS(Table1[#Data])")).ok);
        QVERIFY(s->type(0, 7, 2, QStringLiteral("=SUM(Table1[[Q1]:[Q2]])")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        QVERIFY(s->insertRows(0, 2, 1).ok);   // inside the table
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("B9")), QStringLiteral("n:4"));
        QCOMPARE(valueOf(e, QStringLiteral("A9")), QStringLiteral("n:6"));
        QVERIFY(s->type(0, 2, 1, QStringLiteral("5")).ok);
        QVERIFY(s->type(0, 2, 2, QStringLiteral("50")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A9")), QStringLiteral("n:11"));
        QCOMPARE(valueOf(e, QStringLiteral("C9")), QStringLiteral("n:121"));
        QVERIFY(s->type(0, 4, 1, QStringLiteral("9")).ok);   // the last row of the table, which was the third
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A9")), QStringLiteral("n:17"));
        QCOMPARE(valueOf(e, QStringLiteral("C9")), QStringLiteral("n:127"));
        QVERIFY(s->deleteRows(0, 1, 1).ok);   // the first row of data
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("B8")), QStringLiteral("n:3"));
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:16"));
        QVERIFY(s->insertColumns(0, 0, 1).ok);   // before the table
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("B8")), QStringLiteral("n:16"));
        QCOMPARE(valueOf(e, QStringLiteral("C8")), QStringLiteral("n:3"));
        for (int i = 0; i < 2; ++i) e.win.w.undoStack()->undo();   // the column, and the row taken out
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A9")), QStringLiteral("n:17"));
        QCOMPARE(valueOf(e, QStringLiteral("B9")), QStringLiteral("n:4"));
        for (int i = 0; i < 4; ++i) e.win.w.undoStack()->undo();   // the number typed in the last row, the two typed in the new one, and the row put in
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        QCOMPARE(valueOf(e, QStringLiteral("B8")), QStringLiteral("n:3"));
        QCOMPARE(valueOf(e, QStringLiteral("C8")), QStringLiteral("n:66"));
    }

    // Round 6: a row put in a table, with the fill of its calculated column that follows it as a step of its own, and a row taken out of it: each is undone and done again whole (the table, the
    // cells, the formulas), over and over, and the steps hold a few kilobytes (the table's places, and no cell).
    void aTableEditAndItsFillAreUndoneAndDoneAgainWhole()
    {
        // (below the table, 100,000 rows that each have a height of their own: what the steps hold stays what the edits changed, not what the sheet has)
        QByteArray sheet = salesRows();
        QString tall;
        for (int r = 10; r <= 100000; ++r) tall += QStringLiteral("<row r=\"%1\" ht=\"%2\" customHeight=\"1\"><c r=\"F%1\"><v>%1</v></c></row>").arg(r).arg(15 + r % 5);
        sheet.replace("</sheetData>", tall.toUtf8() + "</sheetData>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({sheet})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 1, 3, QStringLiteral("=[@Q1]*5")).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=SUM(Table1[Q1])")).ok);
        QVERIFY(e.settle());
        QUndoStack *u = e.win.w.undoStack();
        const int steps = u->count();
        const qint64 base = s->undoBytes();
        auto state = [&] {
            QStringList out;
            const Table &t = s->workbook()->tables().first();
            out << QStringLiteral("table %1:%2").arg(t.row1).arg(t.row2);
            for (const char *ref : {"A1", "A2", "A3", "A4", "A5", "A8", "A9", "B2", "B3", "B4", "C3", "C4", "D2", "D3", "D4", "D5"})
                out << QString::fromLatin1(ref) + QLatin1Char('=') + valueOf(e, QString::fromLatin1(ref)) + QLatin1Char(' ') + editOf(e, QString::fromLatin1(ref));
            return out.join(QLatin1Char('|'));
        };
        const QString start = state();
        QVERIFY(s->insertRows(0, 2, 1).ok);   // inside the table: the row, and the fill
        QVERIFY(e.settle());
        QCOMPARE(u->count(), steps + 2);
        QCOMPARE(editOf(e, QStringLiteral("D3")), QStringLiteral("=Table1[[#This Row],[Q1]]*5"));
        const QString grown = state();
        QVERIFY2(s->undoBytes() - base < 16 * 1024, qPrintable(QString::number(s->undoBytes() - base)));
        QVERIFY(s->deleteRows(0, 1, 1).ok);   // the first row of data
        QVERIFY(e.settle());
        QCOMPARE(u->count(), steps + 3);
        const QString shrunk = state();
        QVERIFY2(s->undoBytes() - base < 16 * 1024, qPrintable(QString::number(s->undoBytes() - base)));
        for (int round = 0; round < 3; ++round) {
            u->undo();
            QVERIFY(e.settle());
            QCOMPARE(state(), grown);
            u->undo();   // the fill
            u->undo();   // the row
            QVERIFY(e.settle());
            QCOMPARE(state(), start);
            u->redo();
            u->redo();
            QVERIFY(e.settle());
            QCOMPARE(state(), grown);
            u->redo();
            QVERIFY(e.settle());
            QCOMPARE(state(), shrunk);
        }
        QVERIFY(e.settle());
    }

    // Round 6: formulas that name a table and one that reads the hidden rows are in a row that is deleted; the table is renamed and made smaller and a row is hidden while they are dead; all of it is undone,
    // and then the rows come back: the formulas read what they read, and a change reaches them (they tell the graph what they read, as those of live rows do).
    void formulasThatNameATableAreRegisteredAgainWhenTheirRowsComeBackAfterTheTableChanged()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->type(0, 7, 0, QStringLiteral("=SUM(Table1[Q1])")).ok);
        QVERIFY(s->type(0, 7, 1, QStringLiteral("=SUBTOTAL(109,B2:B4)")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        QUndoStack *u = e.win.w.undoStack();
        QVERIFY(s->deleteRows(0, 7, 1).ok);
        QVERIFY(s->renameTable(0, QStringLiteral("Sales")).ok);
        QVERIFY(s->resizeTable(0, rangeText(QStringLiteral("A1:C3"))).ok);
        crunch::EditSession::SizeChange hide;
        hide.setHidden = hide.hidden = true;
        QVERIFY(s->resizeRows(0, 2, 2, hide, QStringLiteral("Hide")).ok);
        QVERIFY(e.settle());
        for (int i = 0; i < 3; ++i) u->undo();
        QVERIFY(e.settle());
        u->undo();   // the rows come back
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:6"));
        QCOMPARE(valueOf(e, QStringLiteral("B8")), QStringLiteral("n:6"));
        QVERIFY(s->type(0, 1, 1, QStringLiteral("100")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:105"));
        QCOMPARE(valueOf(e, QStringLiteral("B8")), QStringLiteral("n:105"));
        QVERIFY(s->resizeRows(0, 2, 2, hide, QStringLiteral("Hide")).ok);   // (row 3, 2 in B, is left out of the subtotal now)
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("B8")), QStringLiteral("n:103"));
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:105"));
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
        QCOMPARE(calc->content(0, 3, 3, false).formula, QStringLiteral("Table1[[#This Row],[Q1]]+Table1[[#This Row],[Q2]]"));
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
        QCOMPARE(picker->findChildren<QToolButton *>().size(), 61);   // (the looks, and New Table Style)
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
        // and so does its row of names: a filtered table's names stay until the filter is cleared
        const auto namesOff = w.session()->setTableHeaderRow(0, false);
        QVERIFY(!namesOff.ok && namesOff.problem.contains(QStringLiteral("filtered")));
        QCOMPARE(w.session()->workbook()->tables().first().headerRows, 1);
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

    // The button on a total sits at the right of its cell, and the total (a number, so at the right) stops short of it instead of going under it.
    void aTotalIsNotCoveredByItsButton()
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
        e.select(QStringLiteral("A1"));
        e.view()->show();
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("n:60"));   // (the total of the last column is the one that adds up)
        const QRect button = e.view()->headerButtonRect(4, 2);
        QVERIFY(!button.isEmpty());
        const QImage img = e.view()->viewport()->grab().toImage();
        int dark = 0;
        for (int y = button.top(); y <= button.bottom(); ++y)
            for (int x = button.left() - 3; x < button.left(); ++x)
                if (qGray(img.pixel(x, y)) < 160) ++dark;
        QVERIFY2(dark == 0, qPrintable(QStringLiteral("%1 dark pixels of the total are just left of its button: it goes under the button").arg(dark)));
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
        QCOMPARE(w.session()->calculation()->content(0, 2, 2, false).formula, QStringLiteral("2*Table1[[#This Row],[Q1]]"));
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

    // A table's own header buttons filter by a list, a condition, or the top ones; the table's part keeps each, the rows it hides are saved hidden, and the file opens as it was.
    void aTablesFilterOfEveryKindIsSavedAndOpensAsItWas()
    {
        const QString part = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C5\"><autoFilter ref=\"A1:C5\"/>"
                                           "<tableColumns count=\"3\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/><tableColumn id=\"3\" name=\"Q2\"/></tableColumns>"
                                           "<tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/></table>")
                                 .arg(QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
        QString rows = QStringLiteral("<row r=\"1\">%1%2%3</row>").arg(text(QStringLiteral("A1"), QStringLiteral("Region")), text(QStringLiteral("B1"), QStringLiteral("Q1")), text(QStringLiteral("C1"), QStringLiteral("Q2")));
        const char *names[] = {"East", "West", "North", "South"};
        for (int i = 0; i < 4; ++i)
            rows += QStringLiteral("<row r=\"%1\">%2%3%4</row>").arg(i + 2).arg(text(QStringLiteral("A%1").arg(i + 2), QLatin1String(names[i])), num(QStringLiteral("B%1").arg(i + 2), (i + 1) * 10),
                                                                  num(QStringLiteral("C%1").arg(i + 2), 4 - i));
        EditWindow e;
        QVERIFY(e.open(withTables(buildXlsx({worksheet(rows)}), {part})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        using FC = jp::office::sheet::FilterColumn;
        FC custom;
        custom.kind = FC::Kind::Custom;
        custom.conditions.append({FC::Op::Greater, QStringLiteral("10")});
        QVERIFY(s->setFilter(0, s->filterWith(0, 1, custom, 0), QStringLiteral("Filtering"), 0).ok);
        QVERIFY(s->rowFilteredOut(0, 1));                                                  // (East: 10 is not more than 10)
        QVERIFY(!s->rowFilteredOut(0, 2));
        FC top;
        top.kind = FC::Kind::Top10;
        top.top = true;
        top.count = 2;
        QVERIFY(s->setFilter(0, s->filterWith(0, 2, top, 0), QStringLiteral("Filtering"), 0).ok);
        QVERIFY(s->rowFilteredOut(0, 3) && s->rowFilteredOut(0, 4));                       // (North and South: not among the two highest Q2)
        QVERIFY(!s->rowFilteredOut(0, 2));                                                 // (only West is left)
        QVERIFY2(saveNow(e), "saved");
        jp::office::Package pk;
        QVERIFY(pk.readLazyFile(e.path));
        const QByteArray tablePart = pk.part(QStringLiteral("xl/tables/table1.xml"));
        QVERIFY2(tablePart.contains("<customFilters>") && tablePart.contains("<top10 "), tablePart.constData());
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QVERIFY(second.w.workbookLoader().result() != nullptr);
        QCOMPARE(second.w.session()->tableFilter(0).columns.size(), 2);
        QVERIFY(second.w.session()->tableFilter(0).active());
        QVERIFY(second.w.session()->rowFilteredOut(0, 1) && second.w.session()->rowFilteredOut(0, 3) && second.w.session()->rowFilteredOut(0, 4));
        QVERIFY(!second.w.session()->rowFilteredOut(0, 2));
        QVERIFY(!second.w.session()->calculation()->autoFilterOf(0).valid());
    }

    // The count of the rows a table's filter lets through is in the status bar; and a sort inside the table is kept in the table's part, read back, and made again by Reapply.
    void aTablesFilterShowsItsCountAndItsSortIsKeptInTheFile()
    {
        const QString part = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C4\"><autoFilter ref=\"A1:C4\">"
                                           "<filterColumn colId=\"0\"><filters><filter val=\"East\"/><filter val=\"West\"/></filters></filterColumn></autoFilter><tableColumns count=\"3\">"
                                           "<tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/><tableColumn id=\"3\" name=\"Q2\"/></tableColumns>"
                                           "<tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/></table>")
                                 .arg(QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
        QString rows = QStringLiteral("<row r=\"1\">%1%2%3</row>").arg(text(QStringLiteral("A1"), QStringLiteral("Region")), text(QStringLiteral("B1"), QStringLiteral("Q1")), text(QStringLiteral("C1"), QStringLiteral("Q2")));
        const char *names[] = {"East", "West", "North"};
        for (int i = 0; i < 3; ++i)
            rows += QStringLiteral("<row r=\"%1\"%4>%2%3</row>").arg(i + 2).arg(text(QStringLiteral("A%1").arg(i + 2), QLatin1String(names[i])), num(QStringLiteral("B%1").arg(i + 2), i + 1),
                                                                     i == 2 ? QStringLiteral(" hidden=\"1\"") : QString());
        EditWindow e;
        QVERIFY(e.open(withTables(buildXlsx({worksheet(rows)}), {part})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        e.select(QStringLiteral("B2"));
        QLabel *count = w.findChild<QLabel *>(QStringLiteral("filterCount"));
        QVERIFY(count);
        QTRY_COMPARE(count->text(), QStringLiteral("2 of 3 records found"));
        // a sort of the table's rows by Q1, biggest first, is the table's
        jp::crunch::SortSpec spec;
        jp::crunch::SortLevel level;
        level.key = 1;
        level.descending = true;
        spec.levels.append(level);
        spec.hasHeader = true;
        QVERIFY(!w.session()->savedSort(0, 0).valid);
        const int before = w.undoStack()->count();
        QVERIFY(w.session()->sortRange(0, rangeOf(QStringLiteral("A1:C4")), spec).ok);
        QCOMPARE(w.undoStack()->count(), before + 1);
        QVERIFY(w.session()->savedSort(0, 0).valid);
        QVERIFY(!w.session()->savedSort(0).valid);                                      // (not the sheet's: it has no filter)
        QVERIFY2(saveNow(e), "saved");
        jp::office::Package pk;
        QVERIFY(pk.readLazyFile(e.path));
        const QByteArray tablePart = pk.part(QStringLiteral("xl/tables/table1.xml"));
        QVERIFY2(tablePart.contains("<sortState ref=\"A2:C4\""), tablePart.constData());
        QVERIFY(tablePart.contains("<sortCondition ref=\"B2:B4\" descending=\"1\"/>"));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const crunch::EditSession::SavedSort again = second.w.session()->savedSort(0, 0);
        QVERIFY(again.valid);
        QCOMPARE(rangeName(again.range), QStringLiteral("A1:C4"));
        QCOMPARE(again.spec.levels[0].key, 1);
        QVERIFY(again.spec.levels[0].descending);
        // Reapply on the table sorts it by what the file kept
        QVERIFY(second.w.session()->type(0, 1, 1, QStringLiteral("0")).ok);
        second.w.view()->selectCell(1, 1);
        second.w.act(QStringLiteral("filter.reapply"))->trigger();
        QTRY_COMPARE(second.w.session()->displayText(0, 1, 0), QStringLiteral("East"));
        QCOMPARE(second.w.session()->displayText(0, 2, 0), QStringLiteral("West"));
        QCOMPARE(second.w.session()->displayText(0, 3, 0), QStringLiteral("North"));       // (the row the filter hides stays where it is)
    }

    // Reapply on a table sorts by numbers in text by value when the table's sort was made that way in this window.
    void reapplyOnATableKeepsTheNaturalChoice()
    {
        const QString part = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C4\"><autoFilter ref=\"A1:C4\"/>"
                                           "<tableColumns count=\"3\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/><tableColumn id=\"3\" name=\"Q2\"/></tableColumns>"
                                           "<tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/></table>")
                                 .arg(QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
        QString rows = QStringLiteral("<row r=\"1\">%1%2%3</row>").arg(text(QStringLiteral("A1"), QStringLiteral("Region")), text(QStringLiteral("B1"), QStringLiteral("Q1")), text(QStringLiteral("C1"), QStringLiteral("Q2")));
        const char *names[] = {"Item 10", "Item 2", "Item 1"};
        for (int i = 0; i < 3; ++i)
            rows += QStringLiteral("<row r=\"%1\">%2%3</row>").arg(i + 2).arg(text(QStringLiteral("A%1").arg(i + 2), QLatin1String(names[i])), num(QStringLiteral("B%1").arg(i + 2), i + 1));
        EditWindow e;
        QVERIFY(e.open(withTables(buildXlsx({worksheet(rows)}), {part})));
        CrunchWindow &w = e.win.w;
        QVERIFY(e.settle());
        e.select(QStringLiteral("A2"));
        jp::crunch::SortSpec spec;
        jp::crunch::SortLevel level;
        spec.levels.append(level);
        spec.hasHeader = true;
        spec.natural = true;
        QSignalSpy sorted(&w, &CrunchWindow::sortFinished);
        QVERIFY(w.sortRange(rangeOf(QStringLiteral("A1:C4")), spec));
        QTRY_COMPARE(sorted.count(), 1);
        QCOMPARE(w.session()->displayText(0, 1, 0), QStringLiteral("Item 1"));
        QCOMPARE(w.session()->displayText(0, 3, 0), QStringLiteral("Item 10"));
        QVERIFY(w.keepsNaturalSort());
        QVERIFY(w.session()->type(0, 1, 0, QStringLiteral("Item 100")).ok);
        e.select(QStringLiteral("A2"));
        w.act(QStringLiteral("filter.reapply"))->trigger();
        QTRY_COMPARE(sorted.count(), 2);
        QCOMPARE(w.session()->displayText(0, 1, 0), QStringLiteral("Item 2"));
        QCOMPARE(w.session()->displayText(0, 3, 0), QStringLiteral("Item 100"));
    }

    // The rows a table's filter hid in a file are the filter's (not a person's): Clear shows them, Reapply hides them again, and a row hidden in a table that has no conditions is a person's.
    void rowsHiddenByATablesFilterInAFileAreTheFilters()
    {
        const QString part = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C4\"><autoFilter ref=\"A1:C4\">"
                                           "<filterColumn colId=\"0\"><filters><filter val=\"East\"/><filter val=\"West\"/></filters></filterColumn></autoFilter><tableColumns count=\"3\">"
                                           "<tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/><tableColumn id=\"3\" name=\"Q2\"/></tableColumns>"
                                           "<tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/></table>")
                                 .arg(QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
        auto book = [&](const QString &p, int hiddenRow) {
            QString rows = QStringLiteral("<row r=\"1\">%1%2%3</row>").arg(text(QStringLiteral("A1"), QStringLiteral("Region")), text(QStringLiteral("B1"), QStringLiteral("Q1")), text(QStringLiteral("C1"), QStringLiteral("Q2")));
            const char *names[] = {"East", "West", "North"};
            for (int i = 0; i < 3; ++i)
                rows += QStringLiteral("<row r=\"%1\"%4>%2%3</row>").arg(i + 2).arg(text(QStringLiteral("A%1").arg(i + 2), QLatin1String(names[i])), num(QStringLiteral("B%1").arg(i + 2), i + 1),
                                                                         hiddenRow == i + 2 ? QStringLiteral(" hidden=\"1\"") : QString());
            return withTables(buildXlsx({worksheet(rows)}), {p});
        };
        {
            EditWindow e;
            QVERIFY(e.open(book(part, 4)));
            CrunchWindow &w = e.win.w;
            w.session()->setLocale(kUs);
            QVERIFY(e.settle());
            QVERIFY(w.session()->rowFilteredOut(0, 3));      // (the fourth row: North, which the filter does not let through)
            QVERIFY(!w.session()->rowFilteredOut(0, 1));
            e.select(QStringLiteral("B2"));
            w.act(QStringLiteral("filter.clear"))->trigger();
            QTRY_VERIFY(!w.isFiltering());
            QVERIFY(e.settle());
            QVERIFY(!rowHidden(e, 3));
            QVERIFY(!w.session()->tableFilter(0).active());
            w.undoStack()->undo();
            QVERIFY(rowHidden(e, 3));
            w.undoStack()->redo();
            QVERIFY(!rowHidden(e, 3));
            // Reapply: the conditions were taken off, so nothing is hidden; put the filter back by undoing, then reapply keeps North hidden
            w.undoStack()->undo();
            w.act(QStringLiteral("filter.reapply"))->trigger();
            QTRY_VERIFY(!w.isFiltering());
            QVERIFY(rowHidden(e, 3));
        }
        {
            // no conditions: a hidden row is a person's, and the table's Clear leaves it
            QString plain = part;
            plain.replace(QRegularExpression(QStringLiteral("<filterColumn.*</filterColumn>")), QString());
            EditWindow e;
            QVERIFY(e.open(book(plain, 3)));
            QVERIFY(e.settle());
            QVERIFY(!e.win.w.session()->rowFilteredOut(0, 2));
            QVERIFY(rowHidden(e, 2));
        }
    }

    // Sort from the ribbon with the cursor in a table puts the table's rows of data in order, and nothing else: not its row of names, not its row of totals, not the cells under it.
    void sortInATableSortsItsRowsOfDataOnly()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(w.session()->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(w.session()->setTableTotalsRow(0, true).ok);
        QVERIFY(e.settle());
        QVERIFY(w.session()->type(0, 5, 0, QStringLiteral("under")).ok);   // (a cell right under the row of totals)
        QVERIFY(e.settle());
        e.select(QStringLiteral("B3"));
        w.act(QStringLiteral("sort.descending"))->trigger();
        QTRY_VERIFY(!w.isSorting());
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A1")), QStringLiteral("s:Region"));
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("s:North"));
        QCOMPARE(valueOf(e, QStringLiteral("A3")), QStringLiteral("s:West"));
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("s:East"));
        QCOMPARE(valueOf(e, QStringLiteral("A5")), QStringLiteral("s:Total"));
        QCOMPARE(valueOf(e, QStringLiteral("A6")), QStringLiteral("s:under"));
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("n:60"));
        // a selection of cells in the table is sorted among its rows of data, and one that takes in the names is clipped to the data
        e.view()->selectRange(0, 1, 3, 2);
        w.act(QStringLiteral("sort.ascending"))->trigger();
        QTRY_VERIFY(!w.isSorting());
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("B1")), QStringLiteral("s:Q1"));
        QCOMPARE(valueOf(e, QStringLiteral("B2")), QStringLiteral("n:1"));
        QCOMPARE(valueOf(e, QStringLiteral("B4")), QStringLiteral("n:3"));
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("s:North"));   // (the first column was not in it)
    }

    // A defined name can't take a table's name, in any way a name is made: the dialog's check, a change of name, and names from labels.
    void aDefinedNameCannotTakeATablesName()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->renameTable(0, QStringLiteral("Sales")).ok);
        crunch::EditSession::NameDraft d;
        d.name = QStringLiteral("sales");
        d.text = QStringLiteral("Sheet1!$B$2");
        QVERIFY(!s->nameProblem(d, -1).isEmpty());
        QVERIFY(!s->defineName(d).ok);
        d.name = QStringLiteral("Rate");
        int made = -1;
        QVERIFY(s->defineName(d, &made).ok);
        d.name = QStringLiteral("SALES");
        QVERIFY(!s->changeName(made, d).ok);
        QVERIFY(s->names().at(made).name == QLatin1String("Rate"));
        // names from labels: a label that is a table's name makes none
        QVERIFY(s->type(0, 9, 0, QStringLiteral("Sales")).ok);
        QVERIFY(s->type(0, 9, 1, QStringLiteral("7")).ok);
        int madeFromLabels = 0, skipped = 0;
        CellRange block = rangeText(QStringLiteral("A10:B10"));
        QVERIFY(s->defineNamesFromLabels(0, block, 2, &madeFromLabels, &skipped).ok);
        QCOMPARE(madeFromLabels, 0);
        QVERIFY(skipped >= 1);
    }

    // A header typed over renames the column in the formulas other tables keep for their columns too (and Undo gives the name back), and a defined name that names a table's columns reads
    // the table as it is when the table grows.
    void otherTablesAndNamedRangesFollowATable()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->renameTable(0, QStringLiteral("Sales")).ok);
        QVERIFY(s->type(0, 0, 5, QStringLiteral("Sum")).ok);
        QVERIFY(s->type(0, 1, 5, QStringLiteral("x")).ok);
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("F1:G2")), true, QString()).ok);
        QCOMPARE(s->workbook()->tables().last().columns.size(), 2);
        QVERIFY(s->type(0, 1, 6, QStringLiteral("=SUM(Sales[Q1])")).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().last().info[1].calcFormula, QStringLiteral("SUM(Sales[Q1])"));
        QVERIFY(s->type(0, 0, 1, QStringLiteral("First")).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().last().info[1].calcFormula, QStringLiteral("SUM(Sales[First])"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().last().info[1].calcFormula, QStringLiteral("SUM(Sales[Q1])"));
        QCOMPARE(s->workbook()->tables().first().columns.at(1), QStringLiteral("Q1"));
        // a defined name that is a table's column: what reads the name follows the table when it grows by a row typed under it
        crunch::EditSession::NameDraft d;
        d.name = QStringLiteral("Quarter");
        d.text = QStringLiteral("=Sales[Q1]");
        QVERIFY(s->defineName(d).ok);
        QVERIFY(s->type(0, 9, 0, QStringLiteral("=SUM(Quarter)")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A10")), QStringLiteral("n:6"));
        QVERIFY(s->type(0, 4, 1, QStringLiteral("10")).ok);      // (under the table: it grows)
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables().first().row2), 4);
        QCOMPARE(valueOf(e, QStringLiteral("A10")), QStringLiteral("n:16"));
        e.win.w.undoStack()->undo();                               // (the growth)
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A10")), QStringLiteral("n:6"));
    }

    // A formula typed in an empty column of a table of very many rows fills it a slice at a time (no slice is more than a few thousand cells), as one step of Undo that takes the whole
    // column out and puts it back; a table longer than the old limit of 100,000 rows is filled to its end.
    void aVeryLongCalculatedColumnIsFilledInSlices()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"/>"))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:B250001")), true, QString()).ok);
        QUndoStack *u = e.win.w.undoStack();
        QVERIFY(s->type(0, 1, 1, QStringLiteral("=A2*2")).ok);
        QVERIFY(s->fillPending());
        QVERIFY(s->isSlicing());
        QString why;
        QVERIFY(!s->canEdit(&why));                                   // (nothing else is changed while it goes on)
        QTRY_VERIFY_WITH_TIMEOUT(!s->fillPending(), 120000);
        QVERIFY(s->canEdit());
        QVERIFY(s->slicesPushed() >= 10);
        QVERIFY2(s->longestSlice() <= 20000, qPrintable(QString::number(s->longestSlice())));
        QCOMPARE(editOf(e, QStringLiteral("B250001")), QStringLiteral("=A250001*2"));
        QCOMPARE(editOf(e, QStringLiteral("B100001")), QStringLiteral("=A100001*2"));
        QCOMPARE(s->workbook()->tables().first().info[1].calcFormula, QStringLiteral("A2*2"));
        const int steps = u->count();
        u->undo();                                                    // the fill, in one step
        QCOMPARE(u->count(), steps);
        QCOMPARE(editOf(e, QStringLiteral("B3")), QString());
        QCOMPARE(editOf(e, QStringLiteral("B250001")), QString());
        QCOMPARE(editOf(e, QStringLiteral("B2")), QStringLiteral("=A2*2"));
        QCOMPARE(s->workbook()->tables().first().info[1].calcFormula, QString());
        u->redo();
        QCOMPARE(editOf(e, QStringLiteral("B250001")), QStringLiteral("=A250001*2"));
        QVERIFY(e.settle());
    }

    // Undo and Redo wait while a column is being filled a slice at a time: the step before it is not undone from under it, and one Undo afterward takes back the fill.
    void undoWaitsWhileAColumnIsBeingFilled()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"/>"))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:B6001")), true, QString()).ok);
        QVERIFY(s->type(0, 1, 0, QStringLiteral("7")).ok);
        QUndoStack *u = e.win.w.undoStack();
        QVERIFY(s->type(0, 1, 1, QStringLiteral("=A2*2")).ok);
        QVERIFY(s->isSlicing());
        const int index = u->index();
        pressUndo(&e.win.w);
        QCOMPARE(u->index(), index);
        QCOMPARE(editOf(e, QStringLiteral("A2")), QStringLiteral("7"));
        QTRY_VERIFY_WITH_TIMEOUT(!s->fillPending(), 120000);
        QCOMPARE(editOf(e, QStringLiteral("B6001")), QStringLiteral("=A6001*2"));
        QTRY_VERIFY(e.win.w.act(QStringLiteral("edit.undo"))->isEnabled());
        pressUndo(&e.win.w);
        QCOMPARE(u->index(), index);
        QCOMPARE(editOf(e, QStringLiteral("B6001")), QString());
        QCOMPARE(editOf(e, QStringLiteral("A2")), QStringLiteral("7"));
        QVERIFY(e.settle());
    }

    // A sheet that has tables can be deleted: its tables go with it, what names them on other sheets is #REF! (in the cells, and in the file), and Undo brings the tables and the formulas back.
    void aSheetWithTablesCanBeDeleted()
    {
        const QByteArray two = buildXlsx({salesRows(), worksheet(row(1, formula(QStringLiteral("A1"), QStringLiteral("SUM(Sales[Q1])")) + formula(QStringLiteral("B1"), QStringLiteral("SUM(A2:A3)"))))},
                                         QByteArray(), QString(), QString(), QByteArray(), QStringList{QStringLiteral("Data"), QStringLiteral("Other")});
        EditWindow e;
        QVERIFY(e.open(two));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(s->renameTable(0, QStringLiteral("Sales")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("A1"), 1), QStringLiteral("n:6"));
        const auto gone = s->deleteSheet(0);
        QVERIFY2(gone.ok, qPrintable(gone.problem));
        QVERIFY(e.settle());
        QVERIFY(s->workbook()->tables().isEmpty());
        QCOMPARE(editOf(e, QStringLiteral("A1"), 1), QStringLiteral("=SUM(#REF!)"));
        QCOMPARE(valueOf(e, QStringLiteral("A1"), 1), QStringLiteral("e:#REF!"));
        // Undo puts the sheet back with its table and the formulas that named it
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 1);
        QCOMPARE(s->workbook()->tables().first().name, QStringLiteral("Sales"));
        QCOMPARE(editOf(e, QStringLiteral("A1"), 1), QStringLiteral("=SUM(Sales[Q1])"));
        QCOMPARE(valueOf(e, QStringLiteral("A1"), 1), QStringLiteral("n:6"));
        // deleted again and saved, the file has no table part and the formula that named it has #REF!
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QVERIFY(saveNow(e));
        Saved back;
        QVERIFY(back.open(e.path));
        QVERIFY(back.book.tables().isEmpty());
        QVERIFY(!back.package.hasPart(QStringLiteral("/xl/tables/table1.xml")));
        QCOMPARE(back.formula(0, QStringLiteral("A1")), QStringLiteral("SUM(#REF!)"));   // (the first sheet of the file now)
    }

    // The menu of the cells has the table's commands when the cursor is in a table: rows and columns beside the selection (the table takes the one past its end), taking rows and columns out,
    // the row of totals, and back to cells.
    void theMenuOfCellsHasTheTablesCommands()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(w.session()->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        auto menuAction = [&](const QString &text) -> QAction * {
            Q_EMIT e.view()->contextMenuRequested(QPoint(10, 10));
            QMenu *cells = w.findChild<QMenu *>(QStringLiteral("cellMenu"));
            QMenu *table = cells ? cells->findChild<QMenu *>(QStringLiteral("tableMenu")) : nullptr;
            if (!table) return nullptr;
            for (QAction *a : table->actions())
                if (a->text() == text) return a;
            return nullptr;
        };
        e.select(QStringLiteral("F3"));
        Q_EMIT e.view()->contextMenuRequested(QPoint(10, 10));
        QVERIFY(!w.findChild<QMenu *>(QStringLiteral("cellMenu"))->findChild<QMenu *>(QStringLiteral("tableMenu")));   // (outside a table: none)
        delete w.findChild<QMenu *>(QStringLiteral("cellMenu"));
        e.select(QStringLiteral("B3"));
        QAction *above = menuAction(QStringLiteral("Insert Row Above"));
        QVERIFY(above);
        above->trigger();
        QVERIFY(e.settle());
        QCOMPARE(int(w.session()->workbook()->tables().first().row2), 4);
        QCOMPARE(valueOf(e, QStringLiteral("A3")), QStringLiteral("-"));         // (the new row)
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("s:West"));
        e.select(QStringLiteral("B5"));                                             // (the last row of data)
        QAction *below = menuAction(QStringLiteral("Insert Row Below"));
        QVERIFY(below);
        below->trigger();
        QVERIFY(e.settle());
        QCOMPARE(int(w.session()->workbook()->tables().first().row2), 5);
        e.select(QStringLiteral("C2"));                                             // (the last column)
        QAction *right = menuAction(QStringLiteral("Insert Column Right"));
        QVERIFY(right);
        right->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.session()->workbook()->tables().first().columns.size(), 4);
        QAction *left = menuAction(QStringLiteral("Insert Column Left"));
        QVERIFY(left);
        left->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.session()->workbook()->tables().first().columns.size(), 5);
        e.select(QStringLiteral("B3"));
        menuAction(QStringLiteral("Delete Table Rows"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(int(w.session()->workbook()->tables().first().row2), 4);
        menuAction(QStringLiteral("Delete Table Columns"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.session()->workbook()->tables().first().columns.size(), 4);
        QAction *totals = menuAction(QStringLiteral("Totals Row"));
        QVERIFY(totals);
        totals->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.session()->workbook()->tables().first().totalsRows, 1);
        menuAction(QStringLiteral("Back to Plain Cells"))->trigger();
        QVERIFY(w.session()->workbook()->tables().isEmpty());
    }

    // A table is its ids (docs/jeffcrunch-rowids.md, 4.14): a table that moves, or has a row put in or taken out inside it, makes none of the formulas that name it again;
    // an edit that changes where it begins or ends makes those that name it again, and only those.
    void aTableThatMovesOrHasRowsInsideMakesNoFormulaAgain()
    {
        qint64 again[4] = {};
        {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({salesRows()})));
            crunch::EditSession *s = e.win.w.session();
            s->setLocale(kUs);
            QVERIFY(e.settle());
            QVERIFY(s->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);   // Table1: three rows of data
            QVERIFY(s->type(0, 0, 4, QStringLiteral("X")).ok);
            QVERIFY(s->type(0, 0, 5, QStringLiteral("Y")).ok);
            for (int r = 1; r <= 2; ++r) {
                QVERIFY(s->type(0, quint32(r), 4, QString::number(r * 10)).ok);
                QVERIFY(s->type(0, quint32(r), 5, QString::number(r * 100)).ok);
            }
            QVERIFY(s->makeTable(0, rangeText(QStringLiteral("E1:F3")), true, QString()).ok);   // Table2: two rows
            for (quint32 r = 10; r < 110; ++r) {
                QVERIFY(s->type(0, r, 0, QStringLiteral("=SUM(Table1[Q1])")).ok);
                QVERIFY(s->type(0, r, 1, QStringLiteral("=SUM(Table2[Y])")).ok);
            }
            QVERIFY(e.settle());
            QCOMPARE(valueOf(e, QStringLiteral("A11")), QStringLiteral("n:6"));
            QCOMPARE(valueOf(e, QStringLiteral("B11")), QStringLiteral("n:300"));
            const Calculation *calc = s->calculation();
            auto count = [&](int step, const std::function<void()> &edit) {
                const qint64 before = calc->editStats().tablesRegisteredAgain;
                edit();
                QVERIFY(e.settle());
                again[step] = calc->editStats().tablesRegisteredAgain - before;
            };
            count(0, [&] { QVERIFY(s->insertRows(0, 0, 1).ok); });   // above both: they move
            count(1, [&] { QVERIFY(s->insertRows(0, 3, 1).ok); });   // inside both (between rows of data)
            QCOMPARE(valueOf(e, QStringLiteral("A13")), QStringLiteral("n:6"));
            QCOMPARE(valueOf(e, QStringLiteral("B13")), QStringLiteral("n:300"));
            count(2, [&] { QVERIFY(s->deleteRows(0, 5, 1).ok); });   // the last row of data of Table1, which Table2 does not have
            QCOMPARE(valueOf(e, QStringLiteral("A12")), QStringLiteral("n:3"));
            QCOMPARE(valueOf(e, QStringLiteral("B12")), QStringLiteral("n:300"));
            count(3, [&] { e.win.w.undoStack()->undo(); });          // and brought back
            QCOMPARE(valueOf(e, QStringLiteral("A13")), QStringLiteral("n:6"));
        }
        QCOMPARE(again[0], qint64(0));
        QCOMPARE(again[1], qint64(0));
        QCOMPARE(again[2], qint64(100));                        // (Table1's formulas, and not Table2's)
        QCOMPARE(again[3], qint64(100));
    }

    // A small change to a table does not make the sheet's whole picture again (a row added by typing under it), and a subtotal is worked out again when a row is hidden or shown, not when a row
    // is made taller.
    void smallChangesDoNotRedoWhatTheyDoNotTouch()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({salesRows()})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        QVERIFY(w.session()->makeTable(0, rangeText(QStringLiteral("A1:C4")), true, QString()).ok);
        QVERIFY(w.session()->type(0, 7, 0, QStringLiteral("=SUBTOTAL(109,B2:B4)")).ok);
        QVERIFY(e.settle());
        const int shows = w.sheetShows();
        QVERIFY(w.session()->type(0, 4, 1, QStringLiteral("5")).ok);                 // (under the table: it grows)
        QVERIFY(e.settle());
        QCOMPARE(int(w.session()->workbook()->tables().first().row2), 4);
        QCOMPARE(w.sheetShows(), shows);
        // a row made taller: the subtotal is not told to work again; hidden: it is
        const qint64 before = w.session()->calculation()->registrations();
        crunch::EditSession::SizeChange taller;
        taller.setSize = true;
        taller.size = 30;
        QVERIFY(w.session()->resizeRows(0, 2, 2, taller, QStringLiteral("Row height")).ok);
        QVERIFY(e.settle());
        QCOMPARE(w.session()->calculation()->registrations(), before);
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(w.session()->resizeRows(0, 2, 2, hide, QStringLiteral("Hide rows")).ok);
        QVERIFY(e.settle());
        QVERIFY(w.session()->calculation()->registrations() > before);
        QCOMPARE(valueOf(e, QStringLiteral("A8")), QStringLiteral("n:4"));            // (the rows 1 and 3 and the new one of 5: 1 + 3, as the second row is hidden)
    }

    // ---- Get Data from a file
    // JSON rows become a table at the chosen cell in one step; the cells have their kinds; Refresh reads the file again (one step; the table grows); a changed shape, a missing file, and a failed read change nothing.
    void aJsonFileBecomesATableAndRefreshReadsItAgain()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("rows.json"), "[{\"name\":\"Ann\",\"n\":1.5,\"d\":\"2024-02-29\",\"ok\":true,\"code\":\"007\",\"f\":\"=1+1\"},{\"name\":\"Bob\",\"n\":2,\"d\":\"2023-12-01\",\"ok\":false,\"code\":\"010\",\"f\":\"x\"}]");
        int asked = 0;
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            ++asked;
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->place = QStringLiteral("C2");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(asked, 1);
        QCOMPARE(s->workbook()->tables().size(), 1);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        const Table t = s->workbook()->tables().first();
        QCOMPARE(t.columns, (QStringList{QStringLiteral("name"), QStringLiteral("n"), QStringLiteral("d"), QStringLiteral("ok"), QStringLiteral("code"), QStringLiteral("f")}));
        QCOMPARE(t.row1, 1u);
        QCOMPARE(t.row2, 3u);
        QCOMPARE(t.col1, 2u);
        QVERIFY(t.dataSource.startsWith(QLatin1String("jo-data:1?kind=json")));
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("s:Ann"));
        QCOMPARE(valueOf(e, QStringLiteral("D3")), QStringLiteral("n:1.5"));
        QVERIFY(valueOf(e, QStringLiteral("E3")).startsWith(QLatin1String("n:")));   // a date is a date
        QCOMPARE(valueOf(e, QStringLiteral("F3")), QStringLiteral("b:1"));
        QCOMPARE(valueOf(e, QStringLiteral("G3")), QStringLiteral("s:007"));            // an identifier stays text
        QCOMPARE(valueOf(e, QStringLiteral("H3")), QStringLiteral("s:=1+1"));         // and so does what looks like a formula
        QCOMPARE(valueOf(e, QStringLiteral("A9")), QStringLiteral("s:keep"));
        // Refresh: a new row; the table grows; one step
        e.folder.write(QStringLiteral("rows.json"), "[{\"name\":\"Ann\",\"n\":1.5,\"d\":\"2024-02-29\",\"ok\":true,\"code\":\"007\",\"f\":\"=1+1\"},{\"name\":\"Bob\",\"n\":2,\"d\":\"2023-12-01\",\"ok\":false,\"code\":\"010\",\"f\":\"x\"},{\"name\":\"Cy\",\"n\":3,\"d\":\"2023-01-01\",\"ok\":true,\"code\":\"011\",\"f\":\"y\"}]");
        e.select(QStringLiteral("C3"));
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().row2, 4u);
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("s:Cy"));
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().row2, 3u);
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("-"));
        QCOMPARE(e.win.told.size(), 0);
        // Refresh All with the file shorter: the table shrinks and the cells below it are empty
        e.folder.write(QStringLiteral("rows.json"), "[{\"name\":\"Zed\",\"n\":9,\"d\":\"2020-01-01\",\"ok\":true,\"code\":\"1\",\"f\":\"z\"}]");
        e.win.w.act(QStringLiteral("data.refreshAll"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().row2, 2u);
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("s:Zed"));
        QCOMPARE(valueOf(e, QStringLiteral("C4")), QStringLiteral("-"));
        // a changed shape, then a missing file: said, and nothing changes
        const int steps = e.win.w.undoStack()->count();
        // (the same number of columns with other names)
        e.folder.write(QStringLiteral("rows.json"), "[{\"p\":1,\"q\":1,\"r\":1,\"s\":1,\"t\":1,\"u\":1}]");
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY(e.win.told.last().contains(QLatin1String("have changed")));
        QVERIFY(e.win.told.last().contains(QLatin1String("The file no longer has: name, n, d, ok, code, f.")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("s:Zed"));
        e.win.told.clear();
        e.folder.write(QStringLiteral("rows.json"), "[{\"other\":1}]");
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY(e.win.told.last().contains(QLatin1String("nothing was changed"), Qt::CaseInsensitive));
        QVERIFY(QFile::remove(file));
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 2);
        QVERIFY(e.win.told.last().contains(QLatin1String("not there")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("s:Zed"));
    }

    // Values that begin with =, +, -, or @ arrive as text, when the table is made and again at every Refresh.
    void valuesThatLookLikeFormulasStayTextOnRefresh()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("odd.json"), "[{\"v\":\"=\"},{\"v\":\"+A1\"},{\"v\":\"-A1\"},{\"v\":\"@x\"}]");
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->place = QStringLiteral("A1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        const QStringList first{QStringLiteral("s:="), QStringLiteral("s:+A1"), QStringLiteral("s:-A1"), QStringLiteral("s:@x")};
        for (int i = 0; i < 4; ++i) QCOMPARE(valueOf(e, QStringLiteral("A%1").arg(i + 2)), first[i]);
        e.folder.write(QStringLiteral("odd.json"), "[{\"v\":\"@y\"},{\"v\":\"=\"},{\"v\":\"+B1\"},{\"v\":\"-B1\"},{\"v\":\"=SUM(1)\"}]");
        e.select(QStringLiteral("A2"));
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        const QStringList second{QStringLiteral("s:@y"), QStringLiteral("s:="), QStringLiteral("s:+B1"), QStringLiteral("s:-B1"), QStringLiteral("s:=SUM(1)")};
        for (int i = 0; i < 5; ++i) QCOMPARE(valueOf(e, QStringLiteral("A%1").arg(i + 2)), second[i]);
        QCOMPARE(e.win.told.size(), 0);
    }


    // The cells under the table are asked about before they are replaced; a no leaves everything; a protected sheet, another table's cells, and a web address are refused plainly.
    void getDataAsksBeforeReplacingAndRefusesWhatCannotBe()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(2, text(QStringLiteral("B2"), QStringLiteral("mine"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("a.xml"), "<r><i id=\"1\"><v>x</v></i><i id=\"2\"><v>y</v></i></r>");
        QString place = QStringLiteral("A1");
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("xml");
            r->conn.path = file;
            r->conn.list = QStringLiteral("/r/i");
            r->place = place;
            return true;
        });
        int asks = 0;
        bool answer = false;
        e.win.w.setGetDataReplaceAsker([&](int) {
            ++asks;
            return answer;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(asks, 1);
        QCOMPARE(s->workbook()->tables().size(), 0);
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QCOMPARE(valueOf(e, QStringLiteral("B2")), QStringLiteral("s:mine"));
        answer = true;
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(asks, 2);
        QCOMPARE(s->workbook()->tables().size(), 1);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("n:1"));
        QCOMPARE(valueOf(e, QStringLiteral("B2")), QStringLiteral("s:x"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("B2")), QStringLiteral("s:mine"));
        QCOMPARE(s->workbook()->tables().size(), 0);
        // empty cells: no question
        place = QStringLiteral("E5");
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(asks, 2);
        QCOMPARE(s->workbook()->tables().size(), 1);
        // on the table just made: refused, said, nothing changes
        place = QStringLiteral("E6");
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(e.win.told.last().contains(QLatin1String("another table")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // past the last column
        place = QStringLiteral("XFD1");
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.win.told.last().contains(QLatin1String("no room")));
        // a web address and a path on another computer are never read
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = QStringLiteral("https://example.com/a.json");
            r->place = QStringLiteral("H1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.win.told.last().contains(QLatin1String("on this computer")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // a protected sheet
        SheetProtection locked;
        locked.enabled = true;
        QVERIFY(s->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        place = QStringLiteral("K1");
        const int told = e.win.told.size();
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("xml");
            r->conn.path = file;
            r->conn.list = QStringLiteral("/r/i");
            r->place = place;
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.win.told.size() > told);
        QCOMPARE(s->workbook()->tables().size(), 1);
    }

    // A window closed while its Refresh is reading: the read is stopped, the thread calls only its own copy of the test hook, the progress box is not deleted twice, and nothing is applied or shown.
    void closingTheWindowDuringARefreshIsSafe()
    {
        Folder files;
        const QString file = files.write(QStringLiteral("rows.json"), "[{\"a\":1},{\"a\":2}]");
        struct Probe {
            std::shared_ptr<std::atomic<int>> copies;
            Probe(const Probe &o) : copies(o.copies) { ++*copies; }
            Probe(std::shared_ptr<std::atomic<int>> c) : copies(std::move(c)) {}
        };
        auto copies = std::make_shared<std::atomic<int>>(0);
        for (const bool all : {false, true}) {
            auto *e = new EditWindow;
            QPointer<CrunchWindow> guard(&e->win.w);
            QVERIFY(e->open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
            e->win.w.session()->setLocale(kUs);
            e->win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
                r->conn.kind = QStringLiteral("json");
                r->conn.path = file;
                r->place = QStringLiteral("A1");
                return true;
            });
            e->win.w.act(QStringLiteral("data.fromFile"))->trigger();   // (made here: no ask)
            QVERIFY(e->settle());
            QCOMPARE(e->win.w.session()->workbook()->tables().size(), 1);
            std::atomic<int> held{0}, sawCancel{0};
            Probe probe(copies);
            e->win.w.setGetDataHold([&, probe](const std::atomic<bool> &cancel) {
                ++held;
                for (int i = 0; i < 100000000 && !cancel.load(); ++i) std::this_thread::yield();   // (until the window is gone, which sets it)
                if (cancel.load()) ++sawCancel;
            });
            const int before = copies->load();
            e->select(QStringLiteral("A2"));
            QTimer::singleShot(0, qApp, [&] {
                QVERIFY(QTest::qWaitFor([&] { return held.load() == 1; }, 20000));
                delete e;   // (what closing the last window does)
                e = nullptr;
            });
            e->win.w.act(QString::fromLatin1(all ? "data.refreshAll" : "data.refresh"))->trigger();
            QVERIFY(guard.isNull());
            QCOMPARE(int(held), 1);
            QCOMPARE(int(sawCancel), 1);   // (the read was told to stop)
            QVERIFY(copies->load() > before);   // (the thread called a copy of the hook, not the window's own)
        }
    }

    // A query that came from an opened workbook names a file of the sender's choosing: nothing is read until this person has seen the whole path and said yes; the yes lives in that window only.
    void aQueryFromAnOpenedFileAsksBeforeItReads()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        e.win.w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("rows.json"), "[{\"a\":1,\"b\":\"x\"},{\"a\":2,\"b\":\"y\"}]");
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->place = QStringLiteral("A1");
            return true;
        });
        int asked = 0;
        e.win.w.setGetDataTrustAsker([&](const QStringList &, const QString &) {
            ++asked;
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();   // made here: trusted
        QVERIFY(e.settle());
        e.folder.write(QStringLiteral("rows.json"), "[{\"a\":1,\"b\":\"x\"},{\"a\":2,\"b\":\"y\"},{\"a\":3,\"b\":\"z\"}]");
        e.select(QStringLiteral("A2"));
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(asked, 0);
        QCOMPARE(int(e.win.w.session()->workbook()->tables().first().row2), 3);
        QVERIFY(saveNow(e));
        const QString copy = e.copyOfFile();
        QVERIFY(!copy.isEmpty());

        EditWindow f;
        QVERIFY(f.win.open(copy));
        QVERIFY(f.settle());
        f.win.w.session()->setLocale(kUs);
        QStringList paths;
        QString words;
        bool answer = false;
        int asks = 0;
        std::atomic<int> reads{0};
        f.win.w.setGetDataHold([&](const std::atomic<bool> &) { ++reads; });   // (called by the thread just before a file is read)
        f.win.w.setGetDataTrustAsker([&](const QStringList &p, const QString &w) {
            ++asks;
            paths = p;
            words = w;
            return answer;
        });
        e.folder.write(QStringLiteral("rows.json"), "[{\"a\":1,\"b\":\"x\"}]");   // (the file the query names; the sheet must change if it is read)
        f.select(QStringLiteral("A2"));
        const QString before = f.win.w.session()->workbook()->tables().first().dataSource;
        const int rowsBefore = int(f.win.w.session()->workbook()->tables().first().row2);
        // No: nothing is read, nothing changes, nothing to undo
        f.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(asks, 1);
        QCOMPARE(paths, QStringList{jp::office::sheet::dataReadablePath(file)});
        QVERIFY(words.contains(paths.first()));
        QCOMPARE(f.win.w.undoStack()->count(), 0);
        QCOMPARE(f.win.w.session()->workbook()->tables().first().dataSource, before);
        QCOMPARE(int(f.win.w.session()->workbook()->tables().first().row2), rowsBefore);
        // Refresh All, still no: asks once, still nothing read
        f.win.w.act(QStringLiteral("data.refreshAll"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(asks, 2);
        QCOMPARE(f.win.w.undoStack()->count(), 0);
        QCOMPARE(int(reads), 0);   // (nothing was read)
        // Yes: the file is read, and the window does not ask about it again
        answer = true;
        f.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(asks, 3);
        QCOMPARE(int(reads), 1);
        QCOMPARE(f.win.w.undoStack()->count(), 1);
        QCOMPARE(int(f.win.w.session()->workbook()->tables().first().row2), 1);
        f.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(asks, 3);
        // Undo and Redo of that Refresh put rows back without reading the file again
        e.folder.write(QStringLiteral("rows.json"), "[{\"a\":9,\"b\":\"q\"},{\"a\":8,\"b\":\"r\"},{\"a\":7,\"b\":\"s\"}]");
        QCOMPARE(f.win.w.undoStack()->count(), 2);
        f.win.w.undoStack()->undo();
        f.win.w.undoStack()->undo();
        QVERIFY(f.settle());
        QCOMPARE(int(f.win.w.session()->workbook()->tables().first().row2), rowsBefore);
        f.win.w.undoStack()->redo();
        f.win.w.undoStack()->redo();
        QVERIFY(f.settle());
        QCOMPARE(int(f.win.w.session()->workbook()->tables().first().row2), 1);
        QCOMPARE(asks, 3);
        // the yes was not saved: a window opened again from the same file asks again
        EditWindow g;
        QVERIFY(g.win.open(e.copyOfFile()));
        QVERIFY(g.settle());
        int again = 0;
        g.win.w.setGetDataTrustAsker([&](const QStringList &, const QString &) {
            ++again;
            return false;
        });
        g.select(QStringLiteral("A2"));
        g.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(g.settle());
        QCOMPARE(again, 1);
        QCOMPARE(g.win.w.undoStack()->count(), 0);
    }

    // Refresh All asks once for every file, and lists at most 20 of them with a count of the rest.
    void refreshAllAsksOnceAndListsTwentyFiles()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(900, text(QStringLiteral("A900"), QStringLiteral("keep"))))})));
        e.win.w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        const int files = 23;
        QStringList names;
        for (int i = 0; i < files; ++i) names << e.folder.write(QStringLiteral("f%1.json").arg(i), "[{\"a\":1}]");
        int at = 0;
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = names[at];
            r->place = QStringLiteral("A%1").arg(1 + 4 * at);
            ++at;
            return true;
        });
        for (int i = 0; i < files; ++i) {
            e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
            QVERIFY(e.settle());
        }
        QCOMPARE(e.win.w.session()->workbook()->tables().size(), files);
        QVERIFY(saveNow(e));
        EditWindow f;
        QVERIFY(f.win.open(e.copyOfFile()));
        QVERIFY(f.settle());
        int asks = 0;
        QStringList paths;
        QString words;
        std::atomic<int> reads{0};
        f.win.w.setGetDataHold([&](const std::atomic<bool> &) { ++reads; });
        f.win.w.setGetDataTrustAsker([&](const QStringList &p, const QString &w) {
            ++asks;
            paths = p;
            words = w;
            return false;
        });
        f.win.w.act(QStringLiteral("data.refreshAll"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(asks, 1);
        QCOMPARE(int(paths.size()), files);
        QVERIFY(words.contains(jp::office::sheet::dataReadablePath(names[19])));
        QVERIFY(!words.contains(jp::office::sheet::dataReadablePath(names[20])));
        QVERIFY(words.contains(QStringLiteral("and 3 more")));
        QVERIFY(words.trimmed().endsWith(QStringLiteral("Read the Files reads every file named here, including any that are only counted.")));
        QCOMPARE(f.win.w.undoStack()->count(), 0);
        QCOMPARE(int(reads), 0);
    }

    // What the box shows, what is approved, and what is read are one place: a path with a link and ".." is shown and opened as the system resolves it, not as the text reads.
    void theBoxShowsAndTheReadOpensTheSamePlace()
    {
#ifdef Q_OS_WIN
        QSKIP("on Windows a link made this way is a shortcut file, not a symbolic link, and making a real one needs a right a test machine may not have");
#endif
        Folder tree;
        QVERIFY(QDir(tree.dir.path()).mkpath(QStringLiteral("near")));
        QVERIFY(QDir(tree.dir.path()).mkpath(QStringLiteral("far/deep")));
        const QString farFile = tree.write(QStringLiteral("far/x.json"), "[{\"a\":1},{\"a\":2},{\"a\":3}]");   // (what the system finds through the link)
        tree.write(QStringLiteral("near/x.json"), "[{\"a\":9}]");                                                 // (what the text read lexically would find)
        if (!QFile::link(tree.dir.filePath(QStringLiteral("far/deep")), tree.dir.filePath(QStringLiteral("near/link")))) QSKIP("this system cannot make a symbolic link here");
        const QString tricky = tree.dir.filePath(QStringLiteral("near/link/../x.json"));
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        e.win.w.session()->setLocale(kUs);
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = tricky;
            r->place = QStringLiteral("A1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(int(e.win.w.session()->workbook()->tables().first().row2), 3);   // (the made-here read opens the place the system names: three rows)
        QVERIFY(saveNow(e));
        EditWindow f;
        QVERIFY(f.win.open(e.copyOfFile()));
        QVERIFY(f.settle());
        f.win.w.session()->setLocale(kUs);
        QStringList paths;
        QString words;
        std::atomic<int> reads{0};
        f.win.w.setGetDataHold([&](const std::atomic<bool> &) { ++reads; });
        f.win.w.setGetDataTrustAsker([&](const QStringList &p, const QString &w) {
            paths = p;
            words = w;
            return true;
        });
        tree.write(QStringLiteral("far/x.json"), "[{\"a\":1},{\"a\":2}]");   // (changes the place that is really read: two rows now)
        f.select(QStringLiteral("A2"));
        f.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(paths, QStringList{QFileInfo(farFile).canonicalFilePath()});   // (the place shown is the far one, with no link and no "..")
        QVERIFY(!words.contains(QStringLiteral("..")));
        QCOMPARE(int(reads), 1);
        QCOMPARE(int(f.win.w.session()->workbook()->tables().first().row2), 2);   // (and it is the far one that was read)
    }

    // A path or name that could fake the box is shown as visible codes, never raw; a long path is whole.
    void theBoxShowsOddCharactersAsCodes()
    {
        QCOMPARE(jp::getDataVisibleText(QStringLiteral("a\nb\r\tc")), QStringLiteral("a<U+000A>b<U+000D><U+0009>c"));
        QCOMPARE(jp::getDataVisibleText(QString(QChar(0x202E)) + QStringLiteral("gnp.exe")), QStringLiteral("<U+202E>gnp.exe"));
        QCOMPARE(jp::getDataVisibleText(QString(QChar(0x2028)) + QString(QChar(0x200B)) + QChar(0x2066)), QStringLiteral("<U+2028><U+200B><U+2066>"));
        QCOMPARE(jp::getDataVisibleText(QString(QChar(0x7F)) + QChar(0x01)), QStringLiteral("<U+007F><U+0001>"));
        QCOMPARE(jp::getDataVisibleText(QStringLiteral("C:\\Users\\José\\données 日本.json /tmp/a b")), QStringLiteral("C:\\Users\\José\\données 日本.json /tmp/a b"));
        const QString longPath = QStringLiteral("/") + QString(3000, QLatin1Char('d')) + QStringLiteral("/end.json");
        QCOMPARE(jp::getDataVisibleText(longPath), longPath);   // (whole: nothing is cut where the cut could hide the end)
        // in the box itself: a name with a line break in it
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        e.win.w.session()->setLocale(kUs);
        const QString file = e.folder.write(QStringLiteral("evil\nRead the File.json"), "[{\"a\":1}]");
        if (file.isEmpty()) QSKIP("this system cannot make a file name with a line break");
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->place = QStringLiteral("A1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(saveNow(e));
        EditWindow f;
        QVERIFY(f.win.open(e.copyOfFile()));
        QVERIFY(f.settle());
        QString words;
        f.win.w.setGetDataTrustAsker([&](const QStringList &, const QString &w) {
            words = w;
            return false;
        });
        f.select(QStringLiteral("A2"));
        f.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(f.settle());
        QVERIFY(words.contains(QStringLiteral("evil<U+000A>Read the File.json")));
        QVERIFY(!words.contains(QStringLiteral("evil\n")));
    }

    // The link to the file is saved with the table and read back: after a save and an open, Refresh still works; taking the link off (Inspect) leaves the table.
    void theLinkToTheFileSurvivesASaveAndOpen()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("rows.json"), "[{\"a\":1,\"b\":\"x\"},{\"a\":2,\"b\":\"y\"}]");
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->place = QStringLiteral("A1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(saveNow(e));
        EditWindow f;
        const QString copy = e.copyOfFile();
        QVERIFY(!copy.isEmpty());
        QVERIFY(f.win.open(copy));
        f.win.w.setGetDataTrustAsker([](const QStringList &, const QString &) { return true; });   // (this test is about the saved link, not the ask)
        QVERIFY(f.settle());
        QCOMPARE(f.win.w.session()->workbook()->tables().size(), 1);
        QCOMPARE(f.win.w.session()->workbook()->tables().first().dataSource, e.win.w.session()->workbook()->tables().first().dataSource);
        e.folder.write(QStringLiteral("rows.json"), "[{\"a\":1,\"b\":\"x\"},{\"a\":2,\"b\":\"y\"},{\"a\":3,\"b\":\"z\"}]");
        f.select(QStringLiteral("A2"));
        f.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(f.win.w.session()->workbook()->tables().first().row2, 3u);
        QCOMPARE(f.win.told.size(), 0);
    }
    // ---- Get Data steps (slice 1: the steps, their saved form, Refresh through them, and the list of queries)
    static QString stepsFile(QByteArray tail = QByteArray())
    {
        return QString::fromUtf8("[{\"name\":\"Ann\",\"n\":3,\"d\":\"01/02/2024\",\"amt\":\"10\"},{\"name\":\"bob\",\"n\":10,\"d\":\"15/03/2024\",\"amt\":\"1.234,5\"},{\"name\":\"Cy\",\"n\":1,\"d\":\"09/09/2023\",\"amt\":\"1\"},{\"name\":\"Dee\",\"n\":7,\"d\":\"31/12/2022\",\"amt\":\"2,5\"}") + QString::fromUtf8(tail) + QStringLiteral("]");
    }
    static QVector<jp::office::sheet::DataStep> sampleSteps()
    {
        using S = jp::office::sheet::DataStep;
        QVector<S> steps;
        S amt;
        amt.op = S::Op::Kind;
        amt.col = QStringLiteral("amt");
        amt.kind = QStringLiteral("number");
        amt.decimal = QStringLiteral(",");
        amt.group = QStringLiteral(".");
        S date;
        date.op = S::Op::Kind;
        date.col = QStringLiteral("d");
        date.kind = QStringLiteral("date");
        date.order = QStringLiteral("DMY");
        S filter;
        filter.op = S::Op::Filter;
        filter.col = QStringLiteral("n");
        filter.cmp = QStringLiteral("gt");
        filter.value = QStringLiteral("1");
        S sort;
        sort.op = S::Op::Sort;
        sort.col = QStringLiteral("n");
        sort.descending = true;
        S rename;
        rename.op = S::Op::Rename;
        rename.col = QStringLiteral("name");
        rename.to = QStringLiteral("Who \"&\" <more>");
        S keep;
        keep.op = S::Op::Keep;
        keep.cols = {QStringLiteral("Who \"&\" <more>"), QStringLiteral("n"), QStringLiteral("d"), QStringLiteral("amt")};
        steps << amt << date << filter << sort << rename << keep;
        return steps;
    }
    // Load with the steps, save, close, reopen: the steps come back as they were; Refresh in the reopened workbook runs them on the file as it is now (one step of Undo that puts the cells and the source text back); the time and the row count are kept.
    void aQueryWithStepsLoadsRefreshesAndSurvivesASaveAndOpen()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("people.json"), stepsFile().toUtf8());
        const QVector<DataStep> steps = sampleSteps();
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->conn.steps = steps;
            r->conn.name = QStringLiteral("People");
            r->place = QStringLiteral("A1");
            return true;
        });
        const qint64 before = QDateTime::currentSecsSinceEpoch();
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        const qint64 after = QDateTime::currentSecsSinceEpoch();
        QCOMPARE(s->workbook()->tables().size(), 1);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        const Table t = s->workbook()->tables().first();
        QCOMPARE(t.columns, (QStringList{QStringLiteral("Who \"&\" <more>"), QStringLiteral("n"), QStringLiteral("d"), QStringLiteral("amt")}));
        QCOMPARE(t.row2, 3u);   // (Cy is filtered out: three rows and the names)
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("s:bob"));   // (largest n first)
        QCOMPARE(valueOf(e, QStringLiteral("A3")), QStringLiteral("s:Dee"));
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("s:Ann"));
        QCOMPARE(valueOf(e, QStringLiteral("D2")), QStringLiteral("n:1234.5"));
        QCOMPARE(valueOf(e, QStringLiteral("D3")), QStringLiteral("n:2.5"));
        QVERIFY(valueOf(e, QStringLiteral("C2")).startsWith(QLatin1String("n:")));   // the date is a date
        QVERIFY(t.dataSource.startsWith(QLatin1String("jo-data:2?kind=json")));
        DataConnection conn;
        QVERIFY(DataConnection::parse(t.dataSource, &conn));
        QCOMPARE(conn.steps, steps);
        QCOMPARE(conn.name, QStringLiteral("People"));
        QCOMPARE(conn.rowCount, qint64(3));
        QVERIFY(conn.ranAt >= before && conn.ranAt <= after);
        // save, close, reopen: the query is as it was
        QVERIFY(saveNow(e));
        EditWindow f;
        const QString copy = e.copyOfFile();
        QVERIFY(!copy.isEmpty());
        QVERIFY(f.win.open(copy));
        QVERIFY(f.settle());
        f.win.w.session()->setLocale(kUs);
        QCOMPARE(f.win.w.session()->workbook()->tables().size(), 1);
        const QString saved = f.win.w.session()->workbook()->tables().first().dataSource;
        QCOMPARE(saved, t.dataSource);
        // the file gains a row (n = 20, the largest) and a row that the filter drops; Refresh runs the steps on it
        e.folder.write(QStringLiteral("people.json"), stepsFile(",{\"name\":\"Eve\",\"n\":20,\"d\":\"05/05/2025\",\"amt\":\"7\"},{\"name\":\"Fay\",\"n\":0,\"d\":\"05/05/2025\",\"amt\":\"7\"}").toUtf8());
        f.select(QStringLiteral("A2"));
        f.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(f.win.told.size(), 0);
        QCOMPARE(f.win.w.undoStack()->count(), 1);
        QCOMPARE(f.win.w.session()->workbook()->tables().first().row2, 4u);
        QCOMPARE(valueOf(f, QStringLiteral("A2")), QStringLiteral("s:Eve"));
        QCOMPARE(valueOf(f, QStringLiteral("D2")), QStringLiteral("n:7"));
        DataConnection again;
        QVERIFY(DataConnection::parse(f.win.w.session()->workbook()->tables().first().dataSource, &again));
        QCOMPARE(again.rowCount, qint64(4));
        QCOMPARE(again.steps, steps);
        pressUndo(&f.win.w);
        QVERIFY(f.settle());
        QCOMPARE(f.win.w.session()->workbook()->tables().first().dataSource, saved);
        QCOMPARE(f.win.w.session()->workbook()->tables().first().row2, 3u);
        QCOMPARE(valueOf(f, QStringLiteral("A2")), QStringLiteral("s:bob"));
    }

    // A step that can't run is named, the table keeps its data, and nothing is put on the stack of Undo.
    void aStepThatFailsIsNamedAndTheTableKeepsItsData()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("people.json"), stepsFile().toUtf8());
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->conn.steps = sampleSteps();
            r->place = QStringLiteral("A1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        const QString source = s->workbook()->tables().first().dataSource;
        const int steps = e.win.w.undoStack()->count();
        // a value that is not a number in the column the first step makes a number
        e.folder.write(QStringLiteral("people.json"), stepsFile(",{\"name\":\"Gus\",\"n\":4,\"d\":\"05/05/2025\",\"amt\":\"abc\"}").toUtf8());
        e.select(QStringLiteral("A2"));
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY(e.win.told.last().contains(QLatin1String("Step 1")));
        QVERIFY(e.win.told.last().contains(QLatin1String("\"abc\" in row 5 of \"amt\" is not a number")));
        // a column a step names is gone from the file
        e.folder.write(QStringLiteral("people.json"), "[{\"name\":\"Ann\",\"n\":3,\"d\":\"01/02/2024\"}]");
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 2);
        QVERIFY(e.win.told.last().contains(QLatin1String("Step 1")));
        QVERIFY(e.win.told.last().contains(QLatin1String("The column \"amt\" is not there")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(s->workbook()->tables().first().dataSource, source);
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("s:bob"));
        QCOMPARE(valueOf(e, QStringLiteral("A9")), QStringLiteral("s:keep"));
    }

    // The state column says "newer version" only for a higher version number or an unknown kind of step; a garbled steps list, a text from an old form, and a missing path say "can't be read".
    void theQueryStateTellsNewerFromUnreadable()
    {
        auto stateOf = [&](const QString &source) {
            const QString part = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"Q\" displayName=\"Q\" ref=\"F1:G3\">"
                                               "<tableColumns count=\"2\"><tableColumn id=\"1\" name=\"a\"/><tableColumn id=\"2\" name=\"b\"/></tableColumns><tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/>"
                                               "<extLst><ext uri=\"{6A1F0C3E-5B7D-4E29-9C41-0D3B7A52E8F1}\"><jo:dataSource xmlns:jo=\"urn:jeffoffice:data-source\" text=\"%1\"/></ext></extLst></table>").arg(source.toHtmlEscaped());
            QString rows = QStringLiteral("<row r=\"1\">%1%2</row>").arg(text(QStringLiteral("F1"), QStringLiteral("a")), text(QStringLiteral("G1"), QStringLiteral("b")));
            rows += QStringLiteral("<row r=\"2\">%1%2</row><row r=\"3\">%3%4</row>").arg(num(QStringLiteral("F2"), 1), num(QStringLiteral("G2"), 2), num(QStringLiteral("F3"), 3), num(QStringLiteral("G3"), 4));
            EditWindow e;
            if (!e.open(withTables(buildXlsx({worksheet(rows)}), {part}))) return QStringLiteral("not opened");
            QString state;
            e.win.w.setConnectionsAsker([&](jp::ConnectionsPane *pane) { state = pane->table()->rowCount() == 1 ? pane->table()->item(0, 4)->text() : QStringLiteral("no row"); });
            e.win.w.act(QStringLiteral("data.connections"))->trigger();
            return state;
        };
        const QString base = QStringLiteral("jo-data:2?kind=json&path=%2Ftmp%2Fx.json");
        QCOMPARE(stateOf(QStringLiteral("jo-data:3?kind=json&path=%2Ftmp%2Fx.json")), QStringLiteral("Made by a newer version, can't be run"));
        QCOMPARE(stateOf(base + QStringLiteral("&steps=%5B%7B%22op%22%3A%22warp%22%7D%5D")), QStringLiteral("Steps can't be run: made by a newer version"));
        QCOMPARE(stateOf(base + QStringLiteral("&steps=%5B%7B%22op%22%3A")), QStringLiteral("The steps can't be read, so it can't be run"));
        QCOMPARE(stateOf(base + QStringLiteral("&steps=%5B%7B%22op%22%3A%22filter%22%2C%22col%22%3A%22a%22%2C%22cmp%22%3A%22zz%22%7D%5D")), QStringLiteral("The steps can't be read, so it can't be run"));
        QCOMPARE(stateOf(QStringLiteral("jo-data:2?kind=json")), QStringLiteral("Can't be read, so it can't be run"));
        QCOMPARE(stateOf(QStringLiteral("jo-data:1?garbled")), QStringLiteral("Can't be read, so it can't be run"));
        QCOMPARE(stateOf(base), QStringLiteral("Ready"));
    }

    // The list of queries: name, file, last run, rows, state; Refresh runs the chosen one and the list shows the new count; a query from a newer version is listed, can't be run, and is saved as it was.
    void theConnectionsPaneListsQueriesAndRefreshesThem()
    {
        const QString newer = QStringLiteral("jo-data:3?kind=json&path=%2Ftmp%2Fnowhere.json&future=%E2%9C%93");
        const QString part = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"Future\" displayName=\"Future\" ref=\"F1:G3\">"
                                           "<tableColumns count=\"2\"><tableColumn id=\"1\" name=\"a\"/><tableColumn id=\"2\" name=\"b\"/></tableColumns><tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/>"
                                           "<extLst><ext uri=\"{6A1F0C3E-5B7D-4E29-9C41-0D3B7A52E8F1}\"><jo:dataSource xmlns:jo=\"urn:jeffoffice:data-source\" text=\"%1\"/></ext></extLst></table>").arg(newer.toHtmlEscaped());
        QString rows = QStringLiteral("<row r=\"1\">%1%2</row>").arg(text(QStringLiteral("F1"), QStringLiteral("a")), text(QStringLiteral("G1"), QStringLiteral("b")));
        rows += QStringLiteral("<row r=\"2\">%1%2</row><row r=\"3\">%3%4</row>").arg(num(QStringLiteral("F2"), 1), num(QStringLiteral("G2"), 2), num(QStringLiteral("F3"), 3), num(QStringLiteral("G3"), 4));
        EditWindow e;
        QVERIFY(e.open(withTables(buildXlsx({worksheet(rows)}), {part})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 1);
        const QString file = e.folder.write(QStringLiteral("people.json"), stepsFile().toUtf8());
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->conn.steps = sampleSteps();
            r->conn.name = QStringLiteral("People");
            r->place = QStringLiteral("A1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 2);
        e.folder.write(QStringLiteral("people.json"), stepsFile(",{\"name\":\"Eve\",\"n\":20,\"d\":\"05/05/2025\",\"amt\":\"7\"}").toUtf8());
        int opened = 0;
        e.win.w.setConnectionsAsker([&](jp::ConnectionsPane *pane) {
            ++opened;
            QCOMPARE(pane->queryCount(), 2);
            QTableWidget *list = pane->table();
            QCOMPARE(list->rowCount(), 2);
            // (in the order of the workbook's tables)
            QCOMPARE(list->item(0, 0)->text(), QStringLiteral("Future"));
            QCOMPARE(list->item(0, 4)->text(), QStringLiteral("Made by a newer version, can't be run"));
            QCOMPARE(list->item(1, 0)->text(), QStringLiteral("People"));
            QCOMPARE(list->item(1, 1)->text(), file);
            QCOMPARE(list->item(1, 3)->text(), QStringLiteral("3"));
            QCOMPARE(list->item(1, 4)->text(), QStringLiteral("Ready"));
            list->selectRow(0);
            QVERIFY(!pane->refreshButton()->isEnabled());
            list->selectRow(1);
            QVERIFY(pane->refreshButton()->isEnabled());
            pane->refreshButton()->click();
            QCOMPARE(list->item(1, 3)->text(), QStringLiteral("4"));
            QCOMPARE(pane->tableOfRow(1), 1);
        });
        e.win.w.act(QStringLiteral("data.connections"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(opened, 1);
        QCOMPARE(e.win.told.size(), 0);
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("s:Eve"));
        // saved and opened again, the query from a newer version is as it was
        QVERIFY(saveNow(e));
        EditWindow f;
        const QString copy = e.copyOfFile();
        QVERIFY(f.win.open(copy));
        QVERIFY(f.settle());
        const auto &tables = f.win.w.session()->workbook()->tables();
        QCOMPARE(tables.size(), 2);
        QCOMPARE(tables[0].dataSource, newer);   // as it was, byte for byte
    }

    // A query with steps that came out of an opened workbook is read through the list's Refresh only after the same ask as the ribbon's: Don't Read leaves the table and Undo as they were.
    void theListsRefreshAsksBeforeReadingAnOpenedQuery()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        e.win.w.session()->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("people.json"), stepsFile().toUtf8());
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->conn.steps = sampleSteps();
            r->conn.name = QStringLiteral("People");
            r->place = QStringLiteral("A1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(saveNow(e));
        const QString copy = e.copyOfFile();
        QVERIFY(!copy.isEmpty());
        e.folder.write(QStringLiteral("people.json"), stepsFile(",{\"name\":\"Eve\",\"n\":20,\"d\":\"05/05/2025\",\"amt\":\"7\"}").toUtf8());

        EditWindow f;
        QVERIFY(f.win.open(copy));
        QVERIFY(f.settle());
        f.win.w.session()->setLocale(kUs);
        QStringList paths;
        bool answer = false;
        int asks = 0;
        std::atomic<int> reads{0};
        f.win.w.setGetDataHold([&](const std::atomic<bool> &) { ++reads; });
        f.win.w.setGetDataTrustAsker([&](const QStringList &p, const QString &) {
            ++asks;
            paths = p;
            return answer;
        });
        const QString before = f.win.w.session()->workbook()->tables().first().dataSource;
        int opened = 0;
        f.win.w.setConnectionsAsker([&](jp::ConnectionsPane *pane) {
            ++opened;
            pane->table()->selectRow(0);
            pane->refreshButton()->click();
            QCOMPARE(asks, 1);
            QCOMPARE(paths, QStringList{jp::office::sheet::dataReadablePath(file)});
            QCOMPARE(int(reads), 0);   // (Don't Read: the file was not opened)
            QCOMPARE(f.win.w.undoStack()->count(), 0);
            QCOMPARE(f.win.w.session()->workbook()->tables().first().dataSource, before);
            answer = true;
            pane->refreshButton()->click();
            QCOMPARE(asks, 2);
            QCOMPARE(int(reads), 1);
            QCOMPARE(f.win.w.undoStack()->count(), 1);
        });
        f.win.w.act(QStringLiteral("data.connections"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(opened, 1);
        QCOMPARE(valueOf(f, QStringLiteral("A2")), QStringLiteral("s:Eve"));
    }

    // The window shows the first rows and the kind found for each column before anything is loaded; nothing on the sheet is touched; a web address is refused in words.
    void theGetDataWindowPreviewsTheFirstRows()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile f(dir.filePath(QStringLiteral("p.json")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("{\"meta\":1,\"a\":[{\"x\":1,\"y\":\"2024-01-02\"},{\"x\":2,\"y\":\"2024-01-03\"}],\"b\":[{\"q\":\"t\"}]}");
        f.close();
        jp::GetDataDialog d(QStringLiteral("A1"), nullptr);
        QVERIFY(!d.loadButton()->isEnabled());
        d.setPath(f.fileName());
        QCOMPARE(d.listNames(), (QStringList{QStringLiteral("$.a"), QStringLiteral("$.b")}));
        QCOMPARE(d.previewRows(), 2);
        QCOMPARE(d.previewColumns(), 2);
        QCOMPARE(d.previewCell(1, 0), QStringLiteral("2"));
        QVERIFY(d.loadButton()->isEnabled());
        d.chooseList(QStringLiteral("$.b"));
        QCOMPARE(d.previewColumns(), 1);
        const jp::GetDataRequest req = d.request();
        QCOMPARE(req.conn.kind, QStringLiteral("json"));
        QCOMPARE(req.conn.list, QStringLiteral("$.b"));
        QCOMPARE(req.place, QStringLiteral("A1"));
        d.setPath(QStringLiteral("https://example.com/x.json"));
        QVERIFY(!d.loadButton()->isEnabled());
        QVERIFY(d.message().contains(QLatin1String("on this computer")));
        d.setPath(dir.filePath(QStringLiteral("missing.json")));
        QVERIFY(!d.loadButton()->isEnabled());
        QVERIFY(d.message().contains(QLatin1String("not there")));
    }

    // Another workbook's sheet is loaded with the same window path; Inspect lists the link and removes it (the table stays); a copy in an older form says it can't hold the link.
    void aWorkbookSourceLoadsAndInspectRemovesTheLink()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const QByteArray other = buildXlsx({worksheet(row(1, text(QStringLiteral("A1"), QStringLiteral("Item")) + text(QStringLiteral("B1"), QStringLiteral("Qty"))) + row(2, text(QStringLiteral("A2"), QStringLiteral("Pen")) + num(QStringLiteral("B2"), 3)))},
                                           QByteArray(), QString(), QString(), QByteArray(), QStringList{QStringLiteral("Stock")});
        const QString file = e.folder.write(QStringLiteral("other.xlsx"), other);
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("workbook");
            r->conn.path = file;
            r->conn.list = QStringLiteral("sheet:Stock");
            r->place = QStringLiteral("D1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 1);
        QCOMPARE(valueOf(e, QStringLiteral("D1")), QStringLiteral("s:Item"));
        QCOMPARE(valueOf(e, QStringLiteral("E2")), QStringLiteral("n:3"));
        QVERIFY(jp::office::sheet::featuresNotInCopy(*s->workbook(), jp::office::sheet::CopyKind::Ods).join(QLatin1Char('|')).contains(QStringLiteral("links from tables to the files")));
        InspectItem item;
        for (const InspectItem &i : e.win.w.inspectItems())
            if (i.title == QLatin1String("Links to data files")) item = i;
        QCOMPARE(item.count(), 1);
        QCOMPARE(item.remove(), 1);
        QVERIFY(e.settle());
        QCOMPARE(item.count(), 0);
        QCOMPARE(s->workbook()->tables().size(), 1);
        QVERIFY(s->workbook()->tables().first().dataSource.isEmpty());
        QCOMPARE(valueOf(e, QStringLiteral("D1")), QStringLiteral("s:Item"));
        // Refresh says why it can't, and does nothing
        const int steps = e.win.w.undoStack()->count();
        e.select(QStringLiteral("D2"));
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.win.told.last().contains(QLatin1String("made by Get Data")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(item.count(), 0 + 1);
    }
    // Cancel while the file is read changes nothing and leaves no step of Undo, for a load, a Refresh, and a Refresh All. The worker is held (a counted wait for the flag Cancel sets) until the test presses Cancel.
    void cancelWhileTheFileIsReadChangesNothing()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(9, text(QStringLiteral("A9"), QStringLiteral("keep"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("rows.json"), "[{\"a\":1},{\"a\":2}]");
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->place = QStringLiteral("C2");
            return true;
        });
        int held = 0, released = 0;
        bool hold = true;
        e.win.w.setGetDataHold([&](const std::atomic<bool> &cancel) {
            if (!hold) return;
            ++held;
            for (int i = 0; i < 100000000 && !cancel.load(); ++i) std::this_thread::yield();
            if (cancel.load()) ++released;
        });
        auto pressCancel = [&] {
            QTimer::singleShot(0, &e.win.w, [&] {
                if (QProgressDialog *d = e.win.w.findChild<QProgressDialog *>(QStringLiteral("getDataProgress"))) Q_EMIT d->canceled();
            });
        };
        // a load
        pressCancel();
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(held, 1);
        QCOMPARE(released, 1);   // (the worker saw the flag)
        QCOMPARE(s->workbook()->tables().size(), 0);
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QCOMPARE(e.win.told.size(), 0);
        QCOMPARE(valueOf(e, QStringLiteral("C2")), QStringLiteral("-"));
        // a load that is not cancelled, then a Refresh that is
        hold = false;
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().size(), 1);
        const int steps = e.win.w.undoStack()->count();
        e.folder.write(QStringLiteral("rows.json"), "[{\"a\":7},{\"a\":8},{\"a\":9}]");
        hold = true;
        e.select(QStringLiteral("C3"));
        pressCancel();
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(held, 2);
        QCOMPARE(released, 2);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(int(s->workbook()->tables().first().row2), 3);
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("n:1"));
        QCOMPARE(e.win.told.size(), 1);
        QCOMPARE(e.win.told.last(), QStringLiteral("Refresh: The read was stopped. Nothing was changed."));
        // and a Refresh All
        e.win.told.clear();
        pressCancel();
        e.win.w.act(QStringLiteral("data.refreshAll"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(held, 3);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("n:1"));
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("-"));
        // not held and not cancelled: the same Refresh All reads the file
        hold = false;
        e.win.told.clear();
        e.win.w.act(QStringLiteral("data.refreshAll"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("C5")), QStringLiteral("n:9"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
    }

    // Refresh All over three tables, one of whose files is gone: the other two are read, the missing one is said, and the two are one step of Undo.
    void refreshAllReadsEveryTableAndSaysWhichFileIsGone()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(20, text(QStringLiteral("A20"), QStringLiteral("keep"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const char *places[] = {"A1", "D1", "G1"};
        QString files[3];
        QString place;
        QString current;
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = current;
            r->place = place;
            return true;
        });
        for (int i = 0; i < 3; ++i) {
            files[i] = e.folder.write(QStringLiteral("t%1.json").arg(i), "[{\"v\":1},{\"v\":2}]");
            current = files[i];
            place = QString::fromLatin1(places[i]);
            e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
            QVERIFY(e.settle());
        }
        QCOMPARE(s->workbook()->tables().size(), 3);
        const QString second = s->workbook()->tables()[1].name;
        QVERIFY(QFile::remove(files[1]));
        e.folder.write(QStringLiteral("t0.json"), "[{\"v\":5},{\"v\":6},{\"v\":7}]");
        e.folder.write(QStringLiteral("t2.json"), "[{\"v\":8},{\"v\":9},{\"v\":10},{\"v\":11}]");
        const int steps = e.win.w.undoStack()->count();
        e.win.told.clear();
        e.win.w.act(QStringLiteral("data.refreshAll"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables()[0].row2), 3);
        QCOMPARE(int(s->workbook()->tables()[1].row2), 2);
        QCOMPARE(int(s->workbook()->tables()[2].row2), 4);
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("n:7"));
        QCOMPARE(valueOf(e, QStringLiteral("G5")), QStringLiteral("n:11"));
        QCOMPARE(valueOf(e, QStringLiteral("D3")), QStringLiteral("n:2"));   // (the missing one is as it was)
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY(e.win.told.last().contains(second) && e.win.told.last().contains(QLatin1String("not there")));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);   // one step for both
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(int(s->workbook()->tables()[0].row2), 2);
        QCOMPARE(int(s->workbook()->tables()[2].row2), 2);
        QCOMPARE(valueOf(e, QStringLiteral("A4")), QStringLiteral("-"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        // when none can be read nothing is put on the stack
        QVERIFY(QFile::remove(files[0]));
        QVERIFY(QFile::remove(files[2]));
        e.win.told.clear();
        e.win.w.act(QStringLiteral("data.refreshAll"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 1);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
    }

    // A column the person added beside the table stays through a Refresh with more rows and with fewer; a formula in it is filled into the new rows. A change in the file's own columns refuses, and says which.
    void aColumnAddedBesideTheTableSurvivesARefresh()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(row(20, text(QStringLiteral("A20"), QStringLiteral("keep"))))})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(kUs);
        QVERIFY(e.settle());
        const QString file = e.folder.write(QStringLiteral("r.json"), "[{\"a\":1,\"b\":\"x\"},{\"a\":2,\"b\":\"y\"}]");
        e.win.w.setGetDataAsker([&](jp::GetDataRequest *r) {
            r->conn.kind = QStringLiteral("json");
            r->conn.path = file;
            r->place = QStringLiteral("A1");
            return true;
        });
        e.win.w.act(QStringLiteral("data.fromFile"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(s->type(0, 1, 2, QStringLiteral("=[@a]*10")).ok);   // (a calculated column beside the table)
        QVERIFY(s->type(0, 0, 3, QStringLiteral("note")).ok);       // (and a column of notes)
        QVERIFY(s->type(0, 2, 3, QStringLiteral("mine")).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->tables().first().columns.size(), 4);
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("n:20"));
        // more rows
        e.folder.write(QStringLiteral("r.json"), "[{\"a\":1,\"b\":\"x\"},{\"a\":2,\"b\":\"y\"},{\"a\":3,\"b\":\"z\"}]");
        e.select(QStringLiteral("A2"));
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 0);
        QCOMPARE(int(s->workbook()->tables().first().row2), 3);
        QCOMPARE(s->workbook()->tables().first().columns.size(), 4);
        QCOMPARE(valueOf(e, QStringLiteral("B4")), QStringLiteral("s:z"));
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("n:20"));
        QCOMPARE(valueOf(e, QStringLiteral("C4")), QStringLiteral("n:30"));   // (the formula, filled into the new row)
        QCOMPARE(valueOf(e, QStringLiteral("D3")), QStringLiteral("s:mine"));   // (what was typed stays)
        // fewer rows
        e.folder.write(QStringLiteral("r.json"), "[{\"a\":4,\"b\":\"w\"}]");
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 0);
        QCOMPARE(int(s->workbook()->tables().first().row2), 1);
        QCOMPARE(valueOf(e, QStringLiteral("A2")), QStringLiteral("n:4"));
        QCOMPARE(valueOf(e, QStringLiteral("C2")), QStringLiteral("n:40"));
        QCOMPARE(valueOf(e, QStringLiteral("C3")), QStringLiteral("-"));
        QCOMPARE(valueOf(e, QStringLiteral("D3")), QStringLiteral("-"));
        // the file's own columns changed: refused, the missing and the new ones named, nothing changes
        const int steps = e.win.w.undoStack()->count();
        e.folder.write(QStringLiteral("r.json"), "[{\"a\":4,\"z\":\"w\"},{\"a\":5,\"z\":\"q\"}]");
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY(e.win.told.last().contains(QLatin1String("no longer has: b")));
        QVERIFY(e.win.told.last().contains(QLatin1String("now also has: z")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(int(s->workbook()->tables().first().row2), 1);
        // the person's own column is not the file's: a column of the file with its name must come from the file
        e.win.told.clear();
        e.folder.write(QStringLiteral("r.json"), "[{\"a\":4,\"b\":\"w\",\"note\":\"n\"}]");
        e.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY(e.win.told.last().contains(QLatin1String("now also has: note")));
        // the link and the remembered columns survive a save and an open
        e.folder.write(QStringLiteral("r.json"), "[{\"a\":6,\"b\":\"s\"},{\"a\":7,\"b\":\"t\"}]");
        QVERIFY(saveNow(e));
        EditWindow f;
        const QString copy = e.copyOfFile();
        QVERIFY(f.win.open(copy));
        f.win.w.setGetDataTrustAsker([](const QStringList &, const QString &) { return true; });   // (this test is about the saved link, not the ask)
        QVERIFY(f.settle());
        f.select(QStringLiteral("A2"));
        f.win.w.act(QStringLiteral("data.refresh"))->trigger();
        QVERIFY(f.settle());
        QCOMPARE(f.win.told.size(), 0);
        QCOMPARE(int(f.win.w.session()->workbook()->tables().first().row2), 2);
        QCOMPARE(f.win.w.session()->workbook()->tables().first().columns.size(), 4);
    }

    // Refresh All stays one click on the Data tab, beside Refresh (the pivot tables are refreshed by it too).
    void theDataTabHasRefreshAllAsOneClick()
    {
        QFile file(QStringLiteral(JC_SOURCE_DIR "/apps/crunch/resources/ribbon.json"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
        QStringList large;
        for (const QJsonValue &tab : root.value(QStringLiteral("tabs")).toArray()) {
            if (tab.toObject().value(QStringLiteral("name")).toString() != QLatin1String("Data")) continue;
            for (const QJsonValue &group : tab.toObject().value(QStringLiteral("groups")).toArray())
                if (group.toObject().value(QStringLiteral("name")).toString() == QLatin1String("Refresh"))
                    for (const QJsonValue &item : group.toObject().value(QStringLiteral("items")).toArray()) large << item.toObject().value(QStringLiteral("large")).toString();
        }
        QCOMPARE(large, (QStringList{QStringLiteral("data.refreshAll"), QStringLiteral("data.refresh")}));
    }

    // Pictures of the Get Data window for the person who looks at it (skipped unless the folder is named).
    void getDataWindowPictures()
    {
        const QString folder = qEnvironmentVariable("JO_GETDATA_SHOT_DIR");
        if (folder.isEmpty()) QSKIP("no folder for pictures");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile f(dir.filePath(QStringLiteral("orders.json")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("{\"orders\":[{\"id\":1001,\"customer\":{\"name\":\"Ann Lee\",\"city\":\"Oslo\"},\"placed\":\"2026-11-02\",\"paid\":true,\"tags\":[\"rush\",\"gift\"]},"
                "{\"id\":1002,\"customer\":{\"name\":\"Bo Park\",\"city\":\"Lima\"},\"placed\":\"2026-11-05\",\"paid\":false,\"tags\":[]}],\"other\":[{\"a\":1}]}");
        f.close();
        jp::applyUiTheme(1);
        jp::GetDataDialog d(QStringLiteral("B2"), nullptr);
        d.resize(760, 520);
        d.show();
        d.setPath(f.fileName());
        QVERIFY(d.grab().save(QDir(folder).filePath(QStringLiteral("getdata-dialog.png"))));
        d.setPath(QStringLiteral("https://example.com/orders.json"));
        QVERIFY(d.grab().save(QDir(folder).filePath(QStringLiteral("getdata-refused.png"))));
    }
};

JP_TEST_CLASS(CrunchTablesTests)
#include "tst_crunch_tables.moc"
