// Tests of JeffWrite's fields (apps/write/core/fieldcode.*, fieldops.*): the code of a field read into its switches, the formats of a result, the formulas and conditions, the results
// of the fields the program works out, and updating, unlinking, and locking the fields of a document. Each document is built here from a few lines of XML.

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
#include "office/package.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QElapsedTimer>
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
        // One Undo puts it all back.
        QVERIFY(doc->undo());
        QCOMPARE(doc->story()->toPlainText(), before);
        QVERIFY(!doc->canUndo());
        // A second update with nothing to change makes no step.
        updateFields(*doc, o);
        QVERIFY(doc->canUndo());
        updateFields(*doc, o);
        const UpdateReport again = updateFields(*doc, o);
        QCOMPARE(again.changed, 0);
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
        QByteArray deep;
        const int n = 300;
        for (int i = 0; i < n; ++i) deep += "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> IF </w:instrText></w:r>";
        for (int i = 0; i < n; ++i) deep += "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("x") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        auto doc = open(makeDocx(para(deep)));
        QVERIFY(doc);
        QElapsedTimer t;
        t.start();
        updateFields(*doc);
        QVERIFY(t.elapsed() < 5000);
        // A reference that holds itself.
        const QByteArray cyc = para(bookmark(1, "Loop", run("a ") + field("REF Loop", "b")));
        auto d2 = open(makeDocx(cyc));
        QVERIFY(d2);
        updateFields(*d2);
        QVERIFY(lines(*d2)[0].size() < 100);
        // Many sequence fields.
        QByteArray many;
        for (int i = 0; i < 3000; ++i) many += field("SEQ Z", "0");
        auto d3 = open(makeDocx(para(many)));
        QVERIFY(d3);
        t.restart();
        updateFields(*d3);
        QVERIFY(t.elapsed() < 10000);
        QVERIFY(lines(*d3)[0].endsWith(QStringLiteral("3000")));
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
    void theWindowHasTheFieldCommandsAndF9UpdatesTheSelection()
    {
        WriteWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        auto key = [&](const char *id) { return w.act(QString::fromLatin1(id))->shortcut().toString(QKeySequence::NativeText); };
        QCOMPARE(key("fields.update"), QKeySequence(Qt::Key_F9).toString(QKeySequence::NativeText));
        QCOMPARE(key("fields.unlink"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F9).toString(QKeySequence::NativeText));
        QCOMPARE(key("fields.lock"), QKeySequence(Qt::CTRL | Qt::Key_F11).toString(QKeySequence::NativeText));
        QCOMPARE(key("fields.unlock"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F11).toString(QKeySequence::NativeText));
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
};

QTEST_MAIN(FieldTests)
#include "tst_write_fields.moc"
