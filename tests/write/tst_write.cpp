// Tests of JeffWrite (apps/write): the document, editing it, the .docx it reads and writes, the page view, and the window.

#include "core/fonts.h"
#include "app/pagepaint.h"
#include "app/pageview.h"
#include "app/pageoutput.h"
#include "app/spellpane.h"
#include "app/filescreen.h"
#include "app/findreplace.h"
#include "app/help.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "app/ribbon.h"
#include "app/widgets.h"
#include "app/writefindpane.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/printing.h"
#include "core/spellcheck.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "../flow/pageaudit.h"
#include "../office/dochelpers.h"
#include "../timing.h"
#include "office/package.h"
#include "rtcheck.h"
#include "app/spelling.h"
#include "text/textprops.h"
#include "io/zip.h"
#include "wml/wml.h"

#include <QAccessible>
#include <QApplication>
#include <climits>
#include <QMenu>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPrinter>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QSpinBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QClipboard>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QToolButton>
#include <functional>
#include <random>
#include <set>

using namespace jp;
using namespace jp::write;
namespace wml = jp::office::wml;
namespace audit = jp::flowaudit;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

// A package of a document whose body is `body`, with the styles and lists given (a file of its own making, not JeffWrite's).
QByteArray makeDocx(const QByteArray &body, const QByteArray &styles = QByteArray(), const QByteArray &numbering = QByteArray(), const QByteArray &sect = QByteArray())
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + (sect.isEmpty() ? QByteArray() : sect) + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    if (!styles.isEmpty()) {
        w.addPart(QStringLiteral("/word/styles.xml"), xml(QByteArray("<w:styles ") + kNs + ">" + styles + "</w:styles>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    }
    if (!numbering.isEmpty()) {
        w.addPart(QStringLiteral("/word/numbering.xml"), xml(QByteArray("<w:numbering ") + kNs + ">" + numbering + "</w:numbering>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
    }
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

QByteArray paragraphs(int n, int words = 60)
{
    static const char *const w[] = {"the", "court", "rule", "of", "and", "appeal", "section", "evidence", "judgment", "provision", "reasonable", "interpretation"};
    std::mt19937 rng(5);
    QByteArray out;
    for (int i = 0; i < n; ++i) {
        QByteArray text;
        for (int k = 0; k < words; ++k) text += (k ? " " : "") + QByteArray(w[rng() % 12]);
        out += "<w:p><w:pPr><w:spacing w:after=\"120\"/></w:pPr><w:r><w:t>" + text + ".</w:t></w:r></w:p>";
    }
    return out;
}

void typeText(WriteDoc &doc, QTextCursor &c, const QString &text)
{
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n')) insertParagraph(doc, c);
        else insertText(doc, c, QString(ch));
    }
}

QString plain(WriteDoc &doc) { return doc.story()->toPlainText(); }

// A random place in a table's cells (the cursor is not moved when the story has no table).
bool placeInACell(WriteDoc &doc, QTextCursor &c, const std::function<int(int)> &pick)
{
    QVector<QTextBlock> cells;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        if (cellKeyOf(b)) cells << b;
    if (cells.isEmpty()) return false;
    const QTextBlock b = cells[pick(int(cells.size()))];
    c.setPosition(b.position() + pick(b.length()));
    return true;
}

// The table commands, as the random editing picks them (kinds 0 to 5: add a row, a column, Tab, take a row, a column, the table).
void tableCommand(WriteDoc &doc, QTextCursor &c, int kind, const std::function<int(int)> &pick)
{
    if (!placeInACell(doc, c, pick)) return;
    switch (kind) {
    case 0: insertTableRow(doc, c, pick(2)); break;
    case 1: insertTableColumn(doc, c, pick(2)); break;
    case 2: insertTab(doc, c, pick(2)); break;
    case 3: deleteTableRows(doc, c); break;
    case 4: deleteTableColumns(doc, c); break;
    default: if (pick(3) == 0) deleteTable(doc, c); break;
    }
}

const QByteArray kStyles =
    "<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Tinos\" w:hAnsi=\"Tinos\"/><w:sz w:val=\"23\"/></w:rPr></w:rPrDefault>"
    "<w:pPrDefault><w:pPr><w:spacing w:after=\"120\" w:line=\"264\" w:lineRule=\"auto\"/></w:pPr></w:pPrDefault></w:docDefaults>"
    "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:uiPriority w:val=\"9\"/><w:qFormat/>"
    "<w:pPr><w:keepNext/><w:spacing w:before=\"360\" w:after=\"120\"/><w:outlineLvl w:val=\"0\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Arimo\" w:hAnsi=\"Arimo\"/><w:b/><w:color w:val=\"1F3864\"/><w:sz w:val=\"34\"/></w:rPr></w:style>"
    "<w:style w:type=\"character\" w:styleId=\"Strong\"><w:name w:val=\"Strong\"/><w:qFormat/><w:rPr><w:b/></w:rPr></w:style>";

// A document of plain paragraphs (one run each), in the sample styles.
std::unique_ptr<WriteDoc> textDoc(const QStringList &paragraphs, const QByteArray &styles = kStyles)
{
    QByteArray body;
    for (const QString &p : paragraphs) body += "<w:p><w:r><w:t xml:space=\"preserve\">" + p.toUtf8() + "</w:t></w:r></w:p>";
    return open(makeDocx(body, styles));
}

// The words a spelling check flags in the whole story, in order.
QStringList flaggedWords(SpellCheck &sc, WriteDoc &doc)
{
    const QString text = doc.story()->toPlainText();
    QStringList out;
    for (const auto &r : sc.misspelledIn(0, doc.story()->characterCount())) out << text.mid(r.first, r.second - r.first);
    return out;
}


// A document with every kind of thing a file keeps beside its text, for the checks that nothing is lost: paragraph ids, hyperlinks (outside and
// inside), fields, content controls (inline and block), tracked changes, bookmarks and comment ranges (one across a kept table, one with an Id
// used twice), a section that ends on a heading, page breaks, a kept table, and kept parts with their relationships.
QByteArray richDocx()
{
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                          "xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdHd\"/><w:pgSz w:w=\"12240\" w:h=\"15840\" w:code=\"1\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/><w:pgNumType w:fmt=\"decimal\" w:chapStyle=\"1\"/></w:sectPr>";
    QByteArray body;
    body += "<w:p w14:paraId=\"1A2B3C01\" w14:textId=\"77777777\"><w:pPr><w:pStyle w:val=\"Heading1\"/>" + sect + "</w:pPr><w:r><w:t>Heading that ends a section</w:t></w:r></w:p>";
    body += "<w:p w14:paraId=\"1A2B3C02\" w14:textId=\"77777777\"><w:r><w:t xml:space=\"preserve\">Before the link </w:t></w:r><w:hyperlink r:id=\"rIdLink\" w:history=\"1\"><w:r><w:t>the link text in the middle</w:t></w:r></w:hyperlink>"
            "<w:r><w:t xml:space=\"preserve\"> after it, then </w:t></w:r><w:hyperlink w:anchor=\"across\"><w:r><w:t>an anchor link</w:t></w:r></w:hyperlink><w:r><w:t>.</w:t></w:r></w:p>";
    body += "<w:p w14:paraId=\"1A2B3C03\"><w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGE </w:instrText></w:r>"
            "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:r><w:t xml:space=\"preserve\"> and </w:t></w:r>"
            "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>9</w:t></w:r></w:fldSimple><w:r><w:t xml:space=\"preserve\"> pages.</w:t></w:r></w:p>";
    body += "<w:p w14:paraId=\"1A2B3C04\"><w:r><w:t xml:space=\"preserve\">A control: </w:t></w:r><w:sdt><w:sdtPr><w:alias w:val=\"Title\"/><w:tag w:val=\"t1\"/></w:sdtPr><w:sdtContent><w:r><w:t>content control text</w:t></w:r></w:sdtContent></w:sdt>"
            "<w:r><w:t xml:space=\"preserve\"> and a tag: </w:t></w:r><w:smartTag w:uri=\"urn:place\" w:element=\"place\"><w:r><w:t>Paris</w:t></w:r></w:smartTag><w:r><w:t>.</w:t></w:r></w:p>";
    body += "<w:p w14:paraId=\"1A2B3C05\"><w:r><w:t xml:space=\"preserve\">Changes: </w:t></w:r><w:ins w:id=\"10\" w:author=\"Ann\" w:date=\"2020-01-01T00:00:00Z\"><w:r><w:t>inserted words</w:t></w:r></w:ins>"
            "<w:del w:id=\"11\" w:author=\"Ann\" w:date=\"2020-01-01T00:00:00Z\"><w:r><w:delText>deleted words</w:delText></w:r></w:del><w:r><w:t xml:space=\"preserve\"> end.</w:t></w:r></w:p>";
    body += "<w:sdt><w:sdtPr><w:alias w:val=\"Block\"/></w:sdtPr><w:sdtContent><w:p w14:paraId=\"1A2B3C06\"><w:r><w:t>Inside a block control</w:t></w:r></w:p></w:sdtContent></w:sdt>";
    // (A box set at a place that the text goes round, on its right side: what a file's own wrap says is kept as it was.)
    body += "<w:p w14:paraId=\"1A2B3C07\"><w:r><w:t>After the block control, which Backspace must not eat.</w:t></w:r><w:r><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\">"
            "<v:rect id=\"keptWall\" style=\"position:absolute;margin-left:200pt;margin-top:4pt;width:60pt;height:40pt\" fillcolor=\"red\" stroked=\"f\"><w10:wrap type=\"square\" side=\"left\"/></v:rect></w:pict></w:r></w:p>";
    body += "<w:p w14:paraId=\"1A2B3C08\"><w:bookmarkStart w:id=\"20\" w:name=\"across\"/><w:commentRangeStart w:id=\"5\"/><w:r><w:t>Starts before the table</w:t></w:r></w:p>"
            "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cell a</w:t></w:r></w:p></w:tc>"
            "<w:tc><w:p><w:r><w:t>cell b</w:t></w:r><w:bookmarkEnd w:id=\"20\"/></w:p></w:tc></w:tr></w:tbl>"
            "<w:p w14:paraId=\"1A2B3C09\"><w:r><w:t xml:space=\"preserve\">Ends after it</w:t></w:r><w:commentRangeEnd w:id=\"5\"/><w:r><w:commentReference w:id=\"5\"/></w:r></w:p>";
    body += "<w:p><w:bookmarkStart w:id=\"30\" w:name=\"first\"/><w:r><w:t>one</w:t></w:r><w:bookmarkEnd w:id=\"30\"/></w:p>"
            "<w:p><w:bookmarkStart w:id=\"30\" w:name=\"second\"/><w:r><w:t>two with the same Id</w:t></w:r><w:bookmarkEnd w:id=\"30\"/></w:p>";
    body += "<w:p w14:paraId=\"1A2B3C0A\"><w:pPr><w:pBdr><w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"1\" w:color=\"auto\"/></w:pBdr><w:shd w:val=\"clear\" w:fill=\"EEEEEE\"/></w:pPr>"
            "<w:r><w:rPr><w:u w:val=\"single\" w:color=\"FF0000\" w:themeColor=\"accent2\"/><w:shd w:val=\"clear\" w:fill=\"FFFF00\"/></w:rPr><w:t>Underlined in a theme color, with a border</w:t></w:r></w:p>";
    body += "<w:p w14:paraId=\"1A2B3C0B\"><w:r><w:t xml:space=\"preserve\">Before the break </w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after the break</w:t></w:r></w:p>";
    body += "<w:p w14:paraId=\"1A2B3C0C\"><w:r><w:t>Ends with a break</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r></w:p>";
    body += paragraphs(4, 30);
    const QByteArray header = "<w:hdr " + ns + "><w:p><w:r><w:t>A header</w:t></w:r></w:p></w:hdr>";
    const QByteArray comments = "<w:comments " + ns + "><w:comment w:id=\"5\" w:author=\"Ann\" w:date=\"2020-01-01T00:00:00Z\"><w:p><w:r><w:t>A comment</w:t></w:r></w:p></w:comment></w:comments>";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body>" + body + sect + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), xml("<w:styles " + ns + ">" + kStyles + "</w:styles>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addPart(QStringLiteral("/word/header1.xml"), xml(header), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"));
    w.addPart(QStringLiteral("/word/comments.xml"), xml(comments), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.comments+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdSt"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdHd"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdCm"), rel + QStringLiteral("comments"), QStringLiteral("/word/comments.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdLink"), rel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/a?b=1&c=2"), true);
    return w.finish();
}


// A document with headers and footers: `parts` are word/header1.xml and the like (by their part name), each related to the document by the Id
// its name gives ("rId" and the file's name without its extension), as `sect` (a section's properties, with the references) names them.
QByteArray makeDocxWithParts(const QByteArray &body, const QByteArray &sect, const QMap<QString, QByteArray> &parts, const QByteArray &settings = QByteArray(), const QByteArray &styles = kStyles,
                             const QByteArray &numbering = QByteArray())
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + sect + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), xml(QByteArray("<w:styles ") + kNs + ">" + styles + "</w:styles>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    if (!settings.isEmpty()) {
        w.addPart(QStringLiteral("/word/settings.xml"), xml(QByteArray("<w:settings ") + kNs + ">" + settings + "</w:settings>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    }
    if (!numbering.isEmpty()) {
        w.addPart(QStringLiteral("/word/numbering.xml"), xml(QByteArray("<w:numbering ") + kNs + ">" + numbering + "</w:numbering>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
    }
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        const QString id = QStringLiteral("rId") + QFileInfo(it.key()).completeBaseName();
        const QString kind = it.key().contains(QLatin1String("footnotes")) ? QStringLiteral("footnotes") : it.key().contains(QLatin1String("fontTable")) ? QStringLiteral("fontTable")
                             : it.key().contains(QLatin1String("footer")) ? QStringLiteral("footer") : QStringLiteral("header");
        w.addPart(it.key(), xml(it.value()), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.") + kind + "+xml");
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), id, rel + kind, it.key());
    }
    return w.finish();
}

// A header or footer part of the paragraphs given (each the XML of a paragraph's contents).
QByteArray headerPart(const QByteArray &root, const QList<QByteArray> &paragraphs)
{
    QByteArray out = "<w:" + root + " " + kNs + ">";
    for (const QByteArray &p : paragraphs) out += "<w:p>" + p + "</w:p>";
    return out + "</w:" + root + ">";
}

// The tables of the document of tables for the random commands, its styles, and its section's properties.
struct RichTables {
    QByteArray styled, plain, styles, sect, spaced;
};

// A text box of a materialized page by what its story says, for the header's and the footer's.
const TextItem *boxWithText(const flow::MaterializedPage &page, const QString &text)
{
    for (const auto &item : page.doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (const Story *s = page.doc->story(t->storyId))
                if (s->doc->toPlainText().contains(text)) return t;
    return nullptr;
}

// The red wave pixels in a part of a picture, and where they are: the rectangle is in the widget's own units (what caretRect and pageRect give), the picture
// is what grab() made of it, at whatever ratio the screen has. The wave is a thin red line, so any pixel with much red and little else counts.
QRect redBounds(const QImage &img, const QRect &within, int *count = nullptr)
{
    const double r = img.devicePixelRatio();
    const int x0 = std::max(0, int(std::floor(within.left() * r))), y0 = std::max(0, int(std::floor(within.top() * r)));
    const int x1 = std::min(img.width() - 1, int(std::ceil((within.right() + 1) * r)) - 1), y1 = std::min(img.height() - 1, int(std::ceil((within.bottom() + 1) * r)) - 1);
    int left = INT_MAX, top = INT_MAX, right = -1, bottom = -1, n = 0;
    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x) {
            const QColor c = img.pixelColor(x, y);
            if (c.red() > 150 && c.green() < 110 && c.blue() < 110) {
                ++n;
                left = std::min(left, x);
                top = std::min(top, y);
                right = std::max(right, x);
                bottom = std::max(bottom, y);
            }
        }
    if (count) *count = n;
    if (n == 0) return QRect();
    return QRect(QPoint(int(std::floor(left / r)), int(std::floor(top / r))), QPoint(int(std::ceil((right + 1) / r)) - 1, int(std::ceil((bottom + 1) / r)) - 1));
}

// A setting the test needs to have a value (and the program's own value back after): the person's real settings are not the test's to leave changed.
struct SettingGuard {
    QString key;
    QVariant before;
    SettingGuard(const QString &k, const QVariant &value, const QVariant &normally) : key(k), before(Settings::get().value(k, normally))
    {
        Settings::get().setValue(key, value);
    }
    ~SettingGuard() { Settings::get().setValue(key, before); }
};

// Takes the observer of the page painter away however the test ends.
struct ObserverGuard {
    ~ObserverGuard() { write::setPaintObserver(nullptr); }
};


// Selects the characters [from, to) of the window's document.
void selectRange(WriteWindow &w, int from, int to)
{
    QTextCursor c(w.document()->story());
    c.setPosition(from);
    c.setPosition(to, QTextCursor::KeepAnchor);
    w.view()->setCursor(c);
}

// Runs `open`, which makes a modal dialog, and meanwhile runs `fill` on that dialog, then presses OK (Cancel when `accept` is false). `fill` may use QVERIFY and
// QCOMPARE: a failed one leaves `fill`, not the dialog. The dialog is always closed from outside `fill`, and a watchdog closes whatever is still open after ten
// seconds and fails the test, so that a failure ends the test and never leaves it waiting on a dialog (on CI that costs the whole job).
void withDialog(const std::function<void()> &open, const QString &objectName, const std::function<void(QDialog *)> &fill, bool accept = true)
{
    QString opened;
    bool leftOpen = false;
    auto closeAll = [](bool ok) {
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *d = qobject_cast<QDialog *>(top); d && d->isVisible()) {
                if (ok) d->accept();
                else d->reject();
            }
    };
    QTimer once, watchdog;
    once.setSingleShot(true);
    watchdog.setSingleShot(true);
    QObject::connect(&once, &QTimer::timeout, [&] {
        auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (dlg) opened = dlg->objectName();
        if (dlg && opened == objectName) fill(dlg);
        closeAll(accept && opened == objectName);
    });
    QObject::connect(&watchdog, &QTimer::timeout, [&] {
        leftOpen = true;
        closeAll(false);
    });
    once.start(0);
    watchdog.start(10000);
    open();
    QCOMPARE(opened, objectName);
    QVERIFY2(!leftOpen, "the dialog was left open");
}

// The Font dialog, opened by its command on the window's selection.
void withFontDialog(WriteWindow &w, const std::function<void(QDialog *)> &fill, bool accept = true)
{
    withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), fill, accept);
}

// The properties of the run, in a saved document's XML, that holds exactly this text (null when there is no such run or it has none).
QString runPropsOfText(const QByteArray &documentXml, const QString &text)
{
    const QRegularExpression run(QStringLiteral("<w:r>(?:<w:rPr>((?:(?!</w:rPr>).)*)</w:rPr>)?<w:t(?: xml:space=\"preserve\")?>") + QRegularExpression::escape(text) + QStringLiteral("</w:t>"));
    return run.match(QString::fromUtf8(documentXml)).captured(1);
}
} // namespace

class WriteTests : public QObject {
    Q_OBJECT
public:
    // Letters drawn in shades of gray, whatever the machine's font settings say. Some Linux machines (CI's among them) draw letters with colored edges
    // (subpixel smoothing), and the spelling tests look for the red of the wave: a black letter's reddish left edge would count. Qt has no switch for it, so
    // fontconfig gets a settings file that reads the machine's own settings and then turns the colored edges off. QTest calls this before the application
    // starts, and fontconfig reads its settings once, when the first font is asked for.
    static void initMain()
    {
#ifdef Q_OS_LINUX
        static QTemporaryDir dir;
        QFile conf(dir.filePath(QStringLiteral("fonts.conf")));
        if (!dir.isValid() || !conf.open(QIODevice::WriteOnly)) return;
        const QString own = qEnvironmentVariableIsSet("FONTCONFIG_FILE") ? qEnvironmentVariable("FONTCONFIG_FILE") : QStringLiteral("/etc/fonts/fonts.conf");
        conf.write("<?xml version=\"1.0\"?>\n<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n<fontconfig>\n  <include ignore_missing=\"yes\">" + own.toHtmlEscaped().toUtf8()
                   + "</include>\n  <match target=\"font\"><edit name=\"rgba\" mode=\"assign\"><const>none</const></edit></match>\n</fontconfig>\n");
        conf.close();
        qputenv("FONTCONFIG_FILE", QFile::encodeName(conf.fileName()));
#endif
    }

private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();   // the program's own fonts, as JeffWrite loads them (a test machine may have none: Qt on Windows ships no fonts)
    }

    // ---------------------------------------------------------------- the app
    void theWindowHasItsTabsAndBuildsCleanly()
    {
        WriteWindow w;
        QCOMPARE(w.ribbonErrors(), QString());
        QStringList tabs;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
        QCOMPARE(tabs, (QStringList{"Home", "Insert", "Layout", "Review", "View"}));
        QCOMPARE(w.ribbon()->fileKeytip(), QStringLiteral("F"));
        QVERIFY(w.windowTitle().contains(QLatin1String("JeffWrite")));
        QVERIFY(w.windowTitle().startsWith(QLatin1String("Document")));
        // The commands a person looks for.
        for (const char *id : {"file.new", "file.open", "file.save", "file.saveAs", "file.print", "file.exportPdf", "file.options", "edit.undo", "edit.redo", "fmt.bold", "para.bullets", "insert.pageBreak",
                               "layout.landscape", "zoom.pageWidth", "help.show", "review.spelling", "review.checkAsType"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
    }

    void itsIdentityIsItsOwn()
    {
        QCOMPARE(appInfo().name, QStringLiteral("JeffWrite"));
        QCOMPARE(appInfo().accent, QColor(0x1D, 0x55, 0xC4));
        QCOMPARE(appInfo().badge, QStringLiteral("JW"));
        QCOMPARE(appInfo().helpFolder, QStringLiteral("write"));
    }

    void ribbonStringsListed()
    {
        // The list that translators start from is made from the ribbon's file: run tools/ribbon_strings.py after changing its text.
        QFile json(QStringLiteral(JW_SOURCE_DIR "/apps/write/resources/ribbon.json")), listed(QStringLiteral(JW_SOURCE_DIR "/apps/write/app/ribbon_strings.cpp"));
        QVERIFY(json.open(QIODevice::ReadOnly) && listed.open(QIODevice::ReadOnly));
        QSet<QString> want;
        std::function<void(const QJsonValue &)> walk = [&](const QJsonValue &v) {
            if (v.isObject()) {
                const QJsonObject o = v.toObject();
                for (auto it = o.begin(); it != o.end(); ++it) {
                    if (it.value().isString() && QSet<QString>{"name", "text", "launcherTip", "tip", "label", "section", "submenu", "context"}.contains(it.key()) && !it.value().toString().isEmpty())
                        want.insert(it.value().toString());
                    walk(it.value());
                }
            } else if (v.isArray()) {
                for (const QJsonValue &x : v.toArray()) walk(x);
            }
        };
        walk(QJsonDocument::fromJson(json.readAll()).object());
        QSet<QString> have;
        static const QRegularExpression re(QStringLiteral("QT_TRANSLATE_NOOP\\(\"Ribbon\", \"([^\"]*)\"\\)"));
        auto it = re.globalMatch(QString::fromUtf8(listed.readAll()));
        while (it.hasNext()) have.insert(it.next().captured(1));
        QCOMPARE(have, want);
    }

    void theFileScreenHasItsPages()
    {
        WriteWindow w;
        w.show();
        for (const char *page : {"new", "open", "print", "export", "about"}) {
            w.showFileScreen(QString::fromLatin1(page));
            auto *screen = w.findChild<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
            Q_UNUSED(screen);
            QVERIFY(w.findChild<FileScreen *>());
            QCOMPARE(w.findChild<FileScreen *>()->currentPage(), QString::fromLatin1(page));
        }
        w.hideFileScreen();
    }

    void saveAndOpenKeepTheText()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Dear reader,\nThis is the letter."));
        QVERIFY(a.isModified());
        a.setSavePathChooser([&](const QString &suggested, const QString &) { return dir.filePath(QFileInfo(suggested).fileName()); });
        QVERIFY(a.saveAs());
        QVERIFY(!a.isModified());
        QVERIFY(a.filePath().endsWith(QLatin1String(".docx")));
        QVERIFY(QFileInfo(a.filePath()).size() > 1000);
        WriteWindow b;
        QVERIFY(b.openFile(a.filePath()));
        QCOMPARE(b.document()->story()->toPlainText(), QStringLiteral("Dear reader,\nThis is the letter."));
        QCOMPARE(b.filePath(), a.filePath());
        QVERIFY(!b.isModified());
    }

    // ---- an old binary file

    void anOldBinaryFileOpensAndShowsItsText()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        dochelp::DocWriter doc;
        doc.text = QStringLiteral("Dear reader,\rThis is the old letter.\r");
        doc.chars = {{13, 17, dochelp::sprm1(dochelp::sprm::bold, 1)}};
        const QString path = dir.filePath(QStringLiteral("letter.doc"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(doc.build());
        f.close();
        WriteWindow w;
        QStringList notices;
        w.setNotifier([&](const QString &, const QString &text) { notices << text; });
        QVERIFY(w.openFile(path));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Dear reader,\nThis is the old letter."));
        QVERIFY(!w.isModified());
        QVERIFY(notices.isEmpty());
        // The title says it is a .doc until it is saved as something else.
        QVERIFY2(w.windowTitle().startsWith(QLatin1String("letter.doc")), qPrintable(w.windowTitle()));
        // (And the layout made pages of it.)
        QVERIFY(w.document()->pager().pageCount() >= 1);
    }

    void savingAnOldBinaryFileAsksForADocxName()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        dochelp::DocWriter doc;
        doc.text = QStringLiteral("Old text\r");
        const QString path = dir.filePath(QStringLiteral("old.doc"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(doc.build());
        f.close();
        WriteWindow w;
        QStringList notices;
        QString suggested;
        w.setNotifier([&](const QString &, const QString &text) { notices << text; });
        w.setSavePathChooser([&](const QString &name, const QString &) {
            suggested = name;
            return dir.filePath(QStringLiteral("new.docx"));
        });
        QVERIFY(w.openFile(path));
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("!"));
        QVERIFY(w.save());
        // One sentence says why the name is asked for, and the name offered is a .docx beside the old file.
        QCOMPARE(notices.size(), 1);
        QVERIFY2(notices[0].contains(QLatin1String(".docx")) && notices[0].contains(QLatin1String(".doc ")), qPrintable(notices[0]));
        QVERIFY2(suggested.endsWith(QLatin1String("old.docx")), qPrintable(suggested));
        QVERIFY(w.filePath().endsWith(QLatin1String("new.docx")));
        QVERIFY(!w.windowTitle().contains(QLatin1String(".doc")));
        // The old file is as it was, and the new one opens with the edit.
        QFile again(path);
        QVERIFY(again.open(QIODevice::ReadOnly));
        QCOMPARE(again.readAll(), doc.build());
        WriteWindow b;
        QVERIFY(b.openFile(w.filePath()));
        QCOMPARE(b.document()->story()->toPlainText().left(9), QStringLiteral("!Old text"));
    }

    void whatAnOldFileHoldsThatCannotBeShownIsSaid()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        dochelp::DocWriter doc;
        doc.text = QStringLiteral("a\x01""b\r");
        doc.chars = {{1, 2, dochelp::sprm1(dochelp::sprm::special, 1) + dochelp::sprm4(dochelp::sprm::picture, 9000)}};   // (a picture the file doesn't hold)
        const QString path = dir.filePath(QStringLiteral("odd.doc"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(doc.build());
        f.close();
        WriteWindow w;
        QStringList notices;
        w.setNotifier([&](const QString &, const QString &text) { notices << text; });
        QVERIFY(w.openFile(path));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("ab"));
        QCOMPARE(notices.size(), 1);
        QVERIFY2(notices[0].contains(QLatin1String("left out")), qPrintable(notices[0]));
        QVERIFY(w.document()->leftSomethingOut());
    }

    void trackedChangesOfAnOldFileComeInWithTheirAuthorsAndDates()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        dochelp::DocWriter doc;
        // "keep ", then "added" inserted by Ann, " and ", then "dropped" deleted by Bob, then " end."
        doc.text = QStringLiteral("keep added and dropped end.\r");
        doc.revisionAuthors = {QStringLiteral("Ann"), QStringLiteral("Bob")};
        using namespace dochelp;
        doc.chars = {{5, 10, sprm1(sprm::inserted, 0x81) + sprm2(sprm::insertedBy, 0) + sprm4(sprm::insertedAt, dttm(2003, 4, 7, 9, 30))},
                     {15, 22, sprm1(sprm::deleted, 0x81) + sprm2(sprm::deletedBy, 1) + sprm4(sprm::deletedAt, dttm(2004, 11, 23, 16, 5))}};
        const QString path = dir.filePath(QStringLiteral("tracked.doc"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(doc.build());
        f.close();
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        QVERIFY(w.openFile(path));
        // The inserted words are in the text, and the deleted ones are not shown in it (the marks of the changes are objects in it, not text).
        QString shown = w.document()->story()->toPlainText();
        shown.remove(QChar(0xFFFC));
        QVERIFY2(shown.contains(QLatin1String("keep added and")), qPrintable(shown));
        QVERIFY2(!shown.contains(QLatin1String("dropped")), qPrintable(shown));
        // Written out, each is still a tracked change, with its author and its date.
        const QByteArray x = documentXml(w.document()->toDocx());
        const qsizetype ins = x.indexOf("<w:ins ");
        const qsizetype del = x.indexOf("<w:del ");
        QVERIFY2(ins >= 0 && del > ins, x.constData());
        const QByteArray insTag = x.mid(ins, x.indexOf('>', ins) - ins), delTag = x.mid(del, x.indexOf('>', del) - del);
        QVERIFY2(insTag.contains("w:author=\"Ann\"") && insTag.contains("w:date=\"2003-04-07T09:30:00Z\""), insTag.constData());
        QVERIFY2(delTag.contains("w:author=\"Bob\"") && delTag.contains("w:date=\"2004-11-23T16:05:00Z\""), delTag.constData());
        QVERIFY(x.mid(ins, del - ins).contains("<w:t xml:space=\"preserve\">added</w:t>"));
        QVERIFY(x.mid(del, x.indexOf("</w:del>", del) - del).contains("<w:delText xml:space=\"preserve\">dropped</w:delText>"));
    }

    void anOldTemplateOpensAsANewDocumentMadeFromIt()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        dochelp::DocWriter doc;
        doc.text = QStringLiteral("Dear <name>,\r");
        const QString path = dir.filePath(QStringLiteral("letter.dot"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(doc.build());
        f.close();
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        QVERIFY(w.openFile(path));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Dear <name>,"));
        QVERIFY(w.filePath().isEmpty());   // (it is not the template that is changed and saved)
        QVERIFY(w.isModified());
    }

    void aFileThatIsNotADocumentSaysSoAndOpensNothing()
    {
        QString why;
        QVERIFY(!WriteDoc::fromFile(QByteArray("plain bytes, no container"), &why));
        QVERIFY(!why.isEmpty());
        // Both kinds are told apart by what is in the file, not its name.
        dochelp::DocWriter old;
        old.text = QStringLiteral("old kind\r");
        QVERIFY(WriteDoc::fromFile(old.build()));
        QVERIFY(WriteDoc::fromFile(WriteDoc::blank()->toDocx()));
    }

    void unsavedWorkIsKeptAndComesBack()
    {
        QTemporaryDir dir;
        recovery::setRoot(dir.path());
        recovery::setCopySuffix(QStringLiteral(".docx"));
        WriteWindow a;
        QVERIFY(a.recoveryCopy().isEmpty());
        a.autoRecover();   // (nothing is changed yet: no copy)
        QVERIFY(a.recoveryCopy().isEmpty());
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Work that is not saved"));
        a.autoRecover();
        QVERIFY(!a.recoveryCopy().isEmpty());
        QVERIFY(QFileInfo::exists(a.recoveryCopy()));
        const QString copy = a.recoveryCopy();
        // A run that crashed leaves its copy: it opens as unsaved work, and the old copy goes once this run has one of its own.
        recovery::Recovered r;
        r.file = copy;
        r.title = QStringLiteral("Lost letter");
        r.saved = QDateTime::currentDateTime();
        WriteWindow b;
        WriteWindow *opened = b.openRecovered(r);
        QVERIFY(opened);
        QCOMPARE(opened->document()->story()->toPlainText(), QStringLiteral("Work that is not saved"));
        QVERIFY(opened->isModified());
        QCOMPARE(opened->displayName(), QStringLiteral("Lost letter"));
        QVERIFY(!opened->recoveryCopy().isEmpty());
        // Saving takes the copy away.
        opened->setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("letter.docx")); });
        const QString second = opened->recoveryCopy();
        QVERIFY(opened->save());
        QVERIFY(!QFileInfo::exists(second));
        QVERIFY(opened->recoveryCopy().isEmpty());
    }

    void theParagraphDialogChangesWhatWasChanged()
    {
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("one\ntwo"));
        w.view()->selectAll();
        withDialog([&w] { w.act("para.dialog")->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *dlg) {
            auto *alignment = dlg->findChild<QComboBox *>(QStringLiteral("alignment"));
            auto *left = dlg->findChild<QDoubleSpinBox *>(QStringLiteral("left"));
            auto *lineSpacing = dlg->findChild<QComboBox *>(QStringLiteral("lineSpacing"));
            QVERIFY(alignment && left && lineSpacing);
            alignment->setCurrentIndex(1);
            left->setValue(36);
            lineSpacing->setCurrentIndex(2);
        });
        for (int i = 0; i < 2; ++i) {
            const QTextBlockFormat bf = w.document()->story()->findBlockByNumber(i).blockFormat();
            QCOMPARE(bf.alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
            QCOMPARE(bf.leftMargin(), 36.0);
            QCOMPARE(bf.lineHeight(), 200.0);
            QCOMPARE(bf.rightMargin(), 0.0);   // (not changed: left as it was)
        }
        withDialog([&w] { w.act("zoom.dialog")->trigger(); }, QStringLiteral("zoomDialog"), [](QDialog *dlg) {
            auto *percent = dlg->findChild<QSpinBox *>(QStringLiteral("percent"));
            QVERIFY(percent);
            percent->setValue(150);
            for (auto *r : dlg->findChildren<QRadioButton *>())
                if (r->property("zoomChoice").toInt() == 0) r->setChecked(true);
        });
        QCOMPARE(w.view()->zoom(), 1.5);
    }

    // ---------------------------------------------------------------- Find and Replace
    // Home > Editing has Find (Ctrl+F) and Replace (Ctrl+H); each opens the pane on the right.
    void homeTabHasFindAndReplace()
    {
        WriteWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QCOMPARE(w.act(QStringLiteral("edit.find"))->shortcut(), QKeySequence(QKeySequence::Find));
        QCOMPARE(w.act(QStringLiteral("edit.replace"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_H));
        int buttons = 0;
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->defaultAction() == w.act(QStringLiteral("edit.find")) || b->defaultAction() == w.act(QStringLiteral("edit.replace"))) ++buttons;
        QCOMPARE(buttons, 2);
        QVERIFY(!w.findPane());
        w.show();
        w.act(QStringLiteral("edit.find"))->trigger();
        QVERIFY(w.findPane() && w.findPane()->isVisible());
        QVERIFY(!w.findPane()->matchCase()->isChecked() && !w.findPane()->wholeWords()->isChecked());
        w.act(QStringLiteral("edit.replace"))->trigger();
        QVERIFY(w.findPane()->isVisible());
        // The boxes are named for screen readers.
        for (QLineEdit *e : w.findPane()->findChildren<QLineEdit *>()) {
            QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(e);
            QVERIFY(iface);
            QVERIFY2(!iface->text(QAccessible::Name).trimmed().isEmpty(), qPrintable(e->objectName()));
        }
    }

    // Find Next selects the match and scrolls the pages to it, also when the page it is on has not been laid out yet; after the last match it
    // goes round to the first; and with none it says so and leaves the selection where it was.
    void findNextSelectsTheMatchAndScrollsToIt()
    {
        WriteWindow w;
        w.resize(1100, 800);
        w.setDocument(open(makeDocx(paragraphs(300) + "<w:p><w:r><w:t>The zebrafish swims.</w:t></w:r></w:p>" + paragraphs(40), kStyles)), QString());
        w.show();
        w.showFindPane();
        WriteFindPane *pane = w.findPane();
        QCOMPARE(w.view()->verticalScrollBar()->value(), 0);
        // The match is past the pages laid out so far (so that finding it has to lay out the pages as far as it).
        const int matchAt = w.document()->story()->toPlainText().indexOf(QStringLiteral("zebrafish"));
        const int known = w.view()->pageCount();
        QVERIFY(matchAt > 0 && known >= 1);
        QVERIFY(!w.view()->pagesComplete());
        QVERIFY2(w.document()->pager().page(known - 1).start.pos <= matchAt, qPrintable(QStringLiteral("%1 pages known, the last from %2, the match at %3").arg(known).arg(w.document()->pager().page(known - 1).start.pos).arg(matchAt)));
        pane->findBox()->setText(QStringLiteral("zebrafish"));
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("zebrafish"));
        QVERIFY(pane->status().isEmpty());
        QVERIFY(w.view()->pageCount() > known);
        QVERIFY(w.view()->currentPage() > 5);
        QVERIFY(w.view()->verticalScrollBar()->value() > 0);
        QRect r;
        QVERIFY(w.view()->caretRect(&r));
        QVERIFY2(r.top() >= 0 && r.bottom() <= w.view()->viewport()->height(), qPrintable(QStringLiteral("%1..%2 in %3").arg(r.top()).arg(r.bottom()).arg(w.view()->viewport()->height())));
        // The only match, again: round to it.
        const int at = w.view()->cursor().selectionStart();
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectionStart(), at);
        // From the start of the document, the first match is the nearest.
        QTextCursor c(w.document()->story());
        w.view()->setCursor(c);
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectionStart(), at);
        // Nothing found: said, and nothing moves.
        pane->findBox()->setText(QStringLiteral("<b>narwhal"));
        QVERIFY(!pane->findNext());
        QVERIFY2(pane->status().contains(QStringLiteral("\"<b>narwhal\" was not found")), qPrintable(pane->status()));
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("zebrafish"));
        QCOMPARE(w.view()->cursor().selectionStart(), at);
    }

    void findFollowsMatchCaseAndWholeWords()
    {
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Cat cat concat CAT."));
        w.showFindPane();
        WriteFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("cat"));
        // The places the matches start, from the start of the document, round to the first again.
        auto starts = [&] {
            QTextCursor c(w.document()->story());
            w.view()->setCursor(c);
            QList<int> out;
            for (int i = 0; i < 8; ++i) {
                if (!pane->findNext()) break;
                const int at = w.view()->cursor().selectionStart();
                if (!out.isEmpty() && at == out.first()) break;
                out << at;
            }
            return out;
        };
        QCOMPARE(starts(), (QList<int>{0, 4, 11, 15}));
        pane->matchCase()->setChecked(true);
        QCOMPARE(starts(), (QList<int>{4, 11}));
        pane->wholeWords()->setChecked(true);
        QCOMPARE(starts(), (QList<int>{4}));
        pane->matchCase()->setChecked(false);
        QCOMPARE(starts(), (QList<int>{0, 4, 15}));
    }

    // The text of a table's cells is in the story, so it is found there; text that is hidden is not shown, and so it is not found.
    void findReachesCellsAndPassesHiddenText()
    {
        WriteWindow w;
        w.setDocument(open(tableDocx(2, 2)), QString());
        w.show();
        w.showFindPane();
        WriteFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("r1c1"));
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("r1c1"));
        QCOMPARE(w.view()->cursor().block().text(), QStringLiteral("r1c1"));
        // "hide" is hidden in its first place.
        w.setDocument(WriteDoc::blank(), QString());
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("hide and seek, hide"));
        selectRange(w, 0, 4);
        changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) { f.setProperty(tp::HiddenText, true); });
        QTextCursor c(w.document()->story());
        w.view()->setCursor(c);
        pane->findBox()->setText(QStringLiteral("hide"));
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectionStart(), 15);
        pane->replaceBox()->setText(QStringLiteral("show"));
        pane->replaceAll();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("hide and seek, show"));
        QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 1 replacement."));
    }

    // Replace swaps the match that is selected and goes on to the next; when what is selected is not a match, it shows the next match first.
    void replaceReplacesOneMatchAndGoesToTheNext()
    {
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("cat cat cat"));
        QTextCursor c(w.document()->story());
        w.view()->setCursor(c);
        w.showFindPane(true);
        WriteFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("cat"));
        pane->replaceBox()->setText(QStringLiteral("dog"));
        pane->replaceOne();   // (nothing is selected: it shows the first)
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("cat cat cat"));
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("cat"));
        QCOMPARE(w.view()->cursor().selectionStart(), 0);
        pane->replaceOne();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("dog cat cat"));
        QCOMPARE(w.view()->cursor().selectionStart(), 4);
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("cat"));
        pane->replaceOne();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("dog dog cat"));
        // Each swap undoes on its own.
        w.view()->undo();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("dog cat cat"));
        w.view()->undo();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("cat cat cat"));
        // A selection that is not a match is left alone: the next match is shown.
        selectRange(w, 4, 7);
        pane->findBox()->setText(QStringLiteral("t c"));
        pane->replaceOne();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("cat cat cat"));
        QCOMPARE(w.view()->cursor().selectionStart(), 2);
        // Words that the options do not match are not swapped either.
        pane->findBox()->setText(QStringLiteral("at"));
        pane->wholeWords()->setChecked(true);
        selectRange(w, 1, 3);
        pane->replaceOne();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("cat cat cat"));
        QVERIFY2(!pane->status().isEmpty(), "none of the words is \"at\"");
    }

    // Replace All is one step that undoes, says how many it swapped, and leaves what the new words hold alone; the new words look as the old ones did.
    void replaceAllIsOneStepThatUndoesAndSaysHowMany()
    {
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("a cat sat; the cat's hat, a cat."));
        selectRange(w, 15, 18);
        toggleBold(*w.document(), w.view()->cursor());   // the second "cat" is bold
        w.document()->story()->clearUndoRedoStacks();
        w.showFindPane(true);
        WriteFindPane *pane = w.findPane();
        const QString before = w.document()->story()->toPlainText();
        pane->findBox()->setText(QStringLiteral("cat"));
        pane->replaceBox()->setText(QStringLiteral("dog"));
        pane->replaceAll();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("a dog sat; the dog's hat, a dog."));
        QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 3 replacements."));
        QTextCursor probe(w.document()->story());
        probe.setPosition(17);
        QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Bold));
        probe.setPosition(5);
        QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Normal));
        // One step undoes all three, and one redoes them.
        w.view()->undo();
        QCOMPARE(w.document()->story()->toPlainText(), before);
        w.view()->redo();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("a dog sat; the dog's hat, a dog."));
        w.view()->undo();
        QVERIFY2(!w.document()->story()->isUndoAvailable(), "the three were one step");
        // A match of several looks gives its first one to the new words: "AB" is bold at the A and plain at the B.
        w.setDocument(WriteDoc::blank(), QString());
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("x AB y"));
        selectRange(w, 2, 3);
        toggleBold(*w.document(), w.view()->cursor());
        pane->findBox()->setText(QStringLiteral("ab"));
        pane->replaceBox()->setText(QStringLiteral("zz"));
        pane->replaceAll();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("x zz y"));
        probe = QTextCursor(w.document()->story());
        probe.setPosition(3);
        QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Bold));
        probe.setPosition(4);
        QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Bold));
        probe.setPosition(5);
        QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Normal));
        // The words that were put in are not searched again: "a" for "aa" ends; and nothing as the new words deletes.
        w.setDocument(WriteDoc::blank(), QString());
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("banana"));
        pane->findBox()->setText(QStringLiteral("a"));
        pane->replaceBox()->setText(QStringLiteral("aa"));
        pane->replaceAll();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("baanaanaa"));
        pane->findBox()->setText(QStringLiteral("aa"));
        pane->replaceBox()->clear();
        pane->replaceAll();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("bnn"));
        // None found: it says so, and the document is not touched.
        w.document()->story()->setModified(false);
        pane->findBox()->setText(QStringLiteral("zzz"));
        pane->replaceAll();
        QVERIFY2(pane->status().contains(QStringLiteral("made no replacements. \"zzz\" was not found")), qPrintable(pane->status()));
        QVERIFY(!w.document()->isModified());
        pane->findBox()->setText(QStringLiteral("bnn"));
        pane->replaceBox()->setText(QStringLiteral("x"));
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 1 replacement."));
    }

    // ---------------------------------------------------------------- the Font dialog
    // The Font group's launcher and Ctrl+D open the dialog.
    void theFontLauncherAndTheKeyOpenTheDialog()
    {
        WriteWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QCOMPARE(w.act(QStringLiteral("fmt.fontDialog"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_D));
        QToolButton *launcher = nullptr;
        for (RibbonGroup *g : w.ribbon()->tab(QStringLiteral("Home"))->groups())
            if (g->title() == QLatin1String("Font")) launcher = g->launcher();
        QVERIFY(launcher);
        QVERIFY(!keytip(launcher).isEmpty());
        QString title;
        withDialog([launcher] { launcher->click(); }, QStringLiteral("fontDialog"), [&title](QDialog *d) { title = d->windowTitle(); }, false);
        QCOMPARE(title, QStringLiteral("Font"));
    }

    // The dialog shows the look of the selection: the first characters' where they differ, and a box for an effect that only some of the
    // characters have is neither checked nor clear. Cancel changes nothing.
    void theFontDialogShowsTheLookOfTheSelection()
    {
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Some words here"));
        selectRange(w, 5, 10);
        toggleBold(*w.document(), w.view()->cursor());
        toggleItalic(*w.document(), w.view()->cursor());
        setFontSize(*w.document(), w.view()->cursor(), 20);
        setTextColor(*w.document(), w.view()->cursor(), QColor(0xC0, 0, 0));
        changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) {
            f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            f.setProperty(tp::UnderlineKind, 1);   // double
            f.setUnderlineColor(QColor(0, 0, 0xFF));
            f.setFontStrikeOut(true);
            f.setProperty(prop::DoubleStrike, true);
            f.setVerticalAlignment(QTextCharFormat::AlignSubScript);
            f.setFontCapitalization(QFont::SmallCaps);
            f.setProperty(tp::HiddenText, true);
        });
        w.document()->story()->clearUndoRedoStacks();
        const QString was = w.document()->story()->toHtml();
        bool shown = false;
        withFontDialog(w, [&](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("fontStyle"))->currentIndex(), 3);   // bold italic
            QCOMPARE(d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->value(), 20.0);
            QCOMPARE(d->findChild<ColorButton *>(QStringLiteral("fontColor"))->current().rgbValue(), QColor(0xC0, 0, 0));
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->currentText(), QStringLiteral("Double"));
            QCOMPARE(d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->current().rgbValue(), QColor(0, 0, 0xFF));
            for (const char *on : {"doubleStrikethrough", "subscript", "smallCaps", "hidden"}) QCOMPARE(d->findChild<QCheckBox *>(QString::fromLatin1(on))->checkState(), Qt::Checked);
            for (const char *off : {"strikethrough", "superscript", "allCaps"}) QCOMPARE(d->findChild<QCheckBox *>(QString::fromLatin1(off))->checkState(), Qt::Unchecked);
            // (Double strikethrough and single strikethrough are not both on: choosing one clears the other.)
            d->findChild<QCheckBox *>(QStringLiteral("strikethrough"))->setChecked(true);
            QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("doubleStrikethrough"))->checkState(), Qt::Unchecked);
            shown = true;
        }, false);
        QVERIFY(shown);
        QCOMPARE(w.document()->story()->toHtml(), was);
        QVERIFY(!w.document()->story()->isUndoAvailable());
        // Over "Some words": half of it has each look, and what differs shows as nothing (the effects as partly checked).
        selectRange(w, 0, 10);
        shown = false;
        withFontDialog(w, [&](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("fontStyle"))->currentIndex(), -1);
            QVERIFY(d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->text().trimmed().isEmpty());
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->currentIndex(), -1);
            QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("subscript"))->checkState(), Qt::PartiallyChecked);
            QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("hidden"))->checkState(), Qt::PartiallyChecked);
            QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("superscript"))->checkState(), Qt::Unchecked);
            shown = true;
        }, false);
        QVERIFY(shown);
    }

    // OK gives the selection what was changed, as one step that undoes; what was not touched stays as each character had it.
    void theFontDialogAppliesWhatWasChangedAsOneStep()
    {
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Some words here"));
        selectRange(w, 5, 10);
        toggleBold(*w.document(), w.view()->cursor());
        w.document()->story()->clearUndoRedoStacks();
        selectRange(w, 0, 10);   // (the bold "words" with the plain "Some ")
        withFontDialog(w, [](QDialog *d) {
            Q_EMIT d->findChild<FontCombo *>(QStringLiteral("fontName"))->familyChosen(QStringLiteral("Cousine"));
            d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->setValue(18);
            d->findChild<ColorButton *>(QStringLiteral("fontColor"))->setCurrent(ColorRef::rgb(QColor(0, 0x80, 0)));
            d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->setCurrentIndex(5);   // dotted
            d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->setCurrent(ColorRef::rgb(QColor(0xFF, 0, 0)));
            d->findChild<QCheckBox *>(QStringLiteral("strikethrough"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("superscript"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("smallCaps"))->setChecked(true);
        });
        auto at = [&](int position) {
            QTextCursor c(w.document()->story());
            c.setPosition(position + 1);
            return c.charFormat();
        };
        for (int position : {0, 6}) {
            const QTextCharFormat f = at(position);
            QCOMPARE(f.fontFamilies().toStringList().value(0), QStringLiteral("Cousine"));
            QCOMPARE(f.fontPointSize(), 18.0);
            QCOMPARE(f.foreground().color(), QColor(0, 0x80, 0));
            QCOMPARE(f.underlineStyle(), QTextCharFormat::DotLine);
            QCOMPARE(f.underlineColor(), QColor(0xFF, 0, 0));
            QVERIFY(f.fontStrikeOut() && !f.boolProperty(prop::DoubleStrike));
            QCOMPARE(f.verticalAlignment(), QTextCharFormat::AlignSuperScript);
            QCOMPARE(f.fontCapitalization(), QFont::SmallCaps);
        }
        // The style was not touched: "words" is still bold, and "Some " is not.
        QCOMPARE(at(6).fontWeight(), int(QFont::Bold));
        QCOMPARE(at(0).fontWeight(), int(QFont::Normal));
        QVERIFY(at(11).fontPointSize() != 18.0);   // (nothing past the selection)
        // One undo takes all of it back; one redo gives it again.
        w.view()->undo();
        for (int position : {0, 6}) {
            const QTextCharFormat f = at(position);
            QVERIFY(f.fontFamilies().toStringList().value(0) != QLatin1String("Cousine"));
            QVERIFY(f.fontPointSize() != 18.0);
            QCOMPARE(f.underlineStyle(), QTextCharFormat::NoUnderline);
            QVERIFY(!f.fontStrikeOut());
            QCOMPARE(f.verticalAlignment(), QTextCharFormat::AlignNormal);
            QCOMPARE(f.fontCapitalization(), QFont::MixedCase);
        }
        QCOMPARE(at(6).fontWeight(), int(QFont::Bold));
        QVERIFY(!w.document()->story()->isUndoAvailable());
        w.view()->redo();
        QCOMPARE(at(0).fontPointSize(), 18.0);
        QCOMPARE(at(6).underlineStyle(), QTextCharFormat::DotLine);
        // With no selection the choices are for what is typed next.
        QTextCursor end(w.document()->story());
        end.movePosition(QTextCursor::End);
        w.view()->setCursor(end);
        withFontDialog(w, [](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("fontStyle"))->setCurrentIndex(2);
            d->findChild<QCheckBox *>(QStringLiteral("allCaps"))->setChecked(true);
        });
        insertText(*w.document(), w.view()->cursor(), QStringLiteral("!"));
        const QTextCharFormat typed = at(w.document()->story()->characterCount() - 2);
        QCOMPARE(typed.fontWeight(), int(QFont::Bold));
        QCOMPARE(typed.fontCapitalization(), QFont::AllUppercase);
        QCOMPARE(at(w.document()->story()->characterCount() - 3).fontCapitalization(), QFont::MixedCase);
    }

    // Each effect set in the dialog is written to the file as the element it is and comes back as the same effect, on its words alone.
    void aFontEffectIsSavedInTheFileAndComesBack_data()
    {
        QTest::addColumn<QString>("box");
        QTest::addColumn<QString>("xml");
        QTest::newRow("strikethrough") << "strikethrough" << "<w:strike/>";
        QTest::newRow("double strikethrough") << "doubleStrikethrough" << "<w:dstrike/>";
        QTest::newRow("superscript") << "superscript" << "<w:vertAlign w:val=\"superscript\"/>";
        QTest::newRow("subscript") << "subscript" << "<w:vertAlign w:val=\"subscript\"/>";
        QTest::newRow("small caps") << "smallCaps" << "<w:smallCaps/>";
        QTest::newRow("all caps") << "allCaps" << "<w:caps/>";
        QTest::newRow("hidden") << "hidden" << "<w:vanish/>";
    }

    void aFontEffectIsSavedInTheFileAndComesBack()
    {
        QFETCH(QString, box);
        QFETCH(QString, xml);
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("one two three"));
        selectRange(w, 4, 7);
        withFontDialog(w, [&](QDialog *d) {
            d->findChild<QCheckBox *>(box)->setChecked(true);
        });
        auto has = [&](WriteDoc &doc, int position) {
            QTextCursor c(doc.story());
            c.setPosition(position + 1);
            const QTextCharFormat f = c.charFormat();
            if (box == QLatin1String("strikethrough")) return f.fontStrikeOut() && !f.boolProperty(prop::DoubleStrike);
            if (box == QLatin1String("doubleStrikethrough")) return f.fontStrikeOut() && f.boolProperty(prop::DoubleStrike);
            if (box == QLatin1String("superscript")) return f.verticalAlignment() == QTextCharFormat::AlignSuperScript;
            if (box == QLatin1String("subscript")) return f.verticalAlignment() == QTextCharFormat::AlignSubScript;
            if (box == QLatin1String("smallCaps")) return f.fontCapitalization() == QFont::SmallCaps;
            if (box == QLatin1String("allCaps")) return f.fontCapitalization() == QFont::AllUppercase;
            return f.boolProperty(tp::HiddenText);
        };
        QVERIFY(has(*w.document(), 4) && !has(*w.document(), 0) && !has(*w.document(), 8));
        const QByteArray zip = w.document()->toDocx();
        QVERIFY(rt::strictProblems(zip).isEmpty());
        const QByteArray x = documentXml(zip);
        QVERIFY2(runPropsOfText(x, QStringLiteral("two")).contains(xml), x.constData());
        QVERIFY2(runPropsOfText(x, QStringLiteral("one ")).isEmpty() && runPropsOfText(x, QStringLiteral(" three")).isEmpty(), x.constData());
        auto back = open(zip);
        QVERIFY(back);
        QCOMPARE(back->story()->toPlainText(), QStringLiteral("one two three"));
        QVERIFY(has(*back, 4) && !has(*back, 0) && !has(*back, 8));
        // Written once more, the same element.
        QVERIFY(runPropsOfText(documentXml(back->toDocx()), QStringLiteral("two")).contains(xml));
        // And turned off in the dialog, it is gone from the file.
        WriteWindow v;
        v.show();
        v.setDocument(std::move(back), QString());
        selectRange(v, 4, 7);
        withFontDialog(v, [&](QDialog *d) {
            QCOMPARE(d->findChild<QCheckBox *>(box)->checkState(), Qt::Checked);
            d->findChild<QCheckBox *>(box)->setChecked(false);
        });
        QVERIFY(!has(*v.document(), 4));
        QVERIFY2(!documentXml(v.document()->toDocx()).contains(xml.toUtf8()), documentXml(v.document()->toDocx()).constData());
    }

    // The underline's style and color are written as w:u with its val and color, and read back into the dialog as they were set.
    void anUnderlineStyleAndColorFromTheDialogAreSavedInTheFileAndComeBack()
    {
        // The choices of the dialog, in its order, and the val the file has for each.
        const QStringList vals = {QStringLiteral("none"), QStringLiteral("single"), QStringLiteral("words"), QStringLiteral("double"), QStringLiteral("thick"), QStringLiteral("dotted"),
                                  QStringLiteral("dash"), QStringLiteral("dotDash"), QStringLiteral("dotDotDash"), QStringLiteral("wave"), QStringLiteral("wavyDouble")};
        WriteWindow w;
        w.show();
        QString text;
        for (int i = 1; i < vals.size(); ++i) text += QStringLiteral("w%1 ").arg(i, 2, 10, QLatin1Char('0'));
        typeText(*w.document(), w.view()->cursor(), text.trimmed());
        for (int i = 1; i < vals.size(); ++i) {
            selectRange(w, (i - 1) * 4, (i - 1) * 4 + 3);
            withFontDialog(w, [&](QDialog *d) {
                d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->setCurrentIndex(i);
                // (Colored in the second half; the first half's lines take the text's color.)
                if (i > 5) d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->setCurrent(ColorRef::rgb(QColor(0xFF, 0, 0)));
            });
        }
        const QByteArray zip = w.document()->toDocx();
        QVERIFY(rt::strictProblems(zip).isEmpty());
        const QByteArray x = documentXml(zip);
        for (int i = 1; i < vals.size(); ++i) {
            const QString props = runPropsOfText(x, QStringLiteral("w%1").arg(i, 2, 10, QLatin1Char('0')));
            const QString want = i > 5 ? QStringLiteral("<w:u w:val=\"%1\" w:color=\"FF0000\"/>").arg(vals[i]) : QStringLiteral("<w:u w:val=\"%1\"/>").arg(vals[i]);
            QVERIFY2(props.contains(want), qPrintable(QStringLiteral("%1: %2").arg(want, props)));
        }
        // Read again, the dialog shows each choice and color.
        WriteWindow v;
        v.show();
        v.setDocument(open(zip), QString());
        for (int i = 1; i < vals.size(); ++i) {
            selectRange(v, (i - 1) * 4, (i - 1) * 4 + 3);
            withFontDialog(v, [&](QDialog *d) {
                QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->currentIndex(), i);
                const ColorRef color = d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->current();
                if (i > 5) QCOMPARE(color.rgbValue(), QColor(0xFF, 0, 0));
                else QVERIFY(color.isNone());
            }, false);
        }
        // Underline off, and the color automatic again: the file says no line where there was one, and no color where there was one.
        selectRange(v, 20, 23);   // "w06", dashed
        withFontDialog(v, [](QDialog *d) { d->findChild<QComboBox *>(QStringLiteral("underlineStyle"))->setCurrentIndex(0); });
        QVERIFY(!documentXml(v.document()->toDocx()).contains("w:val=\"dash\""));
        selectRange(v, 24, 27);   // "w07", dot dash in red
        withFontDialog(v, [](QDialog *d) { d->findChild<ColorButton *>(QStringLiteral("underlineColor"))->setCurrent(ColorRef::none()); });
        QVERIFY2(runPropsOfText(documentXml(v.document()->toDocx()), QStringLiteral("w07")) == QLatin1String("<w:u w:val=\"dotDash\"/>"), documentXml(v.document()->toDocx()).constData());
    }

    // A file's own effects are read into the dialog's, and written back as they were (a style that has one, and a run that turns it off, too).
    void aFilesEffectsAreReadAndWrittenBack()
    {
        const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Shouting\"><w:name w:val=\"Shouting\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:caps/></w:rPr></w:style>";
        const QByteArray body =
            "<w:p><w:r><w:rPr><w:strike/></w:rPr><w:t>strike</w:t></w:r><w:r><w:t xml:space=\"preserve\"> </w:t></w:r><w:r><w:rPr><w:dstrike/></w:rPr><w:t>dstrike</w:t></w:r></w:p>"
            "<w:p><w:r><w:rPr><w:vertAlign w:val=\"superscript\"/></w:rPr><w:t>super</w:t></w:r><w:r><w:rPr><w:vertAlign w:val=\"subscript\"/></w:rPr><w:t>sub</w:t></w:r></w:p>"
            "<w:p><w:r><w:rPr><w:smallCaps/></w:rPr><w:t>small</w:t></w:r><w:r><w:rPr><w:vanish/></w:rPr><w:t>hidden</w:t></w:r></w:p>"
            "<w:p><w:r><w:rPr><w:u w:val=\"double\" w:color=\"00B050\"/></w:rPr><w:t>double</w:t></w:r><w:r><w:rPr><w:u w:val=\"dottedHeavy\"/></w:rPr><w:t>heavy</w:t></w:r>"
            "<w:r><w:rPr><w:u w:val=\"dashLong\" w:color=\"0070C0\"/></w:rPr><w:t>long</w:t></w:r></w:p>"
            "<w:p><w:pPr><w:pStyle w:val=\"Shouting\"/></w:pPr><w:r><w:t>loud</w:t></w:r><w:r><w:rPr><w:caps w:val=\"0\"/><w:smallCaps/></w:rPr><w:t>quiet</w:t></w:r></w:p>";
        // Every underline a file can name, each in a run of its own (the runs differ, so none is joined to the next).
        const QStringList lines = {QStringLiteral("single"),     QStringLiteral("words"),         QStringLiteral("double"),          QStringLiteral("thick"),        QStringLiteral("dotted"),
                                   QStringLiteral("dottedHeavy"), QStringLiteral("dash"),         QStringLiteral("dashedHeavy"),     QStringLiteral("dashLong"),     QStringLiteral("dashLongHeavy"),
                                   QStringLiteral("dotDash"),   QStringLiteral("dashDotHeavy"), QStringLiteral("dotDotDash"),      QStringLiteral("dashDotDotHeavy"), QStringLiteral("wave"),
                                   QStringLiteral("wavyHeavy"), QStringLiteral("wavyDouble")};
        QByteArray underlines = "<w:p>";
        for (const QString &val : lines) underlines += "<w:r><w:rPr><w:u w:val=\"" + val.toUtf8() + "\"/></w:rPr><w:t>u-" + val.toUtf8() + "</w:t></w:r>";
        underlines += "</w:p>";
        auto doc = open(makeDocx(body + underlines, styles));
        QVERIFY(doc);
        const QByteArray zip = doc->toDocx();
        QVERIFY(rt::strictProblems(zip).isEmpty());
        const QByteArray x = documentXml(zip);
        auto props = [&](const char *word) { return runPropsOfText(x, QString::fromLatin1(word)); };
        for (const QString &val : lines) {
            const QString got = runPropsOfText(x, QStringLiteral("u-") + val);
            QVERIFY2(got == QStringLiteral("<w:u w:val=\"%1\"/>").arg(val), qPrintable(val + QStringLiteral(" came back as ") + got));
        }
        QVERIFY2(props("strike") == QLatin1String("<w:strike/>"), qPrintable(props("strike")));
        QVERIFY2(props("dstrike") == QLatin1String("<w:dstrike/>"), qPrintable(props("dstrike")));
        QVERIFY2(props("super") == QLatin1String("<w:vertAlign w:val=\"superscript\"/>"), qPrintable(props("super")));
        QVERIFY2(props("sub") == QLatin1String("<w:vertAlign w:val=\"subscript\"/>"), qPrintable(props("sub")));
        QVERIFY2(props("small") == QLatin1String("<w:smallCaps/>"), qPrintable(props("small")));
        QVERIFY2(props("hidden") == QLatin1String("<w:vanish/>"), qPrintable(props("hidden")));
        QVERIFY2(props("double") == QLatin1String("<w:u w:val=\"double\" w:color=\"00B050\"/>"), qPrintable(props("double")));
        QVERIFY2(props("heavy") == QLatin1String("<w:u w:val=\"dottedHeavy\"/>"), qPrintable(props("heavy")));
        QVERIFY2(props("long") == QLatin1String("<w:u w:val=\"dashLong\" w:color=\"0070C0\"/>"), qPrintable(props("long")));
        // The paragraph's style is all caps, and so "loud" says nothing of it; "quiet" turns it off and small caps on.
        QVERIFY2(props("loud").isEmpty(), qPrintable(props("loud")));
        QVERIFY2(props("quiet").contains(QLatin1String("<w:caps w:val=\"0\"/>")) && props("quiet").contains(QLatin1String("<w:smallCaps/>")), qPrintable(props("quiet")));
        // The dialog on a style's all caps: clearing it says "not caps" in the run.
        WriteWindow w;
        w.show();
        w.setDocument(std::move(doc), QString());
        const int loud = w.document()->story()->toPlainText().indexOf(QLatin1String("loud"));
        selectRange(w, loud, loud + 4);
        withFontDialog(w, [](QDialog *d) {
            QCOMPARE(d->findChild<QCheckBox *>(QStringLiteral("allCaps"))->checkState(), Qt::Checked);
            d->findChild<QCheckBox *>(QStringLiteral("allCaps"))->setChecked(false);
        });
        QVERIFY2(runPropsOfText(documentXml(w.document()->toDocx()), QStringLiteral("loud")).contains(QLatin1String("<w:caps w:val=\"0\"/>")), documentXml(w.document()->toDocx()).constData());
    }

    // ---------------------------------------------------------------- after the review
    // What a dialog test does to the dialog can fail without leaving the dialog open: the helper closes it from outside, and reports a dialog that is not the one asked for.
    void aFailedCheckInADialogTestStillClosesTheDialog()
    {
        WriteWindow w;
        w.show();
        bool filled = false;
        withFontDialog(w, [&filled](QDialog *) {
            filled = true;
            QEXPECT_FAIL("", "this check is meant to fail", Continue);
            QCOMPARE(1, 2);
        });
        QVERIFY(filled && !QApplication::activeModalWidget());
        // The zoom dialog opens where the paragraph dialog was expected: it is closed without being touched, and the test says so.
        const double zoom = w.view()->zoom();
        QEXPECT_FAIL("", "the dialog that opens is the zoom dialog, not the paragraph dialog: meant to be reported", Continue);
        withDialog([&w] { w.act("zoom.dialog")->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *) { QFAIL("not to run on the wrong dialog"); });
        QVERIFY(!QApplication::activeModalWidget());
        QCOMPARE(w.view()->zoom(), zoom);
    }

    // The welcome topic does not say that headers and footers are missing: they are shown.
    void theWelcomeTopicDoesNotSayHeadersAreNotShown()
    {
        const QString start = jp::help::topic(QStringLiteral("start")).markdown;
        QVERIFY2(!start.contains(QLatin1String("Headers, footers, and page numbers are not shown")), "headers and footers are drawn now");
    }

    // Over characters that differ in font, style, size, color, or underline, the boxes show nothing; whatever is set then goes to all of them, also what the first
    // characters have.
    void aFontBoxOverSelectionsThatDifferShowsNothingAndWhatIsSetGoesToAll()
    {
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Some words here"));
        selectRange(w, 5, 10);
        toggleBold(*w.document(), w.view()->cursor());
        toggleItalic(*w.document(), w.view()->cursor());
        setFontSize(*w.document(), w.view()->cursor(), 20);
        setTextColor(*w.document(), w.view()->cursor(), QColor(0xC0, 0, 0));
        setFontFamily(*w.document(), w.view()->cursor(), QStringLiteral("Cousine"));
        changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) {
            f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            f.setProperty(tp::UnderlineKind, 1);   // double
            f.setUnderlineColor(QColor(0, 0, 0xFF));
        });
        w.document()->story()->clearUndoRedoStacks();
        auto at = [&](int position) {
            QTextCursor c(w.document()->story());
            c.setPosition(position + 1);
            return c.charFormat();
        };
        const QTextCharFormat plain = at(0);   // ("Some ")
        // Opened and closed with nothing touched, the dialog changes nothing.
        selectRange(w, 0, 10);
        withFontDialog(w, [](QDialog *d) {
            auto *font = d->findChild<FontCombo *>(QStringLiteral("fontName"));
            auto *style = d->findChild<QComboBox *>(QStringLiteral("fontStyle"));
            auto *size = d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"));
            auto *underline = d->findChild<QComboBox *>(QStringLiteral("underlineStyle"));
            QVERIFY(font && style && size && underline);
            QVERIFY(font->currentText().trimmed().isEmpty());
            QCOMPARE(style->currentIndex(), -1);
            QVERIFY2(size->text().trimmed().isEmpty(), qPrintable(size->text()));
            QCOMPARE(underline->currentIndex(), -1);
        });
        QVERIFY(!w.document()->story()->isUndoAvailable());
        QCOMPARE(at(6).fontPointSize(), 20.0);
        // Each is set to what "Some " has, which is what the box would have shown before: all of the selection takes it.
        selectRange(w, 0, 10);
        withFontDialog(w, [&](QDialog *d) {
            auto *font = d->findChild<FontCombo *>(QStringLiteral("fontName"));
            auto *style = d->findChild<QComboBox *>(QStringLiteral("fontStyle"));
            auto *size = d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"));
            auto *color = d->findChild<ColorButton *>(QStringLiteral("fontColor"));
            auto *underline = d->findChild<QComboBox *>(QStringLiteral("underlineStyle"));
            QVERIFY(font && style && size && color && underline);
            Q_EMIT font->familyChosen(QStringLiteral("+Body"));
            style->setCurrentIndex(0);
            size->setValue(plain.fontPointSize());
            color->setCurrent(ColorRef::none());
            Q_EMIT color->colorPicked(ColorRef::none());
            underline->setCurrentIndex(0);
        });
        for (int position : {0, 6}) {
            const QTextCharFormat f = at(position);
            QVERIFY2(!f.hasProperty(QTextFormat::FontFamilies) && f.stringProperty(tp::ThemeFont) == QLatin1String("minor"), qPrintable(QString::number(position)));
            QCOMPARE(f.fontPointSize(), plain.fontPointSize());
            QCOMPARE(f.fontWeight(), int(QFont::Normal));
            QVERIFY(!f.fontItalic());
            QVERIFY(!f.hasProperty(QTextFormat::ForegroundBrush) || f.foreground().style() == Qt::NoBrush);
            QCOMPARE(f.underlineStyle(), QTextCharFormat::NoUnderline);
        }
        // One step undoes it.
        w.view()->undo();
        QCOMPARE(at(6).fontPointSize(), 20.0);
        QVERIFY(!w.document()->story()->isUndoAvailable());
    }

    // The colors of underlines that differ are shown as nothing too, and setting one colors the underlines there are, and no other text.
    void anUnderlineColorSetOverUnderlinesThatDifferColorsOnlyTheUnderlines()
    {
        WriteWindow w;
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("red blue plain"));
        selectRange(w, 0, 3);
        changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) { f.setUnderlineStyle(QTextCharFormat::SingleUnderline); f.setUnderlineColor(QColor(0xFF, 0, 0)); });
        selectRange(w, 4, 8);
        changeCharFormat(*w.document(), w.view()->cursor(), [](QTextCharFormat &f) { f.setUnderlineStyle(QTextCharFormat::SingleUnderline); f.setUnderlineColor(QColor(0, 0, 0xFF)); });
        selectRange(w, 0, 14);
        // (Red, which is what the first of the lines has, goes to the blue one as well.)
        withFontDialog(w, [](QDialog *d) {
            auto *color = d->findChild<ColorButton *>(QStringLiteral("underlineColor"));
            QVERIFY(color);
            color->setCurrent(ColorRef::rgb(QColor(0xFF, 0, 0)));
            Q_EMIT color->colorPicked(ColorRef::rgb(QColor(0xFF, 0, 0)));
        });
        auto at = [&](int position) {
            QTextCursor c(w.document()->story());
            c.setPosition(position + 1);
            return c.charFormat();
        };
        QCOMPARE(at(1).underlineColor(), QColor(0xFF, 0, 0));
        QCOMPARE(at(5).underlineColor(), QColor(0xFF, 0, 0));
        QCOMPARE(at(10).underlineStyle(), QTextCharFormat::NoUnderline);   // ("plain" gets no line, and no color for one)
        QVERIFY(!at(10).hasProperty(QTextFormat::TextUnderlineColor));
    }

    // A bookmark or a field's code in a selection is not a character that has no effect: a selection that is struck through all over is, with a bookmark in it.
    void anAtomInTheSelectionDoesNotMakeAnEffectPartial()
    {
        auto run = [](const char *text) { return QByteArray("<w:r><w:rPr><w:strike/></w:rPr><w:t>") + text + "</w:t></w:r>"; };
        auto doc = open(makeDocx("<w:p>" + run("aa") + "<w:bookmarkStart w:id=\"1\" w:name=\"mark\"/>" + run("bb") + "<w:bookmarkEnd w:id=\"1\"/></w:p>", kStyles));
        QVERIFY(doc);
        // (The bookmark is a piece of the story that is not struck through itself.)
        int atoms = 0;
        for (QTextBlock::iterator it = doc->story()->firstBlock().begin(); !it.atEnd(); ++it)
            if (isAtom(it.fragment().charFormat())) {
                ++atoms;
                QVERIFY(!it.fragment().charFormat().fontStrikeOut());
            }
        QVERIFY(atoms >= 1);
        WriteWindow w;
        w.show();
        w.setDocument(std::move(doc), QString());
        w.view()->selectAll();
        withFontDialog(w, [](QDialog *d) {
            QCheckBox *strike = d->findChild<QCheckBox *>(QStringLiteral("strikethrough"));
            QVERIFY(strike);
            QCOMPARE(strike->checkState(), Qt::Checked);
            for (const char *other : {"doubleStrikethrough", "superscript", "subscript", "smallCaps", "allCaps", "hidden"}) QCOMPARE(d->findChild<QCheckBox *>(QString::fromLatin1(other))->checkState(), Qt::Unchecked);
        }, false);
        // The ribbon's button is down, and pressing it takes the line off all of the text.
        QTRY_VERIFY(w.act(QStringLiteral("fmt.strike"))->isChecked());
        w.act(QStringLiteral("fmt.strike"))->trigger();
        for (QTextBlock::iterator it = w.document()->story()->firstBlock().begin(); !it.atEnd(); ++it)
            if (!isAtom(it.fragment().charFormat())) QVERIFY2(!it.fragment().charFormat().fontStrikeOut(), qPrintable(it.fragment().text()));
    }

    // A line with several flags is written as the one the flags are first for (double, then words, then thick); every name a file has for an underline is read as a
    // line and flags and written as the same name; and a long dash is a flag of its own.
    void anUnderlinesFlagsAndNamesMapBothWays()
    {
        auto valOf = [](QTextCharFormat::UnderlineStyle style, int kind) {
            QTextCharFormat f;
            f.setUnderlineStyle(style);
            if (kind) f.setProperty(tp::UnderlineKind, kind);
            return runPropsOf(f).underline->val;
        };
        // (JeffPub's double plus thick underline is kind 5; pasted into JeffWrite and saved it must stay a double line.)
        const QList<QPair<int, QString>> single = {{0, QStringLiteral("single")}, {1, QStringLiteral("double")}, {2, QStringLiteral("words")}, {4, QStringLiteral("thick")}, {3, QStringLiteral("double")},
                                                   {5, QStringLiteral("double")}, {6, QStringLiteral("words")}, {7, QStringLiteral("double")}};
        for (const auto &k : single) QCOMPARE(valOf(QTextCharFormat::SingleUnderline, k.first), k.second);
        const QList<QPair<int, QString>> wave = {{0, QStringLiteral("wave")}, {1, QStringLiteral("wavyDouble")}, {3, QStringLiteral("wavyDouble")}, {5, QStringLiteral("wavyDouble")}, {7, QStringLiteral("wavyDouble")}, {4, QStringLiteral("wavyHeavy")}};
        for (const auto &k : wave) QCOMPARE(valOf(QTextCharFormat::WaveUnderline, k.first), k.second);
        QCOMPARE(valOf(QTextCharFormat::DotLine, 1), QStringLiteral("dotted"));   // (a flag the line has no name for: the plain line)
        struct Name { const char *val; QTextCharFormat::UnderlineStyle style; int kind; };
        const Name names[] = {{"single", QTextCharFormat::SingleUnderline, 0}, {"words", QTextCharFormat::SingleUnderline, 2}, {"double", QTextCharFormat::SingleUnderline, 1}, {"thick", QTextCharFormat::SingleUnderline, 4},
                              {"dotted", QTextCharFormat::DotLine, 0}, {"dottedHeavy", QTextCharFormat::DotLine, 4}, {"dash", QTextCharFormat::DashUnderline, 0}, {"dashedHeavy", QTextCharFormat::DashUnderline, 4},
                              {"dashLong", QTextCharFormat::DashUnderline, 8}, {"dashLongHeavy", QTextCharFormat::DashUnderline, 12}, {"dotDash", QTextCharFormat::DashDotLine, 0},
                              {"dashDotHeavy", QTextCharFormat::DashDotLine, 4}, {"dotDotDash", QTextCharFormat::DashDotDotLine, 0}, {"dashDotDotHeavy", QTextCharFormat::DashDotDotLine, 4},
                              {"wave", QTextCharFormat::WaveUnderline, 0}, {"wavyHeavy", QTextCharFormat::WaveUnderline, 4}, {"wavyDouble", QTextCharFormat::WaveUnderline, 1}};
        for (const Name &n : names) {
            wml::RunProps r;
            wml::Underline u;
            u.val = QString::fromLatin1(n.val);
            r.underline = u;
            const QTextCharFormat f = charFormatOf(r);
            QVERIFY2(f.underlineStyle() == n.style && f.intProperty(tp::UnderlineKind) == n.kind, n.val);
            QCOMPARE(runPropsOf(f).underline->val, QString::fromLatin1(n.val));
        }
    }

    // Replace All takes time in step with the matches, also when they are all in one paragraph (it was the square of them: each swap in a long paragraph cost its length).
    void replaceAllInOneLongParagraphTakesTimeInStepWithTheMatches()
    {
        // A paragraph of 96,000 characters with this many "cat"s in it; the time Replace All takes to turn them into "dog"s.
        auto seconds = [](int matches, double *took) {
            std::unique_ptr<WriteWindow> w;
            WriteFindPane *pane = nullptr;
            QString text;
            const qint64 ns = bestNanoseconds([&] { pane->replaceAll(); }, 5, [&] {
                w.reset();
                const int per = 96000 / matches;
                text.clear();
                while (text.size() < 96000) text += QString(per - 4, QLatin1Char('x')) + QStringLiteral(" cat");
                w = std::make_unique<WriteWindow>();
                QTextCursor c(w->document()->story());
                c.insertText(text);
                w->showFindPane(true);
                pane = w->findPane();
                pane->findBox()->setText(QStringLiteral("cat"));
                pane->replaceBox()->setText(QStringLiteral("dog"));
            });
            *took = ns / 1e9;
            QCOMPARE(pane->status(), QStringLiteral("JeffWrite made %1 replacements.").arg(text.count(QLatin1String("cat"))));
            QVERIFY(!w->document()->story()->toPlainText().contains(QLatin1String("cat")));
        };
        double few = 0, many = 0;
        seconds(2000, &few);
        seconds(16000, &many);
        few = std::max(few, 0.001);
        // (Eight times the matches: eight times the time, with some room; the square would be sixty-four.)
        QVERIFY2(many < few * 24, qPrintable(QStringLiteral("2,000 matches took %1 s, 16,000 took %2 s").arg(few).arg(many)));
    }

    // A bookmark in the middle of words does not hide them: they are found, shown selected, and replaced, and the bookmark is kept.
    void findAndReplaceSeeWordsThatABookmarkSplits()
    {
        const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">say foo </w:t></w:r><w:bookmarkStart w:id=\"0\" w:name=\"_GoBack\"/><w:r><w:t>bar</w:t></w:r><w:bookmarkEnd w:id=\"0\"/>"
                                "<w:r><w:t xml:space=\"preserve\"> and foo bar again</w:t></w:r></w:p>";
        WriteWindow w;
        w.setDocument(open(makeDocx(body, kStyles)), QString());
        w.show();
        QVERIFY2(w.document()->story()->toPlainText().contains(QChar::ObjectReplacementCharacter), "the bookmark is in the story, between the words");
        auto words = [&] {
            QString t = w.document()->story()->toPlainText();
            t.remove(QChar::ObjectReplacementCharacter);
            return t;
        };
        w.showFindPane(true);
        WriteFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("foo bar"));
        QTextCursor start(w.document()->story());
        w.view()->setCursor(start);
        QVERIFY(pane->findNext());
        QString shown = w.view()->cursor().selectedText();
        shown.remove(QChar::ObjectReplacementCharacter);
        QCOMPARE(shown, QStringLiteral("foo bar"));
        QCOMPARE(w.view()->cursor().selectionStart(), 4);
        QVERIFY(pane->findNext());   // the second, which has no bookmark in it
        QVERIFY(w.view()->cursor().selectionStart() > 10);
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("foo bar"));
        // Replace swaps the first: the new words are where the old were, and the bookmark is still there.
        w.view()->setCursor(start);
        pane->replaceBox()->setText(QStringLiteral("baz"));
        pane->replaceOne();   // (shows the first)
        pane->replaceOne();
        QCOMPARE(words(), QStringLiteral("say baz and foo bar again"));
        QVERIFY(w.document()->story()->toPlainText().contains(QChar::ObjectReplacementCharacter));
        QVERIFY(w.document()->toDocx().size() > 0);
        QVERIFY2(documentXml(w.document()->toDocx()).contains("_GoBack"), "the bookmark is saved");
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("foo bar"));   // (on to the next)
        // Undo puts it back as it was, with one step.
        w.view()->undo();
        QCOMPARE(words(), QStringLiteral("say foo bar and foo bar again"));
        // Replace All: both, one step.
        w.document()->story()->clearUndoRedoStacks();
        pane->replaceAll();
        QCOMPARE(words(), QStringLiteral("say baz and baz again"));
        QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 2 replacements."));
        QVERIFY2(documentXml(w.document()->toDocx()).contains("_GoBack"), "the bookmark is saved");
        w.view()->undo();
        QCOMPARE(words(), QStringLiteral("say foo bar and foo bar again"));
        QVERIFY(!w.document()->story()->isUndoAvailable());
    }

    // Replace All of the shared search finds the matches first and puts the new words in from the last (the one way that does not cost a long paragraph its length
    // for each); it gives what putting them in from the first, finding on from each, gave: the same words, the same count, the same looks, also for whole words that
    // touch (the letter before a match is then the last of the words put in place of the one before). JeffDeck and JeffPub swap this way.
    void replacingFromTheLastMatchGivesWhatReplacingFromTheFirstGave()
    {
        int differ = 0;   // (the cases in which the two ways would not give the same words, if the shared search did not tell them apart)
        auto check = [&differ](const QString &text, const QString &q, const QString &with, QTextDocument::FindFlags flags, const QString &where) {
            QTextDocument a, b, c;
            for (QTextDocument *d : {&a, &b, &c}) {
                QTextCursor cursor(d);
                cursor.insertText(text);
                for (int i = 0; i + 1 < d->characterCount(); i += 2) {   // (every other character bold, so that the looks can be told)
                    QTextCursor t(d);
                    t.setPosition(i);
                    t.setPosition(i + 1, QTextCursor::KeepAnchor);
                    QTextCharFormat f;
                    f.setFontWeight(QFont::Bold);
                    t.mergeCharFormat(f);
                }
            }
            // From the first match on, finding on from each swap.
            int expected = 0;
            for (QTextCursor m = a.find(q, 0, flags); !m.isNull(); m = a.find(q, m, flags)) {
                m.insertText(with);
                ++expected;
            }
            // The matches in the words as they are, put in from the last (what would be given if the search did not look at whole words that touch).
            QVector<QPair<int, int>> found;
            for (QTextCursor m = c.find(q, 0, flags); !m.isNull(); m = c.find(q, m, flags)) found.push_back({m.selectionStart(), m.selectionEnd()});
            for (int i = int(found.size()) - 1; i >= 0; --i) {
                QTextCursor m(&c);
                m.setPosition(found[i].first);
                m.setPosition(found[i].second, QTextCursor::KeepAnchor);
                m.insertText(with);
            }
            if (c.toPlainText() != a.toPlainText()) ++differ;
            FindWords words;
            words.story = [&b](const FindStory &) { return &b; };
            QVector<FindStory> stories;
            stories << FindStory{QString(), QStringLiteral("s")};
            const int made = replaceInStories(words, stories, q, flags, with);
            QVERIFY2(made == expected, qPrintable(where + QStringLiteral(" made %1, not %2").arg(made).arg(expected)));
            QVERIFY2(a.toPlainText() == b.toPlainText(), qPrintable(where + QStringLiteral(" gave ") + b.toPlainText() + QStringLiteral(" and not ") + a.toPlainText()));
            for (int i = 0; i + 1 < a.characterCount(); ++i) {
                QTextCursor ca(&a), cb(&b);
                ca.setPosition(i + 1);
                cb.setPosition(i + 1);
                QVERIFY2(ca.charFormat().fontWeight() == cb.charFormat().fontWeight(), qPrintable(where + QStringLiteral(" at %1").arg(i)));
            }
        };
        const QTextDocument::FindFlags whole = QTextDocument::FindWholeWords;
        // Whole words that touch: "--" is two words of "-", until the first is put out for "x", and the second then has a letter before it.
        check(QStringLiteral("-- --"), QStringLiteral("-"), QStringLiteral("x"), whole, QStringLiteral("touching"));
        check(QStringLiteral(" ,, ,"), QStringLiteral(","), QStringLiteral("ab"), whole, QStringLiteral("touching, with two letters"));
        check(QStringLiteral("-- --"), QStringLiteral("-"), QStringLiteral("-"), whole, QStringLiteral("touching, and put out for the same"));
        check(QStringLiteral("-- --"), QStringLiteral("-"), QString(), whole, QStringLiteral("touching, put out for nothing"));
        check(QStringLiteral("cat cat\ncat"), QStringLiteral("cat"), QStringLiteral("dog"), QTextDocument::FindFlags(), QStringLiteral("apart"));
        QVERIFY2(differ >= 2, qPrintable(QString::number(differ)));
        std::mt19937 rng(11);
        const QStringList alphabet = {QStringLiteral("a"), QStringLiteral("b"), QStringLiteral(" "), QStringLiteral("-"), QStringLiteral("A"), QStringLiteral("-"), QStringLiteral(" "), QStringLiteral("\n")};
        const QStringList queries = {QStringLiteral("a"), QStringLiteral("-"), QStringLiteral("ab"), QStringLiteral("aa"), QStringLiteral("b-"), QStringLiteral("A"), QStringLiteral("a-a"), QStringLiteral("-")};
        const QStringList withs = {QString(), QStringLiteral("a"), QStringLiteral("x"), QStringLiteral("ab"), QStringLiteral("-"), QStringLiteral("xyz")};
        for (int round = 0; round < 400; ++round) {
            QString text;
            for (int i = 0, n = 1 + int(rng() % 30); i < n; ++i) text += alphabet[rng() % alphabet.size()];
            QTextDocument::FindFlags flags;
            if (rng() % 2) flags |= QTextDocument::FindCaseSensitively;
            if (rng() % 2) flags |= QTextDocument::FindWholeWords;
            const QString q = queries[rng() % queries.size()], with = withs[rng() % withs.size()];
            check(text, q, with, flags, QStringLiteral("%1: \"%2\" for \"%3\" in \"%4\"").arg(round).arg(q, with, QString(text).replace(QLatin1Char('\n'), QLatin1Char('|'))));
            if (QTest::currentTestFailed()) return;
        }
    }

    void exportPdfWritesAPdfWithAPagePerPage()
    {
        QTemporaryDir dir;
        WriteWindow w;
        w.setDocument(open(makeDocx(paragraphs(120), kStyles)), QString());
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("out.pdf")); });
        QVERIFY(w.exportPdf());
        QFile f(dir.filePath(QStringLiteral("out.pdf")));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        QVERIFY(pdf.startsWith("%PDF"));
        int pages = 0;
        for (qsizetype at = pdf.indexOf("/Type /Page\n"); at >= 0; at = pdf.indexOf("/Type /Page\n", at + 1)) ++pages;
        QCOMPARE(pages, w.view()->pageCount());
        QVERIFY(w.view()->pageCount() > 3);
    }

    void pagesToPrintFollowTheRange()
    {
        WriteWindow w;
        w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
        const int n = (w.view()->paginateAll(), w.view()->pageCount());
        QVERIFY(n > 5);
        WriteOutputOptions o;
        QCOMPARE(w.outputPages(o).size(), n);
        o.range = WriteOutputOptions::Range::Pages;
        o.pages = QStringLiteral("2-4, 2, 9999, x, 1");
        QCOMPARE(w.outputPages(o), (QVector<int>{1, 2, 3, 0}));
        o.range = WriteOutputOptions::Range::Current;
        QCOMPARE(w.outputPages(o).size(), 1);
    }

    // ---------------------------------------------------------------- printing
    void pageRangesAreReadAsTheyAreTyped()
    {
        using R = QVector<int>;
        QCOMPARE(parsePageRange(QStringLiteral("1-3, 5"), 10), (R{0, 1, 2, 4}));
        QCOMPARE(parsePageRange(QStringLiteral("5-3;9"), 10), (R{2, 3, 4, 8}));       // (backwards, and a semicolon)
        QCOMPARE(parsePageRange(QStringLiteral("2-4, 2, 9999, x, 1"), 10), (R{1, 2, 3, 0}));   // (once each, in the order given; what names no page is passed by)
        QCOMPARE(parsePageRange(QStringLiteral(" 7 - 20 "), 10), (R{6, 7, 8, 9}));    // (to the last page there is)
        QCOMPARE(parsePageRange(QStringLiteral("8-"), 10), (R{7, 8, 9}));              // (to the end)
        QCOMPARE(parsePageRange(QStringLiteral("-2"), 10), (R{0, 1}));                 // (from the start)
        QCOMPARE(parsePageRange(QStringLiteral("3"), 3), (R{2}));
        for (const char *bad : {"", "  ", "0", "11", "x", "abc", "1-2-3", ",,", "-", "-0", "0-", "0-5", "11-", "99-", "1.5", "99999999999", "99999999999-", "-,-"})
            QVERIFY2(parsePageRange(QString::fromLatin1(bad), 10).isEmpty(), bad);
        QVERIFY(parsePageRange(QStringLiteral("1"), 0).isEmpty());
        QVERIFY(parsePageRange(QStringLiteral("1-"), 0).isEmpty());
        // An open end is the last page (or the first) and no further: it never turns about.
        QCOMPARE(parsePageRange(QStringLiteral("10-"), 10), (R{9}));
        QCOMPARE(parsePageRange(QStringLiteral("-10"), 10), (R{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
        QCOMPARE(parsePageRange(QStringLiteral("-99"), 10), (R{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
        QCOMPARE(parsePageRange(QStringLiteral("-1"), 10), (R{0}));
        // Backwards ranges read forwards; numbers too big for any page are past the last; the rest of a list is still taken.
        QCOMPARE(parsePageRange(QStringLiteral("5-2"), 10), (R{1, 2, 3, 4}));
        QCOMPARE(parsePageRange(QStringLiteral("3-3"), 10), (R{2}));
        QCOMPARE(parsePageRange(QStringLiteral("8-99999999999"), 10), (R{7, 8, 9}));
        QCOMPARE(parsePageRange(QStringLiteral("99999999999-3"), 10), (R{2, 3, 4, 5, 6, 7, 8, 9}));
        QCOMPARE(parsePageRange(QStringLiteral(" , 4 ,, 2 , abc, 99-, -0"), 10), (R{3, 1}));
        QCOMPARE(parsePageRange(QStringLiteral("1-3,2-4"), 10), (R{0, 1, 2, 3}));
    }

    void copiesComeOutCollatedOrEachPageTogether()
    {
        using R = QVector<int>;
        QCOMPARE(sheetOrder(R{0, 1, 2}, 2, true), (R{0, 1, 2, 0, 1, 2}));
        QCOMPARE(sheetOrder(R{0, 1, 2}, 2, false), (R{0, 0, 1, 1, 2, 2}));
        QCOMPARE(sheetOrder(R{4, 2}, 1, true), (R{4, 2}));
        QCOMPARE(sheetOrder(R{4, 2}, 0, false), (R{4, 2}));   // (never fewer than one)
        QCOMPARE(sheetOrder(R(), 3, true), R());
    }

    void thePaperIsDescribedAsTheSectionSetsIt()
    {
        QCOMPARE(paperDescription(QSizeF(612, 792)), QStringLiteral("Letter, portrait"));
        QCOMPARE(paperDescription(QSizeF(792, 612)), QStringLiteral("Letter, landscape"));
        QCOMPARE(paperDescription(QSizeF(595.3, 841.9)), QStringLiteral("A4, portrait"));
        QVERIFY(paperDescription(QSizeF(500, 300)).endsWith(QStringLiteral(", landscape")));
        QVERIFY(paperDescription(QSizeF(500, 300)).contains(QLatin1Char('x')));
    }

    void thePrintScreenHasThePrinterCopiesCollateRangeAndPreview()
    {
        WriteWindow w;
        w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
        w.show();
        QCOMPARE(w.act("file.print")->shortcut(), QKeySequence(QKeySequence::Print));
        w.act("file.print")->trigger();
        auto *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
        auto *copies = w.findChild<QSpinBox *>(QStringLiteral("printCopies"));
        auto *collate = w.findChild<QCheckBox *>(QStringLiteral("printCollate"));
        auto *paper = w.findChild<QLabel *>(QStringLiteral("printPaper"));
        auto *page = w.findChild<QLabel *>(QStringLiteral("printPage"));
        auto *text = w.findChild<QLineEdit *>(QStringLiteral("printRangeText"));
        auto *print = w.findChild<QPushButton *>(QStringLiteral("printButton"));
        QVERIFY(printer && copies && collate && paper && page && text && print);
        QVERIFY(printer->findText(QStringLiteral("Save as PDF")) >= 0);
        QCOMPARE(copies->value(), 1);
        QVERIFY(!collate->isEnabled());   // (one copy has nothing to collate)
        copies->setValue(3);
        QVERIFY(collate->isEnabled() && collate->isChecked());
        // The paper is the document's: its size and way up.
        QVERIFY(paper->text().endsWith(QStringLiteral("portrait")));
        // A range typed is chosen, and the preview and the count follow it.
        w.findChild<QRadioButton *>(QStringLiteral("printRangePages"))->click();
        text->setText(QStringLiteral("2-3"));
        QVERIFY(w.findChild<QRadioButton *>(QStringLiteral("printRangePages"))->isChecked());
        QTRY_VERIFY(page->text().startsWith(QStringLiteral("Page 2 of ")));
        QVERIFY(print->isEnabled());
        text->setText(QStringLiteral("x"));
        QVERIFY(!print->isEnabled());
        QVERIFY(page->text().contains(QStringLiteral("page numbers")));
    }

    void printingGoesThroughThePainterThePdfExportUses()
    {
        QTemporaryDir dir;
        WriteWindow w;
        // Two sections: a portrait page, then landscape ones.
        {
            auto doc = WriteDoc::blank();
            QTextCursor c(doc->story());
            typeText(*doc, c, QStringLiteral("first section"));
            insertSectionBreak(*doc, c);
            typeText(*doc, c, QStringLiteral("a landscape page"));
            wml::SectionProps s = doc->sectionProps(1);
            s.pageW = 15840;
            s.pageH = 12240;
            s.orient = QStringLiteral("landscape");
            doc->setSectionProps(1, s);
            w.setDocument(std::move(doc), QString());
        }
        w.view()->paginateAll();
        QCOMPARE(w.view()->pageCount(), 2);
        struct Drawn {
            int number;
            bool output;
            TextOutput text;
            int pageCount;
            bool operator==(const Drawn &o) const { return number == o.number && output == o.output && text == o.text && pageCount == o.pageCount; }
        };
        QVector<Drawn> exported, printed;
        QVector<Drawn> *into = &exported;
        const ObserverGuard observer;
        write::setPaintObserver([&](const flow::MaterializedPage &page, const write::PageDrawing &d) { *into << Drawn{page.number, d.output, d.textOutput, d.pageCount}; });
        const QString pdfPath = dir.filePath(QStringLiteral("export.pdf")), printPath = dir.filePath(QStringLiteral("print.pdf"));
        QString next = pdfPath;
        w.setSavePathChooser([&](const QString &, const QString &) { return next; });
        QVERIFY(w.exportPdf());
        into = &printed;
        next = printPath;
        WriteOutputOptions o;
        o.toFile = true;
        QVERIFY(w.printOutput(o));
        write::setPaintObserver(nullptr);
        // The same pages, drawn for output; a printer's text is a printer's, a PDF export's a PDF's.
        QCOMPARE(exported.size(), 2);
        QCOMPARE(printed.size(), 2);
        for (int i = 0; i < 2; ++i) {
            QCOMPARE(printed[i].number, exported[i].number);
            QVERIFY(printed[i].output && exported[i].output);
            QCOMPARE(printed[i].pageCount, exported[i].pageCount);
            QCOMPARE(exported[i].text, TextOutput::Pdf);
            QCOMPARE(printed[i].text, TextOutput::Print);
        }
        // Each sheet is the size of its section's page, the landscape one turned.
        auto boxes = [](const QString &path) {
            QFile f(path);
            f.open(QIODevice::ReadOnly);
            const QByteArray pdf = f.readAll();
            QList<QSizeF> sizes;
            static const QRegularExpression box(QStringLiteral("/MediaBox\\s*\\[\\s*0\\s+0\\s+([0-9.]+)\\s+([0-9.]+)\\s*\\]"));
            auto it = box.globalMatch(QString::fromLatin1(pdf));
            while (it.hasNext()) {
                const auto m = it.next();
                sizes << QSizeF(m.captured(1).toDouble(), m.captured(2).toDouble());
            }
            return sizes;
        };
        const QList<QSizeF> a = boxes(pdfPath), b = boxes(printPath);
        QCOMPARE(a.size(), 2);
        QCOMPARE(b.size(), 2);
        for (int i = 0; i < 2; ++i) {
            QVERIFY2(qAbs(a[i].width() - b[i].width()) < 1 && qAbs(a[i].height() - b[i].height()) < 1, qPrintable(QStringLiteral("%1x%2 or %3x%4").arg(a[i].width()).arg(a[i].height()).arg(b[i].width()).arg(b[i].height())));
        }
        QVERIFY(b[0].height() > b[0].width());
        QVERIFY(b[1].width() > b[1].height());
    }

    void printingTakesThePagesOfTheRangeInTheOrderGiven()
    {
        QTemporaryDir dir;
        WriteWindow w;
        w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("range.pdf")); });
        QVector<int> numbers;
        const ObserverGuard observer;
        write::setPaintObserver([&](const flow::MaterializedPage &page, const write::PageDrawing &) { numbers << page.number; });
        WriteOutputOptions o;
        o.toFile = true;
        o.range = WriteOutputOptions::Range::Pages;
        o.pages = QStringLiteral("3, 1-2");
        QVERIFY(w.printOutput(o));
        write::setPaintObserver(nullptr);
        QCOMPARE(numbers, (QVector<int>{3, 1, 2}));
        QFile f(dir.filePath(QStringLiteral("range.pdf")));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        int pages = 0;
        for (qsizetype at = pdf.indexOf("/Type /Page\n"); at >= 0; at = pdf.indexOf("/Type /Page\n", at + 1)) ++pages;
        QCOMPARE(pages, 3);
        // A range that names no page prints nothing.
        o.pages = QStringLiteral("9999");
        QVERIFY(!w.printOutput(o));
    }

    void aPrinterNamedPdfIsAPrinterAndTheFileEntryIsFoundByItsRole()
    {
        QComboBox box;
        fillPrinterBox(&box, QStringList{QStringLiteral("Front Desk"), QStringLiteral("pdf"), QStringLiteral("Save as PDF")}, QStringLiteral("Front Desk"));
        QCOMPARE(box.count(), 4);
        QCOMPARE(printerChoice(&box).printer, QStringLiteral("Front Desk"));   // (the one to start on)
        for (int i = 0; i < 3; ++i) {
            box.setCurrentIndex(i);
            const PrinterChoice choice = printerChoice(&box);
            QVERIFY(!choice.toFile);
            QCOMPARE(choice.printer, box.itemText(i));   // (a printer called "pdf", or "Save as PDF", is a printer)
        }
        box.setCurrentIndex(3);
        QVERIFY(printerChoice(&box).toFile);
        QVERIFY(printerChoice(&box).printer.isEmpty());
        // With no printer there is the file alone.
        QComboBox none;
        fillPrinterBox(&none, QStringList(), QString());
        QCOMPARE(none.count(), 1);
        QVERIFY(printerChoice(&none).toFile);
    }

    void savingAsPdfThroughPrintLeavesAnExistingFileWholeUntilTheNewOneIsDone()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("old.pdf"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("the old file");
        }
        WriteWindow w;
        w.setDocument(open(makeDocx(paragraphs(60), kStyles)), QString());
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        int drawn = 0, changed = 0;
        const ObserverGuard observer;
        write::setPaintObserver([&](const flow::MaterializedPage &, const write::PageDrawing &) {
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly) || f.readAll() != QByteArray("the old file")) ++changed;
            ++drawn;
        });
        WriteOutputOptions o;
        o.toFile = true;
        QVERIFY(w.printOutput(o));
        write::setPaintObserver(nullptr);
        QVERIFY(drawn > 1);
        QCOMPARE(changed, 0);   // (the file is not touched while the pages are being made)
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(f.readAll().startsWith("%PDF"));
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files).size(), 1);   // (and nothing is left beside it)
    }

    void copiesAndCollateReachAPrinterThatCannotMakeThem()
    {
        QTemporaryDir dir;
        {
            QPrinter probe;
            probe.setOutputFormat(QPrinter::PdfFormat);
            if (probe.supportsMultipleCopies()) QSKIP("this printer engine makes copies itself, so the pages are not sent again");
        }
        WriteWindow w;
        w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
        // A printer that writes to a file stands in for a real one: the window's own way with a printer (not the file entry) is what runs.
        int made = 0;
        w.setPrinterPreparer([&](QPrinter &printer) {
            ++made;
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(dir.filePath(QStringLiteral("copies%1.pdf").arg(made)));
        });
        QVector<int> numbers;
        const ObserverGuard observer;
        write::setPaintObserver([&](const flow::MaterializedPage &page, const write::PageDrawing &) { numbers << page.number; });
        WriteOutputOptions o;
        o.printer = QStringLiteral("a printer");
        o.range = WriteOutputOptions::Range::Pages;
        o.pages = QStringLiteral("1-3");
        o.copies = 2;
        o.collate = true;
        QVERIFY(w.printOutput(o));
        QCOMPARE(numbers, (QVector<int>{1, 2, 3, 1, 2, 3}));
        numbers.clear();
        o.collate = false;
        QVERIFY(w.printOutput(o));
        QCOMPARE(numbers, (QVector<int>{1, 1, 2, 2, 3, 3}));
        numbers.clear();
        o.copies = 1;
        QVERIFY(w.printOutput(o));
        QCOMPARE(numbers, (QVector<int>{1, 2, 3}));
        // A PDF file is made once whatever the copies.
        numbers.clear();
        w.setPrinterPreparer(nullptr);
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("file.pdf")); });
        o.toFile = true;
        o.copies = 3;
        QVERIFY(w.printOutput(o));
        QCOMPARE(numbers, (QVector<int>{1, 2, 3}));
    }

    // ---------------------------------------------------------------- help
    void helpTopicsAreComplete()
    {
        const QString src = QStringLiteral(JW_SOURCE_DIR "/resources/help/en/write");
        QStringList onDisk, built;
        for (QDirIterator it(src, QDir::Files); it.hasNext();) onDisk << QFileInfo(it.next()).fileName();
        for (QDirIterator it(QStringLiteral(":/help/en/write"), QDir::Files); it.hasNext();) built << QFileInfo(it.next()).fileName();
        onDisk.sort();
        built.sort();
        QCOMPARE(built, onDisk);   // resources.qrc lists every file (tools/help_files.py)
        const QStringList ids = jp::help::topicIds();
        QVERIFY2(ids.contains(QLatin1String("start")) && ids.contains(QLatin1String("index")), qPrintable(ids.join(QLatin1Char(' '))));
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)]+)\\)"));
        static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office|Word)\\b"));
        const jp::help::Topic contents = jp::help::topic(QStringLiteral("index"));
        QStringList problems;
        for (const QString &id : ids) {
            const jp::help::Topic t = jp::help::topic(id);
            if (t.title.isEmpty()) problems << id + QStringLiteral(": no title");
            if (t.keywords.isEmpty()) problems << id + QStringLiteral(": no keywords");
            if (const auto m = banned.match(t.markdown); m.hasMatch()) problems << id + QStringLiteral(": says ") + m.captured(1);
            for (auto m = link.globalMatch(t.markdown); m.hasNext();) {
                const QString target = m.next().captured(1);
                if (!target.startsWith(QLatin1String("https://")) && !ids.contains(target)) problems << id + QStringLiteral(": link to ") + target;
            }
            if (id != QLatin1String("index") && !contents.markdown.contains(QStringLiteral("](%1)").arg(id))) problems << id + QStringLiteral(": not in the contents");
            QTextDocument doc;
            WriteWindow w;
            doc.setMarkdown(jp::help::displayMarkdown(t, &w), QTextDocument::MarkdownDialectGitHub);
            if (doc.toPlainText().trimmed().size() < 100) problems << id + QStringLiteral(": nearly empty");
        }
        QFile cf(src + QStringLiteral("/context.json"));
        QVERIFY(cf.open(QIODevice::ReadOnly));
        const QJsonObject context = QJsonDocument::fromJson(cf.readAll()).object();
        for (auto it = context.begin(); it != context.end(); ++it)
            if (!ids.contains(it.value().toString())) problems << QStringLiteral("context %1: no topic %2").arg(it.key(), it.value().toString());
        WriteWindow w;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (jp::help::contextTopic(QStringLiteral("tab:") + w.ribbon()->tabName(i)).isEmpty()) problems << QStringLiteral("no topic for the tab ") + w.ribbon()->tabName(i);
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
        // The keyboard topic's table comes from the commands themselves.
        const QString keys = jp::help::displayMarkdown(jp::help::topic(QStringLiteral("keyboard")), &w);
        QVERIFY(!keys.contains(QLatin1String("<!--")));
        QVERIFY2(keys.contains(QStringLiteral("| Save | %1 |").arg(QKeySequence(QKeySequence::Save).toString(QKeySequence::NativeText))), qPrintable(keys.right(1500)));
        QVERIFY(keys.contains(QStringLiteral("| Bold |")));
    }

    void f1OpensHelpOnWhatIsBeingDone()
    {
        WriteWindow w;
        w.resize(1300, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        w.view()->setFocus();
        QTest::keyClick(w.view(), Qt::Key_F1);
        QVERIFY(w.helpView() && w.helpView()->isVisible());
        QCOMPARE(w.helpView()->currentTopic(), QStringLiteral("index"));
        w.showFileScreen(QStringLiteral("print"));
        w.act("help.show")->trigger();   // over the File screen: a window of its own, on that page's topic
        const auto *over = w.findChild<jp::HelpWindow *>();
        QVERIFY(over && over->isVisible());
        QCOMPARE(over->view()->currentTopic(), QStringLiteral("print"));
        // In the Spelling pane, help is on spelling.
        w.findChild<jp::HelpWindow *>()->close();
        w.hideFileScreen();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        w.act("review.spelling")->trigger();
        w.findChild<SpellPane *>()->findChild<QListWidget *>(QStringLiteral("spellSuggestions"))->setFocus();
        w.act("help.show")->trigger();
        QCOMPARE(w.helpView()->currentTopic(), QStringLiteral("spelling"));
        // The start topic says what JeffWrite is and that it is a preview.
        const jp::help::Topic start = jp::help::topic(QStringLiteral("start"));
        QVERIFY(start.markdown.contains(QLatin1String("Preview")));
        QVERIFY(start.markdown.contains(QLatin1String("What works")) && start.markdown.contains(QLatin1String("What does not work yet")));
    }

    // ---------------------------------------------------------------- the page view
    void theViewShowsTheFirstPageAtOnceAndLaysOutTheRestInTheBackground()
    {
        auto doc = open(makeDocx(paragraphs(3000), kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        QVERIFY(view.pageCount() >= 1);
        view.show();
        QVERIFY(view.grab().width() == 1000);   // the first page is drawn
        // (Without waiting for the document: a few pages are laid out, not all of them.)
        QVERIFY2(doc->pager().pagesLaidOut() < 20 && !view.pagesComplete(), qPrintable(QString::number(doc->pager().pagesLaidOut())));
        QTRY_VERIFY_WITH_TIMEOUT(view.pagesComplete(), 60000);
        QVERIFY(view.pageCount() > 100);
        // The page count grew as pages were laid out, and the scroll range with it.
        QVERIFY(view.verticalScrollBar()->maximum() > 100 * 700);
    }

    // A page in the view that shows the number of pages is made again when that number changes: the first page, drawn while the rest is laid out, doesn't
    // keep saying there are a few.
    void aPageShowingTheTotalIsMadeAgainWhenTheTotalChanges()
    {
        const QByteArray sect = "<w:sectPr><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        const QByteArray footer = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">of </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> NUMPAGES </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"});
        auto doc = open(makeDocxWithParts(paragraphs(2000), sect, {{QStringLiteral("/word/footer1.xml"), footer}}));
        QVERIFY(doc);
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        view.grab();   // (the first page is made when only a few pages are known)
        QVERIFY(!view.pagesComplete());
        const int early = view.shownTotal(0);
        QVERIFY2(early > 0 && early < 100, qPrintable(QString::number(early)));
        QTRY_VERIFY_WITH_TIMEOUT(view.pagesComplete(), 60000);
        QVERIFY(view.pageCount() > 100);
        const int now = view.shownTotal(0);
        QVERIFY2(now < 0 || now == view.pageCount(), qPrintable(QStringLiteral("page 1 says %1 pages of %2").arg(now).arg(view.pageCount())));
    }

    void onlyThePagesInViewAreMade()
    {
        auto doc = open(makeDocx(paragraphs(2000), kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        QTRY_VERIFY_WITH_TIMEOUT(view.pagesComplete(), 60000);
        const qint64 before = doc->pager().pagesLaidOut();
        view.goToPage(60);
        view.grab();
        // Reaching page 60 and drawing it laid out no page again but the ones drawn (a page or two), not sixty.
        QVERIFY2(doc->pager().pagesLaidOut() - before < 12, qPrintable(QString::number(doc->pager().pagesLaidOut() - before)));
        QCOMPARE(view.topPage(), 60);
    }

    void zoomRunsFromTenToFiveHundredPercent()
    {
        auto doc = open(makeDocx(paragraphs(400), kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.paginateAll();
        view.setZoom(0.01);
        QCOMPARE(view.zoom(), 0.1);
        view.setZoom(50);
        QCOMPARE(view.zoom(), 5.0);
        view.setZoom(1.0);
        QCOMPARE(view.pageRect(0).width(), 816);   // 8.5 inches at 96 to the inch
        // Page width fits the page between the margins; one page fits it whole; three pages across.
        view.setZoomMode(PageView::ZoomMode::PageWidth);
        const QRect w = view.pageRect(0);
        QVERIFY(w.width() >= view.viewport()->width() - 2 * 24 - 2 && w.width() <= view.viewport()->width() - 2 * 24 + 2);
        view.setZoomMode(PageView::ZoomMode::OnePage);
        const QRect one = view.pageRect(view.topPage());
        QVERIFY(one.height() <= view.viewport()->height() - 2 * 24 + 2 && one.width() <= view.viewport()->width());
        view.setZoomMode(PageView::ZoomMode::ManyPages, 3, 1);
        QCOMPARE(view.pagesAcross(), 3);
        QCOMPARE(view.pageRect(0).top(), view.pageRect(2).top());
        QVERIFY(view.pageRect(1).left() > view.pageRect(0).right());
        QVERIFY(view.pageRect(3).top() > view.pageRect(0).bottom());
    }

    void thePagesAreDrawnWithTheirText()
    {
        auto doc = open(makeDocx("<w:p><w:r><w:rPr><w:sz w:val=\"48\"/></w:rPr><w:t>HELLO</w:t></w:r></w:p>" + paragraphs(1), kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        const QImage img = view.grab().toImage();
        // The gray ground, the white page, and dark text on it.
        const QRect page = view.pageRect(0);
        QVERIFY(page.width() > 700);
        int dark = 0, white = 0;
        for (int y = page.top(); y < std::min(page.bottom(), img.height()); ++y)
            for (int x = page.left(); x < page.right(); ++x) {
                const QColor c = img.pixelColor(x, y);
                if (c.lightness() < 100) ++dark;
                else if (c.lightness() > 250) ++white;
            }
        QVERIFY2(dark > 200, "text is drawn");
        QVERIFY(white > page.width() * page.height() / 2);
        QVERIFY(img.pixelColor(2, 300).lightness() < 240);   // (the ground is not white)
    }

    void theCaretIsDrawnOnThePageThatHoldsIt()
    {
        auto doc = open(makeDocx(paragraphs(120), kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.paginateAll();
        QTextCursor &c = view.cursor();
        c.movePosition(QTextCursor::End);
        view.setCursor(c);
        const int page = view.currentPage();
        QCOMPARE(page, view.pageCount() - 1);
        QRect r;
        QVERIFY(view.caretRect(&r));
        QVERIFY(view.pageRect(page).adjusted(-2, -2, 2, 2).contains(r.center()));
        QVERIFY(view.topPage() >= page - 1);   // (the view scrolled to it)
    }

    void clickingPlacesTheCaretAndDraggingSelects()
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>Alpha beta gamma delta</w:t></w:r></w:p><w:p><w:r><w:t>Second paragraph here</w:t></w:r></w:p>", kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        QRect page = view.pageRect(0);
        // A click at the left edge of the first line puts the caret at the start; one past its end, at the end.
        const QPoint first(page.left() + 72 * 96 / 72 + 2, page.top() + 72 * 96 / 72 + 8);
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, first);
        QCOMPARE(view.cursor().position(), 0);
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(page.right() - 20, first.y()));
        QCOMPARE(view.cursor().position(), int(QStringLiteral("Alpha beta gamma delta").size()));
        // A double click selects a word; a drag from the start to the second paragraph selects across.
        QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(first.x() + 4, first.y()));
        QCOMPARE(view.cursor().selectedText(), QStringLiteral("Alpha"));
        QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, first);
        QTest::mouseMove(view.viewport(), QPoint(first.x() + 40, first.y() + 60));
        QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(first.x() + 40, first.y() + 60));
        QVERIFY(view.cursor().hasSelection());
        QVERIFY(view.cursor().selectionEnd() > 22);
    }

    // The caret is drawn in a cell where the cell's own layout puts it, a click in a cell puts it there, Up and Down go through the cell's lines and on to the cell above
    // or below, and Home and End are to the cell's line.
    void theCaretAndTheMouseWorkInCells()
    {
        auto doc = open(tableDocx(3, 2));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        view.paginateAll();
        QTextDocument *s = doc->story();
        auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
        auto caretAt = [&](const QString &text, int offset) {
            view.cursor().setPosition(block(text).position() + offset);
            view.setCursor(view.cursor());
            QRect r;
            return view.caretRect(&r) ? r : QRect();
        };
        const QRect a = caretAt(QStringLiteral("r0c0"), 0), b = caretAt(QStringLiteral("r0c1"), 0), c = caretAt(QStringLiteral("r1c0"), 0), d = caretAt(QStringLiteral("r1c1"), 2);
        QVERIFY(a.isValid() && b.isValid() && c.isValid() && d.isValid());
        QVERIFY2(b.x() > a.x() + 100, "the second column is to the right of the first");
        QVERIFY2(c.y() > a.y() + 8 && std::abs(c.x() - a.x()) < 2, "the second row is below the first");
        QVERIFY(d.x() > c.x() + 100 && d.y() == c.y());
        QVERIFY(view.pageRect(0).contains(d.center()));
        // A click in a cell: the cell's text, at the place.
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(b.x() + 1, b.y() + b.height() / 2));
        QCOMPARE(view.cursor().block().text(), QStringLiteral("r0c1"));
        QCOMPARE(view.cursor().positionInBlock(), 0);
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(d.x() + 1, d.y() + 4));
        QCOMPARE(view.cursor().block().text(), QStringLiteral("r1c1"));
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(c.x() + 90, c.y() + 4));   // (past the end of "r1c0": its end)
        QCOMPARE(view.cursor().block().text(), QStringLiteral("r1c0"));
        QCOMPARE(view.cursor().positionInBlock(), 4);
        // Up and Down from a cell go to the cell above and below, along the same x.
        caretAt(QStringLiteral("r1c1"), 1);
        view.typeKey(Qt::Key_Up, Qt::NoModifier);
        QCOMPARE(view.cursor().block().text(), QStringLiteral("r0c1"));
        view.typeKey(Qt::Key_Down, Qt::NoModifier);
        QCOMPARE(view.cursor().block().text(), QStringLiteral("r1c1"));
        view.typeKey(Qt::Key_Down, Qt::NoModifier);
        QCOMPARE(view.cursor().block().text(), QStringLiteral("r2c1"));
        // Home and End are in the cell's line.
        view.typeKey(Qt::Key_End, Qt::NoModifier);
        QCOMPARE(view.cursor().positionInBlock(), 4);
        view.typeKey(Qt::Key_Home, Qt::NoModifier);
        QCOMPARE(view.cursor().positionInBlock(), 0);
        // Typing and Tab.
        view.insertText(QStringLiteral("Z"));
        QCOMPARE(view.cursor().block().text(), QStringLiteral("Zr2c1"));
        view.typeKey(Qt::Key_Backtab, Qt::NoModifier);
        QCOMPARE(view.cursor().selectedText(), QStringLiteral("r2c0"));
        // A selection across cells is drawn in them: the cells' text has the selection color somewhere.
        view.cursor().setPosition(block(QStringLiteral("r0c0")).position());
        view.cursor().setPosition(block(QStringLiteral("r1c1")).position() + 4, QTextCursor::KeepAnchor);
        view.setCursor(view.cursor());
        const QImage img = view.pageImage(0, 816);   // (the page as it is printed: the cells' text is in it)
        QVERIFY(!img.isNull());
        const QRect pr = view.pageRect(0);
        int dark = 0;
        for (int y = std::max(0, a.y() - pr.y() - 4); y < std::min(img.height(), d.bottom() - pr.y() + 30); ++y)
            for (int x = std::max(0, a.x() - pr.x() - 2); x < std::min(img.width(), d.right() - pr.x() + 150); ++x) dark += qGray(img.pixel(x, y)) < 128;
        QVERIFY2(dark > 300, qPrintable(QString::number(dark)));
        view.cursor().setPosition(0);
        QRect r;
        QVERIFY(view.caretRect(&r));
    }

    void theKeysMoveTheCaret()
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>one two three</w:t></w:r></w:p><w:p><w:r><w:t>four five</w:t></w:r></w:p><w:p><w:r><w:t>six</w:t></w:r></w:p>", kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.typeKey(Qt::Key_End, Qt::ControlModifier);
        QCOMPARE(view.cursor().position(), doc->story()->characterCount() - 1);
        view.typeKey(Qt::Key_Home, Qt::ControlModifier);
        QCOMPARE(view.cursor().position(), 0);
        view.typeKey(Qt::Key_End, Qt::NoModifier);
        QCOMPARE(view.cursor().position(), 13);
        view.typeKey(Qt::Key_Down, Qt::NoModifier);
        QCOMPARE(view.cursor().blockNumber(), 1);
        view.typeKey(Qt::Key_Down, Qt::NoModifier);
        QCOMPARE(view.cursor().blockNumber(), 2);
        view.typeKey(Qt::Key_Up, Qt::NoModifier);
        QCOMPARE(view.cursor().blockNumber(), 1);
        // ("six" is shorter than the column the caret keeps, so Up went to the end of "four five": Ctrl+Left goes to the start of "five".)
        view.typeKey(Qt::Key_Left, Qt::ControlModifier);
        QCOMPARE(view.cursor().positionInBlock(), 5);
        view.typeKey(Qt::Key_Home, Qt::NoModifier);
        QCOMPARE(view.cursor().positionInBlock(), 0);
        view.typeKey(Qt::Key_Right, Qt::ShiftModifier);
        view.typeKey(Qt::Key_Right, Qt::ShiftModifier);
        QCOMPARE(view.cursor().selectedText(), QStringLiteral("fo"));
        // Up and Down keep the column they started from, across a short line.
        view.cursor().setPosition(doc->story()->findBlockByNumber(0).position() + 11);
        view.typeKey(Qt::Key_Down, Qt::NoModifier);   // "four five" has 9 letters: the end
        QCOMPARE(view.cursor().blockNumber(), 1);
        view.typeKey(Qt::Key_Down, Qt::NoModifier);   // "six" has 3
        QCOMPARE(view.cursor().blockNumber(), 2);
        view.typeKey(Qt::Key_Up, Qt::NoModifier);
        view.typeKey(Qt::Key_Up, Qt::NoModifier);
        QCOMPARE(view.cursor().positionInBlock(), 11);   // back where it began
    }

    void pageUpAndPageDownMoveByAWindow()
    {
        auto doc = open(makeDocx(paragraphs(200), kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        view.typeKey(Qt::Key_PageDown, Qt::NoModifier);
        const int a = view.cursor().position();
        QVERIFY(a > 500);
        view.typeKey(Qt::Key_PageDown, Qt::NoModifier);
        QVERIFY(view.cursor().position() > a);
        view.typeKey(Qt::Key_PageUp, Qt::NoModifier);
        QVERIFY(view.cursor().position() <= a + 40);
    }

    void upAndDownCrossFromPageToPage()
    {
        auto doc = open(makeDocx(paragraphs(100), kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.paginateAll();
        // To the last line of the first page, then one down: the second page.
        view.cursor().setPosition(doc->pager().page(1).start.pos - 1);
        QCOMPARE(view.currentPage(), 0);
        for (int i = 0; i < 3 && view.currentPage() == 0; ++i) view.typeKey(Qt::Key_Down, Qt::NoModifier);
        QCOMPARE(view.currentPage(), 1);
        for (int i = 0; i < 3 && view.currentPage() == 1; ++i) view.typeKey(Qt::Key_Up, Qt::NoModifier);
        QCOMPARE(view.currentPage(), 0);
    }

    void typingInTheWindowReachesTheDocument()
    {
        WriteWindow w;
        w.show();
        w.view()->setFocus();
        QTest::keyClicks(w.view(), QStringLiteral("Hello"));
        QTest::keyClick(w.view(), Qt::Key_Return);
        QTest::keyClicks(w.view(), QStringLiteral("World"));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\nWorld"));
        QVERIFY(w.isModified());
        QVERIFY(w.windowTitle().contains(QLatin1Char('*')));
        QTest::keyClick(w.view(), Qt::Key_Backspace);
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\nWorl"));
        QTest::keyClick(w.view(), Qt::Key_Home, Qt::ShiftModifier);
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("Worl"));
        QTest::keyClick(w.view(), Qt::Key_Delete);
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\n"));
        // Undo and Redo from the ribbon's own commands.
        QTRY_VERIFY(w.act("edit.undo")->isEnabled());   // (the commands follow the text a moment after it changes)
        w.act("edit.undo")->trigger();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\nWorl"));
        QTRY_VERIFY(w.act("edit.redo")->isEnabled());
        w.act("edit.redo")->trigger();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\n"));
    }

    // ---------------------------------------------------------------- spelling
    void spellCheckFindsTheMisspelledWordsOfTheStory()
    {
        const SettingGuard upper(QStringLiteral("proof/ignoreUpper"), true, true), numbers(QStringLiteral("proof/ignoreNumbers"), true, true);
        auto doc = textDoc({"The quick brwon fox jumpd over teh lazy dog.", "A second paragraph, correctly spelled.", "NASA and abc123 are let go, so are the 3rd and 21st, but Teh is not."});
        SpellCheck sc(doc->story());
        QCOMPARE(flaggedWords(sc, *doc), (QStringList{"brwon", "jumpd", "teh", "Teh"}));
        // Switched off, it finds nothing; on again, the same.
        sc.setEnabled(false);
        QVERIFY(flaggedWords(sc, *doc).isEmpty());
        sc.setEnabled(true);
        QCOMPARE(flaggedWords(sc, *doc).size(), 4);
    }

    void spellCheckKeepsItsAnswerForEachParagraph()
    {
        QStringList texts;
        for (int i = 0; i < 100; ++i) texts << QStringLiteral("Paragraph number %1 has a misspeled word in it.").arg(i);
        auto doc = textDoc(texts);
        QTextDocument *story = doc->story();
        SpellCheck sc(story);
        const QTextBlock third = story->findBlockByNumber(3);
        QCOMPARE(sc.misspelledIn(third.position(), third.position() + third.length()).size(), 1);
        QCOMPARE(sc.paragraphsChecked(), qint64(1));
        sc.misspelledIn(third.position(), third.position() + third.length());
        QCOMPARE(sc.paragraphsChecked(), qint64(1));   // (kept)
        const int end = story->characterCount();
        QCOMPARE(sc.misspelledIn(0, end).size(), 100);
        QCOMPARE(sc.paragraphsChecked(), qint64(100));
        // An edit in one paragraph makes only that one checked again, and the others move without being checked.
        QTextCursor c(story);
        c.setPosition(third.position() + 9);   // (the end of "Paragraph")
        insertText(*doc, c, QStringLiteral("s"));
        QCOMPARE(sc.misspelledIn(0, story->characterCount()).size(), 100);
        QCOMPARE(sc.paragraphsChecked(), qint64(101));
        c.setPosition(0);
        insertParagraph(*doc, c);
        QCOMPARE(sc.misspelledIn(0, story->characterCount()).size(), 100);
        QVERIFY2(sc.paragraphsChecked() <= 103, "a paragraph cut in two is checked, not the rest");
        // Undo brings the old text back; the answers for it are made again, and right.
        story->undo();
        story->undo();
        QCOMPARE(sc.misspelledIn(0, story->characterCount()).size(), 100);
    }

    void spellCheckGivesWayWhenThePassIsOutOfTimeAndFinishesInIdleTime()
    {
        QStringList texts;
        for (int i = 0; i < 100; ++i) texts << QStringLiteral("Paragraph %1 has a misspeled word.").arg(i);
        auto doc = textDoc(texts);
        SpellCheck sc(doc->story());
        QSignalSpy changed(&sc, &SpellCheck::changed);
        const int end = doc->story()->characterCount();
        QVector<QPair<int, int>> some;
        {
            SpellCheck::Pass pass(sc, 0);
            some = sc.misspelledIn(0, end);
        }
        QVERIFY(some.size() < 100);
        QVERIFY(sc.paragraphsChecked() < 100);
        QVERIFY(sc.pending());
        // The rest is done when nothing else is: the event loop's idle time, and the view is told.
        QTRY_VERIFY(!sc.pending());
        QVERIFY(changed.count() > 0);
        QCOMPARE(sc.paragraphsChecked(), qint64(100));
        QCOMPARE(sc.misspelledIn(0, end).size(), 100);
        QCOMPARE(sc.paragraphsChecked(), qint64(100));
    }

    void spellCheckFollowsTheLanguageOfTheText()
    {
        // "colour" is right in British English and wrong in American.
        const QByteArray body = "<w:p><w:r><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr><w:t xml:space=\"preserve\">colour </w:t></w:r><w:r><w:t>colour</w:t></w:r></w:p>"
                                "<w:p><w:pPr><w:pStyle w:val=\"British\"/></w:pPr><w:r><w:t xml:space=\"preserve\">colour </w:t></w:r><w:r><w:rPr><w:lang w:val=\"en-US\"/></w:rPr><w:t>colour</w:t></w:r></w:p>"
                                "<w:p><w:r><w:rPr><w:lang w:val=\"tlh-KX\"/></w:rPr><w:t>qapla brwon</w:t></w:r></w:p>"
                                "<w:p><w:r><w:rPr><w:noProof/></w:rPr><w:t>brwon</w:t></w:r></w:p>";
        const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"British\"><w:name w:val=\"British\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr></w:style>";
        auto doc = open(makeDocx(body, styles));
        SpellCheck sc(doc->story());
        sc.setDefaultLanguage(doc->proofingLanguage());
        const QString text = doc->story()->toPlainText();
        QStringList found;
        for (const auto &r : sc.misspelledIn(0, doc->story()->characterCount())) found << QStringLiteral("%1:%2").arg(doc->story()->findBlock(r.first).blockNumber()).arg(text.mid(r.first, r.second - r.first));
        // A run's own language, else its style's, else the document's (American): the second "colour" of the first paragraph, and the one said American in the second.
        // A language with no dictionary, and text marked "do not check", is let alone.
        QCOMPARE(found, (QStringList{"0:colour", "1:colour"}));
        // The document's own default (docDefaults) is what text that says none is: here British.
        QByteArray british = kStyles;
        british.replace("<w:sz w:val=\"23\"/></w:rPr></w:rPrDefault>", "<w:sz w:val=\"23\"/><w:lang w:val=\"en-GB\"/></w:rPr></w:rPrDefault>");
        auto doc2 = textDoc({"colour and brwon"}, british);
        QCOMPARE(doc2->proofingLanguage(), QStringLiteral("en-GB"));
        SpellCheck sc2(doc2->story());
        sc2.setDefaultLanguage(doc2->proofingLanguage());
        QCOMPARE(flaggedWords(sc2, *doc2), (QStringList{"brwon"}));
        // The file still says what it said.
        const QByteArray saved = documentXml(doc->toDocx());
        QVERIFY(saved.contains("<w:lang w:val=\"en-GB\"/>"));
        QVERIFY(saved.contains("<w:lang w:val=\"tlh-KX\"/>"));
    }

    void theIgnoreListAndTheUserDictionaryChangeTheAnswers()
    {
        auto doc = textDoc({"The zorbifyx and the quixzotl are strange, but vlorpish is stranger."});
        SpellCheck sc(doc->story());
        QCOMPARE(flaggedWords(sc, *doc), (QStringList{"zorbifyx", "quixzotl", "vlorpish"}));
        spellingIgnore(QStringLiteral("zorbifyx"));
        QCOMPARE(flaggedWords(sc, *doc), (QStringList{"quixzotl", "vlorpish"}));
        // (Added words go in a file of the test's own.)
        QStandardPaths::setTestModeEnabled(true);
        spellingAdd(QStringLiteral("quixzotl"));
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/custom.dic"));
        QStandardPaths::setTestModeEnabled(false);
        QCOMPARE(flaggedWords(sc, *doc), (QStringList{"vlorpish"}));
    }

    void misspellingsAreFoundByPositionAndInOrder()
    {
        auto doc = textDoc({"One brwon fox", "and teh dog"});
        SpellCheck sc(doc->story());
        const auto first = sc.nextMisspelling(0);
        QCOMPARE(first, qMakePair(4, 9));
        // At or after a position: from inside the word, the word after it.
        const auto second = sc.nextMisspelling(first.second);
        QCOMPARE(doc->story()->toPlainText().mid(second.first, second.second - second.first), QStringLiteral("teh"));
        QCOMPARE(sc.nextMisspelling(second.second), qMakePair(-1, -1));
        // The word a position is in, or at the end of; not the one after the space.
        QCOMPARE(sc.misspellingAt(4), first);
        QCOMPARE(sc.misspellingAt(7), first);
        QCOMPARE(sc.misspellingAt(9), first);
        QCOMPARE(sc.misspellingAt(10), qMakePair(-1, -1));
        QCOMPARE(sc.misspellingAt(0), qMakePair(-1, -1));
    }

    void misspelledWordsAreUnderlinedInRedOnTheScreenAndNotInTheProof()
    {
        auto doc = textDoc({"Alpha brwon gamma and a second line of words"});
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        // Where the word is on the page: the caret at its start and at its end.
        QTextCursor &c = view.cursor();
        c.setPosition(6);
        view.setCursor(c);
        QRect start;
        QVERIFY(view.caretRect(&start));
        c.setPosition(11);
        view.setCursor(c);
        QRect end;
        QVERIFY(view.caretRect(&end));
        c.setPosition(0);
        view.setCursor(c);
        // (Checked as the page is drawn, or a moment after.)
        int count = 0;
        QRect wave;
        QTRY_VERIFY((wave = redBounds(view.grab().toImage(), view.pageRect(0), &count), count > 8));
        // Under the word and nowhere else, just below the line of its letters.
        QVERIFY2(wave.left() >= start.left() - 3 && wave.right() <= end.left() + 3, qPrintable(QStringLiteral("%1..%2 for the word at %3..%4").arg(wave.left()).arg(wave.right()).arg(start.left()).arg(end.left())));
        QVERIFY(wave.top() >= start.top() && wave.bottom() <= start.bottom() + 8);
        // As printed, and in the proof the picture of a page makes, there is none.
        redBounds(view.pageImage(0, 900), QRect(0, 0, 900, 1200), &count);
        QCOMPARE(count, 0);
        // Switched off, it goes.
        view.setSpellingEnabled(false);
        redBounds(view.grab().toImage(), view.pageRect(0), &count);
        QCOMPARE(count, 0);
        view.setSpellingEnabled(true);
        QTRY_VERIFY((redBounds(view.grab().toImage(), view.pageRect(0), &count), count > 8));
    }

    void aWordStillBeingTypedIsNotUnderlinedUntilTheCaretMovesOn()
    {
        auto doc = textDoc({""});
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        int count = 0;
        for (const QChar ch : QStringLiteral("recieve")) view.insertText(QString(ch));
        redBounds(view.grab().toImage(), view.pageRect(0), &count);
        QCOMPARE(count, 0);
        QCoreApplication::processEvents();   // (what idle time would do)
        QCoreApplication::processEvents();
        redBounds(view.grab().toImage(), view.pageRect(0), &count);
        QCOMPARE(count, 0);
        view.insertText(QStringLiteral(" "));
        QTRY_VERIFY((redBounds(view.grab().toImage(), view.pageRect(0), &count), count > 3));   
    }

    void onlyTheParagraphsOnThePagesInViewAreChecked()
    {
        auto doc = open(makeDocx(paragraphs(150), kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        view.grab();
        SpellCheck *spelling = view.spelling();
        QVERIFY(spelling);
        view.paginateAll();
        QTRY_VERIFY(!spelling->pending());
        const qint64 first = spelling->paragraphsChecked();
        QVERIFY2(first > 0 && first < 40, qPrintable(QString::number(first)));
        // A page further on is checked when it is drawn; the rest of the document, never.
        view.goToPage(4);
        view.grab();
        QTRY_VERIFY(!spelling->pending());
        QVERIFY(spelling->paragraphsChecked() > first);
        QVERIFY(spelling->paragraphsChecked() < 100);
        // A typed character: that paragraph is checked again, and no other.
        view.goToPage(0);
        view.grab();
        const qint64 before = spelling->paragraphsChecked();
        QTextCursor &c = view.cursor();
        c.setPosition(3);
        view.setCursor(c);
        view.insertText(QStringLiteral("x"));
        view.grab();
        QTRY_VERIFY(!spelling->pending());
        QCOMPARE(spelling->paragraphsChecked(), before + 1);
    }

    void theSpellingMenuOffersSuggestionsIgnoreAllAndAddToDictionary()
    {
        auto doc = textDoc({"This is adress test, and this is fine."});
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        QTextCursor &c = view.cursor();
        // Off a misspelled word: nothing is added.
        c.setPosition(2);
        view.setCursor(c);
        QMenu none;
        QVERIFY(!addSpellingSuggestions(none, &view));
        QVERIFY(none.isEmpty());
        // On one (inside it, and at its end): the dictionary's first suggestions, then Ignore All and Add to Dictionary.
        for (int at : {10, 14}) {
            c.setPosition(at);
            view.setCursor(c);
            QMenu menu;
            QVERIFY(addSpellingSuggestions(menu, &view));
            QStringList texts;
            for (QAction *a : menu.actions()) texts << a->text();
            const QStringList wanted = spellingSuggestions(QStringLiteral("adress")).mid(0, 6);
            QVERIFY(!wanted.isEmpty());
            QCOMPARE(texts.mid(0, wanted.size()), wanted);
            QCOMPARE(texts[wanted.size()], QStringLiteral("Ignore All"));
            QCOMPARE(texts[wanted.size() + 1], QStringLiteral("Add to Dictionary"));
            QVERIFY(menu.actions().first()->font().bold());
        }
        // Choosing a suggestion changes the word, in one step that undoes.
        c.setPosition(10);
        view.setCursor(c);
        QMenu menu;
        QVERIFY(addSpellingSuggestions(menu, &view));
        const QString chosen = menu.actions().first()->text();
        menu.actions().first()->trigger();
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("This is %1 test, and this is fine.").arg(chosen));
        doc->story()->undo();
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("This is adress test, and this is fine."));
        // Ignore All lets the word go for the session, and the pages are drawn again.
        QMenu again;
        QVERIFY(addSpellingSuggestions(again, &view));
        QSignalSpy changed(view.spelling(), &SpellCheck::changed);
        for (QAction *a : again.actions())
            if (a->text() == QStringLiteral("Ignore All")) a->trigger();
        QCOMPARE(view.spelling()->misspellingAt(10), qMakePair(-1, -1));
        QVERIFY(changed.count() > 0);
    }

    void rightClickingAMisspelledWordInTheWindowShowsItsSuggestions()
    {
        const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
        WriteWindow w;
        w.setDocument(textDoc({"A word that is wrongg here."}), QString());
        w.show();
        QTextCursor &c = w.view()->cursor();
        c.setPosition(17);
        w.view()->setCursor(c);
        QStringList texts;
        QTimer::singleShot(100, [&] {
            if (auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                for (QAction *a : m->actions()) texts << a->text();
                m->close();
            }
        });
        Q_EMIT w.view()->contextMenuWanted(w.view()->mapToGlobal(QPoint(300, 300)));
        QVERIFY2(texts.contains(QStringLiteral("Ignore All")) && texts.contains(QStringLiteral("Add to Dictionary")), qPrintable(texts.join(QLatin1Char('|'))));
        QVERIFY(texts.indexOf(QStringLiteral("Ignore All")) > 0);   // (the suggestions come first, then the usual commands)
        QVERIFY(texts.size() > texts.indexOf(QStringLiteral("Add to Dictionary")) + 3);
    }

    void reviewSpellingStepsThroughTheMisspellingsFromTheCaret()
    {
        const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
        WriteWindow w;
        w.setDocument(textDoc({"First wrongg word, and wrongg again.", "And a secnd one, plus fourh and then bluh."}), QString());
        w.show();
        QCOMPARE(w.act("review.spelling")->shortcut(), QKeySequence(Qt::Key_F7));
        w.act("review.spelling")->trigger();
        auto *pane = w.findChild<SpellPane *>();
        QVERIFY(pane && pane->isVisible());
        auto button = [&](const char *name) { return pane->findChild<QPushButton *>(QString::fromLatin1(name)); };
        auto *suggestions = pane->findChild<QListWidget *>(QStringLiteral("spellSuggestions"));
        QVERIFY(button("spellChange") && button("spellChangeAll") && button("spellIgnore") && button("spellIgnoreAll") && button("spellAdd") && suggestions);
        // The first misspelled word from the caret is shown, selected in the text, with the dictionary's suggestions.
        QCOMPARE(pane->word(), QStringLiteral("wrongg"));
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("wrongg"));
        QVERIFY(suggestions->count() > 0);
        QCOMPARE(suggestions->currentRow(), 0);
        QCOMPARE(pane->findChild<QLabel *>(QStringLiteral("spellWord"))->text(), QStringLiteral("wrongg"));
        // Change All puts the chosen word in place of every one like it, in one step that undoes.
        const QString chosen = suggestions->currentItem()->text();
        button("spellChangeAll")->click();
        QCOMPARE(w.document()->story()->toPlainText().section(QLatin1Char('\n'), 0, 0), QStringLiteral("First %1 word, and %1 again.").arg(chosen));
        QCOMPARE(pane->word(), QStringLiteral("secnd"));
        w.document()->story()->undo();
        QCOMPARE(w.document()->story()->toPlainText().section(QLatin1Char('\n'), 0, 0), QStringLiteral("First wrongg word, and wrongg again."));   // (both come back)
        w.document()->story()->redo();
        // Ignore passes this one by; Change takes the chosen suggestion; the pane goes on.
        button("spellIgnore")->click();
        QCOMPARE(pane->word(), QStringLiteral("fourh"));
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("fourh"));
        suggestions->setCurrentRow(0);
        const QString fix = suggestions->currentItem()->text();
        button("spellChange")->click();
        QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("plus %1 and then").arg(fix)));
        QCOMPARE(pane->word(), QStringLiteral("bluh"));
        // Ignore All, and Add to Dictionary, take the word out for good; when none is left the check is complete.
        QStandardPaths::setTestModeEnabled(true);
        button("spellAdd")->click();
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/custom.dic"));
        QStandardPaths::setTestModeEnabled(false);
        QCOMPARE(pane->word(), QString());
        QVERIFY(pane->findChild<QLabel *>(QStringLiteral("spellStatus"))->text().contains(QStringLiteral("complete")));
        QVERIFY(!button("spellChange")->isEnabled() && !button("spellIgnore")->isEnabled());
        // (The check came back to where it began: the one left by Ignore is still marked on the page.)
        QCOMPARE(w.view()->spelling()->misspellingAt(w.document()->story()->toPlainText().indexOf(QStringLiteral("secnd"))).first, w.document()->story()->toPlainText().indexOf(QStringLiteral("secnd")));
    }

    void reviewSpellingBeginsAtTheCaretAndComesBackToWhereItBegan()
    {
        const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
        WriteWindow w;
        w.setDocument(textDoc({"Zzalpha is first.", "Then zzbeta follows.", "And zzgamma last."}), QString());
        w.show();
        const QString text = w.document()->story()->toPlainText();
        QTextCursor &c = w.view()->cursor();
        c.setPosition(text.indexOf(QStringLiteral("zzbeta")));
        w.view()->setCursor(c);
        w.act("review.spelling")->trigger();
        auto *pane = w.findChild<SpellPane *>();
        QVERIFY(pane);
        auto ignore = [&] { pane->findChild<QPushButton *>(QStringLiteral("spellIgnore"))->click(); };
        QCOMPARE(pane->word(), QStringLiteral("zzbeta"));
        ignore();
        QCOMPARE(pane->word(), QStringLiteral("zzgamma"));
        ignore();
        QCOMPARE(pane->word(), QStringLiteral("Zzalpha"));   // (from the top)
        ignore();
        QCOMPARE(pane->word(), QString());
        QVERIFY(pane->complete());
        // Ignore All on the last one of a document with one left.
        w.setDocument(textDoc({"Only zzlonely here."}), QString());
        w.act("review.spelling")->trigger();
        QCOMPARE(w.findChild<SpellPane *>()->word(), QStringLiteral("zzlonely"));
        w.findChild<SpellPane *>()->findChild<QPushButton *>(QStringLiteral("spellIgnoreAll"))->click();
        QVERIFY(w.findChild<SpellPane *>()->complete());
        QCOMPARE(w.view()->spelling()->misspellingAt(6), qMakePair(-1, -1));
    }

    void theSpellingPaneNeverHoldsTheWindowForAWholeDocument()
    {
        const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
        WriteWindow w;
        w.setDocument(open(makeDocx(paragraphs(1500) + "<w:p><w:r><w:t>and one recieve at the very end</w:t></w:r></w:p>", kStyles)), QString());
        w.show();
        w.act("review.spelling")->trigger();   // (back at once, with what a slice of time could check)
        auto *pane = w.findChild<SpellPane *>();
        QVERIFY(pane);
        QVERIFY(pane->searching());
        QVERIFY(!pane->complete());
        QVERIFY(pane->findChild<QLabel *>(QStringLiteral("spellStatus"))->text().contains(QStringLiteral("Checking")));
        QVERIFY(!pane->findChild<QPushButton *>(QStringLiteral("spellIgnore"))->isEnabled());
        // The window goes on working, and the pane gets to the word.
        QTRY_COMPARE_WITH_TIMEOUT(pane->word(), QStringLiteral("recieve"), 30000);
        QVERIFY(!pane->searching());
        QVERIFY(pane->findChild<QPushButton *>(QStringLiteral("spellIgnore"))->isEnabled());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("recieve"));
    }

    void changeAllGoesOnFromTheWordWhateverTheEarlierOnesChangedBy()
    {
        const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
        WriteWindow w;
        w.setDocument(textDoc({"wrongg wrongg wrongg", "zztwo wrongg zzthree"}), QString());
        w.show();
        QTextCursor &c = w.view()->cursor();
        c.setPosition(w.document()->story()->findBlockByNumber(1).position());
        w.view()->setCursor(c);
        w.act("review.spelling")->trigger();
        auto *pane = w.findChild<SpellPane *>();
        auto click = [&](const char *name) { pane->findChild<QPushButton *>(QString::fromLatin1(name))->click(); };
        QCOMPARE(pane->word(), QStringLiteral("zztwo"));
        click("spellIgnore");
        QCOMPARE(pane->word(), QStringLiteral("wrongg"));
        auto *suggestions = pane->findChild<QListWidget *>(QStringLiteral("spellSuggestions"));
        QVERIFY(suggestions->count() > 0);
        const QString chosen = suggestions->item(0)->text();
        QVERIFY2(chosen.size() != 6, "a suggestion of another length shifts what follows");
        click("spellChangeAll");
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("%1 %1 %1\nzztwo %1 zzthree").arg(chosen));
        // The next is the one just after the word, though the three before it changed length; then the check ends where it began, not at "zztwo" again.
        QCOMPARE(pane->word(), QStringLiteral("zzthree"));
        click("spellIgnore");
        QVERIFY2(pane->complete(), qPrintable(pane->word()));
    }

    void checkingSpellingAsYouTypeCanBeTurnedOffInTheReviewTab()
    {
        const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);   // (what the test begins with, and what the person had comes back)
        WriteWindow w;
        w.setDocument(textDoc({"A recieve here."}), QString());
        w.show();
        QAction *action = w.act("review.checkAsType");
        QVERIFY(action->isCheckable() && action->isChecked());
        QVERIFY(w.view()->spellingEnabled());
        action->trigger();
        QVERIFY(!action->isChecked());
        QVERIFY(!w.view()->spellingEnabled());
        int count = 0;
        redBounds(w.view()->grab().toImage(), w.view()->pageRect(0), &count);
        QCOMPARE(count, 0);
        // A window opened after remembers the choice.
        WriteWindow other;
        QVERIFY(!other.view()->spellingEnabled());
        action->trigger();
        QVERIFY(w.view()->spellingEnabled());
        WriteWindow third;
        QVERIFY(third.view()->spellingEnabled());
    }

    void aStyleThatSetsALanguageGivesItToTheTextAndTakesItAway()
    {
        const QByteArray styles = kStyles +
                                  "<w:style w:type=\"paragraph\" w:styleId=\"British\"><w:name w:val=\"British\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr></w:style>"
                                  "<w:style w:type=\"character\" w:styleId=\"BritishChar\"><w:name w:val=\"BritishChar\"/><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr></w:style>";
        const QByteArray body = "<w:p><w:r><w:t>colour</w:t></w:r></w:p><w:p><w:r><w:t>colour</w:t></w:r></w:p><w:p><w:r><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr><w:t>colour</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, styles));
        QTextDocument *story = doc->story();
        SpellCheck sc(story);
        sc.setDefaultLanguage(doc->proofingLanguage());
        auto flagged = [&] {
            QList<int> blocks;
            for (const auto &r : sc.misspelledIn(0, story->characterCount())) blocks << story->findBlock(r.first).blockNumber();
            return blocks;
        };
        QCOMPARE(flagged(), (QList<int>{0, 1}));
        // A paragraph style that says British gives it to the paragraph's text; another style takes it away again. What a run says itself stays.
        QTextCursor c(story);
        c.setPosition(story->findBlockByNumber(0).position());
        applyParagraphStyle(*doc, c, QStringLiteral("British"));
        QCOMPARE(flagged(), (QList<int>{1}));
        applyParagraphStyle(*doc, c, QStringLiteral("Normal"));
        QCOMPARE(flagged(), (QList<int>{0, 1}));
        c.setPosition(story->findBlockByNumber(2).position());
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        QCOMPARE(flagged(), (QList<int>{0, 1}));   // (its own British stays)
        applyParagraphStyle(*doc, c, QStringLiteral("Normal"));
        // A character style does the same for the letters it is given to.
        c.setPosition(story->findBlockByNumber(1).position());
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        applyCharacterStyle(*doc, c, QStringLiteral("BritishChar"));
        QCOMPARE(flagged(), (QList<int>{0}));
        // Clearing the formatting leaves a run's own language, as the file keeps it.
        c.setPosition(story->findBlockByNumber(2).position());
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        clearFormatting(*doc, c);
        QCOMPARE(flagged(), (QList<int>{0}));
        QVERIFY(documentXml(doc->toDocx()).contains("<w:lang w:val=\"en-GB\"/>"));
    }

    void searchingForMisspellingsGoesInSlicesOfBoundedWork()
    {
        QStringList texts;
        for (int i = 0; i < 400; ++i) texts << (i == 300 ? QStringLiteral("One recieve here.") : QStringLiteral("Paragraph %1 is correct and clean.").arg(i));
        auto doc = textDoc(texts);
        QTextDocument *story = doc->story();
        SpellCheck sc(story);
        int from = 0, slices = 0, next = 0;
        QPair<int, int> found{-1, -1};
        qint64 before = sc.paragraphsChecked();
        SpellCheck::Step step;
        while ((step = sc.search(from, 0, &found, &next)) == SpellCheck::Step::More) {
            ++slices;
            // (A slice with no time to spend does the work of one paragraph, so that the search goes on.)
            QCOMPARE(sc.paragraphsChecked() - before, qint64(1));
            before = sc.paragraphsChecked();
            QVERIFY(next > from);
            from = next;
            QVERIFY(slices < 1000);
        }
        QCOMPARE(step, SpellCheck::Step::Found);
        QCOMPARE(slices, 300);
        QCOMPARE(story->toPlainText().mid(found.first, found.second - found.first), QStringLiteral("recieve"));
        // On from the word: the rest of the story has none, and the search says so.
        from = found.second;
        while ((step = sc.search(from, 0, &found, &next)) == SpellCheck::Step::More) from = next;
        QCOMPARE(step, SpellCheck::Step::None);
        // What was checked is kept: the whole story again takes one slice and no checking.
        before = sc.paragraphsChecked();
        QCOMPARE(sc.search(0, 0, &found, &next), SpellCheck::Step::Found);
        QCOMPARE(sc.search(found.second, 0, &found, &next), SpellCheck::Step::None);
        QCOMPARE(sc.paragraphsChecked(), before);
    }

    void aLongParagraphIsCheckedInPiecesOfBoundedWork()
    {
        QString text;
        for (int i = 0; i < 30000; ++i) text += (i % 1000 == 999 ? QStringLiteral("recieve ") : QStringLiteral("word "));
        auto doc = textDoc({text});
        QTextDocument *story = doc->story();
        SpellCheck sc(story);
        const int end = story->characterCount();
        {
            SpellCheck::Pass pass(sc, 0);
            sc.misspelledIn(0, end);
        }
        // A pass with no time does one piece of it, not the whole; the rest is for idle time.
        QVERIFY2(sc.wordsChecked() > 0 && sc.wordsChecked() < 3000, qPrintable(QString::number(sc.wordsChecked())));
        QVERIFY(sc.pending());
        QTRY_VERIFY(!sc.pending());
        QCOMPARE(sc.wordsChecked(), qint64(30000));   // (each word once, though it took pieces)
        QCOMPARE(sc.misspelledIn(0, end).size(), 30);
        QCOMPARE(sc.paragraphsChecked(), qint64(1));
        // A typed character in it: the same, one piece within the pass.
        QTextCursor c(story);
        c.setPosition(5);
        insertText(*doc, c, QStringLiteral("s"));
        const qint64 before = sc.wordsChecked();
        {
            SpellCheck::Pass pass(sc, 0);
            sc.misspelledIn(0, story->characterCount());
        }
        QVERIFY2(sc.wordsChecked() - before > 0 && sc.wordsChecked() - before < 3000, qPrintable(QString::number(sc.wordsChecked() - before)));
        QTRY_VERIFY(!sc.pending());
        QCOMPARE(sc.misspelledIn(0, story->characterCount()).size(), 30);
        QCOMPARE(sc.paragraphsChecked(), qint64(2));
        // The search, too, goes through it in pieces: a misspelling near the start is found without checking the rest.
        QTextCursor d(story);
        d.setPosition(7);
        insertText(*doc, d, QStringLiteral("s"));   // ("swsord")
        const qint64 beforeSearch = sc.wordsChecked();
        QPair<int, int> found;
        int next = 0;
        QCOMPARE(sc.search(0, 0, &found, &next), SpellCheck::Step::Found);
        QVERIFY2(sc.wordsChecked() - beforeSearch < 3000, qPrintable(QString::number(sc.wordsChecked() - beforeSearch)));
        QCOMPARE(story->toPlainText().mid(found.first, found.second - found.first), QStringLiteral("swsord"));
    }

    void theSpellingCacheDoesNotKeepTheTextOfTheDocument()
    {
        QStringList texts;
        for (int i = 0; i < 2000; ++i) texts << QStringLiteral("Paragraph number %1 is made of ordinary words that are all spelled right, and it goes on for a while so that it is long enough to count.").arg(i);
        auto doc = textDoc(texts);
        SpellCheck sc(doc->story());
        sc.misspelledIn(0, doc->story()->characterCount());
        QCOMPARE(sc.paragraphsChecked(), qint64(2000));
        const qint64 textBytes = qint64(doc->story()->characterCount()) * 2;
        QVERIFY2(sc.keptBytes() * 2 < textBytes, qPrintable(QStringLiteral("%1 bytes kept for %2 bytes of text").arg(sc.keptBytes()).arg(textBytes)));
    }

    void theUserDictionaryIsReadAndWrittenSafely()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("custom.dic"));
        auto put = [&](const QByteArray &bytes) {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            f.write(bytes);
        };
        auto get = [&] {
            QFile f(path);
            return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
        };
        // A file that doesn't end with a newline: the next word begins a line of its own.
        put("alpha\nbeta");
        QString error;
        QVERIFY(addToUserDictionary(path, QStringLiteral("gamma"), &error));
        QCOMPARE(get(), QByteArray("alpha\nbeta\ngamma\n"));
        QCOMPARE(userDictionaryWords(path), (QStringList{"alpha", "beta", "gamma"}));
        // A word that is empty or has a line in it is not written, so that no two words are made of one.
        QVERIFY(!addToUserDictionary(path, QStringLiteral("  "), &error));
        QVERIFY(!addToUserDictionary(path, QStringLiteral("two\nwords"), &error));
        QCOMPARE(get(), QByteArray("alpha\nbeta\ngamma\n"));
        // A byte order mark, Windows line ends, blanks, and spaces around a word.
        put("\xEF\xBB\xBF" "one\r\n\r\n  two \r\nthree");
        QCOMPARE(userDictionaryWords(path), (QStringList{"one", "two", "three"}));
        // The read is bounded: a file of a great many words is read as far as the limit.
        QByteArray big;
        for (int i = 0; i < 400000; ++i) big += "word" + QByteArray::number(i) + "\n";
        put(big);
        const int words = int(userDictionaryWords(path).size());
        QVERIFY2(words > 10000 && words < 400000, qPrintable(QString::number(words)));
        // A file that can't be written is said, with the reason.
        QVERIFY(!addToUserDictionary(dir.filePath(QStringLiteral("none/custom.dic")), QStringLiteral("x"), &error));
        QVERIFY(!error.isEmpty());
        QVERIFY(userDictionaryWords(dir.filePath(QStringLiteral("none/custom.dic"))).isEmpty());
    }

    void addingAWordThatCantBeSavedStillCountsAndIsSaidOnce()
    {
        QTemporaryDir dir;
        struct Restore {
            ~Restore() { Speller::setUserDictionaryPath(QString()); }
        } restore;
        // (The path is a folder, so nothing can be written there.)
        Speller::setUserDictionaryPath(dir.path());
        QString first, second;
        QVERIFY(!spellingAdd(QStringLiteral("qzxfailone"), &first));
        QVERIFY(!first.isEmpty());
        QVERIFY(!spellingAdd(QStringLiteral("qzxfailtwo"), &second));
        QVERIFY(second.isEmpty());   // (said once)
        QVERIFY(Speller::get().check(QStringLiteral("en-US"), QStringLiteral("qzxfailone")));   // (the word is accepted for the session all the same)
        // When a save works the next failure is news again.
        Speller::setUserDictionaryPath(dir.filePath(QStringLiteral("custom.dic")));
        QVERIFY(spellingAdd(QStringLiteral("qzxsaved")));
        QVERIFY(userDictionaryWords(dir.filePath(QStringLiteral("custom.dic"))).contains(QStringLiteral("qzxsaved")));
        Speller::setUserDictionaryPath(dir.path());
        QString third;
        QVERIFY(!spellingAdd(QStringLiteral("qzxfailthree"), &third));
        QVERIFY(!third.isEmpty());
    }

    void rightClickingInsideASelectionOffersTheWordClickedNotTheCaret()
    {
        auto doc = textDoc({"First line has recieve in it and more text after it."});
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        QTextCursor &c = view.cursor();
        const int word = doc->story()->toPlainText().indexOf(QStringLiteral("recieve"));
        c.setPosition(word + 2);
        view.setCursor(c);
        QRect r;
        QVERIFY(view.caretRect(&r));
        // The paragraph is selected, the caret at its end, and the click is on the word.
        c.setPosition(0);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        view.setCursor(c);
        const int selected = int(view.cursor().selectedText().size());
        QTest::mousePress(view.viewport(), Qt::RightButton, Qt::NoModifier, QPoint(r.x() + 1, r.y() + r.height() / 2));
        QCOMPARE(int(view.cursor().selectedText().size()), selected);   // (the selection stays)
        QMenu menu;
        QVERIFY(addSpellingSuggestions(menu, &view));
        const QString chosen = menu.actions().first()->text();
        menu.actions().first()->trigger();
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("First line has %1 in it and more text after it.").arg(chosen));
        QTest::mouseRelease(view.viewport(), Qt::RightButton, Qt::NoModifier, QPoint(r.x() + 1, r.y() + r.height() / 2));
    }

    void aMisspelledWordInATableCellIsUnderlinedLikeAnyOther()
    {
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                                 "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>fine words</w:t></w:r></w:p></w:tc>"
                                 "<w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>a recieve here</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        auto doc = open(makeDocx("<w:p><w:r><w:t>before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        view.paginateAll();
        QTextDocument *story = doc->story();
        int start = -1;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            if (b.text() == QStringLiteral("a recieve here")) start = b.position() + 2;
        QVERIFY(start > 0);
        QTextCursor &c = view.cursor();
        c.setPosition(start);
        view.setCursor(c);
        QRect from;
        QVERIFY(view.caretRect(&from));
        c.setPosition(start + 7);
        view.setCursor(c);
        QRect to;
        QVERIFY(view.caretRect(&to));
        c.setPosition(0);
        view.setCursor(c);
        int count = 0;
        QRect wave;
        QTRY_VERIFY((wave = redBounds(view.grab().toImage(), view.pageRect(0), &count), count > 8));
        // Under the word, just below its letters, and nowhere else on the page (a placeholder line of the table has none).
        QVERIFY2(wave.left() >= from.left() - 3 && wave.right() <= to.left() + 3, qPrintable(QStringLiteral("%1..%2 for the word at %3..%4").arg(wave.left()).arg(wave.right()).arg(from.left()).arg(to.left())));
        QVERIFY2(wave.top() >= from.top() && wave.bottom() <= from.bottom() + 8, qPrintable(QStringLiteral("%1..%2 for the line at %3..%4").arg(wave.top()).arg(wave.bottom()).arg(from.top()).arg(from.bottom())));
        // As printed, no.
        redBounds(view.pageImage(0, 900), QRect(0, 0, 900, 1200), &count);
        QCOMPARE(count, 0);
    }

    void theRibbonCommandsDoTheirWork()
    {
        WriteWindow w;
        w.show();
        PageView *v = w.view();
        QTest::keyClicks(v, QStringLiteral("alpha beta"));
        QTest::keyClick(v, Qt::Key_Return);
        QTest::keyClicks(v, QStringLiteral("gamma"));
        v->selectAll();
        QTRY_VERIFY(w.act("edit.copy")->isEnabled());
        w.act("fmt.bold")->trigger();
        QTRY_VERIFY(w.act("fmt.bold")->isChecked());
        w.act("fmt.italic")->trigger();
        w.act("fmt.alignCenter")->trigger();
        QTRY_VERIFY(w.act("fmt.alignCenter")->isChecked());
        w.act("para.bullets")->trigger();
        QTRY_VERIFY(w.act("para.bullets")->isChecked());
        QVERIFY(w.document()->story()->findBlockByNumber(1).textList());
        w.act("para.bullets")->trigger();
        QVERIFY(!w.document()->story()->findBlockByNumber(1).textList());
        // The page.
        QTextCursor &c = v->cursor();
        c.movePosition(QTextCursor::End);
        v->setCursor(c);
        w.act("insert.pageBreak")->trigger();
        QTRY_COMPARE(v->pageCount(), 2);   // (the second page is laid out in the background)
        QTRY_COMPARE(w.findChild<QLabel *>(QStringLiteral("pageNumber"))->text(), QStringLiteral("Page 2 of 2"));
        w.act("layout.landscape")->trigger();
        QTRY_VERIFY(w.act("layout.landscape")->isChecked());
        QVERIFY(w.document()->flow().section(0).orientation == flow::Section::Orientation::Landscape);
        QCOMPARE(w.document()->pager().page(0).size.width() > w.document()->pager().page(0).size.height(), true);
        w.act("layout.columns.2")->trigger();
        QCOMPARE(w.document()->flow().section(0).columns, 2);
        w.act("layout.margins.narrow")->trigger();
        QCOMPARE(w.document()->flow().section(0).margins.left(), 36.0);
        w.act("layout.size.a4")->trigger();
        QCOMPARE(w.document()->flow().section(0).paper.width(), 595.3);
        // The zoom.
        w.act("zoom.200")->trigger();
        QCOMPARE(v->zoom(), 2.0);
        w.act("zoom.pageWidth")->trigger();
        QVERIFY(v->zoomMode() == PageView::ZoomMode::PageWidth);
        w.act("zoom.100")->trigger();
        QCOMPARE(v->zoom(), 1.0);
        // The styles, by key and by the gallery's command.
        c.setPosition(0);
        v->setCursor(c);
        w.act("style.heading1")->trigger();
        QCOMPARE(w.document()->story()->firstBlock().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
        w.act("style.normal")->trigger();
        QCOMPARE(w.document()->story()->firstBlock().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Normal"));
    }

    // A developer's hook, not a test: JW_ROUNDTRIP_IN names a folder (searched through) of .docx files; each is opened, written again, and opened
    // again, and a summary says which could not be, and which came back with other text or other sections. JW_ROUNDTRIP_OUT, if set, is where
    // the files JeffWrite wrote go (to be opened by other programs); JW_ROUNDTRIP_LIMIT stops after that many files.
    void developerRoundTripOfAFolder()
    {
        const QString in = qEnvironmentVariable("JW_ROUNDTRIP_IN"), out = qEnvironmentVariable("JW_ROUNDTRIP_OUT");
        if (in.isEmpty()) QSKIP("JW_ROUNDTRIP_IN is not set");
        if (!out.isEmpty()) QDir().mkpath(out);
        const int limit = qEnvironmentVariableIntValue("JW_ROUNDTRIP_LIMIT") > 0 ? qEnvironmentVariableIntValue("JW_ROUNDTRIP_LIMIT") : 1 << 30;
        int files = 0, opened = 0, refused = 0, changed = 0;
        qint64 largest = 0;
        double slowest = 0;
        QStringList refusals, changes;
        for (QDirIterator it(in, {QStringLiteral("*.docx")}, QDir::Files, QDirIterator::Subdirectories); it.hasNext() && files < limit;) {
            const QString path = it.next();
            ++files;
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly)) continue;
            const QByteArray bytes = f.readAll();
            largest = std::max<qint64>(largest, bytes.size());
            QElapsedTimer t;
            t.start();
            QString err;
            auto doc = WriteDoc::fromDocx(bytes, &err);
            if (!doc) {
                ++refused;
                refusals << QFileInfo(path).fileName() + QStringLiteral(": ") + err.left(80);
                continue;
            }
            ++opened;
            const QByteArray zip = doc->toDocx(&err);
            doc->pager().pageCount();
            slowest = std::max(slowest, t.elapsed() / 1000.0);
            if (zip.isEmpty()) {
                ++changed;
                changes << QFileInfo(path).fileName() + QStringLiteral(": not written: ") + err;
                continue;
            }
            auto back = WriteDoc::fromDocx(zip, &err);
            QString a = doc->story()->toPlainText(), b = back ? back->story()->toPlainText() : QString();
            a.remove(QChar::ObjectReplacementCharacter);
            b.remove(QChar::ObjectReplacementCharacter);
            if (!back || a != b || back->sectionCount() != doc->sectionCount()) {
                ++changed;
                changes << QFileInfo(path).fileName() + (back ? QStringLiteral(": text or sections differ") : QStringLiteral(": does not open again: ") + err);
            } else if (!out.isEmpty()) {
                QFile o(QDir(out).filePath(QFileInfo(path).fileName()));
                if (o.open(QIODevice::WriteOnly)) o.write(zip);
            }
        }
        qInfo("round trip: %d files, %d opened, %d refused, %d came back changed; the largest %lld bytes, the slowest %.1f s", files, opened, refused, changed, largest, slowest);
        for (const QString &r : std::as_const(refusals)) qInfo("refused: %s", qPrintable(r));
        for (const QString &c : std::as_const(changes)) qInfo("changed: %s", qPrintable(c));
    }

    // A developer's hook, not a test: the strict round trip. JW_STRICT_IN names a folder (searched through) of .docx files and JW_STRICT_LIST a file
    // of paths, one to a line; each is opened and written again, and the file written is checked for well-formed XML in every part the program wrote,
    // for losing or changing what the file held (rtcheck.h), and for a story that is not the one that was opened. The summary counts the files for
    // each kind of problem; JW_STRICT_OUT keeps the files written; JW_STRICT_LIMIT stops after that many files; JW_STRICT_REPORT names a file to put every
    // problem of every file in (the log shows the first sixty).
    void developerStrictRoundTrip()
    {
        const QString in = qEnvironmentVariable("JW_STRICT_IN"), list = qEnvironmentVariable("JW_STRICT_LIST"), out = qEnvironmentVariable("JW_STRICT_OUT");
        if (in.isEmpty() && list.isEmpty()) QSKIP("JW_STRICT_IN and JW_STRICT_LIST are not set");
        if (!out.isEmpty()) QDir().mkpath(out);
        const int limit = qEnvironmentVariableIntValue("JW_STRICT_LIMIT") > 0 ? qEnvironmentVariableIntValue("JW_STRICT_LIMIT") : 1 << 30;
        QFile report(qEnvironmentVariable("JW_STRICT_REPORT"));
        if (!report.fileName().isEmpty()) report.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QStringList paths;
        if (!in.isEmpty())
            for (QDirIterator it(in, {QStringLiteral("*.docx"), QStringLiteral("*.docm"), QStringLiteral("*.dotx")}, QDir::Files, QDirIterator::Subdirectories); it.hasNext();) paths << it.next();
        if (!list.isEmpty()) {
            QFile lf(list);
            if (lf.open(QIODevice::ReadOnly))
                for (const QByteArray &l : lf.readAll().split('\n'))
                    if (!l.trimmed().isEmpty()) paths << QString::fromLocal8Bit(l.trimmed());
        }
        int files = 0, refused = 0, notWritten = 0, malformed = 0, lost = 0, story = 0, clean = 0;
        double slowest = 0;
        QString slowestName;
        QMap<QString, int> kinds;
        QStringList lines;
        for (const QString &path : std::as_const(paths)) {
            if (files >= limit) break;
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly)) continue;
            const QByteArray bytes = f.readAll();
            ++files;
            QString err;
            QElapsedTimer fileTimer;
            fileTimer.start();
            auto doc = WriteDoc::fromDocx(bytes, &err);
            if (!doc) {
                ++refused;
                continue;
            }
            const QByteArray zip = doc->toDocx(&err);
            if (fileTimer.elapsed() / 1000.0 > slowest) {
                slowest = fileTimer.elapsed() / 1000.0;
                slowestName = QFileInfo(path).fileName();
            }
            if (zip.isEmpty()) {
                ++notWritten;
                lines << QFileInfo(path).fileName() + QStringLiteral(": not written: ") + err;
                continue;
            }
            const QStringList strict = rt::strictProblems(zip, bytes), loss = rt::lossProblems(bytes, zip);
            QStringList differs;
            auto back = WriteDoc::fromDocx(zip, &err);
            if (!back) differs << QStringLiteral("does not open again: ") + err;
            else differs = rt::storyProblems(*doc, *back);
            malformed += !strict.isEmpty();
            lost += !loss.isEmpty();
            story += !differs.isEmpty();
            clean += strict.isEmpty() && loss.isEmpty() && differs.isEmpty();
            for (const QStringList *l : std::initializer_list<const QStringList *>{&strict, &loss, &differs})
                for (const QString &p : *l) {
                    QString kind = p.left(p.indexOf(QRegularExpression(QStringLiteral("[:0-9>-]"))) > 0 ? p.indexOf(QRegularExpression(QStringLiteral("[:0-9>-]"))) : 24);
                    kind = (l == &strict ? QStringLiteral("strict: ") : l == &loss ? QStringLiteral("loss: ") : QStringLiteral("story: ")) + kind.trimmed();
                    kinds[kind] += 1;
                }
            if (report.isOpen())
                for (const QStringList *l : std::initializer_list<const QStringList *>{&strict, &loss, &differs})
                    for (const QString &p : *l) report.write((QFileInfo(path).fileName() + QLatin1Char('\t') + (l == &strict ? QStringLiteral("strict") : l == &loss ? QStringLiteral("loss") : QStringLiteral("story")) + QLatin1Char('\t') + p + QLatin1Char('\n')).toUtf8());
            if (lines.size() < 60 && (!strict.isEmpty() || !loss.isEmpty() || !differs.isEmpty()))
                lines << QFileInfo(path).fileName() + QStringLiteral(": ") + (strict + loss + differs).join(QStringLiteral(" | ")).left(300);
            if (!out.isEmpty()) {
                QFile o(QDir(out).filePath(QFileInfo(path).fileName()));
                if (o.open(QIODevice::WriteOnly)) o.write(zip);
            }
        }
        qInfo("strict round trip: %d files, %d refused, %d not written, %d not well-formed, %d lost or changed something, %d came back with another story, %d clean; the slowest to open and save, %s, %.1f s", files, refused, notWritten, malformed, lost, story, clean, qPrintable(slowestName), slowest);
        for (auto it = kinds.cbegin(); it != kinds.cend(); ++it) qInfo("  %s: %d", qPrintable(it.key()), it.value());
        for (const QString &l : std::as_const(lines)) qInfo("  %s", qPrintable(l));
    }

    // A developer's hook, not a test: JW_BENCH times what editing sections costs in long documents (milliseconds, to look at).
    void developerSectionTimings()
    {
        if (!qEnvironmentVariableIsSet("JW_BENCH")) QSKIP("JW_BENCH is not set");
        auto ms = [](QElapsedTimer &t) { const double v = t.nsecsElapsed() / 1e6; t.restart(); return v; };
        {
            auto doc = open(makeDocx(paragraphs(5000, 60), kStyles));
            QElapsedTimer t;
            t.start();
            const int pages = doc->pager().pageCount();
            const double paginate = ms(t);
            QTextCursor c(doc->story());
            c.setPosition(doc->story()->findBlockByNumber(2500).position() + 5);
            doc->pager().paginate(doc->pager().pageOfPosition(c.position()) + 2);
            t.restart();
            insertSectionBreak(*doc, c);
            const double insert = ms(t);
            doc->pager().paginate(doc->pager().pageOfPosition(c.position()) + 2);
            const double after = ms(t);
            qInfo("sections: %d pages, paginating %.0f ms; a section break in the middle %.0f ms, and the pages around it %.0f ms", pages, paginate, insert, after);
        }
        {
            QByteArray body;
            const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
            for (int i = 0; i < 20000; ++i) body += "<w:p><w:pPr>" + sect + "</w:pPr><w:r><w:t>section " + QByteArray::number(i) + "</w:t></w:r></w:p>";
            QElapsedTimer t;
            t.start();
            auto doc = open(makeDocx(body, kStyles));
            const double open_ = ms(t);
            doc->pager().paginate(5);
            const double first = ms(t);
            QTextCursor c(doc->story());
            c.setPosition(doc->story()->findBlockByNumber(10000).position() + 5);
            insertParagraph(*doc, c);
            const double enter = ms(t);
            insertText(*doc, c, QStringLiteral("x"));
            const double type = ms(t);
            qInfo("20,000 sections: open %.0f ms, first pages %.0f ms; Enter %.0f ms; a letter %.0f ms", open_, first, enter, type);
        }
    }

    // A developer's hook, not a test: JW_SHOTS names a folder to put screenshots of the window in (each ribbon tab, the File screen's pages, the
    // text selected, the paragraph marks, Help), of the document JW_SHOTS_DOC names.
    void developerScreenshots()
    {
        const QString dir = qEnvironmentVariable("JW_SHOTS"), doc = qEnvironmentVariable("JW_SHOTS_DOC");
        if (dir.isEmpty()) QSKIP("JW_SHOTS is not set");
        QDir().mkpath(dir);
        WriteWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        if (!doc.isEmpty()) QVERIFY(w.openFile(doc));
        w.view()->paginateAll();
        w.view()->setCaretWithoutFocus(true);
        for (const char *tab : {"Insert", "Layout", "View"}) {
            w.ribbon()->showTab(w.ribbon()->tab(QString::fromLatin1(tab)));
            QTest::qWait(100);
            w.grab().save(QDir(dir).filePath(QStringLiteral("tab-%1.png").arg(QString::fromLatin1(tab).toLower())));
        }
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Home")));
        // Text selected across two paragraphs, and the paragraph marks.
        QTextCursor &c = w.view()->cursor();
        c.setPosition(w.document()->story()->findBlockByNumber(3).position() + 10);
        c.setPosition(w.document()->story()->findBlockByNumber(4).position() + 30, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        w.act("view.marks")->trigger();
        QTest::qWait(150);
        w.grab().save(QDir(dir).filePath(QStringLiteral("selection-marks.png")));
        w.act("view.marks")->trigger();
        w.showHelp(QStringLiteral("start"));
        QTest::qWait(150);
        w.grab().save(QDir(dir).filePath(QStringLiteral("help.png")));
        for (const char *page : {"new", "open", "print", "export", "about"}) {
            w.showFileScreen(QString::fromLatin1(page));
            QTest::qWait(250);
            w.grab().save(QDir(dir).filePath(QStringLiteral("file-%1.png").arg(QString::fromLatin1(page))));
        }
    }

    // ---------------------------------------------------------------- editing
    void typingIsOneStepThatUndoesAndRedoes()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("Hello"));
        QCOMPARE(plain(*doc), QStringLiteral("Hello"));
        doc->story()->undo(&c);
        QCOMPARE(plain(*doc), QString());
        doc->story()->redo(&c);
        QCOMPARE(plain(*doc), QStringLiteral("Hello"));
        // The commands are what is undone, not the document: a word is one step, and the step after it another.
        typeText(*doc, c, QStringLiteral(" world"));
        insertParagraph(*doc, c);
        typeText(*doc, c, QStringLiteral("Next"));
        QCOMPARE(doc->story()->blockCount(), 2);
        doc->story()->undo(&c);   // "Next"
        QCOMPARE(plain(*doc), QStringLiteral("Hello world\n"));
        doc->story()->undo(&c);   // the Enter
        QCOMPARE(doc->story()->blockCount(), 1);
        doc->story()->undo(&c);   // " world", which joined the typing before it
        QCOMPARE(plain(*doc), QString());
        QVERIFY(!doc->story()->isUndoAvailable());
    }

    void anEditCostsTheCommandNotTheDocument()
    {
        // 20,000 paragraphs: an undo that kept the whole document would take time in proportion to it (the design's reason for commands). The same
        // edits in a tenth of the paragraphs take about as long, not a tenth: compared with each other, not with a clock.
        auto timeEdits = [](int paragraphCount) {
            std::unique_ptr<WriteDoc> doc;
            QTextCursor c;
            const qint64 ns = bestNanoseconds([&] {
                for (int i = 0; i < 50; ++i) insertText(*doc, c, QStringLiteral("x"));
                insertParagraph(*doc, c);
                for (int i = 0; i < 50; ++i) doc->story()->undo(&c);
            }, 5, [&] {
                c = QTextCursor();
                doc = open(makeDocx(paragraphs(paragraphCount, 12), kStyles));
                c = QTextCursor(doc->story());
                c.setPosition(5000);
            });
            const double ms = ns / 1e6;
            return doc->story()->blockCount() == paragraphCount ? ms : 1e9;
        };
        timeEdits(2000);   // (the first of anything is slow)
        const double small = std::max(timeEdits(2000), 5.0), large = timeEdits(20000);
        QVERIFY2(large < 4 * small, qPrintable(QStringLiteral("2,000 paragraphs %1 ms, 20,000 paragraphs %2 ms").arg(small).arg(large)));
    }

    void enterAndBackspaceSplitAndJoinParagraphs()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("abcd"));
        c.setPosition(2);
        insertParagraph(*doc, c);
        QCOMPARE(plain(*doc), QStringLiteral("ab\ncd"));
        QCOMPARE(c.blockNumber(), 1);
        QCOMPARE(c.positionInBlock(), 0);
        deleteBack(*doc, c, false);
        QCOMPARE(plain(*doc), QStringLiteral("abcd"));
        QCOMPARE(c.position(), 2);
        deleteForward(*doc, c, false);
        QCOMPARE(plain(*doc), QStringLiteral("abd"));
        // A selection typed over is replaced; the replacement is one step.
        c.setPosition(0);
        c.setPosition(2, QTextCursor::KeepAnchor);
        insertText(*doc, c, QStringLiteral("Z"));
        QCOMPARE(plain(*doc), QStringLiteral("Zd"));
        doc->story()->undo(&c);
        QCOMPARE(plain(*doc), QStringLiteral("abd"));
    }

    void aHeadingIsFollowedByBodyText()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("Title words"));
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        QCOMPARE(paragraphStyleAt(*doc, c), QStringLiteral("Heading1"));
        const QTextCharFormat heading = c.block().charFormat();
        QVERIFY(c.block().begin().fragment().charFormat().fontPointSize() > 12);
        insertParagraph(*doc, c);   // at the end: the style that follows
        QCOMPARE(paragraphStyleAt(*doc, c), QStringLiteral("Normal"));
        typeText(*doc, c, QStringLiteral("body"));
        const QTextCharFormat body = c.block().begin().fragment().charFormat();
        QVERIFY(body.fontPointSize() < c.document()->firstBlock().begin().fragment().charFormat().fontPointSize());
        QCOMPARE(body.fontWeight(), int(QFont::Normal));
        Q_UNUSED(heading);
    }

    void characterFormatsApplyToTheSelectionAndSurviveASaveAndOpen()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("Some words here"));
        c.setPosition(5);
        c.setPosition(10, QTextCursor::KeepAnchor);
        toggleBold(*doc, c);
        toggleItalic(*doc, c);
        toggleUnderline(*doc, c);
        setFontSize(*doc, c, 20);
        setTextColor(*doc, c, QColor(0xC0, 0, 0));
        setHighlight(*doc, c, QStringLiteral("yellow"));
        setFontFamily(*doc, c, QStringLiteral("Cousine"));
        QCOMPARE(c.charFormat().fontWeight(), int(QFont::Bold));
        // Toggling again, with all of it bold, turns it off.
        QVERIFY(allSelected(c, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; }));
        const QByteArray zip = doc->toDocx();
        QVERIFY(!zip.isEmpty());
        const QByteArray x = documentXml(zip);
        // (The word "words" is the run with the properties; the rest has none of its own, as the style gives it its look.)
        QRegularExpression run(QStringLiteral("<w:r><w:rPr>(.*?)</w:rPr><w:t xml:space=\"preserve\">words</w:t>"));
        const auto m = run.match(QString::fromUtf8(x));
        QVERIFY2(m.hasMatch(), x.constData());
        const QString props = m.captured(1);
        QVERIFY(props.contains(QLatin1String("<w:b/>")));
        QVERIFY(props.contains(QLatin1String("<w:i/>")));
        QVERIFY(props.contains(QLatin1String("<w:u w:val=\"single\"/>")));
        QVERIFY(props.contains(QLatin1String("<w:sz w:val=\"40\"/>")));
        QVERIFY(props.contains(QLatin1String("<w:color w:val=\"C00000\"/>")));
        QVERIFY(props.contains(QLatin1String("<w:highlight w:val=\"yellow\"/>")));
        QVERIFY(props.contains(QLatin1String("Cousine")));
        QVERIFY(x.contains("<w:t xml:space=\"preserve\">Some </w:t>"));
        const int firstRun = x.indexOf("<w:r>");
        QVERIFY(x.mid(firstRun, 40).startsWith("<w:r><w:t"));   // (no direct properties on text that has none of its own)
        // Opening it again gives the same look.
        auto back = open(zip);
        QTextBlock b = back->story()->firstBlock();
        QTextCursor t(back->story());
        t.setPosition(7);
        const QTextCharFormat f = t.charFormat();
        QCOMPARE(f.fontWeight(), int(QFont::Bold));
        QVERIFY(f.fontItalic());
        QCOMPARE(f.fontPointSize(), 20.0);
        QCOMPARE(f.foreground().color(), QColor(0xC0, 0, 0));
        QCOMPARE(f.fontFamilies().toStringList().value(0), QStringLiteral("Cousine"));
        QVERIFY(f.underlineStyle() != QTextCharFormat::NoUnderline);
        t.setPosition(2);
        QCOMPARE(t.charFormat().fontWeight(), int(QFont::Normal));
        Q_UNUSED(b);
    }

    void paragraphFormatsApplyToTheParagraphsSelected()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("one\ntwo\nthree"));
        c.setPosition(1);
        c.setPosition(doc->story()->findBlockByNumber(1).position() + 1, QTextCursor::KeepAnchor);
        setAlignment(*doc, c, Qt::AlignHCenter);
        setIndents(*doc, c, 36, 18, -18);
        setSpacing(*doc, c, 6, 12);
        setLineSpacing(*doc, c, QTextBlockFormat::ProportionalHeight, 1.5);
        QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
        QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().leftMargin(), 36.0);
        QCOMPARE(doc->story()->findBlockByNumber(2).blockFormat().leftMargin(), 0.0);   // (the third was not selected)
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:spacing w:before=\"120\" w:after=\"240\" w:line=\"360\" w:lineRule=\"auto\"/>"), x.constData());
        QVERIFY(x.contains("<w:ind w:left=\"720\" w:right=\"360\" w:hanging=\"360\"/>"));
        QVERIFY(x.contains("<w:jc w:val=\"center\"/>"));
        QCOMPARE(int(x.count("<w:jc w:val=\"center\"/>")), 2);
        // Undone as one step each.
        for (int i = 0; i < 4; ++i) doc->story()->undo(&c);
        QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().leftMargin(), 0.0);
        QVERIFY(!documentXml(doc->toDocx()).contains("w:jc"));
    }

    void listsAreTurnedOnAndOffAndKeptInTheFile()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("apple\nbanana\ncherry"));
        c.setPosition(0);
        c.setPosition(doc->story()->characterCount() - 1, QTextCursor::KeepAnchor);
        toggleList(*doc, c, true);
        QCOMPARE(listKindOf(*doc, doc->story()->firstBlock()), ListKind::Numbers);
        QTextList *list = doc->story()->findBlockByNumber(2).textList();
        QVERIFY(list);
        QCOMPARE(list->itemText(doc->story()->findBlockByNumber(2)), QStringLiteral("3."));
        QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("<w:numPr>")), 3);
        QVERIFY(x.contains("<w:ilvl w:val=\"0\"/>"));
        // The file it makes opens to the same list.
        auto back = open(doc->toDocx());
        QCOMPARE(back->story()->findBlockByNumber(1).textList()->itemText(back->story()->findBlockByNumber(1)), QStringLiteral("2."));
        // Bullets over numbers change the kind; again, they turn off.
        toggleList(*doc, c, false);
        QCOMPARE(listKindOf(*doc, doc->story()->firstBlock()), ListKind::Bullets);
        toggleList(*doc, c, false);
        QCOMPARE(listKindOf(*doc, doc->story()->firstBlock()), ListKind::None);
        x = documentXml(doc->toDocx());
        QVERIFY(!x.contains("<w:numPr>"));
        // Enter on an empty item ends the list; a Tab makes an item a level deeper.
        toggleList(*doc, c, false);
        c.movePosition(QTextCursor::End);
        insertParagraph(*doc, c);
        QCOMPARE(c.block().blockFormat().intProperty(prop::NumId) > 0, true);
        insertParagraph(*doc, c);
        QVERIFY(!c.block().blockFormat().hasProperty(prop::NumId));
        QCOMPARE(doc->story()->blockCount(), 4);
    }

    // A page break is a character of its paragraph (the page ends after the line that has it); Insert > Page Break puts one in and goes on in a
    // paragraph of its own, as the file does it, and the file keeps what it was given: the break in the paragraph's runs, one for one.
    void aPageBreakIsACharacterOfItsParagraphAndTheFileKeepsItWhole()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("before after"));
        c.setPosition(7);
        insertPageBreak(*doc, c);
        QCOMPARE(doc->pager().pageCount(), 2);
        QCOMPARE(doc->story()->blockCount(), 2);
        QCOMPARE(doc->story()->findBlockByNumber(0).text(), QStringLiteral("before ") + flow::kPageBreak);
        QCOMPARE(c.block().text(), QStringLiteral("after"));   // (the cursor is at the top of the next page)
        QCOMPARE(c.position(), doc->story()->findBlockByNumber(1).position());
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("<w:p>")) + int(x.count("<w:p ")), 2);
        QCOMPARE(int(x.count("<w:br w:type=\"page\"/>")), 1);
        QVERIFY2(x.indexOf("<w:br w:type=\"page\"/>") < x.indexOf("</w:p>"), "the break ends the first paragraph");
        auto back = open(doc->toDocx());
        QCOMPARE(back->story()->blockCount(), 2);
        QCOMPARE(back->pager().pageCount(), 2);
        QCOMPARE(documentXml(back->toDocx()), x);
        // Backspace at the start of the second takes the paragraph mark out, and again the break.
        QTextCursor d(back->story());
        d.setPosition(back->story()->findBlockByNumber(1).position());
        deleteBack(*back, d, false);
        QCOMPARE(back->story()->blockCount(), 1);
        QCOMPARE(back->pager().pageCount(), 2);
        deleteBack(*back, d, false);
        QCOMPARE(back->pager().pageCount(), 1);
        QVERIFY(!documentXml(back->toDocx()).contains("w:br"));
        // A break in the middle of a paragraph of a file, a run of its own or in the text's, is read as the character, and written as before.
        auto mid = open(makeDocx("<w:p><w:r><w:t>one</w:t><w:br w:type=\"page\"/><w:t>two</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>three</w:t></w:r></w:p>"));
        QCOMPARE(mid->story()->findBlockByNumber(0).text(), QStringLiteral("one") + flow::kPageBreak + QStringLiteral("two") + flow::kPageBreak + QStringLiteral("three"));
        QCOMPARE(mid->story()->blockCount(), 1);
        QCOMPARE(mid->pager().pageCount(), 3);
        QCOMPARE(int(documentXml(mid->toDocx()).count("<w:br w:type=\"page\"/>")), 2);
    }

    // Page Setup on the last section (or the only one) is a step of the undo, as on the others, and marks the document as changed.
    void pageSetupOnTheLastSectionUndoesAndMarksTheDocumentModified()
    {
        for (int which = 0; which < 2; ++which) {
            std::unique_ptr<WriteDoc> doc = which == 0 ? WriteDoc::blank() : open(richDocx());
            QVERIFY(doc);
            const int last = doc->sectionCount() - 1;
            QCOMPARE(last, which);
            const QByteArray was = packSection(doc->sectionProps(last));
            const QSizeF size = doc->pager().page(doc->pager().pageCount() - 1).size;
            doc->setModified(false);
            const int steps = doc->story()->availableUndoSteps();
            wml::SectionProps p = doc->sectionProps(last);
            std::swap(p.pageW, p.pageH);
            p.orient = QStringLiteral("landscape");
            doc->setSectionProps(last, p);
            QVERIFY2(doc->isModified(), "the document is marked as changed");
            QCOMPARE(doc->story()->availableUndoSteps(), steps + 1);
            QCOMPARE(doc->pager().page(doc->pager().pageCount() - 1).size, QSizeF(size.height(), size.width()));
            doc->story()->undo();
            QCOMPARE(packSection(doc->sectionProps(last)), was);
            QCOMPARE(doc->pager().page(doc->pager().pageCount() - 1).size, size);
            doc->story()->redo();
            QCOMPARE(doc->pager().page(doc->pager().pageCount() - 1).size, QSizeF(size.height(), size.width()));
            // And the file has it.
            auto back = open(doc->toDocx());
            QCOMPARE(back->sectionProps(back->sectionCount() - 1).orient, QStringLiteral("landscape"));
        }
    }

    // A style given to the first paragraph, and undone: the file is what it was (the engine can't undo a change to the first paragraph's mark
    // by itself; the program keeps it where it can).
    void undoingAStyleOnTheFirstParagraphRestoresItsMark()
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>first</w:t></w:r></w:p><w:p><w:r><w:t>second</w:t></w:r></w:p>", kStyles));
        QVERIFY(doc);
        const QByteArray before = documentXml(doc->toDocx());
        for (int block = 0; block < 2; ++block) {
            QTextCursor c(doc->story());
            c.setPosition(doc->story()->findBlockByNumber(block).position());
            applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
            QVERIFY(documentXml(doc->toDocx()) != before);
            QVERIFY(markFormat(doc->story()->findBlockByNumber(block)).fontPointSize() == 17.0);
            doc->story()->undo();
            QCOMPARE(documentXml(doc->toDocx()), before);
            doc->story()->redo();
            QVERIFY(markFormat(doc->story()->findBlockByNumber(block)).fontPointSize() == 17.0);
            doc->story()->undo();
        }
        QCOMPARE(documentXml(doc->toDocx()), before);
    }

    // Attributes the program has no use for, on elements it does understand, go back as they were read: a theme color of an underline, the paper
    // code, the numbering of chapters in the page numbers.
    void unknownAttributesOnKnownElementsGoBackAsRead()
    {
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\" w:code=\"1\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\" w:odd=\"x\"/>"
                                "<w:pgNumType w:fmt=\"decimal\" w:chapStyle=\"1\" w:chapSep=\"colon\"/><w:cols w:num=\"2\" w:space=\"720\" w:odd=\"y\"/></w:sectPr>";
        auto doc = open(makeDocx("<w:p><w:r><w:rPr><w:u w:val=\"single\" w:color=\"FF0000\" w:themeColor=\"accent2\" w:themeTint=\"99\"/></w:rPr><w:t>underlined</w:t></w:r></w:p>", kStyles, QByteArray(), sect));
        QVERIFY(doc);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("w:themeColor=\"accent2\"") && x.contains("w:themeTint=\"99\""), x.constData());
        QVERIFY2(x.contains("w:code=\"1\""), x.constData());
        QVERIFY2(x.contains("w:chapStyle=\"1\"") && x.contains("w:chapSep=\"colon\""), x.constData());
        QVERIFY2(x.contains("w:odd=\"x\"") && x.contains("w:odd=\"y\""), x.constData());
        // They survive an edit of the page setup, and turning the underline off takes its theme color with it.
        wml::SectionProps p = doc->sectionProps(0);
        p.cols = 1;
        doc->setSectionProps(0, p);
        QVERIFY(documentXml(doc->toDocx()).contains("w:chapSep=\"colon\""));
        QTextCursor c(doc->story());
        c.select(QTextCursor::Document);
        toggleUnderline(*doc, c);
        QVERIFY(!documentXml(doc->toDocx()).contains("themeColor"));
        QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), documentXml(doc->toDocx()));
    }

    // A file's headers and footers are drawn on its pages: the first-page, even-page, and ordinary ones by the section's titlePg and the file's
    // evenAndOddHeaders, the page's number and the page counts live; and saving keeps their parts as they were.
    void aFilesHeadersAndFootersAreDrawnAndKept()
    {
        const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:headerReference w:type=\"first\" r:id=\"rIdheader2\"/>"
                                "<w:headerReference w:type=\"even\" r:id=\"rIdheader3\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>"
                                "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"500\" w:footer=\"600\" w:gutter=\"0\"/>"
                                "<w:pgNumType w:start=\"4\"/><w:titlePg/></w:sectPr>";
        const QByteArray footer = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGE \\* ROMAN </w:instrText></w:r>"
                                                       "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>IV</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>"
                                                       "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>1</w:t></w:r></w:fldSimple><w:r><w:t xml:space=\"preserve\"> or </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>"
                                                       "<w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>4</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"});
        const QMap<QString, QByteArray> parts{
            {QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>ORDINARY HEADER</w:t></w:r>"})},
            {QStringLiteral("/word/header2.xml"), headerPart("hdr", {"<w:r><w:t>FIRST HEADER</w:t></w:r>"})},
            {QStringLiteral("/word/header3.xml"), headerPart("hdr", {"<w:r><w:t>EVEN HEADER</w:t></w:r>"})},
            {QStringLiteral("/word/footer1.xml"), footer}};
        const QByteArray zip = makeDocxWithParts(paragraphs(60, 40), sect, parts, "<w:evenAndOddHeaders/>");
        auto doc = open(zip);
        QVERIFY(doc);
        const int pages = doc->pager().pageCount();
        QVERIFY(pages >= 4);
        // (The numbers go 4, 5, 6, 7: the first page is the section's, the sixth is the even one.)
        const QStringList want{QStringLiteral("FIRST HEADER"), QStringLiteral("ORDINARY HEADER"), QStringLiteral("EVEN HEADER"), QStringLiteral("ORDINARY HEADER")};
        for (int k = 0; k < 4; ++k) {
            const auto page = doc->pager().materialize(k);
            QVERIFY(page);
            QCOMPARE(page->number, 4 + k);
            const TextItem *h = boxWithText(*page, want[k]);
            QVERIFY2(h, qPrintable(QStringLiteral("page %1 has no header %2").arg(k).arg(want[k])));
            QVERIFY(std::abs(h->rect.top() - 25) < 0.01);   // the header distance, 500 twentieths of a point
            // (The footer is the odd pages' only: the first page and the even pages have none.)
            const TextItem *f = boxWithText(*page, QStringLiteral("Page "));
            if (k != 1 && k != 3) {
                QVERIFY(!f);
                continue;
            }
            QVERIFY(f);
            QVERIFY(std::abs(f->rect.bottom() - (792 - 30)) < 0.02);   // the footer distance
            // The page's number and the count of the pages are fields the page's document answers.
            const QString shown = page->doc->story(f->storyId)->doc->toPlainText();
            QCOMPARE(int(shown.count(QChar::ObjectReplacementCharacter)), 3);
            FieldContext ctx;
            ctx.doc = page->doc.get();
            QCOMPARE(ctx.resolve(QStringLiteral("page:ROMAN")), QStringLiteral("IV|V|VI|VII").split(QLatin1Char('|'))[k]);
            QCOMPARE(ctx.resolve(QStringLiteral("page")), QString::number(4 + k));
            QCOMPARE(ctx.resolve(QStringLiteral("pages")), QString::number(pages));
        }
        // The parts and the references are kept as they were.
        const QByteArray saved = doc->toDocx();
        office::Package before, after;
        before.read(zip);
        after.read(saved);
        for (auto it = parts.cbegin(); it != parts.cend(); ++it) QCOMPARE(after.part(it.key()), before.part(it.key()));
        const QByteArray x = documentXml(saved);
        QCOMPARE(int(x.count("headerReference")), 3);
        QCOMPARE(int(x.count("footerReference")), 1);
    }

    // A section that names no header or footer has the section before's; one that names its own has it.
    void sectionsTakeTheHeadersOfTheSectionBefore()
    {
        const QByteArray one = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        const QByteArray two = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        const QByteArray three = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader2\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        const QByteArray body = "<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr>" + two + "</w:pPr><w:r><w:t>two</w:t></w:r></w:p><w:p><w:r><w:t>three</w:t></w:r></w:p>";
        const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>FIRST HEADER</w:t></w:r>"})},
                                              {QStringLiteral("/word/header2.xml"), headerPart("hdr", {"<w:r><w:t>THIRD HEADER</w:t></w:r>"})}};
        auto doc = open(makeDocxWithParts(body, three, parts));
        QVERIFY(doc);
        QCOMPARE(doc->pager().pageCount(), 3);
        const QStringList want{QStringLiteral("FIRST HEADER"), QStringLiteral("FIRST HEADER"), QStringLiteral("THIRD HEADER")};
        for (int k = 0; k < 3; ++k) {
            const auto page = doc->pager().materialize(k);
            QVERIFY2(page && boxWithText(*page, want[k]), qPrintable(QString::number(k)));
        }
    }

    // The file's font table says what a font it names is set in where the machine lacks it: its alternate, or by its kind a serif face or a fixed-width one.
    void theFontTableGivesTheStandIns()
    {
        const QByteArray table = "<w:fonts " + QByteArray(kNs) + "><w:font w:name=\"Palatino\"><w:altName w:val=\"Book Antiqua\"/><w:family w:val=\"roman\"/></w:font>"
                                 "<w:font w:name=\"Old Serif\"><w:family w:val=\"roman\"/></w:font><w:font w:name=\"Typewriter\"><w:family w:val=\"modern\"/></w:font>"
                                 "<w:font w:name=\"Sans\"><w:family w:val=\"swiss\"/></w:font></w:fonts>";
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {{QStringLiteral("/word/fontTable.xml"), table}}));
        QVERIFY(doc);
        const auto &standIns = doc->flow().env.standIns;
        QCOMPARE(standIns.value(QStringLiteral("palatino")), QStringLiteral("Book Antiqua"));
        QCOMPARE(standIns.value(QStringLiteral("old serif")), QStringLiteral("Times New Roman"));
        QCOMPARE(standIns.value(QStringLiteral("typewriter")), QStringLiteral("Courier New"));
        QVERIFY(!standIns.contains(QStringLiteral("sans")));
    }

    // A font table of thousands of fonts that say nothing is read in step with its size.
    void aLongFontTableCostsInStepWithItsSize()
    {
        auto seconds = [&](int count) {
            QByteArray table = "<w:fonts " + QByteArray(kNs) + ">";
            for (int i = 0; i < count; ++i) table += "<w:font w:name=\"F" + QByteArray::number(i) + "\"/>";
            table += "</w:fonts>";
            const QByteArray zip = makeDocxWithParts(paragraphs(1, 5), QByteArray(), {{QStringLiteral("/word/fontTable.xml"), table}});
            std::unique_ptr<WriteDoc> doc;
            return bestNanoseconds([&] { doc = open(zip); }, 5, [&] { doc.reset(); }) / 1e9;
        };
        const double small = seconds(5000), large = seconds(20000);
        QVERIFY2(large < 0.2 + small * 8, qPrintable(QStringLiteral("5000 fonts %1 s, 20000 fonts %2 s").arg(small).arg(large)));
    }

    // The notes' numbers restart in each section when the settings say so of the footnotes (not of the endnotes).
    void footnoteNumbersRestartWhenTheSettingsSaySo()
    {
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>Note.</w:t></w:r></w:p></w:footnote></w:footnotes>";
        auto restarts = [&](const QByteArray &settings) {
            auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}, settings));
            return doc && doc->flow().restartNotesInSections;
        };
        QVERIFY(restarts("<w:footnotePr><w:numRestart w:val=\"eachSect\"/></w:footnotePr>"));
        QVERIFY(!restarts("<w:footnotePr><w:numRestart w:val=\"continuous\"/></w:footnotePr>"));
        QVERIFY(!restarts("<w:footnotePr><w:numFmt w:val=\"decimal\"/></w:footnotePr><w:endnotePr><w:numRestart w:val=\"eachSect\"/></w:endnotePr>"));
        QVERIFY(!restarts(QByteArray()));
    }

    // A section's page number format, as the file names it and as the pages count with it, each way: dashes too.
    void pageNumberFormatsMapBothWays()
    {
        const QPair<QString, QString> pairs[] = {{"lowerRoman", "roman"}, {"upperRoman", "ROMAN"}, {"lowerLetter", "alpha"}, {"upperLetter", "ALPHA"}, {"numberInDash", "dash"}, {"decimal", ""}};
        for (const auto &pair : pairs) {
            office::wml::SectionProps props;
            props.pageFormat = pair.first;
            flow::Section section = flowSectionOf(props, false);
            QCOMPARE(section.pageNumberFormat, pair.second);
            // (Back: a section changed to the format has the file's name for it.)
            office::wml::SectionProps blank;
            section.pageNumberFormat = pair.second;
            flow::Section plain = flowSectionOf(blank, false);
            if (pair.second != plain.pageNumberFormat) {
                updateSectionProps(&blank, section);
                QCOMPARE(blank.pageFormat, pair.first == QLatin1String("decimal") ? QString() : pair.first);
            }
        }
        // The pages show it: the page's own number is in dashes.
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgNumType w:fmt=\"numberInDash\" w:start=\"7\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), sect, {}));
        QVERIFY(doc);
        QCOMPARE(doc->flow().sections[0].pageNumberFormat, QStringLiteral("dash"));
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        FieldContext ctx;
        ctx.doc = page->doc.get();
        ctx.pageNumber = 1;
        QCOMPARE(ctx.resolve(QStringLiteral("page")), QStringLiteral("- 7 -"));
        QVERIFY(documentXml(doc->toDocx()).contains("numberInDash"));
    }

    // Punctuation hangs past the margin only in a file that has the East Asian layout on, and not where it also says no punctuation hangs; each option
    // is read by its value: stated off, it is off.
    void punctuationHangsOnlyWithTheEastAsianLayout()
    {
        auto hangs = [&](const QByteArray &option) {
            auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + option + "</w:compat>"));
            return doc && !doc->flow().env.wordNoHang;
        };
        QVERIFY(!hangs(""));
        QVERIFY(!hangs("<w:doNotWrapTextWithPunct/>"));
        QVERIFY(!hangs("<w:useFELayout w:val=\"0\"/>"));
        QVERIFY(hangs("<w:useFELayout/>"));
        QVERIFY(hangs("<w:useFELayout w:val=\"1\"/>"));
        QVERIFY(!hangs("<w:useFELayout/><w:doNotWrapTextWithPunct/>"));
        QVERIFY(!hangs("<w:useFELayout/><w:doNotWrapTextWithPunct w:val=\"1\"/>"));
        QVERIFY(hangs("<w:useFELayout/><w:doNotWrapTextWithPunct w:val=\"0\"/>"));
        QVERIFY(hangs("<w:useFELayout/><w:doNotWrapTextWithPunct w:val=\"false\"/>"));
    }

    // The mark after a page break goes to the next page in a file that says so (splitPgBreakAndParaMark) and in the files of the recent programs (compatibility
    // mode 15); an older file's mark stays where the break is.
    void theMarkAfterABreakFollowsTheFilesOption()
    {
        auto follows = [&](const QByteArray &compat) {
            auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + compat + "</w:compat>"));
            return doc && doc->flow().markFollowsBreak;
        };
        QVERIFY(!follows(""));
        QVERIFY(follows("<w:splitPgBreakAndParaMark/>"));
        QVERIFY(!follows("<w:splitPgBreakAndParaMark w:val=\"0\"/>"));
        QVERIFY(!follows("<w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"14\"/>"));
        QVERIFY(follows("<w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"15\"/>"));
    }

    // What a tracked change deleted is not in the lines: a paragraph whose mark was deleted and that shows nothing else takes no room, nor do the
    // breaks and tabs of deleted runs; one whose mark was deleted but that shows text is still there. The file is saved with the changes as they were.
    void whatATrackedChangeDeletedTakesNoRoom()
    {
        const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
        auto para = [](const char *text) { return QByteArray("<w:p><w:r><w:t>") + text + "</w:t></w:r></w:p>"; };
        const QByteArray deletedMark = "<w:p><w:pPr><w:rPr><w:del" + rev + "/></w:rPr></w:pPr></w:p>";
        const QByteArray deletedBreak = "<w:p><w:r><w:t>two</w:t></w:r><w:del" + rev + "><w:r><w:tab/><w:br/><w:delText>x</w:delText></w:r></w:del></w:p>";
        const QByteArray keptText = "<w:p><w:pPr><w:rPr><w:del" + rev + "/></w:rPr></w:pPr><w:r><w:t>text</w:t></w:r></w:p>";
        auto lines = [&](const QByteArray &body) {
            auto doc = open(makeDocx(body));
            return doc ? doc->pager().lines(0) : QVector<StoryLayout::LineBrief>();
        };
        const auto plain = lines(para("one") + para("two") + para("three"));
        QCOMPARE(plain.size(), 3);
        const auto edited = lines(para("one") + deletedMark + deletedBreak + para("three"));
        QCOMPARE(edited.size(), plain.size());
        for (int i = 0; i < plain.size(); ++i) {
            QVERIFY2(std::abs(edited[i].rect.top() - plain[i].rect.top()) < 0.01, qPrintable(QStringLiteral("line %1 is at %2, not %3").arg(i).arg(edited[i].rect.top()).arg(plain[i].rect.top())));
            QVERIFY(std::abs(edited[i].rect.height() - plain[i].rect.height()) < 0.01);
        }
        QCOMPARE(lines(para("one") + keptText + para("three")).size(), 3);
        // Saved after an edit: the deleted parts are as they were, and nothing is made hidden.
        auto doc = open(makeDocx(para("one") + deletedMark + deletedBreak + para("three")));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(1);
        insertText(*doc, c, QStringLiteral("x"));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("vanish"), x.constData());
        QVERIFY(x.contains("<w:del"));
        QVERIFY(x.contains("<w:br/>"));
        // Text typed beside deleted content is not deleted.
        QTextCursor at(doc->story());
        at.setPosition(doc->story()->findBlockByNumber(2).position() + 3);   // (after "two", before what was deleted)
        insertText(*doc, at, QStringLiteral("!"));
        QVERIFY(!at.charFormat().boolProperty(tp::HiddenText));
        QVERIFY(doc->pager().lines(0).size() == 3);
    }

    // What a file says of hidden text on what a tracked change deleted is what is saved: a w:vanish on a deleted run or on a deleted paragraph mark stays, and
    // text that is hidden only because it is deleted is not made hidden in the file (nor, in a style that hides text, shown by a w:vanish of value 0).
    void whatAFileSaysOfHiddenTextOnDeletedTextIsSavedAsItWas()
    {
        const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
        auto savedAfterTyping = [&](const QByteArray &body, const QByteArray &styles) {
            auto doc = open(makeDocx(body, styles));
            if (!doc) return QByteArray();
            QTextCursor c(doc->story());
            c.setPosition(1);
            insertText(*doc, c, QStringLiteral("x"));
            return documentXml(doc->toDocx());
        };
        // A file that hides the deleted run and the deleted mark itself.
        const QByteArray vanished = "<w:p><w:r><w:t>one</w:t></w:r><w:del" + rev + "><w:r><w:rPr><w:vanish/></w:rPr><w:delText>gone</w:delText></w:r></w:del></w:p>"
                                    "<w:p><w:pPr><w:rPr><w:vanish/><w:del" + rev + "/></w:rPr></w:pPr><w:del" + rev + "><w:r><w:delText>mark</w:delText></w:r></w:del></w:p>"
                                    "<w:p><w:r><w:t>tail</w:t></w:r></w:p>";
        QByteArray x = savedAfterTyping(vanished, QByteArray());
        QVERIFY2(x.count("<w:vanish/>") == 2, x.constData());
        QVERIFY2(x.contains("<w:delText>gone</w:delText>") && x.contains("<w:delText>mark</w:delText>"), x.constData());
        // A deleted run in a style that hides text, which says nothing of it: nothing is added.
        const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Hid\"><w:name w:val=\"Hid\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:vanish/></w:rPr></w:style>";
        const QByteArray inHidden = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val=\"Hid\"/></w:pPr><w:del" + rev + "><w:r><w:delText>gone</w:delText></w:r></w:del></w:p>";
        x = savedAfterTyping(inHidden, styles);
        QVERIFY2(!x.contains("vanish w:val") && x.count("vanish") == 0 && x.contains("<w:delText>gone</w:delText>"), x.constData());
        // One that says it is not hidden stays so.
        const QByteArray shown = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val=\"Hid\"/></w:pPr><w:del" + rev + "><w:r><w:rPr><w:vanish w:val=\"0\"/></w:rPr><w:delText>gone</w:delText></w:r></w:del></w:p>";
        x = savedAfterTyping(shown, styles);
        QVERIFY2(x.contains("<w:vanish w:val=\"0\"/>"), x.constData());
    }

    // The round-trip check sees what a save did to hidden text and to deleted text.
    void theRoundTripCheckSeesLostHiddenAndDeletedText()
    {
        const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
        auto file = [&](const QByteArray &runProps, const QByteArray &markProps, const QByteArray &deleted) {
            return makeDocx("<w:p><w:pPr><w:rPr>" + markProps + "</w:rPr></w:pPr><w:del" + rev + "><w:r><w:rPr>" + runProps + "</w:rPr><w:delText>" + deleted + "</w:delText></w:r></w:del></w:p>");
        };
        const QByteArray base = file("<w:vanish/>", "<w:vanish/>", "gone");
        QVERIFY(rt::lossProblems(base, base).isEmpty());
        QVERIFY(!rt::lossProblems(base, file(QByteArray(), "<w:vanish/>", "gone")).isEmpty());   // (the run's w:vanish lost)
        QVERIFY(!rt::lossProblems(base, file("<w:vanish/>", QByteArray(), "gone")).isEmpty());   // (the mark's)
        QVERIFY(!rt::lossProblems(base, file("<w:vanish w:val=\"0\"/>", "<w:vanish/>", "gone")).isEmpty());   // (changed)
        QVERIFY(!rt::lossProblems(base, file("<w:vanish/>", "<w:vanish/>", "gone!")).isEmpty());   // (the deleted text changed)
    }

    // Text typed beside what a tracked change deleted is never inside the deletion in the file: typed before it or just inside its start, it goes before it;
    // typed after its content, just inside its end, or after it, it goes after it.
    void textTypedBesideADeletionIsOutsideIt()
    {
        const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
        const QByteArray body = "<w:p><w:r><w:t>one</w:t></w:r><w:del" + rev + "><w:r><w:delText>gone</w:delText></w:r></w:del><w:r><w:t>tail</w:t></w:r></w:p>";
        // (In the story: "one" 0 to 2, the deletion's start 3, its text 4, its end 5, "tail" from 6.)
        for (int at = 3; at <= 6; ++at) {
            auto doc = open(makeDocx(body));
            QVERIFY(doc);
            QTextCursor c(doc->story());
            c.setPosition(at);
            insertText(*doc, c, QStringLiteral("ZZ"));
            const QByteArray x = documentXml(doc->toDocx());
            const int open = x.indexOf("<w:del "), close = x.indexOf("</w:del>"), typed = x.indexOf("ZZ");
            QVERIFY2(open >= 0 && close > open && typed >= 0, x.constData());
            QVERIFY2(typed < open || typed > close, qPrintable(QStringLiteral("typed at %1 is inside the deletion: %2").arg(at).arg(QString::fromUtf8(x))));
            QVERIFY2((at <= 4) == (typed < open), qPrintable(QStringLiteral("typed at %1 is on the wrong side: %2").arg(at).arg(QString::fromUtf8(x))));
            QVERIFY(x.contains("<w:delText>gone</w:delText>"));
        }
    }

    // Editing acts on what shows: Backspace at the start of a paragraph after one a tracked change deleted joins it with the paragraph that shows before, Delete at
    // the end of one before such a paragraph does the same, and what is deleted stays, as paragraphs of its own: no paragraph that has text keeps the deleted mark.
    void editingJoinsTheParagraphsThatShowAcrossADeletedOne()
    {
        const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"", mark = " w:id=\"9\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
        const QByteArray body = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:rPr><w:del" + mark + "/></w:rPr></w:pPr><w:del" + rev + "><w:r><w:delText>gone</w:delText></w:r></w:del></w:p>"
                                "<w:p><w:r><w:t>two</w:t></w:r></w:p>";
        // The paragraphs of the saved file that have text, and whether the deleted one is still there.
        auto check = [&](WriteDoc &doc, const QString &what) {
            const QByteArray x = documentXml(doc.toDocx());
            QVERIFY2(x.contains("<w:delText>gone</w:delText>") && x.contains("w:id=\"9\""), qPrintable(what + QLatin1String(": the deletion is gone: ") + QString::fromUtf8(x)));
            QVector<QByteArray> withText;
            for (int from = x.indexOf("<w:p>"); from >= 0; from = x.indexOf("<w:p>", from + 1)) {
                const QByteArray para = x.mid(from, x.indexOf("</w:p>", from) - from);
                if (para.contains("<w:t>") || para.contains("<w:t ")) withText << para;
            }
            QCOMPARE(withText.size(), 1);   // (one, and two, are one paragraph)
            QVERIFY2(!withText[0].contains("w:id=\"9\"") && withText[0].contains("one") && withText[0].contains("two"), qPrintable(what + QLatin1String(": ") + QString::fromUtf8(withText[0])));
            QCOMPARE(doc.pager().lines(0).size(), 1);
        };
        {
            auto doc = open(makeDocx(body));
            QVERIFY(doc);
            QTextCursor c(doc->story());
            c.setPosition(doc->story()->findBlockByNumber(2).position());
            deleteBack(*doc, c, false);
            check(*doc, QStringLiteral("Backspace"));
        }
        {
            auto doc = open(makeDocx(body));
            QVERIFY(doc);
            QTextCursor c(doc->story());
            c.setPosition(doc->story()->findBlockByNumber(0).position() + 3);
            deleteForward(*doc, c, false);
            check(*doc, QStringLiteral("Delete"));
        }
    }

    // A selection that begins or ends in a paragraph that shows nothing because a tracked change deleted it begins or ends where the paragraphs that show do:
    // what survives is never left in the deleted paragraph.
    void aSelectionAtADeletedParagraphLeavesNoTextInIt()
    {
        const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"", mark = " w:id=\"9\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
        const QByteArray body = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:rPr><w:del" + mark + "/></w:rPr></w:pPr><w:del" + rev + "><w:r><w:delText>gone</w:delText></w:r></w:del></w:p>"
                                "<w:p><w:r><w:t>two</w:t></w:r></w:p>";
        auto textParagraphsKeepNoDeletedMark = [&](WriteDoc &doc, const QString &what) {
            const QByteArray x = documentXml(doc.toDocx());
            for (int from = x.indexOf("<w:p>"); from >= 0; from = x.indexOf("<w:p>", from + 1)) {
                const QByteArray para = x.mid(from, x.indexOf("</w:p>", from) - from);
                if ((para.contains("<w:t>") || para.contains("<w:t ")) && para.contains("w:id=\"9\"")) return QStringLiteral("%1: text in the deleted paragraph: %2").arg(what, QString::fromUtf8(para));
            }
            return QString();
        };
        auto run = [&](const char *name, std::function<void(QTextDocument *, QTextCursor &)> select, const QString &expected) {
            auto doc = open(makeDocx(body));
            QVERIFY(doc);
            QTextCursor c(doc->story());
            select(doc->story(), c);
            deleteBack(*doc, c, false);
            QVERIFY2(textParagraphsKeepNoDeletedMark(*doc, QString::fromLatin1(name)).isEmpty(), qPrintable(textParagraphsKeepNoDeletedMark(*doc, QString::fromLatin1(name))));
            QString shown;
            for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) shown += b.text().remove(QChar::ObjectReplacementCharacter) + QLatin1Char('|');
            QCOMPARE(shown, expected);
        };
        // From the start of the deleted paragraph to inside the next: the next loses its letters; the deleted one is not what was selected.
        run("from the deleted paragraph", [](QTextDocument *d, QTextCursor &c) {
            c.setPosition(d->findBlockByNumber(1).position());
            c.setPosition(d->findBlockByNumber(2).position() + 1, QTextCursor::KeepAnchor);
        }, QStringLiteral("one||wo|"));
        // From inside the paragraph before to the start of the deleted one: only the letters go.
        run("to the deleted paragraph", [](QTextDocument *d, QTextCursor &c) {
            c.setPosition(d->findBlockByNumber(0).position() + 1);
            c.setPosition(d->findBlockByNumber(1).position(), QTextCursor::KeepAnchor);
        }, QStringLiteral("o||two|"));
    }

    // A run that asks for ligatures (w14:ligatures) has them in the layout, by the value it states; the file keeps the element as it was.
    void aRunThatAsksForLigaturesIsReadSo()
    {
        const QByteArray ns = " xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
        auto ligaturesOf = [&](const char *val) {
            const QByteArray body = "<w:p><w:r><w:rPr><w14:ligatures" + ns + " w14:val=\"" + val + "\"/></w:rPr><w:t>office</w:t></w:r></w:p>";
            auto doc = open(makeDocx(body));
            if (!doc) return -2;
            const QTextCharFormat f = doc->story()->begin().begin().fragment().charFormat();
            if (!f.hasProperty(tp::Ligatures)) return -1;
            QTextCursor c(doc->story());
            insertText(*doc, c, QStringLiteral("x"));
            if (!documentXml(doc->toDocx()).contains(QByteArray("w14:val=\"") + val + "\"")) return -3;
            return f.intProperty(tp::Ligatures);
        };
        QCOMPARE(ligaturesOf("none"), 1);
        QCOMPARE(ligaturesOf("standard"), 0);
        QCOMPARE(ligaturesOf("standardContextual"), 0);
        QCOMPARE(ligaturesOf("all"), 2);
        QCOMPARE(ligaturesOf("discretional"), 2);
        auto doc = open(makeDocx("<w:p><w:r><w:t>office</w:t></w:r></w:p>"));
        QVERIFY(doc);
        QVERIFY(!doc->story()->begin().begin().fragment().charFormat().hasProperty(tp::Ligatures));
    }

    // A paragraph that turns overflowPunct off, itself or by its style, does not let punctuation hang; the file keeps the element as it was.
    void overflowPunctOffIsReadFromTheParagraphAndItsStyle()
    {
        const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Tight\"><w:name w:val=\"Tight\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:overflowPunct w:val=\"0\"/></w:pPr></w:style>";
        auto noHang = [&](const QByteArray &props, int block) {
            auto doc = open(makeDocx("<w:p><w:pPr>" + props + "</w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r></w:p>", styles));
            return doc && doc->story()->findBlockByNumber(block).blockFormat().boolProperty(tp::NoHangPunct);
        };
        QVERIFY(noHang("<w:overflowPunct w:val=\"0\"/>", 0));
        QVERIFY(!noHang("<w:overflowPunct/>", 0));
        QVERIFY(!noHang("", 0));
        QVERIFY(noHang("<w:pStyle w:val=\"Tight\"/>", 0));
        QVERIFY(!noHang("<w:pStyle w:val=\"Tight\"/><w:overflowPunct/>", 0));   // (the paragraph says it does over its style)
        QVERIFY(!noHang("<w:overflowPunct w:val=\"0\"/>", 1));               // (the next paragraph is not the one)
        auto doc = open(makeDocx("<w:p><w:pPr><w:overflowPunct w:val=\"0\"/></w:pPr><w:r><w:t>one</w:t></w:r></w:p>", styles));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        insertText(*doc, c, QStringLiteral("x"));
        QVERIFY(documentXml(doc->toDocx()).contains("<w:overflowPunct w:val=\"0\"/>"));
    }

    // Clearing the formatting of text keeps what a tracked change deleted deleted: its tabs and breaks do not come back into the lines, and the file keeps
    // them as they were.
    void clearingFormattingKeepsDeletedTextDeleted()
    {
        const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
        const QByteArray body = "<w:p><w:r><w:t>one</w:t></w:r></w:p><w:p><w:r><w:t>two</w:t></w:r><w:del" + rev + "><w:r><w:tab/><w:br/><w:delText>x</w:delText></w:r></w:del></w:p>"
                                "<w:p><w:r><w:t>three</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const auto before = doc->pager().lines(0);
        QCOMPARE(before.size(), 3);
        QTextCursor c(doc->story());
        c.select(QTextCursor::Document);
        clearFormatting(*doc, c);
        const auto after = doc->pager().lines(0);
        QCOMPARE(after.size(), 3);
        for (int i = 0; i < 3; ++i) QVERIFY(std::abs(after[i].rect.top() - before[i].rect.top()) < 0.01);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:del") && x.contains("<w:delText>x</w:delText>") && x.contains("<w:br/>") && !x.contains("vanish"), x.constData());
    }

    // An endnote that a tracked change deleted is no endnote in the numbering: the one after it shows the number the first would.
    void aDeletedEndnoteReferenceTakesNoNumber()
    {
        const QByteArray rev = " w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"";
        const QByteArray body = "<w:p><w:r><w:t>a</w:t></w:r><w:del" + rev + "><w:r><w:endnoteReference w:id=\"1\"/></w:r></w:del><w:r><w:t>b</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QStringList shown;
        for (QTextBlock::iterator it = doc->story()->begin().begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (cf.hasProperty(prop::Atom) && unpackAtom(storedBytes(cf, prop::Atom)).tag == QLatin1String("endnoteReference")) shown << cf.stringProperty(tp::Field);
        }
        QCOMPARE(shown, QStringList({QStringLiteral("raw:i"), QStringLiteral("raw:i")}));   // (the deleted one is "i" too, as nothing shows it)
    }

    // A file that says its hanging indents add no tab stop (noTabHangInd) is read so, by its value.
    void hangingIndentsAreNoTabStopsWhereTheFileSaysSo()
    {
        auto noStop = [&](const QByteArray &option) {
            auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + option + "</w:compat>"));
            return doc && doc->flow().env.wordNoHangStop;
        };
        QVERIFY(!noStop(""));
        QVERIFY(noStop("<w:noTabHangInd/>"));
        QVERIFY(!noStop("<w:noTabHangInd w:val=\"0\"/>"));
    }

    // A file that does not use the web page's way of spacing paragraphs has the space after one and the space before the next added; it is the larger of the
    // two otherwise.
    void paragraphSpaceAddsInAFileThatTurnsTheWebWayOff()
    {
        auto sums = [&](const QByteArray &option) {
            auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + option + "</w:compat>"));
            return doc && doc->flow().env.wordSpaceSums;
        };
        QVERIFY(!sums(""));
        QVERIFY(sums("<w:doNotUseHTMLParagraphAutoSpacing/>"));
        QVERIFY(sums("<w:doNotUseHTMLParagraphAutoSpacing w:val=\"1\"/>"));
        QVERIFY(!sums("<w:doNotUseHTMLParagraphAutoSpacing w:val=\"0\"/>"));
        // The pages the paginator makes carry it, so that a page's text is spaced as the paginator spaced it.
        auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat><w:doNotUseHTMLParagraphAutoSpacing/></w:compat>"));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        QVERIFY(page->doc->wordSpaceSums);
    }

    // Spacing a person sets on paragraphs that have automatic spacing is the spacing they have: the automatic gap goes, on the page and in the file.
    void spacingSetOnAutomaticSpacingReplacesIt()
    {
        auto para = [](const char *text) { return QByteArray("<w:p><w:pPr><w:spacing w:beforeAutospacing=\"1\" w:afterAutospacing=\"1\"/></w:pPr><w:r><w:t>") + text + "</w:t></w:r></w:p>"; };
        // (A file whose styles give spacing of their own, so that none set on a paragraph is written.)
        const QByteArray styles = "<w:docDefaults><w:pPrDefault><w:pPr><w:spacing w:before=\"100\" w:after=\"200\"/></w:pPr></w:pPrDefault></w:docDefaults>"
                                  "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>";
        auto doc = open(makeDocx(para("one") + para("two") + para("three"), styles));
        QVERIFY(doc);
        auto pitch = [&] {
            const auto lines = doc->pager().lines(0);
            return lines.size() >= 2 ? lines[1].rect.top() - lines[0].rect.top() : -1.0;
        };
        const double automatic = pitch();
        QTextCursor c(doc->story());
        c.select(QTextCursor::Document);
        setSpacing(*doc, c, 0, 0);
        const double none = pitch();
        QVERIFY2(automatic - none > 10, qPrintable(QStringLiteral("automatic %1, set to none %2").arg(automatic).arg(none)));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("Autospacing"), x.constData());
        QVERIFY2(x.contains("w:before=\"0\"") && x.contains("w:after=\"0\""), x.constData());
        // (Spacing before alone, as the paragraph fields of the ribbon and the dialog set it, takes the automatic spacing before and keeps the one after.)
        auto again = open(makeDocx(para("one") + para("two"), styles));
        QTextCursor c2(again->story());
        c2.select(QTextCursor::Document);
        changeBlockFormat(*again, c2, [](QTextBlockFormat &bf) { setSpaceBefore(bf, 3); });
        const QByteArray y = documentXml(again->toDocx());
        QVERIFY2(!y.contains("beforeAutospacing") && y.contains("afterAutospacing=\"1\""), y.constData());
        QVERIFY2(y.contains("w:before=\"60\""), y.constData());
    }

    // A header's last paragraph's space after is part of the header: the text starts below it.
    void aHeadersSpaceAfterTakesRoomOnThePage()
    {
        auto top = [&](int after) {
            const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"
                                    "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
            const QByteArray head = "<w:pPr><w:spacing w:after=\"" + QByteArray::number(after) + "\"/></w:pPr><w:r><w:t>HEAD</w:t></w:r>";
            auto doc = open(makeDocxWithParts(paragraphs(3), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {head})}}));
            return doc ? doc->pager().page(0).content.top() : -1.0;
        };
        const double plain = top(0), spaced = top(400);   // (twenty points)
        QVERIFY(plain > 0 && spaced > 0);
        QVERIFY2(std::abs(spaced - plain - 20) < 0.01, qPrintable(QStringLiteral("body top %1 against %2").arg(spaced).arg(plain)));
    }

    // A paragraph whose mark a tracked change deleted is joined to the next: an empty paragraph after one that shows text is the rest of it and takes no room,
    // but after one that shows nothing (the deleted paragraph itself takes none) it is a blank line as it was.
    void aParagraphAfterOneWithADeletedMarkThatShowsNothingTakesNoRoom()
    {
        const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
        auto para = [&](const char *text, bool markDeleted) {
            return "<w:p><w:pPr>" + QByteArray(markDeleted ? deleted : QByteArray()) + "</w:pPr>" + (*text ? "<w:r><w:t>" + QByteArray(text) + "</w:t></w:r>" : QByteArray()) + "</w:p>";
        };
        auto gap = [&](const QByteArray &body) {
            auto doc = open(makeDocx(body, kStyles));
            const auto lines = doc ? doc->pager().lines(0) : QVector<StoryLayout::LineBrief>();
            return lines.size() >= 2 ? lines.last().rect.top() - lines.first().rect.top() : -1.0;
        };
        const double normal = gap(para("alpha", false) + para("", false) + para("gamma", false));   // (the text, a blank line, the text)
        const double joined = gap(para("alpha", true) + para("", false) + para("gamma", false));   // (the blank line is the rest of "alpha")
        QVERIFY2(normal > 0 && joined > 0, "no lines");
        QVERIFY2(std::abs(joined * 2 - normal) < 0.01, qPrintable(QStringLiteral("a line less: %1 against %2").arg(joined).arg(normal)));
        const double nothing = gap(para("alpha", false) + para("", true) + para("", false) + para("gamma", false));   // (alpha, one blank line, gamma)
        QVERIFY2(std::abs(nothing - normal) < 0.01, qPrintable(QStringLiteral("a deleted empty paragraph is no line: %1 against %2").arg(nothing).arg(normal)));
    }

    // What a paste from another document brings that points into the first's notes and objects points at nothing of the second's: a reference to a note is
    // left out (its text isn't there), and an object's place on the page isn't carried (the object isn't there); the rest of what was copied is.
    void aPasteFromAnotherDocumentLeavesNoDanglingNoteOrObject()
    {
        const QByteArray ns = "xmlns:v=\"urn:schemas-microsoft-com:vml\"";
        const QByteArray body = "<w:p " + ns + "><w:r><w:t xml:space=\"preserve\">Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:pict><v:rect id=\"r\" style=\"position:absolute;margin-left:10pt;margin-top:5pt;width:50pt;height:20pt\" fillcolor=\"red\"/></w:pict></w:r><w:r><w:t> end.</w:t></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>The note.</w:t></w:r></w:p></w:footnote></w:footnotes>";
        auto source = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(source);
        auto has = [](const QTextDocument *d, auto test) {
            for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it)
                    if (test(it.fragment().charFormat())) return true;
            return false;
        };
        QVERIFY(has(source->story(), [](const QTextCharFormat &f) { return f.stringProperty(tp::Field).startsWith(QLatin1String("footnote:")); }));
        QVERIFY(has(source->story(), [](const QTextCharFormat &f) { return f.hasProperty(flow::fp::AnchorId); }));
        QTextCursor sel(source->story());
        sel.select(QTextCursor::Document);
        const Clip clip = copySelection(*source, sel);
        auto target = open(makeDocx("<w:p><w:r><w:t>Target.</w:t></w:r></w:p>", kStyles));
        QVERIFY(target);
        QTextCursor t(target->story());
        t.movePosition(QTextCursor::End);
        pasteClip(*target, t, clip);
        QVERIFY2(!has(target->story(), [](const QTextCharFormat &f) { return f.stringProperty(tp::Field).startsWith(QLatin1String("footnote:")); }), "a reference to a note of the other document");
        QVERIFY2(!has(target->story(), [](const QTextCharFormat &f) { return f.hasProperty(flow::fp::AnchorId); }), "an object's place from the other document");
        const QByteArray saved = target->toDocx();
        office::Package p;
        QVERIFY(p.read(saved));
        const QByteArray x = p.part(p.mainPart());
        QVERIFY2(!x.contains("footnoteReference"), x.constData());
        QVERIFY(x.contains("Cited") && x.contains(" end."));
        QVERIFY2(rt::danglingReferences(saved).isEmpty(), qPrintable(rt::danglingReferences(saved).join(QLatin1Char(' '))));
    }

    // A symbol of a font the machine lacks is set in a stand-in on the page only: what is saved of a file nobody touched has the run as it was.
    void aSymbolOfAMissingFontIsSavedAsItWas()
    {
        const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">Before </w:t></w:r><w:r><w:sym w:font=\"No Such Face Anywhere\" w:char=\"0047\"/></w:r><w:r><w:t xml:space=\"preserve\"> after</w:t></w:r></w:p>";
        // (A file whose styles say what font the text is in, as files do.)
        const QByteArray styles = "<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/></w:rPr></w:rPrDefault></w:docDefaults>"
                                  "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>";
        const QByteArray original = makeDocx(body, styles);
        auto doc = open(original);
        QVERIFY(doc);
        const QByteArray saved = doc->toDocx();
        const QStringList loss = rt::lossProblems(original, saved);
        QVERIFY2(loss.isEmpty(), qPrintable(loss.join(QLatin1String("; "))));
        office::Package pkg;
        QVERIFY(pkg.read(saved));
        const QString main = QString::fromUtf8(pkg.part(pkg.mainPart()));
        QVERIFY2(!main.contains(QLatin1String("rFonts")), qPrintable(main.mid(main.indexOf(QLatin1String("<w:body")), 600)));
    }

    // The round-trip check itself: a run that gains a property the file never had is a difference, though what it adds is one the program understands.
    void theRoundTripCheckSeesAGainedRunProperty()
    {
        auto withRun = [&](const QByteArray &props) { return makeDocx("<w:p><w:r>" + props + "<w:t>text</w:t></w:r></w:p>"); };
        const QByteArray plain = withRun(QByteArray()), gained = withRun("<w:rPr><w:rFonts w:ascii=\"Foo\"/></w:rPr>");
        QVERIFY(rt::lossProblems(plain, plain).isEmpty());
        QVERIFY(rt::lossProblems(gained, plain).isEmpty());   // (one left out is normalizing)
        const QStringList added = rt::lossProblems(plain, gained);
        QVERIFY2(!added.isEmpty() && added.first().contains(QLatin1String("gains run properties")), qPrintable(added.join(QLatin1String("; "))));
    }

    // The round-trip check reads a property as a reader does: an on/off property said "false", "0", or "off" is off, and one said "true", "1", "on", or with no value is on,
    // whichever the file spelled; hidden text that is not hidden is nothing; an underline of none that names a color, and a font said for the face alone, say what the
    // underline and the fonts said. What the file never said is still a gain, and another font is another font.
    void theRoundTripCheckReadsPropertiesAsAReaderDoes()
    {
        auto withRun = [&](const QByteArray &props) { return makeDocx("<w:p><w:r><w:rPr>" + props + "</w:rPr><w:t>text</w:t></w:r></w:p>"); };
        auto same = [&](const QByteArray &was, const QByteArray &now) { return rt::lossProblems(withRun(was), withRun(now)).isEmpty(); };
        QVERIFY(same("<w:b w:val=\"false\"/>", "<w:b w:val=\"0\"/>"));
        QVERIFY(same("<w:caps w:val=\"off\"/><w:smallCaps w:val=\"0\"/>", "<w:caps w:val=\"false\"/><w:smallCaps w:val=\"false\"/>"));
        QVERIFY(same("<w:i w:val=\"true\"/><w:strike w:val=\"on\"/>", "<w:i/><w:strike w:val=\"1\"/>"));
        QVERIFY(same("<w:vanish w:val=\"false\"/>", QByteArray()));
        QVERIFY(same("<w:u w:val=\"none\" w:color=\"112233\"/>", "<w:u w:val=\"none\"/>"));
        QVERIFY(same("<w:rFonts w:ascii=\"Face\" w:hAnsi=\"Face\" w:cs=\"Face\"/>", "<w:rFonts w:cs=\"Face\"/>"));
        QVERIFY(!same(QByteArray(), "<w:b w:val=\"0\"/>"));
        QVERIFY(!same("<w:b/>", "<w:b w:val=\"0\"/>"));
        QVERIFY(!same("<w:rFonts w:ascii=\"Face\" w:cs=\"Face\"/>", "<w:rFonts w:cs=\"Other\"/>"));
        QVERIFY(!same("<w:u w:val=\"none\"/>", "<w:u w:val=\"single\"/>"));
        QVERIFY(!same("<w:vanish/>", QByteArray()));
        QVERIFY(!same(QByteArray(), "<w:vanish/>"));
    }

    // Text holds one kind of capitals and one kind of strike-through: a run in capitals over a style that says small capitals is in capitals, and one in a double strike
    // over a style's single strike is double. What the run says of the other kind is not "off" for the format has no room for it: it is not given an off that the file never
    // said. A person who turns the capitals off over a style of small capitals still saves the look they see (no small capitals).
    void capitalsAndDoubleStrikeOverTheStylesOtherKindAreSavedAsTheFileSaidThem()
    {
        const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Small\"><w:name w:val=\"Small\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:smallCaps/><w:strike/></w:rPr></w:style>";
        const QByteArray original = makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Small\"/></w:pPr><w:r><w:rPr><w:caps/><w:dstrike/></w:rPr><w:t>Both</w:t></w:r></w:p>", styles);
        auto doc = open(original);
        QVERIFY(doc);
        const QByteArray saved = doc->toDocx();
        const QByteArray x = documentXml(saved);
        QVERIFY2(x.contains("<w:caps/>") && x.contains("<w:dstrike/>"), x.constData());
        QVERIFY2(!x.contains("smallCaps") && !x.contains("<w:strike"), x.constData());
        const QStringList loss = rt::lossProblems(original, saved);
        QVERIFY2(loss.isEmpty(), qPrintable(loss.join(QLatin1String("; "))));
        // (The person turns the capitals off: the text is not in small capitals on the screen, so the file says so.)
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.setPosition(4, QTextCursor::KeepAnchor);
        changeCharFormat(*doc, c, [](QTextCharFormat &f) { f.setFontCapitalization(QFont::MixedCase); });
        const QByteArray after = documentXml(doc->toDocx());
        QVERIFY2(after.contains("<w:smallCaps w:val=\"0\"/>") && !after.contains("<w:caps"), after.constData());
    }

    // Bold in a character style over a paragraph style that is bold turns it off, as it does in the original program; a run that says bold itself over both is bold, and is saved
    // saying so: the file's own bold is not left out as if it were the style's.
    void aToggleOfACharacterStyleFlipsTheParagraphStylesAndADirectOneStaysSaid()
    {
        const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Head\"><w:name w:val=\"Head\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:b/><w:i/></w:rPr></w:style>"
                                  "<w:style w:type=\"character\" w:styleId=\"Strong\"><w:name w:val=\"Strong\"/><w:rPr><w:b/></w:rPr></w:style>";
        const QByteArray original = makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Head\"/></w:pPr><w:r><w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t>flipped</w:t></w:r>"
                                             "<w:r><w:rPr><w:rStyle w:val=\"Strong\"/><w:b/></w:rPr><w:t>said</w:t></w:r></w:p>",
                                             styles);
        auto looks = [](WriteDoc &doc) {
            QTextCursor c(doc.story());
            c.setPosition(3);
            const QTextCharFormat first = c.charFormat();
            c.setPosition(doc.story()->toPlainText().indexOf(QLatin1String("said")) + 2);
            return QVector<QTextCharFormat>{first, c.charFormat()};
        };
        auto doc = open(original);
        QVERIFY(doc);
        QVector<QTextCharFormat> f = looks(*doc);
        QVERIFY2(f[0].fontWeight() < QFont::DemiBold && f[0].fontItalic(), "the character style's bold flips the heading's");
        QVERIFY2(f[1].fontWeight() >= QFont::DemiBold && f[1].fontItalic(), "bold said by the run itself");
        const QByteArray saved = doc->toDocx();
        const QByteArray x = documentXml(saved);
        QVERIFY2(x.contains("<w:rStyle w:val=\"Strong\"/><w:b/></w:rPr><w:t xml:space=\"preserve\">said"), x.constData());
        QVERIFY2(x.contains("<w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t xml:space=\"preserve\">flipped"), x.constData());
        const QStringList loss = rt::lossProblems(original, saved);
        QVERIFY2(loss.isEmpty(), qPrintable(loss.join(QLatin1String("; "))));
        auto back = open(saved);
        QVERIFY(back);
        f = looks(*back);
        QVERIFY(f[0].fontWeight() < QFont::DemiBold && f[1].fontWeight() >= QFont::DemiBold);
    }

    // A file that flags none of its paragraph styles as the default has paragraphs that say "Normal": they are saved saying it (the program takes Normal for the default of such
    // a file, and another reader may not, so leaving it out could change the paragraph's look). A file that flags Normal as its default has it left out of the paragraphs, as before.
    void aParagraphStyleIsSavedSaidWhenTheFileFlagsNoDefault()
    {
        auto saved = [&](const QByteArray &normalAttrs) {
            const QByteArray styles = "<w:style w:type=\"paragraph\"" + normalAttrs + " w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:rPr><w:b/></w:rPr></w:style>";
            const QByteArray original = makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Normal\"/></w:pPr><w:r><w:t>said</w:t></w:r></w:p>", styles);
            auto doc = open(original);
            return doc ? documentXml(doc->toDocx()) : QByteArray();
        };
        const QByteArray unflagged = saved(QByteArray());
        QVERIFY2(unflagged.contains("<w:pStyle w:val=\"Normal\"/>"), unflagged.constData());
        const QByteArray flagged = saved(" w:default=\"1\"");
        QVERIFY2(!flagged.contains("<w:pStyle"), flagged.constData());
        // (A file with no styles has none to say.)
        auto bare = open(makeDocx("<w:p><w:r><w:t>none</w:t></w:r></w:p>"));
        QVERIFY(bare);
        QVERIFY(!documentXml(bare->toDocx()).contains("<w:pStyle"));
    }

    // An empty paragraph that ends a section, when the next section is continuous, takes no room; before a section on a page of its own it does.
    void anEmptyBreakParagraphBeforeAContinuousSectionTakesNoRoom()
    {
        auto linesWith = [&](const QByteArray &type) {
            const QByteArray first = "<w:p><w:r><w:t>First</w:t></w:r></w:p><w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr></w:pPr></w:p>";
            const QByteArray second = "<w:p><w:r><w:t>Second</w:t></w:r></w:p>";
            const QByteArray last = "<w:sectPr>" + type + "<w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
            auto doc = open(makeDocxWithParts(first + second, last, {}));
            return doc ? doc->pager().lines(0) : QVector<StoryLayout::LineBrief>();
        };
        const auto joined = linesWith("<w:type w:val=\"continuous\"/>");
        QVERIFY(joined.size() >= 3);
        // (The second paragraph is where it would be with no break paragraph between.)
        const QByteArray plain = "<w:p><w:r><w:t>First</w:t></w:r></w:p><w:p><w:r><w:t>Second</w:t></w:r></w:p>";
        auto alone = open(makeDocxWithParts(plain, "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>", {}));
        QVERIFY(alone);
        const auto direct = alone->pager().lines(0);
        QVERIFY(direct.size() >= 2);
        QVERIFY2(std::abs(joined[2].rect.top() - direct[1].rect.top()) < 0.01, qPrintable(QString::number(joined[2].rect.top() - direct[1].rect.top())));
        // (Before a section on a page of its own the empty paragraph keeps its line: the section after starts on the next page.)
        const auto apart = linesWith(QByteArray());
        QCOMPARE(int(apart.size()), 2);
    }

    // The empty paragraph that ends a section before a continuous section stays on the page the text before it ends on, even when the page has no room left for its
    // line (it takes none): the section after, which needs a page of its own for its other paper, is the second page, not the third.
    void anEmptyBreakParagraphAtTheFootOfAFullPageStaysThere()
    {
        auto line = [](const char *text) {
            return "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>" + QByteArray(text) + "</w:t></w:r></w:p>";
        };
        QByteArray body;
        for (int i = 0; i < 50; ++i) body += line("Filling");   // (600 points of the 648 of the text area)
        const QByteArray margins = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
        // (The break paragraph's own line is 60 points: more than the 48 that are left.)
        body += "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"1200\" w:lineRule=\"exact\"/><w:sectPr>" + margins + "</w:sectPr></w:pPr></w:p>" + line("Second");
        const QByteArray last = "<w:sectPr><w:type w:val=\"continuous\"/><w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, last, {}));
        QVERIFY(doc);
        QCOMPARE(doc->pager().pageCount(), 2);
        const auto second = doc->pager().lines(1);
        QVERIFY(!second.isEmpty());
        QVERIFY2(std::abs(second[0].rect.top()) < 0.01, qPrintable(QString::number(second[0].rect.top())));   // (the section after begins at the top of its page)
    }

    // A paragraph that begins with a page break and is kept with the next puts the break at the foot of the page the text before it ends on: the text after the break
    // is on the next page, so the keeping is not of that page's room (and no page holds only the break).
    void aKeptParagraphThatBeginsWithAPageBreakStaysOnTheFullPage()
    {
        // (A line of 596 of the 648 points of the text area; the break's paragraph is of 40-point text, which takes about 46 of the 52 that are left, and what follows it a line of
        // 12-point text, which does not fit after it: kept with that, the paragraph would leave the page.)
        const QByteArray filler = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"11920\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>Filling</w:t></w:r></w:p>";
        const QByteArray big = "<w:p><w:pPr><w:keepNext/><w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr><w:r><w:rPr><w:sz w:val=\"80\"/></w:rPr><w:br w:type=\"page\"/></w:r><w:r><w:rPr><w:sz w:val=\"80\"/></w:rPr><w:t>Chapter</w:t></w:r></w:p>";
        const QByteArray after = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr><w:r><w:t>After.</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(filler + big + after, sect, {}));
        QVERIFY(doc);
        QCOMPARE(doc->pager().pageCount(), 2);
        QCOMPARE(int(doc->pager().lines(0).size()), 2);   // (the filler's line and the break's)
        const auto second = doc->pager().lines(1);
        QVERIFY(second.size() >= 2);
        QVERIFY2(std::abs(second[0].rect.top()) < 0.01, qPrintable(QString::number(second[0].rect.top())));
    }

    // A symbol of a font that isn't there is its character in the stand-in (Calibri): the line it is on is Calibri's, taller than Times New Roman's.
    void aSymbolOfAMissingFontMakesItsLineTheStandIns()
    {
        auto run = [](const QByteArray &inner) { return "<w:r><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/></w:rPr>" + inner + "</w:r>"; };
        auto para = [&](const QByteArray &inner) { return "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>" + run(inner) + "</w:p>"; };
        const QByteArray body = para("<w:t>Plain words</w:t>") + para("<w:t>Words </w:t></w:r><w:r><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/></w:rPr><w:sym w:font=\"No Such Face Anywhere\" w:char=\"0047\"/>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const auto lines = doc->pager().lines(0);
        QVERIFY(lines.size() >= 2);
        QVERIFY2(std::abs(lines[0].rect.height() - 12 * (1825 + 443 + 87) / 2048.0) < 0.02, qPrintable(QString::number(lines[0].rect.height())));
        QVERIFY2(std::abs(lines[1].rect.height() - 12 * (1950 + 550) / 2048.0) < 0.02, qPrintable(QString::number(lines[1].rect.height())));
    }

    // A multiple of the line that is written as a negative number (a file converted from an older format has them) is the same multiple as a positive
    // one: the line is as tall as it would be for its size, not as one with nothing in it.
    void aNegativeMultipleOfTheLineIsReadAsItsSize()
    {
        auto lineOf = [](const char *line) {
            const QByteArray body = QByteArray("<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"") + line + "\" w:lineRule=\"auto\"/></w:pPr><w:r><w:t>Words</w:t></w:r></w:p>";
            auto doc = open(makeDocx(body));
            return doc->pager().lines(0).first().rect.height();
        };
        const double single = lineOf("240");
        QVERIFY(single > 5);
        QVERIFY2(std::abs(lineOf("-480") - 2 * single) < 0.02, qPrintable(QString::number(lineOf("-480"))));
        QVERIFY2(std::abs(lineOf("-120") - lineOf("120")) < 0.02, qPrintable(QString::number(lineOf("-120"))));
    }

    // A number in a list is the size its level gives it (half-points, as a bullet's is), not the text's: a ten-point number before twelve-point text.
    void aListNumberHasTheSizeItsLevelGives()
    {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/>"
                                     "<w:pPr><w:ind w:left=\"360\" w:hanging=\"360\"/></w:pPr><w:rPr><w:sz w:val=\"20\"/></w:rPr></w:lvl></w:abstractNum>"
                                     "<w:abstractNum w:abstractNumId=\"1\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                     "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>";
        const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:rPr><w:sz w:val=\"24\"/></w:rPr><w:t>Sized</w:t></w:r></w:p>"
                                "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"2\"/></w:numPr></w:pPr><w:r><w:rPr><w:sz w:val=\"24\"/></w:rPr><w:t>Plain</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles, numbering));
        QVERIFY(doc);
        const QTextBlock sized = doc->story()->begin(), plain = sized.next();
        QVERIFY(sized.textList() && plain.textList());
        QCOMPARE(sized.textList()->format().property(tp::BulletSize).toDouble(), 10.0);
        QVERIFY(!plain.textList()->format().hasProperty(tp::BulletSize));
    }

    // A page number in a frame (set in the middle or at the side, anchored to the text) takes no room: a footer of it and an empty paragraph is
    // one line tall, and the number is set as the frame says.
    void aFrameParagraphInAFooterTakesNoRoom()
    {
        const QByteArray sect = "<w:sectPr><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                                "<w:pgMar w:top=\"1080\" w:right=\"1440\" w:bottom=\"1080\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto footerWith = [](const QByteArray &frame) {
            return headerPart("ftr", {"<w:pPr>" + frame + "</w:pPr><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>", QByteArray()});
        };
        auto bottomOf = [&](const QByteArray &frame) {
            auto doc = open(makeDocxWithParts(paragraphs(60, 40), sect, {{QStringLiteral("/word/footer1.xml"), footerWith(frame)}}));
            return std::make_pair(doc->pager().page(0).content.bottom(), doc->pager().materialize(0)->doc->pages[0]->items.size());
        };
        const auto framed = bottomOf("<w:framePr w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"center\" w:y=\"1\"/>");
        const auto plain = bottomOf(QByteArray());
        // As one line (the empty paragraph's), the footer leaves the text its bottom margin; as two it takes more.
        QVERIFY2(std::abs(framed.first - (792 - 54)) < 0.01, qPrintable(QString::number(framed.first)));
        QVERIFY2(plain.first < framed.first - 5, qPrintable(QString::number(plain.first)));
    }

    // A paragraph of a footer whose mark a tracked change deleted, with nothing else of its own, is gone from the footer as the text stands: it takes no room
    // (twenty points a line here), so the text keeps what the footer leaves it.
    void aDeletedParagraphOfAFooterTakesNoRoom()
    {
        const QByteArray sect = "<w:sectPr><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                                "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray spacing = "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"400\" w:lineRule=\"exact\"/>";
        const QByteArray gone = "<w:pPr>" + spacing + "<w:rPr><w:del w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"/></w:rPr></w:pPr>"
                                "<w:del w:id=\"8\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"><w:r><w:delText>gone</w:delText></w:r></w:del>";
        const QByteArray footer = headerPart("ftr", {"<w:pPr>" + spacing + "</w:pPr><w:r><w:t>ONE</w:t></w:r>", gone, "<w:pPr>" + spacing + "</w:pPr><w:r><w:t>TWO</w:t></w:r>"});
        auto doc = open(makeDocxWithParts(paragraphs(60, 40), sect, {{QStringLiteral("/word/footer1.xml"), footer}}));
        QVERIFY(doc);
        // Two lines of twenty points under the footer distance of thirty-six: the text ends at 792 - 36 - 40.
        QVERIFY2(std::abs(doc->pager().page(0).content.bottom() - (792 - 36 - 40)) < 0.01, qPrintable(QString::number(doc->pager().page(0).content.bottom())));
    }

    // A file's footnotes are at the foot of the pages that cite them: the note in a box at the bottom of the text area with a rule above it, its mark
    // the note's number, and the reference in the text showing the same number; saving keeps the part and the reference as they were.
    void aFilesFootnotesAreAtTheFootOfItsPages()
    {
        const QByteArray body = "<w:p><w:r><w:t>First paragraph with a note</w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:t xml:space=\"preserve\"> and more words after it.</w:t></w:r></w:p>" + paragraphs(40, 40);
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> FOOTNOTE TWO says something.</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray zip = makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}});
        auto doc = open(zip);
        QVERIFY(doc);
        QVERIFY(doc->pager().pageCount() >= 2);
        const auto page = doc->pager().materialize(0);
        const TextItem *note = boxWithText(*page, QStringLiteral("FOOTNOTE TWO"));
        QVERIFY(note);
        const double bottom = 792 - 72;
        QVERIFY2(std::abs(note->rect.bottom() - bottom) < 0.1, qPrintable(QString::number(note->rect.bottom())));
        // The mark that begins the note and the reference in the text show the number.
        auto showsOne = [&](const QString &text) {
            const TextItem *box = boxWithText(*page, text);
            if (!box) return false;
            for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it)
                    if (it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("raw:1")) return true;
            return false;
        };
        QVERIFY(showsOne(QStringLiteral("FOOTNOTE TWO")));
        QVERIFY(showsOne(QStringLiteral("First paragraph")));
        bool rule = false;
        for (const auto &item : page->doc->pages[0]->items) rule |= dynamic_cast<const LineItem *>(item.get()) != nullptr;
        QVERIFY(rule);
        // Kept as it was: the part, and the reference in the paragraph.
        const QByteArray saved = doc->toDocx();
        office::Package before, after;
        before.read(zip);
        after.read(saved);
        QCOMPARE(after.part(QStringLiteral("/word/footnotes.xml")), before.part(QStringLiteral("/word/footnotes.xml")));
        QCOMPARE(int(documentXml(saved).count("footnoteReference")), 1);
        // A page without the reference has no note.
        const auto next = doc->pager().materialize(1);
        QVERIFY(!boxWithText(*next, QStringLiteral("FOOTNOTE TWO")));
    }

    // The room between the text and the notes is the file's own separator paragraph, not a line of the Normal style: a separator thirty points tall
    // (exactly) leaves the text fifty lines of twelve points above a note one line tall, where the Normal style's line would leave it one more.
    void theNoteSeparatorIsTheFilesOwnParagraph()
    {
        const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
        QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
        for (int i = 0; i < 120; ++i) body += "<w:p>" + exact + "<w:r><w:t>x</w:t></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr>"
                                 "<w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Note.</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        const auto lines = doc->pager().lines(0);
        QVERIFY(lines.size() > 10);
        QVERIFY2(std::abs(lines.last().rect.bottom() - lines.first().rect.top() - 50 * 12) < 0.01, qPrintable(QString::number(lines.last().rect.bottom() - lines.first().rect.top())));
    }

    // Automatic spacing (the 14 points of a web page's paragraphs) is none between the items of one list, of whatever level, and 14 where a list follows another.
    void autoSpacingIsNoneInAListAndFourteenBetweenLists()
    {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/></w:lvl>"
                                     "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/></w:lvl></w:abstractNum>"
                                     "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/></w:num>";
        auto item = [](int num, int level, const char *text) {
            return "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"" + QByteArray::number(level) + "\"/><w:numId w:val=\"" + QByteArray::number(num) + "\"/></w:numPr>"
                   "<w:spacing w:before=\"100\" w:beforeAutospacing=\"1\" w:after=\"100\" w:afterAutospacing=\"1\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>" + QByteArray(text) + "</w:t></w:r></w:p>";
        };
        auto doc = open(makeDocx(item(1, 0, "a") + item(1, 1, "b") + item(1, 0, "c") + item(2, 0, "d"), kStyles, numbering));
        QVERIFY(doc);
        const auto lines = doc->pager().lines(0);
        QCOMPARE(int(lines.size()), 4);
        // (Lines of twelve points exactly: the items of one list follow one another, and the next list has the fourteen points between.)
        QVERIFY2(std::abs(lines[1].rect.top() - lines[0].rect.top() - 12) < 0.01, qPrintable(QString::number(lines[1].rect.top() - lines[0].rect.top())));
        QVERIFY2(std::abs(lines[2].rect.top() - lines[1].rect.top() - 12) < 0.01, qPrintable(QString::number(lines[2].rect.top() - lines[1].rect.top())));
        QVERIFY2(std::abs(lines[3].rect.top() - lines[2].rect.top() - 26) < 0.01, qPrintable(QString::number(lines[3].rect.top() - lines[2].rect.top())));
    }

    // A line of a word processor's text is not broken after a slash ("and/or", an address): it breaks at spaces and hyphens, and a word longer than the line anywhere.
    void aLineIsNotBrokenAfterASlash()
    {
        const char *words[] = {"alpha/beta/gamma", "delta/epsilon", "zeta/eta/theta/iota", "kappa", "lambda/mu"};
        QByteArray text;
        for (int i = 0; i < 90; ++i) text += QByteArray(words[(i * 7) % 5]) + " ";
        auto doc = open(makeDocx("<w:p><w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r></w:p>", kStyles));
        QVERIFY(doc);
        const auto lines = doc->pager().lines(0);
        QVERIFY(lines.size() > 8);
        const QString plain = doc->story()->toPlainText();
        for (const auto &l : lines) {
            const QString shown = plain.mid(l.pos, l.end - l.pos).trimmed();
            QVERIFY2(!shown.endsWith(QLatin1Char('/')), qPrintable(shown));
        }
    }

    // A rule drawn as a rectangle that is anchored in a paragraph of its own (or at the end of one) is drawn where the paragraph is; a shape that says it has no fill or no
    // line has none (not the fill and line a new shape is given).
    void aRuleAnchoredAtAParagraphsEndIsDrawnAsTheFileSaysIt()
    {
        const QByteArray vml = "xmlns:v=\"urn:schemas-microsoft-com:vml\"";
        auto rect = [&](const QByteArray &attributes) {
            return "<w:r><w:pict " + vml + "><v:rect id=\"r\" style=\"position:absolute;margin-left:1.5pt;margin-top:-2.25pt;width:300pt;height:1.4pt\" " + attributes + "/></w:pict></w:r>";
        };
        auto shapeOf = [&](const QByteArray &attributes, QRectF *rectOut) -> std::shared_ptr<ShapeItem> {
            auto doc = open(makeDocx("<w:p><w:r><w:t>Heading</w:t></w:r></w:p><w:p>" + rect(attributes) + "</w:p><w:p><w:r><w:t>Below</w:t></w:r></w:p>", kStyles));
            if (!doc) return nullptr;
            const auto lines = doc->pager().lines(0);
            const auto page = doc->pager().materialize(0);
            for (const auto &item : page->doc->pages[0]->items)
                if (auto shape = std::dynamic_pointer_cast<ShapeItem>(item)) {
                    *rectOut = QRectF(QPointF(shape->rect.left(), shape->rect.top() - doc->pager().page(0).content.top() - lines[1].rect.top()), shape->rect.size());
                    return shape;
                }
            return nullptr;
        };
        QRectF at;
        auto black = shapeOf("fillcolor=\"black\" stroked=\"f\"", &at);
        QVERIFY(black);
        QVERIFY(black->fill.type == Fill::Solid && black->stroke.isNone());
        QVERIFY2(std::abs(at.top() - (-2.25)) < 0.01 && std::abs(at.width() - 300) < 0.01, qPrintable(QString::number(at.top())));
        auto outlined = shapeOf("filled=\"f\" strokecolor=\"#165189\" strokeweight=\"1.5pt\"", &at);
        QVERIFY(outlined);
        QVERIFY(outlined->fill.isNone());
        QVERIFY2(!outlined->stroke.isNone() && std::abs(outlined->stroke.width - 1.5) < 0.01, qPrintable(QString::number(outlined->stroke.width)));
    }

    // A number or bullet counts in its line by its ascent over the text's descent: a thirty-six point bullet before twelve-point text makes the line its ascent and the text's
    // descent, not its whole height (the room it has below the baseline is not the line's).
    void aBulletRaisesItsLineByItsAscentOverTheTextsDescent()
    {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"\xEF\x82\xB7\"/>"
                                     "<w:rPr><w:rFonts w:ascii=\"Symbol\" w:hAnsi=\"Symbol\"/><w:sz w:val=\"72\"/></w:rPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>"
                                "<w:r><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/></w:rPr><w:t>Item</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles, numbering));
        QVERIFY(doc);
        const auto lines = doc->pager().lines(0);
        QVERIFY(!lines.isEmpty());
        // (The Symbol line has the ascent 2059 and the Times New Roman the descent 443, of 2048 to the em.)
        const double want = 36 * 2059 / 2048.0 + 12 * 443 / 2048.0;
        QVERIFY2(std::abs(lines[0].rect.height() - want) < 0.05, qPrintable(QString::number(lines[0].rect.height()) + " want " + QString::number(want)));
    }

    // A footnote reference that says its mark follows it as text (w:customMarkFollows) shows no number and takes none: the notes after it count from where they were.
    void aFootnoteWithACustomMarkTakesNoNumber()
    {
        const QByteArray body = "<w:p><w:r><w:t>First</w:t></w:r><w:r><w:footnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r><w:r><w:t xml:space=\"preserve\"> second</w:t></w:r>"
                                "<w:r><w:footnoteReference w:id=\"3\"/></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p><w:r><w:t>* CUSTOM note</w:t></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"3\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> AUTO note</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        auto marks = [&](const QString &text) {
            QStringList out;
            const TextItem *box = boxWithText(*page, text);
            if (!box) return QStringList{QStringLiteral("no box")};
            for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                    if (code.startsWith(QLatin1String("raw:"))) out << code;
                }
            return out;
        };
        QCOMPARE(marks(QStringLiteral("second")), (QStringList{QStringLiteral("raw:"), QStringLiteral("raw:1")}));   // (the custom reference shows nothing; the other is the first number)
        QCOMPARE(marks(QStringLiteral("AUTO note")), (QStringList{QStringLiteral("raw:1")}));
    }

    // The notes of a page sit at its foot whatever follows the page: the first paragraph of the next section, which a section break takes to the next page, has a note of its own,
    // and the room for it is not kept under this page's notes.
    void aSectionBreakDoesNotKeepNoteRoomOnThePageBefore()
    {
        const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
        const QByteArray margins = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
        const QByteArray body = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/><w:sectPr>" + margins + "</w:sectPr></w:pPr><w:r><w:t>One</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>"
                                "<w:p>" + exact + "<w:r><w:t>Two</w:t></w:r><w:r><w:footnoteReference w:id=\"3\"/></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> FIRST note</w:t></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"3\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> SECOND note</w:t></w:r></w:p></w:footnote></w:footnotes>";
        auto doc = open(makeDocxWithParts(body, "<w:sectPr>" + margins + "</w:sectPr>", {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        QCOMPARE(doc->pager().pageCount(), 2);
        const auto page = doc->pager().materialize(0);
        const TextItem *note = boxWithText(*page, QStringLiteral("FIRST note"));
        QVERIFY(note);
        QVERIFY2(std::abs(note->rect.bottom() - (792 - 72)) < 0.1, qPrintable(QString::number(note->rect.bottom())));
    }

    // A file that gives the notes a continuation notice has a page keep room for it under its notes (a word processor does, whether or not a note goes on): the notes end the
    // notice's height (twenty points, exactly) above the foot of the text area.
    void aContinuationNoticeKeepsItsRoomUnderTheNotes()
    {
        const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
        const QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:type=\"continuationNotice\" w:id=\"1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"400\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>(continued)</w:t></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> THE note</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        const TextItem *note = boxWithText(*page, QStringLiteral("THE note"));
        QVERIFY(note);
        QVERIFY2(std::abs(note->rect.bottom() - (792 - 72 - 20)) < 0.1, qPrintable(QString::number(note->rect.bottom())));
    }

    // The rule above the notes is drawn at three fifths of the separator paragraph (a line of thirty points here), not the fifth of it that a short rule's place was taken from.
    void theNoteRuleIsThreeFifthsDownTheSeparator()
    {
        const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
        const QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr>"
                                 "<w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> RULED note</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        const TextItem *note = boxWithText(*page, QStringLiteral("RULED note"));
        QVERIFY(note);
        double rule = -1;
        for (const auto &item : page->doc->pages[0]->items)
            if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line->p1.y();
        QVERIFY(rule > 0);
        // (The note begins where the separator ends: thirty points under the top of it; the rule is eighteen under the top, twelve above the note.)
        QVERIFY2(std::abs(note->rect.top() - rule - 12) < 0.01, qPrintable(QString::number(note->rect.top() - rule)));
    }

    // A separator that is taller than a quarter of the text area is cut to that quarter: one of two hundred paragraphs leaves a page its text (a hundred and sixty-two points less
    // than the room above the note) and its note, and the document lays out.
    void aTallNoteSeparatorIsCutToAQuarterOfThePage()
    {
        const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
        QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
        for (int i = 0; i < 120; ++i) body += "<w:p>" + exact + "<w:r><w:t>x</w:t></w:r></w:p>";
        QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\">";
        for (int i = 0; i < 200; ++i) notes += "<w:p>" + exact + "<w:r><w:t>.</w:t></w:r></w:p>";
        notes += "</w:footnote><w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> TALL note</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        QVERIFY(doc->pager().pageCount() >= 3);
        const auto lines = doc->pager().lines(0);
        QVERIFY(lines.size() > 10);
        // (The text area is 648 points: a quarter is 162; the note is twelve, so the text ends 162 + 12 above the foot: thirty-nine lines of twelve.)
        QVERIFY2(std::abs(lines.last().rect.bottom() - lines.first().rect.top() - 39 * 12) < 0.01, qPrintable(QString::number(lines.last().rect.bottom() - lines.first().rect.top())));
        QVERIFY(boxWithText(*doc->pager().materialize(0), QStringLiteral("TALL note")));
        // A continuation notice of as many is cut to a quarter as well: the note, the separator, and the notice take 12 + 162 + 162, and the text, in lines of ten points, thirty-one.
        QByteArray notice = "<w:footnote w:type=\"continuationNotice\" w:id=\"1\">";
        for (int i = 0; i < 200; ++i) notice += "<w:p>" + exact + "<w:r><w:t>.</w:t></w:r></w:p>";
        notice += "</w:footnote>";
        QByteArray both = notes;
        both.replace("<w:footnote w:id=\"2\">", notice + "<w:footnote w:id=\"2\">");
        QByteArray tens = body;
        tens.replace("w:line=\"240\"", "w:line=\"200\"");
        auto second = open(makeDocxWithParts(tens, sect, {{QStringLiteral("/word/footnotes.xml"), both}}));
        QVERIFY(second);
        const auto more = second->pager().lines(0);
        QVERIFY(more.size() > 10);
        QVERIFY2(std::abs(more.last().rect.bottom() - more.first().rect.top() - 31 * 10) < 0.01, qPrintable(QString::number(more.last().rect.bottom() - more.first().rect.top())));
    }

    // With the compatibility option that a number's tab does not use the hanging indent (w:doNotUseIndentAsNumberingTabStop), the text after a number begins at the next tab
    // stop past it, of the paragraph's own or the default ones, not at the indent; the lines after the first are at the indent.
    void aNumbersTabSkipsTheHangingIndentWhenTheFileSaysSo()
    {
        // Where the first line's text begins, in points from the text area's left edge: the left-most dark pixel of its row past the number, in the page as it is printed.
        double second = -1;   // where the second line of the last paragraph began
        auto startOf = [&](const QByteArray &settings, const QByteArray &levelTabs, const QByteArray &paragraphTabs) {
            const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1)\"/>"
                                         "<w:pPr>" + levelTabs + "<w:ind w:left=\"1440\" w:hanging=\"1440\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
            QByteArray words;
            for (int i = 0; i < 40; ++i) words += "HHHH ";
            const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr>" + paragraphTabs + "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/>"
                                    "<w:ind w:left=\"1440\" w:hanging=\"1440\"/></w:pPr><w:r><w:t xml:space=\"preserve\">" + words + "</w:t></w:r></w:p>";
            const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
            auto doc = open(makeDocxWithParts(body, sect, {}, settings, kStyles, numbering));
            const auto lines = doc->pager().lines(0);
            second = lines.size() > 1 ? lines[1].rect.left() : -1;
            PageView view;
            view.resize(1000, 700);
            view.setDocument(doc.get());
            const QImage img = view.pageImage(0, 816);   // (96 to the inch: 4/3 pixel to the point; the first line is the 16 pixels under the top margin of 96)
            const double perPoint = 816 / 612.0;
            for (int x = int(std::ceil((72 + 18) * perPoint)); x < img.width(); ++x)   // (a number is under 18 points wide)
                for (int y = int(72 * perPoint); y < int(84 * perPoint); ++y)
                    if (qGray(img.pixel(x, y)) < 128) return x / perPoint - 72;
            return -1.0;
        };
        const QByteArray flag = "<w:compat><w:doNotUseIndentAsNumberingTabStop/></w:compat>";
        // Without the option the text begins at the indent, 72 points.
        QVERIFY2(std::abs(startOf(QByteArray(), QByteArray(), QByteArray()) - 72) < 2, qPrintable(QString::number(startOf(QByteArray(), QByteArray(), QByteArray()))));
        // With it: the first stop after the number is the default one at 36 points (720 twentieths); the lines after the first are at the indent all the same.
        QVERIFY2(std::abs(startOf(flag, QByteArray(), QByteArray()) - 36) < 2, qPrintable(QString::number(startOf(flag, QByteArray(), QByteArray()))));
        QCOMPARE(second, 72.0);
        // The paragraph's own stop is taken (50 points), and so is the level's list tab (54 points) when the paragraph has none of its own.
        QVERIFY2(std::abs(startOf(flag, QByteArray(), "<w:tabs><w:tab w:val=\"left\" w:pos=\"1000\"/></w:tabs>") - 50) < 2, "own stop");
        QVERIFY2(std::abs(startOf(flag, "<w:tabs><w:tab w:val=\"num\" w:pos=\"1080\"/></w:tabs>", QByteArray()) - 54) < 2, "list tab");
        // A stop of the level that the paragraph clears is gone.
        QVERIFY2(std::abs(startOf(flag, "<w:tabs><w:tab w:val=\"num\" w:pos=\"1080\"/></w:tabs>", "<w:tabs><w:tab w:val=\"clear\" w:pos=\"1080\"/></w:tabs>") - 36) < 2, "cleared stop");
    }

    // A bullet whose level asks for the outline effect (w:outline) is drawn hollow, as an outlined box is: the middle of a big black square is dark, and that of the same
    // square outlined is not.
    void theCheckBoxOfAnOutlinedBulletIsHollow()
    {
        auto middleOf = [&](const QByteArray &effect) {
            const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"\xE2\x96\xA0\"/>"
                                         "<w:pPr><w:ind w:left=\"1440\" w:hanging=\"720\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Arial\" w:hAnsi=\"Arial\"/>" + effect + "<w:sz w:val=\"96\"/></w:rPr></w:lvl></w:abstractNum>"
                                         "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
            const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>Item</w:t></w:r></w:p>";
            const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
            auto doc = open(makeDocxWithParts(body, sect, {}, QByteArray(), kStyles, numbering));
            PageView view;
            view.resize(1000, 700);
            view.setDocument(doc.get());
            const QImage img = view.pageImage(0, 816);
            // The marker's dark pixels: left of the text, which begins at 72 + 72 points from the page's edge.
            const double perPoint = 816 / 612.0;
            QRect ink;
            for (int y = int(60 * perPoint); y < int(300 * perPoint); ++y)
                for (int x = int(72 * perPoint); x < int(140 * perPoint); ++x)
                    if (qGray(img.pixel(x, y)) < 215) ink = ink.isNull() ? QRect(x, y, 1, 1) : ink.united(QRect(x, y, 1, 1));   // (a hairline is a pale gray)
            if (ink.width() < 8) return -1;
            return qGray(img.pixel(ink.center()));
        };
        const int solid = middleOf(QByteArray()), hollow = middleOf("<w:outline/>"), notOutlined = middleOf("<w:outline w:val=\"0\"/>");
        QVERIFY2(solid >= 0 && solid < 100, qPrintable(QString::number(solid)));
        QVERIFY2(notOutlined >= 0 && notOutlined < 100, qPrintable(QString::number(notOutlined)));
        QVERIFY2(hollow > 235, qPrintable(QString::number(hollow)));
    }

    // The notes of a page are laid out the same way when the document has a table anywhere (the page is then laid out beside its tables): a separator thirty points tall and a notice
    // twenty tall, a note in a cell, and a section break after which the next section's first paragraph has a note of its own.
    void theNotesOfAPageAreLaidOutTheSameWithATableInTheDocument()
    {
        const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
        const QByteArray margins = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
        auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
        auto para = [&](const QByteArray &runs, const QByteArray &pPrExtra = QByteArray()) { return "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/>" + pPrExtra + "</w:pPr>" + runs + "</w:p>"; };
        auto table = [&](const QByteArray &cell) {
            return "<w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr>" + cell + "</w:tc></w:tr></w:tbl>";
        };
        auto note = [&](int id, const char *t) { return "<w:footnote w:id=\"" + QByteArray::number(id) + "\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r>" + text(t) + "</w:p></w:footnote>"; };
        auto notesPart = [&](const QByteArray &separator, const QByteArray &notice, const QByteArray &notes) {
            return "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\">" + separator + "</w:footnote>" + (notice.isEmpty() ? QByteArray() : "<w:footnote w:type=\"continuationNotice\" w:id=\"1\">" + notice + "</w:footnote>") + notes + "</w:footnotes>";
        };
        const QByteArray separator30 = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr><w:r><w:separator/></w:r></w:p>";
        const QByteArray notice20 = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"400\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>(continued)</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr>" + margins + "</w:sectPr>";
        auto ruleOf = [](const flow::MaterializedPage &page) {
            double rule = -1;
            for (const auto &item : page.doc->pages[0]->items)
                if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line->p1.y();
            return rule;
        };
        {
            // A note in the text and one in a cell: the last ends the notice's height above the foot, the rule is 12 above the first (the separator's 30 less its 18).
            const QByteArray body = para(text("Cited") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>") + table(para(text("In the cell") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r>")) + para(text("After"));
            auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notesPart(separator30, notice20, note(2, " ONE note") + note(3, " TWO note"))}}));
            QVERIFY(doc);
            const auto page = doc->pager().materialize(0);
            const TextItem *one = boxWithText(*page, QStringLiteral("ONE note")), *two = boxWithText(*page, QStringLiteral("TWO note"));
            QVERIFY(one && two);
            QVERIFY2(std::abs(two->rect.bottom() - (792 - 72 - 20)) < 0.1, qPrintable(QString::number(two->rect.bottom())));
            QVERIFY2(std::abs(one->rect.bottom() - two->rect.top()) < 0.1, qPrintable(QString::number(one->rect.bottom())));
            QVERIFY2(std::abs(one->rect.top() - ruleOf(*page) - 12) < 0.05, qPrintable(QString::number(one->rect.top() - ruleOf(*page))));
        }
        {
            // A table opens the first section, and the first paragraph of the next section has a note and is followed by one: the page before has its notes at the foot, all the same.
            const QByteArray body = table(para(text("Top cell"))) + para(text("One") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>", "<w:sectPr>" + margins + "</w:sectPr>") +
                                    para(text("Two") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r>") + table(para(text("in a cell"))) + para(text("End"));
            auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notesPart(separator30, QByteArray(), note(2, " FIRST note") + note(3, " SECOND note"))}}));
            QVERIFY(doc);
            QCOMPARE(doc->pager().pageCount(), 2);
            const TextItem *first = boxWithText(*doc->pager().materialize(0), QStringLiteral("FIRST note"));
            QVERIFY(first);
            QVERIFY2(std::abs(first->rect.bottom() - (792 - 72)) < 0.1, qPrintable(QString::number(first->rect.bottom())));
        }
    }

    // A reference whose mark follows it takes no number among the notes of a page with a table either: a custom one, an automatic note, one in a cell, and one after the table are
    // numbered 1, 2, and 3.
    void aCustomMarkTakesNoNumberAmongTheNotesOfATablesPage()
    {
        auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
        auto ref = [](int id) { return "<w:r><w:footnoteReference w:id=\"" + QByteArray::number(id) + "\"/></w:r>"; };
        const QByteArray custom = "<w:r><w:footnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r>";
        const QByteArray cell = "<w:p>" + text("In the cell") + ref(4) + "</w:p>";
        const QByteArray body = "<w:p>" + text("First") + custom + text(" second") + ref(3) + "</w:p><w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                                "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr>" + cell + "</w:tc></w:tr></w:tbl><w:p>" + text("After") + ref(5) + "</w:p>";
        auto note = [&](int id, const char *t) { return "<w:footnote w:id=\"" + QByteArray::number(id) + "\"><w:p><w:r><w:footnoteRef/></w:r>" + text(t) + "</w:p></w:footnote>"; };
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p>" + text("* custom note") + "</w:p></w:footnote>" + note(3, " AUTOA note") + note(4, " AUTOB note") + note(5, " AUTOC note") + "</w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        auto marks = [&](const QString &needle) {
            QStringList out;
            const TextItem *box = boxWithText(*page, needle);
            if (!box) return QStringList{QStringLiteral("no box")};
            for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                    if (code.startsWith(QLatin1String("raw:"))) out << code;
                }
            return out;
        };
        QCOMPARE(marks(QStringLiteral("AUTOA note")), (QStringList{QStringLiteral("raw:1")}));
        QCOMPARE(marks(QStringLiteral("AUTOB note")), (QStringList{QStringLiteral("raw:2")}));
        QCOMPARE(marks(QStringLiteral("AUTOC note")), (QStringList{QStringLiteral("raw:3")}));
    }

    // Clearing the formatting leaves a note reference's custom mark as it was: the next automatic note is still the first, and the file still says the mark follows.
    void clearingTheFormattingKeepsACustomNoteMark()
    {
        auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
        const QByteArray body = "<w:p>" + text("First") + "<w:r><w:footnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r>" + text(" second") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p>" + text("* custom note") + "</w:p></w:footnote><w:footnote w:id=\"3\"><w:p><w:r><w:footnoteRef/></w:r>" + text(" AUTO note") + "</w:p></w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.select(QTextCursor::Document);
        clearFormatting(*doc, c);
        bool kept = false;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) kept |= it.fragment().charFormat().boolProperty(tp::NoteMarkCustom);
        QVERIFY(kept);
        const auto page = doc->pager().materialize(0);
        const TextItem *box = boxWithText(*page, QStringLiteral("AUTO note"));
        QVERIFY(box);
        QString mark;
        for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("raw:"))) mark = it.fragment().charFormat().stringProperty(tp::Field);
        QCOMPARE(mark, QStringLiteral("raw:1"));
        QVERIFY(documentXml(doc->toDocx()).contains("customMarkFollows"));
    }

    // A level's outline effect is looked for once for the level, not once for each item that is in the list: two hundred items of one level look at its run properties once.
    void aLevelsOutlineEffectIsLookedForOncePerLevel()
    {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/><w:rPr><w:outline/></w:rPr></w:lvl></w:abstractNum>"
                                     "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        QByteArray body;
        for (int i = 0; i < 200; ++i) body += "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>item</w:t></w:r></w:p>";
        const int before = levelsScannedForOutlineForTest();
        auto doc = open(makeDocx(body, kStyles, numbering));
        QVERIFY(doc);
        QVERIFY(doc->story()->begin().textList()->format().boolProperty(tp::BulletOutline));
        QCOMPARE(levelsScannedForOutlineForTest() - before, 1);
    }

    // The separator is cut to a quarter of the text area of the page it is on, not of the smallest page of the file: a section with a body of forty points does not shorten it on the
    // full-size pages (a rule twelve above the note, the separator's thirty less its eighteen); on its own pages it is ten, and the rule four above the note.
    void theNoteSeparatorIsCutPerSection()
    {
        const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
        auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
        auto note = [&](int id, const char *t) { return "<w:footnote w:id=\"" + QByteArray::number(id) + "\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r>" + text(t) + "</w:p></w:footnote>"; };
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr>"
                                 "<w:r><w:separator/></w:r></w:p></w:footnote>" + note(2, " BIG note") + note(3, " SMALL note") + "</w:footnotes>";
        const QByteArray full = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
        const QByteArray small = "<w:pgSz w:w=\"12240\" w:h=\"2240\"/><w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/>";
        const QByteArray body = "<w:p>" + exact.left(exact.size() - 8) + "<w:sectPr>" + full + "</w:sectPr></w:pPr>" + text("Big") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>"
                                "<w:p>" + exact + text("Small") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r></w:p>";
        auto doc = open(makeDocxWithParts(body, "<w:sectPr>" + small + "</w:sectPr>", {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        QCOMPARE(doc->pager().pageCount(), 2);
        auto gap = [&](int index, const QString &needle) {
            const auto page = doc->pager().materialize(index);
            const TextItem *n = boxWithText(*page, needle);
            double rule = -1;
            for (const auto &item : page->doc->pages[0]->items)
                if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line->p1.y();
            return n && rule > 0 ? n->rect.top() - rule : -1.0;
        };
        QVERIFY2(std::abs(gap(0, QStringLiteral("BIG note")) - 12) < 0.05, qPrintable(QString::number(gap(0, QStringLiteral("BIG note")))));
        QVERIFY2(std::abs(gap(1, QStringLiteral("SMALL note")) - 4) < 0.05, qPrintable(QString::number(gap(1, QStringLiteral("SMALL note")))));
    }

    // A note of a reference with a custom mark has no number to show for the reference mark its text begins with: it shows nothing (not a 0).
    void aCustomNotesOwnReferenceMarkShowsNothing()
    {
        auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
        const QByteArray body = "<w:p>" + text("Cited") + "<w:r><w:footnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r>" + text(" BODY text") + "</w:p></w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        const TextItem *box = boxWithText(*page, QStringLiteral("BODY text"));
        QVERIFY(box);
        QStringList marks;
        for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("raw:"))) marks << it.fragment().charFormat().stringProperty(tp::Field);
        QCOMPARE(marks, (QStringList{QStringLiteral("raw:")}));
    }

    // An endnote reference whose mark follows it as text shows no number and takes none, as a footnote's does.
    void anEndnoteWithACustomMarkTakesNoNumber()
    {
        auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
        const QByteArray body = "<w:p>" + text("First") + "<w:r><w:endnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r>" + text(" second") + "<w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        const TextItem *box = boxWithText(*page, QStringLiteral("second"));
        QVERIFY(box);
        QStringList marks;
        for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("raw:"))) marks << it.fragment().charFormat().stringProperty(tp::Field);
        QCOMPARE(marks, (QStringList{QStringLiteral("raw:"), QStringLiteral("raw:i")}));
    }

    // Items of different lists are told apart by their numbering id whether or not the level they name is in the definition (an item moved to a level the list lacks).
    void listsAreToldApartWhateverTheLevel()
    {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/></w:lvl></w:abstractNum>"
                                     "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/></w:num>";
        auto doc = open(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>", kStyles, numbering));
        QVERIFY(doc);
        QCOMPARE(doc->listFormat(1, 0).intProperty(tp::ListKey), 1);
        QCOMPARE(doc->listFormat(2, 0).intProperty(tp::ListKey), 2);
        QCOMPARE(doc->listFormat(1, 7).intProperty(tp::ListKey), 1);   // (a level the definition lacks)
        QCOMPARE(doc->listFormat(2, 7).intProperty(tp::ListKey), 2);
    }

    // A numbered level's outline effect draws its numbers hollow too.
    void theOutlineEffectReachesNumbers()
    {
        auto doc = open(makeDocx("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>item</w:t></w:r></w:p>", kStyles,
                                 "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:rPr><w:outline/></w:rPr></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>"));
        QVERIFY(doc);
        QVERIFY(doc->story()->begin().textList()->format().boolProperty(tp::BulletOutline));
    }

    // A bullet drawn to the box the reference pages measured (Wingdings' heavy arrow) is that box outlined when its level has the outline effect, not the font's own glyph at its own
    // size: solid and outlined, it has the same box to a pixel or two.
    void anOutlinedBulletOfAMeasuredFontIsItsBoxOutlined()
    {
        auto boxOf = [&](const QByteArray &effect) {
            const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"\xEF\x83\xA8\"/>"
                                         "<w:pPr><w:ind w:left=\"1440\" w:hanging=\"720\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Wingdings\" w:hAnsi=\"Wingdings\"/>" + effect + "<w:sz w:val=\"96\"/></w:rPr></w:lvl></w:abstractNum>"
                                         "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
            const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>Item</w:t></w:r></w:p>";
            const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
            auto d = open(makeDocxWithParts(body, sect, {}, QByteArray(), kStyles, numbering));
            PageView view;
            view.resize(1000, 700);
            view.setDocument(d.get());
            const QImage img = view.pageImage(0, 816);
            const double perPoint = 816 / 612.0;
            QRect ink;
            for (int y = int(60 * perPoint); y < int(300 * perPoint); ++y)
                for (int x = int(60 * perPoint); x < int(140 * perPoint); ++x)
                    if (qGray(img.pixel(x, y)) < 215) ink = ink.isNull() ? QRect(x, y, 1, 1) : ink.united(QRect(x, y, 1, 1));
            return ink;
        };
        const QRect solid = boxOf(QByteArray()), hollow = boxOf("<w:outline/>");
        QVERIFY(solid.width() > 8 && hollow.width() > 8);
        QVERIFY2(std::abs(solid.left() - hollow.left()) <= 2 && std::abs(solid.top() - hollow.top()) <= 2 && std::abs(solid.width() - hollow.width()) <= 2 && std::abs(solid.height() - hollow.height()) <= 2,
                 qPrintable(QStringLiteral("%1,%2 %3x%4 against %5,%6 %7x%8").arg(solid.left()).arg(solid.top()).arg(solid.width()).arg(solid.height()).arg(hollow.left()).arg(hollow.top()).arg(hollow.width()).arg(hollow.height())));
    }

    // A section break in the middle of a long document lays out the pages around it, not those before it again; and Enter in a document of twenty
    // thousand sections makes no more than the sections it changed (counted, not timed).
    void editingSectionsTouchesOnlyWhatChanged()
    {
        auto doc = open(makeDocx(paragraphs(3000, 60), kStyles));
        const int pages = doc->pager().pageCount();
        QVERIFY(pages > 200);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(1500).position() + 5);
        const qint64 laid = doc->pager().pagesLaidOut();
        insertSectionBreak(*doc, c);
        doc->pager().paginate(doc->pager().pageOfPosition(c.position()) + 2);
        QVERIFY2(doc->pager().pagesLaidOut() - laid < 12, qPrintable(QString::number(doc->pager().pagesLaidOut() - laid)));
        QCOMPARE(doc->sectionCount(), 2);
        QVERIFY(doc->pager().pageCount() >= pages);   // (and the rest settles, one page more for the section's own)
        // Twenty thousand sections.
        QByteArray body;
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        for (int i = 0; i < 20000; ++i) body += "<w:p><w:pPr>" + sect + "</w:pPr><w:r><w:t>section " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        auto many = open(makeDocx(body, kStyles));
        QCOMPARE(many->sectionCount(), 20001);
        const qint64 made = many->sectionsMadeForTest();
        QTextCursor m(many->story());
        m.setPosition(many->story()->findBlockByNumber(10000).position() + 5);
        insertParagraph(*many, m);
        QVERIFY2(many->sectionsMadeForTest() - made <= 3, qPrintable(QString::number(many->sectionsMadeForTest() - made)));
        QCOMPARE(many->sectionCount(), 20001);   // (the break stays with the second half: still 20,000 breaks and the last)
        insertText(*many, m, QStringLiteral("x"));
        QVERIFY(many->sectionsMadeForTest() - made <= 3);
        // A page setup change of one section makes one.
        wml::SectionProps p = many->sectionProps(5000);
        p.pageW = 15840;
        p.pageH = 12240;
        const qint64 before = many->sectionsMadeForTest();
        many->setSectionProps(5000, p);
        QVERIFY(many->sectionsMadeForTest() - before <= 2);
        QCOMPARE(many->sectionProps(5000).pageW, 15840);
        QCOMPARE(many->sectionProps(4999).pageW, 12240);
    }

    void aSectionThatEndsTheFileLeavesABlankLastPageThatIsDrawn()
    {
        // The only paragraph ends a next-page section and nothing follows: the second section has no text, so its page has no lines.
        const QByteArray one = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocx("<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>the only words</w:t></w:r></w:p>", kStyles, QByteArray(), one));
        doc->pager().complete();
        QCOMPARE(doc->pager().pageCount(), 2);
        PageView view;
        view.resize(1000, 900);
        view.setDocument(doc.get());
        view.show();
        view.setZoomMode(PageView::ZoomMode::ManyPages, 2, 1);
        const QImage img = view.grab().toImage();
        const QRect blank = view.pageRect(1);
        QVERIFY(blank.width() > 100 && blank.right() < img.width());
        int ink = 0, white = 0;
        for (int y = blank.top() + 4; y < std::min(blank.bottom() - 4, img.height()); ++y)   // (inside the sheet, clear of its shadow)
            for (int x = blank.left() + 4; x < blank.right() - 4; ++x) (img.pixelColor(x, y).lightness() > 250 ? white : ink) += 1;
        QVERIFY2(ink == 0 && white > 0, "the last page is a blank sheet");
        // The caret ends the text, on the first page, and the keys that cross pages leave it there.
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::End);
        view.setCursor(c);
        QCOMPARE(view.currentPage(), 0);
        view.typeKey(Qt::Key_PageDown, Qt::NoModifier);
        view.typeKey(Qt::Key_Down, Qt::NoModifier);
        QVERIFY(view.cursor().position() <= doc->story()->characterCount() - 1);
        // Saving keeps the two sections.
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:sectPr")), 2);
    }

    void sectionsHaveTheirOwnPages()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("first section"));
        insertSectionBreak(*doc, c);
        typeText(*doc, c, QStringLiteral("second section"));
        QCOMPARE(doc->sectionCount(), 2);
        QCOMPARE(doc->pager().pageCount(), 2);
        // The second is landscape with two columns (as the Layout tab sets them), and it survives the file.
        wml::SectionProps s = doc->sectionProps(1);
        s.pageW = 15840;
        s.pageH = 12240;
        s.orient = QStringLiteral("landscape");
        s.cols = 2;
        doc->setSectionProps(1, s);
        QCOMPARE(doc->pager().pageCount(), 2);
        QCOMPARE(doc->pager().page(1).size, QSizeF(792, 612));
        QCOMPARE(doc->pager().page(1).columns, 2);
        QCOMPARE(doc->pager().page(0).size, doc->pager().page(0).size);
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("<w:sectPr")), 2);
        QVERIFY(x.contains("w:orient=\"landscape\""));
        QVERIFY(x.contains("<w:cols w:num=\"2\""));
        auto back = open(doc->toDocx());
        QCOMPARE(back->sectionCount(), 2);
        QCOMPARE(back->pager().pageCount(), 2);
        QCOMPARE(back->pager().page(1).size, QSizeF(792, 612));
        // Deleting the break takes the two sections into one.
        QTextCursor d(back->story());
        d.setPosition(back->story()->findBlockByNumber(1).position());
        deleteBack(*back, d, false);
        QCOMPARE(back->sectionCount(), 1);
        QCOMPARE(back->pager().pageCount(), 1);
        // And undo brings it back.
        back->story()->undo(&d);
        QCOMPARE(back->sectionCount(), 2);
        QCOMPARE(back->pager().pageCount(), 2);
    }

    void aSectionBreakInTheMiddleOfTextAlsoUndoes()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("a\nb\nc"));
        c.setPosition(doc->story()->findBlockByNumber(1).position());
        insertSectionBreak(*doc, c);
        QCOMPARE(doc->sectionCount(), 2);
        doc->story()->undo(&c);
        QCOMPARE(doc->sectionCount(), 1);
        doc->story()->redo(&c);
        QCOMPARE(doc->sectionCount(), 2);
    }

    void copyAndPasteKeepTheFormats()
    {
        auto doc = open(makeDocx("<w:p><w:r><w:rPr><w:b/><w:color w:val=\"FF0000\"/></w:rPr><w:t>Bold red</w:t></w:r><w:r><w:t> and plain</w:t></w:r></w:p>", kStyles));
        PageView view;
        view.setDocument(doc.get());
        view.selectAll();
        view.copy();
        view.cursor().movePosition(QTextCursor::End);
        view.insertText(QStringLiteral(" | "));
        view.paste();
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("Bold red and plain | Bold red and plain"));
        QTextCursor t(doc->story());
        t.setPosition(26 + 2);
        QCOMPARE(t.charFormat().fontWeight(), int(QFont::Bold));
        QCOMPARE(t.charFormat().foreground().color(), QColor(0xFF, 0, 0));
        // Cut takes the text out, and undo puts it back.
        view.cursor().setPosition(0);
        view.cursor().setPosition(8, QTextCursor::KeepAnchor);
        view.cut();
        QVERIFY(doc->story()->toPlainText().startsWith(QLatin1String(" and plain")));
        view.undo();
        QVERIFY(doc->story()->toPlainText().startsWith(QLatin1String("Bold red")));
        // Plain text pastes as paragraphs.
        QApplication::clipboard()->setText(QStringLiteral("one\ntwo\r\nthree"));
        view.cursor().movePosition(QTextCursor::End);
        const int blocks = doc->story()->blockCount();
        view.paste();
        QCOMPARE(doc->story()->blockCount(), blocks + 2);
    }

    void piecesKeptAsXmlStayWhenTheTextAroundThemChanges()
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>ab</w:t></w:r><w:bookmarkStart w:id=\"3\" w:name=\"here\"/><w:r><w:t>cd</w:t></w:r><w:bookmarkEnd w:id=\"3\"/><w:r><w:t>ef</w:t></w:r></w:p>", kStyles));
        QCOMPARE(doc->story()->toPlainText().size(), 8);   // (the bookmark's two ends are two characters)
        QTextCursor c(doc->story());
        // Right passes the bookmark on the way to the next letter; Left does the same the other way.
        c.setPosition(2);
        moveCursor(c, QTextCursor::NextCharacter, false);
        QCOMPARE(c.position(), 4);   // over the mark and the letter c
        moveCursor(c, QTextCursor::PreviousCharacter, false);
        QCOMPARE(c.position(), 3);
        moveCursor(c, QTextCursor::PreviousCharacter, false);
        QCOMPARE(c.position(), 1);   // over the mark and the letter b
        // Backspace after the bookmark's end deletes the letter before it, and the bookmark stays.
        c.setPosition(6);
        deleteBack(*doc, c, false);
        QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains("<w:bookmarkStart w:id=\"3\" w:name=\"here\"/>"));
        QVERIFY(x.contains("<w:bookmarkEnd w:id=\"3\"/>"));
        QVERIFY(x.contains(">c</w:t>"));
        QVERIFY(!x.contains(">cd</w:t>"));
        // Selecting across one end and deleting takes the pair out (the file never has half).
        QTextCursor all(doc->story());
        all.setPosition(0);
        all.setPosition(doc->story()->characterCount() - 1, QTextCursor::KeepAnchor);
        removeSelection(*doc, all);
        x = documentXml(doc->toDocx());
        QVERIFY(!x.contains("bookmark"));
    }

    void theStyleOfAParagraphIsWhatTheFileSaysAndNothingMore()
    {
        // The paragraph has the style's look, and its direct formatting is only what it adds: nothing is written twice.
        const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Plain heading</w:t></w:r></w:p>"
                                "<w:p><w:r><w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t>strong</w:t></w:r><w:r><w:rPr><w:i/></w:rPr><w:t> italic</w:t></w:r></w:p>"
                                "<w:p><w:pPr><w:jc w:val=\"right\"/><w:spacing w:before=\"60\"/></w:pPr><w:r><w:t>right</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles));
        QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
        QTextCursor t(doc->story());
        t.setPosition(3);
        QCOMPARE(t.charFormat().fontWeight(), int(QFont::Bold));   // (from the style)
        QCOMPARE(t.charFormat().fontPointSize(), 17.0);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t xml:space=\"preserve\">Plain heading</w:t></w:r></w:p>"), x.constData());
        QVERIFY2(x.contains("<w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t xml:space=\"preserve\">strong</w:t>"), x.constData());
        QVERIFY2(x.contains("<w:rPr><w:i/></w:rPr><w:t xml:space=\"preserve\"> italic</w:t>"), x.constData());
        QVERIFY2(x.contains("<w:pPr><w:spacing w:before=\"60\" w:after=\"120\" w:line=\"264\" w:lineRule=\"auto\"/><w:jc w:val=\"right\"/></w:pPr>") ||
                     x.contains("<w:pPr><w:spacing w:before=\"60\"/><w:jc w:val=\"right\"/></w:pPr>"), x.constData());
        // It comes out of a second round the same.
        QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
    }

    // ---------------------------------------------------------------- .docx
    void aDocumentOpensAndPaginates()
    {
        auto doc = open(makeDocx(paragraphs(300), kStyles, QByteArray(), "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>"));
        QVERIFY(doc);
        QCOMPARE(doc->story()->blockCount(), 300);
        QVERIFY(doc->story()->toPlainText().startsWith(QLatin1String("The ")) || doc->story()->toPlainText().size() > 1000);
        QCOMPARE(doc->pager().page(0).size, QSizeF(595.3, 841.9));   // A4, as the file says
        const int pages = doc->pager().pageCount();
        QVERIFY2(pages > 8 && pages < 40, qPrintable(QString::number(pages)));
        const auto page = doc->pager().materialize(2);
        QVERIFY(page);
        QCOMPARE(page->number, 3);
    }

    void listsInTheFileAreLists()
    {
        const QByteArray numbering =
            "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/>"
            "<w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"&#xF0B7;\"/><w:lvlJc w:val=\"left\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Symbol\" w:hAnsi=\"Symbol\"/></w:rPr></w:lvl></w:abstractNum>"
            "<w:abstractNum w:abstractNumId=\"1\"><w:multiLevelType w:val=\"multilevel\"/>"
            "<w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"432\" w:hanging=\"432\"/></w:pPr></w:lvl>"
            "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.%2\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"576\"/></w:pPr></w:lvl></w:abstractNum>"
            "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>";
        auto item = [](int num, int lvl, const char *t) { return QByteArray("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"") + QByteArray::number(lvl) + "\"/><w:numId w:val=\"" + QByteArray::number(num) + "\"/></w:numPr></w:pPr><w:r><w:t>" + t + "</w:t></w:r></w:p>"; };
        auto doc = open(makeDocx(item(1, 0, "bullet") + item(2, 0, "one") + item(2, 1, "one point one") + item(2, 1, "one point two") + item(2, 0, "two") + item(2, 1, "two point one") + "<w:p><w:r><w:t>plain</w:t></w:r></w:p>" + item(2, 0, "three"), kStyles, numbering));
        QTextDocument *s = doc->story();
        auto marker = [&](int n) {
            const QTextBlock b = s->findBlockByNumber(n);
            return b.blockFormat().stringProperty(tp::BulletChar).isEmpty() ? (b.textList() ? b.textList()->itemText(b) : QString()) : b.blockFormat().stringProperty(tp::BulletChar);
        };
        QVERIFY(s->findBlockByNumber(0).textList());
        QCOMPARE(s->findBlockByNumber(0).blockFormat().leftMargin(), 36.0);      // the level's indents
        QCOMPARE(s->findBlockByNumber(0).blockFormat().textIndent(), -18.0);
        QCOMPARE(marker(1), QStringLiteral("1."));
        QCOMPARE(marker(2), QStringLiteral("1.1"));   // several levels in the marker: made as the file says
        QCOMPARE(marker(3), QStringLiteral("1.2"));
        QCOMPARE(marker(4), QStringLiteral("2."));
        QCOMPARE(marker(5), QStringLiteral("2.1"));   // (restarted under the new second item)
        QVERIFY(!s->findBlockByNumber(6).textList());
        QCOMPARE(marker(7), QStringLiteral("3."));    // (the list goes on after the plain paragraph)
        // The list comes back out as it was.
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("<w:numId w:val=\"2\"/>")), 6);
        QCOMPARE(int(x.count("<w:numId w:val=\"1\"/>")), 1);
        QVERIFY(x.contains("<w:ilvl w:val=\"1\"/>"));
    }

    void whatIsNotUnderstoodIsKeptWhereItWas()
    {
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        const QByteArray body = "<w:p><w:r><w:t>before</w:t></w:r></w:p>" + table +
                                "<w:p><w:r><w:t>after </w:t></w:r><w:r><w:drawing><wp:inline xmlns:wp=\"urn:wp\"><wp:extent cx=\"1\" cy=\"1\"/></wp:inline></w:drawing></w:r>"
                                "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>7</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>"
                                "<w:sdt><w:sdtPr><w:id w:val=\"9\"/></w:sdtPr><w:sdtContent><w:p><w:r><w:t>in a control</w:t></w:r></w:p></w:sdtContent></w:sdt>";
        auto doc = open(makeDocx(body, kStyles));
        // The table is its cell's paragraph in the story; the picture, the field, and the control have nothing that shows beyond their text.
        QTextDocument *s = doc->story();
        QVERIFY(!isLocked(s->findBlockByNumber(1)));
        QCOMPARE(s->findBlockByNumber(1).text(), QStringLiteral("cell"));
        QVERIFY(s->findBlockByNumber(1).blockFormat().intProperty(flow::fp::Cell) > 0);
        QVERIFY(s->toPlainText().contains(QLatin1String("in a control")));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains(table));   // (a table that was not changed is written as it was)
        QVERIFY(x.contains("<wp:inline xmlns:wp=\"urn:wp\">"));
        QVERIFY(x.contains("<w:instrText> PAGE </w:instrText>"));
        QVERIFY(x.contains("<w:sdt><w:sdtPr><w:id w:val=\"9\"/></w:sdtPr><w:sdtContent>"));
        QVERIFY(x.contains("</w:sdtContent></w:sdt>"));
        // Editing around them keeps them; typing in the cell changes the table's text and keeps the rest of it, and Enter in the cell makes another paragraph in it.
        QTextCursor c(s);
        c.setPosition(s->findBlockByNumber(1).position());
        insertText(*doc, c, QStringLiteral("zzz"));
        QCOMPARE(s->findBlockByNumber(1).text(), QStringLiteral("zzzcell"));
        insertParagraph(*doc, c);
        QCOMPARE(s->blockCount(), 5);
        QCOMPARE(s->findBlockByNumber(2).blockFormat().intProperty(flow::fp::Cell), s->findBlockByNumber(1).blockFormat().intProperty(flow::fp::Cell));
        const QByteArray y = documentXml(doc->toDocx());
        QVERIFY(!y.contains(table));
        QCOMPARE(int(y.count("<w:tbl>")), 1);
        QVERIFY(y.contains("<w:tblW w:w=\"0\" w:type=\"auto\"/>"));
        QVERIFY(y.contains("<w:gridCol w:w=\"2000\"/>"));
        QVERIFY(y.contains("zzz</w:t></w:r></w:p><w:p><w:r><w:t xml:space=\"preserve\">cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"));
    }

    // ---- tables

    // A grid of `rows` by `cols` cells of "r<row>c<col>", with `before` and `after` paragraphs around it (a file of its own making).
    static QByteArray tableDocx(int rows, int cols, const QByteArray &cellExtra = QByteArray())
    {
        QByteArray t = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders></w:tblPr><w:tblGrid>";
        for (int c = 0; c < cols; ++c) t += "<w:gridCol w:w=\"2000\"/>";
        t += "</w:tblGrid>";
        for (int r = 0; r < rows; ++r) {
            t += "<w:tr>";
            for (int c = 0; c < cols; ++c) t += "<w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/>" + cellExtra + "</w:tcPr><w:p><w:r><w:t>r" + QByteArray::number(r) + "c" + QByteArray::number(c) + "</w:t></w:r></w:p></w:tc>";
            t += "</w:tr>";
        }
        t += "</w:tbl>";
        return makeDocx("<w:p><w:r><w:t>before</w:t></w:r></w:p>" + t + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles);
    }

    // The heights of the rows of the first table on the first page.
    static QVector<double> rowHeightsOf(const QByteArray &body)
    {
        auto doc = open(makeDocx(body, kStyles));
        QVector<double> out;
        const auto page = doc->pager().materialize(0);
        if (!page || page->tables.isEmpty()) return out;
        const flow::TableFragment &f = *page->tables[0];
        for (int r = f.firstRow; r < f.endRow; ++r) out << f.heightOf(r);
        return out;
    }
    static QByteArray lineX(const char *side, const char *val, int sz) { return "<w:" + QByteArray(side) + " w:val=\"" + val + "\" w:sz=\"" + QByteArray::number(sz) + "\" w:space=\"0\" w:color=\"auto\"/>"; }

    // The lines a file gives a table take room in its rows (a row is as tall as its cells and the line at its top; the last, the table's foot line too): the heights with lines
    // are those without them and the lines' widths (a size of 8 is a point).
    void theLinesOfATableTakeRoomInItsRowsAsTheFileGivesThem()
    {
        auto table = [](const QByteArray &borders, const QByteArray &firstCellPr = QByteArray(), const QByteArray &otherCellPr = QByteArray()) {
            return tableX(rowX(cellX(cellP("a"), firstCellPr)) + rowX(cellX(cellP("b"), otherCellPr)) + rowX(cellX(cellP("c"), otherCellPr)), 1, 2400, "<w:tblW w:w=\"0\" w:type=\"auto\"/>" + borders);
        };
        const QVector<double> bare = rowHeightsOf(table(QByteArray()));
        QCOMPARE(bare.size(), 3);
        const QByteArray lines = "<w:tblBorders>" + lineX("top", "single", 8) + lineX("bottom", "single", 12) + lineX("insideH", "single", 8) + "</w:tblBorders>";
        const QVector<double> got = rowHeightsOf(table(lines));
        QCOMPARE(got.size(), 3);
        QVERIFY2(std::abs(got[0] - (bare[0] + 1)) < 0.01, qPrintable(QString::number(got[0] - bare[0])));
        QVERIFY2(std::abs(got[1] - (bare[1] + 1)) < 0.01, qPrintable(QString::number(got[1] - bare[1])));
        QVERIFY2(std::abs(got[2] - (bare[2] + 1 + 1.5)) < 0.01, qPrintable(QString::number(got[2] - bare[2])));
        // A cell that says none for a line, with the cell beyond it saying nothing, leaves the table's line; both saying none take it out.
        const QVector<double> oneSays = rowHeightsOf(table(lines, "<w:tcBorders>" + lineX("bottom", "nil", 0) + "</w:tcBorders>"));
        QVERIFY2(std::abs(oneSays[1] - (bare[1] + 1)) < 0.01, qPrintable(QString::number(oneSays[1] - bare[1])));
        const QVector<double> bothSay = rowHeightsOf(table(lines, "<w:tcBorders>" + lineX("bottom", "nil", 0) + "</w:tcBorders>", "<w:tcBorders>" + lineX("top", "nil", 0) + "</w:tcBorders>"));
        QVERIFY2(std::abs(bothSay[1] - bare[1]) < 0.01, qPrintable(QString::number(bothSay[1] - bare[1])));
    }

    // A table that says it has space between its cells, even none, has its lines round the lot of them and not the cells' own (so a cell that says none of its lines does not
    // take the table's away): the first row has the top line and the last the foot line.
    void aTableWithSpaceBetweenItsCellsHasItsLinesRoundThem()
    {
        const QByteArray none = "<w:tcBorders>" + lineX("top", "nil", 0) + lineX("left", "nil", 0) + lineX("bottom", "nil", 0) + lineX("right", "nil", 0) + "</w:tcBorders>";
        auto table = [&](const QByteArray &spacing) {
            const QByteArray pr = "<w:tblW w:w=\"0\" w:type=\"auto\"/>" + spacing + "<w:tblBorders>" + lineX("top", "single", 12) + lineX("left", "single", 12) + lineX("bottom", "single", 16) + lineX("right", "single", 12) + "</w:tblBorders>";
            return tableX(rowX(cellX(cellP("a"), none)) + rowX(cellX(cellP("b"), none)) + rowX(cellX(cellP("c"), none)), 1, 2400, pr);
        };
        const QVector<double> apart = rowHeightsOf(table("<w:tblCellSpacing w:w=\"0\" w:type=\"dxa\"/>")), together = rowHeightsOf(table(QByteArray()));
        QCOMPARE(apart.size(), 3);
        QCOMPARE(together.size(), 3);
        // (Without the space, the cells say none of the lines they have at the edge of the table, and none is left.)
        QVERIFY2(std::abs(apart[0] - (together[0] + 1.5)) < 0.01, qPrintable(QString::number(apart[0] - together[0])));
        QVERIFY2(std::abs(apart[1] - together[1]) < 0.01, qPrintable(QString::number(apart[1] - together[1])));
        QVERIFY2(std::abs(apart[2] - (together[2] + 2)) < 0.01, qPrintable(QString::number(apart[2] - together[2])));
    }

    // A triple line takes the room of its three lines and the two spaces between them (five times its width), and is read as such, not as a double one.
    void aTripleLineTakesRoomForItsThreeLinesAndTheSpaces()
    {
        auto table = [](const QByteArray &borders) {
            return tableX(rowX(cellX(cellP("a"))) + rowX(cellX(cellP("b"))), 1, 2400, "<w:tblW w:w=\"0\" w:type=\"auto\"/>" + borders);
        };
        const QVector<double> bare = rowHeightsOf(table(QByteArray()));
        const QVector<double> triple = rowHeightsOf(table("<w:tblBorders>" + lineX("insideH", "triple", 8) + "</w:tblBorders>")), twice = rowHeightsOf(table("<w:tblBorders>" + lineX("insideH", "double", 8) + "</w:tblBorders>"));
        QCOMPARE(bare.size(), 2);
        QCOMPARE(triple.size(), 2);
        QVERIFY2(std::abs(triple[1] - (bare[1] + 5)) < 0.01, qPrintable(QString::number(triple[1] - bare[1])));
        QVERIFY2(std::abs(twice[1] - (bare[1] + 3)) < 0.01, qPrintable(QString::number(twice[1] - bare[1])));
    }

    // The saved document is well formed and opens again with the same text.
    static void checkWhole(WriteDoc &doc)
    {
        const QByteArray zip = doc.toDocx();
        QVERIFY2(!zip.isEmpty(), "the document saves");
        QVERIFY2(rt::strictProblems(zip).isEmpty(), qPrintable(rt::strictProblems(zip).join(QLatin1Char('\n'))));
        auto back = open(zip);
        QVERIFY(back);
        QString a = doc.story()->toPlainText(), b = back->story()->toPlainText();
        a.remove(QChar::ObjectReplacementCharacter);
        b.remove(QChar::ObjectReplacementCharacter);
        QCOMPARE(b, a);
        QCOMPARE(documentXml(back->toDocx()), documentXml(zip));
        QVERIFY(doc.pager().pageCount() >= 1);
        QVERIFY(doc.pager().materialize(0));
    }

    // The cell the cursor is in is a cell of the table: Tab goes to the next with its text selected, Shift+Tab to the one before, and Tab in the last cell adds a
    // row; Enter in a cell is another paragraph of it; and Backspace and Delete at a cell's edge do not reach into the next cell.
    void tabEnterAndDeleteWorkInCells()
    {
        auto doc = open(tableDocx(2, 2));
        QTextDocument *s = doc->story();
        auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
        QTextCursor c(s);
        c.setPosition(block(QStringLiteral("r0c0")).position() + 2);
        insertTab(*doc, c, false);
        QCOMPARE(c.selectedText(), QStringLiteral("r0c1"));
        typeText(*doc, c, QStringLiteral("X"));   // (replaces it)
        QVERIFY(block(QStringLiteral("X")).isValid());
        insertTab(*doc, c, true);
        QCOMPARE(c.selectedText(), QStringLiteral("r0c0"));
        QVERIFY(!c.hasSelection() || c.position() != c.anchor());
        // Tab on from the last cell: a row is added, the cursor is in its first cell.
        c.setPosition(block(QStringLiteral("r1c1")).position());
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tr>")), 2);
        insertTab(*doc, c, false);
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tr>")), 3);
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 6);
        QCOMPARE(cellKeyOf(c.block()) != 0, true);
        QVERIFY(c.block().text().isEmpty());
        typeText(*doc, c, QStringLiteral("new"));
        checkWhole(*doc);
        // Enter in a cell: the cell has another paragraph.
        c.setPosition(block(QStringLiteral("r1c0")).position() + 1);
        insertParagraph(*doc, c);
        QVERIFY(cellKeyOf(block(QStringLiteral("r"))) != 0);
        QCOMPARE(cellKeyOf(block(QStringLiteral("r"))), cellKeyOf(block(QStringLiteral("1c0"))));
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 6);
        // Backspace at the start of a cell, Delete at the end of one: nothing joins.
        const int blocks = s->blockCount();
        c.setPosition(block(QStringLiteral("r1c1")).position());
        deleteBack(*doc, c, false);
        QCOMPARE(s->blockCount(), blocks);
        c.setPosition(block(QStringLiteral("1c0")).position() + 3);
        deleteForward(*doc, c, false);
        QCOMPARE(s->blockCount(), blocks);
        checkWhole(*doc);
    }

    // A file that multiplies what it holds past the limits (more cells than a table may have, tables nested deeper than they are read, more columns than there are) is
    // kept whole as the file had it and shown as a stand-in; one at the limits opens, is laid out, and saves.
    void tablesPastTheLimitsAreKeptWholeAndOnesAtThemWork()
    {
        auto tableOf = [](int rows, int cols) {
            QByteArray t = "<w:tbl><w:tblGrid>";
            for (int c = 0; c < cols; ++c) t += "<w:gridCol w:w=\"500\"/>";
            t += "</w:tblGrid>";
            for (int r = 0; r < rows; ++r) {
                t += "<w:tr>";
                for (int c = 0; c < cols; ++c) t += "<w:tc><w:p/></w:tc>";
                t += "</w:tr>";
            }
            return t + "</w:tbl>";
        };
        auto standIns = [](WriteDoc &doc) {
            int n = 0;
            for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) n += isLocked(b);
            return n;
        };
        // 1,001 rows of 100 columns: more cells than one table may have.
        const QByteArray many = tableOf(1001, 100);
        auto big = open(makeDocx(many + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles));
        QVERIFY(big);
        QCOMPARE(standIns(*big), 1);
        QVERIFY(documentXml(big->toDocx()).contains(many));
        // 40 rows of 256 columns, and 3,000 rows of 3: at the limits, they work.
        auto wide = open(makeDocx(tableOf(40, 256) + "<w:p/>", kStyles));
        QVERIFY(wide);
        QCOMPARE(standIns(*wide), 0);
        QVERIFY(wide->pager().pageCount() >= 1);
        QVERIFY(wide->pager().materialize(0));
        checkWhole(*wide);
        auto tall = open(makeDocx(tableOf(3000, 3) + "<w:p/>", kStyles));
        QVERIFY(tall);
        QCOMPARE(standIns(*tall), 0);
        QVERIFY(tall->pager().pageCount() > 10);
        // More columns than the limit in one row: a stand-in.
        auto toowide = open(makeDocx(tableOf(2, 257) + "<w:p/>", kStyles));
        QVERIFY(toowide);
        QCOMPARE(standIns(*toowide), 1);
        // Nested nine deep: eight are read, and what is in the ninth is a stand-in.
        QByteArray deep;
        for (int i = 0; i < 9; ++i) deep += "<w:tbl><w:tblGrid><w:gridCol w:w=\"1000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>level " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        for (int i = 0; i < 9; ++i) deep += "</w:tc></w:tr></w:tbl><w:p/>";
        auto nested = open(makeDocx(deep, kStyles));
        QVERIFY(nested);
        QCOMPARE(standIns(*nested), 1);
        QVERIFY(documentXml(nested->toDocx()).contains("level 8"));
        QVERIFY(nested->pager().pageCount() >= 1);
        checkWhole(*nested);
    }

    // A section does not end in a table's cell: the command there changes nothing.
    void aSectionBreakIsNotMadeInACell()
    {
        auto doc = open(tableDocx(1, 2));
        QTextDocument *s = doc->story();
        QTextCursor c(s);
        for (QTextBlock b = s->begin(); b.isValid(); b = b.next())
            if (b.text() == QLatin1String("r0c1")) c.setPosition(b.position() + 1);
        const int blocks = s->blockCount();
        insertSectionBreak(*doc, c);
        QCOMPARE(s->blockCount(), blocks);
        QCOMPARE(doc->sectionCount(), 1);
        QVERIFY(!s->isUndoAvailable());
    }

    // Rows and columns are added beside the cell's and taken away, as one step each that undoes to the very file it began as.
    void rowsAndColumnsAreAddedAndTakenAway()
    {
        const QByteArray original = tableDocx(3, 3);
        auto doc = open(original);
        QTextDocument *s = doc->story();
        const QByteArray was = documentXml(doc->toDocx());
        auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
        auto count = [&](const char *what) { return int(documentXml(doc->toDocx()).count(what)); };
        QTextCursor c(s);
        c.setPosition(block(QStringLiteral("r1c1")).position());
        insertTableRow(*doc, c, true);
        QCOMPARE(count("<w:tr>"), 4);
        QCOMPARE(count("<w:tc>"), 12);
        checkWhole(*doc);
        s->undo();
        QCOMPARE(documentXml(doc->toDocx()), was);   // (as it was, to the byte: the table that nothing changed)
        s->redo();
        QCOMPARE(count("<w:tr>"), 4);
        insertTableRow(*doc, c, false);
        QCOMPARE(count("<w:tr>"), 5);
        // The new rows are empty paragraphs in cells, in the places: the row after r1 has r1's cells' widths.
        QStringList texts;
        for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) texts << b.text();
        QCOMPARE(texts, (QStringList{"before", "r0c0", "r0c1", "r0c2", "", "", "", "r1c0", "r1c1", "r1c2", "", "", "", "r2c0", "r2c1", "r2c2", "after"}));
        checkWhole(*doc);
        // A column: three in each row become four, the grid has four columns, and the new cells are empty.
        c.setPosition(block(QStringLiteral("r1c1")).position());
        insertTableColumn(*doc, c, true);
        QCOMPARE(count("<w:tc>"), 5 * 4);
        QCOMPARE(count("<w:gridCol "), 4);
        checkWhole(*doc);
        s->undo();
        QCOMPARE(count("<w:gridCol "), 3);
        s->redo();
        insertTableColumn(*doc, c, false);
        QCOMPARE(count("<w:gridCol "), 5);
        QCOMPARE(count("<w:tc>"), 5 * 5);
        checkWhole(*doc);
        // Taking away: the row and the column of the cursor, then the whole table when no row or no column is left.
        c.setPosition(block(QStringLiteral("r1c1")).position());
        deleteTableRows(*doc, c);
        QCOMPARE(count("<w:tr>"), 4);
        QVERIFY(!block(QStringLiteral("r1c1")).isValid());
        QVERIFY(block(QStringLiteral("r1c0")).isValid() == false);
        c.setPosition(block(QStringLiteral("r2c1")).position());
        deleteTableColumns(*doc, c);
        QCOMPARE(count("<w:gridCol "), 4);
        QCOMPARE(count("<w:tc>"), 4 * 4);
        QVERIFY(!block(QStringLiteral("r2c1")).isValid());
        QVERIFY(block(QStringLiteral("r2c0")).isValid() && block(QStringLiteral("r2c2")).isValid());
        checkWhole(*doc);
        c.setPosition(block(QStringLiteral("r2c0")).position());
        deleteTable(*doc, c);
        QCOMPARE(count("<w:tbl>"), 0);
        QCOMPARE(s->toPlainText(), QStringLiteral("before\nafter"));
        checkWhole(*doc);
        while (s->isUndoAvailable()) s->undo();
        QCOMPARE(documentXml(doc->toDocx()), was);
    }

    // A column through cells that are joined across makes them one wider; a row through cells joined down is a row of the same joined cells; and deleting the row a joined
    // cell begins in gives the cell below it what it was.
    void rowsAndColumnsKeepJoinedCellsWhole()
    {
        QByteArray t = "<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
                       "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"4000\" w:type=\"dxa\"/><w:gridSpan w:val=\"2\"/></w:tcPr><w:p><w:r><w:t>wide</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:vMerge w:val=\"restart\"/></w:tcPr><w:p><w:r><w:t>tall</w:t></w:r></w:p></w:tc></w:tr>"
                       "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>a</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>b</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:vMerge/></w:tcPr><w:p/></w:tc></w:tr>"
                       "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>c</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>d</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:vMerge/></w:tcPr><w:p/></w:tc></w:tr></w:tbl>";
        auto doc = open(makeDocx(t + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles));
        QTextDocument *s = doc->story();
        auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
        auto xmlNow = [&] { return documentXml(doc->toDocx()); };
        QTextCursor c(s);
        // A column to the right of the first (it is inside "wide"): that cell is wider, the rows below have a new cell beside "a".
        c.setPosition(block(QStringLiteral("a")).position());
        insertTableColumn(*doc, c, true);
        QVERIFY(xmlNow().contains("<w:gridSpan w:val=\"3\"/>"));
        QCOMPARE(int(xmlNow().count("<w:gridCol ")), 4);
        QCOMPARE(int(xmlNow().count("<w:vMerge/>")), 2);
        checkWhole(*doc);
        s->undo();
        // A row below the first: it is inside "tall".
        c.setPosition(block(QStringLiteral("wide")).position());
        insertTableRow(*doc, c, true);
        QCOMPARE(int(xmlNow().count("<w:vMerge/>")), 3);
        checkWhole(*doc);
        s->undo();
        // The row "tall" begins in goes: "tall" is now at the next row, with the rest of its cells (one row of it left, so not joined).
        c.setPosition(block(QStringLiteral("wide")).position());
        deleteTableRows(*doc, c);
        QVERIFY(!xmlNow().contains("tall"));
        QCOMPARE(int(xmlNow().count("<w:tr>")), 2);
        QVERIFY(xmlNow().contains("<w:vMerge w:val=\"restart\"/>"));
        checkWhole(*doc);
    }

    // ---- tables with gaps in their rows, and commands that take columns out

    static QByteArray cellP(const QByteArray &text) { return "<w:p><w:r><w:t>" + text + "</w:t></w:r></w:p>"; }
    static QByteArray cellX(const QByteArray &content, const QByteArray &pr = QByteArray(), int width = 2400)
    {
        return "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(width) + "\" w:type=\"dxa\"/>" + pr + "</w:tcPr>" + content + "</w:tc>";
    }
    static QByteArray rowX(const QByteArray &cells, const QByteArray &pr = QByteArray()) { return "<w:tr>" + (pr.isEmpty() ? QByteArray() : "<w:trPr>" + pr + "</w:trPr>") + cells + "</w:tr>"; }
    static QByteArray tableX(const QByteArray &rows, int columns, int columnWidth = 2400, const QByteArray &width = "<w:tblW w:w=\"0\" w:type=\"auto\"/>")
    {
        QByteArray t = "<w:tbl><w:tblPr>" + width + "</w:tblPr><w:tblGrid>";
        for (int i = 0; i < columns; ++i) t += "<w:gridCol w:w=\"" + QByteArray::number(columnWidth) + "\"/>";
        return t + "</w:tblGrid>" + rows + "</w:tbl>";
    }
    // The story's paragraphs, as one text.
    static QString storyText(WriteDoc &doc) { return doc.story()->toPlainText().replace(QLatin1Char('\n'), QLatin1Char('|')); }
    static QTextCursor cursorAt(WriteDoc &doc, const QString &text)
    {
        QTextCursor c(doc.story());
        for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
            if (b.text() == text) c.setPosition(b.position());
        return c;
    }
    // Taking away columns takes the paragraphs of the cells in them and no others, whatever rows there are between: a row that has only a gap and a cell in the columns
    // goes with them, and what is outside the table is as it was.
    void deletingColumnsTakesOnlyTheCellsInThem()
    {
        const QByteArray body = cellP("top") +
                                tableX(rowX(cellX(cellP("A0")) + cellX(cellP("A1")) + cellX(cellP("A2"))) +
                                           rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 4800), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"2400\" w:type=\"dxa\"/>") +
                                           rowX(cellX(cellP("R0")) + cellX(cellP("R1")) + cellX(cellP("R2"))),
                                       3) +
                                cellP("tail1") + cellP("tail2") + cellP("tail3");
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c = cursorAt(*doc, QStringLiteral("A1"));
        c.setPosition(cursorAt(*doc, QStringLiteral("A2")).position() + 1, QTextCursor::KeepAnchor);
        const QString was = storyText(*doc);
        deleteTableColumns(*doc, c);
        QCOMPARE(storyText(*doc), QStringLiteral("top|A0|R0|tail1|tail2|tail3"));
        checkWhole(*doc);
        doc->story()->undo();
        QCOMPARE(storyText(*doc), was);
        // Columns 1 and 2 of a table where the row with the gap is the first, the gap's cell is on the edge of the columns, and one more row follows.
        const QByteArray body2 = cellP("top") +
                                 tableX(rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 4800), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"2400\" w:type=\"dxa\"/>") +
                                            rowX(cellX(cellP("B0")) + cellX(cellP("B1")) + cellX(cellP("B2"))) + rowX(cellX(cellP("C0")) + cellX(cellP("C1")) + cellX(cellP("C2"))),
                                        3) +
                                 cellP("tail1");
        auto second = open(makeDocx(body2, kStyles));
        QTextCursor d = cursorAt(*second, QStringLiteral("B1"));
        d.setPosition(cursorAt(*second, QStringLiteral("B2")).position() + 1, QTextCursor::KeepAnchor);
        deleteTableColumns(*second, d);
        QCOMPARE(storyText(*second), QStringLiteral("top|B0|C0|tail1"));
        checkWhole(*second);
    }

    // A row that has a gap before its first cell or after its last keeps it where it is the gap's, in every command: a row added beside it has the same, a column taken
    // away takes from the gap what is of it, a column added in the gap makes it wider, and the table's width goes with its columns.
    void gapsInRowsFollowTheRowAndColumnCommands()
    {
        const QByteArray gapRow = rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 4800), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"2400\" w:type=\"dxa\"/>");
        const QByteArray body = cellP("top") +
                                tableX(rowX(cellX(cellP("A0")) + cellX(cellP("A1")) + cellX(cellP("A2"))) + gapRow + rowX(cellX(cellP("D0")) + cellX(cellP("D1")) + cellX(cellP("D2"))), 3, 2400,
                                       "<w:tblW w:w=\"7200\" w:type=\"dxa\"/>") +
                                cellP("tail");
        auto count = [](WriteDoc &d, const char *what) { return int(documentXml(d.toDocx()).count(what)); };
        {   // a row beside the row with a gap has the gap
            auto doc = open(makeDocx(body, kStyles));
            QTextCursor c = cursorAt(*doc, QStringLiteral("G"));
            insertTableRow(*doc, c, true);
            QCOMPARE(count(*doc, "<w:gridBefore "), 2);
            QCOMPARE(count(*doc, "<w:wBefore "), 2);
            QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
            checkWhole(*doc);
        }
        {   // a column taken from the gap makes it narrower, and the grid's width less: the row still has as many columns as the grid
            auto doc = open(makeDocx(body, kStyles));
            QTextCursor c = cursorAt(*doc, QStringLiteral("D0"));
            deleteTableColumns(*doc, c);
            QCOMPARE(count(*doc, "<w:gridBefore "), 0);   // (the gap was that column)
            QCOMPARE(count(*doc, "<w:wBefore "), 0);
            QCOMPARE(count(*doc, "<w:gridCol "), 2);
            QVERIFY2(documentXml(doc->toDocx()).contains("<w:tblW w:w=\"4800\" w:type=\"dxa\"/>"), documentXml(doc->toDocx()).left(900).constData());
            QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
            checkWhole(*doc);
        }
        for (const bool right : {false, true}) {   // a column added left of the first: the gap is a column wider (the new one is in it)
            auto doc = open(makeDocx(body, kStyles));
            QTextCursor c = cursorAt(*doc, QStringLiteral("A0"));
            insertTableColumn(*doc, c, right);
            const QByteArray x = documentXml(doc->toDocx());
            QCOMPARE(count(*doc, "<w:gridCol "), 4);
            QVERIFY2(x.contains("<w:tblW w:w=\"9600\" w:type=\"dxa\"/>"), x.left(900).constData());
            QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
            // The new cell of the gap's row is in the gap when it is left of the gap's end, and a cell of its own when it is right of it.
            const QByteArray gap = x.mid(x.indexOf("<w:gridBefore "), 40);
            QVERIFY2(right ? gap.contains("w:val=\"1\"") : gap.contains("w:val=\"2\""), gap.constData());
            checkWhole(*doc);
            doc->story()->undo();
            QCOMPARE(count(*doc, "<w:gridCol "), 3);
        }
        {   // the gap after the last cell grows when a column is added in it, and loses what a deleted column held of it
            const QByteArray b2 = cellP("top") +
                                  tableX(rowX(cellX(cellP("A0")) + cellX(cellP("A1")) + cellX(cellP("A2"))) + rowX(cellX(cellP("E0")) + cellX(cellP("E1")), "<w:gridAfter w:val=\"1\"/><w:wAfter w:w=\"2400\" w:type=\"dxa\"/>"), 3) +
                                  cellP("tail");
            auto doc = open(makeDocx(b2, kStyles));
            QTextCursor c = cursorAt(*doc, QStringLiteral("A2"));
            deleteTableColumns(*doc, c);   // (the last column: the row with the gap loses it)
            QCOMPARE(count(*doc, "<w:gridAfter "), 0);
            QCOMPARE(count(*doc, "<w:gridCol "), 2);
            QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
            checkWhole(*doc);
            QTextCursor e = cursorAt(*doc, QStringLiteral("A1"));
            insertTableColumn(*doc, e, true);   // (right of the last column)
            QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
            checkWhole(*doc);
        }
    }

    // A row or a column added beside others is like them: the row says what the row beside it says of its height and of not being split, and each new cell says what
    // the cell beside it says of its shading and its place of its text (the properties of a start that a file leaves its root's declarations to be read by).
    void newRowsAndCellsAreLikeTheOnesBesideThem()
    {
        const QByteArray pr = "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"FF0000\"/><w:vAlign w:val=\"center\"/>";
        const QByteArray body = cellP("top") +
                                tableX(rowX(cellX(cellP("A0"), pr) + cellX(cellP("A1"), pr), "<w:cantSplit/><w:trHeight w:val=\"500\" w:hRule=\"exact\"/>") + rowX(cellX(cellP("B0"), pr) + cellX(cellP("B1"), pr)), 2) + cellP("tail");
        auto doc = open(makeDocx(body, kStyles));
        auto count = [&](const char *what) { return int(documentXml(doc->toDocx()).count(what)); };
        QTextCursor c = cursorAt(*doc, QStringLiteral("A0"));
        insertTableRow(*doc, c, true);
        QCOMPARE(count("<w:cantSplit/>"), 2);
        QCOMPARE(count("<w:trHeight "), 2);
        QCOMPARE(count("w:fill=\"FF0000\""), 6);
        QCOMPARE(count("<w:vAlign w:val=\"center\"/>"), 6);
        c = cursorAt(*doc, QStringLiteral("B0"));
        insertTableColumn(*doc, c, true);
        QCOMPARE(count("w:fill=\"FF0000\""), 9);
        QCOMPARE(count("<w:vAlign w:val=\"center\"/>"), 9);
        checkWhole(*doc);
    }

    // A paragraph in a cell stays in its cell when it is given a style, and so does the paragraph that Enter makes after it in the style that follows (a heading is
    // followed by body text): a paragraph's format is made over in both, and the cell is not what the style says.
    void aParagraphInACellStaysThereWhateverIsDoneToItsFormat()
    {
        auto doc = open(tableDocx(2, 2));
        QTextDocument *s = doc->story();
        QTextCursor c = cursorAt(*doc, QStringLiteral("r0c1"));
        const int key = cellKeyOf(c.block());
        QVERIFY(key != 0);
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        QCOMPARE(cellKeyOf(cursorAt(*doc, QStringLiteral("r0c1")).block()), key);
        c.movePosition(QTextCursor::EndOfBlock);
        insertParagraph(*doc, c);   // (at the end of a heading: the next paragraph is body text)
        QCOMPARE(cellKeyOf(c.block()), key);
        QCOMPARE(c.block().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Normal"));
        typeText(*doc, c, QStringLiteral("more"));
        QVERIFY(plain(*doc).contains(QStringLiteral("r0c1\nmore\nr1c0")));
        checkWhole(*doc);
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("<w:tc>")), 4);   // (all four cells are cells still)
        while (s->isUndoAvailable()) s->undo();
        QCOMPARE(plain(*doc), QStringLiteral("before\nr0c0\nr0c1\nr1c0\nr1c1\nafter"));
    }

    // Rows added beside a row whose cells are all empty (so that no paragraph says which is which but its place) have their cells in the same order: the cell joined to the
    // one above is the first, as the row beside it has it, for a row put above and for one put below.
    void rowsAddedBesideEmptyCellsKeepTheOrderOfTheirCells()
    {
        const QByteArray empty = "<w:p/>";
        const QByteArray body = cellP("top") +
                                tableX(rowX(cellX(empty, "<w:vMerge w:val=\"restart\"/>") + cellX(empty) + cellX(empty)) + rowX(cellX(empty, "<w:vMerge/>") + cellX(empty) + cellX(empty)) +
                                           rowX(cellX(empty) + cellX(empty) + cellX(empty)),
                                       3) +
                                cellP("tail");
        for (const bool below : {false, true}) {
            auto doc = open(makeDocx(body, kStyles));
            QTextCursor c(doc->story());
            // (The second row's first cell: the second table paragraph in the story, after the first row's three.)
            QVector<QTextBlock> cells;
            for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
                if (cellKeyOf(b)) cells << b;
            QCOMPARE(int(cells.size()), 9);
            c.setPosition(cells[3].position());
            insertTableRow(*doc, c, below);
            const QByteArray saved = doc->toDocx();
            QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
            // The joined cells are the first of their rows: three rows of three become four, and three of the four have a joined first cell where the row before has one.
            const QByteArray x = documentXml(saved);
            QCOMPARE(int(x.count("<w:tr>")), 4);
            const qsizetype firstRowEnd = x.indexOf("</w:tr>");
            QVERIFY(x.left(firstRowEnd).contains("w:val=\"restart\""));
            // Each row's joined cell is its first: no row has one after a cell that is not.
            qsizetype from = 0;
            for (int row = 0; row < 4; ++row) {
                const qsizetype end = x.indexOf("</w:tr>", from);
                const QByteArray text = x.mid(from, end - from);
                const qsizetype joined = std::max(text.indexOf("<w:vMerge/>"), text.indexOf("<w:vMerge w:val"));
                if (joined >= 0) QCOMPARE(int(text.left(joined).count("<w:tc>")), 1);
                from = end + 7;
            }
            checkWhole(*doc);
        }
    }

    // Tab in the last cell of a table whose last row is joined to the one above adds a row after the table's true last row, with the cursor in its first cell, and its
    // cells are not joined.
    void tabInTheLastCellOfAJoinedLastRowAddsARowAfterTheTable()
    {
        const QByteArray body = cellP("x") +
                                tableX(rowX(cellX(cellP("r0a"), "<w:vMerge w:val=\"restart\"/>") + cellX(cellP("r0b"), "<w:vMerge w:val=\"restart\"/>")) +
                                           rowX(cellX("<w:p/>", "<w:vMerge/>") + cellX("<w:p/>", "<w:vMerge/>")),
                                       2) +
                                cellP("y");
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c = cursorAt(*doc, QStringLiteral("r0b"));
        insertTab(*doc, c, false);
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("<w:tr>")), 3);
        QCOMPARE(int(x.count("<w:vMerge/>")), 2);   // (the new row's cells are not joined to the ones above)
        const CellPlace place = cellPlace(*doc, c);
        QVERIFY(place.inTable);
        QCOMPARE(place.row, 2);
        QCOMPARE(place.column, 0);
        checkWhole(*doc);
    }

    // A selection across cells empties them without joining what is in different cells, and one that holds a whole table takes it out.
    void deletingAcrossCellsEmptiesThemAndTakesWholeTablesOut()
    {
        auto doc = open(tableDocx(2, 2));
        QTextDocument *s = doc->story();
        auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
        const QByteArray was = documentXml(doc->toDocx());
        QTextCursor c(s);
        // From the middle of r0c0 to the middle of r1c0: r0c0 loses its end, r0c1 is emptied, r1c0 loses its start.
        c.setPosition(block(QStringLiteral("r0c0")).position() + 1);
        c.setPosition(block(QStringLiteral("r1c0")).position() + 1, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        QStringList texts;
        for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) texts << b.text();
        QCOMPARE(texts, (QStringList{"before", "r", "", "1c0", "r1c1", "after"}));
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 4);
        checkWhole(*doc);
        s->undo();
        QCOMPARE(documentXml(doc->toDocx()), was);
        // From inside the paragraph before to inside the one after: the table goes, and what is left of the two is one paragraph.
        c.setPosition(block(QStringLiteral("before")).position() + 3);
        c.setPosition(block(QStringLiteral("after")).position() + 2, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        QCOMPARE(s->toPlainText(), QStringLiteral("befter"));
        QVERIFY(!documentXml(doc->toDocx()).contains("<w:tbl>"));
        checkWhole(*doc);
        s->undo();
        QCOMPARE(documentXml(doc->toDocx()), was);
    }

    // What is pasted into a cell is in that cell, whatever cell it was copied from, and what is pasted outside a table is in none.
    void pastedParagraphsAreInTheCellTheyWerePastedInto()
    {
        auto doc = open(tableDocx(1, 2));
        QTextDocument *s = doc->story();
        auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
        QTextCursor c(s);
        c.setPosition(block(QStringLiteral("r0c0")).position() + 4);
        insertParagraph(*doc, c);
        typeText(*doc, c, QStringLiteral("second"));
        c.setPosition(block(QStringLiteral("r0c0")).position());
        c.setPosition(block(QStringLiteral("second")).position() + 6, QTextCursor::KeepAnchor);
        const QTextDocumentFragment two = selectionFragment(c);   // (two paragraphs of the first cell)
        c.setPosition(block(QStringLiteral("r0c1")).position() + 4);
        pasteFragment(*doc, c, two);
        const int other = cellKeyOf(block(QStringLiteral("r0c1r0c0")));
        QVERIFY(other != 0 && other != cellKeyOf(block(QStringLiteral("r0c0"))));
        int inCell = 0;
        for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) inCell += cellKeyOf(b) == other;
        QCOMPARE(inCell, 2);
        // Into the text before the table: no cell.
        c.setPosition(block(QStringLiteral("before")).position() + 6);
        pasteFragment(*doc, c, two);
        QCOMPARE(block(QStringLiteral("beforer0c0")).isValid(), true);
        QCOMPARE(cellKeyOf(block(QStringLiteral("beforer0c0"))), 0);
        int outside = 0;
        for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) outside += b.text() == QLatin1String("second") && cellKeyOf(b) == 0;
        QCOMPARE(outside, 1);
        checkWhole(*doc);
    }

    void aSavedDocumentRoundTripsItsOwnParagraphFormatting()
    {
        // A document with direct formatting of every kind comes out with the same properties on every paragraph and run.
        const QByteArray body =
            "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/><w:jc w:val=\"center\"/><w:ind w:left=\"360\" w:firstLine=\"180\"/><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"480\" w:lineRule=\"exact\"/>"
            "<w:tabs><w:tab w:val=\"right\" w:leader=\"dot\" w:pos=\"7200\"/></w:tabs><w:keepLines/><w:pageBreakBefore/></w:pPr>"
            "<w:r><w:rPr><w:rFonts w:ascii=\"Georgia\" w:hAnsi=\"Georgia\"/><w:i/><w:caps/><w:strike/><w:color w:val=\"00AA00\"/><w:sz w:val=\"30\"/><w:vertAlign w:val=\"superscript\"/><w:spacing w:val=\"20\"/></w:rPr><w:t>Heading with tab</w:t><w:tab/><w:t>3</w:t></w:r></w:p>"
            "<w:p><w:r><w:rPr><w:smallCaps/><w:u w:val=\"double\"/><w:highlight w:val=\"cyan\"/><w:lang w:val=\"fr-FR\"/></w:rPr><w:t>Second</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles));
        const QByteArray once = documentXml(doc->toDocx());
        wml::Document a, b;
        QVERIFY(wml::readDocx(makeDocx(body, kStyles), &a));
        QVERIFY(wml::readDocx(doc->toDocx(), &b));
        QCOMPARE(a.body.size(), b.body.size());
        for (int i = 0; i < a.body.size(); ++i) {
            const wml::Paragraph &pa = a.body[i].para, &pb = b.body[i].para;
            QCOMPARE(wml::paraPropsXml(pb.props), wml::paraPropsXml(pa.props));
            QCOMPARE(pb.content.size(), pa.content.size());
            for (int k = 0; k < pa.content.size(); ++k) {
                QCOMPARE(wml::runPropsXml(pb.content[k].props), wml::runPropsXml(pa.content[k].props));
                QCOMPARE(pb.content[k].text, pa.content[k].text);
            }
        }
        QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), once);
    }

    void whatAParagraphSaysThatHasNoFieldIsKept()
    {
        // Shading, a suppressed line number, and attributes of spacing and indent that this program has no field for come back as they were.
        const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/><w:suppressLineNumbers/><w:shd w:val=\"clear\" w:fill=\"C0C0C0\"/><w:spacing w:before=\"100\" w:beforeAutospacing=\"1\"/>"
                                "<w:ind w:left=\"360\" w:leftChars=\"150\"/><w:rPr><w:b/><w:shd w:val=\"clear\" w:fill=\"FFFF00\"/></w:rPr></w:pPr>"
                                "<w:r><w:rPr><w:shd w:val=\"clear\" w:fill=\"00FFFF\"/><w:bdr w:val=\"single\"/></w:rPr><w:t>shaded</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:suppressLineNumbers/><w:shd w:val=\"clear\" w:fill=\"C0C0C0\"/>"), x.constData());
        QVERIFY2(x.contains("w:beforeAutospacing=\"1\""), x.constData());
        QVERIFY2(x.contains("w:leftChars=\"150\""), x.constData());
        QVERIFY2(x.contains("<w:shd w:val=\"clear\" w:fill=\"FFFF00\"/>"), x.constData());
        QVERIFY2(x.contains("<w:bdr w:val=\"single\"/><w:shd w:val=\"clear\" w:fill=\"00FFFF\"/>"), x.constData());   // (in the schema's order)
        QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
        // Applying another style leaves them.
        QTextCursor c(doc->story());
        applyParagraphStyle(*doc, c, QStringLiteral("Normal"));
        const QByteArray y = documentXml(doc->toDocx());
        QVERIFY(y.contains("w:fill=\"C0C0C0\"") && y.contains("w:leftChars=\"150\""));
    }

    void tabStopsTheEngineHasNoKindForAreKeptAndNotMade()
    {
        // A list's own tab (num) is the definition's, and a bar tab is the paragraph's: neither is turned into a left tab, and the bar stays.
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"5\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/>"
                                     "<w:pPr><w:tabs><w:tab w:val=\"num\" w:pos=\"1080\"/></w:tabs><w:ind w:left=\"1080\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"5\"><w:abstractNumId w:val=\"5\"/></w:num>";
        auto doc = open(makeDocx("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"5\"/></w:numPr></w:pPr><w:r><w:t>item</w:t></w:r></w:p>"
                                 "<w:p><w:pPr><w:tabs><w:tab w:val=\"bar\" w:pos=\"2000\"/><w:tab w:val=\"right\" w:pos=\"9000\"/></w:tabs></w:pPr><w:r><w:t>bar</w:t></w:r></w:p>", kStyles, numbering));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("w:val=\"left\""), x.constData());
        QVERIFY2(x.contains("<w:tabs><w:tab w:val=\"bar\" w:pos=\"2000\"/><w:tab w:val=\"right\" w:pos=\"9000\"/></w:tabs>"), x.constData());
        QCOMPARE(int(x.count("<w:tabs>")), 1);
    }

    void aKindOfJustifiedIsKeptUntilTheAlignmentIsChosen()
    {
        auto doc = open(makeDocx("<w:p><w:pPr><w:jc w:val=\"distribute\"/></w:pPr><w:r><w:t>spread</w:t></w:r></w:p>"
                                 "<w:p><w:pPr><w:jc w:val=\"both\"/></w:pPr><w:r><w:t>plain</w:t></w:r></w:p>", kStyles));
        QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignJustify);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:jc w:val=\"distribute\"/>") && x.contains("<w:jc w:val=\"both\"/>"), x.constData());
        // Choosing an alignment, even the justified one, makes it the plain kind.
        QTextCursor c(doc->story());
        setAlignment(*doc, c, Qt::AlignJustify);
        const QByteArray y = documentXml(doc->toDocx());
        QVERIFY2(!y.contains("distribute") && int(y.count("<w:jc w:val=\"both\"/>")) == 2, y.constData());
    }

    void aParagraphMarkThatSaysItsSizeKeepsSayingItUnderACharacterStyle()
    {
        // An empty paragraph's mark has a character style that is small, and says its size is the paragraph's own: the size stays, or the
        // empty line gets as short as the style.
        const QByteArray styles = kStyles + "<w:style w:type=\"character\" w:styleId=\"Small\"><w:name w:val=\"Small\"/><w:rPr><w:sz w:val=\"16\"/></w:rPr></w:style>";
        auto doc = open(makeDocx("<w:p><w:pPr><w:rPr><w:rStyle w:val=\"Small\"/><w:sz w:val=\"23\"/></w:rPr></w:pPr></w:p><w:p><w:r><w:t>text</w:t></w:r></w:p>", styles));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:rPr><w:rStyle w:val=\"Small\"/><w:sz w:val=\"23\"/></w:rPr>"), x.constData());
        QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
        // A mark that says only the style is left as that, and is as small as the style on the page.
        auto plain = open(makeDocx("<w:p><w:pPr><w:rPr><w:rStyle w:val=\"Small\"/></w:rPr></w:pPr></w:p><w:p><w:r><w:t>text</w:t></w:r></w:p>", styles));
        QVERIFY2(documentXml(plain->toDocx()).contains("<w:rPr><w:rStyle w:val=\"Small\"/></w:rPr></w:pPr>"), documentXml(plain->toDocx()).constData());
        QCOMPARE(plain->story()->findBlockByNumber(0).charFormat().fontPointSize(), 8.0);
    }

    void aStyleThatNumbersKeepsItsOwnIndentsOverTheList()
    {
        // The style says it is numbered and says its indents; the list's level says other indents. The style's are the paragraph's. A
        // paragraph that says "hanging by nothing" takes the style's first-line indent away, and the file says so after the save.
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"5\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"none\"/><w:lvlText w:val=\"\"/>"
                                     "<w:pPr><w:ind w:left=\"0\" w:hanging=\"0\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"5\"><w:abstractNumId w:val=\"5\"/></w:num>"
                                     "<w:abstractNum w:abstractNumId=\"6\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/>"
                                     "<w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"6\"><w:abstractNumId w:val=\"6\"/></w:num>";
        const QByteArray styles = kStyles
            + "<w:style w:type=\"paragraph\" w:styleId=\"Sub\"><w:name w:val=\"Sub\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"5\"/></w:numPr>"
              "<w:ind w:firstLine=\"708\" w:left=\"4956\"/></w:pPr></w:style>"
              "<w:style w:type=\"paragraph\" w:styleId=\"Bare\"><w:name w:val=\"Bare\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"6\"/></w:numPr></w:pPr></w:style>";
        auto doc = open(makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Sub\"/></w:pPr><w:r><w:t>plain</w:t></w:r></w:p>"
                                 "<w:p><w:pPr><w:pStyle w:val=\"Sub\"/><w:ind w:hanging=\"0\" w:left=\"5245\"/></w:pPr><w:r><w:t>flush</w:t></w:r></w:p>"
                                 "<w:p><w:pPr><w:pStyle w:val=\"Bare\"/></w:pPr><w:r><w:t>bare</w:t></w:r></w:p>"
                                 "<w:p><w:pPr><w:pStyle w:val=\"Bare\"/><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"6\"/></w:numPr></w:pPr><w:r><w:t>said</w:t></w:r></w:p>", styles, numbering));
        QTextBlockFormat f0 = doc->story()->findBlockByNumber(0).blockFormat(), f1 = doc->story()->findBlockByNumber(1).blockFormat();
        QCOMPARE(f0.leftMargin(), 4956 / 20.0);
        QCOMPARE(f0.textIndent(), 708 / 20.0);
        QCOMPARE(f1.leftMargin(), 5245 / 20.0);
        QCOMPARE(f1.textIndent(), 0.0);
        QCOMPARE(doc->story()->findBlockByNumber(2).blockFormat().leftMargin(), 36.0);   // (the list's, for the style says none)
        QCOMPARE(doc->story()->findBlockByNumber(2).blockFormat().textIndent(), -18.0);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:ind w:left=\"5245\" w:firstLine=\"0\"/>") || x.contains("<w:ind w:firstLine=\"0\" w:left=\"5245\"/>"), x.constData());
        QCOMPARE(int(x.count("<w:ind")), 1);                    // (nothing else is said about indents)
        QCOMPARE(int(x.count("<w:numPr>")), 1);                 // (only the one the file said again)
        QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
        auto back = open(doc->toDocx());
        QCOMPARE(back->story()->findBlockByNumber(1).blockFormat().textIndent(), 0.0);
        QCOMPARE(back->story()->findBlockByNumber(0).blockFormat().textIndent(), 708 / 20.0);
    }

    void theFontsOfOtherScriptsAreKeptAsTheRunSaidThem()
    {
        // The engine's formats hold the Latin font; what a run says of its East Asian and complex-script fonts, and of its hint, goes with it.
        const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Cjk\"><w:name w:val=\"Cjk\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:rFonts w:eastAsia=\"Noto Sans CJK\"/></w:rPr></w:style>";
        auto doc = open(makeDocx("<w:p><w:r><w:rPr><w:rFonts w:ascii=\"Arial\" w:hAnsi=\"Arial\" w:eastAsia=\"Arial\" w:cs=\"Arial\" w:hint=\"eastAsia\"/></w:rPr><w:t>all four</w:t></w:r></w:p>"
                                 "<w:p><w:r><w:rPr><w:rFonts w:eastAsia=\"Arial\"/></w:rPr><w:t>only one</w:t></w:r></w:p>"
                                 "<w:p><w:pPr><w:pStyle w:val=\"Cjk\"/></w:pPr><w:r><w:rPr><w:rFonts w:eastAsia=\"Noto Sans CJK\"/></w:rPr><w:t>as the style</w:t></w:r>"
                                 "<w:r><w:rPr><w:rFonts w:eastAsia=\"Other\"/></w:rPr><w:t>not as the style</w:t></w:r></w:p>", styles));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:rFonts w:hint=\"eastAsia\" w:ascii=\"Arial\" w:hAnsi=\"Arial\" w:eastAsia=\"Arial\" w:cs=\"Arial\"/>"), x.constData());
        QVERIFY2(x.contains("<w:rFonts w:eastAsia=\"Arial\"/>"), x.constData());
        QVERIFY2(x.contains("<w:rFonts w:eastAsia=\"Other\"/>"), x.constData());
        QCOMPARE(int(x.count("Noto Sans CJK")), 0);   // (said as the style says: the style's, not the run's)
        QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
    }

    void aSectionThatStartsPageNumbersAtZeroKeepsSayingIt()
    {
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>"
                                "<w:pgNumType w:start=\"0\" w:fmt=\"decimal\"/><w:titlePg/></w:sectPr>";
        auto doc = open(makeDocx("<w:p><w:r><w:t>cover</w:t></w:r></w:p>", kStyles, QByteArray(), sect));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:pgNumType w:fmt=\"decimal\" w:start=\"0\"/>"), x.constData());
        // And a file that says nothing of it says nothing.
        auto plain = open(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>", kStyles));
        QVERIFY(!documentXml(plain->toDocx()).contains("pgNumType"));
    }

    void aParagraphThatSaysNoFirstLineIndentKeepsSayingIt()
    {
        // The style indents the first line; the paragraph says "hanging" by nothing, which takes that indent away.
        const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Indented\"><w:name w:val=\"Indented\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:ind w:firstLine=\"720\" w:left=\"0\" w:right=\"0\"/></w:pPr></w:style>";
        auto doc = open(makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Indented\"/><w:ind w:hanging=\"0\" w:right=\"0\"/></w:pPr><w:r><w:t>flush</w:t></w:r></w:p>"
                                 "<w:p><w:pPr><w:pStyle w:val=\"Indented\"/></w:pPr><w:r><w:t>indented</w:t></w:r></w:p>", styles));
        QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().textIndent(), 0.0);
        QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().textIndent(), 36.0);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:ind w:firstLine=\"0\"/>"), x.constData());
        QCOMPARE(int(x.count("<w:ind")), 1);
        // And it reads back the same: no first-line indent on the first, one on the second.
        auto back = open(doc->toDocx());
        QCOMPARE(back->story()->findBlockByNumber(0).blockFormat().textIndent(), 0.0);
        QCOMPARE(back->story()->findBlockByNumber(1).blockFormat().textIndent(), 36.0);
    }

    void hostileDocumentsAreHeldToLimits()
    {
        QString err;
        // Not a package, not a document, not XML.
        QVERIFY(!WriteDoc::fromDocx(QByteArray("not a zip at all"), &err));
        QVERIFY(!err.isEmpty());
        QByteArray deep;
        for (int i = 0; i < 300; ++i) deep += "<w:p>";
        for (int i = 0; i < 300; ++i) deep += "</w:p>";
        QStringList deepNotes;
        QVERIFY2(WriteDoc::fromDocx(makeDocx(deep), &err, &deepNotes), qPrintable(err));   // (nested past the limit: opened, what is below it made plain, and said)
        QVERIFY(!deepNotes.isEmpty());
        // Sizes and counts that no page could have are clamped, and the document opens.
        const QByteArray odd = "<w:p><w:pPr><w:ind w:left=\"99999999999\" w:hanging=\"88888888\"/><w:spacing w:before=\"2000000000\" w:line=\"-5\"/><w:numPr><w:ilvl w:val=\"500\"/><w:numId w:val=\"77\"/></w:numPr></w:pPr>"
                               "<w:r><w:rPr><w:sz w:val=\"9999999\"/></w:rPr><w:t>x</w:t></w:r></w:p>";
        auto doc = open(makeDocx(odd, kStyles, QByteArray(), "<w:sectPr><w:pgSz w:w=\"1\" w:h=\"2000000000\"/><w:pgMar w:top=\"-900000000\" w:left=\"900000000\"/><w:cols w:num=\"100000\"/></w:sectPr>"));
        QVERIFY(doc);
        QVERIFY(doc->pager().pageCount() >= 1);
        QVERIFY(doc->pager().page(0).size.width() >= 36 && doc->pager().page(0).size.height() <= 3168);
        QVERIFY(doc->flow().section(0).columnCount() <= 45);
        // A big document: 30,000 paragraphs open in time in proportion to their number (ten times the paragraphs, about ten times as long), lay out
        // the first page at once, and save.
        auto openTime = [](int count) {
            const QByteArray zip = makeDocx(paragraphs(count, 25), kStyles);
            std::unique_ptr<WriteDoc> d;
            const qint64 ns = bestNanoseconds([&] { d = open(zip); }, 3, [&] { d.reset(); });
            return d ? ns / 1e6 : 1e9;
        };
        openTime(3000);   // (the first of anything is slow)
        const double few = std::max(openTime(3000), 20.0), many = openTime(30000);
        QVERIFY2(many < 25 * few, qPrintable(QStringLiteral("3,000 paragraphs %1 ms, 30,000 paragraphs %2 ms").arg(few).arg(many)));
        auto big = open(makeDocx(paragraphs(30000, 25), kStyles));
        QVERIFY(big->pager().materialize(0));
        QVERIFY(big->pager().pagesLaidOut() <= 2);
        QVERIFY(!big->toDocx().isEmpty());
        QCOMPARE(big->story()->blockCount(), 30000);
    }

    // What the file keeps beside a paragraph's text (its w14:paraId, a bookmark) is unique to each paragraph in a modern file, and the formats must
    // not be compared with every other one each time one is made: opening takes about as long with them as without. (Compared with each other,
    // not against a clock: the same machine, the same load.)
    void openingDoesNotSlowDownWithUniqueIdsAndBookmarks()
    {
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
        auto build = [&](int kind) {
            QByteArray body;
            for (int i = 0; i < 8000; ++i) {
                const QByteArray n = QByteArray::number(i);
                if (kind == 1) body += "<w:p w14:paraId=\"" + QByteArray::number(0x10000000 + i, 16).toUpper() + "\" w14:textId=\"77777777\">";
                else body += "<w:p>";
                if (kind == 2) body += "<w:bookmarkStart w:id=\"" + n + "\" w:name=\"b" + n + "\"/>";
                body += "<w:r><w:t>paragraph " + n + "</w:t></w:r>";
                if (kind == 2) body += "<w:bookmarkEnd w:id=\"" + n + "\"/>";
                body += "</w:p>";
            }
            office::PackageWriter w;
            w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body>" + body + "</w:body></w:document>"),
                      QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
            w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
            return w.finish();
        };
        auto openTime = [&](const QByteArray &zip) {
            std::unique_ptr<WriteDoc> doc;
            const qint64 ns = bestNanoseconds([&] { doc = open(zip); }, 3, [&] { doc.reset(); });
            return doc ? ns / 1e6 : 1e9;
        };
        const QByteArray plain = build(0), withIds = build(1), withBookmarks = build(2);
        openTime(plain);   // (the first of anything is slow: fonts, the first layout)
        const double base = std::max(openTime(plain), 20.0), ids = openTime(withIds), marks = openTime(withBookmarks);
        QVERIFY2(ids < 10 * base, qPrintable(QStringLiteral("with ids %1 ms, without %2 ms").arg(ids).arg(base)));
        QVERIFY2(marks < 10 * base, qPrintable(QStringLiteral("with bookmarks %1 ms, without %2 ms").arg(marks).arg(base)));
    }

    // The root of the main part declares its prefixes on lines of their own; a piece kept as XML uses one the writer would add if it were missing.
    void aRootWithDeclarationsOnSeparateLinesGetsNoneTwice()
    {
        const QByteArray doc = "<w:document\r\n xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"\r\n\txmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"\n"
                               "xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"><w:body><w:p><w:sdt><w:sdtPr><w14:checkbox><w14:checked w14:val=\"0\"/></w14:checkbox></w:sdtPr>"
                               "<w:sdtContent><w:r><w:t>box</w:t></w:r></w:sdtContent></w:sdt></w:p></w:body></w:document>";
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xml(doc), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        const QByteArray original = w.finish();
        auto d = open(original);
        QVERIFY(d);
        const QByteArray saved = d->toDocx();
        const QStringList strict = rt::strictProblems(saved, original);
        QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
        QVERIFY(documentXml(saved).contains("w14:checkbox"));
    }

    // A prefix declared on the paragraph itself (not on the root) goes back with the attributes that use it.
    void aNamespaceDeclaredOnAParagraphStaysDeclared()
    {
        auto d = open(makeDocx("<w:p xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" w14:paraId=\"1234ABCD\"><w:r><w:t>local</w:t></w:r></w:p>"));
        QVERIFY(d);
        const QByteArray saved = d->toDocx();
        const QStringList strict = rt::strictProblems(saved);
        QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
        QVERIFY(documentXml(saved).contains("1234ABCD"));
    }

    // The styles and the lists have links of their own (a picture bullet's image): they are kept, and the image is not left without one.
    void theLinksOfTheStylesAndTheListsAreKept()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter w;
        const QByteArray ns = kNs;
        w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body><w:p><w:r><w:t>text</w:t></w:r></w:p></w:body></w:document>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/styles.xml"), xml("<w:styles " + ns + ">" + kStyles + "</w:styles>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
        w.addPart(QStringLiteral("/word/numbering.xml"), xml("<w:numbering " + ns + "><w:numPicBullet w:numPicBulletId=\"0\"><w:pict><v:imagedata r:id=\"rIdPic\"/></w:pict></w:numPicBullet></w:numbering>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
        w.addPart(QStringLiteral("/word/media/bullet.png"), QByteArray("\x89PNG-bytes"), QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/word/media/swatch.png"), QByteArray("\x89PNG-other"), QStringLiteral("image/png"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdSt"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdNum"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/numbering.xml"), QStringLiteral("rIdPic"), rel + QStringLiteral("image"), QStringLiteral("/word/media/bullet.png"));
        w.addRelationshipWithId(QStringLiteral("/word/styles.xml"), QStringLiteral("rIdSw"), rel + QStringLiteral("image"), QStringLiteral("/word/media/swatch.png"));
        const QByteArray original = w.finish();
        auto d = open(original);
        QVERIFY(d);
        const QByteArray saved = d->toDocx();
        office::Package p;
        QVERIFY(p.read(saved));
        QCOMPARE(p.relationship(QStringLiteral("/word/numbering.xml"), QStringLiteral("rIdPic")).target, QStringLiteral("/word/media/bullet.png"));
        QCOMPARE(p.relationship(QStringLiteral("/word/styles.xml"), QStringLiteral("rIdSw")).target, QStringLiteral("/word/media/swatch.png"));
        QVERIFY(p.hasPart(QStringLiteral("/word/media/bullet.png")) && p.hasPart(QStringLiteral("/word/media/swatch.png")));
    }

    // A bookmark or a comment range may begin in a paragraph and end inside a table that is kept as XML (or the other way about), and two
    // bookmarks may carry the same Id: all of them stay, each start with its end, and the second of an Id is given an Id of its own.
    void bookmarksAndRangesAcrossKeptTablesAndOfRepeatedIdsStay()
    {
        const QByteArray table1 = "<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r><w:bookmarkEnd w:id=\"1\"/><w:commentRangeEnd w:id=\"7\"/></w:p></w:tc></w:tr></w:tbl>";
        const QByteArray table2 = "<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:bookmarkStart w:id=\"2\" w:name=\"fromTable\"/><w:commentRangeStart w:id=\"8\"/><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        const QByteArray body = "<w:p><w:bookmarkStart w:id=\"1\" w:name=\"intoTable\"/><w:commentRangeStart w:id=\"7\"/><w:r><w:t>before</w:t></w:r></w:p>" + table1 +
                                "<w:p><w:r><w:t>between</w:t></w:r></w:p>" + table2 +
                                "<w:p><w:r><w:t>after</w:t></w:r><w:bookmarkEnd w:id=\"2\"/><w:commentRangeEnd w:id=\"8\"/></w:p>"
                                "<w:p><w:bookmarkStart w:id=\"3\" w:name=\"one\"/><w:r><w:t>x</w:t></w:r><w:bookmarkEnd w:id=\"3\"/></w:p>"
                                "<w:p><w:bookmarkStart w:id=\"3\" w:name=\"two\"/><w:r><w:t>y</w:t></w:r><w:bookmarkEnd w:id=\"3\"/></w:p>";
        auto d = open(makeDocx(body));
        QVERIFY(d);
        const QByteArray saved = d->toDocx();
        const QStringList strict = rt::strictProblems(saved);
        QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
        const QByteArray x = documentXml(saved);
        QCOMPARE(int(x.count("<w:bookmarkStart")), 4);
        QCOMPARE(int(x.count("<w:bookmarkEnd")), 4);
        QCOMPARE(int(x.count("<w:commentRangeStart")), 2);
        QCOMPARE(int(x.count("<w:commentRangeEnd")), 2);
        // Every bookmark start has an end of its Id, once; the names are all there.
        QRegularExpression starts(QStringLiteral("<w:bookmarkStart w:id=\"(\\d+)\" w:name=\"(\\w+)\"")), ends(QStringLiteral("<w:bookmarkEnd w:id=\"(\\d+)\""));
        QMap<QString, QString> name;
        for (auto it = starts.globalMatch(QString::fromUtf8(x)); it.hasNext();) {
            const auto m = it.next();
            QVERIFY2(!name.contains(m.captured(1)), "an Id is used by two bookmarks");
            name[m.captured(1)] = m.captured(2);
        }
        QMap<QString, int> endCount;
        for (auto it = ends.globalMatch(QString::fromUtf8(x)); it.hasNext();) ++endCount[it.next().captured(1)];
        QStringList names = name.values();
        names.sort();
        QCOMPARE(names, (QStringList{"fromTable", "intoTable", "one", "two"}));
        for (auto it = name.cbegin(); it != name.cend(); ++it) QVERIFY2(endCount.value(it.key()) == 1, qPrintable(QStringLiteral("bookmark %1 (Id %2) has %3 ends").arg(it.value(), it.key()).arg(endCount.value(it.key()))));
        const QStringList loss = rt::lossProblems(makeDocx(body), saved);
        for (const QString &l : loss) QVERIFY2(!l.startsWith(QLatin1String("element counts")) || (!l.contains(QLatin1String("bookmark")) && !l.contains(QLatin1String("commentRange"))), qPrintable(l));
    }

    // A table nobody touched is written as the file had it, whatever the file had in it that the program would write otherwise: a bookmark that starts and has no end, and a
    // second of one name, a style that is the default said again, an indent of nothing, a toggle spelled "false", and a row that doesn't reach the end of the grid. Once
    // something in it changes it is written from the story, and then the bookmark with no end is left out.
    void aTableNobodyTouchedComesBackAsTheFileHadIt()
    {
        const QByteArray table =
            "<w:tbl><w:tblPr><w:tblW w:w=\"2000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"14\"/></w:tblGrid>"
            "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/></w:tcPr><w:p><w:pPr><w:pStyle w:val=\"Normal\"/><w:ind w:left=\"0\" w:hanging=\"0\"/></w:pPr>"
            "<w:bookmarkStart w:id=\"0\" w:name=\"Check1\"/><w:bookmarkStart w:id=\"1\" w:name=\"Check1\"/><w:bookmarkEnd w:id=\"1\"/>"
            "<w:r><w:rPr><w:b w:val=\"false\"/></w:rPr><w:t>one</w:t></w:r></w:p></w:tc>"
            "<w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>two</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        const QByteArray original = makeDocx("<w:p><w:r><w:t>before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles);
        auto doc = open(original);
        QVERIFY(doc);
        const QByteArray saved = doc->toDocx();
        QVERIFY2(documentXml(saved).contains(table), documentXml(saved).constData());
        const QStringList strict = rt::strictProblems(saved, original), loss = rt::lossProblems(original, saved);
        QVERIFY2(strict.isEmpty() && loss.isEmpty(), qPrintable((strict + loss).join(QLatin1String("; "))));
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->toPlainText().indexOf(QLatin1String("one")));
        insertText(*doc, c, QStringLiteral("x"));
        const QByteArray changed = documentXml(doc->toDocx());
        QVERIFY2(!changed.contains(table) && changed.contains("xone"), changed.constData());
        QCOMPARE(int(changed.count("<w:bookmarkStart")), 1);
        QCOMPARE(int(changed.count("<w:bookmarkEnd")), 1);
        // A table written from the story is one a reader needn't repair: its rows reach the end of the grid (the file's row stops short of the last column, which the
        // row then says it leaves empty), whatever the rows said before.
        const QStringList shape = rt::strictProblems(doc->toDocx(), original);
        QVERIFY2(shape.isEmpty(), qPrintable(shape.join(QLatin1String("; "))));
        QVERIFY2(changed.contains("<w:trPr><w:gridAfter w:val=\"1\"/></w:trPr>"), changed.constData());
    }

    // Enter inside a hyperlink, a content control, a tracked change, or a tag keeps the wrapper on both halves, with its attributes (and so its
    // link): one of each kind becomes two, and the second revision mark gets an Id of its own.
    void enterInsideAWrapperKeepsItOnBothHalves()
    {
        struct Case { const char *tag, *inText; int offset; };
        const Case cases[] = {{"w:hyperlink", "link text in the middle", 9}, {"w:sdt", "content control text", 7}, {"w:ins", "inserted words", 4}, {"w:smartTag", "Paris", 2}};
        for (const Case &k : cases) {
            auto doc = open(richDocx());
            QVERIFY(doc);
            const QMap<QString, int> before = rt::keptCountsOf(doc->toDocx());
            QTextCursor c(doc->story());
            for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
                const int at = b.text().indexOf(QString::fromLatin1(k.inText));
                if (at >= 0) {
                    c.setPosition(b.position() + at + k.offset);
                    break;
                }
            }
            insertParagraph(*doc, c);
            const QByteArray saved = doc->toDocx();
            const QStringList strict = rt::strictProblems(saved);
            QVERIFY2(strict.isEmpty(), qPrintable(QStringLiteral("%1: %2").arg(QString::fromLatin1(k.tag), strict.join(QLatin1Char('\n')))));
            const QMap<QString, int> now = rt::keptCountsOf(saved);
            QVERIFY2(now.value(QString::fromLatin1(k.tag)) == before.value(QString::fromLatin1(k.tag)) + 1, qPrintable(QStringLiteral("%1: %2 before, %3 now").arg(QString::fromLatin1(k.tag)).arg(before.value(QString::fromLatin1(k.tag))).arg(now.value(QString::fromLatin1(k.tag)))));
            const QByteArray x = documentXml(saved);
            if (QByteArray(k.tag) == "w:hyperlink") QCOMPARE(int(x.count("r:id=\"rIdLink\"")), 2);
            if (QByteArray(k.tag) == "w:ins") {
                QSet<QByteArray> ids;
                for (qsizetype at = x.indexOf("<w:ins "); at >= 0; at = x.indexOf("<w:ins ", at + 1)) ids.insert(x.mid(at, x.indexOf('>', at) - at).split('"')[1]);
                for (qsizetype at = x.indexOf("<w:del "); at >= 0; at = x.indexOf("<w:del ", at + 1)) ids.insert(x.mid(at, x.indexOf('>', at) - at).split('"')[1]);
                QCOMPARE(int(ids.size()), 3);   // (the file's two and the new one)
            }
        }
    }

    // Enter at the end of a paragraph leaves the paragraph mark's own with the second half: a section that ends in the paragraph, and the XML
    // kept after it. The first keeps what comes before.
    void enterAtTheEndOfAParagraphMovesTheMarksPropertiesToTheNewOne()
    {
        auto doc = open(richDocx());
        QVERIFY(doc);
        QCOMPARE(doc->sectionCount(), 2);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(0).position() + doc->story()->findBlockByNumber(0).length() - 1);   // the end of the heading that ends the first section
        insertParagraph(*doc, c);
        QCOMPARE(doc->sectionCount(), 2);
        QVERIFY(!doc->story()->findBlockByNumber(0).blockFormat().boolProperty(flow::fp::SectionBreakAfter));
        QVERIFY(doc->story()->findBlockByNumber(1).blockFormat().boolProperty(flow::fp::SectionBreakAfter));
        QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
        QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Normal"));   // (what follows a heading)
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:sectPr")), 2);
        auto back = open(doc->toDocx());
        QCOMPARE(back->sectionCount(), 2);
        // The XML kept after the last paragraph: a block content control's end.
        auto d = open(makeDocx("<w:sdt><w:sdtPr><w:alias w:val=\"B\"/></w:sdtPr><w:sdtContent><w:p><w:r><w:t>in the control</w:t></w:r></w:p></w:sdtContent></w:sdt>"));
        QTextCursor e(d->story());
        e.movePosition(QTextCursor::End);
        insertParagraph(*d, e);
        const QByteArray x = documentXml(d->toDocx());
        QVERIFY2(rt::strictProblems(d->toDocx()).isEmpty(), x.constData());
        QCOMPARE(int(x.count("<w:sdt>")), 1);
        QVERIFY2(x.indexOf("<w:sdt>") < x.indexOf("in the control") && x.indexOf("in the control") < x.indexOf("</w:p><w:p") && x.indexOf("</w:p><w:p") < x.indexOf("</w:sdtContent>"), x.constData());
    }

    // Backspace at the start of a paragraph that has XML kept before it (a block content control's end) joins it to the one before, and the
    // control stays.
    void backspaceIntoAParagraphWithXmlBeforeItKeepsTheXml()
    {
        for (int how = 0; how < 2; ++how) {
            auto doc = open(richDocx());
            QVERIFY(doc);
            const QMap<QString, int> before = rt::keptCountsOf(doc->toDocx());
            QTextCursor c(doc->story());
            QTextBlock after = doc->story()->begin();
            for (; after.isValid(); after = after.next())
                if (after.text().startsWith(QLatin1String("After the block control"))) break;
            QVERIFY(after.isValid());
            const int blocks = doc->story()->blockCount();
            if (how == 0) {
                c.setPosition(after.position());
                deleteBack(*doc, c, false);
            } else {
                c.setPosition(after.previous().position() + after.previous().length() - 1);
                deleteForward(*doc, c, false);
            }
            QCOMPARE(doc->story()->blockCount(), blocks - 1);
            const QByteArray saved = doc->toDocx();
            const QStringList strict = rt::strictProblems(saved);
            QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
            const QMap<QString, int> now = rt::keptCountsOf(saved);
            QCOMPARE(now.value(QStringLiteral("w:sdt")), before.value(QStringLiteral("w:sdt")));
            QVERIFY(documentXml(saved).contains("Inside a block controlAfter the block control"));
        }
    }

    // Pasting before a heading that ends a section leaves the heading its style and its section break, whether what is pasted is words or
    // paragraphs; the section break is not moved to what was pasted.
    void pastingBeforeAHeadingThatEndsASectionLeavesItAlone()
    {
        for (int paragraphs = 1; paragraphs <= 2; ++paragraphs) {
            auto doc = open(richDocx());
            QVERIFY(doc);
            QTextDocument *story = doc->story();
            QTextCursor sel(story);
            const QTextBlock src = story->findBlockByNumber(1);
            sel.setPosition(src.position() + 2);
            sel.setPosition(paragraphs == 1 ? src.position() + 12 : story->findBlockByNumber(2).position() + 6, QTextCursor::KeepAnchor);
            const QTextDocumentFragment held = selectionFragment(sel);
            QTextCursor at(story);
            at.setPosition(0);
            pasteFragment(*doc, at, held);
            QCOMPARE(doc->sectionCount(), 2);
            // The block that holds the heading's own words is the last of what was pasted into, and it is the heading.
            QTextBlock heading = story->begin();
            while (heading.isValid() && !heading.text().endsWith(QLatin1String("Heading that ends a section"))) heading = heading.next();
            QVERIFY(heading.isValid());
            QCOMPARE(heading.blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
            QVERIFY(heading.blockFormat().boolProperty(flow::fp::SectionBreakAfter));
            int breaks = 0;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) breaks += b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
            QCOMPARE(breaks, 1);
            QCOMPARE(int(documentXml(doc->toDocx()).count("<w:sectPr")), 2);
        }
    }

    // A copy made in one document and pasted into another carries its pictures (the bytes, in a part of the new file, with a relationship of its
    // own) and its hyperlinks (a relationship); a piece that refers to a part the new file can't be given (a chart) is not carried, and its
    // text is pasted as text. Nothing saved points at a part that is not there.
    void pastingFromAnotherDocumentCarriesPicturesAndLinksAndNothingDangles()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                              "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                              "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"";
        const QByteArray png("\x89PNG\r\n\x1a\n-picture-bytes-");
        const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">A picture: </w:t></w:r><w:r><w:drawing><wp:inline><wp:extent cx=\"100\" cy=\"100\"/><wp:docPr id=\"1\" name=\"Pic\"/><a:graphic>"
                                "<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"p.png\"/><pic:cNvPicPr/></pic:nvPicPr>"
                                "<pic:blipFill><a:blip r:embed=\"rIdImg\"/></pic:blipFill><pic:spPr/></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r>"
                                "<w:r><w:t xml:space=\"preserve\"> a </w:t></w:r><w:hyperlink r:id=\"rIdLink\"><w:r><w:t>link</w:t></w:r></w:hyperlink><w:r><w:t xml:space=\"preserve\"> a chart: </w:t></w:r>"
                                "<w:r><w:drawing><wp:inline><wp:extent cx=\"100\" cy=\"100\"/><wp:docPr id=\"2\" name=\"Chart\"/><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\">"
                                "<c:chart r:id=\"rIdChart\"/></a:graphicData></a:graphic></wp:inline></w:drawing></w:r><w:r><w:t> end.</w:t></w:r></w:p>";
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body>" + body + "</w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/media/image1.png"), png, QStringLiteral("image/png"));
        w.addPart(QStringLiteral("/word/charts/chart1.xml"), xml("<c:chartSpace " + ns + "/>"), QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdImg"), rel + QStringLiteral("image"), QStringLiteral("/word/media/image1.png"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdLink"), rel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/target"), true);
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdChart"), rel + QStringLiteral("chart"), QStringLiteral("/word/charts/chart1.xml"));
        Clip clip;
        {
            auto source = open(w.finish());
            QVERIFY(source);
            QTextCursor sel(source->story());
            sel.select(QTextCursor::Document);
            clip = copySelection(*source, sel);
            QCOMPARE(int(clip.links.size()), 3);
            // (The same document: nothing is made over.)
            auto same = open(w.finish());
            QTextCursor at(same->story());
            at.movePosition(QTextCursor::End);
            auto target = open(makeDocx("<w:p><w:r><w:t>Target.</w:t></w:r></w:p>", kStyles));
            QVERIFY(target);
            QTextCursor t(target->story());
            t.movePosition(QTextCursor::End);
            pasteClip(*target, t, clip);
            const QByteArray saved = target->toDocx();
            const QStringList strict = rt::strictProblems(saved);
            QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
            QVERIFY2(rt::danglingReferences(saved).isEmpty(), qPrintable(rt::danglingReferences(saved).join(QLatin1Char(' '))));
            office::Package p;
            QVERIFY(p.read(saved));
            const QByteArray x = p.part(p.mainPart());
            QVERIFY(x.contains("A picture: ") && x.contains("link") && x.contains(" end."));
            // The picture: a part of the new file with the bytes, a link to it of the new file's own.
            QStringList images;
            for (const QString &n : p.partNames())
                if (n.startsWith(QLatin1String("/word/media/"))) images << n;
            QCOMPARE(images.size(), 1);
            QCOMPARE(p.part(images[0]), png);
            QCOMPARE(int(x.count("r:embed=")), 1);
            // The hyperlink: its address, in a relationship of the new file.
            const QVector<office::Relationship> links = p.relationshipsOfType(p.mainPart(), QStringLiteral("/hyperlink"));
            QCOMPARE(links.size(), 1);
            QCOMPARE(links[0].target, QStringLiteral("https://example.com/target"));
            QVERIFY(links[0].external);
            // The chart: not carried; the file has no chart part and no reference to one.
            QVERIFY(!x.contains("c:chart"));
            for (const QString &n : p.partNames()) QVERIFY(!n.contains(QLatin1String("chart")));
        }
        // The last document is closed: the kept bytes a copy's formats stood for are gone, and a paste of it is the text.
        auto later = open(makeDocx("<w:p><w:r><w:t>Later.</w:t></w:r></w:p>", kStyles));
        QTextCursor lc(later->story());
        lc.movePosition(QTextCursor::End);
        pasteClip(*later, lc, clip);
        QVERIFY(later->story()->toPlainText().contains(QStringLiteral("link")));
        const QByteArray lsaved = later->toDocx();
        QVERIFY(rt::strictProblems(lsaved).isEmpty() && rt::danglingReferences(lsaved).isEmpty());
    }

    // A copy that took the start of a tag (a content control) and not its end, pasted inside another tag (a hyperlink), leaves the other tag as
    // it was: the stray start is the one that is left out.
    void aStrayStartOfATagPastedInsideAnotherLeavesTheOthersWhole()
    {
        auto doc = open(richDocx());
        QVERIFY(doc);
        const QMap<QString, int> before = rt::keptCountsOf(doc->toDocx());
        QTextDocument *story = doc->story();
        const QTextBlock control = story->findBlockByNumber(3), link = story->findBlockByNumber(1);
        QTextCursor sel(story);
        sel.setPosition(control.position() + 2);                       // inside "A control: "
        sel.setPosition(control.position() + control.text().indexOf(QChar::ObjectReplacementCharacter) + 4, QTextCursor::KeepAnchor);   // past the start of the control and a little text
        const QTextDocumentFragment held = selectionFragment(sel);
        QTextCursor at(story);
        at.setPosition(link.position() + link.text().indexOf(QStringLiteral("link text")) + 4);   // inside the hyperlink's text
        pasteFragment(*doc, at, held);
        const QByteArray saved = doc->toDocx();
        QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
        const QMap<QString, int> now = rt::keptCountsOf(saved);
        QCOMPARE(now.value(QStringLiteral("w:hyperlink")), before.value(QStringLiteral("w:hyperlink")));
        QCOMPARE(now.value(QStringLiteral("w:sdt")), before.value(QStringLiteral("w:sdt")));
    }

    // A file with macros is saved without them, all of them (the project, its signature, the data it keeps, and the links), and the person is told
    // once, in a sentence, at the save.
    void savingADocumentWithMacrosLeavesThemOutAndSaysSo()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + QByteArray(kNs) + "><w:body><w:p><w:r><w:t>with macros</w:t></w:r></w:p></w:body></w:document>"),
                  QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml"));
        w.addPart(QStringLiteral("/word/vbaProject.bin"), QByteArray("VBA-project"), QStringLiteral("application/vnd.ms-office.vbaProject"));
        w.addPart(QStringLiteral("/word/vbaData.xml"), xml("<wne:vbaSuppData xmlns:wne=\"http://schemas.microsoft.com/office/word/2006/wordml\"/>"), QStringLiteral("application/vnd.ms-word.vbaData+xml"));
        w.addPart(QStringLiteral("/word/customizations.xml"), xml("<x/>"), QStringLiteral("application/vnd.ms-word.keyMapCustomizations+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdVba"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject"), QStringLiteral("/word/vbaProject.bin"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdKeys"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/keyMapCustomizations"), QStringLiteral("/word/customizations.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/vbaProject.bin"), QStringLiteral("rIdData"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/wordVbaData"), QStringLiteral("/word/vbaData.xml"));
        const QByteArray original = w.finish();
        auto doc = open(original);
        QVERIFY(doc);
        QVERIFY(doc->hadMacros());
        const QByteArray saved = doc->toDocx();
        office::Package p;
        QVERIFY(p.read(saved));
        QVERIFY(!p.hasPart(QStringLiteral("/word/vbaProject.bin")) && !p.hasPart(QStringLiteral("/word/vbaData.xml")));
        QVERIFY(p.hasPart(QStringLiteral("/word/customizations.xml")));   // (not a macro: kept)
        for (const office::Relationship &r : p.relationships(p.mainPart())) QVERIFY2(!r.type.contains(QLatin1String("vba")), qPrintable(r.type));
        QCOMPARE(p.contentType(p.mainPart()), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        for (const QString &n : p.partNames())
            for (const office::Relationship &r : p.relationships(n)) QVERIFY2(r.external || p.hasPart(r.target), qPrintable(n + QStringLiteral(" -> ") + r.target));
        // Saved from the window: one sentence, once.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile f(dir.filePath(QStringLiteral("macros.docm")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(original);
        f.close();
        WriteWindow win;
        QStringList said;
        win.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(win.openFile(f.fileName()));
        QVERIFY(win.document()->hadMacros());
        QVERIFY(said.isEmpty());
        QVERIFY(win.saveTo(dir.filePath(QStringLiteral("plain.docx"))));
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QLatin1String("macros")) && said[0].contains(QLatin1String(".docx")) && said[0].count(QLatin1Char('.')) <= 2, qPrintable(said[0]));
        QVERIFY(win.saveTo(dir.filePath(QStringLiteral("again.docx"))));
        QCOMPARE(said.size(), 1);
    }

    // A file that could not be read whole (here, a part named so that it can't be one) says so on opening, and is not written over: Save asks
    // for a new name, with a sentence on why, and the original stays as it was.
    void aFileThatCouldNotBeReadWholeIsNotSavedOverItsOriginal()
    {
        QMap<QString, QByteArray> items;
        QVERIFY(jp::readZip(makeDocx("<w:p><w:r><w:t>whole enough</w:t></w:r></w:p>", kStyles), items));
        jp::ZipWriter z;
        for (auto it = items.cbegin(); it != items.cend(); ++it) z.add(it.key(), it.value(), true);
        z.add(QStringLiteral("word/../../evil.xml"), "x", true);
        const QByteArray original = z.finish();
        QStringList notes;
        QString err;
        auto doc = WriteDoc::fromDocx(original, &err, &notes);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY(doc->leftSomethingOut());
        QVERIFY2(!notes.isEmpty() && notes.join(QLatin1Char(' ')).contains(QLatin1String("left out")), qPrintable(notes.join(QLatin1Char(' '))));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("original.docx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(original);
        f.close();
        WriteWindow win;
        QStringList said;
        win.setNotifier([&](const QString &, const QString &text) { said << text; });
        const QString copy = dir.filePath(QStringLiteral("copy.docx"));
        QStringList asked;
        win.setSavePathChooser([&](const QString &suggested, const QString &) { asked << suggested; return copy; });
        QVERIFY(win.openFile(path));
        QCOMPARE(said.size(), 1);   // (what was left out)
        win.view()->insertText(QStringLiteral("edited "));
        QVERIFY(win.save());
        QCOMPARE(asked.size(), 1);
        QCOMPARE(said.size(), 2);
        QVERIFY2(said[1].contains(QLatin1String("new name")), qPrintable(said[1]));
        QVERIFY(QFileInfo::exists(copy));
        QFile again(path);
        QVERIFY(again.open(QIODevice::ReadOnly));
        QCOMPARE(again.readAll(), original);   // (the original is as it was)
        // Save As onto the original is refused, too.
        QVERIFY(!win.saveTo(path));
        QCOMPARE(said.size(), 3);
        QFile once(path);
        QVERIFY(once.open(QIODevice::ReadOnly));
        QCOMPARE(once.readAll(), original);
        // And a document whole is saved where it was, as before.
        const QString whole = dir.filePath(QStringLiteral("whole.docx"));
        QFile wf(whole);
        QVERIFY(wf.open(QIODevice::WriteOnly));
        wf.write(makeDocx("<w:p><w:r><w:t>fine</w:t></w:r></w:p>", kStyles));
        wf.close();
        WriteWindow other;
        QStringList otherSaid;
        other.setNotifier([&](const QString &, const QString &text) { otherSaid << text; });
        QVERIFY(other.openFile(whole));
        other.view()->insertText(QStringLiteral("x"));
        QVERIFY(other.save());
        QVERIFY(otherSaid.isEmpty());
    }

    // A part with a % (or a space, or a letter outside ASCII) in its name is legal in a package, and stays: with its bytes, its content type, and
    // the link that names it.
    void partsWithPercentSignsAndSpacesInTheirNamesAreKept()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + QByteArray(kNs) + "><w:body><w:p><w:r><w:t>pictures</w:t></w:r></w:p></w:body></w:document>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        const QStringList names = {QStringLiteral("/word/media/a%b.png"), QStringLiteral("/word/media/my image.png"), QStringLiteral("/word/media/\u00fc.png"), QStringLiteral("/word/media/50%25.png")};
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        for (int i = 0; i < names.size(); ++i) {
            w.addPart(names[i], QByteArray("png-") + QByteArray::number(i), QStringLiteral("image/png"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdP%1").arg(i), rel + QStringLiteral("image"), names[i]);
        }
        const QByteArray original = w.finish();
        office::Package check;
        QVERIFY(check.read(original));
        for (int i = 0; i < names.size(); ++i) QCOMPARE(check.part(names[i]), QByteArray("png-") + QByteArray::number(i));
        auto doc = open(original);
        QVERIFY(doc);
        const QByteArray saved = doc->toDocx();
        office::Package p;
        QVERIFY(p.read(saved));
        for (int i = 0; i < names.size(); ++i) {
            QVERIFY2(p.hasPart(names[i]), qPrintable(names[i]));
            QCOMPARE(p.part(names[i]), QByteArray("png-") + QByteArray::number(i));
            QCOMPARE(p.contentType(names[i]), QStringLiteral("image/png"));
            QCOMPARE(p.relationship(p.mainPart(), QStringLiteral("rIdP%1").arg(i)).target, names[i]);
        }
        const QStringList loss = rt::lossProblems(original, saved);
        for (const QString &l : loss) QVERIFY2(!l.startsWith(QLatin1String("part")) && !l.startsWith(QLatin1String("relationships")) && !l.startsWith(QLatin1String("content type")), qPrintable(l));
        // The names in the ZIP are the escaped ones.
        QMap<QString, QByteArray> items;
        QVERIFY(jp::readZip(saved, items));
        QVERIFY(items.contains(QStringLiteral("word/media/a%25b.png")) && items.contains(QStringLiteral("word/media/my%20image.png")));
    }

    // Tables nested deeper than the limit (as in a real file that tests such readers) don't refuse the file: the structure down to the limit is kept,
    // and the text below it comes as plain paragraphs, with a note. What is saved opens again, plainly.
    void tablesNestedPastTheLimitOpenWithTheTextBelowAsPlainParagraphs()
    {
        const int levels = 700;
        QByteArray body;
        for (int i = 0; i < levels; ++i) body += "<w:tbl><w:tblGrid><w:gridCol w:w=\"2400\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>Nested level " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        for (int i = 0; i < levels; ++i) body += "</w:tc></w:tr></w:tbl><w:p><w:r><w:t>after " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        const QByteArray original = makeDocx(body);
        QString err;
        QStringList notes;
        auto doc = WriteDoc::fromDocx(original, &err, &notes);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY2(!notes.isEmpty() && notes.join(QLatin1Char(' ')).contains(QLatin1String("deeper")), qPrintable(notes.join(QLatin1Char(' '))));
        QVERIFY(doc->leftSomethingOut());
        // (The table is a piece kept as XML, shown as a stand-in; what is in it is in the file when it is saved.)
        int locked = 0;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) locked += isLocked(b);
        QCOMPARE(locked, 1);
        const QByteArray saved = doc->toDocx();
        QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
        const QByteArray x = documentXml(saved);
        for (int i : {0, 1, 199, 200, levels - 1}) {
            QVERIFY2(x.contains("Nested level " + QByteArray::number(i) + "<"), qPrintable(QString::number(i)));
            QVERIFY2(x.contains("after " + QByteArray::number(i) + "<"), qPrintable(QString::number(i)));
        }
        QStringList again;
        auto back = WriteDoc::fromDocx(saved, &err, &again);
        QVERIFY2(back, qPrintable(err));
        // (The table below the limit is the stand-in still, and the file's reader says so; nothing is made plain this time.)
        QVERIFY2(again.size() == 1 && again[0].contains(QLatin1String("stand-in")), qPrintable(again.join(QLatin1Char(' '))));
        QCOMPARE(documentXml(back->toDocx()), x);
    }

    void aRichDocumentComesBackFaithfulAndWellFormed()
    {
        const QByteArray original = richDocx();
        auto doc = open(original);
        QVERIFY(doc);
        QString err;
        const QByteArray saved = doc->toDocx(&err);
        QVERIFY2(!saved.isEmpty(), qPrintable(err));
        const QStringList strict = rt::strictProblems(saved, original), loss = rt::lossProblems(original, saved);
        QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
        QVERIFY2(loss.isEmpty(), qPrintable(loss.join(QLatin1Char('\n'))));
        auto back = open(saved);
        QVERIFY(back);
        const QStringList story = rt::storyProblems(*doc, *back);
        QVERIFY2(story.isEmpty(), qPrintable(story.join(QLatin1Char('\n'))));
    }

    // Random editing of a document that holds everything a file keeps: after each few commands the file it makes is well-formed, opens again,
    // writes what it wrote, has a section for each break, and (when the commands only add) has lost nothing that the file had.
    // JW_FUZZ_SEEDS says how many seeds; JW_FUZZ_FROM where to begin; JW_FUZZ_COUNTS=1 prints how many seeds failed each check.
    void randomEditingOfARichDocumentKeepsWhatItHolds()
    {
        const QByteArray original = richDocx();
        const int seeds = qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 8;
        QMap<QString, int> failed;
        QMap<QString, QString> example;
        QSet<QString> seedsFailing;
        for (int mode = 0; mode < 3; ++mode) {   // 0: commands that only add; 1: those that take away too; 2: those, and then every one undone
            auto base = open(original);
            QVERIFY(base);
            const QMap<QString, int> start = rt::keptCountsOf(base->toDocx());
            for (unsigned seed = std::max(1, qEnvironmentVariableIntValue("JW_FUZZ_FROM")); seed <= unsigned(seeds); ++seed) {
                auto doc = open(original);
                const QByteArray baseline = documentXml(doc->toDocx());
                std::mt19937 rng(seed * 2 + mode);
                QTextCursor c(doc->story());
                auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
                auto place = [&] { c.setPosition(pick(doc->story()->characterCount() - 1)); };
                auto selectSome = [&] {
                    const int a = pick(doc->story()->characterCount() - 1), len = pick(60);
                    c.setPosition(a);
                    c.setPosition(std::min(a + len, doc->story()->characterCount() - 1), QTextCursor::KeepAnchor);
                };
                QTextDocumentFragment held;
                QStringList log;
                bool seedFailed = false;   // (the file can't be made or read: no more steps)
                QSet<QString> thisSeed;
                auto fail = [&](const QString &check, const QString &detail, bool fatal = false) {
                    const QString key = QStringLiteral("mode %1: %2").arg(mode).arg(check);
                    if (!thisSeed.contains(key)) {
                        thisSeed.insert(key);
                        failed[key] += 1;
                        if (!example.contains(key)) example[key] = QStringLiteral("seed %1: %2\nlast steps: %3").arg(seed).arg(detail, log.mid(std::max<qsizetype>(0, log.size() - 4)).join(QStringLiteral(" | ")));
                        if (qEnvironmentVariableIsSet("JW_FUZZ_DUMP")) {
                            QString dump;
                            for (QTextBlock bk = doc->story()->begin(); bk.isValid(); bk = bk.next()) {
                                QString line;
                                for (QTextBlock::iterator it = bk.begin(); !it.atEnd(); ++it) {
                                    const QTextFragment fr = it.fragment();
                                    if (fr.charFormat().hasProperty(prop::Atom)) {
                                        const wml::Inline a = unpackAtom(storedBytes(fr.charFormat(), prop::Atom));
                                        const QString mark = (a.role == wml::Inline::Open ? QStringLiteral("<") : a.role == wml::Inline::Close ? QStringLiteral("</") : QStringLiteral("[")) + a.tag + (a.role == wml::Inline::Single ? QStringLiteral("]") : QStringLiteral(">"));
                                        for (QChar ch : fr.text()) line += ch == QChar::ObjectReplacementCharacter ? mark : QString(ch);
                                    } else {
                                        line += fr.text();
                                    }
                                }
                                dump += QStringLiteral("  [%1] %2%3%4\n").arg(bk.blockNumber()).arg(line.left(200), bk.blockFormat().boolProperty(flow::fp::SectionBreakAfter) ? QStringLiteral(" SECT") : QString(), isLocked(bk) ? QStringLiteral(" LOCKED") : QString());
                            }
                            qInfo("seed %d mode %d %s: %s\n%s", int(seed), mode, qPrintable(check), qPrintable(detail), qPrintable(dump));
                        }
                    }
                    seedFailed = seedFailed || fatal;
                    seedsFailing.insert(QStringLiteral("%1/%2").arg(mode).arg(seed));
                };
                for (int step = 0; step < 120 && !seedFailed; ++step) {
                    int op = pick(mode == 0 ? 15 : 23);
                    if (mode == 0 && op >= 12) op += 5;   // (the commands that add to the tables: a row, a column, Tab)
                    if (mode == 2 && (op == 10 || op == 11)) continue;   // (no undo or redo among the commands: they are all undone at the end)
                    log << QStringLiteral("op%1@%2").arg(op).arg(c.position());
                    switch (op) {
                    case 0: place(); typeText(*doc, c, QStringLiteral("word ")); break;
                    case 1: place(); insertParagraph(*doc, c); break;
                    case 2: place(); insertPageBreak(*doc, c); break;
                    case 3: place(); applyParagraphStyle(*doc, c, pick(2) ? QStringLiteral("Heading1") : QStringLiteral("Normal")); break;
                    case 4: selectSome(); toggleBold(*doc, c); break;
                    case 5: selectSome(); toggleList(*doc, c, pick(2)); break;
                    case 6: selectSome(); held = selectionFragment(c); break;
                    case 7: case 8: place(); pasteFragment(*doc, c, held); break;
                    case 9: if (pick(3) == 0) { place(); insertSectionBreak(*doc, c); } break;
                    case 10: doc->story()->undo(&c); break;
                    case 11: doc->story()->redo(&c); break;
                    case 12: place(); deleteBack(*doc, c, false); break;
                    case 13: place(); deleteForward(*doc, c, false); break;
                    case 14: selectSome(); removeSelection(*doc, c); break;
                    case 15: place(); insertParagraph(*doc, c); deleteBack(*doc, c, false); break;
                    case 17: case 18: case 19: case 20: case 21: case 22: tableCommand(*doc, c, op - 17, pick); break;
                    default: selectSome(); c.removeSelectedText(); break;
                    }
                    if (qEnvironmentVariableIsSet("JW_FUZZ_TRACE")) {
                        QString line;
                        for (QTextBlock bk = doc->story()->begin(); bk.isValid() && bk.blockNumber() < 3; bk = bk.next()) {
                            QString t;
                            for (QTextBlock::iterator it = bk.begin(); !it.atEnd(); ++it) {
                                const QTextFragment fr = it.fragment();
                                if (fr.charFormat().hasProperty(prop::Atom)) {
                                    const wml::Inline a = unpackAtom(storedBytes(fr.charFormat(), prop::Atom));
                                    for (QChar ch : fr.text()) t += ch == QChar::ObjectReplacementCharacter ? QStringLiteral("{%1%2}").arg(a.role == wml::Inline::Close ? QStringLiteral("/") : QString(), a.tag.left(3)) : QString(ch);
                                } else {
                                    t += fr.text();
                                }
                            }
                            line += QStringLiteral(" [%1]").arg(t.left(110));
                        }
                        qInfo("step %d op%d@%d: %s", step, op, c.position(), qPrintable(line));
                    }
                    if (mode == 2) continue;
                    if (step % 6 != 5) continue;
                    QString err;
                    const QByteArray zip = doc->toDocx(&err);
                    if (zip.isEmpty()) { fail(QStringLiteral("not written"), err, true); break; }
                    const QStringList strict = rt::strictProblems(zip, original);
                    if (!strict.isEmpty()) { fail(QStringLiteral("not well-formed"), strict.first().left(200), true); break; }
                    auto back = WriteDoc::fromDocx(zip, &err);
                    if (!back) { fail(QStringLiteral("does not open again"), err, true); break; }
                    int breaks = 0;
                    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) breaks += b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
                    if (doc->sectionCount() != breaks + 1 || back->sectionCount() != doc->sectionCount()) fail(QStringLiteral("sections"), QStringLiteral("%1 sections, %2 breaks, %3 after reopening").arg(doc->sectionCount()).arg(breaks).arg(back->sectionCount()));
                    const QByteArray again = documentXml(back->toDocx());
                    if (again != documentXml(zip)) {
                        const QByteArray one = documentXml(zip);
                        int at = 0;
                        while (at < one.size() && at < again.size() && one[at] == again[at]) ++at;
                        fail(QStringLiteral("drifts"), QStringLiteral("the second save differs from the first at %1: %2 | %3").arg(at).arg(QString::fromUtf8(one.mid(std::max(0, at - 150), 330)), QString::fromUtf8(again.mid(std::max(0, at - 150), 330))));
                    }
                    if (mode == 0) {
                        const QMap<QString, int> now = rt::keptCountsOf(zip);
                        for (auto it = start.cbegin(); it != start.cend(); ++it)
                            if (now.value(it.key()) < it.value()) fail(QStringLiteral("lost ") + it.key(), QStringLiteral("%1 %2->%3").arg(it.key()).arg(it.value()).arg(now.value(it.key())));
                    }
                }
                if (mode == 2) {
                    // Every command undone: the file it makes is what the document made before them.
                    int undone = 0;
                    while (doc->story()->isUndoAvailable() && undone < 100000) {
                        doc->story()->undo();
                        ++undone;
                    }
                    const QByteArray now = documentXml(doc->toDocx());
                    if (now != baseline) {
                        int at = 0;
                        while (at < now.size() && at < baseline.size() && now[at] == baseline[at]) ++at;
                        fail(QStringLiteral("undo leaves a change"), QStringLiteral("after %1 undos, the file differs from the one before the commands at %2: %3 | %4").arg(undone).arg(at).arg(QString::fromUtf8(baseline.mid(std::max(0, at - 100), 260)), QString::fromUtf8(now.mid(std::max(0, at - 100), 260))));
                    }
                }
            }
        }
        QStringList report;
        for (auto it = failed.cbegin(); it != failed.cend(); ++it) report << QStringLiteral("%1: %2 seeds\n  %3").arg(it.key()).arg(it.value()).arg(example.value(it.key()));
        if (qEnvironmentVariableIsSet("JW_FUZZ_COUNTS")) qInfo("fuzz: %d of %d runs failed:\n%s", int(seedsFailing.size()), 3 * seeds, qPrintable(report.join(QLatin1Char('\n'))));
        QVERIFY2(failed.isEmpty(), qPrintable(QStringLiteral("%1 of %2 runs failed:\n%3").arg(seedsFailing.size()).arg(3 * seeds).arg(report.join(QLatin1Char('\n')))));
    }

    // ---- hostile tables

    // How long a document takes to open and to page, in seconds.
    static double secondsToOpenAndPage(const QByteArray &zip, int *pages = nullptr, QStringList *notes = nullptr)
    {
        std::unique_ptr<WriteDoc> doc;
        QString err;
        int n = 0;
        // Three runs: a document of thousands of rows takes seconds to open.
        const qint64 ns = bestNanoseconds([&] {
            doc = WriteDoc::fromDocx(zip, &err, notes);
            if (doc) n = doc->pager().pageCount();
        }, 3, [&] {
            doc.reset();
            err.clear();
            if (notes) notes->clear();
        });
        if (!doc) {
            qWarning("open: %s", qPrintable(err));
            return 1e9;
        }
        if (pages) *pages = n;
        return ns / 1e9;
    }

    // A table of `rows` by `columns` short cells.
    static QByteArray manyCells(int rows, int columns)
    {
        const int width = std::max(300, 9000 / columns);
        QByteArray rowsXml;
        for (int r = 0; r < rows; ++r) {
            QByteArray cells;
            for (int c = 0; c < columns; ++c) cells += cellX(cellP("r" + QByteArray::number(r) + "c" + QByteArray::number(c)), QByteArray(), width);
            rowsXml += rowX(cells);
        }
        return tableX(rowsXml, columns, width);
    }

    // A table that has more cells than a document lays out (300 rows of 256 columns) is kept as it was, with a note that says so, and costs next to nothing to open; and a
    // document that has as many as it may opens in seconds, and in time that grows with its cells.
    void hostileTablesOpenInTimeThatGrowsWithTheirCells()
    {
        {   // more cells than the document lays out: a stand-in, with the note, saved as it was
            const QByteArray table = manyCells(300, 256);
            const QByteArray zip = makeDocx(cellP("before") + table + cellP("after"), kStyles);
            QStringList notes;
            int pages = 0;
            const double seconds = secondsToOpenAndPage(zip, &pages, &notes);
            QVERIFY2(seconds < 5, qPrintable(QString::number(seconds)));
            QVERIFY2(notes.size() == 1 && notes[0].contains(QLatin1String("table")), qPrintable(notes.join(QLatin1Char('|'))));
            QCOMPARE(pages, 1);
            auto doc = open(zip);
            QVERIFY(doc);
            QVERIFY(documentXml(doc->toDocx()).contains(table));
        }
        {   // 20,000 rows: real, in pages, in time that is not much more than for a tenth of them
            int small = 0, big = 0;
            const double secondsSmall = secondsToOpenAndPage(makeDocx(cellP("before") + manyCells(2000, 2) + cellP("after"), kStyles), &small);
            const double secondsBig = secondsToOpenAndPage(makeDocx(cellP("before") + manyCells(20000, 2) + cellP("after"), kStyles), &big);
            QVERIFY2(small > 30 && small < 80, qPrintable(QString::number(small)));
            QVERIFY2(big > 10 * small - 10 && big < 10 * small + 20, qPrintable(QStringLiteral("%1 pages for 2,000 rows, %2 for 20,000").arg(small).arg(big)));
            QVERIFY2(secondsBig < 12 * secondsSmall + 3, qPrintable(QStringLiteral("%1 s for 2,000 rows, %2 s for 20,000").arg(secondsSmall).arg(secondsBig)));
        }
        {   // as many cells as a document lays out
            int pages = 0;
            QStringList notes;
            const double seconds = secondsToOpenAndPage(makeDocx(cellP("before") + manyCells(5000, 10) + cellP("after"), kStyles), &pages, &notes);
            QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char('|'))));
            QVERIFY2(pages > 150 && pages < 400, qPrintable(QString::number(pages)));
            QVERIFY2(seconds < 15, qPrintable(QStringLiteral("%1 s for 50,000 cells").arg(seconds)));
        }
    }

    // A cell of three thousand paragraphs (a row of a thousand parts on small pages) opens, and every paragraph of it is on a page.
    void aCellOfThreeThousandLinesOpensAndIsPagedToTheEnd()
    {
        QByteArray lines;
        for (int i = 0; i < 3000; ++i) lines += cellP("line " + QByteArray::number(i));
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"5200\" w:h=\"2400\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray zip = makeDocx(tableX(rowX(cellX(lines, QByteArray(), 3000)), 1, 3000) + cellP("after"), kStyles, QByteArray(), sect);
        std::unique_ptr<WriteDoc> doc;
        int pages = 0;
        const qint64 ms = bestNanoseconds([&] {
            doc = open(zip);
            if (doc) pages = doc->pager().pageCount();
        }, 3, [&] { doc.reset(); }) / 1000000;
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        QVERIFY2(ms < 20000, qPrintable(QString::number(ms)));
        QVERIFY2(pages > 1000 && pages < 4000, qPrintable(QString::number(pages)));
        // Every paragraph of the cell is on a page: the last part has the last, and the parts go on from one to the next.
        double offset = -1;
        for (int k = 0; k < pages; k += 37) {
            const auto page = pg.materialize(k);
            QVERIFY(page && !page->tables.isEmpty());
            const auto *part = page->tables[0]->partOf(0);
            if (part) {
                QVERIFY2(part->piece.offsets[0] > offset, qPrintable(QString::number(k)));
                offset = part->piece.offsets[0];
            }
        }
        int lastWithTable = pages - 1;
        while (lastWithTable > 0 && pg.materialize(lastWithTable)->tables.isEmpty()) --lastWithTable;
        const auto last = pg.materialize(lastWithTable);
        QVERIFY(last && !last->tables.isEmpty() && last->tables[0]->partOf(0) && last->tables[0]->partOf(0)->piece.next == 0);
        QVERIFY(lastWithTable >= pages - 2);
        QVERIFY(plain(*doc).contains(QStringLiteral("line 2999")));
        QCOMPARE(rt::strictProblems(doc->toDocx()).size(), 0);
    }

    // Tables nested past the limit of depth are kept whole where they are, with the note, and the ones above it are tables, laid out with the tables inside them.
    void tablesNestedPastTheLimitAreLaidOutDownToIt()
    {
        QByteArray inner = cellP("level 11");
        for (int level = 10; level >= 0; --level)
            inner = cellP("level " + QByteArray::number(level)) + tableX(rowX(cellX(inner, QByteArray(), 4000) + cellX(cellP("beside " + QByteArray::number(level)), QByteArray(), 2000)), 2, 3000) + cellP("end of " + QByteArray::number(level));
        const QByteArray zip = makeDocx(inner, kStyles);
        QStringList notes;
        int pages = 0;
        const double seconds = secondsToOpenAndPage(zip, &pages, &notes);
        QVERIFY2(seconds < 5, qPrintable(QString::number(seconds)));
        QVERIFY2(notes.size() == 1 && notes[0].contains(QLatin1String("table")), qPrintable(notes.join(QLatin1Char('|'))));
        QVERIFY(pages >= 1 && pages < 20);
        auto doc = open(zip);
        QVERIFY(doc);
        for (int k = 0; k < doc->pager().pageCount(); ++k) QVERIFY(doc->pager().materialize(k));
        const QByteArray saved = doc->toDocx();
        QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
        QVERIFY(documentXml(saved).contains("level 11<"));
    }

    // ---- random editing of tables

    // A document of tables for the random commands: a table with a style, a header row, cells joined down (one with a long text), a row with a gap before its cells and one
    // with a gap after, a long cell, a table in a cell with another in its own, a plain table after it, and text around them; on small pages, so that rows go over page
    // breaks.
    // A box set at a place that the text goes round, as a run (VML, as a file converted from the older form has them): `kind` is the wrap: 0 square, 1 top and bottom, 2 square
    // with the text on the left, 3 on the larger side, 4 tight with the text on the right, 5 a square one set from the page.
    static QByteArray wallRun(int kind, int left, int top, int w, int h)
    {
        static const char *const wraps[] = {"<w10:wrap type=\"square\"/>", "<w10:wrap type=\"topAndBottom\"/>", "<w10:wrap type=\"square\" side=\"left\"/>", "<w10:wrap type=\"square\" side=\"largest\"/>",
                                            "<w10:wrap type=\"tight\" side=\"right\"/>", "<w10:wrap type=\"square\"/>"};
        const QByteArray from = kind == 5 ? ";mso-position-horizontal-relative:page;mso-position-vertical-relative:page" : QByteArray();
        return "<w:r><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\"><v:rect id=\"wall" + QByteArray::number(left * 31 + top) + "\" style=\"position:absolute;margin-left:" +
               QByteArray::number(left) + "pt;margin-top:" + QByteArray::number(top) + "pt;width:" + QByteArray::number(w) + "pt;height:" + QByteArray::number(h) + "pt" + from + "\" fillcolor=\"red\" stroked=\"f\">" + wraps[kind % 6] + "</v:rect></w:pict></w:r>";
    }
    // The tables of that document, and its styles and its section's properties; `noteInCell` is the XML of a run put in the cell b1 (a footnote's reference).
    static RichTables richTables(const QByteArray &noteInCell = QByteArray())
    {
        QByteArray longCell, tallCell;
        for (int i = 0; i < 24; ++i) tallCell += cellP("tall " + QByteArray::number(i));
        for (int i = 0; i < 9; ++i) longCell += cellP("merged line " + QByteArray::number(i));
        const QByteArray deep = tableX(rowX(cellX(cellP("deep"), QByteArray(), 700)), 1, 700);
        const QByteArray nested = tableX(rowX(cellX(cellP("n00"), QByteArray(), 800) + cellX(cellP("n01") + deep + cellP("n01 tail"), QByteArray(), 800)) + rowX(cellX(cellP("n10"), QByteArray(), 800) + cellX(cellP("n11"), QByteArray(), 800)), 2, 800);
        const QByteArray styled = tableX(rowX(cellX(cellP("h0"), QByteArray(), 1100) + cellX(cellP("h1"), QByteArray(), 1100) + cellX(cellP("h2"), QByteArray(), 1100), "<w:tblHeader/>") +
                                             rowX(cellX(longCell, "<w:vMerge w:val=\"restart\"/>", 1100) + cellX(cellP("a1"), "<w:tcBorders><w:bottom w:val=\"double\" w:sz=\"12\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders>", 1100) + cellX(cellP("a2") + nested + cellP("a2 tail"), QByteArray(), 1100)) +
                                             rowX(cellX("<w:p/>", "<w:vMerge/>", 1100) + cellX("<w:p><w:r><w:t>b1</w:t></w:r>" + noteInCell + "</w:p>", QByteArray(), 1100) + cellX(cellP("b2"), QByteArray(), 1100)) +
                                             rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 2200), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"1100\" w:type=\"dxa\"/>") +
                                             rowX(cellX(cellP("d0"), QByteArray(), 1100) + cellX(tallCell, QByteArray(), 1100) + cellX(cellP("d2"), "<w:tcBorders><w:top w:val=\"single\" w:sz=\"24\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders>", 1100), "<w:trHeight w:val=\"700\" w:hRule=\"atLeast\"/>") +
                                             rowX(cellX(cellP("e0"), QByteArray(), 1100) + cellX(cellP("e1"), QByteArray(), 1100), "<w:gridAfter w:val=\"1\"/><w:wAfter w:w=\"1100\" w:type=\"dxa\"/>"),
                                         3, 1100, "<w:tblStyle w:val=\"TG\"/><w:tblW w:w=\"3300\" w:type=\"dxa\"/><w:tblLook w:val=\"0420\"/>");
        const QByteArray plain = tableX(rowX(cellX(cellP("p00")) + cellX(cellP("p01")), "<w:trHeight w:val=\"600\" w:hRule=\"atLeast\"/>") +
                                            rowX(cellX(cellP("p10")) + cellX(cellP("p11"), "<w:tcBorders><w:left w:val=\"nil\"/><w:bottom w:val=\"single\" w:sz=\"18\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders><w:vAlign w:val=\"bottom\"/>"),
                                                 "<w:trHeight w:val=\"500\" w:hRule=\"atLeast\"/>"),
                                        2, 1600);
        // A table with space between its cells (its lines round the lot, the cells' own as they say), rows of a set height, and cells joined down.
        const QByteArray none = "<w:tcBorders><w:top w:val=\"nil\"/><w:left w:val=\"nil\"/><w:bottom w:val=\"nil\"/><w:right w:val=\"nil\"/></w:tcBorders>";
        const QByteArray spaced = tableX(rowX(cellX(cellP("s00"), none, 1200) + cellX(cellP("s01"), "<w:tcBorders><w:bottom w:val=\"single\" w:sz=\"24\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders>", 1200), "<w:trHeight w:val=\"500\" w:hRule=\"atLeast\"/>") +
                                         rowX(cellX(cellP("s10") + cellP("s10 more"), "<w:vMerge w:val=\"restart\"/>", 1200) + cellX(cellP("s11"), none, 1200)) +
                                         rowX(cellX("<w:p/>", "<w:vMerge/>", 1200) + cellX(cellP("s21"), "<w:tcBorders><w:top w:val=\"double\" w:sz=\"8\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders>", 1200)),
                                         2, 1200, "<w:tblW w:w=\"0\" w:type=\"auto\"/><w:tblCellSpacing w:w=\"15\" w:type=\"dxa\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"12\" w:space=\"0\" w:color=\"auto\"/><w:left w:val=\"single\" w:sz=\"8\" w:space=\"0\" w:color=\"auto\"/>"
                                         "<w:bottom w:val=\"single\" w:sz=\"12\" w:space=\"0\" w:color=\"auto\"/><w:right w:val=\"single\" w:sz=\"8\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders>");
        const QByteArray styles = kStyles +
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
                                  "<w:pPr><w:spacing w:before=\"120\" w:after=\"60\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Arimo\" w:hAnsi=\"Arimo\"/><w:b/><w:sz w:val=\"26\"/></w:rPr></w:style>"
                                  "<w:style w:type=\"table\" w:styleId=\"TG\"><w:name w:val=\"TG\"/><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
                                  "<w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
                                  "<w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders></w:tblPr><w:tblStylePr w:type=\"firstRow\"><w:rPr><w:b/></w:rPr></w:tblStylePr>"
                                  "<w:tblStylePr w:type=\"band2Horz\"><w:tcPr><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"EEEEEE\"/></w:tcPr></w:tblStylePr></w:style>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"5200\" w:h=\"4400\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
        return {styled, plain, styles, sect, spaced};
    }
    // A table set at a place, with the text going round it: two rows of two cells, `wide` points a cell, at the right margin, from the paragraph after it.
    static QByteArray floatingTable(int wide)
    {
        const QByteArray pr = "<w:tblpPr w:leftFromText=\"120\" w:rightFromText=\"120\" w:vertAnchor=\"text\" w:horzAnchor=\"margin\" w:tblpXSpec=\"right\" w:tblpY=\"60\"/><w:tblOverlap w:val=\"never\"/><w:tblW w:w=\"0\" w:type=\"auto\"/>";
        const int twips = wide * 20;
        return tableX(rowX(cellX(cellP("float a"), QByteArray(), twips) + cellX(cellP("float b words"), QByteArray(), twips)) + rowX(cellX(cellP("float c"), QByteArray(), twips) + cellX(cellP("d"), QByteArray(), twips)), 2, twips, pr);
    }
    static QByteArray richTableDocx()
    {
        const RichTables t = richTables();
        // (Objects that the text goes round: in the paragraphs around the tables, of each wrap: edits that move their anchors across the pages change which pages have them.)
        const QByteArray frame = "<w:framePr w:w=\"1800\" w:hSpace=\"120\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\" w:y=\"60\"/>";
        return makeDocx(plainP("before the tables and a few more words so that a line goes round the box", wallRun(0, 110, 4, 50, 40)) + t.styled + framedP("FRAME one of two paragraphs", frame) + framedP("and the second of them, which is longer than a line of the frame", frame) +
                            plainP("between them, with enough words that more than a line or two are beside the box that is set from the page", wallRun(5, 150, 60, 40, 50)) +
                            floatingTable(60) + plainP("after the table that is set at a place, with words enough for the text to go round it for a few lines, and a few more words") + t.plain +
                            plainP("then one with space between its cells") + t.spaced +
                            plainP("after the tables, and a line or two of words more", wallRun(1, 20, 6, 40, 14) + wallRun(2, 60, 14, 40, 30)),
                        t.styles, QByteArray(), t.sect);
    }

    // A file for the random commands with the tables, notes, headers, footers, and sections together: a first section of tables (the one of richTableDocx, a note cited in
    // the paragraph before them, one in a cell, and a long one after them), with headers and footers for the first, odd, and even pages of other heights; a continuous section
    // of two columns with a table in it and a note; and a section on a page of its own with a header of its own, a note, and a page break.
    static QByteArray richMixedDocx()
    {
        const RichTables t = richTables(refTo(3));
        const QString rel = QStringLiteral("rId");
        auto lines = [](const char *name, int n) {
            QList<QByteArray> out;
            for (int i = 1; i <= n; ++i) out << "<w:r><w:t>" + QByteArray(name) + " " + QByteArray::number(i) + "</w:t></w:r>";
            return out;
        };
        QMap<QString, QByteArray> parts;
        parts[QStringLiteral("/word/header1.xml")] = headerPart("hdr", lines("ODD HEAD", 1));
        parts[QStringLiteral("/word/header2.xml")] = headerPart("hdr", lines("FIRST HEAD", 2));
        parts[QStringLiteral("/word/header3.xml")] = headerPart("hdr", lines("EVEN HEAD", 4));
        parts[QStringLiteral("/word/header4.xml")] = headerPart("hdr", lines("THIRD HEAD", 3));
        parts[QStringLiteral("/word/footer1.xml")] = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple><w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>"
                                                                       "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>1</w:t></w:r></w:fldSimple>"});
        parts[QStringLiteral("/word/footer2.xml")] = headerPart("ftr", lines("FIRST FOOT", 2));
        parts[QStringLiteral("/word/footer3.xml")] = headerPart("ftr", lines("EVEN FOOT", 3));
        parts[QStringLiteral("/word/footnotes.xml")] = notesPart({{2, "ALPHA note of the paragraph before the tables"}, {3, wordsOf("BETA", 25)}, {4, wordsOf("GAMMA", 220)}, {5, wordsOf("DELTA", 8)}, {6, wordsOf("EPSILON", 15)}});
        const QByteArray refsOne = "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:headerReference w:type=\"first\" r:id=\"rIdheader2\"/><w:headerReference w:type=\"even\" r:id=\"rIdheader3\"/>"
                                   "<w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:footerReference w:type=\"first\" r:id=\"rIdfooter2\"/><w:footerReference w:type=\"even\" r:id=\"rIdfooter3\"/>";
        const QByteArray one = sectionXml(7200, 6600, 720, refsOne, QByteArray(), "<w:titlePg/>");
        const QByteArray two = sectionXml(7200, 6600, 720, QByteArray(), "<w:type w:val=\"continuous\"/>", "<w:cols w:num=\"2\" w:space=\"360\"/>");
        const QByteArray three = sectionXml(7200, 6600, 900, "<w:headerReference w:type=\"default\" r:id=\"rIdheader4\"/>");
        // (Objects the text goes round: one of each kind among the notes, tables, and sections, so that the text goes round them across pages and columns.)
        QByteArray body = plainP(wordsOf("before the tables", 14), refTo(2) + wallRun(0, 150, 4, 60, 50)) + t.styled + plainP(wordsOf("between them", 12), refTo(4) + wallRun(1, 40, 8, 50, 16)) + t.plain +
                          plainP("then one with space between its cells") + t.spaced + plainP("after the tables", QByteArray(), one);
        body += floatingTable(50) + plainP(wordsOf("after the table set at a place", 20)) + framedP("FRAME of the second section", "<w:framePr w:w=\"1400\" w:h=\"500\" w:hRule=\"exact\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"text\" w:xAlign=\"left\"/>") +
                plainP(wordsOf("Second section", 18), wallRun(3, 110, 10, 50, 40)) + tableOfRows(9, 2, 1000, [](int r, int c) { return r == 4 && c == 1 ? plainP("cites", refTo(5)) : cellP("r" + QByteArray::number(r) + "c" + QByteArray::number(c)); }) +
                plainP(wordsOf("end of the second", 30), wallRun(2, 60, 2, 40, 30), two);
        body += plainP(wordsOf("Third section", 40), refTo(6) + wallRun(4, 160, 6, 70, 40) + wallRun(5, 200, 120, 60, 60)) + "<w:p><w:r><w:t>before a break</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after it</w:t></w:r></w:p>" + plainP(wordsOf("Last", 60), wallRun(0, 20, 30, 50, 24));
        return makeDocxWithParts(body, three, parts, "<w:evenAndOddHeaders/>", t.styles);
    }

    // What a paginator that was told of the edits has that a new one on the same story does not (empty when there is nothing): the page count, and for each page its start,
    // its lines, the tables on it with the heights of their rows, its notes, its header and footer, and its boxes (tests/flow/pageaudit.h).
    static QString layoutDiffersFromFresh(WriteDoc &doc, bool made = true)
    {
        flow::Paginator fresh(&doc.flow());
        return audit::differenceFromFresh(doc.pager(), fresh, made);
    }

    // The story's paragraphs with their places and cells, and the pages' starts of the paginator and of a new one (for JW_FUZZ_DUMP).
    static QString layoutDump(WriteDoc &doc)
    {
        QString out;
        for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
            const flow::CellRecord *rec = doc.flow().tables.get(cellKeyOf(b));
            QString shown = b.text().left(24);
            shown.replace(QChar(flow::kPageBreak), QStringLiteral("<PB>"));
            if (b.blockFormat().pageBreakPolicy() != QTextFormat::PageBreak_Auto) shown += QStringLiteral(" {policy %1}").arg(int(b.blockFormat().pageBreakPolicy()));
            if (b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) shown += QStringLiteral(" {SECT}");
            out += QStringLiteral("  @%1 [%2] key %3 row %4%5\n").arg(b.position()).arg(shown).arg(cellKeyOf(b)).arg(rec ? reinterpret_cast<quintptr>(rec->row.get()) % 100000 : 0)
                       .arg(rec ? QStringLiteral(" span %1 merge %2 gap %3/%4 parent %5").arg(rec->cell.span).arg(rec->cell.vmerge == flow::CellData::VMerge::Restart ? QLatin1String("restart") : rec->cell.vmerge == flow::CellData::VMerge::Continue ? QLatin1String("continue") : QLatin1String("-")).arg(rec->row->before).arg(rec->row->after).arg(rec->parent) : QString());
        }
        flow::Paginator fresh(&doc.flow());
        flow::Paginator &pg = doc.pager();
        out += QStringLiteral("pages (the paginator / a new one):");
        for (int k = 0; k < std::max(pg.pageCount(), fresh.pageCount()); ++k) {
            const auto a = k < pg.pageCount() ? pg.page(k).start : flow::Paginator::PageStart(), b = k < fresh.pageCount() ? fresh.page(k).start : flow::Paginator::PageStart();
            out += QStringLiteral(" %1:(%2,%3,%4)/(%5,%6,%7)").arg(k).arg(a.pos).arg(a.skip).arg(a.piece).arg(b.pos).arg(b.skip).arg(b.piece);
        }
        return out;
    }

    // The story as the commands leave it: each paragraph's text, its cell's record, and its style (undo gives it back as it was).
    static QString storyState(WriteDoc &doc)
    {
        QString out;
        for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
            out += QStringLiteral("[%1|%2|%3|%4]").arg(b.text()).arg(cellKeyOf(b)).arg(b.blockFormat().stringProperty(tp::StyleName)).arg(int(b.blockFormat().alignment()));
        return out;
    }

    // Hundreds of random commands on a document of tables (every one that works on a table, the ones that delete across cells, paste, and type in cells that are joined or in
    // tables in cells): after each, the file it makes is well-formed and has tables of the shape tables have, the paginator that was told of the edits has the pages and the
    // rows' heights a new one finds, and what was done undoes and does again. JW_FUZZ_SEEDS says how many seeds, JW_FUZZ_FROM where to begin, JW_FUZZ_STEPS how many commands;
    // JW_FUZZ_COUNTS=1 says what the seeds found, JW_FUZZ_TRACE=1 what each did (JW_FUZZ_PAGES=1: and how many pages the file had after each). A second, shorter run is of a file
    // with the notes, headers, footers, and sections as well (richMixedDocx), whose commands also type where notes are cited, put in section breaks, and change the page setup
    // and the headers of a section; there the pages (their lines, table rows, notes, headers and footers, and boxes) are compared, and the file is not written (JP_FUZZ=n: n seeds of it).
    void randomEditingOfTablesKeepsThemWholeAndTheLayoutRight()
    {
        QMap<QString, int> failed;
        QMap<QString, QString> example;
        QSet<QString> failing;
        int seedsRun = 0;
        // (Two kinds of file: the tables alone, and then the tables with notes, headers, footers, and sections: a short run of those, whose edits also type where notes are
        // cited, put section breaks in, and change what the headers and footers are. JP_FUZZ=n: seeds 1 to n of the second, for a longer run.)
        for (const bool mixed : {false, true}) {
            const QByteArray original = mixed ? richMixedDocx() : richTableDocx();
            const int seeds = mixed ? (qEnvironmentVariableIntValue("JP_FUZZ") > 0 ? qEnvironmentVariableIntValue("JP_FUZZ") : 6)
                                    : (qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 12);
            const int steps = qEnvironmentVariableIntValue("JW_FUZZ_STEPS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_STEPS") : (mixed ? 40 : 70);
            for (unsigned seed = std::max(1, qEnvironmentVariableIntValue("JW_FUZZ_FROM")); seed <= unsigned(seeds); ++seed) {
                auto doc = open(original);
                QVERIFY(doc);
                ++seedsRun;
                std::mt19937 rng(seed + (mixed ? 1000u : 0u));
                auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
                QTextCursor c(doc->story());
                QTextDocumentFragment held;
                QStringList log;
                QString lastDump;
                auto fail = [&](const QString &what, const QString &detail) {
                    failing.insert((mixed ? QStringLiteral("mixed ") : QString()) + QString::number(seed));
                    if (failed[what]++ == 0) example[what] = QStringLiteral("%1seed %2: %3 | last: %4").arg(mixed ? "mixed " : "").arg(seed).arg(detail.left(260), log.mid(std::max<qsizetype>(0, log.size() - 5)).join(QStringLiteral(" , ")));
                    if (qEnvironmentVariableIsSet("JW_FUZZ_DUMP")) qInfo("seed %d: %s: %s\nlast: %s\nBEFORE THE LAST COMMAND:\n%s\nAFTER:\n%s", int(seed), qPrintable(what), qPrintable(detail), qPrintable(log.mid(std::max<qsizetype>(0, log.size() - 5)).join(QStringLiteral(" , "))), qPrintable(lastDump), qPrintable(layoutDump(*doc)));
                };
                static const char *const names[] = {"type", "enter", "insertRow", "insertCol", "tab", "delRow", "delCol", "delTable", "removeSel", "bigSel", "backspace", "delete", "copy", "paste", "undo", "redo",
                                                    "style", "bold", "align", "pageBreak", "list", "typeInCell", "removeAcross", "shiftTab", "typeNoted", "sectionBreak", "pageSetup", "headerSwap"};
                if (const QString d = layoutDiffersFromFresh(*doc); !d.isEmpty()) {
                    fail(QStringLiteral("layout differs from a new paginator's as the file is opened"), d);
                    continue;
                }
                auto selectCells = [&] {   // (from a place in one cell to a place in another)
                    QVector<QTextBlock> cells;
                    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
                        if (cellKeyOf(b)) cells << b;
                    if (cells.isEmpty()) return false;
                    const QTextBlock a = cells[pick(int(cells.size()))], z = cells[pick(int(cells.size()))];
                    c.setPosition(a.position() + pick(a.length()));
                    c.setPosition(z.position() + pick(z.length()), QTextCursor::KeepAnchor);
                    return true;
                };
                for (int step = 0; step < steps; ++step) {
                    const int op = pick(mixed ? 28 : 24);
                    const QString before = storyState(*doc);
                    lastDump = qEnvironmentVariableIsSet("JW_FUZZ_DUMP") ? layoutDump(*doc) : QString();
                    auto place = [&] { c.setPosition(pick(doc->story()->characterCount() - 1)); };
                    auto selectSome = [&](int most) {
                        const int a = pick(doc->story()->characterCount() - 1), len = pick(most);
                        c.setPosition(a);
                        c.setPosition(std::min(a + len, doc->story()->characterCount() - 1), QTextCursor::KeepAnchor);
                    };
                    bool one = false;   // (a command that is one step to undo)
                    switch (op) {
                    case 0: place(); typeText(*doc, c, QStringLiteral("word ")); break;
                    case 1: place(); insertParagraph(*doc, c); break;
                    case 2: case 3: case 5: case 6:
                        if (pick(3) == 0 && selectCells()) {
                            if (op == 2) insertTableRow(*doc, c, pick(2));
                            else if (op == 3) insertTableColumn(*doc, c, pick(2));
                            else if (op == 5) deleteTableRows(*doc, c);
                            else deleteTableColumns(*doc, c);
                        } else {
                            tableCommand(*doc, c, op == 2 ? 0 : op == 3 ? 1 : op == 5 ? 3 : 4, pick);
                        }
                        one = true;
                        break;
                    case 4: tableCommand(*doc, c, 2, pick); break;
                    case 7: tableCommand(*doc, c, 5, pick); one = true; break;
                    case 8: selectSome(100); removeSelection(*doc, c); one = true; break;
                    case 9: selectSome(mixed ? 1500 : 100000); removeSelection(*doc, c); one = true; break;
                    case 10: place(); deleteBack(*doc, c, false); break;
                    case 11: place(); deleteForward(*doc, c, false); break;
                    case 12: selectSome(200); held = selectionFragment(c); break;
                    case 13: place(); pasteFragment(*doc, c, held); one = true; break;
                    case 14: doc->story()->undo(); break;
                    case 15: doc->story()->redo(); break;
                    case 16: selectSome(300); applyParagraphStyle(*doc, c, pick(2) ? QStringLiteral("Heading2") : QStringLiteral("Normal")); one = true; break;
                    case 17: selectSome(200); toggleBold(*doc, c); one = true; break;
                    case 18: selectSome(200); setAlignment(*doc, c, pick(2) ? Qt::AlignRight : Qt::AlignHCenter); one = true; break;
                    case 19: place(); insertPageBreak(*doc, c); break;
                    case 20: selectSome(150); toggleList(*doc, c, pick(2)); one = true; break;
                    case 21: if (placeInACell(*doc, c, pick)) typeText(*doc, c, QStringLiteral("typed in a cell with words")); break;
                    case 22: if (selectCells()) removeSelection(*doc, c); one = true; break;
                    case 23: if (placeInACell(*doc, c, pick)) insertTab(*doc, c, true); break;
                    case 24: {   // typing in a paragraph, or a cell, that cites a note
                        QVector<QTextBlock> noted;
                        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
                            for (auto it = b.begin(); !it.atEnd(); ++it)
                                if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("footnote:"))) {
                                    noted << b;
                                    break;
                                }
                        if (!noted.isEmpty()) {
                            const QTextBlock b = noted[pick(int(noted.size()))];
                            c.setPosition(b.position() + pick(b.length()));
                            typeText(*doc, c, QStringLiteral("noted words "));
                        }
                        break;
                    }
                    case 25: place(); insertSectionBreak(*doc, c); break;
                    case 26: {   // a section's page setup: whether its first page has a header of its own, how far the header and the footer are from the edge, the margins
                        const int index = pick(doc->sectionCount());
                        wml::SectionProps props = doc->sectionProps(index);
                        switch (pick(4)) {
                        case 0: props.titlePg = !props.titlePg; break;
                        case 1: props.header = 360 + 360 * pick(5); break;
                        case 2: props.top = 720 + 360 * pick(4); break;
                        default: props.footer = 360 + 360 * pick(5); props.bottom = 720 + 360 * pick(3); break;
                        }
                        doc->setSectionProps(index, props);
                        break;
                    }
                    default: {   // the headers a section names swapped for other parts (the odd pages' for the even pages', the third section's for the first page's)
                        const int index = pick(doc->sectionCount());
                        wml::SectionProps props = doc->sectionProps(index);
                        bool swapped = false;
                        for (wml::RawChild &child : props.extra) {
                            if (!child.xml.contains("headerReference")) continue;
                            for (const auto &pair : {std::make_pair("rIdheader1", "rIdheader3"), std::make_pair("rIdheader3", "rIdheader1"), std::make_pair("rIdheader4", "rIdheader2"), std::make_pair("rIdheader2", "rIdheader4")})
                                if (!swapped && child.xml.contains(pair.first)) {
                                    child.xml.replace(pair.first, pair.second);
                                    swapped = true;
                                }
                        }
                        if (swapped) doc->setSectionProps(index, props);
                        break;
                    }
                    }
                    log << QStringLiteral("%1@%2").arg(QLatin1String(names[op])).arg(c.position());
                if (qEnvironmentVariableIsSet("JW_FUZZ_PAGES")) log.last() += QStringLiteral("(%1)").arg(doc->pager().pageCount());
                    const QString what = QLatin1String(names[op]);
                    // The paginator that was told of the edits has the pages and the rows' heights that a new one finds.
                    if (const QString d = layoutDiffersFromFresh(*doc); !d.isEmpty()) {
                        fail(QStringLiteral("layout differs from a new paginator's after ") + what, d);
                        break;
                    }
                    // The text goes round the objects, on the pages that have them, as the page layer draws it (every third command: the pages are made for it).
                    if (step % 3 == 2)
                        if (const QString d = audit::wallProblems(doc->pager(), doc->story()); !d.isEmpty()) {
                            fail(QStringLiteral("the text does not go round the objects after ") + what, d);
                            break;
                        }
                    // The text is on the pages whole, and every frame and table set at a place is drawn once or in the text (every third command: the pages are made for it).
                    if (const QString d = audit::coverageProblems(doc->pager(), doc->story()); !d.isEmpty()) {
                        fail(QStringLiteral("text is lost from the pages after ") + what, d);
                        break;
                    }
                    if (step % 3 == 1)
                        if (const QString d = audit::floatProblems(doc->pager(), doc->flow()); !d.isEmpty()) {
                            fail(QStringLiteral("a frame or a table set at a place is lost after ") + what, d);
                            break;
                        }
                    if (!mixed) {
                        QString err;
                        const QByteArray zip = doc->toDocx(&err);
                        if (zip.isEmpty()) { fail(QStringLiteral("not written"), err); break; }
                        if (const QStringList strict = rt::strictProblems(zip, original); !strict.isEmpty()) { fail(QStringLiteral("saved file after %1: ").arg(what) + strict.first().left(120), strict.join(QLatin1Char(';'))); break; }
                    }
                    if (step % 4 != 3) continue;
                    if (!mixed) {
                        QString err;
                        const QByteArray zip = doc->toDocx(&err);
                        auto back = WriteDoc::fromDocx(zip, &err);
                        if (!back) { fail(QStringLiteral("does not open again"), err); break; }
                        if (documentXml(back->toDocx()) != documentXml(zip)) fail(QStringLiteral("the second save differs"), QString());
                        QString a = plain(*doc), b = plain(*back);
                        a.remove(QChar::ObjectReplacementCharacter);
                        b.remove(QChar::ObjectReplacementCharacter);
                        if (a != b) fail(QStringLiteral("what opens again has other text"), QString());
                    }
                    // What was done undoes, and does again, and the pages follow both.
                    if (one && op != 14 && op != 15) {
                        const QString after = storyState(*doc);
                        if (after != before) {
                            int undone = 0;
                            while (undone < 8 && doc->story()->isUndoAvailable() && storyState(*doc) != before) {
                                doc->story()->undo();
                                ++undone;
                            }
                            if (storyState(*doc) != before) {
                                fail(QStringLiteral("undo of %1 does not give the story back").arg(what), QString());
                                break;
                            }
                            if (const QString d = layoutDiffersFromFresh(*doc); !d.isEmpty()) {
                                fail(QStringLiteral("layout differs from a new paginator's after undoing ") + what, d);
                                break;
                            }
                            for (int i = 0; i < undone; ++i) doc->story()->redo();
                            if (storyState(*doc) != after) {
                                fail(QStringLiteral("redo of %1 does not do it again").arg(what), QString());
                                break;
                            }
                            if (const QString d = layoutDiffersFromFresh(*doc); !d.isEmpty()) {
                                fail(QStringLiteral("layout differs from a new paginator's after redoing ") + what, d);
                                break;
                            }
                        }
                    }
                }
        
                if (qEnvironmentVariableIsSet("JW_FUZZ_TRACE")) qInfo("%sseed %u: %d pages; %s", mixed ? "mixed " : "", seed, doc->pager().pageCount(), qPrintable(log.join(QLatin1Char(' '))));
            }
        }
        QStringList report;
        for (auto it = failed.cbegin(); it != failed.cend(); ++it) report << QStringLiteral("%1: %2 times\n  %3").arg(it.key()).arg(it.value()).arg(example.value(it.key()));
        if (qEnvironmentVariableIsSet("JW_FUZZ_COUNTS")) qInfo("table fuzz: %d of %d seeds failed:\n%s", int(failing.size()), seedsRun, qPrintable(report.join(QLatin1Char('\n'))));
        QVERIFY2(failed.isEmpty(), qPrintable(QStringLiteral("%1 of %2 seeds failed:\n%3").arg(failing.size()).arg(seedsRun).arg(report.join(QLatin1Char('\n')))));
    }

    // The documents of the random commands have objects that the text goes round, on their pages (else the commands would not try the walls).
    void theDocumentsOfTheRandomCommandsHaveObjectsTheTextGoesRound()
    {
        for (const bool mixed : {false, true}) {
            auto doc = open(mixed ? richMixedDocx() : richTableDocx());
            QVERIFY(doc);
            int walls = 0, pagesWithWalls = 0;
            for (int k = 0; k < doc->pager().pageCount(); ++k) {
                const auto page = doc->pager().materialize(k);
                QVERIFY(page);
                const int here = int(audit::wallsOf(*page).size());
                walls += here;
                pagesWithWalls += here > 0;
            }
            QVERIFY2(walls >= 4 && pagesWithWalls >= 2, qPrintable(QStringLiteral("%1 walls on %2 pages").arg(walls).arg(pagesWithWalls)));
            QVERIFY(doc->flow().frames.size() >= 1);
            int floating = 0;
            for (int k = 0; k < doc->pager().pageCount(); ++k) {
                const auto page = doc->pager().materialize(k);   // (kept: the tables are the page's, which a temporary would take with it before the loop)
                QVERIFY(page);
                for (const auto &t : page->tables) floating += t->floating;
            }
            QVERIFY2(floating >= 1, "a table set at a place is drawn on some page");
            QVERIFY2(audit::wallProblems(doc->pager(), doc->story()).isEmpty(), qPrintable(audit::wallProblems(doc->pager(), doc->story())));
        }
    }

    // A shape that says it has no fill or no line has none (a shape's own fill is a color it was not given); one that says nothing is filled white, with a line, as the file's form has it.
    void aShapeWithNoFillOrLineHasNone()
    {
        const QByteArray vml = "xmlns:v=\"urn:schemas-microsoft-com:vml\"";
        auto objectOf = [&](const QByteArray &attributes) {
            const QByteArray body = "<w:p><w:r><w:t>text</w:t></w:r><w:r><w:pict " + vml + "><v:rect id=\"r\" style=\"position:absolute;margin-left:10pt;margin-top:5pt;width:50pt;height:20pt\" " + attributes + "/></w:pict></w:r></w:p>";
            auto doc = open(makeDocx(body, kStyles));
            return doc && doc->flow().objects.size() == 1 ? doc->flow().objects[0].item : ItemPtr();
        };
        const ItemPtr none = objectOf("filled=\"f\" stroked=\"f\""), plain = objectOf(""), red = objectOf("fillcolor=\"red\" strokecolor=\"blue\" strokeweight=\"2pt\"");
        QVERIFY(none && plain && red);
        QVERIFY(none->fill.isNone() && none->stroke.isNone());
        QVERIFY(!plain->fill.isNone() && !plain->stroke.isNone());
        QVERIFY(!red->fill.isNone() && !red->stroke.isNone() && red->stroke.width == 2.0);
    }

    // ---- floating tables

    // A table that says it is set at a place (w:tblpPr) has its place: from the text, the margin, or the page, by an offset or at the start, middle, or end, and the room the text
    // keeps from it; a table that says nothing of the kind has none; and the file comes back as it was.
    void aTableSetAtAPlaceHasItsPlace()
    {
        const QByteArray pr = "<w:tblpPr w:leftFromText=\"187\" w:rightFromText=\"200\" w:topFromText=\"40\" w:bottomFromText=\"60\" w:vertAnchor=\"page\" w:horzAnchor=\"margin\" w:tblpXSpec=\"right\" w:tblpY=\"1440\"/>"
                              "<w:tblOverlap w:val=\"never\"/><w:tblW w:w=\"0\" w:type=\"auto\"/>";
        const QByteArray floating = tableX(rowX(cellX(cellP("a")) + cellX(cellP("b"))), 2, 1600, pr), inText = tableX(rowX(cellX(cellP("c"))), 1, 1600);
        const QByteArray zip = makeDocx(plainP("Before") + floating + plainP("After") + inText + plainP("End"), kStyles);
        auto doc = open(zip);
        QVERIFY(doc);
        QVector<std::optional<flow::TableData::Place>> places;
        int lastKey = 0;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
            const int key = b.blockFormat().intProperty(flow::fp::Cell);
            if (key && key != lastKey) {
                const flow::CellRecord *rec = doc->flow().tables.get(key);
                if (places.isEmpty() || (places.size() == 1 && rec->table.get() != doc->flow().tables.get(lastKey)->table.get())) places << rec->table->place;
            }
            lastKey = key ? key : lastKey;
        }
        QCOMPARE(int(places.size()), 2);
        QVERIFY(places[0].has_value() && !places[1].has_value());
        const flow::TableData::Place &p = *places[0];
        using Place = flow::TableData::Place;
        QVERIFY(p.across == Place::From::Margin && p.down == Place::From::Page);
        QVERIFY(p.acrossSpec == Place::Spec::End && p.downSpec == Place::Spec::None);
        QCOMPARE(p.y, 72.0);
        QCOMPARE(p.x, 0.0);
        QCOMPARE(p.room.left(), 9.35);
        QCOMPARE(p.room.right(), 10.0);
        QCOMPARE(p.room.top(), 2.0);
        QCOMPARE(p.room.bottom(), 3.0);
        const QByteArray saved = doc->toDocx();
        QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
        QVERIFY(documentXml(saved).contains("w:tblpPr"));
        auto back = open(saved);
        QVERIFY(back);
        QCOMPARE(documentXml(back->toDocx()), documentXml(saved));
    }

    // ---- frames

    static QByteArray framedP(const QByteArray &text, const QByteArray &framePr)
    {
        return "<w:p><w:pPr>" + framePr + "</w:pPr><w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r></w:p>";
    }

    // The paragraphs of a frame in the body (w:framePr) are one frame, set apart from the text: no lines and no room there, and a text box where the frame asks, at the paragraph
    // after them, with their text; and the file comes back as it was.
    void framePrParagraphsAreAFrameSetWhereItAsks()
    {
        const QByteArray pr = "<w:framePr w:w=\"2400\" w:h=\"1000\" w:hRule=\"atLeast\" w:hSpace=\"180\" w:vSpace=\"60\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\" w:y=\"200\"/>";
        const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", pr) + framedP("FRAMEWORDS TWO", pr) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
        auto doc = open(makeDocx(body, kStyles));
        QVERIFY(doc);
        const flow::FlowDocument &fd = doc->flow();
        QCOMPARE(int(fd.frames.size()), 1);
        const flow::FloatingObject &o = fd.objects[fd.frames[0] - 1];
        QCOMPARE(o.frame, 1);
        QCOMPARE(o.frameWidth, 120.0);
        QCOMPARE(o.frameHeight, 50.0);
        QVERIFY(!o.frameExactHeight);
        QVERIFY(o.hRel == flow::FloatingObject::Rel::Margin && o.vRel == flow::FloatingObject::Rel::Paragraph);
        QVERIFY(o.hAlign == flow::FloatingObject::Align::End);
        QCOMPARE(o.dy, 10.0);
        QVERIFY(o.wrap == flow::FloatingObject::Wrap::Square);
        QCOMPARE(o.gap.left(), 9.0);
        QCOMPARE(o.gap.top(), 3.0);
        int inFrame = 0;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (b.blockFormat().intProperty(flow::fp::Frame) == 1) {
                ++inFrame;
                QVERIFY(b.blockFormat().boolProperty(tp::OutOfLayout));
            }
        QCOMPARE(inFrame, 2);
        // On the page: no lines of the frame's paragraphs, a text box at the right margin with their text, and the text of the paragraph after goes round it.
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        const TextItem *box = nullptr;
        for (const auto &it : page->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(it.get()))
                if (const Story *st = page->doc->story(t->storyId))
                    if (st->doc->toPlainText().contains(QStringLiteral("FRAMEWORDS ONE")) && st->doc->toPlainText().contains(QStringLiteral("FRAMEWORDS TWO")) && t->id.startsWith(QLatin1String("flow-object-"))) box = t;
        QVERIFY(box);
        const flow::Section &sec = fd.section(0);
        QVERIFY2(std::abs(box->rect.right() - (sec.pageSize().width() - sec.margins.right())) < 0.01 && std::abs(box->rect.width() - 120) < 0.01, qPrintable(audit::rectText(box->rect)));
        QVERIFY(box->rect.height() >= 50 - 0.01);
        QVERIFY2(audit::problems(doc->pager(), doc->story()).isEmpty(), qPrintable(audit::problems(doc->pager(), doc->story())));
        for (const auto &l : doc->pager().lines(0)) QVERIFY(doc->story()->findBlock(l.pos).blockFormat().intProperty(flow::fp::Frame) == 0);
        // The file comes back as it was: the frame's paragraphs, with their properties, where they were.
        const QByteArray saved = doc->toDocx();
        const QByteArray x = documentXml(saved);
        QVERIFY2(x.count("w:framePr") == 2 && x.contains("FRAMEWORDS ONE") && x.indexOf("FRAMEWORDS ONE") < x.indexOf("FRAMEWORDS TWO") && x.indexOf("FRAMEWORDS TWO") < x.indexOf("After the frame"), x.left(1500).constData());
        QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
        auto back = open(saved);
        QVERIFY(back);
        QCOMPARE(int(back->flow().frames.size()), 1);
        QCOMPARE(documentXml(back->toDocx()), x);
    }

    // What the properties of a frame say: its place (from the text, the margin, or the page, by an offset or at the start, middle, or end of it), its height (at least what is
    // given, exactly that, or the text's), the room round it, and how the text goes round it; a drop cap is a frame at the start of its paragraph.
    void aFramesPropertiesAreRead()
    {
        auto frameOf = [&](const QByteArray &pr) {
            auto doc = open(makeDocx(plainP("Before") + framedP("frame", "<w:framePr " + pr + "/>") + plainP("After"), kStyles));
            return doc && doc->flow().frames.size() == 1 ? doc->flow().objects[doc->flow().frames[0] - 1] : flow::FloatingObject();
        };
        using FO = flow::FloatingObject;
        FO o = frameOf("w:w=\"1000\" w:h=\"800\" w:hRule=\"exact\" w:hAnchor=\"page\" w:vAnchor=\"page\" w:x=\"1440\" w:y=\"-720\"");
        QCOMPARE(o.frameWidth, 50.0);
        QCOMPARE(o.frameHeight, 40.0);
        QVERIFY(o.frameExactHeight);
        QVERIFY(o.hRel == FO::Rel::Page && o.vRel == FO::Rel::Page);
        QCOMPARE(o.dx, 72.0);
        QCOMPARE(o.dy, -36.0);
        QVERIFY(o.hAlign == FO::Align::None && o.vAlign == FO::Align::None);
        o = frameOf("w:w=\"1000\" w:h=\"800\" w:hRule=\"auto\" w:hAnchor=\"margin\" w:vAnchor=\"margin\" w:xAlign=\"center\" w:yAlign=\"bottom\" w:wrap=\"notBeside\"");
        QCOMPARE(o.frameHeight, 0.0);
        QVERIFY(o.hRel == FO::Rel::Margin && o.vRel == FO::Rel::Margin && o.hAlign == FO::Align::Center && o.vAlign == FO::Align::End);
        QVERIFY(o.wrap == FO::Wrap::TopBottom);
        o = frameOf("w:wrap=\"none\" w:xAlign=\"inside\" w:yAlign=\"top\"");
        QVERIFY(o.wrap == FO::Wrap::None && o.hAlign == FO::Align::Start && o.vAlign == FO::Align::Start);
        QCOMPARE(o.frameWidth, 0.0);   // (as wide as its text)
        o = frameOf("w:wrap=\"tight\" w:hSpace=\"240\"");
        QVERIFY(o.wrap == FO::Wrap::Square);
        QCOMPARE(o.gap.right(), 12.0);
        o = frameOf("w:dropCap=\"drop\" w:lines=\"3\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"text\"");
        QVERIFY(o.hRel == FO::Rel::Column && o.vRel == FO::Rel::Paragraph && o.hAlign == FO::Align::Start && o.wrap == FO::Wrap::Square);
        QVERIFY(o.frameShrinks);   // (a drop cap is as wide as its letter; any other frame with no width is as wide as the column)
        QVERIFY(!frameOf("w:wrap=\"tight\" w:hSpace=\"240\"").frameShrinks);
        // Paragraphs with other properties are other frames; the same properties with a paragraph that is not in the frame between them are another frame.
        const QByteArray one = "<w:framePr w:w=\"1000\" w:hAnchor=\"page\" w:x=\"100\"/>", two = "<w:framePr w:w=\"1000\" w:hAnchor=\"page\" w:x=\"200\"/>";
        auto doc = open(makeDocx(plainP("Before") + framedP("a", one) + framedP("b", one) + framedP("c", two) + plainP("Between") + framedP("d", one) + plainP("After"), kStyles));
        QVERIFY(doc);
        QCOMPARE(int(doc->flow().frames.size()), 3);
        QVector<int> keys;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) keys << b.blockFormat().intProperty(flow::fp::Frame);
        QCOMPARE(keys, (QVector<int>{0, 1, 1, 2, 0, 3, 0}));
        // A paragraph with framePr in a cell of a table is no frame; nor is one in a header (which sets it as before).
        const QByteArray cell = cellX(framedP("in a cell", one));
        auto inCell = open(makeDocx(plainP("Before") + tableX(rowX(cell), 1) + plainP("After"), kStyles));
        QVERIFY(inCell);
        QCOMPARE(int(inCell->flow().frames.size()), 0);
    }

    // ---- editing next to frames

    // The paragraphs of a document in a line each: the start of its text, with F for a paragraph of a frame (and H when it is set out of the text).
    static QStringList kindsOf(WriteDoc &doc)
    {
        QStringList out;
        for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
            out << QStringLiteral("%1[%2%3]").arg(b.text().left(14)).arg(b.blockFormat().intProperty(flow::fp::Frame) ? QStringLiteral("F") : QString()).arg(b.blockFormat().boolProperty(tp::OutOfLayout) ? QStringLiteral("H") : QString());
        return out;
    }
    static QByteArray aFramePr() { return "<w:framePr w:w=\"2400\" w:hSpace=\"180\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\"/>"; }
    static QTextBlock blockNumber(WriteDoc &doc, int n) { return doc.story()->findBlockByNumber(n); }

    // A frame's text is set apart from the text round it, so no edit joins the two: Backspace at the start of the paragraph after a frame and Delete at the end of the one before
    // it do nothing (as at the edge of a cell), and so do they at the frame's own edges; inside the frame they join its paragraphs.
    void backspaceAndDeleteNeverJoinAFramesTextAndTheTextRoundIt()
    {
        const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", aFramePr()) + framedP("FRAMEWORDS TWO", aFramePr()) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
        auto fresh = [&]() { auto d = open(makeDocx(body, kStyles)); return d; };
        {
            auto doc = fresh();
            QVERIFY(doc);
            const QStringList was = kindsOf(*doc);
            QCOMPARE(int(was.size()), 5);
            QTextCursor c(doc->story());
            c.setPosition(blockNumber(*doc, 3).position());   // the start of the paragraph after the frame
            deleteBack(*doc, c, false);
            QCOMPARE(kindsOf(*doc), was);
            const QByteArray x = documentXml(doc->toDocx());
            QCOMPARE(int(x.count("w:framePr")), 2);
        }
        {
            auto doc = fresh();
            const QStringList was = kindsOf(*doc);
            QTextCursor c(doc->story());
            const QTextBlock b = blockNumber(*doc, 0);
            c.setPosition(b.position() + b.length() - 1);   // the end of the paragraph before the frame
            deleteForward(*doc, c, false);
            QCOMPARE(kindsOf(*doc), was);
        }
        {
            auto doc = fresh();
            const QStringList was = kindsOf(*doc);
            QTextCursor c(doc->story());
            c.setPosition(blockNumber(*doc, 1).position());   // the start of the frame
            deleteBack(*doc, c, false);
            QCOMPARE(kindsOf(*doc), was);
            const QTextBlock last = blockNumber(*doc, 2);
            c.setPosition(last.position() + last.length() - 1);   // the end of the frame
            deleteForward(*doc, c, false);
            QCOMPARE(kindsOf(*doc), was);
        }
        {
            // Inside the frame: its two paragraphs become one, still the frame's.
            auto doc = fresh();
            QTextCursor c(doc->story());
            c.setPosition(blockNumber(*doc, 2).position());
            deleteBack(*doc, c, false);
            QCOMPARE(int(kindsOf(*doc).size()), 4);
            QCOMPARE(blockNumber(*doc, 1).text(), QStringLiteral("FRAMEWORDS ONEFRAMEWORDS TWO"));
            QVERIFY(blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame) > 0 && blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
            QVERIFY(blockNumber(*doc, 2).blockFormat().intProperty(flow::fp::Frame) == 0);
            QCOMPARE(int(documentXml(doc->toDocx()).count("w:framePr")), 1);
            QVERIFY2(audit::floatProblems(doc->pager(), doc->flow()).isEmpty(), qPrintable(audit::floatProblems(doc->pager(), doc->flow())));
        }
        {
            // A selection from the text into the frame cuts what it holds of each and joins neither to the other.
            auto doc = fresh();
            const QTextBlock b0 = blockNumber(*doc, 0);
            QTextCursor c(doc->story());
            c.setPosition(b0.position() + b0.length() - 1 - 5);
            c.setPosition(blockNumber(*doc, 1).position() + 3, QTextCursor::KeepAnchor);
            deleteBack(*doc, c, false);
            QCOMPARE(int(kindsOf(*doc).size()), 5);
            QVERIFY(blockNumber(*doc, 0).blockFormat().intProperty(flow::fp::Frame) == 0);
            QCOMPARE(blockNumber(*doc, 1).text(), QStringLiteral("MEWORDS ONE"));
            QVERIFY(blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
            QVERIFY(!blockNumber(*doc, 0).text().contains(QLatin1String("MEWORDS")));
        }
    }

    // A paragraph style gives a paragraph its look, not its place: a frame's paragraph that is made a heading is still the frame's (and the file still says so).
    void aStyleAppliedToAFramesParagraphLeavesItInTheFrame()
    {
        const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", aFramePr()) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
        auto doc = open(makeDocx(body, kStyles));
        QVERIFY(doc);
        const int key = blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame);
        QVERIFY(key > 0);
        QTextCursor c(doc->story());
        c.setPosition(blockNumber(*doc, 1).position() + 2);
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        const QTextBlockFormat bf = blockNumber(*doc, 1).blockFormat();
        QCOMPARE(bf.stringProperty(tp::StyleName), QStringLiteral("Heading1"));
        QCOMPARE(bf.intProperty(flow::fp::Frame), key);
        QVERIFY(bf.boolProperty(tp::OutOfLayout));
        // Still no line of the text, and a box with its words.
        for (int k = 0; k < doc->pager().pageCount(); ++k)
            for (const auto &l : doc->pager().lines(k)) QVERIFY(doc->story()->findBlock(l.pos).blockFormat().intProperty(flow::fp::Frame) == 0);
        QVERIFY2(audit::floatProblems(doc->pager(), doc->flow()).isEmpty(), qPrintable(audit::floatProblems(doc->pager(), doc->flow())));
        QVERIFY2(audit::coverageProblems(doc->pager(), doc->story()).isEmpty(), qPrintable(audit::coverageProblems(doc->pager(), doc->story())));
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("w:framePr")), 1);
        QVERIFY(x.indexOf("Heading1") > 0 && x.indexOf("Heading1") < x.indexOf("FRAMEWORDS ONE") && x.indexOf("w:framePr") < x.indexOf("FRAMEWORDS ONE"));
        // And so with a style that is put on the paragraph again, and one taken off.
        applyParagraphStyle(*doc, c, doc->defaultParagraphStyle());
        QCOMPARE(blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame), key);
        QVERIFY(blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
    }

    // Enter in a frame's paragraph makes another paragraph of the frame (even at its end, where the style says what follows it); Enter at the end of the paragraph before the
    // frame makes one of the text, and what is typed in it is text.
    void enterNextToAFrameMakesParagraphsOfTheRightPlace()
    {
        const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", aFramePr()) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
        {
            auto doc = open(makeDocx(body, kStyles));
            const int key = blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame);
            QTextCursor c(doc->story());
            const QTextBlock f = blockNumber(*doc, 1);
            c.setPosition(f.position() + f.length() - 1);
            insertParagraph(*doc, c);
            typeText(*doc, c, QStringLiteral("NEWFRAMEPARA"));
            QCOMPARE(int(kindsOf(*doc).size()), 5);
            QCOMPARE(blockNumber(*doc, 2).text(), QStringLiteral("NEWFRAMEPARA"));
            QCOMPARE(blockNumber(*doc, 2).blockFormat().intProperty(flow::fp::Frame), key);
            QVERIFY(blockNumber(*doc, 2).blockFormat().boolProperty(tp::OutOfLayout));
            QCOMPARE(int(documentXml(doc->toDocx()).count("w:framePr")), 2);
        }
        {
            // A heading in the frame, and Enter at its end: the next paragraph is of the heading's next style, and still the frame's.
            auto doc = open(makeDocx(body, kStyles));
            const int key = blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame);
            QTextCursor c(doc->story());
            c.setPosition(blockNumber(*doc, 1).position() + 2);
            applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
            const QTextBlock f = blockNumber(*doc, 1);
            c.setPosition(f.position() + f.length() - 1);
            insertParagraph(*doc, c);
            typeText(*doc, c, QStringLiteral("NEXTSTYLE"));
            QCOMPARE(blockNumber(*doc, 2).text(), QStringLiteral("NEXTSTYLE"));
            QCOMPARE(blockNumber(*doc, 2).blockFormat().intProperty(flow::fp::Frame), key);
            QVERIFY(blockNumber(*doc, 2).blockFormat().boolProperty(tp::OutOfLayout));
            QVERIFY(blockNumber(*doc, 2).blockFormat().stringProperty(tp::StyleName) != QStringLiteral("Heading1"));
        }
        {
            auto doc = open(makeDocx(body, kStyles));
            QTextCursor c(doc->story());
            const QTextBlock b = blockNumber(*doc, 0);
            c.setPosition(b.position() + b.length() - 1);
            insertParagraph(*doc, c);
            typeText(*doc, c, QStringLiteral("TYPEDINTEXT"));
            QCOMPARE(int(kindsOf(*doc).size()), 5);
            QCOMPARE(blockNumber(*doc, 1).text(), QStringLiteral("TYPEDINTEXT"));
            QVERIFY(blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame) == 0 && !blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
            QVERIFY(blockNumber(*doc, 2).blockFormat().boolProperty(tp::OutOfLayout));
            bool shown = false;
            for (const auto &l : doc->pager().lines(0)) shown |= doc->story()->findBlock(l.pos).text() == QStringLiteral("TYPEDINTEXT");
            QVERIFY(shown);
        }
    }

    // Text copied from a frame is paragraphs of the text where it is pasted (a frame's key is its document's, and the paragraphs would be hidden in another's); pasted into a
    // frame's paragraph it is the frame's.
    void pastedFrameTextIsTextOrIsThatOfTheFrameItIsPastedInto()
    {
        const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", aFramePr()) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
        auto a = open(makeDocx(body, kStyles));
        auto b = open(makeDocx(plainP(wordsOf("Other first", 20)) + plainP(wordsOf("Other second", 20)), kStyles));
        QVERIFY(a && b);
        QTextCursor sel(a->story());
        sel.setPosition(blockNumber(*a, 1).position());
        sel.setPosition(blockNumber(*a, 2).position() + 5, QTextCursor::KeepAnchor);
        const Clip clip = copySelection(*a, sel);
        QTextCursor at(b->story());
        at.setPosition(blockNumber(*b, 1).position() + 3);
        pasteClip(*b, at, clip);
        bool found = false;
        for (QTextBlock x = b->story()->begin(); x.isValid(); x = x.next()) {
            QVERIFY2(x.blockFormat().intProperty(flow::fp::Frame) == 0 && !x.blockFormat().boolProperty(tp::OutOfLayout), qPrintable(x.text().left(30)));
            found |= x.text().contains(QLatin1String("FRAMEWORDS"));
        }
        QVERIFY(found);
        QVERIFY2(audit::coverageProblems(b->pager(), b->story()).isEmpty(), qPrintable(audit::coverageProblems(b->pager(), b->story())));
        QCOMPARE(int(documentXml(b->toDocx()).count("w:framePr")), 0);
        // Into the frame's own paragraph: the frame's.
        QTextCursor inFrame(a->story());
        inFrame.setPosition(blockNumber(*a, 1).position() + 4);
        const int key = blockNumber(*a, 1).blockFormat().intProperty(flow::fp::Frame);
        QTextCursor from(b->story());
        from.setPosition(blockNumber(*b, 0).position());
        from.setPosition(blockNumber(*b, 2).position() + 4, QTextCursor::KeepAnchor);
        pasteClip(*a, inFrame, copySelection(*b, from));
        int framed = 0, others = 0;
        for (QTextBlock x = a->story()->begin(); x.isValid(); x = x.next()) {
            if (x.blockFormat().intProperty(flow::fp::Frame) == key && x.blockFormat().boolProperty(tp::OutOfLayout)) ++framed;
            else if (x.blockFormat().intProperty(flow::fp::Frame) != 0 || x.blockFormat().boolProperty(tp::OutOfLayout)) ++others;
        }
        QVERIFY2(framed >= 2 && others == 0, qPrintable(QStringLiteral("%1 framed, %2 other").arg(framed).arg(others)));
        QVERIFY2(audit::floatProblems(a->pager(), a->flow()).isEmpty(), qPrintable(audit::floatProblems(a->pager(), a->flow())));
    }

    // The caret is in a frame's text where the frame's box draws it, on the page that holds the frame (the page of the paragraph the frame is set from: here the second, with the
    // frame's paragraph before the break), a click in the box puts the caret in the frame's text, and Home, End, and Up and Down go by the box's lines.
    void theCaretAndTheMouseWorkInAFramesText()
    {
        const QByteArray body = plainP("First page text") + framedP("FRAMEWORDS ONE and more words to make a second line in the box of the frame", aFramePr()) + plainP(wordsOf("Second page", 30), QByteArray(), "<w:pageBreakBefore/>") + plainP("Tail");
        auto doc = open(makeDocx(body, kStyles));
        QVERIFY(doc);
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        view.paginateAll();
        QCOMPARE(view.pageCount(), 2);
        const QTextBlock f = blockNumber(*doc, 1);
        QVERIFY(f.blockFormat().boolProperty(tp::OutOfLayout));
        QCOMPARE(doc->pager().pageOfPosition(f.position() + 3), 1);
        const auto page = doc->pager().materialize(1);
        QVERIFY(page);
        const TextItem *box = nullptr;
        for (const auto &it : page->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(it.get()))
                if (const Story *st = page->doc->story(t->storyId))
                    if (st->doc->toPlainText().contains(QStringLiteral("FRAMEWORDS")) && t->id.startsWith(QLatin1String("flow-object-"))) box = t;
        QVERIFY(box);
        auto caretAt = [&](int offset) {
            view.cursor().setPosition(f.position() + offset);
            view.setCursor(view.cursor());
            QRect r;
            return view.caretRect(&r) ? r : QRect();
        };
        const QRect a = caretAt(3);
        QVERIFY(a.isValid());
        QCOMPARE(view.currentPage(), 1);
        const QRect pr = view.pageRect(1);   // (where the view has scrolled to)
        const double sc = pr.width() / page->size.width();
        const QRectF boxDev(pr.left() + box->rect.left() * sc, pr.top() + box->rect.top() * sc, box->rect.width() * sc, box->rect.height() * sc);
        QVERIFY2(boxDev.adjusted(-3, -3, 3, 3).contains(a.center()), qPrintable(QStringLiteral("caret %1,%2 and the box %3,%4 %5x%6").arg(a.x()).arg(a.y()).arg(boxDev.x()).arg(boxDev.y()).arg(boxDev.width()).arg(boxDev.height())));
        const QRect b = caretAt(8);
        QVERIFY(b.isValid() && b.x() > a.x() + 10 && b.y() == a.y());
        // Home and End by the box's first line; Down goes to its second.
        view.cursor().setPosition(f.position() + 8);
        view.setCursor(view.cursor());
        view.typeKey(Qt::Key_Home, Qt::NoModifier);
        QCOMPARE(view.cursor().position(), f.position());
        view.typeKey(Qt::Key_End, Qt::NoModifier);
        QVERIFY2(view.cursor().position() > f.position() + 10 && view.cursor().position() < f.position() + f.length() - 1, qPrintable(QString::number(view.cursor().position() - f.position())));
        view.cursor().setPosition(f.position() + 8);
        view.setCursor(view.cursor());
        view.typeKey(Qt::Key_Down, Qt::NoModifier);
        QVERIFY2(view.cursor().position() > f.position() + 17 && view.cursor().position() < f.position() + f.length(), qPrintable(QString::number(view.cursor().position() - f.position())));
        // A click in the box's first line: the frame's text, near the place.
        const QRect again = caretAt(8);   // (the view may have scrolled)
        view.cursor().setPosition(f.position());   // (on the same page: the view stays)
        view.setCursor(view.cursor());
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(again.x() + 1, again.y() + again.height() / 2));
        QVERIFY2(std::abs(view.cursor().position() - (f.position() + 8)) <= 1, qPrintable(QString::number(view.cursor().position() - f.position())));
        // Typing goes in the frame; the first page's text and the second page's are as they were.
        typeText(*doc, view.cursor(), QStringLiteral("TYPED"));
        QVERIFY(f.text().contains(QLatin1String("TYPED")) || blockNumber(*doc, 1).text().contains(QLatin1String("TYPED")));
        QVERIFY(blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
        QVERIFY(!blockNumber(*doc, 2).text().contains(QLatin1String("TYPED")));
    }

    // ---- tables with the notes, headers and footers, and sections of a file (the two waves of JeffWrite together)

    static QByteArray refTo(int id) { return "<w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"" + QByteArray::number(id) + "\"/></w:r>"; }
    static QByteArray plainP(const QByteArray &text, const QByteArray &after = QByteArray(), const QByteArray &pPr = QByteArray())
    {
        return "<w:p>" + (pPr.isEmpty() ? QByteArray() : "<w:pPr>" + pPr + "</w:pPr>") + "<w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r>" + after + "</w:p>";
    }
    static QByteArray wordsOf(const QByteArray &first, int count)
    {
        QByteArray out = first;
        for (int i = 0; i < count; ++i) out += " word" + QByteArray::number(i);
        return out;
    }
    // A footnotes part of the notes given (an Id and a text each).
    static QByteArray notesPart(const QList<std::pair<int, QByteArray>> &notes)
    {
        QByteArray out = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>";
        for (const auto &n : notes)
            out += "<w:footnote w:id=\"" + QByteArray::number(n.first) + "\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> " + n.second + "</w:t></w:r></w:p></w:footnote>";
        return out + "</w:footnotes>";
    }
    // A section's properties: a page of `w` by `h` twenty-ths of a point with `margin` on every side (header and footer 18 points from the edge), the references and break
    // type that come before the page's size, and the elements that come after its margins (columns, a first page of its own).
    static QByteArray sectionXml(int w, int h, int margin, const QByteArray &refs = QByteArray(), const QByteArray &type = QByteArray(), const QByteArray &after = QByteArray())
    {
        const QByteArray m = QByteArray::number(margin);
        return "<w:sectPr>" + refs + type + "<w:pgSz w:w=\"" + QByteArray::number(w) + "\" w:h=\"" + QByteArray::number(h) + "\"/><w:pgMar w:top=\"" + m + "\" w:right=\"" + m + "\" w:bottom=\"" + m + "\" w:left=\"" + m +
               "\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/>" + after + "</w:sectPr>";
    }
    static QByteArray tableOfRows(int rows, int columns, int width, const std::function<QByteArray(int, int)> &cell)
    {
        QByteArray out;
        for (int r = 0; r < rows; ++r) {
            QByteArray cells;
            for (int c = 0; c < columns; ++c) cells += cellX(cell(r, c), QByteArray(), width);
            out += rowX(cells);
        }
        return tableX(out, columns, width);
    }
    // How many rows the tables of the made pages show in all (a row shown in parts on two pages counts once).
    static int rowsShown(flow::Paginator &pg)
    {
        std::set<std::pair<const void *, int>> rows;
        for (int k = 0; k < pg.pageCount(); ++k)
            if (const auto m = pg.materialize(k))
                for (const auto &t : m->tables)
                    for (int r = t->firstRow; r < t->endRow; ++r) rows.insert({t->layout.get(), r});
        return int(rows.size());
    }

    // A note cited in the paragraph before a table, one cited in a cell of it, and one in the text after it, in a file (each alone, then the three together): each is on the
    // page whose lines cite it, below the rows, at the number its reference has.
    void aFilesNotesAroundATableAreOnThePagesThatCiteThem_data()
    {
        QTest::addColumn<bool>("before");
        QTest::addColumn<bool>("cell");
        QTest::addColumn<bool>("after");
        QTest::newRow("in the paragraph before the table") << true << false << false;
        QTest::newRow("in a cell") << false << true << false;
        QTest::newRow("in the text after the table") << false << false << true;
        QTest::newRow("all three") << true << true << true;
    }

    void aFilesNotesAroundATableAreOnThePagesThatCiteThem()
    {
        QFETCH(bool, before);
        QFETCH(bool, cell);
        QFETCH(bool, after);
        const QByteArray body = plainP(wordsOf("Opening", 40)) + plainP(wordsOf("Second", 30)) + plainP("The paragraph before the table", (before ? refTo(2) : QByteArray()) + "<w:r><w:t> and its end.</w:t></w:r>") +
                                tableOfRows(60, 2, 1500, [&](int r, int c) { return cell && c == 1 && r == 33 ? plainP("row 33 cites", refTo(3)) : cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) +
                                plainP("The paragraph after the table", (after ? refTo(4) : QByteArray()) + "<w:r><w:t> and its end.</w:t></w:r>") + plainP(wordsOf("Last", 20));
        const QByteArray notes = notesPart({{2, "ALPHA note of the paragraph before the table"}, {3, wordsOf("BETA", 25)}, {4, wordsOf("GAMMA", 8)}});
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        const int pages = pg.pageCount();
        QVERIFY(pages >= 4);
        QMap<QString, int> pageOf;
        for (int k = 0; k < pages; ++k)
            for (const QString &id : audit::citedNotes(pg, doc->story(), k)) pageOf[id] = k;
        QCOMPARE(int(pageOf.size()), int(before) + int(cell) + int(after));
        if (before && cell && after)
            QVERIFY2(pageOf["fn2"] < pageOf["fn3"] && pageOf["fn3"] < pageOf["fn4"], qPrintable(QStringLiteral("the notes are on the pages %1 %2 %3").arg(pageOf["fn2"]).arg(pageOf["fn3"]).arg(pageOf["fn4"])));
        const QString bad = audit::problems(pg, doc->story(), {{"fn2", "ALPHA"}, {"fn3", "BETA"}, {"fn4", "GAMMA"}});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        QCOMPARE(rowsShown(pg), 60);
    }

    // The first page, the even pages, and the odd ones have headers and footers of other heights, over a table that runs across many pages: its rows stay in the text area
    // that they leave on each, none over the footer, and a page holds as many rows as its own area has room for.
    void aFilesTableRowsStayInTheBodyThatItsHeadersAndFootersLeave()
    {
        const QByteArray refs = "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:headerReference w:type=\"first\" r:id=\"rIdheader2\"/><w:headerReference w:type=\"even\" r:id=\"rIdheader3\"/>"
                                "<w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:footerReference w:type=\"first\" r:id=\"rIdfooter2\"/><w:footerReference w:type=\"even\" r:id=\"rIdfooter3\"/>";
        auto lines = [](const char *name, int n) {
            QList<QByteArray> out;
            for (int i = 1; i <= n; ++i) out << "<w:r><w:t>" + QByteArray(name) + " " + QByteArray::number(i) + "</w:t></w:r>";
            return out;
        };
        const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", lines("ODD HEAD", 2))}, {QStringLiteral("/word/header2.xml"), headerPart("hdr", lines("FIRST HEAD", 1))},
                                              {QStringLiteral("/word/header3.xml"), headerPart("hdr", lines("EVEN HEAD", 5))}, {QStringLiteral("/word/footer1.xml"), headerPart("ftr", lines("ODD FOOT", 1))},
                                              {QStringLiteral("/word/footer2.xml"), headerPart("ftr", lines("FIRST FOOT", 3))}, {QStringLiteral("/word/footer3.xml"), headerPart("ftr", lines("EVEN FOOT", 4))}};
        const QByteArray body = plainP("Before the table") + tableOfRows(90, 2, 1500, [](int r, int c) { return r % 7 == 3 ? cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)) + cellP("a second line") : cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) +
                                plainP("After the table");
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, refs, QByteArray(), "<w:titlePg/>"), parts, "<w:evenAndOddHeaders/>"));
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        const int pages = pg.pageCount();
        QVERIFY(pages >= 6);
        QSet<int> heights;
        for (int k = 0; k < pages; ++k) heights << int(std::lround(pg.page(k).content.height() * 10));
        QVERIFY2(heights.size() >= 3, "the pages' text areas are of three heights (the first page's, the odd pages', the even pages')");
        const QString bad = audit::problems(pg, doc->story());
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            const QString want = k == 0 ? QStringLiteral("FIRST") : (k + 1) % 2 == 0 ? QStringLiteral("EVEN") : QStringLiteral("ODD");
            const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box")), foots = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
            QVERIFY2(heads.size() == 1 && foots.size() == 1, qPrintable(QStringLiteral("page %1 has %2 headers and %3 footers").arg(k).arg(heads.size()).arg(foots.size())));
            QVERIFY2(audit::storyOf(*page, heads[0]).startsWith(want + QStringLiteral(" HEAD")), qPrintable(QStringLiteral("page %1: %2").arg(k).arg(audit::storyOf(*page, heads[0]))));
            QVERIFY2(audit::storyOf(*page, foots[0]).startsWith(want + QStringLiteral(" FOOT")), qPrintable(QStringLiteral("page %1: %2").arg(k).arg(audit::storyOf(*page, foots[0]))));
            // The rows end above the footer, and the page is as full as its room lets it be.
            if (!page->tables.isEmpty()) {
                const auto &last = page->tables.last();
                QVERIFY2(last->rect().bottom() <= foots[0]->rect.top() + 0.05, qPrintable(QStringLiteral("page %1: rows end at %2, the footer begins at %3").arg(k).arg(last->rect().bottom()).arg(foots[0]->rect.top())));
                const auto next = k + 1 < pages ? pg.materialize(k + 1) : nullptr;
                if (next && !next->tables.isEmpty() && next->tables.first()->firstRow == last->endRow)
                    QVERIFY2(last->rect().bottom() + next->tables.first()->heightOf(last->endRow) > pg.page(k).content.bottom() + 0.05, qPrintable(QStringLiteral("page %1 leaves room for the next row").arg(k)));
            }
        }
        QCOMPARE(rowsShown(pg), 90);
    }

    // A note cited in the section before a continuous section of two columns, on the page where that begins: the notes are at the foot of the page, under the columns, and the
    // text of the first section is in the width of the page (the columns begin below it); the audit finds no overlap, nothing lost, and no empty page.
    void aFilesNoteCitedBeforeAContinuousTwoColumnSectionIsAtThePageFoot()
    {
        int checked = 0;
        for (const int noteWords : {5, 40, 120})
            for (const int cited : {0, 1}) {
                const QByteArray one = sectionXml(6000, 8000, 720);
                const QByteArray two = sectionXml(6000, 8000, 720, QByteArray(), "<w:type w:val=\"continuous\"/>", "<w:cols w:num=\"2\" w:space=\"360\"/>");
                QByteArray body = plainP(wordsOf("One", 12), cited == 0 ? refTo(2) : QByteArray()) + plainP(wordsOf("Second", 8), cited == 1 ? refTo(2) : QByteArray());
                body += plainP("end of the first section", QByteArray(), one);
                for (int i = 0; i < 14; ++i) body += plainP(wordsOf("Column", 14));
                auto doc = open(makeDocxWithParts(body, two, {{QStringLiteral("/word/footnotes.xml"), notesPart({{2, wordsOf("COLNOTE", noteWords)}})}}));
                QVERIFY(doc);
                flow::Paginator &pg = doc->pager();
                QCOMPARE(doc->sectionCount(), 2);
                const QString bad = audit::problems(pg, doc->story(), {{"fn2", "COLNOTE"}});
                QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("a note of %1 words, cited in paragraph %2: %3").arg(noteWords).arg(cited).arg(bad)));
                // The first section's lines are as wide as the page's text area, not as one of the columns.
                const double width = pg.page(0).content.width();
                for (const auto &l : pg.lines(0))
                    if (l.pos < doc->story()->findBlock(0).length() + doc->story()->findBlockByNumber(1).length())
                        QVERIFY2(l.rect.width() > width - 1, qPrintable(QStringLiteral("a line of the first section is %1 wide in a text area %2 wide").arg(l.rect.width()).arg(width)));
                QVERIFY2(audit::lostNotes(pg, audit::fullNoteHeight(doc->flow().fields->storyDoc(QStringLiteral("fn2")), width, doc->flow().env)).isEmpty(), "the note is drawn whole");
                ++checked;
            }
        QCOMPARE(checked, 6);
    }

    // A continuous section of two columns with a table in it: it begins below the text of the section before, the table is inside a column and goes on in the next, and the
    // note cited above it, in the first column, and the header and footer keep to their places.
    void aFilesTableInAContinuousTwoColumnSectionIsInsideItsColumn()
    {
        const QByteArray refs = "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>";
        const QByteArray one = sectionXml(6000, 8000, 720, refs);
        const QByteArray two = sectionXml(6000, 8000, 720, QByteArray(), "<w:type w:val=\"continuous\"/>", "<w:cols w:num=\"2\" w:space=\"360\"/>");
        const QByteArray body = plainP(wordsOf("One", 20)) + plainP("end of the first section", QByteArray(), one) + plainP(wordsOf("Two", 5)) + plainP("The paragraph that cites a note", refTo(2)) +
                                tableOfRows(70, 2, 900, [](int r, int c) { return cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) + plainP("After the table");
        auto doc = open(makeDocxWithParts(body, two, {{QStringLiteral("/word/footnotes.xml"), notesPart({{2, wordsOf("COLNOTE", 12)}})}, {QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEADER</w:t></w:r>"})},
                                                       {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:t>FOOTER A</w:t></w:r>", "<w:r><w:t>FOOTER B</w:t></w:r>"})}}));
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        const int pages = pg.pageCount();
        QVERIFY(pages >= 2);
        QCOMPARE(doc->sectionCount(), 2);
        const flow::Section &sec = doc->flow().section(1);
        QCOMPARE(sec.columnCount(), 2);
        double split = 0;
        for (const auto &l : pg.lines(0))
            if (l.frame == 0) split = std::max(split, l.rect.bottom());
        QVERIFY(split > 0);
        const double top = pg.page(0).content.top();
        int fragments = 0;
        for (int k = 0; k < pages; ++k) {
            const auto m = pg.materialize(k);
            QVERIFY(m);
            for (const auto &t : m->tables) {
                const QRectF r = t->rect();
                bool inAColumn = false;
                for (int col = 0; col < sec.columnCount(); ++col) {
                    const QRectF c = sec.columnRect(col);
                    inAColumn |= r.left() >= c.left() - 6 && r.right() <= c.right() + 0.1;   // (a table's edge is left of its text by the margin of its cells: 5.4 points)
                }
                QVERIFY2(inAColumn, qPrintable(QStringLiteral("page %1: the table at %2 is not inside a column of the section (%3 and %4)").arg(k).arg(audit::rectText(r), audit::rectText(sec.columnRect(0)), audit::rectText(sec.columnRect(1)))));
                if (k == 0) QVERIFY2(r.top() >= top + split - 0.1, qPrintable(QStringLiteral("page 0: the table begins at %1, the section before it ends at %2").arg(r.top()).arg(top + split)));
                ++fragments;
            }
        }
        QVERIFY(fragments >= 3);
        const auto first = pg.materialize(0);
        QVERIFY2(first->tables.size() == 2, qPrintable(QString::number(first->tables.size())));
        QVERIFY(first->tables[1]->rect().left() > first->tables[0]->rect().right() - 0.1);
        QVERIFY2(std::abs(first->tables[1]->rect().top() - (top + split)) < 0.1, qPrintable(QStringLiteral("the table's first part is at %1, its second at %2, the columns begin at %3")
                                                                                              .arg(audit::rectText(first->tables[0]->rect()), audit::rectText(first->tables[1]->rect())).arg(top + split)));
        QCOMPARE(rowsShown(pg), 70);
        const QString bad = audit::problems(pg, doc->story(), {{"fn2", "COLNOTE"}});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A note longer than the pages, cited from a cell: it goes on over pages of its own, which are made and drawn, and the end of it is on the last of them.
    void aFilesNoteLongerThanThePageCitedFromACellHasPagesOfItsOwn()
    {
        const QByteArray body = plainP("Opening paragraph.") + tableOfRows(6, 2, 1500, [](int r, int c) { return c == 1 && r == 2 ? plainP("row 2 cites", refTo(2)) : cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) +
                                plainP("Tail one.") + plainP("Tail two.") + plainP("Tail three, which cites a short note.", refTo(3)) + plainP("Tail four.") + plainP("Tail five.");
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/footnotes.xml"), notesPart({{2, wordsOf("LONGNOTE", 700)}, {3, "SHORTNOTE says little"}})}}));
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        const int pages = pg.pageCount();
        int noteOnly = 0;
        QString drawn;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY2(page, qPrintable(QStringLiteral("page %1 of %2 isn't made").arg(k).arg(pages)));
            if (pg.lines(k).isEmpty()) {
                ++noteOnly;
                QVERIFY2(!page->doc->pages[0]->items.empty(), qPrintable(QStringLiteral("page %1 of notes alone has nothing drawn").arg(k)));
            }
            for (const auto &item : page->doc->pages[0]->items) drawn += audit::storyOf(*page, item.get()) + QLatin1Char('\n');
        }
        QVERIFY2(noteOnly > 0, qPrintable(QStringLiteral("%1 pages, none of notes alone").arg(pages)));
        // (A box's story holds the whole rest of its note, whatever the box shows of it: what is drawn is the height of the boxes.)
        const FieldSource *fields = doc->flow().fields;
        QVERIFY(fields);
        const double width = pg.page(0).content.width();
        const QString lost = audit::lostNotes(pg, audit::fullNoteHeight(fields->storyDoc(QStringLiteral("fn2")), width, doc->flow().env) + audit::fullNoteHeight(fields->storyDoc(QStringLiteral("fn3")), width, doc->flow().env));
        QVERIFY2(lost.isEmpty(), qPrintable(lost));
        QVERIFY2(drawn.contains(QStringLiteral("SHORTNOTE")), "the short note is drawn");
        QCOMPARE(rowsShown(pg), 6);
        const QString bad = audit::problems(pg, doc->story(), {{"fn2", "LONGNOTE"}, {"fn3", "SHORTNOTE"}});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A row that goes over page breaks in a file whose headers and footers take room from the text: every paragraph of its cells is found on the page that shows the part of
    // the row it is in.
    void aFilesPositionsInARowThatGoesOverPageBreaksAreOnThePagesThatShowThem()
    {
        const QByteArray refs = "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>";
        QByteArray tall;
        for (int i = 0; i < 140; ++i) tall += cellP("line " + QByteArray::number(i) + " of the tall cell");
        const QByteArray body = plainP("Before the table") + tableX(rowX(cellX(cellP("first"), QByteArray(), 1500) + cellX(cellP("row"), QByteArray(), 1500)) + rowX(cellX(cellP("tall"), QByteArray(), 1500) + cellX(tall, QByteArray(), 1500)) +
                                                                  rowX(cellX(cellP("last"), QByteArray(), 1500) + cellX(cellP("row"), QByteArray(), 1500)), 2, 1500) + plainP("After the table");
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, refs), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>H1</w:t></w:r>", "<w:r><w:t>H2</w:t></w:r>", "<w:r><w:t>H3</w:t></w:r>", "<w:r><w:t>H4</w:t></w:r>"})},
                                                                                {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:t>F1</w:t></w:r>", "<w:r><w:t>F2</w:t></w:r>", "<w:r><w:t>F3</w:t></w:r>"})}}));
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        QVERIFY(pg.pageCount() >= 3);
        QVERIFY2(doc->flow().section(0).content().height() - pg.page(1).content.height() > 10, "the header and the footer take room from the text");
        const QString bad = audit::splitRowProblems(pg, doc->story());
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        const QString rest = audit::problems(pg, doc->story());
        QVERIFY2(rest.isEmpty(), qPrintable(rest));
    }

    // A table under the line heights of a word processor, in a file of the 2003 programs (compatibility mode 11) and one of 2013's and after (15): the stand-in for the table in
    // the page's text does not shift it or the text after it. The table begins where the text before it ends (and its space after), and the page's text has every line
    // where the paginator put it. (The two modes differ in where a table's left edge is: a table's first cell has its text at the margin in the first.)
    void aFilesTableIsNotShiftedByItsPlaceholderUnderProcessorLineHeights()
    {
        struct Case { const char *name; QByteArray spacing; };
        const Case cases[] = {{"auto 1.15, 12 before, 8 after", "<w:spacing w:before=\"240\" w:after=\"160\" w:line=\"276\" w:lineRule=\"auto\"/>"},
                              {"exactly 20, none after", "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"400\" w:lineRule=\"exact\"/>"},
                              {"at least 30, 5 after", "<w:spacing w:after=\"100\" w:line=\"600\" w:lineRule=\"atLeast\"/>"},
                              {"single, 6 after", "<w:spacing w:after=\"120\" w:line=\"240\" w:lineRule=\"auto\"/>"},
                              {"a border below, 6 after", "<w:pBdr><w:bottom w:val=\"single\" w:sz=\"8\" w:space=\"3\" w:color=\"auto\"/></w:pBdr><w:spacing w:after=\"120\"/>"}};
        double edge[2] = {0, 0};
        for (const int mode : {11, 15})
            for (const Case &cs : cases) {
                const QByteArray settings = "<w:compat><w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"" + QByteArray::number(mode) + "\"/></w:compat>";
                const QByteArray body = plainP(wordsOf("First", 25)) + plainP(wordsOf("The last paragraph before the table", 12), QByteArray(), cs.spacing) +
                                        tableOfRows(4, 2, 2000, [](int r, int c) { return cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) +
                                        plainP("After the table", QByteArray(), "<w:spacing w:line=\"360\" w:lineRule=\"auto\"/>") + plainP(wordsOf("Last", 40));
                auto doc = open(makeDocxWithParts(body, sectionXml(7000, 9000, 1000), {}, settings));
                QVERIFY(doc);
                flow::Paginator &pg = doc->pager();
                const QString what = QStringLiteral("mode %1, %2").arg(mode).arg(QLatin1String(cs.name));
                QVERIFY2(pg.pageCount() >= 1, qPrintable(what));
                const auto lines = pg.lines(0);
                int firstRow = 0;
                while (firstRow < lines.size() && !cellKeyOf(doc->story()->findBlock(lines[firstRow].pos))) ++firstRow;
                QVERIFY2(firstRow > 0 && firstRow + 4 <= lines.size(), qPrintable(what));
                // (A paragraph with a border below has the room of the border's lower edge, its space and its width, under its last line before the space after it.)
                const double after = doc->story()->findBlock(lines[firstRow - 1].pos).blockFormat().bottomMargin() + (cs.spacing.contains("pBdr") ? 3.0 + 8 / 8.0 : 0.0);
                QVERIFY2(std::abs(lines[firstRow - 1].rect.bottom() + std::max(0.0, after) - lines[firstRow].rect.top()) < 0.05, qPrintable(what + ": the table's top and the end of the text before it"));
                const auto page = pg.materialize(0);
                QVERIFY(page && page->tables.size() == 1);
                QVERIFY2(std::abs(page->tables[0]->rect().top() - pg.page(0).content.top() - lines[firstRow].rect.top()) < 0.05, qPrintable(what + ": the table is drawn where its rows are"));
                const QString bad = audit::placementProblems(pg, doc->flow(), 0);
                QVERIFY2(bad.isEmpty(), qPrintable(what + QStringLiteral(": ") + bad));
                if (&cs == &cases[0]) edge[mode == 11 ? 0 : 1] = page->tables[0]->rect().left() - pg.page(0).content.left();
            }
        QVERIFY2(edge[0] < edge[1] - 1, qPrintable(QStringLiteral("the table's left edge is %1 from the margin in mode 11, %2 in mode 15").arg(edge[0]).arg(edge[1])));
    }

    // A footer with PAGE and NUMPAGES, in a file whose table spans pages: each page shows its number and the count, and the pages made after rows are put in show the new count.
    void aFilesPageFieldsInAFooterAreRightForATableThatSpansPages()
    {
        const QByteArray footer = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple><w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>"
                                                       "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>1</w:t></w:r></w:fldSimple>",
                                                       "<w:r><w:t>A second line of the footer</w:t></w:r>"});
        const QByteArray refs = "<w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>";
        const QByteArray body = plainP("Before the table") + tableOfRows(100, 2, 1500, [](int r, int c) { return cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) + plainP("After the table");
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, refs), {{QStringLiteral("/word/footer1.xml"), footer}}));
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        auto checkPages = [&](const QString &when) -> QString {
            const int pages = pg.pageCount();
            for (int k = 0; k < pages; ++k) {
                const auto page = pg.materialize(k);
                if (!page) return QStringLiteral("%1: page %2 is not made").arg(when).arg(k);
                const auto foots = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
                if (foots.size() != 1) return QStringLiteral("%1: page %2 has %3 footers").arg(when).arg(k).arg(foots.size());
                FieldContext ctx;
                ctx.doc = page->doc.get();
                if (ctx.resolve(QStringLiteral("page")) != QString::number(k + 1)) return QStringLiteral("%1: page %2 shows the number %3").arg(when).arg(k).arg(ctx.resolve(QStringLiteral("page")));
                if (ctx.resolve(QStringLiteral("pages")) != QString::number(pages)) return QStringLiteral("%1: page %2 shows %3 pages, the file has %4").arg(when).arg(k).arg(ctx.resolve(QStringLiteral("pages"))).arg(pages);
                if (!page->showsTotal || page->pageTotal != pages) return QStringLiteral("%1: page %2 is made with the total %3 (it shows it: %4), the file has %5").arg(when).arg(k).arg(page->pageTotal).arg(page->showsTotal).arg(pages);
            }
            return audit::problems(pg, doc->story());
        };
        const int before = pg.pageCount();
        QVERIFY(before >= 5);
        QString bad = checkPages(QStringLiteral("as opened"));
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        QTextCursor c(doc->story());
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (cellKeyOf(b)) {
                c.setPosition(b.position());
                break;
            }
        for (int i = 0; i < 30; ++i) insertTableRow(*doc, c, true);
        QVERIFY(pg.pageCount() > before);
        bad = checkPages(QStringLiteral("after the rows"));
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        flow::Paginator fresh(&doc->flow());
        bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    void randomEditingKeepsTheDocumentWhole()
    {
        // A document with lists, a section, pieces kept as XML, and a page break; hundreds of random commands on it, and after each few the
        // file it makes opens again with the same text, the same sections, and pages that can be made.
        const QByteArray numbering =
            "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>"
            "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"lowerLetter\"/><w:lvlText w:val=\"%2)\"/><w:pPr><w:ind w:left=\"1440\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        QByteArray body;
        for (int i = 0; i < 4; ++i) body += "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"" + QByteArray::number(i % 2) + "\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>item " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        body += "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"1\" w:name=\"a\"/><w:r><w:t>A heading</w:t></w:r><w:bookmarkEnd w:id=\"1\"/></w:p>";
        body += paragraphs(6, 40);
        // A table of cells joined across and down, with a header row, a cell with two paragraphs, a table in a cell, and a bookmark between the rows.
        body += "<w:tbl><w:tblPr><w:tblStyle w:val=\"Grid\"/><w:tblW w:w=\"6000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
                "<w:tr><w:trPr><w:tblHeader/></w:trPr><w:tc><w:p><w:r><w:t>head a</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>head b</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>head c</w:t></w:r></w:p></w:tc></w:tr>"
                "<w:bookmarkStart w:id=\"2\" w:name=\"inTable\"/><w:bookmarkEnd w:id=\"2\"/>"
                "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"4000\" w:type=\"dxa\"/><w:gridSpan w:val=\"2\"/></w:tcPr><w:p><w:r><w:t>wide</w:t></w:r></w:p><w:p><w:r><w:t>second paragraph of the wide cell</w:t></w:r></w:p></w:tc>"
                "<w:tc><w:tcPr><w:vMerge w:val=\"restart\"/></w:tcPr><w:p><w:r><w:t>tall</w:t></w:r></w:p></w:tc></w:tr>"
                "<w:tr><w:tc><w:p><w:r><w:t>one</w:t></w:r></w:p></w:tc><w:tc><w:tbl><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>in</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>side</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/></w:tc>"
                "<w:tc><w:tcPr><w:vMerge/></w:tcPr><w:p/></w:tc></w:tr></w:tbl>";
        body += "<w:p><w:r><w:t>before break </w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after break</w:t></w:r></w:p>";
        body += "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr></w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>";
        body += paragraphs(5, 30);
        const QByteArray gridStyle =
            "<w:style w:type=\"table\" w:styleId=\"Grid\"><w:name w:val=\"Grid\"/><w:pPr><w:spacing w:after=\"0\"/></w:pPr><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
            "<w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders></w:tblPr>"
            "<w:tblStylePr w:type=\"firstRow\"><w:rPr><w:b/></w:rPr><w:tcPr><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"DDDDDD\"/></w:tcPr></w:tblStylePr></w:style>";
        const int seeds = qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 8;   // (more, to look further)
        for (unsigned seed = std::max(1, qEnvironmentVariableIntValue("JW_FUZZ_FROM")); seed <= unsigned(seeds); ++seed) {
            auto doc = open(makeDocx(body, kStyles + gridStyle, numbering));
            QVERIFY(doc);
            std::mt19937 rng(seed);
            QTextCursor c(doc->story());
            auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
            auto place = [&] { c.setPosition(pick(doc->story()->characterCount() - 1)); };
            auto selectSome = [&] {
                const int a = pick(doc->story()->characterCount() - 1), len = pick(60);
                c.setPosition(a);
                c.setPosition(std::min(a + len, doc->story()->characterCount() - 1), QTextCursor::KeepAnchor);
            };
            QTextDocumentFragment held;
            QStringList log;
            for (int step = 0; step < 300; ++step) {
                const int op = pick(23);
                log << QStringLiteral("%1:op%2 pos=%3 anchor=%4 blocks=%5").arg(step).arg(op).arg(c.position()).arg(c.anchor()).arg(doc->story()->blockCount());
                switch (op) {
                case 0: place(); typeText(*doc, c, QStringLiteral("word ")); break;
                case 1: place(); insertParagraph(*doc, c); break;
                case 2: place(); deleteBack(*doc, c, pick(2)); break;
                case 3: place(); deleteForward(*doc, c, pick(2)); break;
                case 4: selectSome(); removeSelection(*doc, c); break;
                case 5: selectSome(); toggleBold(*doc, c); toggleItalic(*doc, c); setFontSize(*doc, c, 9 + pick(20)); break;
                case 6: selectSome(); toggleList(*doc, c, pick(2)); break;
                case 7: place(); applyParagraphStyle(*doc, c, pick(3) == 0 ? QStringLiteral("Heading1") : QStringLiteral("Normal")); break;
                case 8: place(); insertPageBreak(*doc, c); break;
                case 9: if (pick(4) == 0) { place(); insertSectionBreak(*doc, c); } break;
                case 10: doc->story()->undo(&c); break;
                case 11: doc->story()->redo(&c); break;
                case 12: selectSome(); changeIndent(*doc, c, pick(2) ? 36 : -36); break;
                case 13: selectSome(); setAlignment(*doc, c, pick(2) ? Qt::AlignRight : Qt::AlignJustify); break;
                case 14: selectSome(); held = selectionFragment(c); break;
                case 15: place(); pasteFragment(*doc, c, held); break;
                case 17: case 18: case 19: case 20: case 21: case 22: tableCommand(*doc, c, op - 17, pick); break;
                default: place(); moveCursor(c, pick(2) ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter, pick(2), 1 + pick(3)); break;
                }
                QVERIFY(doc->story()->blockCount() >= 1);
                // A paragraph that stands for a piece kept as XML holds that and nothing else.
                for (QTextBlock bk = doc->story()->begin(); bk.isValid(); bk = bk.next())
                    if (isLocked(bk) && bk.length() != 2) QFAIL(qPrintable(QStringLiteral("seed %1: after %2 the stand-in holds %3 characters\nlast steps:\n%4").arg(seed).arg(log.last()).arg(bk.length() - 1).arg(log.mid(std::max<qsizetype>(0, log.size() - 5)).join(QLatin1Char('\n')))));
                if (step % 12 != 0) continue;
                // The document is whole: sections as the break paragraphs say, pages that can be made, a file that opens to the same.
                int breaks = 0;
                for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) breaks += b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
                QCOMPARE(doc->sectionCount(), breaks + 1);
                const int pages = doc->pager().pageCount();
                QVERIFY(pages >= 1);
                QVERIFY(doc->pager().materialize(pick(pages)));
                QString err;
                const QByteArray zip = doc->toDocx(&err);
                QVERIFY2(!zip.isEmpty(), qPrintable(QStringLiteral("seed %1 step %2: %3").arg(seed).arg(step).arg(err)));
                QVERIFY2(rt::strictProblems(zip).isEmpty(), qPrintable(QStringLiteral("seed %1 step %2: %3").arg(seed).arg(step).arg(rt::strictProblems(zip).join(QLatin1Char(';')))));
                auto back = open(zip);
                QVERIFY2(back, qPrintable(QStringLiteral("seed %1 step %2").arg(seed).arg(step)));
                QString a = doc->story()->toPlainText(), b = back->story()->toPlainText();
                a.remove(QChar::ObjectReplacementCharacter);
                b.remove(QChar::ObjectReplacementCharacter);
                if (a != b) {
                    int d = 0;
                    while (d < a.size() && d < b.size() && a[d] == b[d]) ++d;
                    const QByteArray x = documentXml(zip);
                    QString dump;
                    for (QTextBlock bk = doc->story()->begin(); bk.isValid(); bk = bk.next()) {
                        const QString t = bk.text();
                        dump += QStringLiteral("[%1 len %2 locked=%3 sect=%5] %6\n").arg(bk.blockNumber()).arg(bk.length()).arg(isLocked(bk)).arg(bk.blockFormat().boolProperty(flow::fp::SectionBreakAfter)).arg(t.left(30).replace(QChar::ObjectReplacementCharacter, QLatin1Char('#')));
                    }
                    qWarning("%s", qPrintable(dump));
                    QFAIL(qPrintable(QStringLiteral("seed %1 step %2: the text differs at %3 (blocks %6 and %7):\n%4\n---\n%5\n=== xml:\n%8").arg(seed).arg(step).arg(d).arg(a.mid(std::max(0, d - 60), 120).toHtmlEscaped(), b.mid(std::max(0, d - 60), 120).toHtmlEscaped()).arg(doc->story()->blockCount()).arg(back->story()->blockCount()).arg(QString::fromUtf8(x.mid(std::max<qsizetype>(0, x.indexOf("judgment evidence court of.") - 1500), 3500)))));
                }
                QCOMPARE(back->sectionCount(), doc->sectionCount());
                // (And what it writes again is what it wrote: nothing drifts.)
                const QByteArray x1 = documentXml(zip), x2 = documentXml(back->toDocx());
                if (x1 != x2) {
                    int d = 0;
                    while (d < x1.size() && d < x2.size() && x1[d] == x2[d]) ++d;
                    QFAIL(qPrintable(QStringLiteral("seed %1 step %2: drift at %3:\n%4\n---\n%5").arg(seed).arg(step).arg(d).arg(QString::fromUtf8(x1.mid(std::max(0, d - 200), 400)), QString::fromUtf8(x2.mid(std::max(0, d - 200), 400)))));
                }
            }
        }
    }

    void theBlankDocumentSavesAndOpensAndKeepsItsStyles()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("Title\nBody"));
        c.setPosition(0);
        applyParagraphStyle(*doc, c, QStringLiteral("Title"));
        const QByteArray zip = doc->toDocx();
        office::Package p;
        QVERIFY(p.read(zip));
        QVERIFY(p.hasPart(QStringLiteral("/word/styles.xml")));
        QVERIFY(p.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Title\""));
        QVERIFY(p.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Heading1\""));
        auto back = open(zip);
        QCOMPARE(back->story()->toPlainText(), QStringLiteral("Title\nBody"));
        QCOMPARE(back->story()->firstBlock().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Title"));
        // A style that a file lacks is added when it is first used, and written.
        auto bare = open(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>"));
        // (It is offered before that, with the look it will have, and offering it changes nothing in the file.)
        bool offered = false;
        for (const StyleEntry &e : bare->styleEntries()) offered = offered || (e.id == QLatin1String("Heading2") && e.quick);
        QVERIFY(offered);
        QVERIFY(bare->styleCharFormat(QStringLiteral("Heading2")).fontPointSize() > 11);
        QVERIFY(!bare->styleSheet().find(QStringLiteral("Heading2")));
        QTextCursor d(bare->story());
        applyParagraphStyle(*bare, d, QStringLiteral("Heading2"));
        QVERIFY(bare->styleSheet().find(QStringLiteral("Heading2")));
        office::Package q;
        QVERIFY(q.read(bare->toDocx()));
        QVERIFY(q.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Heading2\""));
    }
};

QTEST_MAIN(WriteTests)
#include "tst_write.moc"
