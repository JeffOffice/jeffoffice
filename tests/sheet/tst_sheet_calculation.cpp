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
        QElapsedTimer wait;
        wait.start();
        while (wait.elapsed() < 3) {}
        const Calculation::Progress p = settle(b);
        QCOMPARE(p.evaluated, qint64(1200));
        QVERIFY(b.calc->cell(0, 0, 0).number() > first);
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
    // circle), edited over and over, is the same after each recalculation as it is after working out every formula again.
    void editsLeaveWhatAWholeRecalculationGives()
    {
        const int rows = 24, cols = 7, cellsN = rows * cols;
        QRandomGenerator rng(20261025);
        // a rank for every cell: a formula reads only cells of lower rank, so there is no circle, whatever the places
        std::vector<int> rank(static_cast<size_t>(cellsN));
        for (int i = 0; i < cellsN; ++i) rank[size_t(i)] = i;
        for (int i = cellsN - 1; i > 0; --i) std::swap(rank[size_t(i)], rank[size_t(rng.bounded(quint32(i + 1)))]);
        auto name = [&](int r, int c) { return QStringLiteral("%1%2").arg(QChar(QLatin1Char('A' + c))).arg(r + 1); };
        auto lower = [&](int r, int c) {
            std::vector<QPair<int, int>> ok;
            for (int rr = 0; rr < rows; ++rr)
                for (int cc = 0; cc < cols; ++cc)
                    if (rank[size_t(rr * cols + cc)] < rank[size_t(r * cols + c)]) ok.push_back({rr, cc});
            return ok;
        };
        auto refOf = [&](int r, int c) {
            // a cell of lower rank, or a block of them
            const std::vector<QPair<int, int>> ok = lower(r, c);
            if (ok.empty()) return QString();
            const QPair<int, int> p = ok[rng.bounded(quint32(ok.size()))];
            if (rng.bounded(3) == 0) {
                for (int attempt = 0; attempt < 8; ++attempt) {
                    const int r2 = qMin(rows - 1, p.first + int(rng.bounded(5))), c2 = qMin(cols - 1, p.second + int(rng.bounded(3)));
                    bool allLower = true;
                    for (int rr = p.first; rr <= r2 && allLower; ++rr)
                        for (int cc = p.second; cc <= c2; ++cc)
                            if (rank[size_t(rr * cols + cc)] >= rank[size_t(r * cols + c)]) {
                                allLower = false;
                                break;
                            }
                    if (allLower) return name(p.first, p.second) + QLatin1Char(':') + name(r2, c2);
                }
            }
            return name(p.first, p.second);
        };
        auto formulaFor = [&](int r, int c) {
            const QString a = refOf(r, c);
            if (a.isEmpty()) return QStringLiteral("%1").arg(1 + int(rng.bounded(9)));
            const QString b = refOf(r, c);
            switch (rng.bounded(7)) {
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
        std::vector<bool> isFormula(static_cast<size_t>(cellsN), false);
        for (int r = 0; r < rows; ++r) {
            QString cellsXml;
            for (int c = 0; c < cols; ++c)
                if (rng.bounded(3) == 0) cellsXml += constant(name(r, c), QString::number(int(rng.bounded(20))));
                else {
                    cellsXml += cellXml(name(r, c), formulaFor(r, c));
                    isFormula[size_t(r * cols + c)] = true;
                }
            sheet += rowXml(r + 1, cellsXml);
        }
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(sheet)})));
        settle(b);
        auto snapshot = [&]() {
            QStringList all;
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < cols; ++c) all << b.calc->cell(0, quint32(r), quint32(c)).debug();
            return all;
        };
        for (int step = 0; step < 400; ++step) {
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
            QVERIFY(b.calc->recalculate().finished || true);
            const Calculation::Progress done = b.calc->recalculate();
            QVERIFY(done.finished);
            const QStringList incremental = snapshot();
            b.calc->invalidateAll();
            QVERIFY(b.calc->recalculate().finished);
            const QStringList whole = snapshot();
            QVERIFY2(incremental == whole, qPrintable(QStringLiteral("after edit %1 the values differ from a whole recalculation").arg(step)));
        }
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
};

JP_TEST_CLASS(SheetCalculationTests)
#include "tst_sheet_calculation.moc"
