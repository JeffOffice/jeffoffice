// Tests of the number-format engine (libs/base/calc/numfmt.h): what a value looks like in a format code, by the format's rules. Every expected text is
// worked out from the rules (ECMA-376 18.8.30 and 18.8.31 and the behavior workbooks depend on), never from what the engine gave.
#include "sheethelpers.h"

#include "calc/numfmt.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

namespace {

// The text of `value` in `code`.
QString fmt(const QString &code, double value, DateSystem system = DateSystem::Date1900, int width = 0)
{
    return NumberFormat(code).format(value, system, width).text;
}

QString fmtText(const QString &code, const QString &text) { return NumberFormat(code).formatText(text).text; }

struct C {
    const char *code;
    double value;
    const char *want;
};

// 2024-03-15 18:00:00, a Friday.
const double kD = 45366.75;

} // namespace

#define CHECK(code, value, want)                                                                                                                       \
    do {                                                                                                                                                 \
        const QString got_ = fmt(QString::fromUtf8(code), (value));                                                                                      \
        const QString want_ = QString::fromUtf8(want);                                                                                                   \
        QVERIFY2(got_ == want_, qPrintable(QStringLiteral("code [%1] value %2: got [%3], wanted [%4]").arg(QString::fromUtf8(code)).arg((value), 0, 'g', 17).arg(got_, want_))); \
    } while (0)

#define CHECK_ALL(table)                                                                                                                                 \
    for (const C &c_ : table) CHECK(c_.code, c_.value, c_.want)

class SheetNumberFormatTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Up to four sections (positive; negative; zero; text) with the defaults when there are fewer, and conditions that choose the section.
    void sectionsAndConditions()
    {
        const C cases[] = {
            {"0.00", 1.5, "1.50"},
            {"0.00", -1.5, "-1.50"},
            {"0.0;(0.0)", 5, "5.0"},
            {"0.0;(0.0)", -5, "(5.0)"},
            {"0.0;(0.0)", 0, "0.0"},
            {"0.0;(0.0);zero", 0, "zero"},
            {"0.0;(0.0);zero", 5, "5.0"},
            {"0.0;(0.0);zero", -5, "(5.0)"},
            {"0.0;\"neg\"", -3, "neg"},
            {"#,##0;-#,##0;\"-\"", 0, "-"},
            {"#,##0;-#,##0;\"-\"", -1234, "-1,234"},
            // The second section takes the sign's place, so it shows none of its own.
            {"0;0;0", -5, "5"},
            {"0;0", -5, "5"},
            {"[>100]\"big\";\"small\"", 150, "big"},
            {"[>100]\"big\";\"small\"", 50, "small"},
            {"[>=1000000]0.0,,\"M\";[>=1000]0.0,\"K\";0", 1500000, "1.5M"},
            {"[>=1000000]0.0,,\"M\";[>=1000]0.0,\"K\";0", 2500, "2.5K"},
            {"[>=1000000]0.0,,\"M\";[>=1000]0.0,\"K\";0", 999, "999"},
            {"[>=1000000]0.0,,\"M\";[>=1000]0.0,\"K\";0", 0, "0"},
            {"[>=1000000]0.0,,\"M\";[>=1000]0.0,\"K\";0", -5, "-5"},
            {"[=0]\"zero\";0.0", 0, "zero"},
            {"[=0]\"zero\";0.0", 3, "3.0"},
            {"[<>5]0;\"five\"", 5, "five"},
            {"[<>5]0;\"five\"", 4, "4"},
            {"[<=9999999]###-####;(###) ###-####", 5551234, "555-1234"},
            {"[<=9999999]###-####;(###) ###-####", 5551234567.0, "(555) 123-4567"},
            {"[>5]\"high\";[<-5]\"low\";\"mid\"", 10, "high"},
            {"[>5]\"high\";[<-5]\"low\";\"mid\"", 3, "mid"},
            {"[<1]0.00;0", 0.5, "0.50"},
            {"[<1]0.00;0", 5, "5"},
            {"[>100]0.0;[>10]0.00;0", 150, "150.0"},
            {"[>100]0.0;[>10]0.00;0", 50, "50.00"},
            {"[>100]0.0;[>10]0.00;0", 5, "5"},
            {"[<0]\"neg\";\"pos\"", -2, "neg"},
            // The section is chosen by the value, not by what it rounds to.
            {"0;-0;\"zero\"", 0.001, "0"},
            {"0;-0;\"zero\"", -0.001, "-0"},
        };
        CHECK_ALL(cases);
        // No section takes a value: it can't be shown.
        QVERIFY(NumberFormat(QStringLiteral("[>10]0;[<-10]0")).format(5).failed);
        QCOMPARE(fmt(QStringLiteral("[>10]0;[<-10]0"), 50), QStringLiteral("50"));
        // Text: the fourth section, else the text as it is.
        QCOMPARE(fmtText(QStringLiteral("0.00"), QStringLiteral("abc")), QStringLiteral("abc"));
        QCOMPARE(fmtText(QStringLiteral("0;0;0;\"text: \"@"), QStringLiteral("hi")), QStringLiteral("text: hi"));
        QCOMPARE(fmtText(QStringLiteral("@\"!\""), QStringLiteral("abc")), QStringLiteral("abc!"));
        QCOMPARE(fmtText(QStringLiteral("0;0;0;@@"), QStringLiteral("ab")), QStringLiteral("abab"));
        QCOMPARE(fmtText(QStringLiteral("0;0;0;\"[\"@\"]\""), QStringLiteral("abc")), QStringLiteral("[abc]"));
        QCOMPARE(fmtText(QStringLiteral(";;;"), QStringLiteral("abc")), QString());
        QCOMPARE(fmtText(QStringLiteral("0;0;0;"), QStringLiteral("abc")), QString());
        QCOMPARE(fmtText(QStringLiteral("0;0;0;\"none\""), QStringLiteral("abc")), QStringLiteral("none"));
        QCOMPARE(fmtText(QStringLiteral("_(@_)"), QStringLiteral("abc")), QStringLiteral(" abc "));
        QCOMPARE(fmtText(QStringLiteral("General;General;General;@"), QStringLiteral("abc")), QStringLiteral("abc"));
        // A text-only code shows a number as General does; every section empty shows nothing.
        QCOMPARE(fmt(QStringLiteral("@"), 1.5), QStringLiteral("1.5"));
        QCOMPARE(fmt(QStringLiteral(";;;"), 5), QString());
        QVERIFY(!NumberFormat(QStringLiteral(";;;")).format(5).failed);
    }

    // When the last section is the text section, every number uses the sections before it as if it were not there: one section takes all numbers, with the
    // minus sign a code of one section has.
    void aLastTextSectionTakesNoNumbers()
    {
        const C cases[] = {
            {"m/d/yy;@", 45366, "3/15/24"},
            {"0.00;@", -5, "-5.00"},
            {"0.00;@", 5, "5.00"},
            {"0.00;@", 0, "0.00"},
            {"#,##0;@", -1234, "-1,234"},
            {"0;-0;@", 5, "5"},
            {"0;-0;@", -5, "-5"},
            {"0;-0;@", 0, "0"},
            {"0;(0);@", -5, "(5)"},
            {"0.0;@\"!\"", -2.5, "-2.5"},
            {"[Red]0.00;@", -5, "-5.00"},
            {"General;@", -5, "-5"},
            {"0%;@", -0.5, "-50%"},
            {"0.00E+00;@", -12345, "-1.23E+04"},
            // Three sections that are all for numbers are as they were.
            {"0;-0;\"zero\"", -5, "-5"},
            {"0;-0;\"zero\"", 0, "zero"},
        };
        CHECK_ALL(cases);
        QVERIFY(NumberFormat(QStringLiteral("m/d/yy;@")).format(-1).failed);
        QVERIFY(!NumberFormat(QStringLiteral("m/d/yy;@")).format(1).failed);
        QCOMPARE(NumberFormat(QStringLiteral("[Red]0.00;@")).format(-5).color, 3);
        QCOMPARE(NumberFormat(QStringLiteral("0.00;[Blue]0.00;@")).format(-5).color, 5);
        QCOMPARE(fmt(QStringLiteral("0.00;[Blue]0.00;@"), -5), QStringLiteral("5.00"));
        QCOMPARE(fmtText(QStringLiteral("0.00;@"), QStringLiteral("abc")), QStringLiteral("abc"));
        QCOMPARE(fmtText(QStringLiteral("0.00;\"t: \"@"), QStringLiteral("abc")), QStringLiteral("t: abc"));
        QCOMPARE(fmtText(QStringLiteral("m/d/yy;@"), QStringLiteral("abc")), QStringLiteral("abc"));
    }

    // One section with a condition takes the numbers that meet it and no others, as the first of two conditioned sections does: the rest can't be shown.
    void aSingleConditionedSectionTakesWhatMatches()
    {
        const C cases[] = {
            {"[>100]0.0", 150, "150.0"},
            {"[>=100]0.0", 100, "100.0"},
            {"[<0]0.0", -5, "5.0"},
            {"[=0]\"zero\"", 0, "zero"},
            {"[<>5]0", 4, "4"},
            {"[>100]0.0;@", 150, "150.0"},
            {"[Red][>100]0", 150, "150"},
        };
        CHECK_ALL(cases);
        for (const char *code : {"[>100]0.0", "[>=100]0.0", "[<0]0.0", "[=0]\"zero\"", "[<>5]0", "[>100]0.0;@", "[Red][>100]0"}) {
            const double miss = QString::fromLatin1(code).contains(QLatin1String("<>")) ? 5 : QString::fromLatin1(code).contains(QLatin1String("[=0]")) ? 1 : QString::fromLatin1(code).contains(QLatin1String("[<0]")) ? 5 : 50;
            const Formatted f = NumberFormat(QString::fromLatin1(code)).format(miss);
            QVERIFY2(f.failed && f.text.isEmpty(), code);
        }
        // Without a condition one section takes everything, and two conditioned sections that miss fail as before.
        QCOMPARE(fmt(QStringLiteral("0.0"), -50), QStringLiteral("-50.0"));
        QVERIFY(NumberFormat(QStringLiteral("[>10]0;[<-10]0")).format(5).failed);
    }

    // [Red], [Blue], ..., [Color1] to [Color56] come back as the palette number; the RGB of each is known.
    void colors()
    {
        struct K {
            const char *code;
            double v;
            int color;
            const char *text;
        };
        const K cases[] = {
            {"[Red]0.0;[Blue]-0.0", 5, 3, "5.0"},
            {"[Red]0.0;[Blue]-0.0", -5, 5, "-5.0"},
            {"[Color12]0", 1, 12, "1"},
            {"[Black]0", 1, 1, "1"},
            {"[White]0", 1, 2, "1"},
            {"[Red]0", 1, 3, "1"},
            {"[Green]0", 1, 4, "1"},
            {"[Blue]0", 1, 5, "1"},
            {"[Yellow]0", 1, 6, "1"},
            {"[Magenta]0", 1, 7, "1"},
            {"[Cyan]0", 1, 8, "1"},
            {"[red]0", 1, 3, "1"},
            {"[RED]0", 1, 3, "1"},
            {"[Color56]0", 1, 56, "1"},
            {"[Color1]0", 1, 1, "1"},
            {"[Color57]0", 1, 0, "1"},
            {"[Color0]0", 1, 0, "1"},
            {"0;0;[Cyan]0", 0, 8, "0"},
            {"0;0;[Cyan]0", 2, 0, "2"},
            {"[Red][>100]0;0", 150, 3, "150"},
            {"[>100][Red]0;0", 150, 3, "150"},
            {"[Red][>100]0;0", 50, 0, "50"},
            {"#,##0 ;[Red](#,##0)", -1234, 3, "(1,234)"},
            {"General;[Red]-General", -5, 3, "-5"},
        };
        for (const K &k : cases) {
            const Formatted f = NumberFormat(QString::fromUtf8(k.code)).format(k.v);
            QVERIFY2(f.text == QString::fromUtf8(k.text) && f.color == k.color,
                     qPrintable(QStringLiteral("code [%1] value %2: got [%3] color %4, wanted [%5] color %6").arg(QString::fromUtf8(k.code)).arg(k.v).arg(f.text).arg(f.color).arg(QString::fromUtf8(k.text)).arg(k.color)));
        }
        const Formatted t = NumberFormat(QStringLiteral("0;0;0;[Green]@")).formatText(QStringLiteral("abc"));
        QCOMPARE(t.text, QStringLiteral("abc"));
        QCOMPARE(t.color, 4);
        QCOMPARE(formatColorRgb(1), quint32(0x000000));
        QCOMPARE(formatColorRgb(2), quint32(0xFFFFFF));
        QCOMPARE(formatColorRgb(3), quint32(0xFF0000));
        QCOMPARE(formatColorRgb(4), quint32(0x00FF00));
        QCOMPARE(formatColorRgb(5), quint32(0x0000FF));
        QCOMPARE(formatColorRgb(6), quint32(0xFFFF00));
        QCOMPARE(formatColorRgb(7), quint32(0xFF00FF));
        QCOMPARE(formatColorRgb(8), quint32(0x00FFFF));
        QCOMPARE(formatColorRgb(9), quint32(0x800000));
        QCOMPARE(formatColorRgb(56), quint32(0x333333));
        QCOMPARE(formatColorRgb(0), quint32(0));
        QCOMPARE(formatColorRgb(57), quint32(0));
    }

    // Text in quotes, a backslash, the characters that need no quotes, _x (a space as wide as x), and *x (fill): where the fill goes.
    void literalsPadsAndFill()
    {
        const C cases[] = {
            {"0 \"units\"", 5, "5 units"},
            {"\"abc\"0", 5, "abc5"},
            {"0\\k\\g", 5, "5kg"},
            {"\\#0", 5, "#5"},
            {"\"0.00\"", 5, "0.00"},
            {"$0.00", 5, "$5.00"},
            {"$#,##0.00", -1234.5, "-$1,234.50"},
            {"(0)", 5, "(5)"},
            {"0 \"€\"", 5, "5 €"},
            {"0.00\" USD\"", 1.5, "1.50 USD"},
            {"+0;-0;0", 5, "+5"},
            {"+0;-0;0", -5, "-5"},
            {"0_)", 5, "5 "},
            {"_(0_)", 5, " 5 "},
            {"0\\%", 5, "5%"},
            {"0 ! & ' ~ { } < > = ^ : / -", 5, "5 ! & ' ~ { } < > = ^ : / -"},
            {"\"a\"\"b\"0", 1, "ab1"},
            {"General\" units\"", 5, "5 units"},
            {"\"x\"General", 1.5, "x1.5"},
            {"General_)", 5, "5 "},
        };
        CHECK_ALL(cases);
        {
            const Formatted f = NumberFormat(QStringLiteral("_(* #,##0_)")).format(1234);
            QCOMPARE(f.text, QStringLiteral(" 1,234 "));
            QCOMPARE(f.fillAt, 1);
            QCOMPARE(f.fillChar, QLatin1Char(' '));
            QCOMPARE(f.pads.size(), 2);
            QCOMPARE(f.pads[0].at, 0);
            QCOMPARE(f.pads[0].like, QLatin1Char('('));
            QCOMPARE(f.pads[1].at, 6);
            QCOMPARE(f.pads[1].like, QLatin1Char(')'));
        }
        {
            const Formatted f = NumberFormat(QStringLiteral("0*-")).format(5);
            QCOMPARE(f.text, QStringLiteral("5"));
            QCOMPARE(f.fillAt, 1);
            QCOMPARE(f.fillChar, QLatin1Char('-'));
        }
        {
            const Formatted f = NumberFormat(QStringLiteral("*x0")).format(5);
            QCOMPARE(f.text, QStringLiteral("5"));
            QCOMPARE(f.fillAt, 0);
            QCOMPARE(f.fillChar, QLatin1Char('x'));
        }
        // Only the first fill counts.
        QCOMPARE(NumberFormat(QStringLiteral("*a0*b")).format(5).fillChar, QLatin1Char('a'));
        QCOMPARE(NumberFormat(QStringLiteral("0")).format(5).fillAt, -1);
    }

    // 0 # ? with the decimal point, groups of three, a division by 1,000 for each comma after the digits, and the carry that adds a digit.
    void digitsPointsAndGroups()
    {
        const C cases[] = {
            {"0", 0.4, "0"},
            {"0", 0.5, "1"},
            {"0", 2.5, "3"},
            {"0", -2.5, "-3"},
            {"0", 1234.5, "1235"},
            {"0.00", 3.14159, "3.14"},
            {"0.00", 0, "0.00"},
            {"0.00", 1.5, "1.50"},
            {"0.00", -1.5, "-1.50"},
            {"#.##", 0.5, ".5"},
            {"#.##", 3, "3."},
            {"#.00", 0.5, ".50"},
            {"0.##", 3.1, "3.1"},
            {"0.##", 3, "3."},
            {"0.0#", 3, "3.0"},
            {"0.0#", 3.14, "3.14"},
            {"0.0#", 3.149, "3.15"},
            {"#", 0, ""},
            {"#", 7, "7"},
            {".00", 5, "5.00"},
            {"00000", 42, "00042"},
            {"0000", 12345, "12345"},
            {"?.??", 3.5, "3.5 "},
            {"??.??", 3.5, " 3.5 "},
            {"0.0?", 1.5, "1.5 "},
            {"???", 5, "  5"},
            {"#,##0", 1234567.891, "1,234,568"},
            {"#,##0.00", -1234.5, "-1,234.50"},
            {"#,###", 0, ""},
            {"#,###", 1000, "1,000"},
            {"#,##0", 999, "999"},
            {"#,##0", 1000, "1,000"},
            {"0,000", 1234567, "1,234,567"},
            {"0,000", 5, "0,005"},
            {"#,##0", 0, "0"},
            {"#,##0", 999999.5, "1,000,000"},
            {"0.0", 99.95, "100.0"},
            {"0", 999999.5, "1000000"},
            {"0,", 1500, "2"},
            {"0,", 0, "0"},
            {"0.0,", 1234567, "1234.6"},
            {"#,##0,", 1234567, "1,235"},
            {"0.0,,", 1234567890, "1234.6"},
            {"0,.0", 1234567, "1234.6"},
            {"0.0,\"K\"", 1500, "1.5K"},
            {"#,##0,,\"M\"", 2500000, "3M"},
            {"0.00", -0.001, "-0.00"},
            {"0", -0.4, "-0"},
            {"0", 1e15, "1000000000000000"},
            {"0", 123456789012345678.0, "123456789012346000"},
            {"0", 1e-10, "0"},
            {"0-0", 123, "12-3"},
            {"00-00", 5, "00-05"},
            {"###-####", 5551234, "555-1234"},
            {"###-####", 123, "-123"},
            {"(###) ###-####", 5551234567.0, "(555) 123-4567"},
            {"0.00000000000000000000", 1.0 / 3, "0.33333333333333300000"},
            {"0.0000000000000000", 0.1 + 0.2, "0.3000000000000000"},
            {"#,##0.0", 1e15, "1,000,000,000,000,000.0"},
        };
        CHECK_ALL(cases);
    }

    // The value is rounded from its 15 significant digits as decimal text (half away from zero), not from the binary value.
    void roundingFollowsTheDecimalForm()
    {
        const C cases[] = {
            {"0.00", 2.675, "2.68"},      // 2.67499999999999982236431605997495353221893310546875 in binary
            {"0.00", 1.005, "1.01"},      // 1.00499999999999989...
            {"0.00", -2.675, "-2.68"},
            {"0.0", 0.05, "0.1"},
            {"0.0", 0.15, "0.2"},         // 0.1499999999999999944...
            {"0.0", 0.25, "0.3"},
            {"0.0", 0.35, "0.4"},         // 0.34999999999999997...
            {"0.0", 0.45, "0.5"},
            {"0.00", 0.285, "0.29"},
            {"0.00", 8.345, "8.35"},
            {"0.0", 1.45, "1.5"},
            {"0", 0.5, "1"},
            {"0", 1.5, "2"},
            {"0", 2.5, "3"},
            {"0", 3.5, "4"},
            {"0", -0.5, "-1"},
            {"0%", 0.145, "15%"},         // 14.499999999999998 in binary
            {"0.0%", 0.285, "28.5%"},
            {"0%", 0.07, "7%"},
            {"0%", 0.005, "1%"},
            {"0.000", 1.0005, "1.001"},
            {"0.0000", 0.00005, "0.0001"},
            {"0.0", 0.04999999999999999, "0.1"},      // 15 digits: 0.0500000000000000
            {"0.0", 0.0499999999999, "0.0"},
            {"0.00", 1234567.895, "1234567.90"},
            {"#,##0.00", 1234567.895, "1,234,567.90"},
            // 15 digits only: the 16th and later are lost, and shown as zeros.
            {"0.000000000000000000", 1.1, "1.100000000000000000"},
            {"0", 1234567890123456.0, "1234567890123460"},
            {"0", 0.1 + 0.2, "0"},
        };
        CHECK_ALL(cases);
        // Thousandths n/1000 rounded to hundredths by integer arithmetic (half away from zero): the half of 5 thousandths is the tie that binary gets wrong.
        std::mt19937_64 rng(1000);
        for (int i = 0; i < 20000; ++i) {
            const qint64 n = qint64(rng() % 2000000000ULL) - 1000000000LL;
            const double v = double(n) / 1000.0;
            const qint64 a = n < 0 ? -n : n;
            const qint64 hundredths = (a + 5) / 10;
            QString want = QStringLiteral("%1.%2").arg(hundredths / 100).arg(hundredths % 100, 2, 10, QLatin1Char('0'));
            if (n < 0) want.prepend(QLatin1Char('-'));
            QVERIFY2(fmt(QStringLiteral("0.00"), v) == want, qPrintable(QStringLiteral("%1: got [%2], wanted [%3]").arg(n).arg(fmt(QStringLiteral("0.00"), v), want)));
            // The same with groups of three, and as a percent (a thousandth is a tenth of a percent, so tenths of a percent are exact).
            QString grouped = QString::number(hundredths / 100);
            for (int k = int(grouped.size()) - 3; k > 0; k -= 3) grouped.insert(k, QLatin1Char(','));
            QString wantGrouped = QStringLiteral("%1.%2").arg(grouped).arg(hundredths % 100, 2, 10, QLatin1Char('0'));
            if (n < 0) wantGrouped.prepend(QLatin1Char('-'));
            QVERIFY2(fmt(QStringLiteral("#,##0.00"), v) == wantGrouped, qPrintable(QStringLiteral("%1: got [%2], wanted [%3]").arg(n).arg(fmt(QStringLiteral("#,##0.00"), v), wantGrouped)));
        }
    }

    // "%" multiplies by 100 each time; E+ E- E show a value in scientific notation, with the exponent a multiple of the integer places.
    void percentAndScientific()
    {
        const C cases[] = {
            {"0%", 0.256, "26%"},
            {"0.0%", 0.1234, "12.3%"},
            {"0.00%", 1, "100.00%"},
            {"0%", -0.25, "-25%"},
            {"0%", 0, "0%"},
            {"%0", 0.5, "%50"},
            {"0.00E+00", 12345.678, "1.23E+04"},
            {"0.00E+00", 0.000123, "1.23E-04"},
            {"0.0E+0", 12345, "1.2E+4"},
            {"0E+0", 5, "5E+0"},
            {"0.00E-00", 12345.678, "1.23E04"},
            {"0.00E-00", 0.000123, "1.23E-04"},
            {"##0.0E+0", 12345, "12.3E+3"},
            {"##0.0E+0", 0.00012345, "123.5E-6"},
            {"##0.0E+0", 1234567, "1.2E+6"},
            {"##0.0E+0", 100, "100.0E+0"},
            {"0.00E+00", 0, "0.00E+00"},
            {"0.00E+00", -12345, "-1.23E+04"},
            {"0.00E+00", 9.999, "1.00E+01"},
            {"0.00e+00", 12345, "1.23e+04"},
            {"#.##E+0", 12345, "1.23E+4"},
            {"0.00E+000", 12345, "1.23E+004"},
            {"0.0E+0", 1e100, "1.0E+100"},
            {"0.00E+00", 1e-100, "1.00E-100"},
            {"0.00E+00", 1234.5678, "1.23E+03"},
        };
        CHECK_ALL(cases);
    }

    // Fractions with up to as many digits as the code has places, and with a fixed denominator.
    void fractions()
    {
        const C cases[] = {
            {"# ?/?", 1.5, "1 1/2"},
            {"# ?/?", 3.14159, "3 1/7"},
            {"# ?" "?/??", 3.14159, "3 14/99"},
            {"# ??" "?/???", 3.14159, "3  16/113"},
            {"# ?/8", 3.14159, "3 1/8"},
            {"# ?/4", 2.5, "2 2/4"},
            {"# ?/2", 1.25, "1 1/2"},
            {"0 ?" "?/100", 0.456, "0 46/100"},
            {"?/16", 0.5, "8/16"},
            {"?/?", 0.75, "3/4"},
            {"?/?", 2.5, "5/2"},
            {"# ?/?", -1.5, "-1 1/2"},
            {"# ?/?", 2, "2    "},
            {"# ?/?", 2.99, "3    "},
            {"0 ?/?", 0.45, "0 4/9"},
            {"0 ?" "?/??", 0.3333, "0  1/3 "},
            {"# ?" "?/??", 12.25, "12  1/4 "},
            {"# ?/?", 0, "0    "},
        };
        CHECK_ALL(cases);
    }

    // d dd ddd dddd, m mm mmm mmmm mmmmm, yy yyyy, h hh, s ss, AM/PM and A/P, the fractions of a second; m is minutes after h and before s.
    void datesAndTimes()
    {
        const C cases[] = {
            {"yyyy-mm-dd", kD, "2024-03-15"},
            {"d/m/yy", kD, "15/3/24"},
            {"dd-mmm-yy", kD, "15-Mar-24"},
            {"dddd, mmmm d, yyyy", kD, "Friday, March 15, 2024"},
            {"ddd", kD, "Fri"},
            {"mmmmm", kD, "M"},
            {"mmm", kD, "Mar"},
            {"yy", kD, "24"},
            {"y", kD, "24"},
            {"yyy", kD, "2024"},
            {"YYYY-MM-DD", kD, "2024-03-15"},
            {"DD/MM/YYYY", kD, "15/03/2024"},
            {"m", kD, "3"},
            {"mm", kD, "03"},
            {"d", 45356, "5"},
            {"dd", 45356, "05"},
            {"yyyy-mm", kD, "2024-03"},
            {"mmmm yyyy", kD, "March 2024"},
            {"mmmm\\ d\\,\\ yyyy", kD, "March 15, 2024"},
            {"d.m.yyyy", kD, "15.3.2024"},
            {"h:mm AM/PM", kD, "6:00 PM"},
            {"hh:mm:ss", kD, "18:00:00"},
            {"h:mm:ss AM/PM", kD, "6:00:00 PM"},
            {"h AM/PM", kD, "6 PM"},
            {"h:mm A/P", kD, "6:00 P"},
            {"h:mm am/pm", kD, "6:00 pm"},
            {"h:mm a/p", kD, "6:00 p"},
            {"h:mm AM/PM", 0.5, "12:00 PM"},
            {"h:mm AM/PM", 0, "12:00 AM"},
            {"h:mm AM/PM", 0.25, "6:00 AM"},
            {"h:mm AM/PM", 86399.0 / 86400, "11:59 PM"},
            {"m/d/yyyy h:mm", kD, "3/15/2024 18:00"},
            {"mm/dd hh:mm", kD, "03/15 18:00"},
            {"d mmm yyyy hh:mm", kD, "15 Mar 2024 18:00"},
            {"mm:ss", 0.0007, "01:00"},
            {"m:ss", 0.0007, "1:00"},
            {"mm:ss", kD, "00:00"},
            {"ss.00", 1.0 / 86400, "01.00"},
            {"hh:mm:ss.0", 0.75 + 0.5 / 86400, "18:00:00.5"},
            {"h:mm:ss.000", 0.5 + 12.345 / 86400, "12:00:12.345"},
            {"hh:mm AM/PM", 0.5, "12:00 PM"},
            {"h:mm", 1.5, "12:00"},
            {"h:mm", 0.0001, "0:00"},
            {"[$-409]dddd", kD, "Friday"},
            // Time is rounded to the least unit the code shows: to the second, and then the day moves on; a code with no time drops it.
            {"yyyy-mm-dd hh:mm:ss", 45366.999999, "2024-03-16 00:00:00"},
            {"yyyy-mm-dd", 45366.999999, "2024-03-15"},
            {"hh:mm:ss", 45366.999999, "00:00:00"},
            {"h:mm", 0.9999, "23:59"},
            {"hh:mm:ss.0", 0.99999999, "00:00:00.0"},
        };
        CHECK_ALL(cases);
    }

    // [h] [hh] [m] [mm] [s] [ss]: the whole time, in hours, minutes, or seconds, and not the part of a day.
    void elapsedTime()
    {
        const C cases[] = {
            {"[h]:mm:ss", 1.5, "36:00:00"},
            {"[h]:mm", 2.75, "66:00"},
            {"[mm]:ss", 0.1, "144:00"},
            {"[ss]", 0.001, "86"},
            {"[hh]:mm", 0.25, "06:00"},
            {"[h]", kD, "1088802"},
            {"[m]", 1, "1440"},
            {"[s]", 1, "86400"},
            {"[h]:mm", 0.999999, "24:00"},
            {"[h]:mm:ss", 0, "0:00:00"},
            {"[h]:mm:ss.00", 1.0 / 24 + 0.5 / 86400, "1:00:00.50"},
            {"[H]:MM", 0.5, "12:00"},
        };
        CHECK_ALL(cases);
        QVERIFY(NumberFormat(QStringLiteral("[h]:mm")).format(-1).failed);
        QVERIFY(NumberFormat(QStringLiteral("yyyy")).format(-1).failed);
        QVERIFY(NumberFormat(QStringLiteral("yyyy")).format(-1).text.isEmpty());
    }

    // Both date systems, the February 29, 1900 that never was, and what lies outside.
    void dateSystems()
    {
        struct S {
            double serial;
            const char *date1900;
            const char *day1900;
        };
        const S s1900[] = {
            {0, "1900-01-00", "Saturday"},
            {1, "1900-01-01", "Sunday"},
            {31, "1900-01-31", "Tuesday"},
            {59, "1900-02-28", "Tuesday"},
            {60, "1900-02-29", "Wednesday"},
            {61, "1900-03-01", "Thursday"},
            {366, "1900-12-31", "Monday"},
            {367, "1901-01-01", "Tuesday"},
            {1462, "1904-01-01", "Friday"},
            {36585, "2000-02-29", "Tuesday"},
            {45366, "2024-03-15", "Friday"},
            {2958465, "9999-12-31", "Friday"},
        };
        for (const S &s : s1900) {
            QCOMPARE(fmt(QStringLiteral("yyyy-mm-dd"), s.serial), QString::fromLatin1(s.date1900));
            QCOMPARE(fmt(QStringLiteral("dddd"), s.serial), QString::fromLatin1(s.day1900));
        }
        QCOMPARE(fmt(QStringLiteral("d/m/yyyy"), 60), QStringLiteral("29/2/1900"));
        QCOMPARE(fmt(QStringLiteral("d/m/yyyy"), 0), QStringLiteral("0/1/1900"));
        const S s1904[] = {
            {0, "1904-01-01", "Friday"},
            {1, "1904-01-02", "Saturday"},
            {58, "1904-02-28", "Sunday"},
            {59, "1904-02-29", "Monday"},
            {1462, "1908-01-02", "Thursday"},
            {43904, "2024-03-15", "Friday"},
            {2957003, "9999-12-31", "Friday"},
        };
        for (const S &s : s1904) {
            QCOMPARE(fmt(QStringLiteral("yyyy-mm-dd"), s.serial, DateSystem::Date1904), QString::fromLatin1(s.date1900));
            QCOMPARE(fmt(QStringLiteral("dddd"), s.serial, DateSystem::Date1904), QString::fromLatin1(s.day1900));
        }
        // The same number is two dates.
        QCOMPARE(fmt(QStringLiteral("yyyy-mm-dd"), 1000), QStringLiteral("1902-09-26"));
        QCOMPARE(fmt(QStringLiteral("yyyy-mm-dd"), 1000, DateSystem::Date1904), QStringLiteral("1906-09-27"));
        QCOMPARE(fmt(QStringLiteral("h:mm:ss"), 0.75, DateSystem::Date1904), QStringLiteral("18:00:00"));
        // Past December 31, 9999 and before day 0: no date. In the 1904 system a time before day 0 shows with its minus sign.
        QVERIFY(NumberFormat(QStringLiteral("yyyy-mm-dd")).format(2958466).failed);
        QVERIFY(NumberFormat(QStringLiteral("yyyy-mm-dd")).format(-0.5).failed);
        QVERIFY(NumberFormat(QStringLiteral("yyyy-mm-dd")).format(2957004, DateSystem::Date1904).failed);
        QVERIFY(NumberFormat(QStringLiteral("yyyy-mm-dd")).format(-1, DateSystem::Date1904).failed);
        QVERIFY(NumberFormat(QStringLiteral("h:mm")).format(-0.5).failed);
        QVERIFY(!NumberFormat(QStringLiteral("h:mm")).format(-0.5, DateSystem::Date1904).failed);
        QCOMPARE(fmt(QStringLiteral("h:mm"), -0.5, DateSystem::Date1904), QStringLiteral("-12:00"));
        QCOMPARE(fmt(QStringLiteral("[h]:mm"), -1.5, DateSystem::Date1904), QStringLiteral("-36:00"));
        // A date with a section of its own for the negative shows the section's text.
        QCOMPARE(fmt(QStringLiteral("h:mm;\"minus \"h:mm"), -0.5, DateSystem::Date1904), QStringLiteral("minus 12:00"));
        // A time on a day after the rounding: 1904 rounds the same way.
        QCOMPARE(fmt(QStringLiteral("yyyy-mm-dd hh:mm:ss"), 43904.999999, DateSystem::Date1904), QStringLiteral("2024-03-16 00:00:00"));
    }

    // [$-LCID] names the language of month and day names; [$text-LCID] adds text, [$-F800] and [$-F400] are the system's long date and time.
    void localesAndCurrencies()
    {
        const struct {
            const char *code;
            const char *want;
        } cases[] = {
            {"[$-409]mmmm d, yyyy", "March 15, 2024"},
            {"[$-809]dddd d mmmm yyyy", "Friday 15 March 2024"},
            {"[$-C09]dddd", "Friday"},
            {"[$-1009]ddd mmm", "Fri Mar"},
            {"[$-407]dddd, d. mmmm yyyy", "Freitag, 15. März 2024"},
            {"[$-407]mmmm", "März"},
            {"[$-40C]dddd d mmmm yyyy", "vendredi 15 mars 2024"},
            {"[$-40A]dddd d mmmm yyyy", "viernes 15 marzo 2024"},
            {"[$-410]dddd d mmmm yyyy", "venerdì 15 marzo 2024"},
            {"[$-416]dddd, d \"de\" mmmm \"de\" yyyy", "sexta-feira, 15 de março de 2024"},
            {"[$-416]ddd mmm", "sex mar"},
            {"[$-413]dddd d mmmm yyyy", "vrijdag 15 maart 2024"},
            {"[$-FFFF]mmmm", "March"},
            {"[$-411]mmmm", "March"},
            {"[$-F800]", "Friday, March 15, 2024"},
            {"[$-F400]", "6:00:00 PM"},
            {"[$€-407]#,##0.00", "€1,234.50"},
            {"#,##0.00\\ [$€-407]", "1,234.50 €"},
            {"[$USD] #,##0", "USD 1,235"},
            {"[$$-409]#,##0.00", "$1,234.50"},
            {"[$-409]#,##0.00", "1,234.50"},
            {"[$£-809]#,##0.00", "£1,234.50"},
            {"[$CHF-100C] 0.0", "CHF 1234.5"},
        };
        for (const auto &c : cases) {
            // The months and days are of 2024-03-15; the numbers are 1,234.5.
            const QString code = QString::fromUtf8(c.code);
            const bool date = code.contains(QLatin1Char('d')) || code.contains(QLatin1Char('m')) || code.contains(QLatin1String("F800")) || code.contains(QLatin1String("F400"));
            const double v = code.contains(QLatin1String("F400")) ? 0.75 : date ? 45366 : 1234.5;
            const QString got = fmt(code, v);
            QVERIFY2(got == QString::fromUtf8(c.want), qPrintable(QStringLiteral("code [%1]: got [%2], wanted [%3]").arg(code, got, QString::fromUtf8(c.want))));
        }
        // The upper half of a locale (a calendar, a number system) doesn't change the language.
        QCOMPARE(fmt(QStringLiteral("[$-1010407]mmmm"), 45366), QStringLiteral("März"));
        QCOMPARE(fmt(QStringLiteral("[$-407]ddd"), 45366), QStringLiteral("Fr"));
        // Separators never change with a locale.
        QCOMPARE(fmt(QStringLiteral("[$-407]#,##0.00"), 1234.5), QStringLiteral("1,234.50"));
    }

    // The system's long date and time ([$-F800], [$-F400], [$-x-sysdate], [$-x-systime]) with the pattern a file stores after them show it once: that pattern is what
    // the file says the style was when it was saved. With none after them the United States' pattern is shown. A locale may be a language tag too.
    void systemPatternsAndLanguageTags()
    {
        const double noon = 44927.5;   // Sunday, January 1, 2023, 12:00
        const struct {
            const char *code;
            const char *want;
        } cases[] = {
            {"[$-F800]dddd\\,\\ mmmm\\ dd\\,\\ yyyy", "Sunday, January 01, 2023"},
            {"[$-F400]h:mm:ss\\ AM/PM", "12:00:00 PM"},
            {"[$-x-sysdate]dddd\\,\\ mmmm\\ dd\\,\\ yyyy", "Sunday, January 01, 2023"},
            {"[$-x-systime]h:mm:ss\\ AM/PM", "12:00:00 PM"},
            {"[$-X-SYSDATE]dddd, mmmm d", "Sunday, January 1"},
            {"[$-F800]", "Sunday, January 1, 2023"},
            {"[$-x-sysdate]", "Sunday, January 1, 2023"},
            {"[$-F400]", "12:00:00 PM"},
            {"[$-x-systime]", "12:00:00 PM"},
            {"dddd[$-F800]", "Sunday"},
            {"[$-en-US]mmmm d", "January 1"},
            {"[$-en-GB]dddd", "Sunday"},
            {"[$-de-DE]mmmm d", "Januar 1"},
            {"[$-fr-FR]dddd", "dimanche"},
            {"[$-es-ES]mmmm", "enero"},
            {"[$-it-IT]dddd", "domenica"},
            {"[$-pt-BR]dddd", "domingo"},
            {"[$-nl-NL]mmmm", "januari"},
            {"[$-ja-JP]mmmm", "January"},
            {"[$-zh-Hans-CN]mmmm", "January"},
            {"[$-DE-de]mmmm", "Januar"},
            {"[$-en-US]0.0", "44927.5"},
            {"[$\xE2\x82\xAC-de-DE]#,##0.00", "\xE2\x82\xAC" "44,927.50"},
            {"[$\xE2\x82\xAC-407]#,##0.00", "\xE2\x82\xAC" "44,927.50"},
            {"[$USD-en-US] 0", "USD 44928"},
            {"[$US-Dollar-409] 0", "US-Dollar 44928"},
        };
        for (const auto &c : cases) {
            const QString code = QString::fromUtf8(c.code);
            const QString got = fmt(code, noon);
            QVERIFY2(got == QString::fromUtf8(c.want), qPrintable(QStringLiteral("code [%1]: got [%2], wanted [%3]").arg(code, got, QString::fromUtf8(c.want))));
        }
        // A tag that is neither a language nor a number stays text, as it always was.
        QCOMPARE(fmt(QStringLiteral("[$-xyz123abc]0"), 5), QStringLiteral("-xyz123abc5"));
    }

    // General: up to 15 significant digits with no trailing zeros, and scientific notation from 1E+15 and under 0.0001; in a cell's width fewer digits fit.
    void generalFormat()
    {
        const C cases[] = {
            {"General", 0, "0"},
            {"General", 1, "1"},
            {"General", -1.5, "-1.5"},
            {"General", 100, "100"},
            {"General", 0.1 + 0.2, "0.3"},
            {"General", 1.0 / 3, "0.333333333333333"},
            {"General", 2.0 / 3, "0.666666666666667"},
            {"General", 1234567.891, "1234567.891"},
            {"General", 123456789012345.0, "123456789012345"},
            {"General", 1e14, "100000000000000"},
            {"General", 1e15, "1E+15"},
            {"General", 1.5e15, "1.5E+15"},
            {"General", -1e15, "-1E+15"},
            {"General", 123456789012345678.0, "1.23456789012346E+17"},
            {"General", 1234567890123456.0, "1.23456789012346E+15"},
            {"General", 0.0001, "0.0001"},
            {"General", 0.000333333333333333, "0.000333333333333333"},
            {"General", 0.00001, "1E-05"},
            {"General", 1.5e-10, "1.5E-10"},
            {"General", 1e-7, "1E-07"},
            {"General", 1.0 / 30000, "3.33333333333333E-05"},
            {"General", 1e100, "1E+100"},
            {"general", 12.5, "12.5"},
            {"GENERAL", 12.5, "12.5"},
            {"", 12.5, "12.5"},
            {"General", 2.5, "2.5"},
            {"General", 99.99999999999999, "100"},
            {"General", 0.1 + 0.7, "0.8"},
        };
        CHECK_ALL(cases);
        // In a column that holds 11 characters (the default), the most digits that fit; scientific when none do.
        const struct {
            double v;
            int width;
            const char *want;
        } narrow[] = {
            {3.14159265358979, 11, "3.141592654"},
            {-3.14159265358979, 11, "-3.14159265"},
            {1.0 / 3, 11, "0.333333333"},
            {123456789012.0, 11, "1.23457E+11"},
            {12345678901.0, 11, "12345678901"},
            {1234567.891, 11, "1234567.891"},
            {12345678.9123, 11, "12345678.91"},
            {123456789.123, 11, "123456789.1"},
            {0.000012345, 11, "1.2345E-05"},
            {1234567.891, 7, "1234568"},
            {100, 11, "100"},
            {0, 11, "0"},
        };
        for (const auto &n : narrow) {
            const QString got = fmt(QStringLiteral("General"), n.v, DateSystem::Date1900, n.width);
            QVERIFY2(got == QString::fromLatin1(n.want), qPrintable(QStringLiteral("value %1 in %2: got [%3], wanted [%4]").arg(n.v, 0, 'g', 17).arg(n.width).arg(got, QString::fromLatin1(n.want))));
        }
        // Nothing fits in a column of one character.
        QVERIFY(NumberFormat().format(123456, DateSystem::Date1900, 1).failed);
        // Against Qt's own conversion to 15 significant digits (%.15G), over every magnitude; and the scientific code against %.2E.
        std::mt19937_64 rng(15);
        for (int i = 0; i < 20000; ++i) {
            const int e = i % 4 == 0 ? int(rng() % 600) - 300 : int(rng() % 60) - 30;
            const QString text = QStringLiteral("%1.%2e%3").arg(rng() % 9 + 1).arg(rng() % 100000000000000ULL, 14, 10, QLatin1Char('0')).arg(e);
            double v = text.toDouble();
            if (i % 3 == 0) v = -v;
            const QString want = QString::number(v, 'G', 15);
            QVERIFY2(fmt(QStringLiteral("General"), v) == want, qPrintable(QStringLiteral("%1: got [%2], wanted [%3]").arg(text, fmt(QStringLiteral("General"), v), want)));
            const QString sci = QString::number(v, 'E', 2);
            QVERIFY2(fmt(QStringLiteral("0.00E+00"), v) == sci, qPrintable(QStringLiteral("%1: got [%2], wanted [%3]").arg(text, fmt(QStringLiteral("0.00E+00"), v), sci)));
        }
        // A number that is not finite can't be shown.
        QVERIFY(NumberFormat().format(std::numeric_limits<double>::infinity()).failed);
        QVERIFY(NumberFormat().format(std::numeric_limits<double>::quiet_NaN()).failed);
    }

    // The built-in formats by their ids.
    void builtInFormats()
    {
        const struct {
            int id;
            double v;
            const char *want;
        } cases[] = {
            {0, 1234.5678, "1234.5678"},
            {1, 1234.5678, "1235"},
            {2, 1234.5678, "1234.57"},
            {3, 1234.5678, "1,235"},
            {4, 1234.5678, "1,234.57"},
            {5, 1234, "$1,234 "},
            {5, -1234, "($1,234)"},
            {6, -1234, "($1,234)"},
            {7, 1234.5, "$1,234.50 "},
            {7, -1234.5, "($1,234.50)"},
            {8, -1234.5, "($1,234.50)"},
            {9, 0.256, "26%"},
            {10, 0.256, "25.60%"},
            {11, 1234.5678, "1.23E+03"},
            {12, 1.5, "1 1/2"},
            {13, 1.25, "1  1/4 "},
            {14, kD, "3/15/2024"},
            {15, kD, "15-Mar-24"},
            {16, kD, "15-Mar"},
            {17, kD, "Mar-24"},
            {18, kD, "6:00 PM"},
            {19, kD, "6:00:00 PM"},
            {20, kD, "18:00"},
            {21, kD, "18:00:00"},
            {22, kD, "3/15/2024 18:00"},
            {37, 1234, "1,234 "},
            {37, -1234, "(1,234)"},
            {38, -1234, "(1,234)"},
            {39, -1234.5, "(1,234.50)"},
            {40, 1234.5, "1,234.50"},
            {41, 1234, " 1,234 "},
            {41, -1234, " (1,234)"},
            {41, 0, " - "},
            {42, 1234, " $1,234 "},
            {43, 0, " -   "},
            {43, 1234.5, " 1,234.50 "},
            {44, 1234.5, " $1,234.50 "},
            {44, -1234.5, " $(1,234.50)"},
            {45, 0.0007, "01:00"},
            {46, 1.5, "36:00:00"},
            {47, 0.0007, "0100.5"},
            {48, 1234.5678, "1.2E+3"},
            {49, 1.5, "1.5"},
        };
        for (const auto &c : cases) {
            const QString got = NumberFormat::builtin(c.id).format(c.v).text;
            QVERIFY2(got == QString::fromLatin1(c.want), qPrintable(QStringLiteral("id %1 value %2: got [%3], wanted [%4]").arg(c.id).arg(c.v, 0, 'g', 17).arg(got, QString::fromLatin1(c.want))));
        }
        QCOMPARE(NumberFormat::builtin(38).format(-1234).color, 3);
        QCOMPARE(NumberFormat::builtin(42).format(1234).fillAt, 2);
        QCOMPARE(NumberFormat::builtin(41).format(1234).fillAt, 1);
        QCOMPARE(NumberFormat::builtin(41).formatText(QStringLiteral("ab")).text, QStringLiteral(" ab "));
        QCOMPARE(NumberFormat::builtin(49).formatText(QStringLiteral("abc")).text, QStringLiteral("abc"));
        QCOMPARE(builtinFormatCode(0), QStringLiteral("General"));
        QCOMPARE(builtinFormatCode(14), QStringLiteral("m/d/yyyy"));
        QCOMPARE(builtinFormatCode(49), QStringLiteral("@"));
        QCOMPARE(builtinFormatCode(48), QStringLiteral("##0.0E+0"));
        QCOMPARE(builtinFormatCode(23), QString());
        QCOMPARE(builtinFormatCode(26), QString());
        QCOMPARE(builtinFormatCode(59), QString());
        QCOMPARE(builtinFormatCode(-1), QString());
        QVERIFY(!builtinFormatCode(30).isEmpty());
        QVERIFY(!builtinFormatCode(36).isEmpty());
        QVERIFY(!builtinFormatCode(50).isEmpty());
        QVERIFY(!builtinFormatCode(58).isEmpty());
        // An id with no format is General.
        QCOMPARE(NumberFormat::builtin(100).format(1.5).text, QStringLiteral("1.5"));
        QVERIFY(NumberFormat::builtin(100).isGeneral());
        for (int id = 0; id <= 49; ++id) {
            if (id >= 23 && id <= 26) continue;
            QVERIFY2(NumberFormat::builtin(id).isValid(), qPrintable(QString::number(id)));
            QCOMPARE(NumberFormat::builtin(id).code(), builtinFormatCode(id));
        }
    }

    // What kind of value a code is made for, and which codes are valid.
    void kindsAndValidity()
    {
        QCOMPARE(NumberFormat(QStringLiteral("General")).kind(), FormatKind::General);
        QCOMPARE(NumberFormat(QString()).kind(), FormatKind::General);
        QCOMPARE(NumberFormat(QStringLiteral("0.00")).kind(), FormatKind::Number);
        QCOMPARE(NumberFormat(QStringLiteral("yyyy-mm-dd")).kind(), FormatKind::Date);
        QCOMPARE(NumberFormat(QStringLiteral("[h]:mm")).kind(), FormatKind::Date);
        QCOMPARE(NumberFormat(QStringLiteral("@")).kind(), FormatKind::Text);
        QCOMPARE(NumberFormat(QStringLiteral("0.00;[Red]0.00")).kind(), FormatKind::Number);
        QVERIFY(NumberFormat(QStringLiteral("0.00")).isValid());
        QVERIFY(NumberFormat(QStringLiteral("General")).isGeneral());
        QVERIFY(NumberFormat(QStringLiteral("GENERAL")).isGeneral());
        QVERIFY(!NumberFormat(QStringLiteral("0.00")).isGeneral());
        QVERIFY(NumberFormat(QString()).isValid());
        // Not valid: a quote never closed, a bracket never closed, a fifth section, a backslash with nothing after it.
        for (const char *bad : {"0\"abc", "0[Red", "0;0;0;0;0", "0\\", "0_", "0*"}) {
            const NumberFormat f{QString::fromLatin1(bad)};
            QVERIFY2(!f.isValid(), bad);
            QVERIFY2(f.isGeneral(), bad);
            QCOMPARE(f.code(), QString::fromLatin1(bad));
            QCOMPARE(f.format(12.5).text, QStringLiteral("12.5"));
        }
        // Valid: 255 places, the longest.
        QVERIFY(NumberFormat(QString(255, QLatin1Char('0'))).isValid());
        QVERIFY(!NumberFormat(QString(256, QLatin1Char('0'))).isValid());
        // A copy is the same format.
        const NumberFormat a(QStringLiteral("0.00"));
        const NumberFormat b = a;
        QCOMPARE(b.format(2.5).text, QStringLiteral("2.50"));
    }

    // FIXED(value, decimals, no_commas) and DOLLAR(value, decimals).
    void fixedAndDollar()
    {
        const struct {
            double v;
            int decimals;
            bool noCommas;
            const char *want;
        } fixed[] = {
            {1234.567, 2, false, "1,234.57"},
            {1234.567, 2, true, "1234.57"},
            {1234.567, 0, false, "1,235"},
            {1234.567, -2, false, "1,200"},
            {-1234.567, 1, false, "-1,234.6"},
            {0.5, 0, false, "1"},
            {2.675, 2, false, "2.68"},
            {1234.5, 3, false, "1,234.500"},
            {1e15, 0, false, "1,000,000,000,000,000"},
            {123, -3, false, "0"},
            {999.9995, 3, false, "1,000.000"},
            {-0.5, 0, false, "-1"},
            {0, 2, false, "0.00"},
        };
        for (const auto &c : fixed) {
            QString got;
            QVERIFY(formatFixed(c.v, c.decimals, c.noCommas, &got));
            QVERIFY2(got == QString::fromLatin1(c.want), qPrintable(QStringLiteral("FIXED(%1, %2, %3): got [%4], wanted [%5]").arg(c.v).arg(c.decimals).arg(c.noCommas).arg(got, QString::fromLatin1(c.want))));
        }
        const struct {
            double v;
            int decimals;
            const char *want;
        } dollar[] = {
            {1234.567, 2, "$1,234.57"},
            {-1234.567, 2, "($1,234.57)"},
            {0, 2, "$0.00"},
            {1234.567, 0, "$1,235"},
            {1234.567, -2, "$1,200"},
            {-0.5, 0, "($1)"},
            {2.675, 2, "$2.68"},
        };
        for (const auto &c : dollar) {
            QString got;
            QVERIFY(formatDollar(c.v, c.decimals, &got));
            QVERIFY2(got == QString::fromLatin1(c.want), qPrintable(QStringLiteral("DOLLAR(%1, %2): got [%3], wanted [%4]").arg(c.v).arg(c.decimals).arg(got, QString::fromLatin1(c.want))));
        }
        QString out;
        QVERIFY(formatFixed(1.5, kMaxFixedDecimals, false, &out));
        QCOMPARE(out.size(), 2 + kMaxFixedDecimals);
        QVERIFY(!formatFixed(1.5, kMaxFixedDecimals + 1, false, &out));
        QVERIFY(!formatFixed(1.5, -kMaxFixedDecimals - 1, false, &out));
        QVERIFY(!formatFixed(std::numeric_limits<double>::quiet_NaN(), 2, false, &out));
        QVERIFY(!formatDollar(std::numeric_limits<double>::infinity(), 2, &out));
        QVERIFY(formatFixed(1e300, 0, false, &out));
        QCOMPARE(out.size(), 301 + 100);
    }

    // DATEVALUE of a date written out: numbers in the order of the United States, or a month's name; what TEXT writes reads back as the day it was.
    void dateTextReadsBack()
    {
        const struct {
            const char *text;
            double serial1900;
        } cases[] = {
            {"3/15/2024", 45366},   {"03/15/24", 45366},     {"3-15-2024", 45366},    {"3.15.2024", 45366}, {"3/15/99", 36234},
            {"3/15/30", 11032},     {"3/15/29", 47192},      {"15-Mar-2024", 45366},  {"15-Mar-24", 45366}, {"15 March 2024", 45366},
            {"Mar 15, 2024", 45366}, {"March 15, 2024", 45366}, {"mar 15 2024", 45366}, {"2/29/2000", 36585}, {"2/29/1900", 60},
            {"1/1/1900", 1},        {"12/31/9999", 2958465}, {" 3/15/2024 ", 45366},  {"1/1/2000", 36526},
        };
        for (const auto &c : cases) {
            double got = -1;
            QVERIFY2(dateValueOf(QString::fromLatin1(c.text), DateSystem::Date1900, &got), c.text);
            QVERIFY2(got == c.serial1900, qPrintable(QStringLiteral("%1: got %2, wanted %3").arg(QString::fromLatin1(c.text)).arg(got).arg(c.serial1900)));
        }
        double s = -1;
        QVERIFY(dateValueOf(QStringLiteral("3/15/2024"), DateSystem::Date1904, &s));
        QCOMPARE(s, 43904.0);
        QVERIFY(dateValueOf(QStringLiteral("1/1/1904"), DateSystem::Date1904, &s));
        QCOMPARE(s, 0.0);
        for (const char *bad : {"", "abc", "2/30/2024", "13/1/2024", "0/1/2024", "1/0/2024", "3/15", "3/15/2024/1", "3/15-2024", "1/1/1899", "12/31/10000", "Mar 15", "Foo 15, 2024",
                                "15-Foo-2024", "3/15/20x4", "3//2024", "3/15/", "-3/15/2024", "3/15/2024 abc"})
            QVERIFY2(!dateValueOf(QString::fromLatin1(bad), DateSystem::Date1900, &s), bad);
        // What a format writes reads back as the same day.
        for (const char *code : {"mm/dd/yy", "m/d/yyyy", "dd-mmm-yy", "d-mmm-yyyy", "mmmm d, yyyy", "mmm d, yyyy", "mm-dd-yyyy"}) {
            for (double day : {36585.0, 45366.0, 44927.0, 25569.0}) {
                double back = -1;
                const QString text = fmt(QString::fromLatin1(code), day);
                QVERIFY2(dateValueOf(text, DateSystem::Date1900, &back), qPrintable(QStringLiteral("%1 -> %2").arg(code, text)));
                QVERIFY2(back == day, qPrintable(QStringLiteral("%1 -> %2 -> %3").arg(code, text).arg(back)));
            }
        }
    }

    // DATEVALUE also reads the year first (2024-03-15, 2024/03/15) and a time after the date, which it ignores.
    void dateValueOfReadsIsoDatesAndTimes()
    {
        const struct {
            const char *text;
            double serial;
        } cases[] = {
            {"2024-03-15", 45366},
            {"2024/03/15", 45366},
            {"2024-3-5", 45356},
            {"2024/3/5", 45356},
            {"3/15/2024 10:30", 45366},
            {"3/15/2024  10:30:15", 45366},
            {"3/15/2024 10:30 PM", 45366},
            {"3/15/2024 12:00 AM", 45366},
            {"3/15/2024 12:59:59 pm", 45366},
            {"2024-03-15 10:30:00", 45366},
            {"2024-03-15 23:59:59.5", 45366},
            {"Mar 15, 2024 10:30", 45366},
            {"15-Mar-2024 8:05 am", 45366},
            {"3/15/24 0:00", 45366},
            {"1900-01-01", 1},
            {"9999-12-31", 2958465},
        };
        for (const auto &c : cases) {
            double got = -1;
            QVERIFY2(dateValueOf(QString::fromLatin1(c.text), DateSystem::Date1900, &got), c.text);
            QVERIFY2(got == c.serial, qPrintable(QStringLiteral("%1: got %2, wanted %3").arg(QString::fromLatin1(c.text)).arg(got).arg(c.serial)));
        }
        double s = -1;
        QVERIFY(dateValueOf(QStringLiteral("2024-03-15"), DateSystem::Date1904, &s));
        QCOMPARE(s, 43904.0);
        for (const char *bad : {"2024-02-30", "2024-13-01", "2024-00-10", "2024-03", "2024-03-15-01", "2024/03-15", "2024-03-32", "1899-12-31", "10000-01-01", "202-01-01", "24-03-15",
                                "3/15/2024 25:00", "3/15/2024 10:70", "3/15/2024 10:30 XM", "3/15/2024 13:00 PM", "3/15/2024 0:30 PM", "2024-03-15 abc", "2024-03-15 10",
                                "3/15/2024 10:30:61", "3/15/2024 :30", "3/15/2024 10:", "3/15/2024 10:30 PM x"})
            QVERIFY2(!dateValueOf(QString::fromLatin1(bad), DateSystem::Date1900, &s), bad);
    }

    // TEXT(value, code): the same text, with no fill, and #VALUE! (false) for what can't be shown.
    void textFunction()
    {
        const struct {
            double v;
            const char *code;
            const char *want;
        } cases[] = {
            {1234.5, "#,##0.00", "1,234.50"},
            {0.5, "0%", "50%"},
            {1234, "_(* #,##0_)", " 1,234 "},
            {5, "*-0", "5"},
            {5, "", ""},
            {kD, "yyyy-mm-dd hh:mm", "2024-03-15 18:00"},
            {-5, "0.0;(0.0)", "(5.0)"},
            {1234.5, "General", "1234.5"},
            {1234.5, "[Red]0.0", "1234.5"},
            {2.675, "0.00", "2.68"},
            {0, "0;-0;\"zero\"", "zero"},
        };
        for (const auto &c : cases) {
            QString got = QStringLiteral("(unset)");
            QVERIFY2(formatTextFunction(c.v, QString::fromLatin1(c.code), DateSystem::Date1900, &got), c.code);
            QVERIFY2(got == QString::fromLatin1(c.want), qPrintable(QStringLiteral("TEXT(%1, \"%2\"): got [%3], wanted [%4]").arg(c.v).arg(QString::fromLatin1(c.code), got, QString::fromLatin1(c.want))));
        }
        QString out = QStringLiteral("(unset)");
        QVERIFY(!formatTextFunction(-1, QStringLiteral("yyyy"), DateSystem::Date1900, &out));
        QVERIFY(!formatTextFunction(1, QStringLiteral("0\"abc"), DateSystem::Date1900, &out));
        QVERIFY(!formatTextFunction(std::numeric_limits<double>::quiet_NaN(), QStringLiteral("0"), DateSystem::Date1900, &out));
        QVERIFY(formatTextFunction(43904, QStringLiteral("yyyy-mm-dd"), DateSystem::Date1904, &out));
        QCOMPARE(out, QStringLiteral("2024-03-15"));
    }

    // Codes made to be slow or large: bounded in length, in sections, in digits, in brackets, and in the text they make.
    void hostileCodesAreBounded()
    {
        QElapsedTimer timer;
        timer.start();
        // Too long: not valid, and the value shows as General.
        {
            const NumberFormat f(QString(1000000, QLatin1Char('0')));
            QVERIFY(!f.isValid());
            QCOMPARE(f.format(5).text, QStringLiteral("5"));
        }
        {
            const NumberFormat f(QString(kMaxFormatCodeChars + 1, QLatin1Char('x')));
            QVERIFY(!f.isValid());
            QCOMPARE(f.format(5).text, QStringLiteral("5"));
        }
        // Thousands of sections.
        {
            const NumberFormat f(QString(100000, QLatin1Char(';')));
            QVERIFY(!f.isValid());
            QCOMPARE(f.format(5).text, QStringLiteral("5"));
            const NumberFormat g(QStringLiteral("0;") .repeated(2000));
            QVERIFY(!g.isValid());
            QCOMPARE(g.format(-5).text, QStringLiteral("-5"));
        }
        // Brackets without end, and so many of them.
        {
            const NumberFormat f(QStringLiteral("[Red]").repeated(1000));
            QVERIFY(!f.isValid());
            QCOMPARE(f.format(5).text, QStringLiteral("5"));
            const NumberFormat g(QStringLiteral("[$-409]").repeated(kMaxFormatBrackets + 1) + QStringLiteral("0"));
            QVERIFY(!g.isValid());
            const NumberFormat h(QStringLiteral("[") + QString(100000, QLatin1Char('a')));
            QVERIFY(!h.isValid());
            const NumberFormat k(QStringLiteral("[$-409]").repeated(kMaxFormatBrackets) + QStringLiteral("0"));
            QVERIFY(k.isValid());
            QCOMPARE(k.format(5).text, QStringLiteral("5"));
        }
        // More digit places than the limit; the limit itself shows in full.
        {
            const NumberFormat f(QString(kMaxFormatDigits + 1, QLatin1Char('#')));
            QVERIFY(!f.isValid());
            const NumberFormat g(QStringLiteral("0.") + QString(kMaxFormatDigits - 1, QLatin1Char('0')));
            QVERIFY(g.isValid());
            QCOMPARE(g.format(1.5).text.size(), 2 + kMaxFormatDigits - 1);
            const NumberFormat h(QStringLiteral("0.0E+0").repeated(600));
            QVERIFY(!h.isValid());
            const NumberFormat k(QStringLiteral("# ") + QString(20, QLatin1Char('?')) + QStringLiteral("/") + QString(20, QLatin1Char('?')));
            QVERIFY(!k.isValid());
            const NumberFormat m(QStringLiteral("# ?/1234567890123"));
            QVERIFY(!m.isValid());
            QCOMPARE(m.format(0.5).text, QStringLiteral("0.5"));
        }
        // A text repeated by the code does not make a text of more than a cell holds.
        {
            const NumberFormat f(QStringLiteral("@").repeated(4000));
            const Formatted r = f.formatText(QString(30000, QLatin1Char('a')));
            QVERIFY(r.failed);
            QVERIFY(r.text.size() <= kMaxFormatOutputChars);
            const Formatted ok = f.formatText(QStringLiteral("ab"));
            QCOMPARE(ok.text.size(), 8000);
            const NumberFormat g(QStringLiteral("\"") + QString(4000, QLatin1Char('x')) + QStringLiteral("\"0"));
            QVERIFY(g.isValid());
            QCOMPARE(g.format(7).text, QString(4000, QLatin1Char('x')) + QStringLiteral("7"));
        }
        // Runs of letters mean the longest form: no output grows with the run.
        {
            QCOMPARE(fmt(QString(4000, QLatin1Char('d')), 45366), QStringLiteral("Friday"));
            QCOMPARE(fmt(QString(4000, QLatin1Char('y')), 45366), QStringLiteral("2024"));
            QCOMPARE(fmt(QString(4000, QLatin1Char('m')), 45366), QStringLiteral("M"));
            QCOMPARE(fmt(QStringLiteral("mm:") + QString(3990, QLatin1Char('s')), 45366.75), QStringLiteral("00:00"));
            QCOMPARE(fmt(QString(4000, QLatin1Char('h')), 45366.75), QStringLiteral("18"));
        }
        // Percent signs and commas shift the value by thousands of places; extremes of value in every kind of code.
        {
            const QString pct = QString(4000, QLatin1Char('%')) + QStringLiteral("0");
            const Formatted f = NumberFormat(pct).format(1e300);
            QVERIFY(f.failed || f.text.size() <= kMaxFormatOutputChars);
            const QString com = QStringLiteral("0") + QString(4000, QLatin1Char(','));
            QCOMPARE(fmt(com, 1e300), QStringLiteral("0"));
            const double extremes[] = {1e308, -1e308, 5e-324, std::numeric_limits<double>::max(), std::numeric_limits<double>::min(), 1e15, 0.5, -0.5, 0, 45366.75, 2958465.9999999};
            const char *codes[] = {"0", "0.00000", "#,##0.00", "0.00E+00", "# ?" "?/??", "yyyy-mm-dd hh:mm:ss.000", "[h]:mm:ss", "General", "0%", "mmmm dddd", "[s].000", "[mm]"};
            for (const char *c : codes)
                for (double v : extremes) {
                    const NumberFormat f2{QString::fromLatin1(c)};
                    QVERIFY2(f2.format(v).text.size() <= kMaxFormatOutputChars, c);
                    QVERIFY2(f2.format(v, DateSystem::Date1904).text.size() <= kMaxFormatOutputChars, c);
                }
            QVERIFY(NumberFormat(QStringLiteral("[h]:mm")).format(1e300).failed);
            QVERIFY(NumberFormat(QStringLiteral("yyyy")).format(1e300).failed);
            QCOMPARE(fmt(QStringLiteral("0"), 1e300).size(), 301);
        }
        QVERIFY2(timer.elapsed() < 30000, qPrintable(QString::number(timer.elapsed())));
    }

    // Random strings of the characters a code is made of: none crashes or loops, none makes a text beyond the limit, and one that is not valid is General.
    void randomCodesNeverCrash()
    {
        // The anchor: this engine works at all.
        QCOMPARE(fmt(QStringLiteral("0.00"), 1.5), QStringLiteral("1.50"));
        QElapsedTimer timer;
        timer.start();
        std::mt19937 rng(20261022);
        const QString pieces[] = {QStringLiteral("0"),    QStringLiteral("#"),       QStringLiteral("?"),        QStringLiteral("."),      QStringLiteral(","),
                                  QStringLiteral("%"),    QStringLiteral(";"),       QStringLiteral("[Red]"),    QStringLiteral("[>5]"),   QStringLiteral("[<=-1]"),
                                  QStringLiteral("[$-407]"), QStringLiteral("[$€-409]"), QStringLiteral("[h]"),   QStringLiteral("[mm]"),   QStringLiteral("[ss]"),
                                  QStringLiteral("["),    QStringLiteral("]"),       QStringLiteral("\""),       QStringLiteral("\\"),     QStringLiteral("_"),
                                  QStringLiteral("*"),    QStringLiteral("@"),       QStringLiteral(" "),        QStringLiteral("E+"),     QStringLiteral("E-"),
                                  QStringLiteral("e"),    QStringLiteral("/"),       QStringLiteral(":"),        QStringLiteral("d"),      QStringLiteral("dd"),
                                  QStringLiteral("ddd"),  QStringLiteral("dddd"),    QStringLiteral("m"),        QStringLiteral("mm"),     QStringLiteral("mmm"),
                                  QStringLiteral("mmmm"), QStringLiteral("mmmmm"),   QStringLiteral("yy"),       QStringLiteral("yyyy"),   QStringLiteral("h"),
                                  QStringLiteral("hh"),   QStringLiteral("s"),       QStringLiteral("ss"),       QStringLiteral("ss.00"),  QStringLiteral("AM/PM"),
                                  QStringLiteral("A/P"),  QStringLiteral("General"), QStringLiteral("-"),        QStringLiteral("("),      QStringLiteral(")"),
                                  QStringLiteral("$"),    QStringLiteral("x"),       QStringLiteral("é"),        QStringLiteral("1"),      QStringLiteral("8"),
                                  QStringLiteral("a"),    QStringLiteral("g"),       QStringLiteral("b"),        QString(QChar(0)),        QString(QChar(0xD800))};
        const int count = int(sizeof(pieces) / sizeof(pieces[0]));
        const double values[] = {0, 1, -1, 0.5, -0.5, 1234.5678, 45366.75, 1e15, 1e-10, 1e300, -1e300, 2958465, 0.9999999, 2.5, 60, 0.0007};
        // (JP_FUZZ=n runs n times as many.)
        const int rounds = 6000 * std::max(1, qEnvironmentVariableIntValue("JP_FUZZ"));
        for (int round = 0; round < rounds; ++round) {
            QString code;
            const int pieceCount = int(rng() % 14);
            for (int i = 0; i < pieceCount; ++i) code += pieces[rng() % count];
            const NumberFormat f(code);
            for (double v : values) {
                for (int width : {0, 11}) {
                    const Formatted a = f.format(v, DateSystem::Date1900, width);
                    QVERIFY2(a.text.size() <= kMaxFormatOutputChars, qPrintable(code));
                    QVERIFY2(!a.failed || a.text.isEmpty(), qPrintable(code));
                    QVERIFY2(a.fillAt >= -1 && a.fillAt <= a.text.size(), qPrintable(code));
                    for (const FormatPad &p : a.pads) QVERIFY2(p.at >= 0 && p.at < a.text.size(), qPrintable(code));
                    QVERIFY2(a.color >= 0 && a.color <= 56, qPrintable(code));
                }
                const Formatted b = f.format(v, DateSystem::Date1904);
                QVERIFY2(b.text.size() <= kMaxFormatOutputChars, qPrintable(code));
            }
            const Formatted t = f.formatText(QStringLiteral("hello"));
            QVERIFY2(t.text.size() <= kMaxFormatOutputChars, qPrintable(code));
            // A code that is not valid shows what General shows.
            if (!f.isValid()) QCOMPARE(f.format(1234.5).text, QStringLiteral("1234.5"));
        }
        QVERIFY2(timer.elapsed() < 60000 * std::max(1, qEnvironmentVariableIntValue("JP_FUZZ")), qPrintable(QString::number(timer.elapsed())));
    }
};

JP_TEST_CLASS(SheetNumberFormatTests)
#include "tst_sheet_numfmt.moc"
