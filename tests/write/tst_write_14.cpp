#include "tst_write.h"
#include "core/linkops.h"
#include "text/autocorrect.h"

namespace {
struct Typer {
    std::unique_ptr<WriteDoc> doc;
    PageView view;
    explicit Typer(const QByteArray &body = "<w:p><w:r><w:t></w:t></w:r></w:p>")
    {
        doc = open(makeDocx(body));
        view.resize(1000, 900);
        view.show();
        view.setDocument(doc.get());
        view.paginateAll();
        view.cursor().movePosition(QTextCursor::End);
    }
    void type(const QString &s) { QTest::keyClicks(&view, s); }
    QString text() const { return doc->story()->toPlainText(); }
};
} // namespace

// The text rules that JeffPub and JeffWrite share are plain functions of a paragraph's text.
void WriteTests::theSharedAutoCorrectRulesSayWhatToPutInPlaceOfWhat()
{
    using namespace jp::autotext;
    const auto word = correctWord(QStringLiteral("say teh"), 7, true);
    QVERIFY(word);
    QCOMPARE(word->from, 4);
    QCOMPARE(word->with, QStringLiteral("the"));
    QVERIFY(!correctWord(QStringLiteral("say the"), 7, true));
    const auto dash = formatWord(QStringLiteral("a--b"));
    QVERIFY(dash);
    QCOMPARE(dash->with, QString(QChar(0x2014)));
    QCOMPARE(formatWord(QStringLiteral("x 1/2"))->with, QStringLiteral("½"));
    QVERIFY(formatWord(QStringLiteral("the 2nd"))->superscript);
    QCOMPARE(smartQuote(QStringLiteral("\""), QChar()), QStringLiteral("“"));
    QCOMPARE(smartQuote(QStringLiteral("\""), QLatin1Char('a')), QStringLiteral("”"));
    QVERIFY(listStart(QStringLiteral("1.")) && listStart(QStringLiteral("1."))->numbered);
    QVERIFY(!listStart(QStringLiteral("a.")));
    const auto link = linkWord(QStringLiteral("see www.example.com."));
    QVERIFY(link);
    QCOMPARE(link->from, 4);
    QCOMPARE(link->to, 19);
    QCOMPARE(link->with, QStringLiteral("https://www.example.com"));
    QVERIFY(linkWord(QStringLiteral("mail me@example.org")));
    const auto bracketed = linkWord(QStringLiteral("(www.example.com)"));
    QVERIFY(bracketed);
    QCOMPARE(bracketed->from, 1);
    QCOMPARE(bracketed->to, 16);
    QVERIFY(!linkWord(QStringLiteral("plain words")));
    const auto mark = markWord(QStringLiteral("a *bold*"));
    QVERIFY(mark);
    QCOMPARE(mark->from, 2);
    QCOMPARE(mark->to, 8);
    QCOMPARE(mark->mark, QLatin1Char('*'));
    QVERIFY(!markWord(QStringLiteral("snake_case_name")));
    QVERIFY(ruleLine(QStringLiteral("---")));
    QVERIFY(!ruleLine(QStringLiteral("--")));
    QVERIFY(!ruleLine(QStringLiteral("-=-")));
}

void WriteTests::typingFixesSlipsSymbolsAndSentenceCapitalsAndOneUndoTakesJustTheFix()
{
    Typer t;
    t.type(QStringLiteral("teh cat said \"hi\" and i didnt mind (c) "));
    QCOMPARE(t.text(), QStringLiteral("The cat said “hi” and I didn’t mind © "));
    // One Undo takes back the last fix and leaves what was typed.
    t.view.undo();
    QCOMPARE(t.text(), QStringLiteral("The cat said “hi” and I didn’t mind (c) "));
    t.view.redo();
    QCOMPARE(t.text(), QStringLiteral("The cat said “hi” and I didn’t mind © "));
    // The first fix undone leaves the word as typed, and the space.
    Typer u;
    u.type(QStringLiteral("teh "));
    QCOMPARE(u.text(), QStringLiteral("The "));
    u.view.undo();
    QCOMPARE(u.text(), QStringLiteral("teh "));
    // Off: nothing is changed.
    const SettingGuard noCorrect(QStringLiteral("proof/autocorrect"), false, true), noQuotes(QStringLiteral("proof/smartQuotes"), false, true);
    Typer v;
    v.type(QStringLiteral("teh \"x\" "));
    QCOMPARE(v.text(), QStringLiteral("teh \"x\" "));
}

void WriteTests::typingMakesDashesFractionsAndRaisedOrdinalsAndOneUndoTakesJustThat()
{
    Typer t;
    t.type(QStringLiteral("Wait--what 1/2 of the 2nd "));
    QCOMPARE(t.text(), QStringLiteral("Wait—what ½ of the 2nd "));
    const int at = t.text().indexOf(QStringLiteral("2nd"));
    QVERIFY(styCharAt(*t.doc, at + 2).verticalAlignment() == QTextCharFormat::AlignSuperScript);
    QVERIFY(styCharAt(*t.doc, at + 3).verticalAlignment() != QTextCharFormat::AlignSuperScript);
    t.view.undo();
    QVERIFY(styCharAt(*t.doc, at + 2).verticalAlignment() != QTextCharFormat::AlignSuperScript);
    QCOMPARE(t.text(), QStringLiteral("Wait—what ½ of the 2nd "));
    Typer u;
    u.type(QStringLiteral("A - b "));
    QCOMPARE(u.text(), QStringLiteral("A – b "));
    u.view.undo();
    QCOMPARE(u.text(), QStringLiteral("A - b "));
    const SettingGuard noFormat(QStringLiteral("proof/autoformat"), false, true);
    Typer v;
    v.type(QStringLiteral("a--b 1/2 "));
    QCOMPARE(v.text(), QStringLiteral("a--b 1/2 "));
}

void WriteTests::typingOneAndAPeriodOrAStarAndASpaceStartsAListInWritesOwnListModelAndOneUndoTakesItBack()
{
    Typer t;
    t.type(QStringLiteral("1. First"));
    QCOMPARE(t.text(), QStringLiteral("First"));
    QCOMPARE(listKindOf(*t.doc, t.doc->story()->firstBlock()), ListKind::Numbers);
    t.view.undo();   // the typing of the words
    t.view.undo();   // the list
    QCOMPARE(t.text(), QStringLiteral("1. "));
    QCOMPARE(listKindOf(*t.doc, t.doc->story()->firstBlock()), ListKind::None);
    Typer u;
    u.type(QStringLiteral("* Item"));
    QCOMPARE(u.text(), QStringLiteral("Item"));
    QCOMPARE(listKindOf(*u.doc, u.doc->story()->firstBlock()), ListKind::Bullets);
    // Not at the start of a paragraph: no list.
    Typer v;
    v.type(QStringLiteral("a 1. b"));
    QCOMPARE(listKindOf(*v.doc, v.doc->story()->firstBlock()), ListKind::None);
}

void WriteTests::aRowOfDashesAndEnterGivesTheParagraphABottomBorderAndOneUndoTakesItBack()
{
    auto bottom = [](Typer &t, int block) {
        const QTextBlock b = t.doc->story()->findBlockByNumber(block);
        return tp::ParaBox::parse(b.blockFormat().stringProperty(tp::Borders)).bottom;
    };
    Typer t(QByteArray("<w:p><w:r><w:t>Above</w:t></w:r></w:p><w:p><w:r><w:t></w:t></w:r></w:p>"));
    t.view.cursor().movePosition(QTextCursor::End);
    t.type(QStringLiteral("---"));
    QTest::keyClick(&t.view, Qt::Key_Return);
    QCOMPARE(t.doc->story()->blockCount(), 3);
    QCOMPARE(t.doc->story()->findBlockByNumber(1).text(), QString());
    QVERIFY(bottom(t, 1).visible());
    QCOMPARE(bottom(t, 1).style, QStringLiteral("single"));
    QVERIFY(!bottom(t, 2).visible());
    QCOMPARE(t.view.cursor().blockNumber(), 2);
    t.view.undo();
    QCOMPARE(t.doc->story()->blockCount(), 2);
    QCOMPARE(t.doc->story()->findBlockByNumber(1).text(), QStringLiteral("---"));
    QVERIFY(!bottom(t, 1).visible());
    // = gives a double line.
    Typer u;
    u.type(QStringLiteral("==="));
    QTest::keyClick(&u.view, Qt::Key_Return);
    QCOMPARE(bottom(u, 0).style, QStringLiteral("double"));
    // Two dashes, or other words: Enter is only Enter.
    Typer v;
    v.type(QStringLiteral("--"));
    QTest::keyClick(&v.view, Qt::Key_Return);
    QVERIFY(!bottom(v, 0).visible());
    QCOMPARE(v.doc->story()->findBlockByNumber(0).text(), QStringLiteral("--"));
}

void WriteTests::aWebOrEmailAddressFollowedBySpaceBecomesALinkAndOneUndoTakesTheLinkBack()
{
    Typer t;
    t.type(QStringLiteral("Visit www.example.com now"));
    const int at = t.text().indexOf(QStringLiteral("www"));
    jp::write::Hyperlink h = jp::write::hyperlinkAt(*t.doc, at + 2);
    QVERIFY(h.valid());
    QCOMPARE(h.address, QStringLiteral("https://www.example.com"));
    QVERIFY(!jp::write::hyperlinkAt(*t.doc, t.text().indexOf(QStringLiteral("now")) + 1).valid());
    t.view.undo();   // the words typed after the link, with the space
    t.view.undo();   // the link
    QVERIFY(!jp::write::hyperlinkAt(*t.doc, at + 2).valid());
    QCOMPARE(t.text(), QStringLiteral("Visit www.example.com "));
    Typer u;
    u.type(QStringLiteral("Write to me@example.org, please"));
    h = jp::write::hyperlinkAt(*u.doc, u.text().indexOf(QStringLiteral("me@")) + 2);
    QVERIFY(h.valid());
    QCOMPARE(h.address, QStringLiteral("mailto:me@example.org"));
    // The comma is not in the link.
    QVERIFY(!jp::write::hyperlinkAt(*u.doc, u.text().indexOf(QLatin1Char(',')) + 1).valid());
}

void WriteTests::starredWordsBecomeBoldAndUnderscoredWordsBecomeUnderlinedAndOneUndoTakesJustThat()
{
    Typer t;
    t.type(QStringLiteral("Is *bold* and _under_ words "));
    QCOMPARE(t.text(), QStringLiteral("Is bold and under words "));
    QVERIFY(styCharAt(*t.doc, t.text().indexOf(QStringLiteral("bold")) + 1).fontWeight() > QFont::Normal);
    QVERIFY(styCharAt(*t.doc, t.text().indexOf(QStringLiteral("under")) + 1).fontUnderline());
    QVERIFY(styCharAt(*t.doc, t.text().indexOf(QStringLiteral("words")) + 1).fontWeight() <= QFont::Normal);
    QVERIFY(!styCharAt(*t.doc, t.text().indexOf(QStringLiteral("words")) + 1).fontUnderline());
    QVERIFY(styCharAt(*t.doc, t.text().indexOf(QStringLiteral("and")) + 1).fontWeight() <= QFont::Normal);
    // Undo, one step at a time: the typing of the last word, then the underline fix, then the typing before it, then the bold fix.
    t.view.undo();
    QCOMPARE(t.text(), QStringLiteral("Is bold and under "));
    t.view.undo();
    QCOMPARE(t.text(), QStringLiteral("Is bold and _under_ "));
    Typer w;
    w.type(QStringLiteral("Is *bold* "));
    w.view.undo();
    QCOMPARE(w.text(), QStringLiteral("Is *bold* "));
    // A name with underscores stays as it is.
    Typer u;
    u.type(QStringLiteral("snake_case_name "));
    QCOMPARE(u.text(), QStringLiteral("snake_case_name "));
}

void WriteTests::aQuoteTypedOverASelectionIsCurledAndAQuoteAfterADigitOrABracketIsChosenByIt()
{
    Typer t(QByteArray("<w:p><w:r><w:t>Say x</w:t></w:r></w:p>"));
    t.view.cursor().setPosition(4);
    t.view.cursor().setPosition(5, QTextCursor::KeepAnchor);
    t.type(QStringLiteral("\""));
    QCOMPARE(t.text(), QStringLiteral("Say “"));
    Typer u;
    u.type(QStringLiteral("5\" ("));
    u.type(QStringLiteral("\"x') 'y"));
    QCOMPARE(u.text(), QStringLiteral("5” (“x’) ‘y"));
}

void WriteTests::aNumberAndAParenthesisOrANumberOtherThanOneStartsNoListInWrite()
{
    Typer t;
    t.type(QStringLiteral("2. Second"));
    QCOMPARE(t.text(), QStringLiteral("2. Second"));
    QCOMPARE(listKindOf(*t.doc, t.doc->story()->firstBlock()), ListKind::None);
    Typer u;
    u.type(QStringLiteral("1) First"));
    QCOMPARE(u.text(), QStringLiteral("1) First"));
    QCOMPARE(listKindOf(*u.doc, u.doc->story()->firstBlock()), ListKind::None);
    Typer v;
    v.type(QStringLiteral("- Dash"));
    QCOMPARE(listKindOf(*v.doc, v.doc->story()->firstBlock()), ListKind::Bullets);
}

void WriteTests::withTrackChangesOnTheFixIsTrackedAndDeletedTextIsNotPartOfTheWord()
{
    Typer t;
    t.doc->setTrackChanges(true);
    t.type(QStringLiteral("Say teh "));
    // The mended word is there, and the marks of the tracked changes are in the text round it.
    QVERIFY(t.text().contains(QStringLiteral("the")));
    QVERIFY(t.text().contains(QChar::ObjectReplacementCharacter));
    // Words that were deleted are not read as the text before the word: "Bye." is gone, so the word after "Hi. " starts a sentence.
    Typer u(QByteArray("<w:p><w:r><w:t>Hi. Bye.</w:t></w:r></w:p>"));
    u.doc->setTrackChanges(true);
    QTextCursor gone(u.doc->story());
    gone.setPosition(4);
    gone.setPosition(8, QTextCursor::KeepAnchor);
    removeSelection(*u.doc, gone);
    u.view.cursor().movePosition(QTextCursor::End);
    u.type(QStringLiteral("teh "));
    QVERIFY2(u.text().contains(QStringLiteral("The ")), qPrintable(u.text()));
}

void WriteTests::aRuleKeepsTheParagraphsOtherBordersAndLeavesTheNextOneAlone()
{
    Typer t;
    t.type(QStringLiteral("---"));
    tp::ParaBox top;
    top.top.style = QStringLiteral("single");
    top.top.eighths = 4;
    changeBlockFormat(*t.doc, t.view.cursor(), [&](QTextBlockFormat &bf) { bf.setProperty(tp::Borders, top.spec()); });
    QTest::keyClick(&t.view, Qt::Key_Return);
    auto box = [&](int n) { return tp::ParaBox::parse(t.doc->story()->findBlockByNumber(n).blockFormat().stringProperty(tp::Borders)); };
    QVERIFY(box(0).top.visible());
    QVERIFY(box(0).bottom.visible());
    QVERIFY(!box(1).bottom.visible());
}

void WriteTests::theLinkRuleFollowsItsOwnBoxAndAnAddressInBracketsBecomesALinkWithoutThem()
{
    Typer t;
    t.type(QStringLiteral("See (www.example.com) now"));
    const int at = t.text().indexOf(QStringLiteral("www"));
    QVERIFY(jp::write::hyperlinkAt(*t.doc, at + 2).valid());
    QVERIFY(!jp::write::hyperlinkAt(*t.doc, at - 2).valid());
    QVERIFY(!jp::write::hyperlinkAt(*t.doc, t.text().indexOf(QLatin1Char(')')) + 1).valid());
    {
        const SettingGuard noLinks(QStringLiteral("proof/autolink"), false, true);
        Typer u;
        u.type(QStringLiteral("Go to www.example.com now"));
        QVERIFY(!jp::write::hyperlinkAt(*u.doc, u.text().indexOf(QStringLiteral("www")) + 2).valid());
    }
    // AutoFormat off does not stop the links: they have their own box.
    const SettingGuard noFormat(QStringLiteral("proof/autoformat"), false, true);
    Typer v;
    v.type(QStringLiteral("Go to www.example.com now"));
    QVERIFY(jp::write::hyperlinkAt(*v.doc, v.text().indexOf(QStringLiteral("www")) + 2).valid());
}

void WriteTests::everyKindOfFixCanBeUndoneAndDoneAgain()
{
    auto round = [&](const QString &typed, const QString &fixed, const QString &asTyped) {
        Typer t;
        t.type(typed);
        QCOMPARE(t.text(), fixed);
        t.view.undo();
        QCOMPARE(t.text(), asTyped);
        t.view.redo();
        QCOMPARE(t.text(), fixed);
    };
    round(QStringLiteral("Wait--what "), QStringLiteral("Wait—what "), QStringLiteral("Wait--what "));
    round(QStringLiteral("It is 1/2 "), QStringLiteral("It is ½ "), QStringLiteral("It is 1/2 "));
    round(QStringLiteral("Is *bold* "), QStringLiteral("Is bold "), QStringLiteral("Is *bold* "));
    round(QStringLiteral("Is _under_ "), QStringLiteral("Is under "), QStringLiteral("Is _under_ "));
    Typer l;
    l.type(QStringLiteral("Visit www.example.com "));
    const int at = l.text().indexOf(QStringLiteral("www")) + 2;
    QVERIFY(jp::write::hyperlinkAt(*l.doc, at).valid());
    l.view.undo();
    QVERIFY(!jp::write::hyperlinkAt(*l.doc, at).valid());
    l.view.redo();
    QVERIFY(jp::write::hyperlinkAt(*l.doc, at).valid());
    Typer r;
    r.type(QStringLiteral("---"));
    QTest::keyClick(&r.view, Qt::Key_Return);
    r.view.undo();
    QCOMPARE(r.doc->story()->blockCount(), 1);
    r.view.redo();
    QCOMPARE(r.doc->story()->blockCount(), 2);
    Typer n;
    n.type(QStringLiteral("1. "));
    QCOMPARE(listKindOf(*n.doc, n.doc->story()->firstBlock()), ListKind::Numbers);
    n.view.undo();
    QCOMPARE(listKindOf(*n.doc, n.doc->story()->firstBlock()), ListKind::None);
    n.view.redo();
    QCOMPARE(listKindOf(*n.doc, n.doc->story()->firstBlock()), ListKind::Numbers);
}
