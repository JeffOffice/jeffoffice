// Tests of JeffWrite opening and saving OpenDocument text (.odt): the window's File > Open and Save As, and the document's reader and writer. Every file is built here from a few lines of XML.

#include "../stablesaves.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/writedoc.h"
#include "odt/odtread.h"
#include "io/zip.h"
#include "odt/odt.h"
#include "office/package.h"
#include "text/textengine.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] =
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"";

QByteArray makeOdt(const QByteArray &body, const QByteArray &autoStyles = QByteArray())
{
    jp::ZipWriter z;
    z.add(QStringLiteral("mimetype"), QByteArray("application/vnd.oasis.opendocument.text"), false);
    z.add(QStringLiteral("content.xml"),
          QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-content") + kNs + " office:version=\"1.2\"><office:automatic-styles>" + autoStyles +
              "</office:automatic-styles><office:body><office:text>" + body + "</office:text></office:body></office:document-content>",
          true);
    z.add(QStringLiteral("styles.xml"),
          QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><office:document-styles") + kNs + " office:version=\"1.2\"><office:styles><style:style style:name=\"Standard\" style:family=\"paragraph\"/>"
              "<style:style style:name=\"Heading_20_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:default-outline-level=\"1\"><style:text-properties fo:font-size=\"20pt\"/></style:style></office:styles>"
              "<office:automatic-styles/><office:master-styles/></office:document-styles>",
          true);
    return z.finish();
}

const QByteArray kBody =
    "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">A heading</text:h><text:p>Plain <text:span text:style-name=\"T1\">bold words</text:span> and more.</text:p>"
    "<text:list text:style-name=\"L1\"><text:list-item><text:p>first item</text:p></text:list-item><text:list-item><text:p>second item</text:p></text:list-item></text:list>"
    "<table:table><table:table-column table:number-columns-repeated=\"2\"/><table:table-row><table:table-cell><text:p>cell a</text:p></table:table-cell><table:table-cell><text:p>cell b</text:p></table:table-cell></table:table-row></table:table>"
    "<text:p>Last paragraph.</text:p>";
const QByteArray kAuto = "<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\"/></style:style>"
                         "<text:list-style style:name=\"L1\"><text:list-level-style-number text:level=\"1\" style:num-format=\"1\" style:num-suffix=\".\"/></text:list-style>";

bool hasBold(const WriteDoc &d, const QString &text)
{
    for (QTextBlock b = d.story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().text().contains(text) && it.fragment().charFormat().fontWeight() > 500) return true;
    return false;
}

QString writeFile(const QString &path, const QByteArray &bytes)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size()) return QString();
    return path;
}

} // namespace

class WriteOdtTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    void anOdtOpensWithItsTextAndLooks()
    {
        QString err;
        QStringList notes;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto), &err, &notes);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY(notes.isEmpty());
        const QString text = doc->story()->toPlainText();
        for (const char *w : {"A heading", "Plain bold words and more.", "first item", "second item", "cell a", "cell b", "Last paragraph."}) QVERIFY2(text.contains(QLatin1String(w)), w);
        QVERIFY(hasBold(*doc, QStringLiteral("bold words")));
        QVERIFY(!hasBold(*doc, QStringLiteral("Plain")));
    }

    // The markup of a paragraph's look is made once for each style, not for each paragraph: five thousand paragraphs of two styles make some looks, not thousands. (Counted.)
    void theLooksOfParagraphsAreMadeOnceForEachStyle()
    {
        QByteArray body;
        for (int i = 0; i < 5000; ++i) body += i % 2 ? "<text:p text:style-name=\"Standard\">Plain <text:span text:style-name=\"T1\">bold</text:span> words.</text:p>" : "<text:h text:style-name=\"Heading_20_1\" text:outline-level=\"1\">Heading</text:h>";
        const qint64 before = jp::office::odt::Reader::looksMadeForTest();
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(body, kAuto), &err, nullptr);
        QVERIFY2(doc, qPrintable(err));
        const qint64 made = jp::office::odt::Reader::looksMadeForTest() - before;
        QVERIFY2(made <= 40, qPrintable(QStringLiteral("%1 looks made for 5000 paragraphs").arg(made)));
    }

    void fileOpenFindsAnOdtByItsContent()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // (The name says nothing: the mimetype does.)
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.dat")), makeOdt(kBody, kAuto));
        WriteWindow w;
        QStringList notices;
        w.setNotifier([&](const QString &, const QString &text) { notices << text; });
        QVERIFY(w.openFile(path));
        QVERIFY(w.document()->story()->toPlainText().contains(QLatin1String("second item")));
        QVERIFY(notices.isEmpty());
        QVERIFY(!w.isModified());
        QVERIFY(w.document()->pager().pageCount() >= 1);
        // A damaged one is refused with a reason.
        QString err;
        QVERIFY(!WriteDoc::fromFile(makeOdt(kBody, kAuto).left(300), &err));
        QVERIFY(!err.isEmpty());
    }

    void saveAsOffersAnOdtAndWritesOne()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.odt")), makeOdt(kBody, kAuto));
        WriteWindow a;
        QVERIFY(a.openFile(path));
        QString suggested, filter;
        a.setSavePathChooser([&](const QString &s, const QString &f) {
            suggested = s;
            filter = f;
            return dir.filePath(QStringLiteral("copy.odt"));
        });
        QVERIFY(a.saveAs());
        // The dialog started on the kind the file is, with the suggested name in that ending.
        QVERIFY2(suggested.endsWith(QLatin1String("letter.odt")), qPrintable(suggested));
        QVERIFY2(filter.contains(QLatin1String("*.odt")), qPrintable(filter));
        QVERIFY(a.filePath().endsWith(QLatin1String("copy.odt")));
        QVERIFY(!a.isModified());
        QFile f(a.filePath());
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        QVERIFY(office::odt::looksLikeOdt(bytes));
        QVERIFY(bytes.startsWith("PK"));
        // It opens again with the same text and looks, and Save writes an .odt again (it is a kind JeffWrite writes).
        WriteWindow b;
        QVERIFY(b.openFile(a.filePath()));
        QCOMPARE(b.document()->story()->toPlainText(), a.document()->story()->toPlainText());
        QVERIFY(hasBold(*b.document(), QStringLiteral("bold words")));
        const qint64 before = QFileInfo(a.filePath()).size();
        QVERIFY(a.save());
        QCOMPARE(QFileInfo(a.filePath()).size(), before);
        QFile again(a.filePath());
        QVERIFY(again.open(QIODevice::ReadOnly));
        QVERIFY(office::odt::looksLikeOdt(again.readAll()));
    }

    void anOdtMadeFromADocxKeepsItsHeadingListAndTable()
    {
        QString err;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto), &err);
        QVERIFY(doc);
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        auto back = WriteDoc::fromFile(odt, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->story()->toPlainText(), doc->story()->toPlainText());
        QVERIFY(hasBold(*back, QStringLiteral("bold words")));
        // The same document's .docx is the same too: what is written is what was read.
        const QByteArray docx1 = doc->toDocx(), docx2 = back->toDocx();
        office::Package p1, p2;
        QVERIFY(p1.read(docx1) && p2.read(docx2));
        QCOMPARE(p1.part(QStringLiteral("/word/document.xml")).count("<w:tc>"), p2.part(QStringLiteral("/word/document.xml")).count("<w:tc>"));
        QCOMPARE(p1.part(QStringLiteral("/word/document.xml")).count("<w:numId"), p2.part(QStringLiteral("/word/document.xml")).count("<w:numId"));
    }

    void theMacroNoticeNamesTheFormatSavedTo()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body><w:p><w:r><w:t>with macros</w:t></w:r></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml"));
        w.addPart(QStringLiteral("/word/vbaProject.bin"), QByteArray("VBA-project"), QStringLiteral("application/vnd.ms-office.vbaProject"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdVba"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject"), QStringLiteral("/word/vbaProject.bin"));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("macros.docm")), w.finish());
        WriteWindow win;
        QStringList said;
        win.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(win.openFile(path));
        QVERIFY(win.document()->hadMacros());
        QVERIFY(win.saveTo(dir.filePath(QStringLiteral("plain.odt"))));
        QVERIFY(!said.isEmpty());
        QVERIFY2(said[0].contains(QLatin1String("macros")) && said[0].contains(QLatin1String(".odt")) && !said[0].contains(QLatin1String(".docx")), qPrintable(said[0]));
    }

    void saveOnAFlatOdtExplainsAndOffersAnOdt()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // (The file is read by its content; its name says it is a flat one.)
        const QString path = writeFile(dir.filePath(QStringLiteral("letter.fodt")), makeOdt(kBody, kAuto));
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(w.openFile(path));
        QString suggested, filter;
        w.setSavePathChooser([&](const QString &s, const QString &f) {
            suggested = s;
            filter = f;
            return dir.filePath(QStringLiteral("copy.odt"));
        });
        QVERIFY(w.save());
        QVERIFY2(!said.isEmpty() && said[0].contains(QLatin1String(".fodt")) && said[0].contains(QLatin1String(".odt file")), qPrintable(said.join(QLatin1Char('|'))));
        QVERIFY2(suggested.endsWith(QLatin1String("letter.odt")), qPrintable(suggested));
        QVERIFY2(filter.contains(QLatin1String("*.odt")), qPrintable(filter));
        QVERIFY(w.filePath().endsWith(QLatin1String("copy.odt")));
    }

    void theNotKeptNoteComesAgainForWhatIsNew()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        office::PackageWriter p;
        p.addPart(QStringLiteral("/word/document.xml"),
                  "<?xml version=\"1.0\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><w:body>"
                  "<w:p><w:r><w:t>first</w:t></w:r><w:r><w:object/></w:r></w:p><w:p><w:r><w:t>second</w:t></w:r><m:oMath><m:r><m:t>x</m:t></m:r></m:oMath></w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        p.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = writeFile(dir.filePath(QStringLiteral("two.docx")), p.finish());
        WriteWindow w;
        QStringList said;
        w.setNotifier([&](const QString &, const QString &text) { said << text; });
        QVERIFY(w.openFile(path));
        // The equation goes first (so the first save has only the object to say), and comes back.
        QTextCursor c(w.document()->story());
        c.setPosition(w.document()->story()->findBlockByNumber(1).position() + 6);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QVERIFY(c.hasSelection());
        removeSelection(*w.document(), c);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("one.odt"))));
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QLatin1String("embedded objects")) && !said[0].contains(QLatin1String("equations")), qPrintable(said[0]));
        QVERIFY(w.document()->undo());
        said.clear();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("two.odt"))));
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QLatin1String("equations")) && !said[0].contains(QLatin1String("embedded objects")), qPrintable(said[0]));
        said.clear();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("three.odt"))));
        QVERIFY(said.isEmpty());
    }

    // Not a test of its own: the work of a process that the next function starts.
    void savesAnOdtForTheChecks()
    {
        QString out;
        if (!stable::childOutput(&out)) return;
        auto doc = WriteDoc::fromFile(makeOdt(kBody, kAuto));
        QVERIFY(doc);
        const QByteArray odt = doc->toOdt();
        QVERIFY(!odt.isEmpty());
        QFile f(out);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QCOMPARE(f.write(odt), odt.size());
    }

    // The file is the same bytes each time it is saved, in whatever process.
    void aSavedOdtIsTheSameBytesInEachProcess() { JP_VERIFY_SAME_SAVES("savesAnOdtForTheChecks"); }
};

QTEST_MAIN(WriteOdtTests)
#include "tst_write_odt.moc"
