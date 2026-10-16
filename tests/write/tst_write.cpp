// Tests of JeffWrite (apps/write): the document, editing it, the .docx it reads and writes, the page view, and the window.

#include "app/pageview.h"
#include "app/filescreen.h"
#include "app/help.h"
#include "app/ribbon.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QApplication>
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

    // A developer's hook, not a test: JW_ROUNDTRIP_IN names a folder of .docx files, JW_ROUNDTRIP_OUT where each is written again by JeffWrite
    // (to be opened by other programs); JW_SHOTS a folder to put screenshots of them in.
    void developerRoundTripOfAFolder()
    {
        const QString in = qEnvironmentVariable("JW_ROUNDTRIP_IN"), out = qEnvironmentVariable("JW_ROUNDTRIP_OUT");
        if (in.isEmpty() || out.isEmpty()) QSKIP("JW_ROUNDTRIP_IN and JW_ROUNDTRIP_OUT are not set");
        QDir().mkpath(out);
        for (const QFileInfo &fi : QDir(in).entryInfoList({QStringLiteral("*.docx")}, QDir::Files)) {
            QFile f(fi.absoluteFilePath());
            QVERIFY(f.open(QIODevice::ReadOnly));
            QString err;
            auto doc = WriteDoc::fromDocx(f.readAll(), &err);
            QVERIFY2(doc, qPrintable(fi.fileName() + ": " + err));
            const QByteArray zip = doc->toDocx(&err);
            QVERIFY2(!zip.isEmpty(), qPrintable(err));
            QFile o(QDir(out).filePath(fi.fileName()));
            QVERIFY(o.open(QIODevice::WriteOnly));
            o.write(zip);
            qInfo("%s: %d paragraphs, %d pages", qPrintable(fi.fileName()), doc->story()->blockCount(), doc->pager().pageCount());
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
