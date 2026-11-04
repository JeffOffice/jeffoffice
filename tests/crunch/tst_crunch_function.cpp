// Tests of Pick a Function (the list of functions against the engine's, the dialog, and what it puts in the formula), and of the message a rule of validation shows beside its cell.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/functionargumentsdialog.h"
#include "app/help.h"
#include "app/theme.h"
#include "app/insertfunctiondialog.h"
#include "calc/functions.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/functioncall.h"
#include "core/functioninfo.h"
#include "grid/gridview.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"

#include <QAction>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QThread>
#include <QScreen>
#include <QSignalSpy>
#include <QToolButton>
#include "../localepin.h"

#include <memory>

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
        m_localeBefore = QLocale();
        m_pin = std::make_unique<PinnedLocale>();   // (put back when the class is done)
    }

    void cleanupTestCase()
    {
        // the computer's language is back, whatever this class pinned (a class that is run after this one in the same process is not told it was English)
        m_pin.reset();
        QCOMPARE(QLocale().name(), m_localeBefore.name());
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
        QVERIFY(kind->count() == 11);
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

    // ---- Function Arguments
    void everyArgumentOfEveryFunctionHasAWordOfHelp()
    {
        int count = 0;
        const crunch::FunctionDoc *docs = crunch::functionDocs(&count);
        QStringList lacking;
        for (int i = 0; i < count; ++i) {
            const crunch::ArgumentPlan plan = crunch::argumentPlanOf(docs[i]);
            QStringList names = plan.fixed + plan.repeating;
            for (const QString &n : names)
                if (crunch::argumentHelpOf(QString::fromLatin1(docs[i].name), n).isEmpty()) lacking << QString::fromLatin1(docs[i].name) + QLatin1Char(':') + n;
        }
        QVERIFY2(lacking.isEmpty(), qPrintable(lacking.join(QLatin1Char(' '))));
        QVERIFY(crunch::argumentHelpOf(QStringLiteral("LARGE"), QStringLiteral("k")) != crunch::argumentHelpOf(QStringLiteral("PERCENTILE"), QStringLiteral("k")));
        QCOMPARE(crunch::repeatedName(QStringLiteral("number1"), 2), QStringLiteral("number3"));
        const crunch::ArgumentPlan sw = crunch::argumentPlanOf(*crunch::docOf(QStringLiteral("SWITCH")));
        QCOMPARE(sw.fixed, QStringList{"expression"});
        QCOMPARE(sw.repeating, (QStringList{"value1", "result1"}));
        const crunch::ArgumentPlan gp = crunch::argumentPlanOf(*crunch::docOf(QStringLiteral("GETPIVOTDATA")));   // (a group that may be left out repeats too)
        QCOMPARE(gp.fixed, (QStringList{"data_field", "pivot_table"}));
        QCOMPARE(gp.repeating, (QStringList{"field1", "item1"}));
    }

    void theArgumentsOfAnExistingCallAreParsedAndTheCursorPicksTheInnermostFunction()
    {
        const QString f = QStringLiteral("=SUM(A1:A3,IF(B1>2,\"a,b\",{1,2;3,4}),(1+2)*3)+5");
        auto at = [&](int cursor) { return crunch::functionCallAt(f, cursor, QLatin1Char(',')); };
        crunch::FunctionCall outer = at(6);
        QVERIFY(outer.found);
        QCOMPARE(outer.name, QStringLiteral("SUM"));
        QCOMPARE(outer.start, 1);
        QCOMPARE(outer.arguments, (QStringList{"A1:A3", "IF(B1>2,\"a,b\",{1,2;3,4})", "(1+2)*3"}));
        QCOMPARE(f.mid(outer.start, outer.end - outer.start), f.mid(1, f.size() - 3));
        const crunch::FunctionCall inner = at(int(f.indexOf(QStringLiteral("B1"))));
        QVERIFY(inner.found);
        QCOMPARE(inner.name, QStringLiteral("IF"));
        QCOMPARE(inner.arguments, (QStringList{"B1>2", "\"a,b\"", "{1,2;3,4}"}));
        // a group's brackets are no function; the call around it is
        QCOMPARE(at(int(f.indexOf(QStringLiteral("1+2")))).name, QStringLiteral("SUM"));
        QVERIFY(!at(f.size() - 1).found);                                           // (in "+5", outside every call)
        QVERIFY(!at(0).found);
        // a call that is not closed yet, and one with no arguments
        const crunch::FunctionCall open = crunch::functionCallAt(QStringLiteral("=ROUND(A1,"), 10, QLatin1Char(','));
        QVERIFY(open.found && !open.closed);
        QCOMPARE(open.arguments, (QStringList{"A1", ""}));
        QVERIFY(crunch::functionCallAt(QStringLiteral("=PI()"), 4, QLatin1Char(',')).arguments.isEmpty());
        // text that is no formula, an unknown function, and the semicolon of another locale
        QVERIFY(!crunch::functionCallAt(QStringLiteral("SUM(1)"), 4, QLatin1Char(',')).found);
        QVERIFY(!crunch::functionCallAt(QStringLiteral("=NOPE(1)"), 6, QLatin1Char(',')).found);
        QCOMPARE(crunch::functionCallAt(QStringLiteral("=SUM(1,5;2)"), 6, QLatin1Char(';')).arguments, (QStringList{"1,5", "2"}));
        QCOMPARE(crunch::callText(QStringLiteral("IF"), {"A1", "", "2", "", ""}, QLatin1Char(',')), QStringLiteral("IF(A1,,2)"));
    }

    void aGroupOfArgumentsGetsAnotherRowWhenTheLastOneIsFilled()
    {
        FunctionArgumentsDialog sum(nullptr, *crunch::docOf(QStringLiteral("SUM")), QStringList(), QLatin1Char(','));
        QCOMPARE(sum.rowCount(), 2);
        QCOMPARE(sum.argumentName(0), QStringLiteral("number1"));
        QCOMPARE(sum.argumentName(1), QStringLiteral("number2"));
        QVERIFY(!sum.isOptional(0));
        QVERIFY(sum.isOptional(1));
        sum.edit(0)->setText(QStringLiteral("A1"));
        QCOMPARE(sum.rowCount(), 2);                                                // (the last one is still empty)
        sum.edit(1)->setText(QStringLiteral("5"));
        QCOMPARE(sum.rowCount(), 3);
        QCOMPARE(sum.argumentName(2), QStringLiteral("number3"));
        sum.edit(2)->setText(QStringLiteral("7"));
        QCOMPARE(sum.rowCount(), 4);
        QCOMPARE(sum.callText(), QStringLiteral("SUM(A1,5,7)"));
        // a call that is being changed brings its arguments, and a spare row
        FunctionArgumentsDialog three(nullptr, *crunch::docOf(QStringLiteral("SUM")), {"A1", "B1", "C1"}, QLatin1Char(','));
        QCOMPARE(three.rowCount(), 4);
        QCOMPARE(three.edit(2)->text(), QStringLiteral("C1"));
        // a group of two (a value and its result) grows two rows at a time; the ones before it stay as they are
        FunctionArgumentsDialog sw(nullptr, *crunch::docOf(QStringLiteral("SWITCH")), QStringList(), QLatin1Char(','));
        QCOMPARE(sw.rowCount(), 6);                                                 // expression, value1, result1, value2, result2, and the default after them
        QCOMPARE(sw.argumentName(4), QStringLiteral("result2"));
        QCOMPARE(sw.argumentName(5), QStringLiteral("default"));
        sw.edit(3)->setText(QStringLiteral("2"));
        QCOMPARE(sw.rowCount(), 8);                                                 // (the default stays last)
        QCOMPARE(sw.argumentName(5), QStringLiteral("value3"));
        QCOMPARE(sw.argumentName(7), QStringLiteral("default"));
        // optional arguments are rows that can be left empty and are left off the call
        FunctionArgumentsDialog vl(nullptr, *crunch::docOf(QStringLiteral("VLOOKUP")), QStringList(), QLatin1Char(','));
        QCOMPARE(vl.rowCount(), 4);
        QVERIFY(!vl.isOptional(2));
        QVERIFY(vl.isOptional(3));
        vl.edit(0)->setText(QStringLiteral("\"b\""));
        vl.edit(1)->setText(QStringLiteral("A1:B3"));
        vl.edit(2)->setText(QStringLiteral("2"));
        QCOMPARE(vl.callText(), QStringLiteral("VLOOKUP(\"b\",A1:B3,2)"));
        // the semicolon of a locale that writes a comma for the point
        FunctionArgumentsDialog de(nullptr, *crunch::docOf(QStringLiteral("SUM")), {"A1", "1,5"}, QLatin1Char(';'));
        QCOMPARE(de.callText(), QStringLiteral("SUM(A1;1,5)"));
    }

    void aSwitchesOddLastArgumentIsItsDefaultInABoxOfItsOwn()
    {
        const crunch::FunctionDoc &doc = *crunch::docOf(QStringLiteral("SWITCH"));
        QCOMPARE(crunch::argumentPlanOf(doc).trailing, QStringLiteral("default"));
        QVERIFY(crunch::argumentPlanOf(*crunch::docOf(QStringLiteral("SUM"))).trailing.isEmpty());
        QVERIFY(!crunch::argumentHelpOf(QStringLiteral("SWITCH"), QStringLiteral("default")).isEmpty());
        // a call with a default: the last argument is in the "default" box, and the pair after the filled ones is empty
        FunctionArgumentsDialog withDefault(nullptr, doc, {"A1", "1", "\"one\"", "2", "\"two\"", "\"other\""}, QLatin1Char(','));
        QCOMPARE(withDefault.rowCount(), 8);                                        // expression, two pairs, a spare pair, the default
        QCOMPARE(withDefault.argumentName(7), QStringLiteral("default"));
        QCOMPARE(withDefault.edit(7)->text(), QStringLiteral("\"other\""));
        QCOMPARE(withDefault.edit(5)->text(), QString());                           // (value3 is empty, not the default)
        QVERIFY(withDefault.isOptional(7));
        QCOMPARE(withDefault.callText(), QStringLiteral("SWITCH(A1,1,\"one\",2,\"two\",\"other\")"));
        // the same with one pair
        FunctionArgumentsDialog one(nullptr, doc, {"A1", "1", "\"one\"", "\"other\""}, QLatin1Char(','));
        QCOMPARE(one.edit(3)->text(), QString());                                   // (value2)
        QCOMPARE(one.edit(one.rowCount() - 1)->text(), QStringLiteral("\"other\""));
        QCOMPARE(one.argumentName(one.rowCount() - 1), QStringLiteral("default"));
        QCOMPARE(one.callText(), QStringLiteral("SWITCH(A1,1,\"one\",\"other\")"));
        // typing another pair keeps the default last and in its place in the call
        one.edit(3)->setText(QStringLiteral("2"));
        one.edit(4)->setText(QStringLiteral("\"two\""));
        QCOMPARE(one.argumentName(one.rowCount() - 1), QStringLiteral("default"));
        QCOMPARE(one.edit(one.rowCount() - 1)->text(), QStringLiteral("\"other\""));
        QCOMPARE(one.callText(), QStringLiteral("SWITCH(A1,1,\"one\",2,\"two\",\"other\")"));
        // no default: the spare pairs are left off, and a default typed after them is not taken for a value
        FunctionArgumentsDialog none(nullptr, doc, QStringList(), QLatin1Char(','));
        none.edit(0)->setText(QStringLiteral("A1"));
        none.edit(1)->setText(QStringLiteral("1"));
        none.edit(2)->setText(QStringLiteral("5"));
        QCOMPARE(none.callText(), QStringLiteral("SWITCH(A1,1,5)"));
        none.edit(none.rowCount() - 1)->setText(QStringLiteral("0"));
        QCOMPARE(none.callText(), QStringLiteral("SWITCH(A1,1,5,0)"));
        // a call with an even number of arguments has no default
        FunctionArgumentsDialog even(nullptr, doc, {"A1", "1", "5"}, QLatin1Char(','));
        QCOMPARE(even.edit(even.rowCount() - 1)->text(), QString());
        QCOMPARE(even.edit(1)->text(), QStringLiteral("1"));
    }

    void theHintsOfReptAndSubstituteSayWhatTheirArgumentsAre()
    {
        QCOMPARE(crunch::argumentHelpOf(QStringLiteral("REPT"), QStringLiteral("number_times")), QStringLiteral("How many times to repeat the text."));
        QCOMPARE(crunch::argumentHelpOf(QStringLiteral("SUBSTITUTE"), QStringLiteral("old_text")), QStringLiteral("The text to find."));
        QCOMPARE(crunch::argumentHelpOf(QStringLiteral("REPLACE"), QStringLiteral("old_text")), QStringLiteral("The text to change."));
    }

    void withNoEntryGoingAFormulaInTheCursorCellIsWhatTheDialogOpensOn()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1==SUM(A1:A2,10)", "B2==1+IF(A1>0,5,6)", "B3=7"})));
        CrunchWindow &w = e.win.w;
        bool picked = false;
        w.setFunctionAsker([&picked](QString *name) {
            picked = true;
            *name = QStringLiteral("SQRT");
            return true;
        });
        // a formula that is a call: the dialog is on that call, with its arguments, and the picker is not shown
        QStringList seen;
        w.setArgumentsAsker([&seen](FunctionArgumentsDialog *d) {
            seen << d->functionName() << d->arguments();
            d->edit(1)->setText(QStringLiteral("20"));
            return true;
        });
        e.select(QStringLiteral("B1"));
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(!picked);
        QCOMPARE(seen, (QStringList{"SUM", "A1:A2", "10", ""}));
        QVERIFY(!w.isEditing());
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 0, 1), QStringLiteral("=SUM(A1:A2,20)"));
        // a call inside a longer formula: only that call is changed
        seen.clear();
        w.setArgumentsAsker([&seen](FunctionArgumentsDialog *d) {
            seen << d->functionName();
            d->edit(2)->setText(QStringLiteral("9"));
            return true;
        });
        e.select(QStringLiteral("B2"));
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(seen, QStringList{"IF"});
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 1, 1), QStringLiteral("=1+IF(A1>0,5,9)"));
        // given up: the formula is as it was and no entry is left going
        w.setArgumentsAsker([](FunctionArgumentsDialog *d) {
            d->edit(0)->setText(QStringLiteral("junk"));
            return false;
        });
        e.select(QStringLiteral("B1"));
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(!w.isEditing());
        QCOMPARE(w.session()->editText(0, 0, 1), QStringLiteral("=SUM(A1:A2,20)"));
        // a cell that holds a constant starts with an empty call, after the function is picked
        picked = false;
        seen.clear();
        w.setArgumentsAsker([&seen](FunctionArgumentsDialog *d) {
            seen << d->functionName() << QString::number(d->rowCount()) << d->edit(0)->text();
            return false;
        });
        e.select(QStringLiteral("B3"));
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(picked);
        QCOMPARE(seen, (QStringList{"SQRT", "1", ""}));
        QCOMPARE(w.session()->editText(0, 2, 1), QStringLiteral("7"));
    }

    void theResultAndTheValueOfEachArgumentAreWorkedOutAsTheyAreTyped()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=a", "B1=1", "A2=b", "B2=2", "A3=c", "B3=3", "D1=4", "E1=5", "F1=6"})));
        CrunchWindow &w = e.win.w;
        crunch::EditSession *session = w.session();
        const QByteArray before = QByteArray::number(qlonglong(session->undoStack()->count()));
        auto make = [&](const char *name, const QStringList &args = QStringList()) {
            auto *d = new FunctionArgumentsDialog(nullptr, *crunch::docOf(QString::fromLatin1(name)), args, QLatin1Char(','));
            d->setEvaluator([session](const QString &typed) { return session->previewFormula(typed, 0, 5, 5); });
            return std::unique_ptr<FunctionArgumentsDialog>(d);
        };
        auto sum = make("SUM");
        sum->edit(0)->setText(QStringLiteral("B1:B3"));
        QVERIFY(sum->resultLabel()->text().contains(QStringLiteral("=")));          // (nothing is worked out until typing stops)
        QVERIFY(sum->valueLabel(0)->text().isEmpty());
        QTRY_COMPARE(sum->resultLabel()->text(), QStringLiteral("Formula result = 6"));   // (the timer does it)
        QCOMPARE(sum->valueLabel(0)->text(), QStringLiteral("= {1;2;3}"));
        sum->edit(1)->setText(QStringLiteral("D1:F1"));
        sum->evaluateNow();
        QCOMPARE(sum->valueLabel(1)->text(), QStringLiteral("= {4,5,6}"));
        QCOMPARE(sum->resultLabel()->text(), QStringLiteral("Formula result = 21"));
        auto vl = make("VLOOKUP", {"\"b\"", "A1:B3", "2", "FALSE"});
        vl->evaluateNow();
        QCOMPARE(vl->resultLabel()->text(), QStringLiteral("Formula result = 2"));
        QCOMPARE(vl->valueLabel(0)->text(), QStringLiteral("= \"b\""));
        QCOMPARE(vl->valueLabel(3)->text(), QStringLiteral("= FALSE"));
        QVERIFY(vl->valueLabel(1)->text().startsWith(QStringLiteral("= {\"a\",1;\"b\",2;\"c\",3}")));
        vl->edit(0)->setText(QStringLiteral("\"zz\""));
        vl->evaluateNow();
        QVERIFY2(vl->resultLabel()->text().contains(QStringLiteral("#N/A")), qPrintable(vl->resultLabel()->text()));
        QVERIFY(vl->problemLabel()->text().contains(QStringLiteral("not found")));      // (the error in plain words)
        auto iff = make("IF", {"B1>5", "\"big\"", "\"small\""});
        iff->evaluateNow();
        QCOMPARE(iff->resultLabel()->text(), QStringLiteral("Formula result = \"small\""));
        QCOMPARE(iff->valueLabel(0)->text(), QStringLiteral("= FALSE"));
        QVERIFY(iff->problemLabel()->text().isEmpty());
        auto bad = make("SQRT", {"-1"});
        bad->evaluateNow();
        QVERIFY(bad->problemLabel()->text().contains(QStringLiteral("number")));
        auto div = make("MOD", {"5", "0"});
        div->evaluateNow();
        QVERIFY(div->problemLabel()->text().contains(QStringLiteral("zero")));
        // what is still needed is said, not an error
        auto empty = make("VLOOKUP", {"\"b\""});
        empty->evaluateNow();
        QCOMPARE(empty->resultLabel()->text(), QStringLiteral("Formula result ="));
        QVERIFY(empty->problemLabel()->text().contains(QStringLiteral("table_array")));
        // nothing was written: no step on the undo stack, no cell changed, and the cell holds what it did
        QCOMPARE(QByteArray::number(qlonglong(session->undoStack()->count())), before);
        QCOMPARE(w.session()->editText(0, 5, 5), QString());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:1"));
    }

    void aSumOverAWholeColumnIsBoundedAndTypingIsNotBlocked()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2"})));
        crunch::EditSession *session = e.win.w.session();
        const auto p = session->previewFormula(QStringLiteral("SUM(A:A)"), 0, 0, 3);
        QVERIFY(p.ok);
        QCOMPARE(p.shown, QStringLiteral("3"));
        // a formula that does too much is said so
        const auto big = session->previewFormula(QStringLiteral("SUM(A1:XFD1048576*A1:XFD1048576)"), 0, 0, 3);
        QVERIFY(!big.ok || big.error);
    }

    void okPutsTheFormulaInTheCellAndCancelChangesNothing()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("C1"));
        w.setFunctionAsker([](QString *name) {
            *name = QStringLiteral("SUM");
            return true;
        });
        // given up in the Function Arguments dialog: nothing happens
        w.setArgumentsAsker([](FunctionArgumentsDialog *d) {
            d->edit(0)->setText(QStringLiteral("A1:A3"));
            return false;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(!w.isEditing());
        QCOMPARE(w.session()->editText(0, 0, 2), QString());
        // OK puts the formula in the cell
        QString seen;
        w.setArgumentsAsker([&seen](FunctionArgumentsDialog *d) {
            seen = d->functionName() + QLatin1Char(':') + QString::number(d->rowCount());
            d->edit(0)->setText(QStringLiteral("A1:A2"));
            d->edit(1)->setText(QStringLiteral("10"));
            return true;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(seen, QStringLiteral("SUM:2"));
        QVERIFY(!w.isEditing());
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 0, 2), QStringLiteral("=SUM(A1:A2,10)"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:13"));
    }

    void aFunctionPickedWhileAFormulaIsTypedGoesInAtTheCursorAndCancelPutsTheEntryBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=4"})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("B1"));
        e.type(QStringLiteral("=1+"));
        w.setFunctionAsker([](QString *name) {
            *name = QStringLiteral("SQRT");
            return true;
        });
        w.setArgumentsAsker([](FunctionArgumentsDialog *) { return false; });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=1+"));                    // (Cancel: as it was, with the cursor where it was)
        QCOMPARE(w.cellEditor()->cursorPosition(), 3);
        w.setArgumentsAsker([](FunctionArgumentsDialog *d) {
            d->edit(0)->setText(QStringLiteral("A1"));
            return true;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=1+SQRT(A1)"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=1+SQRT(A1)"));
        QCOMPARE(w.cellEditor()->cursorPosition(), 11);
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:3"));
        // text that is no formula takes no function
        e.select(QStringLiteral("B2"));
        e.type(QStringLiteral("hello"));
        bool asked = false;
        w.setArgumentsAsker([&asked](FunctionArgumentsDialog *) {
            asked = true;
            return true;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(!asked);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("hello"));
        e.key(Qt::Key_Escape);
    }

    void withTheCursorInsideACallThatCallIsWhatTheDialogChanges()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("C1"));
        e.type(QStringLiteral("=SUM(A1,IF(A2>1,5,6),A3)"));
        QVERIFY(w.isEditing());
        QLineEdit *line = w.cellEditor();
        line->setCursorPosition(int(line->text().indexOf(QStringLiteral("A2>1"))) + 2);   // (inside the IF)
        QStringList seen;
        w.setArgumentsAsker([&seen](FunctionArgumentsDialog *d) {
            seen << d->functionName() << d->arguments();
            d->edit(1)->setText(QStringLiteral("7"));
            return true;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(seen, (QStringList{"IF", "A2>1", "5", "6"}));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(A1,IF(A2>1,7,6),A3)"));   // (the SUM around it and what follows are as they were)
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=SUM(A1,IF(A2>1,7,6),A3)"));
        QCOMPARE(w.cellEditor()->cursorPosition(), int(QStringLiteral("=SUM(A1,IF(A2>1,7,6)").size()));
        // the cursor in the SUM but outside the IF changes the SUM
        line->setCursorPosition(6);
        seen.clear();
        w.setArgumentsAsker([&seen](FunctionArgumentsDialog *d) {
            seen << d->functionName();
            d->edit(0)->setText(QStringLiteral("A2"));
            return true;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(seen, QStringList{"SUM"});
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(A2,IF(A2>1,7,6),A3)"));
        // Cancel leaves the entry exactly as it was
        line->setCursorPosition(6);
        w.setArgumentsAsker([](FunctionArgumentsDialog *d) {
            d->edit(0)->setText(QStringLiteral("junk"));
            return false;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(A2,IF(A2>1,7,6),A3)"));
        QCOMPARE(w.cellEditor()->cursorPosition(), 6);
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 0, 2), QStringLiteral("=SUM(A2,IF(A2>1,7,6),A3)"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:12"));
    }

    void cellsPointedAtOnTheSheetAreWrittenInTheBox()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("C1"));
        w.setFunctionAsker([](QString *name) {
            *name = QStringLiteral("SUM");
            return true;
        });
        w.setArgumentsAsker([&e](FunctionArgumentsDialog *d) {
            d->show();
            d->pickButton(0)->click();
            if (!d->isPicking() || d->isVisible() || !d->stripEdit()) return false;      // (the dialog gives way to the strip)
            e.view()->selectRange(0, 0, 2, 0);
            if (d->edit(0)->text() != QStringLiteral("A1:A3") || d->stripEdit()->text() != QStringLiteral("A1:A3")) return false;
            e.view()->selectCell(1, 0);
            if (d->edit(0)->text() != QStringLiteral("A2")) return false;
            // the strip closed from its frame brings the dialog back, as its button does
            d->stripEdit()->window()->close();
            if (!QTest::qWaitFor([&] { return !d->isPicking(); }, 3000) || !d->isVisible()) return false;
            d->pickButton(0)->click();
            if (!d->isPicking()) return false;
            d->endPicking();
            return !d->isPicking() && d->isVisible();
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 0, 2), QStringLiteral("=SUM(A2)"));
        QCOMPARE(e.selected(), QStringLiteral("C1"));                                   // (the cell the formula is of has the cursor again)
    }

    void theKeysWork()
    {
        FunctionArgumentsDialog d(nullptr, *crunch::docOf(QStringLiteral("VLOOKUP")), QStringList(), QLatin1Char(','));
        d.show();
        if (!QTest::qWaitForWindowActive(&d)) QSKIP("this platform does not activate windows");
        QTRY_VERIFY(d.edit(0)->hasFocus());
        QTest::keyClick(d.edit(0), Qt::Key_Tab);
        QTRY_VERIFY(d.edit(1)->hasFocus());
        QVERIFY(d.argumentHelpLabel()->text().contains(QStringLiteral("table_array")));
        QTest::keyClick(d.edit(1), Qt::Key_Tab);
        QTRY_VERIFY(d.edit(2)->hasFocus());
        QTest::keyClick(d.edit(2), Qt::Key_Tab);
        QTRY_VERIFY(d.edit(3)->hasFocus());
        QVERIFY(d.argumentHelpLabel()->text().contains(QStringLiteral("optional")));
        // screen readers: every box has a name, and the buttons and result too
        for (int i = 0; i < d.rowCount(); ++i) QVERIFY(!d.edit(i)->accessibleName().isEmpty());
        QVERIFY(!d.resultLabel()->accessibleName().isEmpty());
        QVERIFY(!d.pickButton(0)->accessibleName().isEmpty());
        QTest::keyClick(d.edit(3), Qt::Key_Return);
        QTRY_COMPARE(d.result(), int(QDialog::Accepted));
        FunctionArgumentsDialog c(nullptr, *crunch::docOf(QStringLiteral("SUM")), QStringList(), QLatin1Char(','));
        c.show();
        if (!QTest::qWaitForWindowActive(&c)) QSKIP("this platform does not activate windows");
        QTRY_VERIFY(c.edit(0)->hasFocus());
        QTest::keyClick(c.edit(0), Qt::Key_Escape);
        QTRY_COMPARE(c.result(), int(QDialog::Rejected));
    }

    void theDialogIsDrawnInLightAndDark()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=a", "B1=1", "A2=b", "B2=2", "A3=c", "B3=3"})));
        crunch::EditSession *session = e.win.w.session();
        auto draw = [&](const QString &name, const QString &last) {
            FunctionArgumentsDialog d(nullptr, *crunch::docOf(QStringLiteral("VLOOKUP")), {"\"b\"", "A1:B3", "2", last}, QLatin1Char(','));
            d.setEvaluator([session](const QString &typed) { return session->previewFormula(typed, 0, 5, 5); });
            d.setHelpTopic(QStringLiteral("function-arguments"));
            d.show();
            d.edit(2)->setFocus();
            QVERIFY(!d.helpLink()->isHidden());
            d.evaluateNow();
            QVERIFY(!d.valueLabel(1)->text().isEmpty());
            saveShot(&d, name);
        };
        draw(QStringLiteral("function-arguments-light"), QStringLiteral("FALSE"));
        draw(QStringLiteral("function-arguments-light-error"), QStringLiteral("maybe"));
        applyUiTheme(2);
        draw(QStringLiteral("function-arguments-dark"), QStringLiteral("FALSE"));
        draw(QStringLiteral("function-arguments-dark-error"), QStringLiteral("maybe"));
        applyUiTheme(1);
    }

    void aCallKeepsWhatWasWrittenInIt()
    {
        auto call = [](const QString &f) { return crunch::functionCallAt(f, int(f.indexOf(QLatin1Char('('))) + 1, QLatin1Char(',')); };
        // an empty last argument is one the person wrote: VLOOKUP(A1,B:C,2,) is an exact match, and stays so
        for (const QString &f : {QStringLiteral("=VLOOKUP(A1,B:C,2,)"), QStringLiteral("=IF(A1,\"y\",)"), QStringLiteral("=ROUND(x,)")}) {
            const crunch::FunctionCall c = call(f);
            QVERIFY2(c.found, qPrintable(f));
            FunctionArgumentsDialog d(nullptr, *crunch::docOf(c.name), c.arguments, QLatin1Char(','));
            d.setCallName(c.name);
            d.setRawArguments(c.raw);
            QCOMPARE(d.callText(), f.mid(1));
        }
        // an optional row that was not in the text is left off
        FunctionArgumentsDialog fresh(nullptr, *crunch::docOf(QStringLiteral("VLOOKUP")), {"A1", "B:C", "2"}, QLatin1Char(','));
        QCOMPARE(fresh.callText(), QStringLiteral("VLOOKUP(A1,B:C,2)"));
        // the name and the spaces are as they were typed, where nothing was changed
        const crunch::FunctionCall typed = crunch::functionCallAt(QStringLiteral("=sum( a1 , b1 )"), 6, QLatin1Char(','));
        QCOMPARE(typed.name, QStringLiteral("sum"));
        FunctionArgumentsDialog d(nullptr, *crunch::docOf(typed.name), typed.arguments, QLatin1Char(','));
        d.setCallName(typed.name);
        d.setRawArguments(typed.raw);
        QCOMPARE(d.callText(), QStringLiteral("sum( a1 , b1 )"));
        d.edit(0)->setText(QStringLiteral("C1"));
        QCOMPARE(d.callText(), QStringLiteral("sum(C1, b1 )"));
    }

    void bracketsOfTablesAreReadWithTheirOwnQuoting()
    {
        // in a table's brackets an apostrophe makes the next character a plain one, and a quote is not the beginning of a text
        const crunch::FunctionCall a = crunch::functionCallAt(QStringLiteral("=SUM(Table1[['#Items]],2)"), 6, QLatin1Char(','));
        QVERIFY(a.found);
        QCOMPARE(a.arguments, (QStringList{"Table1[['#Items]]", "2"}));
        QCOMPARE(a.end, int(QStringLiteral("=SUM(Table1[['#Items]],2)").size()));
        const crunch::FunctionCall b = crunch::functionCallAt(QStringLiteral("=SUM([Col'[1']],\"x\")"), 6, QLatin1Char(','));
        QVERIFY(b.found);
        QCOMPARE(b.arguments, (QStringList{"[Col'[1']]", "\"x\""}));
        const crunch::FunctionCall c = crunch::functionCallAt(QStringLiteral("=IF(T[[\"q]],1,2)"), 6, QLatin1Char(','));
        QCOMPARE(c.arguments, (QStringList{"T[[\"q]]", "1", "2"}));
    }

    void aTextThatIsNotClosedIsClosedBeforeTheCallIs()
    {
        const crunch::FunctionCall c = crunch::functionCallAt(QStringLiteral("=IF(A1,\"abc"), 10, QLatin1Char(','));
        QVERIFY(c.found && !c.closed);
        QCOMPARE(c.arguments, (QStringList{"A1", "\"abc\""}));
        FunctionArgumentsDialog d(nullptr, *crunch::docOf(c.name), c.arguments, QLatin1Char(','));
        d.setRawArguments(c.raw);
        QCOMPARE(d.callText(), QStringLiteral("IF(A1,\"abc\")"));
        const crunch::FunctionCall s = crunch::functionCallAt(QStringLiteral("=SUM('My sheet!A1"), 8, QLatin1Char(','));
        QCOMPARE(s.arguments, QStringList{"'My sheet!A1'"});
    }

    void theWorkerBeingBusyIsSaidAndTriedAgain()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2"})));
        crunch::EditSession *session = e.win.w.session();
        auto &loader = e.win.w.workbookLoader();
        const qint64 held = loader.holdsTaken();
        QVERIFY(session->previewFormula(QStringLiteral("SUM(A1:A2)"), 0, 5, 5).ok);
        QVERIFY(loader.holdsTaken() > held);                                          // (the cells are read with the worker held back)
        loader.refuseHolds(1000000);
        const auto p = session->previewFormula(QStringLiteral("SUM(A1:A2)"), 0, 5, 5);
        QVERIFY(!p.ok && p.busy);
        QVERIFY(p.problem.contains(QStringLiteral("Still working")));
        FunctionArgumentsDialog d(nullptr, *crunch::docOf(QStringLiteral("SUM")), {"A1:A2"}, QLatin1Char(','));
        d.setEvaluator([session](const QString &typed) { return session->previewFormula(typed, 0, 5, 5); });
        d.evaluateNow();
        QVERIFY(d.problemLabel()->text().isEmpty() || d.problemLabel()->text().contains(QStringLiteral("Still working")));
        QVERIFY(!d.resultLabel()->text().contains(QStringLiteral("3")));
        loader.refuseHolds(0);                                                        // (the worker is free again: the timer tries again)
        QTRY_COMPARE_WITH_TIMEOUT(d.resultLabel()->text(), QStringLiteral("Formula result = 3"), 5000);
    }

    void aValueThatIsNotWorkedOutIsNotShownAndEachLimitSaysItsOwnWords()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        crunch::EditSession *session = e.win.w.session();
        // a reference to another workbook is not worked out: no value is shown, and it is said in words
        const auto ext = session->previewFormula(QStringLiteral("[1]Sheet1!A1+1"), 0, 5, 5);
        QVERIFY(!ext.ok);
        QVERIFY2(ext.unsupported || ext.error, qPrintable(ext.problem));
        QVERIFY(!ext.problem.isEmpty());
        // a result too large to hold, and work that takes too many steps, are told apart
        const auto large = session->previewFormula(QStringLiteral("SUM(A1:XFD1048576*A1:XFD1048576)"), 0, 5, 5);
        QVERIFY(!large.ok || large.error);
        QVERIFY2(!large.problem.isEmpty(), "a problem is said");
        const auto slow = session->previewFormula(QStringLiteral("SUMPRODUCT(ROW(A1:A1048576)*COLUMN(A1:XFD1))"), 0, 5, 5);
        QVERIFY(!slow.ok || slow.error);
        QVERIFY(!slow.problem.isEmpty());
        QVERIFY(large.problem != slow.problem || large.tooMuch == false);
        // what changes each time is marked, so that it is not kept
        QVERIFY(session->previewFormula(QStringLiteral("RAND()"), 0, 5, 5).volatileValue);
        QVERIFY(session->previewFormula(QStringLiteral("OFFSET(A1,0,0)"), 0, 5, 5).volatileValue);
        QVERIFY(!session->previewFormula(QStringLiteral("SUM(A1)"), 0, 5, 5).volatileValue);
    }

    void openingTheDialogWorksNothingOutAndTheWorkIsDoneAFewAtATime()
    {
        int calls = 0;
        QStringList args;
        for (int i = 0; i < 60; ++i) args << QString::number(i);
        FunctionArgumentsDialog d(nullptr, *crunch::docOf(QStringLiteral("SUM")), args, QLatin1Char(','));
        d.setEvaluator([&calls](const QString &typed) {
            ++calls;
            QThread::msleep(2);
            crunch::EditSession::Preview p;
            p.ok = true;
            p.shown = typed;
            return p;
        });
        QCOMPARE(calls, 0);                                                           // (the constructor and the evaluator given worked nothing out)
        d.show();
        QCOMPARE(calls, 0);
        QTRY_VERIFY(calls > 0);
        QTRY_VERIFY(calls >= 61);                                                     // (all of it is done, on the timer: 60 boxes and the whole)
        QCOMPARE(d.valueLabel(59)->text(), QStringLiteral("= 59"));
        // typing in one box works out that box and the whole again, not the others
        const int before = calls;
        d.edit(3)->setText(QStringLiteral("x"));
        d.evaluateNow();
        QVERIFY(calls - before <= 2);
    }

    void aLongResultNeverMakesTheDialogWiderAndTheSizesFollowTheFont()
    {
        FunctionArgumentsDialog d(nullptr, *crunch::docOf(QStringLiteral("SUM")), {"A1"}, QLatin1Char(','));
        d.setEvaluator([](const QString &) {
            crunch::EditSession::Preview p;
            p.ok = true;
            p.shown = QString(600, QLatin1Char('9'));
            return p;
        });
        const int hint = d.sizeHint().width();
        d.show();
        d.evaluateNow();
        QCOMPARE(d.sizeHint().width(), hint);
        QVERIFY(d.resultLabel()->sizeHint().width() < d.width() * 2);
        QVERIFY(d.width() <= d.screen()->availableGeometry().width());
        QVERIFY(d.height() <= d.screen()->availableGeometry().height());
        // a bigger font makes a bigger dialog, not a fixed number of pixels
        QFont big = qApp->font();
        big.setPointSizeF(big.pointSizeF() * 2);
        FunctionArgumentsDialog b(nullptr, *crunch::docOf(QStringLiteral("SUM")), {"A1"}, QLatin1Char(','));
        b.setFont(big);
        FunctionArgumentsDialog n(nullptr, *crunch::docOf(QStringLiteral("SUM")), {"A1"}, QLatin1Char(','));
        QVERIFY(b.minimumWidth() >= n.minimumWidth());
    }

    void theStripStaysInTheScreenAndWhatWasKeptIsLetGoOfAfterAPick()
    {
        QWidget parent;
        const QRect room = parent.screen()->availableGeometry();
        parent.setGeometry(room.right() - 120, room.bottom() - 150, 300, 300);
        parent.show();
        int calls = 0;
        FunctionArgumentsDialog d(&parent, *crunch::docOf(QStringLiteral("SUM")), {"A1"}, QLatin1Char(','));
        d.setEvaluator([&calls](const QString &typed) {
            ++calls;
            crunch::EditSession::Preview p;
            p.ok = true;
            p.shown = typed;
            p.volatileValue = typed.contains(QStringLiteral("RAND"));
            return p;
        });
        d.evaluateNow();
        const int first = calls;
        d.setArgumentText(0, QStringLiteral("A2"));
        d.evaluateNow();
        d.setArgumentText(0, QStringLiteral("A1"));
        d.evaluateNow();
        QCOMPARE(calls, first + 2);                                                   // (A1 again: kept from before)
        d.beginPicking(0);
        QVERIFY(d.isPicking());
        const QWidget *strip = d.stripEdit()->window();
        const QRect g = strip->geometry().adjusted(2, 2, -2, -2);   // (a few pixels of play: a platform may place a window by its frame)
        QVERIFY2(g.left() >= room.left() && g.top() >= room.top() && g.right() <= room.right() && g.bottom() <= room.bottom(), qPrintable(QStringLiteral("strip %1,%2,%3,%4 in %5,%6,%7,%8").arg(g.left()).arg(g.top()).arg(g.right()).arg(g.bottom()).arg(room.left()).arg(room.top()).arg(room.right()).arg(room.bottom())));
        d.endPicking();
        const int before = calls;
        d.evaluateNow();
        QVERIFY(calls > before);                                                      // (the cells may have been different: nothing kept is trusted)
        // a value that differs each time is worked out each time
        d.setArgumentText(0, QStringLiteral("RAND()"));
        d.evaluateNow();
        const int volatileCalls = calls;
        d.setArgumentText(0, QStringLiteral("A1"));
        d.evaluateNow();
        d.setArgumentText(0, QStringLiteral("RAND()"));
        d.evaluateNow();
        QVERIFY(calls >= volatileCalls + 2);
    }

    void aCellThatCannotBeChangedIsToldOfBeforeTheDialog()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rowsOfSpecs({"A1=1"}), QStringLiteral("<sheetProtection sheet=\"1\"/>"))})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("C1"));
        w.setFunctionAsker([](QString *name) {
            *name = QStringLiteral("SUM");
            return true;
        });
        bool asked = false;
        w.setArgumentsAsker([&asked](FunctionArgumentsDialog *) {
            asked = true;
            return true;
        });
        w.setNotifier([](const QString &, const QString &) {});
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QVERIFY(!asked);
        QVERIFY(!w.isEditing());
    }

    void theEntryAndTheSelectionComeBackAfterPointingAtAnotherSheet()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rowsOfSpecs({"A1=1"})), worksheet(rowsOfSpecs({"A1=5"}))})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("C1"));
        e.type(QStringLiteral("=SUM("));
        QVERIFY(w.isEditing());
        w.cellEditor()->setCursorPosition(5);
        w.setArgumentsAsker([&e, &w](FunctionArgumentsDialog *d) {
            d->show();
            d->pickButton(0)->click();
            w.setActiveSheet(1);
            e.view()->selectCell(0, 0);
            if (d->edit(0)->text() != QStringLiteral("Sheet2!A1")) return false;
            d->endPicking();
            return true;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(w.activeSheet(), 0);
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(Sheet2!A1)"));
        QCOMPARE(e.selected(), QStringLiteral("C1"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(w.session()->editText(0, 0, 2), QStringLiteral("=SUM(Sheet2!A1)"));
        QCOMPARE(w.session()->editText(1, 0, 2), QString());                          // (not in the cell of the sheet that was looked at)
        // Cancel after pointing at E9:E12 on both sheets: the sheet and the whole selection (B2:D5, the cursor in C3) are as they were
        w.setFunctionAsker([](QString *name) {
            *name = QStringLiteral("SUM");
            return true;
        });
        e.view()->selectCell(2, 2);
        e.view()->selectRange(1, 1, 4, 3);
        grid::Selection want = e.view()->selection();
        want.activeRow = 2;
        want.activeCol = 2;
        e.view()->setSelection(want);
        const grid::Selection before = e.view()->selection();
        QCOMPARE(before.range.row1, qint64(1));
        QCOMPARE(before.range.col2, qint64(3));
        QCOMPARE(before.activeRow, qint64(2));
        w.setArgumentsAsker([&e, &w](FunctionArgumentsDialog *d) {
            d->show();
            d->pickButton(0)->click();
            e.view()->selectRange(8, 4, 11, 4);                                       // (E9:E12 of the same sheet: the selection moves)
            w.setActiveSheet(1);
            e.view()->selectRange(8, 4, 11, 4);                                       // (and on Sheet2)
            d->endPicking();
            return false;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(w.activeSheet(), 0);
        const grid::Selection after = e.view()->selection();
        QCOMPARE(after.range.row1, qint64(1));
        QCOMPARE(after.range.col1, qint64(1));
        QCOMPARE(after.range.row2, qint64(4));
        QCOMPARE(after.range.col2, qint64(3));
        QCOMPARE(after.activeRow, qint64(2));
        QCOMPARE(after.activeCol, qint64(2));
        QVERIFY(!w.isEditing());
    }

    void whileTheDialogIsOpenNothingElseChangesTheWorkbookAndHelpIsAWindowOfItsOwn()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("C1"));
        w.setFunctionAsker([](QString *name) {
            *name = QStringLiteral("SUM");
            return true;
        });
        int asked = 0;
        bool typed = true, closed = true, helpOwn = false, stillModal = false;
        w.setArgumentsAsker([&](FunctionArgumentsDialog *d) {
            ++asked;
            d->show();
            QTest::keyClick(e.view(), 'x');                                           // (typing in a cell does not begin an entry)
            typed = w.isEditing();
            w.act(QStringLiteral("formulas.insertFunction"))->trigger();               // (nor does a second dialog open)
            closed = w.close();                                                       // (closing gives up the dialog, and the window stays)
            Q_EMIT d->helpRequested(QStringLiteral("function-arguments"));
            helpOwn = d->findChild<HelpWindow *>(QString(), Qt::FindDirectChildrenOnly) != nullptr;
            stillModal = d->windowModality() != Qt::NonModal;
            return d->result() == QDialog::Accepted;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(asked, 1);
        QVERIFY(!typed);
        QVERIFY(!closed);
        QVERIFY(helpOwn);
        QVERIFY(stillModal);
        QVERIFY(!w.isEditing());
        QCOMPARE(w.session()->editText(0, 0, 2), QString());
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
private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchFunctionTests)
#include "tst_crunch_function.moc"
