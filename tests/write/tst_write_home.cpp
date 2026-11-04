// Tests of JeffWrite's Home and Review tab commands that work on text and settings (apps/write/core/homeops.*, proofops.*, and the window's actions).
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/writedialogs.h"
#include "app/writewindow.h"
#include "app/pageview.h"
#include "core/appinfo.h"
#include "core/accessops.h"
#include "core/compareops.h"
#include "core/editops.h"
#include "core/pictureops.h"
#include "core/reviewops.h"
#include "core/tableops.h"
#include "core/fonts.h"
#include "core/homeops.h"
#include "core/proofops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textprops.h"

#include <QAction>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QTimer>
#include <QListWidget>
#include <QBuffer>
#include <QImage>
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QCryptographicHash>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

QByteArray makeDocx(const QByteArray &body, const QByteArray &settings = QByteArray())
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    if (!settings.isEmpty()) {
        w.addPart(QStringLiteral("/word/settings.xml"), xml(QByteArray("<w:settings ") + kNs + ">" + settings + "</w:settings>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    }
    return w.finish();
}

QByteArray part(const QByteArray &zip, const QString &name)
{
    office::Package p;
    p.read(zip);
    return p.part(name);
}

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

QByteArray run(const char *text, const QByteArray &props = QByteArray())
{
    return QByteArray("<w:r>") + (props.isEmpty() ? QByteArray() : "<w:rPr>" + props + "</w:rPr>") + "<w:t xml:space=\"preserve\">" + text + "</w:t></w:r>";
}
QByteArray para(const QByteArray &content, const QByteArray &props = QByteArray()) { return "<w:p>" + (props.isEmpty() ? QByteArray() : "<w:pPr>" + props + "</w:pPr>") + content + "</w:p>"; }

QStringList texts(const WriteDoc &doc)
{
    QStringList out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) out << b.text();
    return out;
}

QTextCursor selectBlocks(QTextDocument *d, int firstBlock, int lastBlock)
{
    QTextCursor c(d);
    c.setPosition(d->findBlockByNumber(firstBlock).position());
    QTextBlock last = d->findBlockByNumber(lastBlock);
    c.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
    return c;
}

// Runs `open` (which opens a modal dialog named `objectName`), fills the dialog in, and accepts it (or rejects it).
void withDialog(const std::function<void()> &open, const QString &objectName, const std::function<void(QDialog *)> &fill, bool accept = true)
{
    QString opened;
    QTimer once, watchdog;
    once.setSingleShot(true);
    watchdog.setSingleShot(true);
    auto closeAll = [](bool ok) {
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *d = qobject_cast<QDialog *>(top); d && d->isVisible()) {
                if (ok) d->accept();
                else d->reject();
            }
    };
    QObject::connect(&once, &QTimer::timeout, [&] {
        auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (dlg) opened = dlg->objectName();
        if (dlg && opened == objectName) fill(dlg);
        closeAll(accept && opened == objectName);
    });
    QObject::connect(&watchdog, &QTimer::timeout, [&] { closeAll(false); });
    once.start(0);
    watchdog.start(10000);
    open();
    QCOMPARE(opened, objectName);
}

} // namespace

class HomeTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    void caseOfWritesEachCase()
    {
        QCOMPARE(caseOf(QStringLiteral("hELLO wORLD. tHIS is it"), CaseMode::Sentence), QStringLiteral("Hello world. This is it"));
        QCOMPARE(caseOf(QStringLiteral("Hello World"), CaseMode::Lower), QStringLiteral("hello world"));
        QCOMPARE(caseOf(QStringLiteral("Hello World"), CaseMode::Upper), QStringLiteral("HELLO WORLD"));
        QCOMPARE(caseOf(QStringLiteral("the cat's hat"), CaseMode::Capitalize), QStringLiteral("The Cat's Hat"));
        QCOMPARE(caseOf(QStringLiteral("Hello World"), CaseMode::Toggle), QStringLiteral("hELLO wORLD"));
        // The middle of a sentence or a word stays in the middle.
        QCOMPARE(caseOf(QStringLiteral("ello"), CaseMode::Sentence, false), QStringLiteral("ello"));
        QCOMPARE(caseOf(QStringLiteral("ello"), CaseMode::Capitalize, true, false), QStringLiteral("ello"));
        // A letter that would become two stays one.
        QCOMPARE(caseOf(QString::fromUtf8("stra\xC3\x9F" "e"), CaseMode::Upper), QString::fromUtf8("STRA\xC3\x9F" "E"));
    }

    void changeCaseKeepsTheLookAndIsOneStep()
    {
        auto doc = open(makeDocx(para(run("hello ", "<w:b/>") + run("wORLD", "<w:i/>"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QCOMPARE(changeCase(*doc, c, CaseMode::Capitalize), 6);
        QCOMPARE(texts(*doc).first(), QStringLiteral("Hello World"));
        QCOMPARE(c.selectedText(), QStringLiteral("Hello World"));
        QTextCursor at(doc->story());
        at.setPosition(1);
        QVERIFY(at.charFormat().fontWeight() >= QFont::DemiBold);
        at.setPosition(9);
        QVERIFY(at.charFormat().fontItalic());
        doc->story()->undo();
        QCOMPARE(texts(*doc).first(), QStringLiteral("hello wORLD"));
    }

    void changeCaseWithoutASelectionChangesTheWord()
    {
        auto doc = open(makeDocx(para(run("one two three"))));
        QTextCursor c(doc->story());
        c.setPosition(5);
        QCOMPARE(changeCase(*doc, c, CaseMode::Upper), 3);
        QCOMPARE(texts(*doc).first(), QStringLiteral("one TWO three"));
        QVERIFY(!c.hasSelection());
    }

    void changeCaseUnderTrackChangesIsATrackedReplacement()
    {
        auto doc = open(makeDocx(para(run("hello"))));
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QCOMPARE(changeCase(*doc, c, CaseMode::Upper), 5);
        const QByteArray out = part(doc->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY(out.contains("<w:del "));
        QVERIFY(out.contains("<w:ins "));
        QVERIFY(out.contains("HELLO"));
        QVERIFY(out.contains("hello"));
    }

    void shiftF3CyclesThroughTheCases()
    {
        QCOMPARE(nextCaseInCycle(QStringLiteral("hello there")), CaseMode::Capitalize);
        QCOMPARE(nextCaseInCycle(QStringLiteral("Hello There")), CaseMode::Upper);
        QCOMPARE(nextCaseInCycle(QStringLiteral("HELLO THERE")), CaseMode::Lower);
        QCOMPARE(nextCaseInCycle(QStringLiteral("Hello there")), CaseMode::Capitalize);
    }

    void sortOrdersParagraphsAndKeepsEachPlacesLook()
    {
        auto doc = open(makeDocx(para(run("pear")) + para(run("Apple", "<w:b/>")) + para(run("fig")) + para(run("tail"))));
        QTextCursor c = selectBlocks(doc->story(), 0, 2);
        SortOptions o;
        o.keys = {SortKey{}};
        QCOMPARE(sortParagraphs(*doc, c, o), 3);
        QCOMPARE(texts(*doc), (QStringList{"Apple", "fig", "pear", "tail"}));
        // The bold goes with its word.
        QTextCursor at(doc->story());
        at.setPosition(1);
        QVERIFY(at.charFormat().fontWeight() >= QFont::DemiBold);
        at.setPosition(doc->story()->findBlockByNumber(1).position() + 1);
        QVERIFY(at.charFormat().fontWeight() < QFont::DemiBold);
        doc->story()->undo();
        QCOMPARE(texts(*doc), (QStringList{"pear", "Apple", "fig", "tail"}));
    }

    void sortByNumberAndFieldAndHeader()
    {
        auto doc = open(makeDocx(para(run("name,qty")) + para(run("a,10")) + para(run("b,9")) + para(run("c,100"))));
        QTextCursor c = selectBlocks(doc->story(), 0, 3);
        SortOptions o;
        SortKey k;
        k.field = 2;
        k.type = SortKey::Number;
        o.keys = {k};
        o.separator = QLatin1Char(',');
        o.header = true;
        QCOMPARE(sortParagraphs(*doc, c, o), 3);
        QCOMPARE(texts(*doc), (QStringList{"name,qty", "b,9", "a,10", "c,100"}));
        o.keys[0].descending = true;
        QCOMPARE(sortParagraphs(*doc, c, o), 3);
        QCOMPARE(texts(*doc), (QStringList{"name,qty", "c,100", "a,10", "b,9"}));
    }

    void sortByDateAndSecondKey()
    {
        auto doc = open(makeDocx(para(run("x 3/1/2020")) + para(run("y 1/15/2020")) + para(run("z 1/15/2020"))));
        QTextCursor c = selectBlocks(doc->story(), 0, 2);
        SortOptions o;
        SortKey d;
        d.type = SortKey::Date;
        d.field = 2;
        SortKey t;
        t.descending = true;
        o.keys = {d, t};
        o.separator = QLatin1Char(' ');
        QCOMPARE(sortParagraphs(*doc, c, o), 3);
        QCOMPARE(texts(*doc), (QStringList{"z 1/15/2020", "y 1/15/2020", "x 3/1/2020"}));
    }

    void sortRefusesWhileTrackingAndForOneParagraph()
    {
        auto doc = open(makeDocx(para(run("b")) + para(run("a"))));
        QTextCursor c = selectBlocks(doc->story(), 0, 1);
        SortOptions o;
        o.keys = {SortKey{}};
        doc->setTrackChanges(true);
        QCOMPARE(sortParagraphs(*doc, c, o), 0);
        doc->setTrackChanges(false);
        QTextCursor one = selectBlocks(doc->story(), 0, 0);
        QCOMPARE(sortParagraphs(*doc, one, o), 0);
        QCOMPARE(texts(*doc), (QStringList{"b", "a"}));
    }

    void wordCountCountsWordsCharactersAndParagraphs()
    {
        auto doc = open(makeDocx(para(run("Two words")) + para(run("")) + para(run("and ") + run("hidden", "<w:vanish/>") + run(" here"))));
        QTextCursor none(doc->story());
        const WordStats s = countWords(*doc, none, true);
        QCOMPARE(s.words, 4);
        QCOMPARE(s.paragraphs, 2);
        QCOMPARE(s.charactersNoSpaces, 8 + 3 + 4);
        QCOMPARE(s.charactersWithSpaces, 9 + 4 + 5);
        QTextCursor sel(doc->story());
        sel.setPosition(0);
        sel.setPosition(3, QTextCursor::KeepAnchor);
        const WordStats part = countWords(*doc, sel, true);
        QCOMPARE(part.words, 1);
        QCOMPARE(part.charactersNoSpaces, 3);
    }

    void wordCountIncludesFootnotesOnlyWhenAsked()
    {
        QByteArray body = para(run("Body text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QTextCursor none(doc->story());
        QCOMPARE(countWords(*doc, none, false).words, 2);
        QVERIFY(countWords(*doc, none, true).words >= 2);
    }

    void pasteMergedKeepsOnlyTheEmphasis()
    {
        auto doc = open(makeDocx(para(run("Say: ", "<w:i/>"))));
        QTextDocument src;
        QTextCursor sc(&src);
        QTextCharFormat plain, boldRed;
        boldRed.setFontWeight(QFont::Bold);
        boldRed.setForeground(Qt::red);
        boldRed.setFontPointSize(30);
        plain.setFontPointSize(30);
        sc.insertText("big ", plain);
        sc.insertText("bold", boldRed);
        QTextCursor c(doc->story());
        c.movePosition(QTextCursor::End);
        pasteMerged(*doc, c, QTextDocumentFragment(&src));
        QCOMPARE(texts(*doc).first(), QStringLiteral("Say: big bold"));
        QTextCursor at(doc->story());
        at.setPosition(8);   // inside "big "
        QVERIFY(at.charFormat().fontWeight() < QFont::DemiBold);
        at.setPosition(11);   // inside "bold"
        QVERIFY(at.charFormat().fontWeight() >= QFont::DemiBold);
        QVERIFY(at.charFormat().fontPointSize() < 20);   // the size is the place's own
        QVERIFY(!at.charFormat().hasProperty(QTextFormat::ForegroundBrush) || at.charFormat().foreground().color() != QColor(Qt::red));
        doc->story()->undo();
        QCOMPARE(texts(*doc).first(), QStringLiteral("Say: "));
    }

    void theProofingLanguageIsSetReadAndWrittenAsWLang()
    {
        auto doc = open(makeDocx(para(run("colour ", "<w:i/>") + run("color", "<w:lang w:val=\"en-US\" w:eastAsia=\"ja-JP\"/>"))));
        QTextCursor c(doc->story());
        QCOMPARE(proofingLanguageAt(*doc, c), QStringLiteral("en-US"));
        c.setPosition(0);
        c.setPosition(6, QTextCursor::KeepAnchor);
        setProofingLanguage(*doc, c, QStringLiteral("en-GB"));
        QCOMPARE(proofingLanguageAt(*doc, c), QStringLiteral("en-GB"));
        // The second run keeps its other attributes and takes the new language.
        QTextCursor d(doc->story());
        d.setPosition(8);
        d.setPosition(11, QTextCursor::KeepAnchor);
        setProofingLanguage(*doc, d, QStringLiteral("fr-FR"));
        const QByteArray out = part(doc->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(out.contains("<w:i/><w:lang w:val=\"en-GB\"/>"), out.constData());
        QVERIFY2(out.contains("w:val=\"fr-FR\"") && out.contains("w:eastAsia=\"ja-JP\""), out.constData());
        // Back to the document's.
        setProofingLanguage(*doc, c, QString());
        QVERIFY(!part(doc->toDocx(), QStringLiteral("/word/document.xml")).contains("en-GB"));
        // The file reads again with the language.
        auto again = open(doc->toDocx());
        QTextCursor r(again->story());
        r.setPosition(9);
        QCOMPARE(proofingLanguageAt(*again, r), QStringLiteral("fr-FR"));
    }

    void doNotCheckSpellingIsAMarkOnTheText()
    {
        auto doc = open(makeDocx(para(run("xyzzy plugh"))));
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.setPosition(5, QTextCursor::KeepAnchor);
        QVERIFY(!noProofAt(c));
        setNoProof(*doc, c, true);
        QVERIFY(noProofAt(c));
        QVERIFY(part(doc->toDocx(), QStringLiteral("/word/document.xml")).contains("<w:noProof/>"));
        setNoProof(*doc, c, false);
        QVERIFY(!part(doc->toDocx(), QStringLiteral("/word/document.xml")).contains("noProof"));
    }

    void translateOpensOnlyAWebAddressWithTheTextInIt()
    {
        const QString url = translateUrl(QStringLiteral("https://translate.example.org/app"), QStringLiteral("fr"), QStringLiteral("Hello & goodbye"));
        QVERIFY2(url.startsWith(QLatin1String("https://translate.example.org/app?")), qPrintable(url));
        QVERIFY(url.contains(QLatin1String("target=fr")));
        QVERIFY(url.contains(QLatin1String("source=auto")));
        QVERIFY(url.contains(QLatin1String("Hello%20%26%20goodbye")) || url.contains(QLatin1String("Hello+%26+goodbye")));
        QVERIFY(translateUrl(QString(), QStringLiteral("de"), QStringLiteral("x")).startsWith(QLatin1String("https://libretranslate.com/")));
        QVERIFY(translateUrl(QStringLiteral("file:///etc/passwd"), QStringLiteral("de"), QStringLiteral("x")).isEmpty());
        QVERIFY(translateUrl(QStringLiteral("javascript:alert(1)"), QStringLiteral("de"), QStringLiteral("x")).isEmpty());
        // A long text is cut at the service's limit.
        const QString longUrl = translateUrl(QString(), QStringLiteral("de"), QString(9000, QLatin1Char('a')));
        QVERIFY(longUrl.count(QLatin1Char('a')) <= 5000 + 40);
        QVERIFY(longUrl.size() < 5300);
    }

    void protectionIsReadWrittenAndKeptAsTheFileHadIt()
    {
        namespace wml = jp::office::wml;
        const QByteArray body = para(run("x"));
        const QByteArray settings = "<w:zoom w:percent=\"100\"/><w:defaultTabStop w:val=\"720\"/>";
        auto doc = open(makeDocx(body, settings));
        QVERIFY(!doc->protection());
        doc->setProtection(wml::makeDocumentProtection(QStringLiteral("trackedChanges"), QStringLiteral("secret")));
        QVERIFY(doc->isModified());
        const QByteArray saved = part(doc->toDocx(), QStringLiteral("/word/settings.xml"));
        // In the schema's place: after the zoom, before the default tab stop.
        QVERIFY2(saved.indexOf("<w:zoom") < saved.indexOf("<w:documentProtection") && saved.indexOf("<w:documentProtection") < saved.indexOf("<w:defaultTabStop"), saved.constData());
        QVERIFY(saved.contains("w:edit=\"trackedChanges\"") && saved.contains("w:enforcement=\"1\"") && saved.contains("w:algorithmName=\"SHA-512\""));
        QVERIFY(!saved.contains("secret"));
        auto again = open(doc->toDocx());
        QVERIFY(again->protection());
        QCOMPARE(again->protection()->edit, QStringLiteral("trackedChanges"));
        QVERIFY(again->protection()->enforced && again->protection()->hasPassword());
        QVERIFY(wml::documentProtectionAccepts(*again->protection(), QStringLiteral("secret")));
        QVERIFY(!wml::documentProtectionAccepts(*again->protection(), QStringLiteral("Secret")));
        QVERIFY(!wml::documentProtectionAccepts(*again->protection(), QString()));
        // Saved again without a change: the element is as it was (its salt and hash stay).
        QCOMPARE(part(again->toDocx(), QStringLiteral("/word/settings.xml")).count("<w:documentProtection"), 1);
        again->setProtection(std::nullopt);
        QVERIFY(!part(again->toDocx(), QStringLiteral("/word/settings.xml")).contains("documentProtection"));
    }

    void aFileWithoutSettingsGetsAPartForTheProtection()
    {
        namespace wml = jp::office::wml;
        auto doc = open(makeDocx(para(run("x"))));
        doc->setProtection(wml::makeDocumentProtection(QStringLiteral("readOnly"), QString()));
        const QByteArray saved = part(doc->toDocx(), QStringLiteral("/word/settings.xml"));
        QVERIFY2(saved.contains("<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\"/>"), saved.constData());
        QVERIFY(wml::documentProtectionAccepts(*open(doc->toDocx())->protection(), QStringLiteral("anything")));
    }

    void aPasswordHashFromAnotherProgramIsCheckedByItsOwnAlgorithm()
    {
        namespace wml = jp::office::wml;
        // SHA-512, salt "salt", 2 rounds, for the password "pw": made here by the algorithm of the file format (the salt and the UTF-16 password hashed, then the hash and a counter).
        QByteArray utf16("p\0w\0", 4);
        QByteArray h = QCryptographicHash::hash(QByteArray("salt") + utf16, QCryptographicHash::Sha512);
        for (quint32 i = 0; i < 2; ++i) {
            QByteArray counter;
            for (int b = 0; b < 4; ++b) counter.append(char((i >> (8 * b)) & 0xFF));
            h = QCryptographicHash::hash(h + counter, QCryptographicHash::Sha512);
        }
        wml::DocumentProtection p;
        p.edit = QStringLiteral("readOnly");
        p.enforced = true;
        p.xml = "<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\" w:algorithmName=\"SHA-512\" w:hashValue=\"" + h.toBase64() + "\" w:saltValue=\"" + QByteArray("salt").toBase64() +
                "\" w:spinCount=\"2\"/>";
        QVERIFY(wml::documentProtectionAccepts(p, QStringLiteral("pw")));
        QVERIFY(!wml::documentProtectionAccepts(p, QStringLiteral("px")));
        // An algorithm that is not known never matches (the protection can't be stopped from here).
        p.xml.replace("SHA-512", "MD5");
        QVERIFY(!wml::documentProtectionAccepts(p, QStringLiteral("pw")));
    }

    void aDocumentProtectedAsReadOnlyIsHeldReadOnlyUntilTheProtectionStops()
    {
        namespace wml = jp::office::wml;
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        auto made = open(makeDocx(para(run("hello")), "<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\"/>"));
        w.setDocument(std::move(made), QString());
        QVERIFY(w.isReadOnly());
        QVERIFY2(w.windowTitle().contains(QLatin1String("Protected")), qPrintable(w.windowTitle()));
        QTextCursor c(w.document()->story());
        c.setPosition(0);
        c.setPosition(5, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        w.act(QStringLiteral("fmt.bold"))->trigger();
        QVERIFY(w.document()->story()->toPlainText() == QStringLiteral("hello"));
        QVERIFY(!w.document()->canUndo());   // (the command was refused)
        w.document()->setProtection(std::nullopt);
        QVERIFY(!w.isReadOnly());
        QVERIFY(!w.windowTitle().contains(QLatin1String("Protected")));
        w.act(QStringLiteral("fmt.bold"))->trigger();
        QVERIFY(w.document()->canUndo());
    }

    void lockedTrackingKeepsTrackChangesOnAndTheChangesUnresolved()
    {
        namespace wml = jp::office::wml;
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("hello")), "<w:documentProtection w:edit=\"trackedChanges\" w:enforcement=\"1\"/>")), QString());
        QVERIFY(!w.isReadOnly());
        QVERIFY(w.document()->trackChanges());
        QVERIFY(!w.act(QStringLiteral("review.trackChanges"))->isEnabled());
        QVERIFY(!w.act(QStringLiteral("review.acceptAll"))->isEnabled());
        w.document()->setProtection(std::nullopt);
        QVERIFY(w.act(QStringLiteral("review.trackChanges"))->isEnabled());
    }

    void aDocumentProtectedForCommentsStaysReadOnlyButTakesComments()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("hello")), "<w:documentProtection w:edit=\"comments\" w:enforcement=\"1\"/>")), QString());
        QVERIFY(w.isReadOnly());
        QTextCursor c(w.document()->story());
        c.setPosition(0);
        c.setPosition(5, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        w.act(QStringLiteral("fmt.bold"))->trigger();
        QVERIFY(!w.document()->canUndo());
    }

    void diffTextTellsWordsApartAndRebuildsTheNewText()
    {
        CompareOptions o;
        const QString from = QStringLiteral("The quick brown fox jumps over the lazy dog."), to = QStringLiteral("The quick red fox leaps over the very lazy dog.");
        const QVector<TextEdit> edits = diffText(from, to, o);
        QVERIFY(edits.size() >= 2);
        // Applying the edits from the end gives the new text.
        QString rebuilt = from;
        for (int i = int(edits.size()) - 1; i >= 0; --i) rebuilt.replace(edits.at(i).deleteFrom, edits.at(i).deleteTo - edits.at(i).deleteFrom, edits.at(i).insertText);
        QCOMPARE(rebuilt, to);
        QVERIFY(diffText(from, from, o).isEmpty());
        o.ignoreCase = true;
        QVERIFY(diffText(QStringLiteral("Hello World"), QStringLiteral("hello world"), o).isEmpty());
    }

    void compareMakesTrackedChangesThatAcceptToTheRevisedAndRejectToTheOriginal()
    {
        const QByteArray one = para(run("Title")) + para(run("The quick brown fox.")) + para(run("Removed paragraph.")) + para(run("Same ending."));
        const QByteArray two = para(run("Title")) + para(run("The quick red fox.")) + para(run("New paragraph here.")) + para(run("Another new one.")) + para(run("Same ending.")) + para(run("Added last."));
        auto original = open(makeDocx(one)), revised = open(makeDocx(two));
        CompareOptions o;
        o.author = QStringLiteral("Reviewer");
        int changes = 0;
        auto result = compareDocuments(*original, *revised, o, &changes);
        QVERIFY(result);
        QVERIFY(changes >= 4);
        QVERIFY(revisionCount(result->story()) > 0);
        // The result is a document of tracked changes, signed with the author.
        const QByteArray out = part(result->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY(out.contains("w:author=\"Reviewer\""));
        QVERIFY(out.contains("<w:ins ") && out.contains("<w:del "));
        // The original is as it was.
        QCOMPARE(texts(*original), (QStringList{"Title", "The quick brown fox.", "Removed paragraph.", "Same ending."}));
        auto reject = open(result->toDocx());
        rejectAll(*reject);
        QCOMPARE(texts(*reject), texts(*original));
        auto accept = open(result->toDocx());
        acceptAll(*accept);
        QCOMPARE(texts(*accept), texts(*revised));
    }

    void compareOfEqualDocumentsHasNoChangesAndOfALastParagraphDeletedLeavesNoEmptyOne()
    {
        auto a = open(makeDocx(para(run("One")) + para(run("Two")))), b = open(makeDocx(para(run("One")) + para(run("Two"))));
        int changes = -1;
        auto same = compareDocuments(*a, *b, CompareOptions(), &changes);
        QVERIFY(same);
        QCOMPARE(changes, 0);
        QCOMPARE(revisionCount(same->story()), 0);
        auto shorter = open(makeDocx(para(run("One"))));
        auto cut = compareDocuments(*a, *shorter, CompareOptions(), &changes);
        QVERIFY(cut);
        acceptAll(*cut);
        QCOMPARE(texts(*cut), (QStringList{"One"}));
    }

    void compareHandlesDocumentsWithManyParagraphsAndEmptyOnes()
    {
        QByteArray one, two;
        for (int i = 0; i < 300; ++i) {
            one += para(run(QByteArray("Paragraph number ") + QByteArray::number(i)));
            two += para(run(QByteArray("Paragraph number ") + QByteArray::number(i % 7 == 0 ? i + 1000 : i)));
        }
        auto a = open(makeDocx(one)), b = open(makeDocx(two));
        auto r = compareDocuments(*a, *b, CompareOptions(), nullptr);
        QVERIFY(r);
        auto accept = open(r->toDocx());
        acceptAll(*accept);
        QCOMPARE(texts(*accept), texts(*b));
        auto empty = open(makeDocx(para(QByteArray())));
        auto r2 = compareDocuments(*empty, *b, CompareOptions(), nullptr);
        QVERIFY(r2);
        acceptAll(*r2);
        QVERIFY(r2->story()->toPlainText().contains(QStringLiteral("Paragraph number 1000")));
    }

    void theCompareCommandOpensTheResultInAWindowOfItsOwn()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile f(dir.filePath(QStringLiteral("revised.docx")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(makeDocx(para(run("Alpha")) + para(run("Beta changed"))));
        f.close();
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("Alpha")) + para(run("Beta")))), QString());
        const int before = QApplication::topLevelWidgets().size();
        CompareOptions o;
        o.author = QStringLiteral("Me");
        QVERIFY(w.compareWith(dir.filePath(QStringLiteral("revised.docx")), o) > 0);
        WriteWindow *made = nullptr;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *ww = qobject_cast<WriteWindow *>(top); ww && ww != &w) made = ww;
        QVERIFY(made);
        QVERIFY(QApplication::topLevelWidgets().size() > before);
        QVERIFY(revisionCount(made->document()->story()) > 0);
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Alpha\nBeta"));   // (the original is as it was)
        QVERIFY(w.compareWith(dir.filePath(QStringLiteral("missing.docx")), o) < 0);
        made->document()->setModified(false);
        made->close();
    }

    void sortInATableSortsItsRows()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("x")))), QString());
        NewTable t;
        t.rows = 3;
        t.columns = 2;
        QTextCursor c(w.document()->story());
        c.setPosition(0);
        insertTable(*w.document(), c, t);
        w.view()->refresh();
        QStringList cells;
        for (QTextBlock b = w.document()->story()->begin(); b.isValid(); b = b.next())
            if (inTable(b)) cells << b.text();
        QCOMPARE(cells.size(), 6);
        // Fill the cells: b 2 / a 1 / c 3.
        const QStringList words = {"b", "2", "a", "1", "c", "3"};
        int i = 0;
        for (QTextBlock b = w.document()->story()->begin(); b.isValid() && i < 6; b = b.next()) {
            if (!inTable(b)) continue;
            QTextCursor t2(w.document()->story());
            t2.setPosition(b.position());
            insertText(*w.document(), t2, words.at(i++));
        }
        QTextCursor in(w.document()->story());
        for (QTextBlock b = w.document()->story()->begin(); b.isValid(); b = b.next())
            if (inTable(b)) {
                in.setPosition(b.position());
                break;
            }
        w.view()->setCursor(in);
        SortOptions o;
        o.keys = {SortKey{}};
        QCOMPARE(w.sortSelection(o), 1);
        QStringList after;
        for (QTextBlock b = w.document()->story()->begin(); b.isValid(); b = b.next())
            if (inTable(b)) after << b.text();
        QCOMPARE(after, (QStringList{"a", "1", "b", "2", "c", "3"}));
    }

    void contrastIsWorkedOutAsTheGuidelinesDo()
    {
        QVERIFY(qAbs(contrastRatio(Qt::black, Qt::white) - 21.0) < 0.01);
        QVERIFY(qAbs(contrastRatio(Qt::white, Qt::white) - 1.0) < 0.001);
        QVERIFY(contrastRatio(QColor(0x77, 0x77, 0x77), Qt::white) > 4.4 && contrastRatio(QColor(0x77, 0x77, 0x77), Qt::white) < 4.6);
    }

    void theAccessibilityCheckFindsAPictureWithoutAltTextAndStopsFindingItOnceDescribed()
    {
        auto doc = open(makeDocx(para(run("Before after"))));
        QImage image(40, 20, QImage::Format_ARGB32);
        image.fill(Qt::blue);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, png, QStringLiteral("blue")), PictureResult::Ok);
        QVector<AccessIssue> issues = checkAccessibility(*doc);
        QCOMPARE(issues.size(), 1);
        QCOMPARE(int(issues.first().kind), int(AccessIssue::MissingAltText));
        QCOMPARE(issues.first().position, 7);
        QVERIFY(setPictureAltText(*doc, 7, QStringLiteral("A blue box")));
        QVERIFY(checkAccessibility(*doc).isEmpty());
        QVERIFY(setPictureWrap(*doc, 7, PictureWrap::Square));
        issues = checkAccessibility(*doc);
        QCOMPARE(issues.size(), 1);
        QCOMPARE(int(issues.first().kind), int(AccessIssue::FloatingPicture));
    }

    void theAccessibilityCheckFindsHeadingsOutOfOrderAndTextThatIsHardToRead()
    {
        const QByteArray styles = "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/></w:style>"
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Heading3\"><w:name w:val=\"heading 3\"/></w:style></w:styles>";
        Q_UNUSED(styles);
        auto doc = open(makeDocx(para(run("One"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Three"), "<w:pStyle w:val=\"Heading3\"/>") + para(run("pale yellow words", "<w:color w:val=\"FFFF99\"/>")) +
                                 para(run("dark words", "<w:color w:val=\"000000\"/>"))));
        const QVector<AccessIssue> issues = checkAccessibility(*doc);
        int order = 0, contrast = 0;
        for (const AccessIssue &i : issues) {
            order += i.kind == AccessIssue::HeadingOrder;
            contrast += i.kind == AccessIssue::LowContrast;
        }
        QCOMPARE(order, 1);
        QCOMPARE(contrast, 1);
    }

    void theAccessibilityCheckFindsATableWithoutAHeaderRowAndBlankCells()
    {
        auto doc = open(makeDocx(para(run("x"))));
        NewTable t;
        t.rows = 2;
        t.columns = 2;
        QTextCursor c(doc->story());
        c.setPosition(0);
        insertTable(*doc, c, t);
        QVector<AccessIssue> issues = checkAccessibility(*doc);
        int header = 0, blank = 0;
        for (const AccessIssue &i : issues) {
            header += i.kind == AccessIssue::TableWithoutHeader;
            blank += i.kind == AccessIssue::BlankTableCell;
        }
        QCOMPARE(header, 1);
        QCOMPARE(blank, 4);
    }

    // With JW_SHOTS set to a folder, saves a picture of each dialog there (for looking at them); skipped otherwise.
    void picturesOfTheDialogs()
    {
        const QString dir = qEnvironmentVariable("JW_SHOTS");
        if (dir.isEmpty()) QSKIP("JW_SHOTS is not set");
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 700);
        w.show();
        w.setDocument(open(makeDocx(para(run("pear tree")) + para(run("Apple pie")) + para(run("cherry")))), QString());
        w.view()->selectAll();
        auto shot = [&](const QString &command, const QString &name, const QString &file) {
            withDialog([&] { w.act(command)->trigger(); }, name, [&](QDialog *d) { d->grab().save(dir + QLatin1Char('/') + file); }, false);
        };
        shot(QStringLiteral("fmt.fontDialog"), QStringLiteral("fontDialog"), QStringLiteral("dialog-font.png"));
        shot(QStringLiteral("para.dialog"), QStringLiteral("paragraphDialog"), QStringLiteral("dialog-paragraph.png"));
        shot(QStringLiteral("para.sort"), QStringLiteral("sortDialog"), QStringLiteral("dialog-sort.png"));
        shot(QStringLiteral("review.wordCount"), QStringLiteral("wordCountDialog"), QStringLiteral("dialog-wordcount.png"));
        shot(QStringLiteral("review.restrictEditing"), QStringLiteral("restrictEditingDialog"), QStringLiteral("dialog-restrict.png"));
        shot(QStringLiteral("review.language"), QStringLiteral("languageDialog"), QStringLiteral("dialog-language.png"));
        shot(QStringLiteral("review.compare"), QStringLiteral("compareDialog"), QStringLiteral("dialog-compare.png"));
        shot(QStringLiteral("review.translate"), QStringLiteral("translateDialog"), QStringLiteral("dialog-translate.png"));
        QTextCursor cursor = w.view()->cursor();
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [&](QDialog *d) { d->grab().save(dir + QStringLiteral("/dialog-tabs.png")); });
        w.act(QStringLiteral("review.accessibility"))->trigger();
        w.grab().save(dir + QStringLiteral("/window-accessibility.png"));
    }

    void theFontBoxLeftAloneChangesNothing()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("plain words")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *) {});
        QVERIFY(!w.document()->canUndo());
    }

    void theFontBoxSetsCharacterSpacingPositionAndKerningAndTheFileKeepsThem()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("spaced out")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("spacingKind"))->setCurrentIndex(1);   // expanded
            d->findChild<QDoubleSpinBox *>(QStringLiteral("spacingBy"))->setValue(1.5);
            d->findChild<QComboBox *>(QStringLiteral("positionKind"))->setCurrentIndex(2);   // lowered
            d->findChild<QDoubleSpinBox *>(QStringLiteral("positionBy"))->setValue(3);
            d->findChild<QCheckBox *>(QStringLiteral("kerning"))->setChecked(true);
            d->findChild<QDoubleSpinBox *>(QStringLiteral("kerningFrom"))->setValue(14);
        });
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(out.contains("<w:spacing w:val=\"30\"/>") && out.contains("<w:position w:val=\"-6\"/>") && out.contains("<w:kern w:val=\"28\"/>"), out.constData());
        // The box reads them back, and leaving them alone changes nothing.
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("spacingKind"))->currentIndex(), 1);
            QCOMPARE(d->findChild<QDoubleSpinBox *>(QStringLiteral("spacingBy"))->value(), 1.5);
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("positionKind"))->currentIndex(), 2);
            QVERIFY(d->findChild<QCheckBox *>(QStringLiteral("kerning"))->isChecked());
        });
        QCOMPARE(part(w.document()->toDocx(), QStringLiteral("/word/document.xml")), out);   // (left alone: the file is as it was)
        // Normal again.
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("spacingKind"))->setCurrentIndex(0);
            d->findChild<QComboBox *>(QStringLiteral("positionKind"))->setCurrentIndex(0);
            d->findChild<QCheckBox *>(QStringLiteral("kerning"))->setChecked(false);
        });
        const QByteArray plain = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(!plain.contains("w:spacing w:val") && !plain.contains("w:position") && !plain.contains("w:kern"), plain.constData());
    }

    void theTabsBoxSetsTabStopsWithTheirLeaders()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("a\tb")))), QString());
        w.view()->selectAll();
        QTextCursor cursor = w.view()->cursor();
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [](QDialog *d) {
            auto *position = d->findChild<QDoubleSpinBox *>(QStringLiteral("tabPosition"));
            position->setValue(72.0);
            d->findChild<QComboBox *>(QStringLiteral("tabType"))->setCurrentIndex(1);
            d->findChild<QComboBox *>(QStringLiteral("tabLeader"))->setCurrentIndex(1);
            d->findChild<QPushButton *>(QStringLiteral("tabSet"))->click();
            QCOMPARE(d->findChild<QListWidget *>(QStringLiteral("tabList"))->count(), 1);
        });
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(out.contains("w:val=\"center\"") && out.contains("w:leader=\"dot\"") && out.contains("<w:tabs>"), out.constData());
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [](QDialog *d) {
            QCOMPARE(d->findChild<QListWidget *>(QStringLiteral("tabList"))->count(), 1);
            d->findChild<QPushButton *>(QStringLiteral("tabClearAll"))->click();
        });
        QVERIFY(!part(w.document()->toDocx(), QStringLiteral("/word/document.xml")).contains("<w:tab w:val"));
    }

    void setAsDefaultGivesTheNormalStyleTheFontChosen()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("some words")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->setValue(15);
            QTimer::singleShot(100, [] {
                for (QWidget *top : QApplication::topLevelWidgets())
                    if (auto *box = qobject_cast<QMessageBox *>(top); box && box->isVisible()) box->button(QMessageBox::Yes)->click();
            });
            d->findChild<QPushButton *>(QStringLiteral("setAsDefault"))->click();
        }, false);
        const QByteArray styles = part(w.document()->toDocx(), QStringLiteral("/word/styles.xml"));
        QVERIFY2(styles.contains("<w:sz w:val=\"30\"/>"), styles.constData());
    }

    void theParagraphBoxSetsLineAndPageBreakChoicesAndTheOutlineLevel()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("a paragraph")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("para.dialog"))->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *d) {
            d->findChild<QCheckBox *>(QStringLiteral("keepWithNext"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("keepLinesTogether"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("pageBreakBefore"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("dontHyphenate"))->setChecked(true);
            d->findChild<QComboBox *>(QStringLiteral("outlineLevel"))->setCurrentIndex(2);   // level 2
        });
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        for (const char *tag : {"<w:keepNext/>", "<w:keepLines/>", "<w:pageBreakBefore/>", "<w:suppressAutoHyphens/>", "<w:outlineLvl w:val=\"1\"/>"}) QVERIFY2(out.contains(tag), tag);
        // Back off.
        withDialog([&w] { w.act(QStringLiteral("para.dialog"))->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *d) {
            QVERIFY(d->findChild<QCheckBox *>(QStringLiteral("keepWithNext"))->isChecked());
            d->findChild<QCheckBox *>(QStringLiteral("keepWithNext"))->setChecked(false);
            d->findChild<QCheckBox *>(QStringLiteral("pageBreakBefore"))->setChecked(false);
            d->findChild<QComboBox *>(QStringLiteral("outlineLevel"))->setCurrentIndex(0);
        });
        const QByteArray back = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(!back.contains("<w:keepNext/>") && !back.contains("<w:pageBreakBefore/>") && !back.contains("outlineLvl"), back.constData());
        QVERIFY(back.contains("<w:keepLines/>"));
    }
};

QTEST_MAIN(HomeTests)
#include "tst_write_home.moc"
