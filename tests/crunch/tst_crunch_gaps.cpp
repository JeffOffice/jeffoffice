// Tests of the everyday editing gaps closed in round A: array formulas entered with Ctrl+Shift+Enter, selecting more than one range, the status bar's words, and notes' boxes. Every workbook is built here
#include "app/celleditor.h"
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "office/package.h"
#include "savedok.h"

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

    // Delete clears every selected range as one step; commands that work on one range say so, and change nothing.
    void deleteClearsEveryRangeAndOtherCommandsSayTheyNeedOne()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C3=2", "E1=3"})));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        e.win.told.clear();
        e.win.w.act(QStringLiteral("fmt.bold"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("several ranges")), qPrintable(e.win.told.join('|')));
        e.win.told.clear();
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("several ranges")), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
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
        // another key ends the mode without moving
        QTest::keyClick(e.view(), Qt::Key_End);
        QTest::keyClick(e.view(), Qt::Key_Escape);
        QVERIFY(!e.view()->endMode());
        QCOMPARE(e.selected(), QStringLiteral("A3"));
    }
};

JP_TEST_CLASS(CrunchGapTests)
#include "tst_crunch_gaps.moc"
