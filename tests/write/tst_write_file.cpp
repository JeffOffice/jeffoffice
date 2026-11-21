// Tests of JeffWrite's File screen: where editing stopped, the warning before a save to a format that holds less, and the Options pages. Every file is made here in code.

#include "../isolatesettings.h"
#include "../withdialog.h"
#include "core/commentops.h"
#include "../cardchecks.h"
#include "../packedpackagecheck.h"
#include "../localepin.h"
#include "../pdfmarkcheck.h"
#include "app/filescreen.h"
#include "app/optionsdialog.h"
#include "app/settings.h"
#include "app/filedialogs.h"
#include "app/help.h"
#include "app/printpage.h"
#include "app/privacy.h"
#include "app/trust.h"
#include "app/pageview.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/styleops.h"
#include "core/pagesetup.h"
#include "core/writeprops.h"
#include "core/pictureops.h"
#include "core/reviewops.h"
#include "core/shrinkops.h"
#include "core/fonts.h"
#include "app/emailops.h"
#include "core/pdfbookmarks.h"
#include "core/linkops.h"
#include "core/refops.h"
#include "core/tocops.h"
#include "io/qtpdf.h"
#include "io/pdftransparency.h"
#include "../pdftransparencyfixtures.h"
#include "core/items.h"
#include "app/pdfprotection.h"
#include "core/printing.h"
#include "app/versionstore.h"
#include "core/templates.h"
#include "render/pdfpage.h"
#include "office/package.h"
#include "office/packageprops.h"
#include "core/writedoc.h"
#include "app/theme.h"
#include "app/packandgo.h"
#include "app/passworddialogs.h"
#include "io/zip.h"

#include <QAction>
#include <QStandardItemModel>
#include <QPrintDialog>
#include <QTimeZone>
#include <QLabel>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QBuffer>
#include <QScopeGuard>
#include "app/pagepaint.h"
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPrinter>
#include <QPdfWriter>
#include <QRegularExpression>
#include <QPushButton>
#include <QTimer>
#include <QTemporaryDir>
#include <QTest>
#include <limits>

using namespace jp;
using namespace jp::write;

namespace {

void typeText(WriteDoc &doc, QTextCursor &c, const QString &text)
{
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n')) insertParagraph(doc, c);
        else insertText(doc, c, QString(ch));
    }
}

bool saveInto(WriteWindow &w, const QTemporaryDir &dir, const QString &name)
{
    w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(name); });
    return w.saveAs();
}

struct Quiet {
    Quiet(WriteWindow &w) { w.setNotifier([this](const QString &, const QString &text) { said << text; }); }
    QStringList said;
};

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
const QString kRel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
const QString kWord = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");

// A .docx with two heading levels (and Normal) whose body is the given paragraphs.
QByteArray headingsDocx(const QByteArray &body)
{
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
              kWord + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"),
              QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs +
                  "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                  "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr></w:style>"
                  "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:pPr><w:outlineLvl w:val=\"1\"/></w:pPr></w:style></w:styles>",
              kWord + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    return w.finish();
}

// A .docx whose styles give paragraphs borders and shading (solid, a percent pattern, a theme fill), with a paragraph of each and a fourth with a box of its own.
QByteArray styledBoxesDocx()
{
    office::PackageWriter w;
    const QByteArray body =
        "<w:p><w:pPr><w:pStyle w:val=\"QuoteA\"/></w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val=\"QuoteB\"/></w:pPr><w:r><w:t>two</w:t></w:r></w:p>"
        "<w:p><w:pPr><w:pStyle w:val=\"QuoteC\"/></w:pPr><w:r><w:t>three</w:t></w:r></w:p>"
        "<w:p><w:pPr><w:pBdr><w:top w:val=\"single\" w:sz=\"4\" w:space=\"1\" w:color=\"00FF00\"/></w:pBdr><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"CCCCCC\"/></w:pPr><w:r><w:t>four</w:t></w:r></w:p>";
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
              kWord + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"),
              QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs +
                  "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                  "<w:style w:type=\"paragraph\" w:styleId=\"QuoteA\"><w:name w:val=\"QuoteA\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:pBdr><w:left w:val=\"triple\" w:sz=\"12\" w:space=\"4\" w:color=\"FF0000\"/>"
                  "<w:bar w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:pBdr><w:shd w:val=\"solid\" w:color=\"FFE0E0\" w:fill=\"auto\"/></w:pPr></w:style>"
                  "<w:style w:type=\"paragraph\" w:styleId=\"QuoteB\"><w:name w:val=\"QuoteB\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:pBdr><w:top w:val=\"single\" w:sz=\"8\" w:space=\"1\" w:color=\"4472C4\" "
                  "w:themeColor=\"accent1\" w:themeTint=\"99\"/></w:pBdr><w:shd w:val=\"pct25\" w:color=\"0000FF\" w:fill=\"FFFF00\"/></w:pPr></w:style>"
                  "<w:style w:type=\"paragraph\" w:styleId=\"QuoteC\"><w:name w:val=\"QuoteC\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:pBdr><w:bottom w:val=\"thinThickSmallGap\" w:sz=\"12\" w:space=\"1\" w:color=\"auto\"/></w:pBdr>"
                  "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"FFFFFF\" w:themeFill=\"accent2\" w:themeFillTint=\"66\"/></w:pPr></w:style></w:styles>",
              kWord + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    return w.finish();
}

QByteArray para(const char *style, const char *text)
{
    return QByteArray("<w:p>") + (style ? QByteArray("<w:pPr><w:pStyle w:val=\"") + style + "\"/></w:pPr>" : QByteArray()) + "<w:r><w:t>" + text + "</w:t></w:r></w:p>";
}

// A .docx with a heading, a paragraph, two paragraphs for a list, a table with a header row, and a 2-inch picture of 600 by 600 pixels with alt text.
QByteArray richDocx(const QString &alt)
{
    QImage image(600, 600, QImage::Format_RGB32);
    for (int y = 0; y < 600; ++y)
        for (int x = 0; x < 600; ++x) image.setPixel(x, y, qRgb((x * 7 + y * 3) & 255, (x * y) & 255, (x ^ y) & 255));
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    const QByteArray drawing = "<w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"1828800\" cy=\"1828800\"/><wp:docPr id=\"7\" name=\"Logo\" descr=\"" + alt.toUtf8() + "\"/>"
                               "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"l.png\"/><pic:cNvPicPr/></pic:nvPicPr>"
                               "<pic:blipFill><a:blip r:embed=\"rId9\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1828800\" cy=\"1828800\"/></a:xfrm>"
                               "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>";
    auto cell = [](const char *text) { return QByteArray("<w:tc><w:p><w:r><w:t>") + text + "</w:t></w:r></w:p></w:tc>"; };
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
                             "<w:tr><w:trPr><w:tblHeader/></w:trPr>" + cell("Name") + cell("Count") + "</w:tr><w:tr>" + cell("Apples") + cell("Twelve") + "</w:tr></w:tbl>";
    const QByteArray body = para("Heading1", "First part") + para(nullptr, "Some body text.") + para(nullptr, "item one") + para(nullptr, "item two") + table +
                            "<w:p><w:r>" + drawing + "</w:r></w:p>";
    const QByteArray ns = QByteArray(kNs) + " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                          "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + ns + "><w:body>" + body + "</w:body></w:document>",
              kWord + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"),
              QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs +
                  "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                  "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr></w:style></w:styles>",
              kWord + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    w.addPart(QStringLiteral("/word/media/p.png"), png, QStringLiteral("image/png"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId9"), kRel + QStringLiteral("image"), QStringLiteral("/word/media/p.png"));
    return w.finish();
}

// A .docx whose picture is linked to the file `picture` (not in the document) and that has INCLUDEPICTURE and INCLUDETEXT fields for `fieldPicture` and `fieldText`, in the font Georgia.
QByteArray linkedDocx(const QString &picture, const QString &fieldPicture, const QString &fieldText)
{
    const QByteArray drawing = "<w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"914400\" cy=\"914400\"/><wp:docPr id=\"7\" name=\"Linked\"/>"
                               "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"l.png\"/><pic:cNvPicPr/></pic:nvPicPr>"
                               "<pic:blipFill><a:blip r:link=\"rId9\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"914400\" cy=\"914400\"/></a:xfrm>"
                               "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>";
    auto field = [](const QString &code) {
        return QByteArray("<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> ") + code.toUtf8() + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
               "<w:r><w:t>result</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    };
    const QByteArray body = "<w:p><w:r><w:rPr><w:rFonts w:ascii=\"Georgia\" w:hAnsi=\"Georgia\"/></w:rPr><w:t>Linked</w:t></w:r><w:r>" + drawing + "</w:r></w:p><w:p>" +
                            field(QStringLiteral("INCLUDEPICTURE \"%1\" \\d").arg(fieldPicture)) + "</w:p><w:p>" + field(QStringLiteral("INCLUDETEXT \"%1\"").arg(fieldText)) + "</w:p>";
    const QByteArray ns = QByteArray(kNs) + " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                          "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + ns + "><w:body>" + body + "</w:body></w:document>",
              kWord + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId9"), kRel + QStringLiteral("image"), picture, true);
    return w.finish();
}

// The widest picture in a PDF, in pixels (0 for none).
int widestPictureIn(const QByteArray &pdf)
{
    jp::QtPdf q;
    if (!q.loadBytes(pdf)) return 0;
    int widest = 0;
    static const QRegularExpression width(QStringLiteral("/Width (\\d+)"));
    for (const jp::QtPdf::Obj &o : q.objects) {
        const QByteArray dict = jp::QtPdf::dictOf(o.body);
        if (!dict.contains("/Subtype /Image")) continue;
        const auto m = width.match(QString::fromLatin1(dict));
        if (m.hasMatch()) widest = std::max(widest, m.captured(1).toInt());
    }
    return widest;
}

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

const char kPageBreak[] = "<w:p><w:r><w:br w:type=\"page\"/></w:r></w:p>";

QByteArray utf16Hex(const QString &t)
{
    QByteArray b("\xFE\xFF", 2);
    for (const QChar c : t) {
        b.append(char(c.unicode() >> 8));
        b.append(char(c.unicode() & 0xFF));
    }
    return b.toHex().toUpper();
}

void moveTo(WriteWindow &w, int pos)
{
    QTextCursor c(w.document()->story());
    c.setPosition(pos);
    w.view()->setCursor(c);
}

} // namespace

// How many pixels of the first page of two PDFs differ by more than a little, drawn on white at the same size.
static int farPixels(const QByteArray &a, const QByteArray &b, const QSize &size)
{
    auto draw = [&](const QByteArray &bytes) {
        const PdfDocument pdf(bytes);
        QImage page(size, QImage::Format_RGB32);
        page.fill(Qt::white);
        if (pdf.isValid()) {
            QPainter on(&page);
            on.drawImage(0, 0, pdf.render(0, size));
        }
        return page;
    };
    const QImage x = draw(a), y = draw(b);
    int far = 0;
    for (int j = 0; j < size.height(); ++j)
        for (int i = 0; i < size.width(); ++i) far += std::abs(qGray(x.pixel(i, j)) - qGray(y.pixel(i, j))) > 60;
    return far;
}

namespace {

// Whether a PDF holds the text: written plainly, or as a string of its information (UTF-16, in hexadecimal of either case, or as bytes). A search for the plain text alone never finds a
// name the information holds.
bool holdsText(const QByteArray &pdf, const QString &text)
{
    QByteArray be;
    for (const QChar c : text) {
        be.append(char(c.unicode() >> 8));
        be.append(char(c.unicode() & 0xFF));
    }
    return pdf.contains(text.toUtf8()) || pdf.contains(be) || pdf.contains(be.toHex().toUpper()) || pdf.contains(be.toHex());
}

// The words of a field of the metadata of a PDF (what stands between its tags, the tags inside left out).
QString xmpField(const QByteArray &pdf, const QByteArray &tag)
{
    const int at = pdf.indexOf("<" + tag + ">");
    if (at < 0) return QString();
    const int end = pdf.indexOf("</" + tag + ">", at);
    if (end < 0) return QString();
    QString inner = QString::fromUtf8(pdf.mid(at + tag.size() + 2, end - at - tag.size() - 2));
    inner.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    return inner.trimmed();
}

// A .docx for Shrink by One Page: `paragraphs` paragraphs of the Normal style, which has a size of its own (`styleHalfPoints`); the runs say a size of their own only with
// `runHalfPoints`. The rest is what a document may hold besides: exact line spacing, a table, a footnote, a header, a locked control, a page break or a section break before a last
// paragraph.
struct ShrinkDocx {
    int paragraphs = 30, styleHalfPoints = 24, runHalfPoints = 0;
    bool exact = false, pageBreak = false, sectionBreak = false, footnote = false, table = false, header = false, lockedControl = false;
    bool breakBefore = false, endnote = false;   // (the last paragraph starts a page itself; a note at the end of the text)
    int tableStyleHalfPoints = 0;                // (the table's style gives its text this size)
    QByteArray build() const
    {
        const QByteArray run = runHalfPoints ? "<w:rPr><w:sz w:val=\"" + QByteArray::number(runHalfPoints) + "\"/></w:rPr>" : QByteArray();
        const QByteArray pPr = exact ? QByteArray("<w:pPr><w:spacing w:line=\"400\" w:lineRule=\"exact\"/></w:pPr>") : QByteArray();
        QByteArray body;
        if (table) body += "<w:tbl><w:tblPr>" + (tableStyleHalfPoints ? QByteArray("<w:tblStyle w:val=\"Small\"/>") : QByteArray()) + "<w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p>" + pPr + "<w:r>" + (tableStyleHalfPoints ? QByteArray() : run) + "<w:t>Cell words</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        for (int i = 0; i < paragraphs; ++i) {
            body += "<w:p>" + pPr + "<w:r>" + run + "<w:t>Paragraph " + QByteArray::number(i) + " of the shrinking test.</w:t></w:r>";
            if (i == 0 && footnote) body += "<w:r><w:footnoteReference w:id=\"1\"/></w:r>";
            if (i == 0 && endnote) body += "<w:r><w:endnoteReference w:id=\"1\"/></w:r>";
            if (i == 0 && lockedControl) body += "<w:sdt><w:sdtPr><w:lock w:val=\"contentLocked\"/><w:text/></w:sdtPr><w:sdtContent><w:r><w:t>Locked words</w:t></w:r></w:sdtContent></w:sdt>";
            body += "</w:p>";
        }
        if (pageBreak) body += QByteArray(kPageBreak);
        const QByteArray sect = QByteArray("<w:sectPr>") + (header ? "<w:headerReference w:type=\"default\" r:id=\"rId20\"/>" : "") +
                                "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        if (sectionBreak) body += "<w:p><w:pPr>" + sect + "</w:pPr></w:p>";
        if (pageBreak || sectionBreak) body += QByteArray("<w:p>") + (breakBefore ? "<w:pPr><w:pageBreakBefore/></w:pPr>" : "") + "<w:r>" + run + "<w:t>The last page.</w:t></w:r></w:p>";
        body += sect;
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
                  kWord + QStringLiteral("document.main+xml"));
        w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/styles.xml"),
                  QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs + "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/>" +
                      (styleHalfPoints ? "<w:rPr><w:sz w:val=\"" + QByteArray::number(styleHalfPoints) + "\"/></w:rPr>" : QByteArray()) + "</w:style>" +
                      (tableStyleHalfPoints ? "<w:style w:type=\"table\" w:styleId=\"Small\"><w:name w:val=\"Small\"/><w:rPr><w:sz w:val=\"" + QByteArray::number(tableStyleHalfPoints) + "\"/></w:rPr></w:style>" : QByteArray()) + "</w:styles>",
                  kWord + QStringLiteral("styles+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        if (footnote) {
            w.addPart(QStringLiteral("/word/footnotes.xml"),
                      QByteArray("<?xml version=\"1.0\"?><w:footnotes ") + kNs +
                          "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>"
                          "<w:footnote w:id=\"1\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:rPr><w:sz w:val=\"20\"/></w:rPr><w:t xml:space=\"preserve\"> Note words.</w:t></w:r></w:p></w:footnote></w:footnotes>",
                      kWord + QStringLiteral("footnotes+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
        }
        if (endnote) {
            w.addPart(QStringLiteral("/word/endnotes.xml"),
                      QByteArray("<?xml version=\"1.0\"?><w:endnotes ") + kNs +
                          "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote><w:endnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:endnote>"
                          "<w:endnote w:id=\"1\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:rPr><w:sz w:val=\"20\"/></w:rPr><w:t xml:space=\"preserve\"> Endnote words.</w:t></w:r></w:p></w:endnote></w:endnotes>",
                      kWord + QStringLiteral("endnotes+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("endnotes"), QStringLiteral("/word/endnotes.xml"));
        }
        if (header) {
            w.addPart(QStringLiteral("/word/header1.xml"),
                      QByteArray("<?xml version=\"1.0\"?><w:hdr ") + kNs + "><w:p><w:r><w:rPr><w:sz w:val=\"24\"/></w:rPr><w:t>Header words</w:t></w:r></w:p></w:hdr>", kWord + QStringLiteral("header+xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId20"), kRel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
        }
        return w.finish();
    }
};

// How many paragraphs of this document make `pages` pages at the fewest (found by making the document and counting its pages: the fonts of the machine decide).
int paragraphsForPages(ShrinkDocx docx, int pages)
{
    auto pagesOfCount = [&](int count) {
        docx.paragraphs = count;
        QString err;
        auto doc = write::WriteDoc::fromDocx(docx.build(), &err, nullptr);
        return doc ? doc->pager().pageCount() : 0;
    };
    int low = 1, high = 1;
    while (high < 20000 && pagesOfCount(high) < pages) high *= 2;
    if (pagesOfCount(high) < pages) return 0;
    while (low < high) {
        const int mid = (low + high) / 2;
        if (pagesOfCount(mid) >= pages) high = mid;
        else low = mid + 1;
    }
    return low;
}

// The document of `docx` with the paragraphs that make a few lines (the last paragraph alone) of a second page, as bytes; `count` says how many.
QByteArray twoPageDocx(int *count, ShrinkDocx docx = ShrinkDocx())
{
    docx.paragraphs = paragraphsForPages(docx, 2);
    if (count) *count = docx.paragraphs;
    return docx.build();
}

// A .docx of two sections (the first ends at the paragraph that says so), each with the margins and the turn it is given: `first` and `last` are the bytes of the w:sectPr children.
QByteArray twoSectionDocx(const QByteArray &first, const QByteArray &last)
{
    const QByteArray body = "<w:p><w:r><w:t>First section.</w:t></w:r></w:p><w:p><w:pPr><w:sectPr>" + first + "</w:sectPr></w:pPr></w:p><w:p><w:r><w:t>Second section.</w:t></w:r></w:p><w:sectPr>" + last + "</w:sectPr>";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
              kWord + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

// The children of a w:sectPr: paper (turned when `landscape`), and the margins in twips.
QByteArray sectionKids(int top, int right, int bottom, int left, int gutter, bool landscape)
{
    return QByteArray(landscape ? "<w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/>" : "<w:pgSz w:w=\"12240\" w:h=\"15840\"/>") + "<w:pgMar w:top=\"" + QByteArray::number(top) + "\" w:right=\"" +
           QByteArray::number(right) + "\" w:bottom=\"" + QByteArray::number(bottom) + "\" w:left=\"" + QByteArray::number(left) + "\" w:header=\"720\" w:footer=\"720\" w:gutter=\"" + QByteArray::number(gutter) + "\"/>";
}

// A window on a document, made from bytes (each is a window of its own: a window that has a document opens the next in another).
std::unique_ptr<WriteWindow> windowOn(const QTemporaryDir &dir, const QByteArray &docx)
{
    static int made = 0;
    auto w = std::make_unique<WriteWindow>();
    const QString path = dir.filePath(QStringLiteral("shrink%1.docx").arg(++made));
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return nullptr;
    f.write(docx);
    f.close();
    return w->openFile(path) ? std::move(w) : nullptr;
}

int pagesOf(WriteWindow &w)
{
    w.view()->paginateAll();
    return w.document()->pager().pageCount();
}

// The sizes the text of a story has, as the pages set it: each run's own, else its paragraph style's. One entry for each run, in order.
QVector<double> effectiveSizes(WriteDoc &doc, QTextDocument *story)
{
    QVector<double> out;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat f = it.fragment().charFormat();
            out << (f.fontPointSize() > 0 ? f.fontPointSize() : doc.styleCharFormat(b.blockFormat().stringProperty(tp::StyleName), f.stringProperty(tp::CharStyle)).fontPointSize());
        }
    return out;
}

double largest(const QVector<double> &sizes) { return sizes.isEmpty() ? 0 : *std::max_element(sizes.begin(), sizes.end()); }
double smallest(const QVector<double> &sizes) { return sizes.isEmpty() ? 0 : *std::min_element(sizes.begin(), sizes.end()); }

// The exact line heights of a story's paragraphs (0 for the others).
QVector<double> exactHeights(QTextDocument *story)
{
    QVector<double> out;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) out << (b.blockFormat().lineHeightType() == QTextBlockFormat::FixedHeight ? b.blockFormat().lineHeight() : 0);
    return out;
}

// The stories of a document besides its text that are a header's or a footer's, and its footnotes'.
QVector<QTextDocument *> headerStories(WriteDoc &doc)
{
    QVector<QTextDocument *> out;
    for (QTextDocument *s : doc.storiesWithChanges(true))
        if (doc.isHeaderFooterStory(s)) out << s;
    return out;
}
QVector<QTextDocument *> noteStories(WriteDoc &doc)
{
    QVector<QTextDocument *> out;
    for (QTextDocument *s : doc.storiesWithChanges(true))
        if (doc.footnoteIdOf(s)) out << s;
    return out;
}

} // namespace

class FileTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // The cards of each page of the File screen show all their text, clear of the chevron, in a wide window and a narrower one.
    void everyCardOfTheFileScreenIsWholeAtEveryWidth()
    {
        WriteWindow w;
        w.show();
        jptest::checkFileScreenCards(&w, {QStringLiteral("new"), QStringLiteral("open"), QStringLiteral("info"), QStringLiteral("print"), QStringLiteral("export")}, 2);
    }
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }
    void init() { Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Tester")); }

    void aDocumentReopensWhereEditingStopped()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("a.docx"));
        {
            WriteWindow w;
            Quiet quiet(w);
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("alpha beta\ngamma delta\nepsilon zeta"));
            QVERIFY(saveInto(w, dir, QStringLiteral("a.docx")));
            moveTo(w, 14);
            w.close();
        }
        WriteWindow v;
        Quiet quiet(v);
        QVERIFY(v.openFile(path));
        QCOMPARE(v.view()->cursor().position(), 14);
    }

    void aPlaceBeyondAShorterFileIsKeptInside()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("a.docx"));
        {
            WriteWindow w;
            Quiet quiet(w);
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("a long line of words to leave behind"));
            QVERIFY(saveInto(w, dir, QStringLiteral("a.docx")));
            moveTo(w, 30);
            w.close();
        }
        {
            WriteWindow w;
            Quiet quiet(w);
            QVERIFY(w.openFile(path));
            QTextCursor c(w.document()->story());
            c.setPosition(0);
            c.setPosition(w.document()->story()->characterCount() - 1, QTextCursor::KeepAnchor);
            c.removeSelectedText();
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("hi"));
            QVERIFY(w.save());
        }
        // the remembered place (30) is past the end of "hi": the caret is put at the end
        WriteWindow v;
        Quiet quiet(v);
        QVERIFY(v.openFile(path));
        QVERIFY(v.view()->cursor().position() <= 2);
    }

    void lastPlaceCyclesThroughTheLastThreeEdits()
    {
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(!w.act(QStringLiteral("edit.lastPlace"))->isEnabled());
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("0123456789"));
        // four far-apart edits: only the last three are kept
        w.document()->story()->setPlainText(QString(2500, QLatin1Char('x')));
        w.resetEditPlaces();
        for (int at : {100, 700, 1300, 1900}) {
            QTextCursor c(w.document()->story());
            c.setPosition(at);
            c.insertText(QStringLiteral("q"));
        }
        QCOMPARE(w.editPlaces().size(), 3);
        QVERIFY(w.act(QStringLiteral("edit.lastPlace"))->isEnabled());
        QStringList went;
        for (int i = 0; i < 4; ++i) {
            w.act(QStringLiteral("edit.lastPlace"))->trigger();
            went << QString::number(w.view()->cursor().position());
        }
        QCOMPARE(went, (QStringList{QStringLiteral("1901"), QStringLiteral("1301"), QStringLiteral("701"), QStringLiteral("1901")}));
    }

    void lastPlacesFollowTheEditsThatCameAfterThem()
    {
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("0123456789"));
        w.document()->story()->setPlainText(QString(2500, QLatin1Char('x')));
        w.resetEditPlaces();
        QTextCursor c(w.document()->story());
        c.setPosition(1900);
        c.insertText(QStringLiteral("q"));
        QCOMPARE(w.editPlaces(), (QList<int>{1901}));
        // 300 letters put in far above it: the first edit's place moves down with its words.
        c.setPosition(100);
        c.insertText(QString(300, QLatin1Char('y')));
        QCOMPARE(w.editPlaces(), (QList<int>{400, 2201}));
        // 500 taken out above both: they move up (the one inside what went is where the cut was).
        c.setPosition(50);
        c.setPosition(550, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        QCOMPARE(w.editPlaces(), (QList<int>{50, 1701}));
        QStringList went;
        for (int i = 0; i < 2; ++i) {
            w.act(QStringLiteral("edit.lastPlace"))->trigger();
            went << QString::number(w.view()->cursor().position());
        }
        QCOMPARE(went, (QStringList{QStringLiteral("50"), QStringLiteral("1701")}));
    }

    void aSaveToOdtAsksFirstAndCanSaveADocxInstead()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("x"));
        w.document()->setPageColor(QColor(0xFF, 0xF4, 0xD6));
        QStringList asked;
        WriteWindow::FormatChoice answer = WriteWindow::FormatChoice::Cancel;
        w.setFormatAsker([&](const QStringList &lost) {
            asked = lost;
            return answer;
        });
        // Cancel: nothing is written, and the document is as it was
        QVERIFY(!saveInto(w, dir, QStringLiteral("a.odt")));
        QVERIFY(asked.join(QLatin1Char(' ')).contains(QLatin1String("page color")));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("a.odt"))));
        QVERIFY(w.isModified());
        // A .docx instead: it is written beside, and the window has that file
        answer = WriteWindow::FormatChoice::Docx;
        QVERIFY(saveInto(w, dir, QStringLiteral("a.odt")));
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("a.docx"))));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("a.odt"))));
        QCOMPARE(QFileInfo(w.filePath()).fileName(), QStringLiteral("a.docx"));
        // Keep: written as an .odt, and not asked again of the same thing
        answer = WriteWindow::FormatChoice::Keep;
        asked.clear();
        QVERIFY(saveInto(w, dir, QStringLiteral("b.odt")));
        QVERIFY(!asked.isEmpty());
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("b.odt"))));
        asked.clear();
        quiet.said.clear();
        QVERIFY(saveInto(w, dir, QStringLiteral("c.odt")));
        QVERIFY(asked.isEmpty());
        QVERIFY(quiet.said.isEmpty());
    }

    // A document marked as final keeps the mark when the save to .odt is given up or goes to a .docx instead.
    void theFinalMarkIsKeptWhenASaveToOdtIsNotMade()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("x"));
        QVERIFY(saveInto(w, dir, QStringLiteral("a.docx")));
        w.setMarkedFinal(true);
        QVERIFY(w.document()->markedAsFinal());
        w.document()->setPageColor(QColor(0xFF, 0xF4, 0xD6));
        WriteWindow::FormatChoice answer = WriteWindow::FormatChoice::Cancel;
        w.setFormatAsker([&](const QStringList &) { return answer; });
        QVERIFY(!saveInto(w, dir, QStringLiteral("b.odt")));
        QVERIFY(w.document()->markedAsFinal());
        answer = WriteWindow::FormatChoice::Docx;
        QVERIFY(saveInto(w, dir, QStringLiteral("b.odt")));
        QVERIFY(w.document()->markedAsFinal());
    }

    void aDocumentAnOdtHoldsWholeIsNotAsked()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("plain"));
        bool asked = false;
        w.setFormatAsker([&](const QStringList &) {
            asked = true;
            return WriteWindow::FormatChoice::Cancel;
        });
        QVERIFY(saveInto(w, dir, QStringLiteral("a.odt")));
        QVERIFY(!asked);
    }

    void theSaveAndDisplayPagesChangeWhatNewWindowsStartWith()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        QVector<OptionsPage> pages = w.optionsPages();
        QStringList titles;
        for (const OptionsPage &p : pages) titles << p.title;
        QVERIFY(titles.contains(QStringLiteral("Save")));
        QVERIFY(titles.contains(QStringLiteral("Display")));
        QVERIFY(titles.contains(QStringLiteral("Advanced")));
        QList<QWidget *> built;
        for (const OptionsPage &p : pages) built << p.build();
        auto in = [&](const char *name) {
            for (QWidget *page : built)
                if (QWidget *f = page->findChild<QWidget *>(QString::fromLatin1(name))) return f;
            return static_cast<QWidget *>(nullptr);
        };
        auto *minutes = qobject_cast<QSpinBox *>(in("autoRecoverMinutes"));
        auto *format = qobject_cast<QComboBox *>(in("defaultFormat"));
        auto *folder = qobject_cast<QLineEdit *>(in("defaultFolder"));
        auto *marks = qobject_cast<QCheckBox *>(in("startWithMarks"));
        auto *grid = qobject_cast<QCheckBox *>(in("startWithGridlines"));
        QVERIFY(minutes && format && folder && marks && grid);
        minutes->setValue(3);
        format->setCurrentIndex(format->findData(QStringLiteral("odt")));
        folder->setText(dir.path());
        marks->setChecked(true);
        grid->setChecked(true);
        auto *units = qobject_cast<QComboBox *>(in("measurementUnits"));
        QVERIFY(units);
        const Unit oldUnit = Settings::get().unit();
        units->setCurrentIndex(units->findData(int(oldUnit == Unit::Millimeter ? Unit::Point : Unit::Millimeter)));
        for (const OptionsPage &p : pages)
            if (p.apply) p.apply();
        qDeleteAll(built);
        QCOMPARE(Settings::get().autoRecoverMinutes(), 3);
        QCOMPARE(Settings::get().unit(), oldUnit == Unit::Millimeter ? Unit::Point : Unit::Millimeter);
        Settings::get().setUnit(oldUnit);
        // this window changes at once
        QVERIFY(w.view()->showMarks());
        QVERIFY(w.act(QStringLiteral("view.marks"))->isChecked());
        // a new window starts so, and its first Save As is an .odt in the folder
        WriteWindow v;
        QVERIFY(v.view()->showMarks());
        QVERIFY(v.view()->showGridlines());
        QString suggested, filter;
        v.setSavePathChooser([&](const QString &s, const QString &f) {
            suggested = s;
            filter = f;
            return QString();
        });
        QVERIFY(!v.saveAs());
        QCOMPARE(QFileInfo(suggested).absolutePath(), QFileInfo(dir.path()).absoluteFilePath());
        QVERIFY(suggested.endsWith(QLatin1String(".odt")));
        QVERIFY(filter.contains(QLatin1String("*.odt")));
        // put back what the other tests expect
        Settings::get().setValue(QStringLiteral("view/marks"), false);
        Settings::get().setValue(QStringLiteral("view/gridlines"), false);
        Settings::get().setValue(QStringLiteral("save/format"), QStringLiteral("docx"));
        Settings::get().setValue(QStringLiteral("dirs/default"), QString());
        Settings::get().setValue(QStringLiteral("save/autoRecoverMinutes"), 10);
    }

    // Borders and Shading: the paragraphs selected take the sides, line, color, distance, and fill the box sets, as one step; the page shows them; the saved .docx has w:pBdr and w:shd
    // (ECMA-376 Part 1 sections 17.3.1.24 and 17.3.1.31); a file opened again through the window has them; Undo takes them back; the quick commands set one side or none.
    void paragraphBordersAndShadingAreSetByTheBoxAndSurviveSaveAndOpen()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow w;
        Quiet quiet(w);
        w.resize(900, 700);
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("first\nsecond\nthird"));
        auto boxOf = [](WriteWindow &win, int block) {
            QTextCursor c(win.document()->story()->findBlockByNumber(block));
            return write::paragraphBoxAt(c);
        };
        QTextCursor &sel = w.view()->cursor();
        sel.setPosition(1);
        sel.setPosition(9, QTextCursor::KeepAnchor);   // (the first two paragraphs)
        const quint64 serial = w.document()->nextStepSerial();
        withDialog([&] { w.act(QStringLiteral("para.bordersShading"))->trigger(); }, QStringLiteral("paragraphBordersDialog"), [](QDialog *d) {
            QVERIFY(d->findChild<QCheckBox *>(QStringLiteral("top")) && d->findChild<QCheckBox *>(QStringLiteral("bottom")));
            d->findChild<QCheckBox *>(QStringLiteral("top"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("bottom"))->setChecked(true);
            auto *style = d->findChild<QComboBox *>(QStringLiteral("style"));
            style->setCurrentIndex(style->findData(QStringLiteral("double")));
            auto *width = d->findChild<QComboBox *>(QStringLiteral("width"));
            width->setCurrentIndex(width->findData(12));
            d->findChild<QSpinBox *>(QStringLiteral("space"))->setValue(3);
            d->findChild<ColorButton *>(QStringLiteral("color"))->setCurrent(ColorRef::rgb(QColor(0xC0, 0, 0)));
            d->findChild<ColorButton *>(QStringLiteral("shading"))->setCurrent(ColorRef::rgb(QColor(0xFF, 0xF2, 0xCC)));
        });
        QCOMPARE(w.document()->nextStepSerial(), serial + 1);
        for (int block : {0, 1}) {
            const tp::ParaBox box = boxOf(w, block);
            QCOMPARE(box.top.style, QStringLiteral("double"));
            QCOMPARE(box.top.eighths, 12);
            QCOMPARE(box.bottom.space, 3);
            QCOMPARE(box.bottom.color, QColor(0xC0, 0, 0));
            QVERIFY(!box.left.visible() && !box.right.visible());
            QCOMPARE(box.fill, QColor(0xFF, 0xF2, 0xCC));
        }
        QVERIFY(!boxOf(w, 2).any());
        // the page shows the fill
        auto fillPixels = [](WriteWindow &win) {
            const QImage page = win.view()->pageImage(0, 600);
            int n = 0;
            for (int y = 0; y < page.height(); ++y)
                for (int x = 0; x < page.width(); ++x) n += QColor(page.pixel(x, y)) == QColor(0xFF, 0xF2, 0xCC);
            return n;
        };
        QVERIFY(fillPixels(w) > 200);
        // saved and opened again through the window
        QVERIFY(saveInto(w, dir, QStringLiteral("boxes.docx")));
        office::Package package;
        package.read(fileBytes(dir.filePath(QStringLiteral("boxes.docx"))));
        const QByteArray xml = package.part(QStringLiteral("/word/document.xml"));
        QCOMPARE(xml.count("<w:pBdr>"), 2);
        QCOMPARE(xml.count("<w:top w:val=\"double\" w:sz=\"12\" w:space=\"3\" w:color=\"C00000\"/>"), 2);
        QCOMPARE(xml.count("<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"FFF2CC\"/>"), 2);
        WriteWindow v;
        Quiet quietV(v);
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("boxes.docx"))));
        QCOMPARE(boxOf(v, 1).bottom.color, QColor(0xC0, 0, 0));
        QCOMPARE(boxOf(v, 0).fill, QColor(0xFF, 0xF2, 0xCC));
        QVERIFY(!boxOf(v, 2).any());
        // a border the file gave is replaced by the box's, not kept beside it
        QTextCursor &again = v.view()->cursor();
        again.setPosition(1);
        v.act(QStringLiteral("para.border.none"))->trigger();
        QVERIFY(!boxOf(v, 0).top.visible());
        QCOMPARE(boxOf(v, 0).fill, QColor(0xFF, 0xF2, 0xCC));   // (the fill stays)
        QVERIFY(saveInto(v, dir, QStringLiteral("boxes2.docx")));
        office::Package second;
        second.read(fileBytes(dir.filePath(QStringLiteral("boxes2.docx"))));
        QCOMPARE(second.part(QStringLiteral("/word/document.xml")).count("<w:pBdr>"), 1);
        // the quick commands: one side only, the others as they were
        again.setPosition(v.document()->story()->findBlockByNumber(2).position() + 1);
        v.act(QStringLiteral("para.border.bottom"))->trigger();
        QVERIFY(boxOf(v, 2).bottom.visible() && !boxOf(v, 2).top.visible());
        QCOMPARE(boxOf(v, 2).bottom.eighths, 4);
        v.act(QStringLiteral("para.border.all"))->trigger();
        QVERIFY(boxOf(v, 2).top.visible() && boxOf(v, 2).left.visible() && boxOf(v, 2).right.visible());
        // Undo takes the first box back in one step
        QVERIFY(w.document()->undo());
        QVERIFY(!boxOf(w, 0).any() && !boxOf(w, 1).any());
        QVERIFY(w.document()->redo());
        QVERIFY(boxOf(w, 1).any());
    }

    // A box a STYLE gives (borders and shading: solid, a percent pattern, a theme fill) is the style's: saved with no edit, no paragraph gains a direct w:pBdr or w:shd, the style part keeps
    // its pattern and theme attributes, the style still drives the paragraphs (a change of its fill reaches them), and what a paragraph says itself stays as it said it.
    void aBoxAStyleGivesIsNeverCopiedIntoTheParagraphsOnSave()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto window = windowOn(dir, styledBoxesDocx());
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        auto boxOf = [](WriteWindow &win, int block) { return write::paragraphBoxAt(QTextCursor(win.document()->story()->findBlockByNumber(block))); };
        QVERIFY(boxOf(w, 0).left.visible() && boxOf(w, 0).fill.isValid());   // (the style's box is drawn)
        QVERIFY(boxOf(w, 1).top.visible() && boxOf(w, 1).fill.isValid());
        QVERIFY(boxOf(w, 2).bottom.visible() && boxOf(w, 2).fill.isValid());
        QVERIFY(saveInto(w, dir, QStringLiteral("styled.docx")));
        office::Package package;
        package.read(fileBytes(dir.filePath(QStringLiteral("styled.docx"))));
        const QByteArray doc = package.part(QStringLiteral("/word/document.xml")), styles = package.part(QStringLiteral("/word/styles.xml"));
        QCOMPARE(doc.count("<w:pBdr>"), 1);   // (the fourth paragraph's own)
        QCOMPARE(doc.count("<w:shd "), 1);
        QVERIFY(doc.contains("w:fill=\"CCCCCC\""));
        for (const char *kept : {"w:val=\"pct25\"", "w:themeFill=\"accent2\"", "w:themeFillTint=\"66\"", "w:themeColor=\"accent1\"", "w:val=\"triple\"", "<w:bar ", "w:val=\"thinThickSmallGap\""})
            QVERIFY2(styles.contains(kept), kept);
        // The style still drives them: opened again, a change of the style's fill reaches the paragraph that has none of its own, and not the one that has.
        WriteWindow v;
        Quiet quietV(v);
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("styled.docx"))));
        const QColor patternFill = boxOf(v, 1).fill;
        office::wml::Style quote = *v.document()->styleSheet().find(QStringLiteral("QuoteA"));
        quote.pPr.shading = office::wml::Shading{QStringLiteral("clear"), QStringLiteral("auto"), QStringLiteral("00FF00")};
        for (office::wml::RawChild &child : quote.pPr.extra)   // (the style's w:shd is kept as written beside the fill: both say the new fill)
            if (child.xml.startsWith("<w:shd")) child.xml = "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"00FF00\"/>";
        QTextCursor c(v.document()->story());
        QString problem;
        QVERIFY2(modifyStyle(*v.document(), c, QStringLiteral("QuoteA"), quote, &problem), qPrintable(problem));
        QCOMPARE(boxOf(v, 0).fill, QColor(0, 0xFF, 0));
        QCOMPARE(boxOf(v, 1).fill, patternFill);
        QCOMPARE(boxOf(v, 3).fill, QColor(0xCC, 0xCC, 0xCC));
        QVERIFY(saveInto(v, dir, QStringLiteral("styled2.docx")));
        office::Package second;
        second.read(fileBytes(dir.filePath(QStringLiteral("styled2.docx"))));
        const QByteArray doc2 = second.part(QStringLiteral("/word/document.xml"));
        QCOMPARE(doc2.count("<w:pBdr>"), 1);
        QCOMPARE(doc2.count("<w:shd "), 1);
        QVERIFY(second.part(QStringLiteral("/word/styles.xml")).contains("w:fill=\"00FF00\""));
    }

    // A tracked format change records the paragraph as it was: what its style gives it (borders, shading) is not in that record, and not in the paragraph.
    void aTrackedFormatChangeDoesNotRecordTheBoxAStyleGives()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto window = windowOn(dir, styledBoxesDocx());
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        w.document()->setTrackChanges(true);
        QTextCursor c(w.document()->story()->findBlockByNumber(0));
        write::setAlignment(*w.document(), c, Qt::AlignHCenter);
        QVERIFY(saveInto(w, dir, QStringLiteral("tracked.docx")));
        office::Package package;
        package.read(fileBytes(dir.filePath(QStringLiteral("tracked.docx"))));
        const QByteArray doc = package.part(QStringLiteral("/word/document.xml"));
        QVERIFY(doc.contains("<w:pPrChange"));
        QCOMPARE(doc.count("<w:pBdr>"), 1);
        QCOMPARE(doc.count("<w:shd "), 1);
    }

    // One edit of one side rewrites that side only: the other sides stay as the file wrote them (a triple line, a bar, a theme color), a Borders and Shading box accepted as it was is no edit,
    // and paragraphs whose boxes differ keep their own sides that the person leaves alone.
    void settingOneBorderSideKeepsTheOtherSidesAsTheFileWroteThem()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QByteArray top = "<w:top w:val=\"triple\" w:sz=\"18\" w:space=\"2\" w:color=\"auto\"/>", bottom = "<w:bottom w:val=\"single\" w:sz=\"8\" w:space=\"1\" w:color=\"C55A11\" w:themeColor=\"accent2\" w:themeShade=\"BF\"/>",
                         bar = "<w:bar w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"00FF00\"/>";
        auto window = windowOn(dir, headingsDocx("<w:p><w:pPr><w:pBdr>" + top + bottom + bar + "</w:pBdr></w:pPr><w:r><w:t>one</w:t></w:r></w:p>"
                                                   "<w:p><w:pPr><w:pBdr><w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"1\" w:color=\"auto\"/></w:pBdr></w:pPr><w:r><w:t>two</w:t></w:r></w:p>"));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        auto pBdr = [&](const QString &name, int which) {
            office::Package p;
            p.read(fileBytes(dir.filePath(name)));
            const QByteArray doc = p.part(QStringLiteral("/word/document.xml"));
            int at = -1;
            for (int i = 0; i <= which; ++i) at = doc.indexOf("<w:pBdr>", at + 1);
            return at < 0 ? QByteArray() : doc.mid(at, doc.indexOf("</w:pBdr>", at) + 9 - at);
        };
        QTextCursor &sel = w.view()->cursor();
        sel.setPosition(1);
        // The Borders and Shading box accepted as it was: no step.
        quint64 serial = w.document()->nextStepSerial();
        withDialog([&] { w.act(QStringLiteral("para.bordersShading"))->trigger(); }, QStringLiteral("paragraphBordersDialog"), [](QDialog *) {});
        QCOMPARE(w.document()->nextStepSerial(), serial);
        // The left side alone.
        tp::ParaBox box;
        box.left = tp::BoxSide{QStringLiteral("single"), 12, 3, QColor(0, 0, 0xFF)};
        write::setParagraphBox(*w.document(), sel, box, write::boxpart::Left);
        QCOMPARE(w.document()->nextStepSerial(), serial + 1);
        write::setParagraphBox(*w.document(), sel, box, write::boxpart::Left);   // (the same again changes nothing)
        QCOMPARE(w.document()->nextStepSerial(), serial + 1);
        QVERIFY(saveInto(w, dir, QStringLiteral("sides.docx")));
        const QByteArray one = pBdr(QStringLiteral("sides.docx"), 0);
        QCOMPARE(one, "<w:pBdr>" + top + "<w:left w:val=\"single\" w:sz=\"12\" w:space=\"3\" w:color=\"0000FF\"/>" + bottom + bar + "</w:pBdr>");
        // Both paragraphs selected: the sides they do not share show as mixed and stay, the one set goes to both.
        sel.setPosition(1);
        sel.setPosition(w.document()->story()->findBlockByNumber(1).position() + 2, QTextCursor::KeepAnchor);
        serial = w.document()->nextStepSerial();
        withDialog([&] { w.act(QStringLiteral("para.bordersShading"))->trigger(); }, QStringLiteral("paragraphBordersDialog"), [](QDialog *d) {
            QCheckBox *topBox = d->findChild<QCheckBox *>(QStringLiteral("top")), *bottomBox = d->findChild<QCheckBox *>(QStringLiteral("bottom")), *rightBox = d->findChild<QCheckBox *>(QStringLiteral("right"));
            QVERIFY(topBox && bottomBox && rightBox);
            QCOMPARE(topBox->checkState(), Qt::PartiallyChecked);
            QCOMPARE(bottomBox->checkState(), Qt::PartiallyChecked);   // (a different line on each)
            QVERIFY(!rightBox->isChecked());
            rightBox->setChecked(true);
        });
        QCOMPARE(w.document()->nextStepSerial(), serial + 1);
        QVERIFY(saveInto(w, dir, QStringLiteral("sides2.docx")));
        const QByteArray first = pBdr(QStringLiteral("sides2.docx"), 0), second = pBdr(QStringLiteral("sides2.docx"), 1);
        QVERIFY(first.contains(top) && first.contains(bottom) && first.contains(bar) && first.contains("<w:left w:val=\"single\" w:sz=\"12\" w:space=\"3\" w:color=\"0000FF\"/>"));
        QVERIFY(first.contains("<w:right "));
        QVERIFY(!second.contains("<w:top") && second.contains("<w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"1\" w:color=\"auto\"/>") && second.contains("<w:right "));
    }

    // A font color picked from the palette's theme row is the color the swatch showed, in the window and in the file saved and opened again, for every slot and every tint the picker offers.
    void aThemeFontColorIsTheColorOfItsSwatchInTheWindowAndInTheSavedFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("word"));
        QVERIFY(w.document()->theme());
        const ColorScheme colors = w.document()->flow().env.colors;
        auto shown = [&](WriteWindow &win) {
            QTextCursor c(win.document()->story());
            c.setPosition(2);
            return c.charFormat().foreground().color();
        };
        QTextCursor sel(w.document()->story());
        sel.setPosition(0);
        sel.setPosition(4, QTextCursor::KeepAnchor);
        int checked = 0;
        for (int slot = 0; slot < SlotCount; ++slot)
            for (const QPair<int, int> &tint : QList<QPair<int, int>>{{0, 0}, {20, 0}, {40, 0}, {60, 0}, {80, 0}, {0, 30}}) {
                const ColorRef ref = ColorRef::scheme(slot, tint.first, tint.second);
                write::setTextColorRef(*w.document(), sel, ref);
                QCOMPARE(shown(w).name(), ref.resolve(colors).name());
                ++checked;
            }
        QCOMPARE(checked, SlotCount * 6);
        const ColorRef ref = ColorRef::scheme(Accent2, 40);
        write::setTextColorRef(*w.document(), sel, ref);
        QVERIFY(saveInto(w, dir, QStringLiteral("theme.docx")));
        WriteWindow v;
        Quiet quietV(v);
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("theme.docx"))));
        QCOMPARE(shown(v).name(), ref.resolve(colors).name());
        office::Package package;
        package.read(fileBytes(dir.filePath(QStringLiteral("theme.docx"))));
        QVERIFY(package.part(QStringLiteral("/word/document.xml")).contains("w:themeColor=\"accent2\""));
    }

    // Options > Save: a window that is open takes a new recovery interval at once, whatever its size (the range Options offers; a value outside it, kept in the settings by hand, is brought in).
    void theRecoveryTimerOfAnOpenWindowFollowsEachNewInterval()
    {
        Settings &st = Settings::get();
        const QVariant old = st.value(QStringLiteral("save/autoRecoverMinutes"));
        WriteWindow w;
        Quiet quiet(w);
        w.show();
        QVector<OptionsPage> pages = w.optionsPages();
        QList<QWidget *> built;
        for (const OptionsPage &p : pages) built << p.build();
        QSpinBox *minutes = nullptr;
        for (QWidget *page : built)
            if (auto *f = page->findChild<QSpinBox *>(QStringLiteral("autoRecoverMinutes"))) minutes = f;
        QVERIFY(minutes);
        for (const int value : {3, 120, 1}) {
            minutes->setValue(value);
            for (const OptionsPage &p : pages)
                if (p.apply) p.apply();
            QCOMPARE(w.recoveryIntervalMs(), value * 60 * 1000);
        }
        minutes->setValue(0);   // (the box stops at its least)
        QCOMPARE(minutes->value(), minutes->minimum());
        qDeleteAll(built);
        // A value kept by hand: none or less is the shortest, and a huge one the longest (it would overflow a timer's milliseconds unbrought in).
        for (const QPair<int, int> &c : QList<QPair<int, int>>{{0, 1}, {-5, 1}, {100000, 120}, {std::numeric_limits<int>::max(), 120}}) {
            st.setValue(QStringLiteral("save/autoRecoverMinutes"), c.first);
            Q_EMIT AutoRecoverNotifier::get().changed(c.first);
            QCOMPARE(w.recoveryIntervalMs(), c.second * 60 * 1000);
            WriteWindow fresh;
            QCOMPARE(fresh.recoveryIntervalMs(), c.second * 60 * 1000);
        }
        st.setValue(QStringLiteral("save/autoRecoverMinutes"), old.isValid() ? old : QVariant(10));
    }

    // A comment's author and initials that need escaping in XML (an ampersand, angle brackets, quotes) or are not Latin survive the save and the opening of the file.
    void aCommentAuthorNeedingXmlEscapingSurvivesSaveAndOpen()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString author = QStringLiteral("Zoë \"Z\" <Smith> & O'Neil 山田"), initials = QStringLiteral("Z&<山");
        Settings::get().setValue(QStringLiteral("user/name"), author);
        Settings::get().setValue(QStringLiteral("user/initials"), initials);
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("some words to remark on"));
        w.document()->setReviewer(author, initials);
        QTextCursor c(w.document()->story());
        c.setPosition(5, QTextCursor::KeepAnchor);
        const int made = write::addComment(*w.document(), c, QStringLiteral("A <remark> & \"more\""));
        QVERIFY(made >= 0);
        QVERIFY(saveInto(w, dir, QStringLiteral("escaped.docx")));
        office::Package package;
        package.read(fileBytes(dir.filePath(QStringLiteral("escaped.docx"))));
        const QByteArray comments = package.part(QStringLiteral("/word/comments.xml"));
        QVERIFY(!comments.isEmpty());
        QVERIFY(!comments.contains("<Smith>") && !comments.contains("\"Z\""));
        QVERIFY(comments.contains("&amp;") && comments.contains("&lt;Smith&gt;"));
        WriteWindow v;
        Quiet quietV(v);
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("escaped.docx"))));
        const std::optional<write::Comment> back = write::commentById(*v.document(), made);
        QVERIFY(back.has_value());
        QCOMPARE(back->author, author);
        QCOMPARE(back->initials, initials);
        QCOMPARE(back->text, QStringLiteral("A <remark> & \"more\""));
        Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Tester"));
        Settings::get().setValue(QStringLiteral("user/initials"), QString());
    }

    // The Help page on Find and Replace keeps the Search choice in the list of choices that narrow the search: one list, no blank line between its items.
    void theFindAndReplaceHelpListsTheSearchChoiceWithTheOthers()
    {
        const QString md = jp::help::topic(QStringLiteral("find-replace")).markdown;
        const int heading = md.indexOf(QStringLiteral("## Narrow the search"));
        QVERIFY(heading >= 0);
        const int next = md.indexOf(QStringLiteral("\n## "), heading + 5);
        const QString section = md.mid(heading, next - heading);
        QVERIFY(section.contains(QStringLiteral("- **Search**")));
        QVERIFY(!section.mid(section.indexOf(QStringLiteral("- **Match case**"))).contains(QStringLiteral("\n\n- ")));
        QTextDocument shown;
        shown.setMarkdown(section);
        QCOMPARE(shown.toHtml().count(QStringLiteral("<ul")), 1);
        QCOMPARE(shown.toHtml().count(QStringLiteral("<li")), 3);
    }

    // Check Compatibility lists what saving as an .odt flattens in paragraph borders and shading (a pattern, a theme color, a line style the format lacks, a bar), and a solid fill is kept.
    void theCompatibilityCheckListsWhatAnOdtFlattensInParagraphBoxes()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto window = windowOn(dir, styledBoxesDocx());
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        const QString kept = w.incompatibilities().join(QLatin1Char('|'));
        QVERIFY2(kept.contains(QStringLiteral("shading patterns")), qPrintable(kept));
        QVERIFY2(kept.contains(QStringLiteral("theme colors of paragraph borders and shading")), qPrintable(kept));
        QVERIFY2(kept.contains(QStringLiteral("border lines other than")), qPrintable(kept));
        QVERIFY2(kept.contains(QStringLiteral("the bar beside a paragraph")), qPrintable(kept));
        QString error;
        QStringList notes;
        const QByteArray odt = w.document()->toOdt(&error, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(error));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(odt, parts));
        const QByteArray styles = parts.value(QStringLiteral("styles.xml")) + parts.value(QStringLiteral("content.xml"));
        QVERIFY(styles.contains("fo:background-color=\"#FFE0E0\""));   // (QuoteA's solid fill, which names its color and no fill)
    }

    // Line spacing: the Paragraph box's at least, exactly, and multiple kinds, and "don't add space between paragraphs of the same style", are written as the file writes them
    // (ECMA-376 Part 1 sections 17.3.1.33 and 17.3.1.9), and a file opened again through the window has the same kinds.
    void lineSpacingKindsAndTheSameStyleBoxSurviveSaveAndOpen()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow w;
        Quiet quiet(w);
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("one\ntwo\nthree"));
        auto setSpacing = [&](int block, int kind, double value, bool sameStyle) {
            QTextCursor &c = w.view()->cursor();
            c.setPosition(w.document()->story()->findBlockByNumber(block).position() + 1);
            withDialog([&] { w.act(QStringLiteral("para.dialog"))->trigger(); }, QStringLiteral("paragraphDialog"), [&](QDialog *d) {
                auto *type = d->findChild<QComboBox *>(QStringLiteral("lineSpacing"));
                QVERIFY(type);
                type->setCurrentIndex(kind);
                if (kind == 5) d->findChild<QDoubleSpinBox *>(QStringLiteral("multiple"))->setValue(value);
                else if (kind >= 3) d->findChild<MeasureSpin *>(QStringLiteral("at"))->setPoints(value);
                d->findChild<QCheckBox *>(QStringLiteral("contextualSpacing"))->setChecked(sameStyle);
            });
        };
        setSpacing(0, 3, 20, false);   // at least 20 pt
        setSpacing(1, 4, 18, true);    // exactly 18 pt, and no space between paragraphs of the same style
        setSpacing(2, 5, 1.15, false);   // multiple 1.15
        QVERIFY(saveInto(w, dir, QStringLiteral("spacing.docx")));
        office::Package package;
        package.read(fileBytes(dir.filePath(QStringLiteral("spacing.docx"))));
        const QByteArray xml = package.part(QStringLiteral("/word/document.xml"));
        QVERIFY2(xml.contains("w:line=\"400\" w:lineRule=\"atLeast\""), xml.left(1500).constData());
        QVERIFY2(xml.contains("w:line=\"360\" w:lineRule=\"exact\""), xml.left(1500).constData());
        QVERIFY2(xml.contains("w:line=\"276\" w:lineRule=\"auto\""), xml.left(1500).constData());
        QCOMPARE(xml.count("<w:contextualSpacing"), 1);
        WriteWindow v;
        Quiet quietV(v);
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("spacing.docx"))));
        const QTextBlockFormat a = v.document()->story()->findBlockByNumber(0).blockFormat(), b = v.document()->story()->findBlockByNumber(1).blockFormat(),
                               c = v.document()->story()->findBlockByNumber(2).blockFormat();
        QCOMPARE(int(a.lineHeightType()), int(QTextBlockFormat::MinimumHeight));
        QCOMPARE(a.lineHeight(), 20.0);
        QCOMPARE(int(b.lineHeightType()), int(QTextBlockFormat::FixedHeight));
        QCOMPARE(b.lineHeight(), 18.0);
        QVERIFY(b.boolProperty(tp::ContextualSpacing) && !a.boolProperty(tp::ContextualSpacing));
        QCOMPARE(int(c.lineHeightType()), int(QTextBlockFormat::ProportionalHeight));
        QVERIFY(std::abs(c.lineHeight() - 115.0) < 0.5);
    }

    // Save As lists every kind JeffWrite writes, a new document starts on .docx, and each kind saves and opens again through the window's own Save As and Open.
    void saveAsListsEveryKindAndEachSavesAndOpensAgain()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("alpha beta\ngamma delta"));
        w.setTextFormChooser([](write::TextForm *, const QByteArray *) { return true; });
        QString suggested, selected;
        w.setSavePathChooser([&](const QString &s, const QString &f) {
            suggested = s;
            selected = f;
            return QString();
        });
        QVERIFY(!w.saveAs());
        QVERIFY2(suggested.endsWith(QLatin1String(".docx")), qPrintable(suggested));
        QVERIFY(selected.contains(QLatin1String("*.docx")));
        const QStringList offered = w.offeredSaveFilters();
        const QStringList endings = {QStringLiteral("docx"), QStringLiteral("dotx"), QStringLiteral("odt"), QStringLiteral("ott"), QStringLiteral("docm"), QStringLiteral("dotm"), QStringLiteral("txt"), QStringLiteral("rtf")};
        QCOMPARE(offered.size(), endings.size());
        for (const QString &ending : endings) {
            bool listed = false;
            for (const QString &f : offered) listed = listed || f.contains(QLatin1String("*.") + ending + QLatin1Char(')'));
            QVERIFY2(listed, qPrintable(ending));
            const QString path = dir.filePath(QStringLiteral("kind.") + ending);
            QVERIFY2(saveInto(w, dir, QStringLiteral("kind.") + ending), qPrintable(ending));
            QVERIFY2(QFileInfo(path).size() > 0, qPrintable(ending));
            WriteWindow v;
            Quiet quietV(v);
            QVERIFY2(v.openFile(path), qPrintable(ending));
            QVERIFY2(v.document()->story()->toPlainText().startsWith(QStringLiteral("alpha beta")), qPrintable(ending));
            QVERIFY2(v.document()->story()->toPlainText().contains(QStringLiteral("gamma delta")), qPrintable(ending));
            // (Save As moved the first window to the file: the next kind is chosen from it; a template opens as a new document, and is left unsaved)
        }
    }

    // Options > Save > recovery interval: a window's recovery timer runs at the minutes Options names.
    void theRecoveryTimerFollowsTheOptionsInterval()
    {
        Settings &st = Settings::get();
        const QVariant old = st.value(QStringLiteral("save/autoRecoverMinutes"));
        st.setValue(QStringLiteral("save/autoRecoverMinutes"), 3);
        {
            WriteWindow w;
            QCOMPARE(w.recoveryIntervalMs(), 3 * 60 * 1000);
        }
        st.setValue(QStringLiteral("save/autoRecoverMinutes"), 7);
        {
            WriteWindow w;
            QCOMPARE(w.recoveryIntervalMs(), 7 * 60 * 1000);
        }
        st.setValue(QStringLiteral("save/autoRecoverMinutes"), old.isValid() ? old : QVariant(10));
    }

    // Options > General: the name and initials typed there are who comments in the open document, and the saved .docx and its reopened comment keep both.
    void theNameAndInitialsOfOptionsAreOnTheCommentsOfASavedFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("some words to remark on"));
        withDialog([&] { w.act(QStringLiteral("file.options"))->trigger(); }, QStringLiteral("optionsDialog"), [](QDialog *d) {
            auto *name = d->findChild<QLineEdit *>(QStringLiteral("userName"));
            auto *initials = d->findChild<QLineEdit *>(QStringLiteral("userInitials"));
            QVERIFY(name && initials);
            name->setText(QStringLiteral("Quincy Xavier"));
            initials->setText(QStringLiteral("QX"));
        });
        QCOMPARE(w.document()->reviewerInitials(), QStringLiteral("QX"));
        QTextCursor c(w.document()->story());
        c.setPosition(5, QTextCursor::KeepAnchor);
        const int made = write::addComment(*w.document(), c, QStringLiteral("A remark"));
        QVERIFY(made >= 0);
        QVERIFY(saveInto(w, dir, QStringLiteral("who.docx")));
        WriteWindow v;
        Quiet quietV(v);
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("who.docx"))));
        const std::optional<write::Comment> back = write::commentById(*v.document(), made);
        QVERIFY(back.has_value());
        QCOMPARE(back->author, QStringLiteral("Quincy Xavier"));
        QCOMPARE(back->initials, QStringLiteral("QX"));
        Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Tester"));
        Settings::get().setValue(QStringLiteral("user/initials"), QString());
    }

    void bookmarksAreAddedToAPdfAndItStillOpens()
    {
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        {
            QPdfWriter pdf(&buf);
            pdf.setPageSize(QPageSize(QPageSize::Letter));
            QPainter p(&pdf);
            for (int i = 0; i < 3; ++i) {
                if (i) pdf.newPage();
                p.drawText(100, 200, QStringLiteral("page %1").arg(i));
            }
        }
        const QByteArray plain = buf.data();
        const QVector<PdfBookmark> marks{{QStringLiteral("Intro"), 0, 0}, {QStringLiteral("Part A"), 1, 1}, {QStringLiteral("Caf\u00E9"), 1, 2}, {QStringLiteral("End"), 0, 2}};
        const QByteArray out = addPdfBookmarks(plain, marks);
        // an update at the end: the pages are as they were, and the file still opens with all its pages
        QVERIFY(out.size() > plain.size());
        QVERIFY(out.startsWith(plain));
        auto doc = PdfDocument::shared(out);
        QVERIFY(doc);
        QCOMPARE(doc->pageCount(), 3);
        QVERIFY(out.contains("/Type /Outlines"));
        QVERIFY(out.contains("/PageMode /UseOutlines"));
        for (const PdfBookmark &m : marks) QVERIFY2(out.contains(utf16Hex(m.title)), qPrintable(m.title));
        // Intro holds Part A and Cafe: it shows two below it; the outline shows all four
        QVERIFY(out.contains("/Count 4"));
        QVERIFY(out.contains("/Count 2"));
        // a PDF that has an outline, or no marks to add, is left alone
        QCOMPARE(addPdfBookmarks(out, marks), out);
        QCOMPARE(addPdfBookmarks(plain, {}), plain);
        QCOMPARE(addPdfBookmarks(QByteArray("not a pdf"), marks), QByteArray("not a pdf"));
        QVERIFY(addPdfBookmarks(plain.left(plain.size() - 30), marks).size() <= plain.size());   // (a cut-off file is not extended)
    }

    void exportedPdfHasBookmarksFromHeadingsAndTheDocumentProperties()
    {
        QTemporaryDir dir;
        const QString docx = dir.filePath(QStringLiteral("h.docx"));
        QByteArray body = para("Heading1", "First part") + para(nullptr, "text") + para("Heading2", "Sub part") + kPageBreak + para("Heading1", "Second part") + para(nullptr, "more");
        {
            QFile f(docx);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(headingsDocx(body));
        }
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(docx));
        office::PackageProps props = w.document()->packageProps();
        props.title = QStringLiteral("A Report");
        props.creator = QStringLiteral("An Author");
        props.keywords = QStringLiteral("alpha, beta");
        w.document()->setPackageProps(props);
        const QString pdfPath = dir.filePath(QStringLiteral("h.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return pdfPath; });
        QVERIFY(w.exportPdf());
        QFile f(pdfPath);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        QVERIFY(pdf.contains("/Type /Outlines"));
        QVERIFY(pdf.contains(utf16Hex(QStringLiteral("First part"))));
        QVERIFY(pdf.contains(utf16Hex(QStringLiteral("Sub part"))));
        QVERIFY(pdf.contains(utf16Hex(QStringLiteral("Second part"))));
        QVERIFY(pdf.contains("An Author"));
        QVERIFY(pdf.contains("alpha, beta"));
        QVERIFY(pdf.contains("A Report"));
        QCOMPARE(PdfDocument::shared(pdf)->pageCount(), 2);
    }

    // Opens a window on the rich test document and makes the two paragraphs "item one" and "item two" a bulleted list.
    static void openRich(WriteWindow &w, const QTemporaryDir &dir, const QString &alt = QStringLiteral("A noisy logo"))
    {
        const QString docx = dir.filePath(QStringLiteral("rich.docx"));
        QFile f(docx);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(richDocx(alt));
        f.close();
        QVERIFY(w.openFile(docx));
        const QString text = w.document()->story()->toPlainText();
        QTextCursor c(w.document()->story());
        c.setPosition(int(text.indexOf(QStringLiteral("item one"))));
        c.setPosition(int(text.indexOf(QStringLiteral("item two"))) + 4, QTextCursor::KeepAnchor);
        toggleList(*w.document(), c, false);
    }

    // Save as PDF has the choices of the reference: Standard or Minimum size, the pages, the document properties, and the bookmarks from headings.
    void theSaveAsPdfBoxHasTheSwitchesAndTheyTakeEffect()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        office::PackageProps props = w.document()->packageProps();
        props.creator = QStringLiteral("Zed Author");
        w.document()->setPackageProps(props);
        QString target;
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        // Runs the box, sets what `set` sets, and returns the PDF.
        auto run = [&](const QString &name, const std::function<void(QDialog *)> &set) {
            target = dir.filePath(name);
            bool found = false;
            std::function<void(int)> answer = [&](int tries) {
                QTimer::singleShot(20, [&, tries] {
                    auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                    if (!d || d->objectName() != QLatin1String("pdfDialog")) {
                        if (tries > 0) answer(tries - 1);
                        return;
                    }
                    found = true;
                    set(d);
                    d->accept();
                });
            };
            answer(250);
            const bool ok = w.exportPdfAsk();
            return ok && found ? fileBytes(target) : QByteArray();
        };
        auto check = [](QDialog *d, const char *name, bool on) {
            auto *box = d->findChild<QCheckBox *>(QString::fromLatin1(name));
            QVERIFY2(box, name);
            box->setChecked(on);
        };
        const QByteArray standard = run(QStringLiteral("standard.pdf"), [](QDialog *) {});
        QVERIFY(standard.startsWith("%PDF"));
        QVERIFY(standard.contains("/Type /Outlines"));
        QVERIFY(standard.contains("Zed Author"));
        QCOMPARE(widestPictureIn(standard), 600);
        const QByteArray minimum = run(QStringLiteral("minimum.pdf"), [](QDialog *d) {
            auto *size = d->findChild<QComboBox *>(QStringLiteral("pdfSize"));
            QVERIFY(size);
            size->setCurrentIndex(1);
        });
        QVERIFY(minimum.startsWith("%PDF"));
        // 2 inches at 150 pixels to the inch is 300 pixels (a little over is the rounding of the downsampling)
        QVERIFY2(widestPictureIn(minimum) > 0 && widestPictureIn(minimum) <= 320, qPrintable(QString::number(widestPictureIn(minimum))));
        QVERIFY(minimum.size() < standard.size());
        const QByteArray bare = run(QStringLiteral("bare.pdf"), [&](QDialog *d) {
            check(d, "pdfProperties", false);
            check(d, "pdfBookmarks", false);
        });
        QVERIFY(bare.startsWith("%PDF"));
        QVERIFY(!bare.contains("/Type /Outlines"));
        QVERIFY(!bare.contains("Zed Author"));
        // the page range is still in the box
        QVERIFY(!run(QStringLiteral("range.pdf"), [](QDialog *d) { QVERIFY(d->findChild<QComboBox *>(QStringLiteral("pdfRange"))); }).isEmpty());
    }

    // The PDF that Print makes to a file has the bookmarks and the tags too.
    void thePdfThatPrintMakesHasBookmarksAndTags()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        const QString target = dir.filePath(QStringLiteral("printed.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        WriteOutputOptions o;
        o.toFile = true;
        QVERIFY(w.printOutput(o));
        const QByteArray pdf = fileBytes(target);
        QVERIFY(pdf.startsWith("%PDF"));
        QVERIFY(pdf.contains("/Type /Outlines"));
        QVERIFY(pdf.contains(utf16Hex(QStringLiteral("First part"))));
        QVERIFY(pdf.contains("/StructTreeRoot"));
        QCOMPARE(pdfcheck::marksLeft(pdf), 0);
    }

    // The PDF carries the document's properties in its information dictionary (what a reader lists under Properties): the title, author, subject, and keywords the document holds, the date
    // it was made, and this program as its creator and producer.
    void thePdfCarriesTheDocumentProperties()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        office::PackageProps props = w.document()->packageProps();
        props.title = QStringLiteral("Quarterly Plan");
        props.creator = QStringLiteral("Zed Author");
        props.subject = QStringLiteral("Budget");
        props.keywords = QStringLiteral("plan, budget");
        w.document()->setPackageProps(props);
        const QString target = dir.filePath(QStringLiteral("props.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        const QDateTime before = QDateTime::currentDateTimeUtc().addSecs(-2);
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        const QDateTime after = QDateTime::currentDateTimeUtc().addSecs(2);
        const PdfDocument pdf(fileBytes(target));
        QVERIFY(pdf.isValid());
        QCOMPARE(pdf.meta(QStringLiteral("Title")), QStringLiteral("Quarterly Plan"));
        QCOMPARE(pdf.meta(QStringLiteral("Author")), QStringLiteral("Zed Author"));
        QCOMPARE(pdf.meta(QStringLiteral("Subject")), QStringLiteral("Budget"));
        QCOMPARE(pdf.meta(QStringLiteral("Keywords")), QStringLiteral("plan, budget"));
        QCOMPARE(pdf.meta(QStringLiteral("Producer")), appInfo().name);
        QCOMPARE(pdf.meta(QStringLiteral("Creator")), appInfo().name);
        const QString date = pdf.meta(QStringLiteral("CreationDate"));
        QVERIFY2(date.startsWith(QStringLiteral("D:")), qPrintable(date));
        QDateTime made = QDateTime::fromString(date.mid(2, 14), QStringLiteral("yyyyMMddHHmmss"));
        made.setTimeZone(QTimeZone::utc());
        QVERIFY2(made.isValid() && made >= before && made <= after, qPrintable(date));
        // with the properties switched off the document's words are not in it; the program still is
        WriteOutputOptions bare;
        bare.pdfProperties = false;
        QVERIFY(w.exportPdfPages(bare));
        const PdfDocument plain(fileBytes(target));
        QCOMPARE(plain.meta(QStringLiteral("Author")), QString());
        QCOMPARE(plain.meta(QStringLiteral("Subject")), QString());
        QCOMPARE(plain.meta(QStringLiteral("Title")), QString());
        QCOMPARE(plain.meta(QStringLiteral("Producer")), appInfo().name);
    }

    // Personal information that was removed, or is to be removed on saving, stays out of the PDF: the author, and a title made from the file's name, in the information, in the metadata,
    // and in every place the bytes hold them (a name is looked for as the strings of the information write it, which a plain search of the bytes never finds).
    void thePdfKeepsRemovedPersonalInformationOut()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        office::PackageProps props = w.document()->packageProps();
        props.title.clear();
        props.creator = QStringLiteral("Zed Author");
        w.document()->setPackageProps(props);
        QVERIFY(saveInto(w, dir, QStringLiteral("Zed Author's Plan.docx")));
        const QString name = w.displayName();
        QVERIFY(name.contains(QStringLiteral("Zed Author")));
        const QString target = dir.filePath(QStringLiteral("names.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        WriteOutputOptions toFile;
        toFile.toFile = true;
        // (with nothing removed, the author and the file's name are in it: the search can find them)
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        QByteArray bytes = fileBytes(target);
        QCOMPARE(PdfDocument(bytes).meta(QStringLiteral("Author")), QStringLiteral("Zed Author"));
        QCOMPARE(PdfDocument(bytes).meta(QStringLiteral("Title")), name);
        QVERIFY(holdsText(bytes, QStringLiteral("Zed Author")));
        for (const bool printed : {false, true}) {
            const bool was = privacy::removeOnSave();
            const auto restore = qScopeGuard([was] { privacy::setRemoveOnSave(was); });
            privacy::setRemoveOnSave(true);
            QVERIFY(printed ? w.printOutput(toFile) : w.exportPdfPages(WriteOutputOptions()));
            bytes = fileBytes(target);
            QVERIFY(PdfDocument(bytes).isValid());
            QCOMPARE(PdfDocument(bytes).meta(QStringLiteral("Author")), QString());
            QCOMPARE(PdfDocument(bytes).meta(QStringLiteral("Title")), QString());
            QVERIFY2(!holdsText(bytes, QStringLiteral("Zed Author")), printed ? "printed" : "exported");
            QVERIFY2(!holdsText(bytes, name), printed ? "printed" : "exported");
            privacy::setRemoveOnSave(false);
        }
        // (Inspect > Remove personal information: the names are gone from the document itself, and its file name stays out of the title)
        w.removePersonalInformation();
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        bytes = fileBytes(target);
        QCOMPARE(PdfDocument(bytes).meta(QStringLiteral("Author")), QString());
        QCOMPARE(PdfDocument(bytes).meta(QStringLiteral("Title")), QString());
        QVERIFY(!holdsText(bytes, QStringLiteral("Zed Author")));
        QVERIFY(!holdsText(bytes, name));
    }

    // Subject and keywords are kept whole up to 4,000 characters, and cut at a character, never in the middle of one.
    void longPropertiesAreKeptWholeAndCutAtACharacter()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        office::PackageProps props = w.document()->packageProps();
        props.title = QStringLiteral("Long");
        props.subject = QString(3000, QLatin1Char('s'));
        // a pair of surrogates that stands across the cut: the whole character goes
        props.keywords = QString(kMostPdfInfoChars - 1, QLatin1Char('k')) + QString::fromUtf8("\xF0\x9F\x98\x80") + QStringLiteral("tail");
        w.document()->setPackageProps(props);
        const QString target = dir.filePath(QStringLiteral("long.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        const PdfDocument pdf(fileBytes(target));
        QCOMPARE(pdf.meta(QStringLiteral("Subject")), props.subject);
        QCOMPARE(pdf.meta(QStringLiteral("Keywords")), QString(kMostPdfInfoChars - 1, QLatin1Char('k')));
        // (a keyword of text that is all whole characters stays whole to the cut)
        props.keywords = QString(kMostPdfInfoChars + 500, QChar(0x00E9));
        w.document()->setPackageProps(props);
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        QCOMPARE(PdfDocument(fileBytes(target)).meta(QStringLiteral("Keywords")), QString(kMostPdfInfoChars, QChar(0x00E9)));
    }

    // A PDF/A file's information and its metadata say the same: the entries of the one are the fields of the other, whether the document's properties are in it or not.
    void aPdfAHasInformationAndMetadataThatAgree()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        office::PackageProps props = w.document()->packageProps();
        props.title = QStringLiteral("Archive Plan");
        props.creator = QStringLiteral("Zed Author");
        props.subject = QStringLiteral("Budget");
        props.keywords = QStringLiteral("plan, budget");
        w.document()->setPackageProps(props);
        const QString target = dir.filePath(QStringLiteral("archive-props.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        for (const bool properties : {true, false}) {
            WriteOutputOptions o;
            o.pdfArchive = true;
            o.pdfProperties = properties;
            QVERIFY(w.exportPdfPages(o));
            const QByteArray bytes = fileBytes(target);
            QVERIFY(bytes.contains("<pdfaid:part>1</pdfaid:part>"));
            const PdfDocument pdf(bytes);
            QVERIFY(pdf.isValid());
            QCOMPARE(xmpField(bytes, "dc:title"), pdf.meta(QStringLiteral("Title")));
            QCOMPARE(xmpField(bytes, "dc:creator"), pdf.meta(QStringLiteral("Author")));
            QCOMPARE(xmpField(bytes, "dc:description"), pdf.meta(QStringLiteral("Subject")));
            QCOMPARE(xmpField(bytes, "pdf:Keywords"), pdf.meta(QStringLiteral("Keywords")));
            QCOMPARE(xmpField(bytes, "pdf:Producer"), pdf.meta(QStringLiteral("Producer")));
            QCOMPARE(xmpField(bytes, "xmp:CreatorTool"), pdf.meta(QStringLiteral("Creator")));
            QVERIFY(!pdf.meta(QStringLiteral("Producer")).isEmpty());
            QString xmpDate = xmpField(bytes, "xmp:CreateDate");
            xmpDate.remove(QRegularExpression(QStringLiteral("[^0-9]")));
            QCOMPARE(xmpDate, pdf.meta(QStringLiteral("CreationDate")).mid(2, 14));
            QCOMPARE(pdf.meta(QStringLiteral("Author")), properties ? QStringLiteral("Zed Author") : QString());
            QCOMPARE(pdf.meta(QStringLiteral("Keywords")), properties ? QStringLiteral("plan, budget") : QString());
        }
    }

    // A PDF with a password has the properties too, once it is opened with the password.
    void anEncryptedPdfStillHasTheProperties()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        office::PackageProps props = w.document()->packageProps();
        props.title = QStringLiteral("Locked Plan");
        props.creator = QStringLiteral("Zed Author");
        props.keywords = QStringLiteral("locked");
        w.document()->setPackageProps(props);
        const QString target = dir.filePath(QStringLiteral("locked.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        WriteOutputOptions o;
        o.pdfProtect.openPassword = QStringLiteral("open sesame");
        QVERIFY(w.exportPdfPages(o));
        const QByteArray bytes = fileBytes(target);
        QVERIFY(!PdfDocument(bytes).isValid());   // (not without the password)
        const PdfDocument pdf(bytes, QStringLiteral("open sesame"));
        QVERIFY(pdf.isValid());
        QCOMPARE(pdf.meta(QStringLiteral("Title")), QStringLiteral("Locked Plan"));
        QCOMPARE(pdf.meta(QStringLiteral("Author")), QStringLiteral("Zed Author"));
        QCOMPARE(pdf.meta(QStringLiteral("Keywords")), QStringLiteral("locked"));
        QCOMPARE(pdf.meta(QStringLiteral("Producer")), appInfo().name);
    }

    // The PDF that Print makes to a file has the properties too.
    void thePdfThatPrintMakesHasTheProperties()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        office::PackageProps props = w.document()->packageProps();
        props.title = QStringLiteral("Printed Plan");
        props.creator = QStringLiteral("Zed Author");
        props.keywords = QStringLiteral("print, plan");
        w.document()->setPackageProps(props);
        const QString target = dir.filePath(QStringLiteral("printed-props.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        WriteOutputOptions o;
        o.toFile = true;
        QVERIFY(w.printOutput(o));
        const PdfDocument pdf(fileBytes(target));
        QVERIFY(pdf.isValid());
        QCOMPARE(pdf.meta(QStringLiteral("Title")), QStringLiteral("Printed Plan"));
        QCOMPARE(pdf.meta(QStringLiteral("Author")), QStringLiteral("Zed Author"));
        QCOMPARE(pdf.meta(QStringLiteral("Keywords")), QStringLiteral("print, plan"));
        QCOMPARE(pdf.meta(QStringLiteral("Producer")), appInfo().name);
        QVERIFY(pdf.outline().size() > 0);   // (the bookmarks added after it are still read)
    }

    // "Open the PDF after saving" is off until it is asked for, is remembered, and opens the file through the program's own opener; nothing is opened without it.
    void aPdfIsOpenedAfterSavingOnlyWhenAskedFor()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        const QString target = dir.filePath(QStringLiteral("open.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        const auto openBefore = help::openUrl;
        const auto restore = qScopeGuard([&] {
            help::openUrl = openBefore;
            setOpenPdfAfterSaving(false);
        });
        QList<QUrl> opened;
        help::openUrl = [&](const QUrl &url) {
            opened << url;
            return true;
        };
        setOpenPdfAfterSaving(false);
        QVERIFY(!openPdfAfterSaving());
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        WriteOutputOptions toFile;
        toFile.toFile = true;
        QVERIFY(w.printOutput(toFile));
        QVERIFY(opened.isEmpty());
        WriteOutputOptions ask;
        ask.openAfter = true;
        QVERIFY(w.exportPdfPages(ask));
        QCOMPARE(opened.size(), 1);
        QCOMPARE(opened.first().toLocalFile(), target);
        toFile.openAfter = true;
        QVERIFY(w.printOutput(toFile));
        QCOMPARE(opened.size(), 2);
        // a PDF that was not made is not opened
        w.setSavePathChooser([&](const QString &, const QString &) { return QString(); });
        QVERIFY(!w.exportPdfPages(ask));
        QCOMPARE(opened.size(), 2);
        // the Export box has the switch, unchecked, and what it says is kept
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        bool seen = false, wasOn = true;
        std::function<void(int)> answer = [&](int tries) {
            QTimer::singleShot(20, [&, tries] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                auto *box = d && d->objectName() == QLatin1String("pdfDialog") ? d->findChild<QCheckBox *>(QStringLiteral("pdfOpenAfter")) : nullptr;
                if (!box) {
                    if (tries > 0) answer(tries - 1);
                    return;
                }
                seen = true;
                wasOn = box->isChecked();
                QVERIFY(!box->accessibleName().isEmpty() || !box->text().isEmpty());
                box->setChecked(true);
                d->accept();
            });
        };
        answer(250);
        QVERIFY(w.exportPdfAsk());
        QVERIFY(seen);
        QVERIFY(!wasOn);
        QCOMPARE(opened.size(), 3);
        QVERIFY(openPdfAfterSaving());
    }

    // When the system won't open the saved PDF, the status bar says so (the file is saved all the same); when it does, it says nothing of the kind.
    void aPdfThatTheSystemWillNotOpenSaysSo()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        const QString target = dir.filePath(QStringLiteral("noviewer.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        const auto openBefore = help::openUrl;
        const auto restore = qScopeGuard([&] { help::openUrl = openBefore; });
        bool answer = false;
        help::openUrl = [&](const QUrl &) { return answer; };
        WriteOutputOptions ask;
        ask.openAfter = true;
        WriteOutputOptions toFile = ask;
        toFile.toFile = true;
        for (const bool printed : {false, true}) {
            w.statusBar()->clearMessage();
            answer = true;
            QVERIFY(printed ? w.printOutput(toFile) : w.exportPdfPages(ask));
            QVERIFY2(!w.statusBar()->currentMessage().contains(QStringLiteral("would not open")), qPrintable(w.statusBar()->currentMessage()));
            answer = false;
            QFile::remove(target);
            QVERIFY(printed ? w.printOutput(toFile) : w.exportPdfPages(ask));
            QVERIFY(QFileInfo::exists(target));
            QVERIFY2(w.statusBar()->currentMessage().contains(QStringLiteral("noviewer.pdf")) && w.statusBar()->currentMessage().contains(QStringLiteral("would not open")), qPrintable(w.statusBar()->currentMessage()));
        }
    }

    // The printer is kept from job to job: a job that made a PDF file leaves no file name on it, so that the next job, on paper, is not printed to that file.
    void aPaperJobAfterAPdfJobIsNotPrintedToTheFile()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        const QString target = dir.filePath(QStringLiteral("first.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        QStringList fileNames;
        QList<int> formats;
        // (what the printer holds when the window has set it up for the job, before the stand-in makes it write somewhere that is not paper)
        w.setPrinterPreparer([&](QPrinter &p) {
            fileNames << p.outputFileName();
            formats << int(p.outputFormat());
            if (p.outputFileName().isEmpty()) {
                p.setOutputFormat(QPrinter::PdfFormat);
                p.setOutputFileName(dir.filePath(QStringLiteral("paper.pdf")));
            }
        });
        WriteOutputOptions toFile;
        toFile.toFile = true;
        QVERIFY(w.printOutput(toFile));
        WriteOutputOptions paper;
        QVERIFY(w.printOutput(paper));
        QCOMPARE(fileNames.size(), 2);
        QVERIFY(!fileNames[0].isEmpty());
        QVERIFY2(fileNames[1].isEmpty(), qPrintable(fileNames[1]));
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("paper.pdf"))));
        QCOMPARE(PdfDocument(fileBytes(target)).pageCount(), PdfDocument(fileBytes(dir.filePath(QStringLiteral("paper.pdf")))).pageCount());
    }

    // Margins on the Print page are the document's own, the same as Page Setup's: a choice is applied to the document through Page Setup's command (one step that undoes), the text
    // reflows, and the page count and the preview follow. The "scale the page inside blank space" boxes are gone.
    void thePrintPageMarginsAreThePageSetupMargins()
    {
        QTemporaryDir dir;
        // a document with a few lines on its second page: narrow margins take them back to one
        int count = 0;
        const QByteArray docx = twoPageDocx(&count);
        QVERIFY(count > 0);
        const QString path = dir.filePath(QStringLiteral("margins.docx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(docx);
        f.close();
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(path));
        QCOMPARE(pagesOf(w), 2);
        w.document()->setModified(false);
        w.resize(1300, 900);
        w.show();
        w.showFileScreen(QStringLiteral("print"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen);
        for (const char *gone : {"printMarginTop", "printMarginBottom", "printMarginLeft", "printMarginRight"}) QVERIFY2(!screen->findChild<QWidget *>(QString::fromLatin1(gone)), gone);
        auto *margins = screen->findChild<QComboBox *>(QStringLiteral("printMargins"));
        auto *label = screen->findChild<QLabel *>(QStringLiteral("printPage"));
        QVERIFY(margins && label);
        QCOMPARE(margins->currentData().toString(), QStringLiteral("normal"));
        QTRY_VERIFY2(label->text().endsWith(QStringLiteral(" of 2")), qPrintable(label->text()));
        // the presets are Page Setup's own: the same names, in the same order
        QStringList names;
        for (int i = 0; i + 2 < margins->count(); ++i) names << margins->itemText(i);   // (then Custom, and the state that the sections differ)
        QCOMPARE(names, (QStringList{w.act(QStringLiteral("layout.margins.normal"))->text(), w.act(QStringLiteral("layout.margins.narrow"))->text(), w.act(QStringLiteral("layout.margins.moderate"))->text(),
                                     w.act(QStringLiteral("layout.margins.wide"))->text(), w.act(QStringLiteral("layout.margins.mirrored"))->text()}));
        QCOMPARE(margins->itemData(margins->count() - 2).toString(), QStringLiteral("custom"));
        QCOMPARE(margins->itemData(margins->count() - 1).toString(), QStringLiteral("different"));
        const int narrow = int(margins->findData(QStringLiteral("narrow")));
        QVERIFY(narrow >= 0);
        Q_EMIT margins->activated(narrow);
        QCOMPARE(w.pageMargins(), QMarginsF(36, 36, 36, 36));
        QCOMPARE(w.marginsPreset(), QStringLiteral("narrow"));
        QVERIFY(w.document()->isModified());
        QCOMPARE(margins->currentData().toString(), QStringLiteral("narrow"));
        QTRY_VERIFY2(label->text().endsWith(QStringLiteral(" of 1")), qPrintable(label->text()));
        QCOMPARE(pagesOf(w), 1);
        // the printed pages are the document's: the PDF has one page
        const QString pdfPath = dir.filePath(QStringLiteral("narrow.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return pdfPath; });
        WriteOutputOptions toFile;
        toFile.toFile = true;
        QVERIFY(w.printOutput(toFile));
        QCOMPARE(PdfDocument(fileBytes(pdfPath)).pageCount(), 1);
        // one step of the history takes it back, and the page shows two again
        w.view()->undo();
        QCOMPARE(w.pageMargins(), QMarginsF(72, 72, 72, 72));
        QCOMPARE(pagesOf(w), 2);
        QVERIFY(!w.document()->isModified());
        w.view()->redo();
        QCOMPARE(w.marginsPreset(), QStringLiteral("narrow"));
        // margins that are none of the presets are said in numbers, in the program's units
        const auto units = Settings::get().unit();
        const auto restoreUnits = qScopeGuard([units] { Settings::get().setUnit(units); });
        for (const Unit unit : {Unit::Inch, Unit::Centimeter}) {
            Settings::get().setUnit(unit);
            const SectionSetup was = pageSetupOf(w.document()->sectionProps(0), false);
            SectionSetup now = was;
            now.top = 50;
            now.left = 40;
            applyPageSetup(*w.document(), 0, PageSetupScope::WholeDocument, was, now);
            w.showFileScreen(QStringLiteral("print"));
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            auto *again = w.findChild<FileScreen *>()->findChild<QComboBox *>(QStringLiteral("printMargins"));
            QVERIFY(again);
            QVERIFY(w.marginsPreset().isEmpty());
            const QString shown = again->currentText();
            QVERIFY2(shown.contains(Settings::get().format(50, 2)) && shown.contains(Settings::get().format(40, 2)), qPrintable(shown));
            QCOMPARE(again->currentData().toString(), QStringLiteral("custom"));
            applyPageSetup(*w.document(), 0, PageSetupScope::WholeDocument, now, was);
        }
        // Custom opens Page Setup itself
        bool opened = false;
        std::function<void(int)> look = [&](int tries) {
            QTimer::singleShot(20, [&, tries] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!d || d->objectName() != QLatin1String("pageSetupDialog")) {
                    if (tries > 0) look(tries - 1);
                    return;
                }
                opened = true;
                d->reject();
            });
        };
        look(250);
        QVERIFY(w.setMargins(QStringLiteral("custom")));
        QVERIFY(opened);
    }

    // The Print page's Margins are the whole document's: a preset gives all four margins of every section its numbers (not only the fields that changed for the section the cursor is in),
    // each section keeps its gutter, its paper, and the way it is turned, and it is one step of the history. The box shows the preset only when every section has it, and says when the
    // sections differ (a state that is shown and not chosen).
    void thePrintMarginsAreTheWholeDocumentsAndSaySoWhenTheSectionsDiffer()
    {
        QTemporaryDir dir;
        auto window = windowOn(dir, twoSectionDocx(sectionKids(1440, 1440, 1440, 1440, 0, false), sectionKids(720, 1080, 720, 1440, 360, true)));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        WriteDoc &doc = *w.document();
        QCOMPARE(doc.sectionCount(), 2);
        QCOMPARE(w.marginsPreset(), QStringLiteral("different"));
        w.resize(1300, 900);
        w.show();
        w.showFileScreen(QStringLiteral("print"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        auto *margins = w.findChild<FileScreen *>()->findChild<QComboBox *>(QStringLiteral("printMargins"));
        QVERIFY(margins);
        QCOMPARE(margins->currentData().toString(), QStringLiteral("different"));
        QVERIFY(margins->currentText().contains(QStringLiteral("Different")));
        doc.setModified(false);
        // choosing what is shown does nothing
        Q_EMIT margins->activated(margins->currentIndex());
        QVERIFY(!doc.isModified() && !doc.canUndo());
        QCOMPARE(margins->currentData().toString(), QStringLiteral("different"));
        const int narrow = int(margins->findData(QStringLiteral("narrow")));
        QVERIFY(narrow >= 0);
        Q_EMIT margins->activated(narrow);
        for (int i = 0; i < 2; ++i) {
            const office::wml::SectionProps p = doc.sectionProps(i);
            QVERIFY2(p.top == 720 && p.right == 720 && p.bottom == 720 && p.left == 720, qPrintable(QStringLiteral("section %1: %2 %3 %4 %5").arg(i).arg(p.top).arg(p.right).arg(p.bottom).arg(p.left)));
        }
        QCOMPARE(doc.sectionProps(0).gutter, 0);
        QCOMPARE(doc.sectionProps(1).gutter, 360);   // (the binding margin of the section is its own)
        QCOMPARE(doc.sectionProps(1).pageW, 15840);   // (and so is the way it is turned)
        QCOMPARE(doc.sectionProps(1).pageH, 12240);
        QCOMPARE(doc.sectionProps(0).pageW, 12240);
        QCOMPARE(w.marginsPreset(), QStringLiteral("narrow"));
        QCOMPARE(margins->currentData().toString(), QStringLiteral("narrow"));
        auto *model = qobject_cast<QStandardItemModel *>(margins->model());
        QVERIFY(model);
        QVERIFY(!model->item(margins->count() - 1)->isEnabled());   // ("Different in parts of the document" is shown, not chosen, so it is off while it is not what the document is)
        QVERIFY(doc.isModified());
        // one Undo gives each section its own margins back
        w.view()->undo();
        QCOMPARE(doc.sectionProps(0).top, 1440);
        QCOMPARE(doc.sectionProps(1).top, 720);
        QCOMPARE(doc.sectionProps(1).right, 1080);
        QCOMPARE(w.marginsPreset(), QStringLiteral("different"));
        QVERIFY(!doc.canUndo());
        // the same margins that are none of the presets, in every section: said in numbers
        for (int i = 0; i < 2; ++i) {
            office::wml::SectionProps p = doc.sectionProps(i);
            p.top = 1000;
            p.bottom = 1000;
            p.left = 900;
            p.right = 900;
            doc.setSectionProps(i, p);
        }
        QVERIFY(w.marginsPreset().isEmpty());
    }

    // Mirrored turns the document's mirror margins on and every other preset turns them off; the Print page's box is the whole document's, and the ribbon's Mirrored is as it was (the margins
    // of the section the cursor is in, and the mirror setting).
    void theMirroredPresetTurnsMirrorMarginsOnAndTheOthersOff()
    {
        QTemporaryDir dir;
        auto window = windowOn(dir, twoSectionDocx(sectionKids(1440, 1440, 1440, 1440, 0, false), sectionKids(720, 1080, 720, 1440, 360, false)));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        WriteDoc &doc = *w.document();
        QVERIFY(!doc.mirrorMargins());
        QVERIFY(w.setMargins(QStringLiteral("mirrored")));
        QVERIFY(doc.mirrorMargins());
        QCOMPARE(w.marginsPreset(), QStringLiteral("mirrored"));
        for (int i = 0; i < 2; ++i) {
            const office::wml::SectionProps p = doc.sectionProps(i);
            QVERIFY2(p.top == 1440 && p.right == 1440 && p.bottom == 1440 && p.left == 1800, qPrintable(QStringLiteral("section %1: %2 %3 %4 %5").arg(i).arg(p.top).arg(p.right).arg(p.bottom).arg(p.left)));
        }
        QCOMPARE(doc.sectionProps(1).gutter, 360);
        QVERIFY(w.setMargins(QStringLiteral("normal")));
        QVERIFY(!doc.mirrorMargins());
        QCOMPARE(w.marginsPreset(), QStringLiteral("normal"));
        QCOMPARE(doc.sectionProps(1).gutter, 360);
        // each one is a step that undoes, the mirror setting with it
        w.view()->undo();
        QVERIFY(doc.mirrorMargins());
        QCOMPARE(w.marginsPreset(), QStringLiteral("mirrored"));
        // (the margins of Mirrored without the mirror setting are not the preset Mirrored)
        doc.setMirrorMargins(false);
        QVERIFY(w.marginsPreset().isEmpty());
        // the ribbon's Mirrored: the margins of the cursor's section (the first) and the mirror setting; the other section is left as it was
        auto other = windowOn(dir, twoSectionDocx(sectionKids(1440, 1440, 1440, 1440, 0, false), sectionKids(720, 1080, 720, 1440, 360, false)));
        QVERIFY(other);
        WriteDoc &odoc = *other->document();
        Quiet oquiet(*other);
        QVERIFY(other->act(QStringLiteral("layout.margins.mirrored")));
        other->act(QStringLiteral("layout.margins.mirrored"))->trigger();
        QVERIFY(odoc.mirrorMargins());
        QCOMPARE(odoc.sectionProps(0).left, 1800);
        QCOMPARE(odoc.sectionProps(1).left, 1440);
        QCOMPARE(odoc.sectionProps(1).top, 720);
    }

    // Cancel in Page Setup, reached from Custom, changes nothing: not the document, not its history, and not the box.
    void cancelInPageSetupFromCustomLeavesTheDocumentAndTheBoxAlone()
    {
        QTemporaryDir dir;
        auto window = windowOn(dir, twoSectionDocx(sectionKids(1440, 1440, 1440, 1440, 0, false), sectionKids(720, 1080, 720, 1440, 360, true)));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        WriteDoc &doc = *w.document();
        w.resize(1300, 900);
        w.show();
        w.showFileScreen(QStringLiteral("print"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        auto *margins = w.findChild<FileScreen *>()->findChild<QComboBox *>(QStringLiteral("printMargins"));
        QVERIFY(margins);
        doc.setModified(false);
        const office::wml::SectionProps first = doc.sectionProps(0), last = doc.sectionProps(1);
        const int chosen = margins->currentIndex();
        bool opened = false;
        std::function<void(int)> look = [&](int tries) {
            QTimer::singleShot(20, [&, tries] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!d || d->objectName() != QLatin1String("pageSetupDialog")) {
                    if (tries > 0) look(tries - 1);
                    return;
                }
                opened = true;
                d->reject();
            });
        };
        look(250);
        Q_EMIT margins->activated(int(margins->findData(QStringLiteral("custom"))));
        QVERIFY(opened);
        auto same = [](const office::wml::SectionProps &a, const office::wml::SectionProps &b) {
            return a.top == b.top && a.right == b.right && a.bottom == b.bottom && a.left == b.left && a.gutter == b.gutter && a.pageW == b.pageW && a.pageH == b.pageH && a.orient == b.orient;
        };
        QVERIFY(same(doc.sectionProps(0), first));
        QVERIFY(same(doc.sectionProps(1), last));
        QVERIFY(!doc.canUndo());
        QVERIFY(!doc.isModified());
        QCOMPARE(margins->currentIndex(), chosen);
        QCOMPARE(margins->currentData().toString(), QStringLiteral("different"));
    }

    // The Print page has Printer Properties, which is hidden (not disabled) where the platform has no window of its own and for a PDF file; it is there for a printer when the platform has
    // one, and what that window says (copies, the paper, and the way it turns) comes back to the page, and is what the job is printed with.
    void thePrintPageHasPropertiesAndTheOpenBox()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        w.resize(1300, 900);
        w.show();
        w.showFileScreen(QStringLiteral("print"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen);
        for (const char *name : {"printMargins", "printShrink", "printPrinterSettings", "printOpenPdf"}) QVERIFY2(screen->findChild<QWidget *>(QString::fromLatin1(name)), name);
        auto *printer = screen->findChild<QComboBox *>(QStringLiteral("printPrinter"));
        auto *button = screen->findChild<QPushButton *>(QStringLiteral("printPrinterSettings"));
        auto *openPdf = screen->findChild<QCheckBox *>(QStringLiteral("printOpenPdf"));
        auto *copies = screen->findChild<QSpinBox *>(QStringLiteral("printCopies"));
        auto *scaleTo = screen->findChild<QComboBox *>(QStringLiteral("printScaleTo"));
        auto *turned = screen->findChild<QComboBox *>(QStringLiteral("printOrientation"));
        QVERIFY(button && printer && openPdf && copies && scaleTo && turned);
        const auto restore = qScopeGuard([] {
            setPrinterPropertiesStandIn({});
            setPrintDialogStandIn({});
            setPrinterNamesStandIn({});
        });
        // (the test display has no window of the system's)
        printer->insertItem(0, QStringLiteral("a printer"), QStringLiteral("a printer"));
        printer->setCurrentIndex(0);
        QTRY_VERIFY(!button->isVisibleTo(screen));
        QVERIFY(!openPdf->isVisibleTo(screen));   // (a printer is not a file)
        setPrinterPropertiesStandIn([] { return true; });
        printer->setCurrentIndex(printer->count() - 1);   // Save as PDF
        QTRY_VERIFY(openPdf->isVisibleTo(screen));
        QVERIFY(!button->isVisibleTo(screen));   // (a PDF file has no printer settings)
        printer->setCurrentIndex(0);
        QTRY_VERIFY(button->isVisibleTo(screen));
        QVERIFY(button->isEnabled());
        QVERIFY(!openPdf->isVisibleTo(screen));
        const QPrinter *configured = nullptr;
        // (a paper that is not the one the printer has already: a window that is closed with the printer's own is no change)
        QPageSize::PageSizeId chosenSize = w.printer().pageLayout().pageSize().id() == QPageSize::A5 ? QPageSize::A3 : QPageSize::A5;
        QPageLayout::Orientation chosenTurn = QPageLayout::Landscape;
        bool change = true;
        setPrintDialogStandIn([&](QPrintDialog &dialog) {
            configured = dialog.printer();
            dialog.printer()->setCopyCount(4);
            if (change) {
                dialog.printer()->setPageSize(QPageSize(chosenSize));
                dialog.printer()->setPageOrientation(chosenTurn);
            }
            return int(QDialog::Accepted);
        });
        QCOMPARE(scaleTo->currentIndex(), 0);
        QCOMPARE(turned->currentData().toInt(), int(PrintTurn::AsSet));
        button->click();
        QVERIFY(configured);
        QCOMPARE(copies->value(), 4);
        QCOMPARE(configured, &w.printer());   // (the very printer the jobs are printed on)
        // the paper and the way it turns, as the window left them, are on the page
        QVERIFY2(std::abs(scaleTo->currentData().toSizeF().width() - 419.528) < 1 && std::abs(scaleTo->currentData().toSizeF().height() - 595.276) < 1, qPrintable(QString::number(scaleTo->currentIndex())));
        QCOMPARE(turned->currentData().toInt(), int(PrintTurn::Wide));
        // a window that changes nothing leaves the page's choices as they are (here put back to the page's own)
        scaleTo->setCurrentIndex(0);
        turned->setCurrentIndex(0);
        change = false;
        button->click();
        QCOMPARE(scaleTo->currentIndex(), 0);
        QCOMPARE(turned->currentData().toInt(), int(PrintTurn::AsSet));
        // and a paper and a turn chosen again are what it takes
        change = true;
        chosenSize = QPageSize::Legal;
        chosenTurn = QPageLayout::Portrait;
        button->click();
        QCOMPARE(scaleTo->currentData().toSizeF(), QSizeF(612, 1008));
        QCOMPARE(turned->currentData().toInt(), int(PrintTurn::Tall));
    }

    // Shrink by One Page makes the text a little smaller until the document is a page shorter, whether the sizes are the runs' own or the style's. It is one step of the history, marks the
    // document changed, and shows the wait cursor while it works.
    void shrinkByOnePageTakesOffAPageInOneStep()
    {
        QTemporaryDir dir;
        for (const int runHalfPoints : {0, 24}) {
            ShrinkDocx docx;
            docx.runHalfPoints = runHalfPoints;
            auto window = windowOn(dir, twoPageDocx(nullptr, docx));
            QVERIFY(window);
            WriteWindow &w = *window;
            Quiet quiet(w);
            QCOMPARE(pagesOf(w), 2);
            QTextDocument *story = w.document()->story();
            const QVector<double> before = effectiveSizes(*w.document(), story);
            QCOMPARE(smallest(before), 12.0);
            QCOMPARE(largest(before), 12.0);
            QVector<std::pair<int, double>> own;   // (the runs' own sizes, to see them come back)
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
                for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) own << std::make_pair(it.fragment().position(), it.fragment().charFormat().fontPointSize());
            w.document()->setModified(false);
            QVERIFY(!w.document()->canUndo());
            int waitSeen = 0;
            connect(w.document(), &WriteDoc::historyChanged, &w, [&] { waitSeen += QApplication::overrideCursor() && QApplication::overrideCursor()->shape() == Qt::WaitCursor; });
            QVERIFY(w.act(QStringLiteral("file.shrinkOnePage")));
            w.act(QStringLiteral("file.shrinkOnePage"))->trigger();
            QVERIFY2(waitSeen > 0, "no wait cursor while the text was made smaller");
            QVERIFY(!QApplication::overrideCursor());
            QCOMPARE(pagesOf(w), 1);
            QVERIFY2(quiet.said.isEmpty(), qPrintable(quiet.said.join(QLatin1Char('|'))));
            const QVector<double> after = effectiveSizes(*w.document(), story);
            QCOMPARE(after.size(), before.size());
            QVERIFY2(largest(after) < 12.0 && smallest(after) >= 6.0, qPrintable(QString::number(smallest(after)) + " " + QString::number(largest(after))));
            QVERIFY(w.document()->isModified());
            QVERIFY(w.document()->canUndo());
            QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("smaller")));
            // one Undo takes all of it back, and one Redo does it again
            w.view()->undo();
            QCOMPARE(effectiveSizes(*w.document(), story), before);
            QVector<std::pair<int, double>> ownAgain;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
                for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) ownAgain << std::make_pair(it.fragment().position(), it.fragment().charFormat().fontPointSize());
            QCOMPARE(ownAgain, own);
            QCOMPARE(pagesOf(w), 2);
            QVERIFY(!w.document()->canUndo());
            QVERIFY(!w.document()->isModified());
            w.view()->redo();
            QCOMPARE(pagesOf(w), 1);
            QCOMPARE(effectiveSizes(*w.document(), story), after);
        }
    }

    // It scales the whole document's text: the body's tables, the footnotes, and the text boxes in the body, and the exact line spacing of the paragraphs by the same share; the header
    // and footer are left as they are; and all of it is one step of the history.
    void shrinkByOnePageScalesTablesNotesBoxesAndExactSpacing()
    {
        QTemporaryDir dir;
        ShrinkDocx docx;
        docx.runHalfPoints = 24;
        docx.exact = true;
        docx.table = true;
        docx.footnote = true;
        docx.header = true;
        auto window = windowOn(dir, twoPageDocx(nullptr, docx));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        WriteDoc &doc = *w.document();
        // a text box with words of 12 points in the body
        {
            QTextCursor c(doc.story());
            c.setPosition(0);
            QCOMPARE(insertTextBox(doc, c, textBoxDesigns().first()), PictureResult::Ok);
        }
        const QVector<FloatingObject> objects = floatingObjects(doc);
        QVERIFY(!objects.isEmpty());
        const int boxAt = objects.first().position;
        {
            std::unique_ptr<QTextDocument> story = textBoxStory(doc, boxAt);
            QVERIFY(story);
            QTextCursor t(story.get());
            t.insertText(QStringLiteral("Box words"));
            t.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(12);
            t.mergeCharFormat(f);
            QVERIFY(setTextBoxStory(doc, boxAt, story.get()));
        }
        const int pages = pagesOf(w);
        QVERIFY2(pages >= 2, qPrintable(QString::number(pages)));
        auto boxSizes = [&] {
            std::unique_ptr<QTextDocument> story = textBoxStory(doc, boxAt);
            QVector<double> sizes;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
                for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) sizes << it.fragment().charFormat().fontPointSize();
            return sizes;
        };
        const QVector<double> bodyBefore = effectiveSizes(doc, doc.story()), boxBefore = boxSizes(), headerBefore = effectiveSizes(doc, headerStories(doc).value(0)),
                              noteBefore = effectiveSizes(doc, noteStories(doc).value(0)), exactBefore = exactHeights(doc.story());
        QCOMPARE(largest(boxBefore), 12.0);
        QVERIFY(!headerStories(doc).isEmpty() && !noteStories(doc).isEmpty());
        QCOMPARE(largest(headerBefore), 12.0);
        QCOMPARE(smallest(noteBefore), 10.0);
        QCOMPARE(largest(exactBefore), 20.0);
        // (the table's paragraph is one of the body's: its size is in the list, and is 12)
        QCOMPARE(smallest(bodyBefore), 12.0);
        // (the size of the words in the table's cell)
        auto cellSize = [&] {
            for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
                if (b.text().contains(QStringLiteral("Cell words"))) {
                    QTextCharFormat f;
                    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) f = it.fragment().charFormat();   // (the last run: the words, after the text box's character)
                    return f.fontPointSize() > 0 ? f.fontPointSize() : doc.styleCharFormat(b.blockFormat().stringProperty(tp::StyleName), f.stringProperty(tp::CharStyle)).fontPointSize();
                }
            return 0.0;
        };
        const double cellBefore = cellSize();
        doc.setModified(false);
        QVERIFY(w.shrinkByOnePage());
        QVERIFY(pagesOf(w) < pages);
        // (each part is looked at on its own: what is wrong is all said at the end)
        QStringList problems;
        auto expect = [&](bool ok, const char *what) {
            if (!ok) problems << QLatin1String(what);
        };
        const QVector<double> bodyAfter = effectiveSizes(doc, doc.story());
        QCOMPARE(bodyAfter.size(), bodyBefore.size());
        bool everyRun = true;
        for (int i = 0; i < bodyAfter.size(); ++i) everyRun = everyRun && bodyAfter[i] < bodyBefore[i];
        expect(everyRun, "a run of the body was not made smaller");
        // the table's cell: the first paragraph, which is in the table
        expect(cellBefore > 0 && cellSize() < cellBefore, "the table's cell was not made smaller");
        expect(largest(boxSizes()) < 12.0, "the text box was not made smaller");
        expect(smallest(effectiveSizes(doc, noteStories(doc).value(0))) < smallest(noteBefore), "the footnote was not made smaller");   // (the one character that is the note's number is the document's own to keep)
        expect(effectiveSizes(doc, headerStories(doc).value(0)) == headerBefore, "the header was changed");
        // the exact spacing is the text's share: 20 points scaled as the sizes were
        const QVector<double> exactAfter = exactHeights(doc.story());
        const double share = largest(bodyAfter) / 12.0;
        bool exactRight = exactAfter.size() == exactBefore.size();
        for (int i = 0; exactRight && i < exactAfter.size(); ++i)
            if (exactBefore[i] > 0) exactRight = std::abs(exactAfter[i] / exactBefore[i] - share) < 0.02;
        expect(exactRight, "the exact line spacing was not scaled by the same share");
        // one Undo gives every part back
        w.view()->undo();
        expect(effectiveSizes(doc, doc.story()) == bodyBefore, "undo: the body");
        expect(boxSizes() == boxBefore, "undo: the text box");
        expect(effectiveSizes(doc, noteStories(doc).value(0)) == noteBefore, "undo: the footnote");
        expect(effectiveSizes(doc, headerStories(doc).value(0)) == headerBefore, "undo: the header");
        expect(exactHeights(doc.story()) == exactBefore, "undo: the exact spacing");
        expect(!doc.isModified(), "undo: still marked changed");
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
    }

    // It is a layout tweak, not an edit: it is not recorded as tracked changes, even when the document tracks them.
    void shrinkByOnePageIsNotATrackedChange()
    {
        QTemporaryDir dir;
        ShrinkDocx docx;
        docx.footnote = true;
        auto window = windowOn(dir, twoPageDocx(nullptr, docx));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        w.document()->setTrackChanges(true);
        QVERIFY(w.document()->trackChanges());
        QVERIFY(w.shrinkByOnePage());
        QCOMPARE(pagesOf(w), 1);
        QCOMPARE(revisionCount(w.document()->story()), 0);
        for (QTextDocument *note : noteStories(*w.document())) QCOMPARE(revisionCount(note), 0);
        QVERIFY(w.document()->trackChanges());
        QVERIFY(largest(effectiveSizes(*w.document(), w.document()->story())) < 12.0);
    }

    // Each refusal says why, in words of its own, and leaves nothing behind: the text as it was, nothing to undo or redo, and the document no more changed than before.
    void shrinkByOnePageRefusesForTheTrueReasonAndChangesNothing()
    {
        QTemporaryDir dir;
        struct Case {
            const char *name;
            ShrinkDocx docx;
            const char *words;   // what its message says
            int pages;           // the pages it has (0: the paragraphs make two)
            bool limited = false;
        };
        ShrinkDocx one;
        one.paragraphs = 3;
        ShrinkDocx apart;
        apart.paragraphs = 3;
        apart.pageBreak = true;
        ShrinkDocx sections;
        sections.paragraphs = 3;
        sections.sectionBreak = true;
        ShrinkDocx tiny;
        tiny.styleHalfPoints = 14;   // 7 points: 4 steps from 6
        ShrinkDocx locked;
        locked.lockedControl = true;
        ShrinkDocx limitedDocx;
        ShrinkDocx tooLong;
        tooLong.styleHalfPoints = 144;
        const QList<Case> cases = {{"one page", one, "one page long already", 1}, {"page break", apart, "page break", 2}, {"section break", sections, "section break", 2},
                                   {"too small", tiny, "smaller than", -2}, {"locked", locked, "locked", 0}, {"formatting limited", limitedDocx, "locked", 0, true},
                                   {"too long", tooLong, "longer than", -300}};
        QStringList problems;
        for (const Case &c : cases) {
            ShrinkDocx docx = c.docx;
            if (c.pages == -2) {
                // two pages that are full at 7 points: the text can't be made small enough, whatever is tried
                docx.paragraphs = paragraphsForPages(docx, 3) - 1;
            } else if (c.pages == -300) {
                docx.paragraphs = paragraphsForPages(docx, kMostShrinkPages + 1);
            } else if (c.pages == 0) {
                docx.paragraphs = paragraphsForPages(docx, 2);
            }
            QVERIFY2(docx.paragraphs > 0, c.name);
            auto window = windowOn(dir, docx.build());
            QVERIFY2(window, c.name);
            WriteWindow &w = *window;
            QStringList said;
            int cursorWhileTold = -1;
            w.setNotifier([&](const QString &, const QString &text) {
                said << text;
                cursorWhileTold = QApplication::overrideCursor() ? 1 : 0;
            });
            if (c.limited) {
                office::wml::DocumentProtection protection;
                protection.enforced = true;
                protection.formatting = true;
                w.document()->setProtection(protection);
            }
            QTextDocument *story = w.document()->story();
            const int pages = pagesOf(w);
            if (c.pages > 0) QCOMPARE(pages, c.pages);
            const QVector<double> sizes = effectiveSizes(*w.document(), story);
            QVector<double> own;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
                for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) own << it.fragment().charFormat().fontPointSize();
            w.document()->setModified(false);
            const bool did = w.shrinkByOnePage();
            auto expect = [&](bool ok, const char *what) {
                if (!ok) problems << QStringLiteral("%1: %2").arg(QLatin1String(c.name), QLatin1String(what));
            };
            expect(!did, "it went ahead");
            expect(said.size() == 1 && said[0].contains(QLatin1String(c.words)), "it did not say why in its own words");
            expect(cursorWhileTold == 0, "the wait cursor was still on when the person was told");
            expect(pagesOf(w) == pages, "the pages changed");
            expect(effectiveSizes(*w.document(), story) == sizes, "the sizes changed");
            QVector<double> ownAfter;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
                for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) ownAfter << it.fragment().charFormat().fontPointSize();
            expect(ownAfter == own, "the runs' own sizes changed");
            expect(!w.document()->canUndo(), "there is a step to undo");
            expect(!w.document()->canRedo(), "there is a step to redo");
            expect(!w.document()->isModified(), "the document is marked changed");
            expect(!story->isUndoAvailable() && !story->isRedoAvailable(), "the story has steps");
            expect(w.act(QStringLiteral("edit.redo")) && !w.act(QStringLiteral("edit.redo"))->isEnabled(), "Redo is on");
            expect(!QApplication::overrideCursor(), "the wait cursor is left on");
        }
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
    }

    // Sizes are kept in half-points, as a file keeps them: the sizes the trial measured are the ones that come back from a saved file, and the page count with them.
    void shrinkByOnePageSizesAreHalfPointsThatSurviveSaveAndReload()
    {
        // every size above the least goes down by a step that changes it, to a half-point, never below the least
        for (double size = 6.5; size <= 60; size += 0.5) {
            const double next = shrunkSize(size);
            QVERIFY2(next < size && next >= kShrinkLeastSize && std::abs(next * 2 - std::round(next * 2)) < 1e-9, qPrintable(QString::number(size) + " " + QString::number(next)));
        }
        QCOMPARE(shrunkSize(6.0), 6.0);
        QCOMPARE(shrunkSize(5.0), 5.0);
        QTemporaryDir dir;
        for (const int runHalfPoints : {0, 21}) {
            ShrinkDocx docx;
            docx.styleHalfPoints = 21;   // (10.5 points: a tenth-point step would give 10.1)
            docx.runHalfPoints = runHalfPoints;
            auto window = windowOn(dir, twoPageDocx(nullptr, docx));
            QVERIFY(window);
            WriteWindow &w = *window;
            Quiet quiet(w);
            QCOMPARE(pagesOf(w), 2);
            QVERIFY(w.shrinkByOnePage());
            QCOMPARE(pagesOf(w), 1);
            const QVector<double> sizes = effectiveSizes(*w.document(), w.document()->story());
            for (const double size : sizes) QVERIFY2(std::abs(size * 2 - std::round(size * 2)) < 1e-9, qPrintable(QString::number(size)));
            QCOMPARE(smallest(sizes), largest(sizes));   // (text that was of one size is still of one size)
            QString error;
            auto again = WriteDoc::fromDocx(w.document()->toDocx(&error), &error, nullptr);
            QVERIFY2(again, qPrintable(error));
            QCOMPARE(effectiveSizes(*again, again->story()), sizes);
            QCOMPARE(again->pager().pageCount(), 1);
        }
    }

    // The trial costs the history nothing: a document that can't be shrunk keeps what Redo had, and the steps that were done and the document's changed mark; a shrink that works is one
    // step of the history for the text, its boxes, and its notes, which one Undo takes back and one Redo does again.
    void shrinkByOnePageLeavesTheHistoryAloneWhenItRefusesAndIsOneStepWhenItWorks()
    {
        QTemporaryDir dir;
        {
            ShrinkDocx tiny;
            tiny.styleHalfPoints = 14;
            tiny.paragraphs = paragraphsForPages(tiny, 3) - 1;   // (two pages, full at 7 points: the trial changes the text and finds no page to take off)
            auto window = windowOn(dir, tiny.build());
            QVERIFY(window);
            WriteWindow &w = *window;
            Quiet quiet(w);
            QCOMPARE(pagesOf(w), 2);
            WriteDoc &doc = *w.document();
            QTextCursor c(doc.story());
            c.setPosition(0);
            insertText(doc, c, QStringLiteral("Typed "));
            w.view()->undo();
            QVERIFY(doc.canRedo());
            const QString text = doc.story()->toPlainText();
            w.view()->redo();
            w.view()->undo();
            doc.setModified(false);
            const int steps = doc.story()->availableUndoSteps();
            QVERIFY(doc.canRedo());
            QVERIFY(!w.shrinkByOnePage());
            QVERIFY2(quiet.said.size() == 1 && quiet.said[0].contains(QStringLiteral("smaller than")), qPrintable(quiet.said.join(QLatin1Char('|'))));
            QVERIFY(doc.canRedo());
            QCOMPARE(doc.story()->availableUndoSteps(), steps);
            QVERIFY(!doc.isModified());
            QCOMPARE(doc.story()->toPlainText(), text);
            w.view()->redo();
            QVERIFY(doc.story()->toPlainText().startsWith(QStringLiteral("Typed ")));
        }
        ShrinkDocx docx;
        docx.runHalfPoints = 24;
        docx.footnote = true;
        auto window = windowOn(dir, twoPageDocx(nullptr, docx));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        WriteDoc &doc = *w.document();
        {
            QTextCursor c(doc.story());
            c.setPosition(0);
            QCOMPARE(insertTextBox(doc, c, textBoxDesigns().first()), PictureResult::Ok);
        }
        const int boxAt = floatingObjects(doc).first().position;
        {
            std::unique_ptr<QTextDocument> story = textBoxStory(doc, boxAt);
            QVERIFY(story);
            QTextCursor t(story.get());
            t.insertText(QStringLiteral("Box words"));
            t.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(12);
            t.mergeCharFormat(f);
            QVERIFY(setTextBoxStory(doc, boxAt, story.get()));
        }
        auto boxSizes = [&] {
            std::unique_ptr<QTextDocument> story = textBoxStory(doc, boxAt);
            QVector<double> sizes;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
                for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) sizes << it.fragment().charFormat().fontPointSize();
            return sizes;
        };
        QTextDocument *note = noteStories(doc).value(0);
        QVERIFY(note);
        QVERIFY(pagesOf(w) >= 2);
        const QVector<double> bodyBefore = effectiveSizes(doc, doc.story()), boxBefore = boxSizes(), noteBefore = effectiveSizes(doc, note);
        // (nothing to undo but the box: take it away from the count by starting from a clean history of the body and the note)
        doc.setModified(false);
        int entries = 0;
        connect(&doc, &WriteDoc::historyChanged, &w, [&] { ++entries; });
        QVERIFY(w.shrinkByOnePage());
        QCOMPARE(entries, 2);   // (one entry for the body with its box, one for the note, whatever the number of paragraphs: and they are done and undone together)
        const QVector<double> bodyAfter = effectiveSizes(doc, doc.story()), boxAfter = boxSizes(), noteAfter = effectiveSizes(doc, note);
        QVERIFY(bodyAfter != bodyBefore && boxAfter != boxBefore && noteAfter != noteBefore);
        w.view()->undo();
        QCOMPARE(effectiveSizes(doc, doc.story()), bodyBefore);
        QCOMPARE(boxSizes(), boxBefore);
        QCOMPARE(effectiveSizes(doc, note), noteBefore);
        QVERIFY(!doc.isModified());
        QVERIFY(doc.canRedo());
        w.view()->redo();
        QCOMPARE(effectiveSizes(doc, doc.story()), bodyAfter);
        QCOMPARE(boxSizes(), boxAfter);
        QCOMPARE(effectiveSizes(doc, note), noteAfter);
        QVERIFY(doc.isModified());
    }

    // A tracked change of a run's look keeps the old look, and Reject All brings it back: the old size is scaled as the text was, so that it does not bring back unshrunk text.
    void shrinkByOnePageScalesTheOldSizeOfTrackedFormatChanges()
    {
        QTemporaryDir dir;
        for (const int runHalfPoints : {0, 24}) {
            ShrinkDocx docx;
            docx.runHalfPoints = runHalfPoints;
            auto window = windowOn(dir, twoPageDocx(nullptr, docx));
            QVERIFY(window);
            WriteWindow &w = *window;
            Quiet quiet(w);
            WriteDoc &doc = *w.document();
            doc.setTrackChanges(true);
            QTextCursor c(doc.story());
            c.setPosition(0);
            c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            toggleBold(doc, c);
            QVERIFY2(revisionCount(doc.story()) > 0, "no tracked format change");
            QCOMPARE(pagesOf(w), 2);
            QVERIFY(w.shrinkByOnePage());
            QVERIFY(largest(effectiveSizes(doc, doc.story())) < 12.0);
            QVERIFY(rejectAll(doc) > 0);
            const QVector<double> sizes = effectiveSizes(doc, doc.story());
            QVERIFY2(largest(sizes) < 12.0 && smallest(sizes) == largest(sizes), qPrintable(QString::number(smallest(sizes)) + " " + QString::number(largest(sizes))));
        }
    }

    // A section break that starts a page and a paragraph that starts a page on its first paragraph are one break: a page that smaller text can take off is not refused for them.
    void shrinkByOnePageCountsASectionBreakAndABreakBeforeItOnce()
    {
        QTemporaryDir dir;
        ShrinkDocx docx;
        docx.sectionBreak = true;
        docx.breakBefore = true;
        docx.paragraphs = paragraphsForPages(docx, 3);
        QVERIFY(docx.paragraphs > 0);
        auto window = windowOn(dir, docx.build());
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        QCOMPARE(pagesOf(w), 3);
        QVERIFY2(w.shrinkByOnePage(), qPrintable(quiet.said.join(QLatin1Char('|'))));
        QCOMPARE(pagesOf(w), 2);
        const ShrinkLook look = shrinkLook(*w.document());
        QCOMPARE(look.forcedBreaks, 1);
    }

    // The size a table's style gives its text counts: text sized by its table's style is measured as it is, and no run is made larger by the explicit size the shrink writes.
    void shrinkByOnePageMeasuresTextThatItsTableStyleSizes()
    {
        QTemporaryDir dir;
        ShrinkDocx docx;
        docx.table = true;
        docx.tableStyleHalfPoints = 18;   // (the table's style says 9 points; the paragraph style says none, so the style of the table is what sizes its cell, and the runs of the text say 12)
        docx.styleHalfPoints = 0;
        docx.runHalfPoints = 24;
        auto window = windowOn(dir, twoPageDocx(nullptr, docx));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        WriteDoc &doc = *w.document();
        auto cellSize = [&] {
            for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
                if (b.text().contains(QStringLiteral("Cell words"))) {
                    const QTextCharFormat f = b.begin().fragment().charFormat();
                    if (f.fontPointSize() > 0) return f.fontPointSize();
                    return doc.styleCharFormat(b.blockFormat().stringProperty(tp::StyleName), f.stringProperty(tp::CharStyle), doc.tableLayerOf(b.blockFormat().intProperty(flow::fp::Cell))).fontPointSize();
                }
            return 0.0;
        };
        // (a file's reading gives such a run the table style's size as its own: here the run is one that says none, as the text of a cell does when its look was cleared)
        for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
            if (b.text().contains(QStringLiteral("Cell words"))) {
                QTextCursor c(b);
                c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                QTextCharFormat f = c.charFormat();
                f.clearProperty(QTextFormat::FontPointSize);
                c.setCharFormat(f);
            }
        QCOMPARE(cellSize(), 9.0);
        QVERIFY(w.shrinkByOnePage());
        QVERIFY2(cellSize() <= 9.0, qPrintable(QString::number(cellSize())));
    }

    // A shrink that takes off more than one page says how many; one that takes off one says so. The notes' numbers and the endnotes are made smaller with the text.
    void shrinkByOnePageSaysHowManyPagesCameOffAndScalesEndnotesAndNoteNumbers()
    {
        QTemporaryDir dir;
        {
            ShrinkDocx docx;
            docx.paragraphs = paragraphsForPages(docx, 30);
            QVERIFY(docx.paragraphs > 0);
            auto window = windowOn(dir, docx.build());
            QVERIFY(window);
            WriteWindow &w = *window;
            Quiet quiet(w);
            const int before = pagesOf(w);
            QVERIFY(w.shrinkByOnePage());
            const int dropped = before - pagesOf(w);
            QVERIFY2(dropped > 1, qPrintable(QString::number(dropped)));
            QVERIFY2(w.statusBar()->currentMessage().contains(QString::number(dropped)), qPrintable(w.statusBar()->currentMessage()));
        }
        ShrinkDocx docx;
        docx.runHalfPoints = 24;
        docx.footnote = true;
        docx.endnote = true;
        auto window = windowOn(dir, twoPageDocx(nullptr, docx));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        WriteDoc &doc = *w.document();
        QTextDocument *note = noteStories(doc).value(0);
        QVERIFY(note);
        auto endnoteSizes = [&] {
            QVector<double> out;
            for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
                if (b.blockFormat().hasProperty(prop::EndnoteBlock))
                    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                        const QTextCharFormat f = it.fragment().charFormat();
                        out << (f.fontPointSize() > 0 ? f.fontPointSize() : doc.styleCharFormat(b.blockFormat().stringProperty(tp::StyleName), f.stringProperty(tp::CharStyle)).fontPointSize());
                    }
            return out;
        };
        const QVector<double> endBefore = endnoteSizes(), noteBefore = effectiveSizes(doc, note);
        QVERIFY(!endBefore.isEmpty());
        QVERIFY(w.shrinkByOnePage());
        const QVector<double> endAfter = endnoteSizes(), noteAfter = effectiveSizes(doc, note);
        QCOMPARE(endAfter.size(), endBefore.size());
        QCOMPARE(noteAfter.size(), noteBefore.size());
        for (int i = 0; i < endAfter.size(); ++i) QVERIFY2(endAfter[i] < endBefore[i], qPrintable(QStringLiteral("endnote run %1: %2 -> %3").arg(i).arg(endBefore[i]).arg(endAfter[i])));
        for (int i = 0; i < noteAfter.size(); ++i) QVERIFY2(noteAfter[i] < noteBefore[i], qPrintable(QStringLiteral("note run %1: %2 -> %3").arg(i).arg(noteBefore[i]).arg(noteAfter[i])));
    }

    // The Print page's button does the same, and the page shows the new count.
    void theShrinkButtonOnThePrintPageTakesAPageOff()
    {
        QTemporaryDir dir;
        auto window = windowOn(dir, twoPageDocx(nullptr));
        QVERIFY(window);
        WriteWindow &w = *window;
        Quiet quiet(w);
        w.resize(1300, 900);
        w.show();
        w.showFileScreen(QStringLiteral("print"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        auto *screen = w.findChild<FileScreen *>();
        auto *label = screen ? screen->findChild<QLabel *>(QStringLiteral("printPage")) : nullptr;
        auto *shrink = screen ? screen->findChild<QPushButton *>(QStringLiteral("printShrink")) : nullptr;
        QVERIFY(label && shrink);
        QTRY_VERIFY2(label->text().endsWith(QStringLiteral(" of 2")), qPrintable(label->text()));
        shrink->click();
        QTRY_VERIFY2(label->text().endsWith(QStringLiteral(" of 1")), qPrintable(label->text()));
        QCOMPARE(pagesOf(w), 1);
    }

    // A tagged PDF: the headings, paragraphs, a list, a table with a header row, and a picture with its alt text are in a structure tree in reading order, the marks are all gone, and the
    // language is named.
    void aTaggedPdfHasTheStructureOfTheDocument()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir, QStringLiteral("A noisy logo"));
        QString target = dir.filePath(QStringLiteral("tagged.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        WriteOutputOptions o;
        o.pdfTags = true;
        QVERIFY(w.exportPdfPages(o));
        const QByteArray pdf = fileBytes(target);
        QVERIFY(pdf.startsWith("%PDF"));
        QVERIFY(pdf.contains("/StructTreeRoot"));
        QVERIFY(pdf.contains("/MarkInfo"));
        QVERIFY(pdf.contains("/Lang (en-US)"));
        jp::QtPdf q;
        QVERIFY(q.loadBytes(pdf));
        QStringList types;
        QByteArray all;
        for (const jp::QtPdf::Obj &obj : q.objects) {
            all += obj.body + "\n";
            if (!obj.body.contains("/Type /StructElem")) continue;
            const int at = int(obj.body.indexOf("/S /"));
            types << QString::fromLatin1(obj.body.mid(at + 4, obj.body.indexOf(' ', at + 4) - at - 4));
        }
        for (const char *t : {"H1", "P", "L", "LI", "LBody", "Table", "TR", "TH", "TD", "Figure"}) QVERIFY2(types.contains(QLatin1String(t)), t);
        QVERIFY(all.contains("/Alt <" + utf16Hex(QStringLiteral("A noisy logo")) + ">"));
        // reading order, as the tree walks it: the heading, the paragraph, the list, the table, then the picture
        QStringList walked;
        std::function<void(int)> walk = [&](int id) {
            const jp::QtPdf::Obj *obj = q.object(id);
            if (!obj) return;
            const int at = int(obj->body.indexOf("/S /"));
            walked << QString::fromLatin1(obj->body.mid(at + 4, obj->body.indexOf(' ', at + 4) - at - 4));
            const int k = int(obj->body.indexOf("/K ["));
            if (k < 0) return;
            static const QRegularExpression ref(QStringLiteral("(\\d+) 0 R"));
            const QString list = QString::fromLatin1(obj->body.mid(k, obj->body.indexOf(']', k) - k));
            for (auto it = ref.globalMatch(list); it.hasNext();) walk(it.next().captured(1).toInt());
        };
        for (const jp::QtPdf::Obj &obj : q.objects)
            if (obj.body.contains("/S /Document")) walk(obj.id);
        for (const char *t : {"H1", "P", "L", "Table", "Figure"}) QVERIFY2(walked.contains(QLatin1String(t)), t);
        QVERIFY(walked.indexOf(QStringLiteral("H1")) < walked.indexOf(QStringLiteral("P")));
        QVERIFY(walked.indexOf(QStringLiteral("P")) < walked.indexOf(QStringLiteral("L")));
        QVERIFY(walked.indexOf(QStringLiteral("L")) < walked.indexOf(QStringLiteral("Table")));
        QVERIFY(walked.indexOf(QStringLiteral("Table")) < walked.indexOf(QStringLiteral("Figure")));
        QCOMPARE(pdfcheck::marksLeft(pdf), 0);
        const QVector<QByteArray> pages = pdfcheck::pageContents(pdf);
        QVERIFY(!pages.isEmpty());
        for (int i = 0; i < pages.size(); ++i) QCOMPARE(pdfcheck::drawnOutsideMarkedContent(pages[i]), 0);
        // the text is still the PDF's text
        QVERIFY(PdfDocument::shared(pdf)->text(0).contains(QStringLiteral("Some body text.")));
        // untagged by default
        target = dir.filePath(QStringLiteral("plain.pdf"));
        QVERIFY(w.exportPdf());
        QVERIFY(!fileBytes(target).contains("/StructTreeRoot"));
    }

    // PDF/A-1b, which is what Qt's writer makes (its output intent names PDF/A-1): the metadata names part 1, conformance B, never part 2; the output intent is there; every font is
    // embedded.
    void aPdfA1bFileHasItsMarksFontsAndOutputIntent()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        const QString target = dir.filePath(QStringLiteral("archive.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        WriteOutputOptions o;
        o.pdfArchive = true;
        o.pdfTags = true;
        QVERIFY(w.exportPdfPages(o));
        const QByteArray pdf = fileBytes(target);
        QVERIFY(pdf.startsWith("%PDF"));
        QVERIFY2(pdf.contains("<pdfaid:part>1</pdfaid:part>"), "not part 1");
        QVERIFY(pdf.contains("<pdfaid:conformance>B</pdfaid:conformance>"));
        QVERIFY(!pdf.contains("<pdfaid:part>2</pdfaid:part>"));
        QVERIFY(pdf.contains("/OutputIntents"));
        QVERIFY(pdf.contains("GTS_PDFA1"));
        QVERIFY(pdf.contains("/DestOutputProfile"));
        jp::QtPdf q;
        QVERIFY(q.loadBytes(pdf));
        int fonts = 0, descriptors = 0;
        for (const jp::QtPdf::Obj &obj : q.objects) {
            const QByteArray dict = jp::QtPdf::dictOf(obj.body);
            if (dict.contains("/Type /Font\n") || dict.contains("/Type /Font ")) {
                ++fonts;
                QVERIFY2(!dict.contains("/Subtype /Type1") && !dict.contains("/Subtype /TrueType"), "a font that is not embedded as a composite font");
            }
            if (dict.contains("/Type /FontDescriptor")) {
                ++descriptors;
                QVERIFY2(obj.body.contains("/FontFile"), "a font that is not embedded");
            }
        }
        QVERIFY(fonts > 0);
        QVERIFY(descriptors > 0);
        // a plain export has no mark
        WriteOutputOptions plain;
        QVERIFY(w.exportPdfPages(plain));
        QVERIFY(!fileBytes(target).contains("pdfaid"));
    }

    // PDF/A-1 allows no transparency: a shape at 50% on a page is drawn as opaque pixels in a PDF/A file (and stays see-through in a plain one).
    void aPdfA1bFileHasNoTransparency()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        const QString target = dir.filePath(QStringLiteral("archive.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        const auto off = qScopeGuard([] { setPaintObserver(nullptr); });
        setPaintObserver([](const flow::MaterializedPage &page, const PageDrawing &) {
            auto shape = std::make_shared<jp::ShapeItem>();
            shape->rect = QRectF(100, 100, 200, 100);
            shape->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(0, 0, 200)));
            shape->fill.transparency = 0.5;
            page.doc->pages[0]->items.push_back(shape);
        });
        QByteArray made[2];
        for (const bool archive : {true, false}) {
            WriteOutputOptions o;
            o.pdfArchive = archive;
            QVERIFY(w.exportPdfPages(o));
            const QByteArray bytes = fileBytes(target);
            QCOMPARE(jp::pdfTransparency(bytes).found, !archive);
            made[archive] = bytes;
        }
        // (the writer leaves a see-through color out of a PDF/A file without a word, which would turn the shape solid: it is flattened first, so the page looks the same)
        QVERIFY2(farPixels(made[0], made[1], QSize(306, 396)) < 306 * 396 / 200, qPrintable(QString::number(farPixels(made[0], made[1], QSize(306, 396)))));
    }

    // A PDF/A export the check refuses is not made, and says which page of the document the see-through part is on (the PDF's page is a sheet: the document's pages on it are named).
    void aRefusedPdfAFileNamesThePage()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        openRich(w, dir);
        jp::setPdfTransparencyProbeForTest([](const QByteArray &) {
            return jptest::handPdf({"<< /Type /Catalog /Pages 2 0 R >>", "<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
                                    "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 100 100] /Resources << /ExtGState << /GS1 5 0 R >> >> /Contents 4 0 R >>", jptest::handStream({}, "/GS1 gs 0 0 9 9 re f"),
                                    "<< /Type /ExtGState /ca 0.5 >>"});
        });
        const auto reset = qScopeGuard([] { jp::setPdfTransparencyProbeForTest(nullptr); });
        WriteOutputOptions o;
        o.pdfArchive = true;
        QString error;
        QVERIFY(w.pdfBytes({0}, &error, o).isEmpty());
        QVERIFY2(error.contains(QLatin1String("page 1")) && !error.contains(QLatin1String("PDF page")) && error.contains(QLatin1String("PDF/A")), qPrintable(error));
        // (the same file without the check's finding is made)
        jp::setPdfTransparencyProbeForTest(nullptr);
        QVERIFY(!w.pdfBytes({0}, &error, o).isEmpty());
    }

    // The links of a PDF are clickable: a web address goes to its address, and a table of contents entry and a cross-reference go to the page of their heading, in a normal PDF and in a
    // PDF/A one.
    void aPdfHasClickableLinks()
    {
        QTemporaryDir dir;
        const QString docx = dir.filePath(QStringLiteral("links.docx"));
        {
            office::PackageWriter w;
            const QByteArray body = para("Heading1", "Intro") + "<w:p><w:r><w:t xml:space=\"preserve\">See </w:t></w:r><w:hyperlink r:id=\"rId20\"><w:r><w:t>the site</w:t></w:r></w:hyperlink></w:p>" +
                                    para(nullptr, "Also: ") + kPageBreak + para("Heading1", "Second part") + para(nullptr, "more");
            w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
                      kWord + QStringLiteral("document.main+xml"));
            w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            w.addPart(QStringLiteral("/word/styles.xml"),
                      QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs +
                          "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                          "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr></w:style></w:styles>",
                      kWord + QStringLiteral("styles+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId20"), kRel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/a"), true);
            QFile f(docx);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(w.finish());
        }
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(docx));
        WriteDoc &doc = *w.document();
        // a cross-reference to the second heading, at the end of "Also: "
        const QVector<RefTarget> targets = crossReferenceTargets(doc, RefKind::Heading);
        int second = -1;
        for (int i = 0; i < targets.size(); ++i)
            if (targets[i].label.contains(QStringLiteral("Second part"))) second = i;
        QVERIFY(second >= 0);
        QTextCursor at(doc.story());
        at.setPosition(int(doc.story()->toPlainText().indexOf(QStringLiteral("Also: "))) + 6);
        QVERIFY(insertCrossReference(doc, at, targets[second], RefKind::Heading, RefWhat::Text, true));
        // and a table of contents at the top
        QTextCursor top(doc.story());
        top.setPosition(0);
        QVERIFY(insertTableOfContents(doc, top, TocOptions()));
        QString target;
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        for (bool archive : {false, true}) {
            target = dir.filePath(QStringLiteral("links%1.pdf").arg(archive));
            WriteOutputOptions o;
            o.pdfArchive = archive;
            QVERIFY(w.exportPdfPages(o));
            const QByteArray pdf = fileBytes(target);
            QVERIFY(pdf.startsWith("%PDF"));
            // the pages' objects, in order, from the file as Qt wrote it (the updates are after its first end)
            jp::QtPdf q;
            QVERIFY(q.loadBytes(pdf.left(int(pdf.indexOf("%%EOF")) + 6)));
            QVector<int> pageIds;
            for (const jp::QtPdf::Obj &obj : q.objects)
                if (obj.body.contains("/Type /Page\n")) pageIds << obj.id;
            QVERIFY(pageIds.size() >= 2);
            QVERIFY2(pdf.contains("/URI (https://example.com/a)"), "no link to the web address");
            const QByteArray toSecond = "/Dest [" + QByteArray::number(pageIds[1]) + " 0 R";
            QVERIFY2(pdf.count(toSecond) >= 2, qPrintable(QStringLiteral("links to the second page: %1").arg(pdf.count(toSecond))));
            QVERIFY(pdf.count("/Subtype /Link") >= 3);
            if (archive) QVERIFY(pdf.contains("<pdfaid:part>1</pdfaid:part>"));
        }
    }

    // Several pages to a sheet (printed to a PDF file): a bookmark goes to the sheet its heading's page is on, a link is over its words where the page's place on the sheet puts them (and
    // a link to another page goes to that page's place on its sheet), and the pages are tagged in the order they lie on the sheet.
    void aPdfOfSeveralPagesToASheetHasBookmarksLinksAndTags()
    {
        QTemporaryDir dir;
        const QString docx = dir.filePath(QStringLiteral("shared.docx"));
        {
            office::PackageWriter w;
            const QByteArray jump = "<w:p><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> HYPERLINK \\l \"third\" </w:instrText></w:r>"
                                    "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>jump</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>";
            const QByteArray web = "<w:p><w:r><w:t xml:space=\"preserve\">See </w:t></w:r><w:hyperlink r:id=\"rId20\"><w:r><w:t>the site</w:t></w:r></w:hyperlink></w:p>";
            QByteArray body = para("Heading1", "Page one") + jump + kPageBreak + para("Heading1", "Page two") + web + kPageBreak +
                              "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"7\" w:name=\"third\"/><w:r><w:t>Page three</w:t></w:r><w:bookmarkEnd w:id=\"7\"/></w:p>" +
                              kPageBreak + para("Heading1", "Page four") + kPageBreak + para("Heading1", "Page five");
            w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
                      kWord + QStringLiteral("document.main+xml"));
            w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            w.addPart(QStringLiteral("/word/styles.xml"),
                      QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs +
                          "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                          "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr></w:style></w:styles>",
                      kWord + QStringLiteral("styles+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId20"), kRel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/two"), true);
            QFile f(docx);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(w.finish());
        }
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(docx));
        const QString target = dir.filePath(QStringLiteral("shared.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        WriteOutputOptions o;
        o.toFile = true;
        o.perSheet = 4;   // (five pages: pages 1 to 4 on the first sheet, page 5 on the second)
        QVERIFY(w.printOutput(o));
        const QByteArray pdf = fileBytes(target);
        QVERIFY(pdf.startsWith("%PDF"));
        jp::QtPdf q;
        QVERIFY(q.loadBytes(pdf));
        QVector<int> pageIds;
        QMap<int, QSizeF> sheetSize;
        for (const jp::QtPdf::Obj &obj : q.objects)
            if (obj.body.contains("/Type /Page\n")) {
                const auto box = QRegularExpression(QStringLiteral("/MediaBox \\[0 0 ([-0-9.]+) ([-0-9.]+)\\]")).match(QString::fromLatin1(obj.body));
                QVERIFY(box.hasMatch());
                sheetSize.insert(obj.id, QSizeF(box.captured(1).toDouble(), box.captured(2).toDouble()));
                pageIds << obj.id;
            }
        QCOMPARE(pageIds.size(), 2);
        // the bookmarks: the first four headings go to the first sheet, the fifth to the second
        QVector<int> goes;
        {
            const QRegularExpression entry(QStringLiteral("/Title <[0-9A-F]+> /Parent \\d+ 0 R[^>]*?/Dest \\[(\\d+) 0 R"));
            for (auto it = entry.globalMatch(QString::fromLatin1(pdf)); it.hasNext();) goes << int(pageIds.indexOf(it.next().captured(1).toInt()));
        }
        QCOMPARE(goes, (QVector<int>{0, 0, 0, 0, 1}));
        // the web link on page 2 is in the top right quarter of the first sheet (2 by 2 pages)
        const QSizeF sheet = sheetSize[pageIds[0]];
        const auto rect = QRegularExpression(QStringLiteral("/URI \\(https://example.com/two\\)")).match(QString::fromLatin1(pdf));
        QVERIFY2(rect.hasMatch(), "no link to the web address");
        bool found = false;
        for (const QByteArray &line : pdf.split('\n')) {
            if (!line.contains("https://example.com/two")) continue;
            const auto r = QRegularExpression(QStringLiteral("/Rect \\[([-0-9.]+) ([-0-9.]+) ([-0-9.]+) ([-0-9.]+)\\]")).match(QString::fromLatin1(line));
            QVERIFY(r.hasMatch());
            found = true;
            QVERIFY2(r.captured(1).toDouble() > sheet.width() / 2 && r.captured(3).toDouble() < sheet.width(), qPrintable(r.captured(1) + QLatin1Char(' ') + r.captured(3)));
            QVERIFY2(r.captured(2).toDouble() > sheet.height() / 2 && r.captured(4).toDouble() < sheet.height(), qPrintable(r.captured(2) + QLatin1Char(' ') + r.captured(4)));
        }
        QVERIFY(found);
        // the link to the bookmark on page 3 goes to its spot in the bottom left quarter of the first sheet
        const QRegularExpression xyz(QStringLiteral("/Dest \\[%1 0 R /XYZ ([-0-9.]+) ([-0-9.]+) null\\]").arg(pageIds[0]));
        const auto spot = xyz.match(QString::fromLatin1(pdf));
        QVERIFY2(spot.hasMatch(), "no link to the third page");
        QVERIFY2(spot.captured(1).toDouble() < sheet.width() / 2 && spot.captured(2).toDouble() < sheet.height() / 2, qPrintable(spot.captured(1) + QLatin1Char(' ') + spot.captured(2)));
        // the tags: each page's words in order, and no mark left
        QVERIFY(pdf.contains("/StructTreeRoot"));
        QCOMPARE(pdfcheck::marksLeft(pdf), 0);
        int headings = 0;
        for (const jp::QtPdf::Obj &obj : q.objects)
            if (obj.body.contains("/Type /StructElem") && obj.body.contains("/S /H1")) ++headings;
        QCOMPARE(headings, 5);
        // nothing is drawn outside marked content on a tagged page: the frames round the pages are decoration, marked as such
        const QVector<QByteArray> contents = pdfcheck::pageContents(pdf);
        QCOMPARE(int(contents.size()), 2);
        for (const QByteArray &c : contents) QCOMPARE(pdfcheck::drawnOutsideMarkedContent(c), 0);
        QCOMPARE(pdfcheck::structureProblems(pdf), QString());
    }

    // A link in a table or in a header has its box where its words are, and a bookmark link (HYPERLINK \l) goes to the spot of its bookmark on the page, not the page's top.
    void linksInTablesAndHeadersAndToBookmarksGoWhereTheyShould()
    {
        QTemporaryDir dir;
        const QString docx = dir.filePath(QStringLiteral("spots.docx"));
        {
            office::PackageWriter w;
            QByteArray later;
            for (int i = 0; i < 12; ++i) later += para(nullptr, "filler");
            const QByteArray jump = "<w:p><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> HYPERLINK \\l \"third\" </w:instrText></w:r>"
                                    "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>jump</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>";
            const QByteArray cell = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"4000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t xml:space=\"preserve\">in a table: </w:t></w:r>"
                                    "<w:hyperlink r:id=\"rId20\"><w:r><w:t>tablelink</w:t></w:r></w:hyperlink></w:p></w:tc></w:tr></w:tbl>";
            const QByteArray body = para("Heading1", "Intro") + jump + cell + para(nullptr, "after") + kPageBreak + para("Heading1", "Second part") + later +
                                    "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"7\" w:name=\"third\"/><w:r><w:t>Third part</w:t></w:r><w:bookmarkEnd w:id=\"7\"/></w:p>" + para(nullptr, "end") + "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdh\"/></w:sectPr>";
            w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
                      kWord + QStringLiteral("document.main+xml"));
            w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            w.addPart(QStringLiteral("/word/styles.xml"),
                      QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs +
                          "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                          "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr></w:style></w:styles>",
                      kWord + QStringLiteral("styles+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId20"), kRel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/table"), true);
            w.addPart(QStringLiteral("/word/header1.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:hdr ") + kNs +
                          "><w:p><w:r><w:t xml:space=\"preserve\">Head </w:t></w:r><w:hyperlink r:id=\"rId30\"><w:r><w:t>headerlink</w:t></w:r></w:hyperlink></w:p></w:hdr>",
                      kWord + QStringLiteral("header+xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdh"), kRel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
            w.addRelationshipWithId(QStringLiteral("/word/header1.xml"), QStringLiteral("rId30"), kRel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/header"), true);
            QFile f(docx);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(w.finish());
        }
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(docx));
        QString target = dir.filePath(QStringLiteral("spots.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        const QByteArray pdf = fileBytes(target);
        jp::QtPdf q;
        QVERIFY(q.loadBytes(pdf.left(int(pdf.indexOf("%%EOF")) + 6)));
        QVector<int> pageIds;
        for (const jp::QtPdf::Obj &obj : q.objects)
            if (obj.body.contains("/Type /Page\n")) pageIds << obj.id;
        QVERIFY(pageIds.size() >= 2);
        auto rectOf = [&](const char *uri) {
            const QRegularExpression re(QStringLiteral("/Rect \\[([-0-9. ]+)\\][^>]*/URI \\(") + QRegularExpression::escape(QString::fromLatin1(uri)) + QStringLiteral("\\)"));
            const auto m = re.match(QString::fromLatin1(pdf));
            QVector<double> v;
            if (m.hasMatch())
                for (const QString &n : m.captured(1).split(QLatin1Char(' '), Qt::SkipEmptyParts)) v << n.toDouble();
            return v;
        };
        // the table's link: a box as small as its words (not the line the table's rows stand for), under the heading
        const QVector<double> table = rectOf("https://example.com/table");
        QVERIFY2(table.size() == 4, "no box for the link in the table");
        QVERIFY(table[2] - table[0] > 10 && table[2] - table[0] < 200);
        QVERIFY(table[3] - table[1] > 4 && table[3] - table[1] < 40);
        // the header's link: in the top margin
        const QVector<double> head = rectOf("https://example.com/header");
        QVERIFY2(head.size() == 4, "no box for the link in the header");
        QVERIFY2(head[1] > 700, qPrintable(QString::number(head[1])));
        // the bookmark link goes to the spot of the bookmark on the second page, well below the page's top
        QRegularExpression xyz(QStringLiteral("/Dest \\[%1 0 R /XYZ ([-0-9.]+) ([-0-9.]+) null\\]").arg(pageIds[1]));
        const auto m = xyz.match(QString::fromLatin1(pdf));
        QVERIFY2(m.hasMatch(), "no link to a spot of the second page");
        QVERIFY2(m.captured(2).toDouble() > 0 && m.captured(2).toDouble() < 600, qPrintable(m.captured(2)));
    }

    // A link in a footnote and in an endnote has its box, and goes to the address its own part names (the notes' relationships are their own: the same id in each part is
    // another address).
    void linksInNotesAreClickable()
    {
        QTemporaryDir dir;
        const QString docx = dir.filePath(QStringLiteral("notes.docx"));
        {
            office::PackageWriter w;
            const QByteArray vml = "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\"";
            const QByteArray box = "<w:r><w:pict " + vml + "><v:rect id=\"t\" style=\"position:absolute;margin-left:200pt;margin-top:300pt;width:160pt;height:40pt;z-index:3\" stroked=\"f\">"
                                   "<v:textbox inset=\"0,0,0,0\"><w:txbxContent><w:p><w:hyperlink r:id=\"rId20\"><w:r><w:t>boxlink</w:t></w:r></w:hyperlink></w:p></w:txbxContent></v:textbox></v:rect></w:pict></w:r>";
            const QByteArray body = "<w:p><w:r><w:t>text</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r>" + box + "</w:p>";
            w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
                      kWord + QStringLiteral("document.main+xml"));
            w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            w.addPart(QStringLiteral("/word/styles.xml"),
                      QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs + "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style></w:styles>",
                      kWord + QStringLiteral("styles+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rId20"), kRel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/box"), true);
            auto note = [&](const char *tag, const char *ref, const char *id) {
                return QByteArray("<w:") + tag + "s " + kNs + "><w:" + tag + " w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:" + tag + "><w:" + tag +
                       " w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:" + tag + "><w:" + tag + " w:id=\"2\"><w:p><w:r><w:" + ref +
                       "/></w:r><w:hyperlink r:id=\"" + id + "\"><w:r><w:t>notelink</w:t></w:r></w:hyperlink></w:p></w:" + tag + "></w:" + tag + "s>";
            };
            w.addPart(QStringLiteral("/word/footnotes.xml"), note("footnote", "footnoteRef", "rId1"), kWord + QStringLiteral("footnotes+xml"));
            w.addPart(QStringLiteral("/word/endnotes.xml"), note("endnote", "endnoteRef", "rId1"), kWord + QStringLiteral("endnotes+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("endnotes"), QStringLiteral("/word/endnotes.xml"));
            w.addRelationshipWithId(QStringLiteral("/word/footnotes.xml"), QStringLiteral("rId1"), kRel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/foot"), true);
            w.addRelationshipWithId(QStringLiteral("/word/endnotes.xml"), QStringLiteral("rId1"), kRel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/end"), true);
            QFile f(docx);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(w.finish());
        }
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(docx));
        const QString target = dir.filePath(QStringLiteral("notes.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        const QByteArray pdf = fileBytes(target);
        QVERIFY2(pdf.contains("/URI (https://example.com/foot)"), "no link in the footnote");
        QVERIFY2(pdf.contains("/URI (https://example.com/end)"), "no link in the endnote");
    }

    void theBuiltInTemplatesOpenAndSaveAsDocx()
    {
        const QVector<BuiltInTemplate> all = builtInTemplates();
        QStringList ids;
        for (const BuiltInTemplate &t : all) {
            ids << t.id;
            QVERIFY(!t.name.isEmpty());
            QVERIFY(!t.category.isEmpty());
            auto doc = builtInTemplateDocument(t.id);
            QVERIFY2(doc, qPrintable(t.id));
            QVERIFY(!doc->story()->toPlainText().trimmed().isEmpty());
            doc->pager().paginate(0);
            QVERIFY(doc->pager().materialize(0));
            // what is saved opens again with the same words
            QString err;
            const QByteArray bytes = doc->toDocx(&err);
            QVERIFY2(!bytes.isEmpty(), qPrintable(err));
            auto again = WriteDoc::fromDocx(bytes, &err, nullptr);
            QVERIFY2(again, qPrintable(err));
            QCOMPARE(again->story()->toPlainText(), doc->story()->toPlainText());
        }
        for (const char *id : {"letter", "report", "brief", "manuscript", "newsletter"}) QVERIFY2(ids.contains(QLatin1String(id)), id);
        QVERIFY(!builtInTemplateDocument(QStringLiteral("no such template")));
    }

    void eachBuiltInTemplateFitsItsFirstPage()
    {
        // (a letter, a brief, and a newsletter are one page of placeholder text: the newsletter's stories begin under its masthead, not on a page of their own)
        for (const char *id : {"letter", "brief", "newsletter"}) {
            auto doc = builtInTemplateDocument(QString::fromLatin1(id));
            QVERIFY(doc);
            flow::Paginator &pg = doc->pager();
            while (!pg.complete()) {
                const int known = pg.pagesKnown();
                pg.paginate(known);
                if (pg.pagesKnown() == known) break;
            }
            QVERIFY2(pg.pagesKnown() == 1, id);
        }
    }

    void printingBothSidesByHandPrintsOddThenEvenAfterAPrompt()
    {
        QTemporaryDir dir;
        WriteWindow w;
        w.show();
        for (int i = 0; i < 4; ++i) {
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("page text"));
            insertPageBreak(*w.document(), w.view()->cursor());
        }
        WriteOutputOptions o;
        o.byHand = true;
        const QVector<int> all = w.outputPages(WriteOutputOptions());
        QVERIFY(all.size() >= 5);
        const QVector<WriteOutputOptions> runs = w.handDuplexRuns(o);
        QCOMPARE(runs.size(), 2);
        // the sheets, one page each: the first, third, ... then the others last first; a last front with no back leaves a blank sheet (-1) first, so every back is behind its front
        QVector<int> odd, even;
        for (int i = 0; i < all.size(); ++i) (i % 2 ? even : odd) << all[i];
        std::reverse(even.begin(), even.end());
        if (all.size() % 2) even.prepend(-1);
        QCOMPARE(w.outputPages(runs[0]), odd);
        QCOMPARE(w.outputPages(runs[1]), even);
        QCOMPARE(runs[1].sides, WriteOutputOptions::Sides::All);
        // the printing: two runs with one prompt between them, and nothing more when the prompt is declined
        QStringList order;
        w.setPrinterPreparer([&](QPrinter &printer) {
            order << QStringLiteral("print");
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(dir.filePath(QStringLiteral("p%1.pdf").arg(order.size())));
        });
        bool turn = true;
        w.setStackTurner([&] {
            order << QStringLiteral("turn");
            return turn;
        });
        QVERIFY(w.printOutput(o));
        QCOMPARE(order, (QStringList{QStringLiteral("print"), QStringLiteral("turn"), QStringLiteral("print")}));
        order.clear();
        turn = false;
        QVERIFY(!w.printOutput(o));
        QCOMPARE(order, (QStringList{QStringLiteral("print"), QStringLiteral("turn")}));
        // one page has no even side: it is printed, and no prompt comes
        order.clear();
        o.range = WriteOutputOptions::Range::Pages;
        o.pages = QStringLiteral("1");
        QVERIFY(w.printOutput(o));
        QCOMPARE(order, (QStringList{QStringLiteral("print")}));
    }

    // Two pages on a sheet: the runs are made of physical sheets, a short last sheet keeps its page, and a front with no back leaves a blank sheet.
    void bothSidesByHandWithSeveralPagesOnASheetGoesBySheets()
    {
        WriteWindow w;
        w.show();
        for (int i = 0; i < 4; ++i) {
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("page text"));
            insertPageBreak(*w.document(), w.view()->cursor());
        }
        const QVector<int> all = w.outputPages(WriteOutputOptions());
        QCOMPARE(all.size(), 5);
        WriteOutputOptions o;
        o.byHand = true;
        o.perSheet = 2;   // (three sheets: pages 1 and 2, 3 and 4, and 5 alone)
        const QVector<WriteOutputOptions> runs = w.handDuplexRuns(o);
        QCOMPARE(w.outputPages(runs[0]), (QVector<int>{all[0], all[1], all[4], -1}));
        QCOMPARE(w.outputPages(runs[1]), (QVector<int>{-1, -1, all[2], all[3]}));
        QCOMPARE(runs[1].perSheet, 2);
        // four on a sheet: two sheets, the first front and the second its back
        o.perSheet = 4;
        const QVector<WriteOutputOptions> four = w.handDuplexRuns(o);
        QCOMPARE(w.outputPages(four[0]), (QVector<int>{all[0], all[1], all[2], all[3]}));
        QCOMPARE(w.outputPages(four[1]), (QVector<int>{all[4], -1, -1, -1}));
        // one sheet: nothing to turn over
        o.perSheet = 6;
        const QVector<WriteOutputOptions> one = w.handDuplexRuns(o);
        QCOMPARE(w.outputPages(one[0]).size(), 6);
        QVERIFY(w.outputPages(one[1]).isEmpty());
    }

    void anEmailMessageHoldsItsAttachmentsAndPictures()
    {
        const QByteArray data = QByteArray("PK\x03\x04 some bytes that are not text \xFF\x00\x01", 36).repeated(10);
        const QByteArray png("\x89PNG fake picture", 17);
        const QByteArray eml = emailMessage(QStringLiteral("Caf\u00E9 \"report\"\nnext line"), QStringLiteral("See attached."),
                                            {{QStringLiteral("report.docx"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document"), data, QString()}},
                                            QStringLiteral("<p>pages</p><img src=\"cid:page1\">"), {{QStringLiteral("page1.png"), QStringLiteral("image/png"), png, QStringLiteral("page1")}});
        // header lines end with CR LF, and the subject is one line, encoded because it has more than ASCII, with no quote marks in it to end it
        QVERIFY(eml.startsWith("Subject: =?UTF-8?B?"));
        QVERIFY(eml.contains("MIME-Version: 1.0\r\n"));
        QVERIFY(eml.contains("X-Unsent: 1\r\n"));
        const int subjectAt = eml.indexOf("Subject: ");
        const QByteArray subjectLine = eml.mid(subjectAt, eml.indexOf("\r\n", subjectAt) - subjectAt);
        const QByteArray encoded = subjectLine.mid(subjectLine.indexOf("?B?") + 3, subjectLine.size() - subjectLine.indexOf("?B?") - 5);
        QCOMPARE(QString::fromUtf8(QByteArray::fromBase64(encoded)), QStringLiteral("Caf\u00E9 'report' next line"));
        // the attachment comes back whole from its base64 lines, none longer than 76 characters
        const int at = eml.indexOf("filename=\"report.docx\"");
        QVERIFY(at > 0);
        const int bodyAt = eml.indexOf("\r\n\r\n", at) + 4;
        const int bodyEnd = eml.indexOf("\r\n--", bodyAt);
        QByteArray b64 = eml.mid(bodyAt, bodyEnd - bodyAt);
        for (const QByteArray &line : b64.split('\n')) QVERIFY(line.trimmed().size() <= 76);
        QCOMPARE(QByteArray::fromBase64(b64.replace("\r\n", "")), data);
        // the picture is inline, named by its content id, inside the part that holds the page's text
        QVERIFY(eml.contains("Content-ID: <page1>"));
        QVERIFY(eml.contains("multipart/related"));
        QVERIFY(eml.contains("multipart/mixed"));
        // two messages never share a boundary with the content: no line of the data starts with the boundary
        const int boundaryAt = eml.indexOf("boundary=\"") + 10;
        const QByteArray boundary = eml.mid(boundaryAt, eml.indexOf('"', boundaryAt) - boundaryAt);
        QVERIFY(boundary.size() >= 16);
        QVERIFY(!data.contains(boundary));
        QVERIFY(eml.trimmed().endsWith("--" + boundary + "--"));
        // without pictures it is a plain list of attachments after the words
        const QByteArray simple = emailMessage(QStringLiteral("Hello"), QStringLiteral("Words"), {{QStringLiteral("a.pdf"), QStringLiteral("application/pdf"), QByteArray("%PDF"), QString()}});
        QVERIFY(simple.contains("Subject: Hello\r\n"));
        QVERIFY(!simple.contains("multipart/related"));
        QVERIFY(simple.contains("Content-Type: text/plain; charset=utf-8"));
    }

    void emailMakesAnEmlWithTheDocumentThePdfOrThePages()
    {
        QTemporaryDir dir;
        WriteWindow w;
        w.show();
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Hello by mail"));
        QString asked;
        w.setSavePathChooser([&](const QString &suggested, const QString &) {
            asked = suggested;
            return dir.filePath(QStringLiteral("m.eml"));
        });
        for (auto what : {WriteWindow::EmailWhat::Document, WriteWindow::EmailWhat::Pdf, WriteWindow::EmailWhat::Pages}) {
            QFile::remove(dir.filePath(QStringLiteral("m.eml")));
            QVERIFY(w.exportEmail(what));
            QVERIFY(asked.endsWith(QLatin1String(".eml")));
            QFile f(dir.filePath(QStringLiteral("m.eml")));
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray eml = f.readAll();
            QVERIFY(eml.contains("Subject: Document"));
            if (what == WriteWindow::EmailWhat::Document) QVERIFY(eml.contains(".docx\""));
            if (what == WriteWindow::EmailWhat::Pdf) QVERIFY(eml.contains("application/pdf"));
            if (what == WriteWindow::EmailWhat::Pages) QVERIFY(eml.contains("image/png") && eml.contains("Content-ID: <page1>"));
        }
    }

    void versionsAreKeptListedReadAndRemoved()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("a.docx"));
        VersionStore store(path);
        store.removeAll();
        QVERIFY(store.list().isEmpty());
        QVERIFY(store.add(QStringLiteral("First draft"), QByteArray("one")));
        QVERIFY(store.add(QStringLiteral("  Second   draft "), QByteArray("two two")));
        QVector<DocVersion> all = store.list();
        QCOMPARE(all.size(), 2);
        QCOMPARE(all[0].name, QStringLiteral("Second draft"));   // newest first, the name tidied
        QCOMPARE(all[1].name, QStringLiteral("First draft"));
        QCOMPARE(all[0].size, qint64(7));
        QCOMPARE(store.read(all[1].file), QByteArray("one"));
        // the list is kept: another store for the same file has it; another file has none
        QCOMPARE(VersionStore(path).list().size(), 2);
        QVERIFY(VersionStore(dir.filePath(QStringLiteral("b.docx"))).list().isEmpty());
        // a name that is a path is no version
        QVERIFY(store.read(QStringLiteral("../versions.json")).isEmpty());
        QVERIFY(store.read(QStringLiteral("nothing.docx")).isEmpty());
        QVERIFY(!store.remove(QStringLiteral("../x.docx")));
        // an empty copy is refused
        QVERIFY(!store.add(QStringLiteral("empty"), QByteArray()));
        QVERIFY(store.remove(all[1].file));
        QCOMPARE(store.list().size(), 1);
        QVERIFY(store.read(all[1].file).isEmpty());
        // only the most recent are kept
        for (int i = 0; i < kMostVersions + 5; ++i) QVERIFY(store.add(QStringLiteral("v%1").arg(i), QByteArray::number(i)));
        all = store.list();
        QCOMPARE(all.size(), kMostVersions);
        QCOMPARE(all.first().name, QStringLiteral("v%1").arg(kMostVersions + 4));
        store.removeAll();
        QVERIFY(store.list().isEmpty());
    }

    void aWindowKeepsComparesAndRestoresVersions()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("first words"));
        // an untitled document has no file to keep versions of
        QVERIFY(!w.saveVersion(QStringLiteral("early")));
        QVERIFY(!quiet.said.isEmpty());
        QVERIFY(saveInto(w, dir, QStringLiteral("v.docx")));
        VersionStore(w.filePath()).removeAll();
        QVERIFY(w.saveVersion(QStringLiteral("Draft one")));
        typeText(*w.document(), w.view()->cursor(), QStringLiteral(" and more words"));
        QVERIFY(w.saveVersion(QStringLiteral("Draft two")));
        QVector<DocVersion> all = w.versions();
        QCOMPARE(all.size(), 2);
        // restore the first: the words go back, the document is changed (to save), and what it was is kept first
        QVERIFY(w.restoreVersion(all[1].file));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("first words"));
        QVERIFY(w.isModified());
        all = w.versions();
        QCOMPARE(all.size(), 3);
        QVERIFY(all.first().name.contains(QLatin1String("restor")));
        QCOMPARE(VersionStore(w.filePath()).read(all.first().file).isEmpty(), false);
        // compare the document with the second draft: the words that differ are found as changes
        QVERIFY(w.compareWithVersion(all[1].file) >= 1);
        QVERIFY(!w.restoreVersion(QStringLiteral("nothing.docx")));
        VersionStore(w.filePath()).removeAll();
    }

    void pageRangesTakeThePageOfASectionAndAWholeSection()
    {
        // six pages in three sections: 0 0 1 1 1 2
        const QVector<int> sections{0, 0, 1, 1, 1, 2};
        auto pages = [&](const char *text) { return parsePageRange(QString::fromLatin1(text), 6, sections); };
        QCOMPARE(pages("p2s2"), (QVector<int>{3}));
        QCOMPARE(pages("s2"), (QVector<int>{2, 3, 4}));
        QCOMPARE(pages("P1S2-p1s3"), (QVector<int>{2, 3, 4, 5}));
        QCOMPARE(pages("s3, 1"), (QVector<int>{5, 0}));
        QCOMPARE(pages("1-2, s3"), (QVector<int>{0, 1, 5}));
        QCOMPARE(pages("p1s1, s1"), (QVector<int>{0, 1}));   // (each page once)
        QCOMPARE(pages("p2s3"), (QVector<int>{}));           // (section 3 has one page)
        QCOMPARE(pages("s4, p0s1, ps1, p1, sx"), (QVector<int>{}));
        // the plain syntax is as it was, with or without the sections
        QCOMPARE(pages("2-3, 8-"), (QVector<int>{1, 2}));
        QCOMPARE(parsePageRange(QStringLiteral("s1"), 6), (QVector<int>{}));
        QCOMPARE(parsePageRange(QStringLiteral("1-3, 5"), 6), (QVector<int>{0, 1, 2, 4}));
    }

    void aWindowPrintsASectionByItsNumber()
    {
        QTemporaryDir dir;
        const QString docx = dir.filePath(QStringLiteral("s.docx"));
        const QByteArray cut = "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr></w:pPr></w:p>";
        const QByteArray body = para(nullptr, "one") + kPageBreak + para(nullptr, "two") + cut + para(nullptr, "three") + kPageBreak + para(nullptr, "four");
        {
            QFile f(docx);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(headingsDocx(body));
        }
        WriteWindow w;
        w.show();
        Quiet quiet(w);
        QVERIFY(w.openFile(docx));
        WriteOutputOptions o;
        QCOMPARE(w.outputPages(o).size(), 4);
        o.range = WriteOutputOptions::Range::Pages;
        o.pages = QStringLiteral("s2");
        QCOMPARE(w.outputPages(o), (QVector<int>{2, 3}));
        o.pages = QStringLiteral("p2s1, p1s2");
        QCOMPARE(w.outputPages(o), (QVector<int>{1, 2}));
    }

    void aPdfCanHoldOnlyTheRangeChosen()
    {
        QTemporaryDir dir;
        WriteWindow w;
        w.show();
        Quiet quiet(w);
        for (int i = 0; i < 3; ++i) {
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("page text"));
            insertPageBreak(*w.document(), w.view()->cursor());
        }
        const QString path = dir.filePath(QStringLiteral("r.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        auto pagesIn = [&](const WriteOutputOptions &o) {
            QFile::remove(path);
            if (!w.exportPdfPages(o)) return -1;
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly)) return -1;
            return PdfDocument::shared(f.readAll())->pageCount();
        };
        WriteOutputOptions o;
        const int all = pagesIn(o);
        QVERIFY(all >= 4);
        o.range = WriteOutputOptions::Range::Pages;
        o.pages = QStringLiteral("2-3");
        QCOMPARE(pagesIn(o), 2);
        o.pages = QStringLiteral("99");
        QCOMPARE(pagesIn(o), -1);   // (no page of the document: nothing is written)
        QVERIFY(!QFileInfo::exists(path));
        o.range = WriteOutputOptions::Range::Current;
        QCOMPARE(pagesIn(o), 1);
    }

    void printingTheSelectionPrintsOnlyIt()
    {
        QTemporaryDir dir;
        WriteWindow w;
        w.show();
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("alpha\nbeta words\ngamma"));
        WriteOutputOptions o;
        o.range = WriteOutputOptions::Range::Selection;
        // nothing is selected: nothing to print
        QVERIFY(w.outputPages(o).isEmpty());
        QVERIFY(!w.selectionDocument());
        QTextCursor c(w.document()->story());
        c.setPosition(6);
        c.setPosition(16, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        QCOMPARE(w.outputPages(o).size(), 1);
        auto doc = w.selectionDocument();
        QVERIFY(doc);
        const QString text = doc->story()->toPlainText();
        QVERIFY(text.contains(QLatin1String("beta words")));
        QVERIFY(!text.contains(QLatin1String("alpha")));
        QVERIFY(!text.contains(QLatin1String("gamma")));
        // the document itself is as it was
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("alpha\nbeta words\ngamma"));
        // printed (to a file here): a page that holds the selection alone
        const QString path = dir.filePath(QStringLiteral("sel.pdf"));
        w.setPrinterPreparer([&](QPrinter &printer) {
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(path);
        });
        QVERIFY(w.printOutput(o));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(PdfDocument::shared(f.readAll())->pageCount(), 1);
        // the image of the page shown for the Print page is the selection's, not the document's
        QVERIFY(!w.outputPageImage(o, 0, 200).isNull());
    }

    void thePrintPreviewShowsSeveralPagesAndZooms()
    {
        WriteWindow w;
        w.resize(1400, 900);
        w.show();
        for (int i = 0; i < 5; ++i) {
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("page text"));
            insertPageBreak(*w.document(), w.view()->cursor());
        }
        const int total = w.outputPages(WriteOutputOptions()).size();
        QVERIFY(total >= 6);
        w.showFileScreen(QStringLiteral("print"));
        FileScreen *screen = w.findChild<FileScreen *>();
        QVERIFY(screen);
        auto *label = screen->findChild<QLabel *>(QStringLiteral("printPage"));
        auto *preview = screen->findChild<QLabel *>(QStringLiteral("printPreview"));
        auto *across = screen->findChild<QComboBox *>(QStringLiteral("printAcross"));
        auto *zoom = screen->findChild<QComboBox *>(QStringLiteral("printZoom"));
        auto *next = screen->findChild<QToolButton *>(QStringLiteral("printNext"));
        QVERIFY(label && preview && across && zoom && next);
        QTRY_VERIFY(!preview->pixmap(Qt::ReturnByValue).isNull());
        QCOMPARE(label->text(), QStringLiteral("Page 1 of %1").arg(total));
        const QSizeF one = preview->pixmap(Qt::ReturnByValue).deviceIndependentSize();
        // two pages side by side: the picture is wider, the words say which pages, and the arrows go two at a time
        across->setCurrentIndex(across->findData(2));
        QTRY_COMPARE(label->text(), QStringLiteral("Pages 1-2 of %1").arg(total));
        // (a page is taller than wide; two side by side are wider than tall)
        QVERIFY(one.height() > one.width());
        QTRY_VERIFY(preview->pixmap(Qt::ReturnByValue).deviceIndependentSize().width() > preview->pixmap(Qt::ReturnByValue).deviceIndependentSize().height());
        next->click();
        QTRY_COMPARE(label->text(), QStringLiteral("Pages 3-4 of %1").arg(total));
        // four: two by two
        across->setCurrentIndex(across->findData(4));
        QTRY_COMPARE(label->text(), QStringLiteral("Pages 1-4 of %1").arg(total));
        // zoom: the page is drawn larger than the one that fits
        across->setCurrentIndex(across->findData(1));
        QTRY_COMPARE(label->text(), QStringLiteral("Page 1 of %1").arg(total));
        const QSizeF fit = preview->pixmap(Qt::ReturnByValue).deviceIndependentSize();
        zoom->setCurrentIndex(zoom->findData(200));
        QTRY_VERIFY(preview->pixmap(Qt::ReturnByValue).deviceIndependentSize().width() > fit.width() * 1.7);
        w.hideFileScreen();
    }

    void aWindowStartsAnUntitledDocumentFromABuiltInTemplate()
    {
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.newFromBuiltInTemplate(QStringLiteral("report")));
        QVERIFY(w.filePath().isEmpty());
        QVERIFY(w.document()->story()->toPlainText().contains(QLatin1String("Summary")));
        QVERIFY(w.isModified());   // (a new document made from a template is asked about on closing, as one made from a saved template is)
        QVERIFY(!w.newFromBuiltInTemplate(QStringLiteral("nothing")));
    }

    // A new document starts on Letter in the United States and Canada, and on A4 elsewhere (the United Kingdom and Australia among them); a template's page follows the same rule.
    void aNewDocumentsPaperFollowsTheRegion()
    {
        struct Case { const char *locale; int w, h; };
        for (const Case &c : {Case{"en_US", 12240, 15840}, Case{"en_CA", 12240, 15840}, Case{"en_GB", 11906, 16838}, Case{"en_AU", 11906, 16838}, Case{"de_DE", 11906, 16838}}) {
            PinnedLocale pinned{QLocale(QLatin1String(c.locale))};
            const auto doc = write::WriteDoc::blank();
            QVERIFY2(doc->sectionProps(0).pageW == c.w && doc->sectionProps(0).pageH == c.h, c.locale);
            WriteWindow w;
            Quiet quiet(w);
            QVERIFY(w.newFromBuiltInTemplate(QStringLiteral("report")));
            QVERIFY2(w.document()->sectionProps(0).pageW == c.w && w.document()->sectionProps(0).pageH == c.h, c.locale);
        }
    }

    // A document from the internet or another computer opens for reading first, unless Options > Security says to open it as any other.
    void aFileFromOutsideOpensForReadingFirst()
    {
        struct Reset {
            ~Reset()
            {
                trust::setOutsideStandIn({});
                trust::setOutsideFilesReadOnly(true);
            }
        } reset;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto make = [&](const QString &name) {
            QFile f(dir.filePath(name));
            if (f.open(QIODevice::WriteOnly)) f.write(headingsDocx(para(nullptr, "Some words.")));
            return dir.filePath(name);
        };
        const QString down = make(QStringLiteral("down.docx")), mine = make(QStringLiteral("mine.docx"));
        QVERIFY(QFileInfo(down).size() > 0 && QFileInfo(mine).size() > 0);
        trust::setOutsideStandIn([&](const QString &p) { return p == down; });
        {
            WriteWindow w;
            Quiet quiet(w);
            QVERIFY(w.openFile(down));
            QVERIFY(w.isReadOnly());
        }
        {
            WriteWindow w;
            Quiet quiet(w);
            QVERIFY(w.openFile(mine));
            QVERIFY(!w.isReadOnly());
        }
        {
            WriteWindow w;
            Quiet quiet(w);
            QVERIFY(w.openFile(down, OpenHow::Copy));
            QVERIFY(!w.isReadOnly());
        }
        trust::setOutsideFilesReadOnly(false);
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(down));
        QVERIFY(!w.isReadOnly());
    }

    // The bar of a document held back has Enable Editing: that file only, for the session (and the note is still there after the document is set).
    void enableEditingOpensThatDocumentForChangesAndOnlyThatOne()
    {
        struct Reset {
            ~Reset() { trust::setOutsideStandIn({}); }
        } reset;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto make = [&](const QString &name) {
            QFile f(dir.filePath(name));
            if (f.open(QIODevice::WriteOnly)) f.write(headingsDocx(para(nullptr, "Some words.")));
            return dir.filePath(name);
        };
        const QString a = make(QStringLiteral("a.docx")), b = make(QStringLiteral("b.docx"));
        QVERIFY(QFileInfo(a).size() > 0 && QFileInfo(b).size() > 0);
        trust::setOutsideStandIn([&](const QString &p) { return p == a || p == b; });
        WriteWindow w;
        Quiet quiet(w);
        w.show();
        QVERIFY(w.openFile(a));
        QVERIFY(w.isReadOnly());
        auto *enable = w.findChild<QPushButton *>(QStringLiteral("enableEditing"));
        QVERIFY2(enable, "the held-back bar has no Enable Editing button");
        enable->click();
        QVERIFY(!w.isReadOnly());
        QTRY_VERIFY(!w.findChild<QWidget *>(QStringLiteral("heldBackBar")));
        WriteWindow again;
        Quiet quietAgain(again);
        QVERIFY(again.openFile(a));
        QVERIFY(!again.isReadOnly());
        WriteWindow other;
        Quiet quietOther(other);
        QVERIFY(other.openFile(b));
        QVERIFY(other.isReadOnly());
    }
    // Pack and Go saves the document with the picture it links to and the files its fields name, and with the links moved inside the package, so that it opens from the new folder.
    void packAndGoTakesTheLinkedFilesAndMovesTheLinks()
    {
        QTemporaryDir dir;
        QImage image(40, 30, QImage::Format_RGB32);
        image.fill(Qt::red);
        const QString picture = dir.filePath(QStringLiteral("elsewhere/photo.png")), fieldPicture = dir.filePath(QStringLiteral("elsewhere/field.png"));
        QVERIFY(QDir().mkpath(dir.filePath(QStringLiteral("elsewhere"))));
        QVERIFY(image.save(picture) && image.save(fieldPicture));
        // (a linked document travels only from the document's own folder)
        const QString text = dir.filePath(QStringLiteral("work/part.docx"));
        QVERIFY(QDir().mkpath(dir.filePath(QStringLiteral("work"))));
        {
            QFile f(text);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("other document");
        }
        const QString docx = dir.filePath(QStringLiteral("work/linked.docx"));
        QVERIFY(QDir().mkpath(dir.filePath(QStringLiteral("work"))));
        {
            QFile f(docx);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(linkedDocx(picture, fieldPicture, text));
        }
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(docx));
        QDir().mkpath(dir.filePath(QStringLiteral("dest")));
        const jp::PackReport report = w.packAndGoTo(dir.filePath(QStringLiteral("dest")), jp::PackMode::Folder);
        QVERIFY2(report.ok, qPrintable(report.error));
        QCOMPARE(report.packed(), 3);
        QVERIFY(QFileInfo(report.path + QStringLiteral("/linked/photo.png")).isFile());
        QVERIFY(QFileInfo(report.path + QStringLiteral("/linked/field.png")).isFile());
        QVERIFY(QFileInfo(report.path + QStringLiteral("/linked/part.docx")).isFile());
        // the package opens from its folder with every link inside
        QFile packed(report.path + QStringLiteral("/") + w.displayName() + QStringLiteral(".docx"));
        QVERIFY(packed.open(QIODevice::ReadOnly));
        const QVector<jp::PackLink> links = jp::OpcPackSource::linksOf(packed.readAll());
        QCOMPARE(links.size(), 3);
        for (const jp::PackLink &l : links) {
            QVERIFY2(l.target.startsWith(QLatin1String("linked/")), qPrintable(l.target));
            QVERIFY2(QFileInfo(report.path + QLatin1Char('/') + l.target).isFile(), qPrintable(l.target));
        }
        // every relationship of the packed file is inside it or external, and it opens with nothing said (no repair)
        int external = 0;
        QCOMPARE(packedcheck::problemsOf(fileBytes(report.path + QStringLiteral("/") + w.displayName() + QStringLiteral(".docx")), &external), QString());
        QVERIFY(external >= 1);
        WriteWindow v;
        Quiet quietV(v);
        QVERIFY(v.openFile(report.path + QStringLiteral("/") + w.displayName() + QStringLiteral(".docx")));
        QVERIFY2(quietV.said.isEmpty(), qPrintable(quietV.said.join(QLatin1Char('|'))));
        // the same, in a ZIP, with the font it names left out of it when the program has none
        const jp::PackReport zip = w.packAndGoTo(dir.filePath(QStringLiteral("pack.zip")), jp::PackMode::Zip);
        QVERIFY2(zip.ok, qPrintable(zip.error));
        QCOMPARE(zip.packed(), 3);
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(fileBytes(dir.filePath(QStringLiteral("pack.zip"))), entries));
        QVERIFY(entries.contains(QStringLiteral("linked/photo.png")));
    }
    // A document protected with an empty stored password (the one every program knows is then the lock) is packed protected still, not left open.
    void aPackedCopyOfAFileWithAnEmptyStoredPasswordStaysProtected()
    {
        QTemporaryDir dir;
        QVERIFY(QDir().mkpath(dir.filePath(QStringLiteral("work"))));
        const QString path = dir.filePath(QStringLiteral("work/plain.docx"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(linkedDocx(QStringLiteral("none.png"), QStringLiteral("none2.png"), QStringLiteral("none.docx")));
        }
        WriteWindow w;
        Quiet quiet(w);
        QVERIFY(w.openFile(path));
        jp::setNewPasswordStandIn([](bool, QString *password) {
            password->clear();
            return jp::NewPasswordChoice::Set;
        });
        w.encryptWithPassword();
        jp::setNewPasswordStandIn(nullptr);
        QVERIFY(w.isPasswordProtected());
        const jp::PackReport zip = w.packAndGoTo(dir.filePath(QStringLiteral("pack.zip")), jp::PackMode::Zip);
        QVERIFY2(zip.ok, qPrintable(zip.error));
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(fileBytes(dir.filePath(QStringLiteral("pack.zip"))), entries));
        QVERIFY(entries.contains(w.displayName() + QStringLiteral(".docx")));
        QVERIFY(entries.value(w.displayName() + QStringLiteral(".docx")).startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)));
    }
    // A picture of the Export page with the two Pack and Go cards (only with JO_SHOT_DIR set: it makes the program's style the new one for the rest of the process).
    void packAndGoPageScreenshot()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        WriteWindow w;
        w.resize(1280, 800);
        w.show();
        w.showFileScreen(QStringLiteral("export"));
        QTRY_VERIFY(w.findChild<QAbstractButton *>(QStringLiteral("packAndGoZip")) && w.findChild<QAbstractButton *>(QStringLiteral("packAndGoZip"))->isVisible());
        QVERIFY(w.grab().save(dir + QStringLiteral("/write-pack-and-go.png")));
    }
};

QTEST_MAIN(FileTests)
#include "tst_write_file.moc"
