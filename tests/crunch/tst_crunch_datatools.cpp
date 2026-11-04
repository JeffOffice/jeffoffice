// Tests of the Data tools that change cells: Remove Duplicates (which rows go, that the rows under them move up in the range and what is under the range stays, headers, case, columns
// compared, formulas and formats that go with their rows, one step of Undo) and Text to Columns (separators, a qualifier, fixed places, kinds of piece, a destination). Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/datatoolsdialogs.h"
#include "app/removeduplicatesdialog.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "app/ribbon.h"

#include <QAction>
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

private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchDataToolsTests)
#include "tst_crunch_datatools.moc"
