// Quick Analysis: the button by a selection of two or more cells with data, its box (formatting, charts, totals, tables, sparklines), a look at a choice that leaves no trace, and a choice made as one step.
#include "../cmdsearchcheck.h"
#include "app/crunchwindow.h"
#include "app/quickanalysis.h"
#include "app/theme.h"
#include "core/editsession.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "savedok.h"

#include <QAction>
#include <QDir>
#include <QPainter>
#include <QApplication>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::CellRange;

namespace {

// Names across the top and down the left, and numbers:
//        A     B     C
//   1   Item   Q1    Q2
//   2   x      10    20
//   3   y      30    40
//   4   z      5     15
QByteArray quickBook()
{
    return sheetOf({QStringLiteral("A1=Item"), QStringLiteral("B1=Q1"), QStringLiteral("C1=Q2"), QStringLiteral("A2=x"), QStringLiteral("B2=10"), QStringLiteral("C2=20"), QStringLiteral("A3=y"),
                    QStringLiteral("B3=30"), QStringLiteral("C3=40"), QStringLiteral("A4=z"), QStringLiteral("B4=5"), QStringLiteral("C4=15")});
}

void pick(EditWindow &e, const QString &range)
{
    const CellRange r = rangeOf(range);
    e.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
}

QuickAnalysis *quick(EditWindow &e) { return e.win.w.quickAnalysis(); }

QWidget *layerOf(EditWindow &e) { return e.view()->viewport()->findChild<QWidget *>(QStringLiteral("quickPreviewLayer")); }

// Opens the box and makes a choice (the box closes first, as it does for a click).
bool choose(EditWindow &e, const QString &id)
{
    if (!quick(e)->open()) return false;
    quick(e)->choose(id);
    return true;
}

} // namespace

class CrunchQuickTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
    }

    void theButtonIsForTwoOrMoreCellsWithData()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        QVERIFY(quick(e));
        e.select(QStringLiteral("B2"));
        QVERIFY(!quick(e)->isShown());   // one cell
        pick(e, QStringLiteral("B2:C4"));
        QTRY_VERIFY(quick(e)->isShown());
        pick(e, QStringLiteral("F10:G12"));
        QVERIFY(!quick(e)->isShown());   // cells with nothing in them
        pick(e, QStringLiteral("A1:C4"));
        QVERIFY(quick(e)->isShown());
        e.view()->selectRange(0, 1, 3, 1);   // (a column of the data: two or more cells)
        QVERIFY(quick(e)->isShown());
        e.select(QStringLiteral("B2"));
        QVERIFY(!quick(e)->isShown());
        // what it looks at to find out is a small constant, so a huge selection is decided at once
        QVERIFY(e.win.w.session()->quickHasData(0, rangeOf(QStringLiteral("A1:XFD1048576"))));
    }

    void theButtonKeepsTheKeyboardClearsTheFillHandleAndFollowsTheSheet()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        e.win.w.activateWindow();
        e.view()->setFocus();
        QTRY_VERIFY(e.view()->hasFocus());
        pick(e, QStringLiteral("A1:C4"));
        QToolButton *b = quick(e)->button();
        QVERIFY(b->isVisible());
        QCOMPARE(b->focusPolicy(), Qt::NoFocus);
        const QRect handle = e.view()->fillHandleRect();
        QVERIFY(!handle.isNull());
        QVERIFY(!b->geometry().intersects(handle));
        QVERIFY(!b->geometry().intersects(e.view()->cellRect(3, 2)));   // (it is outside the selection's last cell too)
        // a real press and release on the button opens the box, and the view has the keyboard again when it closes
        QTest::mouseClick(b, Qt::LeftButton);
        QVERIFY(quick(e)->isOpen());
        quick(e)->close();
        QTRY_VERIFY(!QApplication::activePopupWidget());
        QTRY_VERIFY(e.view()->hasFocus());
        QVERIFY(QApplication::mouseButtons() == Qt::NoButton);
        // zoom moves it with the cells
        const QPoint before = b->pos();
        e.view()->setZoom(2.0);
        QTRY_VERIFY(b->pos() != before);
        QVERIFY(!b->geometry().intersects(e.view()->fillHandleRect()));
        e.view()->setZoom(1.0);
        // scrolled out of sight, it is gone; back, it is back
        e.view()->ensureVisible(100000, 0);
        QTRY_VERIFY(!quick(e)->isShown());
        e.view()->ensureVisible(0, 0);
        QTRY_VERIFY(quick(e)->isShown());
    }

    void ctrlQAndTheRibbonCommandOpenTheBox()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        QAction *a = e.win.w.act(QStringLiteral("edit.quickAnalysis"));
        QVERIFY(a);
        QVERIFY(!a->shortcut().isEmpty());
        QVERIFY(a->isEnabled());
        e.select(QStringLiteral("B2"));
        a->trigger();
        QVERIFY(!quick(e)->isOpen());   // one cell: it says so
        pick(e, QStringLiteral("A1:C4"));
        e.win.w.activateWindow();
        e.view()->setFocus();
        const QKeyCombination k = a->shortcut()[0];
        QTest::keyClick(e.view(), k.key(), k.keyboardModifiers());
        QVERIFY(quick(e)->isOpen());
        quick(e)->close();
        QVERIFY(!quick(e)->isOpen());
        const QString problem = jptest::commandSearchProblem(e.win.w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("instant chart"), QStringLiteral("Quick Analysis"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
    }

    void aLookAtAFormatLeavesNoTrace()
    {
        for (const QString &id : {QStringLiteral("fmt.bars"), QStringLiteral("fmt.scale"), QStringLiteral("fmt.icons"), QStringLiteral("fmt.greater"), QStringLiteral("fmt.top")}) {
            EditWindow e;
            QVERIFY(e.open(quickBook()));
            QVERIFY(e.settle());
            crunch::EditSession *s = e.win.w.session();
            pick(e, QStringLiteral("B2:C4"));
            QVERIFY(quick(e)->open());
            const int steps = e.win.w.undoStack()->count(), index = e.win.w.undoStack()->index();
            const bool modified = s->isModified();
            const auto rules = s->condRules(0);
            quick(e)->look(id);
            QCOMPARE(quick(e)->currentChoice(), id);
            QVERIFY2(s->condPreviewShown(), qPrintable(id));
            QCOMPARE(s->condRules(0).size(), rules.size() + 1);   // (the sheet shows a rule)
            QCOMPARE(e.win.w.undoStack()->count(), steps);
            QCOMPARE(e.win.w.undoStack()->index(), index);
            QCOMPARE(s->isModified(), modified);
            quick(e)->close();
            QVERIFY(!s->condPreviewShown());
            QCOMPARE(s->condRules(0).size(), rules.size());
            QCOMPARE(e.win.w.undoStack()->count(), steps);
            QCOMPARE(s->isModified(), modified);
        }
    }

    void aSaveWhileLookingWritesNoRuleAndEndsTheLook()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 8, 8, QStringLiteral("1")).ok);   // (the workbook has a change to save)
        QVERIFY(e.settle());
        pick(e, QStringLiteral("B2:C4"));
        QVERIFY(quick(e)->open());
        quick(e)->look(QStringLiteral("fmt.top"));   // (this one has a look of its own: a style the file would have to keep)
        QVERIFY(s->condPreviewShown());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QVERIFY(!s->condPreviewShown());
        Window again;
        QVERIFY(again.open(e.copyOfFile()));
        QVERIFY(again.w.session()->condRules(0).isEmpty());
        quick(e)->close();
    }

    void anEditEndsTheLook()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        pick(e, QStringLiteral("B2:C4"));
        QVERIFY(quick(e)->open());
        quick(e)->look(QStringLiteral("fmt.bars"));
        QVERIFY(s->condPreviewShown());
        QVERIFY(s->clearCondRules(0, rangeOf(QStringLiteral("A1:C4"))).ok);
        QVERIFY(!s->condPreviewShown());
        quick(e)->close();
    }

    void aFormatChoiceIsOneStepThatUndoTakesBack()
    {
        for (const QString &id : {QStringLiteral("fmt.bars"), QStringLiteral("fmt.scale"), QStringLiteral("fmt.icons"), QStringLiteral("fmt.greater"), QStringLiteral("fmt.top")}) {
            EditWindow e;
            QVERIFY(e.open(quickBook()));
            QVERIFY(e.settle());
            crunch::EditSession *s = e.win.w.session();
            pick(e, QStringLiteral("B2:C4"));
            const int steps = e.win.w.undoStack()->count();
            QVERIFY(choose(e, id));
            QVERIFY2(e.win.told.isEmpty(), qPrintable(id + e.win.told.join(QLatin1Char('|'))));
            QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
            QCOMPARE(s->condRules(0).size(), 1);
            QVERIFY(!s->condPreviewShown());
            // Clear Format takes it off, as one step
            if (id == QLatin1String("fmt.bars")) {
                pick(e, QStringLiteral("B2:C4"));
                QVERIFY(choose(e, QStringLiteral("fmt.clear")));
                QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
                QCOMPARE(s->condRules(0).size(), 0);
                e.win.w.undoStack()->undo();
                QCOMPARE(s->condRules(0).size(), 1);
            }
            e.win.w.undoStack()->setIndex(steps);
            QCOMPARE(s->condRules(0).size(), 0);
        }
    }

    void totalsAreFormulasInOneStepAndUndoRestoresTheCells()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        pick(e, QStringLiteral("A1:C4"));   // (the names in the first row and column are left out)
        int steps = e.win.w.undoStack()->index();
        QVERIFY(choose(e, QStringLiteral("tot.0.down")));
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->index(), steps + 1);
        QCOMPARE(e.shownAt(QStringLiteral("B5")), QStringLiteral("45"));
        QCOMPARE(e.shownAt(QStringLiteral("C5")), QStringLiteral("75"));
        QCOMPARE(e.shownAt(QStringLiteral("A5")), QString());
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.shownAt(QStringLiteral("B5")), QString());
        // across: the column to the right
        pick(e, QStringLiteral("A1:C4"));
        steps = e.win.w.undoStack()->index();
        QVERIFY(choose(e, QStringLiteral("tot.0.across")));
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->index(), steps + 1);
        QCOMPARE(e.shownAt(QStringLiteral("D2")), QStringLiteral("30"));
        QCOMPARE(e.shownAt(QStringLiteral("D3")), QStringLiteral("70"));
        QCOMPARE(e.shownAt(QStringLiteral("D4")), QStringLiteral("20"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        // average, count, share of the whole, running total (each is made and taken back)
        auto totalIn = [&](const QString &id, const QString &cell) {
            pick(e, QStringLiteral("A1:C4"));
            if (!choose(e, id) || !e.settle()) return QStringLiteral("not made");
            const QString shown = e.shownAt(cell);
            e.win.w.undoStack()->undo();
            e.settle();
            return shown;
        };
        QCOMPARE(totalIn(QStringLiteral("tot.1.down"), QStringLiteral("B5")), QStringLiteral("15"));
        QCOMPARE(totalIn(QStringLiteral("tot.2.down"), QStringLiteral("B5")), QStringLiteral("3"));
        QCOMPARE(totalIn(QStringLiteral("tot.3.down"), QStringLiteral("B5")), QStringLiteral("37.5%"));
        QCOMPARE(totalIn(QStringLiteral("tot.3.down"), QStringLiteral("C5")), QStringLiteral("62.5%"));
        QCOMPARE(totalIn(QStringLiteral("tot.3.across"), QStringLiteral("D2")), QStringLiteral("25.0%"));
        QCOMPARE(totalIn(QStringLiteral("tot.4.down"), QStringLiteral("D2")), QStringLiteral("10"));
        QCOMPARE(totalIn(QStringLiteral("tot.4.down"), QStringLiteral("D4")), QStringLiteral("45"));
        QCOMPARE(totalIn(QStringLiteral("tot.4.down"), QStringLiteral("E4")), QStringLiteral("75"));
        QCOMPARE(totalIn(QStringLiteral("tot.4.across"), QStringLiteral("B5")), QStringLiteral("10"));
        QCOMPARE(totalIn(QStringLiteral("tot.4.across"), QStringLiteral("C7")), QStringLiteral("20"));   // (row 4: 5, then 5 + 15)
        QVERIFY(e.win.told.isEmpty());
    }

    void totalsAskBeforeReplacingWhatIsThere()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 4, 1, QStringLiteral("keep me")).ok);   // B5
        QVERIFY(e.settle());
        pick(e, QStringLiteral("A1:C4"));
        int asked = 0;
        bool answer = false;
        e.win.w.setConfirmAsker([&](const QString &, const QString &) {
            ++asked;
            return answer;
        });
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(choose(e, QStringLiteral("tot.0.down")));
        QCOMPARE(asked, 1);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(e.shownAt(QStringLiteral("B5")), QStringLiteral("keep me"));
        answer = true;
        pick(e, QStringLiteral("A1:C4"));
        QVERIFY(choose(e, QStringLiteral("tot.0.down")));
        QVERIFY(e.settle());
        QCOMPARE(asked, 2);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(e.shownAt(QStringLiteral("B5")), QStringLiteral("45"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.shownAt(QStringLiteral("B5")), QStringLiteral("keep me"));
    }

    void aLookAtTotalsAndTheOthersChangesNothing()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        pick(e, QStringLiteral("A1:C4"));
        QVERIFY(quick(e)->open());
        const int steps = e.win.w.undoStack()->count();
        const bool modified = s->isModified();
        QVERIFY(layerOf(e));
        for (const QString &id : {QStringLiteral("tot.0.down"), QStringLiteral("tot.3.across"), QStringLiteral("tot.4.down"), QStringLiteral("tab.table"), QStringLiteral("tab.pivot"), QStringLiteral("spark.line"),
                                  QStringLiteral("spark.column"), QStringLiteral("spark.winloss")}) {
            quick(e)->look(id);
            QCOMPARE(quick(e)->currentChoice(), id);
            QVERIFY2(layerOf(e)->isVisible(), qPrintable(id));
            e.view()->viewport()->repaint();   // (it paints, and leaves nothing)
            QCOMPARE(e.win.w.undoStack()->count(), steps);
            QCOMPARE(s->isModified(), modified);
            QCOMPARE(e.shownAt(QStringLiteral("B5")), QString());
            QCOMPARE(e.shownAt(QStringLiteral("D2")), QString());
            QVERIFY(s->sparklinesOf(0).isEmpty());
        }
        quick(e)->close();
        QVERIFY(!layerOf(e)->isVisible());
    }

    void aTableASparklineGroupAndAPivotTableAreMade()
    {
        {
            EditWindow e;
            QVERIFY(e.open(quickBook()));
            QVERIFY(e.settle());
            pick(e, QStringLiteral("A1:C4"));
            const int steps = e.win.w.undoStack()->count();
            QVERIFY(choose(e, QStringLiteral("tab.table")));
            QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
            QCOMPARE(e.win.w.session()->workbook()->tables().size(), 1);
            e.win.w.undoStack()->undo();
            QCOMPARE(e.win.w.session()->workbook()->tables().size(), 0);
        }
        {
            EditWindow e;
            QVERIFY(e.open(quickBook()));
            QVERIFY(e.settle());
            pick(e, QStringLiteral("A1:C4"));
            const int steps = e.win.w.undoStack()->count();
            QVERIFY(choose(e, QStringLiteral("spark.column")));
            QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
            const auto groups = e.win.w.session()->sparklinesOf(0);
            QCOMPARE(groups.size(), 1);
            QCOMPARE(groups[0].sparklines.size(), 3);   // (a row each: the names in the first row are left out)
            QCOMPARE(groups[0].sparklines[0].col, quint32(3));
            QCOMPARE(groups[0].sparklines[0].row, quint32(1));
            e.win.w.undoStack()->undo();
            QVERIFY(e.win.w.session()->sparklinesOf(0).isEmpty());
        }
        {
            EditWindow e;
            QVERIFY(e.open(sheetOf({QStringLiteral("A1=Item"), QStringLiteral("B1=Q1"), QStringLiteral("A2=x"), QStringLiteral("B2=10"), QStringLiteral("A3=y"), QStringLiteral("B3=30"), QStringLiteral("A4=x"),
                                    QStringLiteral("B4=5"), QStringLiteral("A5=y"), QStringLiteral("B5=7")})));
            QVERIFY(e.settle());
            pick(e, QStringLiteral("A1:B5"));
            QVERIFY(choose(e, QStringLiteral("tab.pivot")));
            QVERIFY(e.settle());
            QCOMPARE(e.win.w.session()->workbook()->pivots().tableCount(), 1);
        }
    }

    void aChartIsOneOfTheRecommendedKindsAndOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        pick(e, QStringLiteral("A1:C4"));
        QVERIFY(quick(e)->open());
        quick(e)->selectTab(QStringLiteral("charts"));
        QCOMPARE(quick(e)->currentTab(), QStringLiteral("charts"));
        const auto tiles = quick(e)->box()->findChildren<QToolButton *>(QRegularExpression(QStringLiteral("^quickChoice\\.chart\\.")));
        QVERIFY(tiles.size() >= 1 && tiles.size() <= 4);
        const QString id = tiles.first()->objectName().mid(int(QStringLiteral("quickChoice.").size()));
        const int steps = e.win.w.undoStack()->count();
        quick(e)->look(id);
        QVERIFY(layerOf(e)->isVisible());
        QCOMPARE(e.win.w.undoStack()->count(), steps);   // (a look at a chart makes none)
        quick(e)->choose(id);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(e.win.w.undoStack()->command(steps)->text(), QStringLiteral("New Chart"));
        QVERIFY(!layerOf(e)->isVisible());
    }

    void hostileSelectionsAreRefusedBeforeTheyAreScanned()
    {
        EditWindow e;
        // a sheet whose cells reach AAA2000: more cells than Quick Analysis looks at (250,000) when they are all selected
        QVERIFY(e.open(sheetOf({QStringLiteral("A1=1"), QStringLiteral("B1=2"), QStringLiteral("AAA2000=3")})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const auto plan = s->planQuickTotals(0, rangeOf(QStringLiteral("A1:AAA2000")), crunch::EditSession::QuickTotal::Sum, true);
        QVERIFY(!plan.ok);
        QVERIFY(plan.cells.empty());
        QVERIFY(!plan.problem.isEmpty());
        const int steps = e.win.w.undoStack()->count();
        e.view()->selectRange(0, 0, 1999, 702);
        QVERIFY(choose(e, QStringLiteral("tot.0.down")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!e.win.told.isEmpty());
        // the work of finding out whether to show the button is a few thousand cells at the most, however large the selection
        QCOMPARE(crunch::EditSession::kQuickProbe, qint64(4096));
        QVERIFY(s->quickHasData(0, rangeOf(QStringLiteral("A1:AAA2000"))));
    }

    void aProtectedSheetOffersWhatItAllowsAndNothingElse()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::SheetProtection on;
        on.enabled = true;
        QVERIFY(s->setSheetProtection(0, on, QStringLiteral("Protect")).ok);
        QVERIFY(e.settle());
        pick(e, QStringLiteral("B2:C4"));
        QVERIFY(!quick(e)->isShown());   // nothing the box does is allowed
        on.allowFormatCells = true;
        QVERIFY(s->setSheetProtection(0, on, QStringLiteral("Protect")).ok);
        pick(e, QStringLiteral("B2:C4"));
        QVERIFY(quick(e)->isShown());
        QVERIFY(quick(e)->open());
        QToolButton *bars = quick(e)->box()->findChild<QToolButton *>(QStringLiteral("quickChoice.fmt.bars"));
        QToolButton *sum = quick(e)->box()->findChild<QToolButton *>(QStringLiteral("quickChoice.tot.0.down"));
        QVERIFY(bars && sum);
        QVERIFY(bars->isEnabled());
        QVERIFY(!sum->isEnabled());
        quick(e)->close();
    }

    void theArrowKeysLookEnterChoosesAndEscapeLeavesNoTrace()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        pick(e, QStringLiteral("B2:C4"));
        QVERIFY(quick(e)->open());
        const int steps = e.win.w.undoStack()->count();
        QTest::keyClick(quick(e)->box(), Qt::Key_Right);
        QCOMPARE(quick(e)->currentChoice(), QStringLiteral("fmt.bars"));
        QVERIFY(s->condPreviewShown());
        QTest::keyClick(quick(e)->box(), Qt::Key_Right);
        QCOMPARE(quick(e)->currentChoice(), QStringLiteral("fmt.scale"));
        QVERIFY(s->condPreviewShown());
        QCOMPARE(s->condRules(0).size(), 1);   // (one rule shown: the first look was taken out)
        QTest::keyClick(quick(e)->box(), Qt::Key_Down);   // the Charts tab
        QVERIFY(!s->condPreviewShown());
        QCOMPARE(quick(e)->currentTab(), QStringLiteral("charts"));
        QTest::keyClick(quick(e)->box(), Qt::Key_Up);
        QTest::keyClick(quick(e)->box(), Qt::Key_Right);
        QCOMPARE(quick(e)->currentChoice(), QStringLiteral("fmt.bars"));
        QTest::keyClick(quick(e)->box(), Qt::Key_Escape);
        QVERIFY(!quick(e)->isOpen());
        QVERIFY(!s->condPreviewShown());
        QCOMPARE(s->condRules(0).size(), 0);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // Enter makes the choice looked at
        QVERIFY(quick(e)->open());
        QTest::keyClick(quick(e)->box(), Qt::Key_Right);
        QTest::keyClick(quick(e)->box(), Qt::Key_Return);
        QVERIFY(!quick(e)->isOpen());
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(s->condRules(0).size(), 1);
    }

    void aRealClickOnAChoiceMakesIt()
    {
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        pick(e, QStringLiteral("B2:C4"));
        QVERIFY(quick(e)->open());
        const int steps = e.win.w.undoStack()->count();
        QToolButton *tile = quick(e)->box()->findChild<QToolButton *>(QStringLiteral("quickChoice.fmt.scale"));
        QVERIFY(tile);
        QTest::mousePress(tile, Qt::LeftButton, Qt::NoModifier, tile->rect().center());
        QTest::mouseRelease(tile, Qt::LeftButton, Qt::NoModifier, tile->rect().center());
        QVERIFY(!quick(e)->isOpen());
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(e.win.w.session()->condRules(0).size(), 1);
        QVERIFY(QApplication::mouseButtons() == Qt::NoButton);
    }
    // Pictures for Jeff: set QUICK_SHOT_DIR to a folder. (Skipped otherwise: the theme call changes the whole program's style.)
    void picturesOfTheButtonTheBoxAndTheLooks()
    {
        const QString dir = qEnvironmentVariable("QUICK_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("QUICK_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        QDir().mkpath(dir);
        EditWindow e;
        QVERIFY(e.open(quickBook()));
        QVERIFY(e.settle());
        e.win.w.resize(900, 520);
        pick(e, QStringLiteral("A1:C4"));
        auto shot = [&](const QString &name) {
            QPixmap whole = e.win.w.grab();
            if (quick(e)->isOpen()) {
                QPainter p(&whole);
                p.drawPixmap(e.win.w.mapFromGlobal(quick(e)->box()->mapToGlobal(QPoint(0, 0))), quick(e)->box()->grab());
            }
            QVERIFY(whole.save(QDir(dir).filePath(name)));
        };
        shot(QStringLiteral("quick-1-button.png"));
        pick(e, QStringLiteral("B2:C4"));
        QVERIFY(quick(e)->open());
        quick(e)->look(QStringLiteral("fmt.bars"));
        e.view()->viewport()->repaint();
        shot(QStringLiteral("quick-2-formatting-look.png"));
        quick(e)->look(QStringLiteral("fmt.scale"));
        shot(QStringLiteral("quick-3-scale-look.png"));
        quick(e)->close();
        pick(e, QStringLiteral("A1:C4"));
        QVERIFY(quick(e)->open());
        quick(e)->selectTab(QStringLiteral("charts"));
        const auto tiles = quick(e)->box()->findChildren<QToolButton *>(QRegularExpression(QStringLiteral("^quickChoice\\.chart\\.")));
        QVERIFY(!tiles.isEmpty());
        quick(e)->look(tiles.first()->objectName().mid(int(QStringLiteral("quickChoice.").size())));
        shot(QStringLiteral("quick-4-chart-look.png"));
        quick(e)->look(QStringLiteral("tot.0.down"));
        quick(e)->selectTab(QStringLiteral("totals"));
        quick(e)->look(QStringLiteral("tot.0.down"));
        shot(QStringLiteral("quick-5-totals-look.png"));
        quick(e)->look(QStringLiteral("tot.3.across"));
        shot(QStringLiteral("quick-6-percent-look.png"));
        quick(e)->selectTab(QStringLiteral("tables"));
        quick(e)->look(QStringLiteral("tab.table"));
        shot(QStringLiteral("quick-7-table-look.png"));
        quick(e)->selectTab(QStringLiteral("sparklines"));
        quick(e)->look(QStringLiteral("spark.column"));
        shot(QStringLiteral("quick-8-sparklines-look.png"));
        quick(e)->close();
    }
};

JP_TEST_CLASS(CrunchQuickTests)
#include "tst_crunch_quick.moc"
