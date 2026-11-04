// Tests of JeffWrite's File screen: where editing stopped, the warning before a save to a format that holds less, and the Options pages. Every file is made here in code.

#include "../isolatesettings.h"
#include "app/optionsdialog.h"
#include "app/settings.h"
#include "app/pageview.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/pdfbookmarks.h"
#include "core/templates.h"
#include "render/pdfpage.h"
#include "office/package.h"
#include "office/packageprops.h"
#include "core/writedoc.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
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
        for (const OptionsPage &p : pages)
            if (p.apply) p.apply();
        qDeleteAll(built);
        QCOMPARE(Settings::get().autoRecoverMinutes(), 3);
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
