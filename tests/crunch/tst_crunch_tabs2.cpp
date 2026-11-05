// Tests of tools on the Data, Formulas, Review, and View tabs: Workbook Statistics, Goal Seek, Subtotal, and Advanced Filter. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/proofingdialogs.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "core/advancedfilter.h"
#include "core/appinfo.h"
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

#include <QAction>
#include <QListWidget>
#include <QMenu>
#include <QTableWidget>
#include <QSettings>
#include <QToolButton>
#include <QSignalSpy>
#include <QTest>
#include <QUndoStack>
#include "../localepin.h"

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
