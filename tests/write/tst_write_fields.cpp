// Tests of JeffWrite's fields (apps/write/core/fieldcode.*, fieldops.*): the code of a field read into its switches, the formats of a result, the formulas and conditions, the results
// of the fields the program works out, and updating, unlinking, and locking the fields of a document. Each document is built here from a few lines of XML.

#include "core/fieldcode.h"
#include "core/fieldops.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;
using namespace jp::write::fields;

namespace {

QDateTime when(int y, int mo, int d, int h = 0, int mi = 0, int s = 0) { return QDateTime(QDate(y, mo, d), QTime(h, mi, s)); }

} // namespace

class FieldTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // ---- the code of a field
    void aCodeIsReadIntoItsNameArgumentsAndSwitches()
    {
        const Code toc = parseCode(QStringLiteral(" TOC \\o \"1-3\" \\h \\z \\u "));
        QCOMPARE(toc.name, QStringLiteral("TOC"));
        QCOMPARE(toc.argumentOf(QLatin1Char('o')), QStringLiteral("1-3"));
        QVERIFY(toc.has(QLatin1Char('h')) && toc.has(QLatin1Char('z')) && toc.has(QLatin1Char('u')));
        QVERIFY(!toc.has(QLatin1Char('n')));
        const Code ref = parseCode(QStringLiteral("PAGEREF _Toc123 \\h"));
        QCOMPARE(ref.name, QStringLiteral("PAGEREF"));
        QCOMPARE(ref.arguments, QStringList{QStringLiteral("_Toc123")});
        QVERIFY(ref.has(QLatin1Char('H')));   // (the letter is compared without regard to case)
        const Code date = parseCode(QStringLiteral("date \\@ \"dddd, MMMM d, yyyy\" \\* MERGEFORMAT"));
        QCOMPARE(date.name, QStringLiteral("DATE"));
        QVERIFY(date.hasDatePicture);
        QCOMPARE(date.datePicture, QStringLiteral("dddd, MMMM d, yyyy"));
        QVERIFY(date.keepsFormat());
        const Code seq = parseCode(QStringLiteral("SEQ Figure \\* ARABIC \\r 3"));
        QCOMPARE(seq.arguments, QStringList{QStringLiteral("Figure")});
        QCOMPARE(seq.general, QStringList{QStringLiteral("ARABIC")});
        QCOMPARE(seq.argumentOf(QLatin1Char('r')), QStringLiteral("3"));
        const Code num = parseCode(QStringLiteral("= 3 \\# \"0.00\""));
        QCOMPARE(num.name, QStringLiteral("="));
        QCOMPARE(num.numberPicture, QStringLiteral("0.00"));
        // Text that is no code reads as nothing, and a huge one is cut.
        QVERIFY(parseCode(QString()).name.isEmpty());
        QVERIFY(parseCode(QStringLiteral("   ")).name.isEmpty());
        const Code huge = parseCode(QStringLiteral("REF ") + QString(500000, QLatin1Char('a')));
        QVERIFY(huge.arguments.value(0).size() <= kMostCodeChars);
        const Code many = parseCode(QStringLiteral("X ") + QStringLiteral("\\a ").repeated(100000));
        QVERIFY(many.switches.size() <= kMostCodeWords);
    }

    // ---- formats
    void numbersTakeTheFormatsOfTheGeneralSwitch()
    {
        QCOMPARE(formatNumber(1994, {QStringLiteral("ROMAN")}), QStringLiteral("MCMXCIV"));
        QCOMPARE(formatNumber(4, {QStringLiteral("roman")}), QStringLiteral("iv"));
        QCOMPARE(formatNumber(3, {QStringLiteral("alphabetic")}), QStringLiteral("c"));
        QCOMPARE(formatNumber(27, {QStringLiteral("ALPHABETIC")}), QStringLiteral("AA"));
        QCOMPARE(formatNumber(53, {QStringLiteral("alphabetic")}), QStringLiteral("aaa"));
        QCOMPARE(formatNumber(21, {QStringLiteral("Ordinal")}), QStringLiteral("21st"));
        QCOMPARE(formatNumber(12, {QStringLiteral("Ordinal")}), QStringLiteral("12th"));
        QCOMPARE(formatNumber(22, {QStringLiteral("Ordinal")}), QStringLiteral("22nd"));
        QCOMPARE(formatNumber(103, {QStringLiteral("Ordinal")}), QStringLiteral("103rd"));
        QCOMPARE(formatNumber(21, {QStringLiteral("CardText")}), QStringLiteral("twenty-one"));
        QCOMPARE(formatNumber(1205, {QStringLiteral("CardText")}), QStringLiteral("one thousand two hundred five"));
        QCOMPARE(formatNumber(0, {QStringLiteral("CardText")}), QStringLiteral("zero"));
        QCOMPARE(formatNumber(3, {QStringLiteral("OrdText")}), QStringLiteral("third"));
        QCOMPARE(formatNumber(40, {QStringLiteral("OrdText")}), QStringLiteral("fortieth"));
        QCOMPARE(formatNumber(255, {QStringLiteral("Hex")}), QStringLiteral("FF"));
        QCOMPARE(formatNumber(7, {QStringLiteral("arabic")}), QStringLiteral("7"));
        QCOMPARE(formatNumber(7, {QStringLiteral("ArabicDash")}), QStringLiteral("- 7 -"));
        QCOMPARE(formatNumber(21, {QStringLiteral("CardText"), QStringLiteral("Upper")}), QStringLiteral("TWENTY-ONE"));
        QCOMPARE(formatNumber(21, {QStringLiteral("CardText"), QStringLiteral("FirstCap")}), QStringLiteral("Twenty-one"));
        QCOMPARE(formatNumber(21, {QStringLiteral("CardText"), QStringLiteral("Caps")}), QStringLiteral("Twenty-One"));
        QCOMPARE(formatNumber(-5, {QStringLiteral("roman")}), QStringLiteral("-5"));   // (no numeral for it: the digits)
        QCOMPARE(formatNumber(1000000000000LL, {QStringLiteral("CardText")}).isEmpty(), false);
        QCOMPARE(formatNumber(5000, {QStringLiteral("roman")}), QStringLiteral("5000"));
        QCOMPARE(formatText(QStringLiteral("hello wORLD"), {QStringLiteral("Caps")}), QStringLiteral("Hello World"));
        QCOMPARE(formatText(QStringLiteral("hello wORLD"), {QStringLiteral("FirstCap")}), QStringLiteral("Hello world"));
        QCOMPARE(formatText(QStringLiteral("hello"), {QStringLiteral("MERGEFORMAT"), QStringLiteral("Upper")}), QStringLiteral("HELLO"));
        QCOMPARE(formatText(QStringLiteral("Hello"), {QStringLiteral("Lower")}), QStringLiteral("hello"));
    }

    void numberPicturesShowDigitsSignsAndGroups()
    {
        QCOMPARE(numberInPicture(3.14159, QStringLiteral("0.00")), QStringLiteral("3.14"));
        QCOMPARE(numberInPicture(5, QStringLiteral("000")), QStringLiteral("005"));
        QCOMPARE(numberInPicture(1234567.891, QStringLiteral("#,##0.00")), QStringLiteral("1,234,567.89"));
        QCOMPARE(numberInPicture(0.5, QStringLiteral("0%")), QStringLiteral("1%"));   // (0.5 rounds away from zero: no scaling by the picture)
        QCOMPARE(numberInPicture(1234, QStringLiteral("$#,##0.00")), QStringLiteral("$1,234.00"));
        QCOMPARE(numberInPicture(-5, QStringLiteral("0")), QStringLiteral("-5"));
        QCOMPARE(numberInPicture(5, QStringLiteral("-0")), QStringLiteral(" 5"));
        QCOMPARE(numberInPicture(-5, QStringLiteral("-0")), QStringLiteral("-5"));
        QCOMPARE(numberInPicture(5, QStringLiteral("+0")), QStringLiteral("+5"));
        QCOMPARE(numberInPicture(5, QStringLiteral("0;(0)")), QStringLiteral("5"));
        QCOMPARE(numberInPicture(-5, QStringLiteral("0;(0)")), QStringLiteral("(5)"));
        QCOMPARE(numberInPicture(0, QStringLiteral("0;(0);zero")), QStringLiteral("zero"));
        QCOMPARE(numberInPicture(12, QStringLiteral("'No. '0")), QStringLiteral("No. 12"));
        QCOMPARE(numberInPicture(222492, QStringLiteral("x##")), QStringLiteral("492"));   // (x drops the digits to its left)
        QCOMPARE(numberInPicture(2.5, QStringLiteral("0")), QStringLiteral("3"));
        QCOMPARE(numberInPicture(7, QStringLiteral("##0")), QStringLiteral("  7"));
        // A hostile picture or number is bounded.
        QVERIFY(numberInPicture(1e300, QStringLiteral("0")).size() < 400);
        QVERIFY(numberInPicture(1, QString(100000, QLatin1Char('0'))).size() <= 2 * kMostPictureChars);
        QVERIFY(numberInPicture(std::nan(""), QStringLiteral("0")).size() < 10);
    }

    void datePicturesShowTheParts()
    {
        const QDateTime t = when(2026, 3, 7, 15, 4, 9);   // a Saturday
        QCOMPARE(dateInPicture(t, QStringLiteral("dddd, MMMM d, yyyy")), QStringLiteral("Saturday, March 7, 2026"));
        QCOMPARE(dateInPicture(t, QStringLiteral("M/d/yy")), QStringLiteral("3/7/26"));
        QCOMPARE(dateInPicture(t, QStringLiteral("MM/dd/yyyy")), QStringLiteral("03/07/2026"));
        QCOMPARE(dateInPicture(t, QStringLiteral("ddd MMM d")), QStringLiteral("Sat Mar 7"));
        QCOMPARE(dateInPicture(t, QStringLiteral("h:mm AM/PM")), QStringLiteral("3:04 PM"));
        QCOMPARE(dateInPicture(t, QStringLiteral("h:mm am/pm")), QStringLiteral("3:04 pm"));
        QCOMPARE(dateInPicture(t, QStringLiteral("HH:mm:ss")), QStringLiteral("15:04:09"));
        QCOMPARE(dateInPicture(when(2026, 3, 7, 0, 5), QStringLiteral("h:mm A/P")), QStringLiteral("12:05 A"));
        QCOMPARE(dateInPicture(t, QStringLiteral("'Day' d 'of' MMMM")), QStringLiteral("Day 7 of March"));
        QCOMPARE(dateInPicture(t, QStringLiteral("yyyy\\-MM")), QStringLiteral("2026-03"));
        QCOMPARE(dateInPicture(t, QStringLiteral("m")), QStringLiteral("4"));   // (m is minutes, M the month)
        QVERIFY(dateInPicture(t, QString(100000, QLatin1Char('d'))).size() < 2000);
    }

    // ---- formulas
    void formulasAreWorkedOut()
    {
        auto value = [](const char *text) { return evaluateFormula(QString::fromLatin1(text)); };
        QCOMPARE(*value("2 + 3 * 4"), 14.0);
        QCOMPARE(*value("(2 + 3) * 4"), 20.0);
        QCOMPARE(*value("2 ^ 3 ^ 2"), 512.0);
        QCOMPARE(*value("-3 + 5"), 2.0);
        QCOMPARE(*value("50%"), 0.5);
        QCOMPARE(*value("10 / 4"), 2.5);
        QCOMPARE(*value("SUM(1,2,3)"), 6.0);
        QCOMPARE(*value("AVERAGE(2;4;6)"), 4.0);
        QCOMPARE(*value("MAX(1,9,3) - MIN(4,2)"), 7.0);
        QCOMPARE(*value("ABS(-4)"), 4.0);
        QCOMPARE(*value("MOD(10,3)"), 1.0);
        QCOMPARE(*value("INT(-2.5)"), -3.0);
        QCOMPARE(*value("ROUND(2.567, 1)"), 2.6);
        QCOMPARE(*value("IF(3 > 2, 10, 20)"), 10.0);
        QCOMPARE(*value("AND(1, 0)"), 0.0);
        QCOMPARE(*value("PRODUCT(2,3,4)"), 24.0);
        QCOMPARE(*value("COUNT(1,2,3,4)"), 4.0);
        QCOMPARE(*value("SIGN(-9)"), -1.0);
        QCOMPARE(*value("2 = 2"), 1.0);
        QCOMPARE(*value("2 <> 2"), 0.0);
        QVERIFY(!value("1 / 0").has_value());
        QVERIFY(!value("2 +").has_value());
        QVERIFY(!value("").has_value());
        QVERIFY(!value("FOO(2)").has_value());
        // A name is asked of the caller.
        const auto named = evaluateFormula(QStringLiteral("Price * 2"), [](const QString &n) -> std::optional<double> { return n == QLatin1String("Price") ? std::optional<double>(21) : std::nullopt; });
        QVERIFY(named);
        QCOMPARE(*named, 42.0);
        // Hostile: deep nesting and a long text come out with nothing, not a crash.
        QVERIFY(!evaluateFormula(QString(100000, QLatin1Char('(')) + QStringLiteral("1") + QString(100000, QLatin1Char(')'))).has_value());
        QVERIFY(!evaluateFormula(QStringLiteral("1+").repeated(200000) + QStringLiteral("1")).has_value());
        QVERIFY(!evaluateFormula(QStringLiteral("-").repeated(100000) + QStringLiteral("1")).has_value());
        QVERIFY(!evaluateFormula(QStringLiteral("ABS(").repeated(100000) + QStringLiteral("1") + QString(100000, QLatin1Char(')'))).has_value());
    }

    // ---- results
    void thePageAndCountFieldsShowTheNumbersTheyAreGiven()
    {
        Env env;
        env.page = 4;
        env.pageText = QStringLiteral("iv");
        env.pages = 12;
        env.sectionPages = 5;
        env.section = 2;
        QCOMPARE(*compute(parseCode(QStringLiteral("PAGE")), env), QStringLiteral("iv"));   // (the page number as the section shows it)
        QCOMPARE(*compute(parseCode(QStringLiteral("PAGE \\* ROMAN")), env), QStringLiteral("IV"));
        QCOMPARE(*compute(parseCode(QStringLiteral("PAGE \\* arabic")), env), QStringLiteral("4"));
        QCOMPARE(*compute(parseCode(QStringLiteral("NUMPAGES")), env), QStringLiteral("12"));
        QCOMPARE(*compute(parseCode(QStringLiteral("NUMPAGES \\* CardText")), env), QStringLiteral("twelve"));
        QCOMPARE(*compute(parseCode(QStringLiteral("NUMPAGES \\# \"000\"")), env), QStringLiteral("012"));
        QCOMPARE(*compute(parseCode(QStringLiteral("SECTIONPAGES")), env), QStringLiteral("5"));
        QCOMPARE(*compute(parseCode(QStringLiteral("SECTION")), env), QStringLiteral("2"));
        // No pages laid out: the field keeps its result.
        Env none;
        QVERIFY(!compute(parseCode(QStringLiteral("NUMPAGES")), none).has_value());
        QVERIFY(!compute(parseCode(QStringLiteral("PAGE")), none).has_value());
    }

    void referencesShowWhatTheBookmarkHolds()
    {
        Env env;
        env.position = 100;
        env.bookmark = [](const QString &name) -> std::optional<BookmarkInfo> {
            BookmarkInfo b;
            if (name == QLatin1String("Intro")) {
                b.text = QStringLiteral("Introduction");
                b.page = QStringLiteral("3");
                b.number = QStringLiteral("1.2");
                b.fullNumber = QStringLiteral("1.2");
                b.relativeNumber = QStringLiteral("2");
                b.position = 40;
                b.pageIndex = 3;
                return b;
            }
            if (name == QLatin1String("Later")) {
                b.text = QStringLiteral("Conclusion");
                b.page = QStringLiteral("9");
                b.position = 500;
                b.pageIndex = 9;
                return b;
            }
            return std::nullopt;
        };
        QCOMPARE(*compute(parseCode(QStringLiteral("REF Intro \\h")), env), QStringLiteral("Introduction"));
        QCOMPARE(*compute(parseCode(QStringLiteral("REF Intro \\* Upper")), env), QStringLiteral("INTRODUCTION"));
        QCOMPARE(*compute(parseCode(QStringLiteral("REF Intro \\n")), env), QStringLiteral("1.2"));
        QCOMPARE(*compute(parseCode(QStringLiteral("REF Intro \\r")), env), QStringLiteral("2"));
        QCOMPARE(*compute(parseCode(QStringLiteral("REF Intro \\w")), env), QStringLiteral("1.2"));
        QCOMPARE(*compute(parseCode(QStringLiteral("REF Intro \\p")), env), QStringLiteral("above"));
        QCOMPARE(*compute(parseCode(QStringLiteral("REF Later \\p")), env), QStringLiteral("below"));
        QCOMPARE(*compute(parseCode(QStringLiteral("PAGEREF Intro \\h")), env), QStringLiteral("3"));
        QCOMPARE(*compute(parseCode(QStringLiteral("PAGEREF Later \\* roman")), env), QStringLiteral("ix"));
        QCOMPARE(*compute(parseCode(QStringLiteral("PAGEREF \"Intro\"")), env), QStringLiteral("3"));
        QCOMPARE(*compute(parseCode(QStringLiteral("REF Missing \\h")), env), referenceSourceNotFound());
        QCOMPARE(*compute(parseCode(QStringLiteral("PAGEREF Missing")), env), referenceSourceNotFound());
        QVERIFY(referenceSourceNotFound().startsWith(QLatin1String("Error!")));
        // Without a way to look bookmarks up, the result is left as it is.
        Env bare;
        QVERIFY(!compute(parseCode(QStringLiteral("REF Intro")), bare).has_value());
    }

    void sequencesCountInOrderAndResetWhereTheySay()
    {
        QHash<QString, qint64> counters;
        Env env;
        env.sequences = &counters;
        auto next = [&](const char *code) { return compute(parseCode(QString::fromLatin1(code)), env).value_or(QStringLiteral("none")); };
        QCOMPARE(next("SEQ Figure \\* ARABIC"), QStringLiteral("1"));
        QCOMPARE(next("SEQ Figure \\* ARABIC"), QStringLiteral("2"));
        QCOMPARE(next("SEQ Table"), QStringLiteral("1"));
        QCOMPARE(next("SEQ Figure \\c"), QStringLiteral("2"));       // (\c repeats the last)
        QCOMPARE(next("SEQ Figure \\h"), QString());                  // (\h counts and shows nothing)
        QCOMPARE(next("SEQ Figure \\n"), QStringLiteral("4"));
        QCOMPARE(next("SEQ Figure \\r 10"), QStringLiteral("10"));
        QCOMPARE(next("SEQ Figure"), QStringLiteral("11"));
        QCOMPARE(next("SEQ figure"), QStringLiteral("12"));           // (names are compared without regard to case)
        QCOMPARE(next("SEQ Figure \\* ROMAN"), QStringLiteral("XIII"));
        QCOMPARE(next("SEQ Figure \\r 99999999999999"), QStringLiteral("2147483647"));   // (bounded)
        Env no;
        QVERIFY(!compute(parseCode(QStringLiteral("SEQ Figure")), no).has_value());
    }

    void datesTimesAndPropertiesFollowTheEnvironment()
    {
        Env env;
        env.now = when(2026, 11, 2, 9, 30);
        env.properties.author = QStringLiteral("Ann Lee");
        env.properties.title = QStringLiteral("My Report");
        env.properties.created = when(2024, 6, 5, 15, 12);
        env.properties.words = 1234;
        env.properties.statistics = true;
        env.properties.fileName = QStringLiteral("report.docx");
        env.properties.filePath = QStringLiteral("/home/ann/report.docx");
        env.properties.fileSize = 20480;
        QCOMPARE(*compute(parseCode(QStringLiteral("DATE \\@ \"MMMM d, yyyy\"")), env), QStringLiteral("November 2, 2026"));
        QCOMPARE(*compute(parseCode(QStringLiteral("DATE")), env), QStringLiteral("11/2/2026"));
        QCOMPARE(*compute(parseCode(QStringLiteral("TIME \\@ \"h:mm am/pm\"")), env), QStringLiteral("9:30 am"));
        QCOMPARE(*compute(parseCode(QStringLiteral("CREATEDATE \\@ \"yyyy-MM-dd\"")), env), QStringLiteral("2024-06-05"));
        QCOMPARE(*compute(parseCode(QStringLiteral("AUTHOR")), env), QStringLiteral("Ann Lee"));
        QCOMPARE(*compute(parseCode(QStringLiteral("AUTHOR \\* Upper")), env), QStringLiteral("ANN LEE"));
        QCOMPARE(*compute(parseCode(QStringLiteral("TITLE")), env), QStringLiteral("My Report"));
        QCOMPARE(*compute(parseCode(QStringLiteral("DOCPROPERTY Title")), env), QStringLiteral("My Report"));
        QCOMPARE(*compute(parseCode(QStringLiteral("DOCPROPERTY \"Author\"")), env), QStringLiteral("Ann Lee"));
        QCOMPARE(*compute(parseCode(QStringLiteral("NUMWORDS")), env), QStringLiteral("1234"));
        QCOMPARE(*compute(parseCode(QStringLiteral("NUMWORDS \\# \"#,##0\"")), env), QStringLiteral("1,234"));
        QCOMPARE(*compute(parseCode(QStringLiteral("FILENAME")), env), QStringLiteral("report.docx"));
        QCOMPARE(*compute(parseCode(QStringLiteral("FILENAME \\p")), env), QStringLiteral("/home/ann/report.docx"));
        QCOMPARE(*compute(parseCode(QStringLiteral("FILESIZE")), env), QStringLiteral("20480"));
        QCOMPARE(*compute(parseCode(QStringLiteral("FILESIZE \\k")), env), QStringLiteral("20"));
        QVERIFY(!compute(parseCode(QStringLiteral("DOCPROPERTY NoSuchThing")), env).has_value());
        QVERIFY(!compute(parseCode(QStringLiteral("SUBJECT")), env).has_value() || compute(parseCode(QStringLiteral("SUBJECT")), env)->isEmpty());
        QVERIFY(dependsOnTheMachine(QStringLiteral("DATE")) && dependsOnTheMachine(QStringLiteral("FILENAME")) && dependsOnTheMachine(QStringLiteral("AUTHOR")));
        QVERIFY(!dependsOnTheMachine(QStringLiteral("PAGEREF")));
    }

    void conditionsQuotesAndFormulasAreWorkedOut()
    {
        Env env;
        auto result = [&](const char *code) { return compute(parseCode(QString::fromLatin1(code)), env); };
        QCOMPARE(*result("IF 5 > 3 \"yes\" \"no\""), QStringLiteral("yes"));
        QCOMPARE(*result("IF 2 > 3 \"yes\" \"no\""), QStringLiteral("no"));
        QCOMPARE(*result("IF \"abc\" = \"ABC\" \"same\" \"different\""), QStringLiteral("same"));
        QCOMPARE(*result("IF \"abc\" = \"a*\" \"wild\" \"plain\""), QStringLiteral("wild"));
        QCOMPARE(*result("IF 10 <> 10 \"a\" \"b\""), QStringLiteral("b"));
        QCOMPARE(*result("IF 9 >= 10 \"a\" \"b\""), QStringLiteral("b"));   // (numbers compare as numbers: 9 is below 10)
        QCOMPARE(*result("QUOTE \"Hello there\""), QStringLiteral("Hello there"));
        QCOMPARE(*result("= 2 + 3 * 4"), QStringLiteral("14"));
        QCOMPARE(*result("= 10 / 4 \\# \"0.0\""), QStringLiteral("2.5"));
        QCOMPARE(*result("= 1 / 3"), QStringLiteral("0.333333"));
        QCOMPARE(*result("= SUM(1,2,3) \\# \"$#,##0.00\""), QStringLiteral("$6.00"));
        QCOMPARE(*result("= 5 \\* CardText"), QStringLiteral("five"));
        QVERIFY(!result("= 1 / 0").has_value());   // (an error leaves the result as it is)
        QVERIFY(!result("IF 1 >").has_value());
        QVERIFY(!result("NOSUCHFIELD x y").has_value());
        QVERIFY(!result("MERGEFIELD Name").has_value());
        QVERIFY(!result("HYPERLINK \"http://example.com\"").has_value());
        QVERIFY(isKnown(QStringLiteral("PAGEREF")) && !isKnown(QStringLiteral("MERGEFIELD")));
    }
};

QTEST_MAIN(FieldTests)
#include "tst_write_fields.moc"
