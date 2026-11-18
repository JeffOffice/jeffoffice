#pragma once
// The one script every app's comment threads are run through (tests/pub/tst_pub_threads.cpp for the pages of JeffPub and JeffDeck, and a case in JeffWrite's and JeffCrunch's review tests):
// the same twelve steps against each app's own functions must give the same observable result. An adapter says how an app does each step; threads and entries are told by their place (the
// n-th thread in document order, the n-th comment of the thread, 0 the first one):
//   bool start(text)                 a new thread after the others in document order
//   bool reply(t, e, text)           an answer to comment e of thread t
//   bool edit(t, e, text), bool resolve(t, bool done), bool remove(t, e)
//   void removeAll()                 Delete All
//   QStringList state()              per thread: its texts joined with " | ", and " [resolved]" once it is
//   QString author(t, e)
//   int step(t, bool forward)        the thread Next (Previous) reaches from thread t, -1 for none
//   int entries(t)                   how many comments the thread holds
//   int threadCount()
// The three caps (core/threads.h) are asserted at their exact values: the 200th reply is accepted and the 201st refused.

#include "core/threads.h"

#include <QTest>

namespace threadscript {

template <class Adapter>
void run(Adapter &a, const QString &who)
{
    using jp::threads::kMaxChars;
    QVERIFY(a.state().isEmpty());
    // 1 new
    QVERIFY(a.start(QStringLiteral("Q1")));
    QCOMPARE(a.state(), QStringList{QStringLiteral("Q1")});
    QCOMPARE(a.author(0, 0), who);
    // 2 reply
    QVERIFY(a.reply(0, 0, QStringLiteral("R1")));
    QCOMPARE(a.state(), QStringList{QStringLiteral("Q1 | R1")});
    QCOMPARE(a.author(0, 1), who);
    // 3 reply to a reply: into the same thread, after the others
    QVERIFY(a.reply(0, 1, QStringLiteral("R2")));
    QCOMPARE(a.state(), QStringList{QStringLiteral("Q1 | R1 | R2")});
    QCOMPARE(a.threadCount(), 1);
    // 4 edit a reply
    QVERIFY(a.edit(0, 1, QStringLiteral("R1 edited")));
    QCOMPARE(a.state(), QStringList{QStringLiteral("Q1 | R1 edited | R2")});
    // 5 resolve: the whole thread
    QVERIFY(a.resolve(0, true));
    QCOMPARE(a.state(), QStringList{QStringLiteral("Q1 | R1 edited | R2 [resolved]")});
    // 6 reopen
    QVERIFY(a.resolve(0, false));
    QCOMPARE(a.state(), QStringList{QStringLiteral("Q1 | R1 edited | R2")});
    // 7 delete a reply
    QVERIFY(a.remove(0, 2));
    QCOMPARE(a.state(), QStringList{QStringLiteral("Q1 | R1 edited")});
    // 8 delete the first comment: the thread goes
    QVERIFY(a.remove(0, 0));
    QVERIFY(a.state().isEmpty());
    // 9 next and previous in document order, and no wrap
    QVERIFY(a.start(QStringLiteral("A")));
    QVERIFY(a.start(QStringLiteral("B")));
    QVERIFY(a.start(QStringLiteral("C")));
    QCOMPARE(a.state(), (QStringList{QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}));
    QCOMPARE(a.step(0, true), 1);
    QCOMPARE(a.step(1, true), 2);
    QCOMPARE(a.step(2, true), -1);
    QCOMPARE(a.step(2, false), 1);
    QCOMPARE(a.step(1, false), 0);
    QCOMPARE(a.step(0, false), -1);
    // 10 delete all
    a.removeAll();
    QVERIFY(a.state().isEmpty());
    QVERIFY(a.state().isEmpty());
    // 11 over the reply cap: exactly kMaxReplies replies are taken, the next is refused and changes nothing
    QVERIFY(a.start(QStringLiteral("Cap")));
    int taken = 0;
    while (taken < jp::threads::kMaxReplies + 5 && a.reply(0, taken == 0 ? 0 : 1, QStringLiteral("r%1").arg(taken))) ++taken;
    QCOMPARE(taken, jp::threads::kMaxReplies);
    QCOMPARE(a.entries(0), 1 + jp::threads::kMaxReplies);
    const QStringList full = a.state();
    QVERIFY(!a.reply(0, 0, QStringLiteral("one too many")));
    QCOMPARE(a.state(), full);
    a.removeAll();
    QVERIFY(a.state().isEmpty());
    // 12 over the text cap: exactly kMaxChars are kept; one more is refused or cut (an app never keeps more)
    const QString exact(kMaxChars, QLatin1Char('x'));
    QVERIFY(a.start(exact));
    QCOMPARE(int(a.state().value(0).size()), kMaxChars);
    a.removeAll();
    QVERIFY(a.state().isEmpty());
    const QString over(kMaxChars + 1, QLatin1Char('x'));
    if (a.start(over)) QVERIFY(a.state().value(0).size() <= kMaxChars);
    else QVERIFY(a.state().isEmpty());
    a.removeAll();
    // and the cap on threads, lowered so that the test is short: the third thread and the fourth reply are refused
    jp::threads::setCapsForTest(2, 3);
    const bool one = a.start(QStringLiteral("T1")), two = a.start(QStringLiteral("T2")), three = a.start(QStringLiteral("T3"));
    int replies = 0;
    while (replies < 10 && a.reply(0, 0, QStringLiteral("x"))) ++replies;
    const int threads = a.threadCount();
    jp::threads::setCapsForTest(0, 0);
    QVERIFY(one && two && !three);
    QCOMPARE(threads, 2);
    QCOMPARE(replies, 3);
    a.removeAll();
}

} // namespace threadscript
