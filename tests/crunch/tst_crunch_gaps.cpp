// Tests of the everyday editing gaps closed in round A: array formulas entered with Ctrl+Shift+Enter, selecting more than one range, the status bar's words, and notes' boxes. Every workbook is built here
#include "app/celleditor.h"
#include "app/crunchwindow.h"
#include "app/anchordialogs.h"
#include "app/finddialog.h"
#include "core/editsession.h"
#include "core/printjob.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "office/package.h"
#include "savedok.h"

#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

QByteArray savedSheet(const QString &path)
{
    jp::office::Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return QByteArray();
    return p.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
}

// The middle of a cell, in the grid's viewport.
QPoint middleOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.view()->sheet().layout().cellRect(r, c).center();
}

void clickCell(EditWindow &e, const QString &ref, Qt::KeyboardModifiers m = Qt::NoModifier) { QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, m, middleOf(e, ref)); }

}   // namespace

class CrunchGapTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Ctrl+Shift+Enter over a selected block makes one array formula for it: shown in braces in the bar for each of its cells, worked out for the block, one step of Undo.
    void ctrlShiftEnterMakesOneArrayOverTheSelection()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:6"));
        const auto made = e.win.w.session()->arrayAt(0, 1, 2);
        QVERIFY(made.valid);
        QCOMPARE(int(made.row1), 0);
        QCOMPARE(int(made.row2), 2);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("C1:C3"));
        for (const char *ref : {"C1", "C2", "C3"}) {
            e.select(QString::fromLatin1(ref));
            QCOMPARE(e.view()->activeCellText(), QStringLiteral("{=A1:A3*2}"));
        }
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("-"));
        QVERIFY(!e.win.w.session()->arrayAt(0, 1, 2).valid);
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:6"));
        QVERIFY(e.win.w.session()->arrayAt(0, 2, 2).valid);
    }

    // An array is edited as a whole from any of its cells (Enter alone says part of it can't be changed), and deleted as a whole; part of it is refused.
    void anArrayIsEditedAndDeletedAsAWhole()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        // part of it can't be typed over or cleared
        e.select(QStringLiteral("C2"));
        e.enter(QStringLiteral("5"));
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("part of an array")), qPrintable(e.win.told.join('|')));
        e.key(Qt::Key_Escape);
        e.win.told.clear();
        e.view()->selectRange(0, 2, 1, 2);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("part of an array")), qPrintable(e.win.told.join('|')));
        QVERIFY(e.win.w.session()->arrayAt(0, 0, 2).valid);
        // one cell of it, F2, a new formula, Ctrl+Shift+Enter: the whole array has it
        e.select(QStringLiteral("C2"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QVERIFY(e.win.w.cellEditor()->isVisible());
        e.win.w.cellEditor()->setText(QStringLiteral("=A1:A3*3"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:3"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:9"));
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        // the whole block selected, Delete clears it all, as one step that Undo takes back
        e.view()->selectRange(0, 2, 2, 2);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(e.settle());
        for (const char *ref : {"C1", "C2", "C3"}) QCOMPARE(e.valueAt(QString::fromLatin1(ref)), QStringLiteral("-"));
        QVERIFY(!e.win.w.session()->arrayAt(0, 1, 2).valid);
        QCOMPARE(e.win.w.undoStack()->count(), 3);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:6"));
        QVERIFY(e.win.w.session()->arrayAt(0, 2, 2).valid);
        // and the cell typed over, once the array is gone, is an ordinary one again
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        e.select(QStringLiteral("C2"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:5"));
    }

    // An array is saved as one (t="array" with its range) and read back as one.
    void anArrayIsSavedAndReadBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QByteArray xml = savedSheet(e.path);
        QVERIFY2(xml.contains("t=\"array\" ref=\"C1:C3\""), xml.constData());
        EditWindow again;
        QVERIFY(again.open(xml.isEmpty() ? QByteArray() : [&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(again.settle());
        QVERIFY(again.win.w.session()->arrayAt(0, 1, 2).valid);
        QCOMPARE(again.valueAt(QStringLiteral("C3")), QStringLiteral("n:6"));
        again.select(QStringLiteral("C2"));
        QCOMPARE(again.view()->activeCellText(), QStringLiteral("{=A1:A3*2}"));
    }

    // A single cell gets an array too, and a cell outside any array: Ctrl+Shift+Enter on a plain formula makes it an array of one cell.
    void anArrayOfOneCellAndArrayOverCellsThatHoldValues()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "C2=old"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("D1"));
        e.type(QStringLiteral("=SUM(A1:A3*2)"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:12"));
        QVERIFY(e.win.w.session()->arrayAt(0, 0, 3).valid);
        // over cells that hold something: it is replaced, and Undo brings it back
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3+1"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:3"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("s:old"));
    }

    // Ctrl with a click or a drag adds a range; a click without Ctrl leaves one.
    void ctrlWithAClickOrADragAddsARange()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C3=2", "E1=3"})));
        QVERIFY(e.settle());
        e.win.w.resize(1100, 640);
        clickCell(e, QStringLiteral("A1"));
        QVERIFY(!e.view()->hasSeveralRanges());
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QVERIFY(e.view()->hasSeveralRanges());
        QCOMPARE(int(e.view()->extraRanges().size()), 1);
        QCOMPARE(int(e.view()->extraRanges()[0].row1), 0);
        QCOMPARE(int(e.view()->extraRanges()[0].col1), 0);
        QCOMPARE(e.selected(), QStringLiteral("C3"));
        QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, middleOf(e, QStringLiteral("E1")));
        QTest::mouseMove(e.view()->viewport(), middleOf(e, QStringLiteral("F2")));
        QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, middleOf(e, QStringLiteral("F2")));
        const auto all = e.view()->allRanges();
        QCOMPARE(int(all.size()), 3);
        QCOMPARE(int(all[2].row1), 0);
        QCOMPARE(int(all[2].col1), 4);
        QCOMPARE(int(all[2].row2), 1);
        QCOMPARE(int(all[2].col2), 5);
        clickCell(e, QStringLiteral("B2"));
        QVERIFY(!e.view()->hasSeveralRanges());
        QCOMPARE(int(e.view()->allRanges().size()), 1);
    }

    // Delete clears every selected range as one step, and Undo takes it back.
    void deleteClearsEveryRange()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C3=2", "E1=3"})));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:3"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:2"));
    }

    // Every command the window has either refuses with several ranges selected (and changes nothing), or is on this short list of commands that need no range, or do their work on every range.
    void everyCommandRefusesSeveralRangesOrIsOnTheSafeList()
    {
        // (the whole list: a command is safe when its id is one of these, or begins with one of the first)
        const QStringList prefixes{"file.", "help.", "view.", "win.", "zoom.", "sheetopt.", "protect.", "hf.", "chartdesign.", "object.", "sheet.", "margins.", "orient.", "size.", "page.", "pagesetup.", "titles.",
                                   "insert.control.", "insert.shape.", "insert.equation"};
        const QStringList exact{"edit.undo", "edit.redo", "edit.selectAll", "edit.find", "edit.clear.contents", "find.find", "find.replace", "find.next", "find.goto", "find.replaceFonts", "printarea.clear",
                                "format.defaultHeight", "format.defaultWidth", "comments.next", "comments.prev", "notes.next", "notes.prev", "notes.showAll", "insert.headerFooter", "insert.icons",
                                "insert.object.chart", "insert.object.page", "insert.onlinePicture", "insert.picture", "insert.symbol", "insert.textBox", "insert.textBox.draw", "review.accessibility",
                                "review.statistics", "review.thesaurus", "formulas.calcAuto", "formulas.calcBeforeSave", "formulas.calcManual", "formulas.calcNoTables", "formulas.calculate",
                                "formulas.calculateEverything", "formulas.calculateSheet", "formulas.circular", "formulas.errorChecking", "formulas.evaluate", "formulas.insertFunction",
                                "formulas.iteration", "formulas.names", "formulas.nextError", "formulas.options", "formulas.precision", "formulas.previousError", "formulas.rebuildAndCalculate",
                                "formulas.removeArrows", "formulas.traceDependents", "formulas.traceError", "formulas.tracePrecedents", "formulas.useName"};
        auto listed = [&](const QString &id) {
            if (exact.contains(id)) return true;
            for (const QString &p : prefixes)
                if (id.startsWith(p)) return true;
            return false;
        };
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C3=2", "E1=3"})));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QVERIFY(e.view()->hasSeveralRanges());
        const QStringList ids = e.win.w.actionIds();
        QVERIFY(ids.size() > 300);
        int refused = 0;
        for (const QString &id : ids) {
            QCOMPARE(CrunchWindow::worksOnSeveralRanges(id), listed(id));
            if (listed(id)) continue;
            QAction *a = e.win.w.act(id);
            QVERIFY2(a, qPrintable(id));
            a->setEnabled(true);
            const qsizetype told = e.win.told.size();
            a->trigger();
            QVERIFY2(e.win.told.size() == told + 1 && e.win.told.last().contains(QLatin1String("several ranges")), qPrintable(id));
            QVERIFY2(e.view()->hasSeveralRanges(), qPrintable(id));
            ++refused;
        }
        QVERIFY(refused > 200);
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:2"));
    }

    // Find in the selection, Replace All, and printing the selection refuse with several ranges, and write and print nothing
    void searchAndPrintOfTheSelectionRefuseSeveralRanges()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=apple", "C3=apple", "E1=pear"})));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        CrunchWindow &w = e.win.w;
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        QVERIFY(d);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("apple"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("pear"));
        e.win.told.clear();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QVERIFY2(e.win.told.last().contains(QLatin1String("several ranges")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->count(), 0);
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("s:apple"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("s:apple"));
        // the selection printed
        crunch::PrintJobOptions o;
        o.what = crunch::PrintWhat::Selection;
        QString why;
        QVERIFY(!w.makePrintJob(o, &why));
        QVERIFY2(why.contains(QLatin1String("several ranges")), qPrintable(why));
    }

    // The other ranges belong to the sheet they were selected on: a switch to another sheet leaves one range there and back, and Undo of a row put in leaves one, so that Delete never clears
    // coordinates that were of another place.
    void severalRangesAreOneAfterASheetSwitchOrAnUndoOfARowPutIn()
    {
        EditWindow e;
        QVERIFY(e.open(budgetWorkbook()));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QVERIFY(e.view()->hasSeveralRanges());
        e.win.w.setActiveSheet(2);
        QCOMPARE(e.win.w.activeSheet(), 2);
        QVERIFY(!e.view()->hasSeveralRanges());
        QCOMPARE(int(e.view()->allRanges().size()), 1);
        // (Delete there clears the one cell the sheet shows selected, and nothing else)
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:7"));
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:3"));
        e.win.w.setActiveSheet(0);
        QCOMPARE(int(e.view()->allRanges().size()), 1);
        // a row put in, and taken out by Undo
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QVERIFY(e.win.w.session()->insertRows(0, 5, 1).ok);
        QVERIFY(!e.view()->hasSeveralRanges());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        e.win.w.undoStack()->undo();
        QVERIFY(!e.view()->hasSeveralRanges());
        QCOMPARE(int(e.view()->allRanges().size()), 1);
    }

    // An array entered over a block that holds another array, that begins inside it, covers every cell of the new block: none is left out; Undo brings the old array back, and Redo makes the new again.
    void anArrayOverAnotherThatBeginsInsideLeavesNoHole()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4"})));
        QVERIFY(e.settle());
        e.view()->selectRange(1, 3, 2, 3);
        e.type(QStringLiteral("=A1:A2*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->arrayAt(0, 2, 3).valid);
        auto allOfOne = [&](quint32 first, quint32 last) {
            for (quint32 row = 0; row < 5; ++row) {
                const auto r = e.win.w.session()->arrayAt(0, row, 3);
                if (row < first || row > last) {
                    if (r.valid) return false;
                    continue;
                }
                if (!r.valid || r.row1 != first || r.row2 != last) return false;
            }
            return true;
        };
        QVERIFY(allOfOne(1, 2));
        e.view()->selectRange(0, 3, 3, 3);
        e.type(QStringLiteral("=A1:A4+10"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QVERIFY(allOfOne(0, 3));
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:11"));
        QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("n:12"));
        QCOMPARE(e.valueAt(QStringLiteral("D4")), QStringLiteral("n:14"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QVERIFY(allOfOne(1, 2));
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QVERIFY(allOfOne(0, 3));
        QCOMPARE(e.valueAt(QStringLiteral("D3")), QStringLiteral("n:13"));
    }

    // F2 on any cell of an array opens the array's formula, as the bar shows it (not the cell's value); on a protected sheet, and in a range with a password, an array cell is refused or asks as any cell does.
    void f2OnAnArrayCellOpensItsFormulaAndAsksAsAnyCellDoes()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        for (const char *ref : {"C1", "C2", "C3"}) {
            e.select(QString::fromLatin1(ref));
            QTest::keyClick(e.view(), Qt::Key_F2);
            QVERIFY2(e.win.w.cellEditor()->isVisible(), ref);
            QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=A1:A3*2"));
            e.key(Qt::Key_Escape);
            QVERIFY(!e.win.w.cellEditor()->isVisible());
        }
        // a protected sheet: its cells are locked, and the array's are
        e.win.w.anchorHooks().protectAsker = [](ProtectRequest *) { return true; };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(e.win.w.session()->protectionOf(0).enabled);
        e.win.told.clear();
        for (const char *ref : {"C1", "C2"}) {
            e.select(QString::fromLatin1(ref));
            QTest::keyClick(e.view(), Qt::Key_F2);
            QVERIFY2(!e.win.w.cellEditor()->isVisible(), ref);
            QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
            e.win.told.clear();
        }
    }

    void anArrayCellInARangeWithAPasswordAsksForIt()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        e.win.w.anchorHooks().protectAsker = [](ProtectRequest *) { return true; };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(e.win.w.session()->protectionOf(0).enabled);
        e.win.w.anchorHooks().editRangesAsker = [](EditRangesRequest *r) {
            jp::office::sheet::EditRange mine;
            mine.title = QStringLiteral("Mine");
            mine.sqref = QStringLiteral("C1:C3");
            r->ranges.append(mine);
            r->passwords.insert(QStringLiteral("Mine"), QStringLiteral("secret"));
            return true;
        };
        e.win.w.act(QStringLiteral("protect.editRanges"))->trigger();
        QTRY_VERIFY_WITH_TIMEOUT(!e.win.w.session()->protectionOf(0).editRanges.isEmpty(), 60000);
        int asked = 0;
        e.win.w.anchorHooks().passwordAsker = [&](PasswordRequest *) {
            ++asked;
            return false;   // (the person cancels)
        };
        e.select(QStringLiteral("C2"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QTRY_COMPARE(asked, 1);
        QVERIFY(!e.win.w.cellEditor()->isVisible());
    }

    // An array over cells of an array too big for its cells to be flagged is refused by the array's bounds: no array is made inside it, over part of it, or from one cell of it.
    void anArrayInsideOrOverPartOfAHugeArrayIsRefused()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"array\" ref=\"B1:B1000001\">1</f><v>1</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->arrayAt(0, 0, 1).valid);
        QCOMPARE(int(e.win.w.session()->arrayAt(0, 0, 1).row2), 1000000);
        // one cell inside, and a block inside: refused, and nothing changes
        for (const bool block : {false, true}) {
            if (block) e.view()->selectRange(5000, 1, 5002, 1);
            else e.select(QStringLiteral("B5001"));
            e.type(QStringLiteral("=A1+1"));
            e.win.told.clear();
            e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
            QVERIFY(e.settle());
            QVERIFY2(!e.win.told.isEmpty(), block ? "block" : "cell");
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            QCOMPARE(int(e.win.w.session()->arrayAt(0, 0, 1).row2), 1000000);
            QVERIFY(!e.win.w.session()->arrayAt(0, 5000, 1).valid || e.win.w.session()->arrayAt(0, 5000, 1).row1 == 0);
            e.key(Qt::Key_Escape);
        }
    }

    // The first cell of an array, and the whole array, are copied and filled as what they are worth: no formula of one cell that would spill, no error.
    void copyingAndFillingFromAnArrayGiveItsValues()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        // the whole array, to E1
        copyRange(e, QStringLiteral("C1:C3"));
        paste(e, QStringLiteral("E1"));
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("E3")), QStringLiteral("n:6"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 4), QStringLiteral("2"));
        // the first cell alone, to G1
        copyRange(e, QStringLiteral("C1:C1"));
        paste(e, QStringLiteral("G1"));
        QCOMPARE(e.valueAt(QStringLiteral("G1")), QStringLiteral("n:2"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 6), QStringLiteral("2"));
        // Fill Right from the first cell
        e.view()->selectRange(0, 2, 0, 3);
        e.win.w.act(QStringLiteral("fill.right"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:2"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 3), QStringLiteral("2"));
    }

    // An array is kept by a copy of its sheet, and a formula that refers into an array reads its values.
    void anArraySurvivesASheetCopyAndAFormulaReadsItsCells()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("=C3+C2"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:10"));
        int made = -1;
        QVERIFY(e.win.w.session()->copySheet(0, 1, QStringLiteral("Copy"), &made).ok);
        QVERIFY(made >= 0 && made != 0);
        const auto copied = e.win.w.session()->arrayAt(made, 1, 2);
        QVERIFY(copied.valid);
        QCOMPARE(int(copied.row1), 0);
        QCOMPARE(int(copied.row2), 2);
        e.win.w.setActiveSheet(made);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:6"));
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:10"));
        e.select(QStringLiteral("C2"));
        QCOMPARE(e.view()->activeCellText(), QStringLiteral("{=A1:A3*2}"));
    }

    // Go To takes several references with commas between them, and selects them all.
    void goToSelectsSeveralReferences()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.goToReference(QStringLiteral("A1:B2,D4")));
        QCOMPARE(int(e.view()->allRanges().size()), 2);
        QCOMPARE(e.selected(), QStringLiteral("D4"));
        QCOMPARE(int(e.view()->extraRanges()[0].row2), 1);
        QVERIFY(!e.win.w.goToReference(QStringLiteral("A1,Nowhere!B2")));
    }

    // Ctrl+A in data selects the block around the cursor first, and the sheet on the next press; Ctrl+Shift+* is the block alone.
    void ctrlASelectsTheCurrentRegionFirst()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=2", "A2=3", "B2=4", "A3=5", "D8=9"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("B2"));
        QTest::keyClick(e.view(), Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:B3"));
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        QTest::keyClick(e.view(), Qt::Key_A, Qt::ControlModifier);
        QVERIFY(e.view()->selection().whole == grid::Selection::Whole::All);
        e.select(QStringLiteral("A2"));
        QTest::keyClick(e.view(), Qt::Key_Asterisk, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:B3"));
        // a selection that is already past the block goes to the sheet; it does not shrink to the block
        e.view()->selectRange(0, 0, 9, 4);
        QTest::keyClick(e.view(), Qt::Key_A, Qt::ControlModifier);
        QVERIFY(e.view()->selection().whole == grid::Selection::Whole::All);
        // a cell with nothing around it: the whole sheet at once
        e.select(QStringLiteral("H20"));
        QTest::keyClick(e.view(), Qt::Key_A, Qt::ControlModifier);
        QVERIFY(e.view()->selection().whole == grid::Selection::Whole::All);
    }

    // End, then an arrow, goes to the edge of the data as Ctrl and the arrow does; the status bar says End meanwhile.
    void endThenAnArrowGoesToTheEdgeOfTheData()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A6=9"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_End);
        QVERIFY(e.view()->endMode());
        QCOMPARE(e.win.w.statusText(), QStringLiteral("End"));
        QTest::keyClick(e.view(), Qt::Key_Down);
        QTRY_COMPARE(e.selected(), QStringLiteral("A3"));
        QVERIFY(!e.view()->endMode());
        QCOMPARE(e.win.w.statusText(), QStringLiteral("Ready"));
        // a click ends it too, and the status bar stops saying End
        QTest::keyClick(e.view(), Qt::Key_End);
        QCOMPARE(e.win.w.statusText(), QStringLiteral("End"));
        clickCell(e, QStringLiteral("A2"));
        QVERIFY(!e.view()->endMode());
        QCOMPARE(e.win.w.statusText(), QStringLiteral("Ready"));
        e.select(QStringLiteral("A3"));
        // another key ends the mode without moving
        QTest::keyClick(e.view(), Qt::Key_End);
        QTest::keyClick(e.view(), Qt::Key_Escape);
        QVERIFY(!e.view()->endMode());
        QCOMPARE(e.selected(), QStringLiteral("A3"));
    }
};

JP_TEST_CLASS(CrunchGapTests)
#include "tst_crunch_gaps.moc"
