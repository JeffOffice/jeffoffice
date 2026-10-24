// Tests of rows, columns, and cells inserted and deleted in an open workbook: the cells and what refers to them move, the values the window holds move with them, one step undoes it, and the
// keys and menus ask for it. Every workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

QString formulaOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.win.w.session()->editText(0, r, c);
}

} // namespace

class CrunchCellsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // A row inserted at the selected row: the cells below move down, the formulas that read them follow, a range with the row inside grows, the values the window holds move with their cells
    // (before anything is worked out again), and one step of Undo puts everything back.
    void insertingARowMovesTheCellsAndUndoPutsThemBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1==SUM(A1:A3)", "B2==A3*2", "C3==B1+B2"})));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:6"));
        QAction *insert = e.win.w.act(QStringLiteral("cells.insert"));
        QVERIFY(insert && insert->isEnabled());
        e.view()->selectRows(1, 1);                                              // row 2
        insert->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Inserting a row"));
        QVERIFY(e.win.w.isModified());
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:3"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=SUM(A1:A4)"));
        QCOMPARE(formulaOf(e, QStringLiteral("B3")), QStringLiteral("=A4*2"));
        QCOMPARE(formulaOf(e, QStringLiteral("C4")), QStringLiteral("=B1+B3"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:6"));
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:6"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:12"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("-"));
        // a value typed under the new row changes what reads it
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("10"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:16"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:22"));
        pressUndo(&e.win.w);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->index(), 0);
        QVERIFY(!e.win.w.isModified());
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:2"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=SUM(A1:A3)"));
        QCOMPARE(formulaOf(e, QStringLiteral("B2")), QStringLiteral("=A3*2"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:12"));
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:12"));
    }

    // Rows and columns deleted: what read them has #REF!, and Undo gives the cells and the formulas back.
    void deletingRowsAndColumnsMakesRefAndUndoGivesThemBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1==A2*10", "C1==SUM(A1:A3)", "C2==B1+1", "D3==A3"})));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:6"));
        e.view()->selectRows(1, 1);
        e.win.w.act(QStringLiteral("cells.delete"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Deleting a row"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=#REF!*10"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("e:#REF!"));
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=SUM(A1:A2)"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("-"));         // (C2 was in the deleted row)
        QCOMPARE(formulaOf(e, QStringLiteral("D2")), QStringLiteral("=A2"));     // (D3 moved up to D2)
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=A2*10"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:20"));
        QCOMPARE(formulaOf(e, QStringLiteral("C2")), QStringLiteral("=B1+1"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:21"));
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:2"));
        // a column
        e.view()->selectColumns(1, 1);                                           // column B
        e.win.w.act(QStringLiteral("cells.delete"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Deleting a column"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=SUM(A1:A3)"));   // (C1 moved left)
        QCOMPARE(formulaOf(e, QStringLiteral("B2")), QStringLiteral("=#REF!+1"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("C2")), QStringLiteral("=B1+1"));
        QVERIFY(!e.win.w.isModified());
    }

    // Cells inserted and deleted, shifted down, right, left, or up: asked of the person, who can also choose whole rows or columns.
    void cellsAreShiftedTheWayThePersonChooses()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1=10", "B2=20", "C1==SUM(A1:A3)", "C2==B2"})));
        using Way = CrunchWindow::ShiftRequest::Way;
        Way answer = Way::Down;
        bool inserting = true;
        e.win.w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            inserting = r->inserting;
            r->way = answer;
            return true;
        });
        e.view()->selectRange(1, 0, 1, 0);                                       // A2
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(inserting);
        QCOMPARE(shown(e, QStringLiteral("A1:A4")), QStringLiteral("1||2|3"));
        QCOMPARE(shown(e, QStringLiteral("B1:B2")), QStringLiteral("10|20"));    // (the other columns stay)
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=SUM(A1:A4)"));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Inserting cells"));
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("1|2|3"));
        // right
        answer = Way::Right;
        e.view()->selectRange(0, 1, 0, 1);                                       // B1
        e.win.w.act(QStringLiteral("cells.insert.cells"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("1||10|6"));
        QCOMPARE(formulaOf(e, QStringLiteral("D1")), QStringLiteral("=SUM(A1:A3)"));
        pressUndo(&e.win.w);
        // delete: up and left
        answer = Way::Up;
        e.view()->selectRange(0, 0, 0, 0);                                       // A1
        e.win.w.act(QStringLiteral("cells.delete.cells"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(!inserting);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("2|3|"));
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=SUM(A1:A2)"));
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("1|2|3"));
        answer = Way::Left;
        e.view()->selectRange(0, 0, 1, 0);                                       // A1:A2
        e.win.w.act(QStringLiteral("cells.delete.cells"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:B2")), QStringLiteral("10|33|20|20"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=SUM(A1:A3)"));   // (a range across other rows stays as it is)
        QCOMPARE(formulaOf(e, QStringLiteral("B2")), QStringLiteral("=A2"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=SUM(A1:A3)"));
        // a whole row chosen in the question
        answer = Way::Rows;
        e.view()->selectRange(1, 0, 1, 0);
        e.win.w.act(QStringLiteral("cells.delete.cells"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Deleting a row"));
        QCOMPARE(shown(e, QStringLiteral("A1:A2")), QStringLiteral("1|3"));
        // giving it up changes nothing
        const int steps = e.win.w.undoStack()->count();
        e.win.w.setShiftAsker([](CrunchWindow::ShiftRequest *) { return false; });
        e.win.w.act(QStringLiteral("cells.insert.cells"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }

    // Ctrl++ and Ctrl+- (the keys the action has) insert and delete the selected rows or columns; the right-click menu of a header has them.
    void theKeysAndTheMenusAskForIt()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1==A2"})));
        QAction *insert = e.win.w.act(QStringLiteral("cells.insert")), *del = e.win.w.act(QStringLiteral("cells.delete"));
        QCOMPARE(insert->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_Plus).toString(QKeySequence::NativeText));
        QCOMPARE(del->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_Minus).toString(QKeySequence::NativeText));
        e.view()->selectRows(0, 1);                                              // rows 1 and 2
        const QKeyCombination plus = insert->shortcut()[0];
        QTest::keyClick(e.view(), plus.key(), plus.keyboardModifiers());
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Inserting rows"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:1"));
        QCOMPARE(formulaOf(e, QStringLiteral("B3")), QStringLiteral("=A4"));
        const QKeyCombination minus = del->shortcut()[0];
        QTest::keyClick(e.view(), minus.key(), minus.keyboardModifiers());
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Deleting rows"));
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        // a column
        e.view()->selectColumns(0, 0);
        QTest::keyClick(e.view(), plus.key(), plus.keyboardModifiers());
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Inserting a column"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:1"));
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=B2"));
        // the menu of a header: a right click on the number of a row
        const QRect cell = e.view()->cellRect(2, 1);
        const QPoint header(e.view()->viewport()->x() + 10, cell.center().y());
        QTest::mouseClick(e.view()->viewport(), Qt::RightButton, Qt::NoModifier, QPoint(10, cell.center().y()));
        QMenu *menu = e.win.w.findChild<QMenu *>(QStringLiteral("cellMenu"));
        QVERIFY(menu);
        QStringList texts;
        for (QAction *a : menu->actions()) texts << a->text();
        QVERIFY2(texts.contains(QStringLiteral("Insert Sheet Rows")) && texts.contains(QStringLiteral("Delete Sheet Rows")), qPrintable(texts.join('|')));
        QVERIFY(e.view()->selection().whole == grid::Selection::Whole::Rows);
        QCOMPARE(e.view()->selection().range.row1, qint64(2));
        menu->close();
        Q_UNUSED(header);
    }

    // What cannot be done is not: cutting through an array formula, a protected sheet; the person is told, and nothing changes.
    void whatCannotBeDoneIsRefused()
    {
        EditWindow e;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">A1:A3*2</f><v>2</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><v>4</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"C3\"><v>6</v></c></row>");
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        e.view()->selectRows(1, 1);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("array")), qPrintable(e.win.told.join('|')));
        // above the array it moves whole
        e.view()->selectRows(0, 0);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:6"));
        // and it still works out from what it reads
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("10"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:20"));
    }

    // Results worked out before cells moved name them by where they were; they are put where the cells are now (and dropped with the cells that are gone).
    void resultsMadeBeforeTheCellsMovedAreMovedToo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1"})));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->generation(), 0);
        auto item = [](quint32 row, quint32 col, double v) {
            grid::ValueStore::Item it;
            it.sheet = 0;
            it.row = row;
            it.col = col;
            it.value = calc::Value::number(v);
            return it;
        };
        QVERIFY(s->insertRows(0, 2, 3).ok);                                      // 3 rows at row 3
        QVERIFY(s->deleteColumns(0, 5, 1).ok);                                   // column F
        QCOMPARE(s->generation(), 2);
        std::vector<grid::ValueStore::Item> items{item(0, 1, 1), item(1, 1, 2), item(2, 1, 3), item(10, 1, 4), item(1, 5, 5), item(1, 6, 6)};
        s->moveToNow(&items, 0);
        QCOMPARE(int(items.size()), 5);
        QCOMPARE(items[0].row, quint32(0));
        QCOMPARE(items[2].row, quint32(5));
        QCOMPARE(items[3].row, quint32(13));
        QCOMPARE(items[4].col, quint32(5));                                       // (G moved to F)
        // made after the first edit: only the second applies
        std::vector<grid::ValueStore::Item> later{item(10, 1, 4), item(1, 6, 6)};
        s->moveToNow(&later, 1);
        QCOMPARE(later[0].row, quint32(10));
        QCOMPARE(later[1].col, quint32(5));
        QVERIFY(e.settle());
        // the generation is the loader's: results worked out now carry it
        QCOMPARE(e.win.w.workbookLoader().structureGeneration(), 2);
    }
};

JP_TEST_CLASS(CrunchCellsTests)
#include "tst_crunch_cells.moc"
