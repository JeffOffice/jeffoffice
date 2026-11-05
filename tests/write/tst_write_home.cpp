// Tests of JeffWrite's Home and Review tab commands that work on text and settings (apps/write/core/homeops.*, proofops.*, and the window's actions).
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/widgets.h"
#include "app/settings.h"
#include "core/fontfiles.h"
#include "app/writedialogs.h"
#include "app/writewindow.h"
#include "app/pageview.h"
#include "core/appinfo.h"
#include "core/accessops.h"
#include "core/linkops.h"
#include "core/commentops.h"
#include "core/compareops.h"
#include "core/gotoops.h"
#include "core/editops.h"
#include "core/pictureops.h"
#include "core/reviewops.h"
#include "core/tableops.h"
#include "core/fonts.h"
#include "core/homeops.h"
#include "core/styleops.h"
#include "flow/flowdocument.h"
#include "core/proofops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textprops.h"

#include <QAction>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QStatusBar>
#include <QUrlQuery>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QTimer>
#include <QToolButton>
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

QTextCharFormat charFormatAtEnd(WriteWindow &w)
{
    QTextCursor e(w.document()->story());
    e.setPosition(w.document()->story()->characterCount() - 1);
    e.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    return e.charFormat();
}

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

// A picture of a widget for the person looking at the round's work: saved only when JP_SHOT_DIR names a folder.
void shot(QWidget *w, const QString &name)
{
    const QString dir = qEnvironmentVariable("JP_SHOT_DIR");
    if (!dir.isEmpty() && w) w->grab().save(dir + QLatin1Char('/') + name);
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

QByteArray pngBytes()
{
    QImage image(40, 20, QImage::Format_ARGB32);
    image.fill(Qt::blue);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return png;
}

// A package of a document protected as `edit` says (with the password, when given).
QByteArray protectedDocx(const QString &edit, const QString &password)
{
    auto d = open(makeDocx(para(run("hello"))));
    d->setProtection(office::wml::makeDocumentProtection(edit, password));
    return d->toDocx();
}

int picturesIn(WriteDoc &doc)
{
    int n = 0;
    for (int i = 0; i < doc.story()->characterCount(); ++i) n += isPictureAt(doc.story(), i);
    return n;
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

    void characterShadingAndBorderAreSetDrawnAndKept()
    {
        auto doc = open(makeDocx(para(run("one two three"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(4);
        c.setPosition(7, QTextCursor::KeepAnchor);
        setCharShading(*doc, c, QColor(0xFF, 0xD9, 0x66));
        setCharBorder(*doc, c, true);
        QTextCursor at(doc->story());
        at.setPosition(6);
        QCOMPARE(at.charFormat().stringProperty(prop::CharShade), QStringLiteral("FFD966"));
        QVERIFY(at.charFormat().hasProperty(tp::HighlightRefP));   // (drawn as a color behind the letters)
        QVERIFY(at.charFormat().hasProperty(tp::RunBorder));
        at.setPosition(2);
        QVERIFY(!at.charFormat().hasProperty(prop::CharShade));
        QVERIFY(allSelected(c, [](const QTextCharFormat &f) { return f.hasProperty(tp::RunBorder); }));
        // The file says them as a run's border and shading, in the schema's order, and not as a highlight.
        const QByteArray body = part(doc->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(body.contains("<w:bdr w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"FFD966\"/>"), body.constData());
        QVERIFY(!body.contains("w:highlight"));
        // Opened again, both are there.
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QTextCursor r(again->story());
        r.setPosition(6);
        QCOMPARE(r.charFormat().stringProperty(prop::CharShade), QStringLiteral("FFD966"));
        QVERIFY(r.charFormat().hasProperty(tp::RunBorder));
        QVERIFY(r.charFormat().hasProperty(tp::HighlightRefP));
        // Taking them away leaves the run as plain text, and one step of Undo brings one back.
        c.setPosition(4);
        c.setPosition(7, QTextCursor::KeepAnchor);
        setCharShading(*doc, c, QColor());
        setCharBorder(*doc, c, false);
        const QByteArray gone = part(doc->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY(!gone.contains("w:shd"));
        QVERIFY(!gone.contains("w:bdr"));
        doc->story()->undo();
        QVERIFY(part(doc->toDocx(), QStringLiteral("/word/document.xml")).contains("w:bdr"));
    }

    void aFilesCharacterShadingAndBorderCanBeReplaced()
    {
        auto doc = open(makeDocx(para(run("shaded", "<w:bdr w:val=\"double\" w:sz=\"8\" w:space=\"1\" w:color=\"FF0000\"/><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"00FF00\"/>"))));
        QVERIFY(doc);
        QTextCursor at(doc->story());
        at.setPosition(2);
        QCOMPARE(at.charFormat().stringProperty(prop::CharShade), QStringLiteral("00FF00"));
        QVERIFY(at.charFormat().hasProperty(tp::RunBorder));
        QVERIFY(part(doc->toDocx(), QStringLiteral("/word/document.xml")).contains("w:fill=\"00FF00\""));   // (kept as it was)
        QTextCursor c(doc->story());
        c.select(QTextCursor::Document);
        setCharShading(*doc, c, QColor(0, 0, 255));
        setCharBorder(*doc, c, false);
        const QByteArray body = part(doc->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY(body.contains("w:fill=\"0000FF\""));
        QVERIFY(!body.contains("00FF00"));
        QVERIFY(!body.contains("w:bdr"));
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
        QVERIFY(saved.contains("w:edit=\"trackedChanges\"") && saved.contains("w:enforcement=\"1\"") && saved.contains("w:cryptAlgorithmSid=\"14\""));
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

    void theAccessibilityCheckFindsMergedCellsOnceForATableAndLinksWithNoMeaningfulWords()
    {
        auto doc = open(makeDocx(para(run("Read click here now and the site"))));
        QString problem;
        QTextCursor sel(doc->story());
        sel.setPosition(5);
        sel.setPosition(15, QTextCursor::KeepAnchor);
        QVERIFY2(insertHyperlink(*doc, sel, QStringLiteral("https://example.com"), QString(), QString(), QString(), &problem), qPrintable(problem));
        QTextCursor sel2(doc->story());
        const int at = doc->story()->toPlainText().indexOf(QLatin1String("the site"));
        sel2.setPosition(at);
        sel2.setPosition(at + 8, QTextCursor::KeepAnchor);
        QVERIFY2(insertHyperlink(*doc, sel2, QStringLiteral("https://example.org/tides"), QString(), QString(), QString(), &problem), qPrintable(problem));
        QVector<AccessIssue> issues = checkAccessibility(*doc);
        int vague = 0;
        for (const AccessIssue &i : issues) vague += i.kind == AccessIssue::LinkText;
        QCOMPARE(vague, 1);
        QVERIFY(unhelpfulLinkText(QStringLiteral(" Click   HERE ")));
        QVERIFY(unhelpfulLinkText(QStringLiteral("https://example.com/a")));
        QVERIFY(unhelpfulLinkText(QString()));
        QVERIFY(!unhelpfulLinkText(QStringLiteral("the tide tables")));
        NewTable t;
        t.rows = 2;
        t.columns = 3;
        QTextCursor c(doc->story());
        c.setPosition(0);
        insertTable(*doc, c, t);
        for (const AccessIssue &i : checkAccessibility(*doc)) QVERIFY(i.kind != AccessIssue::MergedCells);
        // Two cells of the first row are made one.
        QTextCursor m(doc->story());
        m.setPosition(0);
        QTextBlock first = doc->story()->begin();
        m.setPosition(first.position());
        QTextBlock second = first.next();
        QVERIFY(second.isValid());
        m.setPosition(second.position(), QTextCursor::KeepAnchor);
        QVERIFY(canMergeCells(*doc, m));
        QVERIFY(mergeCells(*doc, m));
        int merged = 0;
        for (const AccessIssue &i : checkAccessibility(*doc)) merged += i.kind == AccessIssue::MergedCells;
        QCOMPARE(merged, 1);
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
            // (Polled: the box is answered when it is there, however long the machine takes to show it.)
            auto *poll = new QTimer(d);
            QObject::connect(poll, &QTimer::timeout, poll, [] {
                for (QWidget *top : QApplication::topLevelWidgets())
                    if (auto *box = qobject_cast<QMessageBox *>(top); box && box->isVisible()) box->button(QMessageBox::Yes)->click();
            });
            poll->start(20);
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

    void theParagraphBoxSuppressesLineNumbersAndKeepsItInTheFile()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("a paragraph")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("para.dialog"))->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *d) {
            QVERIFY(!d->findChild<QCheckBox *>(QStringLiteral("suppressLineNumbers"))->isChecked());
            d->findChild<QCheckBox *>(QStringLiteral("suppressLineNumbers"))->setChecked(true);
        });
        QVERIFY(w.document()->story()->begin().blockFormat().boolProperty(flow::fp::SuppressLineNumbers));
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(out.contains("<w:suppressLineNumbers/>"), out.constData());
        auto again = open(w.document()->toDocx());
        QVERIFY(again->story()->begin().blockFormat().boolProperty(flow::fp::SuppressLineNumbers));
        // Back off: the paragraph says so itself (its style might say otherwise).
        withDialog([&w] { w.act(QStringLiteral("para.dialog"))->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *d) {
            QVERIFY(d->findChild<QCheckBox *>(QStringLiteral("suppressLineNumbers"))->isChecked());
            d->findChild<QCheckBox *>(QStringLiteral("suppressLineNumbers"))->setChecked(false);
        });
        QVERIFY(!w.document()->story()->begin().blockFormat().boolProperty(flow::fp::SuppressLineNumbers));
        const QByteArray back = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(back.contains("<w:suppressLineNumbers w:val=\"0\"/>"), back.constData());
        QVERIFY(!open(w.document()->toDocx())->story()->begin().blockFormat().boolProperty(flow::fp::SuppressLineNumbers));
    }

    void hiddenTextShowsWhileTheFormattingMarksAreOnAndSavesTheSame()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        const QByteArray secret = QByteArray("secret words ").repeated(60);
        w.setDocument(open(makeDocx(para(run("shown ") + run(secret, "<w:vanish/>") + run("end")))), QString());
        const QByteArray before = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QCOMPARE(w.document()->pager().lines(0).size(), 1);
        w.act(QStringLiteral("view.marks"))->trigger();
        QVERIFY(w.view()->showMarks());
        QVERIFY2(w.document()->pager().lines(0).size() > 3, "hidden text is not shown with the marks on");
        QCOMPARE(part(w.document()->toDocx(), QStringLiteral("/word/document.xml")), before);
        w.act(QStringLiteral("view.marks"))->trigger();
        QVERIFY(!w.view()->showMarks());
        QCOMPARE(w.document()->pager().lines(0).size(), 1);
    }

    void theStylesListCanBeSortedByNameTypeBaseOrRecommendation()
    {
        auto doc = open(makeDocx(para(run("text"))));
        const auto names = [](const QVector<ListedStyle> &list) {
            QStringList out;
            for (const ListedStyle &e : list) out << e.name;
            return out;
        };
        const QVector<ListedStyle> alpha = listStyles(*doc, StyleFilter::Recommended, QString(), StyleSort::Alphabetical);
        QVERIFY(alpha.size() > 3);
        for (int i = 1; i < alpha.size(); ++i) QVERIFY2(QString::compare(alpha[i - 1].name, alpha[i].name, Qt::CaseInsensitive) <= 0, qPrintable(alpha[i].name));
        QVERIFY(names(alpha) != names(listStyles(*doc, StyleFilter::Recommended)));
        const QVector<ListedStyle> byType = listStyles(*doc, StyleFilter::All, QString(), StyleSort::ByType);
        bool sawCharacter = false;
        for (const ListedStyle &e : byType) {
            if (e.character) sawCharacter = true;
            else QVERIFY2(!sawCharacter, qPrintable(e.name));
        }
        QVERIFY(sawCharacter);
        const QVector<ListedStyle> byBase = listStyles(*doc, StyleFilter::All, QString(), StyleSort::BasedOn);
        for (int i = 1; i < byBase.size(); ++i) QVERIFY2(QString::compare(byBase[i - 1].basedOn, byBase[i].basedOn, Qt::CaseInsensitive) <= 0, qPrintable(byBase[i].name));
        QVERIFY(!byBase.last().basedOn.isEmpty());
        const QVector<ListedStyle> rec = listStyles(*doc, StyleFilter::All, QString(), StyleSort::AsRecommended);
        QCOMPARE(rec.first().id, doc->defaultParagraphStyle());
        QCOMPARE(names(listStyles(*doc, StyleFilter::All, QString(), StyleSort::Default)), names(listStyles(*doc, StyleFilter::All)));
    }

    void theFontBoxListsTheFontsChosenLastAboveAllFonts()
    {
        jp::Settings::get().setValue(QStringLiteral("fonts/recent"), QStringList());
        const QStringList families = jp::visibleFontFamilies();
        QVERIFY(families.size() >= 2);
        const QString first = families.at(0), second = families.at(1);
        auto texts = [](const jp::FontCombo &c) {
            QStringList out;
            for (int i = 0; i < c.count(); ++i) out << c.itemText(i);
            return out;
        };
        jp::FontCombo box;
        QVERIFY(!texts(box).contains(QStringLiteral("Recently Used")));
        box.lineEdit()->setText(first);
        QTest::keyClick(box.lineEdit(), Qt::Key_Return);
        box.lineEdit()->setText(second);
        QTest::keyClick(box.lineEdit(), Qt::Key_Return);
        box.lineEdit()->setText(QStringLiteral("+Heading"));
        QTest::keyClick(box.lineEdit(), Qt::Key_Return);
        jp::FontCombo later;
        later.showPopup();
        shot(later.view()->window(), QStringLiteral("font-list-recent.png"));
        later.hidePopup();
        for (const jp::FontCombo *c : {&box, &later}) {
            const QStringList t = texts(*c);
            const int at = int(t.indexOf(QStringLiteral("Recently Used")));
            QVERIFY(at > 0);
            QCOMPARE(t.at(at + 1), second);
            QCOMPARE(t.at(at + 2), first);
            QCOMPARE(t.at(at + 3), QStringLiteral("All Fonts"));
        }
        jp::Settings::get().setValue(QStringLiteral("fonts/recent"), QStringList());
    }

    void smartCutTakesOneSpaceWithTheWordsAndSmartPasteAddsOneWhereWordsWouldRunTogether()
    {
        auto doc = open(makeDocx(para(run("one two three, four"))));
        auto text = [&] { return doc->story()->toPlainText(); };
        auto select = [&](int from, int to) {
            QTextCursor c(doc->story());
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            return c;
        };
        // Cut "two": the two spaces round it become one, and one Undo puts it all back.
        const quint64 serial = doc->nextStepSerial();
        QTextCursor c = select(4, 7);
        smartRemoveSelection(*doc, c);
        QCOMPARE(text(), QStringLiteral("one three, four"));
        QCOMPARE(c.position(), 4);
        QVERIFY(doc->undo());
        QCOMPARE(text(), QStringLiteral("one two three, four"));
        QVERIFY(doc->nextStepSerial() > serial);
        // Cut "three": a space before the comma goes.
        c = select(8, 13);
        smartRemoveSelection(*doc, c);
        QCOMPARE(text(), QStringLiteral("one two, four"));
        QVERIFY(doc->undo());
        // Part of a word is cut as it is.
        c = select(5, 7);
        smartRemoveSelection(*doc, c);
        QCOMPARE(text(), QStringLiteral("one t three, four"));
        QVERIFY(doc->undo());
        // Paste between words that touch: spaces are added; in the middle of a word, none.
        auto paste = [&](int at, const QString &words) {
            QTextCursor p(doc->story());
            p.setPosition(at);
            smartPaste(*doc, p, [&] { insertText(*doc, p, words); });
            return p.position();
        };
        auto bare = open(makeDocx(para(run("abcd"))));
        doc = std::move(bare);
        QCOMPARE(paste(2, QStringLiteral("xy")), 4);
        QCOMPARE(text(), QStringLiteral("abxycd"));
        QVERIFY(doc->undo());
        doc = open(makeDocx(para(run("ab, cd"))));
        QCOMPARE(paste(2, QStringLiteral("word")), 7);
        QCOMPARE(text(), QStringLiteral("ab word, cd"));
        QVERIFY(doc->undo());
        doc = open(makeDocx(para(run("end"))));
        QCOMPARE(paste(3, QStringLiteral("next")), 8);
        QCOMPARE(text(), QStringLiteral("end next"));
        QVERIFY(doc->undo());
        QCOMPARE(text(), QStringLiteral("end"));
        doc = open(makeDocx(para(run("go on"))));
        QCOMPARE(paste(2, QStringLiteral("it")), 5);
        QCOMPARE(text(), QStringLiteral("go it on"));
        // Words between spaces are pasted as they are, and a paste that touches the word after it gets a space after.
        doc = open(makeDocx(para(run("go  on"))));
        QCOMPARE(paste(3, QStringLiteral("it")), 5);
        QCOMPARE(text(), QStringLiteral("go it on"));
    }

    void cuttingAndPastingInTheWindowKeepsTheSpacesRightUnlessSmartCutAndPasteIsOff()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("one two three")))), QString());
        auto select = [&](int from, int to) {
            QTextCursor c(w.document()->story());
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            w.view()->setCursor(c);
        };
        jp::Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), true);
        select(4, 7);
        w.view()->cut();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("one three"));
        QTextCursor end(w.document()->story());
        end.setPosition(3);
        w.view()->setCursor(end);
        w.view()->paste();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("one two three"));
        jp::Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), false);
        select(4, 7);
        w.view()->cut();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("one  three"));
        jp::Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), true);
    }

    void aPasteOffersOptionsUntilTheNextEditAndTheChoiceRedoesThePaste()
    {
        jp::Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), true);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("plain ") + run("bold", "<w:b/>") + run(" end")))), QString());
        QTextCursor c(w.document()->story());
        c.setPosition(6);
        c.setPosition(10, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        w.view()->copy();
        QVERIFY(!w.view()->pasteOptionsAvailable());
        c.movePosition(QTextCursor::End);
        w.view()->setCursor(c);
        w.view()->paste();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("plain bold end bold"));
        QVERIFY(w.view()->pasteOptionsAvailable());
        auto *button = w.view()->findChild<QToolButton *>(QStringLiteral("pasteOptionsButton"));
        QVERIFY(button);
        QTRY_VERIFY(button->isVisible());
        shot(&w, QStringLiteral("paste-options.png"));
        withDialog([&w] { w.act(QStringLiteral("edit.goTo"))->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
            d->findChild<QListWidget *>(QStringLiteral("goToKinds"))->setCurrentRow(8);   // Table
            shot(d, QStringLiteral("go-to.png"));
        }, false);
        auto lastIsBold = [&] { return charFormatAtEnd(w).fontWeight() >= QFont::DemiBold; };
        QVERIFY(lastIsBold());
        QVERIFY(w.view()->changeLastPaste(PageView::PasteKind::TextOnly));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("plain bold end bold"));
        QVERIFY(!lastIsBold());
        QVERIFY(w.view()->pasteOptionsAvailable());
        QVERIFY(w.view()->changeLastPaste(PageView::PasteKind::Keep));
        QVERIFY(lastIsBold());
        // One Undo takes the paste back whatever it was changed to.
        QVERIFY(w.document()->undo());
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("plain bold end"));
        // Typing, or moving the caret, ends the offer.
        w.view()->paste();
        QVERIFY(w.view()->pasteOptionsAvailable());
        w.view()->insertText(QStringLiteral("x"));
        QVERIFY(!w.view()->pasteOptionsAvailable());
        QVERIFY(!w.view()->changeLastPaste(PageView::PasteKind::TextOnly));
        QTRY_VERIFY(!button->isVisible());
    }

    // ---- the review's fixes
    void saveAsOfAProtectedDocumentDoesNotLiftTheProtection()
    {
        QTemporaryDir dir;
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(protectedDocx(QStringLiteral("readOnly"), QStringLiteral("pw"))), QString());
        QVERIFY(w.isReadOnly());
        w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("copy.docx")); });
        QVERIFY(w.saveAs());
        QVERIFY2(w.isReadOnly(), "Save As lifted the protection");
        QVERIFY(w.document()->protection() && w.document()->protection()->enforced);
    }

    void restrictEditingAndLockTrackingCanBeRunInAProtectedWindowToStopTheProtection()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(protectedDocx(QStringLiteral("readOnly"), QStringLiteral("pw"))), QString());
        QVERIFY(w.isReadOnly());
        // Lock Tracking opens its box too (and a wrong password stops nothing).
        withDialog([&w] { w.act(QStringLiteral("review.lockTracking"))->trigger(); }, QStringLiteral("restrictEditingDialog"), [](QDialog *d) {
            d->findChild<QLineEdit *>(QStringLiteral("protectPassword"))->setText(QStringLiteral("wrong"));
        });
        QVERIFY(w.isReadOnly());
        withDialog([&w] { w.act(QStringLiteral("review.restrictEditing"))->trigger(); }, QStringLiteral("restrictEditingDialog"), [](QDialog *d) {
            d->findChild<QLineEdit *>(QStringLiteral("protectPassword"))->setText(QStringLiteral("pw"));
        });
        QVERIFY(!w.isReadOnly());
        QVERIFY(!w.document()->protection());
    }

    void theOdtNoteListsProtectionKerningAndNoProofing()
    {
        auto doc = open(makeDocx(para(run("kerned", "<w:kern w:val=\"28\"/>") + run(" skipped", "<w:noProof/>")), "<w:zoom w:percent=\"100\"/>"));
        doc->setProtection(office::wml::makeDocumentProtection(QStringLiteral("readOnly"), QString()));
        QStringList notes;
        QString err;
        QVERIFY(!doc->toOdt(&err, &notes).isEmpty());
        const QString all = notes.join(QLatin1Char('\n'));
        QVERIFY2(all.contains(QLatin1String("protection")) && all.contains(QLatin1String("kerning")) && all.contains(QLatin1String("spelling")), qPrintable(all));
    }

    // The pair of documents for the Compare tests below: a picture between two words that change, and a heading and a list item that are new.
    std::unique_ptr<WriteDoc> compareResult(bool protectOriginal, bool changeTheWords = true)
    {
        auto original = open(makeDocx(para(run("Alpha")) + para(run("one two")) + para(run("Omega"))));
        QTextCursor at(original->story());
        at.setPosition(original->story()->findBlockByNumber(1).position() + 4);
        if (insertPicture(*original, at, pngBytes(), QStringLiteral("p")) != PictureResult::Ok) return nullptr;   // "one <pic>two"
        if (protectOriginal) original->setProtection(office::wml::makeDocumentProtection(QStringLiteral("trackedChanges"), QString()));
        auto revised = open(makeDocx(para(run("Alpha")) + para(run("New heading")) + para(run("New item")) + para(run(changeTheWords ? "three" : "one two")) + para(run("Omega"))));
        QTextCursor h(revised->story());
        h.setPosition(revised->story()->findBlockByNumber(1).position());
        applyParagraphStyle(*revised, h, QStringLiteral("Heading1"));
        QTextCursor l(revised->story());
        l.setPosition(revised->story()->findBlockByNumber(2).position());
        toggleList(*revised, l, false);
        return compareDocuments(*original, *revised, CompareOptions(), nullptr);
    }

    void theCompareResultDoesNotInheritTheOriginalsProtection()
    {
        auto result = compareResult(true);
        QVERIFY(result);
        QVERIFY2(!result->protection(), "the Compare window inherited the protection");
    }

    void compareNeverDeletesAnUnchangedPictureBetweenChangedWords()
    {
        auto result = compareResult(false);
        QVERIFY(result);
        auto accepted = open(result->toDocx());
        acceptAll(*accepted);
        QCOMPARE(picturesIn(*accepted), 1);   // (not swallowed by the deleted words around it)
        auto rejected = open(result->toDocx());
        rejectAll(*rejected);
        QCOMPARE(picturesIn(*rejected), 1);
    }

    void aParagraphCompareInsertsKeepsItsStyle()
    {
        auto result = compareResult(false, false);
        QVERIFY(result);
        auto accepted = open(result->toDocx());
        acceptAll(*accepted);
        bool found = false, heading = false;
        for (QTextBlock b = accepted->story()->begin(); b.isValid(); b = b.next())
            if (b.text() == QLatin1String("New heading")) {
                found = true;
                heading = b.blockFormat().stringProperty(tp::StyleName) == QLatin1String("Heading1");
            }
        QVERIFY(found);
        QVERIFY2(heading, "the inserted heading lost its style");
    }

    void aListItemCompareInsertsStaysAListItem()
    {
        auto result = compareResult(false, false);
        QVERIFY(result);
        auto accepted = open(result->toDocx());
        acceptAll(*accepted);
        bool found = false, item = false;
        for (QTextBlock b = accepted->story()->begin(); b.isValid(); b = b.next())
            if (b.text() == QLatin1String("New item")) {
                found = true;
                item = listKindOf(*accepted, b) == ListKind::Bullets;
            }
        QVERIFY(found);
        QVERIFY2(item, "the inserted list item lost its list");
    }

    void aLargeCompareWithOneInsertedParagraphStaysOneChange()
    {
        QByteArray one, two;
        for (int i = 0; i < 7000; ++i) {
            const QByteArray text = QByteArray("Line number ") + QByteArray::number(i) + " of the long document";
            one += para(run(text.constData()));
            if (i == 3) two += para(run("A paragraph that was added near the top"));
            two += para(run(text.constData()));
        }
        auto a = open(makeDocx(one)), b = open(makeDocx(two));
        int changes = 0;
        auto r = compareDocuments(*a, *b, CompareOptions(), &changes);
        QVERIFY(r);
        QVERIFY2(changes <= 2, qPrintable(QString::number(changes)));
        auto accepted = open(r->toDocx());
        acceptAll(*accepted);
        QCOMPARE(texts(*accepted), texts(*b));
    }

    void sortRefusesTooManyParagraphsWithAMessage()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QByteArray body;
        for (int i = 0; i < 5200; ++i) body += para(run(QByteArray::number(5200 - i).constData()));
        w.setDocument(open(makeDocx(body)), QString());
        w.view()->selectAll();
        SortOptions o;
        o.keys = {SortKey{}};
        QCOMPARE(w.sortSelection(o), 0);
        QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("too many")), qPrintable(w.statusBar()->currentMessage()));
        QCOMPARE(w.document()->story()->findBlockByNumber(0).text(), QStringLiteral("5200"));
    }

    void translateRefusesPlainHttpExceptForThisComputerAndKeepsTheAddressShort()
    {
        QVERIFY(translateUrl(QStringLiteral("http://translate.example.org/"), QStringLiteral("fr"), QStringLiteral("x")).isEmpty());
        QVERIFY(!translateUrl(QStringLiteral("http://localhost:5000/"), QStringLiteral("fr"), QStringLiteral("x")).isEmpty());
        QVERIFY(!translateUrl(QStringLiteral("http://127.0.0.1:5000/"), QStringLiteral("fr"), QStringLiteral("x")).isEmpty());
        // Letters that take nine characters each once written into the address are cut to keep it under 8 KB.
        const QString url = translateUrl(QString(), QStringLiteral("ja"), QString(5000, QChar(0x3042)));
        QVERIFY(!url.isEmpty() && url.size() < 8200);
        // A pair of surrogates is never cut in two.
        QString emoji = QStringLiteral("a");
        for (int i = 0; i < 2600; ++i) emoji += QString::fromUcs4(U"\U0001F600");
        const QString cutUrl = translateUrl(QString(), QStringLiteral("fr"), emoji);
        QUrlQuery q{QUrl(cutUrl)};
        const QString sent = q.queryItemValue(QStringLiteral("q"), QUrl::FullyDecoded);
        QVERIFY(!sent.isEmpty());
        for (int i = 0; i < sent.size(); ++i) {
            if (sent.at(i).isHighSurrogate()) {
                QVERIFY2(i + 1 < sent.size() && sent.at(i + 1).isLowSurrogate(), "a pair was cut");
                ++i;   // (its second half)
            } else {
                QVERIFY2(!sent.at(i).isLowSurrogate(), "a pair was cut");
            }
        }
    }

    void protectionIsWrittenTheWayOtherProgramsReadItInTheDocumentsOwnPrefix()
    {
        namespace wml = jp::office::wml;
        const QByteArray settings = "<x:zoom x:percent=\"100\"/>";
        QByteArray body = "<?xml version=\"1.0\"?><x:document xmlns:x=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><x:body><x:p><x:r><x:t>a</x:t></x:r></x:p></x:body></x:document>";
        Q_UNUSED(body);
        const QByteArray out = wml::settingsWithDocumentProtection("<?xml version=\"1.0\"?><x:settings xmlns:x=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">" + settings + "</x:settings>",
                                                                   wml::makeDocumentProtection(QStringLiteral("readOnly"), QStringLiteral("pw")));
        QVERIFY2(out.contains("<x:documentProtection x:edit=\"readOnly\""), out.constData());
        QVERIFY(!out.contains("<w:") && !out.contains(" w:"));
        for (const char *a : {"cryptProviderType=\"rsaAES\"", "cryptAlgorithmClass=\"hash\"", "cryptAlgorithmType=\"typeAny\"", "cryptAlgorithmSid=\"14\"", "cryptSpinCount=\"100000\"", ":hash=\"", ":salt=\""})
            QVERIFY2(out.contains(a), a);
        // Read back (by either form), it accepts the password.
        const auto read = wml::settingsDocumentProtection(out);
        QVERIFY(read && read->hasPassword() && wml::documentProtectionAccepts(*read, QStringLiteral("pw")) && !wml::documentProtectionAccepts(*read, QStringLiteral("px")));
    }

    void theHashOfAFixedSaltAndPasswordIsTheKnownAnswer()
    {
        namespace wml = jp::office::wml;
        QByteArray salt;
        for (int i = 0; i < 16; ++i) salt.append(char(i));
        // Worked out apart from this code (SHA-512 of the salt and the UTF-16 password, then 10,000 rounds of the hash and a four-byte counter).
        QCOMPARE(wml::protectionHash(salt, QStringLiteral("secret"), 10000).toBase64(),
                 QByteArray("AC+1sAKQzIdc0mQ+PRuDVuw+IuWZnDJeVt5/o//z13rTpptv+u/9lComOtJwhRUVIqRGjBMjO7vSsz9kqvxzqg=="));
        const wml::DocumentProtection p = wml::makeDocumentProtection(QStringLiteral("readOnly"), QStringLiteral("secret"), salt);
        QVERIFY(p.xml.contains("w:salt=\"AAECAwQFBgcICQoLDA0ODw==\""));
        QCOMPARE(wml::checkDocumentPassword(p, QStringLiteral("secret")), wml::PasswordCheck::Right);
        QCOMPARE(wml::checkDocumentPassword(p, QStringLiteral("nope")), wml::PasswordCheck::Wrong);
    }

    void aHashThatIsLegacyOrAsksForTooManyRoundsIsSaidToBeUnsupportedAndIsNeverRun()
    {
        namespace wml = jp::office::wml;
        wml::DocumentProtection p;
        p.edit = QStringLiteral("readOnly");
        p.enforced = true;
        p.xml = "<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\" w:cryptAlgorithmSid=\"1\" w:cryptSpinCount=\"50000\" w:hash=\"AAAA\" w:salt=\"AAAA\"/>";   // (MD2)
        QCOMPARE(wml::checkDocumentPassword(p, QStringLiteral("x")), wml::PasswordCheck::Unsupported);
        p.xml = "<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\" w:algorithmName=\"SHA-512\" w:hashValue=\"AAAA\" w:saltValue=\"AAAA\" w:spinCount=\"2000000000\"/>";
        QElapsedTimer timer;
        timer.start();
        QCOMPARE(wml::checkDocumentPassword(p, QStringLiteral("x")), wml::PasswordCheck::Unsupported);
        QVERIFY(timer.elapsed() < 1000);
        // A check that is canceled stops.
        std::atomic<bool> cancel{true};
        p.xml = "<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\" w:algorithmName=\"SHA-512\" w:hashValue=\"AAAA\" w:saltValue=\"AAAA\" w:spinCount=\"900000\"/>";
        QCOMPARE(wml::checkDocumentPassword(p, QStringLiteral("x"), &cancel), wml::PasswordCheck::Canceled);
    }

    void aProtectedWindowSaysPlainlyWhenItCannotCheckThePassword()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QByteArray docx = protectedDocx(QStringLiteral("readOnly"), QStringLiteral("pw"));
        auto d = open(docx);
        office::wml::DocumentProtection md2;
        md2.edit = QStringLiteral("readOnly");
        md2.enforced = true;
        md2.xml = "<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\" w:cryptAlgorithmSid=\"1\" w:cryptSpinCount=\"10\" w:hash=\"AAAA\" w:salt=\"AAAA\"/>";
        d->setProtection(md2);
        w.setDocument(std::move(d), QString());
        QVERIFY(w.isReadOnly());
        withDialog([&w] { w.act(QStringLiteral("review.restrictEditing"))->trigger(); }, QStringLiteral("restrictEditingDialog"), [](QDialog *dlg) {
            dlg->findChild<QLineEdit *>(QStringLiteral("protectPassword"))->setText(QStringLiteral("anything"));
        });
        QVERIFY(w.isReadOnly());
        QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("can't check")), qPrintable(w.statusBar()->currentMessage()));
        QVERIFY(!w.statusBar()->currentMessage().contains(QLatin1String("not the password")));
    }

    void translateIsRefusedForPlainHttpAndOpensThroughTheBrowserHookWithTheCutTextTold()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("x")))), QString());
        QList<QUrl> opened;
        bool answer = true;
        w.setUrlOpener([&](const QUrl &u) {
            opened << u;
            return answer;
        });
        QVERIFY(!w.openTranslation(QStringLiteral("hello"), QStringLiteral("fr"), QStringLiteral("http://elsewhere.example/")));
        QVERIFY(opened.isEmpty());
        QVERIFY(w.openTranslation(QStringLiteral("hello"), QStringLiteral("fr"), QStringLiteral("https://translate.example/")));
        QCOMPARE(opened.size(), 1);
        QCOMPARE(opened.first().scheme(), QStringLiteral("https"));
        QVERIFY(opened.first().query().contains(QLatin1String("target=fr")));
        // Longer than the limit: opened, and the cut told.
        QVERIFY(w.openTranslation(QString(6000, QLatin1Char('b')), QStringLiteral("fr"), QString()));
        QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("5000")) || w.statusBar()->currentMessage().contains(QLatin1String("5,000")), qPrintable(w.statusBar()->currentMessage()));
        // A browser that does not open is said so.
        answer = false;
        QVERIFY(!w.openTranslation(QStringLiteral("hello"), QStringLiteral("fr"), QString()));
        QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("could not be opened")), qPrintable(w.statusBar()->currentMessage()));
        QCOMPARE(cutAtLetters(QStringLiteral("ab"), 5), QStringLiteral("ab"));
    }

    void aCompareThatIsCanceledMakesNothing()
    {
        auto a = open(makeDocx(para(run("one")) + para(run("two")))), b = open(makeDocx(para(run("one")) + para(run("three"))));
        QString err = QStringLiteral("x");
        auto r = compareDocuments(*a, *b, CompareOptions(), nullptr, &err, [](int, int) { return false; });
        QVERIFY(!r);
        QVERIFY(err.isEmpty());
    }

    void aHorizontalLineIsAParagraphWithABorderUnderItAndTheCaretGoesBelow()
    {
        auto doc = open(makeDocx(para(run("above"))));
        QTextCursor c(doc->story());
        c.setPosition(2);
        QVERIFY(insertHorizontalLine(*doc, c));
        QCOMPARE(texts(*doc), (QStringList{"above", "", ""}));
        QCOMPARE(doc->story()->findBlock(c.position()).blockNumber(), 2);
        const QByteArray out = part(doc->toDocx(), QStringLiteral("/word/document.xml"));
        QCOMPARE(out.count("<w:pBdr>"), 1);
        QVERIFY2(out.contains("<w:bottom w:val=\"single\""), out.constData());
        auto again = open(doc->toDocx());
        QVERIFY(again->story()->findBlockByNumber(1).blockFormat().hasProperty(tp::Borders));
        QVERIFY(!again->story()->findBlockByNumber(2).blockFormat().hasProperty(tp::Borders));
        doc->story()->undo();
        QCOMPARE(texts(*doc), (QStringList{"above"}));
    }

    void revealFormattingTellsWhatIsSetAtTheCaret()
    {
        auto doc = open(makeDocx(para(run("bold words", "<w:b/><w:sz w:val=\"28\"/><w:color w:val=\"FF0000\"/>"), "<w:jc w:val=\"center\"/><w:ind w:left=\"720\"/>")));
        QTextCursor c(doc->story());
        c.setPosition(3);
        QMap<QString, QString> facts;
        for (const auto &f : describeFormatting(*doc, c)) facts.insert(f.first, f.second);
        QCOMPARE(facts.value(QStringLiteral("Look")), QStringLiteral("bold"));
        QCOMPARE(facts.value(QStringLiteral("Size")), QStringLiteral("14 pt"));
        QCOMPARE(facts.value(QStringLiteral("Color")), QStringLiteral("#FF0000"));
        QCOMPARE(facts.value(QStringLiteral("Alignment")), QStringLiteral("Centered"));
        QVERIFY(facts.value(QStringLiteral("Indents (left, right, first line)")).startsWith(QLatin1String("36 pt")));
        QVERIFY(!facts.value(QStringLiteral("Paragraph style")).isEmpty());
    }

    void goToFindsEndnotesCommentsFieldsTablesGraphicsAndEquations()
    {
        const QByteArray cell = "<w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>c</w:t></w:r></w:p></w:tc>";
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"4000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr>" + cell + cell + "</w:tr><w:tr>" + cell + cell + "</w:tr></w:tbl>";
        QByteArray body = "<w:p><w:r><w:t>first words here</w:t></w:r></w:p>";
        body += table + "<w:p><w:r><w:t>between</w:t></w:r></w:p>" + table;
        body += "<w:p><w:r><w:t>a field </w:t></w:r><w:fldSimple w:instr=\" DATE \"><w:r><w:t>today</w:t></w:r></w:fldSimple></w:p>";
        body += "<w:p><m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:r><m:t>x+1</m:t></m:r></m:oMath><w:r><w:t> and last</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        // Made by the program's own commands, from the end back so that earlier places keep theirs.
        const int lastWords = doc->story()->toPlainText().indexOf(QStringLiteral("last"));
        QTextCursor at(doc->story());
        at.setPosition(lastWords);
        QVERIFY(insertPictureImage(*doc, at, QImage(8, 8, QImage::Format_RGB32)) == PictureResult::Ok);
        at.setPosition(3);
        at.setPosition(8, QTextCursor::KeepAnchor);
        QVERIFY(addComment(*doc, at, QStringLiteral("look")) >= 0);
        at = QTextCursor(doc->story());
        at.setPosition(10);
        QVERIFY(insertEndnote(*doc, at));
        QString problem;
        for (GoToKind kind : {GoToKind::Endnote, GoToKind::Comment, GoToKind::Graphic, GoToKind::Equation, GoToKind::Field}) {
            QCOMPARE(goToItems(*doc, kind).size(), 1);
            const int one = goToPosition(*doc, kind, QStringLiteral("1"), 0, &problem);
            QVERIFY2(one >= 0, qPrintable(problem));
            QCOMPARE(goToPosition(*doc, kind, QStringLiteral("2"), 0), -1);
            QCOMPARE(goToPosition(*doc, kind, QStringLiteral("+1"), 0), one);
            QCOMPARE(goToPosition(*doc, kind, QStringLiteral("0"), 0, &problem), -1);
            QVERIFY(!problem.isEmpty());
        }
        const QVector<GoToItem> tables = goToItems(*doc, GoToKind::Table);
        QCOMPARE(tables.size(), 2);
        QVERIFY(tables[0].position < tables[1].position);
        QCOMPARE(goToPosition(*doc, GoToKind::Table, QStringLiteral("2"), 0), tables[1].position);
        QCOMPARE(goToPosition(*doc, GoToKind::Table, QStringLiteral("+1"), tables[0].position + 1), tables[1].position);
        QCOMPARE(goToPosition(*doc, GoToKind::Table, QStringLiteral("3"), 0), -1);
        QVERIFY(goToPosition(*doc, GoToKind::Graphic, QStringLiteral("1"), 0) > goToPosition(*doc, GoToKind::Equation, QStringLiteral("1"), 0));
        QVERIFY(goToPosition(*doc, GoToKind::Comment, QStringLiteral("1"), 0) < goToPosition(*doc, GoToKind::Endnote, QStringLiteral("1"), 0));
    }
};

QTEST_MAIN(HomeTests)
#include "tst_write_home.moc"
