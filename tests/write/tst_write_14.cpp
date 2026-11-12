#include "tst_write.h"
#include "app/grammar.h"
#include "app/spellpane.h"
#include "core/grammarcheck.h"
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


// ---- grammar as you type (core/grammarcheck, libs/shell/app/grammar)

namespace {
const QString kGrammarOn = QStringLiteral("proof/checkGrammarAsYouType");
// A sentence with one grammar issue and no misspelling ("an test"), and one with a misspelling and no grammar issue.
const QString kAnTest = QStringLiteral("This is an test of the checker.");
}

void WriteTests::checkingGrammarAsYouTypeCanBeTurnedOffInTheReviewTab()
{
    const SettingGuard grammar(kGrammarOn, true, true);   // (what the test begins with, and what the person had comes back)
    WriteWindow w;
    w.setDocument(textDoc({kAnTest}), QString());
    w.show();
    QAction *action = w.act("review.checkGrammarAsType");
    QVERIFY(action);
    QVERIFY(action->isCheckable() && action->isChecked());
    QVERIFY(w.view()->grammarEnabled());
    QVERIFY(w.act("review.checkAsType")->isChecked());   // (the spelling's choice is its own)
    action->trigger();
    QVERIFY(!action->isChecked());
    QVERIFY(!w.view()->grammarEnabled());
    QVERIFY(w.view()->spellingEnabled());
    QVERIFY(!grammarAsYouType());
    WriteWindow other;
    QVERIFY(!other.view()->grammarEnabled());   // (a window opened after remembers the choice)
    action->trigger();
    QVERIFY(w.view()->grammarEnabled());
    QVERIFY(grammarAsYouType());
}

void WriteTests::checkingGrammarAsYouTypeIsOneChoiceInOptions()
{
    const SettingGuard grammar(kGrammarOn, true, true);
    WriteWindow w, other;
    for (WriteWindow *x : {&w, &other}) {
        x->setDocument(textDoc({kAnTest}), QString());
        x->show();
    }
    QString boxText;
    bool boxWas = false;
    QTimer watch;
    watch.setInterval(20);
    QObject::connect(&watch, &QTimer::timeout, &watch, [&] {
        auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!d) return;
        watch.stop();
        for (QCheckBox *box : d->findChildren<QCheckBox *>())
            if (box->text().contains(QLatin1String("grammar as you type"), Qt::CaseInsensitive)) {
                boxText = box->text();
                boxWas = box->isChecked();
                box->setChecked(false);
            }
        d->accept();
    });
    watch.start();
    w.act("file.options")->trigger();
    QCOMPARE(boxText, QStringLiteral("Check grammar as you type"));
    QVERIFY(boxWas);
    QVERIFY(!w.view()->grammarEnabled());
    QVERIFY(!other.view()->grammarEnabled());   // (the other window, at once)
    QVERIFY(!other.act("review.checkGrammarAsType")->isChecked());
    QVERIFY(w.view()->spellingEnabled());       // (spelling was left as it was)
    QVERIFY(!Settings::get().value(kGrammarOn, true).toBool());
    WriteWindow later;
    QVERIFY(!later.view()->grammarEnabled());
}

void WriteTests::grammarIssuesAreUnderlinedInBlueAndSpellingStaysRed()
{
    const SettingGuard grammarOn(kGrammarOn, true, true), spellingOn(QStringLiteral("proof/checkAsYouType"), true, true);
    auto doc = textDoc({kAnTest + QStringLiteral(" And a recieve here.")});
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.grammar()->setDelayMs(0);
    view.show();
    // Where the issue is: from the checker itself.
    const auto issues = grammarCheck(doc->story()->toPlainText(), QString());
    QVERIFY(!issues.isEmpty());
    const GrammarIssue issue = issues.first();
    QTextCursor &c = view.cursor();
    c.setPosition(issue.start);
    view.setCursor(c);
    QRect start;
    QVERIFY(view.caretRect(&start));
    c.setPosition(issue.end);
    view.setCursor(c);
    QRect end;
    QVERIFY(view.caretRect(&end));
    c.setPosition(doc->story()->characterCount() - 1);   // (away from it: the caret is not in a word being typed)
    view.setCursor(c);
    int blue = 0, red = 0;
    QRect line;
    QTRY_VERIFY((line = blueBounds(view.grab().toImage(), view.pageRect(0), &blue), blue > 3));
    // Under the issue and nowhere else, just below its letters.
    QVERIFY2(line.left() >= start.left() - 3 && line.right() <= end.left() + 3, qPrintable(QStringLiteral("%1..%2 for the issue at %3..%4").arg(line.left()).arg(line.right()).arg(start.left()).arg(end.left())));
    QVERIFY(line.top() >= start.top() && line.bottom() <= start.bottom() + 8);
    // The misspelled word keeps its red wave, and the issue has no red.
    redBounds(view.grab().toImage(), view.pageRect(0), &red);
    QVERIFY(red > 8);
    QVERIFY(redBounds(view.grab().toImage(), view.pageRect(0)).left() > line.right());
    // As printed, and in the proof the picture of a page makes, there is none.
    blueBounds(view.pageImage(0, 900), QRect(0, 0, 900, 1200), &blue);
    QCOMPARE(blue, 0);
    // Switched off, it goes; on again, it comes back.
    view.setGrammarEnabled(false);
    blueBounds(view.grab().toImage(), view.pageRect(0), &blue);
    QCOMPARE(blue, 0);
    view.setGrammarEnabled(true);
    QTRY_VERIFY((blueBounds(view.grab().toImage(), view.pageRect(0), &blue), blue > 3));
}

void WriteTests::aGrammarIssueOffersItsMessageAndSuggestionsAndAcceptingOneUndoes()
{
    const SettingGuard grammarOn(kGrammarOn, true, true);
    WriteWindow w;
    w.setDocument(textDoc({kAnTest}), QString());
    w.show();
    w.view()->grammar()->setDelayMs(0);
    const int length = w.document()->story()->characterCount();
    QTRY_VERIFY(!w.view()->grammar()->issuesIn(0, length).isEmpty());
    const GrammarCheck::Issue issue = w.view()->grammar()->issuesIn(0, length).first();
    QTextCursor &c = w.view()->cursor();
    c.setPosition(issue.start + 1);
    w.view()->setCursor(c);
    QMenu menu;
    w.fillContextMenu(menu);
    // The message first (it does nothing), then the suggestions, the first in bold.
    QAction *message = nullptr, *accept = nullptr;
    for (QAction *a : menu.actions()) {
        if (a->text() == issue.message) message = a;
        if (!issue.suggestions.isEmpty() && a->text() == issue.suggestions.first()) accept = a;
    }
    QVERIFY(message);
    QVERIFY(!message->isEnabled());
    QVERIFY(!issue.suggestions.isEmpty());
    QVERIFY(accept);
    QVERIFY(accept->font().bold());
    accept->trigger();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("This is a test of the checker."));
    w.view()->undo();   // (one step)
    QCOMPARE(w.document()->story()->toPlainText(), kAnTest);
    // With the setting off there is no menu for it.
    w.view()->setGrammarEnabled(false);
    QMenu off;
    w.fillContextMenu(off);
    for (QAction *a : off.actions()) QVERIFY(a->text() != issue.message);
}

void WriteTests::theSpellingPaneGoesOnToTheGrammarIssues()
{
    const SettingGuard asType(QStringLiteral("proof/checkAsYouType"), true, true), grammarOn(kGrammarOn, true, true);
    WriteWindow w;
    w.setDocument(textDoc({QStringLiteral("A recieve here. ") + kAnTest}), QString());
    w.show();
    w.act("review.spelling")->trigger();
    auto *pane = w.findChild<SpellPane *>();
    QVERIFY(pane);
    auto click = [&](const char *name) { pane->findChild<QPushButton *>(QString::fromLatin1(name))->click(); };
    QTRY_COMPARE(pane->word(), QStringLiteral("recieve"));
    QVERIFY(!pane->inGrammar());
    click("spellIgnore");
    // The spelling is done: the grammar issue is next, with its message, and its suggestion changes the text.
    QTRY_VERIFY(pane->inGrammar());
    QVERIFY(!pane->complete());
    QVERIFY(!pane->message().isEmpty());
    QVERIFY(pane->findChild<QLabel *>(QStringLiteral("spellStatus"))->text().contains(QStringLiteral("Grammar")));
    QVERIFY(!pane->findChild<QPushButton *>(QStringLiteral("spellAdd"))->isEnabled());
    click("spellChange");
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("A recieve here. This is a test of the checker."));
    QTRY_VERIFY(pane->complete());
    QVERIFY(!pane->inGrammar());
    // Grammar off: the pane has the spelling only.
    w.view()->setGrammarEnabled(false);
    setGrammarAsYouType(false);
    w.document()->story()->setPlainText(QStringLiteral("A recieve here. ") + kAnTest);
    w.act("review.spelling")->trigger();
    QTRY_COMPARE(pane->word(), QStringLiteral("recieve"));
    click("spellIgnore");
    QTRY_VERIFY(pane->complete());
    QVERIFY(!pane->inGrammar());
}

void WriteTests::typingFastSendsOnlyTheLatestTextOfAParagraphToTheChecker()
{
    auto doc = textDoc({QStringLiteral("Start.")});
    GrammarCheck check(doc->story());
    check.setDelayMs(100);
    QTextCursor c(doc->story());
    c.movePosition(QTextCursor::End);
    // 1,500 keystrokes, each followed by a repaint that asks for the paragraph's issues.
    for (int i = 0; i < 1500; ++i) {
        c.insertText(QStringLiteral("x"));
        check.issuesIn(0, doc->story()->characterCount());
    }
    QVERIFY(check.pending());
    QCOMPARE(check.paragraphsChecked(), 0);
    QTRY_VERIFY(check.paragraphsChecked() == 1);
    QTRY_VERIFY(!check.pending());
    QCOMPARE(check.paragraphsChecked(), 1);   // (one check, of the text as it was when the typing stopped)
    QCOMPARE(check.keptEntries(), 1);
    // The same text again, after an undo and a redo, is not checked again.
    c.insertText(QStringLiteral("y"));
    c.deletePreviousChar();
    check.issuesIn(0, doc->story()->characterCount());
    QTest::qWait(250);
    QCOMPARE(check.paragraphsChecked(), 1);
    // Off: nothing is found, nothing is sent.
    check.setEnabled(false);
    QVERIFY(check.issuesIn(0, doc->story()->characterCount()).isEmpty());
}

void WriteTests::aBudgetThatIsGoneStopsTheCheckingAndIsSaidOnceAndNothingIsKeptAsFound()
{
    auto doc = textDoc({QStringLiteral("This is an test of the budget."), QStringLiteral("This is another an test after it.")});
    GrammarCheck check(doc->story());
    check.setDelayMs(0);
    check.setBudget(10, 100000);   // (the first paragraph spends it)
    int said = 0;
    QObject::connect(&check, &GrammarCheck::budgetExhausted, &check, [&] { ++said; });
    const int length = doc->story()->characterCount();
    check.issuesIn(0, length);
    QTRY_VERIFY(check.paragraphsChecked() >= 1 && !check.pending());
    QCOMPARE(check.keptEntries(), 1);
    // The second paragraph meets the budget gone: no answer is kept for it (an empty one would say "no issues" for ever), and that is said, once.
    for (int i = 0; i < 5; ++i) {
        check.issuesIn(0, length);
        QTest::qWait(30);
    }
    QTRY_VERIFY(check.budgetGone());
    QCOMPARE(said, 1);
    QCOMPARE(check.keptEntries(), 1);
    // The first paragraph's issue is still shown; the second's is not found.
    const auto issues = check.issuesIn(0, length);
    QCOMPARE(issues.size(), 1);
    QVERIFY(issues.first().end <= doc->story()->findBlockByNumber(1).position());
    // A new budget (the person asks again) checks it.
    check.setBudget(1000000, 100000);
    check.issuesIn(0, length);
    QTRY_VERIFY(check.issuesIn(0, length).size() == 2);
    QCOMPARE(said, 1);
}

void WriteTests::onlyTheEnglishOfAParagraphIsChecked()
{
    auto run = [](const char *lang, const char *text) { return QByteArray("<w:r><w:rPr><w:lang w:val=\"") + lang + "\"/></w:rPr><w:t xml:space=\"preserve\">" + text + "</w:t></w:r>"; };
    // One paragraph that begins in German and goes on in English, one in English and then German, and one all German (the same mistake in each: a repeated word).
    const QByteArray body = "<w:p>" + run("de-DE", "Das ist der der Hund. ") + run("en-US", "We saw saw it.") + "</w:p>"
                            "<w:p>" + run("en-US", "We saw saw it. ") + run("de-DE", "Das ist der der Hund.") + "</w:p>"
                            "<w:p>" + run("de-DE", "Das ist der der Hund und wir sahen sahen es.") + "</w:p>";
    auto doc = open(makeDocx(body));
    QVERIFY(doc);
    GrammarCheck check(doc->story());
    check.setDelayMs(0);
    const int length = doc->story()->characterCount();
    check.issuesIn(0, length);
    QTRY_VERIFY(check.paragraphsChecked() == 2 && !check.pending());   // (the German paragraph is not sent at all)
    const auto issues = check.issuesIn(0, length);
    QCOMPARE(issues.size(), 2);
    for (const auto &is : issues) {
        QTextCursor c(doc->story());
        c.setPosition(is.start);
        c.setPosition(is.end, QTextCursor::KeepAnchor);
        QCOMPARE(c.selectedText(), QStringLiteral(" saw"));   // (the English one: never the German "der der")
    }
}

void WriteTests::ignoringAnIssueIsForThatPlaceAndIgnoreAllIsForTheWords()
{
    auto doc = textDoc({QStringLiteral("It was the the best day. It was the the worst day.")});
    GrammarCheck check(doc->story());
    check.setDelayMs(0);
    const int length = doc->story()->characterCount();
    check.issuesIn(0, length);
    QTRY_COMPARE(check.issuesIn(0, length).size(), 2);
    const auto first = check.issuesIn(0, length).first();
    check.ignoreHere(first);
    auto left = check.issuesIn(0, length);
    QCOMPARE(left.size(), 1);   // (this one only: the same words in the next sentence are still marked)
    QVERIFY(left.first().start > first.start);
    // Text added before it, in another paragraph and in the same one, does not bring it back.
    QTextCursor c(doc->story());
    c.setPosition(0);
    c.insertText(QStringLiteral("Hello there. "));
    QTRY_COMPARE(check.issuesIn(0, doc->story()->characterCount()).size(), 1);
    c.movePosition(QTextCursor::End);
    c.insertText(QStringLiteral(" And more."));
    QTRY_COMPARE(check.issuesIn(0, doc->story()->characterCount()).size(), 1);
    // Ignore All: the words with that message, wherever they are.
    check.ignoreAll(check.issuesIn(0, doc->story()->characterCount()).first());
    QCOMPARE(check.issuesIn(0, doc->story()->characterCount()).size(), 0);
}

void WriteTests::repaintingAsksNothingOfAParagraphThatHasNotChanged()
{
    auto doc = textDoc({QStringLiteral("One paragraph."), QStringLiteral("Another paragraph.")});
    GrammarCheck check(doc->story());
    check.setDelayMs(0);
    const int length = doc->story()->characterCount();
    check.issuesIn(0, length);
    const int looks = check.looksMade();
    QVERIFY(looks >= 2);
    for (int i = 0; i < 200; ++i) {   // (a caret that blinks repaints the page)
        check.issuesIn(0, length);
        check.issueAt(3);
    }
    QCOMPARE(check.looksMade(), looks);
    // A paragraph that changes is looked at again, and only that one (the last: the others do not move).
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1).position() + 2);
    c.insertText(QStringLiteral("x"));
    check.issuesIn(0, doc->story()->characterCount());
    QCOMPARE(check.looksMade(), looks + 1);
}
