// Tests of JeffWrite's File screen: where editing stopped, the warning before a save to a format that holds less, and the Options pages. Every file is made here in code.

#include "../isolatesettings.h"
#include "app/filescreen.h"
#include "app/optionsdialog.h"
#include "app/settings.h"
#include "app/pageview.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/emailops.h"
#include "core/pdfbookmarks.h"
#include "core/printing.h"
#include "app/versionstore.h"
#include "core/templates.h"
#include "render/pdfpage.h"
#include "office/package.h"
#include "office/packageprops.h"
#include "core/writedoc.h"

#include <QAction>
#include <QLabel>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPrinter>
#include <QPdfWriter>
#include <QTemporaryDir>
#include <QTest>

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

QByteArray para(const char *style, const char *text)
{
    return QByteArray("<w:p>") + (style ? QByteArray("<w:pPr><w:pStyle w:val=\"") + style + "\"/></w:pPr>" : QByteArray()) + "<w:r><w:t>" + text + "</w:t></w:r></w:p>";
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

class FileTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
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
        QVector<int> odd, even;
        for (int i : all) ((i + 1) % 2 ? odd : even) << i;
        std::reverse(even.begin(), even.end());
        QCOMPARE(w.outputPages(runs[0]), odd);
        QCOMPARE(w.outputPages(runs[1]), even);
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
};

QTEST_MAIN(FileTests)
#include "tst_write_file.moc"
