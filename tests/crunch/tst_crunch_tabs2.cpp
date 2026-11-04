// Tests of tools on the Data, Formulas, Review, and View tabs: Workbook Statistics, Goal Seek, Subtotal, and Advanced Filter. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/ribbon.h"
#include "core/advancedfilter.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/goalseek.h"
#include "core/outline.h"
#include "sheet/calculation.h"
#include "app/functionargumentsdialog.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"

#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QToolButton>
#include <QTest>
#include <QUndoStack>
#include "../localepin.h"

#include <memory>

using namespace jp;
using namespace crunchtest;
using crunch::EditSession;

namespace {
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
        // nothing used yet
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

private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchTabs2Tests)
#include "tst_crunch_tabs2.moc"
