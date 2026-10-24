// Tests of the rules of validation in the calculation: which rule is for a cell, and whether an entry is allowed by whole-number, decimal, date, time, length, list, and custom rules, with
// the limits written as numbers, cells, and formulas. Every workbook is built in memory.
#include "sheethelpers.h"

#include "sheet/calculation.h"

#include <memory>

namespace {

struct Rules {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool open(const QString &rows, const QString &rules)
    {
        QString why;
        if (!package.readLazy(buildXlsx({worksheet(rows, QStringLiteral("<dataValidations count=\"20\">") + rules + QStringLiteral("</dataValidations>"))}), &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.keepApart = true;
        calc.reset(new Calculation(book, options));
        if (!calc->load()) return false;
        while (!calc->recalculate().finished) {}
        return true;
    }
    bool allows(const QString &ref, const Value &entry, const QString &typed = QString()) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        const Validation *rule = calc->validationAt(0, r, c);
        return !rule || calc->entryAllowed(0, r, c, *rule, entry, typed);
    }
};

QString rule(const QString &type, const QString &op, const QString &sqref, const QString &f1, const QString &f2 = QString(), const QString &more = QString())
{
    return QStringLiteral("<dataValidation type=\"%1\" operator=\"%2\" sqref=\"%3\" showErrorMessage=\"1\" %6><formula1>%4</formula1>%5</dataValidation>")
        .arg(type, op, sqref, f1.toHtmlEscaped(), f2.isEmpty() ? QString() : QStringLiteral("<formula2>%1</formula2>").arg(f2.toHtmlEscaped()), more);
}

} // namespace

class SheetValidateTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Every comparison, on whole numbers, decimals, dates, and times; limits that are numbers, cells, and formulas; and what is not a number.
    void numbersDatesAndTimesAreCompared()
    {
        Rules r;
        const QString cells = QStringLiteral("<row r=\"1\"><c r=\"Z1\"><v>5</v></c><c r=\"Z2\"><v>20</v></c></row>");
        QVERIFY(r.open(cells, rule("whole", "between", "A1", "1", "10") + rule("whole", "notBetween", "A2", "1", "10") + rule("decimal", "equal", "A3", "2.5") + rule("decimal", "notEqual", "A4", "2.5")
                                 + rule("whole", "greaterThan", "A5", "$Z$1") + rule("whole", "lessThan", "A6", "$Z$1*2") + rule("decimal", "greaterThanOrEqual", "A7", "Z1+Z2")
                                 + rule("decimal", "lessThanOrEqual", "A8", "0") + rule("date", "between", "A9", "45000", "45100") + rule("time", "between", "A10", "0.25", "0.75")));
        auto n = [](double v) { return Value::number(v); };
        QVERIFY(r.allows("A1", n(1)) && r.allows("A1", n(10)) && !r.allows("A1", n(11)) && !r.allows("A1", n(0)) && !r.allows("A1", n(2.5)));
        QVERIFY(!r.allows("A1", Value::text("5")) && !r.allows("A1", Value::boolean(true)));
        QVERIFY(r.allows("A2", n(11)) && r.allows("A2", n(0)) && !r.allows("A2", n(5)));
        QVERIFY(r.allows("A3", n(2.5)) && !r.allows("A3", n(2.6)));
        QVERIFY(!r.allows("A4", n(2.5)) && r.allows("A4", n(3)));
        QVERIFY(r.allows("A5", n(6)) && !r.allows("A5", n(5)));
        QVERIFY(r.allows("A6", n(9)) && !r.allows("A6", n(10)));
        QVERIFY(r.allows("A7", n(25)) && !r.allows("A7", n(24.9)));
        QVERIFY(r.allows("A8", n(-1)) && r.allows("A8", n(0)) && !r.allows("A8", n(0.1)));
        QVERIFY(r.allows("A9", n(45050)) && !r.allows("A9", n(44999)) && !r.allows("A9", n(45101)));
        QVERIFY(r.allows("A10", n(0.5)) && !r.allows("A10", n(0.9)));
        // a cell with no rule allows all
        QVERIFY(r.allows("C1", Value::text("anything")));
    }

    // Text length, lists of items and of cells, a blank, and a custom formula worked out with the entry in the cell.
    void lengthsListsBlanksAndFormulas()
    {
        Rules r;
        const QString cells = QStringLiteral("<row r=\"1\"><c r=\"F1\" t=\"inlineStr\"><is><t>one</t></is></c><c r=\"G1\"><v>2</v></c><c r=\"H1\"><v>10</v></c></row>"
                                             "<row r=\"2\"><c r=\"F2\" t=\"inlineStr\"><is><t>Two</t></is></c><c r=\"H2\"><v>20</v></c></row><row r=\"3\"><c r=\"F3\"><v>3</v></c><c r=\"H3\"><v>30</v></c></row>");
        QVERIFY(r.open(cells, rule("textLength", "lessThanOrEqual", "A1", "3") + rule("list", "between", "A2", "\"x,y, z\"") + rule("list", "between", "A3", "$F$1:$F$3", QString(), "allowBlank=\"1\"")
                                 + rule("custom", "between", "A4", "A4>SUM($H$1:$H$3)") + rule("custom", "between", "A5", "ISNUMBER(MATCH(A5,$H$1:$H$3,0))") + rule("list", "between", "A6", "$F$1:$F$3")));
        QVERIFY(r.allows("A1", Value::text("abc"), "abc") && !r.allows("A1", Value::text("abcd"), "abcd") && r.allows("A1", Value::number(123), "123") && !r.allows("A1", Value::number(1234), "1234"));
        QVERIFY(r.allows("A2", Value::text("x"), "x") && r.allows("A2", Value::text("z"), "z") && !r.allows("A2", Value::text("X"), "X") && !r.allows("A2", Value::text("w"), "w"));
        QVERIFY(r.allows("A3", Value::text("ONE"), "ONE") && r.allows("A3", Value::text("two"), "two") && r.allows("A3", Value::number(3), "3") && !r.allows("A3", Value::text("four"), "four"));
        QVERIFY(r.allows("A3", Value()));                       // blank allowed
        QVERIFY(!r.allows("A6", Value()));                      // and not
        QVERIFY(r.allows("A4", Value::number(61)) && !r.allows("A4", Value::number(60)));
        QVERIFY(r.allows("A5", Value::number(20)) && !r.allows("A5", Value::number(25)));
        const Validation *list = r.calc->validationAt(0, 2, 0);
        QCOMPARE(r.calc->listChoices(0, 2, 0, *list), (QStringList{"one", "Two", "3"}));
        QCOMPARE(r.calc->listChoices(0, 1, 0, *r.calc->validationAt(0, 1, 0)), (QStringList{"x", "y", "z"}));
    }

    // A rule's formulas are written for its first cell: for another cell of its range, the cells they name are moved the same distance.
    void relativeLimitsMoveWithTheCell()
    {
        Rules r;
        const QString cells = QStringLiteral("<row r=\"1\"><c r=\"B1\"><v>5</v></c></row><row r=\"2\"><c r=\"B2\"><v>50</v></c></row><row r=\"3\"><c r=\"B3\"><v>500</v></c></row>");
        QVERIFY(r.open(cells, rule("whole", "lessThan", "A1:A3", "B1")));
        QVERIFY(r.allows("A1", Value::number(4)) && !r.allows("A1", Value::number(5)));
        QVERIFY(r.allows("A2", Value::number(49)) && !r.allows("A2", Value::number(50)));
        QVERIFY(r.allows("A3", Value::number(499)) && !r.allows("A3", Value::number(500)));
    }
    // The rule for a cell is found without looking at every rule: with thousands of rules the ranges looked at for a cell are a few (those in the same rows), a rule over a whole column
    // is found, and of two rules over a cell the first of the file's order is the one.
    void theRuleForACellIsFoundWithoutLookingAtEveryRule()
    {
        const int n = 4000;
        QString rules = rule(QStringLiteral("whole"), QStringLiteral("between"), QStringLiteral("B1:B100"), QStringLiteral("1"), QStringLiteral("5"))                  // the first rule over B1:B100
                        + rule(QStringLiteral("whole"), QStringLiteral("between"), QStringLiteral("B50:B60"), QStringLiteral("100"), QStringLiteral("200"));            // another over some of the same
        for (int r = 1; r <= n; ++r) rules += rule(QStringLiteral("decimal"), QStringLiteral("greaterThan"), QStringLiteral("D%1").arg(r * 3), QStringLiteral("%1").arg(r));
        rules += rule(QStringLiteral("whole"), QStringLiteral("equal"), QStringLiteral("F1:F1048576"), QStringLiteral("7"));
        Rules r;
        QVERIFY(r.open(QString(), rules));
        QVERIFY(r.calc->validationAt(0, 55, 1));
        QCOMPARE(r.calc->validationAt(0, 55, 1)->formula1, QStringLiteral("1"));   // (the first of the two)
        QVERIFY(!r.calc->validationAt(0, 200, 1));
        QCOMPARE(r.calc->validationAt(0, 2999, 3)->formula1, QStringLiteral("1000"));   // D3000
        QVERIFY(!r.calc->validationAt(0, 3000, 3));                                    // D3001
        QCOMPARE(r.calc->validationAt(0, 900000, 5)->formula1, QStringLiteral("7"));   // F900001, a rule over the whole column
        QVERIFY(!r.calc->validationAt(0, 900000, 6));
        QVERIFY(r.allows(QStringLiteral("B2"), Value::number(3)));
        QVERIFY(!r.allows(QStringLiteral("B2"), Value::number(30)));
        const qint64 before = r.calc->ruleLookupWork();
        for (int row = 0; row < 12000; ++row) r.calc->validationAt(0, quint32(row), 3);
        const qint64 work = r.calc->ruleLookupWork() - before;
        QVERIFY2(work < 12000 * 30, qPrintable(QStringLiteral("%1 ranges looked at for 12,000 cells with %2 rules").arg(work).arg(n)));
    }

    // Rules that overlap much: thousands of rules, each over a block of columns and many rows, so that a cell is in hundreds of them and a block of 64 rows is in thousands. The rule found
    // for a cell is the first of the file's order that names it (as a check by looking at every rule says), and what is looked at for a cell does not grow with the rules that overlap there
    // (counted: about seventy steps for 1,500 rules and for 6,000, where looking down the rules of the rows took 6,600).
    void theRuleForACellIsFoundInAFewStepsWhateverOverlapsThere()
    {
        struct Box {
            quint32 row1, row2, col1, col2, row3, row4, col3, col4;   // the second range (0, 0, 0, 0 if none)
        };
        auto build = [&](int n, std::vector<Box> *boxes) {
            QString rules;
            for (int i = 0; i < n; ++i) {
                Box b{};
                b.col1 = quint32(i % 90);
                b.col2 = b.col1 + 40 + quint32(i % 7);
                b.row1 = quint32((i % 13) * 50);
                b.row2 = b.row1 + 4000 + quint32(i % 1000);
                QString sqref = rangeName(CellRange{b.row1, b.col1, b.row2, b.col2, true});
                if (i % 10 == 0) {
                    b.row3 = 10;
                    b.row4 = 90000 + quint32(i);
                    b.col3 = 150 + quint32(i % 5);
                    b.col4 = b.col3 + 3;
                    sqref += QLatin1Char(' ') + rangeName(CellRange{b.row3, b.col3, b.row4, b.col4, true});
                }
                boxes->push_back(b);
                rules += rule(QStringLiteral("decimal"), QStringLiteral("greaterThan"), sqref, QString::number(i));
            }
            return rules;
        };
        qint64 mostWork[2] = {0, 0};
        const int sizes[2] = {1500, 6000};
        for (int which = 0; which < 2; ++which) {
            std::vector<Box> boxes;
            Rules r;
            QVERIFY(r.open(QString(), build(sizes[which], &boxes)));
            QRandomGenerator rng(7 + quint32(which));
            for (int trial = 0; trial < 3000; ++trial) {
                const quint32 row = rng.bounded(rng.bounded(3) ? 6000u : 100000u), col = rng.bounded(rng.bounded(4) ? 140u : 170u);
                int expected = -1;
                for (int i = 0; i < int(boxes.size()) && expected < 0; ++i) {
                    const Box &b = boxes[size_t(i)];
                    if ((row >= b.row1 && row <= b.row2 && col >= b.col1 && col <= b.col2) || (b.col4 > 0 && row >= b.row3 && row <= b.row4 && col >= b.col3 && col <= b.col4)) expected = i;
                }
                const qint64 before = r.calc->ruleLookupWork();
                const Validation *found = r.calc->validationAt(0, row, col);
                mostWork[which] = std::max(mostWork[which], r.calc->ruleLookupWork() - before);
                if (expected < 0) QVERIFY2(!found, qPrintable(QStringLiteral("rule %1 found for %2,%3, and none is there").arg(found ? found->formula1 : QString()).arg(row).arg(col)));
                else QVERIFY2(found && found->formula1 == QString::number(expected), qPrintable(QStringLiteral("%1,%2 should have rule %3, and has %4").arg(row).arg(col).arg(expected).arg(found ? found->formula1 : QStringLiteral("none"))));
            }
        }
        QVERIFY2(mostWork[1] <= 200, qPrintable(QStringLiteral("%1 steps for a cell with %2 rules").arg(mostWork[1]).arg(sizes[1])));
        QVERIFY2(mostWork[1] <= mostWork[0] + 60, qPrintable(QStringLiteral("%1 steps with %2 rules, and %3 with %4").arg(mostWork[1]).arg(sizes[1]).arg(mostWork[0]).arg(sizes[0])));
    }
};

JP_TEST_CLASS(SheetValidateTests)
#include "tst_sheet_validate.moc"
