// The grammar service (libs/shell/app/grammar): a known sentence gives a known issue, spelling is left to the dictionaries, offsets are in UTF-16
// code units, and a hostile text finishes within the limits.
#include "app/grammar.h"
#include "app/settings.h"
#include "../isolatesettings.h"
#include "../settingguard.h"
#include "../testrunner.h"

#include <QElapsedTimer>
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
