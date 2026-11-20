// Tests of JeffWrite's Info page: the properties a .docx keeps, Mark as Final, a password, Inspect Document, and Check Compatibility. Every file is made here in code.

#include "../isolatesettings.h"
#include "../aboutchecks.h"
#include "../recoverygate.h"
#include "../privacyhooks.h"
#include "office/privacy.h"
#include "../settingguard.h"
#include "app/recovery.h"
#include "app/filetasks.h"
#include "app/filescreen.h"
#include "app/infopage.h"
#include "app/theme.h"
#include "app/passworddialogs.h"
#include "office/odfcrypt.h"
#include "app/settings.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "app/pageview.h"
#include "core/appinfo.h"
#include "core/commentops.h"
#include "core/editops.h"
#include "core/epubexport.h"
#include "core/pictureops.h"
#include "core/tableops.h"
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QSet>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTextBrowser>
#include <QImageWriter>
#include <QTimer>
#include <QBuffer>
#include <QImage>
#include "core/fieldops.h"
#include "core/fonts.h"
#include "core/inspectops.h"
#include "core/linkops.h"
#include "core/reviewops.h"
#include "io/zip.h"
#include "core/textexport.h"
#include "core/writedoc.h"
#include "office/officecrypt.h"
#include "office/package.h"
#include "office/packageprops.h"
#include "text/textprops.h"
#include "app/pdfprotection.h"
#include "../plainpdfwatch.h"
#include "../emlpdf.h"
#include "io/pdfcrypt.h"
#include "render/pdfpage.h"

#include <QCheckBox>
#include <QScopeGuard>
#include <QDirIterator>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QRegularExpression>
#include <QImage>
#include <QTemporaryDir>
#include <memory>
#include <QTest>
#include <QThread>
#include <QTimer>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
const QString kRel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
const QString kWord = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");

// A .docx of this body, with the extra parts given as (name, content type, bytes, relationship type, from) and the package parts as they are given.
struct Extra {
    QString name, contentType;
    QByteArray bytes;
    QString relType, from, id;
    bool external = false;   // a link to an address outside the file (the name is the address; no part is made)
};

QByteArray makeDocx(const QByteArray &body, const QVector<Extra> &extra = {}, const QVector<Extra> &packageLevel = {})
{
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
              kWord + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), kRel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), QByteArray("<?xml version=\"1.0\"?><w:styles ") + kNs + "><w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style></w:styles>",
              kWord + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), kRel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    for (const Extra &e : extra) {
        if (e.external) {
            w.addRelationshipWithId(e.from.isEmpty() ? QStringLiteral("/word/document.xml") : e.from, e.id, e.relType, e.name, true);
            continue;
        }
        w.addPart(e.name, e.bytes, e.contentType);
        if (!e.relType.isEmpty()) {
            if (e.id.isEmpty()) w.addRelationship(e.from.isEmpty() ? QStringLiteral("/word/document.xml") : e.from, e.relType, e.name);
            else w.addRelationshipWithId(e.from.isEmpty() ? QStringLiteral("/word/document.xml") : e.from, e.id, e.relType, e.name, false);
        }
    }
    for (const Extra &e : packageLevel) {
        w.addPart(e.name, e.bytes, e.contentType);
        w.addRelationship(QStringLiteral("/"), e.relType, e.name);
    }
    return w.finish();
}

QByteArray partOf(const QByteArray &zip, const QString &name)
{
    office::Package p;
    p.read(zip);
    return p.part(name);
}

QStringList partNamesOf(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.partNames();
}

std::unique_ptr<WriteDoc> openDocx(const QByteArray &zip)
{
    QString err;
    return WriteDoc::fromDocx(zip, &err, nullptr);
}

void typeText(WriteDoc &doc, QTextCursor &c, const QString &text)
{
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n')) insertParagraph(doc, c);
        else insertText(doc, c, QString(ch));
    }
}

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// Saves a window's document under `name` in `dir` (the Save As box standing aside).
bool saveInto(WriteWindow &w, const QTemporaryDir &dir, const QString &name)
{
    w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(name); });
    return w.saveAs();
}

const QByteArray kCompound("\xD0\xCF\x11\xE0", 4);

// The file is a protected package: it does not open when the question is cancelled, and it opens with this password.
void verifyProtected(const QString &path, const QString &password)
{
    QVERIFY(fileBytes(path).startsWith(kCompound));
    setOpenPasswordStandIn([](const QString &, bool, QString *) { return false; });
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        QVERIFY(!w.openFile(path));
    }
    setOpenPasswordStandIn([password](const QString &, bool, QString *given) {
        *given = password;
        return true;
    });
    WriteWindow w;
    w.setNotifier([](const QString &, const QString &) {});
    QVERIFY(w.openFile(path));
    QVERIFY(w.isPasswordProtected());
}

void protect(WriteWindow &w, const QString &password)
{
    setNewPasswordStandIn([password](bool, QString *given) {
        *given = password;
        return NewPasswordChoice::Set;
    });
    w.encryptWithPassword();
}

// A book for the e-book tests: front words, then chapters of Heading 1 with Heading 2 and Heading 4 inside, a footnote, a picture, and a table.
void paragraphOf(WriteDoc &doc, QTextCursor &c, const QString &text, const QString &style, bool first = false)
{
    if (!first) insertParagraph(doc, c);
    typeText(doc, c, text);
    applyParagraphStyle(doc, c, style);
}

QByteArray pngBytes(int width, int height)
{
    QImage image(width, height, QImage::Format_RGB32);
    image.fill(Qt::darkGreen);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

bool wellFormed(const QByteArray &xml, QString *why = nullptr)
{
    QXmlStreamReader r(xml);
    while (!r.atEnd()) r.readNext();
    if (r.hasError() && why) *why = r.errorString();
    return !r.hasError();
}

void buildBook(WriteWindow &w)
{
    WriteDoc &doc = *w.document();
    QTextCursor c = w.view()->cursor();
    paragraphOf(doc, c, QStringLiteral("Front words."), QStringLiteral("Normal"), true);
    paragraphOf(doc, c, QStringLiteral("Chapter One"), QStringLiteral("Heading1"));
    paragraphOf(doc, c, QStringLiteral("Noted"), QStringLiteral("Normal"));
    QTextDocument *note = insertFootnote(doc, c);
    QVERIFY(note);
    typeText(doc, c, QStringLiteral("The footnote words."));   // (the cursor is in the note)
    c = QTextCursor(doc.story());
    c.movePosition(QTextCursor::End);
    c.setCharFormat(QTextCharFormat());
    paragraphOf(doc, c, QStringLiteral("A section"), QStringLiteral("Heading2"));
    paragraphOf(doc, c, QStringLiteral("Too deep"), QStringLiteral("Heading4"));
    paragraphOf(doc, c, QStringLiteral("Chapter Two"), QStringLiteral("Heading1"));
    paragraphOf(doc, c, QStringLiteral("Pic: "), QStringLiteral("Normal"));
    QCOMPARE(insertPicture(doc, c, pngBytes(40, 20), QStringLiteral("green"), PictureWrap::Inline, QStringLiteral("A green box")), PictureResult::Ok);
    QCOMPARE(insertPicture(doc, c, pngBytes(40, 20), QStringLiteral("green"), PictureWrap::Inline, QStringLiteral("A green box")), PictureResult::Ok);
    insertParagraph(doc, c);
    NewTable table;
    table.rows = 2;
    table.columns = 2;
    QVERIFY(insertTable(doc, c, table));
}

// Acts on the next dialog that opens and closes it (the test's side of an exec() that waits).
void onNextEpubDialog(const std::function<void(QDialog *)> &act)
{
    auto *timer = new QTimer;
    timer->setInterval(20);
    QObject::connect(timer, &QTimer::timeout, [timer, act] {
        auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog || dialog->objectName() != QLatin1String("epubDialog")) return;
        timer->stop();
        timer->deleteLater();
        act(dialog);
        dialog->accept();
    });
    timer->start();
}

QMap<QString, QByteArray> unzipped(const QByteArray &bytes)
{
    QMap<QString, QByteArray> z;
    QString err;
    if (!readZip(bytes, z, &err)) qWarning("not a zip: %s", qPrintable(err));
    return z;
}

// The book of a .docx as the files it holds.
QMap<QString, QByteArray> bookOfDocx(const QByteArray &docx, const EpubExportOptions &options = EpubExportOptions(), QByteArray *whole = nullptr)
{
    QString err;
    auto doc = WriteDoc::fromDocx(docx, &err, nullptr);
    office::wml::Document snapshot;
    if (!doc || !doc->docxSnapshot(&snapshot)) return {};
    const QByteArray bytes = epubFromDocument(snapshot, options, &err);
    if (whole) *whole = bytes;
    return unzipped(bytes);
}

// Every XML file of a book parses, no id is used twice in a page, and every link inside the book goes to a page and a place that are there.
QString problemsOf(const QMap<QString, QByteArray> &z)
{
    QStringList problems;
    QMap<QString, QSet<QString>> ids;
    static const QRegularExpression idAttr(QStringLiteral("\\sid=\"([^\"]+)\"")), href(QStringLiteral("\\shref=\"([^\"]+)\""));
    for (auto it = z.cbegin(); it != z.cend(); ++it) {
        const QString &name = it.key();
        if (!name.endsWith(QLatin1String(".xhtml")) && !name.endsWith(QLatin1String(".opf")) && !name.endsWith(QLatin1String(".ncx")) && !name.endsWith(QLatin1String(".xml"))) continue;
        QString why;
        if (!wellFormed(it.value(), &why)) problems << name + QStringLiteral(": ") + why;
        const QString text = QString::fromUtf8(it.value());
        QRegularExpressionMatchIterator m = idAttr.globalMatch(text);
        while (m.hasNext()) {
            const QString id = m.next().captured(1);
            if (ids[name].contains(id)) problems << name + QStringLiteral(": id used twice: ") + id;
            ids[name].insert(id);
        }
    }
    for (auto it = z.cbegin(); it != z.cend(); ++it) {
        if (!it.key().endsWith(QLatin1String(".xhtml"))) continue;
        QRegularExpressionMatchIterator m = href.globalMatch(QString::fromUtf8(it.value()));
        while (m.hasNext()) {
            const QString link = m.next().captured(1);
            if (link.contains(QLatin1Char(':')) || link == QLatin1String("style.css")) continue;
            const QString file = link.section(QLatin1Char('#'), 0, 0), fragment = link.contains(QLatin1Char('#')) ? link.section(QLatin1Char('#'), 1) : QString();
            const QString target = file.isEmpty() ? it.key() : QStringLiteral("OEBPS/") + file;
            if (!z.contains(target)) problems << it.key() + QStringLiteral(": no page for ") + link;
            else if (!fragment.isEmpty() && !ids.value(target).contains(fragment)) problems << it.key() + QStringLiteral(": no place for ") + link;
        }
    }
    return problems.join(QLatin1Char('\n'));
}

const char kDrawingNs[] = " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"";

QByteArray drawingRun(const QString &rid, const QString &alt)
{
    return QByteArray("<w:r><w:drawing><wp:inline") + kDrawingNs + "><wp:extent cx=\"914400\" cy=\"457200\"/><wp:docPr id=\"1\" name=\"P\" descr=\"" + alt.toUtf8() + "\"/>"
           "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"p\"/><pic:cNvPicPr/></pic:nvPicPr>"
           "<pic:blipFill><a:blip r:embed=\"" + rid.toUtf8() + "\"/></pic:blipFill><pic:spPr/></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r>";
}

QByteArray emfBytes()
{
    auto le32 = [](QByteArray &b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF)); };
    QByteArray emf;
    le32(emf, 1);
    le32(emf, 88);
    for (int v : {0, 0, 100, 100, 0, 0, 2540, 2540}) le32(emf, quint32(v));   // bounds, frame
    le32(emf, 0x464D4520u);
    le32(emf, 0x10000);
    le32(emf, 88 + 24 + 20);
    le32(emf, 3);
    emf.append(char(1)).append(char(0)).append(char(0)).append(char(0));
    for (int i = 0; i < 3; ++i) le32(emf, 0);
    for (int v : {1024, 768, 320, 240}) le32(emf, quint32(v));
    le32(emf, 43);   // a rectangle
    le32(emf, 24);
    for (int v : {10, 10, 90, 90}) le32(emf, quint32(v));
    le32(emf, 14);   // the end
    le32(emf, 20);
    le32(emf, 0);
    le32(emf, 16);
    le32(emf, 20);
    return emf;
}

QByteArray wmfBytes()
{
    auto le16 = [](QByteArray &b, int v) { b.append(char(v & 0xFF)); b.append(char((v >> 8) & 0xFF)); };
    auto le32 = [](QByteArray &b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF)); };
    QByteArray wmf;
    le32(wmf, 0x9AC6CDD7u);
    le16(wmf, 0);
    for (int v : {0, 0, 1000, 1000}) le16(wmf, v);
    le16(wmf, 1440);
    le32(wmf, 0);
    le16(wmf, 0);
    le16(wmf, 1);       // the header: a memory metafile, 9 words, version 3
    le16(wmf, 9);
    le16(wmf, 0x300);
    le32(wmf, 19);      // words in all
    le16(wmf, 0);
    le32(wmf, 7);       // the longest record
    le16(wmf, 0);
    le32(wmf, 7);       // a rectangle
    le16(wmf, 0x041B);
    for (int v : {900, 900, 100, 100}) le16(wmf, v);
    le32(wmf, 3);       // the end
    le16(wmf, 0);
    return wmf;
}

QByteArray noisePng(int side)
{
    QImage image(side, side, QImage::Format_RGB32);
    QRandomGenerator random(7);
    for (int y = 0; y < side; ++y)
        for (int x = 0; x < side; ++x) image.setPixel(x, y, random.generate() | 0xFF000000u);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

const QString kNotesNs = QStringLiteral("xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"");

QByteArray footnotesPart(const QString &second)
{
    return QByteArray("<?xml version=\"1.0\"?><w:footnotes ") + kNotesNs.toUtf8() + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
           "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>"
           "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> " + second.toUtf8() + "</w:t></w:r></w:p></w:footnote>"
           "<w:footnote w:id=\"3\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Second note.</w:t></w:r></w:p></w:footnote></w:footnotes>";
}

QVector<Extra> notesExtra(const QString &second = QStringLiteral("First note."))
{
    return {{QStringLiteral("/word/footnotes.xml"), kWord + QStringLiteral("footnotes+xml"), footnotesPart(second), kRel + QStringLiteral("footnotes"), QString(), QString()}};
}

struct Quiet {
    Quiet(WriteWindow &w) { w.setNotifier([this](const QString &, const QString &text) { said << text; }); }
    QStringList said;
};

} // namespace

class InfoTests : public QObject {
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
    void cleanup()
    {
        setNewPasswordStandIn(nullptr);
        setOpenPasswordStandIn(nullptr);
    }

    // About is the page all four apps share: Check for Updates (a fake here), the automatic check, and the statistics choice.
    void aboutIsTheSharedPageWithJeffWritesFacts()
    {
        WriteWindow w;
        w.resize(1200, 800);
        w.show();
        w.showFileScreen(QStringLiteral("about"));
        jptest::checkAboutPage(w.findChild<FileScreen *>(), {QStringLiteral("documents"), QStringLiteral("LibreTranslate"), QStringLiteral("libmspub")});
    }

    // Options > Save > "Remove personal information from the file's properties when saving": the names are in the window's properties until a file is written, and not in it.
    void thePrivacyOptionTakesTheNamesOutOfTheFileAsItIsSaved()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("words"));
        office::PackageProps p = a.document()->packageProps();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.title = QStringLiteral("Kept");
        a.applyProperties(p);
        QVERIFY(saveInto(a, dir, QStringLiteral("named.docx")));
        WriteWindow named;
        QVERIFY(named.openFile(dir.filePath(QStringLiteral("named.docx"))));
        QCOMPARE(named.document()->packageProps().creator, QStringLiteral("Ann Lee"));
        QVector<OptionsPage> pages = a.optionsPages();
        QList<QWidget *> built;
        for (const OptionsPage &page : pages) built << page.build();
        QCheckBox *box = nullptr;
        for (QWidget *page : built)
            if (!box) box = page->findChild<QCheckBox *>(QStringLiteral("removePersonalOnSave"));
        QVERIFY(box);
        QVERIFY(!box->isChecked());
        box->setChecked(true);
        for (const OptionsPage &page : pages)
            if (page.apply) page.apply();
        qDeleteAll(built);
        QVERIFY(Settings::get().value(QStringLiteral("privacy/removeOnSave")).toBool());
        a.document()->story()->setModified(true);
        QVERIFY(saveInto(a, dir, QStringLiteral("clean.docx")));
        WriteWindow clean;
        QVERIFY(clean.openFile(dir.filePath(QStringLiteral("clean.docx"))));
        const office::PackageProps back = clean.document()->packageProps();
        QVERIFY(back.creator.isEmpty());
        QVERIFY(back.company.isEmpty());
        QCOMPARE(back.title, QStringLiteral("Kept"));   // (only the names go)
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
    }

    // The option takes every name out of the file, and nothing the file says: the comment's and the change's authors, with the times of changes, as well as the properties.
    void thePrivacyOptionTakesTheNamesOnCommentsAndChangesOutAndKeepsTheContent()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("first words here"));
        a.document()->setReviewer(QStringLiteral("Ann Lee"), QStringLiteral("AL"));
        QTextCursor c(a.document()->story());
        c.setPosition(2);
        c.setPosition(7, QTextCursor::KeepAnchor);
        QVERIFY(addComment(*a.document(), c, QStringLiteral("Check this")) > 0);
        a.document()->setTrackChanges(true);
        QTextCursor at(a.document()->story());
        at.setPosition(16);
        insertText(*a.document(), at, QStringLiteral("!"));
        office::PackageProps p = a.document()->packageProps();
        p.creator = QStringLiteral("Ann Lee");
        p.lastModifiedBy = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo Ray");
        p.title = QStringLiteral("Kept");
        a.applyProperties(p);
        const QString text = a.document()->story()->toPlainText();
        QVERIFY(saveInto(a, dir, QStringLiteral("clean.docx")));
        // Not one name in any part of the file.
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(fileBytes(dir.filePath(QStringLiteral("clean.docx"))), parts));
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            for (const char *name : {"Ann Lee", "Acme", "Bo Ray", "w:initials=\"AL\""})
                QVERIFY2(!it.value().contains(name), qPrintable(it.key() + QStringLiteral(": ") + QString::fromLatin1(name)));
        WriteWindow back;
        QVERIFY(back.openFile(dir.filePath(QStringLiteral("clean.docx"))));
        const office::PackageProps q = back.document()->packageProps();
        QVERIFY(q.creator.isEmpty() && q.lastModifiedBy.isEmpty() && q.company.isEmpty() && q.manager.isEmpty());
        QCOMPARE(q.title, QStringLiteral("Kept"));
        const QVector<Comment> all = comments(*back.document());
        QCOMPARE(all.size(), 1);
        QCOMPARE(all[0].text, QStringLiteral("Check this"));
        QVERIFY(all[0].author != QStringLiteral("Ann Lee"));
        QVERIFY(all[0].initials != QStringLiteral("AL"));
        const QVector<Revision> changes = revisionsIn(back.document()->story());
        QVERIFY(!changes.isEmpty());
        for (const Revision &r : changes) {
            QVERIFY(r.author != QStringLiteral("Ann Lee"));
            QVERIFY(!r.date.isValid());
        }
        QCOMPARE(back.document()->story()->toPlainText(), text);
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
    }

    // The open document is not the thing that is scrubbed: a copy, and a save, leave its names, its history, and its mark as saved exactly as they were.
    void thePrivacyOptionLeavesTheOpenDocumentAsItWas()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QTemporaryDir dir;
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("words"));
        office::PackageProps p = a.document()->packageProps();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        a.applyProperties(p);
        QVERIFY(saveInto(a, dir, QStringLiteral("first.docx")));
        const int steps = a.document()->story()->availableUndoSteps();
        QVERIFY(!a.isWindowModified());
        QVERIFY(a.saveTo(dir.filePath(QStringLiteral("copy.docx")), true));
        QCOMPARE(a.document()->packageProps().creator, QStringLiteral("Ann Lee"));
        QCOMPARE(a.document()->packageProps().company, QStringLiteral("Acme"));
        QCOMPARE(a.document()->story()->availableUndoSteps(), steps);
        QVERIFY(!a.isWindowModified());
        QVERIFY(saveInto(a, dir, QStringLiteral("second.docx")));
        QCOMPARE(a.document()->packageProps().creator, QStringLiteral("Ann Lee"));
        QCOMPARE(a.document()->story()->availableUndoSteps(), steps);
        QVERIFY(!a.isWindowModified());
        // (and the files have none)
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(fileBytes(dir.filePath(QStringLiteral("copy.docx"))), parts));
        QVERIFY(!parts.value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
    }

    // Every kind of file a document can be saved as has no names in it: .rtf and .odt carry the properties, comments, and tracked changes of the document too.
    void thePrivacyOptionKeepsNamesOutOfRtfAndOdtFiles()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QTemporaryDir dir;
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("first words here"));
        a.document()->setReviewer(QStringLiteral("Ann Lee"), QStringLiteral("AL"));
        QTextCursor c(a.document()->story());
        c.setPosition(2);
        c.setPosition(7, QTextCursor::KeepAnchor);
        QVERIFY(addComment(*a.document(), c, QStringLiteral("Check this")) > 0);
        a.document()->setTrackChanges(true);
        QTextCursor at(a.document()->story());
        at.setPosition(16);
        insertText(*a.document(), at, QStringLiteral("!"));
        office::PackageProps p = a.document()->packageProps();
        p.creator = QStringLiteral("Ann Lee");
        p.lastModifiedBy = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo Ray");
        a.applyProperties(p);
        for (const char *name : {"names.rtf", "names.odt"}) {
            QVERIFY(saveInto(a, dir, QString::fromLatin1(name)));
            const QByteArray bytes = fileBytes(dir.filePath(QString::fromLatin1(name)));
            QVERIFY(bytes.size() > 0);
            QMap<QString, QByteArray> parts;
            if (!readZip(bytes, parts)) parts.insert(QStringLiteral("whole"), bytes);
            for (auto it = parts.cbegin(); it != parts.cend(); ++it)
                for (const char *who : {"Ann Lee", "Acme", "Bo Ray", "Ann\\'", "Tester"})
                    QVERIFY2(!it.value().contains(who), qPrintable(QString::fromLatin1(name) + QLatin1Char(' ') + it.key() + QStringLiteral(": ") + QString::fromLatin1(who)));
        }
    }

    // When the names can't be taken out the file is still written, and the person is told, with the reason.
    void whenTheNamesCantBeTakenOutThePersonIsToldAndTheFileIsStillWritten()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] {
            Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
            office::setPackageScrubHookForTest({});
        });
        QTemporaryDir dir;
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("words"));
        office::PackageProps p = a.document()->packageProps();
        p.creator = QStringLiteral("Ann Lee");
        a.applyProperties(p);
        office::setPackageScrubHookForTest(jptest::damagePart("docProps/core.xml"));
        QVERIFY(saveInto(a, dir, QStringLiteral("told.docx")));
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("told.docx"))));
        QVERIFY2(quiet.said.join(QLatin1Char('|')).contains(QLatin1String("personal information could not be taken out")), qPrintable(quiet.said.join(QLatin1Char('|'))));
        QVERIFY(quiet.said.join(QLatin1Char('|')).contains(QLatin1String("docProps/core.xml")));
    }

    // A document sent by email has the names out too.
    void aDocumentSentByEmailHasNoNamesWhenTheOptionIsOn()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        QTemporaryDir dir;
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("words"));
        office::PackageProps p = a.document()->packageProps();
        p.creator = QStringLiteral("Ann Lee");
        a.applyProperties(p);
        a.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("m.eml")); });
        QVERIFY(a.exportEmail(WriteWindow::EmailWhat::Document));
        const QByteArray docx = jptest::attachmentOf(fileBytes(dir.filePath(QStringLiteral("m.eml"))), ".docx");
        QMap<QString, QByteArray> parts;
        QVERIFY2(readZip(docx, parts), "the attachment is a package");
        QVERIFY(!parts.value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
        QCOMPARE(a.document()->packageProps().creator, QStringLiteral("Ann Lee"));
    }

    // With the option off a save is what it was: the parts are those of the document's own export.
    void withThePrivacyOptionOffASaveIsUnchanged()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        QTemporaryDir dir;
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("words"));
        office::PackageProps p = a.document()->packageProps();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        a.applyProperties(p);
        QVERIFY(saveInto(a, dir, QStringLiteral("off.docx")));
        auto stable = [](const QByteArray &zip) {
            QMap<QString, QByteArray> parts;
            readZip(zip, parts);
            QMap<QString, QByteArray> out;
            for (auto it = parts.cbegin(); it != parts.cend(); ++it) out.insert(it.key(), QString::fromUtf8(it.value()).replace(QRegularExpression(QStringLiteral("\\d{4}-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d")), QStringLiteral("T")).toUtf8());
            return out;
        };
        QCOMPARE(stable(fileBytes(dir.filePath(QStringLiteral("off.docx")))), stable(a.document()->toDocx()));
        QVERIFY(fileBytes(dir.filePath(QStringLiteral("off.docx"))).size() > 0);
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(fileBytes(dir.filePath(QStringLiteral("off.docx"))), parts));
        QVERIFY(parts.value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
        QVERIFY(parts.value(QStringLiteral("docProps/app.xml")).contains("Acme"));
    }

    // The Info page has the box for the option, on the setting the apps share.
    void theInfoPageHasTheBoxForThePrivacyOption()
    {
        WriteWindow w;
        const QVariant priorSetting = Settings::get().value(QStringLiteral("privacy/removeOnSave"), false);
        const auto restore = qScopeGuard([priorSetting] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), priorSetting); });
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        std::unique_ptr<QWidget> page(w.buildInfoPage(nullptr));
        auto *box = page->findChild<QCheckBox *>(QStringLiteral("infoRemovePersonal"));
        QVERIFY(box);
        QCOMPARE(box->text(), QStringLiteral("Remove personal information from this file when saving"));
        QVERIFY(!box->isChecked());
        box->setChecked(true);
        QVERIFY(Settings::get().value(QStringLiteral("privacy/removeOnSave")).toBool());
    }

    void propertiesSurviveASaveAndReopen()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("one two three\nfour"));
        office::PackageProps p = a.document()->packageProps();
        p.title = QStringLiteral("Quarterly notes");
        p.creator = QStringLiteral("Ann Lee");
        p.keywords = QStringLiteral("notes, q3");
        p.company = QStringLiteral("Acme");
        p.custom = {{QStringLiteral("Project"), QStringLiteral("lpwstr"), QStringLiteral("Apollo")}};
        a.applyProperties(p);
        QVERIFY(a.isModified());
        QVERIFY(saveInto(a, dir, QStringLiteral("p.docx")));
        WriteWindow b;
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("p.docx"))));
        const office::PackageProps back = b.document()->packageProps();
        QCOMPARE(back.title, QStringLiteral("Quarterly notes"));
        QCOMPARE(back.creator, QStringLiteral("Ann Lee"));
        QCOMPARE(back.company, QStringLiteral("Acme"));
        QCOMPARE(back.words, 4);
        QCOMPARE(back.paragraphs, 2);
        QVERIFY(back.created.isValid());
        QVERIFY(back.modified.isValid());
        QCOMPARE(back.custom.size(), 1);
        QCOMPARE(back.custom[0].value, QStringLiteral("Apollo"));
        // (the parts are in the package with the relationships that name them)
        office::Package pkg;
        QVERIFY(pkg.read(fileBytes(dir.filePath(QStringLiteral("p.docx")))));
        QVERIFY(!pkg.part(QStringLiteral("/docProps/custom.xml")).isEmpty());
        QCOMPARE(pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/custom-properties")).size(), 1);
    }

    void aCustomPropertyIsShownByAField()
    {
        WriteWindow w;
        office::PackageProps p = w.document()->packageProps();
        p.custom = {{QStringLiteral("Client Name"), QStringLiteral("lpwstr"), QStringLiteral("Acme")}, {QStringLiteral("Open"), QStringLiteral("bool"), QStringLiteral("true")}};
        w.applyProperties(p);
        const fields::Properties props = documentProperties(*w.document());
        QCOMPARE(props.custom.value(QStringLiteral("client name")), QStringLiteral("Acme"));
        fields::Env env;
        env.properties = props;
        QCOMPARE(*fields::compute(fields::parseCode(QStringLiteral("DOCPROPERTY \"client name\"")), env), QStringLiteral("Acme"));
        QCOMPARE(*fields::compute(fields::parseCode(QStringLiteral("DOCPROPERTY Open")), env), QStringLiteral("Yes"));
        QVERIFY(!fields::compute(fields::parseCode(QStringLiteral("DOCPROPERTY Nothing")), env).has_value());
    }

    void markedAsFinalOpensForReadingOnly()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Final words"));
        QVERIFY(saveInto(a, dir, QStringLiteral("f.docx")));
        a.setMarkedFinal(true);
        QVERIFY(a.isReadOnly());
        QVERIFY(a.document()->markedAsFinal());
        QVERIFY(!a.isModified());
        WriteWindow b;
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("f.docx"))));
        QVERIFY(b.isReadOnly());
        QVERIFY(b.statusBar()->currentMessage().contains(QLatin1String("Marked as final")));
        b.setMarkedFinal(false);
        QVERIFY(!b.isReadOnly());
        QVERIFY(!b.document()->markedAsFinal());
        // opened as a copy, it is a document to edit
        WriteWindow c;
        QVERIFY(c.openFile(dir.filePath(QStringLiteral("f.docx")), OpenHow::Copy));
        QVERIFY(!c.isReadOnly());
    }

    void alwaysOpenReadOnlyIsKeptInTheFileAndOpensItForReadingOnly()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Read me"));
        QVERIFY(!a.alwaysOpensReadOnly());
        a.setAlwaysOpenReadOnly(true);
        QVERIFY(a.alwaysOpensReadOnly());
        QVERIFY(!a.isReadOnly());   // (it applies the next time the file is opened)
        QVERIFY(saveInto(a, dir, QStringLiteral("r.docx")));
        // opened, it is for reading only, with a note; the Info page's card turns it off and lets the window edit
        WriteWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("r.docx"))));
        QVERIFY(b.isReadOnly());
        QVERIFY(b.alwaysOpensReadOnly());
        QVERIFY(b.statusBar()->currentMessage().contains(QLatin1String("reading only")));
        b.showFileScreen(QStringLiteral("info"));
        auto *card = b.findChild<FileScreen *>()->findChild<QAbstractButton *>(QStringLiteral("infoAlwaysReadOnly"));
        QVERIFY(card);
        QVERIFY(card->isEnabled());
        card->click();
        QVERIFY(!b.alwaysOpensReadOnly());
        QVERIFY(!b.isReadOnly());
        QVERIFY(b.save());
        WriteWindow c;
        Quiet quietC(c);
        QVERIFY(c.openFile(dir.filePath(QStringLiteral("r.docx"))));
        QVERIFY(!c.isReadOnly());
        // opened as a copy it is a document to edit
        a.setAlwaysOpenReadOnly(true);
        QVERIFY(saveInto(a, dir, QStringLiteral("r2.docx")));
        WriteWindow d;
        Quiet quietD(d);
        QVERIFY(d.openFile(dir.filePath(QStringLiteral("r2.docx")), OpenHow::Copy));
        QVERIFY(!d.isReadOnly());
    }

    void turningAlwaysOpenReadOnlyOffKeepsAWindowThatIsReadOnlyForAnotherReason()
    {
        QTemporaryDir dir;
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Words"));
        a.setAlwaysOpenReadOnly(true);
        QVERIFY(saveInto(a, dir, QStringLiteral("k.docx")));
        WriteWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("k.docx")), OpenHow::ReadOnly));
        QVERIFY(b.isReadOnly());
        b.setAlwaysOpenReadOnly(false);
        QVERIFY(!b.alwaysOpensReadOnly());
        QVERIFY(b.isReadOnly());
    }

    // A .odt file can't hold the choice: the card says so, Check Compatibility lists it, and a save as .odt stops the window saying it is kept.
    void alwaysOpenReadOnlyIsListedAndClearedWhenSavedAsAnOdt()
    {
        QTemporaryDir dir;
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Words"));
        QVERIFY(saveInto(a, dir, QStringLiteral("l.docx")));
        QVERIFY(a.alwaysReadOnlyBlocked().isEmpty());
        a.setAlwaysOpenReadOnly(true);
        QVERIFY(a.incompatibilities().join(QLatin1Char('|')).contains(QLatin1String("always open")));
        QVERIFY(saveInto(a, dir, QStringLiteral("l.odt")));
        QVERIFY(!a.alwaysOpensReadOnly());
        QVERIFY(!a.alwaysReadOnlyBlocked().isEmpty());
        a.setAlwaysOpenReadOnly(true);
        QVERIFY(!a.alwaysOpensReadOnly());
        WriteWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("l.odt"))));
        QVERIFY(!b.isReadOnly());
    }

    // Pictures for a person to look at (JW_SHOT_DIR names the folder; nothing is made without it).
    void shotInfoPage()
    {
        const QString dir = qEnvironmentVariable("JW_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JW_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Read me"));
        w.setAlwaysOpenReadOnly(true);
        w.resize(1300, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.showFileScreen(QStringLiteral("info"));
        QTest::qWait(300);
        QDir().mkpath(dir);
        QVERIFY(w.grab().save(dir + QStringLiteral("/write-info.png")));
    }

    void aPasswordProtectsTheFileAndOpensItAgain()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Secret plans"));
        setNewPasswordStandIn([](bool, QString *password) {
            *password = QStringLiteral("hunter2");
            return NewPasswordChoice::Set;
        });
        a.encryptWithPassword();
        QVERIFY(a.isPasswordProtected());
        QVERIFY(saveInto(a, dir, QStringLiteral("s.docx")));
        const QByteArray bytes = fileBytes(dir.filePath(QStringLiteral("s.docx")));
        QVERIFY(bytes.startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)));
        QVERIFY(!bytes.contains("Secret plans"));
        // asked for the password, a wrong one first
        QStringList tries = {QStringLiteral("nope"), QStringLiteral("hunter2")};
        QList<bool> wrongFlags;
        setOpenPasswordStandIn([&](const QString &, bool wrong, QString *password) {
            wrongFlags << wrong;
            if (tries.isEmpty()) return false;
            *password = tries.takeFirst();
            return true;
        });
        WriteWindow b;
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("s.docx"))));
        QCOMPARE(b.document()->story()->toPlainText(), QStringLiteral("Secret plans"));
        QCOMPARE(wrongFlags, (QList<bool>{false, true}));
        QVERIFY(b.isPasswordProtected());
        // a plain save keeps the protection
        typeText(*b.document(), b.view()->cursor(), QStringLiteral("!"));
        QVERIFY(b.save());
        QVERIFY(fileBytes(dir.filePath(QStringLiteral("s.docx"))).startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)));
        // cancelling the question opens nothing, and says nothing
        setOpenPasswordStandIn([](const QString &, bool, QString *) { return false; });
        WriteWindow c;
        QVERIFY(!c.openFile(dir.filePath(QStringLiteral("s.docx"))));
        // the password taken off
        setNewPasswordStandIn([](bool has, QString *) {
            return has ? NewPasswordChoice::Removed : NewPasswordChoice::Cancelled;
        });
        b.encryptWithPassword();
        QVERIFY(!b.isPasswordProtected());
        QVERIFY(b.save());
        QVERIFY(fileBytes(dir.filePath(QStringLiteral("s.docx"))).startsWith("PK"));
    }

    void aTemplateOfAProtectedDocumentIsProtectedToo()
    {
        jptest::TestModeGuard testMode;
        QDir(myTemplatesFolder()).removeRecursively();
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Secret plans"));
        setNewPasswordStandIn([](bool, QString *password) {
            *password = QStringLiteral("hunter2");
            return NewPasswordChoice::Set;
        });
        w.encryptWithPassword();
        setTemplateNameStandIn([](const QString &) { return QStringLiteral("Secret"); });
        const bool saved = w.saveAsTemplate();
        setTemplateNameStandIn({});
        QVERIFY(saved);
        const QStringList found = myTemplateFiles({QStringLiteral("*.dotx")});
        QCOMPARE(found.size(), 1);
        const QByteArray bytes = fileBytes(found.first());
        QVERIFY(bytes.startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)));
        QVERIFY(!bytes.contains("Secret plans"));
        QVERIFY(removeMyTemplate(found.first()));
    }

    // Going back to a version keeps the password, and the version itself was kept protected.
    void goingBackToAVersionOfAProtectedDocumentKeepsThePassword()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Secret plans"));
        protect(w, QStringLiteral("hunter2"));
        QVERIFY(saveInto(w, dir, QStringLiteral("v.docx")));
        const QString path = dir.filePath(QStringLiteral("v.docx"));
        QVERIFY(w.saveVersion(QStringLiteral("First")));
        QVERIFY(fileBytes(write::VersionStore(path).folder() + QLatin1Char('/') + w.versions().first().file).startsWith(kCompound));
        typeText(*w.document(), w.view()->cursor(), QStringLiteral(" and more"));
        QVERIFY(w.restoreVersion(w.versions().first().file));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Secret plans"));
        QVERIFY(w.isPasswordProtected());
        QVERIFY(w.save());
        verifyProtected(path, QStringLiteral("hunter2"));
    }

    // A password put on, changed, or taken off is put on every kept version, so none is left in the clear and Go Back still works.
    void versionsFollowThePassword()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Plans"));
        QVERIFY(saveInto(w, dir, QStringLiteral("f.docx")));
        const QString path = dir.filePath(QStringLiteral("f.docx"));
        QVERIFY(w.saveVersion(QStringLiteral("Plain")));
        const write::VersionStore store(path);
        auto versionFile = [&] { return store.folder() + QLatin1Char('/') + w.versions().first().file; };
        QVERIFY(fileBytes(versionFile()).startsWith("PK"));
        protect(w, QStringLiteral("one"));
        QVERIFY(fileBytes(versionFile()).startsWith("PK"));   // (the file is not saved with it yet)
        QVERIFY(saveInto(w, dir, QStringLiteral("f.docx")));
        QVERIFY(fileBytes(versionFile()).startsWith(kCompound));
        protect(w, QStringLiteral("two"));
        QVERIFY(saveInto(w, dir, QStringLiteral("f.docx")));
        QVERIFY(w.restoreVersion(w.versions().first().file));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Plans"));
        setNewPasswordStandIn([](bool has, QString *) { return has ? NewPasswordChoice::Removed : NewPasswordChoice::Cancelled; });
        w.encryptWithPassword();
        QVERIFY(saveInto(w, dir, QStringLiteral("f.docx")));
        for (const write::DocVersion &v : w.versions()) QVERIFY2(fileBytes(store.folder() + QLatin1Char('/') + v.file).startsWith("PK"), qPrintable(v.name));
    }

    // A password taken off or changed and then not saved leaves the versions as the file on disk has them: protected, and opening with the file's password.
    void aPasswordChangeThatIsNotSavedLeavesTheVersionsAlone()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Plans"));
        protect(w, QStringLiteral("one"));
        QVERIFY(saveInto(w, dir, QStringLiteral("g.docx")));
        const QString path = dir.filePath(QStringLiteral("g.docx"));
        QVERIFY(w.saveVersion(QStringLiteral("V")));
        const write::VersionStore store(path);
        auto versionFile = [&] { return store.folder() + QLatin1Char('/') + w.versions().first().file; };
        auto opensWith = [&](const QString &password) {
            QByteArray package;
            return write::versionPackage(fileBytes(versionFile()), password, &package);
        };
        QVERIFY(fileBytes(versionFile()).startsWith(kCompound));
        setNewPasswordStandIn([](bool has, QString *) { return has ? NewPasswordChoice::Removed : NewPasswordChoice::Cancelled; });
        w.encryptWithPassword();
        QVERIFY(w.saveVersion(QStringLiteral("W")));
        for (const write::DocVersion &v : w.versions()) QVERIFY2(fileBytes(store.folder() + QLatin1Char('/') + v.file).startsWith(kCompound), qPrintable(v.name));
        protect(w, QStringLiteral("two"));
        QVERIFY(opensWith(QStringLiteral("one")));
        QVERIFY(!opensWith(QStringLiteral("two")));
        QVERIFY(saveInto(w, dir, QStringLiteral("g.docx")));
        QVERIFY(opensWith(QStringLiteral("two")));
        QVERIFY(!opensWith(QStringLiteral("one")));
    }

    // A copy of a protected file is protected too: it keeps the password.
    void aCopyOfAProtectedDocumentKeepsThePassword()
    {
        QTemporaryDir dir;
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Secret plans"));
        protect(a, QStringLiteral("hunter2"));
        QVERIFY(saveInto(a, dir, QStringLiteral("p.docx")));
        setOpenPasswordStandIn([](const QString &, bool, QString *given) {
            *given = QStringLiteral("hunter2");
            return true;
        });
        WriteWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(dir.filePath(QStringLiteral("p.docx")), OpenHow::Copy));
        QVERIFY(b.isPasswordProtected());
        QVERIFY(saveInto(b, dir, QStringLiteral("copy.docx")));
        verifyProtected(dir.filePath(QStringLiteral("copy.docx")), QStringLiteral("hunter2"));
    }

    // "Always create backup copy": the backup of a document that is protected is protected, even when the password has only just been put on.
    void theBackupCopyIsProtectedToo()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Plans"));
        QVERIFY(saveInto(w, dir, QStringLiteral("b.docx")));
        const QString path = dir.filePath(QStringLiteral("b.docx"));
        protect(w, QStringLiteral("hunter2"));
        jptest::SettingGuard keep(QStringLiteral("save/backup"), true, false);
        QVERIFY(w.save());
        const QString backup = dir.filePath(QStringLiteral("Backup of b.docx"));
        QVERIFY(QFileInfo::exists(backup));
        verifyProtected(backup, QStringLiteral("hunter2"));
    }

    // Mark as Final on a document that is then saved as a kind of file that can't hold the mark leaves the window open for editing.
    void markAsFinalDoesNotHoldAWindowWhoseFileCantKeepTheMark()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("Words"));
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("m.odt")); });
        w.setMarkedFinal(true);
        QVERIFY(QFileInfo::exists(dir.filePath(QStringLiteral("m.odt"))));
        QVERIFY(!w.document()->markedAsFinal());
        QVERIFY(!w.isReadOnly());
    }

    void aProtectedDocumentIsSavedAsAnOdtWithItsPasswordAndOpensAgainWithIt()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("Secret odt plans"));
        protect(a, QStringLiteral("pw odt"));
        const QString path = dir.filePath(QStringLiteral("o.odt"));
        QVERIFY(a.saveTo(path));
        QVERIFY(quiet.said.isEmpty());
        // a zip whose parts are enciphered: the type first and plain, and none of the text in the clear
        const QByteArray bytes = fileBytes(path);
        QVERIFY(bytes.startsWith("PK"));
        QVERIFY(bytes.mid(30, 8) == "mimetype");
        QVERIFY(!bytes.contains("Secret odt plans"));
        QVERIFY(office::odfEncrypted(bytes));
        // a wrong password is asked again, then the right one opens it, and the window holds the password
        QStringList tries = {QStringLiteral("wrong"), QStringLiteral("pw odt")};
        QList<bool> wrongFlags;
        setOpenPasswordStandIn([&](const QString &, bool wrong, QString *password) {
            wrongFlags << wrong;
            if (tries.isEmpty()) return false;
            *password = tries.takeFirst();
            return true;
        });
        WriteWindow b;
        QVERIFY(b.openFile(path));
        QCOMPARE(b.document()->story()->toPlainText(), QStringLiteral("Secret odt plans"));
        QCOMPARE(wrongFlags, (QList<bool>{false, true}));
        QVERIFY(b.isPasswordProtected());
        // a plain save keeps the protection; cancelling the question opens nothing
        typeText(*b.document(), b.view()->cursor(), QStringLiteral("!"));
        QVERIFY(b.save());
        QVERIFY(office::odfEncrypted(fileBytes(path)));
        setOpenPasswordStandIn([](const QString &, bool, QString *) { return false; });
        WriteWindow c;
        c.setNotifier([](const QString &, const QString &) {});
        QVERIFY(!c.openFile(path));
        // the password taken off: the next save is plain
        setNewPasswordStandIn([](bool has, QString *) { return has ? NewPasswordChoice::Removed : NewPasswordChoice::Cancelled; });
        b.encryptWithPassword();
        QVERIFY(b.save());
        QVERIFY(!office::odfEncrypted(fileBytes(path)));
        QVERIFY(fileBytes(path).contains("PK"));
    }

    void aProtectionThatFailsOnSaveSaysWhyAndWritesNothing()
    {
        QTemporaryDir dir;
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("x"));
        protect(a, QStringLiteral("pw"));
        office::setOdfProtectLimitForTest(10);
        const bool saved = a.saveTo(dir.filePath(QStringLiteral("f.odt")));
        office::setOdfProtectLimitForTest(-1);
        QVERIFY(!saved);
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("f.odt"))));
        QVERIFY2(quiet.said.last().contains(QLatin1String("password couldn't be put on the file")), qPrintable(quiet.said.last()));
    }

    void inspectFindsAndRemovesWhatIsThere()
    {
        WriteWindow w;
        WriteDoc &doc = *w.document();
        typeText(doc, w.view()->cursor(), QStringLiteral("visible text"));
        QTextCursor c(doc.story());
        c.setPosition(7);
        c.setPosition(12, QTextCursor::KeepAnchor);
        QTextCharFormat hidden;
        hidden.setProperty(tp::HiddenText, true);
        c.mergeCharFormat(hidden);
        office::PackageProps p = doc.packageProps();
        p.creator = QStringLiteral("Ann");
        p.company = QStringLiteral("Acme");
        doc.setPackageProps(p);
        doc.putPackagePart(QStringLiteral("/customXml/item1.xml"), QStringLiteral("application/xml"), "<a/>");
        QTextCursor at = w.view()->cursor();
        at.setPosition(3);
        QVERIFY(addComment(doc, at, QStringLiteral("hello")) >= 0);
        const QVector<InspectItem> items = w.inspectItems();
        auto find = [&](const char *title) -> const InspectItem * {
            for (const InspectItem &i : items)
                if (i.title == QLatin1String(title)) return &i;
            return nullptr;
        };
        QCOMPARE(find("Comments")->count(), 1);
        QCOMPARE(find("Document properties and personal information")->count(), 2);
        QCOMPARE(find("Hidden text")->count(), 1);
        QCOMPARE(find("Custom XML data")->count(), 1);
        QVERIFY(find("Tracked changes")->removable);
        QCOMPARE(find("Tracked changes")->count(), 0);
        QCOMPARE(find("Comments")->remove(), 1);
        QCOMPARE(find("Document properties and personal information")->remove(), 2);
        QCOMPARE(find("Hidden text")->remove(), 1);
        QCOMPARE(find("Custom XML data")->remove(), 1);
        QCOMPARE(doc.story()->toPlainText(), QStringLiteral("visible"));
        QCOMPARE(find("Comments")->count(), 0);
        QCOMPARE(find("Document properties and personal information")->count(), 0);
        QCOMPARE(find("Hidden text")->count(), 0);
        QCOMPARE(find("Custom XML data")->count(), 0);
        QVERIFY(doc.partData(QStringLiteral("/customXml/item1.xml")).isEmpty());
        // the dialog says what it found and removes on a press
        InspectDialog dialog(QStringLiteral("Inspect"), items, nullptr);
        QVERIFY(dialog.summaries().first().contains(QLatin1String("None found")));
    }

    // External links are the body's links to other files: a linked picture, a linked object, and a field that reads another file. A web link is not one.
    void inspectFindsAndRemovesLinksToOtherFiles()
    {
        const QByteArray picture = "<w:r><w:drawing><wp:inline xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\"><wp:extent cx=\"914400\" cy=\"914400\"/><wp:docPr id=\"1\" name=\"Linked\"/>"
                                   "<a:graphic xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
                                   "<pic:pic xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"p.png\"/><pic:cNvPicPr/></pic:nvPicPr>"
                                   "<pic:blipFill><a:blip r:link=\"rId9\"/></pic:blipFill><pic:spPr/></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r>";
        const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">Before </w:t></w:r>" + picture + "<w:r><w:t xml:space=\"preserve\"> and </w:t></w:r>"
                                "<w:hyperlink r:id=\"rId8\"><w:r><w:t>web</w:t></w:r></w:hyperlink>"
                                "<w:fldSimple w:instr=\" INCLUDETEXT &quot;other.docx&quot; \"><w:r><w:t>from the other file</w:t></w:r></w:fldSimple>"
                                "<w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple></w:p>";
        const QVector<Extra> extra = {{QStringLiteral("file:///C:/pictures/p.png"), QString(), QByteArray(), kRel + QStringLiteral("image"), QString(), QStringLiteral("rId9"), true},
                                      {QStringLiteral("https://example.com/"), QString(), QByteArray(), kRel + QStringLiteral("hyperlink"), QString(), QStringLiteral("rId8"), true}};
        WriteWindow w;
        QString err;
        w.setDocument(WriteDoc::fromDocx(makeDocx(body, extra), &err, nullptr), QString());
        WriteDoc &doc = *w.document();
        const QVector<InspectItem> items = w.inspectItems();
        const InspectItem *links = nullptr;
        for (const InspectItem &i : items)
            if (i.title == QLatin1String("External links")) links = &i;
        QVERIFY(links);
        // the linked picture and the INCLUDETEXT field are two; the web link and the PAGE field are not
        QCOMPARE(links->count(), 2);
        QCOMPARE(links->remove(), 2);
        QCOMPARE(links->count(), 0);
        const QString text = doc.story()->toPlainText();
        QVERIFY(text.contains(QLatin1String("from the other file")));   // (the field's result stays as text)
        QVERIFY(text.contains(QLatin1String("web")));
        // saved, the file no longer names the other address, and keeps the web one
        QString saveErr;
        const QByteArray saved = doc.toDocx(&saveErr);
        QVERIFY2(!saved.isEmpty(), qPrintable(saveErr));
        office::Package package;
        QVERIFY(package.read(saved));
        QStringList targets;
        for (const office::Relationship &r : package.relationships(QStringLiteral("/word/document.xml")))
            if (r.external) targets << r.target;
        QVERIFY2(!targets.join(QLatin1Char('|')).contains(QLatin1String("pictures/p.png")), qPrintable(targets.join(QLatin1Char('|'))));
        QVERIFY2(targets.contains(QStringLiteral("https://example.com/")), qPrintable(targets.join(QLatin1Char('|'))));
        QVERIFY(!partOf(saved, QStringLiteral("/word/document.xml")).contains("r:link"));
        QVERIFY(!partOf(saved, QStringLiteral("/word/document.xml")).contains("INCLUDETEXT"));
    }

    void inspectRemovesHeadersAndFooters()
    {
        WriteWindow w;
        WriteDoc &doc = *w.document();
        QVERIFY(doc.createHeaderFooter(0, false, 0));
        QTextDocument *header = doc.editableHeaderFooter(0, false, 0);
        QVERIFY(header);
        QTextCursor c(header);
        c.insertText(QStringLiteral("Confidential"));
        QCOMPARE(headerFooterCount(doc), 1);
        QCOMPARE(clearHeadersAndFooters(doc), 1);
        QCOMPARE(headerFooterCount(doc), 0);
    }

    void compatibilityNamesWhatAnOdtLeavesOut()
    {
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("plain"));
        QVERIFY(w.incompatibilities().isEmpty());
        w.checkCompatibility();
        QVERIFY(quiet.said.last().contains(QLatin1String("Nothing")));
        setNewPasswordStandIn([](bool, QString *password) {
            *password = QStringLiteral("pw");
            return NewPasswordChoice::Set;
        });
        w.encryptWithPassword();
        QVERIFY(!w.incompatibilities().join(QLatin1Char(' ')).contains(QLatin1String("password")));   // (an .odt file holds the password now)
    }

    void theInfoPageHasItsCardsAndEditsProperties()
    {
        WriteWindow w;
        Quiet quiet(w);
        w.show();
        w.showFileScreen(QStringLiteral("info"));
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen);
        QCOMPARE(screen->currentPage(), QStringLiteral("info"));
        QWidget *page = screen->findChild<QWidget *>(QStringLiteral("infoPage"));
        QVERIFY(page);
        for (const char *name : {"infoMarkFinal", "infoEncrypt", "infoInspect", "infoCompatibility", "infoAccessibility", "infoCustom", "infoShowFolder", "infoCopyPath"})
            QVERIFY2(page->findChild<QWidget *>(QString::fromLatin1(name)), name);
        QVERIFY(!page->findChild<QPushButton *>(QStringLiteral("infoCopyPath"))->isEnabled());   // (nothing saved yet)
        {   // (Open File Location and Copy Path are both grey for a file that has no place yet.)
            auto *where = page->findChild<QPushButton *>(QStringLiteral("infoShowFolder"));
            QVERIFY(where);
            QCOMPARE(where->text(), QStringLiteral("Open File Location"));
            QVERIFY(!where->isEnabled());
            QVERIFY(!page->findChild<QPushButton *>(QStringLiteral("infoCopyPath"))->isEnabled());
        }
        auto *title = page->findChild<QLineEdit *>(QStringLiteral("infoTitle"));
        QVERIFY(title);
        title->setText(QStringLiteral("From the page"));
        Q_EMIT title->editingFinished();
        QCOMPARE(w.document()->packageProps().title, QStringLiteral("From the page"));
        QVERIFY(w.isModified());
        w.hideFileScreen();
    }

    // Options > Advanced: a link is followed by Ctrl+click until the person asks for a plain click; the choice is kept and a new window starts with it.
    void theAdvancedPageLetsALinkFollowOnAPlainClick()
    {
        Settings::get().setValue(QStringLiteral("edit/ctrlClickLinks"), true);
        Settings::get().setValue(QStringLiteral("edit/dragText"), true);
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("see link here"));
        QTextCursor sel(w.document()->story());
        sel.setPosition(4);
        sel.setPosition(8, QTextCursor::KeepAnchor);
        QString problem;
        QVERIFY2(insertHyperlink(*w.document(), sel, QStringLiteral("https://example.com"), QString(), QString(), QString(), &problem), qPrintable(problem));
        QStringList opened;
        w.setLinkOpener([&](const QString &a) { opened << a; });
        auto click = [&](Qt::KeyboardModifiers keys) {
            QTextCursor c(w.document()->story());
            c.setPosition(6);
            w.view()->setCursor(c);
            QRect r;
            QVERIFY(w.view()->caretRect(&r));
            QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, keys, r.center());
        };
        click(Qt::NoModifier);
        QVERIFY(opened.isEmpty());
        click(Qt::ControlModifier);
        QCOMPARE(opened, (QStringList{"https://example.com"}));
        opened.clear();
        QVector<OptionsPage> pages = w.optionsPages();
        QList<QWidget *> built;
        for (const OptionsPage &p : pages) built << p.build();
        auto box = [&](const char *name) {
            for (QWidget *page : built)
                if (auto *f = page->findChild<QCheckBox *>(QString::fromLatin1(name))) return f;
            return static_cast<QCheckBox *>(nullptr);
        };
        QCheckBox *ctrl = box("ctrlClickLinks"), *drag = box("dragText");
        QVERIFY(ctrl && drag);
        QVERIFY(ctrl->isChecked() && drag->isChecked());
        ctrl->setChecked(false);
        drag->setChecked(false);
        for (const OptionsPage &p : pages)
            if (p.apply) p.apply();
        qDeleteAll(built);
        QVERIFY(!w.view()->linkNeedsCtrl());
        click(Qt::NoModifier);
        QCOMPARE(opened, (QStringList{"https://example.com"}));
        // (A click with Shift is a selection, not a link.)
        opened.clear();
        click(Qt::ShiftModifier);
        QVERIFY(opened.isEmpty());
        WriteWindow v;
        QVERIFY(!v.view()->linkNeedsCtrl());
        Settings::get().setValue(QStringLiteral("edit/ctrlClickLinks"), true);
        Settings::get().setValue(QStringLiteral("edit/dragText"), true);
    }

    // Options > Advanced: typing replaces the selected text unless asked not to, and the Insert key turns on overtype when it is let to.
    void theAdvancedPageSetsWhetherTypingReplacesTheSelectionAndOvertype()
    {
        Settings::get().setValue(QStringLiteral("edit/typingReplaces"), true);
        Settings::get().setValue(QStringLiteral("edit/insertKey"), false);
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("abcdef"));
        auto select = [&](int from, int to) {
            QTextCursor c(w.document()->story());
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            w.view()->setCursor(c);
        };
        auto text = [&] { return w.document()->story()->toPlainText(); };
        select(1, 3);
        QTest::keyClicks(w.view(), QStringLiteral("X"));
        QCOMPARE(text(), QStringLiteral("aXdef"));
        // Overtype does nothing until the Insert key is let to.
        select(1, 1);
        QTest::keyClick(w.view(), Qt::Key_Insert);
        QVERIFY(!w.view()->overtype());
        auto set = [&](const char *name, bool on) {
            QVector<OptionsPage> pages = w.optionsPages();
            QList<QWidget *> built;
            for (const OptionsPage &p : pages) built << p.build();
            for (QWidget *page : built)
                if (auto *b = page->findChild<QCheckBox *>(QString::fromLatin1(name))) b->setChecked(on);
            for (const OptionsPage &p : pages)
                if (p.apply) p.apply();
            qDeleteAll(built);
        };
        set("typingReplaces", false);
        select(1, 3);
        QTest::keyClicks(w.view(), QStringLiteral("Y"));
        QCOMPARE(text(), QStringLiteral("aXdYef"));
        set("typingReplaces", true);
        set("insertKeyOvertype", true);
        select(0, 0);
        QTest::keyClick(w.view(), Qt::Key_Insert);
        QVERIFY(w.view()->overtype());
        QTest::keyClicks(w.view(), QStringLiteral("12"));
        QCOMPARE(text(), QStringLiteral("12dYef"));
        QTest::keyClick(w.view(), Qt::Key_Insert);
        QVERIFY(!w.view()->overtype());
        QTest::keyClicks(w.view(), QStringLiteral("Z"));
        QCOMPARE(text(), QStringLiteral("12Zd" "Yef"));
        Settings::get().setValue(QStringLiteral("edit/typingReplaces"), true);
        Settings::get().setValue(QStringLiteral("edit/insertKey"), false);
    }

    // The Spike: Ctrl+F3 cuts onto it (adding), Ctrl+Shift+F3 pastes all of it as one step and empties it; bold stays.
    void theSpikeCollectsCutsAndPastesThemTogether()
    {
        PageView::emptySpike();
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("alpha beta gamma"));
        auto select = [&](int from, int to) {
            QTextCursor c(w.document()->story());
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            w.view()->setCursor(c);
        };
        auto text = [&] { return w.document()->story()->toPlainText(); };
        select(6, 10);   // "beta"
        QTextCursor bold = w.view()->cursor();
        QTextCharFormat bf;
        bf.setFontWeight(QFont::Bold);
        bold.mergeCharFormat(bf);
        w.view()->setCursor(bold);
        QTest::keyClick(w.view(), Qt::Key_F3, Qt::ControlModifier);
        QCOMPARE(PageView::spikeCount(), 1);
        QCOMPARE(text(), QStringLiteral("alpha gamma"));   // (smart cut takes a space too)
        select(0, 5);   // "alpha"
        QTest::keyClick(w.view(), Qt::Key_F3, Qt::ControlModifier);
        QCOMPARE(PageView::spikeCount(), 2);
        // Nothing selected: nothing is added.
        select(0, 0);
        QTest::keyClick(w.view(), Qt::Key_F3, Qt::ControlModifier);
        QCOMPARE(PageView::spikeCount(), 2);
        const int before = text().size();
        QTest::keyClick(w.view(), Qt::Key_F3, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(PageView::spikeCount(), 0);
        QCOMPARE(text(), QStringLiteral("beta\nalpha gamma"));   // (each cut is a paragraph of its own)
        QTextCursor at(w.document()->story());
        at.setPosition(1);
        QCOMPARE(at.charFormat().fontWeight(), int(QFont::Bold));
        // One step undoes the whole paste; an empty Spike pastes nothing.
        w.view()->undo();
        QCOMPARE(text().size(), before);
        QTest::keyClick(w.view(), Qt::Key_F3, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(text().size(), before);
        PageView::emptySpike();
    }

    // The Spike holds only what was taken out of the text; and it holds a limited number of bytes.
    void theSpikeTakesOnlyWhatWasCutAndHasALimit()
    {
        PageView::emptySpike();
        const auto restore = qScopeGuard([] { PageView::emptySpike(); PageView::setSpikeByteLimit(64 * 1024 * 1024); });
        // A cut in paragraphs that show nothing (a tracked deletion) takes nothing out, so puts nothing on the Spike.
        {
            QString err;
            WriteWindow w;
            w.resize(1100, 800);
            w.show();
            w.setDocument(WriteDoc::fromDocx(makeDocx("<w:p><w:pPr><w:rPr><w:del w:id=\"1\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"/></w:rPr></w:pPr><w:del w:id=\"2\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"><w:r><w:delText>gone</w:delText></w:r></w:del></w:p>"
                                                      "<w:p><w:r><w:t>Zed</w:t></w:r></w:p>"), &err, nullptr), QString());
            QTextDocument *story = w.document()->story();
            const QString before = story->toPlainText();
            QTextCursor c(story);
            c.setPosition(0);
            c.setPosition(story->findBlockByNumber(1).position(), QTextCursor::KeepAnchor);
            w.view()->setCursor(c);
            QTest::keyClick(w.view(), Qt::Key_F3, Qt::ControlModifier);
            QCOMPARE(story->toPlainText(), before);
            QCOMPARE(PageView::spikeCount(), 0);
        }
        // The limit: a cut that would take the Spike past it is refused, and the text keeps its words.
        {
            PageView::setSpikeByteLimit(15);
            WriteWindow w;
            w.resize(1100, 800);
            w.show();
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("alpha beta"));
            QTextDocument *story = w.document()->story();
            auto select = [&](int from, int to) {
                QTextCursor c(story);
                c.setPosition(from);
                c.setPosition(to, QTextCursor::KeepAnchor);
                w.view()->setCursor(c);
            };
            QStringList said;
            connect(w.view(), &PageView::notice, this, [&](const QString &t) { said << t; });
            select(6, 10);   // "beta": 8 bytes
            QTest::keyClick(w.view(), Qt::Key_F3, Qt::ControlModifier);
            QCOMPARE(PageView::spikeCount(), 1);
            select(0, 5);   // "alpha": 10 more, past 15
            QTest::keyClick(w.view(), Qt::Key_F3, Qt::ControlModifier);
            QCOMPARE(PageView::spikeCount(), 1);
            QVERIFY(story->toPlainText().contains(QStringLiteral("alpha")));
            QCOMPARE(said.size(), 1);
        }
    }

    void printingOnlyOddOrEvenPagesAndInReverse()
    {
        WriteWindow w;
        w.show();
        for (int i = 0; i < 4; ++i) {
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("page text"));
            insertPageBreak(*w.document(), w.view()->cursor());
        }
        WriteOutputOptions o;
        const QVector<int> all = w.outputPages(o);
        QVERIFY(all.size() >= 5);
        o.sides = WriteOutputOptions::Sides::Odd;
        QVector<int> want;
        for (int i : all)
            if ((i + 1) % 2 == 1) want << i;
        QCOMPARE(w.outputPages(o), want);
        o.sides = WriteOutputOptions::Sides::Even;
        want.clear();
        for (int i : all)
            if ((i + 1) % 2 == 0) want << i;
        QCOMPARE(w.outputPages(o), want);
        o.sides = WriteOutputOptions::Sides::All;
        o.reverse = true;
        QVector<int> backwards = all;
        std::reverse(backwards.begin(), backwards.end());
        QCOMPARE(w.outputPages(o), backwards);
        // the two together, within a range: pages 2 to 4, even, last first
        o.range = WriteOutputOptions::Range::Pages;
        o.pages = QStringLiteral("2-4");
        o.sides = WriteOutputOptions::Sides::Even;
        QCOMPARE(w.outputPages(o), (QVector<int>{3, 1}));
    }

    void plainTextKeepsTheWordsInTheEncodingChosen()
    {
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("caf\u00e9 \u20ac5\nsecond"));
        int lost = 0;
        QCOMPARE(plainTextFile(*w.document(), TextEncoding::Utf8, LineEnds::Unix, &lost), QStringLiteral("caf\u00e9 \u20ac5\nsecond").toUtf8());
        QCOMPARE(plainTextFile(*w.document(), TextEncoding::Utf8WithMark, LineEnds::Windows).left(5), QByteArray("\xEF\xBB\xBF" "ca", 5));
        QVERIFY(plainTextFile(*w.document(), TextEncoding::Utf8, LineEnds::Windows).contains("\r\nsecond"));
        const QByteArray utf16 = plainTextFile(*w.document(), TextEncoding::Utf16, LineEnds::Unix);
        QCOMPARE(utf16.left(2), QByteArray("\xFF\xFE", 2));
        QCOMPARE(utf16.size(), 2 + 2 * 14);
        const QByteArray latin = plainTextFile(*w.document(), TextEncoding::Latin1, LineEnds::Unix, &lost);
        QCOMPARE(lost, 1);   // (the euro sign is not in ISO 8859-1)
        QCOMPARE(latin, QByteArray("caf\xE9" " ?5\nsecond"));
        QCOMPARE(plainTextFile(*w.document(), TextEncoding::Ascii, LineEnds::Unix, &lost), QByteArray("caf? ?5\nsecond"));
        QCOMPARE(lost, 2);
        // hidden text stays out
        QTextCursor c(w.document()->story());
        c.setPosition(0);
        c.setPosition(4, QTextCursor::KeepAnchor);
        QTextCharFormat hidden;
        hidden.setProperty(tp::HiddenText, true);
        c.mergeCharFormat(hidden);
        QVERIFY(!visibleText(*w.document()).contains(QLatin1String("caf")));
        QTemporaryDir dir;
        QVERIFY(w.exportText(dir.filePath(QStringLiteral("out.txt")), TextEncoding::Utf8, LineEnds::Unix));
        QVERIFY(fileBytes(dir.filePath(QStringLiteral("out.txt"))).endsWith("second"));
    }

    // Save as Web Page writes the document's words as one page with its looks, Save as Filtered Web Page the structure alone; a path of nothing writes nothing.
    void webPagesAreWrittenFromTheDocumentInBothForms()
    {
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("caf\u00e9 <b> & more\nsecond"));
        QTextCursor c(w.document()->story());
        c.setPosition(0);
        c.setPosition(4, QTextCursor::KeepAnchor);
        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);
        c.mergeCharFormat(bold);
        QTemporaryDir dir;
        QVERIFY(!w.exportWeb(QString(), true));
        QVERIFY(w.exportWeb(dir.filePath(QStringLiteral("page.html")), true));
        QVERIFY(w.exportWeb(dir.filePath(QStringLiteral("small.html")), false));
        const QString page = QString::fromUtf8(fileBytes(dir.filePath(QStringLiteral("page.html"))));
        const QString small = QString::fromUtf8(fileBytes(dir.filePath(QStringLiteral("small.html"))));
        QVERIFY2(page.startsWith(QLatin1String("<!DOCTYPE html>")), qPrintable(page));
        QVERIFY2(page.contains(QStringLiteral("<b>caf\u00e9</b> &lt;b&gt; &amp; more")), qPrintable(page));
        QVERIFY(page.contains(QLatin1String(">second</p>")));
        QVERIFY(small.contains(QLatin1String("<p>second</p>")));
        QVERIFY(page.contains(QLatin1String("<style>")));
        QVERIFY(small.contains(QStringLiteral("<b>caf\u00e9</b> &lt;b&gt; &amp; more")));
        QVERIFY(!small.contains(QLatin1String("<style>")));
    }

    // Save as E-book: a chapter for each Heading 1 (the front words are a chapter of their own), the contents from Heading 1 to 3, the notes at each chapter's end with a link back, pictures
    // as files of the book, a table in place, and the cover only when asked for.
    void anEbookHasAChapterForEachHeading1()
    {
        WriteWindow w;
        Quiet quiet(w);
        buildBook(w);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("book.epub"));
        QVERIFY(!w.exportEpub(QString(), QString(), QString(), false));
        QVERIFY(w.exportEpub(path, QStringLiteral("My Book"), QStringLiteral("Pat Writer"), false));
        const QByteArray bytes = fileBytes(path);
        QCOMPARE(bytes.mid(30, 28), QByteArray("mimetypeapplication/epub+zip"));   // first, and stored
        QCOMPARE(int(bytes[8]), 0);
        QMap<QString, QByteArray> z;
        QString err;
        QVERIFY2(readZip(bytes, z, &err), qPrintable(err));
        for (auto it = z.cbegin(); it != z.cend(); ++it) {
            const QString &name = it.key();
            if (!name.endsWith(QLatin1String(".xhtml")) && !name.endsWith(QLatin1String(".opf")) && !name.endsWith(QLatin1String(".ncx")) && !name.endsWith(QLatin1String(".xml"))) continue;
            QVERIFY2(wellFormed(it.value(), &err), qPrintable(name + ": " + err));
        }
        const QString opf = QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")));
        QVERIFY(opf.contains(QStringLiteral("<dc:title>My Book</dc:title>")) && opf.contains(QStringLiteral("<dc:creator>Pat Writer</dc:creator>")));
        QVERIFY(opf.contains(QStringLiteral("<spine toc=\"ncx\">")) && z.contains(QStringLiteral("OEBPS/toc.ncx")));
        QVERIFY(!z.contains(QStringLiteral("OEBPS/chapter4.xhtml")));
        QVERIFY(z.contains(QStringLiteral("OEBPS/chapter3.xhtml")));
        QVERIFY(!opf.contains(QStringLiteral("cover-image")));   // no cover unless asked for
        const QString c1 = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml"))), c2 = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter2.xhtml"))),
                      c3 = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter3.xhtml")));
        QVERIFY(c1.contains(QStringLiteral("Front words.")) && !c1.contains(QStringLiteral("Chapter One")));
        QVERIFY(c2.contains(QStringLiteral("<h1 id=\"h1\">Chapter One</h1>")) && c2.contains(QStringLiteral("A section")));
        QVERIFY(c3.contains(QStringLiteral("Chapter Two")));
        // the footnote: its number links to the note at the end of the chapter, which links back
        QVERIFY2(c2.contains(QStringLiteral("href=\"#fn-1\"")) && c2.contains(QStringLiteral("id=\"fnr-1\"")), qPrintable(c2));
        QVERIFY2(c2.contains(QStringLiteral("id=\"fn-1\"")) && c2.contains(QStringLiteral("The footnote words.")) && c2.contains(QStringLiteral("href=\"#fnr-1\"")), qPrintable(c2));
        // the contents: Heading 1 to 3 (not the Heading 4), each at its chapter and place
        const QString nav = QString::fromUtf8(z.value(QStringLiteral("OEBPS/nav.xhtml")));
        QVERIFY2(nav.contains(QStringLiteral("epub:type=\"toc\"")) && nav.contains(QStringLiteral("<a href=\"chapter2.xhtml\">Chapter One</a>")), qPrintable(nav));
        QVERIFY(nav.contains(QStringLiteral("<a href=\"chapter2.xhtml#h2\">A section</a>")) && nav.contains(QStringLiteral("<a href=\"chapter3.xhtml\">Chapter Two</a>")));
        QVERIFY(!nav.contains(QStringLiteral("Too deep")));
        QVERIFY(c2.contains(QStringLiteral("Too deep")));
        // the picture is a file of the book, used once however many times it is placed; the table is in place
        QCOMPARE(c3.count(QStringLiteral("src=\"images/pic1.png\"")), 2);
        QVERIFY(c3.contains(QStringLiteral("alt=\"A green box\"")) && !c3.contains(QStringLiteral("data:")));
        QVERIFY(z.contains(QStringLiteral("OEBPS/images/pic1.png")) && !z.contains(QStringLiteral("OEBPS/images/pic2.png")));
        QVERIFY(opf.contains(QStringLiteral("href=\"images/pic1.png\" media-type=\"image/png\"")));
        QVERIFY(c3.contains(QStringLiteral("<table>")) && c3.contains(QStringLiteral("</table>")));
        // asked for, the first picture is the cover
        const QString coverPath = dir.filePath(QStringLiteral("cover.epub"));
        QVERIFY(w.exportEpub(coverPath, QString(), QString(), true));
        QMap<QString, QByteArray> cz;
        QVERIFY(readZip(fileBytes(coverPath), cz, &err));
        QVERIFY(QString::fromUtf8(cz.value(QStringLiteral("OEBPS/content.opf"))).contains(QStringLiteral("properties=\"cover-image\"")));
        QVERIFY(!QImage::fromData(cz.value(QStringLiteral("OEBPS/images/cover.jpg"))).isNull());
        QVERIFY(QString::fromUtf8(cz.value(QStringLiteral("OEBPS/cover.xhtml"))).contains(QStringLiteral("images/cover.jpg")));
    }

    // The Save as E-book box starts with the title and the author of the document's properties; what it says goes in the book, and an emptied Author box is a book with no author.
    void anEbookTakesItsTitleAndAuthorFromTheBoxThatStartsWithTheProperties()
    {
        WriteWindow w;
        Quiet quiet(w);
        buildBook(w);
        office::PackageProps p = w.document()->packageProps();
        p.title = QStringLiteral("From Properties");
        p.creator = QStringLiteral("Sam Author");
        w.applyProperties(p);
        QTemporaryDir dir;
        int run = 0;
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("%1.epub").arg(run)); });
        auto bookOf = [&](const std::function<void(QDialog *)> &act) {
            ++run;
            onNextEpubDialog(act);
            return w.exportEpubAsk() ? unzipped(fileBytes(dir.filePath(QStringLiteral("%1.epub").arg(run)))) : QMap<QString, QByteArray>();
        };
        QString titleShown, authorShown;
        QMap<QString, QByteArray> z = bookOf([&](QDialog *d) {
            titleShown = d->findChild<QLineEdit *>(QStringLiteral("epubTitle"))->text();
            authorShown = d->findChild<QLineEdit *>(QStringLiteral("epubAuthor"))->text();
        });
        QCOMPARE(titleShown, QStringLiteral("From Properties"));
        QCOMPARE(authorShown, QStringLiteral("Sam Author"));
        QString opf = QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")));
        QVERIFY2(opf.contains(QStringLiteral("<dc:title>From Properties</dc:title>")) && opf.contains(QStringLiteral("<dc:creator>Sam Author</dc:creator>")), qPrintable(opf));
        QVERIFY(opf.contains(QRegularExpression(QStringLiteral("<dc:language>[a-z]{2,3}(-[A-Za-z0-9]+)*</dc:language>"))));
        // emptied, there is no author (the properties' creator is not put back)
        z = bookOf([](QDialog *d) { d->findChild<QLineEdit *>(QStringLiteral("epubAuthor"))->clear(); });
        opf = QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")));
        QVERIFY2(opf.contains(QStringLiteral("<dc:title>From Properties</dc:title>")) && !opf.contains(QStringLiteral("dc:creator")), qPrintable(opf));
        // a title and an author typed are the book's
        z = bookOf([](QDialog *d) {
            d->findChild<QLineEdit *>(QStringLiteral("epubTitle"))->setText(QStringLiteral("Typed & Title"));
            d->findChild<QLineEdit *>(QStringLiteral("epubAuthor"))->setText(QStringLiteral("Lee"));
        });
        opf = QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")));
        QVERIFY(opf.contains(QStringLiteral("<dc:title>Typed &amp; Title</dc:title>")) && opf.contains(QStringLiteral("<dc:creator>Lee</dc:creator>")));
    }

    // The book's language is a valid BCP 47 tag: underscores become hyphens, and anything that is not a tag is US English.
    void anEbookNamesItsLanguageByAValidTag()
    {
        const QByteArray docx = makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>");
        const QVector<QPair<QString, QString>> cases = {{QStringLiteral("en_US"), QStringLiteral("en-US")}, {QStringLiteral("x-none"), QStringLiteral("en-US")}, {QStringLiteral("de-AT"), QStringLiteral("de-AT")},
                                                        {QStringLiteral("junk!!"), QStringLiteral("en-US")}, {QStringLiteral("fr"), QStringLiteral("fr")}, {QStringLiteral("zh_Hant_TW"), QStringLiteral("zh-Hant-TW")},
                                                        {QStringLiteral("a<b"), QStringLiteral("en-US")}, {QStringLiteral("C"), QStringLiteral("en-US")}};
        for (const auto &c : cases) {
            EpubExportOptions o;
            o.language = c.first;
            const QMap<QString, QByteArray> z = bookOfDocx(docx, o);
            const QString opf = QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")));
            QVERIFY2(opf.contains(QStringLiteral("<dc:language>%1</dc:language>").arg(c.second)), qPrintable(c.first + QStringLiteral(": ") + opf));
            QVERIFY2(opf.contains(QStringLiteral("xml:lang=\"%1\"").arg(c.second)), qPrintable(opf));
        }
    }

    // The same saved document is the same book each time (a reader keeps its place), a document saved elsewhere another book, and one never saved a new book each time.
    void anEbookKeepsItsIdentifierForTheSameDocument()
    {
        QTemporaryDir dir;
        auto identifierOf = [&](WriteWindow &w) {
            const QString path = dir.filePath(QStringLiteral("out.epub"));
            QFile::remove(path);
            if (!w.exportEpub(path, QStringLiteral("T"), QString(), false)) return QString();
            const QString opf = QString::fromUtf8(unzipped(fileBytes(path)).value(QStringLiteral("OEBPS/content.opf")));
            return opf.section(QStringLiteral("<dc:identifier id=\"bookid\">"), 1).section(QLatin1Char('<'), 0, 0);
        };
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("words"));
        const QString first = identifierOf(w), second = identifierOf(w);
        QVERIFY(first.startsWith(QLatin1String("urn:uuid:")));
        QVERIFY(first != second);   // never saved: a new book each time
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("doc.docx")); });
        QVERIFY(w.save());
        const QString saved = identifierOf(w);
        QVERIFY(saved.startsWith(QLatin1String("urn:uuid:")));
        QCOMPARE(identifierOf(w), saved);
        WriteWindow other;
        Quiet quietOther(other);
        typeText(*other.document(), other.view()->cursor(), QStringLiteral("words"));
        other.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("other.docx")); });
        QVERIFY(other.save());
        QVERIFY(identifierOf(other) != saved);
        QVERIFY(QUuid(saved.mid(9)).version() == QUuid::Sha1);
    }

    // The characters XML forbids (a lone surrogate, U+FFFE) are left out of every page of the book, so each page parses.
    void anEbookLeavesOutTheCharactersXmlForbids()
    {
        WriteWindow w;
        Quiet quiet(w);
        WriteDoc &doc = *w.document();
        QTextCursor c = w.view()->cursor();
        paragraphOf(doc, c, QStringLiteral("Chapter") + QString(QChar(0xFFFE)) + QStringLiteral(" One") + QString(QChar(0xD800)), QStringLiteral("Heading1"), true);
        paragraphOf(doc, c, QStringLiteral("bad") + QString(QChar(0xFFFE)) + QStringLiteral("char") + QString(QChar(0xFFFF)) + QStringLiteral("s ") + QString(QChar(0xDC00)) + QStringLiteral("end"), QStringLiteral("Normal"));
        QTemporaryDir dir;
        QVERIFY(w.exportEpub(dir.filePath(QStringLiteral("a.epub")), QStringLiteral("T") + QString(QChar(0xFFFE)), QStringLiteral("A") + QString(QChar(0xD800)), false));
        const QMap<QString, QByteArray> z = unzipped(fileBytes(dir.filePath(QStringLiteral("a.epub"))));
        QCOMPARE(problemsOf(z), QString());
        QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml"))).contains(QStringLiteral("badchars end")));
        QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/nav.xhtml"))).contains(QStringLiteral(">Chapter One</a>")));
    }

    // A table of contents of links to bookmarks, a link to a bookmark that is not there, a footnote inside a link, and a footnote in a heading: every link in the book goes where it says, a link
    // with no place to go is its words alone, a link is never inside a link, and the contents' labels have no note numbers.
    void anEbookLinksToBookmarksAndNeverNestsALink()
    {
        const QByteArray body =
            "<w:p><w:r><w:t>Front</w:t></w:r></w:p>"
            "<w:p><w:hyperlink w:anchor=\"_Toc1\"><w:r><w:t>to one</w:t></w:r></w:hyperlink><w:r><w:t xml:space=\"preserve\"> </w:t></w:r>"
            "<w:hyperlink w:anchor=\"_Toc2\"><w:r><w:t>to two</w:t></w:r></w:hyperlink><w:r><w:t xml:space=\"preserve\"> </w:t></w:r>"
            "<w:hyperlink w:anchor=\"_Missing\"><w:r><w:t>nowhere</w:t></w:r></w:hyperlink></w:p>"
            "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:bookmarkStart w:id=\"1\" w:name=\"_Toc1\"/><w:r><w:t>Chapter One</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r><w:bookmarkEnd w:id=\"1\"/></w:p>"
            "<w:p><w:hyperlink w:anchor=\"_Toc2\"><w:r><w:t>see two</w:t></w:r><w:r><w:footnoteReference w:id=\"3\"/></w:r></w:hyperlink></w:p>"
            "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:bookmarkStart w:id=\"2\" w:name=\"_Toc2\"/><w:r><w:t>Chapter Two</w:t></w:r><w:bookmarkEnd w:id=\"2\"/></w:p>";
        const QMap<QString, QByteArray> z = bookOfDocx(makeDocx(body, notesExtra()));
        QVERIFY(z.contains(QStringLiteral("OEBPS/chapter3.xhtml")) && !z.contains(QStringLiteral("OEBPS/chapter4.xhtml")));
        QCOMPARE(problemsOf(z), QString());
        const QString c1 = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml"))), c2 = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter2.xhtml")));
        QVERIFY2(c1.contains(QStringLiteral("<a href=\"chapter2.xhtml#bm__Toc1\">to one</a>")) && c1.contains(QStringLiteral("<a href=\"chapter3.xhtml#bm__Toc2\">to two</a>")), qPrintable(c1));
        QVERIFY2(c1.contains(QStringLiteral("nowhere")) && !c1.contains(QStringLiteral("_Missing")), qPrintable(c1));
        static const QRegularExpression nested(QStringLiteral("<a[ >](?:(?!</a>).)*<a[ >]"), QRegularExpression::DotMatchesEverythingOption);
        for (auto it = z.cbegin(); it != z.cend(); ++it)
            if (it.key().endsWith(QLatin1String(".xhtml"))) QVERIFY2(!nested.match(QString::fromUtf8(it.value())).hasMatch(), qPrintable(it.key()));
        // the note in the link is where its note links back to; the note in the heading is a link of its own
        QVERIFY2(c2.contains(QStringLiteral("<sup id=\"fnr-2\">")) && c2.contains(QStringLiteral("href=\"#fnr-2\"")), qPrintable(c2));
        QVERIFY(c2.contains(QStringLiteral("id=\"fnr-1\"")) && c2.contains(QStringLiteral("href=\"#fnr-1\"")));
        const QString nav = QString::fromUtf8(z.value(QStringLiteral("OEBPS/nav.xhtml")));
        QVERIFY2(nav.contains(QStringLiteral(">Chapter One</a>")) && nav.contains(QStringLiteral(">Chapter Two</a>")), qPrintable(nav));
        QVERIFY2(QString::fromUtf8(z.value(QStringLiteral("OEBPS/toc.ncx"))).contains(QStringLiteral("<text>Chapter One</text>")), "the NCX labels have no note numbers either");
    }

    // The contents are one list when the headings begin with a Heading 2.
    void anEbooksContentsAreOneListWhateverTheFirstHeadingIs()
    {
        const QByteArray body = "<w:p><w:pPr><w:outlineLvl w:val=\"1\"/></w:pPr><w:r><w:t>Small first</w:t></w:r></w:p>"
                                "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>Big</w:t></w:r></w:p>"
                                "<w:p><w:pPr><w:outlineLvl w:val=\"1\"/></w:pPr><w:r><w:t>Small</w:t></w:r></w:p>";
        const QMap<QString, QByteArray> z = bookOfDocx(makeDocx(body));
        QXmlStreamReader r(z.value(QStringLiteral("OEBPS/nav.xhtml")));
        int depth = 0, topLists = 0, links = 0;
        bool inToc = false;
        while (!r.atEnd()) {
            r.readNext();
            if (r.isStartElement()) {
                ++depth;
                if (r.name() == QLatin1String("nav")) inToc = r.attributes().value(QStringLiteral("http://www.idpf.org/2007/ops"), QStringLiteral("type")) == QLatin1String("toc");
                if (inToc && r.name() == QLatin1String("ol") && depth == 4) ++topLists;
                if (inToc && r.name() == QLatin1String("a")) ++links;
            } else if (r.isEndElement()) {
                --depth;
            }
        }
        QCOMPARE(topLists, 1);
        QCOMPARE(links, 3);
    }

    // A picture a reader can't show is drawn into a PNG (a metafile with our own drawing, a bitmap with the image readers), or is its alternative text; the kind comes from the bytes, not the name.
    void anEbookShowsOnlyPicturesAReaderCanShow()
    {
        auto bookWith = [](const QByteArray &bytes, const QString &name, const QString &type) {
            const QVector<Extra> extra = {{QStringLiteral("/word/media/") + name, type, bytes, kRel + QStringLiteral("image"), QString(), QStringLiteral("rIdP")}};
            return bookOfDocx(makeDocx("<w:p>" + drawingRun(QStringLiteral("rIdP"), QStringLiteral("what it shows")) + "</w:p>", extra));
        };
        auto expectPng = [&](const QByteArray &bytes, const QString &name, const QString &type) {
            const QMap<QString, QByteArray> z = bookWith(bytes, name, type);
            const QString opf = QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")));
            QVERIFY2(opf.contains(QStringLiteral("href=\"images/pic1.png\" media-type=\"image/png\"")), qPrintable(name + QStringLiteral(": ") + opf));
            QVERIFY2(!QImage::fromData(z.value(QStringLiteral("OEBPS/images/pic1.png"))).isNull(), qPrintable(name));
            QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml"))).contains(QStringLiteral("<img src=\"images/pic1.png\" alt=\"what it shows\"/>")));
            QCOMPARE(problemsOf(z), QString());
        };
        QImage small(30, 20, QImage::Format_RGB32);
        small.fill(Qt::red);
        auto encoded = [&](const char *format) {
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            small.save(&buffer, format);
            return bytes;
        };
        expectPng(emfBytes(), QStringLiteral("image1.emf"), QStringLiteral("image/x-emf"));
        expectPng(wmfBytes(), QStringLiteral("image1.wmf"), QStringLiteral("image/x-wmf"));
        expectPng(encoded("BMP"), QStringLiteral("image1.bmp"), QStringLiteral("image/bmp"));
        if (QImageWriter::supportedImageFormats().contains("tiff")) expectPng(encoded("TIFF"), QStringLiteral("image1.tiff"), QStringLiteral("image/tiff"));
        // the bytes decide, not the name: a PNG named like a metafile, and a JPEG named like a PNG, are shown as what they are
        QMap<QString, QByteArray> z = bookWith(encoded("PNG"), QStringLiteral("image1.emf"), QStringLiteral("image/x-emf"));
        QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf"))).contains(QStringLiteral("href=\"images/pic1.png\" media-type=\"image/png\"")));
        QCOMPARE(z.value(QStringLiteral("OEBPS/images/pic1.png")), encoded("PNG"));
        z = bookWith(encoded("JPG"), QStringLiteral("image1.png"), QStringLiteral("image/png"));
        QVERIFY2(QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf"))).contains(QStringLiteral("href=\"images/pic1.jpg\" media-type=\"image/jpeg\"")), qPrintable(QString::fromUtf8(z.value(QStringLiteral("OEBPS/content.opf")))));
        // bytes that are no picture: no picture file, the alternative text where it was
        z = bookWith(QByteArray("this is not a picture of any kind"), QStringLiteral("image1.png"), QStringLiteral("image/png"));
        QVERIFY(!z.contains(QStringLiteral("OEBPS/images/pic1.png")));
        const QString chapter = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml")));
        QVERIFY2(chapter.contains(QStringLiteral("what it shows")) && !chapter.contains(QStringLiteral("<img")), qPrintable(chapter));
        QCOMPARE(problemsOf(z), QString());
    }

    // A picture used again and again is one file of the book, counted once: it is in every place, however many places and however large it is.
    void aPictureUsedManyTimesIsOneFileAndNeverGoesMissing()
    {
        const QByteArray png = noisePng(1200);   // (a few megabytes: the pictures of the page writer's own total would be past its limit in a few dozen uses)
        const QVector<Extra> extra = {{QStringLiteral("/word/media/logo.png"), QStringLiteral("image/png"), png, kRel + QStringLiteral("image"), QString(), QStringLiteral("rIdP")}};
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx("<w:p>" + drawingRun(QStringLiteral("rIdP"), QStringLiteral("logo")) + "</w:p>", extra), &err, nullptr);
        office::wml::Document snapshot;
        QVERIFY(doc && doc->docxSnapshot(&snapshot));
        const office::wml::Block withPicture = snapshot.body.first();
        const int uses = 70;
        for (int i = 1; i < uses; ++i) snapshot.body.push_back(withPicture);
        const QMap<QString, QByteArray> z = unzipped(epubFromDocument(snapshot, EpubExportOptions(), &err));
        QCOMPARE(QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml"))).count(QStringLiteral("<img src=\"images/pic1.png\"")), uses);
        QVERIFY(!z.contains(QStringLiteral("OEBPS/images/pic2.png")));
        QCOMPARE(z.value(QStringLiteral("OEBPS/images/pic1.png")), png);
        // counted once: a bound just above its size holds, and one just below it leaves every use as its words
        EpubExportOptions tight;
        tight.mostPicturesBytes = png.size() + 10;
        QCOMPARE(QString::fromUtf8(unzipped(epubFromDocument(snapshot, tight, &err)).value(QStringLiteral("OEBPS/chapter1.xhtml"))).count(QStringLiteral("<img src=")), uses);
        tight.mostPicturesBytes = png.size() - 10;
        const QMap<QString, QByteArray> none = unzipped(epubFromDocument(snapshot, tight, &err));
        QVERIFY(!none.contains(QStringLiteral("OEBPS/images/pic1.png")));
        QVERIFY(QString::fromUtf8(none.value(QStringLiteral("OEBPS/chapter1.xhtml"))).count(QStringLiteral("logo")) >= uses);
    }

    // One large note referenced many times is written once, whatever the references: the book stays about the size of the note, and a note past the bound on notes written with their looks is
    // still all there, as plain words.
    void aLargeNoteReferencedManyTimesIsWrittenOnce()
    {
        const QString words = QStringLiteral("alpha beta gamma delta ").repeated(9000) + QStringLiteral("zzmarker");   // about 200 KB
        const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">Text</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
        QString err;
        QByteArray notes = footnotesPart(words);
        notes.replace("<w:r><w:t xml:space=\"preserve\"> " + words.toUtf8(), "<w:r><w:rPr><w:b/></w:rPr><w:t xml:space=\"preserve\"> " + words.toUtf8());
        auto doc = WriteDoc::fromDocx(makeDocx(body, {{QStringLiteral("/word/footnotes.xml"), kWord + QStringLiteral("footnotes+xml"), notes, kRel + QStringLiteral("footnotes"), QString(), QString()}}), &err, nullptr);
        office::wml::Document snapshot;
        QVERIFY(doc && doc->docxSnapshot(&snapshot));
        const office::wml::Block referencing = snapshot.body.first();
        const int references = 400;
        for (int i = 1; i < references; ++i) snapshot.body.push_back(referencing);
        auto sizeOfChapters = [](const QMap<QString, QByteArray> &z) {
            qint64 n = 0;
            for (auto it = z.cbegin(); it != z.cend(); ++it)
                if (it.key().endsWith(QLatin1String(".xhtml"))) n += it.value().size();
            return n;
        };
        const QMap<QString, QByteArray> z = unzipped(epubFromDocument(snapshot, EpubExportOptions(), &err));
        const QString chapter = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml")));
        QCOMPARE(chapter.count(QStringLiteral("zzmarker")), 1);
        QCOMPARE(chapter.count(QStringLiteral("<aside ")), references);
        QVERIFY2(sizeOfChapters(z) < 700 * 1024, qPrintable(QString::number(sizeOfChapters(z))));   // (400 copies of the note would be 80 MB)
        QVERIFY(chapter.contains(QStringLiteral("<b>")));
        QCOMPARE(problemsOf(z), QString());
        EpubExportOptions plain;
        plain.mostNoteBytes = 1000;
        const QString bare = QString::fromUtf8(unzipped(epubFromDocument(snapshot, plain, &err)).value(QStringLiteral("OEBPS/chapter1.xhtml")));
        QCOMPARE(bare.count(QStringLiteral("zzmarker")), 1);   // (still all there)
        QVERIFY(!bare.contains(QStringLiteral("<b>")));        // (without its looks)
        QCOMPARE(bare.count(QStringLiteral("<aside ")), references);
    }

    // The book is made a chapter at a time, and Cancel stops it.
    void anEbookStopsWhenCancelled()
    {
        const QByteArray body = "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>One</w:t></w:r></w:p><w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>Two</w:t></w:r></w:p>"
                                "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>Three</w:t></w:r></w:p>";
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx(body), &err, nullptr);
        office::wml::Document snapshot;
        QVERIFY(doc && doc->docxSnapshot(&snapshot));
        QList<QPair<int, int>> calls;
        EpubExportOptions o;
        o.progress = [&](int done, int total) {
            calls << qMakePair(done, total);
            return done < 2;
        };
        QVERIFY(epubFromDocument(snapshot, o, &err).isEmpty());
        QCOMPARE(err, QStringLiteral("Cancelled."));
        QCOMPARE(calls, (QList<QPair<int, int>>{{1, 3}, {2, 3}}));
        calls.clear();
        o.progress = [&](int done, int total) {
            calls << qMakePair(done, total);
            return true;
        };
        QVERIFY(!epubFromDocument(snapshot, o, &err).isEmpty());
        QCOMPARE(calls.size(), 3);
    }

    // A note past the bound keeps all its words as plain text, those a kept element shows in its place too.
    void aPlainNoteKeepsTheWordsOfKeptElements()
    {
        office::wml::Block text, kept;
        office::wml::Inline run;
        run.kind = office::wml::Inline::Text;
        run.text = QStringLiteral("Words & more");
        text.para.content << run;
        kept.isRaw = true;
        kept.tag = QStringLiteral("tbl");
        kept.display = QStringLiteral("[Table: 9 rows by 9 columns]");
        office::wml::Block quiet;
        quiet.isRaw = true;
        QCOMPARE(epubPlainNote({text, kept, quiet}), QStringLiteral("<p>Words &amp; more</p>\n<p>[Table: 9 rows by 9 columns]</p>\n"));
    }

    // A bookmark named in the body and in a note of the chapter is one id in the chapter, and a link to it goes to the one that is there.
    void aBookmarkNamedInTheBodyAndInANoteIsOneIdInTheChapter()
    {
        QByteArray notes = footnotesPart(QStringLiteral("x"));
        notes.replace("<w:r><w:footnoteRef/></w:r>", "<w:bookmarkStart w:id=\"9\" w:name=\"dup\"/><w:r><w:footnoteRef/></w:r><w:bookmarkEnd w:id=\"9\"/>");
        const QByteArray body = "<w:p><w:bookmarkStart w:id=\"1\" w:name=\"dup\"/><w:r><w:t>Here</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r><w:bookmarkEnd w:id=\"1\"/></w:p>"
                                "<w:p><w:hyperlink w:anchor=\"dup\"><w:r><w:t>back to dup</w:t></w:r></w:hyperlink></w:p>";
        const QMap<QString, QByteArray> z = bookOfDocx(makeDocx(body, {{QStringLiteral("/word/footnotes.xml"), kWord + QStringLiteral("footnotes+xml"), notes, kRel + QStringLiteral("footnotes"), QString(), QString()}}));
        const QString chapter = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml")));
        QCOMPARE(problemsOf(z), QString());
        QCOMPARE(chapter.count(QStringLiteral("id=\"bm_dup\"")), 1);
        QVERIFY2(chapter.contains(QStringLiteral("<a href=\"chapter1.xhtml#bm_dup\">back to dup</a>")), qPrintable(chapter));
    }

    // Cancel in the box that opens while the book is made stops it: no warning and no file.
    void cancellingTheEbookExportShowsNoWarningAndLeavesNoFile()
    {
        WriteWindow w;
        Quiet quiet(w);
        buildBook(w);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("gone.epub"));
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        QStringList warnings;
        auto *timer = new QTimer;
        timer->setInterval(0);   // (0, not 1: a 1 ms timer may not be due yet at the export's few processEvents calls, so a fast machine finished the book first: macOS CI, Nov 17)
        QObject::connect(timer, &QTimer::timeout, [&w, timer, &warnings] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                warnings << box->text();
                box->accept();
            }
            if (auto *progress = w.findChild<QProgressDialog *>(QStringLiteral("epubProgress"))) {
                progress->cancel();
                timer->stop();
                timer->deleteLater();
            }
        });
        timer->start();
        onNextEpubDialog([](QDialog *) {});
        QVERIFY(!w.exportEpubAsk());
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join(QLatin1Char('|'))));
        QVERIFY(quiet.said.isEmpty());
        QVERIFY(!QFileInfo::exists(path));
    }

    // A hostile document is held to the bounds: the chapters, the contents' lines, and the pictures (a count and a size) are cut off, and nothing is lost from the text.
    void anEbookIsHeldToItsBounds()
    {
        WriteWindow w;
        Quiet quiet(w);
        buildBook(w);
        office::wml::Document snapshot;
        QVERIFY(w.document()->docxSnapshot(&snapshot));
        auto read = [&](const EpubExportOptions &o) {
            QMap<QString, QByteArray> z;
            QString err;
            const QByteArray bytes = epubFromDocument(snapshot, o, &err);
            readZip(bytes, z, &err);
            return z;
        };
        EpubExportOptions few;
        few.mostChapters = 2;
        QMap<QString, QByteArray> z = read(few);
        QVERIFY(z.contains(QStringLiteral("OEBPS/chapter2.xhtml")) && !z.contains(QStringLiteral("OEBPS/chapter3.xhtml")));
        // (the chapter past the last is not lost: it is in the last one)
        QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter2.xhtml"))).contains(QStringLiteral("Chapter Two")));
        EpubExportOptions lines;
        lines.mostNavEntries = 2;
        z = read(lines);
        const QString nav = QString::fromUtf8(z.value(QStringLiteral("OEBPS/nav.xhtml")));
        QCOMPARE(nav.count(QStringLiteral("<li><a href=\"chapter")), 2);
        EpubExportOptions noPictures;
        noPictures.mostPictures = 0;
        z = read(noPictures);
        QVERIFY(!z.contains(QStringLiteral("OEBPS/images/pic1.png")));
        EpubExportOptions small;
        small.mostPictureBytes = 10;
        z = read(small);
        QVERIFY(!z.contains(QStringLiteral("OEBPS/images/pic1.png")));
        QVERIFY(QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter3.xhtml"))).contains(QStringLiteral("<table>")));
        QVERIFY(read(EpubExportOptions()).contains(QStringLiteral("OEBPS/images/pic1.png")));   // (the bounds are what stopped it)
        // the real bounds are the documented ones
        QCOMPARE(EpubExportOptions().mostChapters, 500);
        QCOMPARE(EpubExportOptions().mostNavEntries, 4000);
        QCOMPARE(EpubExportOptions().mostPictures, 1000);
        QCOMPARE(kMostEpubHeadingLevel, 3);
    }

    // File > Export has Save as E-book.
    void theExportPageOffersSaveAsEbook()
    {
        WriteWindow w;
        w.show();
        w.showFileScreen(QStringLiteral("export"));
        QVERIFY(w.findChild<QAbstractButton *>(QStringLiteral("exportEpub")));
    }

    // Pictures for a person to look at (JW_SHOT_DIR names the folder; nothing is made without it): the export page, the dialog, and chapters as a reader's page.
    void ebookScreenshots()
    {
        const QString dir = qEnvironmentVariable("JW_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JW_SHOT_DIR is not set");
        applyUiTheme(1);   // the program's own look, as a person sees it; a bare test has none
        WriteWindow w;
        Quiet quiet(w);
        w.resize(1100, 800);
        buildBook(w);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        w.showFileScreen(QStringLiteral("export"));
        QTRY_VERIFY(w.findChild<QAbstractButton *>(QStringLiteral("exportEpub")) && w.findChild<QAbstractButton *>(QStringLiteral("exportEpub"))->isVisible());
        w.grab().save(dir + QStringLiteral("/epub-export-page.png"));
        w.showFileScreen(QString());
        auto *timer = new QTimer;
        timer->setInterval(20);
        connect(timer, &QTimer::timeout, [timer, dir] {
            QWidget *modal = QApplication::activeModalWidget();
            if (!modal) return;
            timer->stop();
            timer->deleteLater();
            modal->grab().save(dir + QStringLiteral("/epub-export-dialog.png"));
            static_cast<QDialog *>(modal)->reject();
        });
        timer->start();
        w.setSavePathChooser([](const QString &, const QString &) { return QString(); });
        QVERIFY(!w.exportEpubAsk());
        QTemporaryDir out;
        const QString path = out.filePath(QStringLiteral("book.epub"));
        QVERIFY(w.exportEpub(path, QStringLiteral("My Book"), QStringLiteral("Pat Writer"), false));
        QMap<QString, QByteArray> z;
        QVERIFY(readZip(fileBytes(path), z));
        for (const QString &chapter : {QStringLiteral("chapter2"), QStringLiteral("chapter3"), QStringLiteral("nav")}) {
            QTextBrowser view;
            view.resize(480, 420);
            for (auto it = z.cbegin(); it != z.cend(); ++it)
                if (it.key().startsWith(QLatin1String("OEBPS/images/"))) view.document()->addResource(QTextDocument::ImageResource, QUrl(it.key().mid(6)), QImage::fromData(it.value()));
            QString html = QString::fromUtf8(z.value(QStringLiteral("OEBPS/") + chapter + QStringLiteral(".xhtml")));
            html.remove(QRegularExpression(QStringLiteral("^<\\?xml[^>]*>\\s*<!DOCTYPE html>")));
            html.remove(QRegularExpression(QStringLiteral("<nav epub:type=\"landmarks\".*?</nav>"), QRegularExpression::DotMatchesEverythingOption));   // (a reader hides it; this view does not)
            view.setHtml(html);
            view.show();
            QVERIFY(QTest::qWaitForWindowExposed(&view));
            view.grab().save(dir + QLatin1Char('/') + QStringLiteral("epub-") + chapter + QStringLiteral(".png"));
        }
    }

    // A web page opens as a document (what Save as Web Page wrote comes back with its words), and Save does not write over it.
    void aWebPageOpensAsADocumentAndIsNotWrittenOver()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("caf\u00e9 & more\nsecond"));
        const QString path = dir.filePath(QStringLiteral("page.html"));
        QVERIFY(w.exportWeb(path, true));
        WriteWindow b;
        Quiet quietB(b);
        QVERIFY(b.openFile(path));
        QCOMPARE(b.document()->story()->toPlainText().trimmed(), QStringLiteral("caf\u00e9 & more\nsecond"));
        const QByteArray before = fileBytes(path);
        b.setSavePathChooser([](const QString &, const QString &) { return QString(); });
        QVERIFY(!b.save());   // (it asks for a name, and none is given)
        QCOMPARE(fileBytes(path), before);
    }

    // Opening a web page reads its links, its lists, and the pictures that are in files beside it or in the page itself; nothing is fetched.
    void aWebPageOpensWithItsPicturesLinksAndLists()
    {
        QTemporaryDir dir;
        QImage pic(30, 20, QImage::Format_RGB32);
        pic.fill(Qt::blue);
        QVERIFY(pic.save(dir.filePath(QStringLiteral("beside.png")), "PNG"));
        const QString path = dir.filePath(QStringLiteral("page.html"));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("<html><body><p>See <a href=\"https://example.com/a\">the site</a> <img src=\"beside.png\"> <img src=\"http://example.invalid/no.png\"></p>"
                    "<ul><li>first</li><li>second</li></ul><ol><li>one</li></ol></body></html>");
        }
        WriteWindow b;
        Quiet quiet(b);
        QVERIFY(b.openFile(path));
        const QByteArray docx = b.document()->toDocx();
        office::Package p;
        QVERIFY(p.read(docx));
        QVERIFY2(p.part(QStringLiteral("/word/document.xml")).contains("<w:drawing"), "no picture");
        int media = 0;
        for (const QString &name : p.partNames()) media += name.startsWith(QLatin1String("/word/media/"));
        QCOMPARE(media, 1);
        const QByteArray body = p.part(QStringLiteral("/word/document.xml"));
        QVERIFY(body.contains("<w:hyperlink"));
        QVERIFY(body.contains("<w:numPr>"));
        QVERIFY(!body.contains("\xE2\x80\xA2"));   // (no bullet characters as text)
        QCOMPARE(b.document()->story()->toPlainText().trimmed().section(QLatin1Char('\n'), 1, 1), QStringLiteral("first"));
    }

    // Export to PDF can make a PDF/A-1b file: the standard's mark is in its metadata, and an ordinary export has none.
    void aPdfCanBeExportedAsPdfA()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("keep this"));
        QString target;
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        target = dir.filePath(QStringLiteral("plain.pdf"));
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        WriteOutputOptions o;
        o.pdfArchive = true;
        target = dir.filePath(QStringLiteral("archive.pdf"));
        QVERIFY(w.exportPdfPages(o));
        const QByteArray plain = fileBytes(dir.filePath(QStringLiteral("plain.pdf"))), kept = fileBytes(target);
        QVERIFY(plain.startsWith("%PDF") && kept.startsWith("%PDF"));
        QVERIFY(!plain.contains("pdfaid:part"));
        QVERIFY2(kept.contains("<pdfaid:part>1</pdfaid:part>"), "no PDF/A mark");
        QVERIFY(!kept.contains("<pdfaid:part>2</pdfaid:part>"));
        QVERIFY(kept.contains("GTS_PDFA1") || kept.contains("OutputIntent"));
    }

    // A PDF can be protected with a password: the file is encrypted (AES-256), PDF/A is off with it, the PDF opens with the password and not without, and the plain PDF is not left beside it.
    void aPdfCanBeProtectedWithAPassword()
    {
        QTemporaryDir dir, keep;
        ScratchTemp scratch;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("keep this"));
        QString target = keep.filePath(QStringLiteral("plain.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        // The control: with no password the watch sees the plain PDF.
        PlainPdfWatch watch;
        watch.dirs = {keep.path()};
        QVERIFY(w.exportPdfPages(WriteOutputOptions()));
        QVERIFY2(!watch.plain.isEmpty(), "the watch does not see a plain PDF");
        target = dir.filePath(QStringLiteral("locked.pdf"));
        watch.dirs.clear();
        watch.plain.clear();
        watch.dirs = {dir.path(), scratch.path()};
        WriteOutputOptions o;
        o.pdfArchive = true;
        o.pdfProtect.openPassword = QStringLiteral("pw");
        o.pdfProtect.limited = true;
        o.pdfProtect.ownerPassword = QStringLiteral("owner");
        o.pdfProtect.allowPrint = true;
        o.pdfProtect.allowCopy = false;
        o.pdfProtect.allowChange = false;
        QVERIFY(w.exportPdfPages(o));
        watch.dirs.clear();
        QVERIFY2(watch.plain.isEmpty(), qPrintable(watch.plain.join(QLatin1Char(' '))));
        const QByteArray made = fileBytes(target);
        jp::PdfEncryptionInfo info;
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("pw"), &info));
        QCOMPARE(info.version, 5);
        QCOMPARE(info.revision, 6);
        QVERIFY(info.allowPrint && !info.allowCopy && !info.allowChange);
        QVERIFY(!jp::PdfDocument(made).isValid());
        const jp::PdfDocument opened(made, QStringLiteral("pw"));
        QVERIFY(opened.isValid());
        QVERIFY(opened.text(0).contains(QStringLiteral("keep this")));
        QVERIFY(!jp::decryptPdf(made, QStringLiteral("pw")).contains("pdfaid:part"));
        QCOMPARE(QDir(dir.path()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries), QStringList{QStringLiteral("locked.pdf")});
        QVERIFY2(QDir(scratch.path()).entryList(QDir::Files).isEmpty(), qPrintable(QDir(scratch.path()).entryList(QDir::Files).join(QLatin1Char(' '))));
    }

    // Make an Email with the PDF can put a password on it: the attachment is encrypted, and no plain PDF is on the disk meanwhile.
    void anEmailedPdfCanHaveAPassword()
    {
        QTemporaryDir dir;
        ScratchTemp scratch;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("mailed"));
        const QString target = dir.filePath(QStringLiteral("mail.eml"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        jp::PdfProtection p;
        p.openPassword = QStringLiteral("pw");
        PlainPdfWatch watch;
        watch.dirs = {dir.path(), scratch.path()};
        QVERIFY(w.exportEmail(WriteWindow::EmailWhat::Pdf, p));
        watch.dirs.clear();
        QVERIFY2(watch.plain.isEmpty(), qPrintable(watch.plain.join(QLatin1Char(' '))));
        const QByteArray attached = pdfAttachmentOf(fileBytes(target));
        jp::PdfEncryptionInfo info;
        QVERIFY(jp::readPdfEncryption(attached, QStringLiteral("pw"), &info));
        QCOMPARE(info.version, 5);
        QCOMPARE(info.revision, 6);
        QVERIFY(!jp::PdfDocument(attached).isValid());
        QVERIFY(jp::PdfDocument(attached, QStringLiteral("pw")).isValid());
        // No password: a plain attachment, as before.
        QVERIFY(w.exportEmail(WriteWindow::EmailWhat::Pdf));
        QVERIFY(jp::PdfDocument(pdfAttachmentOf(fileBytes(target))).isValid());
    }

    // The Export to PDF box offers the password, and turns PDF/A off and out of reach while one is set.
    void theExportBoxOffersAPasswordAndTurnsOffPdfA()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("boxed"));
        const QString target = dir.filePath(QStringLiteral("boxed.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        setPdfProtectionStandIn([](bool, jp::PdfProtection *p) {
            p->openPassword = QStringLiteral("pw");
            return NewPasswordChoice::Set;
        });
        bool offered = false, archiveOff = false;
        std::function<void(int)> answer = [&](int tries) {
            QTimer::singleShot(10, [&, tries] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!d || d->objectName() != QLatin1String("pdfDialog")) {
                    if (tries > 0) answer(tries - 1);
                    return;
                }
                auto *archive = d->findChild<QCheckBox *>(QStringLiteral("pdfArchive"));
                auto *button = d->findChild<QPushButton *>(QStringLiteral("pdfEncryptButton"));
                offered = archive && button;
                if (offered) {
                    archive->setChecked(true);
                    button->click();
                    archiveOff = !archive->isChecked() && !archive->isEnabled();
                }
                d->accept();
            });
        };
        answer(200);
        QVERIFY(w.exportPdfAsk());
        setPdfProtectionStandIn(nullptr);
        QVERIFY(offered);
        QVERIFY(archiveOff);
        const QByteArray made = fileBytes(target);
        jp::PdfEncryptionInfo info;
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("pw"), &info));
        QVERIFY(info.encrypted);
        QVERIFY(!jp::decryptPdf(made, QStringLiteral("pw")).contains("pdfaid:part"));
    }

    // The PDF/A choice names the part the file has (part 1: what Qt's writer makes).
    void thePdfAChoiceNamesPartOne()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("boxed"));
        const QString target = dir.filePath(QStringLiteral("boxed.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        QString tip;
        std::function<void(int)> answer = [&](int tries) {
            QTimer::singleShot(10, [&, tries] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!d || d->objectName() != QLatin1String("pdfDialog")) {
                    if (tries > 0) answer(tries - 1);
                    return;
                }
                auto *archive = d->findChild<QCheckBox *>(QStringLiteral("pdfArchive"));
                if (archive) {
                    tip = archive->toolTip();
                    archive->setChecked(true);
                }
                d->accept();
            });
        };
        answer(200);
        QVERIFY(w.exportPdfAsk());
        QVERIFY2(tip.contains(QLatin1String("PDF/A-1b")) && !tip.contains(QLatin1String("2b")), qPrintable(tip));
        const QByteArray made = fileBytes(target);
        QVERIFY(made.contains("<pdfaid:part>1</pdfaid:part>"));
        QVERIFY(!made.contains("<pdfaid:part>2</pdfaid:part>"));
    }

    void textFilesAreReadAndWrittenInTheEncodingAndLineEndsChosen()
    {
        // UTF-16 either way round, the mark deciding; a code page; and the three line ends.
        const QString words = QStringLiteral("café Ж\nsecond");
        const QByteArray be = QByteArray("\xFE\xFF", 2) + QByteArray(QStringEncoder(QStringConverter::Utf16BE)(QStringLiteral("a\r\nb")));
        TextForm f = detectTextForm(be);
        QVERIFY(f.encoding == TextEncoding::Utf16Be && f.ends == LineEnds::Windows);
        QCOMPARE(decodeTextFile(be, f), QStringLiteral("a\nb"));
        const QByteArray utf8 = QByteArray("\xEF\xBB\xBF" "x\ry\rz");
        f = detectTextForm(utf8);
        QVERIFY(f.encoding == TextEncoding::Utf8WithMark && f.ends == LineEnds::ClassicMac);
        QCOMPARE(decodeTextFile(utf8, f), QStringLiteral("x\ny\nz"));
        // bytes that are not UTF-8 are in the page of the language given
        const QByteArray cyr("\xCF\xF0\xE8\xE2\xE5\xF2\n");
        f = detectTextForm(cyr, 1251);
        QVERIFY(f.encoding == TextEncoding::CodePage && f.codePage == 1251 && f.ends == LineEnds::Unix);
        QCOMPARE(decodeTextFile(cyr, f), QString::fromUtf16(u"Привет\n"));
        QVERIFY(looksLikePlainText(cyr) && looksLikePlainText(be));
        QVERIFY(!looksLikePlainText(QByteArray("PK\x03\x04", 4)) && !looksLikePlainText(QByteArray("a\0b", 3)) && !looksLikePlainText(QByteArray("{\\rtf1 x")));
        // written
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), words);
        int lost = 0;
        QByteArray out = plainTextFile(*w.document(), TextEncoding::CodePage, LineEnds::Windows, &lost, 1251);
        QCOMPARE(out, QByteArray("caf? \xC6\r\nsecond"));
        QCOMPARE(lost, 1);   // (the e with an acute accent is not in the Cyrillic page)
        out = plainTextFile(*w.document(), TextEncoding::Utf16Be, LineEnds::ClassicMac);
        QCOMPARE(out.left(2), QByteArray("\xFE\xFF", 2));
        QCOMPARE(decodeTextFile(out, detectTextForm(out)), words);
        QVERIFY(canWriteCodePage(1252) && canWriteCodePage(28592) && !canWriteCodePage(99999));
    }

    void aTextFileOpensWithoutAskingWhenItsEncodingIsPlain()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QByteArray utf16 = QByteArray("\xFE\xFF", 2) + QByteArray(QStringEncoder(QStringConverter::Utf16BE)(QStringLiteral("one é\r\ntwo")));
        const QList<QPair<QByteArray, QString>> files = {{QByteArray("plain ascii\ntwo"), QStringLiteral("plain ascii\ntwo")},
                                                         {QByteArray("\xEF\xBB\xBF" "bom \xC3\xA9"), QStringLiteral("bom é")},
                                                         {QByteArray("utf8 caf\xC3\xA9\r\nx"), QStringLiteral("utf8 café\nx")},
                                                         {utf16, QStringLiteral("one é\ntwo")}};
        int n = 0;
        for (const auto &f : files) {
            const QString path = dir.filePath(QStringLiteral("f%1.txt").arg(n++));
            QFile out(path);
            QVERIFY(out.open(QIODevice::WriteOnly) && out.write(f.first) == f.first.size());
            out.close();
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.setTextFormChooser([&](TextForm *, const QByteArray *) {
                QTest::qFail("asked about a file whose encoding is plain", __FILE__, __LINE__);
                return false;
            });
            QVERIFY(w.openFile(path));
            QCOMPARE(visibleText(*w.document()), f.second);
            // saved again without a question, in the form it was read in
            QVERIFY(w.save());
            QVERIFY(!w.isModified());
        }
        // (a file that is not valid UTF-8 is still asked about: see the next test)
    }

    void aTextFileThatIsNotUtf8IsAskedAboutAndSavesBackTheSameWay()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("notes.txt"));
        const QByteArray original("caf\xE9\r\ntwo\r\n\r\nfour");
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly) && f.write(original) == original.size());
        }
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        QByteArray shown;
        TextForm offered;
        w.setTextFormChooser([&](TextForm *form, const QByteArray *sample) {
            if (sample) {
                shown = *sample;
                offered = *form;
            }
            form->encoding = TextEncoding::CodePage;
            form->codePage = 1252;
            return true;
        });
        QVERIFY(w.openFile(path));
        QCOMPARE(shown, original);
        QVERIFY(offered.encoding == TextEncoding::CodePage && offered.ends == LineEnds::Windows);
        QCOMPARE(visibleText(*w.document()), QStringLiteral("café\ntwo\n\nfour"));
        QCOMPARE(w.document()->story()->blockCount(), 4);
        QVERIFY(!w.isModified());
        w.setTextFormChooser([&](TextForm *, const QByteArray *) {
            QTest::qFail("the form is asked a second time", __FILE__, __LINE__);
            return false;
        });
        QVERIFY(w.save());
        QCOMPARE(fileBytes(path), original);
        QVERIFY(!said.isEmpty() && said.last().contains(QLatin1String("only the words")));
        // cancelling the choice opens nothing
        WriteWindow v;
        v.setTextFormChooser([](TextForm *, const QByteArray *) { return false; });
        QVERIFY(!v.openFile(path));
    }

    void saveAsTextAsksOnlyForTheEncodingAndRtfStillAsksAboutWhatItLoses()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("some words"));
        int askedForm = 0, askedFormat = 0;
        w.setTextFormChooser([&](TextForm *, const QByteArray *) {
            ++askedForm;
            return true;
        });
        w.setFormatAsker([&](const QStringList &) {
            ++askedFormat;
            return WriteWindow::FormatChoice::Keep;
        });
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("out.txt"))));
        QCOMPARE(askedForm, 1);
        QCOMPARE(askedFormat, 0);
        // a Rich Text Format file holds less than the document too: that one asks, and Cancel writes nothing
        w.setFormatAsker([&](const QStringList &lost) {
            ++askedFormat;
            if (lost.isEmpty()) QTest::qFail("nothing named as lost", __FILE__, __LINE__);
            return WriteWindow::FormatChoice::Cancel;
        });
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("!"));
        QTextCursor c(w.document()->story());
        c.select(QTextCursor::Document);
        const QString rtf = dir.filePath(QStringLiteral("out.rtf"));
        // (something the format can't hold: a comment)
        QVERIFY(addComment(*w.document(), c, QStringLiteral("a note")) >= 0);
        QVERIFY(!w.saveTo(rtf));
        QCOMPARE(askedFormat, 1);
        QVERIFY(!QFileInfo::exists(rtf));
    }

    void aDocumentSavedAsTextAsksOnceThenKeepsTheForm()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("café €\nend"));
        int asked = 0;
        w.setTextFormChooser([&](TextForm *form, const QByteArray *) {
            ++asked;
            form->encoding = TextEncoding::CodePage;
            form->codePage = 1252;
            form->ends = LineEnds::Windows;
            return true;
        });
        const QString path = dir.filePath(QStringLiteral("out.txt"));
        QVERIFY(w.saveTo(path));
        QCOMPARE(asked, 1);
        QCOMPARE(fileBytes(path), QByteArray("caf\xE9 \x80\r\nend"));
        QVERIFY(w.save());
        QCOMPARE(asked, 1);
        QVERIFY(!w.isModified());
        // a lost character is said
        typeText(*w.document(), w.view()->cursor(), QStringLiteral(" Ж"));
        QVERIFY(w.save());
        QVERIFY(said.last().contains(QLatin1String("question marks")));
    }

    // ---- the review's items
    void aTitleEditLeavesTheCustomPartAsItWas()
    {
        const QByteArray custom =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\" "
            "xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">"
            "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"2\" name=\"A\"><vt:lpwstr>x</vt:lpwstr></property>"
            "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"3\" name=\"V\"><vt:vector size=\"1\" baseType=\"i4\"><vt:i4>5</vt:i4></vt:vector></property>"
            "<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"4\" name=\"B\"><vt:i8>5</vt:i8></property></Properties>";
        const QByteArray zip = makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>", {},
                                        {{QStringLiteral("/docProps/custom.xml"), QString::fromLatin1(office::kCustomPartType), custom, QString::fromLatin1(office::kCustomRelationship), {}, {}}});
        auto doc = openDocx(zip);
        QVERIFY(doc);
        QCOMPARE(doc->packageProps().custom.size(), 3);
        office::PackageProps p = doc->packageProps();
        p.title = QStringLiteral("Edited");
        doc->setPackageProps(p);
        const QByteArray saved = doc->toDocx();
        QCOMPARE(partOf(saved, QStringLiteral("/docProps/custom.xml")), custom);
        QCOMPARE(openDocx(saved)->packageProps().title, QStringLiteral("Edited"));
        // a change of the list does rewrite it, and keeps what is not modeled
        p.custom[0].value = QStringLiteral("y");
        doc->setPackageProps(p);
        const QByteArray again = partOf(doc->toDocx(), QStringLiteral("/docProps/custom.xml"));
        QVERIFY(again.contains("<vt:vector size=\"1\" baseType=\"i4\"><vt:i4>5</vt:i4></vt:vector>"));
        QVERIFY(again.contains(">y<"));
    }

    void thePropertiesPartsAreFoundByTheirRelationshipNotTheirNames()
    {
        const QByteArray core = "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\"><dc:title>Elsewhere</dc:title></cp:coreProperties>";
        const QByteArray zip = makeDocx("<w:p/>", {},
                                        {{QStringLiteral("/meta/info/c.xml"), QString::fromLatin1(office::kCorePartType), core, QString::fromLatin1(office::kCoreRelationship), {}, {}}});
        auto doc = openDocx(zip);
        QVERIFY(doc);
        QCOMPARE(doc->packageProps().title, QStringLiteral("Elsewhere"));
        office::PackageProps p = doc->packageProps();
        p.title = QStringLiteral("Changed");
        p.company = QStringLiteral("Acme");
        doc->setPackageProps(p);
        const QByteArray saved = doc->toDocx();
        QVERIFY(partOf(saved, QStringLiteral("/meta/info/c.xml")).contains("Changed"));
        QVERIFY(!partNamesOf(saved).contains(QStringLiteral("/docProps/core.xml")));
        office::Package pkg;
        QVERIFY(pkg.read(saved));
        QCOMPARE(pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/core-properties")).size(), 1);
        QCOMPARE(pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/extended-properties")).size(), 1);   // (the app part is made, once)
        QCOMPARE(openDocx(saved)->packageProps().company, QStringLiteral("Acme"));
        // and the fields read the same part
        QCOMPARE(documentProperties(*doc).title, QStringLiteral("Changed"));
    }

    void aPartWithMoreThanIsReadIsShownReadOnlyAndNeverRewritten()
    {
        QByteArray core = "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\"><dc:title>Old</dc:title>";
        for (int i = 0; i < 120000; ++i) core += "<cp:x/>";
        core += "</cp:coreProperties>";
        const QByteArray zip = makeDocx("<w:p/>", {}, {{QStringLiteral("/docProps/core.xml"), QString::fromLatin1(office::kCorePartType), core, QString::fromLatin1(office::kCoreRelationship), {}, {}}});
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        QTemporaryDir dir;
        QFile f(dir.filePath(QStringLiteral("big.docx")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(zip);
        f.close();
        QVERIFY(w.openFile(f.fileName()));
        QVERIFY(w.document()->packageProps().truncated);
        office::PackageProps p = w.document()->packageProps();
        p.title = QStringLiteral("New");
        w.applyProperties(p);
        QCOMPARE(w.document()->propsPartData(QStringLiteral("/core-properties")), core);
        w.show();
        w.showFileScreen(QStringLiteral("info"));
        QWidget *page = w.findChild<FileScreen *>()->findChild<QWidget *>(QStringLiteral("infoPage"));
        QVERIFY(page->findChild<QWidget *>(QStringLiteral("infoPropertiesReadOnly")));
        QVERIFY(page->findChild<QLineEdit *>(QStringLiteral("infoTitle"))->isReadOnly());
        QVERIFY(!page->findChild<QPushButton *>(QStringLiteral("infoCustom"))->isEnabled());
        w.hideFileScreen();
    }

    void removingEveryCommentLeavesNoNameInTheFile()
    {
        const QByteArray commentsPart = "<?xml version=\"1.0\"?><w:comments " + QByteArray(kNs) + "><w:comment w:id=\"0\" w:author=\"Zed Quux\" w:initials=\"ZQ\" w:date=\"2026-01-01T00:00:00Z\"><w:p><w:r><w:t>note</w:t></w:r></w:p></w:comment></w:comments>";
        const QByteArray people = "<?xml version=\"1.0\"?><w15:people xmlns:w15=\"http://schemas.microsoft.com/office/word/2012/wordml\"><w15:person w15:author=\"Zed Quux\"><w15:presenceInfo w15:providerId=\"None\" w15:userId=\"Zed Quux\"/></w15:person></w15:people>";
        const QByteArray ex = "<?xml version=\"1.0\"?><w15:commentsEx xmlns:w15=\"http://schemas.microsoft.com/office/word/2012/wordml\"/>";
        const QByteArray ids = "<?xml version=\"1.0\"?><w16cid:commentsIds xmlns:w16cid=\"http://schemas.microsoft.com/office/word/2016/wordml/cid\"/>";
        const QByteArray zip = makeDocx("<w:p><w:commentRangeStart w:id=\"0\"/><w:r><w:t>hello</w:t></w:r><w:commentRangeEnd w:id=\"0\"/><w:r><w:commentReference w:id=\"0\"/></w:r></w:p>",
                                        {{QStringLiteral("/word/comments.xml"), kWord + QStringLiteral("comments+xml"), commentsPart, kRel + QStringLiteral("comments"), {}, {}},
                                         {QStringLiteral("/word/people.xml"), kWord + QStringLiteral("people+xml"), people, QStringLiteral("http://schemas.microsoft.com/office/2011/relationships/people"), {}, {}},
                                         {QStringLiteral("/word/commentsExtended.xml"), kWord + QStringLiteral("commentsExtended+xml"), ex, QStringLiteral("http://schemas.microsoft.com/office/2011/relationships/commentsExtended"), {}, {}},
                                         {QStringLiteral("/word/commentsIds.xml"), kWord + QStringLiteral("commentsIds+xml"), ids, QStringLiteral("http://schemas.microsoft.com/office/2016/09/relationships/commentsIds"), {}, {}}});
        auto doc = openDocx(zip);
        QVERIFY(doc);
        rememberCommentsAtOpen(*doc);
        QCOMPARE(int(comments(*doc).size()), 1);
        QCOMPARE(deleteAllComments(*doc), 1);
        const QByteArray saved = doc->toDocx();
        office::Package pkg;
        QVERIFY(pkg.read(saved));
        for (const QString &name : pkg.partNames())
            QVERIFY2(!pkg.part(name).contains("Zed Quux"), qPrintable(name));
        QVERIFY(!pkg.partNames().contains(QStringLiteral("/word/people.xml")));
        QVERIFY(!pkg.partNames().contains(QStringLiteral("/word/commentsExtended.xml")));
        QVERIFY(!pkg.partNames().contains(QStringLiteral("/word/commentsIds.xml")));
        QVERIFY(!partOf(saved, QStringLiteral("/word/_rels/document.xml.rels")).contains("people"));
    }

    void hiddenTextIsFoundAndRemovedInEveryStory()
    {
        const QByteArray header = "<w:hdr " + QByteArray(kNs) + "><w:p><w:r><w:t>Visible head</w:t></w:r><w:r><w:rPr><w:vanish/></w:rPr><w:t>SECRETHEAD</w:t></w:r></w:p></w:hdr>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"1\"><w:p><w:r><w:t>Note</w:t></w:r><w:r><w:rPr><w:vanish/></w:rPr><w:t>SECRETNOTE</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray body = "<w:p><w:r><w:t>Body</w:t></w:r><w:r><w:rPr><w:vanish/></w:rPr><w:t>SECRETBODY</w:t></w:r><w:r><w:footnoteReference w:id=\"1\"/></w:r></w:p>"
                                "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdH\"/></w:sectPr>";
        const QByteArray zip = makeDocx(body, {{QStringLiteral("/word/header1.xml"), kWord + QStringLiteral("header+xml"), header, kRel + QStringLiteral("header"), {}, QStringLiteral("rIdH")},
                                               {QStringLiteral("/word/footnotes.xml"), kWord + QStringLiteral("footnotes+xml"), notes, kRel + QStringLiteral("footnotes"), {}, {}}});
        auto doc = openDocx(zip);
        QVERIFY(doc);
        QCOMPARE(hiddenTextCount(*doc), 3);
        QCOMPARE(removeHiddenText(*doc), 3);
        QCOMPARE(hiddenTextCount(*doc), 0);
        const QByteArray saved = doc->toDocx();
        office::Package pkg;
        QVERIFY(pkg.read(saved));
        for (const QString &name : pkg.partNames()) {
            QVERIFY2(!pkg.part(name).contains("SECRETHEAD"), qPrintable(name));
            QVERIFY2(!pkg.part(name).contains("SECRETNOTE"), qPrintable(name));
            QVERIFY2(!pkg.part(name).contains("SECRETBODY"), qPrintable(name));
        }
    }

    void markAsFinalIsRefusedWhereTheFileCantHoldIt()
    {
        QTemporaryDir dir;
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("x"));
        QVERIFY(saveInto(w, dir, QStringLiteral("a.odt")));
        QVERIFY(!w.markFinalBlocked().isEmpty());
        w.setMarkedFinal(true);
        QVERIFY(!w.isReadOnly());
        QVERIFY(!w.document()->markedAsFinal());
        QVERIFY(quiet.said.last().contains(QLatin1String(".odt")));
        // the page says so, and the card does nothing
        w.show();
        w.showFileScreen(QStringLiteral("info"));
        QWidget *card = w.findChild<FileScreen *>()->findChild<QWidget *>(QStringLiteral("infoMarkFinal"));
        QVERIFY(card);
        QVERIFY(!card->isEnabled());
        w.hideFileScreen();
        // a document marked as final and then saved as .odt is not marked, and the window agrees with the file
        WriteWindow v;
        Quiet quietV(v);
        typeText(*v.document(), v.view()->cursor(), QStringLiteral("y"));
        QVERIFY(saveInto(v, dir, QStringLiteral("b.docx")));
        v.setMarkedFinal(true);
        QVERIFY(v.isReadOnly());
        QVERIFY(saveInto(v, dir, QStringLiteral("c.odt")));
        QVERIFY(!v.isReadOnly());
        QVERIFY(!v.document()->markedAsFinal());
    }

    // A password set while a copy is being made: the copy is not kept (it would hold the text in the clear).
    void aPasswordSetWhileACopyIsBeingMadeLeavesNoCopy()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".docx"));
        {
            auto gate = std::make_shared<RecoveryGate>();
            const auto letGo = qScopeGuard([gate] { gate->open(); });
            WriteWindow w;
            Quiet quiet(w);
            typeText(*w.document(), w.view()->cursor(), QStringLiteral("secret words"));
            w.setRecoveryHook(RecoveryGate::hook(gate));
            w.autoRecover();
            QVERIFY(w.recoveryBusy());
            setNewPasswordStandIn([](bool, QString *password) {
                *password = QStringLiteral("pw");
                return NewPasswordChoice::Set;
            });
            w.encryptWithPassword();
            gate->open();
            recovery::AsyncWriter::waitForAll();
            QCoreApplication::processEvents();
            QVERIFY(w.recoveryCopy().isEmpty());
            QDirIterator left(root.path(), {QStringLiteral("*.docx"), QStringLiteral("*.json")}, QDir::Files, QDirIterator::Subdirectories);
            QVERIFY(!left.hasNext());
            w.document()->setModified(false);
        }
        recovery::setRoot(QString());
    }

    void aProtectedDocumentIsNeverRecoveredAsPlainText()
    {
        WriteWindow w;
        Quiet quiet(w);
        typeText(*w.document(), w.view()->cursor(), QStringLiteral("secret words"));
        setNewPasswordStandIn([](bool, QString *password) {
            *password = QStringLiteral("pw");
            return NewPasswordChoice::Set;
        });
        w.encryptWithPassword();
        w.autoRecover();
        QVERIFY(w.recoveryCopy().isEmpty());
        QVERIFY(w.statusBar()->currentMessage().contains(QLatin1String("recovery copy")));
        // (a copy written before the document was protected is taken away, not left with its text in the clear)
        WriteWindow early;
        typeText(*early.document(), early.view()->cursor(), QStringLiteral("before the password"));
        early.autoRecover();
        QTRY_VERIFY_WITH_TIMEOUT(!early.recoveryCopy().isEmpty(), 60000);   // (made on a worker thread)
        const QString clear = early.recoveryCopy();
        QVERIFY(!clear.isEmpty());
        QVERIFY(QFileInfo::exists(clear));
        early.encryptWithPassword();
        early.autoRecover();
        QVERIFY(early.recoveryCopy().isEmpty());
        QVERIFY(!QFileInfo::exists(clear));
        // (an unprotected one is kept, as it was)
        WriteWindow v;
        typeText(*v.document(), v.view()->cursor(), QStringLiteral("plain words"));
        v.autoRecover();
        QTRY_VERIFY_WITH_TIMEOUT(!v.recoveryCopy().isEmpty(), 60000);
        QFile::remove(v.recoveryCopy());
    }

    void removedPersonalInformationStaysOutOfLaterEdits()
    {
        WriteWindow w;
        office::PackageProps p = w.document()->packageProps();
        p.creator = QStringLiteral("Ann");
        p.company = QStringLiteral("Acme");
        w.applyProperties(p);
        QCOMPARE(w.document()->packageProps().lastModifiedBy, QStringLiteral("Tester"));
        QCOMPARE(w.removePersonalInformation(), 3);
        p = w.document()->packageProps();
        p.title = QStringLiteral("After");
        w.applyProperties(p);
        const office::PackageProps now = w.document()->packageProps();
        QCOMPARE(now.title, QStringLiteral("After"));
        QVERIFY(now.lastModifiedBy.isEmpty());
        QVERIFY(now.creator.isEmpty());
        QVERIFY(now.company.isEmpty());
    }

    void theCustomPropertiesBoxRefusesWhatItCannotKeep()
    {
        CustomPropertiesDialog d({}, nullptr);
        d.addRow({QStringLiteral("Count"), QStringLiteral("i4"), QStringLiteral("7"), 0, QString(), QByteArray()});
        QVERIFY(d.problem().isEmpty());
        d.setCell(0, 2, QStringLiteral("seven"));
        QVERIFY(d.problem().contains(QLatin1String("whole number")));
        d.setCell(0, 2, QStringLiteral("7"));
        d.addRow({QStringLiteral("When"), QStringLiteral("filetime"), QStringLiteral("not a date"), 0, QString(), QByteArray()});
        QVERIFY(d.problem().contains(QLatin1String("not a date")));
        d.setCell(1, 2, QStringLiteral("2026-11-04"));
        QVERIFY(d.problem().isEmpty());
        d.addRow({QStringLiteral("count"), QStringLiteral("lpwstr"), QStringLiteral("x"), 0, QString(), QByteArray()});
        QVERIFY(d.problem().contains(QLatin1String("twice")));
        d.setCell(2, 0, QStringLiteral("_MarkAsFinal"));
        QVERIFY(d.problem().contains(QLatin1String("_MarkAsFinal")));
        d.setCell(2, 0, QStringLiteral("Fine"));
        QVERIFY(d.problem().isEmpty());
        d.addRow({QString(), QStringLiteral("lpwstr"), QString(), 0, QString(), QByteArray()});
        QVERIFY(d.problem().contains(QLatin1String("no name")));
    }

    void theCustomPropertiesBoxHoldsThousandsOfRowsAndKeepsWhatItDoesNotModel()
    {
        QVector<office::CustomProperty> many;
        for (int i = 0; i < 5000; ++i) many.push_back({QStringLiteral("p%1").arg(i), QStringLiteral("lpwstr"), QStringLiteral("v"), i + 2, QString(), QByteArray()});
        many[10].raw = "<property pid=\"12\" name=\"p10\"><vt:i8 xmlns:vt=\"urn:v\">1</vt:i8></property>";
        QElapsedTimer t;
        t.start();
        CustomPropertiesDialog d(many, nullptr);
        QCOMPARE(d.rowCount(), 5000);
        QVERIFY2(t.elapsed() < 20000 * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")), "the box takes too long to make");
        QVERIFY(d.problem().isEmpty());
        QCOMPARE(d.result(), many);   // (the property that is not modeled is as it was, and so is every pid)
    }

    void aLongKeyDoesNotFreezeTheWindowAndCanBeCancelled()
    {
        // (the cancel comes from the window's own timer after it has ticked a few times, so a busy machine that starts
        // the timers late can't make the test pass or fail by timing; a stall of a quarter second failed it on Windows CI)
        int ticks = 0;
        QTimer ticker;
        QObject::connect(&ticker, &QTimer::timeout, [&] { if (++ticks == 6) cancelRunningForTest(); });
        ticker.start(10);
        QTimer::singleShot(10000, [] { cancelRunningForTest(); });   // (the events never came: fail below, don't hang)
        const bool finished = runWithCancel(nullptr, QStringLiteral("working"), [](const std::atomic<bool> *cancel) {
            while (!cancel->load()) QThread::msleep(5);
        });
        QVERIFY(!finished);
        QVERIFY2(ticks > 5, "the window's events were not handled while the work ran");
        QVERIFY(runWithCancel(nullptr, QStringLiteral("working"), [](const std::atomic<bool> *) {}));
    }

    // The window closes during the first 300 ms, before the note exists: the note is not made on it.
    void aWindowClosedBeforeTheNoteShowsIsLeftAlone()
    {
        auto *w = new QWidget;
        w->show();
        QTimer::singleShot(100, [w] { delete w; });
        QVERIFY(runWithCancel(w, QStringLiteral("working"), [](const std::atomic<bool> *) { QThread::msleep(600); }));
    }

    // Until the note shows, the window takes no input (it is enabled again after), and a second call while one runs is refused and runs nothing.
    void theWindowTakesNoInputWhileTheWorkRunsAndASecondCallIsRefused()
    {
        QWidget w;
        w.show();
        bool disabledMeanwhile = false, nestedRan = false, nestedResult = true;
        QTimer::singleShot(100, [&] {
            disabledMeanwhile = !w.isEnabled();
            nestedResult = runWithCancel(&w, QStringLiteral("again"), [&](const std::atomic<bool> *) { nestedRan = true; });
        });
        QVERIFY(runWithCancel(&w, QStringLiteral("working"), [](const std::atomic<bool> *) { QThread::msleep(500); }));
        QVERIFY(disabledMeanwhile);
        QVERIFY(!nestedResult);
        QVERIFY(!nestedRan);
        QVERIFY(w.isEnabled());
        QVERIFY(runWithCancel(&w, QStringLiteral("after"), [](const std::atomic<bool> *) {}));   // (the flag of the first was let go)
    }

    // A real key with many rounds of hashing, and a click on the Cancel button of the note that shows while it is made.
    void theCancelButtonOfTheNoteStopsALongKey()
    {
        const QByteArray zip = makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>");
        office::EncryptOptions options;
        options.spinCount = 2000000;
        const QByteArray file = office::encryptPackage(zip, QStringLiteral("right"), options);
        QByteArray info, package;
        QVERIFY(office::splitProtectedPackage(file, &info, &package));
        QTimer press;
        bool pressed = false;
        QObject::connect(&press, &QTimer::timeout, [&] {
            for (QWidget *w : QApplication::topLevelWidgets()) {
                if (w->objectName() != QLatin1String("workNote")) continue;
                if (auto *button = w->findChild<QPushButton *>()) {
                    pressed = true;
                    QTest::mouseClick(button, Qt::LeftButton);
                }
            }
        });
        press.start(50);
        QByteArray out;
        office::CryptResult result = office::CryptResult::Ok;
        QElapsedTimer t;
        t.start();
        const bool finished = runWithCancel(nullptr, QStringLiteral("Checking"), [&](const std::atomic<bool> *cancel) {
            // (again until the click lands: on a fast machine one key, at the most rounds a file may ask for, was made before the note's
            // 300 ms, Windows CI Nov 17; a cancel between two tries is seen at the start of the next, so the result is still Cancelled)
            do {
                result = office::decryptPackage(info, package, QStringLiteral("wrong"), office::kMaxOpenFileBytes, false, &out, [cancel] { return cancel->load(); });
            } while (result != office::CryptResult::Cancelled && t.elapsed() < 20000);
        });
        QVERIFY2(pressed, "the note with a Cancel button did not show");
        QVERIFY(!finished);
        QCOMPARE(result, office::CryptResult::Cancelled);
    }

    // The window is closed (and deleted, as the app sets it) while the password is checked: the open stops there, with no question asked of the dead window.
    void aWindowClosedWhileThePasswordIsCheckedStopsTheOpen()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        office::EncryptOptions options;
        options.spinCount = 1000000;
        const QString path = dir.filePath(QStringLiteral("p.docx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(office::encryptPackage(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>"), QStringLiteral("right"), options));
        f.close();
        bool asked = false;
        setOpenPasswordStandIn([&](const QString &, bool, QString *) {
            asked = true;
            return false;
        });
        auto *w = new WriteWindow;
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
        const QPointer<WriteWindow> alive(w);
        QTimer::singleShot(100, [w] { w->close(); });
        QVERIFY(!w->openFile(path));
        QTRY_VERIFY(!alive);
        QVERIFY2(!asked, "the question was put to the window that was closed");
        setOpenPasswordStandIn(nullptr);
    }

    void aWrongPartIsRemadeNotTrusted()
    {
        WriteWindow w;
        w.document()->putPackagePart(QStringLiteral("/docProps/core.xml"), QString::fromLatin1(office::kCorePartType), "<not xml");
        office::PackageProps p = w.document()->packageProps();
        p.title = QStringLiteral("Fixed");
        w.applyProperties(p);
        QCOMPARE(w.document()->packageProps().title, QStringLiteral("Fixed"));
    }
};

QTEST_MAIN(InfoTests)
#include "tst_write_info.moc"
