// Tests of Insert Function (the list of functions against the engine's, the dialog, and what it puts in the formula), and of the message a rule of validation shows beside its cell.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/insertfunctiondialog.h"
#include "calc/functions.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/functioninfo.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>

using namespace jp;
using namespace crunchtest;

class CrunchFunctionTests : public QObject {
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

    // ---- the list
    void everyFunctionOfTheEngineIsListedWithArgumentsThatFitItAndNoOtherIs()
    {
        int count = 0;
        const crunch::FunctionDoc *docs = crunch::functionDocs(&count);
        QSet<QString> listed;
        for (int i = 0; i < count; ++i) {
            const crunch::FunctionDoc &d = docs[i];
            const QString name = QString::fromLatin1(d.name);
            QVERIFY2(!listed.contains(name), qPrintable(name));
            listed.insert(name);
            const jp::calc::FunctionInfo *fn = jp::calc::findFunction(name);
            QVERIFY2(fn, qPrintable(name + QStringLiteral(" is not a function of the engine")));
            QCOMPARE(QString::fromLatin1(fn->name), name);
            // the arguments: those not in brackets are the fewest the engine takes, and the most is all of them, or any number if the list ends in "..."
            const QStringList args = crunch::argumentsOf(d);
            int required = 0, optional = 0;
            bool more = false;
            for (const QString &a : args) {
                if (a == QLatin1String("...")) more = true;
                else if (a.startsWith(QLatin1Char('['))) ++optional;
                else ++required;
            }
            QVERIFY2(required == fn->minArgs, qPrintable(name + QStringLiteral(": required")));
            QVERIFY2(more ? fn->maxArgs >= 254 : required + optional == fn->maxArgs, qPrintable(name + QStringLiteral(": most")));
            QVERIFY2(!crunch::summaryOf(d).isEmpty() && crunch::summaryOf(d).endsWith(QLatin1Char('.')), qPrintable(name + QStringLiteral(": summary")));
            QVERIFY2(!crunch::categoryName(d.category).isEmpty(), qPrintable(name));
            QVERIFY(crunch::signatureOf(d).startsWith(name + QLatin1Char('(')));
        }
        // every function of the engine is in the list
        QCOMPARE(count, jp::calc::functionCount());
        for (int i = 0; i < jp::calc::functionCount(); ++i) QVERIFY2(listed.contains(QString::fromLatin1(jp::calc::functionAt(i).name)), jp::calc::functionAt(i).name);
        QVERIFY(crunch::docOf(QStringLiteral("sum")));
        QVERIFY(!crunch::docOf(QStringLiteral("NOTAFUNCTION")));
        QCOMPARE(crunch::signatureOf(*crunch::docOf(QStringLiteral("VLOOKUP"))), QStringLiteral("VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])"));
        QCOMPARE(crunch::signatureOf(*crunch::docOf(QStringLiteral("PI"))), QStringLiteral("PI()"));
    }

    // ---- the dialog
    void theDialogListsByKindAndSearchesNamesAndWhatTheyDo()
    {
        InsertFunctionDialog d;
        QVERIFY(d.list()->count() == jp::calc::functionCount());
        // by kind
        QComboBox *kind = d.categoryBox();
        QVERIFY(kind->count() == 10);
        kind->setCurrentIndex(kind->findText(QStringLiteral("Text")));
        QVERIFY(d.list()->count() < jp::calc::functionCount() && d.list()->count() > 10);
        for (int i = 0; i < d.list()->count(); ++i) QCOMPARE(int(crunch::docOf(d.list()->item(i)->text())->category), int(crunch::FunctionCategory::Text));
        kind->setCurrentIndex(0);
        // a search: names that begin with it first, and then those that have it or have it in what they do
        d.searchBox()->setText(QStringLiteral("sum"));
        QCOMPARE(d.list()->item(0)->text(), QStringLiteral("SUM"));
        QStringList names;
        for (int i = 0; i < d.list()->count(); ++i) names << d.list()->item(i)->text();
        QVERIFY(names.contains(QStringLiteral("SUMIF")));
        QVERIFY(names.contains(QStringLiteral("DSUM")));
        QVERIFY(names.contains(QStringLiteral("SUBTOTAL")) || names.contains(QStringLiteral("SUMPRODUCT")));
        d.searchBox()->setText(QStringLiteral("round down"));
        QStringList found;
        for (int i = 0; i < d.list()->count(); ++i) found << d.list()->item(i)->text();
        QVERIFY(found.contains(QStringLiteral("ROUNDDOWN")));
        QVERIFY(found.contains(QStringLiteral("FLOOR")));
        d.searchBox()->setText(QStringLiteral("zzzz"));
        QCOMPARE(d.list()->count(), 0);
        QVERIFY(d.chosen().isEmpty());
        d.searchBox()->clear();
        kind->setCurrentIndex(kind->findText(QStringLiteral("Lookup & Reference")));
        d.searchBox()->setText(QStringLiteral("vlookup"));
        QCOMPARE(d.chosen(), QStringLiteral("VLOOKUP"));
    }

    void theDialogShowsTheArgumentsAndWhatTheFunctionDoes()
    {
        InsertFunctionDialog d(nullptr, QStringLiteral("vlookup"));
        QCOMPARE(d.chosen(), QStringLiteral("VLOOKUP"));
        d.show();
        saveShot(&d, QStringLiteral("insert-function"));
        QCOMPARE(d.signatureLabel()->text(), QStringLiteral("VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])"));
        QVERIFY(!d.summaryLabel()->text().isEmpty());
        QCOMPARE(d.argumentList()->count(), 4);
        QCOMPARE(d.argumentList()->item(0)->text(), QStringLiteral("lookup_value"));
        QCOMPARE(d.argumentList()->item(3)->text(), QStringLiteral("range_lookup (optional)"));
        d.searchBox()->setText(QStringLiteral("sum"));
        QCOMPARE(d.argumentList()->item(1)->text(), QStringLiteral("more of the same, as many as you need"));
        d.searchBox()->setText(QStringLiteral("pi"));
        QStringList all;
        for (int i = 0; i < d.list()->count(); ++i) all << d.list()->item(i)->text();
        d.list()->setCurrentRow(int(all.indexOf(QStringLiteral("PI"))));
        QCOMPARE(d.argumentList()->count(), 1);
        QCOMPARE(d.argumentList()->item(0)->text(), QStringLiteral("(none)"));
    }

    // ---- the formula
    void aFunctionBeginsAFormulaInTheCellWithTheCursor()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        CrunchWindow &w = e.win.w;
        QVERIFY(w.act(QStringLiteral("formulas.insertFunction"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("edit.autosum.more"))->isEnabled());
        e.select(QStringLiteral("C3"));
        w.setFunctionAsker([](QString *name) {
            *name = QStringLiteral("SUM");
            return true;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM()"));
        QCOMPARE(w.cellEditor()->cursorPosition(), 5);                               // (inside the brackets)
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=SUM()"));
        // typing the rest finishes the formula
        QTest::keyClicks(w.cellEditor(), QStringLiteral("A1:A2"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(A1:A2)"));
        QTest::keyClick(w.cellEditor(), Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 2, 2), QStringLiteral("=SUM(A1:A2)"));
    }

    void aFunctionIsPutAtTheCursorOfAFormulaBeingTyped()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=4"})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("B1"));
        e.type(QStringLiteral("=1+"));
        QVERIFY(w.isEditing());
        QVERIFY(w.insertFunction(QStringLiteral("sqrt")));                           // (any case)
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=1+SQRT()"));
        QCOMPARE(w.cellEditor()->cursorPosition(), 8);
        QTest::keyClicks(w.cellEditor(), QStringLiteral("A1"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:3"));
        // text that is not a formula takes no function; a name that is no function is refused
        e.select(QStringLiteral("B2"));
        e.type(QStringLiteral("hello"));
        QVERIFY(!w.insertFunction(QStringLiteral("SUM")));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("hello"));
        QVERIFY(!w.insertFunction(QStringLiteral("NOTAFUNCTION")));
        e.key(Qt::Key_Escape);
        // the dialog given up changes nothing
        w.setFunctionAsker([](QString *) { return false; });
        e.select(QStringLiteral("B3"));
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(!w.isEditing());
    }

    void functionCommandsAreOffWithoutAWorkbook()
    {
        Window blank;
        for (const char *id : {"formulas.insertFunction", "edit.autosum.more"}) {
            QVERIFY2(blank.w.act(QString::fromLatin1(id)), id);
            QVERIFY2(!blank.w.act(QString::fromLatin1(id))->isEnabled(), id);
        }
    }

    // ---- the message of a rule
    void aRuleShowsItsTitleAndMessageWhileItsCellIsSelected()
    {
        const QString after = QStringLiteral(
            "<dataValidations count=\"3\">"
            "<dataValidation type=\"whole\" allowBlank=\"1\" showInputMessage=\"1\" promptTitle=\"Quantity\" prompt=\"Enter a whole number.\" sqref=\"B2\"><formula1>1</formula1><formula2>9</formula2></dataValidation>"
            "<dataValidation type=\"whole\" showInputMessage=\"1\" prompt=\"Only a message.\" sqref=\"B3\"><formula1>1</formula1><formula2>9</formula2></dataValidation>"
            "<dataValidation type=\"whole\" promptTitle=\"Hidden\" prompt=\"Not shown: the rule does not ask.\" sqref=\"B4\"><formula1>1</formula1><formula2>9</formula2></dataValidation></dataValidations>"
            "<extLst><ext uri=\"{CCE6A557-97BC-48b1-807F-77E2F8E8F89B}\"><x14:dataValidations xmlns:x14=\"urn:x14\" xmlns:xm=\"urn:xm\" count=\"1\">"
            "<x14:dataValidation type=\"list\" showInputMessage=\"1\" promptTitle=\"Name\" prompt=\"Pick a name from the list on the other sheet.\"><x14:formula1><xm:f>Sheet2!$A$1:$A$3</xm:f></x14:formula1>"
            "<xm:sqref>D2</xm:sqref></x14:dataValidation></x14:dataValidations></ext></extLst>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rowsOfSpecs({"A1=x", "B1=y"}), after), worksheet(rowsOfSpecs({"A1=ann", "A2=bob", "A3=cy"}))}, QByteArray(), QString(), QString(), findStyles())));
        CrunchWindow &w = e.win.w;
        QVERIFY(!w.inputMessage());
        e.select(QStringLiteral("B2"));
        QWidget *note = w.inputMessage();
        QVERIFY(note);
        QCOMPARE(note->findChild<QLabel *>(QStringLiteral("inputMessageTitle"))->text(), QStringLiteral("Quantity"));
        QCOMPARE(note->findChild<QLabel *>(QStringLiteral("inputMessageText"))->text(), QStringLiteral("Enter a whole number."));
        QVERIFY(note->parent() == e.view()->viewport());
        // it sits beside its cell (under it, at its left)
        const QRect cell = e.view()->cellRect(1, 1);
        QCOMPARE(note->geometry().left(), cell.left());
        QVERIFY(note->geometry().top() > cell.bottom());
        // a message with no title
        e.select(QStringLiteral("B3"));
        note = w.inputMessage();
        QVERIFY(note);
        QVERIFY(!note->findChild<QLabel *>(QStringLiteral("inputMessageTitle"))->isVisibleTo(note));
        QCOMPARE(note->findChild<QLabel *>(QStringLiteral("inputMessageText"))->text(), QStringLiteral("Only a message."));
        // a rule that does not ask for a message shows none, and a cell with no rule none
        e.select(QStringLiteral("B4"));
        QVERIFY(!w.inputMessage());
        e.select(QStringLiteral("A1"));
        QVERIFY(!w.inputMessage());
        // the list from the extension list (a list on another sheet) has its message too, and its choices
        e.select(QStringLiteral("D2"));
        note = w.inputMessage();
        QVERIFY(note);
        QCOMPARE(note->findChild<QLabel *>(QStringLiteral("inputMessageTitle"))->text(), QStringLiteral("Name"));
        QVERIFY(w.calculation()->validationAt(0, 1, 3) != nullptr);
        QCOMPARE(w.calculation()->listChoices(0, 1, 3, *w.calculation()->validationAt(0, 1, 3)), (QStringList{"ann", "bob", "cy"}));
        QVERIFY(!e.view()->dropDownRect().isEmpty());                                   // (and the button for the list)
        // a block of cells selected shows none
        e.view()->selectRange(1, 3, 2, 3);
        QVERIFY(!w.inputMessage());
    }
};

JP_TEST_CLASS(CrunchFunctionTests)
#include "tst_crunch_function.moc"
