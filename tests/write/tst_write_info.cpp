// Tests of JeffWrite's Info page: the properties a .docx keeps, Mark as Final, a password, Inspect Document, and Check Compatibility. Every file is made here in code.

#include "../isolatesettings.h"
#include "app/filescreen.h"
#include "app/infopage.h"
#include "app/passworddialogs.h"
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
