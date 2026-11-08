#pragma once
// Tests of JeffWrite (apps/write): the document, editing it, the .docx it reads and writes, the page view, and the window.

#include "../isolatesettings.h"
#include "core/fonts.h"
#include "../plainprobe.h"
#include "../stablesaves.h"
#include "app/pagepaint.h"
#include "app/pageview.h"
#include "app/pageoutput.h"
#include "app/spellpane.h"
#include "app/stylespane.h"
#include "app/filescreen.h"
#include "app/findreplace.h"
#include "app/help.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "app/filetasks.h"
#include <QListWidget>
#include <QStandardPaths>
#include "app/ribbon.h"
#include "app/widgets.h"
#include "app/writefindpane.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/drawnitems.h"
#include "core/fieldops.h"
#include "core/commentops.h"
#include "core/editops.h"
#include "core/headerops.h"
#include "core/pagenumbering.h"
#include "core/pagesetup.h"
#include "core/pictureops.h"
#include "core/printing.h"
#include "core/rulerops.h"
#include "core/spellcheck.h"
#include "core/styleops.h"
#include "core/gotoops.h"
#include "app/tabledialogs.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "../flow/pageaudit.h"
#include "../office/dochelpers.h"
#include "office/package.h"
#include "wml/xmltree.h"
#include "render/renderer.h"
#include "rtcheck.h"
#include "../timing.h"
#include "app/spelling.h"
#include "text/dictionaries.h"
#include "text/textengine.h"
#include "text/textprops.h"
#include "io/zip.h"
#include "wml/wml.h"

#include <QAccessible>
#include <QTcpServer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QBuffer>
#include <QImageReader>
#include <QUrl>
#include <QImage>
#include <QPainter>
#include <QApplication>
#include <QXmlStreamReader>
#include <climits>
#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif
#include <QMenu>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPrinter>
#include <QStatusBar>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QSpinBox>
#include <QTimer>
#include <QMessageBox>
#include <QPushButton>
#include <QTranslator>
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
#include <limits>
#include <random>
#include <set>

using namespace jp;
using namespace jp::write;
namespace wml = jp::office::wml;
namespace audit = jp::flowaudit;

namespace detail_tst_write {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

inline QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

// A package of a document whose body is `body`, with the styles and lists given (a file of its own making, not JeffWrite's).
inline QByteArray makeDocx(const QByteArray &body, const QByteArray &styles = QByteArray(), const QByteArray &numbering = QByteArray(), const QByteArray &sect = QByteArray())
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

// The namespaces a VML object in a body uses, and a package of a document whose body is `body` with a picture (20 by 10 pixels, black and white) that is related as rIdPic.
const char kVml[] = "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\"";

inline QByteArray makeDocxWithPicture(const QByteArray &body, bool withCompressed = false)
{
    QImage image(20, 10, QImage::Format_RGB32);
    image.fill(Qt::black);
    for (int y = 0; y < 10; ++y)
        for (int x = 10; x < 20; ++x) image.setPixel(x, y, qRgb(255, 255, 255));
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addPart(QStringLiteral("/word/media/image1.png"), png, QStringLiteral("image/png"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdPic"), rel + QStringLiteral("image"), QStringLiteral("/word/media/image1.png"));
    if (withCompressed) {
        // (A compressed picture of the kind the original program keeps beside a picture: the bytes are not a picture's.)
        w.addPart(QStringLiteral("/word/media/image2.pcz"), QByteArray("not a picture"), QStringLiteral("image/x-pcz"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdPcz"), rel + QStringLiteral("image"), QStringLiteral("/word/media/image2.pcz"));
    }
    return w.finish();
}

// The object set in the first paragraph's text (an inline picture, rule, or lettering), or none.
inline ItemPtr inlineObjectOf(const WriteDoc &doc)
{
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QString json = it.fragment().charFormat().stringProperty(tp::InlineObject);
            if (!json.isEmpty()) return Item::fromJsonAny(QJsonDocument::fromJson(json.toUtf8()).object());
        }
    return nullptr;
}

// Every object set in the story's text, in order.
inline QVector<ItemPtr> inlineObjectsOf(const WriteDoc &doc)
{
    QVector<ItemPtr> out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QString json = it.fragment().charFormat().stringProperty(tp::InlineObject);
            if (!json.isEmpty()) out << Item::fromJsonAny(QJsonDocument::fromJson(json.toUtf8()).object());
        }
    return out;
}

inline QByteArray documentXmlOfPart(const QByteArray &zip, const QString &part)
{
    office::Package p;
    return p.read(zip) ? p.part(part) : QByteArray();
}

inline QByteArray documentXml(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/document.xml"));
}

inline QByteArray styleXml(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/styles.xml"));
}

inline std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

inline QByteArray paragraphs(int n, int words = 60)
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

// The text of a long run of paragraphs for the cost tests of space shrinking: `each` words in each. A word is given, or (with none) of two to ten letters in turn so that the
// room left at the lines' ends differs from line to line; each paragraph begins with a word of its own so that no two are the same text.
inline QByteArray justifiedParagraphs(const QByteArray &word, int count, int each)
{
    QByteArray body;
    for (int i = 0; i < count; ++i) {
        body += "<w:p><w:pPr><w:jc w:val=\"both\"/></w:pPr><w:r><w:rPr><w:rFonts w:ascii=\"Tinos\" w:hAnsi=\"Tinos\"/><w:sz w:val=\"24\"/></w:rPr><w:t xml:space=\"preserve\">p" +
                QByteArray::number(i) + " ";
        for (int w = 0; w < each; ++w) body += (word.isEmpty() ? QByteArray("abcdefghij").left(2 + (i * 5 + w * w + w * 7) % 9) : word + QByteArray::number(w % 7)) + " ";
        body += "</w:t></w:r></w:p>";
    }
    return body;
}

inline void typeText(WriteDoc &doc, QTextCursor &c, const QString &text)
{
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n')) insertParagraph(doc, c);
        else insertText(doc, c, QString(ch));
    }
}

inline QString plain(WriteDoc &doc) { return doc.story()->toPlainText(); }

// The text each line of one of a page's text boxes shows (by default the first page's first box, the text's), with the fields in their places, as the page's own box lays it out.
inline QStringList boxLines(WriteDoc &doc, int pageNumber = 0, int box = 0)
{
    const auto page = doc.pager().materialize(pageNumber);
    if (!page || box >= page->boxes.size()) return {};
    const auto *item = dynamic_cast<const TextItem *>(page->doc->item(page->boxes[box]));
    if (!item) return {};
    FrameSpec fs;
    fs.size = item->rect.size();
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout layout;
    layout.build(page->doc->story(item->storyId)->doc.get(), {fs}, doc.flow().env);
    QStringList lines;
    for (const auto &l : layout.lineInfo(0)) lines << QString(l.text).remove(QChar(0x2060)).trimmed();   // (the mark that joins is not shown)
    return lines;
}

// The formats of a story's fragments that are atoms of the tag given (the ones that stand for something the file keeps as XML), in order.
inline QList<QTextCharFormat> atomFormats(WriteDoc &doc, const QString &tag)
{
    QList<QTextCharFormat> out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (cf.hasProperty(prop::Atom) && unpackAtom(storedBytes(cf, prop::Atom)).tag == tag) out << cf;
        }
    return out;
}

// A random place in a table's cells (the cursor is not moved when the story has no table).
inline bool placeInACell(WriteDoc &doc, QTextCursor &c, const std::function<int(int)> &pick)
{
    QVector<QTextBlock> cells;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        if (cellKeyOf(b)) cells << b;
    if (cells.isEmpty()) return false;
    const QTextBlock b = cells[pick(int(cells.size()))];
    c.setPosition(b.position() + pick(b.length()));
    return true;
}

// The table commands, as the random editing picks them (kinds 0 to 4: add a row, a column, Tab, take a row, a column; kind 5: the table's own commands, and sometimes taking the table).
inline void tableCommand(WriteDoc &doc, QTextCursor &c, int kind, const std::function<int(int)> &pick)
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

// The commands of the Table tabs, on a stretch of cells picked at random (kinds 1 to 10: merge, split, shade, border, size, distribute, fit, properties, style, alignment).
inline void tableTabCommand(WriteDoc &doc, QTextCursor &c, int k, const std::function<int(int)> &pick)
{
    QVector<QTextBlock> cells;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        if (cellKeyOf(b)) cells << b;
    if (cells.isEmpty()) return;
    const QTextBlock a = cells[pick(int(cells.size()))], z = cells[pick(int(cells.size()))];
    c.setPosition(a.position());
    if (pick(2) == 0) c.setPosition(z.position() + z.length() - 1, QTextCursor::KeepAnchor);
    switch (k) {
    case 1: mergeCells(doc, c); break;
    case 2: splitCells(doc, c, 1 + pick(3), 1 + pick(2)); break;
    case 3: setShading(doc, c, pick(3) == 0 ? QColor() : QColor(pick(256), pick(256), pick(256))); break;
    case 4: {
        flow::TableLine line;
        if (pick(4)) {
            line.style = flow::TableLine::Style(1 + pick(5));
            line.width = 0.5 + pick(4) * 0.5;
        }
        setBorders(doc, c, 1 + pick(63), line);
        break;
    }
    case 5: pick(2) ? setColumnWidth(doc, c, 20 + pick(200)) : setRowHeight(doc, c, pick(3) ? 10 + pick(60) : 0, pick(2)); break;
    case 6: pick(2) ? distributeColumns(doc, c) : distributeRows(doc, c); break;
    case 7: fitTable(doc, c, TableFit(pick(3))); break;
    case 8: {
        const TableProps was = tablePropsAt(doc, c);
        TableProps now = was;
        now.align = flow::TableData::Align(pick(3));
        now.indent = pick(30);
        now.cantSplit = pick(2);
        now.header = pick(2);
        now.valign = flow::CellAlign(pick(3));
        now.widthKind = TableProps::Width(pick(3));
        now.width = now.widthKind == TableProps::Width::Percent ? 20 + pick(80) : 100 + pick(300);
        setTableProps(doc, c, was, now);
        break;
    }
    case 9: setTableStyle(doc, c, office::wml::builtinTableStyleIds().value(pick(14))); break;
    default: setCellAlignment(doc, c, pick(2) ? Qt::AlignHCenter : Qt::AlignRight, flow::CellAlign(pick(3))); break;
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
inline std::unique_ptr<WriteDoc> textDoc(const QStringList &paragraphs, const QByteArray &styles = kStyles)
{
    QByteArray body;
    for (const QString &p : paragraphs) body += "<w:p><w:r><w:t xml:space=\"preserve\">" + p.toUtf8() + "</w:t></w:r></w:p>";
    return open(makeDocx(body, styles));
}

// The first page drawn 816 pixels across (a letter's width: 4/3 pixel to the point), and what a part of it is: the gray most of it is (its fill) and the gray farthest
// from that (its ink). The part is in points from the page's top left.
struct BackAndInk {
    int back = 0, ink = 0;
};
inline BackAndInk backAndInk(const QImage &img, const QRectF &points)
{
    const double k = img.width() / 612.0;
    int count[3] = {0, 0, 0};
    long sum[3] = {0, 0, 0};
    QVector<int> grays;
    for (int y = int(points.top() * k); y < int(points.bottom() * k); ++y)
        for (int x = int(points.left() * k); x < int(points.right() * k); ++x) {
            const int g = qGray(img.pixel(x, y));
            const int b = g < 85 ? 0 : g < 170 ? 1 : 2;
            ++count[b];
            sum[b] += g;
            grays << g;
        }
    BackAndInk out;
    const int best = count[0] >= count[1] && count[0] >= count[2] ? 0 : count[1] >= count[2] ? 1 : 2;
    if (!count[best]) return out;
    out.back = out.ink = int(sum[best] / count[best]);   // (a part of one gray has no ink: its ink is its fill)
    for (const int g : grays)
        if (std::abs(g - out.back) > std::abs(out.ink - out.back)) out.ink = g;
    return out;
}
inline QImage drawnPage(WriteDoc &doc)
{
    PageView view;
    view.resize(1000, 700);
    view.setDocument(&doc);
    return view.pageImage(0, 816);
}

// The stretches of ink along the first line of a page whose first paragraph begins as the XML says (where each begins, and ends, in points from the page's left edge; a
// gap of a point and a half between two), for the checks of what a symbol takes of the line.
inline QVector<QPair<double, double>> inkAlongFirstLine(const QByteArray &start)
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    QVector<QPair<double, double>> out;
    const QByteArray body = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr>" + start + "<w:r><w:t xml:space=\"preserve\"> Food products are usually imported</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, QByteArray(), sect));
    if (!doc) return out;
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    const QImage img = view.pageImage(0, 816);
    const double perPoint = 816 / 612.0;
    double begin = -1, last = -1;
    for (int x = int(60 * perPoint); x < int(300 * perPoint); ++x) {
        bool ink = false;
        for (int y = int(72 * perPoint); y < int(90 * perPoint) && !ink; ++y) ink = qGray(img.pixel(x, y)) < 160;
        if (!ink) continue;
        if (begin >= 0 && x / perPoint - last > 1.5) {
            out.push_back({begin, last});
            begin = -1;
        }
        if (begin < 0) begin = x / perPoint;
        last = x / perPoint;
    }
    if (begin >= 0) out.push_back({begin, last});
    return out;
}

// The words a spelling check flags in the whole story, in order.
inline QStringList flaggedWords(SpellCheck &sc, WriteDoc &doc)
{
    const QString text = doc.story()->toPlainText();
    QStringList out;
    for (const auto &r : sc.misspelledIn(0, doc.story()->characterCount())) out << text.mid(r.first, r.second - r.first);
    return out;
}


// A document with every kind of thing a file keeps beside its text, for the checks that nothing is lost: paragraph ids, hyperlinks (outside and
// inside), fields, content controls (inline and block), tracked changes, bookmarks and comment ranges (one across a kept table, one with an Id
// used twice), a section that ends on a heading, page breaks, a kept table, and kept parts with their relationships.
inline QByteArray richDocx()
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
inline QByteArray makeDocxWithParts(const QByteArray &body, const QByteArray &sect, const QMap<QString, QByteArray> &parts, const QByteArray &settings = QByteArray(), const QByteArray &styles = kStyles,
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
        const QString kind = it.key().contains(QLatin1String("footnotes")) ? QStringLiteral("footnotes") : it.key().contains(QLatin1String("endnotes")) ? QStringLiteral("endnotes") : it.key().contains(QLatin1String("fontTable")) ? QStringLiteral("fontTable")
                             : it.key().contains(QLatin1String("footer")) ? QStringLiteral("footer") : QStringLiteral("header");
        w.addPart(it.key(), xml(it.value()), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.") + kind + "+xml");
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), id, rel + kind, it.key());
    }
    return w.finish();
}

// The dark pixels of a list item's number, in its own place (the left of the text area, up to the text, which a hanging indent of two inches sets away): the box they are in,
// how many, and how many of them are red. The level of the list has `levelRun` (the XML of its run properties), the paragraph's mark `markRun`, and the item's first run `firstRun`.
struct NumberInk {
    QRect box;
    int dark = 0, red = 0;
};

inline NumberInk numberInk(const QByteArray &levelRun, const QByteArray &markRun, const QByteArray &firstRun)
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/>"
                                 "<w:pPr><w:ind w:left=\"2880\" w:hanging=\"2880\"/></w:pPr>" + (levelRun.isEmpty() ? QByteArray() : "<w:rPr>" + levelRun + "</w:rPr>") + "</w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/><w:rPr>" + markRun +
                            "</w:rPr></w:pPr><w:r><w:rPr>" + firstRun + "</w:rPr><w:t>HHHH</w:t></w:r></w:p>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {}, QByteArray(), kStyles, numbering));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    const QImage img = view.pageImage(0, 816);
    const double perPoint = 816 / 612.0;
    NumberInk ink;
    int x0 = 1 << 20, y0 = 1 << 20, x1 = -1, y1 = -1;
    for (int x = int(72 * perPoint); x < int((72 + 140) * perPoint); ++x)
        for (int y = int(72 * perPoint); y < int((72 + 90) * perPoint); ++y) {
            const QRgb c = img.pixel(x, y);
            if (qGray(c) >= 160) continue;
            ++ink.dark;
            ink.red += qRed(c) > 150 && qGreen(c) < 90 && qBlue(c) < 90;
            x0 = std::min(x0, x);
            x1 = std::max(x1, x);
            y0 = std::min(y0, y);
            y1 = std::max(y1, y);
        }
    if (x1 >= 0) ink.box = QRect(QPoint(x0, y0), QPoint(x1, y1));
    return ink;
}

// The markers of the list items of a story, in order: a paragraph's own marker text where it has one, else what Qt counts.
inline QStringList markersOf(WriteDoc &doc)
{
    QStringList out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        if (b.textList()) out << (b.blockFormat().stringProperty(tp::BulletChar).isEmpty() ? b.textList()->itemText(b) : b.blockFormat().stringProperty(tp::BulletChar));
    return out;
}

// A paragraph numbered by `num` at `level`, with the text `text`.
inline QByteArray itemOf(int num, int level, const char *text = "item")
{
    return QByteArray("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"") + QByteArray::number(level) + "\"/><w:numId w:val=\"" + QByteArray::number(num) + "\"/></w:numPr></w:pPr><w:r><w:t>" + text + "</w:t></w:r></w:p>";
}

// A header or footer part of the paragraphs given (each the XML of a paragraph's contents).
inline QByteArray headerPart(const QByteArray &root, const QList<QByteArray> &paragraphs)
{
    QByteArray out = "<w:" + root + " " + kNs + ">";
    for (const QByteArray &p : paragraphs) out += "<w:p>" + p + "</w:p>";
    return out + "</w:" + root + ">";
}

// ---- headers and footers: editing them

const char kPage[] = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
const char kHeaderType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml";
const char kRelNs[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";

// The text of the header (or the footer) a materialized page shows, with a character for each field.
inline QString shownHeader(const flow::MaterializedPage &page, bool footer = false)
{
    for (const auto &item : page.doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (t->id.startsWith(footer ? QLatin1String("flow-footer-box") : QLatin1String("flow-header-box")))
                if (const Story *s = page.doc->story(t->storyId)) return s->doc->toPlainText();
    return QString();
}

// The text of the header of page `index` of the document.
inline QString headerOfPage(WriteDoc &doc, int index, bool footer = false)
{
    const auto page = doc.pager().materialize(index);
    return page ? shownHeader(*page, footer) : QStringLiteral("<no page>");
}

// Types `text` at the end of a story, a command for each character, as the person does.
inline void typeAtEnd(WriteDoc &doc, QTextDocument *story, const QString &text)
{
    QTextCursor c(story);
    c.movePosition(QTextCursor::End);
    for (const QChar ch : text) insertText(doc, c, QString(ch));
}

// A part of a saved document, and the relationships it has.
inline QByteArray partOf(const QByteArray &zip, const QString &name)
{
    office::Package p;
    p.read(zip);
    return p.part(name);
}

// A section's properties naming the headers and footers given (by relationship Id: "default", "first", and "even" for headers, the same with "f" in front for footers).
inline QByteArray sectionNaming(const QMap<QString, QString> &refs, const QByteArray &more = QByteArray())
{
    QByteArray out = "<w:sectPr>";
    for (auto it = refs.cbegin(); it != refs.cend(); ++it) {
        const bool footer = it.key().startsWith(QLatin1String("f-"));
        const QString type = footer ? it.key().mid(2) : it.key();
        out += QByteArray(footer ? "<w:footerReference" : "<w:headerReference") + " w:type=\"" + type.toUtf8() + "\" r:id=\"" + it.value().toUtf8() + "\"/>";
    }
    return out + kPage + more + "</w:sectPr>";
}

// A package of a document of one paragraph with `background` (the XML of a w:background, or none) before its body.
inline QByteArray makeDocxWithBackground(const QByteArray &background)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + ">" + background + "<w:body><w:p><w:r><w:t>text</w:t></w:r></w:p></w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

// The tables of the document of tables for the random commands, its styles, and its section's properties.
struct RichTables {
    QByteArray styled, plain, styles, sect, spaced;
};

// A text box of a materialized page by what its story says, for the header's and the footer's.
inline const TextItem *boxWithText(const flow::MaterializedPage &page, const QString &text)
{
    for (const auto &item : page.doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (const Story *s = page.doc->story(t->storyId))
                if (s->doc->toPlainText().contains(text)) return t;
    return nullptr;
}

// The red wave pixels in a part of a picture, and where they are: the rectangle is in the widget's own units (what caretRect and pageRect give), the picture
// is what grab() made of it, at whatever ratio the screen has. The wave is a thin red line, so any pixel with much red and little else counts.
inline QRect redBounds(const QImage &img, const QRect &within, int *count = nullptr)
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
inline void selectRange(WriteWindow &w, int from, int to)
{
    QTextCursor c(w.document()->story());
    c.setPosition(from);
    c.setPosition(to, QTextCursor::KeepAnchor);
    w.view()->setCursor(c);
}

// Runs `open`, which makes a modal dialog, and meanwhile runs `fill` on that dialog, then presses OK (Cancel when `accept` is false). `fill` may use QVERIFY and
// QCOMPARE: a failed one leaves `fill`, not the dialog. The dialog is always closed from outside `fill`, and a watchdog closes whatever is still open after ten
// seconds and fails the test, so that a failure ends the test and never leaves it waiting on a dialog (on CI that costs the whole job).
inline void withDialog(const std::function<void()> &open, const QString &objectName, const std::function<void(QDialog *)> &fill, bool accept = true)
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
inline void withFontDialog(WriteWindow &w, const std::function<void(QDialog *)> &fill, bool accept = true)
{
    withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), fill, accept);
}

// The properties of the run, in a saved document's XML, that holds exactly this text (null when there is no such run or it has none).
inline QString runPropsOfText(const QByteArray &documentXml, const QString &text)
{
    const QRegularExpression run(QStringLiteral("<w:r>(?:<w:rPr>((?:(?!</w:rPr>).)*)</w:rPr>)?<w:t(?: xml:space=\"preserve\")?>") + QRegularExpression::escape(text) + QStringLiteral("</w:t>"));
    return run.match(QString::fromUtf8(documentXml)).captured(1);
}
} // namespace
using namespace detail_tst_write;

// A command that changes how paragraphs or characters look, as the ribbon, the shortcuts, and the dialogs run it.
struct LookCommand {
    const char *name;
    std::function<void(WriteDoc &, QTextCursor &)> run;
};

// Where one box that sets a note is: the page, the box, and where in the note it begins.
struct NoteBox {
    int page;
    QRectF rect;
    int from;
};

// What it takes to open a document and to page it, counted: the steps the readers took along the XML's elements, the characters and runs the text engine read, the pages
// laid out, and the rows and cells of tables measured and laid out. (Counted, so that what a busy machine does to the clock changes none of it.)
struct OpenWork {
    qint64 tree = 0, chars = 0, runs = 0, ranges = 0, pages = 0, rows = 0, cells = 0;
    QString text() const { return QStringLiteral("%1 tree steps, %2 characters, %3 runs, %4 ranges, %5 pages, %6 rows, %7 cells").arg(tree).arg(chars).arg(runs).arg(ranges).arg(pages).arg(rows).arg(cells); }
    // Whether this is at most `most` times `base` in every count (a count of nothing in the base is allowed `slack`).
    bool atMost(const OpenWork &base, double most, qint64 slack = 50) const
    {
        auto ok = [&](qint64 mine, qint64 theirs) { return mine <= most * std::max(theirs, slack); };
        return ok(tree, base.tree) && ok(chars, base.chars) && ok(runs, base.runs) && ok(ranges, base.ranges) && ok(pages, base.pages) && ok(rows, base.rows) && ok(cells, base.cells);
    }
};



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
    void cleanupTestCase();
    void initTestCase();
    void theWindowHasItsTabsAndBuildsCleanly();
    void itsIdentityIsItsOwn();
    void ribbonStringsListed();
    void theFileScreenHasItsPages();
    void saveAndOpenKeepTheText();
    void saveACopyKeepsWorkingInTheOriginal();
    void aBackupCopyIsKeptOnSave();
    void aDocumentOpenedReadOnlyCannotBeChanged();
    void aReadOnlyDocumentIsChangedByNoControl();
    void aReadOnlyDocumentIsNeverWrittenOverItsOwnFile();
    void aTemplateOpenedReadOnlyIsOpenedReadOnly();
    void aBackupThatFailsAsksBeforeSaving();
    void theBackupQuestionDefaultsToNo();
    void aFileOpenedAsACopyIsANewDocument();
    void aFileCanBeOpenedInANewWindow();
    void saveAsTemplatePutsADocumentInMyTemplates();
    void aDocumentCanBeSavedAsAnOpenDocumentTemplate();
    void aDamagedDocxOpensRepairedWithWhatCouldBeRead();
    void anOldBinaryFileOpensAndShowsItsText();
    void savingAnOldBinaryFileAsksForADocxName();
    void whatAnOldFileHoldsThatCannotBeShownIsSaid();
    void trackedChangesOfAnOldFileComeInWithTheirAuthorsAndDates();
    void anOldTemplateOpensAsANewDocumentMadeFromIt();
    void aFileThatIsNotADocumentSaysSoAndOpensNothing();
    static QString writeFile(const QTemporaryDir &dir, const QString &name, const QByteArray &bytes);
    void anRtfFileOpensByItsContentAndShowsItsText();
    void savingAnRtfFileWritesAnRtfFileThatReadsBackTheSame();
    void droppingDocumentFilesOpensThemAndAPictureStaysAPicture();
    void aFieldThatNamesAPictureOrAnAddressIsNeverFollowed();
    void anRtfFileWithPartsTheLimitsCutSaysSoAndKeepsTheRest();
    void unsavedWorkIsKeptAndComesBack();
    void theParagraphDialogChangesWhatWasChanged();
    void homeTabHasFindAndReplace();
    void findNextSelectsTheMatchAndScrollsToIt();
    void findFollowsMatchCaseAndWholeWords();
    void findReachesCellsAndPassesHiddenText();
    void replaceReplacesOneMatchAndGoesToTheNext();
    void replaceAllIsOneStepThatUndoesAndSaysHowMany();
    void theFontLauncherAndTheKeyOpenTheDialog();
    void theFontDialogShowsTheLookOfTheSelection();
    void theFontDialogAppliesWhatWasChangedAsOneStep();
    void aFontEffectIsSavedInTheFileAndComesBack_data();
    void aFontEffectIsSavedInTheFileAndComesBack();
    void anUnderlineStyleAndColorFromTheDialogAreSavedInTheFileAndComeBack();
    void aFilesEffectsAreReadAndWrittenBack();
    void aFailedCheckInADialogTestStillClosesTheDialog();
    void theWelcomeTopicDoesNotSayHeadersAreNotShown();
    void theWelcomeTopicSaysWhatThisVersionDoesAndDoesNotDo();
    void aFontBoxOverSelectionsThatDifferShowsNothingAndWhatIsSetGoesToAll();
    void anUnderlineColorSetOverUnderlinesThatDifferColorsOnlyTheUnderlines();
    void anAtomInTheSelectionDoesNotMakeAnEffectPartial();
    void anUnderlinesFlagsAndNamesMapBothWays();
    void replaceAllInOneLongParagraphTakesTimeInStepWithTheMatches();
    void findAndReplaceSeeWordsThatABookmarkSplits();
    void replacingFromTheLastMatchGivesWhatReplacingFromTheFirstGave();
    void exportPdfWritesAPdfWithAPagePerPage();
    void pagesToPrintFollowTheRange();
    void pageRangesAreReadAsTheyAreTyped();
    void copiesComeOutCollatedOrEachPageTogether();
    void thePaperIsDescribedAsTheSectionSetsIt();
    void thePrintScreenHasThePrinterCopiesCollateRangeAndPreview();
    void printingGoesThroughThePainterThePdfExportUses();
    void printingTakesThePagesOfTheRangeInTheOrderGiven();
    void severalPagesShareASheetAndAPageCanBeScaledToAPaperSize();
    void aPrinterNamedPdfIsAPrinterAndTheFileEntryIsFoundByItsRole();
    void savingAsPdfThroughPrintLeavesAnExistingFileWholeUntilTheNewOneIsDone();
    void copiesAndCollateReachAPrinterThatCannotMakeThem();
    void helpTopicsAreComplete();
    void f1OpensHelpOnWhatIsBeingDone();
    void theViewShowsTheFirstPageAtOnceAndLaysOutTheRestInTheBackground();
    void aPageShowingTheTotalIsMadeAgainWhenTheTotalChanges();
    void onlyThePagesInViewAreMade();
    void zoomRunsFromTenToFiveHundredPercent();
    void thePagesAreDrawnWithTheirText();
    void viewGridlinesDrawsTheEdgesOfCellsThatHaveNoLines();
    void draggingAColumnBorderSetsTheColumnsWidth();
    void thePenDrawsATableAndDividesItsCellsAndTheEraserJoinsThem();
    void draggingARowBorderSetsTheRowsHeight();
    void aPressOnABorderIsAClickUntilItMovesAndEscapeLetsGo();
    void movingThePointerOverATableMakesItsCellBoxesOnce();
    void draggingTheTableHandleSetsTheTablesWidth();
    void setTableWidthKeepsTheColumnsProportions();
    void modifyingATableStyleRestylesNestedAndManyTables();
    void builtInTableStylesAreShownByOurOwnNames();
    void aFilesOwnTableStyleWinsOverTheBuiltInOfTheSameName();
    void aTableStyleIsMadeAndModifiedFromChoices();
    void readyTablesPutATableWithItsWordsIn();
    void theCaretIsDrawnOnThePageThatHoldsIt();
    void theCaretAndClicksIgnoreTheHeaderAndTheLineNumbers();
    void clickingPlacesTheCaretAndDraggingSelects();
    void theCaretAndTheMouseWorkInCells();
    void theKeysMoveTheCaret();
    void pageUpAndPageDownMoveByAWindow();
    void upAndDownCrossFromPageToPage();
    void typingInTheWindowReachesTheDocument();
    void spellCheckFindsTheMisspelledWordsOfTheStory();
    void spellCheckKeepsItsAnswerForEachParagraph();
    void spellCheckGivesWayWhenThePassIsOutOfTimeAndFinishesInIdleTime();
    void spellCheckFollowsTheLanguageOfTheText();
    void theIgnoreListAndTheUserDictionaryChangeTheAnswers();
    void misspellingsAreFoundByPositionAndInOrder();
    void misspelledWordsAreUnderlinedInRedOnTheScreenAndNotInTheProof();
    void misspelledWordsInATableCellAreUnderlinedAndThoseInAHeaderAreNot();
    void aWordStillBeingTypedIsNotUnderlinedUntilTheCaretMovesOn();
    void onlyTheParagraphsOnThePagesInViewAreChecked();
    void theSpellingMenuOffersSuggestionsIgnoreAllAndAddToDictionary();
    void rightClickingAMisspelledWordInTheWindowShowsItsSuggestions();
    void reviewSpellingStepsThroughTheMisspellingsFromTheCaret();
    void reviewSpellingBeginsAtTheCaretAndComesBackToWhereItBegan();
    void hostileDocumentTextIsShownAsText();
    void theSpellingPaneNeverHoldsTheWindowForAWholeDocument();
    void changeAllGoesOnFromTheWordWhateverTheEarlierOnesChangedBy();
    void checkingSpellingAsYouTypeCanBeTurnedOffInTheReviewTab();
    void aStyleThatSetsALanguageGivesItToTheTextAndTakesItAway();
    void searchingForMisspellingsGoesInSlicesOfBoundedWork();
    void aLongParagraphIsCheckedInPiecesOfBoundedWork();
    void theSpellingCacheDoesNotKeepTheTextOfTheDocument();
    void theUserDictionaryIsReadAndWrittenSafely();
    void addingAWordThatCantBeSavedStillCountsAndIsSaidOnce();
    void theNamesOfTheSuiteAreNeverFlaggedInAnyLanguage();
    void theWordCountSaysWordAndWordsInEnglish();
    void countedTextHasItsEnglishFormsAndNoneIsLeftAsPluralMarker();
    void rightClickingInsideASelectionOffersTheWordClickedNotTheCaret();
    void aMisspelledWordInATableCellIsUnderlinedLikeAnyOther();
    void theRibbonCommandsDoTheirWork();
    void developerRoundTripOfAFolder();
    void developerStrictRoundTrip();
    void developerSectionTimings();
    void developerScreenshots();
    void aboutListsTheThirdPartySoftware();
    void typingIsOneStepThatUndoesAndRedoes();
    void anEditCostsTheCommandNotTheDocument();
    void enterAndBackspaceSplitAndJoinParagraphs();
    void aHeadingIsFollowedByBodyText();
    void characterFormatsApplyToTheSelectionAndSurviveASaveAndOpen();
    void paragraphFormatsApplyToTheParagraphsSelected();
    void listsAreTurnedOnAndOffAndKeptInTheFile();
    void aPageBreakIsACharacterOfItsParagraphAndTheFileKeepsItWhole();
    void pageSetupOnTheLastSectionUndoesAndMarksTheDocumentModified();
    void undoingAStyleOnTheFirstParagraphRestoresItsMark();
    void unknownAttributesOnKnownElementsGoBackAsRead();
    void aFilesHeadersAndFootersAreDrawnAndKept();
    void sectionsTakeTheHeadersOfTheSectionBefore();
    void typingInAHeaderChangesEveryPageThatUsesItAndOnlyThere();
    void anUneditedHeaderIsWrittenBackByteForByte();
    void anEditedHeaderKeepsWhatTheProgramCannotEdit();
    void aFloatingShapeInAHeaderStaysWhereItIsWhenTheHeaderIsEdited();
    void theFieldsOfAHeaderAreOneCharacterEach();
    void aFieldTakesTheLookAndTheTextBesideItDoesNotEnterIt();
    void insertPageNumberMakesALivePageField();
    void linkToPreviousSharesAndUnlinksAHeader();
    void differentFirstAndOddEvenPagesHaveASlotEach();
    void aNewHeaderIsAValidPartWithItsRelationship();
    void oneHistoryHoldsTheTextAndTheHeaders();
    void aSwitchBetweenStoriesEndsABurst();
    void aHeaderPartTheSectionsShareIsOneStoryAndOnePart();
    void aHeaderPartNamedBySeveralHundredSectionsIsBounded();
    void aReferenceToAMissingPartIsKeptAndCanBeReplaced();
    void aNewHeaderInAFileWithoutTheRelationshipsNamespaceDeclaresIt();
    void anEmptyHeaderPartHasAParagraphToEdit();
    void aHugeHeaderKeepsEveryParagraphWhenOneIsEdited();
    void anUnreadableHeaderPartIsLeftAsItIsAndNotReplaced();
    void morePartsThanTheProgramReadsAreKeptAsTheyAre();
    void doubleClickingInTheHeaderAreaEntersAndEscapeOrADoubleClickInTheTextLeaves();
    void theSlotOfThePageDecidesWhichHeaderIsEntered();
    void insertHeaderAndFooterMakeABlankOneAndEnterIt();
    void theHeaderAndFooterTabNavigatesAndInsertsDatesAndSetsNumbering();
    void removeHeaderAndFooterLetGoOfTheSectionsOwnAndBlankWhatItWouldShare();
    void headerDesignsFillAHeaderWithFieldsAndTabsInOneStep();
    void documentInformationIsPutInAsFields();
    void theHeaderAndFooterTabHasTheOptions();
    void editingInAHeaderWorksAsInTheText();
    void aTallerHeaderPushesTheTextDownOnEveryPageThatUsesIt();
    void anEmptyHeaderParagraphWithNoStyleHasTheDefaultSizeWhereTheFileHasNoHeaderStyle();
    void theHeaderMarkSizeIsNotWrittenBack();
    void onlyALinksFieldResultCarriesTheNoBreakMark();
    void theViewFollowsTheHistoryIntoAndOutOfAHeader();
    void spellingMarksShowInAHeaderWhileItIsEdited();
    void anEditedHeaderIsInTheRecoveryCopy();
    void helpHasATopicForHeadersAndFooters();
    void helpHasATopicForFootnotesAndEndnotes();
    void aHeaderBeingEditedIsShownInTheWindow();
    void aHeaderBeingEditedKeepsTheResultOfAFieldThatShowsAHeading();
    void aPageSaysWhereItsHeaderAndFooterAreAndWhichStoryTheyShow();
    void aTableInAHeaderKeepsItsPlaceAndItsCellTextIsEdited();
    static std::unique_ptr<WriteDoc> listAndHeaderDoc();
    void anEditThatRenumbersAListIsOneStepOfTheHistory();
    void randomEditsThatRenumberKeepTheOrderOfTheHistory();
    void findNextWithAHeaderOpenLeavesTheHeader();
    void aCursorInTheTextGivenToTheViewLeavesTheHeader();
    void theSpellingPaneWithAHeaderOpenLeavesTheHeader();
    void aHeaderTakesNoSectionBreakPageBreakOrNote();
    void theBreakCommandsAreGrayedWhileAHeaderIsEdited();
    void copyingHeaderTextKeepsWhatItsFieldsShow();
    void enteringAHeaderForEditingLeavesThePageAsItWas();
    void aHugeTableInAHeaderIsBoundedWhenItIsEntered();
    static const QByteArray & noteStyles();
    static QByteArray footnotesPart(const QList<std::pair<int, QByteArray>> &notes);
    static QByteArray plainRun(const QByteArray &text);
    static QByteArray citing(const QByteArray &text, int id);
    static QByteArray pageSection();
    static QByteArray notesDocx(const QByteArray &body, const QByteArray &notes, const QByteArray &settings = QByteArray());
    static QByteArray threeNotesBody();
    static QList<std::pair<int, QByteArray>> threeNotes();
    static QByteArray threeNotesDocx();
    static QList<std::pair<int, int>> notesOnPage(WriteDoc &doc, int index);
    static QString noteMark();
    void aFootnoteIsEditedInItsOwnStoryAndTheHistoryHoldsItsSteps();
    void theNumberMarkOfANoteIsOneCharacterThatNoEditTakes();
    void aFootnoteIsEditedAsTheTextIs();
    void thePagesSayWhichStoryEachOfTheirNotesShows();
    void editingAFootnoteLaysThePagesOutAsANewPaginatorDoes();
    void insertFootnoteMakesAReferenceAndANoteAndMovesTheCaretIntoIt();
    void laterNotesAreRenumberedAsReferencesComeAndGo();
    void deletingAReferenceDeletesItsNoteAndUndoBringsBothBack();
    void aNoteCitedTwiceStaysUntilItsLastReferenceGoes();
    void cuttingAndPastingAReferenceMovesItsNote();
    void aFootnoteIsNotInsertedInAHeaderOrInANote();
    void anUneditedFootnotesPartIsWrittenByteForByte();
    void anEditedFootnoteKeepsItsAtomsAndWritesBackAsTheNoteThatWasEdited();
    void aNewFootnotesPartIsValidAndReadsBack();
    void aFootnoteInsertedAndTakenBackLeavesTheFileAsItWas();
    static QPoint pagePoint(PageView &view, int page, double x, double y);
    static bool noteBoxOf(WriteDoc &doc, int page, int id, QRectF *rect);
    void clickingInAFootnoteOnThePagePutsTheCaretThereAndEditsAreTheNotes();
    void theLooksOfTextAreGivenToWordsInAFootnote();
    void aClickOnTheNumberMarkOrTheSeparatorEditsNothing();
    void upAndDownAndAClickInTheTextLeaveTheNote();
    void oneHistoryInterleavesTheTextAHeaderAndANote();
    void insertFootnoteIsOnTheReferencesTabAndMovesTheCaretIntoTheNote();
    void aFootnoteBeingEditedIsShownInTheWindow();
    void aNoteThatGoesOnFromPageToPageIsEditedAnywhereAndThePagesFollow();
    static QVector<QTextBlock> endnoteParagraphs(WriteDoc &doc, int id);
    static QStringList endnoteMarks(WriteDoc &doc);
    static QStringList endnoteReferenceNumbers(WriteDoc &doc);
    static const QByteArray & betaNote();
    static const QByteArray & sourcesNote();
    void theWordsOfAnEndnoteAreEditedWhereTheyStand();
    void noEditReachesTheSeparatorTheNumberOrAnotherEndnote();
    void anEndnoteIsEditedAsTheTextIs();
    void insertEndnoteMakesAReferenceAndANoteAndNumbersTheOthersAnew();
    void deletingAnEndnoteReferenceDeletesItsNoteAndUndoBringsBothBack();
    void cuttingAndPastingAnEndnoteReferenceMovesItsNote();
    void endnotesAtTheEndOfASectionAreEditedAndMadeThere();
    void aNewEndnotesPartIsValidAndReadsBack();
    void thousandsOfFootnotesAreEditedWithinBounds();
    void aReferenceToAMissingNoteIsKeptAndNoNewNoteTakesItsId();
    void aHugeFootnoteKeepsEveryParagraphWhenItIsEdited();
    void notesInAHeaderAndInATableAreKeptAndEdited();
    void anUnreadableFootnotesPartIsLeftAsItIsAndNoNoteIsMadeInIt();
    void twoNotesWithOneIdAreOnlyTheFirstEditedAndTheOtherIsKept();
    void thousandsOfEndnotesAreEditedWithinBounds();
    void aReferenceToAMissingEndnoteIsKeptAndNoNewNoteTakesItsId();
    void anEndnoteCitedTwiceIsEditedInItsFirstCopyOnly();
    void aHugeEndnoteKeepsEveryParagraphWhenItIsEdited();
    void anUnreadableEndnotesPartIsLeftAsItIsAndANoteInATableCellWorks();
    void clickingInAnEndnotePutsTheCaretInItsWordsAndTheKeysLeaveIt();
    void insertEndnoteIsOnTheReferencesTabAndMovesTheCaretIntoTheNote();
    void findAndSpellingWithAFootnoteOpenLeaveIt();
    void theRunsOfAFieldKeepTheirOwnProperties();
    void anEditThatIsUndoneLeavesTheFileAsItWas();
    void aCopyMadeByUnlinkingHasIdsOfItsOwn();
    void aDroppedPartTakesItsOwnPicturesWithIt();
    void aSelfClosingSettingsPartTakesTheOddEvenSetting();
    void aDoubleClickInAMarginWithNoHeaderMakesNothingUntilSomethingIsTyped();
    void aBlankPageOfAnOddPageSectionIsNotEntered();
    void theFontTableGivesTheStandIns();
    void compatibilityOptionsOfLineSpacingReachTheLayout();
    void aLongFontTableCostsInStepWithItsSize();
    void theCompatibilityOptionThatSuppressesTheSpaceBeforeAfterABreakIsKept();
    void footnoteNumbersRestartWhenTheSettingsSaySo();
    void pageNumberFormatsMapBothWays();
    void punctuationHangsOnlyWithTheEastAsianLayout();
    void theMarkAfterABreakFollowsTheFilesOption();
    void whatATrackedChangeDeletedTakesNoRoom();
    void whatAFileSaysOfHiddenTextOnDeletedTextIsSavedAsItWas();
    void theRoundTripCheckSeesLostHiddenAndDeletedText();
    void textTypedBesideADeletionIsOutsideIt();
    void editingJoinsTheParagraphsThatShowAcrossADeletedOne();
    void aSelectionAtADeletedParagraphLeavesNoTextInIt();
    void aRunThatAsksForLigaturesIsReadSo();
    void overflowPunctOffIsReadFromTheParagraphAndItsStyle();
    void clearingFormattingKeepsDeletedTextDeleted();
    void aDeletedEndnoteReferenceTakesNoNumber();
    void endnoteReferencesShowTheFormatTheSettingsGive();
    void endnoteMarksFollowTheFormatsTheNumberAndTheSection();
    void theEndnotesNumberingIsReadOnceForTheDocument();
    void autoHyphenationFollowsTheSettingsAndTheParagraphsMayRefuseIt();
    void hyphenatedWordsBreakAtTheirPointsAndALongStringIsNotLookedUp();
    void hangingIndentsAreNoTabStopsWhereTheFileSaysSo();
    void paragraphSpaceAddsInAFileThatTurnsTheWebWayOff();
    void automaticSpacingIsTheStatedSpacingInAFileThatTurnsTheWebWayOff();
    void automaticSpacingIsFivePointsWhereAFileStatesNone();
    void aPageOfALongParagraphCopiesOnlyItsOwnText();
    void aRaisedSymbolMakesItsLineTallerByWhatItReachesAbove();
    void spacingSetOnAutomaticSpacingReplacesIt();
    void aHeadersSpaceAfterTakesRoomOnThePage();
    void aParagraphAfterOneWithADeletedMarkThatShowsNothingTakesNoRoom();
    void aParagraphWhoseMarkWasDeletedIsOneParagraphWithTheNext();
    void aLongRunOfJoinedParagraphsIsCutIntoParagraphsOfThirtyTwo();
    void aJoinedRunIsCutAtTheSamePlacesWhereverALayoutBegins();
    void thePagesCopiesOfAJoinedRunHoldOnlyTheirOwnText();
    void aPageOfThousandsOfRunsIsCopiedInOneEdit();
    void theLinesOfAParagraphOfThousandsOfRunsReadOnlyTheirOwnRanges();
    void aPageFarDownAParagraphOfAHundredThousandRunsIsCopiedQuickly();
    void aPageBreakEndingAJoinedRunGoesOnAfterTheRun();
    void anEditOfTheLastParagraphOfAJoinedRunLaysOutAgainFromTheFirst();
    void theNumberOfAJoinedRunIsSetFromItsLastParagraph();
    void backspaceAndDeleteAtTheJunctionOfAJoinedRunKeepEveryCharacter();
    void aDeletedMarkNeverJoinsTheParagraphsOfTwoCells();
    void aPasteFromAnotherDocumentLeavesNoDanglingNoteOrObject();
    void aSymbolOfAMissingFontIsSavedAsItWas();
    void aSymbolOfASymbolFontIsSetInThatFont();
    void aSymbolOfASymbolFontIsDrawnWithRoomOfItsOwn();
    void aSymbolTakesItsAdvanceFromItsFont();
    void aFieldResultOfBlanksTakesNoPartInTheLineHeight();
    void blanksTypedAfterABlankFieldResultCountInTheLineHeight();
    void aStyleWithNoIdIsFoundByItsName();
    void aSymbolSetsTheLineOfItsFont();
    void aLinkInATableOfContentsEntryIsNotMarkedAsALink();
    void aPageNumberRunIsTheNumberOfThePage();
    void theRoundTripCheckSeesAGainedRunProperty();
    void theRoundTripCheckReadsPropertiesAsAReaderDoes();
    void capitalsAndDoubleStrikeOverTheStylesOtherKindAreSavedAsTheFileSaidThem();
    void aToggleOfACharacterStyleFlipsTheParagraphStylesAndADirectOneStaysSaid();
    void aParagraphStyleIsSavedSaidWhenTheFileFlagsNoDefault();
    void anEmptyBreakParagraphBeforeAContinuousSectionTakesNoRoom();
    void anEmptyBreakParagraphThatBeginsTheDocumentKeepsItsLine();
    void theColumnsAfterAnEmptyBreakParagraphBeginWhereTheTextAboveEnds();
    void anEmptyBreakParagraphAtTheFootOfAFullPageStaysThere();
    void aKeptParagraphThatBeginsWithAPageBreakStaysOnTheFullPage();
    void aSectionsPageBordersAreRead();
    void anAdvanceFieldSetsItsParagraphAtThePlaceFromThePagesTop();
    void anAdvanceFieldPastTheFootOfThePageLeavesNoTextOut();
    void aLongChainOfStylesCostsNothingForEachParagraph();
    void aSectionBreakMadeInAnOddPageSectionBeginsOnTheNextPage();
    static QByteArray askingParagraphs();
    static double askedPlace(WriteDoc &doc, int block);
    void aStyleAppliedToAParagraphThatAsksForAPlaceKeepsTheAsk();
    void enterInAParagraphThatAsksForAPlaceLeavesTheAskWithTheFirstHalf();
    void takingOutAnAdvanceFieldTakesTheAskAway();
    void aFilesPageColorFillsItsPages();
    void thePageColorIsInThePdfExportAndNotOnAPrinter();
    void aSymbolOfAMissingFontMakesItsLineTheStandIns();
    void aNegativeMultipleOfTheLineIsReadAsItsSize();
    void aListNumberHasTheSizeItsLevelGives();
    void aFrameParagraphInAFooterTakesNoRoom();
    void aHeadersWrappedShapeMovesTheTextBelowIt();
    void aHeaderFrameSetAtAPlaceOnThePageIsATextBoxThere();
    void aHeaderFrameOfTooManyParagraphsIsTextOfTheHeader();
    void theFramesOfAHeaderCostNothingForEachPage();
    void theWrappedShapesOfAHeaderCostNothingForEachPage();
    void aRightTabIntoTheMarginTakesTheTextAfterItsTab();
    void tabsThatEndAParagraphCauseNoWrap();
    void trailingTabsThatAreUnderlinedAreDrawn();
    void aRunBorderAndTheFirstEntryOfADropDownAreShown();
    void aParagraphOfThousandsOfUnendedDropDownsIsScannedOnce();
    void theBodySelectionIsNotPaintedInAHeadersTable();
    void aRunBorderIsClampedAndReadFromStyles();
    void theTablesOfAHeaderAreLaidOutOnceUnlessFieldsMakeThemDiffer();
    void anObjectInAHeaderWithATableIsDrawnAndHoldsTheTextBelowIt();
    void theSectionPagesOfAHeadersTableAreTheSectionsOwn();
    void aTableInAHeaderOrAFooterIsDrawnAndTakesRoom();
    void aDeletedParagraphOfAFooterTakesNoRoom();
    void aNoteThatGoesOnHasTheFilesNoticeAndSeparatorSet();
    void aLongNoticeAndSeparatorAreSetInTheRoomThatWasKept();
    void theRuleOverAContinuedNoteBeginsAfterTheWordsBeforeItsCharacter();
    void aHugeNoticeIsBoundedWhenItIsRead();
    void aVisibleSymbolInANoteIsKept();
    void aNoteThatBeginsWithARaisedSymbolIsTallerByWhatItReaches();
    void aFilesFootnotesAreAtTheFootOfItsPages();
    void deletedEmptyParagraphsAtTheStartOfANoteTakeNoRoom();
    void aNoteOnThePageKeepsTheDeletedMarkOfItsFirstParagraph();
    void aFramePropertyOfAPageInANoteChangesNothing();
    void theNoteSeparatorIsTheFilesOwnParagraph();
    void autoSpacingIsNoneInAListAndFourteenBetweenLists();
    void aLineIsNotBrokenAfterASlash();
    void aRuleAnchoredAtAParagraphsEndIsDrawnAsTheFileSaysIt();
    void aPictureIsCutKeyedAndBordered();
    void letteringOnAPathIsTextArtInTheShapesColors();
    void aHorizontalRuleIsAsWideAsItsShareOfTheText();
    void aShapeOfAKindIsThatShape();
    void aPictureWithACompressedFormIsShownAsMissing();
    void aHiddenRowIsNotShownAndTakesNoRoom();
    void aRowInsertedBesideAHiddenRowTakesANormalHeight();
    void automaticTextIsDrawnWhiteOnADarkFillAndNothingIsSaved();
    void theFillNearestTheTextDecidesItsAutomaticColor();
    void aRulesWidthFollowsItsSectionAndItsIndents();
    void numbersInAFileThatAreNotWholeOrNotFiniteAreClampedOrIgnored();
    void letteringIsBoundedInCountLettersAndSize();
    void letteringDrawnAgainCostsLittle();
    void aPicturesBorderHasTheSidesTheFileSets();
    void aTableInATextBoxIsLaidOutInTheBox();
    void aPictureFillOfNoOpacityIsNotDrawn();
    void aTableInATextBoxOfAHeaderIsLaidOutInTheBox();
    void aTextBoxClipsItsTable();
    void theTablesOfAHeadersTextBoxesAreNotLaidOutAgainOnEachPage();
    void theTablesOfManyTextBoxesAreBoundedInAll();
    void aShapeFilledWithAPictureKeepsItsTextAndItsOutline();
    void aBigKeyedPictureIsKeyedAtADrawnSize();
    void anArcHasTheAnglesItSays();
    void drawnObjectsSurviveASaveAndReopen();
    void aBulletRaisesItsLineByItsAscentOverTheTextsDescent();
    void aFootnoteWithACustomMarkTakesNoNumber();
    void aSectionBreakDoesNotKeepNoteRoomOnThePageBefore();
    void aContinuationNoticeKeepsItsRoomUnderTheNotes();
    void theContinuedPartOfANoteIsSetUnderTheFilesContinuationSeparator();
    void theNoteRuleIsThreeFifthsDownTheSeparator();
    void aTallNoteSeparatorIsCutToAQuarterOfThePage();
    void aNumbersTabSkipsTheHangingIndentWhenTheFileSaysSo();
    void aNumberWithNoListTabGoesToTheNearestStopOrTheHangingIndent();
    void aParagraphsOwnListTabReplacesTheLevelsAndOneBeforeTheNumberIsNone();
    void numberingIdsOfOneDefinitionCountOnTogether();
    void aLegacyNumbersTextBeginsAtTheIndentWhateverTheTabStopOption();
    void spaceShrinkingIsWorkedOnlyWhereALineCouldTakeAnotherWord();
    void spaceShrinkingIsSharedWithThePageThatDrawsTheParagraph();
    void aLineThatExactlyFillsItsRoomStaysOne();
    void fullJustificationTakesInAWordWhereTheSpacesCanGiveWay();
    void aListQtCanCountIsNotGivenMarkersOfItsOwn();
    void editingASharedListNumbersTheRestAgain();
    void aRestartAtOneLevelRestartsThatLevelOnly();
    void definitionsLinkedToOneStyleShareItsLevelsAndItsCount();
    void aLevelWrittenOutInANumberingIdIsUsed();
    void anItemAfterItsDeeperLevelsGoesOnFromTheLevelsStart();
    void aNumberIsInTheSizeOfItsParagraphMark();
    void aNumberIsInTheTypefaceOfItsLevel();
    void aLevelThatSaysNotBoldHasNoBoldNumber();
    void aNumberIsInTheColorOfItsLevel();
    void theCheckBoxOfAnOutlinedBulletIsHollow();
    void theNotesOfAPageAreLaidOutTheSameWithATableInTheDocument();
    void aCustomMarkTakesNoNumberAmongTheNotesOfATablesPage();
    void clearingTheFormattingKeepsACustomNoteMark();
    void aLevelsOutlineEffectIsLookedForOncePerLevel();
    void theNoteSeparatorIsCutPerSection();
    void aCustomNotesOwnReferenceMarkShowsNothing();
    void anEndnoteWithACustomMarkTakesNoNumber();
    void listsAreToldApartWhateverTheLevel();
    void theOutlineEffectReachesNumbers();
    void anOutlinedBulletOfAMeasuredFontIsItsBoxOutlined();
    void editingSectionsTouchesOnlyWhatChanged();
    void aSectionThatEndsTheFileLeavesABlankLastPageThatIsDrawn();
    void sectionsHaveTheirOwnPages();
    void aSectionBreakInTheMiddleOfTextAlsoUndoes();
    void copyAndPasteKeepTheFormats();
    void piecesKeptAsXmlStayWhenTheTextAroundThemChanges();
    void theStyleOfAParagraphIsWhatTheFileSaysAndNothingMore();
    void aDocumentOpensAndPaginates();
    void listsInTheFileAreLists();
    void whatIsNotUnderstoodIsKeptWhereItWas();
    static QByteArray tableDocx(int rows, int cols, const QByteArray &cellExtra = QByteArray());
    static QVector<double> rowHeightsOf(const QByteArray &body);
    static QByteArray lineX(const char *side, const char *val, int sz);
    void theLinesOfATableTakeRoomInItsRowsAsTheFileGivesThem();
    void aTableWithSpaceBetweenItsCellsHasItsLinesRoundThem();
    void aTripleLineTakesRoomForItsThreeLinesAndTheSpaces();
    static void checkWhole(WriteDoc &doc);
    void tabEnterAndDeleteWorkInCells();
    void tablesPastTheLimitsAreKeptWholeAndOnesAtThemWork();
    void aSectionBreakIsNotMadeInACell();
    void rowsAndColumnsAreAddedAndTakenAway();
    void rowsAndColumnsKeepJoinedCellsWhole();
    static QByteArray cellP(const QByteArray &text);
    static QByteArray cellX(const QByteArray &content, const QByteArray &pr = QByteArray(), int width = 2400);
    static QByteArray rowX(const QByteArray &cells, const QByteArray &pr = QByteArray());
    static QByteArray tableX(const QByteArray &rows, int columns, int columnWidth = 2400, const QByteArray &width = "<w:tblW w:w=\"0\" w:type=\"auto\"/>");
    static QString storyText(WriteDoc &doc);
    static QTextCursor cursorAt(WriteDoc &doc, const QString &text);
    void deletingColumnsTakesOnlyTheCellsInThem();
    void gapsInRowsFollowTheRowAndColumnCommands();
    void newRowsAndCellsAreLikeTheOnesBesideThem();
    void aParagraphInACellStaysThereWhateverIsDoneToItsFormat();
    void rowsAddedBesideEmptyCellsKeepTheOrderOfTheirCells();
    void tabInTheLastCellOfAJoinedLastRowAddsARowAfterTheTable();
    static QString tablesSignature(WriteDoc &doc);
    static void checkTablesReopenTheSame(WriteDoc &doc);
    static int selectCells(WriteDoc &doc, QTextCursor &c, const QString &from, const QString &to);
    void insertTableMakesATableThatSavesAndOpensTheSame();
    void insertTableCutsAParagraphAtTheCursorAndKeepsTheFitItWasGiven();
    void mergeCellsAcrossAndSplitThemAgain();
    void mergeCellsOverRowsAndColumnsAndSplitThem();
    void splitCellsMakeColumnsAndRows();
    void convertTextToTableRefusesHostileTextBeforeBuildingAnything();
    static QString rangeText(WriteDoc &doc, int from, int to);
    void convertTextToTableKeepsNotesCommentsAndBookmarks();
    void convertTextToTableAndBack();
    void convertTextToTableByParagraphsOtherCharacterAndAtTheEnd();
    void splitTableCutsAboveTheCursorsRow();
    void splitAndSortKeepHeaderRowsAndSpannedCells();
    void sortTableRowsByTextNumbersAndDates();
    void aBookmarkNamedLikeACellWinsInAFormula();
    void manyFormulasOverAHugeRangeFinishInBoundedTime();
    void updatingManyFormulaFieldsBuildsTheIndexOnce();
    void tableCellValuesAreFoundByNameAndDirection();
    void shadingAndBordersOfTheSelectedCellsAreWrittenAndUndone();
    void cellAlignmentIsAcrossInTheParagraphsAndDownInTheCell();
    void cellTextDirectionIsKeptInTheCellAndInTheFile();
    void cellSizeAndFitAreWrittenWithTheGrid();
    void tablePropertiesAreReadSetAndWritten();
    void aTableStyleAndItsOptionsGiveTheCellsTheirLook();
    void rowsAndColumnsAddedToAStyledTableAreDressedAsTheFileOpensThem();
    void aTablesRecordsAreFreedAsItIsChanged();
    void commandsThatWouldPassTheReadersLimitsAreRefused();
    void splittingAndSizingKeepTheGapsOfTheOtherRowsRight();
    void aBuiltInTableStyleIsNotAddedAsASecondDefault();
    void undoOfATableStyleTakesTheAddedStyleBackToo();
    void builtInTableStylesHaveTheConventionalIds();
    void tablePropertiesKeepWhatThePersonDidNotChange();
    void aColumnsWidthCanBeGivenAsAPercent();
    static void answerDialog(const std::function<void(QDialog *)> &fill);
    void mergingKeepsATableInACellIntact();
    void mergingKeepsThePiecesKeptBetweenCells();
    void aBookmarkBetweenRowsStaysWhenATableIsChanged();
    void everyTableCommandRunsOnHostileTables();
    void theCaretMovingAboutALongTableLooksAtItsTablesOnce();
    void aJoinOfTwoCellsLooksAtTheCellsNextToThem();
    void fittingColumnsToTheirCellsMeasuresAFontOnce();
    void aLineInSmallerTypeIsKeptClearOfAFrameByTheHeightThatWasChecked();
    void undoPastADeleteAcrossCellsFindsItsRecordsAfterManyTableCommands();
    void aCommandThatWouldPassFiftyThousandCellsIsRefused();
    void splittingWhereItCanNotBeDoneSaysWhy();
    void splittingACellJoinedDownAsksForRowsAsItIsJoined();
    void changesKeepATableInACellAndWhatIsAroundIt();
    void deletingAcrossCellsEmptiesThemAndTakesWholeTablesOut();
    void pastedParagraphsAreInTheCellTheyWerePastedInto();
    void aSavedDocumentRoundTripsItsOwnParagraphFormatting();
    void kerningComesFromTheFilesKernSetting();
    void hostileKernValuesAreReadWithoutHarm();
    void whatAParagraphSaysThatHasNoFieldIsKept();
    void tabStopsTheEngineHasNoKindForAreKeptAndNotMade();
    void aKindOfJustifiedIsKeptUntilTheAlignmentIsChosen();
    void aParagraphMarkThatSaysItsSizeKeepsSayingItUnderACharacterStyle();
    void aStyleThatNumbersKeepsItsOwnIndentsOverTheList();
    void theFontsOfOtherScriptsAreKeptAsTheRunSaidThem();
    void aSectionThatStartsPageNumbersAtZeroKeepsSayingIt();
    void aParagraphThatSaysNoFirstLineIndentKeepsSayingIt();
    void hostileDocumentsAreHeldToLimits();
    void openingDoesNotSlowDownWithUniqueIdsAndBookmarks();
    void aRootWithDeclarationsOnSeparateLinesGetsNoneTwice();
    void aNamespaceDeclaredOnAParagraphStaysDeclared();
    void theLinksOfTheStylesAndTheListsAreKept();
    void bookmarksAndRangesAcrossKeptTablesAndOfRepeatedIdsStay();
    void aTableNobodyTouchedComesBackAsTheFileHadIt();
    void enterInsideAWrapperKeepsItOnBothHalves();
    void enterAtTheEndOfAParagraphMovesTheMarksPropertiesToTheNewOne();
    void backspaceIntoAParagraphWithXmlBeforeItKeepsTheXml();
    void pastingBeforeAHeadingThatEndsASectionLeavesItAlone();
    void pastingFromAnotherDocumentCarriesPicturesAndLinksAndNothingDangles();
    void aStrayStartOfATagPastedInsideAnotherLeavesTheOthersWhole();
    void savingADocumentWithMacrosLeavesThemOutAndSaysSo();
    void aFileThatCouldNotBeReadWholeIsNotSavedOverItsOriginal();
    void aFileLeftPartsOutIsNotSavedOverItsOriginalByAnotherName();
    void noTableIsMadeInTheWordsOfAnEndnote();
    void noBlocksOfFieldsOrCaptionsAreMadeInTheWordsOfAnEndnote();
    void theTableCommandsAreDimmedInTheWordsOfAnEndnote();
    void partsWithPercentSignsAndSpacesInTheirNamesAreKept();
    void tablesNestedPastTheLimitOpenWithTheTextBelowAsPlainParagraphs();
    void aRichDocumentComesBackFaithfulAndWellFormed();
    void randomEditingOfARichDocumentKeepsWhatItHolds();
    static OpenWork openAndPageWork(const QByteArray &zip, int *pages = nullptr, QStringList *notes = nullptr, std::unique_ptr<WriteDoc> *keep = nullptr);
    static QByteArray manyCells(int rows, int columns);
    void hostileTablesOpenInTimeThatGrowsWithTheirCells();
    void aCellOfThreeThousandLinesOpensAndIsPagedToTheEnd();
    void tablesNestedPastTheLimitAreLaidOutDownToIt();
    static QByteArray wallRun(int kind, int left, int top, int w, int h);
    static RichTables richTables(const QByteArray &noteInCell = QByteArray());
    static QByteArray floatingTable(int wide);
    static QByteArray richTableDocx();
    static QByteArray richMixedDocx(int gammaWords = 220, bool endnotes = true);
    static QString layoutDiffersFromFresh(WriteDoc &doc, bool made = true);
    static QString layoutDump(WriteDoc &doc);
    static QString storyState(WriteDoc &doc);
    void randomTableTabCommandsKeepTablesWholeAndOpenTheSame();
    void randomEditingOfTablesKeepsThemWholeAndTheLayoutRight();
    void theDocumentsOfTheRandomCommandsHaveObjectsTheTextGoesRound();
    void aShapeWithNoFillOrLineHasNone();
    void aTableSetAtAPlaceHasItsPlace();
    void aTableOfFramedParagraphsHasTheFramesPlace();
    void aGroupInTheLineHoldsItsRoomAndAPictureTypeHasNoLine();
    void aGroupSetAtAPlaceKeepsTheTextOffItsRoom();
    void columnsOfUnequalWidthsAreLaidOutAsTheFileSaysThem();
    void aWallsParagraphStaysInItsColumnWhenColumnsAreMadeEven();
    void aLineOfCourierNewThatExactlyFillsItsRoomBreaksBeforeItsLastWord();
    void aFootersTextAfterATabPastTheRightEdgeDoesNotWrap();
    static QByteArray framedP(const QByteArray &text, const QByteArray &framePr);
    void framePrParagraphsAreAFrameSetWhereItAsks();
    void aParagraphStylesFrameIsItsParagraphsFrameWithTheirOwnOverIt();
    void aFramesTextIsNotPutOutOfItsBoxByAnotherFrameAtThePlace();
    void aFramesPropertiesAreRead();
    static QStringList kindsOf(WriteDoc &doc);
    static QByteArray aFramePr();
    static QTextBlock blockNumber(WriteDoc &doc, int n);
    void backspaceAndDeleteNeverJoinAFramesTextAndTheTextRoundIt();
    void aStyleAppliedToAFramesParagraphLeavesItInTheFrame();
    void enterNextToAFrameMakesParagraphsOfTheRightPlace();
    void pastedFrameTextIsTextOrIsThatOfTheFrameItIsPastedInto();
    void theCaretAndTheMouseWorkInAFramesText();
    static QByteArray endRef(int id);
    static QByteArray endnotesOf(const QList<std::pair<int, QByteArray>> &notes);
    static QByteArray refTo(int id);
    static QByteArray plainP(const QByteArray &text, const QByteArray &after = QByteArray(), const QByteArray &pPr = QByteArray());
    static QByteArray wordsOf(const QByteArray &first, int count);
    static QByteArray notesPart(const QList<std::pair<int, QByteArray>> &notes);
    static QByteArray sectionXml(int w, int h, int margin, const QByteArray &refs = QByteArray(), const QByteArray &type = QByteArray(), const QByteArray &after = QByteArray());
    static QByteArray tableOfRows(int rows, int columns, int width, const std::function<QByteArray(int, int)> &cell);
    static int rowsShown(flow::Paginator &pg);
    void aFilesNotesAroundATableAreOnThePagesThatCiteThem_data();
    void aFilesNotesAroundATableAreOnThePagesThatCiteThem();
    void aFilesTableRowsStayInTheBodyThatItsHeadersAndFootersLeave();
    void deletingARowBesideAContentControlKeepsTheTableAndTheControlsValid();
    void aTableWhoseRowsAndCellsAreInContentControlsShowsThemAndKeepsThem();
    void aSectionsLineNumbersAreDrawnAndKept();
    void aSectionsLineNumbersSkipTheStartAndRunOnWhenTheFileSays();
    void rubyInAHeaderIsShown();
    void anEquationIsDrawnAsALineOfText();
    void anEquationOfItsOwnParagraphIsCentered();
    void anEquationOfItsOwnParagraphIsWrittenBackAsItWas();
    void aStyledLetterInTextIsDrawnAsItsLetter();
    void aFormFieldIsDrawnAsWhatItShows();
    void aFormFieldsTextsHaveTheirCharacterReferencesDecoded();
    void aFrameAlignedInsideOrOutsideFollowsThePageNumber();
    void contentControlsShowTheirContentInEveryPlace();
    void facingPagesMirrorTheMarginsOfEvenPages();
    void aDisplayStyleSetsTheFaceOfAWordRun();
    void anAbsolutePositionTabGoesToTheMarginAndIsKept();
    void theEndnotesAreSetAfterTheLastTextAndAreNotWritten();
    void aLongRunOfEndnotesIsSetInFull();
    void aPageNumberCanCarryTheChaptersNumber();
    void aColumnBreakIsAColumnBreakAndIsKept();
    void aBoxesLineDoesNotTakeRoomFromItsText();
    void aBoxesAndAShapesLineTakeRoomFromTextOnlyInAPublication();
    void anEndlessRunOfEndnotesIsBounded();
    static QByteArray notedDocx();
    static QString savedBody(WriteDoc &doc);
    static QByteArray notesPartOf(const QByteArray &zip);
    static int tailOf(WriteDoc &doc, bool *allLast = nullptr);
    void selectingAllAndTypingChangesOnlyTheText();
    void aSelectionIntoAnEndnoteIsDeletedAsFarAsTheTextGoes();
    void typingInTheSeparatorOfTheEndnotesDoesNothing();
    void theViewsCaretStaysInTheText();
    void whereTheTextEndsFollowsTheStoryThroughUndo();
    void aCopyLeavesTheEndnotesOutAndAPasteMakesNoneInTheText();
    void findAndSpellingLeaveTheEndnotesAlone();
    static QByteArray sectionNotedDocx(const QByteArray &first, const QByteArray &last, const QByteArray &settings = QByteArray());
    static QStringList storyParagraphs(WriteDoc &doc);
    void endnotesAreSetAtTheEndOfEachSectionThatSaysSo();
    void whereTheEndnotesGoFollowsTheSettingsAndTheSection();
    void editingNextToTheEndnotesOfASectionLeavesThem();
    void copyFindAndSpellingLeaveTheEndnotesOfASectionAlone();
    void theCaretPassesTheEndnotesOfASection();
    void formattingASelectionLeavesTheEndnotesOfASectionAlone();
    void deleteAndBackspaceAtTheEndnotesOfASectionEndTheBreak();
    void aSectionBreakPutInASectionMovesItsNotesToTheEndOfTheRightPart();
    void endnotesAtSectionEndsAreBoundedInAll();
    void oneNoteOfThousandsOfParagraphsCitedThousandsOfTimesIsBounded();
    void aColumnBreakInACellOrATextBoxLosesNoText();
    void tagsAreAskedTheirAttributesWithAnExpressionMadeOncePerName();
    void anEndnotesNumberedParagraphsCountOnTheirOwn();
    void theSeparatorOfTheEndnotesIsARule();
    void aPageReferenceShowsThePageItsBookmarkIsOn();
    void thePageReferencesOfAPageAreBounded();
    void aParagraphThatSuppressesLineNumbersIsNeitherNumberedNorCounted();
    static QByteArray styleRefField(const char *instruction, const char *cached = "cached");
    void aHeaderShowsTheStyledParagraphOfItsPage();
    void theFieldsOfAHeaderAreBoundedAndFindTitlesAnyDistanceOff();
    void theFieldsOfAHeaderDoNotWalkTheStoryForEachPage();
    void aLongTitleInAHeaderIsNotClippedAndTwoFieldsAreBothGivenIt();
    void aFilesNoteCitedBeforeAContinuousTwoColumnSectionIsAtThePageFoot();
    void aFilesTableInAContinuousTwoColumnSectionIsInsideItsColumn();
    void aFilesNoteLongerThanThePageCitedFromACellHasPagesOfItsOwn();
    void aFilesPositionsInARowThatGoesOverPageBreaksAreOnThePagesThatShowThem();
    void aFilesTableIsNotShiftedByItsPlaceholderUnderProcessorLineHeights();
    void aFilesPageFieldsInAFooterAreRightForATableThatSpansPages();
    void randomEditingKeepsTheDocumentWhole();
    void theBlankDocumentSavesAndOpensAndKeepsItsStyles();
    static QList<LookCommand> lookCommands();
    static QString tailShape(WriteDoc &doc);
    static QString notePartText(const QByteArray &part, const char *tag, int id);
    void styleAndLookCommandsKeepAnEndnoteParagraphAnEndnote();
    void styleAndLookCommandsKeepAFootnoteWhole();
    void aStyleChosenInANoteIsTheNotesAndIsSavedInIt();
    static QByteArray notesAndHeaderDocx();
    void insertingAHeaderWhileAFootnoteIsEditedLeavesTheNote();
    void enteringANoteOrAHeaderLeavesTheOtherMode();
    void randomInputInTheWindowKeepsOneModeAtATime();
    static QList<std::pair<int, QRectF>> partsOfNote(WriteDoc &doc, int id);
    static bool caretIsInBoxOf(PageView &view, WriteDoc &doc, int id);
    void theCaretStaysInThePartOfANoteThatContinuesOnAnotherPage();
    void theCaretStaysOnTheCopyOfANoteThatWasClicked();
    static QByteArray separatorsAt(const char *tag, int separator, int continuation);
    static QStringList noteElementsOf(const QByteArray &part, const char *tag);
    void aNewNoteTakesAnIdAboveEveryIdOfItsPartTheSeparatorsToo();
    void aNoteStaysWhileAHeaderATextBoxOrAnotherNoteStillCitesIt();
    void aNoteGoesWhenTheLastPlaceThatCitesItLetsItGo();
    void insertingAndUndoingNotesOverAndOverNeverFillsTheDocument();
    void aDocumentWithTheMostNotesTurnsTheCommandsOffAndSaysWhy();
    static QStringList endnoteIdsOfReferences(WriteDoc &doc);
    static QStringList endnoteIdsOfTails(WriteDoc &doc);
    void settlingTheEndnotesCostsWhatTheEditTouchedNotWhatTheStoryHolds();
    static QByteArray sectionAsWritten(const QByteArray &sectPr);
    static QByteArray firstSectionIn(const QByteArray &xml);
    void aSectionInANotesParagraphIsKeptWhenTheNoteIsEdited();
    void turningAListOffInANoteGivesBackTheNotesOwnStyle();
    static QList<NoteBox> boxesOfNote(WriteDoc &doc, int id);
    void theCaretStaysInThePartClickedWhereOneCopyGoesOnAndAnotherBegins();
    void aNoteStaysWhileAnEditedHeaderStillCitesIt();
    void notesThatCiteOnlyOneAnotherGoWithTheTextsLastReference();
    void aDocumentWithTheMostEndnotesTurnsInsertEndnoteOffInTheWindow();
    void aKeystrokeCostsAboutTheSameWhateverTheNumberOfEndnoteReferencesAfterIt();
    void theCaretStaysOnThePageOfAnEndnoteThatContinuesOnAnotherPage();
    void aClickInALaterCopyOfAnEndnoteIsInTheFirstCopyAndTypingUpdatesBoth();
    void ordinaryEditsCostTheSameWorkWhateverTheNumberOfNotes();
    void aKeystrokeInAnEndnoteWithCopiesCostsWhatTheNoteIsNotWhatTheOthersAre();
    void theCopiesOfAnEditedEndnoteSetAgainAloneAreWhatSettingAllOfThemAgainGives();
    void aCopyOfAnEndnoteAtTheEndOfASectionStillShowsWhatTheFirstCopySays();
    void undoAndRedoTakeEveryCopyOfAnEditedEndnoteTogether_data();
    void undoAndRedoTakeEveryCopyOfAnEditedEndnoteTogether();
    void consecutiveCharactersTypedInAnEndnoteWithoutCopiesAreOneUndoStep();
    static QByteArray pngOf(int width, int height, int shade = 0);
    static int picturesIn(const QTextDocument *story);
    static QRectF firstLineRect(WriteDoc &doc);
    static QVector<QRectF> pictureItemsOfPage(WriteDoc &doc);
    static QString longText();
    void aPictureInsertedFromAFileIsOneStepAndSavesAndOpensTheSame();
    void aPictureLargerThanTheColumnIsMadeToFitIt();
    void aPictureFromAnImageIsKeptAsPngAndSavesAndOpensTheSame();
    void eachWrapOfAPictureShowsOnThePageIsOneStepAndSavesAndOpensTheSame();
    void aPictureMarkedAsDecorativeHasNoDescriptionAndKeepsTheMarkWhenSaved();
    void aPictureIsSizedPlacedAndDescribedInOneStepEach();
    void aPictureInTheTextMovesToAnotherPlaceInOneStep();
    static quint32 crc32Of(const QByteArray &bytes);
    void aPictureThatCannotBeReadOrIsTooLargeChangesNothing();
    void aDocumentThatHoldsAsManyPicturesAsItCanShowRefusesMoreAndKeepsItsUndo();
    static QByteArray sectionXml(int w, int h, int top, int right, int bottom, int left, const char *orient = "", const char *type = "");
    static std::unique_ptr<WriteDoc> threeSections();
    static QVector<SectionSetup> setupsOf(const WriteDoc &doc);
    void formatPageNumbersSetsASectionsNumberingInOneStepAndSavesIt();
    void pageSetupChangesTheSectionOrTheWholeDocumentInOneStepAndSavesTheSame();
    void pageSetupTurnsAPageAndKeepsItsMarginsOnIt();
    void pageSetupIsHeldToWhatAPageCanHold();
    void pageSetupOfTenThousandSectionsMakesEachOnceAndIsOneStep();
    void tabStopsAreAddedMovedAndRemovedInOneStepAndSaveTheSame();
    void tabStopsAreHeldToWhatAParagraphKeeps();
    static QByteArray decoratedPictureBody(bool anchored, bool withLink = true);
    void editingAPictureKeepsWhatTheEditDoesNotChange();
    void drawingIdsDoNotCollideWithThoseOfHeadersOrPastes();
    void altTextAndNamesKeepTheirSurrogatePairsAndDropLoneOnes();
    void anUndoneOrDeletedPictureIsNotInTheSavedFile();
    void aPictureIsHeldOnceAndTheDocumentsPicturesHaveATotal();
    static QByteArray svgWith(const QByteArray &inner);
    static void insertFormat(const char *format);
    void aTiffPictureIsInserted();
    void aWebpPictureIsInserted();
    void aBmpPictureIsInserted();
    void anSvgPictureIsInsertedAsThePictureItDraws();
    void anSvgThatReachesOutsideItselfOrIsHugeIsRefused();
    void aPictureFileThatIsNotAFileOrTooLongIsNotReadToTheEnd();
    void thePaperIsNoLargerThanTwentyTwoInches();
    void aSizeThatKeepsItsRatioIsHeldToTheLimitsAsAWhole();
    void tabStopsAFileHasPastTheLimitAreKept();
    void aSectionChangedByPageSetupIsWrittenInSchemaOrder();
    void thirtyThousandEditsOfOnePictureAreNeverRefusedAndItsRecordsAreBounded();
    void aCopiedFloatingPictureIsPastedIntactAfterTheFormsItHadAreFreed();
    static QByteArray pictureBodyWithId(int id);
    static QList<int> drawingIdsOf(const QByteArray &xml);
    void everyPastedPictureHasADrawingIdOfItsOwn();
    void theDrawingIdsOfTheNotesAreCountedToo();
    void aPastedPictureIsHeldToTheTotalOfTheDocumentsPictures();
    void tabStopsAParagraphHasBeyondWhatTheRulerShowsAreNeverDropped();
    void theArrowKeysMovePastAPictureWhicheverWayItsFormWasMade();
    static QTextCharFormat styCharAt(WriteDoc &doc, int position);
    static QString styBlockStyle(WriteDoc &doc, int position);
    static wml::Style styStyle(WriteDoc &doc, const QString &id);
    static QByteArray styCustomStyle(const QString &id, const QString &name, const QByteArray &inside, const QByteArray &type = "paragraph");
    static QStringList styIds(const QVector<ListedStyle> &list);
    void theStylesPaneListsTheStylesAndFiltersThemByUseAndByRecommendation();
    void aStyleMadeFromTheSelectionIsOneStepAndSavesAndOpensTheSame();
    void modifyingAStyleRestylesTheTextKeepsWhatTheTextSaysAndIsOneStep();
    void modifyingAStyleWithNoChangeLeavesTheFileByteForByteAndUndoReturnsToIt();
    void aStylesNameBasisAndNextAreCheckedAndSavedTheSame();
    static QByteArray styManyStyles(int n, QByteArray *chain);
    void aDocumentOfTenThousandStylesIsListedChangedAndUndoneWithinBounds();
    void styleChainsThatNameThemselvesAreFollowedNoFurtherThanOnce();
    void whatTheStepsOfTheHistoryHoldOfTheStylesIsFreedWithTheSteps();
    void aStyleThatUpdatesItselfFollowsTheFormattingOfItsParagraphsInTheSameStep();
    void updatingAStyleToMatchTheSelectionIsOneStep();
    void theFormatPainterCopiesCharactersAndParagraphsAndPastesInOneStep();
    void goToFindsPagesSectionsLinesBookmarksFootnotesAndHeadings();
    void goToIsHeldToWhatAHostileDocumentCanHold();
    void draggingSelectedTextMovesOrCopiesItInOneStep();
    void aBuiltInStyleTheDocumentLacksIsAddedByARealChangeAloneAndOneUndoTakesItAway();
    void theStyleSheetIsNotCopiedForEachRunOfAnAbsentBuiltInStyleWhenAStyleIsChanged();
    void aStyleThatUpdatesItselfFollowsOneParagraphsFormattingByHandAndNothingElse();
    void theSectionsAreFoundInOnePassOverTheParagraphs();
    void theListOfStylesAlwaysHoldsTheOnesTheTextUsesAndTheCaretsWhateverTheCut();
    void aSelectionHoldingADeletedParagraphIsMovedToWhereItWasDropped();
    void aSelectionHoldingAFootnoteReferenceIsMovedToWhereItWasDropped();
    void aStyleChangeNoStepTookIsPutBackAndDoesNotJoinTheNextStep();
    void aStyleKeepsAnUnknownChildAndAttributeThroughModifyAndSave();
    void applyingABuiltInStyleTheDocumentLacksIsOneStepWithItsStyleAndUndoRestoresTheFile();
    void insertingAFootnoteIntoADocumentThatLacksTheFootnoteStylesPutsThemInTheStepAndUndoTakesThemOut();
    void aSectionBreakOnTheLastParagraphIsOneSectionAtItsFirstParagraph();
    static std::unique_ptr<WriteWindow> styWindow(const QByteArray &body, const QByteArray &styles = kStyles);
    static QPoint styPointOf(PageView *view, int position);
    void theStylesPaneShowsTheStylesFiltersThemAndAppliesOneWithAClick();
    void theCreateStyleDialogMakesTheStyleOfTheSelectionAndOneUndoTakesItBack();
    void theModifyStyleDialogChangesTheStyleAndWithNoChangeLeavesTheFileAsItWas();
    void theFormatPainterPaintsOnceOrStaysOnUntilEscapeAndEachPaintIsOneStep();
    void aDragOfTheMouseInTextEndsWhenNoButtonIsHeld();
    void goToOpensWithCtrlGAndPutsTheCaretAtThePlaceNamed();
    void dragAndDropMovesSelectedTextOrCopiesItWithCtrlInOneStep();
    void aClickInsideTheSelectionPutsTheCaretThereAndWithTheDragOffAPressStartsANewSelection();
    void theStylesPaneOfADocumentWithTenThousandStylesListsAtMostWhatItHolds();
    void aCancelledStyleDialogOnABuiltInStyleTheDocumentLacksChangesNothing();
    void savesTheDocumentWithHeaderAndFooter();
    void aSavedDocumentIsTheSameBytesInEachProcess();
    void theSharedAutoCorrectRulesSayWhatToPutInPlaceOfWhat();
    void typingFixesSlipsSymbolsAndSentenceCapitalsAndOneUndoTakesJustTheFix();
    void typingMakesDashesFractionsAndRaisedOrdinalsAndOneUndoTakesJustThat();
    void typingOneAndAPeriodOrAStarAndASpaceStartsAListInWritesOwnListModelAndOneUndoTakesItBack();
    void aRowOfDashesAndEnterGivesTheParagraphABottomBorderAndOneUndoTakesItBack();
    void aWebOrEmailAddressFollowedBySpaceBecomesALinkAndOneUndoTakesTheLinkBack();
    void starredWordsBecomeBoldAndUnderscoredWordsBecomeUnderlinedAndOneUndoTakesJustThat();
    void aQuoteTypedOverASelectionIsCurledAndAQuoteAfterADigitOrABracketIsChosenByIt();
    void aNumberAndAParenthesisOrANumberOtherThanOneStartsNoListInWrite();
    void withTrackChangesOnTheFixIsTrackedAndDeletedTextIsNotPartOfTheWord();
    void aRuleKeepsTheParagraphsOtherBordersAndLeavesTheNextOneAlone();
    void theLinkRuleFollowsItsOwnBoxAndAnAddressInBracketsBecomesALinkWithoutThem();
    void everyKindOfFixCanBeUndoneAndDoneAgain();
};
