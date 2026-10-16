// Tests of JeffWrite (apps/write): the document, editing it, the .docx it reads and writes, the page view, and the window.

#include "app/pageview.h"
#include "app/filescreen.h"
#include "app/help.h"
#include "app/recovery.h"
#include "app/ribbon.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "rtcheck.h"
#include "text/textprops.h"
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
#include <random>

using namespace jp;
using namespace jp::write;
namespace wml = jp::office::wml;

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

const QByteArray kStyles =
    "<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Liberation Serif\" w:hAnsi=\"Liberation Serif\"/><w:sz w:val=\"23\"/></w:rPr></w:rPrDefault>"
    "<w:pPrDefault><w:pPr><w:spacing w:after=\"120\" w:line=\"264\" w:lineRule=\"auto\"/></w:pPr></w:pPrDefault></w:docDefaults>"
    "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:uiPriority w:val=\"9\"/><w:qFormat/>"
    "<w:pPr><w:keepNext/><w:spacing w:before=\"360\" w:after=\"120\"/><w:outlineLvl w:val=\"0\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Liberation Sans\" w:hAnsi=\"Liberation Sans\"/><w:b/><w:color w:val=\"1F3864\"/><w:sz w:val=\"34\"/></w:rPr></w:style>"
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

} // namespace

class WriteTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
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
        QElapsedTimer t;
        t.start();
        view.setDocument(doc.get());
        QVERIFY(view.pageCount() >= 1);
        view.show();
        QVERIFY(view.grab().width() == 1000);   // the first page is drawn
        QVERIFY2(t.elapsed() < 3000, "the first page must show without waiting for the document");
        QTRY_VERIFY_WITH_TIMEOUT(view.pagesComplete(), 60000);
        QVERIFY(view.pageCount() > 100);
        // The page count grew as pages were laid out, and the scroll range with it.
        QVERIFY(view.verticalScrollBar()->maximum() > 100 * 700);
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
        QMap<QString, int> kinds;
        QStringList lines;
        for (const QString &path : std::as_const(paths)) {
            if (files >= limit) break;
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly)) continue;
            const QByteArray bytes = f.readAll();
            ++files;
            QString err;
            auto doc = WriteDoc::fromDocx(bytes, &err);
            if (!doc) {
                ++refused;
                continue;
            }
            const QByteArray zip = doc->toDocx(&err);
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
        qInfo("strict round trip: %d files, %d refused, %d not written, %d not well-formed, %d lost or changed something, %d came back with another story, %d clean", files, refused, notWritten, malformed, lost, story, clean);
        for (auto it = kinds.cbegin(); it != kinds.cend(); ++it) qInfo("  %s: %d", qPrintable(it.key()), it.value());
        for (const QString &l : std::as_const(lines)) qInfo("  %s", qPrintable(l));
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
        // 20,000 paragraphs: an undo that kept the whole document would take seconds and megabytes (the design's reason for commands).
        auto doc = open(makeDocx(paragraphs(20000, 12), kStyles));
        QTextCursor c(doc->story());
        c.setPosition(5000);
        QElapsedTimer t;
        t.start();
        for (int i = 0; i < 50; ++i) insertText(*doc, c, QStringLiteral("x"));
        insertParagraph(*doc, c);
        for (int i = 0; i < 50; ++i) doc->story()->undo(&c);
        QVERIFY2(t.elapsed() < 2000, qPrintable(QString::number(t.elapsed())));
        QCOMPARE(doc->story()->blockCount(), 20000);
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
        setFontFamily(*doc, c, QStringLiteral("Liberation Mono"));
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
        QVERIFY(props.contains(QLatin1String("Liberation Mono")));
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
        QCOMPARE(f.fontFamilies().toStringList().value(0), QStringLiteral("Liberation Mono"));
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

    void aPageBreakCutsTheParagraphAndTheFileKeepsItWhole()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("before after"));
        c.setPosition(7);
        insertPageBreak(*doc, c);
        QCOMPARE(doc->pager().pageCount(), 2);
        QCOMPARE(doc->story()->blockCount(), 2);
        const QByteArray x = documentXml(doc->toDocx());
        // One paragraph with a break in it, as the file says.
        QCOMPARE(int(x.count("<w:p>")) + int(x.count("<w:p ")), 1);
        QVERIFY2(x.contains("<w:br w:type=\"page\"/>"), x.constData());
        auto back = open(doc->toDocx());
        QCOMPARE(back->story()->blockCount(), 2);
        QCOMPARE(back->pager().pageCount(), 2);
        QCOMPARE(documentXml(back->toDocx()), x);
        // Backspace at the start of the second part takes the break out.
        QTextCursor d(back->story());
        d.setPosition(back->story()->findBlockByNumber(1).position());
        deleteBack(*back, d, false);
        QCOMPARE(back->story()->blockCount(), 1);
        QCOMPARE(back->pager().pageCount(), 1);
        QVERIFY(!documentXml(back->toDocx()).contains("w:br"));
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
        // The table is a stand-in paragraph; the picture, the field, and the control have none that shows beyond their text.
        QTextDocument *s = doc->story();
        QVERIFY(isLocked(s->findBlockByNumber(1)));
        QVERIFY(s->findBlockByNumber(1).text().size() == 1);
        QVERIFY(s->toPlainText().contains(QLatin1String("in a control")));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains(table));
        QVERIFY(x.contains("<wp:inline xmlns:wp=\"urn:wp\">"));
        QVERIFY(x.contains("<w:instrText> PAGE </w:instrText>"));
        QVERIFY(x.contains("<w:sdt><w:sdtPr><w:id w:val=\"9\"/></w:sdtPr><w:sdtContent>"));
        QVERIFY(x.contains("</w:sdtContent></w:sdt>"));
        // Editing around them keeps them; the stand-in can't be typed into, and Enter goes after it.
        QTextCursor c(s);
        c.setPosition(s->findBlockByNumber(1).position());
        insertText(*doc, c, QStringLiteral("zzz"));
        QCOMPARE(s->findBlockByNumber(1).text().size(), 1);
        insertParagraph(*doc, c);
        QCOMPARE(s->blockCount(), 5);   // (a paragraph was added after the stand-in)
        QCOMPARE(documentXml(doc->toDocx()).count(table), 1);
        // Deleting the stand-in takes the table out.
        c.setPosition(s->findBlockByNumber(1).position());
        c.setPosition(s->findBlockByNumber(1).position() + 1, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        QVERIFY(!documentXml(doc->toDocx()).contains("<w:tbl>"));
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
        QVERIFY(!WriteDoc::fromDocx(makeDocx(deep), &err));
        // Sizes and counts that no page could have are clamped, and the document opens.
        const QByteArray odd = "<w:p><w:pPr><w:ind w:left=\"99999999999\" w:hanging=\"88888888\"/><w:spacing w:before=\"2000000000\" w:line=\"-5\"/><w:numPr><w:ilvl w:val=\"500\"/><w:numId w:val=\"77\"/></w:numPr></w:pPr>"
                               "<w:r><w:rPr><w:sz w:val=\"9999999\"/></w:rPr><w:t>x</w:t></w:r></w:p>";
        auto doc = open(makeDocx(odd, kStyles, QByteArray(), "<w:sectPr><w:pgSz w:w=\"1\" w:h=\"2000000000\"/><w:pgMar w:top=\"-900000000\" w:left=\"900000000\"/><w:cols w:num=\"100000\"/></w:sectPr>"));
        QVERIFY(doc);
        QVERIFY(doc->pager().pageCount() >= 1);
        QVERIFY(doc->pager().page(0).size.width() >= 36 && doc->pager().page(0).size.height() <= 3168);
        QVERIFY(doc->flow().section(0).columnCount() <= 45);
        // A big document in a time a person would wait: 30,000 paragraphs open, lay out its first page at once, and save.
        QElapsedTimer t;
        t.start();
        auto big = open(makeDocx(paragraphs(30000, 25), kStyles));
        QVERIFY2(t.elapsed() < 30000, qPrintable(QString::number(t.elapsed())));
        QVERIFY(big->pager().materialize(0));
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
        for (int mode = 0; mode < 2; ++mode) {   // 0: commands that only add; 1: those that take away too
            auto base = open(original);
            QVERIFY(base);
            const QMap<QString, int> start = rt::keptCountsOf(base->toDocx());
            for (unsigned seed = std::max(1, qEnvironmentVariableIntValue("JW_FUZZ_FROM")); seed <= unsigned(seeds); ++seed) {
                auto doc = open(original);
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
                    }
                    seedFailed = seedFailed || fatal;
                    seedsFailing.insert(QStringLiteral("%1/%2").arg(mode).arg(seed));
                };
                for (int step = 0; step < 120 && !seedFailed; ++step) {
                    const int op = pick(mode == 0 ? 12 : 17);
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
                    default: selectSome(); c.removeSelectedText(); break;
                    }
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
                    if (again != documentXml(zip)) fail(QStringLiteral("drifts"), QStringLiteral("the second save differs from the first"));
                    if (mode == 0) {
                        const QMap<QString, int> now = rt::keptCountsOf(zip);
                        for (auto it = start.cbegin(); it != start.cend(); ++it)
                            if (now.value(it.key()) < it.value()) fail(QStringLiteral("lost ") + it.key(), QStringLiteral("%1 %2->%3").arg(it.key()).arg(it.value()).arg(now.value(it.key())));
                    }
                }
            }
        }
        QStringList report;
        for (auto it = failed.cbegin(); it != failed.cend(); ++it) report << QStringLiteral("%1: %2 seeds\n  %3").arg(it.key()).arg(it.value()).arg(example.value(it.key()));
        if (qEnvironmentVariableIsSet("JW_FUZZ_COUNTS")) qInfo("fuzz: %d of %d runs failed:\n%s", int(seedsFailing.size()), 2 * seeds, qPrintable(report.join(QLatin1Char('\n'))));
        QVERIFY2(failed.isEmpty(), qPrintable(QStringLiteral("%1 of %2 runs failed:\n%3").arg(seedsFailing.size()).arg(2 * seeds).arg(report.join(QLatin1Char('\n')))));
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
        body += "<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        body += "<w:p><w:r><w:t>before break </w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after break</w:t></w:r></w:p>";
        body += "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr></w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>";
        body += paragraphs(5, 30);
        const int seeds = qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 8;   // (more, to look further)
        for (unsigned seed = std::max(1, qEnvironmentVariableIntValue("JW_FUZZ_FROM")); seed <= unsigned(seeds); ++seed) {
            auto doc = open(makeDocx(body, kStyles, numbering));
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
                const int op = pick(17);
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
                        dump += QStringLiteral("[%1 len %2 locked=%3 join=%4 sect=%5] %6\n").arg(bk.blockNumber()).arg(bk.length()).arg(isLocked(bk)).arg(bk.blockFormat().boolProperty(prop::JoinNext)).arg(bk.blockFormat().boolProperty(flow::fp::SectionBreakAfter)).arg(t.left(30).replace(QChar::ObjectReplacementCharacter, QLatin1Char('#')));
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
