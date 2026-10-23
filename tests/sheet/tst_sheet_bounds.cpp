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

    // ---- time

};

JP_TEST_CLASS(SheetBoundsTests)
#include "tst_sheet_bounds.moc"
