// Tests of tools on the Data, Formulas, Review, and View tabs: Workbook Statistics, Goal Seek, Subtotal, and Advanced Filter. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/proofingdialogs.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "sheet/calculation.h"
#include "core/advancedfilter.h"
#include "core/appinfo.h"
#include "calc/numfmt.h"
#include "core/forecastsheet.h"
#include "core/sheetobjects.h"
#include "app/forecastdialog.h"
#include "core/fonts.h"
#include "core/formulasteps.h"
#include "core/goalseek.h"
#include "core/validationedit.h"
#include "core/outline.h"
#include "sheet/calculation.h"
#include "app/functionargumentsdialog.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"
#include "savedok.h"

#include <QAction>
#include <QListWidget>
#include <QMenu>
#include <QTableWidget>
#include <QSettings>
#include <QToolButton>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QUndoStack>
#include "../localepin.h"

#include <cmath>
#include <limits>
#include <memory>

using namespace jp;
using namespace crunchtest;
using crunch::EditSession;
using jp::office::sheet::ChartKind;

namespace {
void selectCells(EditWindow &e, const QString &range)
{
    const CellRange r = rangeOf(range);
    e.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
}
// Whether the row (from 0) is hidden.
bool rowHidden(EditWindow &e, quint32 row)
{
    for (const jp::office::sheet::RowSpec &spec : e.win.w.session()->calculation()->rowSpecsIn(0, row, row))
        if (spec.row == row) return spec.hidden;
    return false;
}
}

class CrunchTabs2Tests : public QObject {
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
        m_localeBefore = QLocale();
        m_pin = std::make_unique<PinnedLocale>();
    }

    void cleanupTestCase()
    {
        m_pin.reset();
        QCOMPARE(QLocale().name(), m_localeBefore.name());
    }

    void workbookStatisticsCountsWhatTheWorkbookHolds()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=two", "C1==A1+1", "A2=3", "A3==SUM(A1:A2)"})));
        QVERIFY(e.settle());
        const auto rows = e.win.w.statisticsRows();
        QCOMPARE(rows.size(), 5);
        QCOMPARE(rows[0].second, QStringLiteral("1"));   // sheets
        QCOMPARE(rows[1].second, QStringLiteral("5"));   // cells with data
        QCOMPARE(rows[2].second, QStringLiteral("2"));   // formulas
        QCOMPARE(rows[3].second, QStringLiteral("0"));   // charts
        QCOMPARE(rows[4].second, QStringLiteral("0"));   // pivot tables
        // the command is on the Review tab and tells the numbers
        QString told;
        e.win.w.setNotifier([&](const QString &, const QString &text) { told = text; });
        QAction *a = e.win.w.act(QStringLiteral("review.statistics"));
        QVERIFY(a);
        QVERIFY(a->isEnabled());
        a->trigger();
        QVERIFY(told.contains(QStringLiteral("Formulas: 2")));
        // a typed cell counts at once
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("=A1*3"));
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.statisticsRows()[2].second, QStringLiteral("3"));
    }

    void goalSeekFindsTheNumberForLinearAndCurvedFormulas()
    {
        auto line = crunch::goalSeek([](double x) -> std::optional<double> { return 2 * x + 3; }, 1, 11);
        QVERIFY(line.found);
        QVERIFY(qAbs(line.input - 4) < 1e-6);
        auto square = crunch::goalSeek([](double x) -> std::optional<double> { return x * x; }, 1, 49);
        QVERIFY(square.found);
        QVERIFY(qAbs(square.input - 7) < 1e-6);
        // from the start that has no slope (a flat spot), the search widens until the formula crosses the target
        auto flat = crunch::goalSeek([](double x) -> std::optional<double> { return x * x * x; }, 0, 8);
        QVERIFY(flat.found);
        QVERIFY(qAbs(flat.input - 2) < 1e-4);
        // an error in the way is stepped round
        auto holey = crunch::goalSeek([](double x) -> std::optional<double> { return x < 0 ? std::nullopt : std::optional<double>(x + 1); }, 5, 3);
        QVERIFY(holey.found);
        QVERIFY(qAbs(holey.input - 2) < 1e-6);
    }

    void goalSeekGivesUpWhenTheTargetCannotBeReached()
    {
        auto never = crunch::goalSeek([](double x) -> std::optional<double> { return x * x + 1; }, 3, 0);
        QVERIFY(!never.found);
        QVERIFY(never.trials <= crunch::kGoalSeekMaxTrials);
        auto constant = crunch::goalSeek([](double) -> std::optional<double> { return 5; }, 0, 11);
        QVERIFY(!constant.found);
        auto errors = crunch::goalSeek([](double) -> std::optional<double> { return std::nullopt; }, 0, 1);
        QVERIFY(!errors.found);
    }

    void goalSeekChangesTheCellAndIsOneStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1*2+3", "C1==B1+1"})));
        QVERIFY(e.settle());
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.goalSeekIn(QStringLiteral("B1"), QStringLiteral("11"), QStringLiteral("A1")));
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:C1")), QStringLiteral("4|11|12"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:C1")), QStringLiteral("1|5|6"));
        QVERIFY(e.win.w.act(QStringLiteral("data.goalSeek"))->isEnabled());
    }

    void aSaveAskedForInsideGoalSeekWaitsForTheEndAndSavesTheAnswer()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1*2+3", "D1=x"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("y"));                                               // (something to save)
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QString trialAtSave;
        QTimer::singleShot(0, &w, [&] {   // (fires in the events the seek lets through between trials)
            trialAtSave = w.session()->editText(0, 0, 0);
            w.act(QStringLiteral("file.save"))->trigger();
        });
        QVERIFY(w.goalSeekIn(QStringLiteral("B1"), QStringLiteral("11"), QStringLiteral("A1")));
        QVERIFY2(savedOk(saved), "the save failed");
        QVERIFY2(!trialAtSave.isEmpty() && trialAtSave != QLatin1String("4"), qPrintable(trialAtSave));   // (the save was asked for while a trial value was in the cell)
        QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:4"));      // (not a value tried on the way)
        QCOMPARE(savedCell(e.path, QStringLiteral("D1")), QStringLiteral("i:y"));
    }

    void goalSeekIsRefusedWhileASaveWaitsForTheCalculation()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1*2+3"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setWorkerPace(20000);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("2"));
        QVERIFY(w.isCalculating());
        QVERIFY(w.save());                                                          // (waits for the calculation)
        const int steps = w.undoStack()->count();
        QVERIFY(!w.goalSeekIn(QStringLiteral("B1"), QStringLiteral("11"), QStringLiteral("A1")));
        QCOMPARE(w.undoStack()->count(), steps);
        w.setWorkerPace(0);
        QVERIFY(e.settle());
    }

    void goalSeekLeavesTheSheetAloneWhenItFailsOrIsAskedWrong()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1*0+5", "C1==A1+1", "D1=text"})));
        QVERIFY(e.settle());
        QStringList told;
        e.win.w.setNotifier([&](const QString &, const QString &text) { told << text; });
        QVERIFY(!e.win.w.goalSeekIn(QStringLiteral("B1"), QStringLiteral("11"), QStringLiteral("A1")));   // (no number changes it)
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:B1")), QStringLiteral("1|5"));
        QVERIFY(!e.win.w.goalSeekIn(QStringLiteral("A1"), QStringLiteral("11"), QStringLiteral("D1")));   // (no formula in the first)
        QVERIFY(!e.win.w.goalSeekIn(QStringLiteral("B1"), QStringLiteral("11"), QStringLiteral("C1")));   // (a formula in the cell to change)
        QVERIFY(!e.win.w.goalSeekIn(QStringLiteral("B1"), QStringLiteral("abc"), QStringLiteral("A1")));
        QVERIFY(!e.win.w.goalSeekIn(QStringLiteral("B1"), QStringLiteral("11"), QStringLiteral("B1")));
        QCOMPARE(told.size(), 5);
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("1|5|2|text"));
    }

    void functionMenusListEachKindAndRememberWhatWasUsed()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        auto menuOf = [&](const QString &text) -> QMenu * {
            for (QToolButton *b : w.findChildren<QToolButton *>())
                if (b->text().replace(QStringLiteral("&&"), QStringLiteral("&")) == text && b->menu()) return b->menu();
            return nullptr;
        };
        auto names = [](QMenu *m) {
            QStringList out;
            for (QAction *a : m->actions()) out << a->text();
            return out;
        };
        QMenu *financial = menuOf(QStringLiteral("Financial"));
        QVERIFY(financial);
        QVERIFY(names(financial).contains(QStringLiteral("PMT")));
        QVERIFY(!names(financial).contains(QStringLiteral("SUM")));
        for (const char *kind : {"Logical", "Text", "Date & Time", "Lookup & Reference", "Math & Trig"}) {
            QMenu *m = menuOf(QString::fromLatin1(kind));
            QVERIFY2(m, kind);
            QVERIFY2(m->actions().size() > 3, kind);
        }
        QVERIFY(names(menuOf(QStringLiteral("Math & Trig"))).contains(QStringLiteral("SUM")));
        QMenu *more = menuOf(QStringLiteral("More Functions"));
        QVERIFY(more);
        QStringList kinds = names(more);
        QVERIFY(kinds.contains(QStringLiteral("Statistical")));
        QVERIFY(kinds.contains(QStringLiteral("Database")));
        // nothing used yet (an earlier test may have used one)
        Settings::get().setValue(QStringLiteral("formulas/recentFunctions"), QStringList());
        QMenu *recent = menuOf(QStringLiteral("Recently Used"));
        QVERIFY(recent);
        QVERIFY(w.recentFunctions().isEmpty());
        // a function picked from a menu opens the arguments for that function
        QString seen;
        w.setArgumentsAsker([&seen](FunctionArgumentsDialog *d) {
            seen = d->functionName();
            return false;
        });
        for (QAction *a : financial->actions())
            if (a->text() == QStringLiteral("PMT")) a->trigger();
        QCOMPARE(seen, QStringLiteral("PMT"));
        for (QAction *a : menuOf(QStringLiteral("Logical"))->actions())
            if (a->text() == QStringLiteral("IF")) a->trigger();
        QCOMPARE(w.recentFunctions(), (QStringList{"IF", "PMT"}));
        Q_EMIT recent->aboutToShow();
        QCOMPARE(names(recent), (QStringList{"IF", "PMT"}));
    }

    void subtotalAddsARowUnderEachRunAndAGrandTotalAndGroupsTheRows()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Region", "B1=Sales", "C1=Units", "A2=East", "B2=10", "C2=1", "A3=East", "B3=20", "C3=2", "A4=West", "B4=5", "C4=3", "A6=below", "B6=me"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const int steps = w.undoStack()->count();
        e.select(QStringLiteral("A2"));
        QVERIFY(w.subtotalIn(0, 9, {1, 2}));
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:C7")), QStringLiteral("Region|Sales|Units|East|10|1|East|20|2|East Total|30|3|West|5|3|West Total|5|3|Grand Total|35|6"));
        QCOMPARE(shown(e, QStringLiteral("A9:B9")), QStringLiteral("below|me"));   // (what was under the list moved down two rows... and three)
        QCOMPARE(w.session()->editText(0, 3, 1), QStringLiteral("=SUBTOTAL(9,B2:B3)"));
        QCOMPARE(w.session()->editText(0, 6, 1), QStringLiteral("=SUBTOTAL(9,B2:B6)"));
        const crunch::Outline rows = w.session()->outlineOf(0, false);
        QCOMPARE(int(rows.levelOf(1)), 1);
        QCOMPARE(int(rows.levelOf(2)), 1);
        QCOMPARE(int(rows.levelOf(3)), 0);
        QCOMPARE(int(rows.levelOf(4)), 1);
        QCOMPARE(int(rows.levelOf(6)), 0);
        QCOMPARE(w.undoStack()->count(), steps + 1);
        // applied again, the subtotals are replaced, not doubled
        e.select(QStringLiteral("A2"));
        QVERIFY(w.subtotalIn(0, 9, {1}));
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:C7")), QStringLiteral("Region|Sales|Units|East|10|1|East|20|2|East Total|30||West|5|3|West Total|5||Grand Total|35|"));
        // Remove All gives the list back
        e.select(QStringLiteral("A2"));
        QVERIFY(w.removeSubtotalsIn());
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:C4")), QStringLiteral("Region|Sales|Units|East|10|1|East|20|2|West|5|3"));
        QCOMPARE(shown(e, QStringLiteral("A6:B6")), QStringLiteral("below|me"));
        QCOMPARE(int(w.session()->outlineOf(0, false).levelOf(1)), 0);
        // and Undo of the whole is one step
        pressUndo(&w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:C7")), QStringLiteral("Region|Sales|Units|East|10|1|East|20|2|East Total|30||West|5|3|West Total|5||Grand Total|35|"));
    }

    void subtotalRefusesWhatItCannotDo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Region", "B1=Sales", "A2=East", "B2=10"})));
        QVERIFY(e.settle());
        QStringList told;
        e.win.w.setNotifier([&](const QString &, const QString &text) { told << text; });
        e.select(QStringLiteral("A2"));
        QVERIFY(!e.win.w.subtotalIn(0, 9, {}));   // (no column to total)
        QVERIFY(!e.win.w.subtotalIn(0, 99, {1}));   // (no such function)
        e.select(QStringLiteral("H20"));
        QVERIFY(!e.win.w.subtotalIn(0, 9, {1}));   // (no list there)
        QCOMPARE(told.size(), 3);
        QVERIFY(!e.win.w.removeSubtotalsIn());
        QCOMPARE(shown(e, QStringLiteral("A1:B2")), QStringLiteral("Region|Sales|East|10"));
    }

    void advancedFilterHidesRowsThatMissTheConditionsAndClearShowsThem()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=Age", "A2=Ann", "B2=30", "A3=Bob", "B3=40", "A4=Cy", "B4=50", "A5=Anna", "B5=20", "A6=ann", "B6=35",
                                "E1=Name", "F1=Age", "E2=An", "F2=>25"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const int steps = w.undoStack()->count();
        QVERIFY(w.advancedFilterIn(QStringLiteral("A1:B6"), QStringLiteral("E1:F2")));
        QVERIFY(e.settle());
        // (a name starts with "An" without regard to capitals, and the age is over 25: Ann and ann)
        QVERIFY(!rowHidden(e, 1));
        QVERIFY(rowHidden(e, 2));
        QVERIFY(rowHidden(e, 3));
        QVERIFY(rowHidden(e, 4));
        QVERIFY(!rowHidden(e, 5));
        QCOMPARE(w.undoStack()->count(), steps + 1);
        // Clear shows them again
        w.act(QStringLiteral("filter.clear"))->trigger();
        QVERIFY(e.settle());
        for (quint32 row = 1; row <= 5; ++row) QVERIFY(!rowHidden(e, row));
    }

    void advancedFilterAndOrEqualsBlankAndWildcards()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=Age", "A2=Ann", "B2=30", "A3=Bob", "B3=40", "A4=Cy", "B4=50", "A5=Dee", "A6=Bo",
                                "E1=Name", "F1=Age", "E2=Ann", "E3=B*", "F3=>45", "E4=<>B*", "F4=<>"})));
        QVERIFY(e.settle());
        crunch::AdvancedFilterSpec spec;
        spec.list = rangeOf(QStringLiteral("A1:B6"));
        spec.criteria = rangeOf(QStringLiteral("E1:F4"));
        auto found = crunch::advancedFilterRows(*e.win.w.session(), 0, spec);
        QVERIFY2(found.ok, qPrintable(found.problem));
        // "Ann"; or "B*" with an age over 45 (no one); or a name that does not start with B and an age at all (Ann and Cy; Dee has no age)
        QCOMPARE(found.rows, (std::vector<quint32>{1, 3}));
        spec.criteria = rangeOf(QStringLiteral("E1:F3"));
        found = crunch::advancedFilterRows(*e.win.w.session(), 0, spec);
        QCOMPARE(found.rows, (std::vector<quint32>{1}));
        // only a heading row lets every row through
        spec.criteria = rangeOf(QStringLiteral("E1:F1"));
        found = crunch::advancedFilterRows(*e.win.w.session(), 0, spec);
        QCOMPARE(found.rows.size(), size_t(5));
    }

    void advancedFilterCopiesTheRowsAndLeavesOutRepeats()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=Age", "A2=Ann", "B2=30", "A3=Bob", "B3=40", "A4=Ann", "B4=30", "A5=Cy", "B5=50",
                                "E1=Age", "E2=>25"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.advancedFilterIn(QStringLiteral("A1:B5"), QStringLiteral("E1:E2"), QStringLiteral("H1"), true));
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("H1:I4")), QStringLiteral("Name|Age|Ann|30|Bob|40|Cy|50"));
        QCOMPARE(shown(e, QStringLiteral("H5:I5")), QStringLiteral("|"));
        QVERIFY(!rowHidden(e, 1));
        // refused: a place that is over the list
        QStringList told;
        w.setNotifier([&](const QString &, const QString &text) { told << text; });
        QVERIFY(!w.advancedFilterIn(QStringLiteral("A1:B5"), QStringLiteral("E1:E2"), QStringLiteral("A3")));
        QVERIFY(!w.advancedFilterIn(QStringLiteral("A1:B5"), QStringLiteral("A2:B3")));
        QVERIFY(!w.advancedFilterIn(QStringLiteral("nonsense"), QStringLiteral("E1:E2")));
        QCOMPARE(told.size(), 3);
    }

    void formulasReachOtherSheetsWholeColumnsAndNames()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "A2=2", "A3=3"}, {"A1=10", "B2=20"}, {"A1=100"}}, {}, {},
                                    QStringLiteral("<definedName name=\"Rate\">Sheet2!$A$1</definedName>"))));
        QVERIFY(e.settle());
        const QStringList formulas = {"=Sheet2!A1", "=Sheet2!A1:B2", "=SUM(Sheet2!A1:B2)", "=SUM(Sheet1:Sheet3!A1)", "=SUM(A:A)", "=SUM(Sheet2!2:2)", "=Rate*2", "='Sheet2'!B2+Sheet3!A1"};
        for (int i = 0; i < formulas.size(); ++i) {
            e.select(QStringLiteral("D%1").arg(i + 1));
            e.enter(formulas[i]);
        }
        QVERIFY(e.settle());
        QCOMPARE(e.shownAt(QStringLiteral("D1")), QStringLiteral("10"));
        QCOMPARE(e.shownAt(QStringLiteral("D3")), QStringLiteral("30"));
        QCOMPARE(e.shownAt(QStringLiteral("D4")), QStringLiteral("111"));   // (A1 of the three sheets)
        QCOMPARE(e.shownAt(QStringLiteral("D5")), QStringLiteral("6"));   // (column A of Sheet1: 1+2+3; the formulas in D are not in A)
        QCOMPARE(e.shownAt(QStringLiteral("D6")), QStringLiteral("20"));
        QCOMPARE(e.shownAt(QStringLiteral("D7")), QStringLiteral("20"));
        QCOMPARE(e.shownAt(QStringLiteral("D8")), QStringLiteral("120"));
        // and they are kept as written
        QCOMPARE(e.win.w.session()->editText(0, 3, 3), QStringLiteral("=SUM(Sheet1:Sheet3!A1)"));
        QCOMPARE(e.win.w.session()->editText(0, 7, 3), QStringLiteral("='Sheet2'!B2+Sheet3!A1"));   // (as typed)
    }

    void aStepMadeOfSeveralChangesLeavesTheWindowNotCalculating()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1*2+3", "D1=Region", "E1=Sales", "D2=East", "E2=10", "D3=East", "E3=20", "D4=West", "E4=5",
                                "G1=Region", "G2=East"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.goalSeekIn(QStringLiteral("B1"), QStringLiteral("11"), QStringLiteral("A1")));
        QVERIFY(!w.isCalculating());   // (right after: the formulas were worked out before the step ended)
        e.select(QStringLiteral("D2"));
        QVERIFY(w.subtotalIn(0, 9, {1}));
        QTRY_VERIFY_WITH_TIMEOUT(!w.isCalculating(), 5000);
        QVERIFY(e.settle());
        QVERIFY(w.advancedFilterIn(QStringLiteral("D1:E7"), QStringLiteral("G1:G2")));
        QTRY_VERIFY_WITH_TIMEOUT(!w.isCalculating(), 5000);
        QVERIFY(e.settle());
        QVERIFY(!w.isCalculating());
    }

    void errorCheckingFindsFormulaErrorsAndNumbersKeptAsTextAndCanIgnoreThem()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1/0", "A3=word", "B3==\"x\"+1", "A4=7", "B4==A4+1"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("'12"));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        auto problems = w.formulaProblems();
        QCOMPARE(problems.size(), size_t(3));
        QCOMPARE(int(problems[0].row), 0);   // B1: divides by zero
        QCOMPARE(int(problems[0].col), 1);
        QVERIFY(problems[0].explanation.contains(QStringLiteral("divides by zero")));
        QCOMPARE(int(problems[1].row), 1);   // A2: a number kept as text
        QVERIFY(problems[1].kind == crunch::FormulaProblem::Kind::NumberAsText);
        QCOMPARE(int(problems[2].row), 2);   // B3: text where a number is needed
        QVERIFY(problems[2].explanation.contains(QStringLiteral("wrong kind")));
        w.ignoreFormulaProblem(1, 0);
        QCOMPARE(w.formulaProblems().size(), size_t(2));
        // the command goes to each cell and says so when there is none
        QStringList told;
        w.setNotifier([&](const QString &, const QString &text) { told << text; });
        w.ignoreFormulaProblem(0, 1);
        w.ignoreFormulaProblem(2, 1);
        QVERIFY(w.formulaProblems().empty());
        QVERIFY(w.act(QStringLiteral("formulas.errorChecking"))->isEnabled());
        w.act(QStringLiteral("formulas.errorChecking"))->trigger();
        QCOMPARE(told.size(), 1);
    }

    void theWatchWindowListsWatchedCellsAndKeepsTheirValuesCurrent()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "B1==A1*2"}, {"A1=100"}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.act(QStringLiteral("formulas.watchWindow"))->trigger();
        QVERIFY(w.watchWindow());
        QCOMPARE(w.watchWindow()->rowCount(), 0);
        QVERIFY(w.addWatch(0, 0, 1));
        QVERIFY(w.addWatch(1, 0, 0));
        QVERIFY(!w.addWatch(0, 0, 1));   // (once)
        QCOMPARE(w.watchWindow()->rowCount(), 2);
        auto cell = [&](int r, int c) { return w.watchWindow()->table()->item(r, c)->text(); };
        QCOMPARE(cell(0, 0), QStringLiteral("Sheet1"));
        QCOMPARE(cell(0, 1), QStringLiteral("B1"));
        QCOMPARE(cell(0, 2), QStringLiteral("2"));
        QCOMPARE(cell(0, 3), QStringLiteral("=A1*2"));
        QCOMPARE(cell(1, 0), QStringLiteral("Sheet2"));
        QCOMPARE(cell(1, 2), QStringLiteral("100"));
        // an edit that changes what a watched cell comes to shows
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("21"));
        QVERIFY(e.settle());
        QTRY_COMPARE(cell(0, 2), QStringLiteral("42"));
    }

    // The Watch Window's rows are the watches of sheets that are there: Delete and Go To act on the watch of the row picked, not the one at that place in the list of all.
    void watchRowsActOnTheirOwnWatch()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1"}, {"A1=2"}, {"A1=3"}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.act(QStringLiteral("formulas.watchWindow"))->trigger();
        QVERIFY(w.addWatch(1, 0, 0));
        QVERIFY(w.addWatch(0, 0, 0));
        QVERIFY(w.addWatch(2, 0, 0));
        QVERIFY(w.session()->deleteSheet(1).ok);
        Q_EMIT w.calculationFinished();
        auto t = w.watchWindow()->table();
        auto names = [&] {
            QStringList out;
            for (int r = 0; r < t->rowCount(); ++r) out << t->item(r, 0)->text();
            return out;
        };
        QCOMPARE(names(), (QStringList{"Sheet1", "Sheet3"}));
        // Go To on the second row is Sheet3
        Q_EMIT w.watchWindow()->goToRequested(1);
        QCOMPARE(w.activeSheet(), 2);
        // Delete on the second row takes the Sheet3 watch
        Q_EMIT w.watchWindow()->deleteRequested(QList<int>{1});
        QCOMPARE(names(), (QStringList{"Sheet1"}));
        QCOMPARE(w.watchCount(), 2);   // (the watch of the deleted sheet waits for Undo)
    }

    // A watch is on the cell it was put on: rows and columns inserted or deleted move it, and one on a deleted row is let go of.
    void watchesFollowInsertedAndDeletedRowsAndColumns()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=11", "A2=22", "B1=33"}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.act(QStringLiteral("formulas.watchWindow"))->trigger();
        QVERIFY(w.addWatch(0, 0, 0));
        QVERIFY(w.addWatch(0, 1, 0));
        auto t = w.watchWindow()->table();
        QVERIFY(w.session()->insertRows(0, 0, 1).ok);
        QVERIFY(e.settle());
        Q_EMIT w.calculationFinished();
        QCOMPARE(t->item(0, 1)->text(), QStringLiteral("A2"));
        QCOMPARE(t->item(0, 2)->text(), QStringLiteral("11"));
        QCOMPARE(t->item(1, 1)->text(), QStringLiteral("A3"));
        QVERIFY(w.session()->insertColumns(0, 0, 1).ok);
        QVERIFY(e.settle());
        Q_EMIT w.calculationFinished();
        QCOMPARE(t->item(0, 1)->text(), QStringLiteral("B2"));
        QVERIFY(w.session()->deleteRows(0, 2, 1).ok);   // the row of the 22
        QVERIFY(e.settle());
        Q_EMIT w.calculationFinished();
        QCOMPARE(t->rowCount(), 1);
        QCOMPARE(t->item(0, 1)->text(), QStringLiteral("B2"));
        QCOMPARE(t->item(0, 2)->text(), QStringLiteral("11"));
        QCOMPARE(w.watchCount(), 1);
    }

    // The watches belong to the workbook: a window that takes another workbook has none of the old one's.
    void watchesGoWithTheWorkbook()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.addWatch(0, 0, 0));
        QCOMPARE(w.watchCount(), 1);
        const QString path = e.folder.write(QStringLiteral("other.xlsx"), bookOfSheets({{"A1=5"}}));
        QVERIFY(e.win.open(path));
        QCOMPARE(w.watchCount(), 0);
    }

    void evaluateFormulaWorksOutOnePartAtATime()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=2", "A2=3", "A3=5", "B1==(A1+A2)*SUM(A1:A3)"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("B1"));
        auto steps = e.win.w.formulaStepsOfCursorCell();
        QVERIFY(steps);
        QCOMPARE(steps->text(), QStringLiteral("(A1+A2)*SUM(A1:A3)"));
        QCOMPARE(steps->text().mid(steps->underlineStart(), steps->underlineLength()), QStringLiteral("A1"));   // the leftmost cell first
        QStringList seen{steps->text()};
        for (int i = 0; i < 20 && !steps->finished(); ++i) {
            QVERIFY2(steps->step(), qPrintable(steps->problem()));
            seen << steps->text();
        }
        QCOMPARE(seen, (QStringList{"(A1+A2)*SUM(A1:A3)", "(2+A2)*SUM(A1:A3)", "(2+3)*SUM(A1:A3)", "5*SUM(A1:A3)", "5*10", "50"}));
        QVERIFY(steps->finished());
        QVERIFY(!steps->step());
        steps->restart();
        QCOMPARE(steps->text(), QStringLiteral("(A1+A2)*SUM(A1:A3)"));
        // a cell that holds no formula has none to go through
        e.select(QStringLiteral("A1"));
        QVERIFY(!e.win.w.formulaStepsOfCursorCell());
    }

    void evaluateFormulaStopsAtAPartThatCannotBeWorkedOut()
    {
        crunch::FormulaSteps steps(QStringLiteral("1+NOSUCH(2)"), [](const QString &part, QString *problem) -> std::optional<QString> {
            if (part.contains(QStringLiteral("NOSUCH"))) {
                *problem = QStringLiteral("unknown");
                return std::nullopt;
            }
            return part;
        });
        QCOMPARE(steps.text().mid(steps.underlineStart(), steps.underlineLength()), QStringLiteral("NOSUCH(2)"));
        QVERIFY(!steps.step());
        QCOMPARE(steps.problem(), QStringLiteral("unknown"));
        QCOMPARE(steps.text(), QStringLiteral("1+NOSUCH(2)"));
        QVERIFY(!steps.finished());
    }

    void aRuleOfValidationTakesItsCellsFromTheRulesThatHadThem()
    {
        using jp::office::sheet::Validation;
        Validation old;
        old.type = Validation::Type::Whole;
        old.formula1 = "1";
        old.formula2 = "9";
        old.ranges = {rangeOf(QStringLiteral("B1:B10"))};
        Validation fresh;
        fresh.type = Validation::Type::List;
        fresh.formula1 = "\"a,b\"";
        const auto rules = crunch::withRuleOn({old}, rangeOf(QStringLiteral("B3:B4")), fresh);
        QCOMPARE(rules.size(), 2);
        QCOMPARE(rules[0].ranges.size(), 2);   // B1:B2 and B5:B10
        QVERIFY(rules[0].ranges[0] == rangeOf(QStringLiteral("B1:B2")));
        QVERIFY(rules[0].ranges[1] == rangeOf(QStringLiteral("B5:B10")));
        QVERIFY(rules[1].type == Validation::Type::List);
        QVERIFY(rules[1].ranges[0] == rangeOf(QStringLiteral("B3:B4")));
        // a rule that allows any value and has no messages is no rule; it takes the cells' rules away
        Validation none;
        const auto cleared = crunch::withRuleOn(rules, rangeOf(QStringLiteral("A1:C20")), none);
        QVERIFY(cleared.isEmpty());
        // the limits as the file writes them
        bool ok = false;
        QCOMPARE(crunch::validationLimitText(Validation::Type::List, QStringLiteral("a,b \"c\""), &ok), QStringLiteral("\"a,b \"\"c\"\"\""));
        QVERIFY(ok);
        QCOMPARE(crunch::validationLimitText(Validation::Type::List, QStringLiteral("=$A$1:$A$5"), &ok), QStringLiteral("$A$1:$A$5"));
        QCOMPARE(crunch::validationLimitText(Validation::Type::Whole, QStringLiteral("12"), &ok), QStringLiteral("12"));
        QCOMPARE(crunch::validationLimitText(Validation::Type::Date, QStringLiteral("1900-01-02"), &ok), QStringLiteral("2"));
        crunch::validationLimitText(Validation::Type::Whole, QStringLiteral("many"), &ok);
        QVERIFY(!ok);
        QCOMPARE(crunch::validationLimitShown(Validation::Type::List, QStringLiteral("\"a,b\"")), QStringLiteral("a,b"));
    }

    void dataValidationIsEnforcedUndoneAndKeptInTheSavedFile()
    {
        using jp::office::sheet::Validation;
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=5"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const int steps = w.undoStack()->count();
        Validation rule;
        rule.type = Validation::Type::Whole;
        rule.compare = Validation::Compare::Between;
        rule.formula1 = "1";
        rule.formula2 = "9";
        rule.showError = true;
        rule.errorTitle = "Too big";
        rule.error = "Use 1 to 9.";
        rule.showInput = true;
        rule.prompt = "A digit";
        QVERIFY(w.setValidationOn(rangeOf(QStringLiteral("B2:B3")), rule));
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QVERIFY(w.calculation()->validationAt(0, 1, 1));
        QVERIFY(!w.calculation()->validationAt(0, 0, 1));
        // what is typed there is held to it
        QVERIFY(!w.session()->type(0, 1, 1, QStringLiteral("20")).ok);
        QVERIFY(w.session()->type(0, 1, 1, QStringLiteral("4")).ok);
        QVERIFY(e.settle());
        // kept in the file, with its messages, and read back
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(saved.wait(20000) && saved.last().value(1).toBool());
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const Validation *back = second.w.calculation()->validationAt(0, 2, 1);
        QVERIFY(back);
        QVERIFY(back->type == Validation::Type::Whole);
        QCOMPARE(back->formula2, QStringLiteral("9"));
        QCOMPARE(back->error, QStringLiteral("Use 1 to 9."));
        QCOMPARE(back->prompt, QStringLiteral("A digit"));
        QVERIFY(!second.w.calculation()->validationAt(0, 0, 1));
        // taken away again, and Undo gives it back
        QVERIFY(w.setValidationOn(rangeOf(QStringLiteral("B2:B3")), std::nullopt));
        QVERIFY(!w.calculation()->validationAt(0, 1, 1));
        w.undoStack()->undo();
        QVERIFY(w.calculation()->validationAt(0, 1, 1));
        QVERIFY(w.act(QStringLiteral("data.validation"))->isEnabled());
    }

    void consolidateCombinesRangesByPositionAndByLabels()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "B1=2", "A2=3", "B2=4", "G1=Q1", "F2=East", "G2=5", "F3=West", "G3=7"}, {"A1=10", "B1=20", "A2=30", "B2=40", "G1=Q1", "F2=West", "G2=3", "F3=North", "G3=1"}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const int steps = w.undoStack()->count();
        // by position, with links: each result names its source cells
        QVERIFY(w.consolidateIn({"A1:B2", "Sheet2!A1:B2"}, 9, false, false, true, QStringLiteral("D5")));
        QVERIFY(e.settle());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QVERIFY(!w.isCalculating());
        QCOMPARE(shown(e, QStringLiteral("D5:E6")), QStringLiteral("11|22|33|44"));
        QCOMPARE(w.session()->editText(0, 4, 3), QStringLiteral("=SUM(A1,Sheet2!A1)"));
        // by position, as numbers, with another function
        QVERIFY(w.consolidateIn({"A1:B2", "Sheet2!A1:B2"}, 1, false, false, false, QStringLiteral("H5")));
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("H5:I6")), QStringLiteral("5.5|11|16.5|22"));
        QCOMPARE(w.session()->editText(0, 4, 7), QStringLiteral("5.5"));
        // by labels: the same label is combined, wherever it is
        QVERIFY(w.consolidateIn({"F1:G3", "Sheet2!F1:G3"}, 9, true, true, false, QStringLiteral("J10")));
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("J10:K13")), QStringLiteral("|Q1|East|5|West|10|North|1"));
        pressUndo(&w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("J10:K13")), QStringLiteral("|||||||"));
    }

    // 24 points: t = 1..24 in column A, and in column B 10 + t plus a season of 4 (3, -1, -3, 1), a little noise on the second half so that the bounds are not zero
    static int liveSheetCount(CrunchWindow &w)
    {
        int n = 0;
        for (const auto &sh : w.loaded()->book->info().sheets)
            if (!sh.deleted) ++n;
        return n;
    }
    static QStringList seasonalCells(int n = 24)
    {
        const double pat[] = {3, -1, -3, 1};
        QStringList cells;
        for (int i = 1; i <= n; ++i) {
            cells << QStringLiteral("A%1=%2").arg(i).arg(i);
            cells << QStringLiteral("B%1=%2").arg(i).arg(10 + i + pat[(i - 1) % 4] + (i > n / 2 ? (i % 3 - 1) * 0.5 : 0));
        }
        return cells;
    }

    void forecastSheetIsMadeOfLiveFormulasAndRecalculates()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf(seasonalCells())));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const int sheets = liveSheetCount(w);
        QVERIFY(w.forecastSheetIn(QStringLiteral("A1:A24"), QStringLiteral("B1:B24"), 30, std::numeric_limits<double>::quiet_NaN(), 0.95, 1, true, true, 1, false));
        QVERIFY(e.settle());
        QCOMPARE(liveSheetCount(w), sheets + 1);
        const int fs = w.activeSheet();
        QVERIFY(fs != 0);
        QCOMPARE(w.session()->sheetOrder().value(0), fs);   // (put before the sheet that was in view)
        QCOMPARE(shown(e, QStringLiteral("A1:E1")), QStringLiteral("Timeline|Values|Forecast|Lower Confidence Bound|Upper Confidence Bound"));
        // 24 rows of data, the line from the last one, and the six steps 25 to 30
        QCOMPARE(w.session()->editText(fs, 24, 2), QStringLiteral("=B25"));
        QCOMPARE(w.session()->editText(fs, 25, 2), QStringLiteral("=FORECAST.ETS(A26,$B$2:$B$25,$A$2:$A$25,1,1,1)"));
        QCOMPARE(w.session()->editText(fs, 25, 3), QStringLiteral("=C26-FORECAST.ETS.CONFINT(A26,$B$2:$B$25,$A$2:$A$25,0.95,1,1,1)"));
        QCOMPARE(w.session()->editText(fs, 25, 4), QStringLiteral("=C26+FORECAST.ETS.CONFINT(A26,$B$2:$B$25,$A$2:$A$25,0.95,1,1,1)"));
        QVERIFY(w.session()->editText(fs, 26, 0) == QStringLiteral("26") && w.session()->editText(fs, 30, 0) == QStringLiteral("30"));
        QVERIFY(w.session()->editText(fs, 31, 0).isEmpty());
        QCOMPARE(w.session()->editText(fs, 0, 6), QStringLiteral("Forecast Statistics"));
        QCOMPARE(w.session()->editText(fs, 1, 6), QStringLiteral("Alpha"));
        QCOMPARE(w.session()->editText(fs, 7, 6), QStringLiteral("RMSE"));
        QCOMPARE(w.session()->editText(fs, 7, 7), QStringLiteral("=FORECAST.ETS.STAT($B$2:$B$25,$A$2:$A$25,7,1,1,1)"));
        // the numbers are those of the function and of the plan
        crunch::ForecastInput input;
        for (int i = 1; i <= 24; ++i) input.pairs.push_back({double(i), w.session()->valueAt(0, quint32(i - 1), 1).number()});
        crunch::ForecastSettings settings;
        settings.end = 30;
        const crunch::ForecastResult result = crunch::computeForecast(input, settings, jp::calc::DateSystem::Date1900);
        QVERIFY2(result.problem.isEmpty(), qPrintable(result.problem));
        QCOMPARE(int(result.points.size()), 6);
        QCOMPARE(result.season, 4);
        for (int i = 0; i < 6; ++i) {
            const double got = w.session()->valueAt(fs, quint32(25 + i), 2).number();
            QVERIFY(std::fabs(got - result.points[size_t(i)].forecast) < 1e-9);
            QVERIFY(std::fabs(w.session()->valueAt(fs, quint32(25 + i), 3).number() - result.points[size_t(i)].lower) < 1e-9);
            QVERIFY(std::fabs(w.session()->valueAt(fs, quint32(25 + i), 4).number() - result.points[size_t(i)].upper) < 1e-9);
            QVERIFY(w.session()->valueAt(fs, quint32(25 + i), 4).number() > got);
        }
        // a value changed on the new sheet works the forecast out again
        const double before = w.session()->valueAt(fs, 25, 2).number();
        QVERIFY(w.session()->type(fs, 23, 1, QStringLiteral("500")).ok);
        QVERIFY(e.settle());
        QVERIFY(std::fabs(w.session()->valueAt(fs, 25, 2).number() - before) > 1);
        // the chart reads the table
        crunch::SheetObjects *o = w.objects();
        QTRY_VERIFY(o->shownSheet() == fs && o->layer()->count() == 1);
        const jp::office::Chart chart = o->chartNow(fs, 0);
        QStringList refs;
        for (const auto &plot : chart.plots)
            for (const auto &series : plot.series) refs << series.valueRef;
        QCOMPARE(refs, (QStringList{"Forecast!$B$2:$B$31", "Forecast!$C$2:$C$31", "Forecast!$D$2:$D$31", "Forecast!$E$2:$E$31"}));
    }

    void forecastSheetIsOneStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf(seasonalCells())));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const int sheets = liveSheetCount(w), steps = w.undoStack()->count();
        QVERIFY(w.forecastSheetIn(QStringLiteral("A1:A24"), QStringLiteral("B1:B24"), 30, std::numeric_limits<double>::quiet_NaN(), 0.9, 0, false, false, 7, true));
        QVERIFY(e.settle());
        const int fs = w.activeSheet();
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Forecast Sheet"));
        QVERIFY(w.session()->editText(fs, 1, 6).isEmpty());   // (no statistics asked for)
        QCOMPARE(w.session()->editText(fs, 25, 2), QStringLiteral("=FORECAST.ETS(A26,$B$2:$B$25,$A$2:$A$25,0,0,7)"));
        pressUndo(&w);
        QVERIFY(e.settle());
        QCOMPARE(liveSheetCount(w), sheets);
        QVERIFY(!w.isModified());
        pressRedo(&w);
        QVERIFY(e.settle());
        QCOMPARE(liveSheetCount(w), sheets + 1);
        QCOMPARE(w.session()->editText(fs, 25, 2), QStringLiteral("=FORECAST.ETS(A26,$B$2:$B$25,$A$2:$A$25,0,0,7)"));
        QTRY_VERIFY(w.objects()->shownSheet() == fs && w.objects()->layer()->count() == 1);
    }

    void forecastSheetIsSavedWithItsChartAndReopens()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf(seasonalCells())));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.forecastSheetIn(QStringLiteral("A1:A24"), QStringLiteral("B1:B24"), 30, std::numeric_limits<double>::quiet_NaN(), 0.95, 1, true, true, 1, false));
        QVERIFY(e.settle());
        const int fs = w.activeSheet();
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(saved.wait(20000) && saved.last().value(1).toBool());
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.session()->editText(0, 25, 2), QStringLiteral("=FORECAST.ETS(A26,$B$2:$B$25,$A$2:$A$25,1,1,1)"));
        QCOMPARE(second.w.session()->editText(0, 7, 7), QStringLiteral("=FORECAST.ETS.STAT($B$2:$B$25,$A$2:$A$25,7,1,1,1)"));
        QVERIFY(std::fabs(second.w.session()->valueAt(0, 25, 2).number() - w.session()->valueAt(fs, 25, 2).number()) < 1e-9);
        crunch::SheetObjects *o = second.w.objects();
        QTRY_VERIFY(o->shownSheet() == 0 && o->layer()->count() == 1);
        QCOMPARE(int(o->chartNow(0, 0).plots.size()), 1);
        QCOMPARE(o->chartNow(0, 0).plots[0].series.size(), 4);
        QCOMPARE(o->chartNow(0, 0).plots[0].series[2].valueRef, QStringLiteral("Forecast!$D$2:$D$31"));
    }

    // A workbook whose FORECAST.ETS cells have the results a file stored (that are not what this program works out): D1 to G1 read the 24 points of A1:B24, and H1 reads D1.
    static QByteArray forecastFileWithStoredResults()
    {
        const double pat[] = {3, -1, -3, 1};
        QString rows;
        const QString both = QStringLiteral("$B$1:$B$24,$A$1:$A$24");
        for (int i = 1; i <= 24; ++i) {
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%2</v></c>").arg(i).arg(10 + i + pat[(i - 1) % 4] + (i > 12 ? (i % 3 - 1) * 0.5 : 0));
            if (i == 1) {
                rows += QStringLiteral("<c r=\"D1\"><f>_xlfn.FORECAST.ETS(30,%1)</f><v>123.5</v></c>").arg(both);
                rows += QStringLiteral("<c r=\"E1\"><f>_xlfn.FORECAST.ETS.CONFINT(30,%1)</f><v>7.25</v></c>").arg(both);
                rows += QStringLiteral("<c r=\"F1\"><f>_xlfn.FORECAST.ETS.SEASONALITY(%1)</f><v>9</v></c>").arg(both);
                rows += QStringLiteral("<c r=\"G1\"><f>_xlfn.FORECAST.ETS.STAT(%1,7)</f><v>0.5</v></c>").arg(both);
                rows += QStringLiteral("<c r=\"H1\"><f>D1*2</f><v>247</v></c>");
            }
            rows += QStringLiteral("</row>");
        }
        return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>")));
    }

    // The forecast cells of a file keep the results the file stored (this program fits another way, and a file that is only opened and saved is not to have its numbers changed) until a cell they
    // read is changed; from then on they are worked out. A cell typed here is worked out from the start.
    void forecastResultsOfAFileAreKeptUntilTheirDataChanges()
    {
        EditWindow e;
        QVERIFY(e.open(forecastFileWithStoredResults()));
        QVERIFY(e.settle());
        const QStringList cells = {"D1", "E1", "F1", "G1", "H1"};
        const QStringList stored = {"123.5", "7.25", "9", "0.5", "247"};
        for (int i = 0; i < cells.size(); ++i) QCOMPARE(e.shownAt(cells[i]), stored[i]);
        // a cell typed here is worked out at once (a number that is not the stored one), and a cell that is not read by them changes nothing
        e.select(QStringLiteral("J1"));
        e.enter(QStringLiteral("=FORECAST.ETS(30,$B$1:$B$24,$A$1:$A$24)"));
        e.select(QStringLiteral("K1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        const QString typed = e.valueAt(QStringLiteral("J1"));
        QVERIFY2(typed.startsWith(QLatin1String("n:")) && typed != QLatin1String("n:123.5"), qPrintable(typed));
        for (int i = 0; i < cells.size(); ++i) QCOMPARE(e.shownAt(cells[i]), stored[i]);
        // saved, the file has the stored results and the formulas as they were
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        for (int i = 0; i < cells.size(); ++i) QCOMPARE(savedCell(e.path, cells[i]), QStringLiteral("n:") + stored[i]);
        QCOMPARE(savedFormula(e.path, QStringLiteral("D1")), QStringLiteral("_xlfn.FORECAST.ETS(30,$B$1:$B$24,$A$1:$A$24)"));
        // a cell they read is changed: they are worked out, and what reads them follows
        e.select(QStringLiteral("B5"));
        e.enter(QStringLiteral("500"));
        QVERIFY(e.settle());
        for (const QString &c : {QStringLiteral("D1"), QStringLiteral("E1"), QStringLiteral("F1"), QStringLiteral("G1")}) {
            const QString v = e.valueAt(c);
            QVERIFY2(v.startsWith(QLatin1String("n:")), qPrintable(c + v));
        }
        QVERIFY(e.valueAt(QStringLiteral("D1")) != QLatin1String("n:123.5") && e.valueAt(QStringLiteral("E1")) != QLatin1String("n:7.25"));
        QVERIFY(e.valueAt(QStringLiteral("G1")) != QLatin1String("n:0.5"));
        QCOMPARE(e.valueAt(QStringLiteral("D1")), e.valueAt(QStringLiteral("J1")));   // (the same forecast of the same numbers, as a cell made here gives)
        QCOMPARE(e.win.w.session()->valueAt(0, 0, 7).number(), 2 * e.win.w.session()->valueAt(0, 0, 3).number());
    }

    // A Forecast Sheet that is saved and opened again has the results it stored, and works them out when a value on it is changed.
    void forecastSheetStaysLiveWhenItIsOpenedAgain()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf(seasonalCells())));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.forecastSheetIn(QStringLiteral("A1:A24"), QStringLiteral("B1:B24"), 30, std::numeric_limits<double>::quiet_NaN(), 0.95, 1, true, true, 1, false));
        QVERIFY(e.settle());
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(saved.wait(20000) && saved.last().value(1).toBool());
        EditWindow second;
        QFile again(e.copyOfFile());
        QVERIFY(again.open(QIODevice::ReadOnly));
        QVERIFY(second.open(again.readAll()));
        QVERIFY(second.settle());
        const double stored = second.win.w.session()->valueAt(0, 25, 2).number();
        QVERIFY(std::isfinite(stored));
        QVERIFY(second.win.w.session()->type(0, 23, 1, QStringLiteral("500")).ok);
        QVERIFY(second.settle());
        QVERIFY(std::fabs(second.win.w.session()->valueAt(0, 25, 2).number() - stored) > 1);
        QVERIFY(std::fabs(second.win.w.session()->valueAt(0, 25, 4).number() - second.win.w.session()->valueAt(0, 25, 3).number()) > 0.01);
    }

    // The cells of a Forecast Sheet share one fitted model: a sheet of rows and bounds that all read the same series is prepared, searched for a season, and fitted once, and so is the sheet again after
    // a value it reads is changed (one more, not one for each cell).
    void forecastSheetWorksItsSeriesOnce()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf(seasonalCells(36))));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const jp::calc::ets::Counts start = jp::calc::ets::counts();
        QVERIFY(w.forecastSheetIn(QStringLiteral("A1:A36"), QStringLiteral("B1:B36"), 60, std::numeric_limits<double>::quiet_NaN(), 0.95, 1, true, true, 1, false));
        QVERIFY(e.settle());
        const jp::calc::ets::Counts made = jp::calc::ets::counts();
        const int fs = w.activeSheet();
        // (24 rows with a forecast and two bounds each, and the 7 statistics, read the same 36 points)
        QVERIFY(std::isfinite(w.session()->valueAt(fs, 59, 4).number()));
        QVERIFY2(made.fits - start.fits >= 1 && made.fits - start.fits <= 2, qPrintable(QString::number(made.fits - start.fits)));   // (the plan's own forecast is one, and the cells' the other)
        QVERIFY2(made.detects - start.detects <= 2, qPrintable(QString::number(made.detects - start.detects)));
        QVERIFY2(made.prepares - start.prepares <= 3, qPrintable(QString::number(made.prepares - start.prepares)));
        QVERIFY(w.session()->type(fs, 35, 1, QStringLiteral("500")).ok);
        QVERIFY(e.settle());
        const jp::calc::ets::Counts changed = jp::calc::ets::counts();
        QCOMPARE(changed.fits - made.fits, 1LL);
        QCOMPARE(changed.prepares - made.prepares, 1LL);
        QVERIFY(std::fabs(w.session()->valueAt(fs, 59, 2).number()) > 0);
    }

    // A Forecast Sheet of 20,000 points with the default end (10,000 rows of forecast and two bounds each) is worked out from one model: the count of models built is what is tested, not the time (which is
    // printed: it was 17 minutes before the model was shared).
    void forecastSheetOfTwentyThousandPointsBuildsOneModel()
    {
        const int n = 20000;
        QStringList cells;
        const double pat[] = {3, -1, -3, 1};
        for (int i = 1; i <= n; ++i) cells << QStringLiteral("A%1=%1").arg(i) << QStringLiteral("B%1=%2").arg(i).arg(10 + 0.01 * i + pat[(i - 1) % 4] + (i * 7919 % 13) * 0.05, 0, 'g', 12);
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QElapsedTimer timer;
        timer.start();
        const jp::calc::ets::Counts start = jp::calc::ets::counts();
        QStringList told;
        w.setNotifier([&](const QString &, const QString &text) { told << text; });
        QVERIFY2(w.forecastSheetIn(QStringLiteral("A1:A20000"), QStringLiteral("B1:B20000"), n + n / 2, std::numeric_limits<double>::quiet_NaN(), 0.95, 1, false, true, 1, false), qPrintable(told.join(QLatin1Char('|'))));
        const qint64 madeAt = timer.elapsed();
        QVERIFY(e.settle());
        qInfo("Forecast Sheet of %d points: made in %d ms, worked out %d ms later", n, int(madeAt), int(timer.elapsed() - madeAt));
        const jp::calc::ets::Counts made = jp::calc::ets::counts();
        const int fs = w.activeSheet();
        {
            // (what the cells that were made read is looked for in the graph in a few blocks, not for each cell: the readers found stay a small multiple of the cells, not as many as cells times readers)
            const jp::office::sheet::Calculation *c = w.session()->calculation();
            const qint64 asked = c->graph().cellQueries() + c->graph().areaQueries();
            QVERIFY2(asked <= 1000, qPrintable(QString::number(asked)));
            QVERIFY2(c->readersFound() <= 100 * n, qPrintable(QString::number(c->readersFound())));
            QVERIFY2(c->graph().queryWork() <= 100 * n, qPrintable(QString::number(c->graph().queryWork())));
        }
        qInfo("sheets: %d", liveSheetCount(w));
        QVERIFY(std::isfinite(w.session()->valueAt(fs, n + 10000, 4).number()));
        QVERIFY(std::isfinite(w.session()->valueAt(fs, n + 1, 3).number()));
        // (one model for the plan's picture, and one for the cells that read the series)
        QVERIFY2(made.fits - start.fits <= 2 && made.fits - start.fits >= 1, qPrintable(QString::number(made.fits - start.fits)));
        timer.restart();
        QVERIFY(w.session()->type(fs, n - 1, 1, QStringLiteral("500")).ok);
        QVERIFY(e.settle());
        qInfo("a value changed on it, worked out again: %d ms", int(timer.elapsed()));
        const jp::calc::ets::Counts changed = jp::calc::ets::counts();
        QCOMPARE(changed.fits - made.fits, 1LL);
        QCOMPARE(changed.prepares - made.prepares, 1LL);
        QCOMPARE(changed.detects - made.detects, 1LL);
    }

    void changingOneCellOfAFilledDownColumnAsksTheGraphTwice()
    {
        const int n = 20000;
        QStringList cells;
        for (int i = 1; i <= n; ++i) cells << QStringLiteral("A%1=%1").arg(i) << QStringLiteral("B%1==A%1*2").arg(i);
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        QVERIFY(e.settle());
        const jp::office::sheet::Calculation *c = e.win.w.session()->calculation();
        const jp::calc::DependencyGraph &g = c->graph();
        const qint64 w0 = g.queryWork(), q0 = g.cellQueries(), a0 = g.areaQueries(), r0 = c->readersFound();
        QVERIFY(e.win.w.session()->type(0, 0, 0, QStringLiteral("5")).ok);
        QVERIFY(e.settle());
        QVERIFY2(g.cellQueries() + g.areaQueries() - q0 - a0 <= 4, qPrintable(QString::number(g.cellQueries() + g.areaQueries() - q0 - a0)));
        QVERIFY2(c->readersFound() - r0 <= 4, qPrintable(QString::number(c->readersFound() - r0)));
        QVERIFY2(g.queryWork() - w0 <= 1000, qPrintable(QString::number(g.queryWork() - w0)));
    }

    // The sheet is made in any locale: a number with a point (0.95, a time of 12.5) and a formula in its English form are what go in, not what a person in Germany or France would type.
    void forecastSheetIsMadeInAnyLocale()
    {
        const QLocale locales[] = {QLocale(QLocale::German, QLocale::Germany), QLocale(QLocale::French, QLocale::France)};
        for (const QLocale &locale : locales) {
            PinnedLocale pin(locale);
            const QByteArray label = locale.name().toLatin1();
            const double pat[] = {3, -1, -3, 1};
            QStringList cells;
            crunch::ForecastInput input;
            for (int i = 1; i <= 24; ++i) {
                const double t = 0.5 * i, y = 10 + i + pat[(i - 1) % 4] + (i > 12 ? (i % 3 - 1) * 0.5 : 0);
                cells << QStringLiteral("A%1=%2").arg(i).arg(t, 0, 'g', 17) << QStringLiteral("B%1=%2").arg(i).arg(y, 0, 'g', 17);
                input.pairs.push_back({t, y});
            }
            EditWindow e;
            QVERIFY2(e.open(sheetOf(cells)), label.constData());
            QVERIFY(e.settle());
            CrunchWindow &w = e.win.w;
            QVERIFY2(w.forecastSheetIn(QStringLiteral("A1:A24"), QStringLiteral("B1:B24"), 15, std::numeric_limits<double>::quiet_NaN(), 0.95, 1, true, true, 1, false), label.constData());
            QVERIFY(e.settle());
            const int fs = w.activeSheet();
            crunch::ForecastSettings settings;
            settings.end = 15;
            const crunch::ForecastResult result = crunch::computeForecast(input, settings, jp::calc::DateSystem::Date1900);
            QVERIFY2(result.problem.isEmpty(), qPrintable(result.problem));
            QCOMPARE(int(result.points.size()), 6);
            // the times and the numbers are what was written (12.5, not 125), and the forecast is the function's
            QCOMPARE(w.session()->valueAt(fs, 24, 0).number(), 12.0);
            QCOMPARE(w.session()->valueAt(fs, 25, 0).number(), 12.5);
            QCOMPARE(w.session()->valueAt(fs, 30, 0).number(), 15.0);
            for (int i = 0; i < 6; ++i) {
                const crunch::ForecastPoint &p = result.points[size_t(i)];
                QVERIFY2(std::fabs(w.session()->valueAt(fs, quint32(25 + i), 2).number() - p.forecast) < 1e-9, label.constData());
                QVERIFY2(std::fabs(w.session()->valueAt(fs, quint32(25 + i), 3).number() - p.lower) < 1e-9, label.constData());
                QVERIFY2(std::fabs(w.session()->valueAt(fs, quint32(25 + i), 4).number() - p.upper) < 1e-9, label.constData());
            }
            // the formulas the file has are the English ones, with the confidence as 0.95
            QSignalSpy saved(&w, &CrunchWindow::saved);
            QVERIFY(w.save());
            QVERIFY2(savedOk(saved), "the save failed");
            QCOMPARE(savedFormula(e.path, QStringLiteral("C26")), QStringLiteral("_xlfn.FORECAST.ETS(A26,$B$2:$B$25,$A$2:$A$25,1,1,1)"));
            QCOMPARE(savedFormula(e.path, QStringLiteral("D26")), QStringLiteral("C26-_xlfn.FORECAST.ETS.CONFINT(A26,$B$2:$B$25,$A$2:$A$25,0.95,1,1,1)"));
            QCOMPARE(savedCell(e.path, QStringLiteral("A26")), QStringLiteral("n:12.5"));
        }
    }

    // Column A is wide enough for the times as the source's date format shows them (a long format is not a column of #####).
    void forecastSheetTimeColumnFitsALongDateFormat()
    {
        const double pat[] = {3, -1, -3, 1};
        QString rows;
        for (int i = 1; i <= 24; ++i) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" s=\"1\"><v>%2</v></c><c r=\"B%1\"><v>%3</v></c></row>").arg(i).arg(45292 + i * 7).arg(10 + i + pat[(i - 1) % 4]);
        const QByteArray book = buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(),
                                          stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"dddd, mmmm d, yyyy\"/>"), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\" applyNumberFormat=\"1\"/>")));
        EditWindow e;
        QVERIFY(e.open(book));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.forecastSheetIn(QStringLiteral("A1:A24"), QStringLiteral("B1:B24"), 45292 + 24 * 7 + 28, std::numeric_limits<double>::quiet_NaN(), 0.95, 1, false, true, 1, false));
        QVERIFY(e.settle());
        const int fs = w.activeSheet();
        // the longest text the format makes of the times the sheet shows
        const jp::calc::NumberFormat format(QStringLiteral("dddd, mmmm d, yyyy"));
        int longest = 0;
        for (int i = 1; i <= 28; ++i) longest = std::max(longest, int(format.format(45292 + i * 7).text.size()));
        QVERIFY(longest > 20);
        const auto runs = w.session()->calculation()->columnRuns(fs, 0, 0);
        QVERIFY(!runs.empty());
        QVERIFY2(runs.front().width >= longest, qPrintable(QStringLiteral("column A is %1 wide for %2 characters").arg(runs.front().width).arg(longest)));
        QVERIFY(shown(e, QStringLiteral("A26")).size() > 20 && !shown(e, QStringLiteral("A26")).contains(QLatin1Char('#')));
    }

    void forecastSheetSaysWhatIsWrongWithTheData()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=5", "A2=2", "B2=6", "A3=3.3", "B3=7", "A4=4", "B4=9", "D1=1", "E1=5", "D2=2", "E2=six", "D3=3", "E3=7", "D4=4", "E4=9", "G1=1", "H1=2", "G2=2", "H2=3"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QStringList told;
        w.setNotifier([&](const QString &, const QString &text) { told << text; });
        const double none = std::numeric_limits<double>::quiet_NaN();
        const int steps = w.undoStack()->count();
        QVERIFY(!w.forecastSheetIn(QStringLiteral("A1:A4"), QStringLiteral("B1:B4"), 9, none, 0.95, 1, false, true, 1, false));
        QVERIFY(told.last().contains(QStringLiteral("not evenly spaced")));
        QVERIFY(!w.forecastSheetIn(QStringLiteral("G1:G2"), QStringLiteral("H1:H2"), 9, none, 0.95, 1, false, true, 1, false));
        QVERIFY(told.last().contains(QStringLiteral("too few points")));
        QVERIFY(!w.forecastSheetIn(QStringLiteral("D1:D4"), QStringLiteral("E1:E4"), 9, none, 0.95, 1, false, true, 1, false));
        QVERIFY(told.last().contains(QStringLiteral("E2")) && told.last().contains(QStringLiteral("numbers")));
        QVERIFY(!w.forecastSheetIn(QStringLiteral("D1:D4"), QStringLiteral("E1:E3"), 9, none, 0.95, 1, false, true, 1, false));
        QVERIFY(told.last().contains(QStringLiteral("same number of cells")));
        QVERIFY(!w.forecastSheetIn(QStringLiteral("nonsense"), QStringLiteral("E1:E3"), 9, none, 0.95, 1, false, true, 1, false));
        QCOMPARE(told.size(), 5);
        QCOMPARE(w.undoStack()->count(), steps);   // (nothing was made)
        // the dialog says the same in its own words, and keeps Create from working
        const jp::ForecastSourceReader reader = [&](const QString &t, const QString &v, long long maxPoints) {
            jp::ForecastSourceInfo info;
            if (!w.forecastSource(t, v, &info.input.pairs, &info.dates, &info.problem, maxPoints, &info.sampled)) info.input.pairs.clear();
            return info;
        };
        jp::ForecastDialog dialog(reader, jp::calc::DateSystem::Date1900, QStringLiteral("A1:A4"), QStringLiteral("B1:B4"));
        QVERIFY(!dialog.ok());
        QVERIFY(dialog.problemText().contains(QStringLiteral("not evenly spaced")));
        dialog.findChild<QLineEdit *>(QStringLiteral("forecastTimeline"))->setText(QStringLiteral("D1:D4"));
        dialog.findChild<QLineEdit *>(QStringLiteral("forecastValues"))->setText(QStringLiteral("E1:E4"));
        dialog.refreshNow();
        QVERIFY(!dialog.ok() && dialog.problemText().contains(QStringLiteral("E2")));
        dialog.findChild<QLineEdit *>(QStringLiteral("forecastValues"))->setText(QStringLiteral("B1:B4"));
        dialog.refreshNow();
        QVERIFY2(dialog.ok(), qPrintable(dialog.problemText()));
        QVERIFY(dialog.problemText().isEmpty());
        QVERIFY(!dialog.result().points.empty());
    }

    void forecastChoicesAggregateFillAndSeason()
    {
        namespace ets = jp::calc::ets;
        const double nan = std::numeric_limits<double>::quiet_NaN();
        auto prepared = [&](std::vector<std::pair<double, double>> p, ets::Aggregation a, bool interpolate) {
            ets::Prepared out;
            const ets::Problem problem = ets::prepare(p, a, interpolate, jp::calc::DateSystem::Date1900, &out);
            return std::make_pair(problem, out);
        };
        // two values at the time 2, and a gap at 4
        const std::vector<std::pair<double, double>> pairs = {{1, 10}, {2, 4}, {2, 8}, {3, 12}, {5, 20}, {6, 22}};
        const std::pair<ets::Aggregation, double> choices[] = {{ets::Aggregation::Average, 6}, {ets::Aggregation::Count, 2}, {ets::Aggregation::CountA, 2}, {ets::Aggregation::Max, 8}, {ets::Aggregation::Median, 6}, {ets::Aggregation::Min, 4}, {ets::Aggregation::Sum, 12}};
        for (const auto &c : choices) {
            const auto p = prepared(pairs, c.first, true);
            QCOMPARE(int(p.first), int(ets::Problem::None));
            QCOMPARE(p.second.timeline.n, 6LL);
            QCOMPARE(p.second.values[1], c.second);
        }
        QCOMPARE(prepared(pairs, ets::Aggregation::Average, true).second.values[3], 16.0);   // a line between 12 and 20
        QCOMPARE(prepared(pairs, ets::Aggregation::Average, false).second.values[3], 0.0);
        QCOMPARE(prepared(pairs, ets::Aggregation::Average, true).second.filled, 1LL);
        // a blank value is a missing point, an uneven or too short timeline is told
        QCOMPARE(prepared({{1, 1}, {2, nan}, {3, 3}, {4, 4}}, ets::Aggregation::Average, true).second.values[1], 2.0);
        QCOMPARE(int(prepared({{1, 1}, {2, 2}, {4.5, 3}}, ets::Aggregation::Average, true).first), int(ets::Problem::Uneven));
        QCOMPARE(int(prepared({{1, 1}, {2, 2}}, ets::Aggregation::Average, true).first), int(ets::Problem::TooFew));
        QCOMPARE(int(prepared({{1, 1}, {1, 2}, {2, 3}}, ets::Aggregation::Sum, true).first), int(ets::Problem::TooFew));
        QCOMPARE(int(prepared({{1, 1}, {2, 2}, {3e9, 3}}, ets::Aggregation::Sum, true).first), int(ets::Problem::TooMany));
        // dates a month apart are a timeline: 2024-01-31 is day 45322 (the end of each month follows)
        double jan, feb, mar, apr;
        QVERIFY(jp::calc::dateToSerial(2024, 1, 31, jp::calc::DateSystem::Date1900, &jan) && jp::calc::dateToSerial(2024, 2, 29, jp::calc::DateSystem::Date1900, &feb));
        QVERIFY(jp::calc::dateToSerial(2024, 3, 31, jp::calc::DateSystem::Date1900, &mar) && jp::calc::dateToSerial(2024, 4, 30, jp::calc::DateSystem::Date1900, &apr));
        const auto months = prepared({{jan, 1}, {feb, 2}, {mar, 3}, {apr, 4}}, ets::Aggregation::Sum, true);
        QCOMPARE(int(months.first), int(ets::Problem::None));
        QVERIFY(months.second.timeline.months);
        QCOMPARE(months.second.timeline.timeAt(4), [&] { double d; jp::calc::dateToSerial(2024, 5, 31, jp::calc::DateSystem::Date1900, &d); return d; }());
        // dates on one day of each month are a month apart even when two gaps are the same 31 days (Jul 1, Aug 1, Sep 1); gaps of 31 days that are not so are days
        double jul, aug, sep, oct;
        QVERIFY(jp::calc::dateToSerial(2024, 7, 1, jp::calc::DateSystem::Date1900, &jul) && jp::calc::dateToSerial(2024, 8, 1, jp::calc::DateSystem::Date1900, &aug));
        QVERIFY(jp::calc::dateToSerial(2024, 9, 1, jp::calc::DateSystem::Date1900, &sep) && jp::calc::dateToSerial(2024, 10, 1, jp::calc::DateSystem::Date1900, &oct));
        QCOMPARE(aug - jul, 31.0);
        QCOMPARE(sep - aug, 31.0);
        const auto firsts = prepared({{jul, 1}, {aug, 2}, {sep, 3}}, ets::Aggregation::Sum, true);
        QCOMPARE(int(firsts.first), int(ets::Problem::None));
        QVERIFY(firsts.second.timeline.months);
        QCOMPARE(firsts.second.timeline.step, 1.0);
        QCOMPARE(firsts.second.timeline.timeAt(3), oct);
        const auto thirtyOnes = prepared({{aug, 1}, {aug + 31, 2}, {aug + 62, 3}}, ets::Aggregation::Sum, true);   // (Aug 1, Sep 1, and Oct 2: not one day of a month)
        QCOMPARE(int(thirtyOnes.first), int(ets::Problem::None));
        QVERIFY(!thirtyOnes.second.timeline.months);
        QCOMPARE(thirtyOnes.second.timeline.step, 31.0);
        // the season: found at 4, none for a straight line, and each choice of the settings is used
        crunch::ForecastInput seasonal, straight;
        for (int i = 1; i <= 24; ++i) {
            const double pat[] = {3, -1, -3, 1};
            const double noise = ((i * 37 + 11) % 17 - 8) * 0.01;
            seasonal.pairs.push_back({double(i), 10 + i + pat[(i - 1) % 4] + noise});
            straight.pairs.push_back({double(i), 2.0 * i});
        }
        crunch::ForecastSettings s;
        s.end = 30;
        QCOMPARE(crunch::computeForecast(seasonal, s, jp::calc::DateSystem::Date1900).season, 4);
        QCOMPARE(crunch::computeForecast(straight, s, jp::calc::DateSystem::Date1900).season, 0);
        s.seasonality = 0;
        const crunch::ForecastResult none = crunch::computeForecast(seasonal, s, jp::calc::DateSystem::Date1900);
        QCOMPARE(none.season, 0);
        s.seasonality = 4;
        const crunch::ForecastResult four = crunch::computeForecast(seasonal, s, jp::calc::DateSystem::Date1900);
        QCOMPARE(four.season, 4);
        s.seasonality = 1;
        const crunch::ForecastResult found = crunch::computeForecast(seasonal, s, jp::calc::DateSystem::Date1900);
        QCOMPARE(found.points[0].forecast, four.points[0].forecast);
        QVERIFY(std::fabs(none.points[0].forecast - four.points[0].forecast) > 0.1);   // (the season changes the forecast)
        s.seasonality = 13;
        QVERIFY(!crunch::computeForecast(seasonal, s, jp::calc::DateSystem::Date1900).problem.isEmpty());   // (two seasons of 13 are not there)
        // the confidence interval widens with the level and with the distance
        s.seasonality = 1;
        const crunch::ForecastResult wide = [&] { crunch::ForecastSettings t = s; t.confidence = 0.99; return crunch::computeForecast(seasonal, t, jp::calc::DateSystem::Date1900); }();
        QVERIFY(wide.points[0].upper - wide.points[0].lower > found.points[0].upper - found.points[0].lower);
        QVERIFY(found.points[5].upper - found.points[5].lower >= found.points[0].upper - found.points[0].lower);
        // a forecast that starts before the last point is made from the points before it
        s.start = 20;
        s.end = 26;
        const crunch::ForecastResult early = crunch::computeForecast(seasonal, s, jp::calc::DateSystem::Date1900);
        QVERIFY2(early.problem.isEmpty(), qPrintable(early.problem));
        QCOMPARE(int(early.used.size()), 19);
        QCOMPARE(early.points.front().time, 20.0);
        QCOMPARE(early.points.back().time, 26.0);
        QCOMPARE(int(early.later.size()), 5);
    }

    void forecastIsBounded()
    {
        // more than 1,000,000 points are refused before any is read; the forecast is at most 10,000 rows; and the work of a long series is bounded
        int read = 0;
        const crunch::ForecastCellReader reader = [&](quint32, quint32) {
            ++read;
            return jp::calc::Value::number(1);
        };
        crunch::ForecastInput in;
        QString problem;
        const jp::office::sheet::CellRange huge = rangeOf(QStringLiteral("A1:A1000001")), other = rangeOf(QStringLiteral("B1:B1000001")), small = rangeOf(QStringLiteral("A1:A5"));
        QVERIFY(!crunch::readForecastInput(huge, reader, other, reader, &in, &problem));
        QVERIFY(problem.contains(QStringLiteral("1,000,000")));
        QCOMPARE(read, 0);
        QVERIFY(!crunch::readForecastInput(small, reader, rangeOf(QStringLiteral("B1:B6")), reader, &in, &problem));
        QCOMPARE(read, 0);
        crunch::ForecastInput series;
        const int n = 10000;
        for (int i = 1; i <= n; ++i) series.pairs.push_back({double(i), 100 + i * 0.5 + ((i % 12) - 6) * 3.0 + (i * 7919 % 13) * 0.2});
        crunch::ForecastSettings s;
        // the work is a fixed number of passes over the series, however many rows the forecast has or how often it is asked for: one grid and one search for a season for each ask, and one fit
        const jp::calc::ets::Counts before = jp::calc::ets::counts();
        s.end = n + 10001;
        QVERIFY(crunch::computeForecast(series, s, jp::calc::DateSystem::Date1900).problem.contains(QStringLiteral("10,000")));
        // (a forecast that reaches past 100,000 steps says that limit, not the 10,000 rows)
        s.end = n + 100002;
        const QString far = crunch::computeForecast(series, s, jp::calc::DateSystem::Date1900).problem;
        QVERIFY2(far.contains(QStringLiteral("100,000")) && !far.contains(QStringLiteral("10,000 ")), qPrintable(far));
        s.end = n + 10000;
        const crunch::ForecastResult r = crunch::computeForecast(series, s, jp::calc::DateSystem::Date1900);
        const jp::calc::ets::Counts after = jp::calc::ets::counts();
        QVERIFY2(r.problem.isEmpty(), qPrintable(r.problem));
        QCOMPARE(int(r.points.size()), 10000);
        QCOMPARE(r.model->n, 10000LL);
        QCOMPARE(r.season, 12);
        QCOMPARE(after.prepares - before.prepares, 3LL);
        QCOMPARE(after.detects - before.detects, 3LL);
        QCOMPARE(after.fits - before.fits, 1LL);   // (the same series and season are fitted once for the three)
    }

    // The picture of the dialog is made from the last 50,000 points of a longer series, and says so: what is typed in the dialog's fields is not slowed by a million points (they are not read, nor
    // fitted, for it); the sheet itself is made from all of them.
    void forecastDialogPreviewIsBoundedForALongSeries()
    {
        const long long total = 1000000;
        long long asked = -1, served = 0;
        int reads = 0;
        const jp::ForecastSourceReader reader = [&](const QString &, const QString &, long long maxPoints) {
            ++reads;
            asked = maxPoints;
            jp::ForecastSourceInfo info;
            const long long first = maxPoints > 0 && total > maxPoints ? total - maxPoints : 0;
            for (long long i = first; i < total; ++i) info.input.pairs.push_back({double(i + 1), 100 + 0.001 * double(i) + ((i % 12) - 6) * 3.0});
            info.sampled = first > 0;
            served = qint64(info.input.pairs.size());
            return info;
        };
        jp::ForecastDialog dialog(reader, jp::calc::DateSystem::Date1900, QStringLiteral("A1:A1000000"), QStringLiteral("B1:B1000000"));
        QCOMPARE(asked, crunch::kPreviewPoints);
        QVERIFY(served <= 50000);
        QVERIFY2(dialog.ok(), qPrintable(dialog.problemText()));
        QCOMPARE(dialog.result().prepared.timeline.n, 50000LL);
        QVERIFY(dialog.noteText().contains(QStringLiteral("50,000")));
        // typing in the other fields works the picture out again from the points it has, and does not read the cells again
        const int readsBefore = reads;
        dialog.findChild<QLineEdit *>(QStringLiteral("forecastEnd"))->setText(QStringLiteral("1000100"));
        dialog.refreshNow();
        QCOMPARE(reads, readsBefore);
        QVERIFY(dialog.ok());
        QCOMPARE(dialog.result().prepared.timeline.n, 50000LL);
        // the rows from the sheet's own read are the whole series
        std::vector<std::pair<double, double>> pairs;
        bool dates = false, sampled = true;
        QString problem;
        EditWindow e;
        QStringList cells;
        for (int i = 1; i <= 300; ++i) cells << QStringLiteral("A%1=%1").arg(i) << QStringLiteral("B%1=%2").arg(i).arg(i * 2);
        QVERIFY(e.open(sheetOf(cells)));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.forecastSource(QStringLiteral("A1:A300"), QStringLiteral("B1:B300"), &pairs, &dates, &problem, 0, &sampled));
        QCOMPARE(int(pairs.size()), 300);
        QVERIFY(!sampled);
        QVERIFY(e.win.w.forecastSource(QStringLiteral("A1:A300"), QStringLiteral("B1:B300"), &pairs, &dates, &problem, 100, &sampled));
        QCOMPARE(int(pairs.size()), 100);
        QVERIFY(sampled);
        QCOMPARE(pairs.front().first, 201.0);
        QCOMPARE(pairs.back().second, 600.0);
    }

    void forecastDialogStartsFromTheSelectionAndShowsAPreview()
    {
        EditWindow e;
        QStringList cells = {"A1=Time", "B1=Sales"};
        for (const QString &c : seasonalCells()) {
            // (a row of names above the data: the data starts at row 2)
            int row = c.mid(1, c.indexOf('=') - 1).toInt() + 1;
            cells << QStringLiteral("%1%2%3").arg(c[0]).arg(row).arg(c.mid(c.indexOf('=')));
        }
        QVERIFY(e.open(sheetOf(cells)));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 0, 24, 1);
        QString timeline, values, end;
        bool ok = false, grabbed = false;
        QTimer poll;
        QObject::connect(&poll, &QTimer::timeout, [&] {
            auto *d = qobject_cast<jp::ForecastDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            poll.stop();
            timeline = d->timelineText();
            values = d->valuesText();
            end = d->findChild<QLineEdit *>(QStringLiteral("forecastEnd"))->text();
            ok = d->ok() && !d->result().points.empty();
            grabbed = true;
            d->reject();
        });
        poll.start(20);
        e.win.w.act(QStringLiteral("data.forecastSheet"))->trigger();
        poll.stop();
        QVERIFY(grabbed);
        QCOMPARE(timeline, QStringLiteral("A2:A25"));
        QCOMPARE(values, QStringLiteral("B2:B25"));
        QCOMPARE(end, QStringLiteral("36"));   // (half as many steps again as the 24 points)
        QVERIFY(ok);
    }

    void consolidateRefusesWhatItCannotRead()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        QStringList told;
        e.win.w.setNotifier([&](const QString &, const QString &text) { told << text; });
        QVERIFY(!e.win.w.consolidateIn({"nonsense"}, 9, false, false, false, QStringLiteral("D1")));
        QVERIFY(!e.win.w.consolidateIn({"A1:A2"}, 9, false, false, false, QStringLiteral("not a cell")));
        QVERIFY(!e.win.w.consolidateIn({}, 9, false, false, false, QStringLiteral("D1")));
        QVERIFY(!e.win.w.consolidateIn({"A1:A2"}, 99, false, false, false, QStringLiteral("D1")));
        QCOMPARE(told.size(), 4);
        QVERIFY(e.win.w.act(QStringLiteral("data.consolidate"))->isEnabled());
    }

    void circleInvalidDataMarksCellsThatBreakTheirRuleAndClearsThem()
    {
        using jp::office::sheet::Validation;
        EditWindow e;
        QVERIFY(e.open(sheetOf({"B1=5", "B2=50", "B3=7", "B4=text", "D1=100"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        Validation rule;
        rule.type = Validation::Type::Whole;
        rule.compare = Validation::Compare::Between;
        rule.formula1 = "1";
        rule.formula2 = "9";
        QVERIFY(w.setValidationOn(rangeOf(QStringLiteral("B1:B5")), rule));
        QVERIFY(w.circledCells().empty());
        QVERIFY(!w.act(QStringLiteral("data.clearCircles"))->isEnabled());   // (nothing to clear)
        QStringList told;
        w.setNotifier([&](const QString &, const QString &text) { told << text; });
        // (50 and the text are not allowed; 5 and 7 are; the empty cell and the cell with no rule are not looked at)
        QCOMPARE(w.circleInvalidData(), 2);
        QCOMPARE(w.circledCells().size(), size_t(2));
        QCOMPARE(int(w.circledCells()[0].row1), 1);
        QCOMPARE(int(w.circledCells()[1].row1), 3);
        QCOMPARE(w.view()->invalidCircles().size(), size_t(2));
        QVERIFY(w.act(QStringLiteral("data.clearCircles"))->isEnabled());
        // put right, the oval goes when the sheet is worked out
        e.select(QStringLiteral("B2"));
        e.enter(QStringLiteral("8"));
        QVERIFY(e.settle());
        QTRY_COMPARE(w.view()->invalidCircles().size(), size_t(1));
        w.act(QStringLiteral("data.clearCircles"))->trigger();
        QVERIFY(w.circledCells().empty());
        QVERIFY(w.view()->invalidCircles().empty());
        // nothing wrong: the person is told
        e.select(QStringLiteral("B4"));
        e.enter(QStringLiteral("3"));
        QVERIFY(e.settle());
        w.act(QStringLiteral("data.circleInvalid"))->trigger();
        QCOMPARE(told.size(), 1);
    }

    void errorCheckingMarksCellsWithAGreenCornerAndKeepsIgnoredErrorsInTheFile()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "B1==A1/0", "B2==A1/0", "C1=2"}}, {{0, QStringLiteral("<ignoredErrors><ignoredError sqref=\"B2\" evalError=\"1\"/></ignoredErrors>")}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        // the file's own ignored error was read: B2 is left out, B1 is marked
        QCOMPARE(w.session()->ignoredErrorsOf(0).size(), 1);
        QCOMPARE(w.formulaProblems().size(), size_t(1));
        QTRY_COMPARE(w.view()->errorCorners().size(), size_t(1));
        w.ignoreFormulaProblem(0, 1);
        QVERIFY(w.formulaProblems().empty());
        QVERIFY(w.view()->errorCorners().empty());
        QCOMPARE(w.session()->ignoredErrorsOf(0).size(), 2);
        // kept in the file, with the one that was there
        QVERIFY(e.settle());
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(saved.wait(20000) && saved.last().value(1).toBool());
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const auto back = second.w.session()->ignoredErrorsOf(0);
        QCOMPARE(back.size(), 2);
        QCOMPARE(int(back[0].kinds), int(jp::office::sheet::IgnoredError::EvalError));
        QVERIFY(back[0].ranges[0] == rangeOf(QStringLiteral("B2")));
        QVERIFY(back[1].ranges[0] == rangeOf(QStringLiteral("B1")));
        QVERIFY(second.w.formulaProblems().empty());
        // Undo brings the marker back
        w.undoStack()->undo();
        QCOMPARE(w.formulaProblems().size(), size_t(1));
    }

    void checkAccessibilityListsWhatIsHardToUseAndGoesToIt()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><v>3</v></c><c r=\"B2\"><v>4</v></c></row>");
        const QString table = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"Plain\" displayName=\"Plain\" ref=\"A1:B2\" headerRowCount=\"0\">"
                                             "<tableColumns count=\"2\"><tableColumn id=\"1\" name=\"Column1\"/><tableColumn id=\"2\" name=\"Column2\"/></tableColumns></table>");
        const QByteArray merges = "<mergeCells count=\"1\"><mergeCell ref=\"D4:E4\"/></mergeCells>";
        EditWindow e;
        QVERIFY(e.open(withTables(buildXlsx({worksheet(rows, QString::fromLatin1(merges)), worksheet(QString())}, QByteArray(), QString(), QString(), QByteArray(), {QStringLiteral("Sheet1"), QStringLiteral("Budget")}), {table})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QTRY_VERIFY(w.session() != nullptr);
        auto kinds = [](const std::vector<crunch::AccessIssue> &issues) {
            QStringList k;
            for (const auto &i : issues) k << QString::number(int(i.kind));
            return k.join(QLatin1Char(' '));
        };
        // a sheet with its default name, merged cells, and a table with no header row (Sheet1 only: "Budget" was named)
        auto issues = w.accessibilityIssues();
        QCOMPARE(kinds(issues), QStringLiteral("2 1 3"));   // DefaultSheetName, MergedCells, TableWithoutHeader
        QCOMPARE(int(issues[1].row), 3);
        QCOMPARE(int(issues[1].col), 3);
        // a chart with no alt text is an error
        e.select(QStringLiteral("A1"));
        selectCells(e, QStringLiteral("A1:B2"));
        QVERIFY(w.insertChart(ChartKind::Column));
        QVERIFY(e.settle());
        issues = w.accessibilityIssues();
        QCOMPARE(kinds(issues), QStringLiteral("2 0 1 3"));
        QVERIFY(issues[1].level == crunch::AccessIssue::Error);
        // the window lists them and goes to the one clicked
        w.act(QStringLiteral("review.accessibility"))->trigger();
        QVERIFY(w.accessibilityPane());
        QCOMPARE(w.accessibilityPane()->issueCount(), 4);
        w.accessibilityPane()->list()->setCurrentRow(2);
        Q_EMIT w.accessibilityPane()->list()->itemActivated(w.accessibilityPane()->list()->item(2));
        QCOMPARE(e.selected(), QStringLiteral("D4"));
    }

    // Check Accessibility finds text that is hard to read against its fill (pale gray on white), once for each look, and leaves readable text, an empty cell, and a patterned fill alone.
    void checkAccessibilityFindsTextThatIsHardToRead()
    {
        const QByteArray styles = stylesXml(
            QString(),
            QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/><xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\"/>"
                           "<xf numFmtId=\"0\" fontId=\"2\" fillId=\"2\" borderId=\"0\"/><xf numFmtId=\"0\" fontId=\"2\" fillId=\"3\" borderId=\"0\"/>"),
            QStringLiteral("<fonts count=\"3\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font><font><sz val=\"11\"/><color rgb=\"FFD0D0D0\"/><name val=\"Calibri\"/></font>"
                           "<font><sz val=\"11\"/><color rgb=\"FFFFFFFF\"/><name val=\"Calibri\"/></font></fonts>"
                           "<fills count=\"4\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
                           "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF13773A\"/></patternFill></fill>"
                           "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFF0F0F0\"/></patternFill></fill></fills>"
                           "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"));
        // A1 pale on white, B1 white on dark green (readable), C1 empty with the pale look, A2 pale again (the same look: one finding), B2 white on pale gray (hard to read)
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"1\"><v>1</v></c><c r=\"B1\" s=\"2\"><v>2</v></c><c r=\"C1\" s=\"1\"/></row>"
                                            "<row r=\"2\"><c r=\"A2\" s=\"1\"><v>3</v></c><c r=\"B2\" s=\"3\"><v>4</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles, {QStringLiteral("Budget")})));
        QVERIFY(e.settle());
        const auto issues = crunch::checkAccessibility(*e.win.w.session(), nullptr);
        QStringList where;
        for (const auto &i : issues)
            if (i.kind == crunch::AccessIssue::LowContrast) where << QStringLiteral("%1,%2").arg(i.row).arg(i.col);
        QCOMPARE(where, (QStringList{QStringLiteral("0,0"), QStringLiteral("1,1")}));
    }

    // Thesaurus: the synonyms of the word in the active cell come from the word list, and the one chosen goes in the cell in place of the word (a capital stays a capital).
    void theThesaurusFindsSynonymsAndReplacesTheWordInTheCell()
    {
        const QStringList found = jp::thesaurusLookup(QStringLiteral("happy"));
        QVERIFY(!found.isEmpty());
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        QVERIFY(w.act(QStringLiteral("review.thesaurus")));
        e.select(QStringLiteral("E1"));
        e.enter(QStringLiteral("Happy days"));
        QVERIFY(e.settle());
        QCOMPARE(w.thesaurusWordAt(0, 0, 4), QStringLiteral("Happy"));
        QVERIFY(w.replaceWordInCell(0, 0, 4, QStringLiteral("Happy"), QStringLiteral("glad")));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("s:Glad days"));
    }

private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchTabs2Tests)
#include "tst_crunch_tabs2.moc"
