// Tests of JeffWrite's Mailings tab: the recipient lists a merge reads (core/mergesource.h), the merge fields and rules (core/mergeops.h), the merge itself, envelopes and labels, and
// the merge settings kept in a .docx. Every file is built here in code.

#include "app/mailpreview.h"
#include "app/pageview.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "app/ribbon.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/labelops.h"
#include "core/fieldops.h"
#include "core/mergeops.h"
#include "core/mergesource.h"
#include "core/writedoc.h"
#include "office/package.h"
#include "wml/wml.h"

#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QRegularExpression>
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
    add({"Dr.", "Ada", "Lovelace", "", "12 Analytical Way", "London", "", "N1 9GU", "United Kingdom"});
    add({"", "Grace", "Hopper", "Navy", "1 Cobol Ct", "Arlington", "VA", "22201", "United States"});
    add({"", "", "", "", "", "", "", "", ""});
    return m;
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
        const QString path = writeFile(dir, "list.csv", "First Name,Last Name,Notes\r\nAda,Lovelace,\"likes \"\"maths\"\", and poems\"\r\nGrace,Hopper\r\n");
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
            "<table:table-row><table:table-cell office:value-type=\"string\"><text:p>Ada</text:p></table:table-cell><table:table-cell table:style-name=\"zip\" office:value-type=\"float\" office:value=\"2134\"><text:p>02134</text:p></table:table-cell></table:table-row>"
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
        QCOMPARE(mail::addressBlockText(m, 0, none, QString()), QStringLiteral("Dr. Ada Lovelace\u2028" "12 Analytical Way\u2028" "London N1 9GU\u2028" "United Kingdom"));
        QCOMPARE(mail::addressBlockText(m, 1, none, QString()), QStringLiteral("Grace Hopper\u2028" "Navy\u2028" "1 Cobol Ct\u2028" "Arlington, VA 22201\u2028" "United States"));
        // A country that is the home one is not written.
        QCOMPARE(mail::addressBlockText(m, 1, none, QString(), QStringLiteral("united states")), QStringLiteral("Grace Hopper\u2028" "Navy\u2028" "1 Cobol Ct\u2028" "Arlington, VA 22201"));
        QCOMPARE(mail::addressBlockText(m, 2, none, QString()), QString());
        // Match Fields: a column the usual names don't find.
        MergeSource odd;
        odd.fields = {"Who", "Street"};
        odd.rows << QStringList{"Ada Lovelace", "1 Way"};
        odd.include << true;
        QHash<QString, QString> map;
        map.insert(QStringLiteral("First"), QStringLiteral("Who"));
        QCOMPARE(mail::addressBlockText(odd, 0, map, QString()), QStringLiteral("Ada Lovelace\u2028" "1 Way"));
    }

    void aGreetingLineFallsBackWhereTheRecordHasNoName()
    {
        const MergeSource m = people();
        const QHash<QString, QString> none;
        const QString fallback = QStringLiteral("Dear Sir or Madam,");
        QCOMPARE(mail::greetingText(m, 0, none, QStringLiteral("Dear {Title} {Last},"), fallback), QStringLiteral("Dear Dr. Lovelace,"));
        QCOMPARE(mail::greetingText(m, 1, none, QStringLiteral("Dear {Title} {Last},"), fallback), QStringLiteral("Dear Hopper,"));
        QCOMPARE(mail::greetingText(m, 1, none, QStringLiteral("Hello {First}!"), fallback), QStringLiteral("Hello Grace!"));
        QCOMPARE(mail::greetingText(m, 2, none, QStringLiteral("Dear {Title} {Last},"), fallback), fallback);
    }

    void aMergeFieldTakesTextBeforeAndAfterOnlyWhereThereIsAValue()
    {
        const MergeSource m = people();
        const QHash<QString, QString> none;
        const QVector<int> rows{0, 1, 2};
        mail::Evaluator ev(m, none, rows);
        auto answer = [&](const QString &code) { return ev.answer(fields::parseCode(code)).value_or(QStringLiteral("none")); };
        QCOMPARE(answer(QStringLiteral("MERGEFIELD First_Name")), QStringLiteral("Ada"));
        QCOMPARE(answer(QStringLiteral("MERGEFIELD \"First Name\" \\* Upper")), QStringLiteral("ADA"));
        QCOMPARE(answer(QStringLiteral("MERGEFIELD Title \\b \"<\" \\f \"> \"")), QStringLiteral("<Dr.> "));
        QCOMPARE(answer(QStringLiteral("MERGEFIELD Nothing")), QString());
        ev.position = 2;
        QCOMPARE(answer(QStringLiteral("MERGEFIELD Title \\b \"<\" \\f \"> \"")), QString());
        QCOMPARE(answer(QStringLiteral("MERGEREC")), QStringLiteral("3"));
    }

    void aMergeMakesACopyOfTheTextForEachRecord()
    {
        const QByteArray zip = makeDocx(para(run("Dear ") + field("MERGEFIELD First_Name", "\xC2\xABFirst Name\xC2\xBB") + run(",")) + para(run("Hello again")));
        auto doc = open(zip);
        QVERIFY(doc);
        MergeSource m = people();
        m.include[1] = false;   // (excluded)
        mail::MergeOptions options;
        QString error;
        const QString before = textOf(*doc);
        auto merged = mail::mergeToDocument(*doc, m, {}, options, &error);
        QVERIFY2(merged, qPrintable(error));
        QCOMPARE(textOf(*merged), QStringLiteral("Dear Ada,|Hello again|Dear ,|Hello again"));
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
        QCOMPARE(textOf(*merged), QStringLiteral("other Lovelace #1|Virginian Hopper #2"));
    }

    void nextRecordTakesTheNextRecordIntoTheSameText()
    {
        const QByteArray body = para(field("MERGEFIELD First_Name", "x") + run(" and ") + field("NEXT", "") + field("MERGEFIELD First_Name", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        mail::MergeOptions options;
        auto merged = mail::mergeToDocument(*doc, people(), {}, options);
        QVERIFY(merged);
        QCOMPARE(textOf(*merged), QStringLiteral("Ada and Grace| and "));   // (the third record then stands alone, with no one after it)
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
        QVERIFY(textOf(*merged).startsWith(QStringLiteral("Dr. Ada Lovelace|12 Analytical Way|London N1 9GU|United Kingdom|Dear Dr. Lovelace,|Grace Hopper|Navy|")));
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
        auto doc = open(makeDocx(para(run("Dear Grace,")) + para(run("Thank you."))));
        QVERIFY(doc);
        QCOMPARE(doc->sectionCount(), 1);
        auto env = mail::envelopeDocument(mail::envelopeSizes().first(), QStringLiteral("Ada"), mail::textRuns(QStringLiteral("Grace Hopper")));
        QVERIFY(env);
        QVERIFY(mail::addAsFirstSection(*doc, *env));
        QCOMPARE(doc->sectionCount(), 2);
        QCOMPARE(doc->sectionProps(0).pageW, 13680);
        QCOMPARE(doc->sectionProps(0).orient, QStringLiteral("landscape"));
        QCOMPARE(doc->sectionProps(1).pageW, 12240);   // (the letter is as it was)
        QVERIFY(textOf(*doc).contains(QStringLiteral("Grace Hopper")) && textOf(*doc).contains(QStringLiteral("Dear Grace,|Thank you.")));
        QVERIFY(textOf(*doc).indexOf(QStringLiteral("Grace Hopper")) < textOf(*doc).indexOf(QStringLiteral("Dear Grace")));
        QCOMPARE(doc->pager().pageCount(), 2);
        // One step takes it away.
        doc->undo();
        QCOMPARE(doc->sectionCount(), 1);
        QCOMPARE(textOf(*doc), QStringLiteral("Dear Grace,|Thank you."));
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
        auto doc = mail::envelopeDocument(sizes.first(), QStringLiteral("Ada\n1 Way"), mail::textRuns(QStringLiteral("Grace Hopper\n1 Cobol Ct")));
        QVERIFY(doc);
        const QByteArray xml = partOf(doc->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY(xml.contains("w:w=\"13680\"") && xml.contains("w:h=\"5940\""));   // (9 1/2 by 4 1/8 inches)
        QVERIFY(xml.contains("w:orient=\"landscape\""));
        QVERIFY(textOf(*doc).contains(QStringLiteral("Grace Hopper")) && textOf(*doc).contains(QStringLiteral("Ada")));
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
        // Without a list the commands say so.
        w.act(QStringLiteral("mail.check"))->trigger();
        QVERIFY(said.last().contains(QStringLiteral("no list")));
        mail::State &state = mail::stateOf(*doc);
        state.data = people();
        state.loaded = true;
        w.act(QStringLiteral("mail.check"))->trigger();
        QVERIFY2(said.last().contains(QStringLiteral("\"Nickname\"")), qPrintable(said.last()));
        // Preview Results opens a pane with the first recipient; Next goes on; the document is as it was.
        const QString before = textOf(*doc);
        w.act(QStringLiteral("mail.preview"))->trigger();
        auto *pane = w.findChild<write::MailPreviewPane *>();
        QVERIFY(pane);
        QTRY_VERIFY(pane->shown());
        QVERIFY(textOf(*pane->shown()).startsWith(QStringLiteral("Dear Ada")));
        w.act(QStringLiteral("mail.next"))->trigger();
        QCOMPARE(pane->place(), 1);
        QVERIFY(textOf(*pane->shown()).startsWith(QStringLiteral("Dear Grace")));
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
            QTest::qWait(100);
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
        insertText(*doc, c, QStringLiteral("Ace Hardware Supply, 12 Main Street\n"));
        QVERIFY(mail::insertRule(*doc, c, mail::addressBlockCode(QString()), QStringLiteral("\u00ABAddress Block\u00BB")));
        insertText(*doc, c, QStringLiteral("\n\n"));
        QVERIFY(mail::insertRule(*doc, c, mail::greetingCode(QString(), QString()), QStringLiteral("\u00ABGreeting Line\u00BB")));
        insertText(*doc, c, QStringLiteral("\n\nThank you for your order, "));
        QVERIFY(mail::insertRule(*doc, c, mail::mergeFieldCode(QStringLiteral("First Name")), QStringLiteral("\u00ABFirst Name\u00BB")));
        insertText(*doc, c, QStringLiteral(". It ships to "));
        QVERIFY(mail::insertRule(*doc, c, QStringLiteral("IF \"{ MERGEFIELD State }\" = \"VA\" \"Virginia\" \"your state\""), QString()));
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
        QTest::qWait(300);
        pane->next();
        QTest::qWait(300);
        QVERIFY(w.grab().save(dir + QStringLiteral("/mailings-preview.png")));
        }
        // The label sheet and the envelope, as documents.
        auto labels = mail::labelDocument(mail::labelSheets().first(), [](int i) { return i % 7 == 0 ? mail::tightParagraph(mail::textRuns(QStringLiteral("Grace Hopper\n1 Cobol Ct\nArlington, VA 22201"))) : QByteArray(); });
        QVERIFY(labels);
        WriteWindow lw;
        lw.setNotifier([](const QString &, const QString &) {});
        lw.resize(1100, 900);
        lw.setDocument(std::move(labels), QString());
        lw.show();
        lw.view()->setZoomMode(write::PageView::ZoomMode::OnePage);
        QTest::qWait(300);
        QVERIFY(lw.grab().save(dir + QStringLiteral("/labels.png")));
        auto env = mail::envelopeDocument(mail::envelopeSizes().first(), QStringLiteral("Ada Lovelace\n12 Analytical Way\nLondon"), mail::textRuns(QStringLiteral("Grace Hopper\n1 Cobol Ct\nArlington, VA 22201")));
        QVERIFY(env);
        WriteWindow ew;
        ew.setNotifier([](const QString &, const QString &) {});
        ew.resize(1100, 700);
        ew.setDocument(std::move(env), QString());
        ew.show();
        ew.view()->setZoomMode(write::PageView::ZoomMode::OnePage);
        QTest::qWait(300);
        QVERIFY(ew.grab().save(dir + QStringLiteral("/envelope.png")));
    }

    void aListANamedFileIsReadOnlyFromBesideTheDocument()
    {
        QTemporaryDir here, elsewhere;
        const QString far = writeFile(elsewhere, "people.csv", "First Name\r\nAda\r\n");
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
        writeFile(here, "people.csv", "First Name\r\nGrace\r\n");
        w.act(QStringLiteral("mail.preview"))->trigger();
        auto *pane = w.findChild<write::MailPreviewPane *>();
        QVERIFY(pane);
        QTRY_VERIFY(pane->shown());
        QVERIFY(textOf(*pane->shown()).startsWith(QStringLiteral("Grace")));
        QCOMPARE(mail::stateIfAny(*w.document())->sourcePath, far);   // (the link in the file is as it was)
    }

    void theTextOfEachMessageIsTheMergedText()
    {
        auto doc = open(makeDocx(para(run("Dear ") + field("MERGEFIELD First_Name", "x") + run(",")) + para(run("Bye"))));
        QVERIFY(doc);
        MergeSource m = people();
        m.include[1] = false;
        const QStringList texts = mail::mergeToTexts(*doc, m, {}, mail::MergeOptions());
        QCOMPARE(texts, (QStringList{QStringLiteral("Dear Ada,\nBye"), QStringLiteral("Dear ,\nBye")}));
    }

    void aMessageFileIsMadeForEachRecipientWhoHasAnAddress()
    {
        MergeSource m;
        m.fields = {"Name", "Email"};
        m.rows = {{"Ada", "ada@example.com"}, {"Grace", ""}, {"Linus", "linus@example.org\r\nBcc: x@example.net"}, {"Edsger", "ed@example.net"}};
        m.include = QVector<bool>(4, true);
        QTemporaryDir dir;
        const QStringList texts = {"Dear Ada,\nHello \u00E9", "Dear Grace", "Dear Linus", "Dear Edsger"};
        QString error;
        const int written = mail::writeEmailFiles(texts, m, {0, 1, 2, 3}, QStringLiteral("Email"), QStringLiteral("Hi, \u00E9"), dir.filePath(QStringLiteral("out")), &error);
        QCOMPARE(written, 2);   // (no address, and an address with a line break that would add a header, make none)
        const QStringList files = QDir(dir.filePath(QStringLiteral("out"))).entryList(QDir::Files, QDir::Name);
        QCOMPARE(files.size(), 2);
        QVERIFY(files[0].startsWith(QStringLiteral("00001-ada@example.com")) && files[0].endsWith(QStringLiteral(".eml")));
        QFile f(dir.filePath(QStringLiteral("out/") + files[0]));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray eml = f.readAll();
        QVERIFY(eml.startsWith("To: ada@example.com\r\n"));
        QVERIFY(eml.contains("Subject: =?UTF-8?B?" + QByteArray("Hi, \xC3\xA9").toBase64() + "?="));
        const QByteArray body = eml.mid(eml.indexOf("\r\n\r\n") + 4);
        QCOMPARE(QByteArray::fromBase64(body), QByteArray("Dear Ada,\nHello \xC3\xA9"));
        QVERIFY(!eml.contains("Bcc"));
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
