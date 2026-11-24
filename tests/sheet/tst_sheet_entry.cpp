// Tests of reading what a person types into a cell: numbers, percents, money, dates, and times in the person's locale, with the number format that shows them as typed; formulas
// (and the plain message for one that is not); text and its limits; and the text a cell is edited as.
#include "sheethelpers.h"
#include "../localepin.h"

#include "grid/text.h"
#include "calc/functions.h"
#include "calc/numfmt.h"
#include "calc/formula.h"
#include "sheet/entry.h"
#include "sheet/fnnames.h"
#include "sheet/styles.h"

#include <QCoreApplication>
#include <QSet>

#include <algorithm>
#include <QTranslator>

namespace {

const QLocale en(QLocale::English, QLocale::UnitedStates);
const QLocale gb(QLocale::English, QLocale::UnitedKingdom);
const QLocale de(QLocale::German, QLocale::Germany);
const QLocale fr(QLocale::French, QLocale::France);
const QLocale ja(QLocale::Japanese, QLocale::Japan);

EntryOptions options(const QLocale &l, DateSystem system = DateSystem::Date1900)
{
    EntryOptions o;
    o.locale = l;
    o.dateSystem = system;
    o.today = QDate(2025, 6, 15);
    return o;
}

double serial(int y, int m, int d, DateSystem system = DateSystem::Date1900)
{
    double s = 0;
    dateToSerial(y, m, d, system, &s);
    return s;
}

// What was typed as a line: "n:5" a number (with " @code" for its format), "t:text", "b:1", "e:#N/A", "f:formula", "-" nothing, "!" a problem.
QString read(const QString &typed, const EntryOptions &o)
{
    const EntryResult r = readEntry(typed, o);
    if (!r.ok) return QStringLiteral("!");
    const Entry &e = r.entry;
    switch (e.kind) {
    case Entry::Kind::Empty: return QStringLiteral("-");
    case Entry::Kind::Number: return QStringLiteral("n:") + QString::number(e.number, 'g', 15) + (e.formatCode.isEmpty() ? QString() : QStringLiteral(" @") + e.formatCode);
    case Entry::Kind::Text: return QStringLiteral("t:") + e.text;
    case Entry::Kind::Bool: return QStringLiteral("b:") + QString::number(int(e.number));
    case Entry::Kind::Error: return QStringLiteral("e:") + errorKindText(e.error);
    case Entry::Kind::Formula: return QStringLiteral("f:") + e.text;
    }
    return {};
}

QString n(double v, const QString &code = QString()) { return QStringLiteral("n:") + QString::number(v, 'g', 15) + (code.isEmpty() ? QString() : QStringLiteral(" @") + code); }

} // namespace

class SheetEntryTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void plainNumbers()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("5"), o), n(5));
        QCOMPARE(read(QStringLiteral("-5"), o), n(-5));
        QCOMPARE(read(QStringLiteral("+5"), o), n(5));
        QCOMPARE(read(QStringLiteral("(5)"), o), n(-5));
        QCOMPARE(read(QStringLiteral(" 12 "), o), n(12));
        QCOMPARE(read(QStringLiteral(".5"), o), n(0.5));
        QCOMPARE(read(QStringLiteral("5."), o), n(5));
        QCOMPARE(read(QStringLiteral("007"), o), n(7));
        QCOMPARE(read(QStringLiteral("3.14159"), o), n(3.14159));
        QCOMPARE(read(QStringLiteral("1e3"), o), n(1000, QStringLiteral("0.00E+00")));
        QCOMPARE(read(QStringLiteral("1.5E-3"), o), n(0.0015, QStringLiteral("0.00E+00")));
        // thousands separators give the format that shows them
        QCOMPARE(read(QStringLiteral("1,234"), o), n(1234, QStringLiteral("#,##0")));
        QCOMPARE(read(QStringLiteral("1,234,567.5"), o), n(1234567.5, QStringLiteral("#,##0.00")));
        // not numbers
        for (const QString &text : {"1,23", "12,34,567", "0x10", "1.2.3", "12abc", "abc", "-", "+", ".", "1e", "1e+", "5-", "1 000", "e5", "5%%", "$", "%"})
            QVERIFY2(read(text, o).startsWith(QLatin1String("t:")), qPrintable(text + " -> " + read(text, o)));
    }

    void percents()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("5%"), o), n(0.05, QStringLiteral("0%")));
        QCOMPARE(read(QStringLiteral("5.5%"), o), n(0.055, QStringLiteral("0.0%")));
        QCOMPARE(read(QStringLiteral("12.345%"), o), n(0.12345, QStringLiteral("0.000%")));
        QCOMPARE(read(QStringLiteral("-5%"), o), n(-0.05, QStringLiteral("0%")));
        QCOMPARE(read(QStringLiteral("5 %"), o), n(0.05, QStringLiteral("0%")));
        QCOMPARE(read(QStringLiteral("100%"), o), n(1, QStringLiteral("0%")));
        QCOMPARE(read(QStringLiteral("1,000%"), o), n(10, QStringLiteral("0%")));
        QCOMPARE(read(QStringLiteral("(5%)"), o), n(-0.05, QStringLiteral("0%")));
    }

    // A whole number and a fraction of it, "1 1/2", is a number shown as a fraction, with as many places in the fraction as the denominator typed has; "3/4" alone is still a date.
    void fractions()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("1 1/2"), o), n(1.5, QStringLiteral("# ?/?")));
        QCOMPARE(read(QStringLiteral("-2 3/4"), o), n(-2.75, QStringLiteral("# ?/?")));
        QCOMPARE(read(QStringLiteral("0 1/8"), o), n(0.125, QStringLiteral("# ?/?")));
        QCOMPARE(read(QStringLiteral("  12   5/16 "), o), n(12.3125, QStringLiteral("# ??/??")));
        QCOMPARE(read(QStringLiteral("1 1/128"), o), n(1 + 1.0 / 128, QStringLiteral("# ???/???")));
        QCOMPARE(read(QStringLiteral("1 5/4"), o), n(2.25, QStringLiteral("# ?/?")));
        QCOMPARE(read(QStringLiteral("3/4"), o), n(serial(2025, 3, 4), QStringLiteral("m/d/yyyy")));
        for (const QString &text : {"1 a/2", "1.5 1/2", "1 1/123456"})
            QVERIFY2(read(text, o).startsWith(QLatin1String("t:")), qPrintable(text + " -> " + read(text, o)));
    }

    void money()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("$5"), o), n(5, QStringLiteral("\"$\"#,##0")));
        QCOMPARE(read(QStringLiteral("$5.00"), o), n(5, QStringLiteral("\"$\"#,##0.00")));
        QCOMPARE(read(QStringLiteral("$5.5"), o), n(5.5, QStringLiteral("\"$\"#,##0.00")));
        QCOMPARE(read(QStringLiteral("$1,234.50"), o), n(1234.5, QStringLiteral("\"$\"#,##0.00")));
        QCOMPARE(read(QStringLiteral("-$5"), o), n(-5, QStringLiteral("\"$\"#,##0")));
        QCOMPARE(read(QStringLiteral("$-5"), o), n(-5, QStringLiteral("\"$\"#,##0")));
        QCOMPARE(read(QStringLiteral("($5)"), o), n(-5, QStringLiteral("\"$\"#,##0")));
        QCOMPARE(read(QStringLiteral("$ 5"), o), n(5, QStringLiteral("\"$\"#,##0")));
        QVERIFY(read(QStringLiteral("$5%"), o).startsWith(QLatin1String("t:")));
        // another locale: its sign, its marks, and the sign after the amount
        const EntryOptions g = options(de);
        QCOMPARE(read(QStringLiteral("5,00 €"), g), n(5, QStringLiteral("#,##0.00\" €\"")));
        QCOMPARE(read(QStringLiteral("1.234,5 €"), g), n(1234.5, QStringLiteral("#,##0.00\" €\"")));
        QCOMPARE(read(QStringLiteral("5 €"), g), n(5, QStringLiteral("#,##0\" €\"")));
    }

    void localeMarks()
    {
        const EntryOptions g = options(de);
        QCOMPARE(read(QStringLiteral("1,5"), g), n(1.5));
        QCOMPARE(read(QStringLiteral("1.234,5"), g), n(1234.5, QStringLiteral("#,##0.00")));
        QCOMPARE(read(QStringLiteral("1.234"), g), n(1234, QStringLiteral("#,##0")));
        QCOMPARE(read(QStringLiteral("5,5%"), g), n(0.055, QStringLiteral("0.0%")));
        QCOMPARE(read(QStringLiteral("-2,5E3"), g), n(-2500, QStringLiteral("0.00E+00")));
        // the point is not the mark here, so "1.5" is a date (the first of May)
        QCOMPARE(read(QStringLiteral("1.5"), g), n(serial(2025, 5, 1), QStringLiteral("m/d/yyyy")));
        const EntryOptions f = options(fr);
        QCOMPARE(read(QStringLiteral("1 234,5"), f), n(1234.5, QStringLiteral("#,##0.00")));
        QCOMPARE(read(QStringLiteral("1 234,5"), f), n(1234.5, QStringLiteral("#,##0.00")));
        QVERIFY(read(QStringLiteral("1,234.5"), f).startsWith(QLatin1String("t:")));
    }

    void dates()
    {
        const EntryOptions o = options(en);
        const QString code = QStringLiteral("m/d/yyyy");
        QCOMPARE(read(QStringLiteral("3/4/2025"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("3/4/25"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("3/4/29"), o), n(serial(2029, 3, 4), code));
        QCOMPARE(read(QStringLiteral("3/4/30"), o), n(serial(1930, 3, 4), code));
        QCOMPARE(read(QStringLiteral("3/4/99"), o), n(serial(1999, 3, 4), code));
        QCOMPARE(read(QStringLiteral("2025-03-04"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("2025/3/4"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("3-4-2025"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("March 4, 2025"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("Mar 4 2025"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("4 Mar 2025"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("4-Mar-25"), o), n(serial(2025, 3, 4), code));
        // no year: this year
        QCOMPARE(read(QStringLiteral("3/4"), o), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("Mar 4"), o), n(serial(2025, 3, 4), code));
        // a month and a year: the first of it
        QCOMPARE(read(QStringLiteral("Mar 2025"), o), n(serial(2025, 3, 1), code));
        // not dates
        for (const QString &text : {"13/4/2025", "2/30/2025", "0/4/2025", "3/4/12345", "3/4/200", "1900-01-00", "3//4", "a/b/c", "3/4/5/6"})
            QVERIFY2(read(text, o).startsWith(QLatin1String("t:")), qPrintable(text + " -> " + read(text, o)));
        // the leap day that was not: day 60 of the 1900 system; and years before the system's first
        QCOMPARE(read(QStringLiteral("2/29/1900"), o), n(60, code));
        QVERIFY(read(QStringLiteral("12/31/1899"), o).startsWith(QLatin1String("t:")));
        const EntryOptions o1904 = options(en, DateSystem::Date1904);
        QCOMPARE(read(QStringLiteral("1/2/1904"), o1904), n(1, code));
        QVERIFY(read(QStringLiteral("12/31/1903"), o1904).startsWith(QLatin1String("t:")));
        // the order of the locale
        QCOMPARE(read(QStringLiteral("4/3/2025"), options(gb)), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("4.3.2025"), options(de)), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("04.03.25"), options(de)), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("4. März 2025"), options(de)), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("2025/3/4"), options(ja)), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("3/4"), options(ja)), n(serial(2025, 3, 4), code));
        QCOMPARE(read(QStringLiteral("4/3"), options(gb)), n(serial(2025, 3, 4), code));
    }

    void times()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("10:30"), o), n(10.5 / 24, QStringLiteral("h:mm")));
        QCOMPARE(read(QStringLiteral("10:30:15"), o), n((10 * 3600 + 30 * 60 + 15) / 86400.0, QStringLiteral("h:mm:ss")));
        QCOMPARE(read(QStringLiteral("10:30 PM"), o), n(22.5 / 24, QStringLiteral("h:mm AM/PM")));
        QCOMPARE(read(QStringLiteral("10:30pm"), o), n(22.5 / 24, QStringLiteral("h:mm AM/PM")));
        QCOMPARE(read(QStringLiteral("12:00 AM"), o), n(0, QStringLiteral("h:mm AM/PM")));
        QCOMPARE(read(QStringLiteral("12:30 PM"), o), n(12.5 / 24, QStringLiteral("h:mm AM/PM")));
        QCOMPARE(read(QStringLiteral("10 PM"), o), n(22.0 / 24, QStringLiteral("h:mm AM/PM")));
        QCOMPARE(read(QStringLiteral("9:05:30 am"), o), n((9 * 3600 + 5 * 60 + 30) / 86400.0, QStringLiteral("h:mm:ss AM/PM")));
        QCOMPARE(read(QStringLiteral("25:30"), o), n(25.5 / 24, QStringLiteral("[h]:mm")));
        QCOMPARE(read(QStringLiteral("0:00"), o), n(0, QStringLiteral("h:mm")));
        for (const QString &text : {"10:60", "13:00 PM", "0:30 AM", "10:", ":30", "1:2:3:4", "10:3x", "10:30:61", "ab:cd", "10"})
            QVERIFY2(!read(text, o).startsWith(QLatin1String("n:")) || text == QLatin1String("10"), qPrintable(text + " -> " + read(text, o)));
        QCOMPARE(read(QStringLiteral("10"), o), n(10));
    }

    void dateAndTime()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("3/4/2025 10:30"), o), n(serial(2025, 3, 4) + 10.5 / 24, QStringLiteral("m/d/yyyy h:mm")));
        QCOMPARE(read(QStringLiteral("3/4/2025 10:30 PM"), o), n(serial(2025, 3, 4) + 22.5 / 24, QStringLiteral("m/d/yyyy h:mm AM/PM")));
        QCOMPARE(read(QStringLiteral("Mar 4, 2025 8:15:30"), o), n(serial(2025, 3, 4) + (8 * 3600 + 15 * 60 + 30) / 86400.0, QStringLiteral("m/d/yyyy h:mm:ss")));
        QVERIFY(read(QStringLiteral("3/4/2025 25:61"), o).startsWith(QLatin1String("t:")));
    }

    void logicalValuesErrorsAndText()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("true"), o), QStringLiteral("b:1"));
        QCOMPARE(read(QStringLiteral("FALSE"), o), QStringLiteral("b:0"));
        QCOMPARE(read(QStringLiteral("#N/A"), o), QStringLiteral("e:#N/A"));
        QCOMPARE(read(QStringLiteral("#div/0!"), o), QStringLiteral("e:#DIV/0!"));
        QCOMPARE(read(QStringLiteral("#hello"), o), QStringLiteral("t:#hello"));
        QCOMPARE(read(QString(), o), QStringLiteral("-"));
        QCOMPARE(read(QStringLiteral("hello world"), o), QStringLiteral("t:hello world"));
        QCOMPARE(read(QStringLiteral("12 apples"), o), QStringLiteral("t:12 apples"));
        QCOMPARE(read(QStringLiteral("  padded  "), o), QStringLiteral("t:  padded  "));
        QCOMPARE(read(QStringLiteral("line\nbreak"), o), QStringLiteral("t:line\nbreak"));
        // an apostrophe keeps what follows as text
        QCOMPARE(read(QStringLiteral("'123"), o), QStringLiteral("t:123"));
        QCOMPARE(read(QStringLiteral("'=1+1"), o), QStringLiteral("t:=1+1"));
        QCOMPARE(read(QStringLiteral("'3/4/2025"), o), QStringLiteral("t:3/4/2025"));
        QCOMPARE(read(QStringLiteral("'"), o), QStringLiteral("t:'"));
        // a cell formatted as text keeps everything as typed (but a formula is a formula)
        EntryOptions text = o;
        text.textFormat = true;
        QCOMPARE(read(QStringLiteral("123"), text), QStringLiteral("t:123"));
        QCOMPARE(read(QStringLiteral("5%"), text), QStringLiteral("t:5%"));
        QCOMPARE(read(QStringLiteral("3/4/2025"), text), QStringLiteral("t:3/4/2025"));
        QCOMPARE(read(QStringLiteral("=1+2"), text), QStringLiteral("f:1+2"));
    }

    void formulas()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("=1+2"), o), QStringLiteral("f:1+2"));
        QCOMPARE(read(QStringLiteral("=SUM(A1:A3)*2"), o), QStringLiteral("f:SUM(A1:A3)*2"));
        QCOMPARE(read(QStringLiteral("=IF(A1>1,\"yes\",\"no\")"), o), QStringLiteral("f:IF(A1>1,\"yes\",\"no\")"));
        QCOMPARE(read(QStringLiteral("=NOSUCHFUNCTION(1)"), o), QStringLiteral("f:NOSUCHFUNCTION(1)"));   // (its result is #NAME?, as a person expects)
        QCOMPARE(read(QStringLiteral("-A1+2"), o), QStringLiteral("f:-A1+2"));
        QCOMPARE(read(QStringLiteral("+A1"), o), QStringLiteral("f:+A1"));
        QCOMPARE(read(QStringLiteral("-"), o), QStringLiteral("t:-"));
        QCOMPARE(read(QStringLiteral("+"), o), QStringLiteral("t:+"));
        // a formula that is not one: the message says what is wrong and where, in a sentence
        for (const QString &bad : {"=(1+2", "=1+", "=\"unclosed", "=SUM(A1:A3", "=1+2)", "=A1 B1 +", "=#", "=1,2", "=*3", "="}) {
            const EntryResult r = readEntry(bad, o);
            QVERIFY2(!r.ok, qPrintable(bad));
            QVERIFY2(r.problem.size() > 10 && r.problem.endsWith(QLatin1Char('.')), qPrintable(bad + " -> " + r.problem));
            QVERIFY2(r.problemAt >= 0 && r.problemAt <= bad.size(), qPrintable(bad + " at " + QString::number(r.problemAt)));
            QVERIFY(!r.problem.contains(QLatin1String("jp::")));
        }
        // the position counts the "=" the person typed
        QCOMPARE(readEntry(QStringLiteral("=1+2)"), o).problemAt, 4);
    }

    // A call has up to 255 arguments, the most the other program reads in a file (it refuses a file with a formula that has more); 256 are refused with a sentence.
    void aCallOfMoreThan255ArgumentsIsRefused()
    {
        const EntryOptions o = options(en);
        auto sum = [](int n) {
            QString text = QStringLiteral("=SUM(1");
            for (int i = 1; i < n; ++i) text += QStringLiteral(",1");
            return text + QLatin1Char(')');
        };
        QVERIFY(readEntry(sum(255), o).ok);
        const EntryResult r = readEntry(sum(256), o);
        QVERIFY(!r.ok);
        QVERIFY2(r.problem.contains(QStringLiteral("255")), qPrintable(r.problem));
    }

    // A formula that is typed nests up to 64 levels, the most the other program reads in a file; 65 are refused with a sentence that says the limit. A file's formula may nest deeper.
    void aFormulaNestedMoreThan64DeepIsRefused()
    {
        const EntryOptions o = options(en);
        auto nested = [](int levels) {
            QString text = QStringLiteral("=");
            for (int i = 0; i < levels; ++i) text += QStringLiteral("IF(1,");
            text += QLatin1Char('1');
            for (int i = 0; i < levels; ++i) text += QStringLiteral(",2)");
            return text;
        };
        QVERIFY(readEntry(nested(64), o).ok);
        const EntryResult r = readEntry(nested(65), o);
        QVERIFY(!r.ok);
        QVERIFY2(r.problem.contains(QStringLiteral("64")), qPrintable(r.problem));
        QVERIFY2(!r.problem.contains(QLatin1String("jp::")), qPrintable(r.problem));
        // (the parser takes more when a file holds it)
        QVERIFY(jp::calc::parseFormula(nested(65).mid(1)).ok());
    }

    // A whole number typed with more than 15 digits keeps the first 15 and zeros after them; a shorter one, and a number with a decimal mark or an exponent, is as typed.
    void aWholeNumberOfMoreThan15DigitsKeeps15()
    {
        const EntryOptions o = options(en);
        QCOMPARE(readEntry(QStringLiteral("1234567890123456789"), o).entry.number, 1234567890123450000.0);
        QCOMPARE(readEntry(QStringLiteral("-1234567890123456789"), o).entry.number, -1234567890123450000.0);
        QCOMPARE(readEntry(QStringLiteral("1234567890123456"), o).entry.number, 1234567890123450.0);
        QCOMPARE(readEntry(QStringLiteral("123456789012345"), o).entry.number, 123456789012345.0);
        QCOMPARE(readEntry(QStringLiteral("123456789012345.6"), o).entry.number, 123456789012345.6);
    }

    // A formula is kept as a file writes it: functions in capitals, and those added after the format's first release with the prefix the other programs need.
    void aFormulaIsKeptAsAFileWritesIt()
    {
        const EntryOptions o = options(en);
        QCOMPARE(read(QStringLiteral("=stdev.s(a1:a3)+sum(1,2)"), o), QStringLiteral("f:_xlfn.STDEV.S(a1:a3)+SUM(1,2)"));
        QCOMPARE(read(QStringLiteral("=IFNA(A1, 0)"), o), QStringLiteral("f:_xlfn.IFNA(A1, 0)"));
        QCOMPARE(read(QStringLiteral("=_xlfn.STDEV.S(A1:A3)"), o), QStringLiteral("f:_xlfn.STDEV.S(A1:A3)"));
        QCOMPARE(read(QStringLiteral("=CONCAT(\"stdev.s(\",'Sheet(1)'!A1,TEXTJOIN(\",\",1,B1:B2))"), o), QStringLiteral("f:_xlfn.CONCAT(\"stdev.s(\",'Sheet(1)'!A1,_xlfn.TEXTJOIN(\",\",1,B1:B2))"));
        QCOMPARE(read(QStringLiteral("=NoSuch(1)+if(A1,1,2)"), o), QStringLiteral("f:NoSuch(1)+IF(A1,1,2)"));
        QCOMPARE(toFileFormula(QStringLiteral("SUM (A1)")), QStringLiteral("SUM (A1)"));
        QCOMPARE(toFileFormula(QStringLiteral("Table1[Col (1)]+max(1)")), QStringLiteral("Table1[Col (1)]+MAX(1)"));
        // (a name that is not followed by a bracket is not a function)
        QCOMPARE(toFileFormula(QStringLiteral("sum+A1")), QStringLiteral("sum+A1"));
    }

    // A function the format prefixes is prefixed whether this program has it or not (a formula read back for a person, with the prefix taken away, is written the same way again), and
    // the names a LET or a LAMBDA gives are marked in the file's way where they are given and where they are used.
    void everyFunctionTheFormatPrefixesGetsItsPrefixAndParametersAreMarked()
    {
        QCOMPARE(toFileFormula(QStringLiteral("xlookup(A1,B:B,C:C,\"none\")")), QStringLiteral("_xlfn.XLOOKUP(A1,B:B,C:C,\"none\")"));
        QCOMPARE(toFileFormula(QStringLiteral("unique(A1:A9)+sequence(3)+SORTBY(A1:A3,B1:B3)")), QStringLiteral("_xlfn.UNIQUE(A1:A9)+_xlfn.SEQUENCE(3)+_xlfn.SORTBY(A1:A3,B1:B3)"));
        QCOMPARE(toFileFormula(QStringLiteral("filter(A1:A9,B1:B9>1)+sort(A1:A3)")), QStringLiteral("_xlfn._xlws.FILTER(A1:A9,B1:B9>1)+_xlfn._xlws.SORT(A1:A3)"));
        QCOMPARE(toFileFormula(QStringLiteral("textsplit(A1,\",\")+IFS(A1,1,2,3)+maxifs(A1:A3,B1:B3,1)")), QStringLiteral("_xlfn.TEXTSPLIT(A1,\",\")+_xlfn.IFS(A1,1,2,3)+_xlfn.MAXIFS(A1:A3,B1:B3,1)"));
        QCOMPARE(toFileFormula(QStringLiteral("_xlfn.XLOOKUP(A1,B:B,C:C)")), QStringLiteral("_xlfn.XLOOKUP(A1,B:B,C:C)"));   // (already as a file has it)
        QCOMPARE(toFileFormula(QStringLiteral("xlookup+1")), QStringLiteral("xlookup+1"));                                    // (a name that is not called)
        // parameters
        QCOMPARE(toFileFormula(QStringLiteral("let(x,1,y,x+1,x+y)")), QStringLiteral("_xlfn.LET(_xlpm.x,1,_xlpm.y,_xlpm.x+1,_xlpm.x+_xlpm.y)"));
        QCOMPARE(toFileFormula(QStringLiteral("LET( x , A1 , x*2 )")), QStringLiteral("_xlfn.LET( _xlpm.x , A1 , _xlpm.x*2 )"));
        QCOMPARE(toFileFormula(QStringLiteral("lambda(a,b,a*b)(2,3)")), QStringLiteral("_xlfn.LAMBDA(_xlpm.a,_xlpm.b,_xlpm.a*_xlpm.b)(2,3)"));
        QCOMPARE(toFileFormula(QStringLiteral("map(A1:A3,lambda(v,v*2))")), QStringLiteral("_xlfn.MAP(A1:A3,_xlfn.LAMBDA(_xlpm.v,_xlpm.v*2))"));
        QCOMPARE(toFileFormula(QStringLiteral("let(x,\"x\",x&\"x\"&'x'!A1&Sheet2!x1)")), QStringLiteral("_xlfn.LET(_xlpm.x,\"x\",_xlpm.x&\"x\"&'x'!A1&Sheet2!x1)"));
        QCOMPARE(toFileFormula(QStringLiteral("let(x,1,x+y)+x")), QStringLiteral("_xlfn.LET(_xlpm.x,1,_xlpm.x+y)+x"));                       // (a name is a parameter only inside)
        QCOMPARE(toFileFormula(QStringLiteral("_xlfn.LET(_xlpm.x,1,_xlpm.x+1)")), QStringLiteral("_xlfn.LET(_xlpm.x,1,_xlpm.x+1)"));
        QCOMPARE(toFileFormula(QStringLiteral("let(x,let(x,2,x*3),x+1)")), QStringLiteral("_xlfn.LET(_xlpm.x,_xlfn.LET(_xlpm.x,2,_xlpm.x*3),_xlpm.x+1)"));
        // read back for a person, they are as typed (with the names of functions in capitals)
        for (const QString &typed : {"XLOOKUP(A1,B:B,C:C)", "LET(x,1,y,x+1,x+y)", "MAP(A1:A3,LAMBDA(v,v*2))", "FILTER(A1:A9,B1:B9>1)", "SUM(UNIQUE(A1:A9))"})
            QCOMPARE(jp::grid::displayFormula(toFileFormula(typed)), typed);
    }

    // The whole list, one by one: every function the format prefixes is written with its prefix (in any case, and with a space before the bracket), and read back for a person without
    // it; the list is sorted, has no name twice, and has every function this program knows that the format prefixes, and every later-edition function that the plan
    // (docs/features/jeffcrunch.md) names; none of the first release's functions is on it.
    void theWholeListOfPrefixedFunctionsIsChecked()
    {
        const QStringList &all = prefixedFunctionNames();
        QVERIFY2(all.size() > 160, qPrintable(QString::number(all.size())));
        QVERIFY(std::is_sorted(all.begin(), all.end()));
        QCOMPARE(QSet<QString>(all.begin(), all.end()).size(), all.size());
        for (const QString &name : all) {
            const QString prefix = name == QLatin1String("FILTER") || name == QLatin1String("SORT") ? QStringLiteral("_xlfn._xlws.") : QStringLiteral("_xlfn.");
            QCOMPARE(toFileFormula(name.toLower() + QLatin1String("(A1)")), prefix + name + QLatin1String("(A1)"));
            QCOMPARE(toFileFormula(name + QLatin1String(" (A1)")), prefix + name + QLatin1String(" (A1)"));   // (one argument: LET and LAMBDA name no parameter)
            QCOMPARE(jp::grid::displayFormula(prefix + name + QLatin1String("(A1)")), name == QLatin1String("SINGLE") ? QStringLiteral("@A1") : name + QLatin1String("(A1)"));   // (SINGLE is the mark @)
            QCOMPARE(toFileFormula(prefix + name + QLatin1String("(A1)")), prefix + name + QLatin1String("(A1)"));   // (already as a file has it)
        }
        // what this program has of them is all on the list
        int future = 0;
        for (int i = 0; i < jp::calc::functionCount(); ++i) {
            const jp::calc::FunctionInfo &f = jp::calc::functionAt(i);
            if (!(f.flags & jp::calc::kFnFuture)) continue;
            ++future;
            QVERIFY2(all.contains(QString::fromLatin1(f.name)), f.name);
        }
        QVERIFY(future > 5);
        // the later-edition functions that the plan names
        static const char *const newer[] = {
            "ACOT", "ACOTH", "AGGREGATE", "ARABIC", "ARRAYTOTEXT", "BASE", "BETA.DIST", "BETA.INV", "BINOM.DIST", "BINOM.DIST.RANGE", "BINOM.INV", "BYCOL", "BYROW", "CEILING.MATH",
            "CEILING.PRECISE", "CHISQ.DIST", "CHISQ.DIST.RT", "CHISQ.INV", "CHISQ.INV.RT", "CHISQ.TEST", "CHOOSECOLS", "CHOOSEROWS", "COMBINA", "CONCAT", "CONFIDENCE.NORM",
            "CONFIDENCE.T", "COT", "COTH", "COVARIANCE.P", "COVARIANCE.S", "CSC", "CSCH", "DAYS", "DBCS", "DECIMAL", "DETECTLANGUAGE", "DROP", "ENCODEURL", "EXPAND", "EXPON.DIST",
            "F.DIST", "F.DIST.RT", "F.INV", "F.INV.RT", "F.TEST", "FIELDVALUE", "FILTER", "FILTERXML", "FLOOR.MATH", "FLOOR.PRECISE", "FORECAST.ETS", "FORECAST.ETS.CONFINT",
            "FORECAST.ETS.SEASONALITY", "FORECAST.ETS.STAT", "FORECAST.LINEAR", "FORMULATEXT", "GAMMA", "GAMMA.DIST", "GAMMA.INV", "GAMMALN.PRECISE", "GAUSS", "GROUPBY", "HSTACK",
            "HYPGEOM.DIST", "IFNA", "IFS", "IMAGE", "ISFORMULA", "ISO.CEILING", "ISOMITTED", "ISOWEEKNUM", "LAMBDA", "LET", "LOGNORM.DIST", "LOGNORM.INV", "MAKEARRAY", "MAP",
            "MAXIFS", "MINIFS", "MODE.MULT", "MODE.SNGL", "MUNIT", "NEGBINOM.DIST", "NETWORKDAYS.INTL", "NORM.DIST", "NORM.INV", "NORM.S.DIST", "NORM.S.INV", "NUMBERVALUE",
            "PDURATION", "PERCENTILE.EXC", "PERCENTILE.INC", "PERCENTOF", "PERCENTRANK.EXC", "PERCENTRANK.INC", "PERMUTATIONA", "PHI", "PIVOTBY", "POISSON.DIST", "QUARTILE.EXC",
            "QUARTILE.INC", "RANDARRAY", "RANK.AVG", "RANK.EQ", "REDUCE", "REGEXEXTRACT", "REGEXREPLACE", "REGEXTEST", "RRI", "SCAN", "SEC", "SECH", "SEQUENCE", "SHEET", "SHEETS",
            "SKEW.P", "SORT", "SORTBY", "STDEV.P", "STDEV.S", "STOCKHISTORY", "SWITCH", "T.DIST", "T.DIST.2T", "T.DIST.RT", "T.INV", "T.INV.2T", "T.TEST", "TAKE", "TEXTAFTER",
            "TEXTBEFORE", "TEXTJOIN", "TEXTSPLIT", "TOCOL", "TOROW", "TRANSLATE", "TRIMRANGE", "UNICHAR", "UNICODE", "UNIQUE", "VALUETOTEXT", "VAR.P", "VAR.S", "VSTACK",
            "WEBSERVICE", "WEIBULL.DIST", "WORKDAY.INTL", "WRAPCOLS", "WRAPROWS", "XLOOKUP", "XMATCH", "XOR", "Z.TEST"
};
        for (const char *name : newer) QVERIFY2(all.contains(QLatin1String(name)), name);
        // and those of the first release (and the double-byte forms, which are older too) are not prefixed
        static const char *const classic[] = {
            "ASC", "AVERAGEIFS", "BAHTTEXT", "BIN2DEC", "CEILING", "CELL", "CONCATENATE", "CONFIDENCE", "CONVERT", "COUNTIFS", "COVAR", "CUBEVALUE", "DEC2HEX", "ERF", "FINDB",
            "FLOOR", "FORECAST", "GETPIVOTDATA", "HYPERLINK", "IF", "IFERROR", "IMSUM", "INDEX", "INFO", "ISBLANK", "ISERROR", "LEFTB", "LENB", "LINEST", "MATCH", "MIDB", "MMULT",
            "MODE", "NETWORKDAYS", "NORMDIST", "NOW", "PERCENTILE", "PHONETIC", "QUARTILE", "RANK", "REPLACEB", "RIGHTB", "RTD", "SEARCHB", "STDEV", "SUM", "SUMIFS", "SUMPRODUCT",
            "TDIST", "TEXT", "VAR", "VLOOKUP", "WORKDAY", "YEARFRAC"
};
        for (const char *name : classic) {
            QVERIFY2(!all.contains(QLatin1String(name)), name);
            QVERIFY2(!toFileFormula(QString::fromLatin1(name).toLower() + QLatin1String("(A1)")).contains(QLatin1String("_xl")), name);
        }
    }

    // The built-in date formats 14 and 22 are the system's short date: a date typed in a locale is read in its order and shown in the same order (a date typed as 4 March in a locale of
    // day, month, year is not shown as 3/4/2025), and the text F2 gives back reads as the same date.
    void theBuiltInDateFormatsShowInTheOrderOfTheLocale()
    {
        struct Case {
            QLocale locale;
            QString typed, shown, code;
        };
        const Case cases[] = {{en, QStringLiteral("3/4/2025"), QStringLiteral("3/4/2025"), QStringLiteral("m/d/yyyy")},
                              {gb, QStringLiteral("4/3/2025"), QStringLiteral("04/03/2025"), QStringLiteral("dd/mm/yyyy")},
                              {de, QStringLiteral("4.3.2025"), QStringLiteral("04.03.2025"), QStringLiteral("dd\\.mm\\.yyyy")},
                              {fr, QStringLiteral("4/3/2025"), QStringLiteral("04/03/2025"), QStringLiteral("dd/mm/yyyy")},
                              {ja, QStringLiteral("2025/3/4"), QStringLiteral("2025/03/04"), QStringLiteral("yyyy/mm/dd")}};
        for (const Case &c : cases) {
            const QString name = c.locale.name();
            const EntryOptions o = options(c.locale);
            const EntryResult r = readEntry(c.typed, o);
            QVERIFY2(r.ok && r.entry.kind == Entry::Kind::Number, qPrintable(name));
            QCOMPARE(r.entry.number, serial(2025, 3, 4));
            QCOMPARE(r.entry.formatCode, QStringLiteral("m/d/yyyy"));   // (the file's built-in 14, which the file has as the system's)
            QCOMPARE(jp::calc::shortDateFormatCode(c.locale), c.code);
            QCOMPARE(jp::calc::builtinFormatCode(14, c.locale), c.code);
            const jp::calc::NumberFormat f14(jp::calc::builtinFormatCode(14, c.locale)), f22(jp::calc::builtinFormatCode(22, c.locale));
            QVERIFY2(f14.isValid() && f22.isValid(), qPrintable(name));
            QCOMPARE(f14.format(r.entry.number, DateSystem::Date1900).text, c.shown);
            QCOMPARE(f22.format(r.entry.number + 0.5, DateSystem::Date1900).text, c.shown + QStringLiteral(" 12:00"));
            // F2: what the cell shows as an entry reads as the same date
            const QString edit = editTextOfValue(Value::number(r.entry.number), jp::calc::builtinFormatCode(14, c.locale), o);
            const EntryResult again = readEntry(edit, o);
            QVERIFY2(again.ok && again.entry.kind == Entry::Kind::Number && again.entry.number == r.entry.number, qPrintable(name + QLatin1String(": ") + edit));
            // and the format of a cell is the code of the locale
            Styles styles;
            styles.cellXfs = {14, 22, 15};
            QCOMPARE(styles.formatCode(0, c.locale), c.code);
            QCOMPARE(styles.formatCode(1, c.locale), c.code + QStringLiteral(" h:mm"));
            QCOMPARE(styles.formatCode(2, c.locale), jp::calc::builtinFormatCode(15));   // (the others are the same everywhere)
        }
        // the code is a date for every locale the system has, and a locale whose short date has not a day, a month, and a year has the United States'
        for (const QLocale &l : QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry)) {
            const jp::calc::NumberFormat f(jp::calc::shortDateFormatCode(l));
            QVERIFY2(f.isValid() && f.kind() == jp::calc::FormatKind::Date, qPrintable(l.name()));
        }
    }

    // In a locale that writes a comma for the decimal point, the arguments of a function are separated by a semicolon and a number has a comma in it: what is typed is read so, and the
    // file always has the format's own (commas between arguments, dots in numbers). What is in quotes, in brackets, and in braces is left as it is.
    void formulasUseTheSeparatorsOfTheLocale()
    {
        const EntryOptions o = options(de);
        QCOMPARE(read(QStringLiteral("=SUM(A1;A2)"), o), QStringLiteral("f:SUM(A1,A2)"));
        QCOMPARE(read(QStringLiteral("=A1*0,5"), o), QStringLiteral("f:A1*0.5"));
        QCOMPARE(read(QStringLiteral("=SUM(1,5;2,5)+,5"), o), QStringLiteral("f:SUM(1.5,2.5)+.5"));
        QCOMPARE(read(QStringLiteral("=IF(A1>1;\"a,b\";\"c;d\")"), o), QStringLiteral("f:IF(A1>1,\"a,b\",\"c;d\")"));
        QCOMPARE(read(QStringLiteral("=1,5E3+LOG10(100)+STDEV.S(A1:A3)"), o), QStringLiteral("f:1.5E3+LOG10(100)+_xlfn.STDEV.S(A1:A3)"));
        QCOMPARE(read(QStringLiteral("=Table1[[#This Row],[Q1]]*2,5+'a;b'!A1"), o), QStringLiteral("f:Table1[[#This Row],[Q1]]*2.5+'a;b'!A1"));
        QCOMPARE(read(QStringLiteral("=SUM({1,2;3,4})"), o), QStringLiteral("f:SUM({1,2;3,4})"));
        QCOMPARE(read(QStringLiteral("=$A$1+B$2*3,25"), o), QStringLiteral("f:$A$1+B$2*3.25"));
        QCOMPARE(read(QStringLiteral("=-A1;"), o), QStringLiteral("!"));   // (not a formula)
        // the United States: as it was
        QCOMPARE(read(QStringLiteral("=SUM(A1,A2)*0.5"), options(en)), QStringLiteral("f:SUM(A1,A2)*0.5"));
        // French and others with a comma
        QCOMPARE(read(QStringLiteral("=SOMME(A1;A2)"), options(fr)), QStringLiteral("f:SOMME(A1,A2)"));
        // and shown for editing the same way back
        QCOMPARE(formulaToLocale(QStringLiteral("SUM(A1,A2)"), de), QStringLiteral("SUM(A1;A2)"));
        QCOMPARE(formulaToLocale(QStringLiteral("A1*0.5"), de), QStringLiteral("A1*0,5"));
        QCOMPARE(formulaToLocale(QStringLiteral("IF(A1>1,\"a,b\",\"c;d\")"), de), QStringLiteral("IF(A1>1;\"a,b\";\"c;d\")"));
        QCOMPARE(formulaToLocale(QStringLiteral("STDEV.S(A1:A3)*1.5E3+LOG10(2.5)"), de), QStringLiteral("STDEV.S(A1:A3)*1,5E3+LOG10(2,5)"));
        QCOMPARE(formulaToLocale(QStringLiteral("Table1[[#This Row],[Q1]]*2.5+'a,b'!A1"), de), QStringLiteral("Table1[[#This Row],[Q1]]*2,5+'a,b'!A1"));
        QCOMPARE(formulaToLocale(QStringLiteral("SUM({1,2;3,4})+$A$1*.5"), de), QStringLiteral("SUM({1,2;3,4})+$A$1*,5"));
        QCOMPARE(formulaToLocale(QStringLiteral("SUM(A1,A2)*0.5"), en), QStringLiteral("SUM(A1,A2)*0.5"));
        for (const QString &f : {"SUM(A1,A2)*0.5", "IF(A1>1,\"a,b\",2.5E-3)", "A1+B1", "AVERAGE(A1:A3,.5,6.)", "SUM({1,2;3,4},1.5)"})
            QCOMPARE(formulaFromLocale(formulaToLocale(f, de), de), f);
    }

    // Every function the engine has has a row in the table of local names: a local name, or an explicit mark that its English one is kept; no local name is another function's English
    // name, and none is two functions' (both would make a typed name mean two things).
    void everyFunctionHasARowInTheTableOfLocalNames()
    {
        const QStringList rows = formulaNameRows();
        const QSet<QString> have(rows.begin(), rows.end());
        QStringList missing;
        for (int i = 0; i < jp::calc::functionCount(); ++i) {
            const QString name = QString::fromLatin1(jp::calc::functionAt(i).name);
            if (!have.contains(name)) missing << name;
        }
        QVERIFY2(missing.isEmpty(), qPrintable(missing.join(QLatin1Char(' '))));
        for (const QString &row : rows)
            if (!row.startsWith(QLatin1Char('#'))) QVERIFY2(jp::calc::findFunction(row), qPrintable(row));   // (no row for a function that is not there)
        const QStringList languages = formulaNameLanguages();
        QCOMPARE(languages, (QStringList{"de", "fr", "es", "it", "nl", "pt_BR", "pt_PT", "ru", "uk", "id"}));
        for (const QString &language : languages) {
            QHash<QString, QString> seen;
            for (const QString &row : rows) {
                const QString local = localFormulaName(row, language);
                QCOMPARE(englishFormulaName(local, language).isEmpty() && local.compare(row, Qt::CaseInsensitive) != 0, false);
                if (local.compare(row, Qt::CaseInsensitive) == 0) continue;
                QVERIFY2(!seen.contains(local.toUpper()), qPrintable(language + QLatin1Char(' ') + local));
                seen.insert(local.toUpper(), row);
                QVERIFY2(!jp::calc::findFunction(local) || row.startsWith(QLatin1Char('#')), qPrintable(language + QLatin1Char(' ') + row + QLatin1Char('>') + local));
                QCOMPARE(englishFormulaName(local, language), row);
            }
        }
        // (what each language has)
        QVERIFY(localFormulaNameCount(QStringLiteral("de")) >= 200);
        QVERIFY(localFormulaNameCount(QStringLiteral("ru")) >= 200);
        QVERIFY(localFormulaNameCount(QStringLiteral("uk")) >= 1);   // (the lists give Ukrainian and Indonesian few names of their own)
        QVERIFY(localFormulaNameCount(QStringLiteral("id")) >= 1);
        QVERIFY(formulaNameKeptEnglish(QStringLiteral("UNICODE")));
        QVERIFY(formulaNameKeptEnglish(QStringLiteral("FORECAST.ETS")));
        QVERIFY(!formulaNameKeptEnglish(QStringLiteral("SUM")));
        QCOMPARE(formulaLanguageOf(QStringLiteral("es_ES")), QStringLiteral("es"));
        QCOMPARE(formulaLanguageOf(QStringLiteral("pt_PT")), QStringLiteral("pt_PT"));
        QCOMPARE(formulaLanguageOf(QStringLiteral("en_GB")), QString());
        QCOMPARE(formulaLanguageOf(QString()), QString());
    }

    // Twenty well-known functions in each language, with the names written here; each is the name two published lists agree on.
    void wellKnownFunctionsHaveTheirLocalNames()
    {
        using Names = QList<std::pair<QString, QString>>;
        const QHash<QString, Names> expected = {
            {"de", {{"SUM", "SUMME"}, {"IF", "WENN"}, {"VLOOKUP", "SVERWEIS"}, {"COUNTIF", "ZÄHLENWENN"}, {"AVERAGE", "MITTELWERT"}, {"ROUND", "RUNDEN"}, {"LEFT", "LINKS"}, {"DATE", "DATUM"}, {"TODAY", "HEUTE"}, {"INDEX", "INDEX"}, {"MATCH", "VERGLEICH"}, {"CONCATENATE", "VERKETTEN"}, {"AND", "UND"}, {"OR", "ODER"}, {"NOT", "NICHT"}, {"MAX", "MAX"}, {"MIN", "MIN"}, {"LEN", "LÄNGE"}, {"TRIM", "GLÄTTEN"}, {"SUMIF", "SUMMEWENN"}}},
            {"fr", {{"SUM", "SOMME"}, {"IF", "SI"}, {"VLOOKUP", "RECHERCHEV"}, {"COUNTIF", "NB.SI"}, {"AVERAGE", "MOYENNE"}, {"ROUND", "ARRONDI"}, {"LEFT", "GAUCHE"}, {"DATE", "DATE"}, {"TODAY", "AUJOURDHUI"}, {"INDEX", "INDEX"}, {"MATCH", "EQUIV"}, {"CONCATENATE", "CONCATENER"}, {"AND", "ET"}, {"OR", "OU"}, {"NOT", "NON"}, {"MAX", "MAX"}, {"MIN", "MIN"}, {"LEN", "NBCAR"}, {"TRIM", "SUPPRESPACE"}, {"SUMIF", "SOMME.SI"}}},
            {"es", {{"SUM", "SUMA"}, {"IF", "SI"}, {"VLOOKUP", "BUSCARV"}, {"COUNTIF", "CONTAR.SI"}, {"AVERAGE", "PROMEDIO"}, {"ROUND", "REDONDEAR"}, {"LEFT", "IZQUIERDA"}, {"DATE", "FECHA"}, {"TODAY", "HOY"}, {"INDEX", "INDICE"}, {"MATCH", "COINCIDIR"}, {"CONCATENATE", "CONCATENAR"}, {"AND", "Y"}, {"OR", "O"}, {"NOT", "NO"}, {"MAX", "MAX"}, {"MIN", "MIN"}, {"LEN", "LARGO"}, {"TRIM", "ESPACIOS"}, {"SUMIF", "SUMAR.SI"}}},
            {"it", {{"SUM", "SOMMA"}, {"IF", "SE"}, {"VLOOKUP", "CERCA.VERT"}, {"COUNTIF", "CONTA.SE"}, {"AVERAGE", "MEDIA"}, {"ROUND", "ARROTONDA"}, {"LEFT", "SINISTRA"}, {"DATE", "DATA"}, {"TODAY", "OGGI"}, {"INDEX", "INDICE"}, {"MATCH", "CONFRONTA"}, {"CONCATENATE", "CONCATENA"}, {"AND", "E"}, {"OR", "O"}, {"NOT", "NON"}, {"MAX", "MAX"}, {"MIN", "MIN"}, {"LEN", "LUNGHEZZA"}, {"TRIM", "ANNULLA.SPAZI"}, {"SUMIF", "SOMMA.SE"}}},
            {"nl", {{"SUM", "SOM"}, {"IF", "ALS"}, {"VLOOKUP", "VERT.ZOEKEN"}, {"COUNTIF", "AANTAL.ALS"}, {"AVERAGE", "GEMIDDELDE"}, {"ROUND", "AFRONDEN"}, {"LEFT", "LINKS"}, {"DATE", "DATUM"}, {"TODAY", "VANDAAG"}, {"INDEX", "INDEX"}, {"MATCH", "VERGELIJKEN"}, {"CONCATENATE", "TEKST.SAMENVOEGEN"}, {"AND", "EN"}, {"OR", "OF"}, {"NOT", "NIET"}, {"MAX", "MAX"}, {"MIN", "MIN"}, {"LEN", "LENGTE"}, {"TRIM", "SPATIES.WISSEN"}, {"SUMIF", "SOM.ALS"}}},
            {"pt_BR", {{"SUM", "SOMA"}, {"IF", "SE"}, {"VLOOKUP", "PROCV"}, {"COUNTIF", "CONT.SE"}, {"AVERAGE", "MÉDIA"}, {"ROUND", "ARRED"}, {"LEFT", "ESQUERDA"}, {"DATE", "DATA"}, {"TODAY", "HOJE"}, {"INDEX", "ÍNDICE"}, {"MATCH", "CORRESP"}, {"CONCATENATE", "CONCATENAR"}, {"AND", "E"}, {"OR", "OU"}, {"NOT", "NÃO"}, {"MAX", "MÁXIMO"}, {"MIN", "MÍNIMO"}, {"LEN", "NÚM.CARACT"}, {"TRIM", "ARRUMAR"}, {"SUMIF", "SOMASE"}}},
            {"pt_PT", {{"SUM", "SOMA"}, {"IF", "SE"}, {"VLOOKUP", "PROCV"}, {"AVERAGE", "MÉDIA"}, {"ROUND", "ARRED"}, {"LEFT", "ESQUERDA"}, {"DATE", "DATA"}, {"TODAY", "HOJE"}, {"INDEX", "ÍNDICE"}, {"CONCATENATE", "CONCATENAR"}, {"AND", "E"}, {"NOT", "NÃO"}, {"SUMIF", "SOMA.SE"}, {"NOW", "AGORA"}, {"YEAR", "ANO"}, {"MONTH", "MÊS"}, {"DAY", "DIA"}, {"RIGHT", "DIREITA"}, {"SUBSTITUTE", "SUBST"}, {"IFERROR", "SE.ERRO"}}},
            {"ru", {{"SUM", "СУММ"}, {"IF", "ЕСЛИ"}, {"VLOOKUP", "ВПР"}, {"COUNTIF", "СЧЁТЕСЛИ"}, {"AVERAGE", "СРЗНАЧ"}, {"ROUND", "ОКРУГЛ"}, {"LEFT", "ЛЕВСИМВ"}, {"DATE", "ДАТА"}, {"TODAY", "СЕГОДНЯ"}, {"INDEX", "ИНДЕКС"}, {"MATCH", "ПОИСКПОЗ"}, {"CONCATENATE", "СЦЕПИТЬ"}, {"AND", "И"}, {"OR", "ИЛИ"}, {"NOT", "НЕ"}, {"MAX", "МАКС"}, {"MIN", "МИН"}, {"LEN", "ДЛСТР"}, {"TRIM", "СЖПРОБЕЛЫ"}, {"SUMIF", "СУММЕСЛИ"}}},
            {"uk", {{"SUM", "SUM"}, {"IF", "IF"}, {"VLOOKUP", "VLOOKUP"}, {"COUNTIF", "COUNTIF"}, {"AVERAGE", "AVERAGE"}, {"ROUND", "ROUND"}, {"LEFT", "LEFT"}, {"DATE", "DATE"}, {"TODAY", "TODAY"}, {"INDEX", "INDEX"}, {"MATCH", "MATCH"}, {"CONCATENATE", "CONCATENATE"}, {"AND", "AND"}, {"OR", "OR"}, {"NOT", "NOT"}, {"MAX", "MAX"}, {"MIN", "MIN"}, {"LEN", "LEN"}, {"TRIM", "TRIM"}, {"SUMIF", "SUMIF"}}},
            {"id", {{"SUM", "SUM"}, {"IF", "IF"}, {"VLOOKUP", "VLOOKUP"}, {"COUNTIF", "COUNTIF"}, {"AVERAGE", "AVERAGE"}, {"ROUND", "ROUND"}, {"LEFT", "LEFT"}, {"DATE", "DATE"}, {"TODAY", "TODAY"}, {"INDEX", "INDEX"}, {"MATCH", "MATCH"}, {"CONCATENATE", "CONCATENATE"}, {"AND", "AND"}, {"OR", "OR"}, {"NOT", "NOT"}, {"MIN", "MIN"}, {"LEN", "LEN"}, {"TRIM", "TRIM"}, {"SUMIF", "SUMIF"}, {"ABS", "ABS"}}}};
        QCOMPARE(expected.size(), formulaNameLanguages().size());
        for (auto it = expected.constBegin(); it != expected.constEnd(); ++it) {
            QCOMPARE(it.value().size(), 20);
            for (const auto &pair : it.value()) {
                const QString what = it.key() + QLatin1Char(' ') + pair.first;
                QVERIFY2(localFormulaName(pair.first, it.key()).compare(pair.second, Qt::CaseInsensitive) == 0, qPrintable(what));
                if (pair.second.compare(pair.first, Qt::CaseInsensitive) != 0) QCOMPARE(englishFormulaName(pair.second, it.key()), pair.first);
            }
        }
    }

    // A function with two names in a language shows the first and reads both.
    void alsoAcceptedNamesAreRead()
    {
        QCOMPARE(localFormulaName(QStringLiteral("DATEVALUE"), QStringLiteral("de")), QStringLiteral("DATWERT"));
        QCOMPARE(formulaNameAlternatives(QStringLiteral("DATEVALUE"), QStringLiteral("de")), QStringList{QStringLiteral("DATUMWERT")});
        QCOMPARE(englishFormulaName(QStringLiteral("datumwert"), QStringLiteral("de")), QStringLiteral("DATEVALUE"));
        QCOMPARE(englishFormulaName(QStringLiteral("DATWERT"), QStringLiteral("de")), QStringLiteral("DATEVALUE"));
        EntryOptions o = options(de);
        o.formulaLanguage = QStringLiteral("de");
        QCOMPARE(read(QStringLiteral("=DATUMWERT(A1)"), o), QStringLiteral("f:DATEVALUE(A1)"));
        EntryLocale l(de);
        l.language = QStringLiteral("de");
        QCOMPARE(formulaToLocale(QStringLiteral("DATEVALUE(A1)"), l), QStringLiteral("DATWERT(A1)"));
        // every other name of every function is read back as that function, in every language
        for (const QString &language : formulaNameLanguages())
            for (const QString &row : formulaNameRows())
                for (const QString &other : formulaNameAlternatives(row, language))
                    QCOMPARE(englishFormulaName(other, language), row);
    }

    // A table where one local name means two functions, or is another function's English name, is refused; the shipped one is sound.
    void anAmbiguousTableIsRefused()
    {
        QVERIFY2(formulaNameProblems().isEmpty(), qPrintable(formulaNameProblems().join(QLatin1Char('\n'))));
        const QString head = QStringLiteral("#columns de fr\n");
        QVERIFY(formulaNameProblems(head + QStringLiteral("SUM SUMME SOMME\nSUMIF SUMME SOMME.SI\n")).size() == 1);                // two functions, one name
        QVERIFY(formulaNameProblems(head + QStringLiteral("SUM SUMME SOMME\nSUMIF SUMME|X SOMME.SI\n")).size() == 1);             // (as the second name of one)
        QVERIFY(formulaNameProblems(head + QStringLiteral("SUM SUMME SOMME\nSUMIF SUM SOMME.SI\n")).size() == 1);                 // another function's English name
        QVERIFY(formulaNameProblems(head + QStringLiteral("SUM SUMME SOMME\nSUMIF SUMMEWENN|SUMIF SOMME.SI\n")).isEmpty());       // (its own name is fine)
        QVERIFY(formulaNameProblems(head + QStringLiteral("SUM SUMME SOMME\nSUMIF SUMMEWENN SOMME.SI\n")).isEmpty());
    }

    // With the program in German, a formula is typed and shown with German names, TRUE and FALSE, and errors; English ones are read too; the file's form is English.
    void formulasAreTypedAndShownWithLocalNames()
    {
        EntryOptions o = options(de);
        o.formulaLanguage = QStringLiteral("de");
        QCOMPARE(read(QStringLiteral("=SUMME(A1;A2)*0,5"), o), QStringLiteral("f:SUM(A1,A2)*0.5"));
        QCOMPARE(read(QStringLiteral("=summe(A1;A2)"), o), QStringLiteral("f:SUM(A1,A2)"));
        QCOMPARE(read(QStringLiteral("=SUM(A1;A2)"), o), QStringLiteral("f:SUM(A1,A2)"));   // (English, as well)
        QCOMPARE(read(QStringLiteral("=WENN(A1>1;WAHR;FALSCH)"), o), QStringLiteral("f:IF(A1>1,TRUE,FALSE)"));
        QCOMPARE(read(QStringLiteral("=WENNFEHLER(SVERWEIS(A1;B1:C9;2;FALSCH);#NV)"), o), QStringLiteral("f:_xlfn.IFERROR(VLOOKUP(A1,B1:C9,2,FALSE),#N/A)").replace(QStringLiteral("_xlfn."), QString()));
        QCOMPARE(read(QStringLiteral("=STABW(A1:A3)"), o), QStringLiteral("f:STDEV(A1:A3)"));
        QCOMPARE(read(QStringLiteral("=TEXT(A1;\"WENN;SUMME\")&\"WAHR\""), o), QStringLiteral("f:TEXT(A1,\"WENN;SUMME\")&\"WAHR\""));   // (text is text)
        QCOMPARE(read(QStringLiteral("=SUMME({1,5;2\\3;4})"), o), QStringLiteral("f:SUM({1.5,2;3,4})"));
        QCOMPARE(read(QStringLiteral("=SUMME({WAHR;#NV})"), o), QStringLiteral("f:SUM({TRUE;#N/A})"));   // (no backslash: the file's form, two rows)
        QCOMPARE(read(QStringLiteral("=SUMME({WAHR;#NV\\1;2})"), o), QStringLiteral("f:SUM({TRUE,#N/A;1,2})"));
        // a name of a sheet or of a workbook is not a function or TRUE
        QCOMPARE(read(QStringLiteral("=Summe!A1+WAHR!B2"), o), QStringLiteral("f:Summe!A1+WAHR!B2"));
        o.definedName = [](const QString &w) { return w.compare(QLatin1String("wahr"), Qt::CaseInsensitive) == 0; };
        QCOMPARE(read(QStringLiteral("=WAHR+1"), o), QStringLiteral("f:WAHR+1"));
        // shown
        EntryLocale l(de);
        l.language = QStringLiteral("de");
        QCOMPARE(formulaToLocale(QStringLiteral("IF(SUM(A1,A2)*0.5>1,TRUE,FALSE)"), l), QStringLiteral("WENN(SUMME(A1;A2)*0,5>1;WAHR;FALSCH)"));
        QCOMPARE(formulaToLocale(QStringLiteral("IFERROR(VLOOKUP(A1,B1:C9,2,FALSE),#N/A)+#REF!"), l), QStringLiteral("WENNFEHLER(SVERWEIS(A1;B1:C9;2;FALSCH);#NV)+#BEZUG!"));
        QCOMPARE(formulaToLocale(QStringLiteral("SUM({1.5,2;3,4})+Summe!A1+TEXT(A1,\"SUM,TRUE\")"), l), QStringLiteral("SUMME({1,5;2\\3;4})+Summe!A1+TEXT(A1;\"SUM,TRUE\")"));
        // a name the language has no word for stays; so does a language without names, and without a language the text is as it was
        QCOMPARE(formulaToLocale(QStringLiteral("ABS(A1)+MyName(2)"), l), QStringLiteral("ABS(A1)+MyName(2)"));
        EntryLocale indonesian(QLocale(QLocale::Indonesian, QLocale::Indonesia));
        indonesian.language = QStringLiteral("id");
        QCOMPARE(formulaToLocale(QStringLiteral("SUM(A1,A2)*0.5"), indonesian), QStringLiteral("SUM(A1;A2)*0,5"));
        QCOMPARE(formulaToLocale(QStringLiteral("SUM({1,2;3,4})"), de), QStringLiteral("SUM({1,2;3,4})"));
        // Russian: the names in Cyrillic, and the separators of a locale with a comma
        EntryLocale ru(QLocale(QLocale::Russian, QLocale::Russia));
        ru.language = QStringLiteral("ru");
        QCOMPARE(formulaToLocale(QStringLiteral("IF(A1>0.5,SUM(A1:A3),#DIV/0!)"), ru), QStringLiteral("ЕСЛИ(A1>0,5;СУММ(A1:A3);#ДЕЛ/0!)"));
        QCOMPARE(formulaFromLocale(QStringLiteral("ЕСЛИ(A1>0,5;СУММ(A1:A3);#ДЕЛ/0!)"), ru), QStringLiteral("IF(A1>0.5,SUM(A1:A3),#DIV/0!)"));
        // a United States locale with a Spanish program: names are Spanish, arguments are separated by commas
        EntryLocale us(QLocale(QLocale::English, QLocale::UnitedStates));
        us.language = QStringLiteral("es");
        QCOMPARE(formulaToLocale(QStringLiteral("IF(SUM(A1,A2)>0.5,TRUE,#NAME?)"), us), QStringLiteral("SI(SUMA(A1,A2)>0.5,VERDADERO,#¿NOMBRE?)"));
        QCOMPARE(formulaFromLocale(QStringLiteral("SI(SUMA(A1,A2)>0.5,VERDADERO,#¿NOMBRE?)"), us), QStringLiteral("IF(SUM(A1,A2)>0.5,TRUE,#NAME?)"));
    }

    // parse(show(f)) is f for every function, with every separator combination and in every language the table has (a generated test), and a workbook made in one language reads in another.
    void everyFunctionGoesRoundInEveryLanguage()
    {
        const QStringList rows = formulaNameRows();
        const QList<QLocale> locales = {de, QLocale(QLocale::English, QLocale::UnitedStates), QLocale(QLocale::Russian, QLocale::Russia), QLocale(QLocale::French, QLocale::France)};
        int checked = 0;
        for (const QString &language : formulaNameLanguages()) {
            for (const QLocale &locale : locales) {
                EntryLocale l(locale);
                l.language = language;
                for (const QString &row : rows) {
                    if (row.startsWith(QLatin1Char('#'))) continue;
                    const QString f = row + QStringLiteral("(A1,2.5,\"a,b;c\",{1.5,2;TRUE,#N/A},B2:C3)+") + row + QStringLiteral("()-FALSE*Sheet1!A1+#DIV/0!");
                    const QString shown = formulaToLocale(f, l);
                    QCOMPARE(formulaFromLocale(shown, l), f);
                    ++checked;
                }
                // every error and TRUE and FALSE
                for (const QString &w : {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A", "TRUE", "FALSE"}) {
                    const QString f = QStringLiteral("IF(") + w + QStringLiteral(",{") + w + QStringLiteral("},1)");
                    QCOMPARE(formulaFromLocale(formulaToLocale(f, l), l), f);
                }
            }
        }
        QVERIFY(checked > 4 * 10 * 250);
        // made in one language, shown in another: the file's form is the one thing between them
        EntryLocale german(de), spanish(QLocale(QLocale::Spanish, QLocale::Spain));
        german.language = QStringLiteral("de");
        spanish.language = QStringLiteral("es");
        const QString file = formulaFromLocale(QStringLiteral("WENN(SUMME(A1;A2)>1,5;WAHR;#WERT!)"), german);
        QCOMPARE(file, QStringLiteral("IF(SUM(A1,A2)>1.5,TRUE,#VALUE!)"));
        QCOMPARE(formulaToLocale(file, spanish), QStringLiteral("SI(SUMA(A1;A2)>1,5;VERDADERO;#¡VALOR!)"));
    }

    // Hostile text is bounded: a name too long to be one is left, mixed separators and unknown words pass through, and an unfinished array or text does not run on.
    void localNamesInHostileFormulasAreBounded()
    {
        EntryLocale l(de);
        l.language = QStringLiteral("de");
        const QString longName(5000, QLatin1Char('S'));
        QCOMPARE(formulaFromLocale(longName + QStringLiteral("(A1;2)"), l), longName + QStringLiteral("(A1,2)"));
        QCOMPARE(formulaToLocale(longName + QStringLiteral("(A1,2)"), l), longName + QStringLiteral("(A1;2)"));
        QCOMPARE(englishFormulaName(longName, QStringLiteral("de")), QString());
        QCOMPARE(formulaFromLocale(QStringLiteral("SUMME(A1,A2;3)+{1;2"), l), QStringLiteral("SUM(A1.A2,3)+{1;2"));
        QCOMPARE(formulaFromLocale(QStringLiteral("SUMME(\"offen;WAHR"), l), QStringLiteral("SUM(\"offen;WAHR"));
        QCOMPARE(formulaFromLocale(QStringLiteral("#NV#NV##WERT"), l), QStringLiteral("#N/A#N/A##WERT"));   // (a word after a "#" that is not an error stays)
        QCOMPARE(formulaFromLocale(QString(), l), QString());
        EntryLocale nothing(de);
        nothing.language = QStringLiteral("xx");   // (no such column: the English words)
        QCOMPARE(formulaToLocale(QStringLiteral("SUM(A1,2)"), nothing), QStringLiteral("SUM(A1;2)"));
        QCOMPARE(formulaFromLocale(QStringLiteral("SUMME(A1;2)"), nothing), QStringLiteral("SUMME(A1,2)"));
    }

    // An array typed where the comma is the decimal mark is read in the local form (columns by ";", rows by "\", a decimal comma) only when it holds a backslash outside its texts;
    // every other array is as ambiguous as the file's form ("{1,2;3,4}") and is read as it was before names had languages. One with more than one row is shown in the local form.
    void anArrayIsReadInTheLocalFormOnlyWhenItIsUnambiguous()
    {
        const QList<QLocale> commaLocales = {de, fr, QLocale(QLocale::Russian, QLocale::Russia)};
        for (const QString &language : {QStringLiteral("de"), QStringLiteral("fr")}) {
            for (const QLocale &locale : commaLocales) {
                EntryOptions o = options(locale);
                o.formulaLanguage = language;
                const QString what = language + QLatin1Char(' ') + locale.name();
                // the file's form, typed: kept as typed
                QCOMPARE(read(QStringLiteral("={1,2;3,4}"), o), QStringLiteral("f:{1,2;3,4}"));
                QCOMPARE(read(QStringLiteral("={1.5,2;3,4}"), o), QStringLiteral("f:{1.5,2;3,4}"));
                QCOMPARE(read(QStringLiteral("={1,2,3}"), o), QStringLiteral("f:{1,2,3}"));
                QCOMPARE(read(QStringLiteral("={1;2;3}"), o), QStringLiteral("f:{1;2;3}"));
                QCOMPARE(read(QStringLiteral("={\"a,b;c\",1;2,3}"), o), QStringLiteral("f:{\"a,b;c\",1;2,3}"));
                QCOMPARE(read(QStringLiteral("={TRUE,#N/A}"), o), QStringLiteral("f:{TRUE,#N/A}"));
                // the local form, with a backslash: read as local, decimal commas inside it, texts untouched
                QCOMPARE(read(QStringLiteral("={1,5;2\\3;4}"), o), QStringLiteral("f:{1.5,2;3,4}"));
                QCOMPARE(read(QStringLiteral("={1;2\\3;4}"), o), QStringLiteral("f:{1,2;3,4}"));
                QCOMPARE(read(QStringLiteral("={\"a;b,c\";2,5\\\"x\\\\y\";4}"), o), QStringLiteral("f:{\"a;b,c\",2.5;\"x\\\\y\",4}"));
                // a backslash inside a text is a text, so the array is still ambiguous
                QCOMPARE(read(QStringLiteral("={\"a\\\\b\",1;2,3}"), o), QStringLiteral("f:{\"a\\\\b\",1;2,3}"));
                // shown: more than one row is local, one row is as the file has it
                EntryLocale l(locale);
                l.language = language;
                QCOMPARE(formulaToLocale(QStringLiteral("SUM({1.5,2;3,4})"), l), QString(localFormulaName(QStringLiteral("SUM"), language) + QStringLiteral("({1,5;2\\3;4})")));
                QCOMPARE(formulaToLocale(QStringLiteral("{1.5,2,3}"), l), QStringLiteral("{1.5,2,3}"));
                QCOMPARE(formulaToLocale(QStringLiteral("{\"a;b\",1.5}"), l), QStringLiteral("{\"a;b\",1.5}"));
            }
        }
        // show, read, and show again, in every language and every kind of locale, for one-row, many-row, and text-bearing arrays
        const QStringList arrays = {"{1}", "{1.5}", "{1,2,3}", "{1.5,2}", "{1;2;3}", "{1.5,2;3,4}", "{1,2;3,4;5,6}", "{1.5,2;TRUE,#N/A}", "{\"a,b;c\",1;2,3}", "{\"\\\\\",1.5;FALSE,2}", "{TRUE,FALSE}", "{#DIV/0!;#N/A}", "{1E+5,2.5E-3;4,5}"};
        const QList<QLocale> locales = {de, fr, QLocale(QLocale::Russian, QLocale::Russia), QLocale(QLocale::English, QLocale::UnitedStates)};
        int checked = 0;
        for (const QString &language : formulaNameLanguages()) {
            for (const QLocale &locale : locales) {
                EntryLocale l(locale);
                l.language = language;
                for (const QString &array : arrays) {
                    const QString f = QStringLiteral("SUM(") + array + QStringLiteral(",A1)");
                    const QString shown = formulaToLocale(f, l);
                    QCOMPARE(formulaFromLocale(shown, l), f);
                    QCOMPARE(formulaToLocale(formulaFromLocale(shown, l), l), shown);
                    ++checked;
                }
            }
        }
        QVERIFY(checked == formulaNameLanguages().size() * 4 * arrays.size());
    }

    // Every error value, in the name the language shows and every other name it reads, is read in a formula and in an array, longest name first, and only as a whole name.
    void localErrorValuesAreReadByEveryNameAndOnlyWhole()
    {
        const QStringList errors = {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A"};
        int checked = 0;
        for (const QString &language : formulaNameLanguages()) {
            EntryLocale l(en);
            l.language = language;
            for (const QString &english : errors) {
                QStringList names = formulaNameAlternatives(english, language);
                names.prepend(localFormulaName(english, language));
                names.append(english);
                for (const QString &name : names) {
                    const QString what = language + QLatin1Char(' ') + name;
                    QVERIFY2(formulaFromLocale(QStringLiteral("IF(1,") + name + QStringLiteral(",2)"), l) == QStringLiteral("IF(1,") + english + QStringLiteral(",2)"), qPrintable(what));
                    QVERIFY2(formulaFromLocale(QStringLiteral("{") + name + QStringLiteral("}"), l) == QStringLiteral("{") + english + QStringLiteral("}"), qPrintable(what));
                    QVERIFY2(formulaFromLocale(name.toLower(), l) == english, qPrintable(what));
                    // not a whole name: a letter, a digit, or a dot goes on
                    for (const QString &more : {QStringLiteral("X"), QStringLiteral("5"), QStringLiteral(".")}) {
                        const QString text = QStringLiteral("1+") + name + more;
                        QVERIFY2(formulaFromLocale(text, l) == text, qPrintable(what + more));
                    }
                    ++checked;
                }
                // shown: the first name
                QCOMPARE(formulaToLocale(QStringLiteral("1+") + english, l), QStringLiteral("1+") + localFormulaName(english, language));
            }
        }
        QVERIFY(checked > 7 * 10);
        // the look-alikes: a local name that is the start of a longer one of the same or another error
        EntryLocale french(fr), brazil(QLocale(QLocale::Portuguese, QLocale::Brazil));
        french.language = QStringLiteral("fr");
        brazil.language = QStringLiteral("pt_BR");
        QCOMPARE(formulaFromLocale(QStringLiteral("#N/DISP"), french), QStringLiteral("#N/DISP"));
        QCOMPARE(formulaFromLocale(QStringLiteral("#N/D"), french), QStringLiteral("#N/A"));
        QCOMPARE(formulaFromLocale(QStringLiteral("#N/DISP"), brazil), QStringLiteral("#N/A"));
        QCOMPARE(formulaFromLocale(QStringLiteral("#N/D"), brazil), QStringLiteral("#N/A"));
        QCOMPARE(formulaFromLocale(QStringLiteral("#N/D+#N/DISP"), brazil), QStringLiteral("#N/A+#N/A"));
        QCOMPARE(formulaFromLocale(QStringLiteral("#NULL!"), french), QStringLiteral("#NULL!"));
        QCOMPARE(formulaFromLocale(QStringLiteral("#NUL!"), french), QStringLiteral("#NULL!"));
    }

    // A table that gives one local name two meanings, or a local name that is another function's English name, gives its language no local names: the English ones, not the last row.
    void aLanguageWhoseTableIsAmbiguousKeepsTheEnglishNames()
    {
        const QString head = QStringLiteral("#columns de fr\n");
        const QString bad = head + QStringLiteral("SUM SUMME SOMME\nSUMIF SUMME SOMME.SI\nAVERAGE MITTEL MOYENNE\n");
        QCOMPARE(localFormulaNameIn(bad, QStringLiteral("SUM"), QStringLiteral("de")), QStringLiteral("SUM"));
        QCOMPARE(localFormulaNameIn(bad, QStringLiteral("SUMIF"), QStringLiteral("de")), QStringLiteral("SUMIF"));
        QCOMPARE(localFormulaNameIn(bad, QStringLiteral("AVERAGE"), QStringLiteral("de")), QStringLiteral("AVERAGE"));
        QCOMPARE(englishFormulaNameIn(bad, QStringLiteral("SUMME"), QStringLiteral("de")), QString());
        QCOMPARE(englishFormulaNameIn(bad, QStringLiteral("MITTEL"), QStringLiteral("de")), QString());
        // (the other language of the table is sound and is used)
        QCOMPARE(localFormulaNameIn(bad, QStringLiteral("SUM"), QStringLiteral("fr")), QStringLiteral("SOMME"));
        QCOMPARE(englishFormulaNameIn(bad, QStringLiteral("moyenne"), QStringLiteral("fr")), QStringLiteral("AVERAGE"));
        // a local name that is another function's English name
        const QString clash = head + QStringLiteral("SUM SUMME SOMME\nSUMIF SUM SOMME.SI\n");
        QCOMPARE(localFormulaNameIn(clash, QStringLiteral("SUM"), QStringLiteral("de")), QStringLiteral("SUM"));
        QCOMPARE(localFormulaNameIn(clash, QStringLiteral("SUMIF"), QStringLiteral("de")), QStringLiteral("SUMIF"));
        QCOMPARE(localFormulaNameIn(clash, QStringLiteral("SUMIF"), QStringLiteral("fr")), QStringLiteral("SOMME.SI"));
        // a sound table is used whole, with the second names
        const QString good = head + QStringLiteral("SUM SUMME|SUMMEN SOMME\n");
        QCOMPARE(localFormulaNameIn(good, QStringLiteral("SUM"), QStringLiteral("de")), QStringLiteral("SUMME"));
        QCOMPARE(englishFormulaNameIn(good, QStringLiteral("summen"), QStringLiteral("de")), QStringLiteral("SUM"));
        // the table the program ships is sound, so every language keeps its names
        QVERIFY(formulaNameProblems().isEmpty());
        QCOMPARE(localFormulaName(QStringLiteral("SUM"), QStringLiteral("de")), QStringLiteral("SUMME"));
    }

    // A name the workbook defines does not change what a call is: as in English, a word followed by a bracket is a function, so a defined name equal to a local function name
    // is still read as that function there, and is a name where it stands alone.
    void aDefinedNameDoesNotTurnALocalFunctionNameIntoANameWithBrackets()
    {
        EntryOptions o = options(de);
        o.formulaLanguage = QStringLiteral("de");
        o.definedName = [](const QString &w) { return w.compare(QLatin1String("summe"), Qt::CaseInsensitive) == 0 || w.compare(QLatin1String("sum"), Qt::CaseInsensitive) == 0; };
        QCOMPARE(read(QStringLiteral("=SUMME(A1;A2)+SUMME"), o), QStringLiteral("f:SUM(A1,A2)+SUMME"));
        QCOMPARE(read(QStringLiteral("=SUM(A1;A2)+SUM"), o), QStringLiteral("f:SUM(A1,A2)+SUM"));
        EntryLocale l(de);
        l.language = QStringLiteral("de");
        l.definedName = o.definedName;
        QCOMPARE(formulaToLocale(QStringLiteral("SUM(A1,A2)+SUMME"), l), QStringLiteral("SUMME(A1;A2)+SUMME"));
    }

    // The digits of other scripts and the apostrophe of Swiss German are read as numbers: ar-EG, fa-IR, and de-CH.
    void nativeDigitsAndTheSwissApostropheAreRead()
    {
        const QLocale ar(QLocale::Arabic, QLocale::Egypt), fa(QLocale::Persian, QLocale::Iran), ch(QLocale::German, QLocale::Switzerland);
        auto number = [&](const QString &typed, const QLocale &l) {
            const EntryResult r = readEntry(typed, options(l));
            return r.ok && r.entry.kind == Entry::Kind::Number ? r.entry.number : -12345.0;
        };
        QCOMPARE(number(QString::fromUtf8("\u0663\u066b\u0665"), ar), 3.5);            // ٣٫٥
        QCOMPARE(number(QString::fromUtf8("\u0661\u0662\u0663"), ar), 123.0);
        QCOMPARE(number(QString::fromUtf8("-\u0661\u0662"), ar), -12.0);
        QCOMPARE(number(QString::fromUtf8("\u06f1\u06f2\u06f3"), fa), 123.0);          // ۱۲۳
        QCOMPARE(number(QStringLiteral("123"), ar), 123.0);                                  // (the digits of the keyboard are read too)
        const EntryResult percent = readEntry(QString::fromUtf8("\u0665\u066a"), options(ar));   // ٥٪
        QVERIFY(percent.ok && percent.entry.kind == Entry::Kind::Number);
        QCOMPARE(percent.entry.number, 0.05);
        QVERIFY(percent.entry.formatCode.contains(QLatin1Char('%')));
        // Swiss German: the apostrophe (typed as ' or as the right single quote the locale uses) groups the thousands, and the dot is the decimal point
        QCOMPARE(number(QStringLiteral("1'234.5"), ch), 1234.5);
        QCOMPARE(number(QString::fromUtf8("1\u2019234'567"), ch), 1234567.0);
        QCOMPARE(number(QStringLiteral("1'23"), ch), -12345.0);                             // (not a group of three: not a number)
        const EntryResult grouped = readEntry(QStringLiteral("1'234.50"), options(ch));
        QCOMPARE(grouped.entry.formatCode, QStringLiteral("#,##0.00"));
        // text that has digits of other scripts among letters is text, as it was typed
        const EntryResult word = readEntry(QString::fromUtf8("\u0663 \u0645\u0646"), options(ar));
        QVERIFY(word.ok && word.entry.kind == Entry::Kind::Text);
        QCOMPARE(word.entry.text, QString::fromUtf8("\u0663 \u0645\u0646"));
        // a text with an apostrophe in front is still a text
        QCOMPARE(read(QStringLiteral("'1'234"), options(ch)), QStringLiteral("t:1'234"));
    }

    // A formula that can't be read is told to the person in plain words that say what is wrong and where (no "token"), and the message is translatable (it has a context of its own).
    void aFormulaThatIsNoneIsToldInPlainWords()
    {
        const EntryOptions o = options(en);
        struct Case {
            QString typed;
            QString phrase;     // what the message says
            int at;             // the character it points at (counting the "=")
        };
        const Case cases[] = {
            {QStringLiteral("=1+2)"), QStringLiteral("There's a ) without a matching ("), 4},
            {QStringLiteral("=(1+2"), QStringLiteral("( is never closed"), 5},
            {QStringLiteral("=SUM(A1"), QStringLiteral("( is never closed"), 7},
            {QStringLiteral("=SUM(A1 2)"), QStringLiteral("is missing here"), 8},
            {QStringLiteral("=1+"), QStringLiteral("ends where a value is needed"), 3},
            {QStringLiteral("=*3"), QStringLiteral("An operator is where a value is needed"), 1},
            {QStringLiteral("=1,2"), QStringLiteral("comma"), 2},
            {QStringLiteral("=A1 5"), QStringLiteral("operator is missing"), 4},
            {QStringLiteral("=\"unclosed"), QStringLiteral("quote"), 1},
            {QStringLiteral("='Sheet 1"), QStringLiteral("apostrophe"), 1},
            {QStringLiteral("=#"), QStringLiteral("# starts an error"), 1},
            {QStringLiteral("=A1+$"), QStringLiteral("$ belongs in a cell reference"), 4},
            {QStringLiteral("=12abc"), QStringLiteral("can't run into letters"), 1},
            {QStringLiteral("=1+~"), QStringLiteral("can't be in a formula"), 3},
            {QStringLiteral("=NOW(1)"), QStringLiteral("NOW can't take the number of arguments given (1)"), 1},
            {QStringLiteral("=SUM({1,A1})"), QStringLiteral("only be numbers, texts"), 8},
            {QStringLiteral("=SUM({1,2;3})"), QStringLiteral("same number of values"), 12},
        };
        for (const Case &c : cases) {
            const EntryResult r = readEntry(c.typed, o);
            QVERIFY2(!r.ok, qPrintable(c.typed));
            QVERIFY2(r.problem.contains(c.phrase), qPrintable(c.typed + QLatin1String(" -> ") + r.problem));
            QVERIFY2(r.problem.contains(QLatin1String("character ") + QString::number(r.problemAt + 1)) || r.problem.contains(QLatin1String("end of what you typed")), qPrintable(c.typed + QLatin1String(" -> ") + r.problem));
            for (const char *jargon : {"token", "Unexpected", "Expected", "any real one", "grid"}) QVERIFY2(!r.problem.contains(QLatin1String(jargon)), qPrintable(c.typed + QLatin1String(" -> ") + r.problem));
            QVERIFY2(r.problem.endsWith(QLatin1Char('.')), qPrintable(r.problem));
            QVERIFY2(r.problemAt == c.at, qPrintable(c.typed + QLatin1String(" at ") + QString::number(r.problemAt)));
        }
        // a formula that is too long, and one nested too deep, are told too
        const EntryResult nested = readEntry(QStringLiteral("=") + QString(300, QLatin1Char('(')) + QStringLiteral("1") + QString(300, QLatin1Char(')')), o);
        QVERIFY(!nested.ok);
        QVERIFY2(nested.problem.contains(QLatin1String("too many")), qPrintable(nested.problem));
        // the messages are looked up for translation, in a context of their own
        class Translator : public QTranslator {
        public:
            QString translate(const char *context, const char *source, const char *, int) const override
            {
                return QLatin1String(context) == QLatin1String("jp::calc::Formula") ? QLatin1String("[x] ") + QLatin1String(source) : QString();
            }
        } translator;
        QCoreApplication::installTranslator(&translator);
        const EntryResult shown = readEntry(QStringLiteral("=1+2)"), o);
        QCoreApplication::removeTranslator(&translator);
        QVERIFY2(shown.problem.contains(QLatin1String("[x] ")), qPrintable(shown.problem));
    }

    void limits()
    {
        const PinnedLocale english;
        const EntryOptions o = options(en);
        QCOMPARE(read(QString(32767, QLatin1Char('x')), o).size(), 2 + 32767);
        const EntryResult r = readEntry(QString(32768, QLatin1Char('x')), o);
        QVERIFY(!r.ok);
        QVERIFY2(r.problem.contains(QLatin1String("32,767")) && r.problem.contains(QLatin1String("32,768")), qPrintable(r.problem));
        const EntryResult f = readEntry(QStringLiteral("=1") + QString::fromLatin1("+1").repeated(4100), o);
        QVERIFY(!f.ok);
        QVERIFY2(f.problem.contains(QLatin1String("8,192")), qPrintable(f.problem));
        // a formula of 8,192 characters is fine
        QVERIFY(readEntry(QStringLiteral("=1") + QString::fromLatin1("+1").repeated(4095) + QLatin1Char(' '), o).ok);
        // a long text is not read for a number or a date (and is not slow to refuse)
        QVERIFY(read(QString(20000, QLatin1Char('9')), o).startsWith(QLatin1String("t:")));
    }

    // A cell is edited as the text that gives its value back.
    void editTextGivesTheValueBack()
    {
        using jp::calc::Value;
        const EntryOptions o = options(en);
        QCOMPARE(editTextOfValue(Value::number(1.5), QStringLiteral("General"), o), QStringLiteral("1.5"));
        QCOMPARE(editTextOfValue(Value::number(1.5), QStringLiteral("General"), options(de)), QStringLiteral("1,5"));
        QCOMPARE(editTextOfValue(Value::number(0.1 + 0.2), QStringLiteral("0.00"), o), QStringLiteral("0.3"));
        QCOMPARE(editTextOfValue(Value::number(0.055), QStringLiteral("0.0%"), o), QStringLiteral("5.5%"));
        QCOMPARE(editTextOfValue(Value::number(0.05), QStringLiteral("0%"), o), QStringLiteral("5%"));
        QCOMPARE(editTextOfValue(Value::number(1234.5), QStringLiteral("\"$\"#,##0.00"), o), QStringLiteral("1234.5"));
        QCOMPARE(editTextOfValue(Value::number(serial(2025, 3, 4)), QStringLiteral("m/d/yyyy"), o), QStringLiteral("3/4/2025"));
        QCOMPARE(editTextOfValue(Value::number(serial(2025, 3, 4)), QStringLiteral("m/d/yyyy"), options(gb)), QStringLiteral("04/03/2025"));
        QCOMPARE(editTextOfValue(Value::number(serial(2025, 3, 4)), QStringLiteral("m/d/yyyy"), options(de)), QStringLiteral("04.03.2025"));
        QCOMPARE(editTextOfValue(Value::number(serial(2025, 3, 4) + 10.5 / 24), QStringLiteral("m/d/yyyy h:mm"), o), QStringLiteral("3/4/2025 10:30:00 AM"));
        QCOMPARE(editTextOfValue(Value::number(22.5 / 24), QStringLiteral("h:mm AM/PM"), o), QStringLiteral("10:30:00 PM"));
        QCOMPARE(editTextOfValue(Value::number(22.5 / 24), QStringLiteral("h:mm"), options(de)), QStringLiteral("22:30:00"));
        QCOMPARE(editTextOfValue(Value::boolean(true), QStringLiteral("General"), o), QStringLiteral("TRUE"));
        QCOMPARE(editTextOfValue(Value::error(ErrorKind::NA), QStringLiteral("General"), o), QStringLiteral("#N/A"));
        QCOMPARE(editTextOfValue(Value(), QStringLiteral("General"), o), QString());
        QCOMPARE(editTextOfValue(Value::text(QStringLiteral("hello")), QStringLiteral("General"), o), QStringLiteral("hello"));
        // text that would be read as something else gets the apostrophe
        for (const QString &t : {"123", "5%", "3/4/2025", "TRUE", "=1+2", "'quoted", "#N/A", "10:30", "$5", "-A1+2"})
            QCOMPARE(editTextOfValue(Value::text(t), QStringLiteral("General"), o), QStringLiteral("'") + t);
        EntryOptions text = o;
        text.textFormat = true;
        QCOMPARE(editTextOfValue(Value::text(QStringLiteral("123")), QStringLiteral("@"), text), QStringLiteral("123"));
        // and typing back what is shown gives the value
        struct Case {
            Value v;
            QString code;
            const QLocale *locale;
        };
        const Case cases[] = {{Value::number(1.5), "General", &en}, {Value::number(1.5), "General", &de}, {Value::number(1234567.891), "General", &fr}, {Value::number(0.055), "0.0%", &en},
                              {Value::number(serial(2025, 3, 4)), "m/d/yyyy", &en}, {Value::number(serial(2025, 3, 4)), "m/d/yyyy", &gb}, {Value::number(serial(2025, 3, 4)), "m/d/yyyy", &de},
                              {Value::number(serial(2025, 3, 4) + 0.4375), "m/d/yyyy h:mm", &en}, {Value::number(0.9375), "h:mm AM/PM", &en}, {Value::number(0.9375), "h:mm", &de},
                              {Value::number(-0.000123), "General", &en}, {Value::number(1e21), "General", &en}, {Value::boolean(false), "General", &en},
                              {Value::text(QStringLiteral("42")), "General", &en}, {Value::text(QStringLiteral("hello")), "General", &en}};
        for (const Case &c : cases) {
            const EntryOptions opt = options(*c.locale);
            const QString shown = editTextOfValue(c.v, c.code, opt);
            const EntryResult r = readEntry(shown, opt);
            QVERIFY2(r.ok, qPrintable(shown));
            switch (c.v.kind()) {
            case Value::Kind::Number:
                QVERIFY2(r.entry.kind == Entry::Kind::Number && sameTo15Digits(r.entry.number, c.v.number()), qPrintable(shown));
                break;
            case Value::Kind::Bool: QVERIFY2(r.entry.kind == Entry::Kind::Bool && r.entry.number == c.v.number(), qPrintable(shown)); break;
            default: QVERIFY2(r.entry.kind == Entry::Kind::Text && r.entry.text == c.v.text(), qPrintable(shown)); break;
            }
        }
    }
};

JP_TEST_CLASS(SheetEntryTests)
#include "tst_sheet_entry.moc"
