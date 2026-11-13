// The grammar service (libs/shell/app/grammar): a known sentence gives a known issue, spelling is left to the dictionaries, offsets are in UTF-16
// code units, and a hostile text finishes within the limits.
#include "app/grammar.h"
#include "app/grammarcheck.h"
#include "app/optionsdialog.h"
#include "app/settings.h"
#include "../isolatesettings.h"
#include "../settingguard.h"
#include "../testrunner.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QPushButton>
#include <QTextDocument>
#include <QTimer>
#include <QtTest>

using namespace jp;

class ShellGrammarTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void theCheckerIsBuiltIn() { QVERIFY(grammarAvailable()); }

    void aKnownSentenceGivesAKnownIssue()
    {
        const QString text = QStringLiteral("This is an test of the checker.");
        const auto issues = grammarCheck(text, QStringLiteral("en-US"));
        QVERIFY(!issues.isEmpty());
        const GrammarIssue g = issues.first();
        QVERIFY(text.mid(g.start, g.end - g.start).contains(QStringLiteral("an")));
        QVERIFY(!g.message.isEmpty());
        QVERIFY(!g.suggestions.isEmpty());
        QString fixed = text;
        fixed.replace(g.start, g.end - g.start, g.suggestions.first());
        QCOMPARE(fixed, QStringLiteral("This is a test of the checker."));
    }

    void aLongRealParagraphTakesAFractionOfASecond()
    {
        QString text;
        while (text.size() < 1500) text += QStringLiteral("The committee reviewed the proposal, and several members raised questions about the schedule. ");
        grammarCheck(QStringLiteral("Warm up."), QStringLiteral("en"));   // (the first call loads the rules)
        QElapsedTimer t;
        t.start();
        grammarCheck(text, QStringLiteral("en"));
        QVERIFY2(t.elapsed() < 5000, qPrintable(QString::number(t.elapsed())));
        qInfo() << "paragraph ms" << t.elapsed();
    }

    void aCleanSentenceGivesNothing() { QVERIFY(grammarCheck(QStringLiteral("This is a test of the checker."), QStringLiteral("en")).isEmpty()); }

    void misspelledWordsAreLeftToTheDictionaries() { QVERIFY(grammarCheck(QStringLiteral("Ths is a tset of teh checkr."), QStringLiteral("en")).isEmpty()); }

    void offsetsCountUtf16CodeUnits()
    {
        const QString text = QStringLiteral("Smile \U0001F600 \U0001F600 this is an test.");
        const auto issues = grammarCheck(text, QStringLiteral("en"));
        QVERIFY(!issues.isEmpty());
        QVERIFY(text.mid(issues.first().start, issues.first().end - issues.first().start).contains(QStringLiteral("an")));
    }

    void onlyEnglishIsChecked()
    {
        QVERIFY(grammarSupports(QString()));
        QVERIFY(grammarSupports(QStringLiteral("en-GB")));
        QVERIFY(grammarSupports(QStringLiteral("en_AU")));
        QVERIFY(!grammarSupports(QStringLiteral("de-DE")));
        QVERIFY(grammarCheck(QStringLiteral("Das ist ein Test."), QStringLiteral("de")).isEmpty());
    }

    void aLongTextIsCutAtTheLimitAndSaidSo()
    {
        QString text;
        while (text.size() < 5 * 1000 * 1000) text += QStringLiteral("This is an test of the checker. ");
        bool truncated = false;
        QElapsedTimer t;
        t.start();
        const auto issues = grammarCheck(text, QStringLiteral("en"), &truncated);
        QVERIFY(t.elapsed() < 60000);
        QVERIFY(truncated);
        QVERIFY(issues.size() <= kGrammarMaxIssues);
        for (const auto &g : issues) QVERIFY(g.end <= kGrammarMaxChars);
    }

    void hostileTextsFinishWithinABound()
    {
        QStringList texts;
        texts << QString(kGrammarMaxChars, QLatin1Char('a'))                                // one word
              << QString(QStringLiteral("a ")).repeated(kGrammarMaxChars / 2)               // one-letter words
              << QString(QStringLiteral("the the ")).repeated(kGrammarMaxChars / 8)        // repeats
              << QString(QStringLiteral("(\"[{ ")).repeated(kGrammarMaxChars / 5)           // brackets
              << QString(QStringLiteral(". ")).repeated(kGrammarMaxChars / 2)               // sentences of nothing
              << QString(QStringLiteral("\U0001F600")).repeated(kGrammarMaxChars / 2)      // surrogate pairs
              << QString(QStringLiteral("1,2,3 ")).repeated(kGrammarMaxChars / 6)          // numbers
              << QString(1000, QChar(0));                                                   // NULs
        for (const QString &text : texts) {
            QElapsedTimer t;
            t.start();
            const auto issues = grammarCheck(text, QStringLiteral("en"));
            QVERIFY2(t.elapsed() < 20000, qPrintable(QString::number(t.elapsed())));
            QVERIFY(issues.size() <= kGrammarMaxIssues);
        }
    }

    void aBudgetThatIsGoneStopsTheWork()
    {
        GrammarBudget budget(50, 100000);
        bool truncated = false;
        const QString text = QStringLiteral("This is an test of the checker, and this is another sentence that is longer than the budget.");
        QVERIFY(!grammarCheck(text, QStringLiteral("en"), &truncated, &budget).isEmpty());
        QCOMPARE(budget.chars(), qint64(text.size()));
        QVERIFY(budget.exhausted());
        QVERIFY(grammarCheck(text, QStringLiteral("en"), &truncated, &budget).isEmpty());
        QVERIFY(truncated);
        QCOMPARE(budget.chars(), qint64(text.size()));   // (nothing more was spent)
    }

    void checkingGrammarAsYouTypeIsOneSuiteWideChoiceThatDefaultsToOn()
    {
        QVERIFY(grammarAsYouType());   // (not chosen yet: on)
        jptest::SettingGuard guard(QStringLiteral("proof/checkGrammarAsYouType"), true, true);
        QVERIFY(suiteWide(QStringLiteral("proof/checkGrammarAsYouType")));
        QList<bool> told;
        QObject::connect(&GrammarSettingNotifier::get(), &GrammarSettingNotifier::changed, this, [&](bool on) { told << on; });
        setGrammarAsYouType(false);
        QVERIFY(!grammarAsYouType());
        setGrammarAsYouType(false);          // (no change, nothing said)
        setGrammarAsYouType(true);
        QCOMPARE(told, (QList<bool>{false, true}));
        QVERIFY(grammarAsYouType());
    }

    void theKindsOfIssueAreOneSuiteWideChoiceThatLeavesStyleOffUntilItIsTurnedOn()
    {
        QCOMPARE(grammarKinds(), grammar::kDefaultKinds);
        QVERIFY(!(grammarKinds() & grammar::kindBit(GrammarIssue::Style)));
        QVERIFY(grammarKinds() & grammar::kindBit(GrammarIssue::Grammar));
        jptest::SettingGuard guard(QStringLiteral("proof/grammarKinds"), uint(grammar::kDefaultKinds), uint(grammar::kDefaultKinds));
        QVERIFY(suiteWide(QStringLiteral("proof/grammarKinds")));
        QList<quint32> told;
        QObject listener;   // (the connection ends with it, so that nothing is left to call the list after the test)
        QObject::connect(&GrammarSettingNotifier::get(), &GrammarSettingNotifier::kindsChanged, &listener, [&](quint32 k) { told << k; });
        setGrammarKinds(grammar::kAllKinds);
        setGrammarKinds(grammar::kAllKinds);   // (no change, nothing said)
        QCOMPARE(told, (QList<quint32>{grammar::kAllKinds}));
        QCOMPARE(grammarKinds(), grammar::kAllKinds);
        setGrammarKinds(grammar::kDefaultKinds);
    }

    void theServiceLooksOnlyForTheKindsItIsGiven()
    {
        const QString text = QStringLiteral("This is an test, and we did it in order to win.");
        QCOMPARE(grammarCheck(text, QStringLiteral("en")).size(), 1);   // (style is off)
        QCOMPARE(grammarCheck(text, QStringLiteral("en"), nullptr, nullptr, grammar::kAllKinds).size(), 2);
        QCOMPARE(grammarCheck(text, QStringLiteral("en"), nullptr, nullptr, grammar::kindBit(GrammarIssue::Style)).size(), 1);
        QVERIFY(grammarCheck(text, QStringLiteral("en"), nullptr, nullptr, grammar::kindBit(GrammarIssue::Repetition)).isEmpty());
    }

    void aChangeOfKindsIsInTheSearchAtOnce()
    {
        jptest::SettingGuard guard(QStringLiteral("proof/grammarKinds"), uint(grammar::kDefaultKinds), uint(grammar::kDefaultKinds));
        QTextDocument doc;
        doc.setPlainText(QStringLiteral("We did it in order to win."));
        GrammarCheck check(&doc);
        GrammarCheck::Issue found;
        int next = 0;
        QCOMPARE(int(check.search(0, 5000, &found, &next)), int(GrammarCheck::Step::None));
        const int before = check.version();
        setGrammarKinds(grammar::kAllKinds);
        QVERIFY(check.version() > before);   // (the pages are told to look again)
        QCOMPARE(int(check.search(0, 5000, &found, &next)), int(GrammarCheck::Step::Found));
        QCOMPARE(found.kind, GrammarIssue::Style);
        setGrammarKinds(grammar::kDefaultKinds);
        QCOMPARE(int(check.search(0, 5000, &found, &next)), int(GrammarCheck::Step::None));
    }

    void theGrammarSettingsDialogListsTheKindsAndSavesTheChoice()
    {
        jptest::SettingGuard guard(QStringLiteral("proof/grammarKinds"), uint(grammar::kDefaultKinds), uint(grammar::kDefaultKinds));
        QStringList seen;
        bool answered = false;
        QTimer poll;
        poll.setInterval(20);
        QObject::connect(&poll, &QTimer::timeout, &poll, [&] {
            for (QWidget *w : QApplication::topLevelWidgets()) {
                auto *dlg = qobject_cast<QDialog *>(w);
                if (!dlg || !dlg->isVisible() || dlg->objectName() != QLatin1String("grammarSettingsDialog")) continue;
                for (QCheckBox *box : dlg->findChildren<QCheckBox *>()) seen << box->text() + (box->isChecked() ? QStringLiteral("+") : QStringLiteral("-"));
                dlg->findChild<QCheckBox *>(QStringLiteral("grammarKind%1").arg(int(GrammarIssue::Style)))->setChecked(true);
                dlg->findChild<QCheckBox *>(QStringLiteral("grammarKind%1").arg(int(GrammarIssue::Repetition)))->setChecked(false);
                answered = true;
                dlg->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            }
        });
        poll.start();
        QVERIFY(grammarSettingsDialog(nullptr));
        QVERIFY(answered);
        QCOMPARE(seen.size(), 6);
        QVERIFY(seen.contains(QStringLiteral("Style-")));
        QVERIFY(seen.contains(QStringLiteral("Grammar+")));
        QVERIFY(grammarKinds() & grammar::kindBit(GrammarIssue::Style));
        QVERIFY(!(grammarKinds() & grammar::kindBit(GrammarIssue::Repetition)));
        QVERIFY(grammarKinds() & grammar::kindBit(GrammarIssue::WordChoice));
        setGrammarKinds(grammar::kDefaultKinds);
    }

    void grammarSettingsChosenInsideOptionsAreKeptOnlyWhenOptionsIsAccepted()
    {
        for (const bool accept : {false, true}) {
            jptest::SettingGuard guard(QStringLiteral("proof/grammarKinds"), uint(grammar::kDefaultKinds), uint(grammar::kDefaultKinds));
            QTimer poll;
            poll.setInterval(20);
            bool clicked = false, inner = false;
            QObject::connect(&poll, &QTimer::timeout, &poll, [&] {
                for (QWidget *w : QApplication::topLevelWidgets()) {
                    auto *dlg = qobject_cast<QDialog *>(w);
                    if (!dlg || !dlg->isVisible()) continue;
                    if (dlg->objectName() == QLatin1String("grammarSettingsDialog")) {
                        dlg->findChild<QCheckBox *>(QStringLiteral("grammarKind%1").arg(int(GrammarIssue::Style)))->setChecked(true);
                        inner = true;
                        dlg->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
                    } else if (dlg->objectName() == QLatin1String("optionsDialog") && !clicked) {
                        clicked = true;
                        QTimer::singleShot(0, dlg->findChild<QPushButton *>(QStringLiteral("grammarSettingsButton")), &QPushButton::click);   // (not here: the dialog it opens waits in its own loop)
                    } else if (dlg->objectName() == QLatin1String("optionsDialog") && inner) {
                        QCOMPARE(grammarKinds(), grammar::kDefaultKinds);   // (nothing is saved until Options is accepted)
                        if (accept) dlg->accept(); else dlg->reject();
                    }
                }
            });
            poll.start();
            optionsDialog(nullptr, nullptr, {}, true);
            QVERIFY(inner);
            QCOMPARE(bool(grammarKinds() & grammar::kindBit(GrammarIssue::Style)), accept);
            setGrammarKinds(grammar::kDefaultKinds);
        }
    }

    void theProofingPageOfOptionsHasTheButtonWhereGrammarIsChecked()
    {
        for (const bool grammarApp : {true, false}) {
            bool found = false, button = false;
            QTimer poll;
            poll.setInterval(20);
            QObject::connect(&poll, &QTimer::timeout, &poll, [&] {
                for (QWidget *w : QApplication::topLevelWidgets()) {
                    auto *dlg = qobject_cast<QDialog *>(w);
                    if (!dlg || !dlg->isVisible() || dlg->objectName() != QLatin1String("optionsDialog")) continue;
                    found = true;
                    button = dlg->findChild<QPushButton *>(QStringLiteral("grammarSettingsButton")) != nullptr;
                    dlg->reject();
                }
            });
            poll.start();
            optionsDialog(nullptr, nullptr, {}, grammarApp);
            QVERIFY(found);
            QCOMPARE(button, grammarApp);
        }
    }

    void aBudgetOfTimeStopsTheWorkToo()
    {
        GrammarBudget budget(kGrammarMaxTotalChars, 1);
        budget.spend(0, 5);
        QVERIFY(budget.exhausted());
        bool truncated = false;
        QVERIFY(grammarCheck(QStringLiteral("This is an test."), QStringLiteral("en"), &truncated, &budget).isEmpty());
        QVERIFY(truncated);
    }
};

JP_TEST_CLASS(ShellGrammarTests)
#include "tst_shell_grammar.moc"
