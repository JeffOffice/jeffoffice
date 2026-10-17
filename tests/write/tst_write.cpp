// Tests of JeffWrite (apps/write): the document, editing it, the .docx it reads and writes, the page view, and the window.

#include "core/fonts.h"
#include "app/pageview.h"
#include "app/filescreen.h"
#include "app/help.h"
#include "app/recovery.h"
#include "app/ribbon.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/tableops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "../flow/pageaudit.h"
#include "../office/dochelpers.h"
#include "office/package.h"
#include "rtcheck.h"
#include "text/textprops.h"
#include "io/zip.h"
#include "wml/wml.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QRadioButton>
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
#include <QRegularExpression>
#include <QScrollBar>
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
    body += "<w:p w14:paraId=\"1A2B3C07\"><w:r><w:t>After the block control, which Backspace must not eat.</w:t></w:r></w:p>";
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
QByteArray makeDocxWithParts(const QByteArray &body, const QByteArray &sect, const QMap<QString, QByteArray> &parts, const QByteArray &settings = QByteArray(), const QByteArray &styles = kStyles)
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
    QByteArray styled, plain, styles, sect;
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

} // namespace

class WriteTests : public QObject {
    Q_OBJECT
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
        QCOMPARE(tabs, (QStringList{"Home", "Insert", "Layout", "View"}));
        QCOMPARE(w.ribbon()->fileKeytip(), QStringLiteral("F"));
        QVERIFY(w.windowTitle().contains(QLatin1String("JeffWrite")));
        QVERIFY(w.windowTitle().startsWith(QLatin1String("Document")));
        // The commands a person looks for.
        for (const char *id : {"file.new", "file.open", "file.save", "file.saveAs", "file.print", "file.exportPdf", "file.options", "edit.undo", "edit.redo", "fmt.bold", "para.bullets", "insert.pageBreak",
                               "layout.landscape", "zoom.pageWidth", "help.show"})
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

    void trackedChangesOfAnOldFileComeInWithTheirAuthors()
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
        // Written out, each is still a tracked change with its author.
        const QByteArray x = documentXml(w.document()->toDocx());
        const qsizetype ins = x.indexOf("<w:ins ");
        const qsizetype del = x.indexOf("<w:del ");
        QVERIFY2(ins >= 0 && del > ins, x.constData());
        const QByteArray insTag = x.mid(ins, x.indexOf('>', ins) - ins), delTag = x.mid(del, x.indexOf('>', del) - del);
        QVERIFY2(insTag.contains("w:author=\"Ann\""), insTag.constData());
        QVERIFY2(delTag.contains("w:author=\"Bob\""), delTag.constData());
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
        QTimer::singleShot(0, [&] {
            auto *dlg = w.findChild<QDialog *>(QStringLiteral("paragraphDialog"));
            QVERIFY(dlg);
            dlg->findChild<QComboBox *>(QStringLiteral("alignment"))->setCurrentIndex(1);
            auto *left = dlg->findChild<QDoubleSpinBox *>(QStringLiteral("left"));
            left->setValue(36);
            dlg->findChild<QComboBox *>(QStringLiteral("lineSpacing"))->setCurrentIndex(2);
            dlg->accept();
        });
        w.act("para.dialog")->trigger();
        for (int i = 0; i < 2; ++i) {
            const QTextBlockFormat bf = w.document()->story()->findBlockByNumber(i).blockFormat();
            QCOMPARE(bf.alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
            QCOMPARE(bf.leftMargin(), 36.0);
            QCOMPARE(bf.lineHeight(), 200.0);
            QCOMPARE(bf.rightMargin(), 0.0);   // (not changed: left as it was)
        }
        QTimer::singleShot(0, [&] {
            auto *dlg = w.findChild<QDialog *>(QStringLiteral("zoomDialog"));
            QVERIFY(dlg);
            dlg->findChild<QSpinBox *>(QStringLiteral("percent"))->setValue(150);
            for (auto *r : dlg->findChildren<QRadioButton *>())
                if (r->property("zoomChoice").toInt() == 0) r->setChecked(true);
            dlg->accept();
        });
        w.act("zoom.dialog")->trigger();
        QCOMPARE(w.view()->zoom(), 1.5);
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
        QCOMPARE(over->view()->currentTopic(), QStringLiteral("open-save"));
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
    // each kind of problem; JW_STRICT_OUT keeps the files written; JW_STRICT_LIMIT stops after that many files.
    void developerStrictRoundTrip()
    {
        const QString in = qEnvironmentVariable("JW_STRICT_IN"), list = qEnvironmentVariable("JW_STRICT_LIST"), out = qEnvironmentVariable("JW_STRICT_OUT");
        if (in.isEmpty() && list.isEmpty()) QSKIP("JW_STRICT_IN and JW_STRICT_LIST are not set");
        if (!out.isEmpty()) QDir().mkpath(out);
        const int limit = qEnvironmentVariableIntValue("JW_STRICT_LIMIT") > 0 ? qEnvironmentVariableIntValue("JW_STRICT_LIMIT") : 1 << 30;
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
            auto doc = open(makeDocx(paragraphs(paragraphCount, 12), kStyles));
            QTextCursor c(doc->story());
            c.setPosition(5000);
            QElapsedTimer t;
            t.start();
            for (int i = 0; i < 50; ++i) insertText(*doc, c, QStringLiteral("x"));
            insertParagraph(*doc, c);
            for (int i = 0; i < 50; ++i) doc->story()->undo(&c);
            const double ms = t.nsecsElapsed() / 1e6;
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
            double best = 1e9;
            for (int k = 0; k < 2; ++k) {
                QElapsedTimer t;
                t.start();
                auto doc = open(zip);
                best = std::min(best, t.nsecsElapsed() / 1e9);
            }
            return best;
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

    // The option that no punctuation hangs past the margin is read by its value: stated off, it is off.
    void noHangingPunctuationIsReadByItsValue()
    {
        auto hangs = [&](const QByteArray &option) {
            auto doc = open(makeDocxWithParts(paragraphs(1, 5), QByteArray(), {}, "<w:compat>" + option + "</w:compat>"));
            return doc && !doc->flow().env.wordNoHang;
        };
        QVERIFY(!hangs("<w:doNotWrapTextWithPunct/>"));
        QVERIFY(!hangs("<w:doNotWrapTextWithPunct w:val=\"1\"/>"));
        QVERIFY(hangs("<w:doNotWrapTextWithPunct w:val=\"0\"/>"));
        QVERIFY(hangs("<w:doNotWrapTextWithPunct w:val=\"false\"/>"));
        QVERIFY(hangs(""));
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
            QElapsedTimer t;
            t.start();
            auto d = open(zip);
            return d ? t.nsecsElapsed() / 1e6 : 1e9;
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
            QElapsedTimer t;
            t.start();
            auto doc = open(zip);
            const double ms = t.nsecsElapsed() / 1e6;
            return doc ? ms : 1e9;
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
        QElapsedTimer t;
        t.start();
        QString err;
        auto doc = WriteDoc::fromDocx(zip, &err, notes);
        if (!doc) {
            qWarning("open: %s", qPrintable(err));
            return 1e9;
        }
        const int n = doc->pager().pageCount();
        if (pages) *pages = n;
        return t.nsecsElapsed() / 1e9;
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
        QElapsedTimer t;
        t.start();
        auto doc = open(zip);
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        const int pages = pg.pageCount();
        QVERIFY2(t.elapsed() < 20000, qPrintable(QString::number(t.elapsed())));
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
    // The tables of that document, and its styles and its section's properties; `noteInCell` is the XML of a run put in the cell b1 (a footnote's reference).
    static RichTables richTables(const QByteArray &noteInCell = QByteArray())
    {
        QByteArray longCell, tallCell;
        for (int i = 0; i < 24; ++i) tallCell += cellP("tall " + QByteArray::number(i));
        for (int i = 0; i < 9; ++i) longCell += cellP("merged line " + QByteArray::number(i));
        const QByteArray deep = tableX(rowX(cellX(cellP("deep"), QByteArray(), 700)), 1, 700);
        const QByteArray nested = tableX(rowX(cellX(cellP("n00"), QByteArray(), 800) + cellX(cellP("n01") + deep + cellP("n01 tail"), QByteArray(), 800)) + rowX(cellX(cellP("n10"), QByteArray(), 800) + cellX(cellP("n11"), QByteArray(), 800)), 2, 800);
        const QByteArray styled = tableX(rowX(cellX(cellP("h0"), QByteArray(), 1100) + cellX(cellP("h1"), QByteArray(), 1100) + cellX(cellP("h2"), QByteArray(), 1100), "<w:tblHeader/>") +
                                             rowX(cellX(longCell, "<w:vMerge w:val=\"restart\"/>", 1100) + cellX(cellP("a1"), QByteArray(), 1100) + cellX(cellP("a2") + nested + cellP("a2 tail"), QByteArray(), 1100)) +
                                             rowX(cellX("<w:p/>", "<w:vMerge/>", 1100) + cellX("<w:p><w:r><w:t>b1</w:t></w:r>" + noteInCell + "</w:p>", QByteArray(), 1100) + cellX(cellP("b2"), QByteArray(), 1100)) +
                                             rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 2200), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"1100\" w:type=\"dxa\"/>") +
                                             rowX(cellX(cellP("d0"), QByteArray(), 1100) + cellX(tallCell, QByteArray(), 1100) + cellX(cellP("d2"), QByteArray(), 1100)) +
                                             rowX(cellX(cellP("e0"), QByteArray(), 1100) + cellX(cellP("e1"), QByteArray(), 1100), "<w:gridAfter w:val=\"1\"/><w:wAfter w:w=\"1100\" w:type=\"dxa\"/>"),
                                         3, 1100, "<w:tblStyle w:val=\"TG\"/><w:tblW w:w=\"3300\" w:type=\"dxa\"/><w:tblLook w:val=\"0420\"/>");
        const QByteArray plain = tableX(rowX(cellX(cellP("p00")) + cellX(cellP("p01"))) + rowX(cellX(cellP("p10")) + cellX(cellP("p11"))), 2, 1600);
        const QByteArray styles = kStyles +
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
                                  "<w:pPr><w:spacing w:before=\"120\" w:after=\"60\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Arimo\" w:hAnsi=\"Arimo\"/><w:b/><w:sz w:val=\"26\"/></w:rPr></w:style>"
                                  "<w:style w:type=\"table\" w:styleId=\"TG\"><w:name w:val=\"TG\"/><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
                                  "<w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
                                  "<w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders></w:tblPr><w:tblStylePr w:type=\"firstRow\"><w:rPr><w:b/></w:rPr></w:tblStylePr>"
                                  "<w:tblStylePr w:type=\"band2Horz\"><w:tcPr><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"EEEEEE\"/></w:tcPr></w:tblStylePr></w:style>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"5200\" w:h=\"4400\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
        return {styled, plain, styles, sect};
    }
    static QByteArray richTableDocx()
    {
        const RichTables t = richTables();
        return makeDocx(cellP("before the tables") + t.styled + cellP("between them") + t.plain + cellP("after the tables"), t.styles, QByteArray(), t.sect);
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
        QByteArray body = plainP("before the tables", refTo(2)) + t.styled + plainP("between them", refTo(4)) + t.plain + plainP("after the tables", QByteArray(), one);
        body += plainP(wordsOf("Second section", 18)) + tableOfRows(9, 2, 1000, [](int r, int c) { return r == 4 && c == 1 ? plainP("cites", refTo(5)) : cellP("r" + QByteArray::number(r) + "c" + QByteArray::number(c)); }) +
                plainP(wordsOf("end of the second", 30), QByteArray(), two);
        body += plainP(wordsOf("Third section", 40), refTo(6)) + "<w:p><w:r><w:t>before a break</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after it</w:t></w:r></w:p>" + plainP(wordsOf("Last", 60));
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
        QVERIFY2(drawn.contains(QStringLiteral("word699")), "the end of the long note is drawn");
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
