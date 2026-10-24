// Tests of the rules of validation: typed and pasted values are checked against the rule of the cell (whole number, decimal, list, date, time, text length, custom formula), the rule's style
// says what happens (stop refuses, warning asks, information tells), its own title and message are used, and a list is offered from a button on the cell. Every workbook is built here.
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
#include <QClipboard>
#include <QMenu>
#include <QMimeData>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// The rules (cells are A1:J3):
//   A1:A3   whole number from 1 to 10 (stop, with its own title and message)
//   B1      decimal over 0.5 (warning)
//   C1:C3   one of Yes, No, Maybe (written in the rule)
//   D1      one of the cells F1:F3 (a, b, c), blanks allowed
//   E1      a date from 45000 on (information)
//   G1      text of at most 5 characters
//   H1:H3   custom: the value is not already in H1:H3
//   J1      whole number equal to 5, but the file asks for no alert
QByteArray rulesBook()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"F1\" t=\"inlineStr\"><is><t>a</t></is></c><c r=\"H1\"><v>7</v></c></row>"
                                        "<row r=\"2\"><c r=\"F2\" t=\"inlineStr\"><is><t>b</t></is></c></row><row r=\"3\"><c r=\"F3\" t=\"inlineStr\"><is><t>c</t></is></c></row>");
    const QString rules = QStringLiteral(
        "<dataValidations count=\"8\">"
        "<dataValidation type=\"whole\" operator=\"between\" showErrorMessage=\"1\" errorTitle=\"Count\" error=\"Whole numbers from 1 to 10 only.\" sqref=\"A1:A3\"><formula1>1</formula1><formula2>10</formula2></dataValidation>"
        "<dataValidation type=\"decimal\" operator=\"greaterThan\" errorStyle=\"warning\" showErrorMessage=\"1\" sqref=\"B1\"><formula1>0.5</formula1></dataValidation>"
        "<dataValidation type=\"list\" showErrorMessage=\"1\" sqref=\"C1:C3\"><formula1>\"Yes,No,Maybe\"</formula1></dataValidation>"
        "<dataValidation type=\"list\" allowBlank=\"1\" showErrorMessage=\"1\" sqref=\"D1\"><formula1>$F$1:$F$3</formula1></dataValidation>"
        "<dataValidation type=\"date\" operator=\"greaterThanOrEqual\" errorStyle=\"information\" showErrorMessage=\"1\" sqref=\"E1\"><formula1>45000</formula1></dataValidation>"
        "<dataValidation type=\"textLength\" operator=\"lessThanOrEqual\" showErrorMessage=\"1\" sqref=\"G1\"><formula1>5</formula1></dataValidation>"
        "<dataValidation type=\"custom\" showErrorMessage=\"1\" sqref=\"H1:H3\"><formula1>COUNTIF($H$1:$H$3,H1)=1</formula1></dataValidation>"
        "<dataValidation type=\"whole\" operator=\"equal\" sqref=\"J1\"><formula1>5</formula1></dataValidation>"
        "</dataValidations>");
    return buildXlsx({worksheet(rows, rules)});
}

} // namespace

class CrunchRulesTests : public QObject {
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

    // The rules are read from the file, in the order it has them, with their cells, kind, comparison, style, and messages.
    void theRulesAreRead()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        const QVector<office::sheet::Validation> &rules = e.win.w.calculation()->sheet(0).validations;
        QCOMPARE(rules.size(), 8);
        using V = office::sheet::Validation;
        QVERIFY(rules[0].type == V::Type::Whole && rules[0].compare == V::Compare::Between && rules[0].style == V::Style::Stop && rules[0].showError);
        QCOMPARE(rules[0].errorTitle, QStringLiteral("Count"));
        QCOMPARE(rules[0].formula2, QStringLiteral("10"));
        QVERIFY(rules[1].style == V::Style::Warning && rules[1].compare == V::Compare::Greater);
        QVERIFY(rules[2].type == V::Type::List && rules[2].formula1 == QLatin1String("\"Yes,No,Maybe\""));
        QVERIFY(rules[3].allowBlank);
        QVERIFY(rules[4].style == V::Style::Information && rules[4].type == V::Type::Date);
        QVERIFY(!rules[7].showError);
        QCOMPARE(e.win.w.calculation()->validationAt(0, 1, 0), &rules[0]);
        QVERIFY(!e.win.w.calculation()->validationAt(0, 5, 0));
    }

    // A rule that stops: the value is refused with the rule's own title and message, and the cell is as it was; a value the rule allows is entered.
    void aRuleThatStopsRefusesWhatItDoesNotAllow()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        e.select(QStringLiteral("A1"));
        e.win.told.clear();
        for (const char *bad : {"11", "0", "2.5", "words", "-3"}) {
            e.type(QString::fromLatin1(bad));
            e.key(Qt::Key_Return);
            QVERIFY2(!e.win.told.isEmpty() && e.win.told.last() == QStringLiteral("Count: Whole numbers from 1 to 10 only."), bad);
            QVERIFY(e.win.w.isEditing());
            QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("-"));
            e.win.told.clear();
            e.key(Qt::Key_Escape);
            QVERIFY(!e.win.w.isEditing());
        }
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("10"));
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:10"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("1"));
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:1"));
        // clearing is not held to the rule
        e.select(QStringLiteral("A2"));
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("-"));
        // a rule for which the file asks for no alert lets anything in
        e.select(QStringLiteral("J1"));
        e.enter(QStringLiteral("7"));
        QCOMPARE(e.valueAt(QStringLiteral("J1")), QStringLiteral("n:7"));
    }

    // A rule that warns asks whether to keep the value, and one that only informs says so and asks to go on or give up.
    void aWarningAsksAndInformationTells()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        QStringList asked;
        bool answer = false;
        e.win.w.setRuleAsker([&](const QString &title, const QString &text, int style) {
            asked << QStringLiteral("%1|%2|%3").arg(title, text).arg(style);
            return answer;
        });
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("0.2"));
        QCOMPARE(asked.size(), 1);
        QVERIFY2(asked.last().endsWith(QStringLiteral("|1")) && asked.last().startsWith(QStringLiteral("Invalid Entry|")), qPrintable(asked.last()));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("-"));                       // (answered no: the entry stays to be changed)
        QVERIFY(e.win.w.isEditing());
        e.key(Qt::Key_Escape);
        answer = true;
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("0.2"));
        QCOMPARE(asked.size(), 2);
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:0.2"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // a value that fits does not ask
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("3"));
        QCOMPARE(asked.size(), 2);
        // information: an old date is told; going on enters it
        e.select(QStringLiteral("E1"));
        e.enter(QStringLiteral("2020-01-01"));
        QCOMPARE(asked.size(), 3);
        QVERIFY(asked.last().endsWith(QStringLiteral("|2")));
        QVERIFY(e.valueAt(QStringLiteral("E1")).startsWith(QStringLiteral("n:")));
        e.select(QStringLiteral("E1"));
        e.enter(QStringLiteral("2030-01-01"));
        QCOMPARE(asked.size(), 3);
    }

    // Lists: the items of the rule, or the cells it names; a literal list is exact, a list of cells is not careful of capitals; a blank is allowed only if the rule says so.
    void listsOfItemsAndOfCells()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        e.select(QStringLiteral("C1"));
        e.enter(QStringLiteral("Yes"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:Yes"));
        for (const char *bad : {"yes", "Perhaps", "5"}) {
            e.select(QStringLiteral("C2"));
            e.type(QString::fromLatin1(bad));
            e.key(Qt::Key_Return);
            QVERIFY2(e.win.w.isEditing(), bad);
            e.key(Qt::Key_Escape);
        }
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("-"));
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("B"));
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("s:B"));
        e.select(QStringLiteral("D1"));
        e.type(QStringLiteral("z"));
        e.key(Qt::Key_Return);
        QVERIFY(e.win.w.isEditing());
        e.key(Qt::Key_Escape);
        // the choices
        const office::sheet::Calculation *calc = e.win.w.calculation();
        QCOMPARE(calc->listChoices(0, 0, 2, *calc->validationAt(0, 0, 2)), (QStringList{"Yes", "No", "Maybe"}));
        QCOMPARE(calc->listChoices(0, 0, 3, *calc->validationAt(0, 0, 3)), (QStringList{"a", "b", "c"}));
    }

    // Numbers of other kinds: a decimal, a date, a time-like number, the length of a text, and a custom formula worked out with the entry in the cell.
    void textLengthAndCustomFormulas()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        e.select(QStringLiteral("G1"));
        e.type(QStringLiteral("abcdef"));
        e.key(Qt::Key_Return);
        QVERIFY(e.win.w.isEditing());
        e.key(Qt::Key_Escape);
        e.enter(QStringLiteral("abcde"));
        QCOMPARE(e.valueAt(QStringLiteral("G1")), QStringLiteral("s:abcde"));
        // a custom rule: H1 holds 7, so 7 is not allowed again in H1:H3, and 8 is
        e.select(QStringLiteral("H2"));
        e.type(QStringLiteral("7"));
        e.key(Qt::Key_Return);
        QVERIFY(e.win.w.isEditing());
        e.key(Qt::Key_Escape);
        QCOMPARE(e.valueAt(QStringLiteral("H2")), QStringLiteral("-"));
        e.enter(QStringLiteral("8"));
        QCOMPARE(e.valueAt(QStringLiteral("H2")), QStringLiteral("n:8"));
    }

    // Only what is typed is held to the rules, as in the programs people know: a pasted value (text from another program, cells copied here, a Paste Special) goes in as it is, and no
    // rule is worked out for it (a paste of a hundred thousand cells under a rule that reads a million rows would not end).
    void pastedValuesAreNotChecked()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        QStringList asked;
        e.win.w.setRuleAsker([&](const QString &, const QString &text, int style) {
            asked << QStringLiteral("%1|%2").arg(style).arg(text);
            return true;
        });
        auto put = [&](const QString &text) {
            auto *md = new QMimeData;
            md->setText(text);
            QApplication::clipboard()->setMimeData(md);
        };
        const qint64 before = e.win.w.calculation()->ruleEvaluations();
        // three numbers into A1:A3 (a rule of whole numbers from 1 to 10), one of them over the limit: it goes in
        put(QStringLiteral("4\n40\n6\n"));
        e.select(QStringLiteral("A1"));
        e.win.told.clear();
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("4|40|6"));
        QVERIFY(e.win.told.isEmpty());
        // a value over a warning's limit, and cells with no rule
        put(QStringLiteral("0.9\n"));
        e.select(QStringLiteral("B1"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:0.9"));
        QVERIFY(asked.isEmpty());
        // copied cells, and Paste Special, the same
        e.view()->selectRange(0, 0, 0, 0);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QTRY_VERIFY(!e.win.w.isCopying());
        e.select(QStringLiteral("A5"));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        e.select(QStringLiteral("A6"));
        e.win.w.act(QStringLiteral("paste.values"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A5")), QStringLiteral("n:4"));
        QCOMPARE(e.win.w.calculation()->ruleEvaluations(), before);   // (no rule was worked out for any of it)
        // what is typed still is
        e.select(QStringLiteral("A1"));
        e.win.told.clear();
        e.enter(QStringLiteral("40"));
        QVERIFY(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("Whole numbers from 1 to 10 only.")));
        QCOMPARE(e.win.w.calculation()->ruleEvaluations(), before + 1);
    }

    // A rule is worked out with the worker held back (it reads the cells through the calculation, which the worker works in): the entry typed under a rule, and the list a button shows,
    // each take the hold.
    void aRuleIsWorkedOutWithTheWorkerHeld()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        auto &loader = e.win.w.workbookLoader();
        e.select(QStringLiteral("K1"));   // (no rule)
        qint64 holds = loader.holdsTaken();
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        const qint64 plain = loader.holdsTaken() - holds;   // (the step's own)
        e.select(QStringLiteral("A2"));   // (whole numbers from 1 to 10)
        holds = loader.holdsTaken();
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QCOMPARE(loader.holdsTaken() - holds, plain + 1);
        // the choices of a list
        e.select(QStringLiteral("C1"));
        QVERIFY2(e.win.w.calculation()->validationAt(0, 0, 2) && e.win.w.calculation()->validationAt(0, 0, 2)->type == office::sheet::Validation::Type::List, "C1 holds a list in the book");
        holds = loader.holdsTaken();
        QTest::keyClick(e.view(), Qt::Key_Down, Qt::AltModifier);
        QCOMPARE(loader.holdsTaken() - holds, qint64(1));
        for (QMenu *m : e.win.w.findChildren<QMenu *>(QStringLiteral("listMenu"))) m->close();
    }

    // A list is offered from a button on the cell: the button is at the right edge, a click (or Alt+Down) opens the choices, and the one chosen is entered; a cell with no list has no button.
    void aListHasAButtonOnTheCell()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        e.win.w.resize(1100, 640);
        e.select(QStringLiteral("A1"));
        QVERIFY(e.view()->dropDownRect().isEmpty());
        e.select(QStringLiteral("C2"));
        const QRect button = e.view()->dropDownRect();
        QVERIFY(!button.isEmpty());
        const QRect cell = e.view()->dropDownCellRect();
        QVERIFY(cell.contains(button));
        QCOMPARE(button.right() + 1, cell.right());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, button.center());
        QMenu *menu = e.win.w.findChild<QMenu *>(QStringLiteral("listMenu"));
        QVERIFY(menu);
        QStringList items;
        for (QAction *a : menu->actions()) items << a->text();
        QCOMPARE(items, (QStringList{"Yes", "No", "Maybe"}));
        menu->actions()[1]->trigger();
        menu->close();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("s:No"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // from the keys
        e.select(QStringLiteral("D1"));
        QTest::keyClick(e.view(), Qt::Key_Down, Qt::AltModifier);
        QMenu *again = e.win.w.findChild<QMenu *>(QStringLiteral("listMenu"));
        QVERIFY(again);
        QCOMPARE(again->actions().size(), 3);
        again->close();
        // a list on more than one cell: the button is on the one with the cursor; and not while it is not a list
        e.select(QStringLiteral("B1"));
        QVERIFY(e.view()->dropDownRect().isEmpty());
        e.view()->selectRange(1, 2, 2, 2);
        QVERIFY(!e.view()->dropDownRect().isEmpty());                                         // (C2:C3: the cursor cell is C2)
    }

    // The rules follow their cells when rows are inserted above them, and are put back by Undo; the saved file has them where they are.
    void rulesFollowInsertedRows()
    {
        EditWindow e;
        QVERIFY(e.open(rulesBook()));
        e.view()->selectRows(0, 0);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(e.win.w.calculation()->validationAt(0, 2, 0));
        QVERIFY(!e.win.w.calculation()->validationAt(0, 0, 0));
        e.select(QStringLiteral("A2"));
        e.type(QStringLiteral("11"));
        e.key(Qt::Key_Return);
        QVERIFY(e.win.w.isEditing());
        e.key(Qt::Key_Escape);
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(e.win.w.calculation()->validationAt(0, 0, 0));
        QVERIFY(!e.win.w.calculation()->validationAt(0, 3, 0));
        // a rule that names cells (a list from F1:F3) follows them
        e.view()->selectRows(0, 1);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QVERIFY(e.settle());
        const office::sheet::Calculation *calc = e.win.w.calculation();
        const office::sheet::Validation *rule = calc->validationAt(0, 2, 3);
        QVERIFY(rule);
        QCOMPARE(rule->formula1, QStringLiteral("$F$3:$F$5"));
        QCOMPARE(calc->listChoices(0, 2, 3, *rule), (QStringList{"a", "b", "c"}));
    }
};

JP_TEST_CLASS(CrunchRulesTests)
#include "tst_crunch_rules.moc"
