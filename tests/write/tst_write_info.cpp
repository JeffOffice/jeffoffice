// Tests of JeffWrite's Info page: the properties a .docx keeps, Mark as Final, a password, Inspect Document, and Check Compatibility. Every file is made here in code.

#include "../isolatesettings.h"
#include "app/filescreen.h"
#include "app/infopage.h"
#include "app/passworddialogs.h"
#include "app/settings.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "app/pageview.h"
#include "core/appinfo.h"
#include "core/commentops.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/fonts.h"
#include "core/inspectops.h"
#include "core/linkops.h"
#include "core/textexport.h"
#include "core/writedoc.h"
#include "office/officecrypt.h"
#include "office/package.h"
#include "office/packageprops.h"
#include "text/textprops.h"

#include <QCheckBox>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QTemporaryDir>
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

    void aProtectedDocumentIsNotSavedAsAnOdtWithoutItsPassword()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow a;
        Quiet quiet(a);
        typeText(*a.document(), a.view()->cursor(), QStringLiteral("x"));
        setNewPasswordStandIn([](bool, QString *password) {
            *password = QStringLiteral("pw");
            return NewPasswordChoice::Set;
        });
        a.encryptWithPassword();
        QVERIFY(!a.saveTo(dir.filePath(QStringLiteral("o.odt"))));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("o.odt"))));
        QVERIFY(quiet.said.last().contains(QLatin1String("password")));
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
        QVERIFY(w.incompatibilities().join(QLatin1Char(' ')).contains(QLatin1String("password")));
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
        for (const char *name : {"infoMarkFinal", "infoEncrypt", "infoInspect", "infoCompatibility", "infoCustom", "infoShowFolder", "infoCopyPath"})
            QVERIFY2(page->findChild<QWidget *>(QString::fromLatin1(name)), name);
        QVERIFY(!page->findChild<QPushButton *>(QStringLiteral("infoCopyPath"))->isEnabled());   // (nothing saved yet)
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
        QVERIFY(!v.recoveryCopy().isEmpty());
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
        QVERIFY2(t.elapsed() < 20000, "the box takes too long to make");
        QVERIFY(d.problem().isEmpty());
        QCOMPARE(d.result(), many);   // (the property that is not modeled is as it was, and so is every pid)
    }

    void aLongKeyDoesNotFreezeTheWindowAndCanBeCancelled()
    {
        int ticks = 0;
        QTimer ticker;
        QObject::connect(&ticker, &QTimer::timeout, [&] { ++ticks; });
        ticker.start(10);
        QTimer::singleShot(250, [] { cancelRunningForTest(); });
        const bool finished = runWithCancel(nullptr, QStringLiteral("working"), [](const std::atomic<bool> *cancel) {
            while (!cancel->load()) QThread::msleep(5);
        });
        QVERIFY(!finished);
        QVERIFY2(ticks > 5, "the window's events were not handled while the work ran");
        QVERIFY(runWithCancel(nullptr, QStringLiteral("working"), [](const std::atomic<bool> *) {}));
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
            result = office::decryptPackage(info, package, QStringLiteral("wrong"), office::kMaxOpenFileBytes, false, &out, [cancel] { return cancel->load(); });
        });
        QVERIFY2(pressed, "the note with a Cancel button did not show");
        QVERIFY(!finished);
        QCOMPARE(result, office::CryptResult::Cancelled);
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
