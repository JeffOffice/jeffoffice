// Tests of JeffWrite's Mailings tab: the recipient lists a merge reads (core/mergesource.h), the merge fields and rules (core/mergeops.h), the merge itself, envelopes and labels, and
// the merge settings kept in a .docx. Every file is built here in code.

#include "../withdialog.h"
#include "app/mailpreview.h"
#include "app/pageview.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "app/ribbon.h"
#include "core/appinfo.h"
#include "core/barcode.h"
#include "core/barcodeitem.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/labelops.h"
#include "core/fieldops.h"
#include "core/mergeops.h"
#include "core/mergesource.h"
#include "core/pictureops.h"
#include "core/writedoc.h"
#include "office/package.h"
#include "wml/wml.h"

#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QDockWidget>
#include <QFile>
#include <QMenu>
#include <QRegularExpression>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QTest>

using namespace jp;
using namespace jp::write;

namespace {

QString writeFile(const QTemporaryDir &dir, const QString &name, const QByteArray &bytes)
{
    const QString path = dir.filePath(name);
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) f.write(bytes);
    return path;
}

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
const QString kRel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");

QByteArray run(const QByteArray &text) { return "<w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r>"; }
QByteArray para(const QByteArray &content, const QByteArray &props = QByteArray()) { return "<w:p>" + (props.isEmpty() ? QByteArray() : "<w:pPr>" + props + "</w:pPr>") + content + "</w:p>"; }
QByteArray field(const QByteArray &code, const QByteArray &result)
{
    return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + code + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run(result) +
           "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
}

// A document of these paragraphs; `settings` is the settings part (none if empty) and `source` the target of its data source relationship.
QByteArray makeDocx(const QByteArray &body, const QByteArray &settings = QByteArray(), const QString &source = QString())
{
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs + "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style></w:styles>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    if (!settings.isEmpty()) {
        w.addPart(QStringLiteral("/word/settings.xml"), settings, QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
        if (!source.isEmpty()) w.addRelationshipWithId(QStringLiteral("/word/settings.xml"), QStringLiteral("rId1"), kRel + QStringLiteral("mailMergeSource"), source, true);
    }
    return w.finish();
}

QByteArray partOf(const QByteArray &zip, const QString &name)
{
    office::Package p;
    p.read(zip);
    return p.part(name);
}

bool documentHas(const QByteArray &zip, const QByteArray &what) { return partOf(zip, QStringLiteral("/word/document.xml")).contains(what); }

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

MergeSource people()
{
    MergeSource m;
    m.fields = {"Title", "First Name", "Last Name", "Company", "Address Line 1", "City", "State", "ZIP Code", "Country"};
    auto add = [&](QStringList row) {
        m.rows << row;
        m.include << true;
    };
    add({"Dr.", "Mara", "Fenwick", "", "12 Harbor Way", "London", "", "N1 9GU", "United Kingdom"});
    add({"", "Nora", "Pell", "Birchline", "1 Quarry Ct", "Arlington", "VA", "22201", "United States"});
    add({"", "", "", "", "", "", "", "", ""});
    return m;
}

// A field whose code holds a field: `before`, the inner field's code, `after` (as IF "{ MERGEFIELD State }" = "VA" is stored).
QByteArray nestedField(const QByteArray &before, const QByteArray &innerCode, const QByteArray &after)
{
    return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + before + "</w:instrText></w:r>" + field(innerCode, "x") + "<w:r><w:instrText xml:space=\"preserve\">" + after +
           " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
}

// A spreadsheet file with this sheet XML (the rows of the sheet's sheetData) and nothing else.
QByteArray makeXlsx(const QByteArray &rows)
{
    const QByteArray ns = "xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/xl/workbook.xml"), "<?xml version=\"1.0\"?><workbook " + ns + "><sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/xl/workbook.xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), "<?xml version=\"1.0\"?><worksheet " + ns + "><sheetData>" + rows + "</sheetData></worksheet>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), kRel + QStringLiteral("worksheet"), QStringLiteral("/xl/worksheets/sheet1.xml"));
    return w.finish();
}

// The data of every barcode picture of the text, in order.
QStringList barcodeDataOf(WriteDoc &doc)
{
    QStringList out;
    for (int i = 0; i + 1 < doc.story()->characterCount(); ++i)
        if (isPictureAt(doc.story(), i)) {
            const QJsonObject b = pictureAt(doc, doc.story(), i).barcode;
            if (!b.isEmpty()) out << b.value(QLatin1String("data")).toString();
        }
    return out;
}

// A document with "Code: " and a barcode of the column Name, as the field a template holds.
QByteArray barcodeLetter(const QByteArray &typeAndSwitches)
{
    return makeDocx(para(run("Code: ") + "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> DISPLAYBARCODE \"</w:instrText></w:r>" + field("MERGEFIELD Name", "x") +
                         "<w:r><w:instrText xml:space=\"preserve\">\" " + typeAndSwitches + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("sample") +
                         "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"));
}

MergeSource threePeople()
{
    MergeSource m;
    m.fields = {"Name", "Email"};
    for (const QStringList &r : {QStringList{"Ann", "ann@example.com"}, QStringList{"Bo", "bo@example.com"}, QStringList{"Cy", "cy@example.com"}}) {
        m.rows << r;
        m.include << true;
    }
    return m;
}

bool isShade(const QImage &image, int x, int y)
{
    if (x >= image.width()) return false;
    const QColor c = image.pixelColor(x, y);
    return c.red() == 227 && c.green() == 227 && c.blue() == 227;
}

QString textOf(WriteDoc &doc)
{
    QString t = doc.story()->toPlainText();
    t.replace(QChar::LineSeparator, QLatin1Char('/')).replace(QChar::ParagraphSeparator, QLatin1Char('|')).replace(QLatin1Char('\n'), QLatin1Char('|'));
    return t;
}

} // namespace

class MailTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        jp::initCore();
    }

    void aCsvListHasColumnsRowsAndQuotedCells()
    {
        QTemporaryDir dir;
        const QString path = writeFile(dir, "list.csv", "First Name,Last Name,Notes\r\nAda,Fenwick,\"likes \"\"maths\"\", and poems\"\r\nGrace,Pell\r\n");
        MergeSource m;
        QString error;
        QVERIFY2(loadMergeSource(path, &m, &error), qPrintable(error));
        QCOMPARE(m.fields, (QStringList{"First Name", "Last Name", "Notes"}));
        QCOMPARE(m.rows.size(), 2);
        QCOMPARE(m.value(0, "Notes"), QStringLiteral("likes \"maths\", and poems"));
        QCOMPARE(m.value(1, "Notes"), QString());   // (a short row is padded)
        QCOMPARE(m.includedRows(), (QVector<int>{0, 1}));
    }

    void aTabSeparatedListIsRecognized()
    {
        QTemporaryDir dir;
        const QString path = writeFile(dir, "list.txt", "Name\tCity\nAda\tLondon\n");
        MergeSource m;
        QVERIFY(loadMergeSource(path, &m, nullptr));
        QCOMPARE(m.fields, (QStringList{"Name", "City"}));
        QCOMPARE(m.value(0, "City"), QStringLiteral("London"));
    }

    void anOpenDocumentSpreadsheetIsReadWithItsNumberFormats()
    {
        QTemporaryDir dir;
        const QByteArray fods =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<office:document xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" "
            "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" "
            "xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" "
            "office:version=\"1.2\" office:mimetype=\"application/vnd.oasis.opendocument.spreadsheet\">"
            "<office:automatic-styles><number:number-style style:name=\"N5\"><number:number number:decimal-places=\"0\" number:min-integer-digits=\"5\"/></number:number-style>"
            "<style:style style:name=\"zip\" style:family=\"table-cell\" style:data-style-name=\"N5\"/></office:automatic-styles>"
            "<office:body><office:spreadsheet><table:table table:name=\"Sheet1\">"
            "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>Name</text:p></table:table-cell><table:table-cell office:value-type=\"string\"><text:p>ZIP Code</text:p></table:table-cell></table:table-row>"
            "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>Mara</text:p></table:table-cell><table:table-cell table:style-name=\"zip\" office:value-type=\"float\" office:value=\"2134\"><text:p>02134</text:p></table:table-cell></table:table-row>"
            "</table:table></office:spreadsheet></office:body></office:document>";
        const QString path = writeFile(dir, "list.fods", fods);
        MergeSource m;
        QString error;
        QVERIFY2(loadMergeSource(path, &m, &error), qPrintable(error));
        QCOMPARE(m.fields, (QStringList{"Name", "ZIP Code"}));
        QCOMPARE(m.value(0, "ZIP Code"), QStringLiteral("02134"));
    }

    void anAddressBlockLeavesOutEmptyLinesAndTheCommasRoundThem()
    {
        const MergeSource m = people();
        const QHash<QString, QString> none;
        QCOMPARE(mail::addressBlockText(m, 0, none, QString()), QStringLiteral("Dr. Mara Fenwick\u2028" "12 Harbor Way\u2028" "London N1 9GU\u2028" "United Kingdom"));
        QCOMPARE(mail::addressBlockText(m, 1, none, QString()), QStringLiteral("Nora Pell\u2028" "Birchline\u2028" "1 Quarry Ct\u2028" "Arlington, VA 22201\u2028" "United States"));
        // A country that is the home one is not written.
        QCOMPARE(mail::addressBlockText(m, 1, none, QString(), QStringLiteral("united states")), QStringLiteral("Nora Pell\u2028" "Birchline\u2028" "1 Quarry Ct\u2028" "Arlington, VA 22201"));
        QCOMPARE(mail::addressBlockText(m, 2, none, QString()), QString());
        // Match Fields: a column the usual names don't find.
        MergeSource odd;
        odd.fields = {"Who", "Street"};
        odd.rows << QStringList{"Mara Fenwick", "1 Way"};
        odd.include << true;
        QHash<QString, QString> map;
        map.insert(QStringLiteral("First"), QStringLiteral("Who"));
        QCOMPARE(mail::addressBlockText(odd, 0, map, QString()), QStringLiteral("Mara Fenwick\u2028" "1 Way"));
    }

    void aGreetingLineFallsBackWhereTheRecordHasNoName()
    {
        const MergeSource m = people();
        const QHash<QString, QString> none;
        const QString fallback = QStringLiteral("Dear Sir or Madam,");
        QCOMPARE(mail::greetingText(m, 0, none, QStringLiteral("Dear {Title} {Last},"), fallback), QStringLiteral("Dear Dr. Fenwick,"));
        QCOMPARE(mail::greetingText(m, 1, none, QStringLiteral("Dear {Title} {Last},"), fallback), QStringLiteral("Dear Nora Pell,"));   // (no title: the default format uses the first name)
        QCOMPARE(mail::greetingText(m, 1, none, QStringLiteral("To {Title} {Last}:"), fallback), QStringLiteral("To Pell:"));   // (a format that was chosen is kept)
        QCOMPARE(mail::greetingText(m, 1, none, QStringLiteral("Hello {First}!"), fallback), QStringLiteral("Hello Nora!"));
        QCOMPARE(mail::greetingText(m, 2, none, QStringLiteral("Dear {Title} {Last},"), fallback), fallback);
    }

    void aMergeFieldTakesTextBeforeAndAfterOnlyWhereThereIsAValue()
    {
        const MergeSource m = people();
        const QHash<QString, QString> none;
        const QVector<int> rows{0, 1, 2};
        mail::Evaluator ev(m, none, rows);
        auto answer = [&](const QString &code) { return ev.answer(fields::parseCode(code)).value_or(QStringLiteral("none")); };
        QCOMPARE(answer(QStringLiteral("MERGEFIELD First_Name")), QStringLiteral("Mara"));
        QCOMPARE(answer(QStringLiteral("MERGEFIELD \"First Name\" \\* Upper")), QStringLiteral("MARA"));
        QCOMPARE(answer(QStringLiteral("MERGEFIELD Title \\b \"<\" \\f \"> \"")), QStringLiteral("<Dr.> "));
        QCOMPARE(answer(QStringLiteral("MERGEFIELD Nothing")), QString());
        ev.position = 2;
        QCOMPARE(answer(QStringLiteral("MERGEFIELD Title \\b \"<\" \\f \"> \"")), QString());
        QCOMPARE(answer(QStringLiteral("MERGEREC")), QStringLiteral("3"));
    }

    void aMergeMakesACopyOfTheTextForEachRecord()
    {
        const QByteArray zip = makeDocx(para(run("Dear ") + field("MERGEFIELD First_Name", "\xC2\xAB" "First Name\xC2\xBB") + run(",")) + para(run("Hello again")));
        auto doc = open(zip);
        QVERIFY(doc);
        MergeSource m = people();
        m.include[1] = false;   // (excluded)
        mail::MergeOptions options;
        QString error;
        const QString before = textOf(*doc);
        auto merged = mail::mergeToDocument(*doc, m, {}, options, &error);
        QVERIFY2(merged, qPrintable(error));
        QCOMPARE(textOf(*merged), QStringLiteral("Dear Mara,|Hello again|Dear ,|Hello again"));
        // Each record after the first begins a page; the original is as it was.
        int breaks = 0;
        for (QTextBlock b = merged->story()->begin(); b.isValid(); b = b.next()) breaks += (b.blockFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore) != 0;
        QCOMPARE(breaks, 1);
        QCOMPARE(textOf(*doc), before);
        // The merged document saves and opens again with the same text.
        auto again = open(merged->toDocx());
        QVERIFY(again);
        QCOMPARE(textOf(*again), textOf(*merged));
    }

    // A barcode whose value is a merge field: each record's picture holds its own value, so the three codes differ.
    void aMergeMakesABarcodeForEachRecord()
    {
        auto doc = open(barcodeLetter("CODE128 \\s 100 \\t"));
        QVERIFY(doc);
        mail::MergeOptions options;
        QString error;
        auto merged = mail::mergeToDocument(*doc, threePeople(), {}, options, &error);
        QVERIFY2(merged, qPrintable(error));
        QCOMPARE(barcodeDataOf(*merged), (QStringList{"Ann", "Bo", "Cy"}));
        QCOMPARE(textOf(*merged).count(QStringLiteral("Code: ")), 3);
        QVERIFY(!textOf(*merged).contains(QStringLiteral("sample")));
        const QJsonObject a{{"type", 4}, {"data", "Ann"}}, b{{"type", 4}, {"data", "Bo"}};
        QVERIFY(barcodeLayout(a).bars != barcodeLayout(b).bars);
        // Saved and opened again, the merged document still holds the three barcodes.
        auto again = open(merged->toDocx());
        QVERIFY(again);
        QCOMPARE(barcodeDataOf(*again), (QStringList{"Ann", "Bo", "Cy"}));
        // The text of an email merge has the value in place of the picture.
        const QStringList texts = mail::mergeToTexts(*doc, threePeople(), {}, options, &error);
        QCOMPARE(texts, (QStringList{"Code: Ann", "Code: Bo", "Code: Cy"}));
    }

    // A value the code can't hold gets a clear message in the text, not a bad code; the other records are unharmed.
    void aValueTheCodeCannotHoldShowsAMessage()
    {
        auto doc = open(barcodeLetter("CODE128"));
        QVERIFY(doc);
        MergeSource m;
        m.fields = {"Name"};
        for (const char *v : {"A1", "Z\xC3\xA9", "B2"}) {
            m.rows << QStringList{QString::fromUtf8(v)};
            m.include << true;
        }
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, m, {}, options);
        QVERIFY(merged);
        QCOMPARE(barcodeDataOf(*merged), (QStringList{"A1", "B2"}));
        QVERIFY2(textOf(*merged).contains(QStringLiteral("Code 128 holds")), qPrintable(textOf(*merged)));
        // An empty value makes no barcode and no message.
        m.rows[0] = QStringList{QString()};
        auto empty = mail::mergeToDocument(*doc, m, {}, options);
        QVERIFY(empty);
        QCOMPARE(barcodeDataOf(*empty), (QStringList{"B2"}));
    }

    // Insert Barcode on a merge field: the field is saved in the file as a field, and merges to a code for each record.
    void aBarcodeFieldIsSavedAndMerges()
    {
        auto doc = open(makeDocx(para(run("Code: "))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::EndOfBlock);
        const QJsonObject settings{{"type", int(barcode::Type::Qr)}, {"data", "x"}, {"qrLevel", 2}};
        QVERIFY(mail::insertBarcodeField(*doc, c, QStringLiteral("Name"), settings));
        const QByteArray saved = doc->toDocx();
        QVERIFY(documentHas(saved, "DISPLAYBARCODE"));
        QVERIFY(documentHas(saved, "MERGEFIELD Name"));
        auto reopened = open(saved);
        QVERIFY(reopened);
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*reopened, threePeople(), {}, options);
        QVERIFY(merged);
        QCOMPARE(barcodeDataOf(*merged), (QStringList{"Ann", "Bo", "Cy"}));
        QVERIFY(merged->story()->toPlainText().contains(QStringLiteral("Code: ")));
        // (QR, at the level asked for.)
        for (int i = 0; i + 1 < merged->story()->characterCount(); ++i)
            if (isPictureAt(merged->story(), i)) {
                const QJsonObject b = pictureAt(*merged, merged->story(), i).barcode;
                QCOMPARE(b.value(QLatin1String("type")).toInt(), int(barcode::Type::Qr));
                QCOMPARE(b.value(QLatin1String("qrLevel")).toInt(), 2);
            }
    }

    // The barcode fields of the text are listed with the value their result shows (a sample where it shows none the code can hold), and one is changed to another type or look as one step that undoes;
    // a field as it is made (one character) and as a file has it (marks, with the merge field inside) are both changed; saved, opened, and saved again, it is the same.
    void aBarcodeFieldIsListedChangedAndKept()
    {
        auto doc = open(makeDocx(para(run("Code: "))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::EndOfBlock);
        QVERIFY(mail::insertBarcodeField(*doc, c, QStringLiteral("Name"), QJsonObject{{"type", int(barcode::Type::Qr)}, {"data", "x"}, {"qrLevel", 2}}));
        QVector<mail::BarcodeField> list = mail::barcodeFields(*doc);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].column, QStringLiteral("Name"));
        QCOMPARE(list[0].settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Qr));
        QCOMPARE(list[0].settings.value(QLatin1String("qrLevel")).toInt(), 2);
        QVERIFY(list[0].sample);   // (the field shows its name, not a value)
        QVERIFY(barcodeLayout(list[0].settings).error.isEmpty());
        QVERIFY(list[0].from >= 0 && list[0].to > list[0].from);
        QVERIFY(mail::barcodeFieldAt(*doc, list[0].begin).has_value());
        QVERIFY(!mail::barcodeFieldAt(*doc, 0).has_value());
        // Another type and look.
        const QJsonObject code39{{"type", int(barcode::Type::Code39)}, {"data", "x"}, {"magnification", 150}, {"text", true}};
        QVERIFY(mail::changeBarcodeField(*doc, list[0].begin, code39));
        list = mail::barcodeFields(*doc);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].column, QStringLiteral("Name"));
        QCOMPARE(list[0].settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Code39));
        QCOMPARE(list[0].settings.value(QLatin1String("magnification")).toInt(), 150);
        QVERIFY(list[0].settings.value(QLatin1String("text")).toBool());
        QVERIFY(!mail::changeBarcodeField(*doc, list[0].begin, QJsonObject()));
        QVERIFY(!mail::changeBarcodeField(*doc, 0, code39));
        // One step: Undo brings the QR code back.
        doc->undo();
        list = mail::barcodeFields(*doc);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Qr));
        doc->redo();
        QCOMPARE(mail::barcodeFields(*doc).value(0).settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Code39));
        // Saved, opened, and saved again: the same field, and the same bytes from then on.
        const QByteArray first = doc->toDocx();
        QVERIFY(documentHas(first, "CODE39"));
        auto reopened = open(first);
        QVERIFY(reopened);
        list = mail::barcodeFields(*reopened);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].column, QStringLiteral("Name"));
        QCOMPARE(list[0].settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Code39));
        QCOMPARE(list[0].settings.value(QLatin1String("magnification")).toInt(), 150);
        const QByteArray second = reopened->toDocx();
        auto again = open(second);
        QVERIFY(again);
        QCOMPARE(again->toDocx(), second);
        QCOMPARE(mail::barcodeFields(*again).value(0).settings, list[0].settings);
        // The field as a file has it (marks, with the merge field in its code) is changed too.
        QVERIFY(mail::changeBarcodeField(*reopened, list[0].begin, QJsonObject{{"type", int(barcode::Type::Code128)}, {"data", "x"}}));
        const QVector<mail::BarcodeField> now = mail::barcodeFields(*reopened);
        QCOMPARE(now.size(), 1);
        QCOMPARE(now[0].column, QStringLiteral("Name"));
        QCOMPARE(now[0].settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Code128));
        QVERIFY(documentHas(reopened->toDocx(), "CODE128"));
        QVERIFY(documentHas(reopened->toDocx(), "MERGEFIELD Name"));
    }

    // What a barcode field shows is its result, drawn as a code when the code can hold it; where it can't (or the result is the field's name), a sample of the type stands in.
    void aBarcodeFieldShowsItsResultOrASample()
    {
        auto doc = open(barcodeLetter("CODE128 \\s 100 \\t"));   // (its result is "sample")
        QVERIFY(doc);
        QVector<mail::BarcodeField> list = mail::barcodeFields(*doc);
        QCOMPARE(list.size(), 1);
        QVERIFY(!list[0].sample);
        QCOMPARE(list[0].settings.value(QLatin1String("data")).toString(), QStringLiteral("sample"));
        auto ean = open(barcodeLetter("EAN13 \\s 100"));   // ("sample" is no EAN-13)
        QVERIFY(ean);
        list = mail::barcodeFields(*ean);
        QCOMPARE(list.size(), 1);
        QVERIFY(list[0].sample);
        QVERIFY(barcodeLayout(list[0].settings).error.isEmpty());
        QCOMPARE(list[0].settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Ean13));
    }

    // The editor draws the bars where the field is (not its name), and a double click, or Edit Barcode in the right-click menu, opens the barcode dialog to change it.
    void theEditorDrawsABarcodeFieldAndEditsItAgain()
    {
        auto doc = open(barcodeLetter("CODE128 \\s 100"));
        QVERIFY(doc);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        w.setDocument(std::move(doc), QString());
        const int begin = mail::barcodeFields(*w.document()).value(0).begin;
        QVERIFY(begin >= 0);
        QRect box;
        QTRY_VERIFY(w.view()->barcodeFieldRect(begin, &box));
        QVERIFY(box.width() > 20 && box.height() > 5);
        QCOMPARE(w.view()->barcodeFieldAtPoint(box.center()), begin);
        QCOMPARE(w.view()->barcodeFieldAtPoint(QPoint(2, 2)), -1);
        // Bars: the same pattern of dark and light at two heights of the box (the bars run its height; lettering would not repeat), changing across it many times. The box is in the window's
        // units; the picture of it is in device pixels.
        auto bars = [&](int *changes, int *matching, int *total) {
            const QImage image = w.view()->viewport()->grab().toImage();
            const double r = image.devicePixelRatio() > 0 ? w.view()->viewport()->devicePixelRatioF() : 1.0;
            const QRect b(QPointF(box.left() * r, box.top() * r).toPoint(), QSizeF(box.width() * r, box.height() * r).toSize());
            const int y1 = b.top() + b.height() / 4, y2 = b.top() + b.height() * 3 / 4;
            *changes = *matching = *total = 0;
            int last = -1;
            for (int x = b.left() + 2; x < b.right() - 2; ++x) {
                const bool d1 = qGray(image.pixel(x, y1)) < 128, d2 = qGray(image.pixel(x, y2)) < 128;
                ++*total;
                *matching += d1 == d2;
                if (last >= 0 && int(d1) != last) ++*changes;
                last = d1;
            }
        };
        int changes = 0, matching = 0, total = 0;
        QTRY_VERIFY((bars(&changes, &matching, &total), total > 10 && changes >= 10 && matching * 100 >= total * 95));
        // A double click opens the barcode dialog on it: the type there is changed and the field has it.
        auto changeType = [&](barcode::Type type) {
            return [type](QDialog *d) {
                auto *types = d->findChild<QComboBox *>();
                QVERIFY(types);
                types->setCurrentIndex(int(type));
            };
        };
        withDialog([&] { QTest::mouseDClick(w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, box.center()); }, QStringLiteral("barcodeDialog"), changeType(barcode::Type::Code39));
        QCOMPARE(mail::barcodeFields(*w.document()).value(0).settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Code39));
        QVERIFY(documentHas(w.document()->toDocx(), "CODE39"));
        // Cancelling changes nothing.
        QTRY_VERIFY(w.view()->barcodeFieldRect(mail::barcodeFields(*w.document()).value(0).begin, &box));
        withDialog([&] { QTest::mouseDClick(w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, box.center()); }, QStringLiteral("barcodeDialog"), changeType(barcode::Type::Qr), false);
        QCOMPARE(mail::barcodeFields(*w.document()).value(0).settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Code39));
        // The right-click menu on the field has Edit Barcode.
        QTextCursor in(w.document()->story());
        in.setPosition(mail::barcodeFields(*w.document()).value(0).from);
        w.view()->setCursor(in);
        w.refreshNow();
        QMenu menu;
        w.fillContextMenu(menu);
        QAction *edit = nullptr;
        for (QAction *a : menu.actions())
            if (a->objectName() == QLatin1String("ctx.editBarcode")) edit = a;
        QVERIFY(edit);
        QCOMPARE(edit->text(), QStringLiteral("Edit Barcode\u2026"));
        withDialog([&] { edit->trigger(); }, QStringLiteral("barcodeDialog"), changeType(barcode::Type::Qr));
        QCOMPARE(mail::barcodeFields(*w.document()).value(0).settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Qr));
    }

    // A value of megabytes is not hashed or encoded: it is shown as the sample, in a short time, for every type.
    void aHugeBarcodeFieldValueIsNotEncoded()
    {
        const QByteArray huge(3 * 1024 * 1024, 'A');
        for (const char *type : {"CODE128", "QR", "EAN13", "CODE39"}) {
            auto doc = open(makeDocx(para(run("x") + "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> DISPLAYBARCODE \"</w:instrText></w:r>" +
                                            field("MERGEFIELD Name", "x") + "<w:r><w:instrText xml:space=\"preserve\">\" " + type + " \\s 100 </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" +
                                            run(huge) + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>")));
            QVERIFY(doc);
            QElapsedTimer t;
            t.start();
            const QVector<mail::BarcodeField> list = mail::barcodeFields(*doc);
            QVERIFY2(t.elapsed() < 250, type);
            QCOMPARE(list.size(), 1);
            QVERIFY(list[0].sample);
            QVERIFY(list[0].settings.value(QLatin1String("data")).toString().size() < 100);
        }
    }

    // Other switches and words of the field stay when its type and look are changed; a field with no merge field in it is listed but is not editable.
    void changingABarcodeFieldKeepsWhatTheDialogDoesNotOwn()
    {
        auto doc = open(barcodeLetter("CODE128 \\s 100 \\b 5 \\d"));
        QVERIFY(doc);
        QVector<mail::BarcodeField> list = mail::barcodeFields(*doc);
        QCOMPARE(list.size(), 1);
        QVERIFY(list[0].editable);
        QVERIFY(mail::changeBarcodeField(*doc, list[0].begin, QJsonObject{{"type", int(barcode::Type::Qr)}, {"data", "x"}}));
        const QString code = fieldCodeAt(*doc, mail::barcodeFields(*doc).value(0).begin);
        QVERIFY2(code.contains(QStringLiteral("\\b 5 \\d")), qPrintable(code));
        QVERIFY(code.contains(QStringLiteral("QR")));
        QVERIFY(!code.contains(QStringLiteral("CODE128")));
        auto fixed = open(makeDocx(para(field("DISPLAYBARCODE \"12345\" CODE128", "12345"))));
        QVERIFY(fixed);
        list = mail::barcodeFields(*fixed);
        QCOMPARE(list.size(), 1);
        QVERIFY(!list[0].editable);
        QVERIFY(!mail::changeBarcodeField(*fixed, list[0].begin, QJsonObject{{"type", int(barcode::Type::Qr)}, {"data", "x"}}));
    }

    // A field kept as one w:fldSimple element is listed, changed, saved, and opened again.
    void aSimpleBarcodeFieldIsChangedAndKept()
    {
        auto doc = open(makeDocx(para(run("Code: ") + "<w:fldSimple w:instr=\" DISPLAYBARCODE &quot;MERGEFIELD Name&quot; CODE128 \\s 100 \"><w:r><w:t>sample</w:t></w:r></w:fldSimple>")));
        QVERIFY(doc);
        QVector<mail::BarcodeField> list = mail::barcodeFields(*doc);
        QCOMPARE(list.size(), 1);
        QVERIFY(list[0].editable);
        QCOMPARE(list[0].column, QStringLiteral("Name"));
        QVERIFY(mail::changeBarcodeField(*doc, list[0].begin, QJsonObject{{"type", int(barcode::Type::Qr)}, {"data", "x"}}));
        list = mail::barcodeFields(*doc);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Qr));
        QVERIFY(doc->story()->toPlainText().contains(QStringLiteral("Code: ")));
        const QByteArray first = doc->toDocx();
        auto reopened = open(first);
        QVERIFY(reopened);
        list = mail::barcodeFields(*reopened);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].column, QStringLiteral("Name"));
        QCOMPARE(list[0].settings.value(QLatin1String("type")).toInt(), int(barcode::Type::Qr));
        const QByteArray second = reopened->toDocx();
        QCOMPARE(open(second)->toDocx(), second);
    }

    // The fields are found again only when an edit touches one; the bars of a field are made once however often the page is drawn; and only the fields of the page being drawn are measured.
    void barcodeFieldsAreNotFoundOrEncodedAgainForEveryEdit()
    {
        QByteArray body;
        for (int i = 0; i < 40; ++i) {
            QByteArray line = run("Row " + QByteArray::number(i) + " ");
            for (int k = 0; k < 6; ++k) line += field("DISPLAYBARCODE \"MERGEFIELD Name\" CODE128 \\s 100", "v" + QByteArray::number(i * 10 + k)) + run(" ");
            body += para(line);
        }
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 900);
        w.show();
        w.setDocument(std::move(doc), QString());
        QRect first;
        QTRY_VERIFY(w.view()->barcodeFieldRect(mail::barcodeFields(*w.document()).value(0).begin, &first));
        auto paint = [&] { w.view()->viewport()->update(); return w.view()->viewport()->grab().toImage(); };
        paint();
        paint();
        const int scans = PageView::barcodeScansForTest(), encodes = barcodeEncodes();
        // The page is drawn again: nothing is encoded again, though more fields are drawn than the encoder's own cache holds.
        paint();
        QCOMPARE(barcodeEncodes(), encodes);
        QCOMPARE(PageView::barcodeScansForTest(), scans);
        // Typing in a line that has no field: no new search for the fields, none encoded again, and the fields after it are where they are.
        QTextCursor c(w.document()->story());
        c.setPosition(2);
        c.insertText(QStringLiteral("xyz"));
        QRect box;
        paint();
        QCOMPARE(PageView::barcodeScansForTest(), scans);
        QCOMPARE(barcodeEncodes(), encodes);
        const QVector<mail::BarcodeField> now = mail::barcodeFields(*w.document());
        QVERIFY(w.view()->barcodeFieldRect(now.value(5).begin, &box));
        // Typing in a field's result finds them again.
        QTextCursor in(w.document()->story());
        in.setPosition(now[0].from);
        in.insertText(QStringLiteral("9"));
        paint();
        QVERIFY(PageView::barcodeScansForTest() > scans);
        // Only the fields of the page drawn are measured (the pages have 40 lines of six: far fewer than all of them are on the first).
        const int boxes = PageView::barcodeBoxesForTest();
        paint();
        QVERIFY2(PageView::barcodeBoxesForTest() - boxes <= 240, qPrintable(QString::number(PageView::barcodeBoxesForTest() - boxes)));
    }

    // A barcode field as the program puts it in (one character) is drawn as bars too.
    void aBarcodeFieldJustPutInIsDrawn()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        QTextCursor c = w.view()->cursor();
        QVERIFY(mail::insertBarcodeField(*w.document(), c, QStringLiteral("Name"), QJsonObject{{"type", int(barcode::Type::Qr)}, {"data", "x"}}));
        w.refreshNow();
        QRect box;
        QTRY_VERIFY(w.view()->barcodeFieldRect(mail::barcodeFields(*w.document()).value(0).begin, &box));
        QVERIFY(box.width() > 5 && box.height() > 5);
        const QImage image = w.view()->viewport()->grab().toImage();
        int dark = 0;
        for (int y = box.top() + 1; y < box.bottom(); ++y)
            for (int x = box.left() + 1; x < box.right(); ++x) dark += qGray(image.pixel(x, y)) < 60;
        QVERIFY(dark > 20);
    }

    // A merge makes at most so many barcodes (the limit set here is smaller than the real one, kMostBarcodes), and a value that fails the layout check counts toward it as well.
    void aMergeMakesAtMostSoManyBarcodes()
    {
        auto doc = open(barcodeLetter("CODE128"));
        QVERIFY(doc);
        mail::MergeOptions options;
        options.mostBarcodes = 2;
        QString error;
        QVERIFY(!mail::mergeToDocument(*doc, threePeople(), {}, options, &error));
        QVERIFY2(error.contains(QStringLiteral("barcodes")), qPrintable(error));
        // Three values the code can't hold are counted too.
        MergeSource bad;
        bad.fields = {"Name"};
        for (int i = 0; i < 3; ++i) {
            bad.rows << QStringList{QString::fromUtf8("Z\xC3\xA9")};
            bad.include << true;
        }
        error.clear();
        QVERIFY(!mail::mergeToDocument(*doc, bad, {}, options, &error));
        QVERIFY2(error.contains(QStringLiteral("barcodes")), qPrintable(error));
        QCOMPARE(mail::kMostBarcodes, 2000);
    }

    // A new Code 128 or Code 39 barcode holds at most mostChars() characters; a barcode a file holds is read at any length the encoder draws.
    void aNewBarcodeIsHeldToItsLengthAndAnOldOneIsNot()
    {
        barcode::Options o;
        o.type = barcode::Type::Code128;
        o.data = QString(barcode::mostChars(barcode::Type::Code128), QLatin1Char('A'));
        QVERIFY(barcode::tooLong(o).isEmpty());
        o.data += QLatin1Char('A');
        QVERIFY(!barcode::tooLong(o).isEmpty());
        o.type = barcode::Type::Code39;
        QVERIFY(!barcode::tooLong(o).isEmpty());
        o.type = barcode::Type::Qr;
        QVERIFY(barcode::tooLong(o).isEmpty());
        // (Making the code is not refused: a longer one that was saved before the limit still draws.)
        o.type = barcode::Type::Code128;
        QVERIFY(barcode::make(o).error.isEmpty());
        // A merge refuses it with a message, like any value that can't be held.
        auto doc = open(barcodeLetter("CODE128"));
        QVERIFY(doc);
        MergeSource m;
        m.fields = {"Name"};
        m.rows << QStringList{QString(100000, QLatin1Char('A'))};
        m.include << true;
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, m, {}, options);
        QVERIFY(merged);
        QVERIFY(barcodeDataOf(*merged).isEmpty());
        const QString text = textOf(*merged);
        QVERIFY2(text.contains(QStringLiteral("[Barcode:")) && text.size() < 300, qPrintable(text.left(300)));
    }

    // What the field syntax says is what is made: height (\h), size (\s), QR level (\q), and the digits (\t) go into the field and come back out of it; ISBN is written as EAN13.
    void theLookOfAFieldIsKeptInItsSwitches()
    {
        auto doc = open(makeDocx(para(run("Code: "))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::EndOfBlock);
        const QJsonObject settings{{"type", int(barcode::Type::Code128)}, {"data", "x"}, {"magnification", 150}, {"height", 60}, {"text", false}};
        QVERIFY(mail::insertBarcodeField(*doc, c, QStringLiteral("Name"), settings));
        QTextCursor d(doc->story());
        d.movePosition(QTextCursor::End);
        const QJsonObject isbn{{"type", int(barcode::Type::Isbn)}, {"data", "x"}, {"priceMode", 0}};
        QVERIFY(mail::insertBarcodeField(*doc, d, QStringLiteral("Isbn"), isbn));
        const QByteArray saved = doc->toDocx();
        const QByteArray xml = partOf(saved, QStringLiteral("/word/document.xml"));
        QVERIFY2(QRegularExpression(QStringLiteral("CODE128 \\\\s 150 \\\\h \\d+")).match(QString::fromUtf8(xml)).hasMatch(), xml.constData());
        QVERIFY(xml.contains("EAN13"));
        QVERIFY(!xml.contains("ISBN"));
        QVERIFY(!xml.contains("\\k"));
        auto reopened = open(saved);
        QVERIFY(reopened);
        MergeSource m;
        m.fields = {"Name", "Isbn"};
        m.rows << QStringList{"A1", "978-1-64002-163-1"};
        m.include << true;
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*reopened, m, {}, options);
        QVERIFY(merged);
        QJsonObject first, second;
        for (int i = 0; i + 1 < merged->story()->characterCount(); ++i)
            if (isPictureAt(merged->story(), i)) {
                const QJsonObject b = pictureAt(*merged, merged->story(), i).barcode;
                (first.isEmpty() ? first : second) = b;
            }
        QCOMPARE(first.value(QLatin1String("data")).toString(), QStringLiteral("A1"));
        QCOMPARE(first.value(QLatin1String("height")).toInt(), 60);
        QCOMPARE(first.value(QLatin1String("magnification")).toInt(), 150);
        QCOMPARE(first.value(QLatin1String("text")).toBool(true), false);
        QCOMPARE(second.value(QLatin1String("type")).toInt(), int(barcode::Type::Ean13));
        QCOMPARE(second.value(QLatin1String("data")).toString(), QStringLiteral("978-1-64002-163-1"));
    }

    // QR error correction is written as 0 to 3 and read as 0 to 3 or as L, M, Q, and H.
    void theQrLevelIsANumberAndReadsAsLetters()
    {
        auto doc = open(makeDocx(para(run("Code: "))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::EndOfBlock);
        QVERIFY(mail::insertBarcodeField(*doc, c, QStringLiteral("Name"), QJsonObject{{"type", int(barcode::Type::Qr)}, {"data", "x"}, {"qrLevel", 3}}));
        QVERIFY(QString::fromUtf8(partOf(doc->toDocx(), QStringLiteral("/word/document.xml"))).contains(QStringLiteral("\\q 3")));
        const QStringList words = {"0", "1", "2", "3", "L", "m", "Q", "h"};
        const QList<int> levels = {0, 1, 2, 3, 0, 1, 2, 3};
        for (int i = 0; i < words.size(); ++i) {
            auto d = open(barcodeLetter("QR \\q " + words[i].toUtf8()));
            QVERIFY(d);
            mail::MergeOptions options;
            auto merged = mail::mergeToDocument(*d, threePeople(), {}, options);
            QVERIFY(merged);
            for (int p = 0; p + 1 < merged->story()->characterCount(); ++p)
                if (isPictureAt(merged->story(), p)) {
                    QCOMPARE(pictureAt(*merged, merged->story(), p).barcode.value(QLatin1String("qrLevel")).toInt(), levels[i]);
                    break;
                }
        }
    }

    // A value is data, never field code: quotes and switches inside it change nothing, and a value that holds the mark of a barcode (or another field's code) is only text.
    void aValueIsNeverReadAsFieldCode()
    {
        auto doc = open(barcodeLetter("CODE128"));
        QVERIFY(doc);
        const QString hostile = QStringLiteral("x\" QR \\s 300 \"y");
        MergeSource m;
        m.fields = {"Name"};
        m.rows << QStringList{hostile} << QStringList{QStringLiteral("a\\q 3 \\h 1")} << QStringList{QStringLiteral("{ DISPLAYBARCODE \"z\" QR }")};
        for (int i = 0; i < 3; ++i) m.include << true;
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, m, {}, options);
        QVERIFY(merged);
        QCOMPARE(barcodeDataOf(*merged), (QStringList{hostile, QStringLiteral("a\\q 3 \\h 1"), QStringLiteral("{ DISPLAYBARCODE \"z\" QR }")}));
        for (int i = 0; i + 1 < merged->story()->characterCount(); ++i)
            if (isPictureAt(merged->story(), i)) {
                const QJsonObject b = pictureAt(*merged, merged->story(), i).barcode;
                QCOMPARE(b.value(QLatin1String("type")).toInt(), int(barcode::Type::Code128));
                QCOMPARE(b.value(QLatin1String("magnification")).toInt(), 100);
                QCOMPARE(b.value(QLatin1String("height")).toInt(), 100);
            }
    }

    // A merge field's value with the characters that mark a barcode in the merge's own text stays text, and does not take a barcode's place.
    void aValueCannotStandInForABarcodeMark()
    {
        const QByteArray body = para(field("MERGEFIELD Note", "x")) +
                                para("<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> DISPLAYBARCODE \"</w:instrText></w:r>" + field("MERGEFIELD Name", "x") +
                                     "<w:r><w:instrText xml:space=\"preserve\">\" CODE128 </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("sample") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const QString forged = QString(QChar(0xE000)) + QStringLiteral("0") + QChar(0xE001);
        MergeSource m;
        m.fields = {"Note", "Name"};
        m.rows << QStringList{forged, "Real"};
        m.include << true;
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, m, {}, options);
        QVERIFY(merged);
        QCOMPARE(barcodeDataOf(*merged), (QStringList{"Real"}));
        QVERIFY(merged->story()->toPlainText().contains(QStringLiteral("0")));
    }

    // A field the merge has nothing to make of stays as it is, with its result: one with a value written out (not from the list), one with no value, and one of a type that is not drawn here.
    void aBarcodeFieldWithNothingToMergeIsLeftAlone()
    {
        for (const char *code : {"DISPLAYBARCODE \"12345\" CODE128", "DISPLAYBARCODE CODE128", "DISPLAYBARCODE"}) {
            const QByteArray zip = makeDocx(para(run("Code: ") + field(code, "sample")));
            auto doc = open(zip);
            QVERIFY(doc);
            mail::MergeOptions options;
            auto merged = mail::mergeToDocument(*doc, threePeople(), {}, options);
            QVERIFY(merged);
            QVERIFY2(barcodeDataOf(*merged).isEmpty(), code);
            QCOMPARE(textOf(*merged), QStringLiteral("Code: sample|Code: sample|Code: sample"));
        }
        for (const char *type : {"UPCE", "ITF14", "JPPOST", "NOSUCH"}) {
            auto doc = open(barcodeLetter(QByteArray(type) + " \\r 90 \\b 255 \\f 0"));
            QVERIFY(doc);
            mail::MergeOptions options;
            auto merged = mail::mergeToDocument(*doc, threePeople(), {}, options);
            QVERIFY(merged);
            QVERIFY2(barcodeDataOf(*merged).isEmpty(), type);
            QCOMPARE(textOf(*merged), QStringLiteral("Code: sample|Code: sample|Code: sample"));
        }
        // The switches for rotation and colors are read past: the barcode is made as the others say.
        auto doc = open(barcodeLetter("CODE128 \\r 90 \\b 255 \\f 0 \\s 120"));
        QVERIFY(doc);
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, threePeople(), {}, options);
        QVERIFY(merged);
        QCOMPARE(barcodeDataOf(*merged), (QStringList{"Ann", "Bo", "Cy"}));
    }

    // A barcode field inside another's code is no harm: it keeps its result, which is the outer one's value.
    void aBarcodeFieldInsideABarcodeFieldMakesOneBarcode()
    {
        const QByteArray inner = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> DISPLAYBARCODE \"</w:instrText></w:r>" + field("MERGEFIELD Name", "x") +
                                 "<w:r><w:instrText xml:space=\"preserve\">\" QR </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("in") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        const QByteArray outer = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> DISPLAYBARCODE \"</w:instrText></w:r>" + inner +
                                 "<w:r><w:instrText xml:space=\"preserve\">\" CODE128 </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("out") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        auto doc = open(makeDocx(para(outer)));
        QVERIFY(doc);
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, threePeople(), {}, options);
        QVERIFY(merged);
        QCOMPARE(barcodeDataOf(*merged), (QStringList{"in", "in", "in"}));
    }

    void aMergeKeepsTheLookOfEachParagraphOfEveryCopy()
    {
        const QByteArray zip = makeDocx(para(field("MERGEFIELD First_Name", "x"), "<w:jc w:val=\"center\"/><w:spacing w:before=\"240\"/>") +
                                        para(run("&lt;b&gt;") + run("plain"), "<w:ind w:left=\"720\"/>"));
        auto doc = open(zip);
        QVERIFY(doc);
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, people(), {}, options);
        QVERIFY(merged);
        int centered = 0, indented = 0, blocks = 0;
        for (QTextBlock b = merged->story()->begin(); b.isValid(); b = b.next(), ++blocks) {
            centered += b.blockFormat().alignment().testFlag(Qt::AlignHCenter) ? 1 : 0;
            indented += b.blockFormat().leftMargin() > 30 ? 1 : 0;
        }
        QCOMPARE(blocks, 6);
        QCOMPARE(centered, 3);   // (the first paragraph of each copy, not only of the first)
        QCOMPARE(indented, 3);
    }

    void rulesChooseWhatEachRecordShows()
    {
        const QByteArray body = para(field("SKIPIF \"{ MERGEFIELD Last_Name }\" = \"\"", "") + field("IF \"{ MERGEFIELD State }\" = \"VA\" \"Virginian \" \"other \"", "x") + field("MERGEFIELD Last_Name", "x") +
                                run(" #") + field("MERGESEQ", "0"));
        // (A field in the code of another is a nested field; built here as the file has it.)
        QByteArray nested = "<w:p><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> SKIPIF \"</w:instrText></w:r>" + field("MERGEFIELD Last_Name", "x") +
                            "<w:r><w:instrText xml:space=\"preserve\">\" = \"\" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>" +
                            "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> IF \"</w:instrText></w:r>" + field("MERGEFIELD State", "x") +
                            "<w:r><w:instrText xml:space=\"preserve\">\" = \"VA\" \"Virginian \" \"other \" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("x") +
                            "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>" + field("MERGEFIELD Last_Name", "x") + run(" #") + field("MERGESEQ", "0") + "</w:p>";
        Q_UNUSED(body)
        auto doc = open(makeDocx(nested));
        QVERIFY(doc);
        mail::MergeOptions options;
        QString error;
        auto merged = mail::mergeToDocument(*doc, people(), {}, options, &error);
        QVERIFY2(merged, qPrintable(error));
        // (The third record has no last name: it is skipped.)
        QCOMPARE(textOf(*merged), QStringLiteral("other Fenwick #1|Virginian Pell #2"));
    }

    void nextRecordTakesTheNextRecordIntoTheSameText()
    {
        const QByteArray body = para(field("MERGEFIELD First_Name", "x") + run(" and ") + field("NEXT", "") + field("MERGEFIELD First_Name", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, people(), {}, options);
        QVERIFY(merged);
        QCOMPARE(textOf(*merged), QStringLiteral("Mara and Nora| and "));   // (the third record then stands alone, with no one after it)
    }

    void anAddressBlockFieldFromTheProgramMergesAndKeepsItsLineBreaks()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        QVERIFY(insertField(*doc, c, mail::addressBlockCode(QString()), QStringLiteral("\u00ABAddress Block\u00BB")));
        c.insertBlock();
        QVERIFY(insertField(*doc, c, mail::greetingCode(QString(), QString()), QStringLiteral("\u00ABGreeting Line\u00BB")));
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, people(), {}, options);
        QVERIFY(merged);
        QVERIFY(textOf(*merged).startsWith(QStringLiteral("Dr. Mara Fenwick|12 Harbor Way|London N1 9GU|United Kingdom|Dear Dr. Fenwick,|Nora Pell|Birchline|")));
        QCOMPARE(merged->story()->blockCount(), 3 * 2 + 0);   // (the address and the greeting are a paragraph each: line breaks, not paragraphs, inside the address)
        auto again = open(merged->toDocx());
        QVERIFY(again);
        QCOMPARE(textOf(*again), textOf(*merged));
        QCOMPARE(again->story()->blockCount(), merged->story()->blockCount());   // (the lines of an address are breaks inside a paragraph, in the file too)
        QVERIFY(documentHas(merged->toDocx(), "<w:br/>"));
        // Nothing of the merge is left in it.
        QVERIFY(!merged->toDocx().isEmpty());
        QVERIFY(!documentHas(merged->toDocx(), "ADDRESSBLOCK"));
    }

    void theMergeOfAFileIsKeptWhenItIsSavedAgain()
    {
        const QByteArray ns = kNs;
        const QByteArray settings = "<?xml version=\"1.0\"?><w:settings " + ns + "><w:zoom w:percent=\"100\"/><w:mailMerge><w:mainDocumentType w:val=\"formLetters\"/><w:linkToQuery/><w:dataType w:val=\"textFile\"/>"
                                    "<w:connectString w:val=\"\"/><w:query w:val=\"SELECT * FROM /data/list.csv\"/><w:dataSource r:id=\"rId1\"/><w:viewMergedData/></w:mailMerge></w:settings>";
        const QByteArray zip = makeDocx(para(run("x")), settings, QStringLiteral("file:///data/list.csv"));
        auto doc = open(zip);
        QVERIFY(doc);
        mail::State *state = mail::stateIfAny(*doc);
        QVERIFY(state);
        QCOMPARE(state->kind(), mail::Kind::Letters);
        QCOMPARE(state->sourcePath, QStringLiteral("/data/list.csv"));
        // Saved without a change, the settings are as the file had them.
        QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/settings.xml")), settings);
        // A change is written, with the link to the list as an external relationship of the settings part.
        state->settings.activeRecord = 2;
        state->sourcePath = QStringLiteral("/other/people.csv");
        state->changed = true;
        const QByteArray saved = doc->toDocx();
        office::wml::MailMergeSettings back;
        QVERIFY(office::wml::readMailMerge(partOf(saved, QStringLiteral("/word/settings.xml")), &back));
        QCOMPARE(back.activeRecord, 2);
        QCOMPARE(back.type, QStringLiteral("formLetters"));
        office::Package package;
        QVERIFY(package.read(saved));
        const QVector<office::Relationship> links = package.relationshipsOfType(QStringLiteral("/word/settings.xml"), QStringLiteral("mailMergeSource"));
        QCOMPARE(links.size(), 1);
        QVERIFY(links[0].external);
        QCOMPARE(links[0].target, QStringLiteral("file:///other/people.csv"));
        QCOMPARE(links[0].id, back.dataSourceId);
        auto reopened = open(saved);
        QVERIFY(reopened);
        QVERIFY(mail::stateIfAny(*reopened));
        QCOMPARE(mail::stateIfAny(*reopened)->sourcePath, QStringLiteral("/other/people.csv"));
        QCOMPARE(mail::stateIfAny(*reopened)->settings.activeRecord, 2);
    }

    void aDocumentWithNoMergeGetsOneOnlyWhenOneIsMade()
    {
        auto doc = WriteDoc::blank();
        QVERIFY(!mail::stateIfAny(*doc));
        QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/settings.xml")).contains("mailMerge"));
        mail::State &state = mail::stateOf(*doc);
        state.settings.type = mail::kindValue(mail::Kind::Labels);
        state.changed = true;
        const QByteArray settings = partOf(doc->toDocx(), QStringLiteral("/word/settings.xml"));
        office::wml::MailMergeSettings back;
        QVERIFY(office::wml::readMailMerge(settings, &back));
        QCOMPARE(back.type, QStringLiteral("mailingLabels"));
    }

    void aSheetOfLabelsIsATableWithACellForEachLabel()
    {
        const QVector<mail::LabelSheet> sheets = mail::labelSheets();
        QVERIFY(sheets.size() >= 8);
        // The names say the count and the size, and nothing else.
        for (const mail::LabelSheet &s : sheets) QVERIFY2(QRegularExpression(QStringLiteral("^\\d+ per sheet, [0-9 /.]+ x [0-9 /.]+ (in|mm)(, A4)?$")).match(s.name).hasMatch(), qPrintable(s.name));
        QCOMPARE(sheets.first().perPage(), 30);
        QVERIFY(sheets.first().name.contains(QStringLiteral("1 x 2 5/8 in")));
        auto doc = mail::labelDocument(sheets.first(), [](int i) { return mail::tightParagraph(mail::textRuns(QStringLiteral("Label %1").arg(i + 1))); });
        QVERIFY(doc);
        const QByteArray saved = doc->toDocx();
        const QByteArray xml = partOf(saved, QStringLiteral("/word/document.xml"));
        QCOMPARE(xml.count("<w:tbl>"), 1);
        QCOMPARE(xml.count("<w:tr>") + xml.count("<w:tr "), 10);
        QCOMPARE(xml.count("<w:tc>"), 10 * (3 + 2));   // (a label's cell, and a narrow one between labels)
        QVERIFY(xml.contains("w:w=\"12240\"") && xml.contains("w:h=\"15840\""));
        QVERIFY(textOf(*doc).contains(QStringLiteral("Label 1|")) && textOf(*doc).contains(QStringLiteral("Label 30")));
        // Each sheet of the list is one page, even one that fills the paper.
        for (const mail::LabelSheet &sheet : sheets) {
            auto page = mail::labelDocument(sheet, [](int) { return mail::tightParagraph(mail::textRuns(QStringLiteral("x"))); });
            QVERIFY2(page, qPrintable(sheet.name));
            QVERIFY2(page->pager().pageCount() == 1, qPrintable(sheet.name));
        }
        auto again = open(saved);
        QVERIFY(again);
        QCOMPARE(textOf(*again), textOf(*doc));
    }

    void aLabelMergeFillsEachLabelWithTheNextRecordAndStartsAnotherPage()
    {
        mail::LabelSheet small = mail::labelSheets().first();
        small.columns = 2;
        small.rows = 2;
        auto doc = mail::labelDocument(small, [](int i) {
            QByteArray runs = mail::fieldRuns(mail::mergeFieldCode(QStringLiteral("First Name")), QStringLiteral("\u00ABFirst Name\u00BB"));
            if (i > 0) runs = mail::fieldRuns(QStringLiteral("NEXT"), QString()) + runs;
            return mail::tightParagraph(runs);
        });
        QVERIFY(doc);
        MergeSource m = people();
        m.fields = {"First Name"};
        m.rows = {{"A"}, {"B"}, {"C"}, {"D"}, {"E"}};
        m.include = QVector<bool>(5, true);
        mail::MergeOptions options;
        QString error;
        auto merged = mail::mergeToDocument(*doc, m, {}, options, &error);
        QVERIFY2(merged, qPrintable(error));
        const QString text = textOf(*merged);
        // The first page holds A to D; the second begins with E, and its other labels are empty.
        QVERIFY2(text.indexOf(QLatin1Char('A')) < text.indexOf(QLatin1Char('B')) && text.indexOf(QLatin1Char('B')) < text.indexOf(QLatin1Char('C')) && text.indexOf(QLatin1Char('D')) < text.indexOf(QLatin1Char('E')),
                 qPrintable(text));
        QCOMPARE(merged->pager().pageCount(), 2);   // (a second page with E; no page for the paragraph that follows a table)
        auto again = open(merged->toDocx());
        QVERIFY(again);
        QCOMPARE(again->pager().pageCount(), merged->pager().pageCount());
    }

    void anEnvelopeAddedToADocumentIsItsFirstPageWithItsOwnSize()
    {
        auto doc = open(makeDocx(para(run("Dear Nora,")) + para(run("Thank you."))));
        QVERIFY(doc);
        QCOMPARE(doc->sectionCount(), 1);
        auto env = mail::envelopeDocument(mail::envelopeSizes().first(), QStringLiteral("Mara"), mail::textRuns(QStringLiteral("Nora Pell")));
        QVERIFY(env);
        QVERIFY(mail::addAsFirstSection(*doc, *env));
        QCOMPARE(doc->sectionCount(), 2);
        QCOMPARE(doc->sectionProps(0).pageW, 13680);
        QCOMPARE(doc->sectionProps(0).orient, QStringLiteral("landscape"));
        QCOMPARE(doc->sectionProps(1).pageW, 12240);   // (the letter is as it was)
        QVERIFY(textOf(*doc).contains(QStringLiteral("Nora Pell")) && textOf(*doc).contains(QStringLiteral("Dear Nora,|Thank you.")));
        QVERIFY(textOf(*doc).indexOf(QStringLiteral("Nora Pell")) < textOf(*doc).indexOf(QStringLiteral("Dear Nora")));
        QCOMPARE(doc->pager().pageCount(), 2);
        // One step takes it away.
        doc->undo();
        QCOMPARE(doc->sectionCount(), 1);
        QCOMPARE(textOf(*doc), QStringLiteral("Dear Nora,|Thank you."));
        // It saves and opens again.
        doc->redo();
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(again->sectionCount(), 2);
        QCOMPARE(again->sectionProps(0).pageW, 13680);
    }

    void anEnvelopeIsAPageOfTheEnvelopesSize()
    {
        const QVector<mail::EnvelopeSize> sizes = mail::envelopeSizes();
        QVERIFY(sizes.size() >= 8);
        auto doc = mail::envelopeDocument(sizes.first(), QStringLiteral("Mara\n1 Way"), mail::textRuns(QStringLiteral("Nora Pell\n1 Quarry Ct")));
        QVERIFY(doc);
        const QByteArray xml = partOf(doc->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY(xml.contains("w:w=\"13680\"") && xml.contains("w:h=\"5940\""));   // (9 1/2 by 4 1/8 inches)
        QVERIFY(xml.contains("w:orient=\"landscape\""));
        QVERIFY(textOf(*doc).contains(QStringLiteral("Nora Pell")) && textOf(*doc).contains(QStringLiteral("Mara")));
        QCOMPARE(doc->pager().pageCount(), 1);
        // For a merge its delivery address is an address block.
        auto template_ = mail::envelopeDocument(sizes.first(), QString(), mail::fieldRuns(mail::addressBlockCode(QString()), QStringLiteral("\u00ABAddress Block\u00BB")));
        QVERIFY(template_);
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*template_, people(), {}, options);
        QVERIFY(merged);
        QVERIFY(textOf(*merged).contains(QStringLiteral("Arlington, VA 22201")));
        QCOMPARE(merged->pager().pageCount(), 3);   // (three recipients: a page each)
    }

    void theMailingsTabStartsAMergePreviewsItAndClearsIt()
    {
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        w.show();
        write::WriteDoc *doc = w.document();
        QTextCursor c = w.view()->cursor();
        insertText(*doc, c, QStringLiteral("Dear "));
        QVERIFY(insertField(*doc, c, mail::mergeFieldCode(QStringLiteral("First Name")), QStringLiteral("\u00ABFirst Name\u00BB")));
        QVERIFY(insertField(*doc, c, mail::mergeFieldCode(QStringLiteral("Nickname")), QStringLiteral("\u00ABNickname\u00BB")));
        QVERIFY(!mail::stateIfAny(*doc));
        w.act(QStringLiteral("mail.type.letters"))->trigger();
        QVERIFY(mail::stateIfAny(*doc));
        QCOMPARE(mail::stateIfAny(*doc)->kind(), mail::Kind::Letters);
        QVERIFY(doc->isModified());
        // Without a list the commands that need one are dimmed.
        QVERIFY(!w.act(QStringLiteral("mail.check"))->isEnabled());
        QVERIFY(!w.act(QStringLiteral("mail.preview"))->isEnabled());
        QVERIFY(!w.act(QStringLiteral("mail.addressBlock"))->isEnabled());
        mail::State &state = mail::stateOf(*doc);
        state.data = people();
        state.loaded = true;
        w.refreshNow();
        QVERIFY(w.act(QStringLiteral("mail.check"))->isEnabled());
        w.act(QStringLiteral("mail.check"))->trigger();
        QVERIFY2(said.last().contains(QStringLiteral("\"Nickname\"")), qPrintable(said.last()));
        // Preview Results opens a pane with the first recipient; Next goes on; the document is as it was.
        const QString before = textOf(*doc);
        w.act(QStringLiteral("mail.preview"))->trigger();
        auto *pane = w.findChild<write::MailPreviewPane *>();
        QVERIFY(pane);
        QTRY_VERIFY(pane->shown());
        QVERIFY(textOf(*pane->shown()).startsWith(QStringLiteral("Dear Mara")));
        w.act(QStringLiteral("mail.next"))->trigger();
        QCOMPARE(pane->place(), 1);
        QVERIFY(textOf(*pane->shown()).startsWith(QStringLiteral("Dear Nora")));
        w.act(QStringLiteral("mail.last"))->trigger();
        QCOMPARE(pane->place(), 2);   // (all three records are included)
        w.act(QStringLiteral("mail.first"))->trigger();
        QCOMPARE(pane->place(), 0);
        QCOMPARE(textOf(*doc), before);
        // Clear Merge Type makes it an ordinary document: nothing of the merge is saved.
        w.act(QStringLiteral("mail.type.clear"))->trigger();
        QCOMPARE(mail::stateIfAny(*doc)->kind(), mail::Kind::None);
        QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/settings.xml")).contains("mailMerge"));
    }

    void anAddressBlockInsertedFromTheTabStaysAFieldUntilTheMerge()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        write::WriteDoc *doc = w.document();
        mail::State &state = mail::stateOf(*doc);
        state.data = people();
        state.loaded = true;
        QTextCursor c = w.view()->cursor();
        QVERIFY(mail::insertRule(*doc, c, QStringLiteral("IF \"{ MERGEFIELD State }\" = \"VA\" \"Virginian\" \"elsewhere\""), QString()));
        auto merged = mail::mergeToDocument(*doc, state.data, {}, mail::MergeOptions());
        QVERIFY(merged);
        QCOMPARE(textOf(*merged), QStringLiteral("elsewhere|Virginian|elsewhere"));
    }

    void anOdtSaysItHasNoPlaceForTheMergeSettings()
    {
        auto doc = WriteDoc::blank();
        QStringList notes;
        QVERIFY(!doc->toOdt(nullptr, &notes).isEmpty());
        QVERIFY2(notes.join(QLatin1Char(';')).indexOf(QStringLiteral("mail merge")) < 0, qPrintable(notes.join(QLatin1Char(';'))));
        mail::State &state = mail::stateOf(*doc);
        state.settings.type = mail::kindValue(mail::Kind::Letters);
        state.changed = true;
        notes.clear();
        QVERIFY(!doc->toOdt(nullptr, &notes).isEmpty());
        QVERIFY2(notes.join(QLatin1Char(';')).contains(QStringLiteral("mail merge settings")), qPrintable(notes.join(QLatin1Char(';'))));
    }


    void aWindowShowingASheetOfLabelsOrAnEnvelopeCloses()
    {
        for (int which = 0; which < 2; ++which) {
            auto d = which == 0 ? mail::labelDocument(mail::labelSheets().first(), [](int) { return mail::tightParagraph(mail::textRuns(QStringLiteral("x"))); })
                                : mail::envelopeDocument(mail::envelopeSizes().first(), QStringLiteral("a"), mail::textRuns(QStringLiteral("b")));
            QVERIFY(d);
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.resize(900, 700);
            w.setDocument(std::move(d), QString());
            w.show();
            w.view()->setZoomMode(write::PageView::ZoomMode::OnePage);
            QTRY_VERIFY(w.view()->pageCount() >= 1);
            QVERIFY(w.view()->pageCount() >= 1);
        }
    }

    // Screenshots for a person to look at (JW_SHOT_DIR names the folder; nothing is made without it).
    void screenshots()
    {
        const QString dir = qEnvironmentVariable("JW_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JW_SHOT_DIR is not set");
        {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1440, 900);
        w.show();
        write::WriteDoc *doc = w.document();
        QTextCursor c = w.view()->cursor();
        insertText(*doc, c, QStringLiteral("Birchline Supply, 12 Main Street\n"));
        QVERIFY(mail::insertRule(*doc, c, mail::addressBlockCode(QString()), QStringLiteral("\u00ABAddress Block\u00BB")));
        insertText(*doc, c, QStringLiteral("\n\n"));
        QVERIFY(mail::insertRule(*doc, c, mail::greetingCode(QString(), QString()), QStringLiteral("\u00ABGreeting Line\u00BB")));
        insertText(*doc, c, QStringLiteral("\n\nThank you for your order, "));
        QVERIFY(mail::insertRule(*doc, c, mail::mergeFieldCode(QStringLiteral("First Name")), QStringLiteral("\u00ABFirst Name\u00BB")));
        insertText(*doc, c, QStringLiteral(". It ships to "));
        QVERIFY(mail::insertRule(*doc, c, QStringLiteral("IF \"{ MERGEFIELD State }\" = \"VA\" \"Virginia\" \"your state\""), QStringLiteral("\u00ABIf\u2026Then\u2026Else\u00BB")));
        insertText(*doc, c, QStringLiteral(" this week.\n"));
        w.view()->setCursor(c);
        mail::State &state = mail::stateOf(*doc);
        state.settings.type = mail::kindValue(mail::Kind::Letters);
        state.data = people();
        state.data.rows.removeLast();
        state.data.include.removeLast();
        state.loaded = true;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (w.ribbon()->tabName(i) == QLatin1String("Mailings")) w.ribbon()->showTab(w.ribbon()->tabPage(i));
        w.act(QStringLiteral("mail.preview"))->trigger();
        auto *pane = w.findChild<write::MailPreviewPane *>();
        QVERIFY(pane);
        QTRY_VERIFY(pane->shown());
        pane->next();
        QTRY_VERIFY(textOf(*pane->shown()).contains(QStringLiteral("Nora")));
        QVERIFY(w.grab().save(dir + QStringLiteral("/mailings-preview.png")));
        }
        // The label sheet and the envelope, as documents.
        auto labels = mail::labelDocument(mail::labelSheets().first(), [](int i) { return i % 7 == 0 ? mail::tightParagraph(mail::textRuns(QStringLiteral("Nora Pell\n1 Quarry Ct\nArlington, VA 22201"))) : QByteArray(); });
        QVERIFY(labels);
        WriteWindow lw;
        lw.setNotifier([](const QString &, const QString &) {});
        lw.resize(1100, 900);
        lw.setDocument(std::move(labels), QString());
        lw.show();
        lw.view()->setZoomMode(write::PageView::ZoomMode::OnePage);
        QTRY_VERIFY(lw.view()->pageCount() >= 1);
        QVERIFY(lw.grab().save(dir + QStringLiteral("/labels.png")));
        auto env = mail::envelopeDocument(mail::envelopeSizes().first(), QStringLiteral("Mara Fenwick\n12 Harbor Way\nLondon"), mail::textRuns(QStringLiteral("Nora Pell\n1 Quarry Ct\nArlington, VA 22201")));
        QVERIFY(env);
        WriteWindow ew;
        ew.setNotifier([](const QString &, const QString &) {});
        ew.resize(1100, 700);
        ew.setDocument(std::move(env), QString());
        ew.show();
        ew.view()->setZoomMode(write::PageView::ZoomMode::OnePage);
        QTRY_VERIFY(ew.view()->pageCount() >= 1);
        QVERIFY(ew.grab().save(dir + QStringLiteral("/envelope.png")));
    }

    void aListANamedFileIsReadOnlyFromBesideTheDocument()
    {
        QTemporaryDir here, elsewhere;
        const QString far = writeFile(elsewhere, "people.csv", "First Name\r\nMara\r\n");
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        auto doc = open(makeDocx(para(field("MERGEFIELD First_Name", "x"))));
        QVERIFY(doc);
        mail::State &state = mail::stateOf(*doc);
        state.settings.type = mail::kindValue(mail::Kind::Letters);
        state.sourcePath = far;
        w.setDocument(std::move(doc), here.filePath(QStringLiteral("letter.docx")));
        w.show();
        // A file the document names somewhere else is not read: the person is asked to choose it.
        w.act(QStringLiteral("mail.preview"))->trigger();
        QVERIFY(!w.findChild<write::MailPreviewPane *>());
        QVERIFY2(said.last().contains(QStringLiteral("not next to it")), qPrintable(said.last()));
        QVERIFY(!mail::stateIfAny(*w.document())->loaded);
        // A file of the same name beside the document is.
        writeFile(here, "people.csv", "First Name\r\nNora\r\n");
        w.act(QStringLiteral("mail.preview"))->trigger();
        auto *pane = w.findChild<write::MailPreviewPane *>();
        QVERIFY(pane);
        QTRY_VERIFY(pane->shown());
        QVERIFY(textOf(*pane->shown()).startsWith(QStringLiteral("Nora")));
        QCOMPARE(mail::stateIfAny(*w.document())->sourcePath, far);   // (the link in the file is as it was)
    }

    void theTextOfEachMessageIsTheMergedText()
    {
        auto doc = open(makeDocx(para(run("Dear ") + field("MERGEFIELD First_Name", "x") + run(",")) + para(run("Bye"))));
        QVERIFY(doc);
        MergeSource m = people();
        m.include[1] = false;
        const QStringList texts = mail::mergeToTexts(*doc, m, {}, mail::MergeOptions());
        QCOMPARE(texts, (QStringList{QStringLiteral("Dear Mara,\nBye"), QStringLiteral("Dear ,\nBye")}));
    }

    void aMessageFileIsMadeForEachRecipientWhoHasAnAddress()
    {
        MergeSource m;
        m.fields = {"Name", "Email"};
        m.rows = {{"Mara", "mara@example.com"}, {"Nora", ""}, {"Tomas", "tomas@example.org\r\nBcc: x@example.net"}, {"Ivo", "ivo@example.net"}};
        m.include = QVector<bool>(4, true);
        QTemporaryDir dir;
        const QStringList texts = {"Dear Mara,\nHello \u00E9", "Dear Nora", "Dear Tomas", "Dear Ivo"};
        QString error;
        const int written = mail::writeEmailFiles(texts, m, {0, 1, 2, 3}, QStringLiteral("Email"), QStringLiteral("Hi, \u00E9"), dir.filePath(QStringLiteral("out")), &error);
        QCOMPARE(written, 2);   // (no address, and an address with a line break that would add a header, make none)
        const QStringList files = QDir(dir.filePath(QStringLiteral("out"))).entryList(QDir::Files, QDir::Name);
        QCOMPARE(files.size(), 2);
        QVERIFY(files[0].startsWith(QStringLiteral("00001-mara@example.com")) && files[0].endsWith(QStringLiteral(".eml")));
        QFile f(dir.filePath(QStringLiteral("out/") + files[0]));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray eml = f.readAll();
        QVERIFY(eml.startsWith("To: mara@example.com\r\n"));
        QVERIFY(eml.contains("Subject: =?UTF-8?B?" + QByteArray("Hi, \xC3\xA9").toBase64() + "?="));
        const QByteArray body = eml.mid(eml.indexOf("\r\n\r\n") + 4);
        QCOMPARE(QByteArray::fromBase64(body), QByteArray("Dear Mara,\nHello \xC3\xA9"));
        QVERIFY(!eml.contains("Bcc"));
    }

    void highlightMergeFieldsShadesTheFieldsOnTheScreenOnly()
    {
        auto doc = open(makeDocx(para(run("Dear ") + field("MERGEFIELD First_Name", "\u00ABFirst_Name\u00BB") + run(","))));
        QVERIFY(doc);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        w.setDocument(std::move(doc), QString());
        QAction *a = w.act(QStringLiteral("mail.highlight"));
        QVERIFY(a);
        QVERIFY(a->isCheckable());
        QVERIFY(!a->isChecked());
        QVERIFY(!w.view()->shadeFields());
        // The shade is black at 11% over the white page: a gray of 227 (the page's own pixels are white, and the text's are darker or lighter only at its edges).
        auto shaded = [](const QImage &image, QRect *box) {
            int count = 0;
            int x0 = image.width(), x1 = -1, y0 = image.height(), y1 = -1;
            for (int y = 0; y < image.height(); ++y)
                for (int x = 0; x < image.width(); ++x)
                    if (isShade(image, x, y) && isShade(image, x + 5, y)) {   // (a run, not the odd gray pixel on a letter's edge)
                        ++count;
                        x0 = qMin(x0, x);
                        x1 = qMax(x1, x);
                        y0 = qMin(y0, y);
                        y1 = qMax(y1, y);
                    }
            *box = count ? QRect(QPoint(x0, y0), QPoint(x1, y1)) : QRect();
            return count;
        };
        auto dark = [](const QImage &image) {
            int n = 0;
            for (int y = 0; y < image.height(); y += 2)
                for (int x = 0; x < image.width(); x += 2) n += image.pixelColor(x, y).red() < 100;
            return n;
        };
        QImage plain;
        QTRY_VERIFY((plain = w.view()->viewport()->grab().toImage(), dark(plain) > 20));   // (the page, with its text, has been painted)
        QRect box;
        QCOMPARE(shaded(plain, &box), 0);
        a->trigger();
        QVERIFY(w.view()->shadeFields());
        QImage with;
        QTRY_VERIFY((with = w.view()->viewport()->grab().toImage(), shaded(with, &box) > 100));
        // One shaded box, as wide as a field's name and no taller than a line, and the text is still there on top of it.
        QVERIFY2(box.width() > 40 && box.height() > 6 && box.height() < 40, qPrintable(QStringLiteral("%1x%2").arg(box.width()).arg(box.height())));
        QVERIFY(dark(with) > 20);
        a->trigger();
        QVERIFY(!w.view()->shadeFields());
        QTRY_COMPARE(shaded(w.view()->viewport()->grab().toImage(), &box), 0);
    }

    void aWideHeaderDoesNotMultiplyTheCellsOfTheRows()
    {
        QTemporaryDir dir;
        QByteArray csv;
        for (int i = 0; i < 5000; ++i) csv += "c" + QByteArray::number(i) + ",";
        csv += "last\n";
        for (int i = 0; i < 2000; ++i) csv += "x\n";
        MergeSource m;
        QVERIFY(loadMergeSource(writeFile(dir, "wide.csv", csv), &m, nullptr));
        QVERIFY(m.fields.size() <= mostMergeColumns());
        QVERIFY(qint64(m.rows.size()) * qMax<qint64>(1, m.fields.size()) <= mostMergeCells());
        QVERIFY(m.truncated);
        QCOMPARE(m.include.size(), m.rows.size());
    }

    void aSheetWithOneCellFarAwayIsReadFromItsCellsAlone()
    {
        QTemporaryDir dir;
        const QString path = writeFile(dir, "sparse.xlsx",
            makeXlsx("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Name</t></is></c></row><row r=\"200000\"><c r=\"F200000\" t=\"inlineStr\"><is><t>far</t></is></c></row>"));
        MergeSource m;
        QString error;
        QVERIFY2(loadMergeSource(path, &m, &error), qPrintable(error));
        QCOMPARE(m.rows.size(), 1);   // (the rows with no cell are not recipients)
        QCOMPARE(m.rows[0].value(5), QStringLiteral("far"));
        QVERIFY(m.fields.size() <= mostMergeColumns());
    }

    void aBlankRowOfAListIsARecipientUntilTheWriterDropsIt()
    {
        QTemporaryDir dir;
        MergeSource m;
        QVERIFY(loadMergeSource(writeFile(dir, "blank.csv", "Name,City,Zip\n,,\nAnn,Oslo,\n,,\n"), &m, nullptr));
        QCOMPARE(m.rows.size(), 3);   // (as JeffPub has always read it)
        mail::dropBlankRecords(m);
        QCOMPARE(m.rows.size(), 1);
        QCOMPARE(m.include.size(), 1);
        QCOMPARE(m.value(0, "Name"), QStringLiteral("Ann"));
    }

    void aListLongerThanTheMostRecordsKeepsItsFirstOnes()
    {
        MergeSource m;
        m.fields = {"A"};
        for (int i = 0; i < mail::kMostRecords + 5; ++i) m.rows << QStringList{QString::number(i)};
        m.include = QVector<bool>(m.rows.size(), true);
        mail::limitRecords(m);
        QCOMPARE(m.rows.size(), mail::kMostRecords);
        QCOMPARE(m.include.size(), mail::kMostRecords);
    }

    void aRuleInsertedWithNoResultShowsItsNameInTheEditor()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        QVERIFY(mail::insertRule(*doc, c, QStringLiteral("IF \"{ MERGEFIELD State }\" = \"VA\" \"a\" \"b\""), QString()));
        QVERIFY(mail::insertRule(*doc, c, QStringLiteral("NEXT"), QString()));
        const QByteArray docx = doc->toDocx();
        QVERIFY(documentHas(docx, "\xC2\xAB" "IF\xE2\x80\xA6\xC2\xBB"));
        QVERIFY(documentHas(docx, "\xC2\xAB" "NEXT\xE2\x80\xA6\xC2\xBB"));
    }

    void aNamedListIsFoundFromTheDocumentsFolderAndNotThroughALinkOutOfIt()
    {
        QTemporaryDir here, elsewhere;
        QDir(here.path()).mkpath(QStringLiteral("lists"));
        writeFile(here, "lists/people.csv", "First Name\r\nNora\r\n");
        const QString outside = writeFile(elsewhere, "secret.csv", "First Name\r\nPrivate\r\n");
        auto make = [&](const QString &named, WriteWindow &w) {
            auto doc = open(makeDocx(para(field("MERGEFIELD First_Name", "x"))));
            mail::State &state = mail::stateOf(*doc);
            state.settings.type = mail::kindValue(mail::Kind::Letters);
            state.sourcePath = named;
            w.setDocument(std::move(doc), here.filePath(QStringLiteral("letter.docx")));
            w.show();
        };
        {
            // A relative name is the document's folder's, not the working directory's.
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            make(QStringLiteral("lists/people.csv"), w);
            w.act(QStringLiteral("mail.preview"))->trigger();
            auto *pane = w.findChild<write::MailPreviewPane *>();
            QVERIFY(pane);
            QTRY_VERIFY(pane->shown());
            QVERIFY(textOf(*pane->shown()).startsWith(QStringLiteral("Nora")));
        }
        // A link in the folder to a file outside it is not followed, by its own name or by the name beside the document.
        // On Windows QFile::link makes a shortcut file, not a link the file system follows, so there is nothing to test there.
        if (!QFile::link(outside, here.filePath(QStringLiteral("people.csv"))) || !QFileInfo(here.filePath(QStringLiteral("people.csv"))).isSymbolicLink()) QSKIP("links are not made here");
        for (const QString &named : {here.filePath(QStringLiteral("people.csv")), QStringLiteral("people.csv"), QStringLiteral("/nowhere/people.csv")}) {
            WriteWindow w;
            QStringList said;
            w.setNotifier([&](const QString &, const QString &text) { said << text; });
            make(named, w);
            w.act(QStringLiteral("mail.preview"))->trigger();
            QVERIFY(!w.findChild<write::MailPreviewPane *>());
            QVERIFY2(!said.isEmpty() && said.last().contains(QStringLiteral("not next to it")), qPrintable(named));
        }
    }

    void anEmailAddressIsOnePersonsOrTheRecordIsLeftOut()
    {
        MergeSource m;
        m.fields = {"Email"};
        const QStringList addresses = {"a@example.com, b@example.com", "c@example.com;d@example.com", "Name <e@example.com>", "no-at-sign", "f@example.com", "g\u00E9@example.com", "h@exam ple.com", "@example.com", "i@example"};
        for (const QString &a : addresses) m.rows << QStringList{a};
        m.include = QVector<bool>(m.rows.size(), true);
        QStringList texts;
        QVector<int> rows;
        for (int i = 0; i < m.rows.size(); ++i) {
            texts << QStringLiteral("t");
            rows << i;
        }
        QTemporaryDir dir;
        int refused = 0;
        QString error;
        const int written = mail::writeEmailFiles(texts, m, rows, QStringLiteral("Email"), QStringLiteral("s"), dir.path(), &error, &refused);
        QCOMPARE(written, 2);   // (f@example.com, and the one with a letter past ASCII, which is a valid address)
        QCOMPARE(refused, 7);
        const QStringList files = QDir(dir.path()).entryList(QDir::Files, QDir::Name);
        QCOMPARE(files.size(), 2);
        QVERIFY(files[0].startsWith(QStringLiteral("00005-f")));
        QVERIFY(files[1].startsWith(QStringLiteral("00006-")));
    }

    void aLongSubjectIsSplitIntoEncodedWordsOfAtMost75Characters()
    {
        MergeSource m;
        m.fields = {"Email"};
        m.rows << QStringList{"a@example.com"};
        m.include = {true};
        QString subject;
        for (int i = 0; i < 40; ++i) subject += QStringLiteral("Caf\u00E9 caf\u00E9 \u00FCber ");
        QTemporaryDir dir;
        QCOMPARE(mail::writeEmailFiles({QStringLiteral("t")}, m, {0}, QStringLiteral("Email"), subject, dir.path()), 1);
        QFile f(dir.filePath(QDir(dir.path()).entryList(QDir::Files).first()));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray eml = f.readAll();
        const QByteArray head = eml.left(eml.indexOf("\r\n\r\n"));
        QByteArray decoded;
        int words = 0;
        for (const QByteArray &line : head.split('\n')) {
            QByteArray l = line;
            if (l.endsWith('\r')) l.chop(1);
            QVERIFY2(l.size() <= 76, l.constData());
            if (!l.startsWith("Subject:") && !l.startsWith(' ')) continue;
            for (const QByteArray &word : l.mid(l.startsWith(' ') ? 1 : 9).split(' ')) {
                if (word.isEmpty()) continue;
                QVERIFY2(word.size() <= 75 && word.startsWith("=?UTF-8?B?") && word.endsWith("?="), word.constData());
                decoded += QByteArray::fromBase64(word.mid(10, word.size() - 12));
                ++words;
            }
        }
        QVERIFY(words > 3);
        QCOMPARE(QString::fromUtf8(decoded), subject);   // (no character is cut in two)
    }

    void aSavedMergeWhoseListIsOverTheCapsSaysSoWhenItIsRead()
    {
        QTemporaryDir here;
        QByteArray csv = "A\n";
        for (int i = 0; i < mail::kMostRecords + 5; ++i) csv += "x\n";
        writeFile(here, "people.csv", csv);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        auto doc = open(makeDocx(para(field("MERGEFIELD A", "x"))));
        QVERIFY(doc);
        mail::State &state = mail::stateOf(*doc);
        state.settings.type = mail::kindValue(mail::Kind::Letters);
        state.sourcePath = QStringLiteral("people.csv");
        w.setDocument(std::move(doc), here.filePath(QStringLiteral("letter.docx")));
        w.show();
        w.act(QStringLiteral("mail.preview"))->trigger();
        QTRY_VERIFY(mail::stateIfAny(*w.document())->loaded);
        QCOMPARE(mail::stateIfAny(*w.document())->data.rows.size(), mail::kMostRecords);
        QVERIFY2(w.statusBar()->currentMessage().contains(QStringLiteral("too big")), qPrintable(w.statusBar()->currentMessage()));
    }

    void aSpreadsheetThatIsCutShortIsNotReadAsAShorterList()
    {
        QTemporaryDir dir;
        const QByteArray whole = makeXlsx("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Name</t></is></c></row><row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>Ann</t></is></c></row>");
        MergeSource ok;
        QVERIFY(loadMergeSource(writeFile(dir, "whole.xlsx", whole), &ok, nullptr));
        QCOMPARE(ok.rows.size(), 1);
        // The file cut off part way (the end of a zip is its directory: what is left has none).
        MergeSource cut;
        QString error;
        QVERIFY(!loadMergeSource(writeFile(dir, "cut.xlsx", whole.left(whole.size() / 2)), &cut, &error));
        QVERIFY(!error.isEmpty());
        // A sheet part that is not whole XML, in a package that is: an error, not a list of what was read.
        office::PackageWriter w;
        const QByteArray ns = "xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        w.addPart(QStringLiteral("/xl/workbook.xml"), "<?xml version=\"1.0\"?><workbook " + ns + "><sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
        w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/xl/workbook.xml"));
        w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), "<?xml version=\"1.0\"?><worksheet " + ns + "><sheetData><row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Name</t></is></c></row><row r=\"2\"><c r=\"A2\" t=\"inl",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), kRel + QStringLiteral("worksheet"), QStringLiteral("/xl/worksheets/sheet1.xml"));
        MergeSource half;
        error.clear();
        QVERIFY(!loadMergeSource(writeFile(dir, "half.xlsx", w.finish()), &half, &error));
        QVERIFY(!error.isEmpty());
        // A package with no workbook part is read for its first sheet alone; a sheet cut short there is an error too.
        office::PackageWriter bare;
        bare.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), "<?xml version=\"1.0\"?><worksheet " + ns + "><sheetData><row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Name</t></is></c></row><row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>Ann</t></is></c></row><row r=\"3\"><c r=\"A3\" t=\"inl",
                     QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
        MergeSource bareHalf;
        error.clear();
        QVERIFY(!loadMergeSource(writeFile(dir, "barehalf.xlsx", bare.finish()), &bareHalf, &error));
        QVERIFY(!error.isEmpty());
    }

    void aMessageIsAddressedToTheRecipientItWasMadeFrom()
    {
        // The first record is skipped: the texts are Bo's and Cy's, and the files go to Bo and Cy.
        auto doc = open(makeDocx(para(nestedField("SKIPIF \"", "MERGEFIELD Name", "\" = \"Ann\"") + run("Hi ") + field("MERGEFIELD Name", "x"))));
        QVERIFY(doc);
        const MergeSource m = threePeople();
        QStringList texts;
        QVector<int> rows;
        {
            QString error;
            texts = mail::mergeToTexts(*doc, m, {}, mail::MergeOptions(), &error, &rows);
        }
        QCOMPARE(texts, (QStringList{QStringLiteral("Hi Bo"), QStringLiteral("Hi Cy")}));
        QCOMPARE(rows, (QVector<int>{1, 2}));
        QTemporaryDir dir;
        QCOMPARE(mail::writeEmailFiles(texts, m, rows, QStringLiteral("Email"), QStringLiteral("s"), dir.path()), 2);
        const QStringList files = QDir(dir.path()).entryList(QDir::Files, QDir::Name);
        QCOMPARE(files.size(), 2);
        QVERIFY(files[0].contains(QStringLiteral("bo@example.com")));
        QFile f(dir.filePath(files[0]));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray eml = f.readAll();
        QVERIFY(eml.startsWith("To: bo@example.com\r\n"));
        QCOMPARE(QByteArray::fromBase64(eml.mid(eml.indexOf("\r\n\r\n") + 4)), QByteArray("Hi Bo"));
    }

    void aMessageThatTakesTheNextRecordIsAddressedToTheFirstOfThem()
    {
        auto doc = open(makeDocx(para(field("MERGEFIELD Name", "x") + run(" and ") + field("NEXT", "") + field("MERGEFIELD Name", "x"))));
        QVERIFY(doc);
        QVector<int> rows;
        const QStringList texts = mail::mergeToTexts(*doc, threePeople(), {}, mail::MergeOptions(), nullptr, &rows);
        QCOMPARE(texts, (QStringList{QStringLiteral("Ann and Bo"), QStringLiteral("Cy and ")}));
        QCOMPARE(rows, (QVector<int>{0, 2}));
    }

    void aMergeStopsAtTheMostPagesItMakes()
    {
        auto doc = open(makeDocx(para(field("MERGEFIELD Name", "x"))));
        QVERIFY(doc);
        mail::MergeOptions o;
        o.mostPages = 2;
        QString error;
        QVERIFY(!mail::mergeToDocument(*doc, threePeople(), {}, o, &error));
        QVERIFY(!error.isEmpty());
        QVERIFY(mail::mergeToDocument(*doc, threePeople(), {}, mail::MergeOptions(), &error));
        QVERIFY(mail::mergeToTexts(*doc, threePeople(), {}, o, &error).isEmpty());
        QVERIFY(!error.isEmpty());
    }

    void theQuestionsOfAMergeAreFewAndACancelStopsIt()
    {
        QByteArray body;
        for (int i = 0; i < 30; ++i) body += para(field("FILLIN \"question " + QByteArray::number(i) + "\" \\d \"dflt\"", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        int asked = 0;
        mail::MergeOptions o;
        o.ask = [&](const QString &, const QString &def) {
            ++asked;
            return def;
        };
        QString error;
        auto merged = mail::mergeToDocument(*doc, threePeople(), {}, o, &error);
        QVERIFY(merged);
        QCOMPARE(asked, mail::kMostPrompts);
        QVERIFY(textOf(*merged).contains(QStringLiteral("dflt")));   // (the ones past the limit use their defaults)
        // A cancel on a prompt stops the merge.
        asked = 0;
        bool cancelled = false;
        mail::MergeOptions c;
        c.cancelled = &cancelled;
        c.ask = [&](const QString &, const QString &def) {
            ++asked;
            cancelled = true;
            return def;
        };
        QVERIFY(!mail::mergeToDocument(*doc, threePeople(), {}, c, &error));
        QCOMPARE(asked, 1);
    }

    void highlightMergeFieldsShadesMergeFieldsAndNotOtherFields()
    {
        auto doc = open(makeDocx(para(run("Dear ") + field("MERGEFIELD First_Name", "\u00ABFirst_Name\u00BB") + run(" then ") + field("PAGE", "88888888") + run(" and ") +
                                      field("IF \"a\" = \"a\" \"b\" \"c\"", "\u00ABIf\u00BB") + run(" and ") + field("REF nothing", "99999999"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::End);
        c.insertBlock();
        QVERIFY(mail::insertRule(*doc, c, QStringLiteral("NEXT"), QStringLiteral("\u00ABNext Record\u00BB")));   // (a field put in by the program: one character)
        c.insertText(QStringLiteral("  "));
        QVERIFY(insertField(*doc, c, QStringLiteral("PAGE"), QStringLiteral("77777777")));
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        w.setDocument(std::move(doc), QString());
        w.act(QStringLiteral("mail.highlight"))->trigger();
        // The shaded stretches of each line: the merge field and the rule of the first line, the rule of the second, and none for PAGE or REF.
        auto stretches = [&](const QImage &image, int y) {
            int n = 0, last = -100;
            for (int x = 100; x < 700; ++x)   // (inside the page's text area)
                if (isShade(image, x, y) && isShade(image, x + 5, y)) {
                    if (x - last > 25) ++n;
                    last = x;
                }
            return n;
        };
        QImage image;
        QTRY_VERIFY((image = w.view()->viewport()->grab().toImage(), stretches(image, 130) > 0));
        // (The shade of the two lines, a line's height apart, runs from the top of the first to the bottom of the second.)
        int top = image.height(), bottom = 0;
        for (int y = 0; y < image.height(); ++y)
            if (stretches(image, y)) {
                top = qMin(top, y);
                bottom = qMax(bottom, y);
            }
        const int firstLine = stretches(image, top + 4), secondLine = stretches(image, bottom - 10);   // (in the middle of the line, clear of the letters' tails)
        QCOMPARE(firstLine, 2);   // (the merge field and the IF rule, not PAGE or REF between and after them)
        QCOMPARE(secondLine, 1);   // (the rule the program put in, not the page number after it)
    }

    void theMatchedFieldsAreKeptInTheDataSourcesFieldMap()
    {
        const QByteArray ns = kNs;
        const QByteArray settings = "<?xml version=\"1.0\"?><w:settings " + ns + "><w:mailMerge><w:mainDocumentType w:val=\"formLetters\"/><w:dataType w:val=\"textFile\"/><w:connectString w:val=\"\"/>"
                                    "<w:dataSource r:id=\"rId1\"/><w:odso><w:udl w:val=\"x\"/><w:fieldMapData><w:type w:val=\"dbColumn\"/><w:name w:val=\"Efternavn\"/><w:mappedName w:val=\"Last Name\"/>"
                                    "<w:column w:val=\"1\"/><w:lid w:val=\"en-US\"/></w:fieldMapData></w:odso></w:mailMerge></w:settings>";
        auto doc = open(makeDocx(para(run("x")), settings, QStringLiteral("file:///data/list.csv")));
        QVERIFY(doc);
        mail::State *state = mail::stateIfAny(*doc);
        QVERIFY(state);
        QCOMPARE(state->fieldMap.value(QStringLiteral("Last")), QStringLiteral("Efternavn"));
        // Saved as it is, nothing changes; matching another field adds to the map and keeps the rest of the element.
        QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/settings.xml")), settings);
        state->data.fields = {"Fornavn", "Efternavn"};
        state->fieldMap.insert(QStringLiteral("First"), QStringLiteral("Fornavn"));
        state->changed = true;
        auto again = open(doc->toDocx());
        QVERIFY(again);
        mail::State *back = mail::stateIfAny(*again);
        QVERIFY(back);
        QCOMPARE(back->fieldMap.value(QStringLiteral("First")), QStringLiteral("Fornavn"));
        QCOMPARE(back->fieldMap.value(QStringLiteral("Last")), QStringLiteral("Efternavn"));
        QVERIFY(back->settings.odso.contains("<w:udl w:val=\"x\"/>"));
    }
};

QTEST_MAIN(MailTests)
#include "tst_write_mail.moc"
