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
#include "core/textexport.h"
#include "core/writedoc.h"
#include "office/officecrypt.h"
#include "office/package.h"
#include "office/packageprops.h"
#include "text/textprops.h"

#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QElapsedTimer>
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
