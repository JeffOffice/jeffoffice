// Tests of JeffWrite's Home and Review tab commands that work on text and settings (apps/write/core/homeops.*, proofops.*, and the window's actions).
// Each document is built here from a few lines of XML.

#include "../cmdsearchcheck.h"
#include "../isolatesettings.h"
#include "app/widgets.h"
#include "app/theme.h"
#include <QTabWidget>
#include "app/settings.h"
#include "core/fontfiles.h"
#include "app/writedialogs.h"
#include "app/writewindow.h"
#include "app/pageview.h"
#include "app/pageoutput.h"
#include "render/pdfpage.h"
#include <QBuffer>
#include <QScopeGuard>
#include <QPdfWriter>
#include "app/ruler.h"
#include "app/writefindpane.h"
#include "core/appinfo.h"
#include "core/accessops.h"
#include "core/linkops.h"
#include "core/commentops.h"
#include "core/compareops.h"
#include "core/gotoops.h"
#include "core/outlineops.h"
#include "core/normaltemplate.h"
#include "app/normaltemplateui.h"
#include "office/packageprops.h"
#include "core/pagesetup.h"
#include "app/userdefaults.h"
#include "core/editops.h"
#include "core/pictureops.h"
#include "core/reviewops.h"
#include "core/tableops.h"
#include "wml/wml.h"
#include "core/fonts.h"
#include "core/findops.h"
#include "core/homeops.h"
#include "core/insertops.h"
#include "core/styleops.h"
#include "flow/flowdocument.h"
#include "core/proofops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textprops.h"

#include <QAction>
#include <QClipboard>
#include <QMimeData>
#include <QTextList>
#include <QLockFile>
#include <QThread>
#include <thread>
#include <QElapsedTimer>
#include <QSignalSpy>
#include <QLabel>
#include <QMenu>
#include <QLineEdit>
#include <QStatusBar>
#include <QUrlQuery>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>
#include <QListWidget>
#include <QBuffer>
#include <QImage>
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QCryptographicHash>
#include <QScrollBar>
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
// With JO_SHOT_DIR set, saves a picture of the widget there as `name`.png (for the screenshots of a change's dialogs); otherwise does nothing.
void shotOf(QWidget *widget, const QString &name)
{
    const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
    if (!dir.isEmpty() && widget) widget->grab().save(dir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

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

// A counting mime object: the data of a format is read only through retrieveData, so the count says how often it was.
struct CountingMime : QMimeData {
    mutable int reads = 0;
protected:
    QVariant retrieveData(const QString &format, QMetaType type) const override
    {
        ++reads;
        return QMimeData::retrieveData(format, type);
    }
};


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

    // The command and its box are called Word Count, as in the other apps.
    void wordCountIsCalledWordCountLikeTheOtherApps()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 700);
        w.show();
        w.setDocument(open(makeDocx(para(run("Two words")))), QString());
        QCOMPARE(w.act(QStringLiteral("review.wordCount"))->text(), QStringLiteral("Word Count"));
        withDialog([&] { w.act(QStringLiteral("review.wordCount"))->trigger(); }, QStringLiteral("wordCountDialog"),
                   [&](QDialog *d) { QCOMPARE(d->windowTitle(), QStringLiteral("Word Count")); }, false);
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

    // With JO_SHOT_DIR set to a folder, saves a picture of each dialog there (for looking at them); skipped otherwise.
    void picturesOfTheDialogs()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
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
        QTextCursor cursor = w.view()->cursor();
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [&](QDialog *d) { d->grab().save(dir + QStringLiteral("/dialog-tabs.png")); });
        w.act(QStringLiteral("review.accessibility"))->trigger();
        w.grab().save(dir + QStringLiteral("/window-accessibility.png"));
    }

    // With JO_SHOT_DIR set: pictures of the Tabs box (with bar stops listed) and of a page with bar tab stops and scaled runs, for the person looking at the round's work; skipped otherwise.
    void picturesOfTheTabsBoxAndABarAndScaledLetters()
    {
        if (qEnvironmentVariable("JO_SHOT_DIR").isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 700);
        w.show();
        const QByteArray bars = "<w:pPr><w:tabs><w:tab w:val=\"bar\" w:pos=\"2880\"/><w:tab w:val=\"bar\" w:pos=\"5760\"/></w:tabs></w:pPr>";
        w.setDocument(open(makeDocx(onePage("<w:p>" + bars + "<w:r><w:t>Name</w:t></w:r></w:p><w:p>" + bars + "<w:r><w:t xml:space=\"preserve\">Plain letters, then </w:t></w:r><w:r><w:rPr><w:w w:val=\"200\"/></w:rPr><w:t>wide</w:t></w:r>"
                                              "<w:r><w:t xml:space=\"preserve\"> and </w:t></w:r><w:r><w:rPr><w:w w:val=\"50\"/></w:rPr><w:t>narrow letters</w:t></w:r></w:p><w:p>" + bars + "<w:r><w:t>Third line</w:t></w:r></w:p>"), "<w:defaultTabStop w:val=\"360\"/>")), QString());
        QTextCursor cursor(w.document()->story());
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [&](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("tabType"))->setCurrentIndex(4);
            QCOMPARE(d->findChild<QListWidget *>(QStringLiteral("tabList"))->count(), 2);
            shotOf(d, QStringLiteral("write-tabs-box-bars"));
        }, false);
        w.view()->pageImage(0, 1224).copy(0, 0, 1224, 400).save(qEnvironmentVariable("JO_SHOT_DIR") + QStringLiteral("/write-page-bars-and-scaled.png"));
    }

    // With JO_SHOT_DIR set: a picture of every dialog the commands of JW_GRAB_IDS (or a standard list) open, named after the command.
    void morePicturesOfTheDialogs()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 700);
        w.show();
        w.setDocument(open(makeDocx(para(run("pear tree")) + para(run("Apple pie")) + para(run("cherry")))), QString());
        w.view()->selectAll();
        const QStringList ids = qEnvironmentVariable("JW_GRAB_IDS", QStringLiteral("edit.find,edit.replace,edit.goTo,insert.bookmark,insert.hyperlink,insert.field,insert.dateTime,layout.customMargins,layout.columns.more,layout.hyphenation.options,layout.lineNumbers.options,design.pageBorders,design.watermark.custom,refs.footnote,refs.crossReference,refs.insertCaption,refs.insertIndex,refs.markTocEntry,table.properties,table.sort,table.formula,mail.envelopes,mail.labels,insert.dropCap.options,zoom.dialog,fmt.charBorder,mail.wizard,insert.textFromFile")).split(QLatin1Char(','));
        for (const QString &id : ids) {
            QAction *a = w.act(id);
            if (!a || !a->isEnabled()) { qWarning("no enabled action %s", qPrintable(id)); continue; }
            bool grabbed = false;
            QTimer poll, watchdog;
            watchdog.setSingleShot(true);
            QObject::connect(&poll, &QTimer::timeout, [&] {
                QWidget *m = QApplication::activeModalWidget();
                if (!m) return;
                poll.stop();
                QTimer::singleShot(150, [&, m] {
                    m->grab().save(dir + QLatin1Char('/') + id + QStringLiteral(".png"));
                    grabbed = true;
                    if (auto *d = qobject_cast<QDialog *>(m)) d->reject(); else m->close();
                });
            });
            QObject::connect(&watchdog, &QTimer::timeout, [&] {
                if (QWidget *m = QApplication::activeModalWidget()) m->close();
            });
            poll.start(20);
            watchdog.start(8000);
            a->trigger();
            poll.stop();
            if (!grabbed) qWarning("no dialog for %s", qPrintable(id));
        }
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

    // The Advanced page of the Font box holds the OpenType choices: they are set for what is selected, written to the file, shown again, and left alone changing nothing.
    void theFontBoxAdvancedPageSetsOpenTypeChoicesAndTheFileKeepsThem()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("office 2026")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            auto *tabs = d->findChild<QTabWidget *>(QStringLiteral("fontTabs"));
            QVERIFY(tabs);
            QCOMPARE(tabs->tabText(1), QStringLiteral("Advanced"));
            tabs->setCurrentIndex(1);
            d->findChild<QComboBox *>(QStringLiteral("ligatures"))->setCurrentIndex(3);            // standard and contextual
            d->findChild<QComboBox *>(QStringLiteral("numberSpacing"))->setCurrentIndex(2);        // tabular
            d->findChild<QComboBox *>(QStringLiteral("numberForms"))->setCurrentIndex(2);          // old-style
            d->findChild<QComboBox *>(QStringLiteral("stylisticSet"))->setCurrentIndex(3);         // set 3
            d->findChild<QComboBox *>(QStringLiteral("contextualAlternates"))->setCurrentIndex(2); // off
            shotOf(d, QStringLiteral("write-font-advanced"));
        });
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(out.contains("<w14:ligatures w14:val=\"standardContextual\"/>") && out.contains("<w14:numSpacing w14:val=\"tabular\"/>") && out.contains("<w14:numForm w14:val=\"oldStyle\"/>")
                     && out.contains("<w14:styleSet w14:id=\"3\" w14:val=\"1\"/>") && out.contains("<w14:cntxtAlts w14:val=\"0\"/>"), out.constData());
        // The box shows them again, and leaving them alone changes nothing.
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("ligatures"))->currentIndex(), 3);
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("numberSpacing"))->currentIndex(), 2);
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("numberForms"))->currentIndex(), 2);
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("stylisticSet"))->currentIndex(), 3);
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("contextualAlternates"))->currentIndex(), 2);
        });
        QCOMPARE(part(w.document()->toDocx(), QStringLiteral("/word/document.xml")), out);
        // Back to the font's own: nothing of them is left in the file.
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            for (const char *name : {"ligatures", "numberSpacing", "numberForms", "stylisticSet", "contextualAlternates"}) d->findChild<QComboBox *>(QString::fromLatin1(name))->setCurrentIndex(0);
        });
        QVERIFY(!part(w.document()->toDocx(), QStringLiteral("/word/document.xml")).contains("w14:"));
    }

    // Where the characters selected differ, a box of the Advanced page shows (mixed) and, left alone, leaves each as it was.
    void aMixedChoiceOnTheAdvancedPageIsLeftAlone()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        const QByteArray ns = " xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
        w.setDocument(open(makeDocx("<w:p><w:r><w:rPr><w14:numForm" + ns + " w14:val=\"lining\"/></w:rPr><w:t>one </w:t></w:r><w:r><w:rPr><w14:numForm" + ns + " w14:val=\"oldStyle\"/></w:rPr><w:t>two</w:t></w:r></w:p>")), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            auto *forms = d->findChild<QComboBox *>(QStringLiteral("numberForms"));
            QCOMPARE(forms->currentText(), QStringLiteral("(mixed)"));
            d->findChild<QComboBox *>(QStringLiteral("numberSpacing"))->setCurrentIndex(1);   // (only this is set)
        });
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(out.contains("w14:val=\"lining\"") && out.contains("w14:val=\"oldStyle\"") && out.contains("w14:val=\"proportional\""), out.constData());
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

    // The first dark pixel of a page's first line, in points from its left edge, and the last (a page 612 points wide, drawn 816 pixels wide).
    static QPair<double, double> inkOf(PageView &view)
    {
        const QImage img = view.pageImage(0, 816);
        const double perPoint = 816 / 612.0;
        int first = -1, last = -1;
        for (int x = 0; x < img.width(); ++x)
            for (int y = int(72 * perPoint); y < int(92 * perPoint); ++y)
                if (qGray(img.pixel(x, y)) < 128) {
                    if (first < 0) first = x;
                    last = x;
                    break;
                }
        return {first < 0 ? -1.0 : first / perPoint, last < 0 ? -1.0 : last / perPoint};
    }

    static QByteArray onePage(const QByteArray &body)
    {
        return body + "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    }

    void theTabsBoxSetsTheDistanceOfTheDefaultTabStopsAndTheLayoutAndTheFileFollow()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        const QByteArray plain = "<w:p><w:r><w:t>HHHH</w:t></w:r></w:p>", tabbed = "<w:p><w:r><w:tab/><w:t>HHHH</w:t></w:r></w:p>";
        PageView ref;
        ref.resize(900, 700);
        auto base = open(makeDocx(onePage(plain), "<w:defaultTabStop w:val=\"720\"/>"));
        ref.setDocument(base.get());
        const double margin = inkOf(ref).first;
        QVERIFY(margin > 60 && margin < 90);
        w.setDocument(open(makeDocx(onePage(tabbed), "<w:defaultTabStop w:val=\"720\"/>")), QString());
        QVERIFY2(std::abs(inkOf(*w.view()).first - margin - 36) < 3, qPrintable(QString::number(inkOf(*w.view()).first - margin)));
        w.view()->selectAll();
        QTextCursor cursor = w.view()->cursor();
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [](QDialog *d) {
            auto *distance = d->findChild<QDoubleSpinBox *>(QStringLiteral("defaultTabStop"));
            QVERIFY(distance);
            QCOMPARE(distance->value(), 36.0);
            distance->setValue(18.0);
            shotOf(d, QStringLiteral("write-tabs-box"));
        });
        QCOMPARE(w.document()->defaultTabStop(), 360);
        QTRY_VERIFY2(std::abs(inkOf(*w.view()).first - margin - 18) < 3, qPrintable(QString::number(inkOf(*w.view()).first - margin)));
        QVERIFY2(part(w.document()->toDocx(), QStringLiteral("/word/settings.xml")).contains("<w:defaultTabStop w:val=\"360\"/>"), "settings.xml");
        QCOMPARE(open(w.document()->toDocx())->defaultTabStop(), 360);
        // One step of the history: undone, the stops are as they were.
        QTextCursor at(w.document()->story());
        w.document()->undo(&at);
        QCOMPARE(w.document()->defaultTabStop(), 720);
        // Left alone, the box changes nothing.
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [](QDialog *) {});
        QCOMPARE(w.document()->defaultTabStop(), 720);
    }

    // Scaled letters and a bar stop go to a PDF as the page shows them (the page is drawn the same way for the screen, PDF, and the printer).
    void scaledLettersAndBarStopsAreInThePdfAsOnThePage()
    {
        auto pdfInk = [](const QByteArray &body, int *barColumns = nullptr) {
            auto doc = open(makeDocx(onePage(body)));
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            {
                QPdfWriter pdf(&buffer);
                if (!paintSheets(*doc, &pdf, {0}, 1)) return QPair<double, double>(-1, -1);
            }
            PdfDocument pages(buffer.data());
            if (!pages.isValid()) return QPair<double, double>(-1, -1);
            QImage img(816, 1056, QImage::Format_RGB32);
            img.fill(Qt::white);
            {
                QPainter on(&img);
                on.drawImage(0, 0, pages.render(0, QSize(816, 1056)));
            }
            const double perPoint = 816 / 612.0;
            int first = -1, last = -1;
            for (int x = 0; x < img.width(); ++x)
                for (int y = int(72 * perPoint); y < int(92 * perPoint); ++y)
                    if (qGray(img.pixel(x, y)) < 128) {
                        if (first < 0) first = x;
                        last = x;
                        break;
                    }
            if (barColumns) {
                *barColumns = 0;
                for (int x = int(136 * perPoint); x <= int(152 * perPoint); ++x) {
                    int run = 0, best = 0;
                    for (int y = int(72 * perPoint); y < int(120 * perPoint); ++y) {
                        run = qGray(img.pixel(x, y)) < 200 ? run + 1 : 0;
                        best = std::max(best, run);
                    }
                    *barColumns = std::max(*barColumns, best);
                }
            }
            return QPair<double, double>(first / perPoint, last / perPoint);
        };
        const QPair<double, double> plain = pdfInk("<w:p><w:r><w:t>HHHH</w:t></w:r></w:p>"), wide = pdfInk("<w:p><w:r><w:rPr><w:w w:val=\"200\"/></w:rPr><w:t>HHHH</w:t></w:r></w:p>");
        QVERIFY2(plain.first > 0 && wide.first > 0, qPrintable(QStringLiteral("%1 %2 %3 %4").arg(plain.first).arg(plain.second).arg(wide.first).arg(wide.second)));
        const double ratio = (wide.second - wide.first) / (plain.second - plain.first);
        QVERIFY2(ratio > 1.8 && ratio < 2.2, qPrintable(QString::number(ratio)));
        int bar = 0;
        pdfInk("<w:p><w:pPr><w:tabs><w:tab w:val=\"bar\" w:pos=\"1440\"/></w:tabs></w:pPr><w:r><w:t>ab</w:t></w:r></w:p>", &bar);
        QVERIFY2(bar >= 8, qPrintable(QString::number(bar)));
    }

    // The list of stops is in the unit of the position field (the person's measurement unit), not always points.
    void theTabsBoxListsItsStopsInThePersonsUnit()
    {
        const Unit was = Settings::get().unit();
        const auto restore = qScopeGuard([was] { Settings::get().setUnit(was); });
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("ab"), "<w:tabs><w:tab w:val=\"bar\" w:pos=\"2880\"/></w:tabs>"))), QString());
        QTextCursor cursor(w.document()->story());
        QString inches, points;
        Settings::get().setUnit(Unit::Inch);
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [&](QDialog *d) { inches = d->findChild<QListWidget *>(QStringLiteral("tabList"))->item(0)->text(); }, false);
        Settings::get().setUnit(Unit::Point);
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [&](QDialog *d) { points = d->findChild<QListWidget *>(QStringLiteral("tabList"))->item(0)->text(); }, false);
        QVERIFY2(inches.startsWith(QLatin1String("2\"")) && inches.contains(QLatin1String("bar")), qPrintable(inches));
        QVERIFY2(points.startsWith(QLatin1String("144")) && points != inches, qPrintable(points));
    }

    // The most bar stops a paragraph keeps is 64: setTabStopsOf drops the rest, and the Tabs box refuses a 65th and says so.
    void bar64StopsAreTheMostAndTheTabsBoxRefusesAnother()
    {
        QTextBlockFormat bf;
        QVector<TabStopEntry> many;
        for (int i = 0; i < 70; ++i) {
            TabStopEntry e;
            e.position = 10 + i * 5;
            e.bar = true;
            many.push_back(e);
        }
        setTabStopsOf(bf, many);
        QCOMPARE(int(tabStopsOf(bf).size()), 64);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("ab")))), QString());
        QTextCursor cursor(w.document()->story());
        many.resize(64);
        setTabStops(*w.document(), cursor, many);
        QCOMPARE(int(tabStopsOf(w.document()->story()->begin().blockFormat()).size()), 64);
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [](QDialog *d) {
            QCOMPARE(d->findChild<QListWidget *>(QStringLiteral("tabList"))->count(), 64);
            QVERIFY(d->findChild<QLabel *>(QStringLiteral("tabNote"))->text().isEmpty());
            d->findChild<QDoubleSpinBox *>(QStringLiteral("tabPosition"))->setValue(500.0);
            auto *type = d->findChild<QComboBox *>(QStringLiteral("tabType"));
            type->setCurrentIndex(type->findText(QStringLiteral("Bar")));
            d->findChild<QPushButton *>(QStringLiteral("tabSet"))->click();
            QCOMPARE(d->findChild<QListWidget *>(QStringLiteral("tabList"))->count(), 64);
            QVERIFY(!d->findChild<QLabel *>(QStringLiteral("tabNote"))->text().isEmpty());
        });
        QCOMPARE(int(tabStopsOf(w.document()->story()->begin().blockFormat()).size()), 64);
    }

    void theTabsBoxSetsBarStopsThatAreDrawnAsLinesAndKeptInTheFile()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        w.setDocument(open(makeDocx(onePage("<w:p><w:r><w:t>ab</w:t></w:r></w:p><w:p><w:r><w:t>cd</w:t></w:r></w:p>"))), QString());
        // The longest run of dark pixels in a column, in the stretch of the first line at 144 points from the page's edge.
        auto barHeight = [&] {
            const QImage img = w.view()->pageImage(0, 816);
            const double perPoint = 816 / 612.0;
            int best = 0;
            for (int x = int(136 * perPoint); x <= int(152 * perPoint); ++x) {
                int run = 0;
                for (int y = int(72 * perPoint); y < int(120 * perPoint); ++y) {
                    run = qGray(img.pixel(x, y)) < 200 ? run + 1 : 0;
                    best = std::max(best, run);
                }
            }
            return best;
        };
        QVERIFY(barHeight() < 4);
        QTextCursor cursor(w.document()->story());
        cursor.setPosition(0);
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [](QDialog *d) {
            d->findChild<QDoubleSpinBox *>(QStringLiteral("tabPosition"))->setValue(72.0);
            auto *type = d->findChild<QComboBox *>(QStringLiteral("tabType"));
            const int bar = type->findText(QStringLiteral("Bar"));
            QVERIFY(bar >= 0);
            type->setCurrentIndex(bar);
            d->findChild<QPushButton *>(QStringLiteral("tabSet"))->click();
            QCOMPARE(d->findChild<QListWidget *>(QStringLiteral("tabList"))->count(), 1);
            QVERIFY(d->findChild<QListWidget *>(QStringLiteral("tabList"))->item(0)->text().contains(QLatin1String("bar")));
        });
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(out.contains("<w:tab w:val=\"bar\" w:pos=\"1440\"/>"), out.constData());
        QCOMPARE(int(out.count("w:val=\"bar\"")), 1);   // (the first paragraph's alone)
        QTRY_VERIFY2(barHeight() >= 8, qPrintable(QString::number(barHeight())));
        const QVector<TabStopEntry> stops = tabStopsOf(open(w.document()->toDocx())->story()->begin().blockFormat());
        QCOMPARE(stops.size(), 1);
        QVERIFY(stops[0].bar);
        QCOMPARE(stops[0].position, 72.0);
        // The ruler shows the stops the text goes to, not the line.
        QVERIFY(w.ruler()->tabStops().isEmpty());
        // The stop is listed again and taken away by Clear.
        withDialog([&] { tabsDialog(&w, w.document(), &cursor); }, QStringLiteral("tabsDialog"), [](QDialog *d) {
            QCOMPARE(d->findChild<QListWidget *>(QStringLiteral("tabList"))->count(), 1);
            d->findChild<QPushButton *>(QStringLiteral("tabClearAll"))->click();
        });
        QVERIFY(!part(w.document()->toDocx(), QStringLiteral("/word/document.xml")).contains("w:val=\"bar\""));
        QTRY_VERIFY(barHeight() < 4);
    }

    void theFontBoxScalesCharactersAndTheLayoutAndTheFileFollow()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        w.setDocument(open(makeDocx(onePage("<w:p><w:r><w:t>HHHH</w:t></w:r></w:p>"))), QString());
        const QPair<double, double> before = inkOf(*w.view());
        const double wideBefore = before.second - before.first;
        QVERIFY(wideBefore > 10);
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            auto *scaling = d->findChild<QSpinBox *>(QStringLiteral("scaling"));
            QVERIFY(scaling);
            QCOMPARE(scaling->value(), 100);
            QCOMPARE(scaling->maximum(), 600);
            QCOMPARE(scaling->minimum(), 1);
            scaling->setValue(200);
        });
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/document.xml"));
        QVERIFY2(out.contains("<w:w w:val=\"200\"/>"), out.constData());
        QTRY_VERIFY(inkOf(*w.view()).second > 0);
        const QPair<double, double> after = inkOf(*w.view());
        const double ratio = (after.second - after.first) / wideBefore;
        QVERIFY2(ratio > 1.8 && ratio < 2.2, qPrintable(QString::number(ratio)));
        QCOMPARE(open(w.document()->toDocx())->story()->begin().begin().fragment().charFormat().fontStretch(), 200);
        // The box reads it back, and left alone changes nothing.
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) { QCOMPARE(d->findChild<QSpinBox *>(QStringLiteral("scaling"))->value(), 200); });
        QCOMPARE(part(w.document()->toDocx(), QStringLiteral("/word/document.xml")), out);
        // Back to 100: the element goes.
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) { d->findChild<QSpinBox *>(QStringLiteral("scaling"))->setValue(100); });
        QVERIFY(!part(w.document()->toDocx(), QStringLiteral("/word/document.xml")).contains("<w:w "));
    }

    void setAsDefaultGivesTheNormalStyleTheFontChosen()
    {
        QFile::remove(normalTemplatePath());
        setDefaultScopeStandIn([](const QString &) { return DefaultScope::ThisDocument; });
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("some words")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->setValue(15);
            d->findChild<QPushButton *>(QStringLiteral("setAsDefault"))->click();
        }, false);
        setDefaultScopeStandIn(nullptr);
        const QByteArray styles = part(w.document()->toDocx(), QStringLiteral("/word/styles.xml"));
        QVERIFY2(styles.contains("<w:sz w:val=\"30\"/>"), styles.constData());
        // (this document only: no template was made)
        QVERIFY(!QFile::exists(normalTemplatePath()));
    }

    // ---- Set as Default for all new documents: the Normal template
    void theNormalTemplateIsMadeOnFirstUseAndNewDocumentsStartFromIt()
    {
        QFile::remove(normalTemplatePath());
        QVERIFY(!QFile::exists(normalTemplatePath()));
        QCOMPARE(pageSetupOf(newBlankDocument()->sectionProps(0), false).left, 72.0);
        QVERIFY(changeNormalTemplate([](WriteDoc &d) {
            office::wml::SectionProps p = d.sectionProps(0);
            p.left = 720;   // (twentieths of a point: half an inch)
            d.setSectionProps(0, p);
        }));
        QVERIFY(QFile::exists(normalTemplatePath()));
        QCOMPARE(pageSetupOf(newBlankDocument()->sectionProps(0), false).left, 36.0);
        // A second change keeps the first.
        QVERIFY(changeNormalTemplate([](WriteDoc &d) {
            office::wml::SectionProps p = d.sectionProps(0);
            p.right = 1080;
            d.setSectionProps(0, p);
        }));
        const SectionSetup now = pageSetupOf(newBlankDocument()->sectionProps(0), false);
        QCOMPARE(now.left, 36.0);
        QCOMPARE(now.right, 54.0);
        QFile::remove(normalTemplatePath());
    }

    void aNewWindowStartsFromTheNormalTemplate()
    {
        QFile::remove(normalTemplatePath());
        QVERIFY(changeNormalTemplate([](WriteDoc &d) {
            office::wml::SectionProps p = d.sectionProps(0);
            p.top = 360;
            d.setSectionProps(0, p);
        }));
        WriteWindow w;
        QCOMPARE(pageSetupOf(w.document()->sectionProps(0), false).top, 18.0);
        QFile::remove(normalTemplatePath());
    }

    void fontSetAsDefaultForAllNewDocumentsWritesTheNormalTemplate()
    {
        QFile::remove(normalTemplatePath());
        QString asked;
        setDefaultScopeStandIn([&asked](const QString &what) {
            asked = what;
            return DefaultScope::AllNew;
        });
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("some words")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("fmt.fontDialog"))->trigger(); }, QStringLiteral("fontDialog"), [](QDialog *d) {
            d->findChild<QDoubleSpinBox *>(QStringLiteral("fontSize"))->setValue(17);
            shotOf(d, QStringLiteral("write-font-dialog"));
            d->findChild<QPushButton *>(QStringLiteral("setAsDefault"))->click();
        }, false);
        setDefaultScopeStandIn(nullptr);
        QVERIFY(!asked.isEmpty());
        QVERIFY(QFile::exists(normalTemplatePath()));
        QVERIFY2(part(newBlankDocument()->toDocx(), QStringLiteral("/word/styles.xml")).contains("<w:sz w:val=\"34\"/>"), "the new document's Normal style is 17 pt");
        QFile::remove(normalTemplatePath());
    }

    void paragraphSetAsDefaultSetsTheNormalStyleSpacing()
    {
        QFile::remove(normalTemplatePath());
        setDefaultScopeStandIn([](const QString &) { return DefaultScope::AllNew; });
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("a paragraph")))), QString());
        w.view()->selectAll();
        withDialog([&w] { w.act(QStringLiteral("para.dialog"))->trigger(); }, QStringLiteral("paragraphDialog"), [](QDialog *d) {
            d->findChild<QDoubleSpinBox *>(QStringLiteral("after"))->setValue(24);
            shotOf(d, QStringLiteral("write-paragraph-dialog"));
            d->findChild<QPushButton *>(QStringLiteral("setAsDefault"))->click();
        }, false);
        setDefaultScopeStandIn(nullptr);
        QVERIFY(QFile::exists(normalTemplatePath()));
        const QByteArray styles = part(newBlankDocument()->toDocx(), QStringLiteral("/word/styles.xml"));
        QVERIFY2(styles.contains("w:after=\"480\""), styles.constData());
        QFile::remove(normalTemplatePath());
    }

    void pageSetupSetAsDefaultForAllNewDocumentsWritesTheNormalTemplate()
    {
        QFile::remove(normalTemplatePath());
        setDefaultScopeStandIn([](const QString &) { return DefaultScope::AllNew; });
        auto doc = open(makeDocx(para(run("text"))));
        withDialog([&doc] { pageSetupDialog(nullptr, doc.get(), 0); }, QStringLiteral("pageSetupDialog"), [](QDialog *d) {
            d->findChild<QDoubleSpinBox *>(QStringLiteral("left"))->setValue(30);
            shotOf(d, QStringLiteral("write-page-setup-dialog"));
            d->findChild<QPushButton *>(QStringLiteral("setAsDefault"))->click();
        }, false);
        setDefaultScopeStandIn(nullptr);
        QVERIFY(QFile::exists(normalTemplatePath()));
        QCOMPARE(pageSetupOf(newBlankDocument()->sectionProps(0), false).left, 30.0);
        QCOMPARE(pageSetupOf(doc->sectionProps(0), false).left, 30.0);   // (and this document, as OK does)
        QFile::remove(normalTemplatePath());
    }

    // ---- the Normal template: what can go wrong with the file
    static void putTemplateFile(const QByteArray &bytes)
    {
        QFile f(normalTemplatePath());
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(bytes);
    }

    void aCorruptNormalTemplateFallsBackToTheProgramsOwnSettingsWithANote()
    {
        putTemplateFile("this is not a document");
        QCOMPARE(int(normalTemplateState()), int(NormalTemplateState::Unreadable));
        QString note;
        auto doc = newBlankDocument(&note);
        QVERIFY(doc);
        QVERIFY(note.contains(QStringLiteral("Normal template")));
        QCOMPARE(pageSetupOf(doc->sectionProps(0), false).left, 72.0);
        WriteWindow w;
        QVERIFY2(w.statusBar()->currentMessage().contains(QStringLiteral("Normal template")), qPrintable(w.statusBar()->currentMessage()));
        QFile::remove(normalTemplatePath());
    }

    void aNormalTemplateOverFiftyMegabytesIsNotUsed()
    {
        QFile::remove(normalTemplatePath());
        {
            QFile f(normalTemplatePath());
            QVERIFY(f.open(QIODevice::WriteOnly));
            QVERIFY(f.resize(kMaxNormalTemplateBytes + 1));
        }
        QCOMPARE(int(normalTemplateState()), int(NormalTemplateState::Unreadable));
        QString note;
        QVERIFY(newBlankDocument(&note));
        QVERIFY(!note.isEmpty());
        QFile::remove(normalTemplatePath());
    }

    void aNormalTemplateDeletedWhileTheProgramRunsFallsBackCleanly()
    {
        QFile::remove(normalTemplatePath());
        QVERIFY(changeNormalTemplate([](WriteDoc &d) {
            office::wml::SectionProps p = d.sectionProps(0);
            p.left = 720;
            d.setSectionProps(0, p);
        }));
        WriteWindow first;
        QCOMPARE(pageSetupOf(first.document()->sectionProps(0), false).left, 36.0);
        QVERIFY(QFile::remove(normalTemplatePath()));
        QString note;
        auto fresh = newBlankDocument(&note);
        QVERIFY(fresh && note.isEmpty());
        QCOMPARE(pageSetupOf(fresh->sectionProps(0), false).left, 72.0);
        WriteWindow second;
        QCOMPARE(pageSetupOf(second.document()->sectionProps(0), false).left, 72.0);
        QVERIFY(changeNormalTemplate([](WriteDoc &) {}));   // (made again)
        QVERIFY(QFile::exists(normalTemplatePath()));
        QFile::remove(normalTemplatePath());
    }

    void aReadOnlyNormalTemplateStartsDocumentsButIsNotChangedAndSaysWhy()
    {
        QFile::remove(normalTemplatePath());
        QVERIFY(changeNormalTemplate([](WriteDoc &d) {
            office::wml::SectionProps p = d.sectionProps(0);
            p.left = 720;
            d.setSectionProps(0, p);
        }));
        const QString folder = QFileInfo(normalTemplatePath()).absolutePath();
        const auto folderWas = QFile::permissions(folder);
        QFile::setPermissions(normalTemplatePath(), QFileDevice::ReadOwner);
        QFile::setPermissions(folder, QFileDevice::ReadOwner | QFileDevice::ExeOwner);
        struct Restore {
            QString folder;
            QFileDevice::Permissions was;
            ~Restore()
            {
                QFile::setPermissions(folder, was);
                QFile::setPermissions(normalTemplatePath(), QFileDevice::ReadOwner | QFileDevice::WriteOwner);
                QFile::remove(normalTemplatePath());
            }
        } restore{folder, folderWas};
        QCOMPARE(pageSetupOf(newBlankDocument()->sectionProps(0), false).left, 36.0);   // (it is still read)
        QString error;
        const bool changed = changeNormalTemplate([](WriteDoc &) {}, &error);
        if (changed) QSKIP("this account can write to a read-only folder");
        QVERIFY(!error.isEmpty());
        QString shown;
        setNormalTemplatePrompts(nullptr, [&shown](const QString &text) { shown = text; });
        QVERIFY(!setNormalTemplate(nullptr, [](WriteDoc &) {}));
        setNormalTemplatePrompts(nullptr, nullptr);
        QVERIFY2(shown.startsWith(QStringLiteral("couldn't save the default: ")), qPrintable(shown));
    }

    void aTemplateKeepsWhatItHoldsButStartsOverItsPropertiesCommentsAndChanges()
    {
        QFile::remove(normalTemplatePath());
        auto t = open(makeDocx(para(run("kept words")) + "<w:p><w:ins w:id=\"1\" w:author=\"A\" w:date=\"2026-01-01T00:00:00Z\"><w:r><w:t>tracked words</w:t></w:r></w:ins></w:p>"));
        QTextCursor c(t->story());
        c.setPosition(2);
        c.setPosition(4, QTextCursor::KeepAnchor);
        QVERIFY(addComment(*t, c, QStringLiteral("a note")) >= 0);
        office::PackageProps props;
        props.creator = QStringLiteral("Someone Else");
        props.title = QStringLiteral("Old title");
        props.created = QDateTime(QDate(2020, 1, 2), QTime(3, 4), Qt::UTC);
        t->setPackageProps(props);
        office::wml::SectionProps p = t->sectionProps(0);
        p.left = 720;
        t->setSectionProps(0, p);
        const QByteArray bytes = t->toDocx();
        QVERIFY(!bytes.isEmpty());
        // (the file does have them)
        auto raw = WriteDoc::fromDocx(bytes);
        QVERIFY(raw && !comments(*raw).isEmpty() && revisionCount(raw->story()) > 0 && raw->packageProps().creator == QStringLiteral("Someone Else"));
        putTemplateFile(bytes);
        auto fresh = newBlankDocument();
        QVERIFY(fresh);
        const QString text = fresh->story()->toPlainText();
        QVERIFY2(text.contains(QStringLiteral("kept words")) && text.contains(QStringLiteral("tracked words")), qPrintable(text));
        QCOMPARE(pageSetupOf(fresh->sectionProps(0), false).left, 36.0);
        QVERIFY(comments(*fresh).isEmpty());
        QCOMPARE(revisionCount(fresh->story()), 0);
        const office::PackageProps now = fresh->packageProps();
        QVERIFY(now.creator.isEmpty() && now.title.isEmpty() && !now.created.isValid());
        QVERIFY(!fresh->isModified());
        QFile::remove(normalTemplatePath());
    }

    // An unreadable template is not replaced unasked: the person is asked, and a No leaves it as it is.
    void anUnreadableNormalTemplateIsReplacedOnlyWhenThePersonSaysYes()
    {
        putTemplateFile("damaged");
        int asked = 0;
        setNormalTemplatePrompts([&asked] {
            ++asked;
            return false;
        }, nullptr);
        QVERIFY(!setNormalTemplate(nullptr, [](WriteDoc &) {}));
        QCOMPARE(asked, 1);
        QCOMPARE(int(normalTemplateState()), int(NormalTemplateState::Unreadable));
        setNormalTemplatePrompts([&asked] {
            ++asked;
            return true;
        }, nullptr);
        QVERIFY(setNormalTemplate(nullptr, [](WriteDoc &) {}));
        QCOMPARE(asked, 2);
        QCOMPARE(int(normalTemplateState()), int(NormalTemplateState::Usable));
        setNormalTemplatePrompts(nullptr, nullptr);
        QFile::remove(normalTemplatePath());
    }

    // Two windows changing the template at once do not lose an update: a change waits for the file's lock, and then starts from what the other left.
    void aChangeWaitsForTheLockAndKeepsTheOtherWindowsChange()
    {
        QFile::remove(normalTemplatePath());
        // The other window's change, made ready: a template with a top margin of its own.
        auto theirs = WriteDoc::blank();
        office::wml::SectionProps top = theirs->sectionProps(0);
        top.top = 360;
        theirs->setSectionProps(0, top);
        const QByteArray theirBytes = theirs->toDocx();
        QLockFile other(normalTemplatePath() + QStringLiteral(".lock"));
        QVERIFY(other.tryLock());
        std::thread finish([&other, theirBytes] {
            QThread::msleep(400);
            putTemplateFile(theirBytes);   // (the other window saves, then lets go of the lock)
            other.unlock();
        });
        QElapsedTimer waited;
        waited.start();
        const bool changed = changeNormalTemplate([](WriteDoc &d) {
            office::wml::SectionProps p = d.sectionProps(0);
            p.right = 1080;
            d.setSectionProps(0, p);
        });
        finish.join();
        QVERIFY(changed);
        QVERIFY2(waited.elapsed() >= 300, qPrintable(QString::number(waited.elapsed())));
        // (it read the file once the lock was let go: both changes are there)
        const SectionSetup now = pageSetupOf(newBlankDocument()->sectionProps(0), false);
        QCOMPARE(now.top, 18.0);
        QCOMPARE(now.right, 54.0);
        QFile::remove(normalTemplatePath());
    }

    // Set as Default for all new documents carries Book fold too (and the mirrored margins it brings).
    void pageSetupSetAsDefaultCarriesBookFold()
    {
        QFile::remove(normalTemplatePath());
        setDefaultScopeStandIn([](const QString &) { return DefaultScope::AllNew; });
        auto doc = open(makeDocx(para(run("text"))));
        withDialog([&doc] { pageSetupDialog(nullptr, doc.get(), 0); }, QStringLiteral("pageSetupDialog"), [](QDialog *d) {
            auto *multiple = d->findChild<QComboBox *>(QStringLiteral("multiplePages"));
            multiple->setCurrentIndex(multiple->findData(true));
            d->findChild<QComboBox *>(QStringLiteral("sheetsPerBooklet"))->setCurrentIndex(d->findChild<QComboBox *>(QStringLiteral("sheetsPerBooklet"))->findData(8));
            d->findChild<QPushButton *>(QStringLiteral("setAsDefault"))->click();
        }, false);
        setDefaultScopeStandIn(nullptr);
        auto fresh = newBlankDocument();
        QVERIFY(fresh->bookFold());
        QCOMPARE(fresh->bookFoldSheets(), 8);
        QVERIFY(fresh->mirrorMargins());
        QFile::remove(normalTemplatePath());
    }

    // Save is the page every app shares: the default format, the recovery interval, and the default folder.
    void optionsSaveIsTheSharedPage()
    {
        WriteWindow w;
        bool all = false;
        QStringList kinds;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            auto *box = d->findChild<QComboBox *>(QStringLiteral("defaultFormat"));
            all = box && d->findChild<QSpinBox *>(QStringLiteral("autoRecoverMinutes")) && d->findChild<QLineEdit *>(QStringLiteral("defaultFolder"));
            if (all)
                for (int i = 0; i < box->count(); ++i) kinds << box->itemData(i).toString();
            if (auto *tabs = d->findChild<QTabWidget *>()) {
                for (int i = 0; i < tabs->count(); ++i)
                    if (tabs->tabText(i) == QLatin1String("Save")) tabs->setCurrentIndex(i);
            }
            shotOf(d, QStringLiteral("write-options-save"));
            d->reject();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QVERIFY(all);
        QCOMPARE(kinds, (QStringList{QStringLiteral("docx"), QStringLiteral("odt")}));
    }

    void pageSetupOffersBookFoldWithSheetsPerBooklet()
    {
        auto doc = open(makeDocx(para(run("text"))));
        QVERIFY(!doc->bookFold());
        withDialog([&doc] { pageSetupDialog(nullptr, doc.get(), 0); }, QStringLiteral("pageSetupDialog"), [](QDialog *d) {
            auto *multiple = d->findChild<QComboBox *>(QStringLiteral("multiplePages"));
            auto *sheets = d->findChild<QComboBox *>(QStringLiteral("sheetsPerBooklet"));
            QVERIFY(multiple && sheets);
            QVERIFY(!sheets->isEnabled());
            QCOMPARE(sheets->itemText(0), QStringLiteral("All"));
            QCOMPARE(sheets->count(), 11);   // All, then 4 to 40 in fours
            QCOMPARE(sheets->itemData(1).toInt(), 4);
            QCOMPARE(sheets->itemData(10).toInt(), 40);
            multiple->setCurrentIndex(multiple->findText(QStringLiteral("Book fold")));
            QVERIFY(sheets->isEnabled());
            sheets->setCurrentIndex(sheets->findData(8));
            shotOf(d, QStringLiteral("write-page-setup-book-fold"));
        });
        QVERIFY(doc->bookFold());
        QCOMPARE(doc->bookFoldSheets(), 8);
        // Back to normal.
        withDialog([&doc] { pageSetupDialog(nullptr, doc.get(), 0); }, QStringLiteral("pageSetupDialog"), [](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("sheetsPerBooklet"))->currentData().toInt(), 8);
            d->findChild<QComboBox *>(QStringLiteral("multiplePages"))->setCurrentIndex(0);
        });
        QVERIFY(!doc->bookFold());
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

    // A window with two numbered lists (A1, A2 and B1, B2, with plain paragraphs between) and the two B items, with their paragraph marks, copied; the caret is in the empty paragraph after A2.
    void windowWithTwoLists(WriteWindow &w)
    {
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("A1")) + para(run("A2")) + para(QByteArray()) + para(run("gap")) + para(run("B1")) + para(run("B2")) + para(run("tail")))), QString());
        QTextDocument *s = w.document()->story();
        auto range = [&](int a, int b) {
            QTextCursor c(s);
            c.setPosition(s->findBlockByNumber(a).position());
            c.setPosition(s->findBlockByNumber(b).position(), QTextCursor::KeepAnchor);
            return c;
        };
        for (const auto &pair : {std::pair<int, int>(0, 1), std::pair<int, int>(4, 5)}) {
            QTextCursor c = range(pair.first, pair.second);
            c.setPosition(s->findBlockByNumber(pair.second).position() + 1, QTextCursor::KeepAnchor);
            toggleList(*w.document(), c, true);
        }
        w.view()->setCursor(range(4, 6));
        w.view()->copy();
        w.view()->setCursor(range(2, 2));
    }

    // The numbers the list items of the blocks `first` on show (the first number of the list, and the item's place in it).
    static QVector<int> listNumbers(WriteWindow &w, int first, int count)
    {
        QVector<int> out;
        const QTextDocument *s = w.document()->story();
        for (int i = first; i < first + count; ++i) {
            const QTextBlock b = s->findBlockByNumber(i);
            out << (b.textList() ? b.textList()->itemNumber(b) + b.textList()->format().start() : 0);
        }
        return out;
    }

    // A window with the list A1, A2, A3, a gap, and the list B1, B2; the two B items, with their paragraph marks, are copied; the caret is at the start of A2.
    void windowWithAListToPasteInto(WriteWindow &w)
    {
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("A1")) + para(run("A2")) + para(run("A3")) + para(QByteArray()) + para(run("B1")) + para(run("B2")) + para(run("tail")))), QString());
        QTextDocument *s = w.document()->story();
        auto range = [&](int a, int b) {
            QTextCursor c(s);
            c.setPosition(s->findBlockByNumber(a).position());
            c.setPosition(s->findBlockByNumber(b).position(), QTextCursor::KeepAnchor);
            return c;
        };
        for (const auto &pair : {std::pair<int, int>(0, 2), std::pair<int, int>(4, 5)}) {
            QTextCursor c = range(pair.first, pair.second);
            c.setPosition(s->findBlockByNumber(pair.second).position() + 1, QTextCursor::KeepAnchor);
            toggleList(*w.document(), c, true);
        }
        w.view()->setCursor(range(4, 6));
        w.view()->copy();
        w.view()->setCursor(range(1, 1));
    }

    void continuedListItemsJoinTheListSoTheNumbersFollowEveryLaterEdit()
    {
        jp::Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), true);
        // Continue: the items go in the list beside them: one list, numbered 1 to 5 with no number twice, and a later edit renumbers.
        {
            WriteWindow w;
            windowWithAListToPasteInto(w);
            w.act(QStringLiteral("edit.pasteListContinue"))->trigger();
            QCOMPARE(w.document()->story()->toPlainText().left(15), QStringLiteral("A1\n\nB1\nB2\nA2\nA3"));   // (the paragraph pasted into stays, a list item of its own)
            QCOMPARE(listNumbers(w, 0, 6), (QVector<int>{1, 2, 3, 4, 5, 6}));
            const QTextDocument *s = w.document()->story();
            for (int i = 1; i < 6; ++i) QCOMPARE(s->findBlockByNumber(i).blockFormat().intProperty(prop::NumId), s->findBlockByNumber(0).blockFormat().intProperty(prop::NumId));
            QTextCursor gone(w.document()->story());
            gone.setPosition(s->findBlockByNumber(2).position());
            gone.setPosition(s->findBlockByNumber(3).position(), QTextCursor::KeepAnchor);
            removeSelection(*w.document(), gone);
            QCOMPARE(listNumbers(w, 0, 5), (QVector<int>{1, 2, 3, 4, 5}));
        }
        // New: a list of its own that begins at 1; the list it was put in goes on by itself.
        {
            WriteWindow w;
            windowWithAListToPasteInto(w);
            w.act(QStringLiteral("edit.pasteListNew"))->trigger();
            QCOMPARE(listNumbers(w, 0, 6), (QVector<int>{1, 1, 1, 2, 2, 3}));   // (A1; the paragraph pasted into and the B items, each list begun at 1; A2 and A3)
        }
        // Merge: the look of the list beside them, so the numbers go on too.
        {
            WriteWindow w;
            windowWithAListToPasteInto(w);
            w.act(QStringLiteral("edit.pasteListMerge"))->trigger();
            QCOMPARE(listNumbers(w, 0, 6), (QVector<int>{1, 2, 3, 4, 5, 6}));
        }
    }

    // Ctrl+V pastes as it always did: list items keep the list they came from, a tab in the text is a tab, and a table in HTML comes in with its links.
    void ordinaryPasteDoesNothingTheChoicesDo()
    {
        jp::Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), true);
        {
            WriteWindow w;
            windowWithTwoLists(w);
            w.view()->paste();
            const QTextDocument *s = w.document()->story();
            QCOMPARE(s->toPlainText(), QStringLiteral("A1\nA2\n\nB1\nB2\n\ngap\nB1\nB2\ntail"));
            QCOMPARE(s->findBlockByNumber(3).blockFormat().intProperty(prop::NumId), s->findBlockByNumber(7).blockFormat().intProperty(prop::NumId));
            QVERIFY(s->findBlockByNumber(3).blockFormat().intProperty(prop::NumId) != s->findBlockByNumber(1).blockFormat().intProperty(prop::NumId));
            QVERIFY(w.view()->lastPasteHasListItems());
        }
        // Tab-indented words are words: no table, and no parse of the clipboard.
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            w.setDocument(open(makeDocx(para(QByteArray()))), QString());
            auto *mime = new QMimeData;
            mime->setText(QStringLiteral("a\tb\nc\td\n"));
            QApplication::clipboard()->setMimeData(mime);
            const int parses = office::wml::clipTableParses();
            w.view()->paste();
            QCOMPARE(cellTexts(w), QStringList());
            QVERIFY(w.document()->story()->toPlainText().startsWith(QStringLiteral("a\tb")));
            QCOMPARE(office::wml::clipTableParses(), parses);
            QVERIFY(w.view()->pasteOptionsAvailable());
            QVERIFY(!w.view()->lastPasteWasCells());
        }
        // HTML that is one formatted table with a link stays what the HTML says (no cells of plain words).
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            w.setDocument(open(makeDocx(para(QByteArray()))), QString());
            auto *mime = new QMimeData;
            mime->setHtml(QStringLiteral("<html><body><table><tr><td><a href=\"https://example.com/x\">linked</a></td><td><b>strong</b></td></tr><tr><td>c</td><td>d</td></tr></table></body></html>"));
            QApplication::clipboard()->setMimeData(mime);
            w.view()->paste();
            QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("linked")));
            bool linked = false;
            for (QTextBlock b = w.document()->story()->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it)
                    if (it.fragment().charFormat().isAnchor() && it.fragment().text().contains(QStringLiteral("linked"))) linked = true;
            QVERIFY(linked);
            QVERIFY(!w.view()->lastPasteWasCells());
        }
    }

    // The Paste Options button of the last paste, its menu open.
    static void openOptions(WriteWindow &w, QMenu **menu)
    {
        auto *button = w.view()->findChild<QToolButton *>(QStringLiteral("pasteOptionsButton"));
        QVERIFY(button);
        QTRY_VERIFY(button->isVisible());
        *menu = button->menu();
        (*menu)->popup(button->mapToGlobal(QPoint(0, button->height())));
        QTRY_VERIFY((*menu)->isVisible());
    }

    void pastedListItemsContinueStartOverOrJoinTheListBeside()
    {
        jp::Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), true);
        struct Outcome { int start, numId, listIsBesides; QString text; };
        auto after = [](WriteWindow &w) {
            QTextDocument *s = w.document()->story();
            const QTextBlock b = s->findBlockByNumber(3);
            const int besideId = s->findBlockByNumber(1).blockFormat().intProperty(prop::NumId);
            return Outcome{b.textList() ? b.textList()->format().start() : -1, b.blockFormat().intProperty(prop::NumId) == besideId ? 1 : 0, b.textList() == s->findBlockByNumber(1).textList() ? 1 : 0, s->toPlainText()};
        };
        const QString plain = QStringLiteral("A1\nA2\n\nB1\nB2\n\ngap\nB1\nB2\ntail");
        // Continue and Merge: the pasted items are items of the list beside them; New: a list of its own that begins at 1.
        {
            WriteWindow w;
            windowWithTwoLists(w);
            const QString before = w.document()->story()->toPlainText();
            w.act(QStringLiteral("edit.pasteListContinue"))->trigger();
            const Outcome o = after(w);
            QCOMPARE(o.text, plain);
            QCOMPARE(o.numId, 1);
            QCOMPARE(o.listIsBesides, 1);
            QVERIFY(w.view()->lastPasteHasListItems());
            // One Undo takes it all back, and Redo does it again.
            QVERIFY(w.document()->undo());
            QCOMPARE(w.document()->story()->toPlainText(), before);
            QVERIFY(w.document()->redo());
            QCOMPARE(after(w).listIsBesides, 1);
            QCOMPARE(w.document()->story()->toPlainText(), plain);
        }
        {
            WriteWindow w;
            windowWithTwoLists(w);
            w.act(QStringLiteral("edit.pasteListNew"))->trigger();
            const Outcome o = after(w);
            QCOMPARE(o.text, plain);
            QCOMPARE(o.start, 1);
            QCOMPARE(o.listIsBesides, 0);
            QVERIFY(w.document()->undo());
        }
        {
            WriteWindow w;
            windowWithTwoLists(w);
            const QString before = w.document()->story()->toPlainText();
            w.act(QStringLiteral("edit.pasteListMerge"))->trigger();
            const Outcome o = after(w);
            QCOMPARE(o.text, plain);
            QCOMPARE(o.numId, 1);
            QCOMPARE(o.listIsBesides, 1);
            QVERIFY(w.document()->undo());
            QCOMPARE(w.document()->story()->toPlainText(), before);
            QVERIFY(w.document()->redo());
            QCOMPARE(after(w).listIsBesides, 1);
        }
        // The Paste Options button offers the three, and a choice redoes the paste as one step.
        {
            WriteWindow w;
            windowWithTwoLists(w);
            w.view()->paste();
            QMenu *menu = nullptr;
            openOptions(w, &menu);
            QVERIFY(menu->findChild<QAction *>(QStringLiteral("pasteList%1").arg(int(ListPaste::Merge))));
            QVERIFY(menu->findChild<QAction *>(QStringLiteral("pasteList%1").arg(int(ListPaste::Continue))));
            QVERIFY(menu->findChild<QAction *>(QStringLiteral("pasteList%1").arg(int(ListPaste::New))));
            menu->close();
            QVERIFY(w.view()->changeLastPasteList(ListPaste::Merge));
            QCOMPARE(after(w).listIsBesides, 1);
            QVERIFY(w.view()->changeLastPasteList(ListPaste::New));
            QCOMPARE(after(w).start, 1);
            QVERIFY(w.document()->undo());
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("A1\nA2\n\ngap\nB1\nB2\ntail"));
        }
    }

    // The clipboard holds spreadsheet cells as an HTML table and as tab-separated lines.
    static void putCells(const QStringList &rows)
    {
        auto *mime = new QMimeData;
        QString html = QStringLiteral("<html><body><table>"), text;
        for (const QString &row : rows) {
            html += QStringLiteral("<tr>");
            for (const QString &cell : row.split(QLatin1Char('|'))) html += QStringLiteral("<td>") + cell + QStringLiteral("</td>");
            html += QStringLiteral("</tr>");
            text += QString(row).replace(QLatin1Char('|'), QLatin1Char('\t')) + QLatin1Char('\n');
        }
        mime->setHtml(html + QStringLiteral("</table></body></html>"));
        mime->setText(text);
        QApplication::clipboard()->setMimeData(mime);
    }

    static QStringList cellTexts(WriteWindow &w)
    {
        QStringList out;
        for (QTextBlock b = w.document()->story()->begin(); b.isValid(); b = b.next())
            if (cellKeyOf(b)) out << b.text();
        return out;
    }

    // The second table begins before the first cell ends: it is in a cell.
    static bool tableIsNested(WriteWindow &w)
    {
        office::Package p;
        p.read(w.document()->toDocx());
        const QByteArray xml = p.part(QStringLiteral("/word/document.xml"));
        const qsizetype second = xml.indexOf("<w:tbl>", xml.indexOf("<w:tbl>") + 1);
        return second > 0 && second < xml.indexOf("</w:tc>");
    }

    static QByteArray bodyXml(const QByteArray &docx)
    {
        office::Package p;
        p.read(docx);
        return p.part(QStringLiteral("/word/document.xml"));
    }

    static int count(WriteWindow &w, const char *what) { return int(bodyXml(w.document()->toDocx()).count(what)); }

    // A window with one table of rows by columns, the caret in its first cell.
    static void tableWindow(WriteWindow &w, int rows = 2, int columns = 2)
    {
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("top")))), QString());
        QTextCursor at(w.document()->story());
        at.setPosition(0);
        NewTable t;
        t.rows = rows;
        t.columns = columns;
        QVERIFY(insertTable(*w.document(), at, t));
        w.view()->setCursor(at);
    }

    static void toFirstCell(WriteWindow &w)
    {
        QTextCursor first(w.document()->story());
        for (QTextBlock b = w.document()->story()->begin(); b.isValid(); b = b.next())
            if (cellKeyOf(b)) {
                first.setPosition(b.position());
                break;
            }
        w.view()->setCursor(first);
    }

    void pastedSpreadsheetCellsMakeATableAndInATableTheChoicesApply()
    {
        // Outside a table: the Table choice makes one, in one step.
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            w.setDocument(open(makeDocx(para(run("before")) + para(QByteArray()))), QString());
            putCells({QStringLiteral("a|b"), QStringLiteral("c|d")});
            QTextCursor c(w.document()->story());
            c.setPosition(w.document()->story()->findBlockByNumber(1).position());
            w.view()->setCursor(c);
            w.act(QStringLiteral("edit.pasteTable"))->trigger();
            QCOMPARE(cellTexts(w), (QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c"), QStringLiteral("d")}));
            QCOMPARE(count(w, "<w:tr>"), 2);
            QVERIFY(w.view()->lastPasteWasCells());
            QVERIFY(w.view()->pasteOptionsAvailable());
            QVERIFY(w.document()->undo());
            QCOMPARE(cellTexts(w), QStringList());
            QVERIFY(w.document()->redo());
            QCOMPARE(count(w, "<w:tr>"), 2);
        }
        // One cell is text.
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            w.setDocument(open(makeDocx(para(QByteArray()))), QString());
            putCells({QStringLiteral("only")});
            w.view()->paste();
            QCOMPARE(cellTexts(w), QStringList());
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("only\n"));
            w.view()->pasteCellsAs(TablePaste::NewTable);
            QCOMPARE(cellTexts(w), QStringList());
        }
        // In a table.
        {
            WriteWindow w;
            tableWindow(w);
            putCells({QStringLiteral("a|b|c"), QStringLiteral("d|e|f")});
            // Nest Table: a table in the cell, with a paragraph after it.
            w.act(QStringLiteral("edit.pasteTableNest"))->trigger();
            QCOMPARE(count(w, "<w:tbl>"), 2);
            QVERIFY(tableIsNested(w));
            QCOMPARE(cellTexts(w).count(QStringLiteral("a")), 1);
            QVERIFY(w.view()->lastPasteWasCells() && w.view()->lastPasteStartedInTable());
            QVERIFY(w.document()->undo());
            QCOMPARE(count(w, "<w:tbl>"), 1);
            QVERIFY(w.document()->redo());
            QCOMPARE(count(w, "<w:tbl>"), 2);
            QVERIFY(w.document()->undo());
            toFirstCell(w);
            w.view()->paste();   // (Ctrl+V: the words, as ever)
            QVERIFY(w.view()->pasteOptionsAvailable());
            QCOMPARE(count(w, "<w:tbl>"), 1);
            // The same choices as the options button's, one at a time (the first from the plain paste of the same words).
            QVERIFY(w.view()->changeLastPasteCells(TablePaste::NewRows));
            QCOMPARE(count(w, "<w:tbl>"), 1);
            QCOMPARE(count(w, "<w:tr>"), 4);
            QCOMPARE(count(w, "<w:gridCol "), 2);
            QVERIFY(w.view()->changeLastPasteCells(TablePaste::Merge));
            QCOMPARE(count(w, "<w:tr>"), 4);
            QCOMPARE(count(w, "<w:gridCol "), 3);
            QVERIFY(w.view()->changeLastPasteCells(TablePaste::Overwrite));
            QCOMPARE(count(w, "<w:tr>"), 2);
            QCOMPARE(count(w, "<w:gridCol "), 2);
            // (Three columns on two: the third is left out.)
            QCOMPARE(cellTexts(w), (QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("d"), QStringLiteral("e")}));
            QVERIFY(!w.document()->tableNotice.isEmpty());
            QVERIFY(w.document()->undo());
            QCOMPARE(count(w, "<w:tr>"), 2);
            QVERIFY(cellTexts(w).first().isEmpty());
        }
        // The commands: only where they fit.
        {
            WriteWindow w;
            tableWindow(w);
            putCells({QStringLiteral("1|2"), QStringLiteral("3|4")});
            w.act(QStringLiteral("edit.pasteTableMerge"))->trigger();
            QCOMPARE(count(w, "<w:tr>"), 4);
            w.act(QStringLiteral("edit.pasteTable"))->trigger();   // (in a table: a table needs the caret out of one)
            QCOMPARE(count(w, "<w:tr>"), 4);
        }
    }

    void aPastedTableHasAtMostFiveHundredRowsAndHostileCellTextIsCutAndSaid()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(QByteArray()))), QString());
        QStringList rows;
        for (int r = 0; r < 520; ++r) rows << QString::number(r) + QStringLiteral("|") + QString(r == 0 ? 5000 : 3, QLatin1Char('w'));
        putCells(rows);
        w.view()->pasteCellsAs(TablePaste::NewTable);
        QCOMPARE(count(w, "<w:tr>"), 500);
        QCOMPARE(count(w, "<w:tc>"), 1000);
        QCOMPARE(cellTexts(w).at(1).size(), 4000);
        // What was cut is said.
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("left out")));
        QVERIFY(w.document()->tableNotice.contains(QStringLiteral("500")));
    }

    // A table nested again and again stops where a saved file still reads it as tables (kMaxTableDepth), and one whose columns would be slivers is refused; a refusal changes nothing.
    void nestingStopsAtTheDepthAFileReadsAndAtSliverColumns()
    {
        {
            WriteWindow w;
            tableWindow(w);
            putCells({QStringLiteral("a"), QStringLiteral("b")});
            for (int i = 0; i < 12; ++i) w.view()->pasteCellsAs(TablePaste::Nest);
            QCOMPARE(count(w, "<w:tbl>"), office::wml::kMaxTableDepth);
            QVERIFY(!w.document()->tableNotice.isEmpty());
            // It opens again with every table a table (none a stand-in).
            auto again = open(w.document()->toDocx());
            QCOMPARE(int(bodyXml(again->toDocx()).count("<w:tbl>")), office::wml::kMaxTableDepth);
        }
        {
            WriteWindow w;
            tableWindow(w);
            QStringList wide;
            for (int r = 0; r < 2; ++r) wide << QStringLiteral("x|").repeated(19) + QStringLiteral("x");
            putCells(wide);
            const QByteArray before = w.document()->toDocx();
            w.view()->pasteCellsAs(TablePaste::Nest);
            QCOMPARE(count(w, "<w:tbl>"), 1);
            QCOMPARE(w.document()->toDocx(), before);
            QVERIFY(!w.document()->tableNotice.isEmpty());
            QVERIFY(!w.view()->lastPasteWasCells());   // (a paste that did nothing is not one to offer choices for)
            putCells({QStringLiteral("a|b|c"), QStringLiteral("d|e|f")});
            w.view()->pasteCellsAs(TablePaste::Nest);
            QCOMPARE(count(w, "<w:tbl>"), 2);
        }
    }

    // Every limit is looked at before anything is changed: a choice that does not fit leaves the document as it was.
    void aPasteOfCellsThatDoesNotFitChangesNothing()
    {
        WriteWindow w;
        tableWindow(w, 300, 2);
        QStringList rows;
        for (int r = 0; r < 500; ++r) rows << QStringLiteral("x|").repeated(62) + QStringLiteral("x");
        putCells(rows);
        const QByteArray before = w.document()->toDocx();
        const int revision = w.document()->story()->revision();
        w.view()->pasteCellsAs(TablePaste::Merge);   // (300 + 500 rows of 63 cells are more than a table can hold)
        QCOMPARE(w.document()->toDocx(), before);
        QCOMPARE(count(w, "<w:gridCol "), 2);
        QCOMPARE(w.document()->story()->revision(), revision);
        QVERIFY(!w.document()->tableNotice.isEmpty());
        QVERIFY(!w.view()->lastPasteWasCells());
        // A selection that a table would replace stays when the cells are too many for a table.
        office::wml::ClipTable big;
        for (int r = 0; r < 501; ++r) big.rows.push_back(QStringList{QStringLiteral("a"), QStringLiteral("b")});
        WriteWindow v;
        v.setNotifier([](const QString &, const QString &) {});
        v.setDocument(open(makeDocx(para(run("keep this")))), QString());
        QTextCursor sel(v.document()->story());
        sel.select(QTextCursor::Document);
        QVERIFY(!pasteCells(*v.document(), sel, big, TablePaste::NewTable));
        QCOMPARE(v.document()->story()->toPlainText(), QStringLiteral("keep this"));
    }

    // Overwrite Cells puts a cell's words in place of all it held (several paragraphs, a table in it), and a cell that spans columns takes the first of the cells' words that are over it.
    void overwritingCellsReplacesWholeCellsAndMapsMergedColumns()
    {
        auto tc = [](const QByteArray &content, int span = 1) {
            return "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(2400 * span) + "\" w:type=\"dxa\"/>" + (span > 1 ? "<w:gridSpan w:val=\"" + QByteArray::number(span) + "\"/>" : QByteArray()) + "</w:tcPr>" + content + "</w:tc>";
        };
        auto tbl = [](int columns, const QByteArray &rows) {
            QByteArray grid;
            for (int i = 0; i < columns; ++i) grid += "<w:gridCol w:w=\"2400\"/>";
            return "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid>" + grid + "</w:tblGrid>" + rows + "</w:tbl>";
        };
        auto tr = [](const QByteArray &cells) { return "<w:tr>" + cells + "</w:tr>"; };
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            const QByteArray inner = tbl(1, tr(tc(para(run("n1")))) + tr(tc(para(run("n2")))));
            w.setDocument(open(makeDocx(tbl(2, tr(tc(para(run("x")) + para(run("y")) + inner + para(run("z"))) + tc(para(run("old")))) + tr(tc(para(run("c"))) + tc(para(run("d"))))) + para(run("end")))), QString());
            toFirstCell(w);
            putCells({QStringLiteral("a|b"), QStringLiteral("e|f")});
            w.view()->pasteCellsAs(TablePaste::Overwrite);
            QCOMPARE(cellTexts(w), (QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("e"), QStringLiteral("f")}));
            QCOMPARE(count(w, "<w:tbl>"), 1);
            const QByteArray xml = bodyXml(w.document()->toDocx());
            for (qsizetype at = xml.indexOf("</w:tc>"); at >= 0; at = xml.indexOf("</w:tc>", at + 1)) QCOMPARE(xml.mid(at - 6, 6), QByteArray("</w:p>"));
            QVERIFY(w.document()->undo());
            QCOMPARE(count(w, "<w:tbl>"), 2);
            QVERIFY(cellTexts(w).contains(QStringLiteral("n2")));
        }
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            w.setDocument(open(makeDocx(tbl(3, tr(tc(para(run("p"))) + tc(para(run("q")), 2)) + tr(tc(para(run("r"))) + tc(para(run("s"))) + tc(para(run("t"))))) + para(run("end")))), QString());
            toFirstCell(w);
            putCells({QStringLiteral("a|b|c"), QStringLiteral("d|e|f")});
            w.view()->pasteCellsAs(TablePaste::Overwrite);
            QCOMPARE(cellTexts(w), (QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("d"), QStringLiteral("e"), QStringLiteral("f")}));
            QVERIFY(w.document()->tableNotice.contains(QStringLiteral("left out")));   // (the c over the second half of the spanning cell)
        }
    }

    // Commands are on or off by a look at the clipboard that parses nothing; the cells are parsed once for all that is done with them, and the menu is built when it opens.
    void theClipboardOfCellsIsParsedOnceAndTheMenuBuiltWhenOpened()
    {
        WriteWindow w;
        tableWindow(w);
        const int parses = office::wml::clipTableParses();
        putCells({QStringLiteral("a|b"), QStringLiteral("c|d")});
        QTRY_VERIFY(w.act(QStringLiteral("edit.pasteTableNest"))->isEnabled());
        QCOMPARE(office::wml::clipTableParses(), parses);
        w.act(QStringLiteral("edit.pasteTableNest"))->trigger();
        QCOMPARE(office::wml::clipTableParses(), parses + 1);
        for (int i = 0; i < 3; ++i) QVERIFY(w.view()->canPasteCellsAs(TablePaste::Overwrite));
        QVERIFY(w.view()->clipboardHasCells());
        QVERIFY(w.view()->changeLastPasteCells(TablePaste::NewRows));
        QVERIFY(w.view()->changeLastPasteCells(TablePaste::Merge));
        QCOMPARE(office::wml::clipTableParses(), parses + 1);
        auto *button = w.view()->findChild<QToolButton *>(QStringLiteral("pasteOptionsButton"));
        QVERIFY(button);
        QVERIFY(button->menu()->actions().isEmpty());   // (nothing was built for a menu nobody opened)
        QMenu *menu = nullptr;
        openOptions(w, &menu);
        QVERIFY(!menu->actions().isEmpty());
        menu->close();
    }

    // What a choice pastes is what was pasted, whatever the clipboard holds by then.
    void aChoiceAfterAPasteUsesTheClipboardOfThePaste()
    {
        {
            WriteWindow w;
            tableWindow(w);
            putCells({QStringLiteral("a|b"), QStringLiteral("c|d")});
            w.view()->pasteCellsAs(TablePaste::Nest);
            auto *other = new QMimeData;
            other->setText(QStringLiteral("something else"));
            QApplication::clipboard()->setMimeData(other);
            QVERIFY(w.view()->changeLastPasteCells(TablePaste::NewRows));
            QCOMPARE(count(w, "<w:tr>"), 4);
            QVERIFY(cellTexts(w).contains(QStringLiteral("d")));
            QVERIFY(!w.document()->story()->toPlainText().contains(QStringLiteral("something")));
        }
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            w.setDocument(open(makeDocx(para(QByteArray()))), QString());
            auto *mime = new QMimeData;
            mime->setHtml(QStringLiteral("<p>some <b>bold</b> words</p>"));
            QApplication::clipboard()->setMimeData(mime);
            w.view()->paste();
            auto *other = new QMimeData;
            other->setText(QStringLiteral("something else"));
            QApplication::clipboard()->setMimeData(other);
            QVERIFY(w.view()->changeLastPaste(jp::write::PageView::PasteKind::TextOnly));
            QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("some bold words")));
            QVERIFY(!w.document()->story()->toPlainText().contains(QStringLiteral("something")));
        }
    }

    static QMimeData *rtfMime()
    {
        auto *mime = new QMimeData;
        mime->setData(QStringLiteral("text/rtf"), QByteArray("{\\rtf1\\ansi{\\fonttbl{\\f0 Arial;}}\\f0 plain \\b strong\\b0  words\\par}"));
        mime->setText(QStringLiteral("TEXTCOPY"));
        mime->setHtml(QStringLiteral("<p>HTMLCOPY</p>"));
        return mime;
    }

    static bool boldAt(WriteWindow &w, const QString &word)
    {
        const QTextDocument *s = w.document()->story();
        const int at = s->toPlainText().indexOf(word);
        QTextCursor c(const_cast<QTextDocument *>(s));
        c.setPosition(at + 1);
        return c.charFormat().fontWeight() >= QFont::DemiBold;
    }

    static void emptyWindow(WriteWindow &w)
    {
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(QByteArray()))), QString());
    }

    void formattedTextFromAnotherProgramPastesAsRtfThenHtmlThenText()
    {
        {   // RTF first, over the HTML and the text that come with it; its looks stay.
            WriteWindow w;
            emptyWindow(w);
            QApplication::clipboard()->setMimeData(rtfMime());
            w.view()->paste();
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("plain strong words"));
            QVERIFY(boldAt(w, QStringLiteral("strong")));
            QVERIFY(!boldAt(w, QStringLiteral("plain")));
            QVERIFY(w.view()->pasteOptionsAvailable());
            // The choices apply to it: keep text only, then merge formatting (which keeps the bold).
            QVERIFY(w.view()->changeLastPaste(jp::write::PageView::PasteKind::TextOnly));
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("TEXTCOPY"));
            QVERIFY(w.view()->changeLastPaste(jp::write::PageView::PasteKind::Merge));
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("plain strong words"));
            QVERIFY(w.view()->canPaste(jp::write::PageView::PasteKind::Rtf));
        }
        {   // one step of Undo
            WriteWindow w;
            emptyWindow(w);
            QApplication::clipboard()->setMimeData(rtfMime());
            w.view()->paste();
            QVERIFY(w.document()->undo());
            QCOMPARE(w.document()->story()->toPlainText(), QString());
            QVERIFY(!w.document()->canUndo());
        }
        {   // Without RTF: the HTML, then the text.
            WriteWindow w;
            emptyWindow(w);
            auto *mime = new QMimeData;
            mime->setText(QStringLiteral("TEXTCOPY"));
            mime->setHtml(QStringLiteral("<p>HTMLCOPY</p>"));
            QApplication::clipboard()->setMimeData(mime);
            QVERIFY(!w.view()->canPaste(jp::write::PageView::PasteKind::Rtf));
            w.view()->paste();
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("HTMLCOPY"));
        }
        {   // RTF that is not RTF is not read.
            WriteWindow w;
            emptyWindow(w);
            auto *mime = new QMimeData;
            mime->setData(QStringLiteral("text/rtf"), QByteArray("not rich text"));
            mime->setText(QStringLiteral("TEXTCOPY"));
            QApplication::clipboard()->setMimeData(mime);
            w.view()->paste();
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("TEXTCOPY"));
        }
    }

    void rtfIsReadFromEveryNameTheSystemsGiveItAndFromLinesEndedWithReturnAndLineFeed()
    {
        const QByteArray rtf("{\\rtf1\\ansi\r\n{\\fonttbl{\\f0 Arial;}}\r\n\\f0 plain \\b strong\\b0  words\\par}\r\n");
        for (const QString &name : {QStringLiteral("text/rtf"), QStringLiteral("application/rtf"), QStringLiteral("application/x-qt-windows-mime;value=\"Rich Text Format\"")}) {
            WriteWindow w;
            emptyWindow(w);
            auto *mime = new QMimeData;
            mime->setData(name, rtf);
            mime->setText(QStringLiteral("TEXTCOPY"));
            QApplication::clipboard()->setMimeData(mime);
            w.view()->paste();
            QVERIFY2(w.document()->story()->toPlainText() == QStringLiteral("plain strong words"), qPrintable(name));
            QVERIFY(boldAt(w, QStringLiteral("strong")));
        }
    }

    void spreadsheetCellsPasteAsTheirWordsEvenWithRtfAndHtmlBesideThem()
    {
        WriteWindow w;
        emptyWindow(w);
        auto *mime = rtfMime();
        mime->setHtml(QStringLiteral("<html><body><table><tr><td>a</td><td>b</td></tr><tr><td>c</td><td>d</td></tr></table></body></html>"));
        mime->setText(QStringLiteral("a\tb\nc\td\n"));
        QApplication::clipboard()->setMimeData(mime);
        w.view()->paste();
        QVERIFY(w.document()->story()->toPlainText().startsWith(QStringLiteral("a\tb")));
        QVERIFY(!w.document()->story()->toPlainText().contains(QStringLiteral("strong")));
    }

    void htmlFromABrowserPastesFormattedAndKeepTextOnlyGivesTheWords()
    {
        WriteWindow w;
        emptyWindow(w);
        auto *mime = new QMimeData;
        mime->setHtml(QStringLiteral("<p>some <b>bold</b> words</p>"));
        mime->setText(QStringLiteral("PLAINCOPY"));
        QApplication::clipboard()->setMimeData(mime);
        w.view()->paste();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("some bold words"));
        QVERIFY(boldAt(w, QStringLiteral("bold")));
        QVERIFY(w.view()->changeLastPaste(jp::write::PageView::PasteKind::TextOnly));
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("PLAINCOPY"));
        QApplication::clipboard()->setMimeData([] { auto *m = new QMimeData; m->setHtml(QStringLiteral("<p>one <i>step</i></p>")); return m; }());
        WriteWindow one;
        emptyWindow(one);
        one.view()->paste();
        QVERIFY(one.document()->undo());
        QCOMPARE(one.document()->story()->toPlainText(), QString());
        QVERIFY(!one.document()->canUndo());
    }

    void pastedHtmlNeverReadsAFileOffTheDiskAndIsHeldToItsSize()
    {
        QTemporaryDir dir;
        QImage image(4, 4, QImage::Format_RGB32);
        image.fill(Qt::red);
        const QString png = dir.filePath(QStringLiteral("secret.png"));
        QVERIFY(image.save(png));
        const QString html = QStringLiteral("<p>x<img src=\"%1\">y<img src='file:///etc/hostname'>z<span style=\"background:url(%1)\">w</span><img src=\"https://example.com/a.png\"></p>").arg(QUrl::fromLocalFile(png).toString());
        const QString safe = jp::write::safeClipHtml(html);
        QVERIFY(!safe.contains(QStringLiteral("file:")));
        QVERIFY(safe.contains(QStringLiteral("https://example.com/a.png")));
        QVERIFY(jp::write::safeClipHtml(QStringLiteral("<img src=\"data:image/png;base64,AAAA\">")).contains(QStringLiteral("data:image")));
        // (an allowed address in another attribute, or a second src, does not let a file through)
        for (const QString &tricky : {QStringLiteral("<img data-src=\"https://example.com/a.png\" src=\"file:///etc/hostname\">"),
                                      QStringLiteral("<img src=\"https://example.com/a.png\" src=\"file:///etc/hostname\">"),
                                      QStringLiteral("<img xsrc=\"https://example.com/a.png\" src='file:///etc/hostname'>"),
                                      QStringLiteral("<table data-background=\"https://example.com/a.png\" background=\"file:///etc/hostname\"><tr><td>x</td></tr></table>")})
            QVERIFY2(!jp::write::safeClipHtml(tricky).contains(QStringLiteral("file:")), qPrintable(tricky));
        WriteWindow w;
        emptyWindow(w);
        auto *mime = new QMimeData;
        mime->setHtml(html);
        QApplication::clipboard()->setMimeData(mime);
        w.view()->paste();
        int images = 0;
        for (QTextBlock b = w.document()->story()->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString name = it.fragment().charFormat().toImageFormat().name();
                if (it.fragment().charFormat().isImageFormat() && name.startsWith(QStringLiteral("file:"))) ++images;
            }
        QCOMPARE(images, 0);
        // over the limit: the words are used, the HTML is not parsed
        WriteWindow big;
        emptyWindow(big);
        auto *large = new QMimeData;
        large->setHtml(QStringLiteral("<p>HTMLCOPY") + QString(int(jp::write::kMostPastedHtml), QLatin1Char(' ')) + QStringLiteral("</p>"));
        large->setText(QStringLiteral("TEXTCOPY"));
        QApplication::clipboard()->setMimeData(large);
        big.view()->paste();
        QCOMPARE(big.document()->story()->toPlainText(), QStringLiteral("TEXTCOPY"));
    }

    void askingWhetherAPasteIsPossibleReadsNothing()
    {
        WriteWindow w;
        emptyWindow(w);
        auto *mime = new CountingMime;
        mime->setData(QStringLiteral("text/rtf"), QByteArray("{\\rtf1 abc}"));
        mime->setText(QStringLiteral("TEXTCOPY"));
        mime->setHtml(QStringLiteral("<p>HTMLCOPY</p>"));
        QApplication::clipboard()->setMimeData(mime);
        mime->reads = 0;
        using Kind = jp::write::PageView::PasteKind;
        for (Kind k : {Kind::Keep, Kind::Merge, Kind::TextOnly, Kind::Html, Kind::Rtf, Kind::Picture}) w.view()->canPaste(k);
        QCOMPARE(mime->reads, 0);
        w.view()->paste();
        QVERIFY(mime->reads > 0);
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("abc"));
    }

    void rtfOnTheClipboardIsReadOnlyUpToSixteenMegabytes()
    {
        auto big = [](qint64 bytes) {
            const QByteArray head("{\\rtf1\\ansi big text{\\*\\pad "), tail("}}");
            const QByteArray rtf = head + QByteArray(int(bytes - head.size() - tail.size()), 'a') + tail;
            auto *mime = new QMimeData;
            mime->setData(QStringLiteral("text/rtf"), rtf);
            mime->setText(QStringLiteral("TEXTCOPY"));
            return mime;
        };
        {
            WriteWindow w;
            emptyWindow(w);
            QApplication::clipboard()->setMimeData(big(jp::write::kMostPastedRtf));
            w.view()->paste();
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("big text"));
        }
        {
            WriteWindow w;
            emptyWindow(w);
            QApplication::clipboard()->setMimeData(big(jp::write::kMostPastedRtf + 1));
            w.view()->paste();
            QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("TEXTCOPY"));
        }
    }

    // The Paste Options of a list and of cells, open, as pictures for the docs (only with JW_SHOT_DIR: a folder).
    void picturesOfThePasteOptions()
    {
        const QString dir = qEnvironmentVariable("JW_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JW_SHOT_DIR names no folder");
        jp::applyUiTheme(1);
        jp::Settings::get().setValue(QStringLiteral("edit/smartCutPaste"), true);
        auto grabMenu = [&](WriteWindow &w, const QString &name) {
            QMenu *menu = nullptr;
            openOptions(w, &menu);
            QVERIFY(menu->grab().save(dir + QLatin1Char('/') + name));
            menu->close();
        };
        {
            WriteWindow w;
            windowWithTwoLists(w);
            w.view()->paste();
            grabMenu(w, QStringLiteral("paste-options-list.png"));
        }
        {
            WriteWindow w;
            tableWindow(w);
            putCells({QStringLiteral("a|b|c"), QStringLiteral("d|e|f")});
            w.view()->paste();
            grabMenu(w, QStringLiteral("paste-options-table.png"));
        }
    }

    // A nested table is written with a paragraph after it in its cell (a cell ends with one), and reads again.
    void aNestedTableSavesWithAParagraphAfterItAndOpensAgain()
    {
        WriteWindow w;
        tableWindow(w);
        putCells({QStringLiteral("a|b"), QStringLiteral("c|d")});
        w.view()->pasteCellsAs(TablePaste::Nest);
        auto check = [](const QByteArray &xml) {
            QCOMPARE(int(xml.count("<w:tbl>")), 2);
            int ends = 0;
            for (qsizetype at = xml.indexOf("</w:tbl>"); at >= 0; at = xml.indexOf("</w:tbl>", at + 1)) {
                QVERIFY(xml.mid(at + 8, 5) == "<w:p>" || xml.mid(at + 8, 5) == "<w:p ");
                ++ends;
            }
            QCOMPARE(ends, 2);
            for (qsizetype at = xml.indexOf("</w:tc>"); at >= 0; at = xml.indexOf("</w:tc>", at + 1)) QCOMPARE(xml.mid(at - 6, 6), QByteArray("</w:p>"));
        };
        check(bodyXml(w.document()->toDocx()));
        auto again = open(w.document()->toDocx());
        check(bodyXml(again->toDocx()));
    }

    void findCountsTheMatchesAndHighlightsThemOnThePages()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(para(run("The cat and the hat. Then THE end.")) + para(run("Gather the others, the tide, and the line.")))), QString());
        w.act(QStringLiteral("edit.find"))->trigger();
        WriteFindPane *pane = w.findPane();
        QVERIFY(pane);
        QCOMPARE(pane->matchCount(), 0);
        QTest::keyClicks(pane->findBox(), QStringLiteral("the"));
        // Typing counts: "the" is in "The", "the", "Then", "THE", "Gather", "the", "others", "the", and "the".
        QTRY_COMPARE(pane->matchCount(), 9);
        QVERIFY2(pane->status().contains(QStringLiteral("9")), qPrintable(pane->status()));
        QCOMPARE(w.view()->findHighlights().size(), 9);
        pane->matchCase()->setChecked(true);
        QTRY_COMPARE(pane->matchCount(), 6);
        pane->wholeWords()->setChecked(true);
        QTRY_COMPARE(pane->matchCount(), 4);
        QCOMPARE(w.view()->findHighlights().size(), 4);
        QVERIFY(pane->findNext());
        QVERIFY2(pane->status().contains(QStringLiteral("1 of 4")), qPrintable(pane->status()));
        QVERIFY(pane->findNext());
        QVERIFY2(pane->status().contains(QStringLiteral("2 of 4")), qPrintable(pane->status()));
        // The highlights are drawn: more yellow on the pages than without them.
        auto yellow = [&] {
            const QImage img = w.view()->viewport()->grab().toImage();
            int n = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x) {
                    const QColor c = img.pixelColor(x, y);
                    if (c.red() > 200 && c.green() > 200 && c.blue() < 170) ++n;
                }
            return n;
        };
        const int with = yellow();
        shot(&w, QStringLiteral("find-highlights.png"));
        pane->findBox()->clear();
        QTRY_COMPARE(pane->matchCount(), 0);
        QVERIFY(w.view()->findHighlights().isEmpty());
        QVERIFY2(with > yellow() + 100, qPrintable(QString::number(with)));
        // Editing the text keeps the count right.
        pane->matchCase()->setChecked(false);
        pane->wholeWords()->setChecked(false);
        QTest::keyClicks(pane->findBox(), QStringLiteral("cat"));
        QTRY_COMPARE(pane->matchCount(), 1);
        w.view()->insertText(QStringLiteral("cat "));
        QTRY_COMPARE(pane->matchCount(), 2);
    }

    void wildcardsAndSpecialCodesMakeAPatternAndTheReplacementCanUseTheGroups()
    {
        auto all = [](const QString &pattern, const QString &text, bool matchCase = false) {
            const FindPattern p = wildcardPattern(pattern, matchCase);
            QStringList out;
            if (!p.error.isEmpty()) return QStringList{QStringLiteral("!") + p.error};
            auto it = p.regex.globalMatch(text);
            while (it.hasNext()) out << it.next().captured(0);
            return out;
        };
        QCOMPARE(all(QStringLiteral("c?t"), QStringLiteral("cat cot cut cart")), (QStringList{"cat", "cot", "cut"}));
        QCOMPARE(all(QStringLiteral("c*t"), QStringLiteral("cat cart")), (QStringList{"cat", "cart"}));
        QCOMPARE(all(QStringLiteral("<the>"), QStringLiteral("the other then the")), (QStringList{"the", "the"}));
        QCOMPARE(all(QStringLiteral("<the"), QStringLiteral("the other then")), (QStringList{"the", "the"}));
        QCOMPARE(all(QStringLiteral("[bc]at"), QStringLiteral("bat cat rat")), (QStringList{"bat", "cat"}));
        QCOMPARE(all(QStringLiteral("[!bc]at"), QStringLiteral("bat cat rat")), (QStringList{"rat"}));
        QCOMPARE(all(QStringLiteral("[a-c]{2}"), QStringLiteral("abxcc")), (QStringList{"ab", "cc"}));
        QCOMPARE(all(QStringLiteral("lo@l"), QStringLiteral("lol lool lxl")), (QStringList{"lol", "lool"}));
        QCOMPARE(all(QStringLiteral("^#^#"), QStringLiteral("a12b3")), (QStringList{"12"}));
        QCOMPARE(all(QStringLiteral("a^tb"), QStringLiteral("a\tb a b")), (QStringList{"a\tb"}));
        QCOMPARE(all(QStringLiteral("a^pb"), QStringLiteral("a\nb")), (QStringList{"a\nb"}));
        QCOMPARE(all(QStringLiteral("\\?"), QStringLiteral("what? why")), (QStringList{"?"}));
        QCOMPARE(all(QStringLiteral("a.b"), QStringLiteral("a.b axb")), (QStringList{"a.b"}));
        QCOMPARE(all(QStringLiteral("CAT"), QStringLiteral("cat CAT"), true), (QStringList{"CAT"}));
        QCOMPARE(all(QStringLiteral("CAT"), QStringLiteral("cat CAT"), false), (QStringList{"cat", "CAT"}));
        QVERIFY(all(QStringLiteral("[abc"), QStringLiteral("x")).first().startsWith(QLatin1Char('!')));
        QVERIFY(all(QStringLiteral("(ab"), QStringLiteral("x")).first().startsWith(QLatin1Char('!')));
        QVERIFY(all(QStringLiteral("a{2"), QStringLiteral("x")).first().startsWith(QLatin1Char('!')));
        // Groups in the replacement, and the whole match.
        const FindPattern names = wildcardPattern(QStringLiteral("(<[A-Za-z]@>), (<[A-Za-z]@>)"), false);
        QVERIFY2(names.error.isEmpty(), qPrintable(names.error));
        const QString text = QStringLiteral("Smith, John");
        const QRegularExpressionMatch m = names.regex.match(text);
        QVERIFY(m.hasMatch());
        QCOMPARE(expandReplacement(QStringLiteral("\\2 \\1"), m), QStringLiteral("John Smith"));
        QCOMPARE(expandReplacement(QStringLiteral("[^&]"), m), QStringLiteral("[Smith, John]"));
        QCOMPARE(expandReplacement(QStringLiteral("a^tb\\\\"), m), QStringLiteral("a\tb\\"));
        QCOMPARE(expandReplacement(QStringLiteral("\\5"), m), QString());
        QVERIFY(!wildcardPattern(QString(), false).error.isEmpty());
    }

    void advancedFindReadsSoundsFormsAndLettersWithPunctuationBetween()
    {
        QCOMPARE(soundexOf(QStringLiteral("Smith")), QStringLiteral("S530"));
        QCOMPARE(soundexOf(QStringLiteral("smyth")), QStringLiteral("S530"));
        QCOMPARE(soundexOf(QStringLiteral("Robert")), QStringLiteral("R163"));
        QCOMPARE(soundexOf(QStringLiteral("Rupert")), QStringLiteral("R163"));
        QCOMPARE(soundexOf(QStringLiteral("Tymczak")), QStringLiteral("T522"));
        QCOMPARE(soundexOf(QStringLiteral("Pfister")), QStringLiteral("P236"));
        QCOMPARE(soundexOf(QStringLiteral("123")), QString());
        QCOMPARE(wordStem(QStringLiteral("Running")), wordStem(QStringLiteral("runs")));
        QCOMPARE(wordStem(QStringLiteral("ran")), wordStem(QStringLiteral("run")));
        QCOMPARE(wordStem(QStringLiteral("making")), wordStem(QStringLiteral("made")));
        QCOMPARE(wordStem(QStringLiteral("tries")), wordStem(QStringLiteral("try")));
        QCOMPARE(wordStem(QStringLiteral("buses")), wordStem(QStringLiteral("bus")));
        QCOMPARE(wordStem(QStringLiteral("walked")), wordStem(QStringLiteral("walking")));
        QVERIFY(wordStem(QStringLiteral("string")) != wordStem(QStringLiteral("str")));
        QVERIFY(wordStem(QStringLiteral("cat")) != wordStem(QStringLiteral("cart")));
        auto all = [](const QString &text, AdvancedFind o) {
            QStringList out;
            for (int at = 0;;) {
                const QPair<int, int> m = advancedMatch(text, at, o);
                if (m.first < 0) break;
                out << text.mid(m.first, m.second - m.first);
                at = std::max(m.second, at + 1);
            }
            return out;
        };
        AdvancedFind o;
        o.query = QStringLiteral("Smith");
        o.soundsLike = true;
        QCOMPARE(all(QStringLiteral("Mr. Smyth met Smith and Schmidt, not Jones."), o), (QStringList{"Smyth", "Smith", "Schmidt"}));   // (Schmidt has the same code, S530)
        o = AdvancedFind();
        o.query = QStringLiteral("run");
        o.wordForms = true;
        QCOMPARE(all(QStringLiteral("He runs, ran, and was running; a rung and a runner."), o), (QStringList{"runs", "ran", "running", "runner"}));   // ("rung" is another word)
        o.query = QStringLiteral("big dog");
        QCOMPARE(all(QStringLiteral("Two bigger dogs; one big dog; a big, dog."), o), (QStringList{"bigger dogs", "big dog"}));
        o = AdvancedFind();
        o.query = QStringLiteral("e-mail");
        o.ignorePunctuation = true;
        QCOMPARE(all(QStringLiteral("email, e-mail, e.mail, e mail"), o), (QStringList{"email", "e-mail", "e.mail"}));
        o.ignorePunctuation = false;
        o.ignoreSpaces = true;
        o.query = QStringLiteral("e mail");
        QCOMPARE(all(QStringLiteral("email, e-mail, e   mail, e\tmail"), o), (QStringList{"email", "e   mail", "e\tmail"}));
        o.wholeWords = true;
        o.query = QStringLiteral("ab");
        QCOMPARE(all(QStringLiteral("a b, cab, a  b"), o), (QStringList{"a b", "a  b"}));
        o = AdvancedFind();
        o.query = QStringLiteral("AB");
        o.ignoreSpaces = true;
        o.matchCase = true;
        QCOMPARE(all(QStringLiteral("a b A B ab AB"), o), (QStringList{"A B", "AB"}));
        // A search that has nothing to look for finds nothing.
        o = AdvancedFind();
        o.ignorePunctuation = true;
        o.query = QStringLiteral("-.");
        QVERIFY(advancedMatch(QStringLiteral("a-b"), 0, o).first < 0);
        // Format filters.
        FormatFilter f;
        QVERIFY(!f.any());
        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);
        QTextCharFormat plain;
        f.bold = 1;
        QVERIFY(f.any() && f.matchesChar(bold) && !f.matchesChar(plain));
        f.bold = 0;
        QVERIFY(!f.matchesChar(bold) && f.matchesChar(plain));
        f = FormatFilter();
        f.font = QStringLiteral("arial");
        QTextCharFormat arial;
        arial.setFontFamilies({QStringLiteral("Arial")});
        QVERIFY(f.matchesChar(arial) && !f.matchesChar(plain));
        f = FormatFilter();
        f.underline = 1;
        f.highlighted = 1;
        QTextCharFormat both;
        both.setFontUnderline(true);
        both.setProperty(tp::HighlightRefP, QStringLiteral("rgb:FFFF00"));
        QVERIFY(f.matchesChar(both));
        both.setFontUnderline(false);
        QVERIFY(!f.matchesChar(both));
        f = FormatFilter();
        f.style = QStringLiteral("Heading1");
        QVERIFY(f.matchesStyle(QStringLiteral("Heading1")) && !f.matchesStyle(QString()));
    }

    void theFindPaneSearchesBySoundByWordFormsAndByLettersWithPunctuationBetween()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(para(run("Mr. Smyth met Smith, who is running; he runs, ran, and e-mail came by email.")))), QString());
        w.act(QStringLiteral("edit.find"))->trigger();
        WriteFindPane *pane = w.findPane();
        QVERIFY(pane && pane->soundsLike() && pane->wordForms() && pane->ignorePunctuation() && pane->ignoreSpaces() && pane->specialButton() && pane->formatButton());
        pane->findBox()->setText(QStringLiteral("Smith"));
        QTRY_COMPARE(pane->matchCount(), 1);
        pane->soundsLike()->setChecked(true);
        QTRY_COMPARE(pane->matchCount(), 2);
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("Smyth"));
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("Smith"));
        QVERIFY(!pane->wordForms()->isEnabled());   // (one reading at a time)
        pane->soundsLike()->setChecked(false);
        pane->findBox()->setText(QStringLiteral("run"));
        QTRY_COMPARE(pane->matchCount(), 2);   // (the letters alone: "running" and "runs")
        pane->wordForms()->setChecked(true);
        QTRY_COMPARE(pane->matchCount(), 3);   // (and "ran")
        pane->findBox()->setText(QStringLiteral("ran"));
        QTRY_COMPARE(pane->matchCount(), 3);
        pane->wordForms()->setChecked(false);
        pane->findBox()->setText(QStringLiteral("email"));
        QTRY_COMPARE(pane->matchCount(), 1);
        pane->ignorePunctuation()->setChecked(true);
        QTRY_COMPARE(pane->matchCount(), 2);
        pane->findBox()->setText(QStringLiteral("e-mail"));
        QTRY_COMPARE(pane->matchCount(), 2);
        // Replace All reads the same way and is one step.
        pane->replaceBox()->setText(QStringLiteral("message"));
        const quint64 serial = w.document()->nextStepSerial();
        pane->replaceAll();
        QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("and message came by message.")));
        QCOMPARE(w.document()->nextStepSerial(), serial + 1);
    }

    void theSpecialCodesWorkWithoutWildcardsAndTheMenuTypesThem()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(para(run("a? b*")) + para("<w:r><w:t>c</w:t><w:tab/><w:t>d 12</w:t></w:r>") + para(run("end")))), QString());
        w.act(QStringLiteral("edit.find"))->trigger();
        WriteFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("^p"));
        QTRY_COMPARE(pane->matchCount(), 2);
        pane->findBox()->setText(QStringLiteral("^t"));
        QTRY_COMPARE(pane->matchCount(), 1);
        pane->findBox()->setText(QStringLiteral("^#^#"));
        QTRY_COMPARE(pane->matchCount(), 1);
        // The other characters are themselves: "?" is a question mark here, not any letter.
        pane->findBox()->setText(QStringLiteral("a?^p"));
        QTRY_COMPARE(pane->matchCount(), 0);
        pane->findBox()->setText(QStringLiteral("b*^p"));
        QTRY_COMPARE(pane->matchCount(), 1);
        pane->findBox()->clear();
        QAction *digit = nullptr;
        for (QAction *a : pane->specialButton()->menu()->actions())
            if (a->data().toString() == QStringLiteral("^#")) digit = a;
        QVERIFY(digit);
        digit->trigger();
        QCOMPARE(pane->findBox()->text(), QStringLiteral("^#"));
        QTRY_COMPARE(pane->matchCount(), 2);
    }

    void theFindPaneFindsByFormatWithOrWithoutText()
    {
        const QByteArray body = para(run("Plain words ") + run("bold words", "<w:b/>") + run(" and plain end")) + para(run("A second bold line", "<w:b/>"));
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        w.act(QStringLiteral("edit.replace"))->trigger();
        WriteFindPane *pane = w.findPane();
        write::FormatFilter bold;
        bold.bold = 1;
        pane->setFormatFilter(bold);
        // Only the look: each stretch of bold text is a match.
        QTRY_COMPARE(pane->matchCount(), 2);
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("bold words"));
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("A second bold line"));
        // A word, and the look.
        pane->findBox()->setText(QStringLiteral("words"));
        QTRY_COMPARE(pane->matchCount(), 1);
        pane->findBox()->setText(QStringLiteral("plain"));
        QTRY_COMPARE(pane->matchCount(), 0);
        bold.bold = 0;
        pane->setFormatFilter(bold);
        QTRY_COMPARE(pane->matchCount(), 2);
        QVERIFY(pane->findChild<QLabel *>(QStringLiteral("findFormatLabel"))->isVisible());
        // Replace All changes the words that have the look and no others.
        bold.bold = 1;
        pane->setFormatFilter(bold);
        pane->findBox()->setText(QStringLiteral("words"));
        pane->replaceBox()->setText(QStringLiteral("text"));
        pane->replaceAll();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Plain words bold text and plain end\nA second bold line"));
        pane->setFormatFilter(write::FormatFilter());
        QVERIFY(!pane->findChild<QLabel *>(QStringLiteral("findFormatLabel"))->isVisible());
    }

    void findHighlightsAlsoShowInTableCellsAndHeaders()
    {
        auto yellow = [](QWidget *view) {
            const QImage img = view->grab().toImage();
            int n = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x) {
                    const QColor c = img.pixelColor(x, y);
                    if (c.red() > 200 && c.green() > 200 && c.blue() < 170) ++n;
                }
            return n;
        };
        const QByteArray cell = "<w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>needle in a cell</w:t></w:r></w:p></w:tc>";
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"4000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr>" + cell + cell + "</w:tr></w:tbl>";
        const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdHead\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        office::PackageWriter pw;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        pw.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + para(run("body text")) + table + para(run("end")) + sect + "</w:body></w:document>"),
                   QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        pw.addPart(QStringLiteral("/word/header1.xml"), xml(QByteArray("<w:hdr ") + kNs + "><w:p><w:r><w:t>needle in the header</w:t></w:r></w:p></w:hdr>"),
                   QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"));
        pw.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        pw.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdHead"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 900);
        w.show();
        w.setDocument(open(pw.finish()), QString());
        w.act(QStringLiteral("edit.find"))->trigger();
        WriteFindPane *pane = w.findPane();
        QTRY_VERIFY(w.view()->pageCount() >= 1);
        QTest::qWait(50);
        const int none = yellow(w.view()->viewport());
        pane->findBox()->setText(QStringLiteral("needle"));
        QTRY_COMPARE(pane->matchCount(), 2);   // (the two cells: the header is counted by the pages as they draw it)
        QTRY_VERIFY(yellow(w.view()->viewport()) > none + 300);
        shot(&w, QStringLiteral("find-cells-header.png"));
        // The cells alone, then the header alone: each adds its own band.
        const int both = yellow(w.view()->viewport());
        QVERIFY(!pane->matchesInSideStory(w.document()->headerFooterStory(0, false, 0)).isEmpty());
        pane->findBox()->setText(QStringLiteral("cell"));
        QTRY_COMPARE(pane->matchCount(), 2);
        QTRY_VERIFY(yellow(w.view()->viewport()) < both);
        QTRY_VERIFY(yellow(w.view()->viewport()) > none + 200);
        QVERIFY(pane->matchesInSideStory(w.document()->headerFooterStory(0, false, 0)).isEmpty());
    }

    // A document with a header, a footer, and a note that each hold "needle", and a body that does not (Find Next, Replace, and Replace All must reach them).
    QByteArray docWithSideNeedles(bool bodyNeedle = false)
    {
        const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdHead\"/><w:footerReference w:type=\"default\" r:id=\"rIdFoot\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        office::PackageWriter pw;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QString wml = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");
        pw.addPart(QStringLiteral("/word/document.xml"),
                   xml(QByteArray("<w:document ") + kNs + "><w:body>" + para(run(bodyNeedle ? "body needle text" : "body text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>") + para(run("end")) + sect + "</w:body></w:document>"),
                   wml + QStringLiteral("document.main+xml"));
        pw.addPart(QStringLiteral("/word/header1.xml"), xml(QByteArray("<w:hdr ") + kNs + "><w:p><w:r><w:t>needle in the header</w:t></w:r></w:p></w:hdr>"), wml + QStringLiteral("header+xml"));
        pw.addPart(QStringLiteral("/word/footer1.xml"), xml(QByteArray("<w:ftr ") + kNs + "><w:p><w:r><w:t>the footer needle</w:t></w:r></w:p></w:ftr>"), wml + QStringLiteral("footer+xml"));
        pw.addPart(QStringLiteral("/word/footnotes.xml"),
                   xml(QByteArray("<w:footnotes ") + kNs + "><w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> a note needle</w:t></w:r></w:p></w:footnote></w:footnotes>"),
                   wml + QStringLiteral("footnotes+xml"));
        pw.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        pw.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdHead"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
        pw.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdFoot"), rel + QStringLiteral("footer"), QStringLiteral("/word/footer1.xml"));
        pw.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
        return pw.finish();
    }

    void findNextAndReplaceReachHeadersFootersAndNotes()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 900);
        w.show();
        w.setDocument(open(docWithSideNeedles()), QString());
        w.act(QStringLiteral("edit.replace"))->trigger();
        WriteFindPane *pane = w.findPane();
        QTRY_VERIFY(w.view()->pageCount() >= 1);
        pane->findBox()->setText(QStringLiteral("needle"));
        pane->replaceBox()->setText(QStringLiteral("pin"));
        QTRY_VERIFY(pane->matchCount() == 0);
        // The body has none: the search goes on into the header, then the footer, then the note, then (nothing in the text) round to the header again.
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("needle"));
        QVERIFY(w.view()->editedSideStory() != nullptr);
        QCOMPARE(w.view()->editedSideStory(), w.document()->headerFooterStory(0, false, 0));
        QVERIFY2(pane->status().contains(QStringLiteral("header of page 1")), qPrintable(pane->status()));
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->editedSideStory(), w.document()->headerFooterStory(0, true, 0));
        QVERIFY2(pane->status().contains(QStringLiteral("footer of page 1")), qPrintable(pane->status()));
        QVERIFY(pane->findNext());
        int id = 0;
        QVERIFY(w.view()->editedSideStory() && w.document()->footnoteIdOf(w.view()->editedSideStory(), &id));
        QVERIFY2(pane->status().contains(QStringLiteral("note on page 1")), qPrintable(pane->status()));
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("needle"));
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->editedSideStory(), w.document()->headerFooterStory(0, false, 0));
        // Replace swaps the match in the header and goes on to the footer.
        pane->replaceOne();
        QCOMPARE(w.document()->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("pin in the header"));
        QCOMPARE(w.view()->editedSideStory(), w.document()->headerFooterStory(0, true, 0));
        shot(&w, QStringLiteral("find-reaches-footer.png"));
        // Replace All swaps the other two, and one Undo brings back every change.
        pane->replaceAll();
        QVERIFY2(pane->status().contains(QStringLiteral("2")), qPrintable(pane->status()));
        QCOMPARE(w.document()->headerFooterStory(0, true, 0)->toPlainText(), QStringLiteral("the footer pin"));
        QVERIFY(w.document()->editableFootnote(2)->toPlainText().contains(QStringLiteral("a note pin")));
        QVERIFY(!pane->findNext());
        QVERIFY(w.document()->undo());
        QCOMPARE(w.document()->headerFooterStory(0, true, 0)->toPlainText(), QStringLiteral("the footer needle"));
        QVERIFY(w.document()->editableFootnote(2)->toPlainText().contains(QStringLiteral("a note needle")));
        QCOMPARE(w.document()->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("pin in the header"));
    }

    void replaceAllInTextAndHeaderIsOneUndoStep()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 900);
        w.show();
        w.setDocument(open(docWithSideNeedles(true)), QString());
        w.act(QStringLiteral("edit.replace"))->trigger();
        WriteFindPane *pane = w.findPane();
        QTRY_VERIFY(w.view()->pageCount() >= 1);
        pane->findBox()->setText(QStringLiteral("needle"));
        pane->replaceBox()->setText(QStringLiteral("pin"));
        QTRY_COMPARE(pane->matchCount(), 1);
        pane->replaceAll();
        QVERIFY2(pane->status().contains(QStringLiteral("4")), qPrintable(pane->status()));
        QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("body pin text")));
        QCOMPARE(w.document()->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("pin in the header"));
        QVERIFY(w.document()->undo());
        QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("body needle text")));
        QCOMPARE(w.document()->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("needle in the header"));
        QCOMPARE(w.document()->headerFooterStory(0, true, 0)->toPlainText(), QStringLiteral("the footer needle"));
    }

    // Three headings of two levels, each with a paragraph or two under it.
    static QByteArray headingsDoc()
    {
        auto heading = [](const char *text, int level) { return para(run(text), "<w:outlineLvl w:val=\"" + QByteArray::number(level) + "\"/>"); };
        return makeDocx(heading("Alpha", 0) + para(run("under alpha one")) + para(run("under alpha two")) + heading("Alpha child", 1) + para(run("under the child")) + heading("Beta", 0) +
                        para(run("under beta")));
    }

    void collapsingAHeadingHidesItsParagraphsOnTheScreenOnly()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(headingsDoc()), QString());
        QTRY_VERIFY(w.view()->pageCount() >= 1);
        const QString text = w.document()->story()->toPlainText();
        const int beta = int(text.indexOf(QStringLiteral("Beta"))), child = int(text.indexOf(QStringLiteral("Alpha child")));
        QRect before, after, line;
        QVERIFY(w.view()->positionRect(beta, &before));
        const QByteArray saved = w.document()->toDocx();
        QVERIFY(w.view()->toggleHeadingCollapsed(0));
        QVERIFY(headingCollapsed(*w.document(), 0));
        QVERIFY(hiddenByCollapse(*w.document(), text.indexOf(QStringLiteral("under alpha two"))));
        QVERIFY(hiddenByCollapse(*w.document(), child));   // (the lower heading goes with what is under the higher one)
        QVERIFY(!hiddenByCollapse(*w.document(), beta));
        QTRY_VERIFY(w.view()->positionRect(beta, &after) && after.top() < before.top() - 60);
        QVERIFY(w.view()->positionRect(0, &line));
        QCOMPARE(w.document()->story()->toPlainText(), text);   // (the text is as it was)
        // The paper and the file are of every paragraph: the file is the same, and the pages are laid out as they were while one is made.
        QCOMPARE(w.document()->toDocx(), saved);
        w.document()->suspendCollapse(true);
        QTRY_VERIFY(w.view()->positionRect(beta, &after) && after.top() == before.top());
        w.document()->suspendCollapse(false);
        QTRY_VERIFY(w.view()->positionRect(beta, &after) && after.top() < before.top() - 60);
        shot(&w, QStringLiteral("collapse-heading.png"));
        // Opening it brings them back; a paragraph that is no heading does not collapse.
        QVERIFY(w.view()->toggleHeadingCollapsed(0));
        QVERIFY(!headingCollapsed(*w.document(), 0));
        QTRY_VERIFY(w.view()->positionRect(beta, &after) && after.top() == before.top());
        QVERIFY(!setHeadingCollapsed(*w.document(), text.indexOf(QStringLiteral("under beta")), true));
        // A lower heading collapses alone, up to the next of its level or above.
        QVERIFY(w.view()->toggleHeadingCollapsed(child));
        QVERIFY(hiddenByCollapse(*w.document(), text.indexOf(QStringLiteral("under the child"))));
        QVERIFY(!hiddenByCollapse(*w.document(), text.indexOf(QStringLiteral("under alpha two"))));
        QVERIFY(!hiddenByCollapse(*w.document(), beta));
        // Moving the caret into what is hidden opens it.
        QTextCursor in(w.document()->story());
        in.setPosition(text.indexOf(QStringLiteral("under the child")) + 3);
        w.view()->setCursor(in);
        QVERIFY(!hiddenByCollapse(*w.document(), in.position()));
        QVERIFY(!w.document()->collapseActive());
    }

    void theTriangleByAHeadingShowsOnHoverAndCollapsesOnAClick()
    {
        auto grey = [](QWidget *view) {
            const QImage img = view->grab().toImage();
            int n = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width() / 3; ++x) {
                    const QColor c = img.pixelColor(x, y);
                    if (c.red() == 0x60 && c.green() == 0x60 && c.blue() == 0x60) ++n;
                }
            return n;
        };
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(headingsDoc()), QString());
        QTRY_VERIFY(w.view()->pageCount() >= 1);
        QWidget *vp = w.view()->viewport();
        QRect line;
        QVERIFY(w.view()->positionRect(0, &line));
        QTest::qWait(30);
        const int none = grey(vp);
        const QPoint onHeading(line.left() + 10, line.center().y());
        QTest::mouseMove(vp, onHeading);
        QTRY_VERIFY(grey(vp) > none + 10);   // (a triangle by the heading the pointer is on)
        shot(&w, QStringLiteral("collapse-triangle.png"));
        const QPoint triangle(line.left() - 14, line.center().y());
        QTest::mouseMove(vp, triangle);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, triangle);
        QVERIFY(headingCollapsed(*w.document(), 0));
        // (A collapsed heading keeps its triangle, pointing right, with the pointer elsewhere.)
        QTest::mouseMove(vp, QPoint(vp->width() - 5, vp->height() - 5));
        QTRY_VERIFY(grey(vp) > none + 10);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, triangle);
        QVERIFY(!headingCollapsed(*w.document(), 0));
    }

    void webLayoutSetsTheTextAsWideAsTheWindowAndGivesTheNormalPagesBack()
    {
        QByteArray body;
        for (int i = 0; i < 90; ++i) body += para(run("A paragraph of plain words that is long enough to run onto a second line when the window gets narrow, and then some more words after it."));
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        QTRY_VERIFY(w.view()->pageCount() >= 2);
        // (all pages laid out first: pages are set a few at a time, so a count read before that can be short of the whole)
        w.view()->paginateAll();
        const int printPages = w.view()->pageCount();
        const double printWidth = w.document()->flow().section(0).paper.width();
        const QByteArray saved = w.document()->toDocx();
        QRect last;
        const int lastAt = w.document()->story()->characterCount() - 2;
        w.act(QStringLiteral("view.webLayout"))->trigger();
        QVERIFY(w.view()->webLayout());
        QVERIFY(w.act(QStringLiteral("view.webLayout"))->isChecked());
        QVERIFY(!w.act(QStringLiteral("view.printLayout"))->isChecked());
        // One page's width of the window, no page breaks to speak of, and the window's width: a narrower window sets more lines.
        const double webWidth = w.document()->flow().section(0).paper.width();
        QVERIFY2(webWidth > 300 && webWidth != printWidth, qPrintable(QString::number(webWidth)));
        QTRY_VERIFY(w.view()->pageCount() < printPages);
        QTRY_VERIFY(w.view()->positionRect(0, &last));
        const QSize wide = w.view()->viewport()->size();
        QRect wideLast;
        w.view()->showCursor([&] { QTextCursor c(w.document()->story()); c.setPosition(lastAt); return c; }());
        QTRY_VERIFY(w.view()->positionRect(lastAt, &wideLast));
        const int wideHeight = int(w.view()->verticalScrollBar()->maximum()) + wide.height();
        w.resize(560, 800);
        QTRY_VERIFY(w.document()->flow().section(0).paper.width() < webWidth - 200);
        QTRY_VERIFY(int(w.view()->verticalScrollBar()->maximum()) + w.view()->viewport()->height() > wideHeight + 200);
        shot(&w, QStringLiteral("web-layout.png"));
        // The file is as it was, and fields' page numbers are of the pages the file has.
        QCOMPARE(w.document()->toDocx(), saved);
        {
            const write::WriteDoc::PrintLayout print(*w.document());
            QCOMPARE(w.document()->flow().section(0).paper.width(), printWidth);
        }
        QTRY_VERIFY(w.document()->flow().section(0).paper.width() < webWidth - 200);
        // The page view comes back as the file has it.
        w.act(QStringLiteral("view.printLayout"))->trigger();
        QVERIFY(!w.view()->webLayout());
        QCOMPARE(w.document()->flow().section(0).paper.width(), printWidth);
        QTRY_COMPARE(w.view()->pageCount(), printPages);
        QVERIFY(w.act(QStringLiteral("view.printLayout"))->isChecked());
        QVERIFY(!w.act(QStringLiteral("view.webLayout"))->isChecked());
        QCOMPARE(w.document()->toDocx(), saved);
    }

    void theSpecialCodesFindSectionBreaksFieldsAndNoteMarks()
    {
        const QString wml = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray body =
            para(run("Alpha ") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>7</w:t></w:r></w:fldSimple>") +
            "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr></w:pPr><w:r><w:t>ends the section</w:t></w:r></w:p>" +
            para(run("Beta ") + "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> DATE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>today</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>" +
                 "<w:r><w:endnoteReference w:id=\"2\"/></w:r> end of it");
        office::PackageWriter pw;
        pw.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr></w:body></w:document>"), wml + QStringLiteral("document.main+xml"));
        pw.addPart(QStringLiteral("/word/footnotes.xml"), xml(QByteArray("<w:footnotes ") + kNs + "><w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> a footnote</w:t></w:r></w:p></w:footnote></w:footnotes>"), wml + QStringLiteral("footnotes+xml"));
        pw.addPart(QStringLiteral("/word/endnotes.xml"), xml(QByteArray("<w:endnotes ") + kNs + "><w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> an endnote</w:t></w:r></w:p></w:endnote></w:endnotes>"), wml + QStringLiteral("endnotes+xml"));
        pw.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        pw.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
        pw.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("endnotes"), QStringLiteral("/word/endnotes.xml"));
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(pw.finish()), QString());
        w.act(QStringLiteral("edit.replace"))->trigger();
        WriteFindPane *pane = w.findPane();
        QTRY_VERIFY(w.view()->pageCount() >= 1);
        auto count = [&](const QString &q) {
            pane->findBox()->setText(q);
            QTest::qWait(60);
            pane->findBox()->setText(q);
            return pane->matchCount();
        };
        QTRY_COMPARE(count(QStringLiteral("^b")), 1);   // (the one section break)
        QTRY_COMPARE(count(QStringLiteral("^f")), 1);
        QTRY_COMPARE(count(QStringLiteral("^e")), 1);
        QTRY_COMPARE(count(QStringLiteral("^d")), 2);   // (the simple field and the one made of marks)
        QTRY_COMPARE(count(QStringLiteral("Alpha ^f")), 1);
        // The words that show are as they were: text across a field's start is found as it was (without ^d), and the marks are not text.
        QTRY_COMPARE(count(QStringLiteral("Beta today")), 1);
        QTRY_COMPARE(count(QStringLiteral("ends the section^bBeta")), 1);
        QTRY_COMPARE(count(QStringLiteral("^p")), 1);   // (the paragraph mark between the others: the section break is not one)
        // The Special menu has them.
        QStringList names;
        for (QAction *a : pane->specialButton()->menu()->actions()) names << a->text();
        for (const char *n : {"Section Break", "Field", "Footnote Mark", "Endnote Mark"}) QVERIFY2(names.contains(QString::fromLatin1(n)), n);
        // Find Next goes to each, and Replace leaves them be (a mark that is replaced would lose what it stands for).
        pane->findBox()->setText(QStringLiteral("^b"));
        QVERIFY(pane->findNext());
        const QString before = w.document()->story()->toPlainText();
        pane->replaceBox()->setText(QStringLiteral("X"));
        pane->replaceAll();
        QVERIFY2(pane->status().contains(QStringLiteral("no replacements")), qPrintable(pane->status()));
        QCOMPARE(w.document()->story()->toPlainText(), before);
        QTRY_COMPARE(count(QStringLiteral("^b")), 1);
        pane->findBox()->setText(QStringLiteral("^d"));
        QVERIFY(pane->findNext());
        pane->replaceOne();
        QVERIFY2(pane->status().contains(QStringLiteral("not replaced")), qPrintable(pane->status()));
        QCOMPARE(w.document()->story()->toPlainText(), before);
        // With wildcards on, the codes work the same.
        auto *wild = pane->findChild<QCheckBox *>(QStringLiteral("useWildcards"));
        wild->setChecked(true);
        QTRY_COMPARE(count(QStringLiteral("^b")), 1);
        QTRY_COMPARE(count(QStringLiteral("^f")), 1);
    }

    void webLayoutAfterAFitZoomKeepsTheZoomAndStaysStable()
    {
        QByteArray body;
        for (int i = 0; i < 30; ++i) body += para(run("A paragraph of plain words."));
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        QTRY_VERIFY(w.view()->pageCount() >= 1);
        // A zoom that fits a page to the window, then Web Layout: the zoom is the one there was, and the width settles.
        w.act(QStringLiteral("zoom.onePage"))->trigger();
        const double zoom = w.view()->zoom();
        w.act(QStringLiteral("view.webLayout"))->trigger();
        QTest::qWait(100);
        QCOMPARE(w.view()->zoom(), zoom);
        const double width = w.document()->flow().section(0).paper.width();
        QTest::qWait(100);
        QCOMPARE(w.document()->flow().section(0).paper.width(), width);
        QVERIFY(width > 120);
        w.act(QStringLiteral("zoom.pageWidth"))->trigger();   // (asking for a fit while it is on changes nothing)
        QTest::qWait(100);
        QCOMPARE(w.view()->zoom(), zoom);
        QCOMPARE(w.document()->flow().section(0).paper.width(), width);
    }

    void theFindPaneCanSearchWithWildcardsAndReplaceWithGroups()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(para(run("Smith, John and Jones, Mary went home.")) + para(run("Lee, Ann stayed.")))), QString());
        w.act(QStringLiteral("edit.replace"))->trigger();
        WriteFindPane *pane = w.findPane();
        QVERIFY(pane);
        auto *wild = pane->findChild<QCheckBox *>(QStringLiteral("useWildcards"));
        QVERIFY(wild);
        wild->setChecked(true);
        pane->findBox()->setText(QStringLiteral("(<[A-Z]@>), (<[A-Z]@>)"));
        QTRY_COMPARE(pane->matchCount(), 3);
        shot(&w, QStringLiteral("find-wildcards.png"));
        QVERIFY(pane->findNext());
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("Smith, John"));
        pane->replaceBox()->setText(QStringLiteral("\\2 \\1"));
        pane->replaceOne();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("John Smith and Jones, Mary went home.\nLee, Ann stayed."));
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("Jones, Mary"));
        pane->replaceAll();
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("John Smith and Mary Jones went home.\nAnn Lee stayed."));
        QVERIFY2(pane->status().contains(QStringLiteral("2")), qPrintable(pane->status()));
        QVERIFY(w.document()->undo());
        QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("John Smith and Jones, Mary went home.\nLee, Ann stayed."));
        // A pattern that is not one says so and finds nothing.
        pane->findBox()->setText(QStringLiteral("[abc"));
        QTRY_VERIFY(pane->status().contains(QStringLiteral("]")));
        QCOMPARE(pane->matchCount(), 0);
        QVERIFY(!pane->findNext());
        // Off, the same text is plain words.
        wild->setChecked(false);
        pane->findBox()->setText(QStringLiteral("Jones, M"));
        QTRY_COMPARE(pane->matchCount(), 1);
    }

    // A picture of the Home ribbon (with JW_SHOT_DIR set), for looking at the Bold button's weight.
    void grabTheHomeRibbonAndTheMiniToolbar()
    {
        const QString dir = qEnvironmentVariable("JW_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JW_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        QDir().mkpath(dir);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(para(run("Some words to pick out of the line.")))), QString());
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        QVERIFY(w.grab().save(dir + QStringLiteral("/write-ribbon.png")));
    }

    void theMiniToolbarFloatsAboveWordsSelectedWithTheMouseAndGoesWhenTheSelectionDoes()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(para(run("Some words to pick out of the line.")))), QString());
        auto *mini = w.view()->findChild<QFrame *>(QStringLiteral("miniToolbar"));
        QVERIFY(mini);
        QVERIFY(!mini->isVisible());
        QTextCursor c(w.document()->story());
        c.setPosition(5);
        c.setPosition(10, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        // (A selection made with the keys brings none: only one made with the mouse.)
        QVERIFY(!mini->isVisible());
        Q_EMIT w.view()->selectionMade();
        QTRY_VERIFY(mini->isVisible());
        QRect first;
        QVERIFY(w.view()->positionRect(5, &first));
        QVERIFY2(mini->geometry().bottom() <= first.top() + 2, qPrintable(QStringLiteral("mini %1,%2 selection top %3").arg(mini->x()).arg(mini->y()).arg(first.top())));
        QVERIFY(w.view()->viewport()->rect().contains(mini->geometry()));
        shot(&w, QStringLiteral("mini-toolbar.png"));
        // Its buttons are the window's own commands: Bold takes the words bold, in one step.
        QToolButton *bold = nullptr;
        for (QToolButton *b : mini->findChildren<QToolButton *>())
            if (b->defaultAction() == w.act(QStringLiteral("fmt.bold"))) bold = b;
        QVERIFY(bold);
        QVERIFY(mini->findChildren<QToolButton *>().size() >= 6);
        bold->click();
        QTextCursor probe(w.document()->story());
        probe.setPosition(6);
        probe.setPosition(7, QTextCursor::KeepAnchor);
        QVERIFY(probe.charFormat().fontWeight() >= QFont::DemiBold);
        QVERIFY(mini->isVisible());   // (it stays for the next choice)
        // Collapsing the selection, or typing, takes it away.
        c.setPosition(2);
        w.view()->setCursor(c);
        QTRY_VERIFY(!mini->isVisible());
        c.setPosition(5);
        c.setPosition(10, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        Q_EMIT w.view()->selectionMade();
        QTRY_VERIFY(mini->isVisible());
        w.view()->insertText(QStringLiteral("x"));
        QTRY_VERIFY(!mini->isVisible());
    }

    void theMiniToolbarHasTheFontBoxAndTheFontColorButton()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(para(run("Some words to pick from here")))), QString());
        QTextCursor c(w.document()->story());
        c.setPosition(5);
        c.setPosition(10, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        Q_EMIT w.view()->selectionMade();
        auto *mini = w.view()->findChild<QFrame *>(QStringLiteral("miniToolbar"));
        QVERIFY(mini);
        QTRY_VERIFY(mini->isVisible());
        auto *font = mini->findChild<FontCombo *>();
        auto *color = mini->findChild<ColorButton *>();
        QVERIFY(font && color);
        shot(&w, QStringLiteral("mini-toolbar.png"));
        // The font box follows the selection and sets the font of it.
        Q_EMIT font->familyChosen(QStringLiteral("Courier New"));
        QTextCursor in(w.document()->story());
        in.setPosition(7);
        in.setPosition(8, QTextCursor::KeepAnchor);
        QCOMPARE(in.charFormat().fontFamilies().toStringList().value(0), QStringLiteral("Courier New"));
        Q_EMIT color->colorPicked(ColorRef::rgb(QColor(0x12, 0x34, 0x56)));
        QCOMPARE(in.charFormat().foreground().color(), QColor(0x12, 0x34, 0x56));
        QVERIFY(mini->isVisible());
    }

    void manageStylesChangesWhichStylesAreRecommendedHiddenAndInWhatOrderInOneStep()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("a paragraph")))), QString());
        QVERIFY(w.act(QStringLiteral("style.manage")));
        auto recommended = [&] {
            QStringList ids;
            for (const ListedStyle &e : listStyles(*w.document(), StyleFilter::Recommended)) ids << e.id;
            return ids;
        };
        QVERIFY(recommended().contains(QStringLiteral("Heading1")));
        QVERIFY(recommended().contains(QStringLiteral("Heading2")));
        withDialog([&w] { w.act(QStringLiteral("style.manage"))->trigger(); }, QStringLiteral("manageStylesDialog"), [](QDialog *d) {
            auto *list = d->findChild<QListWidget *>(QStringLiteral("manageStylesList"));
            QVERIFY(list);
            auto pick = [&](const QString &id) {
                for (int i = 0; i < list->count(); ++i)
                    if (list->item(i)->data(Qt::UserRole).toString() == id) {
                        list->setCurrentRow(i);
                        return true;
                    }
                return false;
            };
            auto *gallery = d->findChild<QCheckBox *>(QStringLiteral("manageInGallery"));
            auto *hidden = d->findChild<QCheckBox *>(QStringLiteral("manageHidden"));
            auto *priority = d->findChild<QSpinBox *>(QStringLiteral("managePriority"));
            QVERIFY(gallery && hidden && priority);
            QVERIFY(pick(QStringLiteral("Heading1")));
            shot(d, QStringLiteral("manage-styles.png"));
            QVERIFY(gallery->isChecked());
            QVERIFY(!hidden->isChecked());
            gallery->setChecked(false);
            priority->setValue(99);
            QVERIFY(pick(QStringLiteral("Heading2")));
            hidden->setChecked(true);
            priority->setValue(50);
            QVERIFY(pick(QStringLiteral("Heading1")));   // (what was chosen for it is kept while the box is open)
            QVERIFY(!gallery->isChecked());
            QCOMPARE(priority->value(), 99);
        });
        QVERIFY(!recommended().contains(QStringLiteral("Heading1")));
        QVERIFY(!recommended().contains(QStringLiteral("Heading2")));
        const std::optional<office::wml::Style> h1 = styleOfDocument(*w.document(), QStringLiteral("Heading1"));
        QVERIFY(h1 && !h1->qFormat && h1->uiPriority.value_or(0) == 99);
        const QByteArray out = part(w.document()->toDocx(), QStringLiteral("/word/styles.xml"));
        QVERIFY2(out.contains("<w:uiPriority w:val=\"50\"/>") && out.contains("<w:hidden/>"), out.constData());
        QVERIFY(styleOfDocument(*w.document(), QStringLiteral("Heading2"))->hidden);
        // One step takes both back.
        QVERIFY(w.document()->undo());
        QVERIFY(recommended().contains(QStringLiteral("Heading1")));
        QVERIFY(recommended().contains(QStringLiteral("Heading2")));
        // Nothing changed, nothing done.
        const quint64 serial = w.document()->nextStepSerial();
        withDialog([&w] { w.act(QStringLiteral("style.manage"))->trigger(); }, QStringLiteral("manageStylesDialog"), [](QDialog *) {});
        QCOMPARE(w.document()->nextStepSerial(), serial);
    }

    void deletingAStyleOfYourOwnPutsItsTextBackToNormalInOneStepThatUndoes()
    {
        auto doc = open(makeDocx(para(run("one")) + para(run("two"))));
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        toggleBold(*doc, c);
        QString problem;
        office::wml::Style loud = styleFromSelection(*doc, c, QStringLiteral("Loud"));
        loud.basedOn = QStringLiteral("Normal");
        const QString id = createStyle(*doc, c, loud, &problem, true);
        QVERIFY2(!id.isEmpty(), qPrintable(problem));
        // A child style, based on the one to delete.
        office::wml::Style kid = styleFromSelection(*doc, c, QStringLiteral("Louder"));
        kid.basedOn = id;
        const QString kidId = createStyle(*doc, c, kid, &problem, false);
        QVERIFY2(!kidId.isEmpty(), qPrintable(problem));
        QCOMPARE(doc->story()->begin().blockFormat().stringProperty(tp::StyleName), id);
        QVERIFY(doc->story()->begin().begin().fragment().charFormat().fontWeight() >= QFont::DemiBold);
        // Built-in styles and styles the document lacks stay.
        QVERIFY(!deleteStyle(*doc, QStringLiteral("Heading1"), &problem));
        QVERIFY(!problem.isEmpty());
        QVERIFY(!deleteStyle(*doc, QStringLiteral("NoSuchStyle")));
        const quint64 serial = doc->nextStepSerial();
        QVERIFY(deleteStyle(*doc, id, &problem));
        QCOMPARE(doc->nextStepSerial(), serial + 1);
        QVERIFY(!doc->styleSheet().find(id));
        QVERIFY(doc->styleSheet().find(kidId));
        QCOMPARE(doc->styleSheet().find(kidId)->basedOn, QStringLiteral("Normal"));
        QVERIFY(doc->story()->begin().blockFormat().stringProperty(tp::StyleName).isEmpty());
        QVERIFY(doc->story()->begin().begin().fragment().charFormat().fontWeight() < QFont::DemiBold);
        for (const ListedStyle &e : listStyles(*doc, StyleFilter::All)) QVERIFY(e.id != id);
        const QByteArray saved = doc->toDocx();
        QVERIFY(!part(saved, QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Loud\""));
        QVERIFY(!part(saved, QStringLiteral("/word/document.xml")).contains("Loud"));
        QVERIFY(doc->undo());
        QVERIFY(doc->styleSheet().find(id));
        QCOMPARE(doc->story()->begin().blockFormat().stringProperty(tp::StyleName), id);
        QVERIFY(doc->story()->begin().begin().fragment().charFormat().fontWeight() >= QFont::DemiBold);
        QCOMPARE(doc->styleSheet().find(kidId)->basedOn, id);
        QVERIFY(doc->redo());
        QVERIFY(!doc->styleSheet().find(id));
    }

    void aLockedStyleIsWrittenAndFormattingCanBeLimitedToTheStylesThatAreNotLocked()
    {
        auto doc = open(makeDocx(para(run("some words"))));
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QString problem;
        office::wml::Style loud = styleFromSelection(*doc, c, QStringLiteral("Loud"));
        loud.basedOn = QStringLiteral("Normal");
        const QString loudId = createStyle(*doc, c, loud, &problem, false);
        QVERIFY2(!loudId.isEmpty(), qPrintable(problem));
        QVERIFY(!styleLocked(*doc, loudId));
        QVERIFY(setStyleLocked(*doc, loudId, true));
        QVERIFY(styleLocked(*doc, loudId));
        QVERIFY2(part(doc->toDocx(), QStringLiteral("/word/styles.xml")).contains("<w:locked/>"), "the lock is written");
        // (Not limited yet: anything goes.)
        QVERIFY(!doc->formattingLimited());
        toggleBold(*doc, c);
        QVERIFY(!doc->takeFormattingRefused());
        QVERIFY(doc->undo());
        // Limited: no formatting by hand, no locked style, and the styles that are not locked still work.
        doc->setProtection(office::wml::protectionWithFormatting(std::nullopt, true));
        QVERIFY(doc->formattingLimited());
        const QString text = doc->story()->toPlainText();
        toggleBold(*doc, c);
        QVERIFY(doc->takeFormattingRefused());
        QVERIFY(doc->story()->begin().begin().fragment().charFormat().fontWeight() < QFont::DemiBold);
        setAlignment(*doc, c, Qt::AlignHCenter);
        QVERIFY(doc->takeFormattingRefused());
        applyParagraphStyle(*doc, c, loudId);
        QVERIFY(doc->takeFormattingRefused());
        QVERIFY(doc->story()->begin().blockFormat().stringProperty(tp::StyleName) != loudId);
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        QVERIFY(!doc->takeFormattingRefused());
        QCOMPARE(doc->story()->begin().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
        QCOMPARE(doc->story()->toPlainText(), text);
        // The settings part says so, and a file that says so is limited when it is opened.
        const QByteArray bytes = doc->toDocx();
        const QByteArray settings = part(bytes, QStringLiteral("/word/settings.xml"));
        QVERIFY2(settings.contains("<w:documentProtection") && settings.contains("w:formatting=\"1\"") && settings.contains("w:enforcement=\"1\""), settings.constData());
        auto again = open(bytes);
        QVERIFY(again->formattingLimited());
        QVERIFY(styleLocked(*again, loudId));
        // Limited off: the element goes (it said nothing else).
        QVERIFY(!office::wml::protectionWithFormatting(again->protection(), false).formatting);
        again->setProtection(std::nullopt);
        QVERIFY(!again->formattingLimited());
        QVERIFY(!part(again->toDocx(), QStringLiteral("/word/settings.xml")).contains("documentProtection"));
        // A protection that is there keeps what else it says.
        const office::wml::DocumentProtection edit = office::wml::makeDocumentProtection(QStringLiteral("trackedChanges"), QString());
        const office::wml::DocumentProtection both = office::wml::protectionWithFormatting(edit, true);
        QVERIFY(both.formatting && both.enforced && both.edit == QLatin1String("trackedChanges") && both.xml.contains("w:edit=\"trackedChanges\""));
        QVERIFY(!office::wml::protectionWithFormatting(both, false).xml.contains("formatting"));
    }

    void exportStylesWritesATemplateThatImportStylesCanTakeThemFrom()
    {
        auto doc = open(makeDocx(para(run("some words"))));
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        toggleBold(*doc, c);
        QString problem;
        office::wml::Style loud = styleFromSelection(*doc, c, QStringLiteral("Loud"));
        loud.basedOn = QStringLiteral("Normal");
        const QString loudId = createStyle(*doc, c, loud, &problem, false);
        QVERIFY2(!loudId.isEmpty(), qPrintable(problem));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("styles.dotx"));
        QVERIFY2(exportStyles(*doc, {loudId}, path, &problem), qPrintable(problem));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        QVERIFY(bytes.size() > 500);
        office::Package pack;
        QVERIFY(pack.read(bytes));
        QVERIFY2(pack.contentType(QStringLiteral("/word/document.xml")).contains(QLatin1String("template.main+xml")), "a template");
        auto back = open(bytes);
        QVERIFY(back);
        const office::wml::Style *got = back->styleSheet().find(loudId);
        QVERIFY(got);
        QCOMPARE(got->name, QStringLiteral("Loud"));
        // Nothing to write to is a problem said, and no file.
        QVERIFY(!exportStyles(*doc, {loudId}, dir.filePath(QStringLiteral("no/such/folder/x.dotx")), &problem));
        QVERIFY(!problem.isEmpty());
        // The dialog's own button: styles to check, then a file (here the dialog is only looked at).
        auto target = open(makeDocx(para(run("target"))));
        QCOMPARE(importStyles(*target, *back, {loudId}, false, &problem), 1);
    }

    void manageStylesLocksAStyleAndLimitsFormattingFromTheDialog()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("a paragraph")))), QString());
        QVERIFY(!w.document()->formattingLimited());
        withDialog([&w] { w.act(QStringLiteral("style.manage"))->trigger(); }, QStringLiteral("manageStylesDialog"), [](QDialog *d) {
            auto *list = d->findChild<QListWidget *>(QStringLiteral("manageStylesList"));
            QVERIFY(list);
            for (int i = 0; i < list->count(); ++i)
                if (list->item(i)->data(Qt::UserRole).toString() == QLatin1String("Heading2")) list->setCurrentRow(i);
            auto *locked = d->findChild<QCheckBox *>(QStringLiteral("manageLocked"));
            auto *limit = d->findChild<QCheckBox *>(QStringLiteral("manageLimitFormatting"));
            QVERIFY(locked && limit && d->findChild<QPushButton *>(QStringLiteral("manageExport")));
            QVERIFY(!locked->isChecked() && !limit->isChecked());
            locked->setChecked(true);
            limit->setChecked(true);
            shot(d, QStringLiteral("manage-styles-restrict.png"));
        });
        QVERIFY(styleLocked(*w.document(), QStringLiteral("Heading2")));
        QVERIFY(w.document()->formattingLimited());
        // The Bold command is refused, and the window says why.
        QVERIFY(w.act(QStringLiteral("fmt.bold")));
        w.act(QStringLiteral("fmt.bold"))->trigger();
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("limited")));
        // Turned off again from the dialog: the protection goes, the lock stays.
        withDialog([&w] { w.act(QStringLiteral("style.manage"))->trigger(); }, QStringLiteral("manageStylesDialog"), [](QDialog *d) {
            auto *limit = d->findChild<QCheckBox *>(QStringLiteral("manageLimitFormatting"));
            QVERIFY(limit && limit->isChecked());
            limit->setChecked(false);
        });
        QVERIFY(!w.document()->formattingLimited());
        QVERIFY(!w.document()->protection().has_value());
        QVERIFY(styleLocked(*w.document(), QStringLiteral("Heading2")));
    }

    void importingStylesFromAnotherDocumentCopiesThemWithTheirBaseInOneStep()
    {
        auto from = open(makeDocx(para(run("source text"))));
        QTextCursor c(from->story());
        c.setPosition(0);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        toggleBold(*from, c);
        QString problem;
        office::wml::Style loud = styleFromSelection(*from, c, QStringLiteral("Loud"));
        loud.basedOn = QStringLiteral("Normal");
        const QString loudId = createStyle(*from, c, loud, &problem, true);
        QVERIFY2(!loudId.isEmpty(), qPrintable(problem));
        office::wml::Style kin = styleFromSelection(*from, c, QStringLiteral("Kin"));
        kin.basedOn = loudId;
        const QString kinId = createStyle(*from, c, kin, &problem, false);
        QVERIFY2(!kinId.isEmpty(), qPrintable(problem));
        auto to = open(makeDocx(para(run("target text")) + para(run("more"))));
        QVERIFY(!to->styleSheet().find(loudId) && !to->styleSheet().find(kinId));
        const quint64 serial = to->nextStepSerial();
        // One style asked for brings the style it is based on.
        QCOMPARE(importStyles(*to, *from, {kinId}, false, &problem), 2);
        QCOMPARE(to->nextStepSerial(), serial + 1);
        QVERIFY(to->styleSheet().find(loudId) && to->styleSheet().find(kinId));
        QCOMPARE(to->styleSheet().find(kinId)->basedOn, loudId);
        QCOMPARE(to->styleSheet().find(loudId)->rPr.bold, std::optional<bool>(true));
        QVERIFY(to->undo());
        QVERIFY(!to->styleSheet().find(loudId) && !to->styleSheet().find(kinId));
        QVERIFY(to->redo());
        // Styles the document has already are left alone unless asked to replace them; a replaced style restyles its text.
        QCOMPARE(importStyles(*to, *from, {kinId, loudId}, false, &problem), 0);
        office::wml::Style italicLoud = *from->styleSheet().find(loudId);
        italicLoud.rPr.bold = false;
        italicLoud.rPr.italic = true;
        QTextCursor none;
        QVERIFY(modifyStyle(*from, none, loudId, italicLoud, &problem));
        QCOMPARE(importStyles(*to, *from, {loudId}, true, &problem), 1);
        QCOMPARE(to->styleSheet().find(loudId)->rPr.italic, std::optional<bool>(true));
        // A style that is not in the other document, or is a table's, is no style to import.
        QCOMPARE(importStyles(*to, *from, {QStringLiteral("NoSuchStyle"), QStringLiteral("TableGrid")}, true, &problem), 0);
    }

    void theVerticalRulerShowsTheMarginsOfThePagesAndComesAndGoesWithTheCommand()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 900);
        w.show();
        w.setDocument(open(makeDocx(para(run("one")) + para(run("two")))), QString());
        QVERIFY(w.act(QStringLiteral("view.verticalRuler")));
        auto *ruler = w.findChild<write::VerticalRuler *>(QStringLiteral("verticalRuler"));
        QVERIFY(ruler);
        QVERIFY(!w.act(QStringLiteral("view.verticalRuler"))->isChecked());
        QVERIFY(!ruler->isVisible());
        w.act(QStringLiteral("view.verticalRuler"))->trigger();
        QTRY_VERIFY(ruler->isVisible());
        QVERIFY(ruler->mapToGlobal(QPoint(ruler->width(), 0)).x() <= w.view()->mapToGlobal(QPoint(0, 0)).x() + 2);   // (left of the pages)
        double top = 0, bottom = 0;
        QTRY_VERIFY(ruler->textAreaOf(0, &top, &bottom));
        const QRect page = w.view()->pageRect(0);
        const double scale = w.view()->zoom() * 96.0 / 72.0;
        const double pageTop = ruler->mapFromGlobal(w.view()->viewport()->mapToGlobal(QPoint(0, page.top()))).y();
        QVERIFY2(std::abs((top - pageTop) - 72 * scale) < 2, qPrintable(QString::number(top - pageTop)));   // (the page's top margin: an inch)
        QVERIFY(bottom > top + 100);
        // The text area is white and the margins are gray, in the ruler's own picture.
        const QImage img = ruler->grab().toImage();
        const int mid = int((top + bottom) / 2), inMargin = int(pageTop + 20 * scale);
        QVERIFY(mid < img.height() && inMargin >= 0 && inMargin < img.height());
        QVERIFY(img.pixelColor(img.width() / 2, mid).lightness() > img.pixelColor(img.width() / 2, inMargin).lightness());
        shot(&w, QStringLiteral("vertical-ruler.png"));
        // The tree of the outline view has no pages: the ruler goes, and comes back with the pages.
        w.act(QStringLiteral("view.outline"))->trigger();
        QTRY_VERIFY(!ruler->isVisible());
        w.act(QStringLiteral("view.printLayout"))->trigger();
        QTRY_VERIFY(ruler->isVisible());
        w.act(QStringLiteral("view.verticalRuler"))->trigger();
        QTRY_VERIFY(!ruler->isVisible());
    }

    void manageStylesCanDeleteAStyleOfYourOwnAndImportStylesFromAnotherDocument()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("a paragraph")))), QString());
        QString problem;
        QTextCursor c(w.document()->story());
        c.setPosition(0);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        office::wml::Style mine = styleFromSelection(*w.document(), c, QStringLiteral("Mine"));
        mine.basedOn = QStringLiteral("Normal");
        const QString mineId = createStyle(*w.document(), c, mine, &problem, false);
        QVERIFY2(!mineId.isEmpty(), qPrintable(problem));
        withDialog([&w] { w.act(QStringLiteral("style.manage"))->trigger(); }, QStringLiteral("manageStylesDialog"), [&](QDialog *d) {
            auto *list = d->findChild<QListWidget *>(QStringLiteral("manageStylesList"));
            auto *del = d->findChild<QPushButton *>(QStringLiteral("manageDelete"));
            auto *imp = d->findChild<QPushButton *>(QStringLiteral("manageImport"));
            QVERIFY(list && del && imp);
            auto pick = [&](const QString &id) {
                for (int i = 0; i < list->count(); ++i)
                    if (list->item(i)->data(Qt::UserRole).toString() == id) {
                        list->setCurrentRow(i);
                        return true;
                    }
                return false;
            };
            QVERIFY(pick(QStringLiteral("Heading1")));
            QVERIFY(!del->isEnabled());   // (a style that comes with the program stays)
            QVERIFY(pick(mineId));
            QVERIFY(del->isEnabled());
            shot(d, QStringLiteral("manage-styles-delete.png"));
            del->click();
            QVERIFY(!pick(mineId));
        });
        QVERIFY(!w.document()->styleSheet().find(mineId));
        QVERIFY(w.document()->undo());
        QVERIFY(w.document()->styleSheet().find(mineId));
        // Import: a document with two styles of its own; check one, and it comes (and the style it is based on).
        auto from = open(makeDocx(para(run("other"))));
        QTextCursor fc(from->story());
        fc.setPosition(0);
        fc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        office::wml::Style base = styleFromSelection(*from, fc, QStringLiteral("Base Look"));
        base.basedOn = QStringLiteral("Normal");
        const QString baseId = createStyle(*from, fc, base, &problem, false);
        office::wml::Style top = styleFromSelection(*from, fc, QStringLiteral("Top Look"));
        top.basedOn = baseId;
        const QString topId = createStyle(*from, fc, top, &problem, false);
        QVERIFY(!baseId.isEmpty() && !topId.isEmpty());
        bool imported = false;
        withDialog([&] { imported = importStylesDialog(&w, w.document(), *from); }, QStringLiteral("importStylesDialog"), [&](QDialog *d) {
            auto *list = d->findChild<QListWidget *>(QStringLiteral("importStylesList"));
            QVERIFY(list);
            QCOMPARE(list->count(), 2);
            shot(d, QStringLiteral("import-styles.png"));
            for (int i = 0; i < list->count(); ++i)
                if (list->item(i)->data(Qt::UserRole).toString() == topId) list->item(i)->setCheckState(Qt::Checked);
        });
        QVERIFY(imported);
        QVERIFY(w.document()->styleSheet().find(topId) && w.document()->styleSheet().find(baseId));
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
    // The window has the command search box, in the ribbon's top row, and its extra words name only what the app has.
    void theCommandSearchBoxIsInTheWindow()
    {
        Q_INIT_RESOURCE(write);   // (JeffWrite's ribbon and the command search's words)
        WriteWindow w;
        w.resize(1000, 700);
        w.show();
        const QString problem = jptest::commandSearchProblem(w, QStringLiteral(":/write/cmdsearch.json"), QStringLiteral("page size"), QStringLiteral("Size"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        jptest::saveCommandSearchPicture(w, QStringLiteral("jeffwrite"), QStringLiteral("page size"));
    }

    // In a read-only document the controls that change it are turned off, the menus' items too: the search lists them grey, and Enter runs nothing.
    void aReadOnlyDocumentsTurnedOffMenuItemsAreNotRunFromTheSearch()
    {
        Q_INIT_RESOURCE(write);
        WriteWindow w;
        w.resize(1000, 700);
        w.show();
        w.setReadOnly(true);
        auto *box = w.findChild<jp::CommandSearchBox *>();
        QVERIFY(box);
        const QList<jp::cmdsearch::Entry> all = jp::cmdsearch::collect(&w, jp::cmdsearch::Words());
        const jp::cmdsearch::Entry *target = nullptr;
        for (const auto &e : all)   // a menu item that is on itself, in a menu whose button was turned off
            if (e.kind == jp::cmdsearch::Kind::Command && e.action && e.action->isEnabled() && e.button && !e.button->isEnabled() && !e.opensMenu && !e.name.isEmpty() && !e.action->objectName().isEmpty()) {
                target = &e;
                break;
            }
        QVERIFY2(target, "no menu item under a turned-off button");
        QSignalSpy ran(target->action, &QAction::triggered);
        QVERIFY(!jp::cmdsearch::available(*target, &w));
        box->setText(target->name);
        const int row = [&] {
            for (int i = 0; i < box->list()->count(); ++i)
                if (box->list()->item(i)->data(Qt::UserRole).toInt() >= 0 && box->shownResults().value(box->list()->item(i)->data(Qt::UserRole).toInt()).id == target->id) return i;
            return -1;
        }();
        QVERIFY(row >= 0);
        QVERIFY(!(box->list()->item(row)->flags() & Qt::ItemIsEnabled));
        for (int i = 0; i < 12; ++i) QTest::keyClick(box, Qt::Key_Down);
        QTest::keyClick(box, Qt::Key_Return);
        QCOMPARE(ran.count(), 0);
    }

    // The search's time with every gallery item of the ribbon: the index (every tab, menu, and gallery) made when the box gets the cursor, and a keystroke's search over it.
    void theSearchKeepsUpWithEveryGalleryItemOfTheRibbon()
    {
        Q_INIT_RESOURCE(write);
        WriteWindow w;
        w.resize(1000, 700);
        w.show();
        QElapsedTimer clock;
        clock.start();
        const QList<jp::cmdsearch::Entry> all = jp::cmdsearch::collect(&w, jp::cmdsearch::Words());
        const qint64 indexMs = clock.elapsed();
        int gallery = 0;
        for (const auto &e : all) {
            QVERIFY(e.indexed);
            if (e.kind == jp::cmdsearch::Kind::Gallery) ++gallery;
        }
        const QStringList queries = {QStringLiteral("h"), QStringLiteral("he"), QStringLiteral("hea"), QStringLiteral("head"), QStringLiteral("headi"), QStringLiteral("heading"),
                                     QStringLiteral("page siz"), QStringLiteral("paragrph"), QStringLiteral("line spacing"), QStringLiteral("table style")};
        clock.restart();
        for (int round = 0; round < 20; ++round)
            for (const QString &q : queries) jp::cmdsearch::find(all, q);
        const double perKeystroke = double(clock.nsecsElapsed()) / 1e6 / (20.0 * queries.size());
        qInfo("search timing: %d entries (%d gallery items), index %lld ms, %.3f ms per keystroke", int(all.size()), gallery, indexMs, perKeystroke);
    }
};

QTEST_MAIN(HomeTests)
#include "tst_write_home.moc"
