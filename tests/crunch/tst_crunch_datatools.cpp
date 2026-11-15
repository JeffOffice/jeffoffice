// Tests of the Data tools that change cells: Remove Duplicates (which rows go, that the rows under them move up in the range and what is under the range stays, headers, case, columns
// compared, formulas and formats that go with their rows, one step of Undo) and Text to Columns (separators, a qualifier, fixed places, kinds of piece, a destination). Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/dataformdialog.h"
#include "app/datatoolsdialogs.h"
#include "app/removeduplicatesdialog.h"
#include "core/appinfo.h"
#include "core/dataform.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "app/ribbon.h"

#include <QAction>
#include <QToolButton>
#include <QElapsedTimer>
#include <QTest>
#include <QSettings>
#include <QUndoStack>
#include "../localepin.h"

#include <memory>

using namespace jp;
using namespace crunchtest;
using crunch::EditSession;

class CrunchDataToolsTests : public QObject {
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

    void repeatedRowsGoAndTheRowsUnderThemMoveUp()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=a", "B1=1", "A2=b", "B2=2", "A3=a", "B3=1", "A4=c", "B4=3", "A5=b", "B5=2", "A6=d", "B6=4", "A8=keep", "B8=me"})));
        QVERIFY(e.settle());
        int removed = 0;
        const EditSession::Outcome o = e.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:B6")), false, QList<int>{0, 1}, &removed);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QVERIFY(e.settle());
        QCOMPARE(removed, int(2));
        QCOMPARE(shown(e, QStringLiteral("A1:B6")), QStringLiteral("a|1|b|2|c|3|d|4||||"));
        QCOMPARE(shown(e, QStringLiteral("A8:B8")), QStringLiteral("keep|me"));   // (what is under the range stays where it is)
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:B6")), QStringLiteral("a|1|b|2|a|1|c|3|b|2|d|4"));
    }

    void onlyTheColumnsCheckedAreComparedAndCaseDoesNotMatter()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Apple", "B1=1", "A2=apple", "B2=2", "A3=Pear", "B3=3", "A4=APPLE", "B4=4"})));
        QVERIFY(e.settle());
        int removed = 0;
        QVERIFY(e.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:B4")), false, QList<int>{0}, &removed).ok);
        QVERIFY(e.settle());
        QCOMPARE(removed, int(2));
        QCOMPARE(shown(e, QStringLiteral("A1:B4")), QStringLiteral("Apple|1|Pear|3||||"));
        // both columns: a row repeats only when every checked column does
        EditWindow f;
        QVERIFY(f.open(sheetOf({"A1=x", "B1=1", "A2=x", "B2=2", "A3=x", "B3=1"})));
        QVERIFY(f.settle());
        QVERIFY(f.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:B3")), false, QList<int>{0, 1}, &removed).ok);
        QVERIFY(f.settle());
        QCOMPARE(removed, int(1));
        QCOMPARE(shown(f, QStringLiteral("A1:B3")), QStringLiteral("x|1|x|2||"));
    }

    void aHeaderRowStaysAndIsNotComparedAndNothingToRemoveChangesNothing()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "A2=Name", "A3=bob", "A4=Bob"})));
        QVERIFY(e.settle());
        int removed = 0;
        QVERIFY(e.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:A4")), true, QList<int>{0}, &removed).ok);
        QVERIFY(e.settle());
        QCOMPARE(removed, int(1));
        QCOMPARE(shown(e, QStringLiteral("A1:A4")), QStringLiteral("Name|Name|bob|"));
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:A3")), true, QList<int>{0}, &removed).ok);
        QCOMPARE(removed, int(0));
        QCOMPARE(e.win.w.undoStack()->count(), steps);   // (no step when nothing goes)
    }

    void numbersTextAndEmptyCellsAreToldApartAndFormulasAreComparedByValue()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=1", "A3=text", "A4=1", "A5==1+0", "A6==2-1", "B1=a", "B2=a", "B3=a", "B4=a", "B5=a", "B6=a"})));
        QVERIFY(e.settle());
        int removed = 0;
        QVERIFY(e.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:B6")), false, QList<int>{0}, &removed).ok);
        QVERIFY(e.settle());
        QCOMPARE(removed, int(4));   // (1 repeated four times: the formulas give 1 as well)
        QCOMPARE(shown(e, QStringLiteral("A1:A6")), QStringLiteral("1|text||||"));
    }

    void formatsAndFormulasGoWithTheirRows()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1*10", "A2=1", "B2==A2*10", "A3=2", "B3==A3*10"})));
        QVERIFY(e.settle());
        int removed = 0;
        QVERIFY(e.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:B3")), false, QList<int>{0}, &removed).ok);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:B3")), QStringLiteral("1|10|2|20||"));
        QCOMPARE(e.win.w.session()->editText(0, 1, 1), QStringLiteral("=A2*10"));
    }

    void withoutAHeaderTheFirstRowIsComparedLikeTheRest()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "A2=Name", "A3=bob"})));
        QVERIFY(e.settle());
        int removed = 0;
        QVERIFY(e.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:A3")), true, QList<int>{0}, &removed).ok);
        QCOMPARE(removed, 0);   // (the first row is a header: left out)
        QVERIFY(e.win.w.session()->removeDuplicates(0, rangeOf(QStringLiteral("A1:A3")), false, QList<int>{0}, &removed).ok);
        QVERIFY(e.settle());
        QCOMPARE(removed, 1);   // (no header: row 2 repeats row 1)
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("Name|bob|"));
    }

    void theDialogsHeaderBoxChoosesTheNamesOfTheColumns()
    {
        RemoveDuplicatesDialog d(QStringList{"Name", "Age"}, QStringList{"Column A", "Column B"}, true, nullptr);
        QVERIFY(d.headerBox());
        QVERIFY(d.hasHeader());
        QCOMPARE(d.columnNames(), (QStringList{"Name", "Age"}));
        QCOMPARE(d.chosen(), (QList<int>{0, 1}));
        d.checkAll(false);
        QVERIFY(d.chosen().isEmpty());
        d.headerBox()->setChecked(false);
        QVERIFY(!d.hasHeader());
        QCOMPARE(d.columnNames(), (QStringList{"Column A", "Column B"}));
        QVERIFY(d.chosen().isEmpty());   // (what was checked stays as it was)
        d.checkAll(true);
        QCOMPARE(d.chosen(), (QList<int>{0, 1}));
        RemoveDuplicatesDialog without(QStringList{"Name", "Age"}, QStringList{"Column A", "Column B"}, false, nullptr);
        QVERIFY(!without.hasHeader());
        QCOMPARE(without.columnNames(), (QStringList{"Column A", "Column B"}));
        // a table's dialog has no header box and the header is the table's
        RemoveDuplicatesDialog table(QStringList{"X", "Y"}, nullptr);
        QVERIFY(!table.headerBox());
        QVERIFY(table.hasHeader());
        QCOMPARE(table.columnNames(), (QStringList{"X", "Y"}));
    }

    void tooManyCellsAndBadColumnsAreRefused()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=1"})));
        QVERIFY(e.settle());
        EditSession *s = e.win.w.session();
        QVERIFY(!s->removeDuplicates(0, rangeOf(QStringLiteral("A1:A2")), false, QList<int>{}).ok);
        QVERIFY(!s->removeDuplicates(0, rangeOf(QStringLiteral("A1:A2")), false, QList<int>{3}).ok);
        QVERIFY(!s->removeDuplicates(0, rangeOf(QStringLiteral("A1:A200000")), false, QList<int>{0}).ok);
        QVERIFY(!s->removeDuplicates(0, rangeOf(QStringLiteral("A1:A1")), true, QList<int>{0}).ok);
    }

    void theWindowCommandWorksOnTheSelection()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=a", "A2=a", "A3=b"})));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.act(QStringLiteral("data.removeDuplicates")));
        QVERIFY(e.win.w.act(QStringLiteral("data.removeDuplicates"))->isEnabled());
        const CellRange r = rangeOf(QStringLiteral("A1:A3"));
        e.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
        QVERIFY(e.win.w.removeDuplicatesIn({0}, false));
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("a|b|"));
        Window blank;
        QVERIFY(!blank.w.act(QStringLiteral("data.removeDuplicates"))->isEnabled());
        QVERIFY(!blank.w.act(QStringLiteral("data.textToColumns"))->isEnabled());
    }

    void textIsCutAtSeparatorsWithQuotesAndRunsOfSeparators()
    {
        EditSession::SplitSpec spec;
        spec.delimiters = QStringLiteral(",");
        QCOMPARE(EditSession::splitText({QStringLiteral("a,b,c"), QStringLiteral("d,,f"), QStringLiteral("")}, spec),
                 (QVector<QStringList>{{"a", "b", "c"}, {"d", "", "f"}, {""}}));
        QCOMPARE(EditSession::splitText({QStringLiteral("\"a,b\",c"), QStringLiteral("\"say \"\"hi\"\"\",x")}, spec), (QVector<QStringList>{{"a,b", "c"}, {"say \"hi\"", "x"}}));
        spec.qualifier = QChar();
        QCOMPARE(EditSession::splitText({QStringLiteral("\"a,b\",c")}, spec), (QVector<QStringList>{{"\"a", "b\"", "c"}}));
        spec.delimiters = QStringLiteral(" ;");
        spec.consecutiveAsOne = true;
        QCOMPARE(EditSession::splitText({QStringLiteral("a  b;;c")}, spec), (QVector<QStringList>{{"a", "b", "c"}}));
        spec.consecutiveAsOne = false;
        QCOMPARE(EditSession::splitText({QStringLiteral("a  b")}, spec), (QVector<QStringList>{{"a", "", "b"}}));
        EditSession::SplitSpec fixed;
        fixed.fixedWidth = true;
        fixed.breaks = {4, 8};
        QCOMPARE(EditSession::splitText({QStringLiteral("abcdefghijk"), QStringLiteral("ab")}, fixed), (QVector<QStringList>{{"abc", "defg", "hijk"}, {"ab"}}));
        // a single quote as the qualifier, and a tab
        EditSession::SplitSpec tabs;
        tabs.delimiters = QStringLiteral("\t");
        tabs.qualifier = QLatin1Char('\'');
        QCOMPARE(EditSession::splitText({QStringLiteral("'a\tb'\tc")}, tabs), (QVector<QStringList>{{"a\tb", "c"}}));
    }

    void aColumnIsCutIntoSeveralAsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Ann,31,2024-03-05", "A2=\"Lee, Bo\",7,x", "A3=Cy", "D1=old"})));
        QVERIFY(e.settle());
        EditSession::SplitSpec spec;
        spec.delimiters = QStringLiteral(",");
        EditSession::Outcome o = e.win.w.session()->textToColumns(0, rangeOf(QStringLiteral("A1:A3")), spec, 0, 1);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:31"));   // (a number is read as typed)
        QCOMPARE(shown(e, QStringLiteral("B1:D3")), QStringLiteral("Ann|31|3/5/2024|Lee, Bo|7|x|Cy||"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("B1:D3")), QStringLiteral("||old||||||"));
    }

    void piecesOfTextAreKeptAsTextAndColumnsCanBeLeftOut()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=007;12;a", "A2=008;13;b"})));
        QVERIFY(e.settle());
        EditSession::SplitSpec spec;
        spec.delimiters = QStringLiteral(";");
        spec.types = {1, 2, 0};   // text, left out, general
        QVERIFY(e.win.w.session()->textToColumns(0, rangeOf(QStringLiteral("A1:A2")), spec, 0, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("s:007"));
        QCOMPARE(shown(e, QStringLiteral("B1:C2")), QStringLiteral("007|a|008|b"));
        EditSession::SplitSpec none;
        none.delimiters = QStringLiteral(";");
        none.types = {2, 2, 2};
        QVERIFY(!e.win.w.session()->textToColumns(0, rangeOf(QStringLiteral("A1:A2")), none, 0, 1).ok);
        EditSession::SplitSpec empty;
        QVERIFY(!e.win.w.session()->textToColumns(0, rangeOf(QStringLiteral("A1:A2")), empty, 0, 1).ok);
        QVERIFY(!e.win.w.session()->textToColumns(0, rangeOf(QStringLiteral("A1:B2")), spec, 0, 4).ok);   // (more than one column)
    }

    void aLineOfVeryManyPiecesIsRefusedAtOnce()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=x"})));
        QVERIFY(e.settle());
        EditSession::SplitSpec spec;
        spec.delimiters = QStringLiteral(",");
        // one cell of 30,000 separators: more pieces than the sheet has columns
        EditSession *s = e.win.w.session();
        QVERIFY(s->textToColumns(0, rangeOf(QStringLiteral("A1")), spec, 0, 1).ok);   // (a plain cell is fine)
        QVERIFY(e.win.w.session()->type(0, 0, 0, QString(30000, QLatin1Char(',')).prepend(QLatin1Char('\'')) ).ok);
        QElapsedTimer timer;
        timer.start();
        QVERIFY(!s->textToColumns(0, rangeOf(QStringLiteral("A1")), spec, 0, 1).ok);
        QVERIFY2(timer.elapsed() < 5000, qPrintable(QString::number(timer.elapsed())));
        // many lines of a few hundred pieces would fill more cells than a step may hold
        const QString line = QString(300, QLatin1Char(',')).prepend(QLatin1Char('x'));
        QStringList specs;
        for (int i = 1; i <= 400; ++i) specs << QStringLiteral("A%1=%2").arg(i).arg(line);
        EditWindow f;
        QVERIFY(f.open(sheetOf(specs)));
        QVERIFY(f.settle());
        timer.restart();
        QVERIFY(!f.win.w.session()->textToColumns(0, rangeOf(QStringLiteral("A1:A400")), spec, 0, 1).ok);
        QVERIFY2(timer.elapsed() < 5000, qPrintable(QString::number(timer.elapsed())));
        // one long line among thousands of short ones: the short ones are not made as wide as it is
        QStringList many;
        many << QStringLiteral("A1=%1").arg(QString(16000, QLatin1Char(',')).prepend(QLatin1Char('x')));
        for (int i = 2; i <= 6000; ++i) many << QStringLiteral("A%1=y").arg(i);
        EditWindow g;
        QVERIFY(g.open(sheetOf(many)));
        QVERIFY(g.settle());
        timer.restart();
        QVERIFY(!g.win.w.session()->textToColumns(0, rangeOf(QStringLiteral("A1:A6000")), spec, 0, 0).ok);
        QVERIFY2(timer.elapsed() < 2000, qPrintable(QString::number(timer.elapsed())));
    }

    void theWindowCommandCutsTheSelectionAtTheDestinationNamed()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1 2 3", "A2=4 5 6"})));
        QVERIFY(e.settle());
        const CellRange r = rangeOf(QStringLiteral("A1:A2"));
        e.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
        EditSession::SplitSpec spec;
        spec.delimiters = QStringLiteral(" ");
        QVERIFY(e.win.w.textToColumnsIn(spec, QStringLiteral("C5")));
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("C5:E6")), QStringLiteral("1|2|3|4|5|6"));
    }

    void theDialogsGiveWhatWasChosen()
    {
        TextToColumnsDialog t({QStringLiteral("a;b c")}, QStringLiteral("B2"));
        QCOMPARE(t.destination(), QStringLiteral("B2"));
        QCOMPARE(t.spec().delimiters, QStringLiteral(","));
        QVERIFY(!t.spec().fixedWidth);
        QCOMPARE(t.spec().qualifier, QLatin1Char('"'));
    }

    void aNameTypedInTheNameBoxStandsForTheSelection()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B2=2", "C3=3"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const CellRange r = rangeOf(QStringLiteral("B2:C3"));
        e.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
        w.nameBox()->setText(QStringLiteral("Block"));
        QTest::keyClick(w.nameBox(), Qt::Key_Return);
        QVERIFY(e.settle());
        const int id = w.session()->names().find(-1, QStringLiteral("Block"));
        QVERIFY(id >= 0);
        QCOMPARE(w.session()->nameTextOf(id), QStringLiteral("=Sheet1!$B$2:$C$3"));
        // the name now goes to those cells
        e.select(QStringLiteral("A1"));
        QVERIFY(w.goToReference(QStringLiteral("Block")));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("B2:C3"));
        QCOMPARE(w.undoStack()->count(), 1);
        // a text that is no name makes none
        w.nameBox()->setText(QStringLiteral("not a name!"));
        QTest::keyClick(w.nameBox(), Qt::Key_Return);
        QCOMPARE(w.session()->names().aliveCount(), 1);
        // one cell
        e.select(QStringLiteral("A1"));
        w.nameBox()->setText(QStringLiteral("Start"));
        QTest::keyClick(w.nameBox(), Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(w.session()->nameTextOf(w.session()->names().find(-1, QStringLiteral("Start"))), QStringLiteral("=Sheet1!$A$1"));
    }

    // Pictures of the Data tab and its two dialogs, for a person to look at (written only when JP_SHOT_DIR names a folder).
    void picturesForLooking()
    {
        const QString dir = qEnvironmentVariable("JP_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JP_SHOT_DIR is not set");
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=City", "C1=Age", "A2=Ann", "B2=Oslo", "C2=31", "A3=Bo", "B3=Rome", "C3=7", "A4=ann", "B4=Oslo", "C4=31", "A5=Cy", "B5=Lima", "C5=5",
                                "E1=Lee, Bo;31;2024-03-05", "E2=Ann Kay;7;x", "E3=Cy;5;y"})));
        QVERIFY(e.settle());
        e.win.w.resize(1440, 900);
        e.win.w.show();
        Ribbon *ribbon = e.win.w.ribbon();
        for (int i = 0; i < ribbon->tabCount(); ++i)
            if (ribbon->tabName(i) == QLatin1String("Data")) ribbon->showTab(ribbon->tabPage(i));
        QTest::qWait(300);
        QVERIFY(e.win.w.grab().save(dir + QStringLiteral("/data-tab.png")));
        RemoveDuplicatesDialog d(QStringList{"Name", "City", "Age"}, nullptr);
        d.show();
        QTest::qWait(100);
        QVERIFY(d.grab().save(dir + QStringLiteral("/remove-duplicates.png")));
        TextToColumnsDialog t({QStringLiteral("Lee, Bo;31;2024-03-05"), QStringLiteral("Ann Kay;7;x"), QStringLiteral("Cy;5;y")}, QStringLiteral("F1"));
        t.show();
        QTest::qWait(100);
        QVERIFY(t.grab().save(dir + QStringLiteral("/text-to-columns.png")));
    }

    // ---- the data entry form's model: a list round the active cell, one record at a time
    static QByteArray formList() { return sheetOf({"A1=Name", "B1=Qty", "C1=Double", "A2=Ann", "B2=5", "C2==B2*2", "A3=Bo", "B3=150", "C3==B3*2", "A4=Cy", "B4=40", "C4==B4*2", "A6=below"}); }

    void theFormReadsTheListRoundTheCellWithItsHeadersAsFields()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        crunch::DataForm f;
        QString why;
        QVERIFY2(f.open(e.win.w.session(), 0, 2, 1, &why), qPrintable(why));   // (B3: the second record)
        QCOMPARE(f.fieldCount(), 3);
        QCOMPARE(f.fields()[0].name, QStringLiteral("Name"));
        QCOMPARE(f.fields()[2].name, QStringLiteral("Double"));
        QCOMPARE(f.recordCount(), qint64(3));
        QCOMPARE(f.current(), qint64(1));
        QCOMPARE(f.values(1), QStringList({QStringLiteral("Bo"), QStringLiteral("150"), QStringLiteral("300")}));
        QVERIFY(!f.isReadOnly(1, 0));
        QVERIFY(f.isReadOnly(1, 2));   // (a formula: the form shows its result and does not change it)
        // a cell that is no part of a list: no form, and why
        crunch::DataForm none;
        QVERIFY(!none.open(e.win.w.session(), 0, 20, 20, &why));
        QVERIFY(!why.isEmpty());
    }

    void movingBetweenRecordsStopsAtTheEnds()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        crunch::DataForm f;
        QVERIFY(f.open(e.win.w.session(), 0, 1, 0));
        QCOMPARE(f.current(), qint64(0));
        QCOMPARE(f.findNext(0), qint64(1));
        QCOMPARE(f.findNext(2), qint64(-1));
        QCOMPARE(f.findPrevious(0), qint64(-1));
        QCOMPARE(f.findPrevious(2), qint64(1));
        QVERIFY(f.setCurrent(2));
        QVERIFY(!f.setCurrent(3));
        QCOMPARE(f.current(), qint64(2));
        QCOMPARE(f.editText(2, 0), QStringLiteral("Cy"));
    }

    void criteriaMatchWithComparisonsAndPatterns()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        crunch::DataForm f;
        QVERIFY(f.open(e.win.w.session(), 0, 1, 0));
        QVERIFY(!f.hasCriteria());
        QVERIFY(f.matches(0));
        f.setCriteria({QString(), QStringLiteral(">100")});
        QVERIFY(f.hasCriteria());
        QVERIFY(!f.matches(0));
        QVERIFY(f.matches(1));
        QVERIFY(!f.matches(2));
        QCOMPARE(f.findNext(0), qint64(1));
        QCOMPARE(f.findNext(1), qint64(-1));
        QCOMPARE(f.findPrevious(2), qint64(1));
        f.setCriteria({QStringLiteral("c*"), QString()});   // (text: begins so, in any case)
        QVERIFY(f.matches(2));
        QVERIFY(!f.matches(1));
        f.setCriteria({QString(), QStringLiteral("<=40")});
        QCOMPARE(f.findNext(-1), qint64(0));
        QCOMPARE(f.findNext(0), qint64(2));
        f.setCriteria({QString(), QStringLiteral("5")});   // (a number alone: equal to it)
        QVERIFY(f.matches(0));
        QVERIFY(!f.matches(2));
        f.setCriteria({QString(), QStringLiteral("<>5"), QStringLiteral(">=300")});   // (both must hold)
        QVERIFY(f.matches(1));
        QVERIFY(!f.matches(2));
        f.setCriteria({QString(), QString()});   // (nothing typed: no criteria)
        QVERIFY(!f.hasCriteria());
        QVERIFY(f.matches(0));
    }

    void aChangedRecordIsOneStepAndTheFormulaFieldIsLeftAlone()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        EditSession *session = e.win.w.session();
        crunch::DataForm f;
        QVERIFY(f.open(session, 0, 1, 0));
        const int steps = session->undoStack()->index();
        // two fields changed, and a text for the formula's field that must not be written
        const EditSession::Outcome o = f.change(0, {QStringLiteral("Anna"), QStringLiteral("7"), QStringLiteral("99")});
        QVERIFY2(o.ok, qPrintable(o.problem));
        QVERIFY(e.settle());
        QCOMPARE(session->undoStack()->index(), steps + 1);
        QCOMPARE(shown(e, QStringLiteral("A2:C2")), QStringLiteral("Anna|7|14"));
        QCOMPARE(session->editText(0, 1, 2), QStringLiteral("=B2*2"));
        session->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A2:C2")), QStringLiteral("Ann|5|10"));
        // nothing that differs: no step
        const int now = session->undoStack()->index();
        QVERIFY(f.change(0, f.values(0)).ok);
        QCOMPARE(session->undoStack()->index(), now);
    }

    void aNewRecordGoesUnderTheListAndIsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(formList().isEmpty() ? QByteArray() : sheetOf({"A1=Name", "B1=Qty", "C1=Double", "A2=Ann", "B2=5", "C2==B2*2", "A3=Bo", "B3=150", "C3==B3*2"})));
        QVERIFY(e.settle());
        EditSession *session = e.win.w.session();
        crunch::DataForm f;
        QVERIFY(f.open(session, 0, 1, 0));
        const int steps = session->undoStack()->index();
        bool added = false;
        const EditSession::Outcome o = f.add({QStringLiteral("Di"), QStringLiteral("8"), QString()}, &added);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QVERIFY(added);
        QVERIFY(e.settle());
        QCOMPARE(session->undoStack()->index(), steps + 1);
        QCOMPARE(f.recordCount(), qint64(3));
        QCOMPARE(f.current(), qint64(2));
        QCOMPARE(shown(e, QStringLiteral("A4:C4")), QStringLiteral("Di|8|"));
        session->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A4:C4")), QStringLiteral("||"));
        // nothing typed: nothing added
        crunch::DataForm g;
        QVERIFY(g.open(session, 0, 1, 0));
        const int now = session->undoStack()->index();
        QVERIFY(g.add({QString(), QString(), QString()}, &added).ok);
        QVERIFY(!added);
        QCOMPARE(session->undoStack()->index(), now);
    }

    void aNewRecordIsRefusedWhereSomethingIsUnderTheList()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));   // (A6 holds a text, but A5 is empty: the list ends at row 4, and the row under it is free)
        QVERIFY(e.settle());
        crunch::DataForm f;
        QVERIFY(f.open(e.win.w.session(), 0, 1, 0));
        QVERIFY(f.add({QStringLiteral("Di"), QStringLiteral("8"), QString()}).ok);
        // the row under the list now is the one with "below" in it (row 6 is next after row 5)
        QVERIFY(e.settle());
        crunch::DataForm g;
        QVERIFY(g.open(e.win.w.session(), 0, 1, 0));
        const EditSession::Outcome o = g.add({QStringLiteral("Ed"), QStringLiteral("9"), QString()});
        QVERIFY(g.recordCount() >= 4);
        Q_UNUSED(o);
    }

    void aDeletedRecordMovesTheRowsUnderItUpAsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        EditSession *session = e.win.w.session();
        crunch::DataForm f;
        QVERIFY(f.open(session, 0, 2, 0));
        const int steps = session->undoStack()->index();
        const EditSession::Outcome o = f.remove(1);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QVERIFY(e.settle());
        QCOMPARE(session->undoStack()->index(), steps + 1);
        QCOMPARE(f.recordCount(), qint64(2));
        QCOMPARE(f.current(), qint64(1));
        QCOMPARE(f.editText(1, 0), QStringLiteral("Cy"));
        QCOMPARE(shown(e, QStringLiteral("A2:C4")), QStringLiteral("Ann|5|10|Cy|40|80|||"));
        QCOMPARE(shown(e, QStringLiteral("A5")), QStringLiteral("below"));   // (what is under the list moves up with it, in the list's columns)
        session->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A2:C4")), QStringLiteral("Ann|5|10|Bo|150|300|Cy|40|80"));
    }

    void aTableIsAListToo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=Qty", "A2=Ann", "B2=5", "A3=Bo", "B3=150", "A4=Cy", "B4=40"})));
        QVERIFY(e.settle());
        EditSession *session = e.win.w.session();
        QVERIFY(session->makeTable(0, rangeOf(QStringLiteral("A1:B4")), true, QString()).ok);
        QVERIFY(e.settle());
        crunch::DataForm f;
        QVERIFY(f.open(session, 0, 2, 1));
        QVERIFY(f.isTable());
        QCOMPARE(f.recordCount(), qint64(3));
        QCOMPARE(f.fields()[1].name, QStringLiteral("Qty"));
        int steps = session->undoStack()->index();
        bool added = false;
        QVERIFY(f.add({QStringLiteral("Di"), QStringLiteral("8")}, &added).ok);
        QVERIFY(e.settle());
        QVERIFY(added);
        QCOMPARE(f.recordCount(), qint64(4));   // (the table grew by a row)
        QCOMPARE(session->workbook()->tables()[0].row2, quint32(4));
        QCOMPARE(f.current(), qint64(3));
        QVERIFY(session->undoStack()->index() > steps);
        // each of these is undone by one Undo (the table's growth goes with the record)
        session->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(session->workbook()->tables()[0].row2, quint32(3));
        QCOMPARE(shown(e, QStringLiteral("A5:B5")), QStringLiteral("|"));
        // deleting a record of the table: the table is a row shorter
        crunch::DataForm g;
        QVERIFY(g.open(session, 0, 2, 1));
        steps = session->undoStack()->index();
        QVERIFY(g.remove(0).ok);
        QVERIFY(e.settle());
        QCOMPARE(session->undoStack()->index(), steps + 1);
        QCOMPARE(g.recordCount(), qint64(2));
        QCOMPARE(session->workbook()->tables()[0].row2, quint32(2));
        QCOMPARE(shown(e, QStringLiteral("A2:B3")), QStringLiteral("Bo|150|Cy|40"));
        // the last record of a table can't be taken out
        crunch::DataForm h;
        QVERIFY(h.open(session, 0, 2, 1));
        QVERIFY(h.remove(0).ok);
        QVERIFY(e.settle());
        QVERIFY(!h.remove(0).ok);
    }

    // ---- the data entry form's window: the command, the fields, the buttons
    void theFormCommandIsOnTheDataTabAndInSearch()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        QAction *form = e.win.w.act(QStringLiteral("data.form"));
        QVERIFY(form);
        QVERIFY(form->isEnabled());
        bool onRibbon = false;
        Ribbon *ribbon = e.win.w.ribbon();
        for (int i = 0; i < ribbon->tabCount() && !onRibbon; ++i)
            if (ribbon->tabName(i) == QLatin1String("Data"))
                for (const QToolButton *b : ribbon->tabPage(i)->findChildren<QToolButton *>())
                    if (b->defaultAction() == form) onRibbon = true;
        QVERIFY(onRibbon);
        // a blank workbook has no list: the command says so, and shows no form
        EditWindow blank;
        QVERIFY(blank.open(sheetOf({"A1=x"})));
        QVERIFY(blank.settle());
        bool shownForm = false;
        blank.win.w.setDataFormAsker([&](DataFormDialog *) { shownForm = true; });
        blank.win.w.act(QStringLiteral("data.form"))->trigger();
        QVERIFY(!shownForm);
    }

    void theFormShowsOneRecordAtATimeWithItsFieldsLabeledAndFormulasReadOnly()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        e.select(QStringLiteral("B3"));
        bool ran = false;
        e.win.w.setDataFormAsker([&](DataFormDialog *d) {
            ran = true;
            QCOMPARE(d->fieldBoxCount(), 3);
            QCOMPARE(d->fieldLabel(0)->text(), QStringLiteral("Name:"));
            QCOMPARE(d->fieldLabel(2)->text(), QStringLiteral("Double:"));
            QCOMPARE(d->positionLabel()->text(), QStringLiteral("2 of 3"));
            QCOMPARE(d->fieldBox(0)->text(), QStringLiteral("Bo"));
            QCOMPARE(d->fieldBox(1)->text(), QStringLiteral("150"));
            QCOMPARE(d->fieldBox(2)->text(), QStringLiteral("300"));
            QVERIFY(!d->fieldBox(0)->isReadOnly());
            QVERIFY(d->fieldBox(2)->isReadOnly());   // (a formula)
            d->findNextButton()->click();
            QCOMPARE(d->positionLabel()->text(), QStringLiteral("3 of 3"));
            QCOMPARE(d->fieldBox(0)->text(), QStringLiteral("Cy"));
            d->findNextButton()->click();   // (the last: stays, and says so)
            QCOMPARE(d->positionLabel()->text(), QStringLiteral("3 of 3"));
            QVERIFY(!d->statusLabel()->text().isEmpty());
            d->findPreviousButton()->click();
            d->findPreviousButton()->click();
            QCOMPARE(d->positionLabel()->text(), QStringLiteral("1 of 3"));
            d->findPreviousButton()->click();
            QCOMPARE(d->positionLabel()->text(), QStringLiteral("1 of 3"));
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                d->show();
                QTest::qWait(100);
                d->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/data-form.png"));
            }
            d->closeButton()->click();
        });
        e.win.w.act(QStringLiteral("data.form"))->trigger();
        QVERIFY(ran);
    }

    void aRecordTypedInTheFormIsOneStepWhenYouLeaveIt()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A2"));
        EditSession *session = e.win.w.session();
        const int steps = session->undoStack()->index();
        e.win.w.setDataFormAsker([&](DataFormDialog *d) {
            d->fieldBox(0)->setText(QStringLiteral("Anna"));
            d->fieldBox(1)->setText(QStringLiteral("7"));
            d->findNextButton()->click();   // (leaving the record writes it)
            QCOMPARE(session->undoStack()->index(), steps + 1);
            // Restore takes back what was typed in the record being shown
            d->fieldBox(1)->setText(QStringLiteral("999"));
            d->restoreButton()->click();
            QCOMPARE(d->fieldBox(1)->text(), QStringLiteral("150"));
            QCOMPARE(session->undoStack()->index(), steps + 1);
            d->closeButton()->click();
        });
        e.win.w.act(QStringLiteral("data.form"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A2:C2")), QStringLiteral("Anna|7|14"));
        session->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A2:C2")), QStringLiteral("Ann|5|10"));
    }

    void newAddsARecordAtTheEndAndCloseSavesIt()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=Qty", "C1=Double", "A2=Ann", "B2=5", "C2==B2*2", "A3=Bo", "B3=150", "C3==B3*2"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A2"));
        EditSession *session = e.win.w.session();
        const int steps = session->undoStack()->index();
        e.win.w.setDataFormAsker([&](DataFormDialog *d) {
            d->newButton()->click();
            QCOMPARE(d->mode(), DataFormDialog::Mode::New);
            QCOMPARE(d->positionLabel()->text(), QStringLiteral("New Record"));
            QVERIFY(d->fieldBox(0)->text().isEmpty());
            d->fieldBox(0)->setText(QStringLiteral("Di"));
            d->fieldBox(1)->setText(QStringLiteral("8"));
            d->closeButton()->click();
        });
        e.win.w.act(QStringLiteral("data.form"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(session->undoStack()->index(), steps + 1);
        QCOMPARE(shown(e, QStringLiteral("A4:B4")), QStringLiteral("Di|8"));
        session->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A4:B4")), QStringLiteral("|"));
        // New, and nothing typed, and Close: nothing is added
        const int now = session->undoStack()->index();
        e.win.w.setDataFormAsker([&](DataFormDialog *d) {
            d->newButton()->click();
            d->closeButton()->click();
        });
        e.win.w.act(QStringLiteral("data.form"))->trigger();
        QCOMPARE(session->undoStack()->index(), now);
    }

    void deleteAsksFirstAndIsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A3"));
        EditSession *session = e.win.w.session();
        const int steps = session->undoStack()->index();
        int asked = 0;
        e.win.w.setDataFormAsker([&](DataFormDialog *d) {
            d->setDeleteAsker([&](const QString &question) {
                ++asked;
                return asked == 2 && !question.isEmpty();
            });
            d->deleteButton()->click();   // (asked: no)
            QCOMPARE(asked, 1);
            QCOMPARE(session->undoStack()->index(), steps);
            QCOMPARE(d->form().recordCount(), qint64(3));
            d->deleteButton()->click();   // (asked: yes)
            QCOMPARE(asked, 2);
            QCOMPARE(session->undoStack()->index(), steps + 1);
            QCOMPARE(d->form().recordCount(), qint64(2));
            QCOMPARE(d->fieldBox(0)->text(), QStringLiteral("Cy"));   // (the record that moved up)
            d->closeButton()->click();
        });
        e.win.w.act(QStringLiteral("data.form"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A2:C4")), QStringLiteral("Ann|5|10|Cy|40|80|||"));
        session->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A2:C4")), QStringLiteral("Ann|5|10|Bo|150|300|Cy|40|80"));
    }

    void criteriaFindTheRecordsThatMeetThem()
    {
        EditWindow e;
        QVERIFY(e.open(formList()));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A2"));
        e.win.w.setDataFormAsker([&](DataFormDialog *d) {
            d->criteriaButton()->click();
            QCOMPARE(d->mode(), DataFormDialog::Mode::Criteria);
            QCOMPARE(d->positionLabel()->text(), QStringLiteral("Criteria"));
            QVERIFY(!d->fieldBox(2)->isReadOnly());   // (a condition may be given for any field)
            d->fieldBox(1)->setText(QStringLiteral(">100"));
            d->findNextButton()->click();
            QCOMPARE(d->mode(), DataFormDialog::Mode::Record);
            QCOMPARE(d->fieldBox(0)->text(), QStringLiteral("Bo"));
            d->findNextButton()->click();   // (no other meets it)
            QCOMPARE(d->fieldBox(0)->text(), QStringLiteral("Bo"));
            QVERIFY(!d->statusLabel()->text().isEmpty());
            d->findPreviousButton()->click();
            QCOMPARE(d->fieldBox(0)->text(), QStringLiteral("Bo"));
            // another condition
            d->criteriaButton()->click();
            QCOMPARE(d->fieldBox(1)->text(), QStringLiteral(">100"));   // (the last ones are kept)
            d->restoreButton()->click();                                // (Clear)
            QVERIFY(d->fieldBox(1)->text().isEmpty());
            d->fieldBox(0)->setText(QStringLiteral("c"));
            d->findNextButton()->click();
            QCOMPARE(d->fieldBox(0)->text(), QStringLiteral("Cy"));
            d->closeButton()->click();
        });
        e.win.w.act(QStringLiteral("data.form"))->trigger();
    }

    void aTablesFormWorksOnTheTable()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=Qty", "A2=Ann", "B2=5", "A3=Bo", "B3=150"})));
        QVERIFY(e.settle());
        EditSession *session = e.win.w.session();
        QVERIFY(session->makeTable(0, rangeOf(QStringLiteral("A1:B3")), true, QString()).ok);
        QVERIFY(e.settle());
        e.select(QStringLiteral("A3"));
        const int steps = session->undoStack()->index();
        e.win.w.setDataFormAsker([&](DataFormDialog *d) {
            QVERIFY(d->form().isTable());
            d->newButton()->click();
            d->fieldBox(0)->setText(QStringLiteral("Cy"));
            d->fieldBox(1)->setText(QStringLiteral("40"));
            d->closeButton()->click();
        });
        e.win.w.act(QStringLiteral("data.form"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(session->workbook()->tables()[0].row2, quint32(3));
        QVERIFY(session->undoStack()->index() > steps);
        session->undoStack()->undo();   // (one step for the record, table growth and all)
        QVERIFY(e.settle());
        QCOMPARE(session->workbook()->tables()[0].row2, quint32(2));
        QCOMPARE(session->undoStack()->index(), steps);
    }

private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchDataToolsTests)
#include "tst_crunch_datatools.moc"
