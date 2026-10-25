// Tests of the matching Find and Replace use on the text of cells: wildcards, case, whole cells, and the bounds on what a hostile text or pattern can cost.
#include "sheet/textmatch.h"

#include <QtTest>

#include "../testrunner.h"

using namespace jp::office::sheet;

namespace {

TextPattern pattern(const QString &text, bool matchCase = false, bool whole = false, bool wildcards = true)
{
    TextPattern p;
    p.text = text;
    p.matchCase = matchCase;
    p.wholeCell = whole;
    p.wildcards = wildcards;
    return p;
}

// Where the first match is, as "start+length", or "-" for none.
QString first(const TextPattern &p, const QString &text, qsizetype from = 0)
{
    qsizetype s = 0, n = 0;
    return TextMatcher(p).find(text, from, &s, &n) ? QStringLiteral("%1+%2").arg(s).arg(n) : QStringLiteral("-");
}

} // namespace

class SheetTextMatchTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void plainTextIsFoundAnywhereAndWithoutCase()
    {
        QCOMPARE(first(pattern(QStringLiteral("cat")), QStringLiteral("The Cat sat")), QStringLiteral("4+3"));
        QCOMPARE(first(pattern(QStringLiteral("cat"), true), QStringLiteral("The Cat sat")), QStringLiteral("-"));
        QCOMPARE(first(pattern(QStringLiteral("Cat"), true), QStringLiteral("The Cat sat")), QStringLiteral("4+3"));
        QCOMPARE(first(pattern(QStringLiteral("a")), QStringLiteral("banana"), 2), QStringLiteral("3+1"));
        QCOMPARE(first(pattern(QStringLiteral("a")), QStringLiteral("")), QStringLiteral("-"));
    }

    void wholeCellNeedsTheWholeText()
    {
        QCOMPARE(first(pattern(QStringLiteral("cat"), false, true), QStringLiteral("CAT")), QStringLiteral("0+3"));
        QCOMPARE(first(pattern(QStringLiteral("cat"), false, true), QStringLiteral("cats")), QStringLiteral("-"));
        QCOMPARE(first(pattern(QStringLiteral("cat"), false, true), QStringLiteral("a cat")), QStringLiteral("-"));
        QCOMPARE(first(pattern(QStringLiteral("c*t"), false, true), QStringLiteral("coat")), QStringLiteral("0+4"));
        QCOMPARE(first(pattern(QStringLiteral("c*t"), false, true), QStringLiteral("coats")), QStringLiteral("-"));
        QCOMPARE(first(pattern(QStringLiteral("a*b*c"), false, true), QStringLiteral("a--b--c")), QStringLiteral("0+7"));
        QCOMPARE(first(pattern(QStringLiteral("a*b*c"), false, true), QStringLiteral("a--c--b")), QStringLiteral("-"));
        // (the pieces must not overlap each other)
        QCOMPARE(first(pattern(QStringLiteral("ab*ba"), false, true), QStringLiteral("aba")), QStringLiteral("-"));
        QCOMPARE(first(pattern(QStringLiteral("ab*ba"), false, true), QStringLiteral("abba")), QStringLiteral("0+4"));
        // a whole-cell match is only at the start
        QCOMPARE(first(pattern(QStringLiteral("cat"), false, true), QStringLiteral("cat"), 1), QStringLiteral("-"));
    }

    void aQuestionMarkIsOneCharacterAndAStarIsAnyRun()
    {
        QCOMPARE(first(pattern(QStringLiteral("c?t")), QStringLiteral("a cot")), QStringLiteral("2+3"));
        QCOMPARE(first(pattern(QStringLiteral("c?t")), QStringLiteral("ct")), QStringLiteral("-"));
        QCOMPARE(first(pattern(QStringLiteral("c*t")), QStringLiteral("a ct")), QStringLiteral("2+2"));
        // the longest match from the leftmost place
        QCOMPARE(first(pattern(QStringLiteral("a*c")), QStringLiteral("abcxc")), QStringLiteral("0+5"));
        QCOMPARE(first(pattern(QStringLiteral("a*")), QStringLiteral("xabc")), QStringLiteral("1+3"));
        QCOMPARE(first(pattern(QStringLiteral("*c")), QStringLiteral("abcd")), QStringLiteral("0+3"));
        QCOMPARE(first(pattern(QStringLiteral("*")), QStringLiteral("abc")), QStringLiteral("0+3"));
        QCOMPARE(first(pattern(QStringLiteral("a**b")), QStringLiteral("a--b")), QStringLiteral("0+4"));
    }

    void aTildeMakesTheNextCharacterPlain()
    {
        QCOMPARE(first(pattern(QStringLiteral("a~*b")), QStringLiteral("a-b a*b")), QStringLiteral("4+3"));
        QCOMPARE(first(pattern(QStringLiteral("what~?")), QStringLiteral("what! what?")), QStringLiteral("6+5"));
        QCOMPARE(first(pattern(QStringLiteral("a~~b")), QStringLiteral("a~b")), QStringLiteral("0+3"));
        // a tilde before anything else, or at the end, is a tilde
        QCOMPARE(first(pattern(QStringLiteral("a~b")), QStringLiteral("a~b")), QStringLiteral("0+3"));
        QCOMPARE(first(pattern(QStringLiteral("a~")), QStringLiteral("a~")), QStringLiteral("0+2"));
    }

    void withoutWildcardsTheMarksArePlain()
    {
        QCOMPARE(first(pattern(QStringLiteral("a*b"), false, false, false), QStringLiteral("a--b a*b")), QStringLiteral("5+3"));
        QCOMPARE(first(pattern(QStringLiteral("a?"), false, false, false), QStringLiteral("ab a?")), QStringLiteral("3+2"));
    }

    void replacingPutsTheTextInPlaceOfEveryMatch()
    {
        int n = 0;
        QCOMPARE(TextMatcher(pattern(QStringLiteral("a"))).replaced(QStringLiteral("Banana"), QStringLiteral("o"), &n), QStringLiteral("Bonono"));
        QCOMPARE(n, 3);
        QCOMPARE(TextMatcher(pattern(QStringLiteral("a"), true)).replaced(QStringLiteral("Banana A"), QStringLiteral("xx"), &n), QStringLiteral("Bxxnxxnxx A"));
        QCOMPARE(n, 3);
        QCOMPARE(TextMatcher(pattern(QStringLiteral("z"))).replaced(QStringLiteral("Banana"), QStringLiteral("o"), &n), QStringLiteral("Banana"));
        QCOMPARE(n, 0);
        // a whole-cell match replaces the whole text
        QCOMPARE(TextMatcher(pattern(QStringLiteral("b*"), false, true)).replaced(QStringLiteral("Banana"), QStringLiteral("x"), &n), QStringLiteral("x"));
        QCOMPARE(n, 1);
        // matches do not overlap, and what is put in is not searched again
        QCOMPARE(TextMatcher(pattern(QStringLiteral("aa"))).replaced(QStringLiteral("aaaa"), QStringLiteral("a"), &n), QStringLiteral("aa"));
        QCOMPARE(n, 2);
        // the replacement has no wildcards
        QCOMPARE(TextMatcher(pattern(QStringLiteral("b"))).replaced(QStringLiteral("abc"), QStringLiteral("*?"), &n), QStringLiteral("a*?c"));
    }

    void anEmptyOrTooLongPatternIsNotValid()
    {
        QVERIFY(!TextMatcher(pattern(QString())).valid());
        QVERIFY(!TextMatcher(pattern(QString(kMaxPatternChars + 1, QLatin1Char('a')))).valid());
        QVERIFY(TextMatcher(pattern(QString(kMaxPatternChars, QLatin1Char('a')))).valid());
        QCOMPARE(first(pattern(QString()), QStringLiteral("abc")), QStringLiteral("-"));
    }

    void theWorkOnOneTextIsCountedAndBounded()
    {
        // a text of the longest length and a pattern made to take many steps: it gives up (no match) after the steps it may take, and says how many it took
        const QString text(int(kMaxMatchChars), QLatin1Char('a'));
        const QString p = QString(100, QLatin1Char('a')) + QStringLiteral("b");
        TextMatcher m(pattern(p));
        qint64 steps = 0;
        qsizetype s = 0, n = 0;
        QVERIFY(!m.find(text, 0, &s, &n, &steps));
        QVERIFY2(steps > 0 && steps <= kMaxMatchSteps + 1000, qPrintable(QString::number(steps)));
        // several stars make no more of it than the steps allow
        TextMatcher stars(pattern(QStringLiteral("*a*a*a*a*a*b")));
        steps = 0;
        QVERIFY(!stars.find(text, 0, &s, &n, &steps));
        QVERIFY(steps <= kMaxMatchSteps + 1000);
        // a text longer than kMaxMatchChars is looked at only that far
        QString longText(int(kMaxMatchChars) + 10, QLatin1Char('a'));
        longText.replace(int(kMaxMatchChars) + 2, 1, QLatin1Char('b'));
        QVERIFY(!TextMatcher(pattern(QStringLiteral("b"))).matches(longText));
    }
};

JP_TEST_CLASS(SheetTextMatchTests)
#include "tst_sheet_textmatch.moc"
