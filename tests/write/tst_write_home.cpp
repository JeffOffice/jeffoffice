// Tests of JeffWrite's Home and Review tab commands that work on text and settings (apps/write/core/homeops.*, proofops.*, and the window's actions).
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/writewindow.h"
#include "app/pageview.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/homeops.h"
#include "core/proofops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textprops.h"

#include <QAction>
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
};

QTEST_MAIN(HomeTests)
#include "tst_write_home.moc"
