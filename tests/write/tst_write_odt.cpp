// Tests of JeffWrite opening and saving OpenDocument text (.odt): the window's File > Open and Save As, and the document's reader and writer. Every file is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "../stablesaves.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/fonts.h"
#include "core/pictureops.h"
#include "core/styleops.h"
#include "core/tableops.h"
#include "core/writedoc.h"
#include "odt/odtread.h"
#include "io/zip.h"
#include "odt/odt.h"
#include "office/package.h"
#include "text/textengine.h"

#include <QBuffer>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] =
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"";

QByteArray makeOdt(const QByteArray &body, const QByteArray &autoStyles = QByteArray())
{
    jp::ZipWriter z;
    z.add(QStringLiteral("mimetype"), QByteArray("application/vnd.oasis.opendocument.text"), false);
    z.add(QStringLiteral("content.xml"),
          QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-content") + kNs + " office:version=\"1.2\"><office:automatic-styles>" + autoStyles +
              "</office:automatic-styles><office:body><office:text>" + body + "</office:text></office:body></office:document-content>",
          true);
    z.add(QStringLiteral("styles.xml"),
          QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-styles") + kNs + " office:version=\"1.2\"><office:styles><style:style style:name=\"Standard\" style:family=\"paragraph\"/>"
              "<style:style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:default-outline-level=\"1\"><style:text-properties fo:font-size=\"20pt\"/></style:style></office:styles>"
              "<office:automatic-styles/><office:master-styles/></office:document-styles>",
          true);
    return z.finish();
}

const QByteArray kBody =
    "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">A heading</text:h><text:p>Plain <text:span text:style-name=\"T1\">bold words</text:span> and more.</text:p>"
    "<text:list text:style-name=\"L1\"><text:list-item><text:p>first item</text:p></text:list-item><text:list-item><text:p>second item</text:p></text:list-item></text:list>"
    "<table:table><table:table-column table:number-columns-repeated=\"2\"/><table:table-row><table:table-cell><text:p>cell a</text:p></table:table-cell><table:table-cell><text:p>cell b</text:p></table:table-cell></table:table-row></table:table>"
    "<text:p>Last paragraph.</text:p>";
const QByteArray kAuto = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\"/></style:style>"
                         "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\" style:num-suffix=\".\"/></text:list-style>";

bool hasBold(const WriteDoc &d, const QString &text)
{
    for (QTextBlock b = d.story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().text().contains(text) && it.fragment().charFormat().fontWeight() > 500) return true;
    return false;
}

QString writeFile(const QString &path, const QByteArray &bytes)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) return QString();
    return path;
}

QByteArray readAll(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

} // namespace

class WriteOdtTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // The run's settings are its own, and none reached the person's (checked at the end).
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }

    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    void anOdtOpensWithItsTextAndLooks()
    {
        QString err;
        QStringList notes;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto), &err, &notes);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY(notes.isEmpty());
        const QString text = doc->story()->toPlainText();
        for (const char *w : {"A heading", "Plain bold words and more.", "first item", "second item", "cell a", "cell b", "Last paragraph."}) QVERIFY2(text.contains(QLatin1String(w)), w);
        QVERIFY(hasBold(*doc, QStringLiteral("bold words")));
        QVERIFY(!hasBold(*doc, QStringLiteral("Plain")));
    }

    // The markup of a paragraph's look is made once for each style, not for each paragraph: five thousand paragraphs of two styles make some looks, not thousands. (Counted.)
    void theLooksOfParagraphsAreMadeOnceForEachStyle()
    {
        QByteArray body;
        for (int i = 0; i < 5000; ++i) body += i % 2 ? "<text:p text:style-name=\"Standard\">Plain <text:span text:style-name=\"T1\">bold</text:span> words.</text:p>" : "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">Heading</text:h>";
        const qint64 before = jp::office::odt::Reader::looksMadeForTest();
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(body, kAuto), &err, nullptr);
        QVERIFY2(doc, qPrintable(err));
        const qint64 made = jp::office::odt::Reader::looksMadeForTest() - before;
        QVERIFY2(made <= 40, qPrintable(QStringLiteral("%1 looks made for 5000 paragraphs").arg(made)));
    }

    void fileOpenFindsAnOdtByItsContent()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // (The name says nothing: the mimetype does.)
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.dat")), makeOdt(kBody, kAuto));
        WriteWindow w;
        QStringList notices;
        w.setNotifier([&](const QString &, const QString &text) { notices << text; });
        QVERIFY(w.openFile(path));
        QVERIFY(w.document()->story()->toPlainText().contains(QLatin1String("second item")));
        QVERIFY(notices.isEmpty());
        QVERIFY(!w.isModified());
        QVERIFY(w.document()->pager().pageCount() >= 1);
        // A damaged one is refused with a reason.
        QString err;
        QVERIFY(!WriteDoc::fromFile(makeOdt(kBody, kAuto).left(300), &err));
        QVERIFY(!err.isEmpty());
    }

    void saveAsOffersAnOdtAndWritesOne()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.odt")), makeOdt(kBody, kAuto));
        WriteWindow a;
        QStringList said;
        a.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(a.openFile(path));
        QString suggested, filter;
        a.setSavePathChooser([&](const QString &s, const QString &f) {
            suggested = s;
            filter = f;
            return dir.filePath(QStringLiteral("copy.odt"));
        });
        QVERIFY2(a.saveAs(), qPrintable(said.join(QLatin1Char('\n'))));
        // The dialog started on the kind the file is, with the suggested name in that ending.
        QVERIFY2(suggested.endsWith(QLatin1String("letter.odt")), qPrintable(suggested));
        QVERIFY2(filter.contains(QLatin1String("*.odt")), qPrintable(filter));
        QVERIFY(a.filePath().endsWith(QLatin1String("copy.odt")));
        QVERIFY(!a.isModified());
        QByteArray bytes;
        {
            // Closed before the next save: Windows refuses to replace a file another handle holds open.
            QFile f(a.filePath());
            QVERIFY(f.open(QIODevice::ReadOnly));
            bytes = f.readAll();
        }
        QVERIFY(office::odt::looksLikeOdt(bytes));
        QVERIFY(bytes.startsWith("PK"));
        // It opens again with the same text and looks, and Save writes an .odt again (it is a kind JeffWrite writes).
        WriteWindow b;
        QVERIFY(b.openFile(a.filePath()));
        QCOMPARE(b.document()->story()->toPlainText(), a.document()->story()->toPlainText());
        QVERIFY(hasBold(*b.document(), QStringLiteral("bold words")));
        const qint64 before = QFileInfo(a.filePath()).size();
        QVERIFY2(a.save(), qPrintable(said.join(QLatin1Char('\n'))));
        QCOMPARE(QFileInfo(a.filePath()).size(), before);
        QFile again(a.filePath());
        QVERIFY(again.open(QIODevice::ReadOnly));
        QVERIFY(office::odt::looksLikeOdt(again.readAll()));
    }

    void anOdtMadeFromADocxKeepsItsHeadingListAndTable()
    {
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto), &err);
        QVERIFY(doc);
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->story()->toPlainText(), doc->story()->toPlainText());
        QVERIFY(hasBold(*back, QStringLiteral("bold words")));
        // The same document's .docx is the same too: what is written is what was read.
        const QByteArray docx1 = doc->toDocx(), docx2 = back->toDocx();
        office::Package p1, p2;
        QVERIFY(p1.read(docx1) && p2.read(docx2));
        QCOMPARE(p1.part(QStringLiteral("/word/document.xml")).count("<w:tc>"), p2.part(QStringLiteral("/word/document.xml")).count("<w:tc>"));
        QCOMPARE(p1.part(QStringLiteral("/word/document.xml")).count("<w:numId"), p2.part(QStringLiteral("/word/document.xml")).count("<w:numId"));
    }

    void theMacroNoticeNamesTheFormatSavedTo()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body><w:p><w:r><w:t>with macros</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml"));
        w.addPart(QStringLiteral("/word/vbaProject.bin"), QByteArray("VBA-project"), QStringLiteral("application/vnd.ms-office.vbaProject"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdVba"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject"), QStringLiteral("/word/vbaProject.bin"));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("macros.docm")), w.finish());
        WriteWindow win;
        QStringList said;
        win.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(win.openFile(path));
        QVERIFY(win.document()->hadMacros());
        QVERIFY(win.saveTo(dir.filePath(QStringLiteral("plain.odt"))));
        QVERIFY(!said.isEmpty());
        QVERIFY2(said[0].contains(QLatin1String("macros")) && said[0].contains(QLatin1String(".odt")) && !said[0].contains(QLatin1String(".docx")), qPrintable(said[0]));
    }

    // ---- Rich Text Format
    static QByteArray formattedDocx()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document " + ns + "><w:body>"
                  "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Big Title</w:t></w:r></w:p>"
                  "<w:p><w:pPr><w:jc w:val=\"center\"/><w:spacing w:before=\"120\" w:after=\"240\"/><w:ind w:left=\"720\"/></w:pPr>"
                  "<w:r><w:rPr><w:b/><w:i/><w:u w:val=\"single\"/></w:rPr><w:t>Bold caf\xC3\xA9 \xE2\x82\xAC</w:t></w:r>"
                  "<w:r><w:rPr><w:color w:val=\"FF0000\"/><w:sz w:val=\"28\"/><w:rFonts w:ascii=\"Courier New\" w:hAnsi=\"Courier New\"/><w:highlight w:val=\"yellow\"/></w:rPr><w:t xml:space=\"preserve\"> red {braces} \\ slash</w:t></w:r>"
                  "<w:r><w:tab/><w:t>after tab</w:t></w:r>"
                  "<w:hyperlink r:id=\"rIdLink\"><w:r><w:t>a link</w:t></w:r></w:hyperlink></w:p>"
                  "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>first item</w:t></w:r></w:p>"
                  "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>second item</w:t></w:r></w:p>"
                  "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/><w:left w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/>"
                  "<w:bottom w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/><w:right w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/><w:insideH w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/>"
                  "<w:insideV w:val=\"single\" w:sz=\"8\" w:color=\"000000\"/></w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                  "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"6000\" w:type=\"dxa\"/><w:gridSpan w:val=\"2\"/><w:shd w:val=\"clear\" w:fill=\"DDEEFF\"/></w:tcPr><w:p><w:r><w:t>joined</w:t></w:r></w:p></w:tc></w:tr>"
                  "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>left cell</w:t></w:r></w:p></w:tc>"
                  "<w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>right cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
                  "<w:p><w:pPr><w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdHead\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr></w:pPr>"
                  "<w:r><w:t>end of one</w:t></w:r></w:p>"
                  "<w:p><w:r><w:t>second section</w:t></w:r></w:p>"
                  "<w:sectPr><w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/>"
                  "<w:pgNumType w:fmt=\"lowerRoman\" w:start=\"3\"/></w:sectPr></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/styles.xml"),
                  "<?xml version=\"1.0\"?><w:styles " + ns + "><w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Georgia\" w:hAnsi=\"Georgia\"/><w:sz w:val=\"22\"/></w:rPr></w:rPrDefault></w:docDefaults>"
                  "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                  "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
                  "<w:pPr><w:keepNext/><w:outlineLvl w:val=\"0\"/></w:pPr><w:rPr><w:b/><w:sz w:val=\"36\"/></w:rPr></w:style></w:styles>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
        w.addPart(QStringLiteral("/word/numbering.xml"),
                  "<?xml version=\"1.0\"?><w:numbering " + ns + "><w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/>"
                  "<w:numFmt w:val=\"upperRoman\"/><w:lvlText w:val=\"%1)\"/><w:lvlJc w:val=\"left\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum>"
                  "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num></w:numbering>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
        w.addPart(QStringLiteral("/word/header1.xml"), "<?xml version=\"1.0\"?><w:hdr " + ns + "><w:p><w:r><w:t>Running head</w:t></w:r></w:p></w:hdr>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdStyles"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdNum"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdHead"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdLink"), rel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/page"), true);
        return w.finish();
    }

    void aDocumentIsSavedAsRichTextThatReadsBackWithItsLooks()
    {
        auto doc = write::WriteDoc::fromDocx(formattedDocx());
        QVERIFY(doc);
        QStringList notKept;
        QString err;
        const QByteArray rtf = doc->toRtf(&err, &notKept);
        QVERIFY2(!rtf.isEmpty(), qPrintable(err));
        QVERIFY(notKept.isEmpty());
        // the file as the format has it: the header words and tables, in order, braces balanced, and every character outside ASCII a \u number
        QVERIFY(rtf.startsWith("{\\rtf1\\ansi"));
        for (const char *word : {"\\fonttbl", "\\colortbl", "\\stylesheet", "\\listtable", "\\listoverridetable", "\\trowd", "\\cellx", "\\row", "\\cell", "\\sectd", "\\sect", "\\headerr", "\\fldinst HYPERLINK", "\\lndscpsxn", "\\pgnlcrm"})
            QVERIFY2(rtf.contains(word), word);
        QVERIFY2(rtf.contains("caf\\u233?") && rtf.contains("\\u8364?") && rtf.contains("{braces}") == false && rtf.contains("\\{braces\\}") && rtf.contains("\\\\ slash"), rtf.constData());
        int depth = 0;
        bool escaped = false;
        for (const char c : rtf) {
            if (escaped) {
                escaped = false;
                continue;
            }
            if (c == '\\') escaped = true;
            else if (c == '{') ++depth;
            else if (c == '}') QVERIFY(--depth >= 0);
        }
        QCOMPARE(depth, 0);
        for (const char c : rtf) QVERIFY(uchar(c) < 0x80);
        // read back: the words, the looks, the list, the table, the link, the sections
        auto back = write::WriteDoc::fromRtf(rtf, &err);
        QVERIFY2(back, qPrintable(err));
        const QString plain = back->story()->toPlainText();
        for (const QString &words : {QStringLiteral("Big Title"), QStringLiteral("café €"), QStringLiteral("red {braces} \\ slash"), QStringLiteral("after tab"), QStringLiteral("a link"),
                                     QStringLiteral("first item"), QStringLiteral("joined"), QStringLiteral("right cell"), QStringLiteral("second section")})
            QVERIFY2(plain.contains(words), qPrintable(words));
        office::Package p;
        QVERIFY(p.read(back->toDocx()));
        const QString xml = QString::fromUtf8(p.part(QStringLiteral("/word/document.xml")));
        if (qEnvironmentVariableIsSet("JW_DEBUG_RTF")) { QFile f(QStringLiteral("/tmp/w-ctx-home/out.rtf")); f.open(QIODevice::WriteOnly); f.write(rtf); QFile g(QStringLiteral("/tmp/w-ctx-home/out.xml")); g.open(QIODevice::WriteOnly); g.write(xml.toUtf8()); }
        for (const QString &needle : {QStringLiteral("<w:b/>"), QStringLiteral("<w:i/>"), QStringLiteral("w:val=\"FF0000\""), QStringLiteral("w:val=\"center\""), QStringLiteral("w:val=\"yellow\""), QStringLiteral("w:sz w:val=\"28\""),
                                      QStringLiteral("Courier New"), QStringLiteral("<w:tbl>"), QStringLiteral("w:gridSpan"), QStringLiteral("DDEEFF"), QStringLiteral("<w:numPr>"), QStringLiteral("w:orient=\"landscape\""),
                                      QStringLiteral("lowerRoman"), QStringLiteral("w:left=\"720\""), QStringLiteral("w:before=\"120\""), QStringLiteral("w:after=\"240\"")})
            QVERIFY2(xml.contains(needle), qPrintable(needle));
        QCOMPARE(back->sectionCount(), 2);
        QVERIFY(QString::fromUtf8(p.part(QStringLiteral("/word/numbering.xml"))).contains(QLatin1String("upperRoman")));
        QVERIFY(p.part(QStringLiteral("/word/document.xml")).contains("w:hyperlink") || xml.contains(QLatin1String("HYPERLINK")));
        // the heading style kept its name and look
        const QString styles = QString::fromUtf8(p.part(QStringLiteral("/word/styles.xml")));
        QVERIFY2(styles.contains(QLatin1String("heading 1"), Qt::CaseInsensitive), qPrintable(styles.left(400)));
        // the header of the first section
        bool header = false;
        for (int i = 0; i < 6 && !header; ++i) header = p.part(QStringLiteral("/word/header%1.xml").arg(i)).contains("Running head");
        QVERIFY(header);
        // and what was read is written again as the same words
        const QByteArray again = back->toRtf();
        auto third = write::WriteDoc::fromRtf(again, &err);
        QVERIFY(third);
        QCOMPARE(third->story()->toPlainText(), plain);
    }

    void aPictureIsSavedInRichTextAsAnImageThatReadsBack()
    {
        auto doc = write::WriteDoc::blank();
        QImage image(40, 20, QImage::Format_RGB32);
        image.fill(Qt::red);
        QByteArray png;
        QBuffer buffer(&png);
        QVERIFY(buffer.open(QIODevice::WriteOnly) && image.save(&buffer, "PNG"));
        QTextCursor c(doc->story());
        QCOMPARE(write::insertPicture(*doc, c, png), write::PictureResult::Ok);
        QStringList notKept;
        const QByteArray rtf = doc->toRtf(nullptr, &notKept);
        QVERIFY2(rtf.contains("\\pngblip") && rtf.contains(png.left(8).toHex()), rtf.left(600).constData());
        QVERIFY(notKept.isEmpty());
        auto back = write::WriteDoc::fromRtf(rtf);
        QVERIFY(back);
        office::Package p;
        QVERIFY(p.read(back->toDocx()));
        bool found = false;
        for (const QString &name : p.partNames()) found = found || (name.contains(QLatin1String("media/")) && p.part(name) == png);
        QVERIFY(found);
    }

    void richTextSaysWhatItCannotHold()
    {
        QStringList told;
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document " + ns + "><w:body><w:p><w:r><w:t>note here</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        auto doc = write::WriteDoc::fromDocx(w.finish());
        QVERIFY(doc);
        QStringList notKept;
        const QByteArray rtf = doc->toRtf(nullptr, &notKept);
        QVERIFY(rtf.contains("note here"));
        QVERIFY2(!notKept.isEmpty() && notKept.join(QLatin1Char(' ')).contains(QLatin1String("footnote")), qPrintable(notKept.join(QLatin1Char('|'))));
    }

    void aMacroFileSavedAsDocmKeepsItsProjectUntouched()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QString vbaRel = QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject");
        const QByteArray project = QByteArray("VBA-project \x00\x01\x02\xFF bytes that must come back as they went", 56);
        for (const bool tmpl : {false, true}) {
            office::PackageWriter w;
            w.addPart(QStringLiteral("/word/document.xml"),
                      "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body><w:p><w:r><w:t>with macros</w:t></w:r></w:p></w:body></w:document>",
                      tmpl ? QStringLiteral("application/vnd.ms-word.template.macroEnabledTemplate.main+xml") : QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml"));
            w.addPart(QStringLiteral("/word/vbaProject.bin"), project, QStringLiteral("application/vnd.ms-office.vbaProject"));
            w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdVba"), vbaRel, QStringLiteral("/word/vbaProject.bin"));
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            const QString ext = tmpl ? QStringLiteral("dotm") : QStringLiteral("docm");
            const QString path = writeFile(dir.filePath(QStringLiteral("macros.") + ext), w.finish());
            WriteWindow win;
            QStringList said;
            win.setNotifier([&](const QString &, const QString &text) { said << text; });
            QVERIFY(win.openFile(path, tmpl ? OpenHow::ReadOnly : OpenHow::Normal));
            QVERIFY(win.document()->hadMacros());
            const QString out = dir.filePath(QStringLiteral("saved.") + ext);
            QVERIFY(win.saveTo(out));
            QVERIFY2(said.isEmpty(), qPrintable(said.join(QLatin1Char('|'))));
            office::Package p;
            QVERIFY(p.read(readAll(out)));
            QCOMPARE(p.part(QStringLiteral("/word/vbaProject.bin")), project);
            QVERIFY(p.contentType(QStringLiteral("/word/document.xml")).contains(tmpl ? QLatin1String("macroEnabledTemplate.main+xml") : QLatin1String("macroEnabled.main+xml")));
            QCOMPARE(p.contentType(QStringLiteral("/word/vbaProject.bin")), QStringLiteral("application/vnd.ms-office.vbaProject"));
            QVERIFY(!p.relationshipsOfType(QStringLiteral("/word/document.xml"), QStringLiteral("/vbaProject")).isEmpty());
            // Reopened, it still has them; and saved as a .docx they are left out, said so.
            WriteWindow again;
            QStringList told;
            again.setNotifier([&](const QString &, const QString &text) { told << text; });
            QVERIFY(again.openFile(out, OpenHow::ReadOnly));
            QVERIFY(again.document()->hadMacros());
            if (tmpl) continue;
            QVERIFY(again.saveTo(dir.filePath(QStringLiteral("plain.docx")), true));
            office::Package q;
            QVERIFY(q.read(readAll(dir.filePath(QStringLiteral("plain.docx")))));
            QVERIFY(q.part(QStringLiteral("/word/vbaProject.bin")).isEmpty());
            QVERIFY(!told.isEmpty() && told[0].contains(QLatin1String(".docx")));
        }
    }

    void saveOnAFlatOdtExplainsAndOffersAnOdt()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // (The file is read by its content; its name says it is a flat one.)
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.fodt")), makeOdt(kBody, kAuto));
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(w.openFile(path));
        QString suggested, filter;
        w.setSavePathChooser([&](const QString &s, const QString &f) {
            suggested = s;
            filter = f;
            return dir.filePath(QStringLiteral("copy.odt"));
        });
        QVERIFY(w.save());
        QVERIFY2(!said.isEmpty() && said[0].contains(QLatin1String(".fodt")) && said[0].contains(QLatin1String(".odt file")), qPrintable(said.join(QLatin1Char('|'))));
        QVERIFY2(suggested.endsWith(QLatin1String("letter.odt")), qPrintable(suggested));
        QVERIFY2(filter.contains(QLatin1String("*.odt")), qPrintable(filter));
        QVERIFY(w.filePath().endsWith(QLatin1String("copy.odt")));
    }

    void theNotKeptNoteComesAgainForWhatIsNew()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter p;
        p.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><w:body>"
                  "<w:p><w:r><w:t>first</w:t></w:r><w:r><w:object/></w:r></w:p><w:p><w:r><w:t>second</w:t></w:r><m:oMath><m:r><m:t>x</m:t></m:r></m:oMath></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        p.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("two.docx")), p.finish());
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(w.openFile(path));
        // The equation goes first (so the first save has only the object to say), and comes back.
        QTextCursor c(w.document()->story());
        c.setPosition(w.document()->story()->findBlockByNumber(1).position() + 6);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QVERIFY(c.hasSelection());
        removeSelection(*w.document(), c);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("one.odt"))));
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QLatin1String("embedded objects")) && !said[0].contains(QLatin1String("equations")), qPrintable(said[0]));
        QVERIFY(w.document()->undo());
        said.clear();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("two.odt"))));
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QLatin1String("equations")) && !said[0].contains(QLatin1String("embedded objects")), qPrintable(said[0]));
        said.clear();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("three.odt"))));
        QVERIFY(said.isEmpty());
    }

    static QByteArray odtPart(const QByteArray &odt, const QString &name)
    {
        QMap<QString, QByteArray> parts;
        return jp::readZip(odt, parts) ? parts.value(name) : QByteArray();
    }

    // A formula in a table cell is saved as the field's expression and comes back as the same formula, with the result it showed.
    void aFormulaInATableCellSurvivesSavingAsOdt()
    {
        const QByteArray body = "<table:table><table:table-column table:number-columns-repeated=\"2\"/><table:table-row><table:table-cell><text:p>a</text:p></table:table-cell><table:table-cell><text:p>5</text:p></table:table-cell></table:table-row>"
                                "<table:table-row><table:table-cell><text:p>b</text:p></table:table-cell><table:table-cell><text:p>7</text:p></table:table-cell></table:table-row>"
                                "<table:table-row><table:table-cell><text:p>Total</text:p></table:table-cell><table:table-cell><text:p/></table:table-cell></table:table-row></table:table><text:p>after</text:p>";
        auto doc = WriteDoc::fromFile(makeOdt(body));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (b.text().isEmpty() && cellKeyOf(b)) c.setPosition(b.position());
        QVERIFY(insertField(*doc, c, QStringLiteral("= SUM(ABOVE) \\# \"0.00\"")));
        updateFields(*doc);
        auto docxText = [](WriteDoc &d) {
            office::Package pk;
            pk.read(d.toDocx());
            return pk.part(QStringLiteral("/word/document.xml"));
        };
        QVERIFY2(docxText(*doc).contains(">12.00<"), docxText(*doc).right(1500).constData());
        const QByteArray odt = doc->toOdt();
        QVERIFY(!odt.isEmpty());
        const QByteArray content = odtPart(odt, QStringLiteral("content.xml"));
        QVERIFY2(content.contains("text:expression") && content.contains("of:=SUM(ABOVE)"), content.right(1500).constData());
        auto back = WriteDoc::fromFile(odt);
        QVERIFY(back);
        const QByteArray x = docxText(*back);
        QVERIFY2(x.contains("SUM(ABOVE)") && x.contains(">12.00<"), x.right(1500).constData());
        // (and it can be updated again)
        updateFields(*back);
        QVERIFY2(docxText(*back).contains("SUM(ABOVE)") && docxText(*back).contains(">12.00<"), docxText(*back).right(1500).constData());   // (the number format came back too)
        QVERIFY(docxText(*back).contains("0.00"));
    }

    // What a table style gives the cells (the header row's fill, the bands) is written on the cells, so the table looks the same when the file is opened again.
    void aTableStylesFillsAreKeptOnTheCellsWhenSavingAsOdt()
    {
        auto doc = WriteDoc::fromFile(makeOdt("<table:table><table:table-column table:number-columns-repeated=\"2\"/>"
                                              "<table:table-row><table:table-cell><text:p>h1</text:p></table:table-cell><table:table-cell><text:p>h2</text:p></table:table-cell></table:table-row>"
                                              "<table:table-row><table:table-cell><text:p>x</text:p></table:table-cell><table:table-cell><text:p>y</text:p></table:table-cell></table:table-row>"
                                              "<table:table-row><table:table-cell><text:p>z</text:p></table:table-cell><table:table-cell><text:p>w</text:p></table:table-cell></table:table-row></table:table><text:p>after</text:p>"));
        QVERIFY(doc);
        office::wml::TableStyleSpec spec;
        spec.name = QStringLiteral("Red Head");
        spec.headerFill = QStringLiteral("FF0000");
        spec.bandedRowFill = QStringLiteral("00FF00");
        QString problem;
        const QString id = createTableStyle(*doc, spec, &problem);
        QVERIFY2(!id.isEmpty(), qPrintable(problem));
        QTextCursor c(doc->story());
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (b.text() == QLatin1String("h1")) c.setPosition(b.position());
        setTableStyle(*doc, c, id);
        const QByteArray odt = doc->toOdt();
        const QByteArray content = odtPart(odt, QStringLiteral("content.xml")).toLower();
        QVERIFY(content.contains("fo:background-color=\"#ff0000\""));   // (a style the two header cells share)
        QVERIFY(content.contains("fo:background-color=\"#00ff00\""));       // (the banded row)
        auto back = WriteDoc::fromFile(odt);
        QVERIFY(back);
        office::Package p;
        QVERIFY(p.read(back->toDocx()));
        QCOMPARE(int(p.part(QStringLiteral("/word/document.xml")).toUpper().count("W:FILL=\"FF0000\"")), 2);
    }

    // The text looks a table style gives the header row (bold, a color) are written on its cells' words, and the other rows' words have none.
    void aTableStylesTextLooksAreKeptOnTheWordsWhenSavingAsOdt()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ns = " xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        auto cell = [](const char *word) { return QByteArray("<w:tc><w:tcPr><w:tcW w:w=\"2400\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>") + word + "</w:t></w:r></w:p></w:tc>"; };
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document" + ns + "><w:body><w:tbl><w:tblPr><w:tblStyle w:val=\"BoldHead\"/><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2400\"/><w:gridCol w:w=\"2400\"/></w:tblGrid>"
                  "<w:tr>" + cell("h1") + cell("h2") + "</w:tr><w:tr>" + cell("x") + cell("y") + "</w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/styles.xml"),
                  "<?xml version=\"1.0\"?><w:styles" + ns + "><w:style w:type=\"table\" w:styleId=\"BoldHead\"><w:name w:val=\"Bold Head\"/><w:tblPr/>"
                  "<w:tblStylePr w:type=\"firstRow\"><w:rPr><w:b/><w:color w:val=\"0000FF\"/></w:rPr></w:tblStylePr></w:style></w:styles>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        auto doc = WriteDoc::fromFile(w.finish());
        QVERIFY(doc);
        auto back = WriteDoc::fromFile(doc->toOdt());
        QVERIFY(back);
        office::Package p;
        QVERIFY(p.read(back->toDocx()));
        const QByteArray x = p.part(QStringLiteral("/word/document.xml"));
        auto runOf = [&](const char *word) {
            const qsizetype at = x.indexOf(QByteArray(">") + word + "<");
            const qsizetype start = std::max(x.lastIndexOf("<w:r>", at), x.lastIndexOf("<w:r ", at));
            return x.mid(int(start), int(at - start));
        };
        for (const char *head : {"h1", "h2"}) {
            QVERIFY2(runOf(head).contains("<w:b/>") || runOf(head).contains("<w:b "), runOf(head).constData());
            QVERIFY2(runOf(head).contains("0000FF"), runOf(head).constData());
        }
        for (const char *body : {"x", "y"}) QVERIFY2(!runOf(body).contains("<w:b/>") && !runOf(body).contains("0000FF"), runOf(body).constData());
    }

    // Not a test of its own: the work of a process that the next function starts.
    void savesAnOdtForTheChecks()
    {
        QString out;
        if (!stable::childOutput(&out)) return;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto));
        QVERIFY(doc);
        const QByteArray odt = doc->toOdt();
        QVERIFY(!odt.isEmpty());
        QFile f(out);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QCOMPARE(f.write(odt), odt.size());
    }

    // The file is the same bytes each time it is saved, in whatever process.
    void aSavedOdtIsTheSameBytesInEachProcess() { JP_VERIFY_SAME_SAVES("savesAnOdtForTheChecks"); }
};

QTEST_MAIN(WriteOdtTests)
#include "tst_write_odt.moc"
