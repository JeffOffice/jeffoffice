// Tests of what one evaluation may spend (calc/eval.h): the memory of the arrays and texts it builds, and the time of what the step count can't see at a glance. A bound
// is tested by what it counts (the steps taken, the cells held, the status it ends in), and by the memory the process reached where the system tells it; never by how
// many milliseconds it took. Each trigger here was found to take gigabytes or half a minute before the work was charged for it.
#include "calchelpers.h"

#include <QElapsedTimer>

#include <cmath>
#include <set>

using namespace jp::calc;

namespace {

QString statusName(EvalStatus s)
{
    switch (s) {
    case EvalStatus::Ok: return QStringLiteral("Ok");
    case EvalStatus::TooMuchWork: return QStringLiteral("TooMuchWork");
    case EvalStatus::TooLarge: return QStringLiteral("TooLarge");
    case EvalStatus::TooDeep: return QStringLiteral("TooDeep");
    }
    return {};
}

// What an outcome says, in one line, for a failure to show.
QString said(const Outcome &o, qint64 growthMegabytes = -1)
{
    QString s = QStringLiteral("%1 (status %2, work %3, most cells held %4").arg(o.text.left(40), statusName(o.status)).arg(o.work).arg(o.peakCells);
    if (growthMegabytes >= 0) s += QStringLiteral(", memory grew %1 MB").arg(growthMegabytes);
    return s + QLatin1Char(')');
}

QString repeatedArguments(const QString &argument, int count)
{
    QStringList list;
    for (int i = 0; i < count; ++i) list << argument;
    return list.join(QLatin1Char(','));
}

// The most megabytes the process may grow by while a build is refused: the cells an evaluation may hold at once, and the text it may make, are a few hundred.
constexpr qint64 kMostGrowth = 500;

} // namespace

class SheetBoundsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- memory

    // A whole column of nothing is an array of a million cells that takes 56 megabytes: 40 of them, and 255, are refused as they are asked for, not after they are all built.
    void manyWholeColumnsAreRefusedBeforeTheyAreBuilt()
    {
        Grid g;
        g.n("A1", 1);
        {
            const MemoryWatch watch;
            const Outcome forty = evOutcome("SUMPRODUCT(" + repeatedArguments("Z:Z", 40) + ")", g);
            QVERIFY2(forty.text.startsWith("e:") && forty.status == EvalStatus::TooLarge, qPrintable(said(forty, watch.growthMegabytes())));
            QVERIFY2(watch.growthMegabytes() < kMostGrowth, qPrintable(said(forty, watch.growthMegabytes())));
        }
        const MemoryWatch watch;
        const Outcome all = evOutcome("SUMPRODUCT(" + repeatedArguments("Z:Z", 255) + ")", g);
        QVERIFY2(all.text.startsWith("e:") && all.status == EvalStatus::TooLarge, qPrintable(said(all, watch.growthMegabytes())));
        QVERIFY2(watch.growthMegabytes() < kMostGrowth, qPrintable(said(all, watch.growthMegabytes())));
        // the cells held at once stay inside the budget, which is what the memory follows
        QVERIFY2(all.peakCells <= kMaxLiveArrayCells + kMaxArrayCells, qPrintable(said(all)));
        // a few of them, as formulas have them, are no trouble
        const Outcome few = evOutcome("SUMPRODUCT(Z:Z,Z:Z,Z:Z)", g);
        QVERIFY2(few.text == "n:0" && few.status == EvalStatus::Ok, qPrintable(said(few)));
    }

    // The rows of a whole column are an array too, and ROW builds it: it is charged by its size and counted among the cells held.
    void theRowsOfAWholeColumnAreChargedAndCounted()
    {
        Grid g;
        g.n("A1", 1);
        Settings array;
        array.array = true;
        {
            const MemoryWatch watch;
            const Outcome product = evOutcome("SUMPRODUCT(" + repeatedArguments("ROW(A:A)", 40) + ")", g);
            QVERIFY2(product.text.startsWith("e:") && product.status == EvalStatus::TooLarge, qPrintable(said(product, watch.growthMegabytes())));
            QVERIFY2(watch.growthMegabytes() < kMostGrowth, qPrintable(said(product, watch.growthMegabytes())));
        }
        const MemoryWatch watch;
        const Outcome sum = evOutcome("SUM(" + repeatedArguments("ROW(A:A)", 60) + ")", g, array);
        QVERIFY2(sum.text.startsWith("e:") && sum.status == EvalStatus::TooLarge, qPrintable(said(sum, watch.growthMegabytes())));
        QVERIFY2(watch.growthMegabytes() < kMostGrowth, qPrintable(said(sum, watch.growthMegabytes())));
        // one of them is charged the cells it makes
        const Outcome one = evOutcome("SUM(ROW(A:A))", g, array);
        QVERIFY2(one.text == "n:549756338176" && one.work >= 1048576, qPrintable(said(one)));
    }

    // A text of 16,000 characters in every cell of a column, joined to itself, is a million texts of 32,000: the characters made are counted, and past a budget it stops.
    void textsMadeInAnArrayAreCountedByTheirSize()
    {
        Grid g;
        QVariantList cells;
        const QString big(16000, QLatin1Char('x'));
        for (int i = 0; i < 8000; ++i) cells << big;
        g.column("A1", cells);
        {
            const MemoryWatch watch;
            const Outcome joined = evOutcome("SUMPRODUCT(LEN(A1:A8000&A1:A8000))", g);
            QVERIFY2(joined.text.startsWith("e:") && joined.status == EvalStatus::TooLarge, qPrintable(said(joined, watch.growthMegabytes())));
            QVERIFY2(watch.growthMegabytes() < kMostGrowth, qPrintable(said(joined, watch.growthMegabytes())));
        }
        // as many texts as a budget holds are no trouble
        const Outcome some = evOutcome("SUMPRODUCT(LEN(A1:A1000&A1:A1000))", g);
        QVERIFY2(some.text == "n:32000000" && some.status == EvalStatus::Ok, qPrintable(said(some)));
    }

    // SUBSTITUTE stops as the text it makes passes what a cell holds; it does not make a gigabyte and then look.
    void substituteStopsWhereACellEnds()
    {
        {
            const MemoryWatch watch;
            const Outcome grow = evOutcome("LEN(SUBSTITUTE(REPT(\"a\",32767),\"a\",REPT(\"b\",32767)))", Grid());
            QVERIFY2(grow.text == "e:#VALUE!", qPrintable(said(grow, watch.growthMegabytes())));
            QVERIFY2(watch.growthMegabytes() < 100, qPrintable(said(grow, watch.growthMegabytes())));
        }
        // the longest text that fits is made, one past it is not
        QCOMPARE(ev("LEN(SUBSTITUTE(REPT(\"a\",16383),\"a\",\"bb\"))"), num(32766));
        QCOMPARE(ev("LEN(SUBSTITUTE(REPT(\"a\",32767),\"a\",\"b\"))"), num(32767));
        QCOMPARE(ev("LEN(SUBSTITUTE(REPT(\"a\",32767),\"a\",\"bb\"))"), QStringLiteral("e:#VALUE!"));
        QCOMPARE(ev("LEN(SUBSTITUTE(REPT(\"a\",16384),\"a\",\"bb\"))"), QStringLiteral("e:#VALUE!"));
        QCOMPARE(ev("SUBSTITUTE(\"banana\",\"an\",\"AN\")"), QStringLiteral("s:bANANa"));
        QCOMPARE(ev("SUBSTITUTE(\"banana\",\"an\",\"AN\",2)"), QStringLiteral("s:banANa"));
        QCOMPARE(ev("SUBSTITUTE(\"banana\",\"\",\"x\")"), QStringLiteral("s:banana"));
    }

    // The texts INDIRECT is given are kept to be worked out again, up to a number of bytes, not a number of texts: two thousand long ones don't take hundreds of megabytes.
    void theTextsOfIndirectAreKeptWithinAByteBudget()
    {
        Grid g;
        g.n("A1", 1);
        Settings array;
        array.array = true;
        // 2,000 texts of 21,000 characters each, every one different: a formula of 7,000 references, which a program of some 400 kilobytes comes of
        const MemoryWatch watch;
        const Outcome many = evOutcome("SUMPRODUCT(--ISERROR(INDIRECT(REPT(\"A1+\",7000)&ROW(1:2000))))", g, array);
        QVERIFY2(many.status == EvalStatus::Ok, qPrintable(said(many, watch.growthMegabytes())));
        QVERIFY2(watch.growthMegabytes() < 300, qPrintable(said(many, watch.growthMegabytes())));
    }

    // ---- time

    // A name that stands for itself, directly or through others, is a circular reference: it is met once and gives the error such a name has always given, not a
    // tree of three to the sixteenth evaluations of it.
    void aNameThatRefersToItselfIsMetOnce()
    {
        Grid g;
        g.name("M", "M+M+M");
        g.name("Q", "SUM(Q,Q,Q,Q)");
        g.name("Alpha", "Beta+Beta");
        g.name("Beta", "Alpha+Alpha");
        g.name("P", "P&P");
        g.name("R", "INDIRECT(\"R\")+INDIRECT(\"R\")");
        g.name("Chain", "Chain2*2");
        g.name("Chain2", "Chain3*2");
        g.name("Chain3", "5");
        for (const char *f : {"M", "Q", "Alpha", "LEN(P)", "R", "M+Q+Alpha+Beta", "SUM(M,M,M)"}) {
            const Outcome o = evOutcome(QString::fromLatin1(f), g, Settings(), 2000000);
            QVERIFY2(o.text == "e:#REF!" && o.status == EvalStatus::TooDeep && o.work < 500, qPrintable(QString::fromLatin1(f) + QStringLiteral(": ") + said(o)));
        }
        // names that call one another without a circle are worked out as they always were
        const Outcome chain = evOutcome("Chain+Chain", g);
        QVERIFY2(chain.text == "n:40" && chain.status == EvalStatus::Ok, qPrintable(said(chain)));
        // a name used twice in one formula is not a circle
        g.name("Twice", "Chain3+1");
        QCOMPARE(ev("Twice*Twice", g), num(36));
    }

    // Comparing two texts takes as long as they are, and is counted so: a column of long texts matched against a long text runs out of steps, instead of taking seconds
    // at a hundred thousand steps.
    void comparingLongTextsIsChargedByTheirLength()
    {
        Grid g;
        QVariantList cells;
        const QString big(32767, QLatin1Char('x'));
        for (int i = 0; i < 60000; ++i) cells << big;
        g.column("A1", cells);
        g.s("B1", big);
        Settings array;
        array.array = true;
        const Outcome countIf = evOutcome("COUNTIF(A1:A60000,B1)", g);
        QVERIFY2(countIf.text.startsWith("e:") && countIf.status == EvalStatus::TooMuchWork, qPrintable(said(countIf)));
        const Outcome product = evOutcome("SUMPRODUCT(--(A1:A60000=B1))", g);
        QVERIFY2(product.text.startsWith("e:") && product.status == EvalStatus::TooMuchWork, qPrintable(said(product)));
        // fewer of them are counted by the length of what they compare, the same formula over short texts by almost nothing
        const Outcome fewLong = evOutcome("COUNTIF(A1:A2000,B1)", g);
        QVERIFY2(fewLong.text == "n:2000" && fewLong.work >= 2000 * (32767 / 64), qPrintable(said(fewLong)));
        Grid shortTexts;
        QVariantList small;
        for (int i = 0; i < 2000; ++i) small << QStringLiteral("xyz");
        shortTexts.column("A1", small);
        shortTexts.s("B1", "xyz");
        const Outcome fewShort = evOutcome("COUNTIF(A1:A2000,B1)", shortTexts);
        QVERIFY2(fewShort.text == "n:2000" && fewShort.work < 2100, qPrintable(said(fewShort)));
    }

};

JP_TEST_CLASS(SheetBoundsTests)
#include "tst_sheet_bounds.moc"
