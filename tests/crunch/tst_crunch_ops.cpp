// Tests of what a person does with a block of cells besides typing in it: AutoSum and the functions of its menu, fill, the clipboard, inserting and deleting, the sizes of rows and
// columns, and the rules of validation. Every workbook is built here, in a temporary folder; each step is one on the undo stack.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/filldialogs.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/flashfill.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"
#include "grid/gridview.h"
#include "sheet/pivotbook.h"

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QClipboard>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QToolButton>
#include <QSettings>
#include <QSignalSpy>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// Numbers to total:
//        A    B    C    D
//   1    1    2    x
//   2    3    4
//   3    5    6
//   4
//   5    7    8
QByteArray numbersBook()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\" t=\"inlineStr\"><is><t>x</t></is></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>3</v></c><c r=\"B2\"><v>4</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\"><v>5</v></c><c r=\"B3\"><v>6</v></c></row>"
                                        "<row r=\"5\"><c r=\"A5\"><v>7</v></c><c r=\"B5\"><v>8</v></c></row>");
    return buildXlsx({worksheet(rows)});
}

} // namespace

class CrunchOpsTests : public QObject {
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

    // ---- fill

    // The cells past a block go on as the block does: numbers by their step (a trend through several), a single number copied, text that ends in a number counting up (with its zeros),
    // the names of weekdays and months in turn (in their capitals), and a formula with its references moved; up and left go the other way.
    void fillGoesOnAsTheBlockDoes()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "C1=5", "C2=3", "D1=7", "E1=Item 1", "E2=Item 2", "F1=Item 009", "G1=Mon", "G2=Tue", "H1=january", "I1=Dec", "J1=x", "K1==A1*2+$A$1+B$1", "L1=1", "L2=2",
                                 "L3=4", "M1=MONDAY", "N1=Sat", "N2=Sun", "O1=1.5", "O2=2", "A10=3", "A11=4", "P20=1", "Q20=2", "R1=@2024-01-15", "R2=@2024-02-15", "S1=@2024-03-01", "S2=@2025-03-01",
                                 "T1=@2024-03-01", "U1=%0.5"})));
        using Dir = crunch::EditSession::FillDirection;
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        auto fill = [&](const QString &ref, Dir d, quint32 n) {
            const crunch::EditSession::Outcome o = s->fill(0, rangeOf(ref), d, n);
            QVERIFY2(o.ok, qPrintable(o.problem));
        };
        fill(QStringLiteral("A1:A2"), Dir::Down, 3);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("1|2|3|4|5"));
        fill(QStringLiteral("C1:C2"), Dir::Down, 3);
        QCOMPARE(shown(e, QStringLiteral("C1:C5")), QStringLiteral("5|3|1|-1|-3"));
        fill(QStringLiteral("D1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("D1:D3")), QStringLiteral("7|7|7"));                                  // (a single number is copied)
        fill(QStringLiteral("E1:E2"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("E1:E4")), QStringLiteral("Item 1|Item 2|Item 3|Item 4"));
        fill(QStringLiteral("F1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("F1:F3")), QStringLiteral("Item 009|Item 010|Item 011"));
        fill(QStringLiteral("G1:G2"), Dir::Down, 3);
        QCOMPARE(shown(e, QStringLiteral("G1:G5")), QStringLiteral("Mon|Tue|Wed|Thu|Fri"));
        fill(QStringLiteral("H1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("H1:H3")), QStringLiteral("january|february|march"));
        fill(QStringLiteral("I1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("I1:I3")), QStringLiteral("Dec|Jan|Feb"));
        fill(QStringLiteral("J1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("J1:J3")), QStringLiteral("x|x|x"));
        fill(QStringLiteral("K1"), Dir::Down, 2);
        QCOMPARE(s->editText(0, 1, 10), QStringLiteral("=A2*2+$A$1+B$1"));
        QCOMPARE(s->editText(0, 2, 10), QStringLiteral("=A3*2+$A$1+B$1"));
        fill(QStringLiteral("L1:L3"), Dir::Down, 1);                                                         // 1, 2, 4: the trend through them
        QCOMPARE(s->editText(0, 3, 11), QStringLiteral("5.33333333333333"));
        fill(QStringLiteral("M1"), Dir::Down, 1);
        QCOMPARE(shown(e, QStringLiteral("M1:M2")), QStringLiteral("MONDAY|TUESDAY"));
        fill(QStringLiteral("N1:N2"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("N1:N4")), QStringLiteral("Sat|Sun|Mon|Tue"));
        fill(QStringLiteral("O1:O2"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("O1:O4")), QStringLiteral("1.5|2|2.5|3"));
        fill(QStringLiteral("A10:A11"), Dir::Up, 2);                                                         // up goes the other way
        QCOMPARE(shown(e, QStringLiteral("A8:A11")), QStringLiteral("1|2|3|4"));
        fill(QStringLiteral("P20:Q20"), Dir::Right, 2);
        QCOMPARE(shown(e, QStringLiteral("P20:S20")), QStringLiteral("1|2|3|4"));
        fill(QStringLiteral("Q20:P20"), Dir::Left, 1);                                                       // (the block P20:Q20 again: left of it)
        QCOMPARE(shown(e, QStringLiteral("O20:P20")), QStringLiteral("0|1"));                                  // (1 and 2 going left: 0)
        // dates: months, years, and a single date by days
        fill(QStringLiteral("R1:R2"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("R1:R4")), QStringLiteral("1/15/2024|2/15/2024|3/15/2024|4/15/2024"));
        fill(QStringLiteral("S1:S2"), Dir::Down, 1);
        QCOMPARE(shown(e, QStringLiteral("S3")), QStringLiteral("3/1/2026"));
        fill(QStringLiteral("T1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("T1:T3")), QStringLiteral("3/1/2024|3/2/2024|3/3/2024"));
        // formats follow the cells
        fill(QStringLiteral("U1"), Dir::Down, 1);
        QCOMPARE(shown(e, QStringLiteral("U1:U2")), QStringLiteral("50%|50%"));
        // one step each
        QCOMPARE(e.win.w.undoStack()->count(), 21);
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("U2")), QString());
    }

    // The names of weekdays and months go in turn in the person's language (the locale is set here), and the English ones too.
    void fillNamesInTheUsersLanguage()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Montag", "B1=Januar", "C1=Mon", "D1=Dezember"})));
        using Dir = crunch::EditSession::FillDirection;
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::German, QLocale::Germany));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("Montag|Dienstag|Mittwoch"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("B1:B3")), QStringLiteral("Januar|Februar|M\u00e4rz"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("Mon|Tue|Wed"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("D1")), Dir::Down, 1).ok);
        QCOMPARE(shown(e, QStringLiteral("D1:D2")), QStringLiteral("Dezember|Januar"));
        s->setLocale(QLocale(QLocale::French, QLocale::France));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A3")), Dir::Down, 1).ok);          // (a German name in a French locale: not a name there; copied)
        QCOMPARE(shown(e, QStringLiteral("A3:A4")), QStringLiteral("Mittwoch|Mittwoch"));
    }

    // A custom list (kept in Options) goes in turn like the weekdays: a cell that holds one of its items is followed by the next, round to the first, without regard to capitals;
    // cells that hold items of different lists are copied.
    void fillFollowsCustomLists()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Small", "B1=large", "C1=Small", "D1=Large", "E1=Small"})));
        using Dir = crunch::EditSession::FillDirection;
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("Small|Small|Small"));          // (not a list yet: copied)
        s->setCustomLists({{QStringLiteral("Small"), QStringLiteral("Medium"), QStringLiteral("Large")}});
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C1")), Dir::Down, 4).ok);
        QCOMPARE(shown(e, QStringLiteral("C1:C5")), QStringLiteral("Small|Medium|Large|Small|Medium"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1")), Dir::Down, 2).ok);                       // lower case: the next ones in lower case
        QCOMPARE(shown(e, QStringLiteral("B1:B3")), QStringLiteral("large|small|medium"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("E1")), Dir::Left, 1).ok);
        QCOMPARE(shown(e, QStringLiteral("D1:E1")), QStringLiteral("Large|Small"));
        s->setCustomLists({});                                                                     // taken away: copied again
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C1")), Dir::Down, 1).ok);
        QCOMPARE(shown(e, QStringLiteral("C1:C2")), QStringLiteral("Small|Small"));
    }

    // The Custom Lists page of Options: the lists typed there are kept in the settings (one line each, the words between commas) and are what Fill goes through at once; a workbook that is opened
    // does not rewrite the settings; the limits of a list hold.
    void customListsPageAndSettingsRoundTrip()
    {
        auto &settings = Settings::get();
        struct Cleanup {
            ~Cleanup() { Settings::get().setValue(QStringLiteral("crunch/customLists"), QStringList()); }   // (the person's settings are put back whether or not a check fails)
        } cleanup;
        settings.setValue(QStringLiteral("crunch/customLists"), QStringList{QStringLiteral(" Small ,Medium,  Large ")});   // (written by hand, not as the program writes it)
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Medium", "C1=Red"})));
        QCOMPARE(settings.value(QStringLiteral("crunch/customLists")).toStringList(), QStringList{QStringLiteral(" Small ,Medium,  Large ")});   // (opening did not rewrite it)
        using Dir = crunch::EditSession::FillDirection;
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("Medium|Large|Small"));
        // the page
        // (the window is waited for: a timer looks for it until it is there)
        auto *watch = new QTimer(&e.win.w);
        watch->setInterval(10);
        connect(watch, &QTimer::timeout, watch, [watch] {
            QWidget *dialog = QApplication::activeModalWidget();
            if (!dialog || dialog->findChildren<QPlainTextEdit *>(QStringLiteral("optionCustomLists")).isEmpty()) return;
            watch->stop();
            for (QPlainTextEdit *edit : dialog->findChildren<QPlainTextEdit *>(QStringLiteral("optionCustomLists"))) {
                QCOMPARE(edit->toPlainText(), QStringLiteral("Small, Medium, Large"));
                edit->setPlainText(QStringLiteral("Red, Green, Blue\nlonely\n"));
            }
            if (auto *buttons = dialog->findChild<QDialogButtonBox *>()) buttons->button(QDialogButtonBox::Ok)->click();
        });
        watch->start();
        e.win.w.act(QStringLiteral("file.options"))->trigger();
        QCOMPARE(settings.value(QStringLiteral("crunch/customLists")).toStringList(), QStringList{QStringLiteral("Red, Green, Blue")});   // (a list of one word is no list)
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("Red|Green|Blue"));
        // the limits: 50 lists, 1,000 words in a list, 255 characters in a word
        QList<QStringList> many;
        for (int i = 0; i < 60; ++i) many.append({QStringLiteral("a%1").arg(i), QStringLiteral("b%1").arg(i)});
        s->setCustomLists(many);
        QCOMPARE(int(s->customLists().size()), crunch::EditSession::kMaxCustomLists);
        QStringList longList;
        for (int i = 0; i < 1200; ++i) longList << QStringLiteral("w%1").arg(i);
        longList << QString(300, QLatin1Char('x'));
        s->setCustomLists({longList});
        QCOMPARE(int(s->customLists().first().size()), crunch::EditSession::kMaxCustomListItems);
        // a word of more than 255 characters, even the first of its list, is left out
        s->setCustomLists({{QString(300, QLatin1Char('x')), QStringLiteral("b"), QStringLiteral("c")}});
        QCOMPARE(s->customLists().first(), QStringList({QStringLiteral("b"), QStringLiteral("c")}));
        // and the same limits hold for what the settings hold: they are read through the window's own parser
        QStringList lines = {QStringLiteral("%1, p, q").arg(QString(300, QLatin1Char('y')))};
        QString big;
        for (int i = 0; i < 1200; ++i) big += QStringLiteral("w%1,").arg(i);
        lines << big << QStringLiteral("only");
        for (int i = 0; i < 60; ++i) lines << QStringLiteral("m%1, n%1").arg(i);
        settings.setValue(QStringLiteral("crunch/customLists"), lines);
        EditWindow again;
        QVERIFY(again.open(sheetOf({"A1=p"})));
        const QList<QStringList> held = again.win.w.session()->customLists();
        QCOMPARE(int(held.size()), crunch::EditSession::kMaxCustomLists);
        QCOMPARE(held[0], QStringList({QStringLiteral("p"), QStringLiteral("q")}));
        QCOMPARE(int(held[1].size()), crunch::EditSession::kMaxCustomListItems);
        QCOMPARE(settings.value(QStringLiteral("crunch/customLists")).toStringList(), lines);   // (opening wrote nothing)
    }

    // The right click on the numbers of the selection in the status bar: a menu with the six choices that are checked as they are on, one at least stays on, and the choice is kept.
    void theStatusBarMenuChoosesWhichNumbersAreShown()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=6"})));
        e.view()->selectRange(0, 0, 2, 0);
        QTRY_VERIFY(e.win.w.findChild<QLabel *>(QStringLiteral("selectionStats"))->text().startsWith(QLatin1String("Average: ")));
        QMenu *menu = e.win.w.statsChoiceMenu();
        QVERIFY(menu);
        QStringList names;
        for (QAction *a : menu->actions()) names << a->text() + (a->isChecked() ? QStringLiteral("*") : QString());
        QCOMPARE(names, QStringList({"Average*", "Count*", "Numerical Count", "Minimum", "Maximum", "Sum*"}));
        menu->actions().at(3)->trigger();                                          // Minimum on
        QVERIFY(e.win.w.findChild<QLabel *>(QStringLiteral("selectionStats"))->text().contains(QLatin1String("Minimum: 1")));
        QCOMPARE(Settings::get().value(QStringLiteral("crunch/statusStats")).toInt(), grid::kDefaultStatChoices | grid::StatMinimum);
        for (int i : {0, 1, 5, 3}) e.win.w.statsChoiceMenu()->actions().at(i)->trigger();   // Average, Count, Sum, then Minimum off: the last one stays
        QCOMPARE(e.view()->statChoices(), int(grid::StatMinimum));
        Settings::get().setValue(QStringLiteral("crunch/statusStats"), grid::kDefaultStatChoices);   // (the choice is the person's: the next window starts as it was)
    }

    // Flash Fill finds how the cells typed by hand come from the cells beside them, and fills the empty cells of the column the same way, as one step.
    void flashFillLearnsFromTheExamples()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Smith, Ann", "A2=Jones, Bob", "A3=Lee, Cy", "A4=Park, Di", "B1=Ann Smith"})));
        crunch::EditSession *s = e.win.w.session();
        int filled = 0;
        const crunch::EditSession::Outcome first = s->flashFill(0, 0, 1, &filled);
        QVERIFY2(first.ok, qPrintable(first.problem));
        QCOMPARE(filled, 3);
        QCOMPARE(shown(e, QStringLiteral("B1:B4")), QStringLiteral("Ann Smith|Bob Jones|Cy Lee|Di Park"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("B1:B4")), QStringLiteral("Ann Smith|||"));
    }

    void flashFillChangesCaseAndTakesInitialsAndPieces()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=ann smith", "A2=bob jones", "A3=cy lee", "B1=A. SMITH", "B2=B. JONES", "C1=Ann", "D1=x-100", "D2=y-250", "D3=z-7", "E1=100"})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->flashFill(0, 0, 1).ok);
        QCOMPARE(shown(e, QStringLiteral("B1:B3")), QStringLiteral("A. SMITH|B. JONES|C. LEE"));
        QVERIFY(s->flashFill(0, 0, 4).ok);   // the number after the dash
        QCOMPARE(shown(e, QStringLiteral("E1:E3")), QStringLiteral("100|250|7"));
    }

    // The search is held to limits: long texts and many neighbors make no unbounded work, and a pattern that is only typed text is not one.
    void flashFillSearchIsBounded()
    {
        QVector<crunch::FlashFillExample> ex;
        for (int i = 0; i < 40; ++i) {
            crunch::FlashFillExample x;
            for (int c = 0; c < 40; ++c) x.sources << QString(QByteArray("ab cd ef ").repeated(200)) + QString::number(i + c);
            x.output = QString(QByteArray("zq%1 ").repeated(3)).arg(i);
            ex << x;
        }
        const crunch::FlashFillPattern none = crunch::learnFlashFill(ex);
        QVERIFY(!none.isValid());
        crunch::FlashFillExample same;
        same.sources << QStringLiteral("a");
        same.output = QStringLiteral("typed");
        QVERIFY(!crunch::learnFlashFill({same}).isValid());
        QVERIFY(!crunch::learnFlashFill({}).isValid());
    }

    void flashFillRefusesWhenThereIsNothingToLearnOrFill()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=one", "A2=two", "B1=zzz"})));
        crunch::EditSession *s = e.win.w.session();
        crunch::EditSession::Outcome o = s->flashFill(0, 0, 1);   // "zzz" is in no neighbor
        QVERIFY(!o.ok && !o.problem.isEmpty());
        QVERIFY(!s->flashFill(0, 5, 3).ok);   // nothing beside row 6
        QCOMPARE(e.win.w.undoStack()->count(), 0);
    }

    void flashFillIsACommandOnTheFillMenu()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Smith, Ann", "A2=Jones, Bob", "B1=Ann"})));
        QAction *a = e.win.w.act(QStringLiteral("fill.flash"));
        QVERIFY(a && a->isEnabled());
        QCOMPARE(a->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_E).toString(QKeySequence::NativeText));
        e.select(QStringLiteral("B2"));
        a->trigger();
        QTRY_COMPARE(shown(e, QStringLiteral("B1:B2")), QStringLiteral("Ann|Bob"));
    }

    // Recommended pivot tables: layouts picked from the shape of the data (a column that repeats its values is grouped by, numbers are totaled, the lines are counted), and the one chosen is made with
    // its fields placed, on a sheet of its own, as one step of Undo.
    void recommendedPivotTablesFollowTheShapeOfTheData()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Region", "B1=Month", "C1=Amount", "A2=East", "B2=Jan", "C2=10", "A3=East", "B3=Jan", "C3=5", "A4=East", "B4=Feb", "C4=20", "A5=West", "B5=Jan", "C5=7",
                                "A6=West", "B6=Feb", "C6=3", "A7=West", "B7=Feb", "C7=4"})));
        crunch::EditSession *s = e.win.w.session();
        QString why;
        const QVector<crunch::PivotAdvice> advice = s->recommendPivots(QStringLiteral("Sheet1!A1:C7"), &why);
        QVERIFY2(advice.size() >= 3, qPrintable(why));
        QCOMPARE(advice[0].title, QStringLiteral("Sum of Amount by Region"));
        QCOMPARE(advice[0].rowField, 0);
        QCOMPARE(advice[0].valueField, 2);
        QVERIFY(!advice[0].count);
        QCOMPARE(advice[1].title, QStringLiteral("Count of Region"));
        QVERIFY(advice[1].count);
        int made = -1;
        QVERIFY(s->createRecommendedPivot(QStringLiteral("Sheet1!A1:C7"), advice[0], &made).ok);
        QCOMPARE(s->workbook()->pivots().tableCount(), 1);
        const jp::office::sheet::PivotTable &t = *s->workbook()->pivots().table(made).table;
        QCOMPARE(t.rowFields, QVector<int>{0});
        QCOMPARE(t.dataFields.size(), 1);
        QCOMPARE(t.dataFields[0].fld, 2);
        QCOMPARE(t.dataFields[0].subtotal, QStringLiteral("sum"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);   // (the sheet it is on is part of the step)
        pressUndo(e.view());
        QCOMPARE(s->workbook()->pivots().tableCount(), 0);
        QCOMPARE(int(s->workbook()->info().order.size()), 1);
        // from the window: the cursor in the data, and the second layout chosen
        QVector<crunch::PivotAdvice> offered;
        e.win.w.setPivotAdviceAsker([&](const QVector<crunch::PivotAdvice> &list, int *chosen) {
            offered = list;
            *chosen = 1;
            return true;
        });
        e.select(QStringLiteral("A3"));
        e.win.w.act(QStringLiteral("insert.pivotRecommended"))->trigger();
        QCOMPARE(offered.size(), advice.size());
        QCOMPARE(s->workbook()->pivots().tableCount(), 1);
        QCOMPARE(s->workbook()->pivots().table(0).table->dataFields[0].subtotal, QStringLiteral("count"));
        // data with nothing that repeats has nothing to suggest
        EditWindow plain;
        QVERIFY(plain.open(sheetOf({"A1=Id", "B1=Size", "A2=a", "B2=1", "A3=b", "B3=2"})));
        QVERIFY(plain.win.w.session()->recommendPivots(QStringLiteral("Sheet1!A1:B3"), &why).isEmpty());
        QVERIFY(!why.isEmpty());
    }

    // Ctrl+D and Ctrl+R copy the first row (column) of the selection over the rest, and a single cell takes what is above (left of) it; one step each; the keys are the platform's.
    void controlDAndControlRFillWhatIsSelected()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=5", "B1=6", "A2=x"})));
        QAction *down = e.win.w.act(QStringLiteral("fill.down")), *right = e.win.w.act(QStringLiteral("fill.right"));
        QVERIFY(down && right && down->isEnabled() && right->isEnabled());
        QCOMPARE(down->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_D).toString(QKeySequence::NativeText));
        QCOMPARE(right->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_R).toString(QKeySequence::NativeText));
        e.view()->selectRange(0, 0, 3, 1);                                      // A1:B4
        const QKeyCombination d = down->shortcut()[0];
        QTest::keyClick(e.view(), d.key(), d.keyboardModifiers());
        QCOMPARE(shown(e, QStringLiteral("A1:B4")), QStringLiteral("5|6|5|6|5|6|5|6"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.select(QStringLiteral("D1"));                                          // a single cell takes the one above it (here none: nothing)
        down->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.select(QStringLiteral("A6"));
        down->trigger();                                                         // (A5 is empty: A6 is emptied of nothing)
        e.select(QStringLiteral("A5"));
        down->trigger();                                                         // A5 takes A4's 5
        QCOMPARE(shown(e, QStringLiteral("A5")), QStringLiteral("5"));
        e.view()->selectRange(0, 0, 0, 3);                                       // A1:D1
        const QKeyCombination r = right->shortcut()[0];
        QTest::keyClick(e.view(), r.key(), r.keyboardModifiers());
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("5|5|5|5"));
        e.select(QStringLiteral("E2"));                                          // a single cell takes the one to its left (D2: empty)
        right->trigger();
        QCOMPARE(shown(e, QStringLiteral("D2:E2")), QStringLiteral("|"));
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("5|6||"));
        // up and left copy from the last row and column
        e.view()->selectRange(5, 0, 7, 0);                                       // A6:A8
        e.win.w.act(QStringLiteral("fill.up"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("A6:A8")), QStringLiteral("||"));
        e.select(QStringLiteral("B8"));
        e.win.w.act(QStringLiteral("fill.left"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("A8:B8")), QStringLiteral("|"));
    }

    // The fill handle: a small square at the corner of the selection; dragging it along a column (a row) and letting go fills the series the block makes, or a copy with Ctrl; the
    // selection is then the block and what was filled.
    void theFillHandleFillsASeries()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "C1=Item 1", "E1=10", "E2=20"})));
        e.win.w.resize(1100, 640);
        e.view()->selectRange(0, 0, 1, 0);                                       // A1:A2
        QRect handle = e.view()->fillHandleRect();
        QVERIFY(!handle.isEmpty());
        QSignalSpy requested(e.view(), &grid::GridView::fillRequested);
        auto drag = [&](const QString &to, Qt::KeyboardModifiers modifiers) {
            QRect h = e.view()->fillHandleRect();
            QTest::mousePress(e.view()->viewport(), Qt::LeftButton, modifiers, h.center());
            quint32 row = 0, col = 0;
            parseCellRef(to, &row, &col);
            const QPoint at = e.view()->cellRect(row, col).center();
            QMouseEvent move(QEvent::MouseMove, QPointF(at), QPointF(e.view()->viewport()->mapToGlobal(at)), Qt::NoButton, Qt::LeftButton, modifiers);
            QApplication::sendEvent(e.view()->viewport(), &move);
            QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, modifiers, at);
        };
        drag(QStringLiteral("A5"), Qt::NoModifier);
        QCOMPARE(requested.count(), 1);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("1|2|3|4|5"));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:A5"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // a single text that ends in a number counts up; with Ctrl it is copied
        e.select(QStringLiteral("C1"));
        drag(QStringLiteral("C3"), Qt::NoModifier);
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("Item 1|Item 2|Item 3"));
        e.select(QStringLiteral("C1"));
        drag(QStringLiteral("D1"), Qt::NoModifier);                              // (to the right: along the row)
        QCOMPARE(shown(e, QStringLiteral("C1:D1")), QStringLiteral("Item 1|Item 2"));
        e.view()->selectRange(0, 4, 1, 4);                                       // E1:E2
        drag(QStringLiteral("E4"), Qt::ControlModifier);
        QCOMPARE(shown(e, QStringLiteral("E1:E4")), QStringLiteral("10|20|10|20"));
        // a drag that does not leave the block asks for nothing; a selection of whole columns has no handle
        const int count = requested.count();
        e.select(QStringLiteral("H1"));
        drag(QStringLiteral("H1"), Qt::NoModifier);
        QCOMPARE(requested.count(), count);
        e.view()->selectColumns(0, 1);
        QVERIFY(e.view()->fillHandleRect().isEmpty());
    }

    // A double click on the fill handle fills down as far as the data beside the block goes: the column to its left, else the one to its right; one step.
    void doubleClickOnTheFillHandleFillsDownAsFarAsTheDataBeside()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1=10", "D1=7", "E2=x", "E3=y"})));
        e.win.w.resize(1100, 640);
        auto doubleClick = [&] {
            const QRect h = e.view()->fillHandleRect();
            QVERIFY(!h.isEmpty());
            QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, h.center());
            QTest::mouseDClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, h.center());
        };
        e.select(QStringLiteral("B1"));                                          // data to its left: A1:A5
        doubleClick();
        QCOMPARE(shown(e, QStringLiteral("B1:B5")), QStringLiteral("10|10|10|10|10"));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("B1:B5"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.select(QStringLiteral("D1"));                                          // nothing to its left (C is empty); E has data under row 1
        doubleClick();
        QCOMPARE(shown(e, QStringLiteral("D1:D3")), QStringLiteral("7|7|7"));
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        e.select(QStringLiteral("G1"));                                          // data on neither side: nothing happens
        doubleClick();
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        // one step of Undo takes the whole fill back
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("D1:D3")), QStringLiteral("7||"));
        QCOMPARE(e.win.w.undoStack()->index(), 1);
    }

    // The fill stops before the first cell that already holds something (it does not write over it), and a row that is hidden (or filtered out) is filled as Fill Down fills it.
    void doubleClickOnTheFillHandleStopsAtACellThatHoldsSomething()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1=10", "B4=99"})));
        e.win.w.resize(1100, 640);
        e.select(QStringLiteral("B1"));
        const QRect h = e.view()->fillHandleRect();
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, h.center());
        QTest::mouseDClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, h.center());
        QCOMPARE(shown(e, QStringLiteral("B1:B5")), QStringLiteral("10|10|10|99|"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // a hidden row inside the run is filled too, as Fill Down does
        EditWindow f;
        QVERIFY(f.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1=10"})));
        f.win.w.resize(1100, 640);
        f.view()->selectRows(1, 1);
        f.win.w.act(QStringLiteral("format.hideRows"))->trigger();
        f.select(QStringLiteral("B1"));
        const QRect g = f.view()->fillHandleRect();
        QTest::mouseClick(f.view()->viewport(), Qt::LeftButton, Qt::NoModifier, g.center());
        QTest::mouseDClick(f.view()->viewport(), Qt::LeftButton, Qt::NoModifier, g.center());
        QCOMPARE(shown(f, QStringLiteral("B1:B3")), QStringLiteral("10|10|10"));
    }

    // Dragging the border of the selection moves the block there (Ctrl copies it); the formulas that point at it follow, it is one step, and a click on the border is still a click.
    void draggingTheSelectionBorderMovesOrCopiesTheBlock()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "C1==A1+A2"})));
        e.win.w.resize(1100, 640);
        auto drag = [&](const QString &toCell, Qt::KeyboardModifiers modifiers) {
            const QRect r = e.view()->cellRect(0, 0);
            const QPoint onBorder(r.left() + r.width() / 2, r.top());           // the top edge of A1
            QTest::mousePress(e.view()->viewport(), Qt::LeftButton, modifiers, onBorder);
            quint32 row = 0, col = 0;
            parseCellRef(toCell, &row, &col);
            const QRect target = e.view()->cellRect(row, col);
            const QPoint at(target.center().x(), target.top() + (onBorder.y() - r.top()) + r.height() / 2);
            QMouseEvent move(QEvent::MouseMove, QPointF(at), QPointF(e.view()->viewport()->mapToGlobal(at)), Qt::NoButton, Qt::LeftButton, modifiers);
            QApplication::sendEvent(e.view()->viewport(), &move);
            QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, modifiers, at);
        };
        e.view()->selectRange(0, 0, 1, 0);                                       // A1:A2
        QSignalSpy moved(e.view(), &grid::GridView::moveRequested);
        drag(QStringLiteral("E1"), Qt::NoModifier);
        QCOMPARE(moved.count(), 1);
        QCOMPARE(shown(e, QStringLiteral("E1:E2")), QStringLiteral("1|2"));
        QCOMPARE(shown(e, QStringLiteral("A1:A2")), QStringLiteral("|"));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("E1:E2"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.session()->calculation()->content(0, 0, 2).formula, QStringLiteral("E1+E2"));
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("A1:A2")), QStringLiteral("1|2"));
        // with Ctrl: a copy
        e.view()->selectRange(0, 0, 1, 0);
        drag(QStringLiteral("G1"), Qt::ControlModifier);
        QCOMPARE(shown(e, QStringLiteral("A1:A2")), QStringLiteral("1|2"));
        QCOMPARE(shown(e, QStringLiteral("G1:G2")), QStringLiteral("1|2"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(&e.win.w);                                                     // one step takes the copy back
        QCOMPARE(shown(e, QStringLiteral("G1:G2")), QStringLiteral("|"));
        QCOMPARE(shown(e, QStringLiteral("A1:A2")), QStringLiteral("1|2"));
        e.view()->selectRange(0, 0, 1, 0);
        // a press and release on the border without moving is a click: it selects the cell
        e.view()->selectRange(0, 0, 1, 0);
        const int count = moved.count();
        const QRect r = e.view()->cellRect(0, 0);
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(r.left() + r.width() / 2, r.top()));
        QCOMPARE(moved.count(), count);
        QCOMPARE(e.view()->selection().range.row1, qint64(0));
        QCOMPARE(e.view()->selection().range.row2, qint64(1));                  // (a selection of several cells is not collapsed by it)
    }

    // What points at moved cells follows them when the border is dragged: a formula on another sheet, a defined name, and a conditional format.
    void draggingTheBorderKeepsReferencesFromElsewhereRight()
    {
        const QString rowsOne = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row>");
        const QByteArray one = worksheet(rowsOne, QStringLiteral("<conditionalFormatting sqref=\"A1:A2\"><cfRule type=\"cellIs\" priority=\"1\" operator=\"greaterThan\"><formula>1</formula></cfRule></conditionalFormatting>"));
        const QByteArray two = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>Sheet1!A1+Sheet1!A2</f><v>3</v></c></row>"));
        EditWindow e;
        QVERIFY(e.open(buildXlsx({one, two}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QStringLiteral("<definedName name=\"Pts\">Sheet1!$A$1:$A$2</definedName>"))));
        e.win.w.resize(1100, 640);
        e.view()->selectRange(0, 0, 1, 0);
        Q_EMIT e.view()->moveRequested(grid::GridRange{0, 0, 1, 0}, grid::GridRange{0, 2, 1, 2}, false);   // A1:A2 dropped on C1:C2
        QCOMPARE(shown(e, QStringLiteral("C1:C2")), QStringLiteral("1|2"));
        const jp::office::sheet::Calculation *calc = e.win.w.session()->calculation();
        QCOMPARE(calc->content(1, 0, 0).formula, QStringLiteral("Sheet1!C1+Sheet1!C2"));
        QString pts;
        QVERIFY(calc->nameText(-1, QStringLiteral("Pts"), &pts));
        QCOMPARE(pts.remove(QLatin1Char('=')), QStringLiteral("Sheet1!$C$1:$C$2"));
        const auto &rules = calc->sheet(0).conditionalFormats;
        QVERIFY(!rules.isEmpty());
        QCOMPARE(rangeName(rules.first().ranges.first()), QStringLiteral("C1:C2"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(&e.win.w);
        QCOMPARE(calc->content(1, 0, 0).formula, QStringLiteral("Sheet1!A1+Sheet1!A2"));
        QCOMPARE(rangeName(calc->sheet(0).conditionalFormats.first().ranges.first()), QStringLiteral("A1:A2"));
    }

    // A press just outside the selection (a couple of pixels below its border) and a drag selects as ever: nothing is moved.
    void aDragThatBeginsJustOutsideTheSelectionBorderSelects()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"B2=1", "B3=2", "B4=3"})));
        e.win.w.resize(1100, 640);
        e.select(QStringLiteral("B2"));
        QSignalSpy moved(e.view(), &grid::GridView::moveRequested);
        const QRect below = e.view()->cellRect(2, 1);                            // B3
        const QPoint press(below.center().x(), below.top() + 2);                  // (two pixels under the border of B2)
        QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, press);
        const QPoint at = e.view()->cellRect(5, 1).center();                      // B6
        QMouseEvent move(QEvent::MouseMove, QPointF(at), QPointF(e.view()->viewport()->mapToGlobal(at)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(e.view()->viewport(), &move);
        QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, at);
        QCOMPARE(moved.count(), 0);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("B3:B6"));
        QCOMPARE(shown(e, QStringLiteral("B2:B4")), QStringLiteral("1|2|3"));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
    }

    // Fill > Series asks how the series goes (kind, step, stop, unit) and fills the selection from its first row (column).
    void fillSeriesDialogFillsTheSelection()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C1=2", "D2=@2024-03-01"})));
        using Kind = crunch::EditSession::FillKind;
        CrunchWindow::SeriesRequest wanted;
        int asked = 0;
        e.win.w.setSeriesAsker([&](CrunchWindow::SeriesRequest *r) {
            ++asked;
            const bool inRows = r->inRows;
            *r = wanted;
            r->inRows = inRows;
            return true;
        });
        wanted.options.kind = Kind::Linear;
        wanted.options.hasStep = true;
        wanted.options.step = 2;
        e.view()->selectRange(0, 0, 4, 0);                                       // A1:A5
        e.win.w.act(QStringLiteral("fill.series"))->trigger();
        QCOMPARE(asked, 1);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("1|3|5|7|9"));
        wanted.options.kind = Kind::Growth;
        wanted.options.step = 3;
        e.view()->selectRange(0, 2, 0, 5);                                       // C1:F1 (a row: the series is in rows)
        e.win.w.act(QStringLiteral("fill.series"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("C1:F1")), QStringLiteral("2|6|18|54"));
        wanted = CrunchWindow::SeriesRequest();
        wanted.options.kind = Kind::Date;
        wanted.options.unit = crunch::EditSession::DateUnit::Month;
        e.view()->selectRange(1, 3, 3, 3);                                       // D2:D4
        e.win.w.act(QStringLiteral("fill.series"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("D2:D4")), QStringLiteral("3/1/2024|4/1/2024|5/1/2024"));
        // one cell selected: nothing to fill, and it says so
        const int steps = e.win.w.undoStack()->count();
        e.select(QStringLiteral("A9"));
        e.win.w.act(QStringLiteral("fill.series"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!e.win.told.isEmpty());
    }

    // The options of Fill Series: a single number counting up (or by a step), growth, a stop value, dates by weekdays and months (clamped to the month's end), and a copy.
    void fillSeriesOptions()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=7", "B1=2", "B2=4", "C1=1", "C2=2", "D1=@2024-03-01", "E1=@2024-01-31", "F1=1", "F2=2", "G1=10"})));
        using Dir = crunch::EditSession::FillDirection;
        using Kind = crunch::EditSession::FillKind;
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        crunch::EditSession::FillOptions linear;
        linear.kind = Kind::Linear;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2, linear).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("7|8|9"));
        crunch::EditSession::FillOptions stepped = linear;
        stepped.hasStep = true;
        stepped.step = 5;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("G1")), Dir::Down, 2, stepped).ok);
        QCOMPARE(shown(e, QStringLiteral("G1:G3")), QStringLiteral("10|15|20"));
        crunch::EditSession::FillOptions growth;
        growth.kind = Kind::Growth;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1:B2")), Dir::Down, 2, growth).ok);
        QCOMPARE(shown(e, QStringLiteral("B1:B4")), QStringLiteral("2|4|8|16"));
        crunch::EditSession::FillOptions stop = linear;
        stop.hasStop = true;
        stop.stop = 5;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C1:C2")), Dir::Down, 10, stop).ok);
        QCOMPARE(shown(e, QStringLiteral("C1:C7")), QStringLiteral("1|2|3|4|5||"));                          // (it stops at the stop value)
        crunch::EditSession::FillOptions weekdays;
        weekdays.kind = Kind::Date;
        weekdays.unit = crunch::EditSession::DateUnit::Weekday;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("D1")), Dir::Down, 3, weekdays).ok);                       // a Friday: then Monday, Tuesday, Wednesday
        QCOMPARE(shown(e, QStringLiteral("D1:D4")), QStringLiteral("3/1/2024|3/4/2024|3/5/2024|3/6/2024"));
        crunch::EditSession::FillOptions months;
        months.kind = Kind::Date;
        months.unit = crunch::EditSession::DateUnit::Month;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("E1")), Dir::Down, 2, months).ok);
        QCOMPARE(shown(e, QStringLiteral("E1:E3")), QStringLiteral("1/31/2024|2/29/2024|3/31/2024"));
        crunch::EditSession::FillOptions copy;
        copy.kind = Kind::Copy;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("F1:F2")), Dir::Down, 3, copy).ok);
        QCOMPARE(shown(e, QStringLiteral("F1:F5")), QStringLiteral("1|2|1|2|1"));
    }

    // A fill never goes into or out of a merged area: it would leave values in cells the area hides. The cells outside any merged area fill as always.
    void fillRefusesMergedCells()
    {
        using Dir = crunch::EditSession::FillDirection;
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c><c r=\"C1\"><v>7</v></c></row>"),
                                            QStringLiteral("<mergeCells count=\"2\"><mergeCell ref=\"A3:B3\"/><mergeCell ref=\"D1:D2\"/></mergeCells>"))})));
        crunch::EditSession *s = e.win.w.session();
        const int steps = e.win.w.undoStack()->count();
        crunch::EditSession::Outcome o = s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 3);   // A2:A4: A3 is in a merged area
        QVERIFY(!o.ok);
        QVERIFY2(o.problem.contains(QLatin1String("merged")), qPrintable(o.problem));
        o = s->fill(0, rangeOf(QStringLiteral("C1")), Dir::Right, 1);                               // D1 is in a merged area
        QVERIFY(!o.ok);
        o = s->fill(0, rangeOf(QStringLiteral("D1")), Dir::Down, 2);                                // from a merged area
        QVERIFY(!o.ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(shown(e, QStringLiteral("A2:A4")), QStringLiteral("||"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 1).ok);                        // A2 is clear of them
        QCOMPARE(shown(e, QStringLiteral("A1:A2")), QStringLiteral("5|5"));
    }

    // A fill is refused only where a merged area would be left wrong: one that straddles the edge between the cells filled from and those filled, one that is cut by either, and one in the cells
    // filled that is not the size of the area in the same place of the cells filled from. A merged area wholly inside the cells filled from fills as the values are, with or without a like area
    // where it goes.
    void fillGoesPastMergedAreasThatMatch()
    {
        using Dir = crunch::EditSession::FillDirection;
        auto book = [](const QString &merges) {
            const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>x</t></is></c><c r=\"B1\"><v>1</v></c></row><row r=\"2\"><c r=\"B2\"><v>2</v></c></row>");
            return buildXlsx({worksheet(rows, merges.isEmpty() ? QString() : QStringLiteral("<mergeCells count=\"%1\">%2</mergeCells>").arg(merges.count(QLatin1String("<mergeCell "))).arg(merges))});
        };
        auto cell = [](const char *ref) { return QStringLiteral("<mergeCell ref=\"%1\"/>").arg(QLatin1String(ref)); };
        {
            EditWindow e;   // the area in the cells filled from, and a like area where the fill goes
            QVERIFY(e.open(book(cell("A1:A2") + cell("A3:A4"))));
            QVERIFY(e.win.w.session()->fill(0, rangeOf(QStringLiteral("A1:B2")), Dir::Down, 2).ok);
            QCOMPARE(shown(e, QStringLiteral("A3:B4")), QStringLiteral("x|3||4"));   // (the numbers go on as a series)
        }
        {
            EditWindow e;   // the area in the cells filled from only
            QVERIFY(e.open(book(cell("A1:A2"))));
            QVERIFY(e.win.w.session()->fill(0, rangeOf(QStringLiteral("A1:B2")), Dir::Down, 2).ok);
            QCOMPARE(shown(e, QStringLiteral("A3:B4")), QStringLiteral("x|3||4"));   // (the numbers go on as a series)
        }
        {
            EditWindow e;   // an area where it goes that is not the size of the one it is copied from
            QVERIFY(e.open(book(cell("A1:A2") + cell("A3:B3"))));
            const auto o = e.win.w.session()->fill(0, rangeOf(QStringLiteral("A1:B2")), Dir::Down, 2);
            QVERIFY(!o.ok);
            QVERIFY(o.problem.contains(QLatin1String("merged")));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
        }
        {
            EditWindow e;   // an area that begins in the cells filled and goes on past them
            QVERIFY(e.open(book(cell("A3:A4"))));
            QVERIFY(!e.win.w.session()->fill(0, rangeOf(QStringLiteral("A1:B2")), Dir::Down, 1).ok);
        }
        {
            EditWindow e;   // an area that is cut by the cells filled from
            QVERIFY(e.open(book(cell("A2:A3"))));
            QVERIFY(!e.win.w.session()->fill(0, rangeOf(QStringLiteral("A1:B2")), Dir::Down, 1).ok);
        }
    }

    // At the edges of the sheet: a fill that is asked to go past the last row (or the first) stops there, and a merged area on the edge is judged by the cells that are filled, with no arithmetic that
    // wraps.
    void fillAtTheEdgesOfTheSheetStopsAndJudgesMergedAreas()
    {
        using Dir = crunch::EditSession::FillDirection;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row>"
                                            "<row r=\"1048575\"><c r=\"A1048575\"><v>9</v></c></row>");
        auto book = [&](const QString &merge) {
            return buildXlsx({worksheet(rows, merge.isEmpty() ? QString() : QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"%1\"/></mergeCells>").arg(merge))});
        };
        {
            EditWindow e;
            QVERIFY(e.open(book(QString())));
            crunch::EditSession *s = e.win.w.session();
            QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1048575")), Dir::Down, 4000000000u, crunch::EditSession::FillOptions()).ok);   // (one row is left)
            QCOMPARE(s->calculation()->content(0, 1048575, 0).value.number(), 9.0);
            QVERIFY(s->fill(0, rangeOf(QStringLiteral("A2")), Dir::Up, 4000000000u).ok);               // (one row is above)
            QCOMPARE(s->calculation()->content(0, 0, 0).value.number(), 2.0);
        }
        {
            EditWindow e;   // the last row is in an area wider than the cells filled from
            QVERIFY(e.open(book(QStringLiteral("A1048576:B1048576"))));
            QVERIFY(!e.win.w.session()->fill(0, rangeOf(QStringLiteral("A1048575")), Dir::Down, 4000000000u).ok);
        }
        {
            EditWindow e;
            QVERIFY(e.open(book(QStringLiteral("A1:B1"))));
            QVERIFY(!e.win.w.session()->fill(0, rangeOf(QStringLiteral("A2")), Dir::Up, 4000000000u).ok);
        }
    }

    // Across Worksheets keeps to the rules of a fill: it does not write over the header row of a table on a sheet it goes to, and it does not go into a merged area on a sheet unless the sheet it is
    // copied from has a like area in the same cells.
    void fillAcrossWorksheetsKeepsToTheRulesOfAFill()
    {
        using What = crunch::EditSession::FillWhat;
        const QString merged = QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A1:B1\"/></mergeCells>");
        {
            EditWindow e;   // a table's header row on the sheet it goes to
            QVERIFY(e.open(bookOfSheets({{"A1=5", "B1=6"}, {"A1=head1", "B1=head2", "A2=1", "B2=2"}})));
            crunch::EditSession *s = e.win.w.session();
            QVERIFY(s->makeTable(1, rangeOf(QStringLiteral("A1:B2")), true, QStringLiteral("TableStyleMedium2")).ok);
            const int steps = e.win.w.undoStack()->count();
            const auto o = s->fillAcrossSheets(0, rangeOf(QStringLiteral("A1:B1")), {1}, What::All);
            QVERIFY(!o.ok);
            QVERIFY2(o.problem.contains(QLatin1String("table")), qPrintable(o.problem));
            QCOMPARE(e.win.w.undoStack()->count(), steps);
        }
        {
            EditWindow e;   // a merged area on the sheet it goes to that the sheet it comes from does not have
            QVERIFY(e.open(bookOfSheets({{"A1=5", "B1=6"}, {"C1=1"}}, {{1, merged}})));
            const int steps = e.win.w.undoStack()->count();
            const auto o = e.win.w.session()->fillAcrossSheets(0, rangeOf(QStringLiteral("A1:B1")), {1}, What::All);
            QVERIFY(!o.ok);
            QVERIFY2(o.problem.contains(QLatin1String("merged")), qPrintable(o.problem));
            QCOMPARE(e.win.w.undoStack()->count(), steps);
        }
        {
            EditWindow e;   // the same area on both
            QVERIFY(e.open(bookOfSheets({{"A1=5", "B1=6"}, {"C1=1"}}, {{0, merged}, {1, merged}})));
            QVERIFY(e.win.w.session()->fillAcrossSheets(0, rangeOf(QStringLiteral("A1:B1")), {1}, What::All).ok);
            QCOMPARE(e.win.w.session()->calculation()->content(1, 0, 0).value.number(), 5.0);
        }
        {
            EditWindow e;   // an area on the sheet it comes from that the range cuts
            QVERIFY(e.open(bookOfSheets({{"A1=5", "B1=6"}, {"C1=1"}}, {{0, merged}})));
            QVERIFY(!e.win.w.session()->fillAcrossSheets(0, rangeOf(QStringLiteral("A1:A1")), {1}, What::All).ok);
        }
    }

    // A fill can copy just the formats, or just the contents, of the cells it goes from.
    void fillCanCopyJustTheFormatsOrJustTheContents()
    {
        using Dir = crunch::EditSession::FillDirection;
        using What = crunch::EditSession::FillWhat;
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=%0.25", "A2=7", "B1=%0.5", "C1=%0.75"})));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        crunch::EditSession::FillOptions copy;
        copy.kind = crunch::EditSession::FillKind::Copy;
        copy.what = What::FormatsOnly;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2, copy).ok);   // A2 keeps its 7, now as a percent; A3 gets the format and nothing in it
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("25%|700%|"));
        QCOMPARE(s->calculation()->content(0, 2, 0).style, s->calculation()->content(0, 0, 0).style);
        copy.what = What::ValuesOnly;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1")), Dir::Down, 2, copy).ok);   // B2 and B3 are General cells: they take 0.5 and are shown so
        QCOMPARE(shown(e, QStringLiteral("B1:B3")), QStringLiteral("50%|0.5|0.5"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C1")), Dir::Right, 1, crunch::EditSession::FillOptions()).ok);   // (and the usual copies both)
        QCOMPARE(shown(e, QStringLiteral("C1:D1")), QStringLiteral("75%|75%"));
        QCOMPARE(e.win.w.undoStack()->count(), 3);
    }

    // Justify puts the text of a column's cells into lines no wider than the width given, in the cells from the first one down; a text that needs more cells than were selected is
    // only put in when asked for.
    void justifyRefillsTheLines()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=The quick brown fox", "A2=jumps over", "A3=the lazy dog", "B1=x", "C1=5", "D1=text", "D2=more"})));
        crunch::EditSession *s = e.win.w.session();
        const auto measure = [](const QString &t) { return double(t.size()); };
        crunch::EditSession::Outcome o = s->justify(0, rangeOf(QStringLiteral("A1:A3")), 12, measure);   // four lines in three cells: more room is asked for
        QVERIFY(!o.ok && o.confirm);
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY(s->justify(0, rangeOf(QStringLiteral("A1:A3")), 12, measure, true).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A4")), QStringLiteral("The quick|brown fox|jumps over|the lazy dog"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // wider: the lines are fewer, and the cells left over are emptied
        QVERIFY(s->justify(0, rangeOf(QStringLiteral("A1:A4")), 50, measure).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A4")), QStringLiteral("The quick brown fox jumps over the lazy dog|||"));
        // a word longer than the width has a line to itself
        QVERIFY(s->justify(0, rangeOf(QStringLiteral("D1:D2")), 3, measure, true).ok);
        QCOMPARE(shown(e, QStringLiteral("D1:D3")), QStringLiteral("text|more|"));
        // numbers, and more than one column, are refused
        QVERIFY(!s->justify(0, rangeOf(QStringLiteral("C1")), 12, measure).ok);
        QVERIFY(!s->justify(0, rangeOf(QStringLiteral("A1:B1")), 12, measure).ok);
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("D1:D2")), QStringLiteral("text|more"));
    }

    // Fill > Across Worksheets copies the cells selected to the same places on the sheets chosen (everything, the contents, or the formats), as one step of Undo.
    void fillAcrossWorksheets()
    {
        using What = crunch::EditSession::FillWhat;
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=5~1", "B1=x", "A2==A1*2"}, {"A1=9", "C1=keep"}, {}})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->fillAcrossSheets(0, rangeOf(QStringLiteral("A1:B2")), {1, 2}, What::All).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        for (const int sheet : {1, 2}) {
            QCOMPARE(s->calculation()->content(sheet, 0, 0).value.number(), 5.0);
            QCOMPARE(s->calculation()->content(sheet, 0, 0).style, s->calculation()->content(0, 0, 0).style);
            QCOMPARE(s->calculation()->content(sheet, 0, 1).value.text(), QStringLiteral("x"));
            QCOMPARE(s->calculation()->content(sheet, 1, 0).formula, QStringLiteral("A1*2"));
        }
        QCOMPARE(s->calculation()->content(1, 0, 2).value.text(), QStringLiteral("keep"));   // (outside the cells: untouched)
        pressUndo(e.view());
        QVERIFY(e.settle());
        QTRY_COMPARE(s->calculation()->content(1, 0, 0).value.number(), 9.0);
        QVERIFY(!s->calculation()->content(2, 0, 1).exists);
        // just the formats: the cells keep what they hold
        QVERIFY(s->fillAcrossSheets(0, rangeOf(QStringLiteral("A1")), {1}, What::FormatsOnly).ok);
        QCOMPARE(s->calculation()->content(1, 0, 0).value.number(), 9.0);
        QCOMPARE(s->calculation()->content(1, 0, 0).style, s->calculation()->content(0, 0, 0).style);
        // just the contents: the formats stay
        QVERIFY(s->fillAcrossSheets(0, rangeOf(QStringLiteral("A1")), {2}, What::ValuesOnly).ok);
        QCOMPARE(s->calculation()->content(2, 0, 0).value.number(), 5.0);
        QCOMPARE(s->calculation()->content(2, 0, 0).style, quint32(0));
        // not the sheet it is on, a sheet that is not there, or none
        QVERIFY(!s->fillAcrossSheets(0, rangeOf(QStringLiteral("A1")), {0}, What::All).ok);
        QVERIFY(!s->fillAcrossSheets(0, rangeOf(QStringLiteral("A1")), {7}, What::All).ok);
        QVERIFY(!s->fillAcrossSheets(0, rangeOf(QStringLiteral("A1")), {}, What::All).ok);
    }

    // After the fill handle is dragged, a small button offers how the cells were filled: copied, as a series, just the formats, without the formats, and (for dates) by days, weekdays, months, or
    // years. Choosing one takes the fill back and does it again that way, so the fill is still one step; any other edit takes the button away.
    void theFillHandleOffersHowTheCellsWereFilled()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=%0.25", "C1=@2024-03-01"})));
        e.win.w.session()->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(!e.win.w.fillOptionsButton() || !e.win.w.fillOptionsButton()->isVisible());
        auto drag = [&](qint64 col, qint64 lastRow) {
            grid::GridRange source, extent;
            source.row1 = source.row2 = 0;
            source.col1 = source.col2 = col;
            extent = source;
            extent.row2 = lastRow;
            Q_EMIT e.view()->fillRequested(source, extent, false);
        };
        auto choices = [&] {
            QStringList out;
            QToolButton *b = e.win.w.fillOptionsButton();
            if (b && b->menu())
                for (QAction *a : b->menu()->actions()) out << a->text();
            return out;
        };
        auto choose = [&](const char *id) {
            QToolButton *b = e.win.w.fillOptionsButton();
            QAction *found = nullptr;
            if (b && b->menu())
                for (QAction *a : b->menu()->actions())
                    if (a->objectName() == QLatin1String(id)) found = a;
            if (!found) return false;
            found->trigger();
            return true;
        };
        drag(0, 2);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("25%|25%|25%"));
        QVERIFY(e.win.w.fillOptionsButton() && e.win.w.fillOptionsButton()->isVisible());
        QCOMPARE(choices(), (QStringList{"Copy Cells", "Fill Series", "Fill Formatting Only", "Fill Without Formatting"}));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(choose("fillOpt.valuesOnly"));
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("25%|0.25|0.25"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(choose("fillOpt.formatsOnly"));
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("25%||"));
        QCOMPARE(e.win.w.session()->calculation()->content(0, 2, 0).style, e.win.w.session()->calculation()->content(0, 0, 0).style);
        QVERIFY(choose("fillOpt.copy"));
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("25%|25%|25%"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(e.win.w.fillOptionsButton()->isVisible());
        e.select(QStringLiteral("E1"));
        e.enter(QStringLiteral("1"));
        QVERIFY(!e.win.w.fillOptionsButton()->isVisible());
        // a date: by days at first; the unit can be changed
        drag(2, 2);
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("3/1/2024|3/2/2024|3/3/2024"));
        QVERIFY(choices().contains(QStringLiteral("Fill Months")) && choices().contains(QStringLiteral("Fill Weekdays")));
        QVERIFY(choose("fillOpt.months"));
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("3/1/2024|4/1/2024|5/1/2024"));
        QVERIFY(choose("fillOpt.years"));
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("3/1/2024|3/1/2025|3/1/2026"));
        QCOMPARE(e.win.w.undoStack()->count(), 3);
        pressUndo(e.view());
        QVERIFY(!e.win.w.fillOptionsButton()->isVisible());
    }

    // Fill > Justify and Fill > Across Worksheets are in the Fill menu; Justify breaks a long text into the cells of its column to the width of the column; the dialog of Across Worksheets
    // lists the other sheets and what to copy.
    void justifyAndAcrossWorksheetsAreCommandsOfTheFillMenu()
    {
        {
            EditWindow e;
            QVERIFY(e.open(sheetOf({"A1=alpha beta gamma delta epsilon zeta eta theta iota kappa lambda", "B1=x"})));
            QVERIFY(e.win.w.act(QStringLiteral("fill.justify")) && e.win.w.act(QStringLiteral("fill.across")));
            e.select(QStringLiteral("A1"));
            e.win.w.act(QStringLiteral("fill.justify"))->trigger();
            QStringList lines;
            for (int r = 1; r <= 12; ++r)
                if (const QString t = e.shownAt(QStringLiteral("A%1").arg(r)); !t.isEmpty()) lines << t;
            QVERIFY2(lines.size() >= 3, qPrintable(lines.join(QLatin1Char('|'))));
            QCOMPARE(lines.join(QLatin1Char(' ')), QStringLiteral("alpha beta gamma delta epsilon zeta eta theta iota kappa lambda"));
            QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("x"));
            QCOMPARE(e.win.w.undoStack()->count(), 1);
            // a number cannot be justified
            e.select(QStringLiteral("D1"));
            e.type(QStringLiteral("5"));
            e.key(Qt::Key_Return);
            e.select(QStringLiteral("D1"));
            e.win.w.act(QStringLiteral("fill.justify"))->trigger();
            QVERIFY(!e.win.told.isEmpty());
        }
        {
            EditWindow e;
            QVERIFY(e.open(bookOfSheets({{"A1=5", "B1=x"}, {}, {}})));
            e.view()->selectRange(0, 0, 0, 1);
            QVERIFY(e.win.w.fillAcrossSheets({1}, crunch::EditSession::FillWhat::All));
            QCOMPARE(e.win.w.session()->calculation()->content(1, 0, 1).value.text(), QStringLiteral("x"));
            QVERIFY(!e.win.w.session()->calculation()->content(2, 0, 1).exists);
            FillAcrossDialog d(QStringList{"Sheet1", "Sheet2", "Sheet3"}, 0);
            auto *list = d.findChild<QListWidget *>(QStringLiteral("fillAcrossSheets"));
            QVERIFY(list);
            QCOMPARE(list->count(), 2);
            QCOMPARE(list->item(0)->text(), QStringLiteral("Sheet2"));
            QVERIFY(d.sheets().empty());
            list->item(1)->setCheckState(Qt::Checked);
            QCOMPARE(d.sheets(), (std::vector<int>{2}));
            auto *what = d.findChild<QComboBox *>(QStringLiteral("fillAcrossWhat"));
            QVERIFY(what);
            QCOMPARE(d.what(), crunch::EditSession::FillWhat::All);
            what->setCurrentIndex(what->findText(QStringLiteral("Formats")));
            QCOMPARE(d.what(), crunch::EditSession::FillWhat::FormatsOnly);
            what->setCurrentIndex(what->findText(QStringLiteral("Contents")));
            QCOMPARE(d.what(), crunch::EditSession::FillWhat::ValuesOnly);
        }
    }

    // Show Formulas widens the columns that hold formulas so the formulas show whole, and puts the widths back when it is turned off; it is a change of the view, not of the workbook.
    void showFormulasWidensTheColumnsThatNeedIt()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "C1==SUM(A1:A2)+SUM(A1:A2)*100+A1/A2+SUM(A1:A2)*3", "D1=7", "E1==A1"})));
        const double c = e.view()->sheet().columnWidth(2), d = e.view()->sheet().columnWidth(3), colE = e.view()->sheet().columnWidth(4);
        e.win.w.act(QStringLiteral("view.formulas"))->trigger();
        QVERIFY(e.view()->sheet().showFormulas());
        QVERIFY2(e.view()->sheet().columnWidth(2) > c + 40, qPrintable(QString::number(e.view()->sheet().columnWidth(2))));
        QCOMPARE(e.view()->sheet().columnWidth(3), d);                   // (no formula in it)
        QCOMPARE(e.view()->sheet().columnWidth(4), colE);                // (a short one fits already)
        QCOMPARE(e.view()->sheet().shown(0, 2, *e.view()->sheet().cellAt(0, 2)).text, QStringLiteral("=SUM(A1:A2)+SUM(A1:A2)*100+A1/A2+SUM(A1:A2)*3"));
        QVERIFY(!e.win.w.isModified());
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        e.win.w.act(QStringLiteral("view.formulas"))->trigger();
        QVERIFY(!e.view()->sheet().showFormulas());
        QCOMPARE(e.view()->sheet().columnWidth(2), c);
    }

    // Showing the formulas measures the cells of the columns in view, up to a limit for each, once; an edit does not measure them again, except the column of a formula that was typed. A sheet of 200
    // columns of 500 formulas is 100,000 cells and fewer than a tenth of that are measured.
    void showFormulasMeasuresOnlyTheColumnsInView()
    {
        QString rows;
        for (int r = 1; r <= 500; ++r) {
            rows += QStringLiteral("<row r=\"%1\">").arg(r);
            for (int c = 0; c < 200; ++c) rows += QStringLiteral("<c r=\"%1%2\"><f>SUM(A1:A2)+%3</f><v>1</v></c>").arg(columnName(quint32(c))).arg(r).arg(r * 7 + c);
            rows += QStringLiteral("</row>");
        }
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        e.win.w.resize(1000, 600);
        const grid::GridSheet &gs = e.view()->sheet();
        const qint64 before = gs.cellsMeasured();
        e.win.w.act(QStringLiteral("view.formulas"))->trigger();
        const qint64 turnedOn = gs.cellsMeasured() - before;
        QVERIFY2(turnedOn > 0 && turnedOn < 10000, qPrintable(QString::number(turnedOn)));
        QVERIFY(gs.columnWidth(0) > 64);
        // an edit somewhere else is not measured again
        e.select(QStringLiteral("C2"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QCOMPARE(gs.cellsMeasured() - before, turnedOn);
        // a formula typed in a column in view makes that column measured again, and no other
        e.select(QStringLiteral("B7"));
        e.enter(QStringLiteral("=SUM(A1:A2)+SUM(A1:A2)+SUM(A1:A2)+SUM(A1:A2)+SUM(A1:A2)+SUM(A1:A2)"));
        QVERIFY(e.settle());
        const qint64 after = gs.cellsMeasured() - before - turnedOn;
        QVERIFY2(after > 0 && after <= 600, qPrintable(QString::number(after)));
    }

    // While the formulas show, a column that scrolls into view for the first time is widened then, from at most 2,000 of its cells; the columns in view when they were turned on are not measured again.
    void showFormulasWidensColumnsWhenTheyFirstScrollIntoView()
    {
        QString rows;
        for (int r = 1; r <= 50; ++r) {
            rows += QStringLiteral("<row r=\"%1\">").arg(r);
            for (int c = 0; c < 200; ++c) rows += QStringLiteral("<c r=\"%1%2\"><f>SUM(A1:A2)+SUM(A1:A2)+%3</f><v>1</v></c>").arg(columnName(quint32(c))).arg(r).arg(r * 7 + c);
            rows += QStringLiteral("</row>");
        }
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        e.win.w.resize(1000, 600);
        const grid::GridSheet &gs = e.view()->sheet();
        e.win.w.act(QStringLiteral("view.formulas"))->trigger();
        const double near = gs.columnWidth(1), far = gs.columnWidth(150);
        QVERIFY(near > 64);
        QCOMPARE(far, 64.0);                                // (never shown: not measured, not widened)
        const qint64 before = gs.cellsMeasured();
        e.select(QStringLiteral("EW1"));                    // (column 152: scrolled to, with column 150 beside it)
        QVERIFY2(gs.columnWidth(150) > 64, qPrintable(QString::number(gs.columnWidth(150))));
        const qint64 measured = gs.cellsMeasured() - before;
        QVERIFY2(measured > 0 && measured <= 64 * 50, qPrintable(QString::number(measured)));
        // and back: nothing is measured again
        const qint64 mid = gs.cellsMeasured();
        e.select(QStringLiteral("A1"));
        QCOMPARE(gs.cellsMeasured(), mid);
    }

    // Zoomed out so that 100 columns or more are in view, every one of them is widened when the formulas are shown (each from its own cells, at most 2,000), and nothing is measured past the columns in view.
    void showFormulasWidensEveryColumnInView()
    {
        QString rows;
        for (int r = 1; r <= 20; ++r) {
            rows += QStringLiteral("<row r=\"%1\">").arg(r);
            for (int c = 0; c < 300; ++c) rows += QStringLiteral("<c r=\"%1%2\"><f>SUM(A1:A2)+SUM(A1:A2)+%3</f><v>1</v></c>").arg(columnName(quint32(c))).arg(r).arg(r * 7 + c);
            rows += QStringLiteral("</row>");
        }
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        e.win.w.resize(1000, 600);
        e.view()->setZoom(0.1);
        const grid::GridSheet &gs = e.view()->sheet();
        const grid::PaneView body = gs.layout().pane(grid::Pane::Body);
        const qint64 shown = body.lastCol - body.firstCol + 1;
        QVERIFY2(shown >= 100, qPrintable(QString::number(shown)));
        const qint64 before = gs.cellsMeasured();
        e.win.w.act(QStringLiteral("view.formulas"))->trigger();
        for (qint64 c = body.firstCol; c <= body.lastCol; ++c) QVERIFY2(gs.columnWidth(c) > 64, qPrintable(QString::number(c)));
        const qint64 measured = gs.cellsMeasured() - before;
        QVERIFY2(measured <= shown * 20 + 20 * 20, qPrintable(QString::number(measured)));   // (the columns in view, 20 cells each; one more turn of the layout may add a column)
    }

    // A fill that would change a cell that can't be changed (a locked cell of a protected sheet, a table's header) changes nothing and says why; a fill past the edge of the sheet stops
    // at it.
    void fillRefusesWhatCannotBeChanged()
    {
        using Dir = crunch::EditSession::FillDirection;
        {
            EditWindow e;
            const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
            const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" s=\"1\"><v>2</v></c></row><row r=\"2\"><c r=\"B2\" s=\"1\"/></row>");
            QVERIFY(e.open(buildXlsx({worksheet(rows, QStringLiteral("<sheetProtection sheet=\"1\"/>"))}, QByteArray(), QString(), QString(), styles)));
            crunch::EditSession *s = e.win.w.session();
            const crunch::EditSession::Outcome locked = s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2);   // A2 is a locked cell
            QVERIFY(!locked.ok);
            QVERIFY2(locked.problem.contains(QLatin1String("protected")), qPrintable(locked.problem));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1")), Dir::Down, 1).ok);                                   // B2 is not locked
            QCOMPARE(e.win.w.undoStack()->count(), 1);
            // the edge of the sheet: nothing past it
            QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1048576")), Dir::Down, 5).ok);
            QCOMPARE(e.win.w.undoStack()->count(), 1);
        }
        {
            EditWindow e;
            const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Region</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>Q1</t></is></c></row><row r=\"3\"><c r=\"B3\"><v>1</v></c></row>");
            const QString table = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"T\" displayName=\"T\" ref=\"A1:B3\">"
                                                 "<tableColumns count=\"2\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/></tableColumns></table>");
            QVERIFY(e.open(withTables(buildXlsx({worksheet(rows)}), {table})));
            const crunch::EditSession::Outcome o = e.win.w.session()->fill(0, rangeOf(QStringLiteral("B3")), Dir::Up, 2);   // up to B1, a header
            QVERIFY(!o.ok);
            QVERIFY2(o.problem.contains(QLatin1String("names of its columns")), qPrintable(o.problem));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
        }
    }

    // ---- AutoSum

    // AutoSum on one cell below numbers puts =SUM over the numbers above it in the cell to be edited (Enter keeps it); with none above it takes the numbers to its left; with neither it
    // gives =SUM() with the cursor inside the brackets. The keys are the platform's.
    void autoSumGuessesTheRangeOfNumbers()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        QAction *sum = e.win.w.act(QStringLiteral("edit.autosum"));
        QVERIFY(sum && sum->isEnabled());
        QCOMPARE(sum->shortcut().toString(QKeySequence::NativeText), crunch::autoSumKey().toString(QKeySequence::NativeText));
        e.select(QStringLiteral("A4"));
        sum->trigger();
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A1:A3)"));
        QCOMPARE(e.selected(), QStringLiteral("A4"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:9"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // the numbers above stop at a blank cell or a text: A5 has the blank A4 above it... now filled, so A6 sums A1:A5
        e.select(QStringLiteral("A6"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A1:A5)"));
        e.key(Qt::Key_Escape);
        // a cell with numbers to its left only
        e.select(QStringLiteral("C5"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A5:B5)"));
        e.key(Qt::Key_Escape);
        // a text above stops the guess, and the numbers to the left are taken
        e.select(QStringLiteral("C2"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A2:B2)"));
        e.key(Qt::Key_Escape);
        // nothing to guess: the brackets, with the cursor between them
        e.select(QStringLiteral("F9"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM()"));
        QCOMPARE(e.win.w.cellEditor()->cursorPosition(), 5);
        e.key(Qt::Key_Escape);
        // and the keys do it
        e.select(QStringLiteral("B4"));
        const QKeyCombination combo = crunch::autoSumKey()[0];
        QTest::keyClick(&e.win.w, combo.key(), combo.keyboardModifiers());
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(B1:B3)"));
        e.key(Qt::Key_Escape);
    }

    // The menu of AutoSum: Average, How Many Numbers, Max, and Min work the same way.
    void autoSumsMenuHasTheOtherFunctions()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        const QList<QPair<QString, QString>> functions = {{QStringLiteral("edit.autosum.average"), QStringLiteral("=AVERAGE(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.count"), QStringLiteral("=COUNT(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.max"), QStringLiteral("=MAX(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.min"), QStringLiteral("=MIN(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.sum"), QStringLiteral("=SUM(B1:B3)")}};
        for (const auto &f : functions) {
            e.select(QStringLiteral("B4"));
            QAction *a = e.win.w.act(f.first);
            QVERIFY2(a && a->isEnabled(), qPrintable(f.first));
            a->trigger();
            QVERIFY(e.win.w.isEditing());
            QCOMPARE(e.win.w.cellEditor()->text(), f.second);
            e.key(Qt::Key_Escape);
        }
        e.select(QStringLiteral("B4"));
        e.win.w.act(QStringLiteral("edit.autosum.average"))->trigger();
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:4"));
    }

    // A block that is selected gets its totals under it (a column each) in one step, or to the right when it is one row; a last row (column) of the selection that is empty is where they go.
    void autoSumOfABlockPutsTheTotalsAtItsEnd()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        e.view()->selectRange(0, 0, 2, 1);                    // A1:B3
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QVERIFY(!e.win.w.isEditing());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:9"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:12"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Sum"));
        pressUndo(&e.win.w);
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("-"));
        // a selection whose last row is empty: the totals go there
        e.view()->selectRange(0, 0, 3, 1);                    // A1:B4
        e.win.w.act(QStringLiteral("edit.autosum.max"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:5"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:6"));
        // one row: the total goes to the right
        e.view()->selectRange(4, 0, 4, 1);                    // A5:B5
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C5")), QStringLiteral("n:15"));
        // a column of text and nothing to total changes nothing
        const int steps = e.win.w.undoStack()->count();
        e.view()->selectRange(0, 2, 0, 3);                    // C1:D1 (a text, and nothing)
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }
};

JP_TEST_CLASS(CrunchOpsTests)
#include "tst_crunch_ops.moc"
