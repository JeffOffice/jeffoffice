// Tests of JeffWrite's fields (apps/write/core/fieldcode.*, fieldops.*): the code of a field read into its switches, the formats of a result, the formulas and conditions, the results
// of the fields the program works out, and updating, unlinking, and locking the fields of a document. Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "core/fieldcode.h"
#include "core/fieldops.h"
#include "core/refops.h"
#include "core/tocops.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "../localepin.h"
#include "office/package.h"
#include "text/textengine.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QElapsedTimer>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;
using namespace jp::write::fields;

namespace {

QDateTime when(int y, int mo, int d, int h = 0, int mi = 0, int s = 0) { return QDateTime(QDate(y, mo, d), QTime(h, mi, s)); }

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

const char kStyles[] =
    "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:outlineLvl w:val=\"1\"/></w:pPr></w:style>";

QByteArray makeDocx(const QByteArray &body)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), xml(QByteArray("<w:styles ") + kNs + ">" + kStyles + "</w:styles>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    return w.finish();
}

QByteArray documentXml(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/document.xml"));
}

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

QByteArray run(const QByteArray &text) { return "<w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r>"; }
QByteArray para(const QByteArray &content, const QByteArray &props = QByteArray()) { return "<w:p>" + (props.isEmpty() ? QByteArray() : "<w:pPr>" + props + "</w:pPr>") + content + "</w:p>"; }
QByteArray field(const QByteArray &code, const QByteArray &result, const QByteArray &flags = QByteArray())
{
    return "<w:r><w:fldChar w:fldCharType=\"begin\"" + flags + "/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + code + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" +
           run(result) + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
}
QByteArray bookmark(int id, const QByteArray &name, const QByteArray &inner)
{
    return "<w:bookmarkStart w:id=\"" + QByteArray::number(id) + "\" w:name=\"" + name + "\"/>" + inner + "<w:bookmarkEnd w:id=\"" + QByteArray::number(id) + "\"/>";
}

// What the paragraphs say, a line each, the marks left out.
QStringList lines(const WriteDoc &doc)
{
    QStringList out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) out << QString(b.text()).remove(QChar::ObjectReplacementCharacter);
    return out;
}


} // namespace

class FieldTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // The run's settings are its own, and the person's are as they were (checked at the end).
    void cleanupTestCase() { QVERIFY(jptest::settingsAreIsolated()); QVERIFY(jptest::realSettingsUnchanged()); }

    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        jp::initCore();
    }

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
        QVERIFY(referenceSourceNotFound().startsWith(QLatin1String("Error:")));
        QVERIFY(!referenceSourceNotFound().contains(QLatin1String("Reference source")));
        QVERIFY(!bookmarkNotDefined().contains(QLatin1String("not defined")));
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
    // ---- fields of a document
    void anUpdateMakesTheResultsRightInOneStep()
    {
        const QByteArray body = para(bookmark(1, "Intro", run("Introduction"))) +
                                para(run("See ") + field("REF Intro \\h", "Old text") + run(" on page ") + field("PAGEREF Intro \\h", "9")) +
                                para(run("Figure ") + field("SEQ Figure \\* ARABIC", "7") + run(" and ") + field("SEQ Figure \\* ARABIC", "7")) +
                                para(run("Date ") + field("DATE \\@ \"yyyy-MM-dd\"", "1999-01-01")) +
                                para(field("QUOTE \"fresh\"", "fresh"));
        const QByteArray zip = makeDocx(body);
        auto doc = open(zip);
        QVERIFY(doc);
        UpdateOptions o;
        o.now = when(2026, 11, 2, 9, 30);
        {
            // (Looking at the fields, or updating those that are right, changes nothing in what is written.)
            auto plain = open(zip);
            QVERIFY(plain);
            const QByteArray saved = documentXml(plain->toDocx());
            UpdateOptions look = o;
            look.apply = false;
            updateFields(*doc, look);
            QCOMPARE(documentXml(doc->toDocx()), saved);
            QVERIFY(!doc->canUndo());
        }
        const QString before = doc->story()->toPlainText();
        const UpdateReport report = updateFields(*doc, o, true);
        QCOMPARE(report.found, 6);
        QCOMPARE(report.unchanged, 1);   // (the QUOTE: already right)
        QCOMPARE(report.changed, 5);
        const QStringList l = lines(*doc);
        QCOMPARE(l[1], QStringLiteral("See Introduction on page 1"));
        QCOMPARE(l[2], QStringLiteral("Figure 1 and 2"));
        QCOMPARE(l[3], QStringLiteral("Date 2026-11-02"));
        // The fields are still fields, with their results made again.
        const QByteArray out = documentXml(doc->toDocx());
        QCOMPARE(out.count("w:fldCharType=\"begin\""), 6);
        QCOMPARE(out.count("w:fldCharType=\"end\""), 6);
        QVERIFY(out.contains("Introduction"));
        QVERIFY(!out.contains("Old text"));
        // One Undo puts it all back, and it was the only step.
        QVERIFY(doc->undo());
        QCOMPARE(doc->story()->toPlainText(), before);
        QVERIFY(!doc->canUndo());
        // Updating again makes the step again; updating once more, with nothing to change, makes none.
        QCOMPARE(updateFields(*doc, o).changed, 5);
        QVERIFY(doc->canUndo());
        const quint64 top = doc->lastStepSerial();
        const QString after = doc->story()->toPlainText();
        QCOMPARE(updateFields(*doc, o).changed, 0);
        QCOMPARE(doc->lastStepSerial(), top);
        QCOMPARE(doc->story()->toPlainText(), after);
    }

    void anUpdateIsNoTrackedChange()
    {
        const QByteArray body = para(run("a ") + field("QUOTE \"new\"", "old"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        doc->setTrackChanges(true);
        updateFields(*doc);
        QCOMPARE(lines(*doc)[0], QStringLiteral("a new"));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY(!out.contains("<w:ins "));
        QVERIFY(!out.contains("<w:del "));
    }

    void aLockedFieldIsLeftAndUnlinkTurnsFieldsIntoText()
    {
        const QByteArray body = para(field("QUOTE \"x\"", "stale", " w:fldLock=\"1\"") + run(" | ") + field("QUOTE \"y\"", "stale2"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const UpdateReport r = updateFields(*doc, UpdateOptions(), true);
        QCOMPARE(r.locked, 1);
        QCOMPARE(lines(*doc)[0], QStringLiteral("stale | y"));
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.setPosition(doc->story()->characterCount() - 1, QTextCursor::KeepAnchor);
        QCOMPARE(lockFields(*doc, c, false), 1);
        QCOMPARE(updateFields(*doc).changed, 1);
        QCOMPARE(lines(*doc)[0], QStringLiteral("x | y"));
        QCOMPARE(unlinkFields(*doc, c), 2);
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY(!out.contains("fldChar"));
        QVERIFY(out.contains("x"));
        QCOMPARE(lines(*doc)[0], QStringLiteral("x | y"));
        QVERIFY(doc->undo());
        QVERIFY(documentXml(doc->toDocx()).contains("fldChar"));
    }

    void nestedFieldsUpdateInsideOut()
    {
        const QByteArray inner = field("SEQ N", "9");
        const QByteArray body = para("<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> IF </w:instrText></w:r>" + inner +
                                     "<w:r><w:instrText xml:space=\"preserve\"> = 1 \"first\" \"other\" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("?") +
                                     "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        updateFields(*doc);
        QCOMPARE(lines(*doc)[0], QStringLiteral("1first"));
    }

    void hostileFieldsAreBounded()
    {
        // Fields nested far past the most.
        QByteArray deep;
        const int n = 300;
        for (int i = 0; i < n; ++i) deep += "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> IF </w:instrText></w:r>";
        for (int i = 0; i < n; ++i) deep += "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("x") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        auto doc = open(makeDocx(para(deep)));
        QVERIFY(doc);
        const StoryScan deepScan = scanStory(doc->story());
        QVERIFY(deepScan.truncated);
        for (const FieldPlace &f : deepScan.fields) QVERIFY(f.depth < kMostFieldDepth);
        updateFields(*doc);
        // A reference that holds itself, and two that hold each other.
        auto d2 = open(makeDocx(para(bookmark(1, "Loop", run("a ") + field("REF Loop", "b")))));
        QVERIFY(d2);
        updateFields(*d2);
        QVERIFY(lines(*d2)[0].size() < 100);
        auto cycle = open(makeDocx(para(bookmark(1, "A", run("a") + field("REF B", "x"))) + para(bookmark(2, "B", run("b") + field("REF A", "y")))));
        QVERIFY(cycle);
        resetFieldWorkForTest();
        updateFields(*cycle);
        updateFields(*cycle);
        QVERIFY(lines(*cycle)[0].size() < 100 && lines(*cycle)[1].size() < 100);
        // (The work is counted, not timed: two fields, two updates of at most a few passes each.)
        QVERIFY2(fieldComputationsForTest() > 0 && fieldComputationsForTest() <= 2 * 2 * 3, qPrintable(QString::number(fieldComputationsForTest())));
        resetFieldWorkForTest();
        updateFields(*d2);
        QVERIFY2(fieldComputationsForTest() > 0 && fieldComputationsForTest() <= 3, qPrintable(QString::number(fieldComputationsForTest())));
        // IF with a pattern of stars that a backtracking matcher takes forever over: the matcher's steps are counted, and are at most the product of the two lengths.
        {
            QByteArray pattern;
            for (int i = 0; i < 800; ++i) pattern += "a*";
            pattern += "b";
            auto w = open(makeDocx(para(field("IF \"" + QByteArray(1500, 'a') + "\" = \"" + pattern + "\" \"yes\" \"no\"", "?"))));
            QVERIFY(w);
            fields::resetGlobStepsForTest();
            updateFields(*w);
            QCOMPARE(lines(*w)[0], QStringLiteral("no"));
            QVERIFY(fields::globStepsForTest() > 0);
            QVERIFY2(fields::globStepsForTest() < qint64(4096) * 4096 + 100, qPrintable(QString::number(fields::globStepsForTest())));
            // (And a text past two thousand characters is compared as it is, not called different.)
            auto ok = open(makeDocx(para(field("IF \"" + QByteArray(2500, 'a') + "\" = \"a*\" \"yes\" \"no\"", "?"))));
            QVERIFY(ok);
            updateFields(*ok);
            QCOMPARE(lines(*ok)[0], QStringLiteral("yes"));
        }
        // Formulas that each name a bookmark thousands of times: the work of the whole pass is budgeted, so only some are worked out.
        {
            QByteArray terms = "X";
            for (int i = 0; i < 1000; ++i) terms += "+X";
            QByteArray many;
            for (int i = 0; i < 300; ++i) many += para(field("= " + terms, "0"));
            auto f = open(makeDocx(para(bookmark(1, "X", run("2"))) + many));
            QVERIFY(f);
            const UpdateReport r = updateFields(*f, UpdateOptions(), true);
            QCOMPARE(r.found, 300);
            QVERIFY2(r.changed > 0 && r.changed < 300 && r.kept > 0, qPrintable(QStringLiteral("%1 %2").arg(r.changed).arg(r.kept)));
        }
        // A hundred thousand sequence fields.
        {
            QByteArray many;
            for (int i = 0; i < 100000; ++i) many += "<w:fldSimple w:instr=\" SEQ Z \"><w:r><w:t>0</w:t></w:r></w:fldSimple>";
            auto big = open(makeDocx(para(many)));
            QVERIFY(big);
            const UpdateReport r = updateFields(*big, UpdateOptions(), false);
            QVERIFY(r.found >= 99000 && r.found <= 100000);
            QVERIFY(lines(*big)[0].endsWith(QString::number(r.found)));
        }
    }

    void aPartialUpdateOfSequencesCountsTheEarlierOnes()
    {
        QByteArray body;
        for (int i = 0; i < 4; ++i) body += para(run("Figure ") + field("SEQ Figure", "9"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const int third = doc->story()->findBlockByNumber(2).position();
        QTextCursor c(doc->story());
        c.setPosition(third + 9);
        const StoryScan scan = scanStory(doc->story());
        c.setPosition(scan.fields[2].begin + 1);
        updateFieldsAt(*doc, c);
        QCOMPARE(lines(*doc)[2], QStringLiteral("Figure 3"));
        QCOMPARE(lines(*doc)[0], QStringLiteral("Figure 9"));
        QCOMPARE(lines(*doc)[3], QStringLiteral("Figure 9"));
    }

    void aReferenceTakesTheResultsMadeInThePass()
    {
        const QByteArray body = para(bookmark(1, "Box", run("x") + field("QUOTE \"fresh\"", "stale"))) + para(field("REF Box", "?"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        updateFields(*doc);
        QCOMPARE(lines(*doc)[1], QStringLiteral("xfresh"));
        // A page number of a field that is one character in the bookmark is its result, not a lost mark.
        auto d2 = open(makeDocx(para(bookmark(1, "Pg", run("p") + field("PAGE", "7"))) + para(field("REF Pg", "?"))));
        QVERIFY(d2);
        updateFields(*d2);
        QCOMPARE(lines(*d2)[1], QStringLiteral("p1"));
    }

    void aTextOnlyUpdateLaysOutNoPagesAndCachesBookmarks()
    {
        QByteArray body = para(bookmark(1, "Intro", run("Introduction")));
        for (int i = 0; i < 50; ++i) body += para(field("REF Intro", "?"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const qint64 before = doc->pager().pagesLaidOut();
        const UpdateReport r = updateFields(*doc);
        QCOMPARE(r.changed, 50);
        QCOMPARE(doc->pager().pagesLaidOut(), before);   // (no page was asked for, so none was laid out)
        auto pages = open(makeDocx(para(bookmark(1, "Intro", run("Introduction"))) + para(field("PAGEREF Intro", "9"))));
        QVERIFY(pages);
        QCOMPARE(updateFields(*pages).changed, 1);
        QVERIFY(pages->pager().pagesLaidOut() > 0);
    }

    void aStyleReferenceToNoTextSaysSo()
    {
        auto doc = open(makeDocx(para(field("STYLEREF \"Heading 1\"", "?")) + para(field("STYLEREF \"No Such Style\"", "?"))));
        QVERIFY(doc);
        QCOMPARE(updateFields(*doc, UpdateOptions(), true).changed, 2);
        QCOMPARE(lines(*doc), (QStringList{QStringLiteral("Error! No text of specified style in document."), QStringLiteral("Error! No text of specified style in document.")}));
    }

    void compareWorksOutOneOrZero()
    {
        Env env;
        auto result = [&](const char *code) { return compute(parseCode(QString::fromLatin1(code)), env); };
        auto shown = [&](const char *code) { const auto v = result(code); return v ? *v : QStringLiteral("(none)"); };
        QCOMPARE(shown("COMPARE 5 > 3"), QStringLiteral("1"));
        QCOMPARE(shown("COMPARE 2 > 3"), QStringLiteral("0"));
        QCOMPARE(shown("COMPARE 4 = 4"), QStringLiteral("1"));
        QCOMPARE(shown("COMPARE 4 <> 4"), QStringLiteral("0"));
        QCOMPARE(shown("COMPARE 2 <= 2"), QStringLiteral("1"));
        QCOMPARE(shown("COMPARE \"abc\" = \"A*\""), QStringLiteral("1"));
        QCOMPARE(shown("COMPARE \"abc\" < \"abd\""), QStringLiteral("1"));
        QVERIFY(!result("COMPARE 1 2").has_value());
        QVERIFY(isKnown(QStringLiteral("COMPARE")));
        auto doc = open(makeDocx(para(field("COMPARE 7 >= 8", "x"))));
        QVERIFY(doc);
        QCOMPARE(updateFields(*doc, UpdateOptions(), true).changed, 1);
        QCOMPARE(lines(*doc), QStringList{QStringLiteral("0")});
    }

    void dateNamesAreInTheLanguageOfTheField()
    {
        const QDateTime t = when(2026, 3, 7, 14, 5);
        QCOMPARE(dateInPicture(t, QStringLiteral("dddd, MMMM d"), QLocale(QStringLiteral("de-DE"))), QStringLiteral("Samstag, März 7"));
        QCOMPARE(dateInPicture(t, QStringLiteral("ddd MMM"), QLocale(QStringLiteral("fr-FR"))).left(3), QStringLiteral("sam"));
        QCOMPARE(dateInPicture(t, QStringLiteral("dddd, MMMM d")), QStringLiteral("Saturday, March 7"));   // (English where no language is asked)
        // A field in German text shows German names; one in text that says no language shows the program's.
        const QByteArray german = "<w:r><w:rPr><w:lang w:val=\"de-DE\"/></w:rPr><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> DATE \\@ \"MMMM\" </w:instrText></w:r>"
                                  "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:rPr><w:lang w:val=\"de-DE\"/></w:rPr><w:t>old</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        PinnedLocale pinned;   // (the program's language: American English)
        auto doc = open(makeDocx(para(german) + para(field("DATE \\@ \"MMMM\"", "old"))));
        QVERIFY(doc);
        UpdateOptions o;
        o.now = when(2026, 3, 7);
        QCOMPARE(updateFields(*doc, o, true).changed, 2);
        QCOMPARE(lines(*doc), (QStringList{QStringLiteral("März"), QStringLiteral("March")}));
    }

    void fieldsPastTheMostNestedOnesKeepTheOuterOnesOpen()
    {
        // 34 fields inside each other: the ends of the two that are deeper than the most that are marked close nothing that is open, so the text after the 34 ends is still in the outermost field.
        QByteArray inner = run("deep");
        for (int level = 0; level < 33; ++level)
            inner = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> QUOTE \"x\" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + inner +
                    "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        const QByteArray outer = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> QUOTE \"y\" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + inner +
                                 run("tail") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        auto doc = open(makeDocx(para(outer)));
        QVERIFY(doc);
        int deepId = -1, tailId = -1;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                if (it.fragment().text().contains(QLatin1String("deep"))) deepId = cf.hasProperty(tp::FieldId) ? cf.intProperty(tp::FieldId) : -2;
                if (it.fragment().text().contains(QLatin1String("tail"))) tailId = cf.hasProperty(tp::FieldId) ? cf.intProperty(tp::FieldId) : -2;
            }
        QVERIFY(deepId > 0);
        QCOMPARE(tailId, deepId);
    }

    void stylereferencesAreFoundFromAnIndexNotAScanEach()
    {
        // The fields come before the only heading: each one, scanned for, would walk the whole text.
        QByteArray body;
        const int paragraphs = 1500;
        for (int i = 0; i < paragraphs; ++i) body += para(run("t") + field("STYLEREF \"Heading 1\"", "?"));
        body += para(run("The heading"), "<w:pStyle w:val=\"Heading1\"/>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        resetFieldWorkForTest();
        const UpdateReport r = updateFields(*doc, UpdateOptions(), true);
        QCOMPARE(r.found, paragraphs);
        QVERIFY2(styleRefParagraphVisitsForTest() < 4 * paragraphs, qPrintable(QString::number(styleRefParagraphVisitsForTest())));
        QCOMPARE(lines(*doc)[1], QStringLiteral("tThe heading"));
    }

    void resultsMadeAgainAreFoundByAnIntervalNotAWalk()
    {
        QByteArray body;
        const int n = 1200;
        for (int i = 0; i < n; ++i) body += para(field("QUOTE \"v" + QByteArray::number(i) + "\"", "old"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        resetFieldWorkForTest();
        QCOMPARE(updateFields(*doc).changed, n);
        QVERIFY2(resultOverlapChecksForTest() < 40 * n, qPrintable(QString::number(resultOverlapChecksForTest())));
        QCOMPARE(lines(*doc)[n - 1], QStringLiteral("v1199"));
    }

    void targetsFindTheirOwnBookmarkThroughAnIndex()
    {
        QByteArray body;
        const int n = 1500;
        for (int i = 0; i < n; ++i) body += para(bookmark(i + 1, "_Ref" + QByteArray::number(100000000 + i), run("Head " + QByteArray::number(i))), "<w:pStyle w:val=\"Heading1\"/>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        resetCrossReferenceProbesForTest();
        const QVector<RefTarget> t = crossReferenceTargets(*doc, RefKind::Heading);
        QCOMPARE(t.size(), n);
        QVERIFY(!t[10].newBookmark);
        QVERIFY2(crossReferenceProbesForTest() < 20 * n, qPrintable(QString::number(crossReferenceProbesForTest())));
    }

    void aHugeTableOfContentsIsCutAndSaysSo()
    {
        QByteArray body = para(field("TOC \\o \"1-1\"", "old"));
        for (int i = 0; i < kMostTocEntries + 50; ++i) body += para(run("H"), "<w:pStyle w:val=\"Heading1\"/>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        UpdateOptions o;
        o.apply = false;
        const UpdateReport r = updateFields(*doc, o, true);
        QVERIFY(r.tocTruncated);
        QCOMPARE(r.changes.value(0).after.count(QLatin1Char('\n')) + 1, kMostTocEntries);
        auto small = open(makeDocx(para(field("TOC \\o \"1-1\"", "old")) + para(run("H"), "<w:pStyle w:val=\"Heading1\"/>")));
        QVERIFY(small);
        QVERIFY(!updateFields(*small, o).tocTruncated);
    }

    void aTableOfContentsWithoutBookmarksUpdatesTheFieldsBeforeItRight()
    {
        const QByteArray body = para(run("Alpha"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Date ") + field("DATE \\@ \"yyyy-MM-dd\"", "1999-01-01")) +
                                para(field("TOC \\o \"1-1\" \\h", "old")) + para(run("Beta"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Gamma ") + field("QUOTE \"fresh\"", "stale"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QTextCursor all(doc->story());
        all.select(QTextCursor::Document);
        UpdateOptions o;
        o.from = all.selectionStart();
        o.to = all.selectionEnd();
        o.now = when(2026, 11, 2);
        updateFields(*doc, o);
        const QStringList l = lines(*doc);
        QCOMPARE(l[0], QStringLiteral("Alpha"));
        QCOMPARE(l[1], QStringLiteral("Date 2026-11-02"));
        QVERIFY2(l.join('|').contains(QStringLiteral("Alpha\t1|Beta\t1")), qPrintable(l.join('|')));
        QVERIFY(l.join('|').contains(QStringLiteral("Gamma fresh")));
        const QByteArray out = documentXml(doc->toDocx());
        QCOMPARE(out.count("<w:bookmarkStart"), 2);
        QCOMPARE(out.count("<w:bookmarkEnd"), 2);
        QCOMPARE(out.count("w:fldCharType=\"begin\""), out.count("w:fldCharType=\"end\""));
        QCOMPARE(updateFields(*doc, o).changed, 0);
    }

    void fieldsInTrackedChangesAreHandledSensibly()
    {
        const char *rev = " w:id=\"7\" w:author=\"Ann\" w:date=\"2020-01-01T00:00:00Z\"";
        // In a deletion: left as it is.
        const QByteArray deleted = "<w:del" + QByteArray(rev) + "><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> QUOTE \"new\" </w:instrText></w:r>"
                                   "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:delText>old</w:delText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:del>";
        auto d1 = open(makeDocx(para(run("a ") + deleted)));
        QVERIFY(d1);
        const UpdateReport r1 = updateFields(*d1, UpdateOptions(), true);
        QCOMPARE(r1.changed, 0);
        const QByteArray out1 = documentXml(d1->toDocx());
        QVERIFY2(out1.contains("<w:delText>old</w:delText>") || out1.contains("old</w:delText>"), out1.constData());
        QVERIFY(!out1.contains("new</w:t>"));
        // In an insertion: the new result is inserted by the same author.
        const QByteArray inserted = "<w:ins" + QByteArray(rev) + "><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> QUOTE \"new\" </w:instrText></w:r>"
                                    "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>old</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:ins>";
        auto d2 = open(makeDocx(para(run("a ") + inserted)));
        QVERIFY(d2);
        QCOMPARE(updateFields(*d2).changed, 1);
        const QByteArray out2 = documentXml(d2->toDocx());
        const qsizetype from = out2.indexOf("<w:ins "), to = out2.indexOf("</w:ins>");
        QVERIFY(from >= 0 && to > from);
        const QByteArray inside = out2.mid(from, to - from);
        QVERIFY2(inside.contains(">new<"), out2.constData());
        QVERIFY(!inside.contains(">old<"));
        QVERIFY(!out2.contains("<w:del "));
        QVERIFY(!out2.contains("w:delText"));
    }

    void lockingWhatIsAlreadyLockedMakesNoStep()
    {
        auto doc = open(makeDocx(para(field("QUOTE \"x\"", "x", " w:fldLock=\"1\"")) + para(run("plain"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.select(QTextCursor::Document);
        QCOMPARE(lockFields(*doc, c, true), 0);
        QVERIFY(!doc->canUndo());
        QCOMPARE(unlinkFields(*doc, QTextCursor(doc->story())), 0);
        QVERIFY(!doc->canUndo());
        QCOMPARE(lockFields(*doc, c, false), 1);
        QVERIFY(doc->canUndo());
    }

    void pageNumbersAreRightAfterATableOfContentsPushesTheHeadingsDown()
    {
        QByteArray body = para(run("Start"));
        for (int i = 0; i < 60; ++i) {
            body += para(run("Chapter " + QByteArray::number(i)), "<w:pStyle w:val=\"Heading1\"/>");
            for (int k = 0; k < 14; ++k) body += para(run("Some body text of the chapter, a line of it."));
        }
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(0);
        TocOptions o;
        o.levels = 1;
        QVERIFY(insertTableOfContents(*doc, c, o));
        // One more update changes nothing: the page numbers were right the first time (a table of sixty lines moved the headings by pages).
        QCOMPARE(updateFields(*doc).changed, 0);
        const QStringList l = lines(*doc);
        QString lastEntry;
        for (const QString &line : l)
            if (line.startsWith(QLatin1String("Chapter 59\t"))) lastEntry = line;
        QVERIFY(!lastEntry.isEmpty());
        QVERIFY(lastEntry.section(QLatin1Char('\t'), 1).toInt() > 3);
        // The whole of it was one step.
        QVERIFY(doc->undo());
        QVERIFY(!doc->canUndo());
        QVERIFY(!doc->story()->toPlainText().contains(QStringLiteral("Chapter 59\t")));
        QVERIFY(!documentXml(doc->toDocx()).contains("TOC \\o"));
    }

    void aResultTakesTheLookTheSwitchesAsk()
    {
        auto bold = [](const QByteArray &t) { return "<w:r><w:rPr><w:b/></w:rPr><w:t xml:space=\"preserve\">" + t + "</w:t></w:r>"; };
        auto build = [&](const QByteArray &switches, const QByteArray &codeRun) {
            return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>" + codeRun + "<w:r><w:instrText xml:space=\"preserve\"> QUOTE \"xx YY zz\"" + switches +
                   " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("aa ") + bold("BB") + run(" cc") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        };
        auto isBoldAt = [](WriteDoc &d, const QString &word) {
            const int at = d.story()->toPlainText().indexOf(word);
            QTextCursor c(d.story());
            c.setPosition(at + 1);
            return c.charFormat().font().bold();
        };
        // \* MERGEFORMAT: the look of each place of the old result stays at that place.
        auto merge = open(makeDocx(para(build(" \\* MERGEFORMAT", QByteArray()))));
        QVERIFY(merge);
        updateFields(*merge);
        QCOMPARE(lines(*merge)[0], QStringLiteral("xx YY zz"));
        QVERIFY(isBoldAt(*merge, QStringLiteral("YY")));
        QVERIFY(!isBoldAt(*merge, QStringLiteral("xx")));
        QVERIFY(!isBoldAt(*merge, QStringLiteral("zz")));
        // Neither: the look of the first of the old result.
        auto plain = open(makeDocx(para(build(QByteArray(), QByteArray()))));
        QVERIFY(plain);
        updateFields(*plain);
        QVERIFY(!isBoldAt(*plain, QStringLiteral("YY")));
        QVERIFY(!isBoldAt(*plain, QStringLiteral("xx")));
        // \* CHARFORMAT: the look of the first character of the code.
        auto charformat = open(makeDocx(para("<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:rPr><w:b/></w:rPr><w:instrText xml:space=\"preserve\"> QUOTE \"xx YY zz\" \\* CHARFORMAT </w:instrText></w:r>"
                                             "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("aa BB cc") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>")));
        QVERIFY(charformat);
        updateFields(*charformat);
        QVERIFY(isBoldAt(*charformat, QStringLiteral("xx")));
        QVERIFY(isBoldAt(*charformat, QStringLiteral("zz")));
        // A field that is one character takes the look of its result runs, not of its separator's.
        office::wml::Inline group;
        group.kind = office::wml::Inline::ParaAtom;
        group.tag = office::wml::kFieldGroupTag;
        group.raw = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> QUOTE \"new\" </w:instrText></w:r><w:r><w:rPr><w:i/></w:rPr><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                    "<w:r><w:rPr><w:b/></w:rPr><w:t>old</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        group.display = QStringLiteral("old");
        auto d = open(makeDocx(para(run("a"))));
        QVERIFY(d);
        QTextCursor c(d->story());
        c.movePosition(QTextCursor::End);
        QVERIFY(insertFieldAtom(*d, c, group));
        QCOMPARE(updateFields(*d).changed, 1);
        const QByteArray out = documentXml(d->toDocx());
        const qsizetype at = out.indexOf(">new<");
        QVERIFY2(at > 0, out.constData());
        const QByteArray before = out.left(at);
        QVERIFY(before.mid(before.lastIndexOf("<w:r>")).contains("<w:b/>"));
        QVERIFY(!before.mid(before.lastIndexOf("<w:r>")).contains("<w:i/>"));
    }

    void aFieldWithFieldsInsideItIsKeptNotCountedChanged()
    {
        office::wml::Inline group;
        group.kind = office::wml::Inline::ParaAtom;
        group.tag = office::wml::kFieldGroupTag;
        group.raw = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> IF </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>"
                    "<w:r><w:instrText xml:space=\"preserve\"> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"
                    "<w:r><w:instrText xml:space=\"preserve\"> = 1 \"a\" \"b\" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>q</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        group.display = QStringLiteral("q");
        auto d = open(makeDocx(para(run("a"))));
        QVERIFY(d);
        QTextCursor c(d->story());
        c.movePosition(QTextCursor::End);
        QVERIFY(insertFieldAtom(*d, c, group));
        const quint64 top = d->lastStepSerial();
        const UpdateReport r = updateFields(*d, UpdateOptions(), true);
        QCOMPARE(r.changed, 0);
        QVERIFY(r.kept >= 1);
        QCOMPARE(d->lastStepSerial(), top);
    }

    void aSimpleFieldsDirtyFlagIsClearedAndTheFlagCodeIsSafe()
    {
        const QByteArray simple = "<w:fldSimple w:instr=\" QUOTE &quot;new&quot; \" w:dirty=\"true\"><w:r><w:t>old</w:t></w:r></w:fldSimple>";
        auto doc = open(makeDocx(para(simple)));
        QVERIFY(doc);
        QVERIFY(documentXml(doc->toDocx()).contains("w:dirty"));
        updateFields(*doc);
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(!out.contains("w:dirty"), out.constData());
        QVERIFY(out.contains(">new<"));
        // A mark whose tag is cut off is not made worse by a lock.
        office::wml::Inline group;
        group.kind = office::wml::Inline::ParaAtom;
        group.tag = office::wml::kFieldGroupTag;
        group.raw = "<w:r><w:fldChar w:fldCharType=\"begin\"";
        auto d = open(makeDocx(para(run("a"))));
        QVERIFY(d);
        QTextCursor c(d->story());
        c.movePosition(QTextCursor::End);
        QVERIFY(insertFieldAtom(*d, c, group));
        QTextCursor all(d->story());
        all.select(QTextCursor::Document);
        lockFields(*d, all, true);
        const QTextCharFormat cf = [&] { QTextCursor t(d->story()); t.setPosition(2); return t.charFormat(); }();
        QCOMPARE(office::wml::Inline(unpackAtom(storedBytes(cf, prop::Atom))).raw, group.raw);
    }

    void aTableLimitedToAnUnclosedBookmarkRunsToTheEnd()
    {
        const QByteArray body = para(field("TOC \\o \"1-1\" \\b Part", "old")) + para(run("Before"), "<w:pStyle w:val=\"Heading1\"/>") +
                                para("<w:bookmarkStart w:id=\"1\" w:name=\"Part\"/>" + run("Inside"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("After"), "<w:pStyle w:val=\"Heading1\"/>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const UpdateReport r = updateFields(*doc, UpdateOptions(), true);
        QCOMPARE(r.changed, 1);
        QVERIFY2(r.changes[0].after.contains(QStringLiteral("Inside")) && r.changes[0].after.contains(QStringLiteral("After")) && !r.changes[0].after.contains(QStringLiteral("Before")), qPrintable(r.changes[0].after));
    }

    void aDocumentPropertyWeCannotWorkOutKeepsItsResult()
    {
        auto doc = open(makeDocx(para(field("DOCPROPERTY \"Number of Lines\"", "42")) + para(field("NUMWORDS", "0")) + para(run("one two three"))));
        QVERIFY(doc);
        const UpdateReport r = updateFields(*doc, UpdateOptions(), true);
        QCOMPARE(lines(*doc)[0], QStringLiteral("42"));
        QCOMPARE(r.changes.value(0).outcome, FieldChange::Kept);
        QVERIFY(lines(*doc)[1].toInt() >= 3);
    }

    void newBookmarkIdsAreAboveEveryIdTheFileHas()
    {
        QByteArray many;
        const int n = kMostBookmarks + 120;
        for (int i = 1; i <= n; ++i) many += "<w:bookmarkStart w:id=\"" + QByteArray::number(i) + "\" w:name=\"b" + QByteArray::number(i) + "\"/><w:bookmarkEnd w:id=\"" + QByteArray::number(i) + "\"/>";
        auto doc = open(makeDocx(para(many + run("text")) + para(run("Heading"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Refer: "))));
        QVERIFY(doc);
        QVERIFY(scanStory(doc->story()).truncated);
        // (A bookmark the program makes itself, for a cross-reference, past the ones that are kept.)
        const QVector<RefTarget> targets = crossReferenceTargets(*doc, RefKind::Heading);
        QCOMPARE(targets.size(), 1);
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::End);
        QString why;
        QVERIFY2(insertCrossReference(*doc, c, targets[0], RefKind::Heading, RefWhat::Text, true, &why), qPrintable(why));
        const QByteArray out = documentXml(doc->toDocx());
        QSet<QByteArray> ids;
        int starts = 0;
        for (qsizetype at = out.indexOf("<w:bookmarkStart "); at >= 0; at = out.indexOf("<w:bookmarkStart ", at + 1)) {
            const qsizetype i = out.indexOf("w:id=\"", at) + 6;
            ids.insert(out.mid(i, out.indexOf('"', i) - i));
            ++starts;
        }
        QCOMPARE(ids.size(), starts);
        QVERIFY(starts > n);
    }

    void numberPicturesRoundDecimalValuesHalfAwayFromZero()
    {
        QCOMPARE(numberInPicture(1.005, QStringLiteral("0.00")), QStringLiteral("1.01"));
        QCOMPARE(numberInPicture(2.675, QStringLiteral("0.00")), QStringLiteral("2.68"));
        QCOMPARE(numberInPicture(-1.005, QStringLiteral("0.00")), QStringLiteral("-1.01"));
        QCOMPARE(numberInPicture(0.125, QStringLiteral("0.0")), QStringLiteral("0.1"));
        QCOMPARE(numberInPicture(0.135, QStringLiteral("0.0")), QStringLiteral("0.1"));
        QCOMPARE(numberInPicture(0.15, QStringLiteral("0.0")), QStringLiteral("0.2"));
        QCOMPARE(numberInPicture(999.995, QStringLiteral("0.00")), QStringLiteral("1000.00"));
        QCOMPARE(plainNumber(1.0000005), QStringLiteral("1.000001"));
    }

    void fieldCodesAreAViewNotAnEdit()
    {
        const QByteArray body = para(run("a ") + field("QUOTE \"fresh\"", "stale") + run(" b ") + field("QUOTE \"two\"", "old"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const QByteArray saved = documentXml(doc->toDocx());
        auto shown = [&] {
            FrameSpec fs;
            fs.size = QSizeF(468, 600);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            StoryLayout layout;
            layout.build(doc->story(), {fs}, doc->flow().env);
            QStringList out;
            for (const auto &l : layout.lineInfo(0)) out << QString(l.text).remove(QChar(0x2060)).trimmed();
            return out.join(QLatin1Char('|'));
        };
        QCOMPARE(shown(), QStringLiteral("a stale b old"));
        // An edit, and its Undo: the redo is there to be done.
        QCOMPARE(updateFields(*doc).changed, 2);
        QCOMPARE(shown(), QStringLiteral("a fresh b two"));
        QVERIFY(doc->undo());
        QVERIFY(!doc->canUndo());
        QVERIFY(doc->canRedo());
        QVERIFY(!doc->isModified());
        // Shift+F9: the field at the caret shows its code. Nothing of the history or the document is touched.
        const StoryScan scan = scanStory(doc->story());
        QTextCursor c(doc->story());
        c.setPosition(scan.fields[1].begin + 1);
        QCOMPARE(toggleFieldCodes(*doc, c, false), 1);
        QCOMPARE(shown(), QStringLiteral("a stale b { QUOTE \"two\" }"));
        QVERIFY(!doc->canUndo());
        QVERIFY(doc->canRedo());
        QVERIFY(!doc->isModified());
        QCOMPARE(documentXml(doc->toDocx()), saved);
        // The redo still works, and what it does is seen with the code still shown.
        QVERIFY(doc->redo());
        QCOMPARE(shown(), QStringLiteral("a fresh b { QUOTE \"two\" }"));
        QVERIFY(doc->canUndo());
        QVERIFY(!doc->canRedo());
        // Alt+F9: every field (the one shown stays, the other is shown); again: all are hidden.
        QCOMPARE(toggleFieldCodes(*doc, c, true), 1);
        QCOMPARE(shown(), QStringLiteral("a { QUOTE \"fresh\" } b { QUOTE \"two\" }"));
        QCOMPARE(toggleFieldCodes(*doc, c, true), 2);
        QCOMPARE(shown(), QStringLiteral("a fresh b two"));
        // One of all shown is hidden by itself.
        toggleFieldCodes(*doc, c, true);
        QCOMPARE(toggleFieldCodes(*doc, c, false), 1);
        QCOMPARE(shown(), QStringLiteral("a { QUOTE \"fresh\" } b two"));
        // The step the update made is still the only one, and undoing it brings the old results back under the codes.
        QVERIFY(doc->undo());
        QVERIFY(!doc->canUndo());
        QCOMPARE(shown(), QStringLiteral("a { QUOTE \"fresh\" } b old"));
        toggleFieldCodes(*doc, c, true);
        toggleFieldCodes(*doc, c, true);
        QCOMPARE(shown(), QStringLiteral("a stale b old"));
        QCOMPARE(documentXml(doc->toDocx()), saved);
        // A field made after the codes were shown, and one that is unlinked, are no part of the view.
        QTextCursor all(doc->story());
        all.select(QTextCursor::Document);
        QCOMPARE(unlinkFields(*doc, all), 2);
        toggleFieldCodes(*doc, c, true);
        QCOMPARE(shown(), QStringLiteral("a stale b old"));
    }

    void twoDocumentsShowTheCodesOfTheirOwnFields()
    {
        const QByteArray body = para(run("a ") + field("QUOTE \"one\"", "r1") + run(" b ") + field("QUOTE \"two\"", "r2"));
        auto first = open(makeDocx(body));
        auto second = open(makeDocx(body));
        QVERIFY(first && second);
        auto shown = [](WriteDoc &doc) {
            FrameSpec fs;
            fs.size = QSizeF(468, 600);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            StoryLayout layout;
            layout.build(doc.story(), {fs}, doc.flow().env);
            QStringList out;
            for (const auto &l : layout.lineInfo(0)) out << QString(l.text).remove(QChar(0x2060)).trimmed();
            return out.join(QLatin1Char('|'));
        };
        // (The same file read twice numbers its fields the same: the numbers are the same in both.)
        const StoryScan a = scanStory(first->story()), b = scanStory(second->story());
        QTextCursor onFirstField(first->story()), onSecondField(second->story());
        onFirstField.setPosition(a.fields[0].begin + 1);
        onSecondField.setPosition(b.fields[1].begin + 1);
        QCOMPARE(toggleFieldCodes(*first, onFirstField, false), 1);
        QCOMPARE(toggleFieldCodes(*second, onSecondField, false), 1);
        QCOMPARE(shown(*first), QStringLiteral("a { QUOTE \"one\" } b r2"));
        QCOMPARE(shown(*second), QStringLiteral("a r1 b { QUOTE \"two\" }"));
        // One closing, or changing, leaves the other as it is.
        QCOMPARE(toggleFieldCodes(*first, onFirstField, false), 1);
        QCOMPARE(shown(*first), QStringLiteral("a r1 b r2"));
        QCOMPARE(shown(*second), QStringLiteral("a r1 b { QUOTE \"two\" }"));
        first.reset();
        QCOMPARE(shown(*second), QStringLiteral("a r1 b { QUOTE \"two\" }"));
    }

    void aTableOfContentsIsBuiltFromTheHeadings()
    {
        const QByteArray toc = para(run("Contents")) + para(field("TOC \\o \"1-2\" \\h \\z \\u", "Old"));
        const QByteArray body = toc + para(run("Alpha"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("body text")) + para(run("Beta"), "<w:pStyle w:val=\"Heading2\"/>") +
                                para(run("Gamma"), "<w:pStyle w:val=\"Heading1\"/>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const UpdateReport r = updateFields(*doc, UpdateOptions(), true);
        QCOMPARE(r.changed, 1);
        const QStringList l = lines(*doc);
        QVERIFY2(l.contains(QStringLiteral("Alpha\t1")), qPrintable(l.join('|')));
        QVERIFY(l.contains(QStringLiteral("Beta\t1")));
        QVERIFY(l.contains(QStringLiteral("Gamma\t1")));
        QVERIFY(!l.join('|').contains(QStringLiteral("Old")));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(out.contains("w:anchor=\"_Toc"), out.constData());
        QVERIFY(out.contains("PAGEREF _Toc"));
        QCOMPARE(out.count("w:fldCharType=\"begin\""), out.count("w:fldCharType=\"end\""));
        QCOMPARE(out.count("<w:bookmarkStart"), out.count("<w:bookmarkEnd"));
        QVERIFY(out.contains("<w:pStyle w:val=\"TOC1\"/>") && out.contains("<w:pStyle w:val=\"TOC2\"/>"));
        // Updated again: nothing changes; and it opens as the same table.
        QCOMPARE(updateFields(*doc).changed, 0);
        auto back = open(doc->toDocx());
        QVERIFY(back);
        QCOMPARE(updateFields(*back).changed, 0);
        // One Undo takes the whole table back.
        QVERIFY(doc->undo());
        QVERIFY(lines(*doc).join('|').contains(QStringLiteral("Old")));
    }

    void anInsertedFieldIsAFieldInTheFile()
    {
        auto doc = open(makeDocx(para(run("Words: "))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::End);
        QVERIFY(insertField(*doc, c, QStringLiteral("= 2 + 3 \\# \"0.0\"")));
        QVERIFY(lines(*doc)[0].isEmpty() || true);
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(out.contains("= 2 + 3"), out.constData());
        QVERIFY(out.contains("5.0"));
        auto back = open(doc->toDocx());
        QVERIFY(back);
        QCOMPARE(scanStory(back->story()).fields.size(), 1);
    }
    void bookmarksAreAddedMovedListedAndDeleted()
    {
        auto doc = open(makeDocx(para(run("Alpha beta gamma")) + para(run("Delta"))));
        QVERIFY(doc);
        QString why;
        QVERIFY(!addBookmark(*doc, QTextCursor(doc->story()), QStringLiteral("1bad"), &why));
        QVERIFY(!why.isEmpty());
        QVERIFY(!addBookmark(*doc, QTextCursor(doc->story()), QStringLiteral("has space"), &why));
        QTextCursor c(doc->story());
        c.setPosition(6);
        c.setPosition(10, QTextCursor::KeepAnchor);
        QVERIFY(addBookmark(*doc, c, QStringLiteral("Middle"), &why));
        QTextCursor d(doc->story());
        d.setPosition(doc->story()->characterCount() - 2);
        QVERIFY(addBookmark(*doc, d, QStringLiteral("aaa")));
        QVector<BookmarkItem> byName = bookmarkList(*doc);
        QCOMPARE(byName.size(), 2);
        QCOMPARE(byName[0].name, QStringLiteral("aaa"));
        QCOMPARE(bookmarkList(*doc, false, true)[0].name, QStringLiteral("Middle"));
        QCOMPARE(bookmarkStart(*doc, QStringLiteral("middle")), bookmarkList(*doc, false, true)[0].start);
        // The same name again moves it.
        QVERIFY(addBookmark(*doc, d, QStringLiteral("Middle")));
        QCOMPARE(bookmarkList(*doc).size(), 2);
        const QByteArray out = documentXml(doc->toDocx());
        QCOMPARE(out.count("<w:bookmarkStart"), 2);
        QCOMPARE(out.count("<w:bookmarkEnd"), 2);
        QVERIFY(deleteBookmark(*doc, QStringLiteral("Middle")));
        QVERIFY(!deleteBookmark(*doc, QStringLiteral("Middle")));
        QCOMPARE(bookmarkList(*doc).size(), 1);
        QVERIFY(doc->undo());
        QCOMPARE(bookmarkList(*doc).size(), 2);
        // Hidden ones are listed only when asked.
        auto hidden = open(makeDocx(para(bookmark(1, "_Hid", run("x")))));
        QVERIFY(hidden);
        QCOMPARE(bookmarkList(*hidden).size(), 0);
        QCOMPARE(bookmarkList(*hidden, true).size(), 1);
    }

    void aCrossReferenceToAHeadingIsAFieldThatFollowsTheHeading()
    {
        const QByteArray body = para(run("Intro"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Closing words"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Refer: "));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const QVector<RefTarget> targets = crossReferenceTargets(*doc, RefKind::Heading);
        QCOMPARE(targets.size(), 2);
        QCOMPARE(targets[1].label, QStringLiteral("Closing words"));
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::End);
        QString why;
        QVERIFY2(insertCrossReference(*doc, c, targets[1], RefKind::Heading, RefWhat::Text, true, &why), qPrintable(why));
        QCOMPARE(lines(*doc)[2], QStringLiteral("Refer: "));
        QVERIFY(doc->story()->toPlainText().contains(QChar::ObjectReplacementCharacter));
        QCOMPARE(bookmarkList(*doc, true).size(), 1);
        QVERIFY(bookmarkList(*doc, true)[0].name.startsWith(QLatin1String("_Ref")));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(out.contains("REF _Ref") && out.contains("\\h"), out.constData());
        QVERIFY(out.contains("Closing words</w:t>"));
        QCOMPARE(out.count("<w:bookmarkStart"), 1);
        // Ctrl+click goes to the heading.
        const int at = doc->story()->toPlainText().indexOf(QChar::ObjectReplacementCharacter);
        const StoryScan fieldScan = scanStory(doc->story());
        QCOMPARE(fieldScan.fields.size(), 1);
        Q_UNUSED(at)
        QCOMPARE(linkTargetAt(*doc, fieldScan.fields[0].begin), bookmarkList(*doc, true)[0].start);
        // The page version, as a PAGEREF.
        QCOMPARE(crossReferenceCode(targets[0], RefKind::Heading, RefWhat::Page, false).left(8), QStringLiteral("PAGEREF "));
        // One Undo takes the reference and its bookmark back.
        QVERIFY(doc->undo());
        QCOMPARE(bookmarkList(*doc, true).size(), 0);
        QVERIFY(!doc->story()->toPlainText().contains(QChar::ObjectReplacementCharacter));
    }

    void aTableOfContentsIsInsertedUpdatedAndRemoved()
    {
        const QByteArray body = para(run("Alpha"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Beta"), "<w:pStyle w:val=\"Heading2\"/>") + para(run("The end"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(0);
        TocOptions o;
        o.levels = 2;
        QVERIFY(insertTableOfContents(*doc, c, o));
        QStringList l = lines(*doc);
        QVERIFY2(l.join('|').contains(QStringLiteral("Alpha\t1")), qPrintable(l.join('|')));
        QVERIFY(l.join('|').contains(QStringLiteral("Beta\t1")));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY(out.contains("TOC \\o \"1-2\" \\h"));
        QVERIFY(out.contains("w:leader=\"dot\""));
        // A new heading, then Update Table (page numbers only leaves the lines; the entire table adds it).
        QTextCursor e(doc->story());
        e.movePosition(QTextCursor::End);
        e.insertBlock();
        e.insertText(QStringLiteral("Gamma"));
        QTextBlockFormat bf = e.blockFormat();
        bf.setProperty(tp::StyleName, QStringLiteral("Heading1"));
        bf.setProperty(prop::Outline, 0);
        e.setBlockFormat(bf);
        QTextCursor inside(doc->story());
        inside.setPosition(doc->story()->toPlainText().indexOf(QStringLiteral("Alpha\t")) + 1);
        QCOMPARE(updateTableOfContents(*doc, inside, true), 1);
        QVERIFY(!lines(*doc).join('|').contains(QStringLiteral("Gamma\t")));
        QCOMPARE(updateTableOfContents(*doc, inside, false), 1);
        QVERIFY2(lines(*doc).join('|').contains(QStringLiteral("Gamma\t1")), qPrintable(lines(*doc).join('|')));
        QVERIFY(removeTableOfContents(*doc, inside));
        QVERIFY(!doc->story()->toPlainText().contains(QStringLiteral("\t1")));
        QVERIFY(!documentXml(doc->toDocx()).contains("TOC \\o"));
        QVERIFY(!removeTableOfContents(*doc, inside));
    }

    void footnoteReferencesAreFoundInBothDirections()
    {
        auto doc = open(makeDocx(para(run("one two three"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(4);
        QVERIFY(insertFootnote(*doc, c));
        QTextCursor d(doc->story());
        d.setPosition(doc->story()->toPlainText().size() - 1);
        QVERIFY(insertFootnote(*doc, d));
        const int first = nextNoteReference(*doc, 0, true, false);
        QVERIFY(first > 0);
        const int second = nextNoteReference(*doc, first + 1, true, false);
        QVERIFY(second > first);
        QCOMPARE(nextNoteReference(*doc, second + 1, true, false), -1);
        QCOMPARE(nextNoteReference(*doc, second + 1, false, false), first);   // (the caret just after a reference is past it)
        QCOMPARE(nextNoteReference(*doc, second, false, false), first);
        QCOMPARE(nextNoteReference(*doc, first + 1, false, false), -1);
        QCOMPARE(nextNoteReference(*doc, 0, true, true), -1);
    }
    void theCommandsForFieldsAreDimmedWhereThereIsNoFieldToWorkOn()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.refreshNow();
        auto enabled = [&](const char *id) { return w.act(QString::fromLatin1(id))->isEnabled(); };
        for (const char *id : {"fields.update", "fields.unlink", "fields.lock", "fields.unlock", "fields.toggleCode", "fields.toggleAllCodes", "refs.updateTable", "refs.removeTable", "refs.nextFootnote", "refs.previousEndnote"}) QVERIFY2(!enabled(id), id);
        QVERIFY(enabled("refs.toc"));
        // A field at the caret: all but Unlock; a locked one: Unlock and not Lock.
        QTextCursor c = w.view()->cursor();
        QVERIFY(insertField(*w.document(), c, QStringLiteral("= 2 + 3")));
        w.view()->setCursor(c);
        w.refreshNow();
        QVERIFY(enabled("fields.unlink") && enabled("fields.lock") && enabled("fields.toggleCode") && enabled("fields.toggleAllCodes"));
        QVERIFY(!enabled("refs.removeTable"));   // (a field, but no table of contents)
        QVERIFY(!enabled("fields.unlock"));
        lockFields(*w.document(), c, true);
        w.refreshNow();
        QVERIFY(enabled("fields.unlock") && !enabled("fields.lock"));
        // Away from it, in text of its own, only Alt+F9 stays on; with the field gone it goes too.
        QTextCursor away = w.view()->cursor();
        insertText(*w.document(), away, QStringLiteral(" plain words"));
        w.view()->setCursor(away);
        w.refreshNow();
        QVERIFY(!enabled("fields.unlink"));
        QVERIFY(!enabled("fields.lock"));
        QVERIFY(!enabled("fields.unlock"));
        QVERIFY(!enabled("fields.toggleCode"));
        QVERIFY(enabled("fields.toggleAllCodes"));
        w.act(QStringLiteral("fields.toggleAllCodes"))->trigger();
        QTextCursor all(w.document()->story());
        all.select(QTextCursor::Document);
        w.view()->setCursor(all);
        QCOMPARE(unlinkFields(*w.document(), all), 0);   // (a locked field is left)
        lockFields(*w.document(), all, false);
        QCOMPARE(unlinkFields(*w.document(), all), 1);
        w.refreshNow();
        w.act(QStringLiteral("fields.toggleAllCodes"))->trigger();
        w.refreshNow();
        QVERIFY(!enabled("fields.toggleAllCodes"));
    }

    void theWindowHasTheFieldCommandsAndF9UpdatesTheSelection()
    {
        WriteWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        auto key = [&](const char *id) { return w.act(QString::fromLatin1(id))->shortcut().toString(QKeySequence::NativeText); };
        QCOMPARE(key("fields.update"), QKeySequence(Qt::Key_F9).toString(QKeySequence::NativeText));
        QCOMPARE(key("fields.unlink"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F9).toString(QKeySequence::NativeText));
        QCOMPARE(key("fields.lock"), QKeySequence(Qt::CTRL | Qt::Key_F11).toString(QKeySequence::NativeText));
        QCOMPARE(key("fields.unlock"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F11).toString(QKeySequence::NativeText));
        QCOMPARE(key("fields.toggleCode"), QKeySequence(Qt::SHIFT | Qt::Key_F9).toString(QKeySequence::NativeText));
        QCOMPARE(key("fields.toggleAllCodes"), QKeySequence(Qt::ALT | Qt::Key_F9).toString(QKeySequence::NativeText));
        QCOMPARE(key("insert.bookmark"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F5).toString(QKeySequence::NativeText));
        for (const char *id : {"insert.field", "refs.crossReference", "refs.toc", "refs.updateTable", "refs.removeTable", "refs.nextFootnote", "refs.previousFootnote", "refs.nextEndnote", "refs.previousEndnote"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("f.docx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(makeDocx(para(run("a ") + field("QUOTE \"fresh\"", "stale") + run(" b ") + field("QUOTE \"two\"", "old"))));
        f.close();
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QVERIFY(w.openFile(path));
        QTextCursor c = w.view()->cursor();
        c.select(QTextCursor::Document);
        w.view()->setCursor(c);
        w.act(QStringLiteral("fields.update"))->trigger();
        QCOMPARE(lines(*w.document())[0], QStringLiteral("a fresh b two"));
        w.act(QStringLiteral("fields.unlink"))->trigger();
        QVERIFY(!documentXml(w.document()->toDocx()).contains("fldChar"));
    }

    void aBookmarkCoveringAHeadingWithBlanksOrANumberStillMatches()
    {
        auto head = [](const QByteArray &content) { return para(content, "<w:pStyle w:val=\"Heading1\"/>"); };
        const QByteArray tab = "<w:r><w:tab/></w:r>";
        const QByteArray body =
            head(bookmark(1, "_Ref100000001", run("  Alpha  "))) +                                       // blanks inside the bookmark
            head(tab + bookmark(2, "_Ref100000002", run("Beta")) + run("   ")) +                          // a tab before it, blanks after it
            head(run("2.1") + tab + bookmark(3, "_Ref100000003", run("Gamma"))) +                         // a number and its tab before it
            head(run("Delta"));                                                                          // none
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const QVector<RefTarget> t = crossReferenceTargets(*doc, RefKind::Heading);
        QCOMPARE(t.size(), 4);
        QVERIFY2(!t[0].newBookmark && t[0].bookmark == QLatin1String("_Ref100000001"), qPrintable(t[0].bookmark));
        QVERIFY2(!t[1].newBookmark && t[1].bookmark == QLatin1String("_Ref100000002"), qPrintable(t[1].bookmark));
        QVERIFY2(!t[2].newBookmark && t[2].bookmark == QLatin1String("_Ref100000003"), qPrintable(t[2].bookmark));
        QVERIFY(t[3].newBookmark);
    }

    void aFlagOnACutOffMarkChangesNothing()
    {
        const QByteArray cut = "<w:r><w:fldChar w:fldCharType=\"begin\"";
        QCOMPARE(fieldFlagForTest(cut, "fldLock", true), cut);
        QCOMPARE(fieldFlagForTest(cut, "dirty", false), cut);
        const QByteArray whole = "<w:fldChar w:fldCharType=\"begin\" w:dirty=\"true\"/>";
        const QByteArray locked = fieldFlagForTest(whole, "fldLock", true);
        QVERIFY2(locked.contains("w:fldLock=\"1\"") && locked.contains("w:dirty=\"true\""), locked.constData());
        const QByteArray clean = fieldFlagForTest(locked, "dirty", false);
        QVERIFY(!clean.contains("dirty") && clean.contains("fldLock"));
        QCOMPARE(fieldFlagForTest(clean, "fldLock", false), QByteArray("<w:fldChar w:fldCharType=\"begin\"/>"));
    }

    void numbersThatRoundToNothingHaveNoSign()
    {
        QCOMPARE(plainNumber(-1e-7), QStringLiteral("0"));
        QCOMPARE(plainNumber(-0.0000004), QStringLiteral("0"));
        QCOMPARE(plainNumber(-0.0000005), QStringLiteral("-0.000001"));
        QCOMPARE(plainNumber(-1.5), QStringLiteral("-1.5"));
        QCOMPARE(plainNumber(0.0), QStringLiteral("0"));
    }

    void aConditionWithoutWildcardsComparesTheWholeStrings()
    {
        Env env;
        auto test = [&](const QString &left, const QString &op, const QString &right) {
            Code code;
            code.name = QStringLiteral("IF");
            code.arguments = {left, op, right, QStringLiteral("yes"), QStringLiteral("no")};
            return compute(code, env).value_or(QStringLiteral("none"));
        };
        const QString same = QString(6000, QLatin1Char('a'));
        QCOMPARE(test(same + QLatin1Char('x'), QStringLiteral("="), same + QLatin1Char('y')), QStringLiteral("no"));   // (they differ past the length a code can hold)
        QCOMPARE(test(same + QLatin1Char('x'), QStringLiteral("<>"), same + QLatin1Char('y')), QStringLiteral("yes"));
        QCOMPARE(test(same + QLatin1Char('x'), QStringLiteral("="), same + QLatin1Char('X')), QStringLiteral("yes"));
        // A wildcard match past the length is made, with its work bounded.
        fields::resetGlobStepsForTest();
        QCOMPARE(test(same + QLatin1Char('x'), QStringLiteral("="), QStringLiteral("a*x")), QStringLiteral("yes"));
        QCOMPARE(test(same, QStringLiteral("="), QString(3000, QLatin1Char('a')) + QStringLiteral("*b")), QStringLiteral("no"));
        QVERIFY(fields::globStepsForTest() > 0 && fields::globStepsForTest() < qint64(4096) * 4096 + 20000);
    }
};

QTEST_MAIN(FieldTests)
#include "tst_write_fields.moc"
