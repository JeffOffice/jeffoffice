// Tests of libs/office/odt: an OpenDocument text read into the parts of a .docx and a .docx written as an OpenDocument text. Every document is built here from a few lines of XML.

#include "io/zip.h"
#include "io/zipindex.h"
#include "odt/odt.h"
#include "odt/odtcommon.h"
#include "office/package.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QElapsedTimer>
#include <QRegularExpression>
#include <QTest>

using namespace jp::office;
using namespace jp::office::odt;

namespace {

const char kNs[] =
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
    " xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\""
    " xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"";

const char kMime[] = "application/vnd.oasis.opendocument.text";

// An .odt: `body` goes in office:text, `autoStyles` in content.xml's automatic styles, `styles` in styles.xml's common styles, `masters` and `layouts` in its master pages and page layouts.
struct Made {
    QByteArray body, autoStyles, styles, masters, layouts, meta;
    QMap<QString, QByteArray> extra;
    QByteArray mime = kMime;
    QByteArray build() const
    {
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), mime, false);
        z.add(QStringLiteral("content.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-content") + kNs + " office:version=\"1.2\"><office:automatic-styles>" + autoStyles +
                  "</office:automatic-styles><office:body><office:text>" + body + "</office:text></office:body></office:document-content>",
              true);
        z.add(QStringLiteral("styles.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-styles") + kNs + " office:version=\"1.2\"><office:styles>" + styles + "</office:styles><office:automatic-styles>" + layouts +
                  "</office:automatic-styles><office:master-styles>" + masters + "</office:master-styles></office:document-styles>",
              true);
        if (!meta.isEmpty()) z.add(QStringLiteral("meta.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-meta") + kNs + " office:version=\"1.2\"><office:meta>" + meta + "</office:meta></office:document-meta>", true);
        z.add(QStringLiteral("META-INF/manifest.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\"/>"), true);
        for (auto it = extra.cbegin(); it != extra.cend(); ++it) z.add(it.key(), it.value(), false);
        return z.finish();
    }
};

const QByteArray kPng = QByteArray::fromBase64("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==");

// The .docx the reader makes, and one of its parts.
QByteArray docxOf(const QByteArray &odt, QStringList *notes = nullptr, QString *error = nullptr)
{
    return toDocxPackage(odt, error, notes);
}

QString part(const QByteArray &docx, const QString &name)
{
    Package p;
    if (!p.read(docx)) return QString();
    return QString::fromUtf8(p.part(name));
}

QString document(const Made &m, QStringList *notes = nullptr)
{
    const QByteArray docx = docxOf(m.build(), notes);
    return part(docx, QStringLiteral("/word/document.xml"));
}

bool wellFormed(const QByteArray &xml)
{
    wml::XmlTree t;
    return t.parse(xml, nullptr);
}

// The first element of the XML with this local name, found by walking the tree (-1 for none): for checks of what a value is, not how the file spells it.
int findElement(const wml::XmlTree &t, const char *local, int from = -1)
{
    for (int i = from + 1; i < t.count(); ++i)
        if (t.localName(i) == local) return i;
    return -1;
}

QString instructionsOf(const QString &documentXml)
{
    QString all;
    wml::XmlTree t;
    if (!t.parse(documentXml.toUtf8(), nullptr)) return all;
    for (int i = findElement(t, "instrText"); i >= 0; i = findElement(t, "instrText", i)) all += t.text(i) + QLatin1Char('\n');
    return all;
}

} // namespace

class OdtTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void detectsByContent()
    {
        Made m;
        m.body = "<text:p>x</text:p>";
        QVERIFY(looksLikeOdt(m.build()));
        Made sheet = m;
        sheet.mime = "application/vnd.oasis.opendocument.spreadsheet";
        QVERIFY(!looksLikeOdt(sheet.build()));
        QString error;
        QVERIFY(docxOf(sheet.build(), nullptr, &error).isEmpty());
        QVERIFY(error.contains(QLatin1String("not a text document")));
        QVERIFY(!looksLikeOdt(QByteArray("plain text")));
        // A package with parts of the other kind is no OpenDocument.
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"/>", QStringLiteral("application/xml"));
        QVERIFY(!looksLikeOdt(w.finish()));
        // The mimetype may be deflated or not the first entry: the directory says.
        jp::ZipWriter z;
        z.add(QStringLiteral("content.xml"), "<x/>", true);
        z.add(QStringLiteral("mimetype"), kMime, true);
        QVERIFY(looksLikeOdt(z.finish()));
        QVERIFY(looksLikeFlatOdt(QByteArray("<?xml version=\"1.0\"?><office:document xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" office:mimetype=\"application/vnd.oasis.opendocument.text\">")));
        QVERIFY(!looksLikeFlatOdt(QByteArray("<office:document office:mimetype=\"application/vnd.oasis.opendocument.spreadsheet\">")));
    }

    void paragraphsAndHeadings()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>"
                   "<style:style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:default-outline-level=\"1\">"
                   "<style:paragraph-properties fo:keep-with-next=\"always\"/><style:text-properties fo:font-size=\"16pt\" fo:font-weight=\"bold\"/></style:style>";
        m.body = "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">Title</text:h><text:p text:style-name=\"Standard\">Body &amp; text</text:p><text:h text:outline-level=\"2\">Sub</text:h>";
        const QString d = document(m);
        QVERIFY(d.contains(QLatin1String("<w:pStyle w:val=\"Heading1\"/>")));
        QVERIFY(d.contains(QLatin1String("Body &amp; text")));
        // A heading with no style of its own keeps its level.
        QVERIFY(d.contains(QLatin1String("<w:outlineLvl w:val=\"1\"/>")));
        const QString s = part(docxOf(m.build()), QStringLiteral("/word/styles.xml"));
        QVERIFY(s.contains(QLatin1String("w:styleId=\"Heading1\"")));
        QVERIFY(s.contains(QLatin1String("<w:name w:val=\"heading 1\"/>")));
        QVERIFY(s.contains(QLatin1String("<w:outlineLvl w:val=\"0\"/>")));
        QVERIFY(s.contains(QLatin1String("<w:keepNext/>")));
        QVERIFY(s.contains(QLatin1String("<w:sz w:val=\"32\"/>")));
        QVERIFY(s.contains(QLatin1String("w:styleId=\"Normal\"")));
    }

    void spansAndProperties()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\" fo:font-style=\"italic\" fo:color=\"#ff0000\" style:text-underline-style=\"solid\" "
                       "style:text-underline-type=\"single\" fo:font-size=\"14pt\" style:font-name=\"Arial\" fo:background-color=\"#ffff00\" style:text-position=\"super 58%\"/></style:style>"
                       "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:text-align=\"center\" fo:margin-top=\"0.1in\" fo:margin-left=\"0.5in\" fo:text-indent=\"-0.25in\" "
                       "fo:line-height=\"150%\" fo:background-color=\"#00ff00\" fo:border-bottom=\"0.5pt solid #000000\" fo:break-before=\"page\"/><style:text-properties fo:font-size=\"20pt\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">plain <text:span text:style-name=\"T1\">marked <text:span text:style-name=\"T1\">deep</text:span></text:span></text:p>";
        const QString d = document(m);
        QVERIFY(d.contains(QLatin1String("<w:jc w:val=\"center\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:spacing w:before=\"144\"")));
        QVERIFY(d.contains(QLatin1String("w:line=\"360\" w:lineRule=\"auto\"")));
        QVERIFY(d.contains(QLatin1String("<w:ind w:left=\"720\" w:hanging=\"360\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:pageBreakBefore/>")));
        QVERIFY(d.contains(QLatin1String("<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"00FF00\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:pBdr><w:bottom w:val=\"single\" w:sz=\"4\"")));
        // The run of plain text has the paragraph's size, and the marked one adds its own.
        QVERIFY(d.contains(QLatin1String("<w:sz w:val=\"40\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:b/>")));
        QVERIFY(d.contains(QLatin1String("<w:i/>")));
        QVERIFY(d.contains(QLatin1String("<w:color w:val=\"FF0000\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:u w:val=\"single\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:sz w:val=\"28\"/>")));
        QVERIFY(d.contains(QLatin1String("w:ascii=\"Arial\"")));
        QVERIFY(d.contains(QLatin1String("<w:vertAlign w:val=\"superscript\"/>")));
        QVERIFY(d.contains(QLatin1String("w:fill=\"FFFF00\"")));
        // The result is a document the .docx reader takes.
        wml::Document doc;
        QVERIFY(wml::readDocx(docxOf(m.build()), &doc));
        QVERIFY(!doc.body.isEmpty());
    }

    void whitespaceTabsAndBreaks()
    {
        Made m;
        m.body = "<text:p>  a   b<text:s text:c=\"3\"/>c<text:tab/>d<text:line-break/>e\n   f<text:soft-page-break/>g</text:p>";
        const QString d = document(m);
        // (Leading space and runs of it collapse; text:s is kept.)
        QVERIFY(d.contains(QLatin1String(">a b</w:t>")));
        QVERIFY(d.contains(QLatin1String(">   </w:t>")));
        QVERIFY(d.contains(QLatin1String("<w:tab/>")));
        QVERIFY(d.contains(QLatin1String("<w:br/>")));
        QVERIFY(d.contains(QLatin1String(">e f</w:t>")));
        QVERIFY(d.contains(QLatin1String(">g</w:t>")));
        QVERIFY(!d.contains(QLatin1String("soft-page")));
    }

    void styleInheritanceAndLoops()
    {
        Made m;
        m.styles = "<style:style style:name=\"A\" style:family=\"paragraph\" style:parent-style-name=\"B\"><style:text-properties fo:font-weight=\"bold\"/></style:style>"
                   "<style:style style:name=\"B\" style:family=\"paragraph\" style:parent-style-name=\"A\"><style:text-properties fo:font-style=\"italic\"/></style:style>"
                   "<style:style style:name=\"C\" style:family=\"paragraph\" style:parent-style-name=\"Missing\"/>";
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"A\"><style:paragraph-properties fo:text-align=\"end\"/></style:style>"
                       "<style:style style:name=\"P2\" style:family=\"paragraph\" style:parent-style-name=\"P3\"/><style:style style:name=\"P3\" style:family=\"paragraph\" style:parent-style-name=\"P2\"/>";
        m.body = "<text:p text:style-name=\"P1\">x</text:p><text:p text:style-name=\"P2\">y</text:p><text:p text:style-name=\"C\">z</text:p>";
        QElapsedTimer timer;
        timer.start();
        const QString d = document(m);
        QVERIFY(timer.elapsed() < 5000);
        QVERIFY(d.contains(QLatin1String("<w:pStyle w:val=\"A\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:jc w:val=\"right\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:pStyle w:val=\"C\"/>")));
        const QString s = part(docxOf(m.build()), QStringLiteral("/word/styles.xml"));
        QVERIFY(s.contains(QLatin1String("<w:basedOn w:val=\"B\"/>")));
        QVERIFY(!s.contains(QLatin1String("<w:basedOn w:val=\"Missing\"/>")));
        // A chain of parents is as long as the standard bound.
        Made deep;
        for (int i = 0; i < 200; ++i) deep.styles += "<style:style style:name=\"S" + QByteArray::number(i) + "\" style:family=\"paragraph\" style:parent-style-name=\"S" + QByteArray::number(i + 1) + "\"/>";
        deep.body = "<text:p text:style-name=\"S0\">x</text:p>";
        QVERIFY(!document(deep).isEmpty());
    }

    void listsAndNumbering()
    {
        Made m;
        m.autoStyles = "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\" style:num-suffix=\".\"><style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
                       "<style:list-level-label-alignment text:label-followed-by=\"listtab\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.5in\"/></style:list-level-properties></text:list-level-style-number>"
                       "<text:list-level-style-number text:level=\"2\" style:num-format=\"a\" style:num-prefix=\"(\" style:num-suffix=\")\" text:start-value=\"3\"/></text:list-style>"
                       "<text:list-style style:name=\"L2\"><text:list-level-style-bullet text:level=\"1\" text:bullet-char=\"\xE2\x80\xA2\"/></text:list-style>";
        m.body = "<text:list text:style-name=\"L1\"><text:list-item><text:p>one</text:p><text:list><text:list-item><text:p>inner</text:p></text:list-item></text:list></text:list-item><text:list-item><text:p>two</text:p></text:list-item></text:list>"
                 "<text:p>between</text:p>"
                 "<text:list text:style-name=\"L1\"><text:list-item><text:p>again</text:p></text:list-item></text:list>"
                 "<text:list text:style-name=\"L1\" text:continue-numbering=\"true\"><text:list-item><text:p>goes on</text:p></text:list-item></text:list>"
                 "<text:list text:style-name=\"L2\"><text:list-item><text:p>dot</text:p></text:list-item></text:list>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml")), n = part(docx, QStringLiteral("/word/numbering.xml"));
        QVERIFY(d.contains(QLatin1String("<w:ilvl w:val=\"1\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:lvlText w:val=\"%1.\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:numFmt w:val=\"lowerLetter\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:lvlText w:val=\"(%2)\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:start w:val=\"3\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:ind w:left=\"720\" w:hanging=\"360\"/>")));
        QVERIFY(n.contains(QLatin1String("<w:numFmt w:val=\"bullet\"/>")));
        // The second list of the same style begins again: a number of its own with the start given; the third goes on with the second's number.
        QRegularExpression ids(QStringLiteral("<w:numId w:val=\"(\\d+)\"/>"));
        QStringList used;
        for (auto it = ids.globalMatch(d); it.hasNext();) used << it.next().captured(1);
        QVERIFY(used.size() >= 5);
        QCOMPARE(used[0], used[1]);   // one and inner: one list
        QVERIFY(used[0] != used[3]);   // again: a new count
        QCOMPARE(used[3], used[4]);   // goes on
        QVERIFY(n.contains(QLatin1String("<w:startOverride")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    void tables()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"Table1\" style:family=\"table\"><style:table-properties style:width=\"4in\" table:align=\"center\"/></style:style>"
                       "<style:style style:name=\"Table1.A\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1in\"/></style:style>"
                       "<style:style style:name=\"Table1.B\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"3in\"/></style:style>"
                       "<style:style style:name=\"Table1.A1\" style:family=\"table-cell\"><style:table-cell-properties fo:background-color=\"#cccccc\" fo:border=\"0.5pt solid #000000\" style:vertical-align=\"middle\"/></style:style>"
                       "<style:style style:name=\"Table1.1\" style:family=\"table-row\"><style:table-row-properties style:min-row-height=\"0.5in\"/></style:style>";
        m.body = "<table:table table:style-name=\"Table1\"><table:table-column table:style-name=\"Table1.A\"/><table:table-column table:style-name=\"Table1.B\" table:number-columns-repeated=\"2\"/>"
                 "<table:table-header-rows><table:table-row table:style-name=\"Table1.1\"><table:table-cell table:style-name=\"Table1.A1\" table:number-columns-spanned=\"2\" table:number-rows-spanned=\"2\"><text:p>wide</text:p></table:table-cell>"
                 "<table:covered-table-cell/><table:table-cell><text:p>c</text:p></table:table-cell></table:table-row></table:table-header-rows>"
                 "<table:table-row><table:covered-table-cell table:number-columns-repeated=\"2\"/><table:table-cell><table:table><table:table-column/><table:table-row><table:table-cell><text:p>nested</text:p></table:table-cell></table:table-row></table:table>"
                 "</table:table-cell></table:table-row></table:table><text:p>after</text:p>";
        const QString d = document(m);
        QVERIFY(d.contains(QLatin1String("<w:tblGrid><w:gridCol w:w=\"1440\"/><w:gridCol w:w=\"4320\"/><w:gridCol w:w=\"4320\"/></w:tblGrid>")));
        QVERIFY(d.contains(QLatin1String("<w:jc w:val=\"center\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:gridSpan w:val=\"2\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:vMerge w:val=\"restart\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:vMerge/>")));
        QVERIFY(d.contains(QLatin1String("<w:tblHeader/>")));
        QVERIFY(d.contains(QLatin1String("<w:trHeight w:val=\"720\" w:hRule=\"atLeast\"/>")));
        QVERIFY(d.contains(QLatin1String("w:fill=\"CCCCCC\"")));
        QVERIFY(d.contains(QLatin1String("<w:tcBorders><w:top w:val=\"single\"")));
        QVERIFY(d.contains(QLatin1String("<w:vAlign w:val=\"center\"/>")));
        // The nested table is in a cell, and the cell ends in a paragraph.
        QCOMPARE(d.count(QLatin1String("<w:tbl>")), 2);
        QVERIFY(d.contains(QLatin1String("nested</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/></w:tc>")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docxOf(m.build()), &doc));
    }

    void pageLayoutAndSections()
    {
        Made m;
        m.layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" style:print-orientation=\"portrait\" fo:margin-top=\"0.5in\" fo:margin-bottom=\"0.5in\" "
                    "fo:margin-left=\"1in\" fo:margin-right=\"1in\"/><style:header-style><style:header-footer-properties fo:min-height=\"0.4in\" fo:margin-bottom=\"0.1in\"/></style:header-style></style:page-layout>"
                    "<style:page-layout style:name=\"pm2\"><style:page-layout-properties fo:page-width=\"11in\" fo:page-height=\"8.5in\" style:print-orientation=\"landscape\" fo:margin=\"0.75in\"/></style:page-layout>";
        m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"><style:header><text:p>H</text:p></style:header></style:master-page>"
                    "<style:master-page style:name=\"Wide\" style:page-layout-name=\"pm2\"/>";
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\" style:master-page-name=\"Wide\"/>"
                       "<style:style style:name=\"Sect1\" style:family=\"section\"><style:section-properties><style:columns fo:column-count=\"2\" fo:column-gap=\"0.25in\"/></style:section-properties></style:style>";
        m.body = "<text:p>first</text:p><text:p text:style-name=\"P1\">landscape</text:p><text:section text:style-name=\"Sect1\"><text:p>in columns</text:p></text:section><text:p>last</text:p>";
        const QString d = document(m);
        // The first section ends in the paragraph before the one that changes the page; the one after it ends there too, at the end of its columns.
        QVERIFY(d.contains(QLatin1String("<w:pgSz w:w=\"12240\" w:h=\"15840\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/>")));
        QVERIFY(d.contains(QLatin1String("w:left=\"1080\"")));
        // (The top margin is the page's plus the header and the space after it.)
        QVERIFY(d.contains(QLatin1String("w:top=\"1440\"")));
        QVERIFY(d.contains(QLatin1String("w:header=\"720\"")));
        QVERIFY(d.contains(QLatin1String("<w:cols w:num=\"2\" w:space=\"360\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:type w:val=\"continuous\"/>")));
        QVERIFY(d.count(QLatin1String("<w:sectPr>")) >= 4);
        wml::Document doc;
        QVERIFY(wml::readDocx(docxOf(m.build()), &doc));
    }

    void headersAndFooters()
    {
        Made m;
        m.layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" fo:margin-top=\"1in\" fo:margin-bottom=\"1in\" fo:margin-left=\"1in\" fo:margin-right=\"1in\"/></style:page-layout>";
        m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"><style:header><text:p>right header <text:page-number>1</text:page-number></text:p></style:header>"
                    "<style:header-left><text:p>left header</text:p></style:header-left><style:header-first><text:p>first header</text:p></style:header-first>"
                    "<style:footer><text:p>footer <text:page-count>3</text:page-count></text:p></style:footer></style:master-page>";
        m.body = "<text:p>body</text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<w:headerReference w:type=\"default\"")));
        QVERIFY(d.contains(QLatin1String("<w:headerReference w:type=\"even\"")));
        QVERIFY(d.contains(QLatin1String("<w:headerReference w:type=\"first\"")));
        QVERIFY(d.contains(QLatin1String("<w:footerReference w:type=\"default\"")));
        QVERIFY(d.contains(QLatin1String("<w:titlePg/>")));
        QVERIFY(part(docx, QStringLiteral("/word/settings.xml")).contains(QLatin1String("<w:evenAndOddHeaders/>")));
        Package p;
        QVERIFY(p.read(docx));
        QString all;
        for (const QString &n : p.partNames())
            if (n.contains(QLatin1String("header")) || n.contains(QLatin1String("footer"))) all += QString::fromUtf8(p.part(n));
        QVERIFY(all.contains(QLatin1String("right header")));
        QVERIFY(all.contains(QLatin1String("left header")));
        QVERIFY(all.contains(QLatin1String("first header")));
        QVERIFY(all.contains(QLatin1String("PAGE")));
        QVERIFY(all.contains(QLatin1String("NUMPAGES")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    void notesAndComments()
    {
        Made m;
        m.body = "<text:p>a<text:note text:id=\"ftn1\" text:note-class=\"footnote\"><text:note-citation>1</text:note-citation><text:note-body><text:p>foot text</text:p></text:note-body></text:note>"
                 "b<text:note text:id=\"edn1\" text:note-class=\"endnote\"><text:note-citation>i</text:note-citation><text:note-body><text:p>end text</text:p></text:note-body></text:note>"
                 "<office:annotation office:name=\"c1\"><dc:creator>Ann</dc:creator><dc:date>2024-01-02T03:04:05</dc:date><text:p>remark</text:p></office:annotation>spanned<office:annotation-end office:name=\"c1\"/>c</text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<w:footnoteReference w:id=\"1\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:endnoteReference w:id=\"1\"/>")));
        QVERIFY(part(docx, QStringLiteral("/word/footnotes.xml")).contains(QLatin1String("foot text")));
        QVERIFY(part(docx, QStringLiteral("/word/endnotes.xml")).contains(QLatin1String("end text")));
        const QString c = part(docx, QStringLiteral("/word/comments.xml"));
        QVERIFY(c.contains(QLatin1String("w:author=\"Ann\"")));
        QVERIFY(c.contains(QLatin1String("remark")));
        QVERIFY(d.contains(QLatin1String("<w:commentRangeStart")));
        QVERIFY(d.contains(QLatin1String("<w:commentRangeEnd")));
        QVERIFY(d.indexOf(QLatin1String("<w:commentRangeStart")) < d.indexOf(QLatin1String("spanned")));
        QVERIFY(d.indexOf(QLatin1String("spanned")) < d.indexOf(QLatin1String("<w:commentRangeEnd")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    void fieldsBookmarksAndContents()
    {
        Made m;
        m.body = "<text:table-of-content><text:table-of-content-source text:outline-level=\"2\"/><text:index-body><text:index-title><text:p>Contents</text:p></text:index-title>"
                 "<text:p>One<text:tab/>1</text:p><text:p>Two<text:tab/>2</text:p></text:index-body></text:table-of-content>"
                 "<text:p><text:bookmark-start text:name=\"here\"/>marked<text:bookmark-end text:name=\"here\"/><text:bookmark text:name=\"point\"/> "
                 "<text:bookmark-ref text:ref-name=\"here\" text:reference-format=\"text\">marked</text:bookmark-ref> <text:bookmark-ref text:ref-name=\"here\" text:reference-format=\"page\">1</text:bookmark-ref> "
                 "<text:title>T</text:title> <text:sequence text:name=\"Figure\" style:num-format=\"1\">1</text:sequence> <text:a xlink:href=\"http://example.com/x\">link</text:a> <text:a xlink:href=\"#here\">inner</text:a></text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("TOC \\o \"1-2\" \\h \\z \\u")));
        QVERIFY(d.indexOf(QLatin1String("fldCharType=\"begin\"")) < d.indexOf(QLatin1String(">One<")));
        QVERIFY(d.lastIndexOf(QLatin1String("fldCharType=\"end\"")) > d.indexOf(QLatin1String(">Two<")));
        QVERIFY(d.contains(QLatin1String("<w:bookmarkStart w:id=\"1\" w:name=\"here\"/>")));
        QVERIFY(d.contains(QLatin1String("w:name=\"point\"")));
        QVERIFY(d.contains(QLatin1String("REF \"here\" \\h")));
        QVERIFY(d.contains(QLatin1String("PAGEREF \"here\" \\h")));
        QVERIFY(d.contains(QLatin1String("TITLE")));
        QVERIFY(d.contains(QLatin1String("SEQ \"Figure\"")));
        QVERIFY(d.contains(QLatin1String("<w:hyperlink r:id=")));
        QVERIFY(d.contains(QLatin1String("<w:hyperlink w:anchor=\"here\"")));
        Package p;
        QVERIFY(p.read(docx));
        bool external = false;
        for (const Relationship &r : p.relationships(QStringLiteral("/word/document.xml")))
            if (r.external && r.target == QLatin1String("http://example.com/x")) external = true;
        QVERIFY(external);
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    void picturesAndTextBoxes()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"fr1\" style:family=\"graphic\"><style:graphic-properties style:wrap=\"parallel\" style:horizontal-pos=\"right\" style:horizontal-rel=\"page\" style:vertical-pos=\"top\" style:vertical-rel=\"paragraph\"/></style:style>";
        m.body = "<text:p>pic <draw:frame draw:name=\"P\" text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"0.5in\"><draw:image xlink:href=\"Pictures/a.png\"/><svg:desc>alt words</svg:desc></draw:frame>"
                 "<draw:frame draw:style-name=\"fr1\" text:anchor-type=\"paragraph\" svg:x=\"1in\" svg:y=\"0.5in\" svg:width=\"2in\" svg:height=\"1in\"><draw:text-box><text:p>in the box</text:p></draw:text-box></draw:frame>"
                 "<draw:frame text:anchor-type=\"paragraph\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/missing.png\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<wp:inline")));
        QVERIFY(d.contains(QLatin1String("<wp:extent cx=\"914400\" cy=\"457200\"/>")));
        QVERIFY(d.contains(QLatin1String("descr=\"alt words\"")));
        QVERIFY(d.contains(QLatin1String("<wp:anchor")));
        QVERIFY(d.contains(QLatin1String("<wp:wrapSquare wrapText=\"bothSides\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:txbxContent>")));
        QVERIFY(d.contains(QLatin1String("in the box")));
        Package p;
        QVERIFY(p.read(docx));
        bool media = false;
        for (const QString &n : p.partNames())
            if (n.startsWith(QLatin1String("/word/media/")) && p.part(n) == kPng) media = true;
        QVERIFY(media);
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
    }

    void trackedChangesAreKept()
    {
        Made m;
        m.body = "<text:tracked-changes><text:changed-region text:id=\"c1\"><text:deletion><office:change-info><dc:creator>Ann</dc:creator><dc:date>2024-05-06T07:08:09</dc:date></office:change-info><text:p>gone text</text:p></text:deletion></text:changed-region>"
                 "<text:changed-region text:id=\"c2\"><text:insertion><office:change-info><dc:creator>Bob</dc:creator></office:change-info></text:insertion></text:changed-region></text:tracked-changes>"
                 "<text:p>keep <text:change-start text:change-id=\"c2\"/>added<text:change-end text:change-id=\"c2\"/><text:change text:change-id=\"c1\"/> end</text:p>";
        const QByteArray docx = docxOf(m.build());
        const QString d = part(docx, QStringLiteral("/word/document.xml"));
        QVERIFY(d.contains(QLatin1String("<w:ins w:id=\"1\" w:author=\"Bob\">")));
        QVERIFY(d.contains(QRegularExpression(QStringLiteral("<w:ins [^>]*><w:r><w:t xml:space=\"preserve\">added</w:t></w:r></w:ins>"))));
        QVERIFY(d.contains(QLatin1String("<w:del w:id=\"2\" w:author=\"Ann\" w:date=\"2024-05-06T07:08:09Z\">")));
        QVERIFY(d.contains(QLatin1String("<w:delText xml:space=\"preserve\">gone text</w:delText>")));
        wml::Document doc;
        QVERIFY(wml::readDocx(docx, &doc));
        // Written back, they are tracked changes of the .odt again.
        const QByteArray odt = fromDocxPackage(docx);
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        QVERIFY(c.contains(QLatin1String("<text:tracked-changes>")));
        QVERIFY(c.contains(QLatin1String("<text:insertion>")));
        QVERIFY(c.contains(QLatin1String("<text:deletion>")));
        QVERIFY(c.contains(QLatin1String("gone text")));
        QVERIFY(c.contains(QLatin1String("<text:change-start")));
        QVERIFY(c.contains(QLatin1String("<text:change text:change-id")));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
    }

    void flatForm()
    {
        const QByteArray flat = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document") + kNs + " office:version=\"1.2\" office:mimetype=\"application/vnd.oasis.opendocument.text\">"
                                "<office:styles><style:style style:name=\"Standard\" style:family=\"paragraph\"/></office:styles><office:automatic-styles/><office:body><office:text><text:p>flat words</text:p></office:text></office:body>"
                                "<office:meta><dc:title>Flat</dc:title></office:meta></office:document>";
        QVERIFY(looksLikeOdt(flat));
        const QByteArray docx = docxOf(flat);
        QVERIFY(part(docx, QStringLiteral("/word/document.xml")).contains(QLatin1String("flat words")));
        QVERIFY(part(docx, QStringLiteral("/docProps/core.xml")).contains(QLatin1String("Flat")));
    }

    void propertiesOfTheDocument()
    {
        Made m;
        m.body = "<text:p>x</text:p>";
        m.meta = "<dc:title>My title</dc:title><meta:initial-creator>Ann</meta:initial-creator><dc:subject>Sub</dc:subject><meta:keyword>k1</meta:keyword><meta:creation-date>2024-01-02T03:04:05</meta:creation-date>";
        const QString core = part(docxOf(m.build()), QStringLiteral("/docProps/core.xml"));
        QVERIFY(core.contains(QLatin1String("<dc:title>My title</dc:title>")));
        QVERIFY(core.contains(QLatin1String("<dc:creator>Ann</dc:creator>")));
        QVERIFY(core.contains(QLatin1String("<dcterms:created xsi:type=\"dcterms:W3CDTF\">2024-01-02T03:04:05Z")));
    }

    void tabStopsAndBreaks()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:break-after=\"page\"><style:tab-stops><style:tab-stop style:position=\"2in\" style:type=\"right\" "
                       "style:leader-style=\"dotted\" style:leader-text=\".\"/><style:tab-stop style:position=\"3in\"/></style:tab-stops></style:paragraph-properties></style:style>";
        m.body = "<text:p text:style-name=\"P1\">a<text:tab/>b</text:p><text:p>next</text:p><text:p>third</text:p>";
        const QString d = document(m);
        QVERIFY(d.contains(QLatin1String("<w:tab w:val=\"right\" w:leader=\"dot\" w:pos=\"2880\"/>")));
        QVERIFY(d.contains(QLatin1String("<w:tab w:val=\"left\" w:pos=\"4320\"/>")));
        // The page break after the first paragraph begins the page of the next.
        const int next = d.indexOf(QLatin1String(">next<"));
        QVERIFY(next > 0);
        QVERIFY(d.lastIndexOf(QLatin1String("<w:pageBreakBefore/>"), next) > d.indexOf(QLatin1String(">a<")));
        QVERIFY(d.indexOf(QLatin1String("<w:pageBreakBefore/>"), d.indexOf(QLatin1String(">next<"))) < 0);
    }

    // ---- hostile input

    void refusesWhatItShould()
    {
        Made m;
        m.body = "<text:p>x</text:p>";
        QByteArray odt = m.build();
        // Truncated at every tenth of its length.
        for (int i = 1; i < 10; ++i) {
            QString error;
            const QByteArray cut = odt.left(odt.size() * i / 10);
            QVERIFY(docxOf(cut, nullptr, &error).isEmpty());
        }
        // A DOCTYPE (an entity) is refused.
        Made dt = m;
        QByteArray content = QByteArray("<?xml version=\"1.0\"?><!DOCTYPE office:document-content [<!ENTITY e \"boom\">]><office:document-content") + kNs + "><office:body><office:text><text:p>&e;</text:p></office:text></office:body></office:document-content>";
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), kMime, false);
        z.add(QStringLiteral("content.xml"), content, true);
        QString error;
        QVERIFY(docxOf(z.finish(), nullptr, &error).isEmpty());
        // A document with parts that are encrypted.
        jp::ZipWriter z2;
        z2.add(QStringLiteral("mimetype"), kMime, false);
        z2.add(QStringLiteral("content.xml"), "<x/>", true);
        z2.add(QStringLiteral("META-INF/manifest.xml"), "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\"><manifest:file-entry manifest:full-path=\"content.xml\"><manifest:encryption-data/></manifest:file-entry></manifest:manifest>", true);
        QVERIFY(docxOf(z2.finish(), nullptr, &error).isEmpty());
        QVERIFY(error.contains(QLatin1String("password")));
        // More parts than a package may have.
        jp::ZipWriter z3;
        z3.add(QStringLiteral("mimetype"), kMime, false);
        z3.add(QStringLiteral("content.xml"), "<x/>", true);
        for (int i = 0; i < 10100; ++i) z3.add(QStringLiteral("Pictures/%1.png").arg(i), "x", false);
        QVERIFY(docxOf(z3.finish(), nullptr, &error).isEmpty());
        // Nothing that is not XML.
        jp::ZipWriter z4;
        z4.add(QStringLiteral("mimetype"), kMime, false);
        z4.add(QStringLiteral("content.xml"), "not xml at all", true);
        QVERIFY(docxOf(z4.finish(), nullptr, &error).isEmpty());
    }

    void boundsWhatAFileCanMultiply()
    {
        // Counts of repeats are cut: a million columns and rows of nothing take no time.
        Made m;
        m.body = "<table:table><table:table-column table:number-columns-repeated=\"1000000000\"/><table:table-row table:number-rows-repeated=\"1000000000\"><table:table-cell table:number-columns-repeated=\"1000000000\"><text:p>x</text:p></table:table-cell></table:table-row></table:table>";
        QElapsedTimer timer;
        timer.start();
        const QByteArray docx = docxOf(m.build());
        QVERIFY(timer.elapsed() < 20000);
        QVERIFY(!docx.isEmpty());
        QVERIFY(document(m).size() < 40 * 1024 * 1024);
        // Nesting: spans, lists, and tables a few thousand deep read in bounded time, or are refused, never crash.
        Made nest;
        QByteArray inner = "x";
        for (int i = 0; i < 3000; ++i) inner = "<text:span>" + inner + "</text:span>";
        nest.body = "<text:p>" + inner + "</text:p>";
        timer.restart();
        document(nest);
        QVERIFY(timer.elapsed() < 20000);
        // Lists and tables nested as deep as the XML reader allows (about 250 levels) or a little more: read in bounded time or refused, never a crash.
        for (const int depth : {30, 70, 300}) {
            Made lists;
            QByteArray inner = "<text:p>x</text:p>";
            for (int i = 0; i < depth; ++i) inner = "<text:list><text:list-item>" + inner + "</text:list-item></text:list>";
            lists.body = inner;
            timer.restart();
            const QByteArray l = docxOf(lists.build());
            QVERIFY(timer.elapsed() < 20000);
            if (depth <= 70) QVERIFY(!l.isEmpty());
            Made tables;
            inner = "<text:p>x</text:p>";
            for (int i = 0; i < depth; ++i) inner = "<table:table><table:table-column/><table:table-row><table:table-cell>" + inner + "</table:table-cell></table:table-row></table:table>";
            tables.body = inner;
            timer.restart();
            const QByteArray tdocx = docxOf(tables.build());
            QVERIFY(timer.elapsed() < 20000);
            if (depth <= 30) QVERIFY(!tdocx.isEmpty());
        }
        // A numbered list used many times makes a bounded number of numberings.
        Made many;
        many.autoStyles = "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\"/></text:list-style>";
        for (int i = 0; i < 30000; ++i) many.body += "<text:list text:style-name=\"L1\"><text:list-item><text:p>x</text:p></text:list-item></text:list>";
        timer.restart();
        const QString numbering = part(docxOf(many.build()), QStringLiteral("/word/numbering.xml"));
        QVERIFY(timer.elapsed() < 20000);
        QVERIFY(numbering.count(QLatin1String("<w:num ")) <= kMaxMadeParts + 1);
    }

    void mutatedFilesDoNotCrash()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>";
        m.autoStyles = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\"/></style:style>";
        m.body = "<text:h text:outline-level=\"1\">T</text:h><text:p>a <text:span text:style-name=\"T1\">b</text:span></text:p><table:table><table:table-column/><table:table-row><table:table-cell><text:p>c</text:p></table:table-cell></table:table-row></table:table>"
                 "<text:list><text:list-item><text:p>l</text:p></text:list-item></text:list>";
        // The parts as XML text, mutated byte by byte (a byte replaced, a piece cut, a piece doubled), and packed again.
        const QString content = QStringLiteral("content.xml");
        jp::ZipEntry dummy;
        Q_UNUSED(dummy);
        Package source;
        QVERIFY(source.readOpenDocument(m.build()));
        const QByteArray xml = source.part(QStringLiteral("/content.xml"));
        unsigned seed = 12345;
        auto next = [&] { seed = seed * 1103515245u + 12345u; return (seed >> 8) & 0xFFFFFF; };
        int opened = 0, refused = 0;
        for (int i = 0; i < 300; ++i) {
            QByteArray mutated = xml;
            const int at = int(next() % uint(mutated.size()));
            switch (i % 3) {
            case 0: mutated[at] = char(next() & 0xFF); break;
            case 1: mutated.remove(at, int(next() % 40)); break;
            default: mutated.insert(at, mutated.mid(at, int(next() % 60))); break;
            }
            jp::ZipWriter z;
            z.add(QStringLiteral("mimetype"), kMime, false);
            z.add(content, mutated, true);
            z.add(QStringLiteral("styles.xml"), source.part(QStringLiteral("/styles.xml")), true);
            QString error;
            const QByteArray docx = docxOf(z.finish(), nullptr, &error);
            if (docx.isEmpty()) ++refused;
            else {
                ++opened;
                wml::Document doc;
                wml::readDocx(docx, &doc);   // (whatever it made, the .docx reader takes without crashing)
            }
        }
        QVERIFY(opened + refused == 300);
        QVERIFY(opened > 0);
    }

    // ---- writing

    void writesAValidPackage()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>";
        m.body = "<text:h text:outline-level=\"1\">Head</text:h><text:p>x<draw:frame text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        QString error;
        QStringList notes;
        const QByteArray odt = fromDocxPackage(docx, &error, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(error));
        QVector<jp::ZipEntry> entries;
        QVERIFY(jp::readZipDirectory(odt, &entries));
        QCOMPARE(entries.first().name, QStringLiteral("mimetype"));
        QCOMPARE(int(entries.first().method), 0);
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        QCOMPARE(files.value(QStringLiteral("mimetype")), QByteArray(kMime));
        for (const QString &n : {QStringLiteral("content.xml"), QStringLiteral("styles.xml"), QStringLiteral("meta.xml"), QStringLiteral("META-INF/manifest.xml")}) {
            QVERIFY2(files.contains(n), qPrintable(n));
            QVERIFY2(wellFormed(files.value(n)), qPrintable(n));
        }
        // The manifest lists every part but itself and the mimetype.
        const QString manifest = QString::fromUtf8(files.value(QStringLiteral("META-INF/manifest.xml")));
        for (auto it = files.cbegin(); it != files.cend(); ++it) {
            if (it.key() == QLatin1String("mimetype") || it.key() == QLatin1String("META-INF/manifest.xml")) continue;
            QVERIFY2(manifest.contains(QLatin1String("manifest:full-path=\"") + it.key() + QLatin1String("\"")), qPrintable(it.key()));
        }
        QVERIFY(files.contains(QStringLiteral("Pictures/image1.png")));
        QCOMPARE(files.value(QStringLiteral("Pictures/image1.png")), kPng);
        // The settings say how the text is laid out, with the prefix they name the set by declared (another program ignores the set without it).
        {
            wml::XmlTree st;
            QVERIFY(st.parse(files.value(QStringLiteral("settings.xml")), nullptr));
            QVERIFY(st.namespaceId("http://openoffice.org/2004/office") >= 0);
            bool found = false;
            for (int i = findElement(st, "config-item"); i >= 0 && !found; i = findElement(st, "config-item", i))
                found = st.attrAny(i, "name") == QLatin1String("AddParaTableSpacing") && st.text(i) == QLatin1String("false");
            QVERIFY(found);
        }
        QVERIFY(QString::fromUtf8(files.value(QStringLiteral("content.xml"))).contains(QLatin1String("<text:h")));
        // And it reads again.
        QVERIFY(looksLikeOdt(odt));
        QVERIFY(!docxOf(odt).isEmpty());
    }

    void roundTripKeepsWhatItHolds()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/>"
                   "<style:style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:default-outline-level=\"1\"><style:text-properties fo:font-size=\"16pt\"/></style:style>";
        m.autoStyles = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\" fo:color=\"#ff0000\"/></style:style>"
                       "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:text-align=\"center\" fo:margin-left=\"0.5in\"/></style:style>"
                       "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\" style:num-suffix=\".\"/></text:list-style>";
        m.layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" fo:margin-top=\"1in\" fo:margin-bottom=\"1in\" fo:margin-left=\"1in\" fo:margin-right=\"1in\"/></style:page-layout>";
        m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"><style:header><text:p>head text</text:p></style:header><style:footer><text:p>page <text:page-number>1</text:page-number></text:p></style:footer></style:master-page>";
        m.body = "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">Heading words</text:h><text:p text:style-name=\"P1\">plain <text:span text:style-name=\"T1\">bold red</text:span> tail</text:p>"
                 "<text:list text:style-name=\"L1\"><text:list-item><text:p>item one</text:p></text:list-item><text:list-item><text:p>item two</text:p></text:list-item></text:list>"
                 "<table:table><table:table-column table:number-columns-repeated=\"2\"/><table:table-row><table:table-cell table:number-columns-spanned=\"2\"><text:p>spanned</text:p></table:table-cell><table:covered-table-cell/></table:table-row>"
                 "<table:table-row><table:table-cell><text:p>c1</text:p></table:table-cell><table:table-cell><text:p>c2</text:p></table:table-cell></table:table-row></table:table>"
                 "<text:p>note<text:note text:note-class=\"footnote\"><text:note-citation>1</text:note-citation><text:note-body><text:p>foot</text:p></text:note-body></text:note></text:p>";
        const QByteArray odt2 = fromDocxPackage(docxOf(m.build()));
        QVERIFY(!odt2.isEmpty());
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt2, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml"))), s = QString::fromUtf8(files.value(QStringLiteral("styles.xml")));
        QVERIFY(c.contains(QLatin1String("<text:h")));
        QVERIFY(c.contains(QLatin1String("text:outline-level=\"1\"")));
        QVERIFY(c.contains(QLatin1String("fo:font-weight=\"bold\"")));
        QVERIFY(c.contains(QLatin1String("fo:color=\"#FF0000\"")));
        QVERIFY(c.contains(QLatin1String("fo:text-align=\"center\"")));
        QVERIFY(c.contains(QLatin1String("<text:list text:style-name=\"L")));
        QVERIFY(c.contains(QLatin1String("style:num-format=\"1\"")));
        QVERIFY(c.contains(QLatin1String("table:number-columns-spanned=\"2\"")));
        QVERIFY(c.contains(QLatin1String("<table:covered-table-cell/>")));
        QVERIFY(c.contains(QLatin1String("<text:note ")));
        QVERIFY(c.contains(QLatin1String("foot")));
        QVERIFY(s.contains(QLatin1String("<style:header>")));
        QVERIFY(s.contains(QLatin1String("head text")));
        QVERIFY(s.contains(QLatin1String("<text:page-number")));
        QVERIFY(s.contains(QLatin1String("fo:page-width=\"8.5000in\"")));
        QVERIFY(s.contains(QLatin1String("style:default-outline-level=\"1\"")));
        // The text is as it was after the whole round trip.
        const QString again = part(docxOf(odt2), QStringLiteral("/word/document.xml"));
        for (const char *word : {"Heading words", "bold red", "item two", "spanned", "c2", "note"}) QVERIFY2(again.contains(QLatin1String(word)), word);
        QVERIFY(again.contains(QLatin1String("<w:b/>")));
        QVERIFY(again.contains(QLatin1String("gridSpan")));
    }

    void writesFieldsPicturesSectionsAndPageBreaks()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                          "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                          "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
        const QByteArray body =
            "<w:p><w:pPr><w:pStyle w:val=\"Title\"/><w:jc w:val=\"both\"/><w:spacing w:before=\"240\" w:after=\"120\" w:line=\"360\" w:lineRule=\"auto\"/><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr>"
            "<w:r><w:rPr><w:b/><w:i/><w:u w:val=\"single\"/><w:color w:val=\"FF0000\"/><w:sz w:val=\"28\"/></w:rPr><w:t xml:space=\"preserve\">Styled  words </w:t></w:r><w:r><w:tab/><w:t>tabbed</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>before break</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after break</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>Page </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
            "<w:r><w:t>7</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>9</w:t></w:r></w:fldSimple>"
            "<w:bookmarkStart w:id=\"3\" w:name=\"mark\"/><w:r><w:t>marked</w:t></w:r><w:bookmarkEnd w:id=\"3\"/><w:hyperlink r:id=\"rId9\"><w:r><w:t>site</w:t></w:r></w:hyperlink></w:p>"
            "<w:p><w:r><w:drawing><wp:inline><wp:extent cx=\"914400\" cy=\"457200\"/><wp:docPr id=\"1\" name=\"Picture 1\" descr=\"a picture\"/><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
            "<pic:pic><pic:blipFill><a:blip r:embed=\"rId8\"/></pic:blipFill></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>"
            "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr></w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>two</w:t></w:r></w:p>"
            "<w:sectPr><w:type w:val=\"continuous\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/><w:cols w:num=\"2\" w:space=\"720\"/></w:sectPr>";
        PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\"?><w:document ") + kW + "><w:body>" + body + "</w:body></w:document>", QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/media/image1.png"), kPng, QStringLiteral("image/png"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId8"), rel + QStringLiteral("image"), QStringLiteral("/word/media/image1.png"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId9"), rel + QStringLiteral("hyperlink"), QStringLiteral("http://example.org/"), true);
        QString error;
        const QByteArray odt = fromDocxPackage(w.finish(), &error);
        QVERIFY2(!odt.isEmpty(), qPrintable(error));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(c.contains(QLatin1String("fo:text-align=\"justify\"")));
        QVERIFY(c.contains(QLatin1String("fo:line-height=\"150%\"")));
        QVERIFY(c.contains(QLatin1String("fo:text-indent=\"-0.2500in\"")));
        QVERIFY(c.contains(QLatin1String("fo:font-size=\"14pt\"")));
        QVERIFY(c.contains(QLatin1String("style:text-underline-style=\"solid\"")));
        QVERIFY(c.contains(QLatin1String("Styled <text:s/>words")));
        QVERIFY(c.contains(QLatin1String("<text:tab/>tabbed")));
        // The page break begins a page in the paragraph after it.
        const int after = c.indexOf(QLatin1String(">after break<"));
        QVERIFY(after > 0);
        QVERIFY(c.contains(QLatin1String("fo:break-before=\"page\"")));
        QVERIFY(c.contains(QLatin1String("<text:page-number text:select-page=\"current\">7</text:page-number>")));
        QVERIFY(c.contains(QLatin1String("<text:page-count>9</text:page-count>")));
        QVERIFY(c.contains(QLatin1String("<text:bookmark-start text:name=\"mark\"/>")));
        QVERIFY(c.contains(QLatin1String("<text:bookmark-end text:name=\"mark\"/>")));
        QVERIFY(c.contains(QLatin1String("xlink:href=\"http://example.org/\"")));
        QVERIFY(c.contains(QLatin1String("<draw:frame")));
        QVERIFY(c.contains(QLatin1String("<svg:desc>a picture</svg:desc>")));
        QVERIFY(c.contains(QLatin1String("svg:width=\"1.0000in\"")));
        QVERIFY(c.contains(QLatin1String("<text:section")));
        QVERIFY(files.contains(QStringLiteral("Pictures/image1.png")));
        const QString s = QString::fromUtf8(files.value(QStringLiteral("styles.xml")));
        QVERIFY(s.contains(QLatin1String("<style:columns fo:column-count=\"2\"")) || c.contains(QLatin1String("<style:columns fo:column-count=\"2\"")));
        QVERIFY(wellFormed(files.value(QStringLiteral("styles.xml"))));
    }

    void writesTablesListsHeadersNotesAndComments()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        auto xml = [&](const char *root, const QByteArray &body) { return QByteArray("<?xml version=\"1.0\"?><w:") + root + " " + kW + ">" + body + "</w:" + root + ">"; };
        const QByteArray body =
            "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Chapter</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>step one</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"1\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>sub step</w:t></w:r></w:p>"
            "<w:p><w:r><w:t>interrupted</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>step two</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"2\"/></w:numPr></w:pPr><w:r><w:t>a bullet</w:t></w:r></w:p>"
            "<w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"pct\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"8\" w:color=\"FF0000\"/><w:insideH w:val=\"single\" w:sz=\"4\"/></w:tblBorders></w:tblPr>"
            "<w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
            "<w:tr><w:trPr><w:tblHeader/></w:trPr><w:tc><w:tcPr><w:vMerge w:val=\"restart\"/><w:shd w:val=\"clear\" w:fill=\"DDEEFF\"/></w:tcPr><w:p><w:r><w:t>tall</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>b1</w:t></w:r></w:p></w:tc></w:tr>"
            "<w:tr><w:tc><w:tcPr><w:vMerge/></w:tcPr><w:p/></w:tc><w:tc><w:p><w:r><w:t>b2</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
            "<w:p><w:pPr><w:tabs><w:tab w:val=\"right\" w:pos=\"4320\"/></w:tabs></w:pPr><w:r><w:t>note here</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r>"
            "<w:commentRangeStart w:id=\"5\"/><w:r><w:t>commented</w:t></w:r><w:commentRangeEnd w:id=\"5\"/><w:r><w:commentReference w:id=\"5\"/></w:r>"
            "<w:hyperlink w:anchor=\"target\"><w:r><w:t>jump</w:t></w:r></w:hyperlink></w:p>"
            "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdH1\"/><w:headerReference w:type=\"first\" r:id=\"rIdH2\"/><w:footerReference w:type=\"default\" r:id=\"rIdF1\"/><w:titlePg/>"
            "<w:pgSz w:w=\"11906\" w:h=\"16838\" w:orient=\"portrait\"/><w:pgMar w:top=\"1700\" w:right=\"1000\" w:bottom=\"1700\" w:left=\"1200\" w:header=\"600\" w:footer=\"700\" w:gutter=\"0\"/></w:sectPr>";
        PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QString type = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");
        w.addPart(QStringLiteral("/word/document.xml"), xml("document", "<w:body>" + body + "</w:body>"), type + QStringLiteral("document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/styles.xml"),
                  xml("styles", "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                                "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:keepNext/><w:outlineLvl w:val=\"0\"/></w:pPr><w:rPr><w:b/><w:sz w:val=\"32\"/></w:rPr></w:style>"),
                  type + QStringLiteral("styles+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        w.addPart(QStringLiteral("/word/numbering.xml"),
                  xml("numbering", "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1)\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>"
                                   "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"lowerRoman\"/><w:lvlText w:val=\"%1.%2\"/></w:lvl></w:abstractNum>"
                                   "<w:abstractNum w:abstractNumId=\"1\"><w:lvl w:ilvl=\"0\"><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/></w:lvl></w:abstractNum>"
                                   "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>"),
                  type + QStringLiteral("numbering+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
        w.addPart(QStringLiteral("/word/settings.xml"), xml("settings", "<w:evenAndOddHeaders/>"), type + QStringLiteral("settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
        auto addPart = [&](const QString &name, const char *root, const QByteArray &inner, const QString &suffix, const QString &id) {
            w.addPart(name, xml(root, inner), type + suffix);
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), id, rel + suffix.left(suffix.indexOf(QLatin1Char('+'))), name);
        };
        addPart(QStringLiteral("/word/header1.xml"), "hdr", "<w:p><w:r><w:t>default header</w:t></w:r></w:p>", QStringLiteral("header+xml"), QStringLiteral("rIdH1"));
        addPart(QStringLiteral("/word/header2.xml"), "hdr", "<w:p><w:r><w:t>first page header</w:t></w:r></w:p>", QStringLiteral("header+xml"), QStringLiteral("rIdH2"));
        addPart(QStringLiteral("/word/footer1.xml"), "ftr", "<w:p><w:r><w:t xml:space=\"preserve\">page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple></w:p>", QStringLiteral("footer+xml"), QStringLiteral("rIdF1"));
        addPart(QStringLiteral("/word/footnotes.xml"), "footnotes", "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>the note text</w:t></w:r></w:p></w:footnote>", QStringLiteral("footnotes+xml"), QStringLiteral("rIdN"));
        addPart(QStringLiteral("/word/comments.xml"), "comments", "<w:comment w:id=\"5\" w:author=\"Cy\" w:initials=\"C\"><w:p><w:r><w:t>comment text</w:t></w:r></w:p></w:comment>", QStringLiteral("comments+xml"), QStringLiteral("rIdC"));
        QString error;
        const QByteArray odt = fromDocxPackage(w.finish(), &error);
        QVERIFY2(!odt.isEmpty(), qPrintable(error));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml"))), st = QString::fromUtf8(files.value(QStringLiteral("styles.xml")));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(wellFormed(files.value(QStringLiteral("styles.xml"))));
        // The heading, and its numbering of the style's own.
        QVERIFY(c.contains(QLatin1String("<text:h text:style-name=\"P1\" text:outline-level=\"1\">Chapter</text:h>")));
        // (The first paragraph names the master page, so its style is an automatic one that follows the heading's.)
        QVERIFY(c.contains(QLatin1String("style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"Heading_20_1\" style:master-page-name=\"MP1\"")));
        // Lists: the second run of the first list goes on (continue-numbering), nested items are a list in an item, and the bullet list is its own.
        QCOMPARE(c.count(QLatin1String("<text:list text:style-name=\"L1\"")), 2);
        QVERIFY(c.contains(QLatin1String("<text:list text:style-name=\"L1\" text:continue-numbering=\"true\">")));
        QVERIFY(c.contains(QRegularExpression(QStringLiteral("sub step</text:p>"))));
        QVERIFY(c.contains(QLatin1String("<text:list text:style-name=\"L2\">")));
        QVERIFY(c.contains(QLatin1String("style:num-suffix=\")\"")));
        QVERIFY(c.contains(QLatin1String("style:num-format=\"i\"")));
        QVERIFY(c.contains(QLatin1String("text:display-levels=\"2\"")));
        // The table: percent width, header row, merged cells, borders and fill.
        QVERIFY(c.contains(QLatin1String("style:rel-width=\"100%\"")));
        QVERIFY(c.contains(QLatin1String("<table:table-header-rows>")));
        QVERIFY(c.contains(QLatin1String("table:number-rows-spanned=\"2\"")));
        QVERIFY(c.contains(QLatin1String("<table:covered-table-cell/>")));
        QVERIFY(c.contains(QLatin1String("fo:background-color=\"#DDEEFF\"")));
        QVERIFY(c.contains(QLatin1String("fo:border-top=\"1.000pt solid #FF0000\"")));
        QVERIFY(c.contains(QLatin1String("fo:border-bottom=\"0.500pt solid #000000\"")));
        // The tab stop, the note, the comment, and the link.
        QVERIFY(c.contains(QLatin1String("<style:tab-stop style:position=\"3.0000in\" style:type=\"right\"/>")));
        QVERIFY(c.contains(QRegularExpression(QStringLiteral("<text:note text:id=\"ftn1\" text:note-class=\"footnote\"><text:note-citation>1</text:note-citation><text:note-body><text:p [^>]*>the note text</text:p>"))));
        QVERIFY(c.contains(QLatin1String("<office:annotation office:name=\"cmt5\">")));
        QVERIFY(c.contains(QLatin1String("<dc:creator>Cy</dc:creator>")));
        QVERIFY(c.contains(QLatin1String("<office:annotation-end office:name=\"cmt5\"/>")));
        QVERIFY(c.indexOf(QLatin1String("<office:annotation ")) < c.indexOf(QLatin1String("commented")) && c.indexOf(QLatin1String("commented")) < c.indexOf(QLatin1String("<office:annotation-end")));
        QVERIFY(c.contains(QLatin1String("xlink:href=\"#target\"")));
        // The page: A4, margins (the header and footer's room is the rest of the top and bottom margin), headers of three kinds, the footer's field.
        QVERIFY(st.contains(QLatin1String("fo:page-width=\"8.2681in\"")));
        QVERIFY(st.contains(QLatin1String("fo:margin-top=\"0.4167in\"")));
        QVERIFY(st.contains(QLatin1String("fo:margin-left=\"0.8333in\"")));
        QVERIFY(st.contains(QLatin1String("<style:header>")));
        QVERIFY(st.contains(QLatin1String("<style:header-left>")));
        QVERIFY(st.contains(QLatin1String("<style:header-first>")));
        QVERIFY(st.contains(QLatin1String("first page header")));
        QVERIFY(st.contains(QLatin1String("<style:footer>")));
        QVERIFY(st.contains(QLatin1String("<text:page-number text:select-page=\"current\">1</text:page-number>")));
        // The reader takes the file back as the same document.
        const QString again = part(docxOf(odt), QStringLiteral("/word/document.xml"));
        for (const char *word : {"Chapter", "step one", "sub step", "interrupted", "step two", "a bullet", "tall", "b2", "note here", "commented", "jump"}) QVERIFY2(again.contains(QLatin1String(word)), word);
        QVERIFY(again.contains(QLatin1String("<w:titlePg/>")));
    }

    // ---- what the review found

    void spacesAreBoundedForAParagraphAndTheDocument()
    {
        Made m;
        QByteArray para = "<text:p>";
        for (int i = 0; i < 1000; ++i) para += "<text:s text:c=\"10000\"/>";
        para += "x</text:p>";
        for (int i = 0; i < 100; ++i) m.body += para;   // (10 million spaces asked for in each paragraph, a billion in all)
        QElapsedTimer timer;
        timer.start();
        QStringList notes;
        const QString d = document(m, &notes);
        QVERIFY(timer.elapsed() < 30000);
        QVERIFY2(d.size() < 120 * 1024 * 1024, qPrintable(QString::number(d.size())));
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("spaces")));
    }

    void repeatedCellsAndRowsAreBoundedInBytes()
    {
        Made m;
        const QByteArray big(200 * 1024, 'w');
        m.body = "<table:table><table:table-column table:number-columns-repeated=\"1000\"/><table:table-row table:number-rows-repeated=\"200\"><table:table-cell table:number-columns-repeated=\"1000\"><text:p>" + big +
                 "</text:p></table:table-cell></table:table-row></table:table>";
        QElapsedTimer timer;
        timer.start();
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        QVERIFY(timer.elapsed() < 120000);
        QVERIFY(!docx.isEmpty());
        Package p;
        QVERIFY(p.read(docx));
        QVERIFY2(p.partSize(QStringLiteral("/word/document.xml")) < qint64(400) << 20, qPrintable(QString::number(p.partSize(QStringLiteral("/word/document.xml")))));
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("repeated")));
    }

    void repeatedCellsAloneAreBoundedInBytes()
    {
        // One row, so only the copies of the cell are what could multiply.
        Made m;
        const QByteArray big(600 * 1024, 'w');
        m.body = "<table:table><table:table-column table:number-columns-repeated=\"1000\"/><table:table-row><table:table-cell table:number-columns-repeated=\"1000\"><text:p>" + big + "</text:p></table:table-cell></table:table-row></table:table>";
        QStringList notes;
        const QByteArray docx = docxOf(m.build(), &notes);
        QVERIFY(!docx.isEmpty());
        Package p;
        QVERIFY(p.read(docx));
        QVERIFY2(p.partSize(QStringLiteral("/word/document.xml")) < qint64(400) << 20, qPrintable(QString::number(p.partSize(QStringLiteral("/word/document.xml")))));
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("repeated")));
    }

    void hugeLengthsAreClamped()
    {
        bool ok = false;
        QCOMPARE(lengthToTwips(QStringLiteral("1e300pt"), &ok), qint64(kMaxLengthPoints * 20));
        QVERIFY(ok);
        QCOMPARE(lengthToTwips(QStringLiteral("-1e300cm"), &ok), qint64(-kMaxLengthPoints * 20));
        QCOMPARE(lengthToEmu(QStringLiteral("1e300in"), &ok), qint64(kMaxLengthPoints * 12700));
        QCOMPARE(clampedInt(1e300), qint64(1e12));
        QCOMPARE(clampedInt(std::nan("")), qint64(0));
        Made m;
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:margin-left=\"1e300pt\" fo:text-indent=\"-1e300cm\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">x<draw:frame text:anchor-type=\"as-char\" svg:width=\"1e300in\" svg:height=\"1e300cm\"><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        QVERIFY(!docx.isEmpty());
        QVERIFY(wellFormed(part(docx, QStringLiteral("/word/document.xml")).toUtf8()));
        // And the other way: a .docx whose numbers are far past what a length can be.
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:pPr><w:ind w:left=\"1e300\" w:hanging=\"-1e300\"/><w:spacing w:before=\"1e999\"/></w:pPr><w:r><w:t>x</w:t></w:r></w:p><w:tbl><w:tblGrid><w:gridCol w:w=\"1e300\"/></w:tblGrid>"
                                                             "<w:tr><w:tc><w:p/></w:tc></w:tr></w:tbl><w:sectPr><w:pgSz w:w=\"1e300\" w:h=\"-1e300\"/></w:sectPr></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        const QByteArray odt = fromDocxPackage(w.finish());
        QVERIFY(!odt.isEmpty());
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(wellFormed(files.value(QStringLiteral("styles.xml"))));
    }

    void charactersXmlCannotHoldAreDroppedBothWays()
    {
        QCOMPARE(esc(QStringLiteral("a\x01" "b\x0b" "c")), QByteArray("abc"));
        QCOMPARE(esc(QString(QChar(0xD800)) + QStringLiteral("x") + QChar(0xDC00)), QByteArray("x"));
        QCOMPARE(esc(QStringLiteral("a\xF0\x9F\x98\x80"), false), QByteArray("a\xC3\xB0\xC2\x9F\xC2\x98\xC2\x80"));   // (Latin-1 text: its characters are kept)
        Made m;
        m.body = "<text:p><text:bookmark text:name=\"b&#1;m\"/><draw:frame draw:name=\"n&#2;&#xFFFE;\" text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame>"
                 "<text:a xlink:href=\"http://e.com/&#3;x\">l</text:a><office:annotation><dc:creator>A&#4;B</dc:creator><text:p>c</text:p></office:annotation>"
                 "<text:change-start text:change-id=\"c1\"/>t<text:change-end text:change-id=\"c1\"/></text:p>"
                 "<text:tracked-changes><text:changed-region text:id=\"c1\"><text:insertion><office:change-info><dc:creator>Z&#5;Z</dc:creator></office:change-info></text:insertion></text:changed-region></text:tracked-changes>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        QVERIFY(!docx.isEmpty());
        Package p;
        QVERIFY(p.read(docx));
        for (const QString &n : p.partNames())
            if (n.endsWith(QLatin1String(".xml"))) QVERIFY2(wellFormed(p.part(n)), qPrintable(n));
        // The writer, from a .docx whose names and authors hold them.
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:bookmarkStart w:id=\"1\" w:name=\"b&#1;m\"/><w:r><w:t>x</w:t></w:r><w:bookmarkEnd w:id=\"1\"/><w:ins w:id=\"2\" w:author=\"A&#2;B\"><w:r><w:t>i</w:t></w:r></w:ins>"
                                                            "<w:r><w:rPr><w:color w:val=\"12&quot; x=&quot;\"/></w:rPr><w:t>c</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        const QByteArray odt = fromDocxPackage(w.finish());
        QVERIFY(!odt.isEmpty());
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(odt, files));
        QVERIFY(wellFormed(files.value(QStringLiteral("content.xml"))));
        QVERIFY(!QString::fromUtf8(files.value(QStringLiteral("content.xml"))).contains(QLatin1String("x=")));   // (a color that is no hex adds no attribute)
    }

    void namesInFieldCodesCannotAddSwitches()
    {
        Made m;
        m.body = "<text:p><text:bookmark-ref text:ref-name=\"x&quot; \\* MERGEFORMAT \\h &quot;y\" text:reference-format=\"text\">r</text:bookmark-ref>"
                 "<text:sequence text:name=\"F&quot; \\r 9\">1</text:sequence></text:p>"
                 "<text:illustration-index><text:illustration-index-source text:caption-sequence-name=\"c&quot; \\o &quot;1-9\"/><text:index-body><text:p>e</text:p></text:index-body></text:illustration-index>";
        const QString instr = instructionsOf(document(m));
        QRegularExpression ref(QStringLiteral("^ REF \"(?:[^\"\\\\]|\\\\.)*\" \\\\h $"), QRegularExpression::MultilineOption);
        QVERIFY2(instr.contains(ref), qPrintable(instr));
        QRegularExpression seq(QStringLiteral("^ SEQ \"(?:[^\"\\\\]|\\\\.)*\" \\\\\\* ARABIC $"), QRegularExpression::MultilineOption);
        QVERIFY2(instr.contains(seq), qPrintable(instr));
        QRegularExpression toc(QStringLiteral("^ TOC \\\\h \\\\z \\\\c \"(?:[^\"\\\\]|\\\\.)*\" $"), QRegularExpression::MultilineOption);
        QVERIFY2(instr.contains(toc), qPrintable(instr));
    }

    void everyLinkStaysALinkAsWritten()
    {
        const QStringList hrefs = {QStringLiteral("http://example.com/a"), QStringLiteral("mailto:a@b.c"), QStringLiteral("javascript:alert(1)"), QStringLiteral("file:///etc/passwd"),
                                   QStringLiteral("vbscript:x"), QStringLiteral("../other.odt")};
        Made m;
        m.body = "<text:p>";
        for (const QString &h : hrefs) m.body += "<text:a xlink:href=\"" + h.toUtf8() + "\">l</text:a>";
        m.body += "<draw:frame text:anchor-type=\"as-char\" svg:width=\"1in\" svg:height=\"1in\"><draw:image xlink:href=\"../Pictures/a.png\"/></draw:frame></text:p>";
        m.extra.insert(QStringLiteral("Pictures/a.png"), kPng);
        const QByteArray docx = docxOf(m.build());
        Package p;
        QVERIFY(p.read(docx));
        QStringList targets;
        for (const Relationship &r : p.relationships(QStringLiteral("/word/document.xml")))
            if (r.external) targets << r.target;
        QStringList want = hrefs;
        targets.sort();
        want.sort();
        QCOMPARE(targets, want);
        QCOMPARE(part(docx, QStringLiteral("/word/document.xml")).count(QLatin1String("<w:hyperlink ")), hrefs.size());
        QVERIFY(!part(docx, QStringLiteral("/word/document.xml")).contains(QLatin1String("<wp:inline")));   // (a picture named by a path that climbs out is not read)
        // And back: the .odt holds each as it was.
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(docx), files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        for (const QString &h : hrefs) QVERIFY2(c.contains(QLatin1String("xlink:href=\"") + h + QLatin1String("\"")), qPrintable(h));
    }

    void aListGoesOnFromTheListItNames()
    {
        Made m;
        m.autoStyles = "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\"/></text:list-style>";
        auto list = [](const char *id, const char *continues, const char *text) {
            return QByteArray("<text:list text:style-name=\"L1\" xml:id=\"") + id + "\"" + (*continues ? QByteArray(" text:continue-list=\"") + continues + "\"" : QByteArray()) + "><text:list-item><text:p>" + text + "</text:p></text:list-item></text:list>";
        };
        m.body = list("a", "", "a1") + list("b", "", "b1") + list("a2", "a", "a2") + list("b2", "b", "b2") + list("a3", "a2", "a3");
        const QString d = part(docxOf(m.build()), QStringLiteral("/word/document.xml"));
        QRegularExpression ids(QStringLiteral("<w:numId w:val=\"(\\d+)\"/>"));
        QStringList used;
        for (auto it = ids.globalMatch(d); it.hasNext();) used << it.next().captured(1);
        QCOMPARE(used.size(), 5);
        QVERIFY(used[0] != used[1]);          // two lists of one style, each its own count
        QCOMPARE(used[2], used[0]);           // a2 goes on from a
        QCOMPARE(used[3], used[1]);           // b2 goes on from b
        QCOMPARE(used[4], used[0]);           // a3 goes on from a2, which was a's
    }

    void valuesAreCheckedFromTheParsedXml()
    {
        // What a value is, read from the tree (not how the file spells it).
        Made m;
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties fo:margin-left=\"1in\" fo:line-height=\"200%\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">x</text:p>";
        wml::XmlTree t;
        QVERIFY(t.parse(document(m).toUtf8(), nullptr));
        const int ind = findElement(t, "ind"), spacing = findElement(t, "spacing");
        QVERIFY(ind >= 0 && spacing >= 0);
        QCOMPARE(t.attr(ind, "left").toInt(), 1440);
        QCOMPARE(t.attr(spacing, "line").toInt(), 480);
        QCOMPARE(t.attr(spacing, "lineRule"), QStringLiteral("auto"));
    }

    void contextualSpacingGoesBothWays()
    {
        Made m;
        m.autoStyles = "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:paragraph-properties style:contextual-spacing=\"true\"/></style:style>";
        m.body = "<text:p text:style-name=\"P1\">x</text:p>";
        const QByteArray docx = docxOf(m.build());
        wml::XmlTree t;
        QVERIFY(t.parse(part(docx, QStringLiteral("/word/document.xml")).toUtf8(), nullptr));
        QVERIFY(findElement(t, "contextualSpacing") >= 0);
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(docx), files));
        QVERIFY(QString::fromUtf8(files.value(QStringLiteral("content.xml"))).contains(QLatin1String("style:contextual-spacing=\"true\"")));
    }

    void anEmptyParagraphWithNoListInAListStyleIsNotNumbered()
    {
        Made m;
        m.styles = "<style:style style:name=\"Standard\" style:family=\"paragraph\"/><style:style style:name=\"Numbered\" style:family=\"paragraph\" style:list-style-name=\"L1\"/>";
        m.autoStyles = "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\"/></text:list-style>"
                       "<style:style style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"Numbered\" style:list-style-name=\"\"/>";
        m.body = "<text:p text:style-name=\"P1\">plain</text:p>";
        const QString d = document(m);
        wml::XmlTree t;
        QVERIFY(t.parse(d.toUtf8(), nullptr));
        const int numId = findElement(t, "numId");
        QVERIFY(numId >= 0);
        QCOMPARE(t.val(numId), QStringLiteral("0"));
    }

    void dynamicSpacingIsInsideTheFooter()
    {
        Made m;
        m.layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" fo:margin-top=\"1in\" fo:margin-bottom=\"0.5in\" fo:margin-left=\"1in\" fo:margin-right=\"1in\"/>"
                    "<style:footer-style><style:header-footer-properties fo:min-height=\"0.5in\" fo:margin-top=\"0.46in\" style:dynamic-spacing=\"true\"/></style:footer-style></style:page-layout>";
        m.masters = "<style:master-page style:name=\"Standard\" style:page-layout-name=\"pm1\"><style:footer><text:p>f</text:p></style:footer></style:master-page>";
        m.body = "<text:p>x</text:p>";
        wml::XmlTree t;
        QVERIFY(t.parse(document(m).toUtf8(), nullptr));
        const int mar = findElement(t, "pgMar");
        QVERIFY(mar >= 0);
        QCOMPARE(t.attr(mar, "bottom").toInt(), 1440);   // 0.5 in and the footer's 0.5 in; its spacing is inside it
    }

    void aFieldIsWrittenOnceWithItsResult()
    {
        const char kW[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
        PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<w:document ") + kW + "><w:body><w:p><w:r><w:t xml:space=\"preserve\">before </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                                                            "<w:r><w:t>7</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:r><w:t xml:space=\"preserve\"> after</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        QMap<QString, QByteArray> files;
        QVERIFY(jp::readZip(fromDocxPackage(w.finish()), files));
        const QString c = QString::fromUtf8(files.value(QStringLiteral("content.xml")));
        {
            // (The paragraph reads as it was: the field's result is once, where the field is.)
            wml::XmlTree t;
            QVERIFY(t.parse(files.value(QStringLiteral("content.xml")), nullptr));
            const int p = findElement(t, "p");
            QVERIFY(p >= 0);
            QCOMPARE(t.text(p), QStringLiteral("before 7 after"));
        }
        QCOMPARE(c.count(QLatin1String("before")), 1);
        QCOMPARE(c.count(QLatin1String(">7<")), 1);
        QCOMPARE(c.count(QLatin1String(" after")), 1);
        QVERIFY(c.indexOf(QLatin1String("before")) < c.indexOf(QLatin1String("<text:page-number")));
        QVERIFY(c.indexOf(QLatin1String("<text:page-number")) < c.indexOf(QLatin1String(" after")));
    }
};

QTEST_MAIN(OdtTests)
#include "tst_odt.moc"
