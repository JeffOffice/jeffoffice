// Tests of the calculation (sheet/calculation.h): a workbook's formulas worked out from its constants in dependency order with an explicit stack, circular references,
// one reading of the clock for a recalculation, a recalculation in slices that can be stopped, edits that recalculate only what depends on them, references that are told
// as a formula is worked out, shared and array formulas, and what is kept apart. Every workbook is built in memory; every expected value is worked out by hand.
#include "sheethelpers.h"

#include "sheet/calculation.h"

#include <QRandomGenerator>

#include <atomic>
#include <memory>

namespace {

QString cellXml(const QString &ref, const QString &formula, const QString &value = QString(), const QString &type = QString())
{
    return QStringLiteral("<c r=\"%1\"%2><f>%3</f>%4</c>").arg(ref, type.isEmpty() ? QString() : QStringLiteral(" t=\"%1\"").arg(type), formula.toHtmlEscaped(),
                                                                value.isEmpty() ? QString() : QStringLiteral("<v>%1</v>").arg(value.toHtmlEscaped()));
}

QString constant(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }

QString rowXml(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

// A workbook read into memory, and a calculation over it.
struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool open(const QByteArray &zip, const CalcOptions &options = CalcOptions())
    {
        QString why;
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        calc.reset(new Calculation(book, options));
        return calc->load();
    }
    quint32 row(const QString &ref) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return r;
    }
    quint32 col(const QString &ref) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return c;
    }
    // What the cell holds, as the engine says it: "n:1.5", "s:text", "e:#DIV/0!", "-" for none.
    QString at(const QString &ref, int sheet = 0) const { return calc->cell(sheet, row(ref), col(ref)).debug(); }
    bool set(const QString &ref, const Value &v, int sheet = 0) { return calc->setValue(sheet, row(ref), col(ref), v); }
    bool setFormula(const QString &ref, const QString &f, int sheet = 0) { return calc->setFormula(sheet, row(ref), col(ref), f); }
    bool circular(const QString &ref, int sheet = 0) const { return calc->isCircular(sheet, row(ref), col(ref)); }
    bool tainted(const QString &ref, int sheet = 0) const { return calc->isTainted(sheet, row(ref), col(ref)); }
};

// Works out all that is to be worked out; the last progress.
Calculation::Progress settle(Book &b)
{
    Calculation::Progress p = b.calc->recalculate();
    return p;
}

} // namespace

class SheetCalculationTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Every formula is worked out from the constants, not from the results the file stored (these are wrong, or none), in the order they depend on each other, whatever
    // the order they are in the file.
    void formulasAreWorkedOutFromConstantsInDependencyOrder()
    {
        const QString rows = rowXml(1, cellXml("A1", "A2*2", "999") + constant("B1", "5") + cellXml("C1", "SUM(A1:B1)+D2", "0"))
            + rowXml(2, cellXml("A2", "A3+1") + cellXml("B2", "\"x\"&B3", "wrong", "str") + cellXml("D2", "B1*10"))
            + rowXml(3, constant("A3", "10") + cellXml("B3", "A1>1", "0", "b"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        const Calculation::Progress p = settle(b);
        QVERIFY(p.finished);
        QCOMPARE(b.at("A2"), QStringLiteral("n:11"));
        QCOMPARE(b.at("A1"), QStringLiteral("n:22"));
        QCOMPARE(b.at("D2"), QStringLiteral("n:50"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:77"));          // 22 + 5 + 50
        QCOMPARE(b.at("B3"), QStringLiteral("b:1"));
        QCOMPARE(b.at("B2"), QStringLiteral("s:xTRUE"));        // "x" and TRUE read as text, as & does
        QCOMPARE(b.at("B1"), QStringLiteral("n:5"));
        QCOMPARE(b.at("A3"), QStringLiteral("n:10"));
        QCOMPARE(b.calc->formulaCount(), qint64(6));
        QVERIFY(!b.calc->isDirty());
    }

    // A running balance of a hundred thousand rows, each reading the one below it, so that the first has to wait for all the others: the stack is explicit, and each formula is
    // worked out twice at the most.
    void aRunningBalanceOfAHundredThousandRowsDoesNotOverflowTheStack()
    {
        const int n = 100000;
        QString rows;
        rows.reserve(n * 60);
        for (int i = 1; i < n; ++i) rows += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("A%1+1").arg(i + 1)));
        rows += rowXml(n, constant(QStringLiteral("A%1").arg(n), "1"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        const Calculation::Progress p = settle(b);
        QVERIFY(p.finished);
        QCOMPARE(b.at("A1"), QStringLiteral("n:100000"));
        QCOMPARE(b.at("A50000"), QStringLiteral("n:50001"));
        QCOMPARE(b.at("A99999"), QStringLiteral("n:2"));
        QVERIFY2(p.evaluated <= 2 * (n - 1), qPrintable(QString::number(p.evaluated)));
        QVERIFY2(p.deepest >= n - 1, qPrintable(QString::number(p.deepest)));
        // the same, each reading the one above: in file order each is worked out once
        QString down;
        down += rowXml(1, constant("A1", "1"));
        for (int i = 2; i <= n; ++i) down += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("A%1+1").arg(i - 1)));
        Book c;
        QVERIFY(c.open(buildXlsx({worksheet(down)})));
        const Calculation::Progress q = settle(c);
        QCOMPARE(c.at(QStringLiteral("A%1").arg(n)), QStringLiteral("n:100000"));
        QCOMPARE(q.evaluated, qint64(n - 1));
    }

    // A circle of formulas gets the result Excel gives it, 0, as does a formula that reads itself, and a formula that reads a block that holds it; what reads a circle reads
    // that 0, and is no part of it.
    void circularReferencesAreFoundAndGiveZero()
    {
        const QString rows = rowXml(1, cellXml("A1", "B1+1") + cellXml("B1", "A1+1") + cellXml("C1", "A1+10") + cellXml("D1", "D1+1") + cellXml("E1", "SUM(F1:F3)"))
            + rowXml(2, cellXml("F2", "E1+1") + constant("F1", "4") + cellXml("G2", "C1*2"))
            + rowXml(3, cellXml("A3", "B3") + cellXml("B3", "C3") + cellXml("C3", "A3") + cellXml("D3", "A3+B3+C3+1"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        const Calculation::Progress p = settle(b);
        QVERIFY(p.finished);
        for (const char *ref : {"A1", "B1", "D1", "E1", "F2", "A3", "B3", "C3"}) {
            QVERIFY2(b.circular(QString::fromLatin1(ref)), ref);
            QCOMPARE(b.at(QString::fromLatin1(ref)), QStringLiteral("n:0"));
        }
        for (const char *ref : {"C1", "G2", "D3"}) QVERIFY2(!b.circular(QString::fromLatin1(ref)), ref);
        QCOMPARE(b.at("C1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("G2"), QStringLiteral("n:20"));
        QCOMPARE(b.at("D3"), QStringLiteral("n:1"));
        QCOMPARE(p.circular, qint64(8));
        // breaking a circle by an edit works the cells out
        QVERIFY(b.set("B1", Value::number(5)));
        const Calculation::Progress q = settle(b);
        QVERIFY(q.finished);
        QCOMPARE(b.at("A1"), QStringLiteral("n:6"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:16"));
        QVERIFY(!b.circular("A1"));
    }

    // NOW is read once for a recalculation: every NOW in it is one moment, and TODAY is its day; the next recalculation reads it again. With a time given, that is the time.
    void theClockIsReadOncePerRecalculation()
    {
        QString cells, row2;
        for (int i = 0; i < 300; ++i) {
            const QString ref = QStringLiteral("%1").arg(i + 1);
            cells += rowXml(i + 1, cellXml("A" + ref, "NOW()") + cellXml("B" + ref, "NOW()=A" + ref) + cellXml("C" + ref, "AND(NOW()-TODAY()>=0,NOW()-TODAY()<1)") + cellXml("D" + ref, "A" + ref + "-TODAY()"));
        }
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(cells)})));
        settle(b);
        const double first = b.calc->cell(0, 0, 0).number();
        QVERIFY(first > 45292);
        for (quint32 r = 0; r < 300; ++r) {
            QCOMPARE(b.calc->cell(0, r, 0).number(), first);
            QCOMPARE(b.calc->cell(0, r, 1).debug(), QStringLiteral("b:1"));
            QCOMPARE(b.calc->cell(0, r, 2).debug(), QStringLiteral("b:1"));
            const double fraction = b.calc->cell(0, r, 3).number();
            QVERIFY(fraction >= 0 && fraction < 1);
        }
        // the next recalculation reads the clock again (the volatile formulas are worked out again)
        const Calculation::Progress p = settle(b);
        QCOMPARE(p.evaluated, qint64(1200));
        QVERIFY(b.calc->cell(0, 0, 0).number() >= first);   // (a clock may tick once in fifteen milliseconds: the reading is again, not before)
        QCOMPARE(b.calc->cell(0, 299, 0).number(), b.calc->cell(0, 0, 0).number());
        // a time that is given is the time
        CalcOptions fixed;
        fixed.now = 45366.5;
        Book c;
        QVERIFY(c.open(buildXlsx({worksheet(cells)}), fixed));
        settle(c);
        QCOMPARE(c.at("A1"), QStringLiteral("n:45366.5"));
        QCOMPARE(c.at("D7"), QStringLiteral("n:0.5"));
    }

    // A recalculation can be done in slices, each of about the work it is given; it ends with the values a whole one gives, and a slice does not start over what an earlier one did.
    void aRecalculationRunsInSlicesAndEndsWithTheSameValues()
    {
        QString rows;
        for (int i = 1; i <= 20000; ++i) rows += rowXml(i, constant(QStringLiteral("A%1").arg(i), QString::number(i)) + cellXml(QStringLiteral("B%1").arg(i), QStringLiteral("A%1*2").arg(i)) + cellXml(QStringLiteral("C%1").arg(i), QStringLiteral("B%1+SUM(A1:A10)").arg(i)));
        Book whole, sliced;
        QVERIFY(whole.open(buildXlsx({worksheet(rows)})));
        QVERIFY(sliced.open(buildXlsx({worksheet(rows)})));
        settle(whole);
        int slices = 0;
        qint64 evaluated = 0;
        for (;;) {
            const Calculation::Progress p = sliced.calc->recalculate(3000);
            evaluated += p.evaluated;
            ++slices;
            QVERIFY(slices < 100000);
            if (p.finished) break;
            QVERIFY(sliced.calc->isDirty());
        }
        QVERIFY2(slices > 5, qPrintable(QString::number(slices)));
        QCOMPARE(evaluated, qint64(40000));        // each formula once
        for (int i = 1; i <= 20000; i += 997) {
            const QString ref = QStringLiteral("C%1").arg(i);
            QCOMPARE(sliced.at(ref), whole.at(ref));
        }
        QCOMPARE(sliced.at("C20000"), QStringLiteral("n:%1").arg(40000 + 55));
    }

    // A recalculation that is stopped leaves the values it has, and the formulas it did not get to still to be worked out; running it again finishes it, with no formula
    // worked out twice that was done.
    void aRecalculationCanBeCancelledAndFinishedLater()
    {
        QString rows;
        for (int i = 1; i <= 5000; ++i) rows += rowXml(i, constant(QStringLiteral("A%1").arg(i), QString::number(i)) + cellXml(QStringLiteral("B%1").arg(i), QStringLiteral("A%1+1").arg(i)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        std::atomic<bool> stop{false};
        const Calculation::Progress p = b.calc->recalculate(500, &stop);
        QVERIFY(!p.finished && !p.cancelled);
        stop = true;
        const Calculation::Progress q = b.calc->recalculate(0, &stop);
        QVERIFY(q.cancelled && !q.finished);
        QVERIFY(b.calc->isDirty());
        stop = false;
        const Calculation::Progress r = b.calc->recalculate(0, &stop);
        QVERIFY(r.finished && !r.cancelled);
        QCOMPARE(p.evaluated + q.evaluated + r.evaluated, qint64(5000));
        QCOMPARE(b.at("B1"), QStringLiteral("n:2"));
        QCOMPARE(b.at("B5000"), QStringLiteral("n:5001"));
        QVERIFY(!b.calc->isDirty());
    }

    // An edit works out what depends on the cell and nothing else: the work is the formulas that read it, and those that read them, whatever else the workbook holds.
    void anEditRecalculatesOnlyWhatDependsOnIt()
    {
        QString rows;
        for (int i = 1; i <= 1000; ++i)
            rows += rowXml(i, constant(QStringLiteral("A%1").arg(i), QString::number(i)) + cellXml(QStringLiteral("B%1").arg(i), QStringLiteral("A%1*2").arg(i))
                              + (i == 1 ? cellXml("C1", "SUM(B1:B1000)") + cellXml("D1", "C1+1") + cellXml("E1", "IF(D1>0,\"up\",\"down\")") : QString())
                              + cellXml(QStringLiteral("G%1").arg(i), QStringLiteral("A%1+1000").arg(i)));
        for (int i = 1001; i <= 6000; ++i) rows += rowXml(i, cellXml(QStringLiteral("H%1").arg(i), QStringLiteral("%1*3").arg(i)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        settle(b);
        QCOMPARE(b.at("C1"), QStringLiteral("n:%1").arg(1000 * 1001));
        QCOMPARE(b.calc->formulaCount(), qint64(1000 + 1000 + 3 + 5000));
        // an input that three formulas depend on, one a block, one on it: B5, then C1, then D1 and E1
        QVERIFY(b.set("A5", Value::number(100)));
        QCOMPARE(b.calc->dirtyCount(), qint64(5));       // B5, G5, C1, D1, E1
        const Calculation::Progress p = settle(b);
        QVERIFY(p.finished);
        QCOMPARE(p.evaluated, qint64(5));
        QCOMPARE(b.at("B5"), QStringLiteral("n:200"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:%1").arg(1000 * 1001 + 190));
        QCOMPARE(b.at("D1"), QStringLiteral("n:%1").arg(1000 * 1001 + 191));
        QCOMPARE(b.at("G5"), QStringLiteral("n:1100"));
        // a formula put in a cell: it and what reads it
        QVERIFY(b.setFormula("A7", "A8+1"));
        QCOMPARE(b.calc->dirtyCount(), qint64(6));       // A7, B7, G7, C1, D1, E1
        const Calculation::Progress q = settle(b);
        QCOMPARE(q.evaluated, qint64(6));
        QCOMPARE(b.at("A7"), QStringLiteral("n:9"));
        QCOMPARE(b.at("B7"), QStringLiteral("n:18"));
        QCOMPARE(b.calc->formulaCount(), qint64(1000 + 1000 + 3 + 5000 + 1));
        // a formula's cell made a constant: what read it is worked out, and it is no formula
        QVERIFY(b.set("B9", Value::number(1)));
        const Calculation::Progress r = settle(b);
        QCOMPARE(r.evaluated, qint64(3));                  // C1, D1, E1
        QCOMPARE(b.at("B9"), QStringLiteral("n:1"));
        QVERIFY(b.at("C1") != QStringLiteral("n:%1").arg(1000 * 1001 + 190));
        // an empty cell inside a block that is read gets a value; a cell cleared
        QVERIFY(b.setFormula("J1", "SUM(K1:K1048576)"));
        settle(b);
        QCOMPARE(b.at("J1"), QStringLiteral("n:0"));
        QVERIFY(b.set("K1000000", Value::number(7)));
        QCOMPARE(b.calc->dirtyCount(), qint64(1));
        settle(b);
        QCOMPARE(b.at("J1"), QStringLiteral("n:7"));
        QVERIFY(b.calc->clearCell(0, b.row("K1000000"), b.col("K1000000")));
        settle(b);
        QCOMPARE(b.at("J1"), QStringLiteral("n:0"));
        // a text, a boolean, and an error as inputs
        QVERIFY(b.setFormula("L1", "A1&\"-\"&M1"));
        QVERIFY(b.set("M1", Value::text("abc")));
        settle(b);
        QCOMPARE(b.at("L1"), QStringLiteral("s:1-abc"));
        QVERIFY(b.set("M1", Value::error(ErrorKind::NA)));
        settle(b);
        QCOMPARE(b.at("L1"), QStringLiteral("e:#N/A"));
        QVERIFY(!b.setFormula("L2", "1+"));                // text that is no formula changes nothing
        QCOMPARE(b.at("L2"), QStringLiteral("-"));
        QVERIFY(!b.calc->isDirty());
    }

    // An edit at the end of a chain of a hundred thousand formulas works them out once each, in the order they depend on each other, whichever way the chain runs.
    void anEditAtTheEndOfALongChainWorksEachFormulaOutOnce()
    {
        const int n = 100000;
        QString down, up;
        down += rowXml(1, constant("A1", "1"));
        for (int i = 2; i <= n; ++i) down += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("A%1+1").arg(i - 1)));
        for (int i = 1; i < n; ++i) up += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("A%1+1").arg(i + 1)));
        up += rowXml(n, constant(QStringLiteral("A%1").arg(n), "1"));
        Book d, u;
        QVERIFY(d.open(buildXlsx({worksheet(down)})));
        QVERIFY(u.open(buildXlsx({worksheet(up)})));
        settle(d);
        settle(u);
        QVERIFY(d.set("A1", Value::number(5)));
        QCOMPARE(d.calc->dirtyCount(), qint64(n - 1));
        const Calculation::Progress p = settle(d);
        QCOMPARE(p.evaluated, qint64(n - 1));
        QCOMPARE(d.at(QStringLiteral("A%1").arg(n)), QStringLiteral("n:%1").arg(n + 4));
        QVERIFY(u.set(QStringLiteral("A%1").arg(n), Value::number(5)));
        QCOMPARE(u.calc->dirtyCount(), qint64(n - 1));
        const Calculation::Progress q = settle(u);
        QCOMPARE(q.evaluated, qint64(n - 1));
        QCOMPARE(u.at("A1"), QStringLiteral("n:%1").arg(n + 4));
        QVERIFY2(q.deepest <= 2, qPrintable(QString::number(q.deepest)));
    }

    // Edits, in any number and order, leave the values a whole recalculation gives: a grid of formulas that read cells and blocks of cells wherever they are (but never in a
    // circle), edited over and over, is the same after each recalculation as it is after working out every formula again. The formulas read through INDEX and the range
    // operator (a block made of references that functions give), through SUMIF and AVERAGEIF with a sum range shorter than the range, and through defined names (of blocks,
    // of other names, of OFFSET, of INDEX, and of a volatile formula).
    void editsLeaveWhatAWholeRecalculationGives()
    {
        const int rows = 24, cols = 7, cellsN = rows * cols;
        QRandomGenerator rng(20261025);
        // a rank for every cell: a formula reads only cells of lower rank, so there is no circle, whatever the places
        std::vector<int> rank(static_cast<size_t>(cellsN));
        for (int i = 0; i < cellsN; ++i) rank[size_t(i)] = i;
        for (int i = cellsN - 1; i > 0; --i) std::swap(rank[size_t(i)], rank[size_t(rng.bounded(quint32(i + 1)))]);
        auto name = [&](int r, int c) { return QStringLiteral("%1%2").arg(QChar(QLatin1Char('A' + c))).arg(r + 1); };
        auto colName = [&](int c) { return QString(QChar(QLatin1Char('A' + c))); };
        auto lower = [&](int r, int c) {
            std::vector<QPair<int, int>> ok;
            for (int rr = 0; rr < rows; ++rr)
                for (int cc = 0; cc < cols; ++cc)
                    if (rank[size_t(rr * cols + cc)] < rank[size_t(r * cols + c)]) ok.push_back({rr, cc});
            return ok;
        };
        auto allLower = [&](int r1, int c1, int r2, int c2, int r, int c) {
            for (int rr = r1; rr <= r2; ++rr)
                for (int cc = c1; cc <= c2; ++cc)
                    if (rank[size_t(rr * cols + cc)] >= rank[size_t(r * cols + c)]) return false;
            return true;
        };
        // the names, and the cells each one stands for
        struct NamedCells {
            QString name, formula;
            int r1, c1, r2, c2;
            bool block;
        };
        const std::vector<NamedCells> names = {
            {QStringLiteral("nBlock"), QStringLiteral("Sheet1!$B$3:$D$5"), 2, 1, 4, 3, true},
            {QStringLiteral("nCell"), QStringLiteral("Sheet1!$A$2"), 1, 0, 1, 0, false},
            {QStringLiteral("nOff"), QStringLiteral("OFFSET(Sheet1!$A$1,5,1,2,2)"), 5, 1, 6, 2, true},
            {QStringLiteral("nNest"), QStringLiteral("nBlock"), 2, 1, 4, 3, true},
            {QStringLiteral("nVol"), QStringLiteral("RAND()*0+Sheet1!$E$2"), 1, 4, 1, 4, false},
            {QStringLiteral("nIdx"), QStringLiteral("INDEX(Sheet1!$A$1:$G$24,4,3)"), 3, 2, 3, 2, false},
            {QStringLiteral("nNow"), QStringLiteral("NOW()*4"), -1, -1, -1, -1, false},   // (reads no cell; the time is set for each step, so that it is what it is worked out from)
        };
        QString definedNames;
        for (const NamedCells &n : names) definedNames += QStringLiteral("<definedName name=\"%1\">%2</definedName>").arg(n.name, n.formula.toHtmlEscaped());
        auto refOf = [&](int r, int c) {
            // a cell of lower rank, or a block of them
            const std::vector<QPair<int, int>> ok = lower(r, c);
            if (ok.empty()) return QString();
            const QPair<int, int> p = ok[rng.bounded(quint32(ok.size()))];
            if (rng.bounded(3) == 0) {
                for (int attempt = 0; attempt < 8; ++attempt) {
                    const int r2 = qMin(rows - 1, p.first + int(rng.bounded(5))), c2 = qMin(cols - 1, p.second + int(rng.bounded(3)));
                    if (allLower(p.first, p.second, r2, c2, r, c)) return name(p.first, p.second) + QLatin1Char(':') + name(r2, c2);
                }
            }
            return name(p.first, p.second);
        };
        // a block of cells of lower rank, of more than one cell
        auto rectOf = [&](int r, int c, int *r1, int *c1, int *r2, int *c2) {
            const std::vector<QPair<int, int>> ok = lower(r, c);
            for (int attempt = 0; attempt < 12 && !ok.empty(); ++attempt) {
                const QPair<int, int> p = ok[rng.bounded(quint32(ok.size()))];
                *r1 = p.first;
                *c1 = p.second;
                *r2 = qMin(rows - 1, *r1 + 1 + int(rng.bounded(4)));
                *c2 = qMin(cols - 1, *c1 + int(rng.bounded(3)));
                if ((*r2 > *r1 || *c2 > *c1) && allLower(*r1, *c1, *r2, *c2, r, c)) return true;
            }
            return false;
        };
        auto formulaFor = [&](int r, int c) {
            const QString a = refOf(r, c);
            if (a.isEmpty()) return QStringLiteral("%1").arg(1 + int(rng.bounded(9)));
            const QString b = refOf(r, c);
            const int kind = int(rng.bounded(16));
            int r1 = 0, c1 = 0, r2 = 0, c2 = 0;
            switch (kind) {
            case 7:   // a block from a cell to the cell a function gives
                if (rectOf(r, c, &r1, &c1, &r2, &c2)) return QStringLiteral("SUM(%1:INDEX(%2:%2,%3))").arg(name(r1, c1), colName(c2)).arg(r2 + 1);
                break;
            case 8:   // a block from the cell a function gives to the cell another gives
                if (rectOf(r, c, &r1, &c1, &r2, &c2)) return QStringLiteral("MAX(INDEX(%1:%1,%2):INDEX(%3:%3,%4))").arg(colName(c1)).arg(r1 + 1).arg(colName(c2)).arg(r2 + 1);
                break;
            case 9: case 10:   // SUMIF and AVERAGEIF with a sum range of one cell, which is the shape of the range
                if (rectOf(r, c, &r1, &c1, &r2, &c2)) {
                    for (int attempt = 0; attempt < 12; ++attempt) {
                        const int sr = int(rng.bounded(quint32(rows - (r2 - r1)))), sc = int(rng.bounded(quint32(cols - (c2 - c1))));
                        if (!allLower(sr, sc, sr + (r2 - r1), sc + (c2 - c1), r, c)) continue;
                        const QString range = name(r1, c1) + QLatin1Char(':') + name(r2, c2);
                        return kind == 9 ? QStringLiteral("SUMIF(%1,\">3\",%2)").arg(range, name(sr, sc)) : QStringLiteral("IFERROR(AVERAGEIF(%1,\">=0\",%2),-1)").arg(range, name(sr, sc));
                    }
                }
                break;
            case 11: case 12: case 13: {   // a defined name, if the cells it stands for are all of lower rank
                std::vector<const NamedCells *> usable;
                for (const NamedCells &n : names)
                    if (n.r1 < 0 || allLower(n.r1, n.c1, n.r2, n.c2, r, c)) usable.push_back(&n);
                if (usable.empty()) break;
                const NamedCells &n = *usable[rng.bounded(quint32(usable.size()))];
                if (!n.block) return QStringLiteral("%1*2+%2").arg(n.name, a.contains(':') ? QStringLiteral("COUNT(%1)").arg(a) : a);
                return kind == 11 ? QStringLiteral("SUM(%1)+1").arg(n.name) : QStringLiteral("MAX(%1,%2)").arg(n.name, a.contains(':') ? QStringLiteral("MIN(%1)").arg(a) : a);
            }
            default: break;
            }
            switch (kind % 7) {
            case 0: return QStringLiteral("SUM(%1)+1").arg(a);
            case 1: return QStringLiteral("%1*2").arg(a.contains(':') ? QStringLiteral("MAX(%1)").arg(a) : a);
            case 2: return QStringLiteral("IF(SUM(%1)>10,SUM(%2),-1)").arg(a, b.isEmpty() ? a : b);
            case 3: return QStringLiteral("COUNT(%1)+SUM(%2)").arg(a, b.isEmpty() ? a : b);
            case 4: return QStringLiteral("AVERAGE(%1,%2)").arg(a, b.isEmpty() ? a : b);
            case 5: return QStringLiteral("IFERROR(1/SUM(%1),0)").arg(a);
            default: return QStringLiteral("SUM(%1)-MIN(%2)").arg(a, b.isEmpty() ? a : b);
            }
        };
        QString sheet;
        for (int r = 0; r < rows; ++r) {
            QString cellsXml;
            for (int c = 0; c < cols; ++c)
                if (rng.bounded(3) == 0) cellsXml += constant(name(r, c), QString::number(int(rng.bounded(20))));
                else cellsXml += cellXml(name(r, c), formulaFor(r, c));
            sheet += rowXml(r + 1, cellsXml);
        }
        Book b;
        CalcOptions at;
        at.now = 45000;
        QVERIFY(b.open(buildXlsx({worksheet(sheet)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), definedNames), at));
        settle(b);
        auto snapshot = [&]() {
            QStringList all;
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < cols; ++c) all << b.calc->cell(0, quint32(r), quint32(c)).debug();
            return all;
        };
        for (int step = 0; step < 600; ++step) {
            b.calc->setNow(45000.0 + 0.25 * step);   // (a day number a step: what reads NOW, through a name or not, is worked out again)
            const int edits = 1 + int(rng.bounded(3));
            for (int e = 0; e < edits; ++e) {
                const int r = int(rng.bounded(rows)), c = int(rng.bounded(cols));
                switch (rng.bounded(4)) {
                case 0: QVERIFY(b.set(name(r, c), Value::number(int(rng.bounded(20))))); break;
                case 1: QVERIFY(b.setFormula(name(r, c), formulaFor(r, c))); break;
                case 2: QVERIFY(b.set(name(r, c), Value::text(QStringLiteral("t")))); break;
                default: QVERIFY(b.calc->clearCell(0, quint32(r), quint32(c))); break;
                }
            }
            // sometimes a slice at a time, sometimes stopped part of the way and then finished
            if (rng.bounded(4) == 0) {
                std::atomic<bool> stop{false};
                b.calc->recalculate(1 + qint64(rng.bounded(30)), &stop);
                if (rng.bounded(2)) {
                    stop = true;
                    b.calc->recalculate(0, &stop);
                }
            }
            QVERIFY(b.calc->recalculate().finished);
            const QStringList incremental = snapshot();
            b.calc->invalidateAll();
            QVERIFY(b.calc->recalculate().finished);
            const QStringList whole = snapshot();
            if (incremental != whole) {
                QString first;
                for (int i = 0; i < incremental.size() && first.isEmpty(); ++i)
                    if (incremental[i] != whole[i]) first = QStringLiteral("%1: %2 after the edits, %3 after a whole recalculation").arg(name(i / cols, i % cols), incremental[i], whole[i]);
                QFAIL(qPrintable(QStringLiteral("after edit %1 the values differ from a whole recalculation: %2").arg(step).arg(first)));
            }
        }
    }

    // The same with circles: formulas that read cells and blocks of cells wherever they are, edited so that circles are made, joined, and broken, are the same after each
    // recalculation (the value of each cell, and whether it is in a circle) as they are after working out every formula again. A circle is found by what the formulas read, with
    // the values of what is not in it, not by the order the work was done in.
    void circlesMadeAndBrokenLeaveWhatAWholeRecalculationGives()
    {
        const int rows = 4, cols = 4;
        qint64 circles = 0;
        for (int seed = 0; seed < 4; ++seed) {
            QRandomGenerator rng(quint32(20261026 + seed));
            auto name = [&](int r, int c) { return QStringLiteral("%1%2").arg(QChar(QLatin1Char('A' + c))).arg(r + 1); };
            auto cell = [&]() { return name(int(rng.bounded(rows)), int(rng.bounded(cols))); };
            auto block = [&]() {
                const int r1 = int(rng.bounded(rows)), c1 = int(rng.bounded(cols));
                return name(r1, c1) + QLatin1Char(':') + name(qMin(rows - 1, r1 + int(rng.bounded(3))), qMin(cols - 1, c1 + int(rng.bounded(3))));
            };
            auto formulaFor = [&]() {
                switch (rng.bounded(18)) {
                case 0: return QStringLiteral("%1+1").arg(cell());
                case 1: return QStringLiteral("SUM(%1)").arg(block());
                case 2: return QStringLiteral("IF(ISNUMBER(%1),%1,%2)").arg(cell(), cell());
                case 3: return QStringLiteral("IF(%1=\"\",%2,1)").arg(cell(), cell());
                case 4: return QStringLiteral("MAX(%1)").arg(block());
                case 5: return QStringLiteral("IFERROR(%1,0)").arg(cell());
                case 6: return QStringLiteral("%1&\"a\"").arg(cell());
                case 7: return QStringLiteral("INDEX(%1,1,1)").arg(block());
                case 8: return QStringLiteral("COUNT(%1)+1").arg(block());
                case 9: return QStringLiteral("IF(%1>3,%2,2)").arg(cell(), cell());
                case 10: return QStringLiteral("INDIRECT(\"%1\")+1").arg(cell());
                case 11: return QStringLiteral("IF(%1<2,%2,%3)").arg(cell(), cell(), cell());
                case 12: return QStringLiteral("SUM(nCorner)+1");
                case 13: return QStringLiteral("MAX(nBlock)");
                case 14: return QStringLiteral("SUM(%1:INDEX(%2:%2,%3))").arg(cell(), QString(QChar(QLatin1Char('A' + int(rng.bounded(cols))))), QString::number(1 + int(rng.bounded(rows))));
                case 15: return QStringLiteral("IF(%1=\"\",%2,%3)").arg(cell(), cell(), cell());
                case 16: return QStringLiteral("SUMIF(%1,\">1\",%2)").arg(block(), cell());
                default: return QStringLiteral("MIN(%1,%2)").arg(cell(), cell());
                }
            };
            QString sheet;
            for (int r = 0; r < rows; ++r) {
                QString cellsXml;
                for (int c = 0; c < cols; ++c)
                    if (rng.bounded(4) == 0) cellsXml += constant(name(r, c), QString::number(int(rng.bounded(10))));
                    else cellsXml += cellXml(name(r, c), formulaFor());
                sheet += rowXml(r + 1, cellsXml);
            }
            Book b;
            QVERIFY(b.open(buildXlsx({worksheet(sheet)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(),
                                     QStringLiteral("<definedName name=\"nCorner\">Sheet1!$A$1</definedName><definedName name=\"nBlock\">Sheet1!$B$2:$C$3</definedName>"))));
            settle(b);
            auto snapshot = [&]() {
                QStringList all;
                for (int r = 0; r < rows; ++r)
                    for (int c = 0; c < cols; ++c) all << b.calc->cell(0, quint32(r), quint32(c)).debug() + (b.circular(name(r, c)) ? QStringLiteral("@circle") : QString());
                return all;
            };
            for (int step = 0; step < 700; ++step) {
                const int edits = 1 + int(rng.bounded(2));
                QStringList what;
                for (int e = 0; e < edits; ++e) {
                    const int r = int(rng.bounded(rows)), c = int(rng.bounded(cols));
                    switch (rng.bounded(4)) {
                    case 0: QVERIFY(b.set(name(r, c), Value::number(int(rng.bounded(10))))); what << name(r, c) + QStringLiteral(" number"); break;
                    case 1: {
                        const QString f = formulaFor();
                        QVERIFY(b.setFormula(name(r, c), f));
                        what << name(r, c) + QStringLiteral(" =") + f;
                        break;
                    }
                    case 2: QVERIFY(b.set(name(r, c), Value::text(QStringLiteral("t")))); what << name(r, c) + QStringLiteral(" text"); break;
                    default: QVERIFY(b.calc->clearCell(0, quint32(r), quint32(c))); what << name(r, c) + QStringLiteral(" cleared"); break;
                    }
                }
                if (rng.bounded(4) == 0) {
                    std::atomic<bool> stop{false};
                    b.calc->recalculate(1 + qint64(rng.bounded(30)), &stop);
                    if (rng.bounded(2)) {
                        stop = true;
                        b.calc->recalculate(0, &stop);
                    }
                }
                QVERIFY(b.calc->recalculate().finished);
                const QStringList incremental = snapshot();
                b.calc->invalidateAll();
                const Calculation::Progress whole = b.calc->recalculate();
                QVERIFY(whole.finished);
                circles += whole.circular;
                const QStringList fresh = snapshot();
                if (incremental != fresh) {
                    QString first;
                    for (int i = 0; i < incremental.size() && first.isEmpty(); ++i)
                        if (incremental[i] != fresh[i]) first = QStringLiteral("%1: %2 after the edits, %3 after a whole recalculation").arg(name(i / cols, i % cols), incremental[i], fresh[i]);
                    QFAIL(qPrintable(QStringLiteral("seed %1, after edit %2 (%3) the values differ from a whole recalculation: %4").arg(seed).arg(step).arg(what.join(QStringLiteral("; ")), first)));
                }
            }
        }
        QVERIFY2(circles > 500, qPrintable(QStringLiteral("only %1 cells were in circles").arg(circles)));   // (the edits do make circles)
    }

    // The range operator over references that functions give makes a block the formula reads whole: an edit to a cell inside it, which no reference of the formula names,
    // reaches the formula.
    void aRangeMadeOfReferencesThatFunctionsGiveIsToldToTheGraph()
    {
        const QString rows = rowXml(1, constant("A1", "1") + constant("B1", "2") + constant("C1", "3") + cellXml("E1", "SUM(A1:INDEX(C:C,3))") + cellXml("F1", "SUM(INDEX(A:A,1):INDEX(C:C,3))"))
            + rowXml(2, constant("A2", "10") + constant("B2", "20") + constant("C2", "30")) + rowXml(3, constant("A3", "100") + constant("B3", "200") + constant("C3", "300"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        settle(b);
        QCOMPARE(b.at("E1"), QStringLiteral("n:666"));
        QCOMPARE(b.at("F1"), QStringLiteral("n:666"));
        QVERIFY(b.set("B2", Value::number(1000)));   // inside the block, and in no column or cell the formulas name
        settle(b);
        QCOMPARE(b.at("E1"), QStringLiteral("n:1646"));
        QCOMPARE(b.at("F1"), QStringLiteral("n:1646"));
    }

    // SUMIF and AVERAGEIF with a sum range shorter than the range read the cells of the shape of the range, from the first of the sum range: those are what the formula reads. (The
    // others that take a range and a sum range, SUMIFS and the rest, give #VALUE! when the shapes differ, and read no more than they name.)
    void aSumRangeShorterThanTheRangeIsReadAsTheShapeOfTheRange()
    {
        const QString rows = rowXml(1, constant("A1", "1") + constant("B1", "10") + cellXml("D1", "SUMIF(A1:A3,\">0\",B1)") + cellXml("E1", "AVERAGEIF(A1:A3,\">0\",B1)") + cellXml("F1", "SUMIFS(B1,A1:A3,\">0\")")
                                           + cellXml("G1", "COUNTIFS(A1:A3,\">0\",B1:B2,\">0\")"))
            + rowXml(2, constant("A2", "2") + constant("B2", "20")) + rowXml(3, constant("A3", "3") + constant("B3", "30"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        settle(b);
        QCOMPARE(b.at("D1"), QStringLiteral("n:60"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:20"));
        QCOMPARE(b.at("F1"), QStringLiteral("e:#VALUE!"));
        QCOMPARE(b.at("G1"), QStringLiteral("e:#VALUE!"));
        QVERIFY(b.set("B3", Value::number(100)));   // read as the sum range's third cell, which is named by no reference
        settle(b);
        QCOMPARE(b.at("D1"), QStringLiteral("n:130"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:%1").arg(130.0 / 3.0, 0, 'g', 15));
        QCOMPARE(b.at("F1"), QStringLiteral("e:#VALUE!"));
        QCOMPARE(b.at("G1"), QStringLiteral("e:#VALUE!"));
    }

    // A defined name that calls a function that changes by itself makes the formulas that use it change by themselves: they are worked out in every recalculation, as a formula
    // that calls RAND is, through a name of a name too; and kept apart, they are the volatile ones.
    void aNameThatCallsAVolatileFunctionMakesItsFormulasVolatile()
    {
        const QString rows = rowXml(1, constant("A1", "5") + cellXml("B1", "tn+A1", "1") + cellXml("C1", "tm*2+A1", "1") + cellXml("D1", "A1+1", "6") + cellXml("E1", "SUM(tp)", "5"));
        const QString names = QStringLiteral("<definedName name=\"tn\">RAND()</definedName><definedName name=\"tm\">tn</definedName><definedName name=\"tp\">Sheet1!$A$1:$A$2</definedName>");
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), names)));
        settle(b);
        const QString b1 = b.at("B1"), c1 = b.at("C1");
        const Calculation::Progress p = settle(b);   // nothing was edited
        QVERIFY2(p.evaluated >= 2, qPrintable(QStringLiteral("%1 formulas worked out again").arg(p.evaluated)));
        QVERIFY(b.at("B1") != b1);
        QVERIFY(b.at("C1") != c1);
        QCOMPARE(b.at("D1"), QStringLiteral("n:6"));
        CalcOptions apart;
        apart.keepApart = true;
        Book k;
        QVERIFY(k.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), names), apart));
        settle(k);
        QCOMPARE(k.calc->keptClass(0, k.row("B1"), k.col("B1")), 'V');
        QCOMPARE(k.calc->keptClass(0, k.row("C1"), k.col("C1")), 'V');
        QCOMPARE(k.calc->keptClass(0, k.row("D1"), k.col("D1")), char(0));
        QCOMPARE(k.calc->keptClass(0, k.row("E1"), k.col("E1")), char(0));
    }

    // A circle through a name is broken by an edit to what the name stands for: the formula that closed the circle told what it read through the name, though the circle stopped
    // its work, so the edit reaches it.
    void aCircleThroughANameIsBrokenByAnEditToWhatItReads()
    {
        const QString rows = rowXml(1, cellXml("A1", "D1+1") + cellXml("D1", "SUM(me)"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QStringLiteral("<definedName name=\"me\">Sheet1!$A$1</definedName>"))));
        settle(b);
        QVERIFY(b.circular("A1") && b.circular("D1"));
        QVERIFY(b.set("A1", Value::number(5)));
        settle(b);
        QCOMPARE(b.at("A1"), QStringLiteral("n:5"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:5"));
        QVERIFY(!b.circular("D1"));
    }

    // What is in a circle is found from what the formulas read, with the values of what is not in it: a formula that reads a cell only when another is empty is no part of a circle
    // when the other holds something, however the work happens to be ordered; and when circles overlap, every formula of them is marked.
    void aCircleIsFoundByWhatIsReadAndEveryFormulaOfItIsMarked()
    {
        // A1 reads C1 only if B1 is empty, and B1 holds the text of D1 (the formula "x")
        const QString rows = rowXml(1, cellXml("A1", "IF(B1=\"\",C1,1)") + cellXml("B1", "D1") + cellXml("C1", "A1+1") + cellXml("D1", "\"x\""))
            + rowXml(3, cellXml("A3", "IF(ISNUMBER(B3),B3,C3)") + cellXml("B3", "A4*2") + cellXml("C3", "A3+1") + constant("A4", "5"))
            // the cycles A6-B6 and A6-C6 share A6; D6 reads them and is in none
            + rowXml(6, cellXml("A6", "B6+C6") + cellXml("B6", "A6") + cellXml("C6", "A6") + cellXml("D6", "A6+B6+C6+1"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        settle(b);
        QCOMPARE(b.at("A1"), QStringLiteral("n:1"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:2"));
        QCOMPARE(b.at("A3"), QStringLiteral("n:10"));
        QCOMPARE(b.at("C3"), QStringLiteral("n:11"));
        for (const char *ref : {"A1", "B1", "C1", "A3", "B3", "C3", "D6"}) QVERIFY2(!b.circular(QString::fromLatin1(ref)), ref);
        for (const char *ref : {"A6", "B6", "C6"}) QVERIFY2(b.circular(QString::fromLatin1(ref)), ref);
        QCOMPARE(b.at("D6"), QStringLiteral("n:1"));
        // B5 reads the block that holds it, and the first cell of the block is an error: the error is the result, and nothing is read in a circle; whichever edit comes first
        const QString block = rowXml(1, cellXml("A1", "IF(X1=1,NA(),2)") + constant("X1", "0")) + rowXml(5, cellXml("B5", "MAX(A1:D6)")) + rowXml(6, constant("D6", "5"));
        for (int order = 0; order < 2; ++order) {
            Book c;
            QVERIFY(c.open(buildXlsx({worksheet(block)})));
            settle(c);
            QVERIFY(c.circular("B5"));            // (A1 is 2, and B5 reads itself)
            if (order == 0) {
                QVERIFY(c.set("D6", Value::number(6)));
                QVERIFY(c.set("X1", Value::number(1)));
            } else {
                QVERIFY(c.set("X1", Value::number(1)));
                QVERIFY(c.set("D6", Value::number(6)));
            }
            settle(c);
            QCOMPARE(c.at("B5"), QStringLiteral("e:#N/A"));
            QVERIFY(!c.circular("B5"));
        }
    }

    // A circle of a hundred thousand formulas, one of them reading a block that holds all the others, is found in time in proportion to its size (counted), not by going over the
    // others again for each.
    void aLongCircleIsFoundInTimeInProportionToItsSize()
    {
        const int n = 60000;
        QString rows;
        for (int i = 1; i < n; ++i) rows += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("A%1+1").arg(i + 1)));
        rows += rowXml(n, cellXml(QStringLiteral("A%1").arg(n), QStringLiteral("SUM(A1:A%1)").arg(n - 1)));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        const Calculation::Progress p = settle(b);
        QVERIFY(p.finished);
        QCOMPARE(p.circular, qint64(n));
        QVERIFY2(p.circleWork <= 10 * qint64(n), qPrintable(QStringLiteral("%1 steps to find a circle of %2").arg(p.circleWork).arg(n)));
        QVERIFY2(p.evaluated <= 4 * qint64(n), qPrintable(QString::number(p.evaluated)));
        QVERIFY(b.circular("A1") && b.circular(QStringLiteral("A%1").arg(n)));
        QVERIFY(b.set(QStringLiteral("A%1").arg(n), Value::number(5)));   // broken at the end: the others are worked out
        const Calculation::Progress q = settle(b);
        QCOMPARE(b.at("A1"), QStringLiteral("n:%1").arg(5 + n - 1));
        QVERIFY2(q.evaluated <= 4 * qint64(n), qPrintable(QString::number(q.evaluated)));
        QVERIFY(!b.circular("A1"));
    }

    // A volatile formula edited over and over is on the list of those worked out in every recalculation once, and a formula that is not volatile any more is not on it.
    void aVolatileFormulaEditedManyTimesIsListedOnce()
    {
        const QString rows = rowXml(1, constant("A1", "1") + cellXml("B1", "RAND()") + cellXml("C1", "NOW()"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        settle(b);
        for (int i = 0; i < 2000; ++i) QVERIFY(b.setFormula("B1", QStringLiteral("RAND()+%1").arg(i)));
        settle(b);
        QVERIFY2(b.calc->alwaysListed() <= 2, qPrintable(QString::number(b.calc->alwaysListed())));
        QVERIFY(b.setFormula("B1", "1+1"));
        QVERIFY(b.setFormula("C1", "A1+1"));
        settle(b);
        QCOMPARE(b.calc->alwaysListed(), qint64(0));
        const Calculation::Progress p = settle(b);
        QCOMPARE(p.evaluated, qint64(0));
    }

    // A graph that is full at a realistic size (thirty thousand formulas, seventeen cells each, a graph of about half that) says no at once to the rest: it is not gone over
    // again for each of the formulas it refuses (counted), and every value is still right.
    void aFullGraphAtARealisticSizeIsNotGoneOverForEachFormulaItRefuses()
    {
        const int n = 30000, k = 17;
        QString rows;
        for (int i = 1; i <= n; ++i) {
            QString f;
            for (int j = 0; j < k; ++j) f += (j ? QStringLiteral("+") : QString()) + QStringLiteral("B%1").arg(1 + ((i + j * 37) % n));
            rows += rowXml(i, constant(QStringLiteral("B%1").arg(i), "1") + cellXml(QStringLiteral("A%1").arg(i), f));
        }
        CalcOptions options;
        options.maxGraphBytes = qint64(n) * k * 12;
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}), options));
        QVERIFY2(b.calc->untrackedCount() > n / 4 && b.calc->untrackedCount() < n, qPrintable(QString::number(b.calc->untrackedCount())));
        QVERIFY2(b.calc->graph().compactions() <= 3, qPrintable(QStringLiteral("%1 compactions").arg(b.calc->graph().compactions())));
        QVERIFY(b.calc->graph().memoryBytes() <= options.maxGraphBytes + options.maxGraphBytes / 8);
        settle(b);
        QCOMPARE(b.at("A7"), QStringLiteral("n:%1").arg(k));
        QVERIFY(b.set("B5", Value::number(7)));
        settle(b);
        QCOMPARE(b.at("A4"), QStringLiteral("n:%1").arg(k + 6));   // (A4 reads B5)
        QCOMPARE(b.at(QStringLiteral("A%1").arg(n)), QStringLiteral("n:%1").arg(k));
    }

    // What a formula reads through a name, INDIRECT, or OFFSET is told as it is worked out, and told again each time: a change to a cell it reads now works it out, one to a
    // cell it read before does not make more than the work it always has.
    void referencesThroughNamesAndIndirectAreToldWhenWorkedOut()
    {
        const QString rows = rowXml(1, constant("A1", "10") + constant("A2", "20") + constant("A3", "30") + constant("B1", "2") + cellXml("C1", "INDIRECT(\"A\"&B1)") + cellXml("D1", "SUM(Far)+B1") + cellXml("E1", "OFFSET(A1,B1,0)"))
            + rowXml(5, constant("C5", "100") + constant("C6", "200"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QStringLiteral("<definedName name=\"Far\">Sheet1!$C$5:$C$6</definedName>"))));
        settle(b);
        QCOMPARE(b.at("C1"), QStringLiteral("n:20"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:302"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:30"));
        auto readsCell = [&](const QString &ref, quint32 node) {
            std::vector<quint32> found;
            b.calc->graph().dependentsOfCell(0, b.row(ref), b.col(ref), &found);
            return std::find(found.begin(), found.end(), node) != found.end();
        };
        const quint32 d1 = b.calc->nodeAt(0, b.row("D1"), b.col("D1"));
        const quint32 c1 = b.calc->nodeAt(0, b.row("C1"), b.col("C1"));
        QVERIFY(readsCell("C6", d1));                       // through the name
        QVERIFY(readsCell("A2", c1));                       // through INDIRECT: where it pointed
        QVERIFY(!readsCell("A3", c1));
        QVERIFY(b.set("C6", Value::number(250)));
        QVERIFY(b.calc->dirtyCount() >= 1);
        settle(b);
        QCOMPARE(b.at("D1"), QStringLiteral("n:352"));
        QVERIFY(b.set("B1", Value::number(3)));
        settle(b);
        QCOMPARE(b.at("C1"), QStringLiteral("n:30"));        // the cell it points to now
        QCOMPARE(b.at("E1"), QStringLiteral("n:0"));         // OFFSET(A1,3,0) is A4, empty
        QVERIFY(readsCell("A3", c1));                        // told again
        QVERIFY(!readsCell("A2", c1));
        QVERIFY(b.set("A3", Value::number(31)));
        settle(b);
        QCOMPARE(b.at("C1"), QStringLiteral("n:31"));
    }

    // A shared formula is one program for all its cells, and a hundred thousand of them are a hundred thousand reads in the graph, not more; an edit finds the one that reads it.
    void sharedFormulasCostWhatTheirCellsCost()
    {
        const int n = 60000;
        QString rows;
        rows += rowXml(1, constant("A1", "1") + QStringLiteral("<c r=\"B1\"><f t=\"shared\" ref=\"B1:B%1\" si=\"0\">A1*2+$A$1</f></c>").arg(n));
        for (int i = 2; i <= n; ++i) rows += rowXml(i, constant(QStringLiteral("A%1").arg(i), QString::number(i)) + QStringLiteral("<c r=\"B%1\"><f t=\"shared\" si=\"0\"/></c>").arg(i));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QCOMPARE(b.calc->formulaCount(), qint64(n));
        QVERIFY2(b.calc->graph().entryCount() <= 2 * n + 10, qPrintable(QString::number(b.calc->graph().entryCount())));
        settle(b);
        QCOMPARE(b.at("B1"), QStringLiteral("n:3"));
        QCOMPARE(b.at("B30000"), QStringLiteral("n:60001"));
        QVERIFY(b.set("A30000", Value::number(5)));
        QCOMPARE(b.calc->dirtyCount(), qint64(1));
        const Calculation::Progress p = settle(b);
        QCOMPARE(p.evaluated, qint64(1));
        QCOMPARE(b.at("B30000"), QStringLiteral("n:11"));
        // the cell every one of them reads in common reaches them all, as many as there are
        QVERIFY(b.set("A1", Value::number(10)));
        QCOMPARE(b.calc->dirtyCount(), qint64(n));
        settle(b);
        QCOMPARE(b.at("B2"), QStringLiteral("n:14"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:30"));
    }

    // The graph holds no more than it may: the formulas past it are not tracked, are worked out each time, and every value is still right.
    void formulasPastTheGraphLimitAreWorkedOutEveryTime()
    {
        QString rows;
        for (int i = 1; i <= 400; ++i) rows += rowXml(i, constant(QStringLiteral("A%1").arg(i), QString::number(i)) + cellXml(QStringLiteral("B%1").arg(i), QStringLiteral("A%1*2").arg(i)) + cellXml(QStringLiteral("C%1").arg(i), QStringLiteral("B%1+1").arg(i)));
        CalcOptions options;
        options.maxGraphBytes = 30000;
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}), options));
        settle(b);
        QVERIFY(b.calc->untrackedCount() > 0);
        QVERIFY(b.calc->graph().memoryBytes() <= 30000 + 8192);
        QCOMPARE(b.at("C400"), QStringLiteral("n:801"));
        QVERIFY(b.set("A10", Value::number(1000)));
        const Calculation::Progress p = settle(b);
        QVERIFY(p.evaluated >= 2);
        QCOMPARE(b.at("C10"), QStringLiteral("n:2001"));
        QVERIFY(b.set("A399", Value::number(-1)));
        settle(b);
        QCOMPARE(b.at("C399"), QStringLiteral("n:-1"));
        QCOMPARE(b.at("C400"), QStringLiteral("n:801"));
    }

    // Kept apart: a formula the engine cannot work out keeps the result the file stored, and what reads it is marked; a circle of ordinary formulas is not marked.
    void whatTheEngineCannotWorkOutIsKeptApartAndMarksWhatReadsIt()
    {
        const QString rows = rowXml(1, constant("A1", "1") + cellXml("B1", "FOO(A1)", "7") + cellXml("C1", "B1+1", "0") + cellXml("D1", "C1*2", "0") + cellXml("E1", "A1+1", "0") + cellXml("F1", "NOW()", "45000")
                                           + cellXml("G1", "F1+1", "0") + cellXml("H1", "[1]Sheet1!A1", "5") + cellXml("I1", "SUM(H1,A1)", "0"));
        CalcOptions options;
        options.keepApart = true;
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}), options));
        settle(b);
        QCOMPARE(b.at("B1"), QStringLiteral("n:7"));         // as the file has it
        QCOMPARE(b.at("C1"), QStringLiteral("n:8"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:16"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:2"));
        QCOMPARE(b.at("F1"), QStringLiteral("n:45000"));
        QCOMPARE(b.at("G1"), QStringLiteral("n:45001"));
        QCOMPARE(b.at("I1"), QStringLiteral("n:6"));
        for (const char *ref : {"B1", "C1", "D1", "F1", "G1", "H1", "I1"}) QVERIFY2(b.tainted(QString::fromLatin1(ref)), ref);
        for (const char *ref : {"E1"}) QVERIFY2(!b.tainted(QString::fromLatin1(ref)), ref);
        QCOMPARE(b.calc->keptClass(0, b.row("B1"), b.col("B1")), 'U');
        QCOMPARE(b.calc->keptClass(0, b.row("F1"), b.col("F1")), 'V');
        QCOMPARE(b.calc->keptClass(0, b.row("H1"), b.col("H1")), 'E');
        QCOMPARE(b.calc->keptClass(0, b.row("C1"), b.col("C1")), char(0));
        // not kept apart, the engine works them out as it does: a function it lacks is an error
        Book c;
        QVERIFY(c.open(buildXlsx({worksheet(rows)})));
        settle(c);
        QCOMPARE(c.at("B1"), QStringLiteral("e:#NAME?"));
        QCOMPARE(c.at("C1"), QStringLiteral("e:#NAME?"));
    }

    // An array formula gives its cells their values, and what reads a cell of the array reads the formula; an edit to what the array reads reaches them.
    void arrayFormulaCellsAreValuesOfTheFormula()
    {
        const QString rows = rowXml(1, constant("A1", "1") + QStringLiteral("<c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">A1:A3*2</f><v>0</v></c>") + cellXml("D1", "C3+C2+1"))
            + rowXml(2, constant("A2", "2") + QStringLiteral("<c r=\"C2\"><v>0</v></c>")) + rowXml(3, constant("A3", "3") + QStringLiteral("<c r=\"C3\"><v>0</v></c>"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        settle(b);
        QCOMPARE(b.at("C1"), QStringLiteral("n:2"));
        QCOMPARE(b.at("C2"), QStringLiteral("n:4"));
        QCOMPARE(b.at("C3"), QStringLiteral("n:6"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:11"));
        QVERIFY(b.set("A3", Value::number(10)));
        QCOMPARE(b.calc->dirtyCount(), qint64(2));           // the array, and D1 that reads a cell of it
        const Calculation::Progress p = settle(b);
        QCOMPARE(p.evaluated, qint64(2));
        QCOMPARE(b.at("C3"), QStringLiteral("n:20"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:25"));
    }

    // An array that is kept apart, and a data table, give their cells the file's values, and what reads one of those cells is marked.
    void cellsOfWhatIsKeptApartInAnArrayAreTheFilesAndMarked()
    {
        const QString rows = rowXml(1, constant("A1", "1") + QStringLiteral("<c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">FOO(A1:A3)</f><v>7</v></c>") + cellXml("D1", "C2+1", "0")
                                           + QStringLiteral("<c r=\"F1\"><f t=\"dataTable\" ref=\"F1:F2\" r1=\"A1\"/><v>5</v></c>") + cellXml("G1", "F2*2", "0") + cellXml("H1", "A1+1", "0"))
            + rowXml(2, QStringLiteral("<c r=\"C2\"><v>8</v></c><c r=\"F2\"><v>6</v></c>")) + rowXml(3, QStringLiteral("<c r=\"C3\"><v>9</v></c>"));
        CalcOptions options;
        options.keepApart = true;
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}), options));
        settle(b);
        QCOMPARE(b.at("C2"), QStringLiteral("n:8"));
        QCOMPARE(b.at("D1"), QStringLiteral("n:9"));
        QVERIFY(b.tainted("D1"));
        QCOMPARE(b.at("G1"), QStringLiteral("n:12"));
        QVERIFY(b.tainted("G1"));
        QVERIFY(!b.tainted("H1"));
        QCOMPARE(b.calc->keptClass(0, b.row("C1"), b.col("C1")), 'U');
        QCOMPARE(b.calc->keptClass(0, b.row("F1"), b.col("F1")), 'D');
    }

    // Formulas of other sheets, three-dimensional references, and a formula that reads the cell it is on through another sheet.
    void formulasOfOtherSheetsAreWorkedOutInOrder()
    {
        const QString one = rowXml(1, constant("A1", "1") + cellXml("B1", "Sheet2!A1+1")) + rowXml(2, constant("A2", "2"));
        const QString two = rowXml(1, cellXml("A1", "Sheet3!A1*10")) + rowXml(2, cellXml("A2", "SUM(Sheet1:Sheet3!A1)+Sheet1!B1"));
        const QString three = rowXml(1, constant("A1", "4"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(one), worksheet(two), worksheet(three)})));
        settle(b);
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:40"));
        QCOMPARE(b.at("B1", 0), QStringLiteral("n:41"));
        QCOMPARE(b.at("A2", 1), QStringLiteral("n:%1").arg(1 + 40 + 4 + 41));
        QVERIFY(b.set("A1", Value::number(2), 2));
        settle(b);
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:20"));
        QCOMPARE(b.at("B1", 0), QStringLiteral("n:21"));
        QCOMPARE(b.at("A2", 1), QStringLiteral("n:%1").arg(1 + 20 + 2 + 21));
    }

    // A walk over a whole column of a sheet of many rows is work, counted: a thousand formulas that sum a column holding a hundred numbers, in a sheet of three thousand rows,
    // take steps in proportion to the rows they walk over, not to the hundred cells they find (forty thousand of them took twenty seconds, and counted a few millions).
    void aWalkOverAWholeColumnIsWorkThatIsCounted()
    {
        const int rows = 3000;
        QString sheet;
        for (int i = 1; i <= rows; ++i) sheet += rowXml(i, (i <= 100 ? constant(QStringLiteral("A%1").arg(i), "1") : QString()) + cellXml(QStringLiteral("C%1").arg(i), "SUM(A:A)"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(sheet)})));
        const Calculation::Progress p = settle(b);
        QVERIFY(p.finished);
        QCOMPARE(b.at("C1"), QStringLiteral("n:100"));
        QVERIFY2(p.work >= qint64(rows) * rows / 8, qPrintable(QStringLiteral("%1 steps for %2 sums of a column of %3 rows").arg(p.work).arg(rows).arg(rows)));
    }

    // A recalculation has a bound on the steps it takes in all (counted, whatever its slices): what is not worked out by then has the error #VALUE!, and the next recalculation,
    // after an edit, has the whole bound again. Formulas that wait for others over and over (nested INDIRECT) are worked out again each time, and are held to it too.
    void aRecalculationHasABoundOnItsTotalWork()
    {
        const int n = 400;
        QString rows;
        for (int i = 1; i <= 1000; ++i) rows += rowXml(i, constant(QStringLiteral("A%1").arg(i), "2") + (i <= n ? cellXml(QStringLiteral("C%1").arg(i), QStringLiteral("SUMPRODUCT(A1:A1000*A1:A1000)+%1").arg(i)) : QString()));
        CalcOptions options;
        options.recalcWorkLimit = 200000;
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}), options));
        const Calculation::Progress p = settle(b);
        QVERIFY(p.finished);
        QVERIFY2(p.overBudget, qPrintable(QStringLiteral("%1 steps, none refused").arg(p.work)));
        QVERIFY2(p.work <= options.recalcWorkLimit + 20000, qPrintable(QString::number(p.work)));
        QVERIFY2(p.refused > 0 && p.refused < n, qPrintable(QString::number(p.refused)));
        QCOMPARE(b.at("C1"), QStringLiteral("n:%1").arg(4000 + 1));
        QCOMPARE(b.at(QStringLiteral("C%1").arg(n)), QStringLiteral("e:#VALUE!"));
        QVERIFY(!b.calc->isDirty());
        // the same in slices: the same bound for all of them
        Book c;
        QVERIFY(c.open(buildXlsx({worksheet(rows)}), options));
        qint64 work = 0, refused = 0;
        bool over = false;
        for (int slices = 0; slices < 100000; ++slices) {
            const Calculation::Progress q = c.calc->recalculate(5000);
            work += q.work;
            refused += q.refused;
            over = over || q.overBudget;
            if (q.finished) break;
        }
        QVERIFY2(over && work <= options.recalcWorkLimit + 20000, qPrintable(QStringLiteral("%1 steps in slices").arg(work)));
        QCOMPARE(refused, p.refused);
        QCOMPARE(c.at(QStringLiteral("C%1").arg(n)), QStringLiteral("e:#VALUE!"));
        // an edit starts a recalculation with the whole bound
        QVERIFY(c.set("A1", Value::number(3)));
        const Calculation::Progress r = settle(c);
        QVERIFY(r.finished && r.overBudget && r.refused > 0 && r.refused < n);
        qint64 worked = 0, errors = 0;
        for (int i = 1; i <= n; ++i) {
            const QString v = c.at(QStringLiteral("C%1").arg(i));
            if (v == QStringLiteral("e:#VALUE!")) ++errors;
            else if (v == QStringLiteral("n:%1").arg(4005 + i)) ++worked;   // 9 and 999 fours, and the row
        }
        QVERIFY2(worked > 0 && errors == r.refused && worked + errors == n, qPrintable(QStringLiteral("%1 worked out, %2 with the error, %3 refused").arg(worked).arg(errors).arg(r.refused)));
        // formulas that read where a text says, nested forty deep, over a formula that takes many steps, are worked out again after each: the bound is on all of them
        QString chain;
        for (int i = 1; i <= 1000; ++i) chain += rowXml(i, constant(QStringLiteral("A%1").arg(i), "2") + (i <= 40 ? cellXml(QStringLiteral("B%1").arg(i), QStringLiteral("\"B%1\"").arg(i + 1)) : QString()));
        QString nest = QStringLiteral("\"B1\"");
        for (int i = 0; i < 40; ++i) nest = QStringLiteral("INDIRECT(%1)").arg(nest);
        chain += rowXml(1001, cellXml("D1", QStringLiteral("SUMPRODUCT(A1:A1000*A1:A1000)*0+LEN(%1)").arg(nest)));
        CalcOptions tight;
        tight.recalcWorkLimit = 20000;
        Book d;
        QVERIFY(d.open(buildXlsx({worksheet(chain)}), tight));
        const Calculation::Progress t = settle(d);
        QVERIFY2(t.finished && t.overBudget && t.work <= tight.recalcWorkLimit + 20000, qPrintable(QStringLiteral("%1 steps, %2 worked out").arg(t.work).arg(t.evaluated)));
    }

    // The texts and arrays that formulas gave are held to a bound for the workbook: a formula whose value would pass it has the error #VALUE! (and the work goes on), and an array
    // formula of one cell holds one value, not the array its formula made.
    void whatFormulasHoldIsHeldToABound()
    {
        const int n = 3000;
        QString rows;
        for (int i = 1; i <= n; ++i) rows += rowXml(i, cellXml(QStringLiteral("A%1").arg(i), QStringLiteral("REPT(\"x\",30000)&%1").arg(i)));
        CalcOptions options;
        options.maxHeldBytes = qint64(40) << 20;
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)}), options));
        const Calculation::Progress p = settle(b);
        QVERIFY(p.finished);
        QVERIFY2(b.calc->heldBytes() <= options.maxHeldBytes, qPrintable(QStringLiteral("%1 MB held of %2").arg(b.calc->heldBytes() >> 20).arg(options.maxHeldBytes >> 20)));
        QVERIFY2(p.refused > n / 2 && p.refused < n, qPrintable(QString::number(p.refused)));
        QCOMPARE(b.calc->cell(0, 0, 0).text().size(), 30001);
        QCOMPARE(b.at(QStringLiteral("A%1").arg(n)), QStringLiteral("e:#VALUE!"));
        // one-cell array formulas, each made of an array of a million cells
        QString arrays;
        for (int i = 1; i <= 40; ++i) arrays += rowXml(i, QStringLiteral("<c r=\"A%1\"><f t=\"array\" ref=\"A%1\">ROW(INDIRECT(\"1:1000000\"))</f></c>").arg(i));
        Book c;
        QVERIFY(c.open(buildXlsx({worksheet(arrays)})));
        const Calculation::Progress q = settle(c);
        QVERIFY(q.finished);
        QCOMPARE(c.at("A1"), QStringLiteral("n:1"));
        QCOMPARE(c.at("A40"), QStringLiteral("n:1"));
        QVERIFY2(c.calc->heldBytes() <= (qint64(1) << 20), qPrintable(QStringLiteral("%1 bytes held").arg(c.calc->heldBytes())));
        QCOMPARE(q.refused, qint64(0));
        // an array formula of a range keeps what the range holds, the rest of its array is cut
        const QString range = rowXml(1, QStringLiteral("<c r=\"A1\"><f t=\"array\" ref=\"A1:B2\">ROW(INDIRECT(\"1:50\"))*COLUMN(A1:C1)</f></c>"));
        Book d;
        QVERIFY(d.open(buildXlsx({worksheet(range)})));
        settle(d);
        QCOMPARE(d.at("B1"), QStringLiteral("n:2"));
        QCOMPARE(d.at("A2"), QStringLiteral("n:2"));
        QCOMPARE(d.at("B2"), QStringLiteral("n:4"));
        QVERIFY2(d.calc->heldBytes() <= 2048, qPrintable(QString::number(d.calc->heldBytes())));
    }
};

JP_TEST_CLASS(SheetCalculationTests)
#include "tst_sheet_calculation.moc"
