// JeffDeck's tests of the Review tab's Language group, which JeffWrite has too: Translate (the selected text of a box or of the notes, opened in the translation service only after
// OK), Set Proofing Language (for the words selected, or for a whole box), and Do Not Check Spelling, each kept in the file (.pptx and .odp) and followed by the spelling.
#include "core/datafolder.h"
#include "deckhelpers.h"

#include "../settingguard.h"
#include "../withdialog.h"

#include <QComboBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidget>
#include <QTemporaryDir>
#include <QUrlQuery>

#include "app/grammar.h"
#include "app/spelling.h"
#include "app/proofingdialogs.h"

#include <QLabel>
#include <QMenu>
#include <QPushButton>

namespace {

const QString kGrammarOn = QStringLiteral("proof/checkGrammarAsYouType");
// A sentence with one grammar issue and no misspelling (as in JeffWrite's tests).
const QString kAnTest = QStringLiteral("This is an test of the checker.");

QJsonObject groupNamed(const QString &tab, const QString &group)
{
    QFile json(QStringLiteral(":/deck/ribbon.json"));
    if (!json.open(QIODevice::ReadOnly)) return {};
    for (const QJsonValue &t : QJsonDocument::fromJson(json.readAll()).object().value(QStringLiteral("tabs")).toArray())
        if (t.toObject().value(QStringLiteral("name")).toString() == tab)
            for (const QJsonValue &g : t.toObject().value(QStringLiteral("groups")).toArray())
                if (g.toObject().value(QStringLiteral("name")).toString() == group) return g.toObject();
    return {};
}

// The language and "no proofing" mark of the run that has `word` in the story of the first slide's title.
struct Marks {
    QString language;
    bool noProof = false;
    bool found = false;
};
Marks marksOf(const jp::Document &d, const QString &word)
{
    Marks m;
    auto *title = static_cast<jp::TextItem *>(d.pages[0]->items[0].get());
    QTextDocument *doc = d.storyDoc(title->storyId);
    if (!doc) return m;
    const int at = doc->toPlainText().indexOf(word);
    if (at < 0) return m;
    QTextCursor c(doc);
    c.setPosition(at + 1);
    m.found = true;
    m.language = c.charFormat().stringProperty(tp::Language);
    m.noProof = c.charFormat().boolProperty(tp::NoProof);
    return m;
}

// Saves a deck as `name` in `dir` and opens the file again.
std::unique_ptr<jp::Document> savedAndOpened(const jp::Document &d, const QTemporaryDir &dir, const QString &name)
{
    const QString path = dir.filePath(name);
    QString err;
    if (!jp::deck::saveAnyPresentationFile(d, path, &err)) return nullptr;
    return jp::deck::importAnyPresentationFile(path, &err);
}

} // namespace

class DeckLanguageTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }
    void init()
    {
        jp::setSpellAsYouType(true);
        jp::setGrammarAsYouType(true);
    }

    // The three commands sit on the Review tab in a Language group, as JeffWrite has them, with the same words, a key tip each, and the Translate and Set Proofing Language buttons large.
    void theReviewTabHasTheLanguageGroupAsJeffWriteHasIt()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QAction *translate = w.act(QStringLiteral("review.translate")), *language = w.act(QStringLiteral("review.language")), *noProof = w.act(QStringLiteral("review.noProof"));
        QVERIFY(translate && language && noProof);
        QCOMPARE(translate->text(), QStringLiteral("Translate"));
        QCOMPARE(language->text(), QStringLiteral("Set Proofing Language…"));
        QCOMPARE(noProof->text(), QStringLiteral("Do Not Check Spelling"));
        QVERIFY(noProof->isCheckable());
        const QJsonObject group = groupNamed(QStringLiteral("Review"), QStringLiteral("Language"));
        QVERIFY(!group.isEmpty());
        const QJsonArray items = group.value(QStringLiteral("items")).toArray();
        QCOMPARE(items.size(), 3);
        QCOMPARE(items[0].toObject().value(QStringLiteral("large")).toString(), QStringLiteral("review.translate"));
        QCOMPARE(items[0].toObject().value(QStringLiteral("keytip")).toString(), QStringLiteral("TR"));
        QCOMPARE(items[1].toObject().value(QStringLiteral("large")).toString(), QStringLiteral("review.language"));
        QCOMPARE(items[1].toObject().value(QStringLiteral("keytip")).toString(), QStringLiteral("LA"));
        QCOMPARE(items[2].toObject().value(QStringLiteral("small")).toString(), QStringLiteral("review.noProof"));
        QCOMPARE(items[2].toObject().value(QStringLiteral("keytip")).toString(), QStringLiteral("NP"));
    }

    // The Help page of the tab names the commands.
    void helpNamesTheCommands()
    {
        QFile page(jp::dataFolder(QStringLiteral("help")) + QStringLiteral("/en/deck/tab-review.md"));
        QVERIFY(page.open(QIODevice::ReadOnly));
        const QString text = QString::fromUtf8(page.readAll());
        for (const char *word : {"**Translate**", "**Set Proofing Language**", "**Do Not Check Spelling**", "**Language**"}) QVERIFY2(text.contains(QLatin1String(word)), word);
    }

    // Translate sends the selected words, and only after OK: the box says where, and a Cancel opens nothing.
    void translateOpensTheSelectedWordsOnlyAfterOk()
    {
        const jptest::SettingGuard target(QStringLiteral("translate/targetCode"), QStringLiteral("es"), QStringLiteral("es"));
        const jptest::SettingGuard server(QStringLiteral("translate/server"), QStringLiteral("https://translate.example/"), QStringLiteral("https://libretranslate.com/"));
        DeckWindow w;
        QList<QUrl> opened;
        w.setUrlOpener([&](const QUrl &u) {
            opened << u;
            return true;
        });
        QVERIFY(w.act(QStringLiteral("review.translate")));
        editTitle(w, QStringLiteral("Hello brave world"), 6, 11);
        withDialog([&] { w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [&](QDialog *d) {
            QVERIFY(opened.isEmpty());   // (nothing is sent while the box is up)
            QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("translateServer"))->text(), QStringLiteral("https://translate.example/"));
            auto *to = d->findChild<QComboBox *>(QStringLiteral("translateTarget"));
            to->setCurrentIndex(to->findData(QStringLiteral("fr")));
        }, false);
        QVERIFY(opened.isEmpty());
        withDialog([&] { w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [&](QDialog *d) {
            auto *to = d->findChild<QComboBox *>(QStringLiteral("translateTarget"));
            to->setCurrentIndex(to->findData(QStringLiteral("fr")));
        });
        QCOMPARE(opened.size(), 1);
        QCOMPARE(opened[0].host(), QStringLiteral("translate.example"));
        const QUrlQuery q(opened[0]);
        QCOMPARE(q.queryItemValue(QStringLiteral("q")), QStringLiteral("brave"));
        QCOMPARE(q.queryItemValue(QStringLiteral("target")), QStringLiteral("fr"));
        QCOMPARE(jp::Settings::get().value(QStringLiteral("translate/targetCode")).toString(), QStringLiteral("fr"));
    }

    // With a box selected and no words, all of its text; with a selection in the notes, those words; with no text, a message and no box.
    void translateTakesTheBoxOrTheNotesOrTellsThereIsNoText()
    {
        const jptest::SettingGuard server(QStringLiteral("translate/server"), QStringLiteral("https://translate.example/"), QStringLiteral("https://libretranslate.com/"));
        DeckWindow w;
        w.show();
        QList<QUrl> opened;
        w.setUrlOpener([&](const QUrl &u) {
            opened << u;
            return true;
        });
        QVERIFY(w.act(QStringLiteral("review.translate")));
        Editor *ed = w.editor();
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        ed->select(QString());
        w.act(QStringLiteral("review.translate"))->trigger();   // nothing selected, nothing typed: no box
        QVERIFY(opened.isEmpty());
        QVERIFY(w.statusBar()->currentMessage().contains(QLatin1String("no text")));
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("Whole box"));
        ed->select(title->id);
        withDialog([&] { w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [](QDialog *) {});
        QCOMPARE(opened.size(), 1);
        QCOMPARE(QUrlQuery(opened[0]).queryItemValue(QStringLiteral("q")), QStringLiteral("Whole box"));
        // The notes: the words selected there.
        ed->select(QString());
        w.notesPane()->setPlainText(QStringLiteral("Say this slowly"));
        QTextCursor c = w.notesPane()->textCursor();
        c.setPosition(4);
        c.setPosition(8, QTextCursor::KeepAnchor);
        w.notesPane()->setTextCursor(c);
        if (w.notesPane()->isHidden()) w.act(QStringLiteral("view.notes"))->trigger();
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QVERIFY2(w.notesPane()->isVisible() && w.notesPane()->isEnabled(), "the notes are shown");
        w.notesPane()->setFocus();
        QTRY_VERIFY(w.notesPane()->hasFocus());
        withDialog([&] { w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [](QDialog *) {});
        QCOMPARE(opened.size(), 2);
        QCOMPARE(QUrlQuery(opened[1]).queryItemValue(QStringLiteral("q")), QStringLiteral("this"));
    }

    // The language of the words selected is set, only theirs, one Undo takes it back, and the spelling follows it.
    void setProofingLanguageMarksTheSelectedWordsAndSpellingFollowsIt()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        QTextDocument *doc = editTitle(w, QStringLiteral("Hello Bonjour monde"), 6, 13);   // "Bonjour"
        const QString storyId = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId;
        QVERIFY(w.act(QStringLiteral("review.language")));
        QTRY_VERIFY(w.act(QStringLiteral("review.language"))->isEnabled());
        const QString whole = doc->toPlainText();
        auto flagged = [&](const QString &word) {
            for (const auto &r : ed->misspelledIn(storyId))
                if (whole.mid(r.first, r.second - r.first) == word) return true;
            return false;
        };
        QVERIFY(flagged(QStringLiteral("Bonjour")));
        QVERIFY(flagged(QStringLiteral("monde")));
        withDialog([&] { w.act(QStringLiteral("review.language"))->trigger(); }, QStringLiteral("languageDialog"), [&](QDialog *d) {
            auto *list = d->findChild<QListWidget *>(QStringLiteral("languages"));
            for (int i = 0; i < list->count(); ++i)
                if (list->item(i)->data(Qt::UserRole).toString() == QLatin1String("fr-FR")) list->setCurrentRow(i);
            QCOMPARE(list->currentItem()->data(Qt::UserRole).toString(), QStringLiteral("fr-FR"));
        });
        QVERIFY(!flagged(QStringLiteral("Bonjour")));
        QVERIFY(flagged(QStringLiteral("monde")));   // (only the selected words changed)
        QCOMPARE(marksOf(*ed->doc(), QStringLiteral("Bonjour")).language, QStringLiteral("fr-FR"));
        QCOMPARE(marksOf(*ed->doc(), QStringLiteral("monde")).language, QString());
        ed->undoStack()->undo();
        QCOMPARE(marksOf(*ed->doc(), QStringLiteral("Bonjour")).language, QString());
    }

    // Do Not Check Spelling marks a selected box's words, shows as ticked, and takes the red lines away; again, and they come back.
    void doNotCheckSpellingMarksABoxAndShowsTicked()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("A mispelled word"));
        ed->select(title->id);
        QAction *noProof = w.act(QStringLiteral("review.noProof"));
        QVERIFY(noProof);
        QTRY_VERIFY(noProof->isEnabled());
        QVERIFY(!ed->misspelledIn(title->storyId).isEmpty());
        QVERIFY(!noProof->isChecked());
        noProof->trigger();
        QVERIFY(ed->misspelledIn(title->storyId).isEmpty());
        QVERIFY(marksOf(*ed->doc(), QStringLiteral("mispelled")).noProof);
        QVERIFY(noProof->isChecked());
        noProof->trigger();
        QVERIFY(!marksOf(*ed->doc(), QStringLiteral("mispelled")).noProof);
        QVERIFY(!ed->misspelledIn(title->storyId).isEmpty());
    }

    // The language of a run and "do not check spelling" are kept in a .pptx (a:rPr lang and noProof) and in a .odp, and read again.
    void theLanguageAndTheMarkAreKeptInPptxAndOdp()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        editTitle(w, QStringLiteral("Hello Bonjour Hallo mispelled"), 6, 13);
        QTRY_VERIFY(w.act(QStringLiteral("review.language"))->isEnabled());
        withDialog([&] { w.act(QStringLiteral("review.language"))->trigger(); }, QStringLiteral("languageDialog"), [&](QDialog *d) {
            auto *list = d->findChild<QListWidget *>(QStringLiteral("languages"));
            for (int i = 0; i < list->count(); ++i)
                if (list->item(i)->data(Qt::UserRole).toString() == QLatin1String("fr-FR")) list->setCurrentRow(i);
        });
        QTextDocument *doc = ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId);
        QTextCursor c = ed->cursor();
        c.setPosition(doc->toPlainText().indexOf(QLatin1String("mispelled")));
        c.setPosition(c.position() + 9, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        w.act(QStringLiteral("review.noProof"))->trigger();
        QVERIFY(marksOf(*ed->doc(), QStringLiteral("mispelled")).noProof);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const char *name : {"lang.pptx", "lang.odp"}) {
            const auto back = savedAndOpened(*ed->doc(), dir, QString::fromLatin1(name));
            QVERIFY2(back, name);
            const Marks bonjour = marksOf(*back, QStringLiteral("Bonjour")), hello = marksOf(*back, QStringLiteral("Hello")), wrong = marksOf(*back, QStringLiteral("mispelled"));
            QVERIFY2(bonjour.found && hello.found && wrong.found, name);
            QCOMPARE(bonjour.language, QStringLiteral("fr-FR"));
            QVERIFY2(!bonjour.noProof, name);
            QVERIFY2(wrong.noProof, name);
            QVERIFY2(!hello.noProof, name);
            QVERIFY2(hello.language != QLatin1String("fr-FR"), name);
        }
    }
    // The box offers the language the text has (a bare tag such as "fr" chooses the nearest language of the list), and US English where the text states none: as JeffWrite's does.
    void theLanguageBoxPreselectsAsJeffWriteDoes()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        editTitle(w, QStringLiteral("Hello Bonjour"), 0, 5);
        QTRY_VERIFY(w.act(QStringLiteral("review.language"))->isEnabled());
        auto chosen = [&] {
            QString code;
            withDialog([&] { w.act(QStringLiteral("review.language"))->trigger(); }, QStringLiteral("languageDialog"), [&](QDialog *d) {
                code = d->findChild<QListWidget *>(QStringLiteral("languages"))->currentItem()->data(Qt::UserRole).toString();
            }, false);
            return code;
        };
        QCOMPARE(chosen(), QStringLiteral("en-US"));
        for (const QString &tag : {QStringLiteral("fr-FR"), QStringLiteral("fr"), QStringLiteral("FR-fr"), QStringLiteral("fr_FR")}) {
            QTextCharFormat f;
            f.setProperty(tp::Language, tag);
            ed->mergeCharFormat(f, QStringLiteral("test"));
            QCOMPARE(chosen(), QStringLiteral("fr-FR"));
        }
    }

    // Text marked with a bare or oddly written tag is checked in that language, here and after a save in each file kind, which keep the tag as it was written.
    void bareLanguageTagsAreCheckedInTheLanguageAndKept()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const QString &tag : {QStringLiteral("fr"), QStringLiteral("FR-fr"), QStringLiteral("fr_FR")}) {
            DeckWindow w;
            w.show();
            Editor *ed = w.editor();
            QTextDocument *doc = editTitle(w, QStringLiteral("Bonjour monde"), 0, 13);
            QTextCharFormat f;
            f.setProperty(tp::Language, tag);
            ed->mergeCharFormat(f, QStringLiteral("test"));
            QVERIFY2(misspellings(doc).isEmpty(), qPrintable(tag));
            for (const char *name : {"tag.pptx", "tag.odp"}) {
                const auto back = savedAndOpened(*ed->doc(), dir, QString::fromLatin1(name));
                QVERIFY2(back, name);
                auto *title = static_cast<TextItem *>(back->pages[0]->items[0].get());
                QTextDocument *again = back->storyDoc(title->storyId);
                QVERIFY2(again && !again->toPlainText().isEmpty(), name);
                QVERIFY2(misspellings(again).isEmpty(), name);
                QVERIFY2(marksOf(*back, QStringLiteral("Bonjour")).language.startsWith(QLatin1String("fr"), Qt::CaseInsensitive), name);
            }
        }
    }

    // Translate follows the editor that has the focus: the notes (their selected words) or the slide's box; not whichever has a selection or none selected.
    void translateFollowsTheFocusedEditor()
    {
        const jptest::SettingGuard server(QStringLiteral("translate/server"), QStringLiteral("https://translate.example/"), QStringLiteral("https://libretranslate.com/"));
        DeckWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QList<QUrl> opened;
        w.setUrlOpener([&](const QUrl &u) {
            opened << u;
            return true;
        });
        Editor *ed = w.editor();
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("Box words"));
        w.notesPane()->setPlainText(QStringLiteral("Say this slowly"));
        QTextCursor c = w.notesPane()->textCursor();
        c.setPosition(4);
        c.setPosition(8, QTextCursor::KeepAnchor);
        w.notesPane()->setTextCursor(c);
        if (w.notesPane()->isHidden()) w.act(QStringLiteral("view.notes"))->trigger();
        ed->select(title->id);   // (a box is selected, and the notes still have their selection)
        w.canvas()->setFocus();
        QTRY_VERIFY(!w.notesPane()->hasFocus());
        auto sent = [&](int index) {
            withDialog([&] { w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [](QDialog *) {});
            return QUrlQuery(opened.value(index)).queryItemValue(QStringLiteral("q"), QUrl::FullyDecoded);
        };
        QCOMPARE(sent(0), QStringLiteral("Box words"));
        w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QVERIFY2(w.notesPane()->isVisible() && w.notesPane()->isEnabled(), "the notes are shown");
        w.notesPane()->setFocus();
        QTRY_VERIFY(w.notesPane()->hasFocus());
        QCOMPARE(sent(1), QStringLiteral("this"));
        // nothing is selected on the slide and the notes keep a selection, but the slide has the focus: there is no text to send (not the notes')
        ed->select(QString());
        w.canvas()->setFocus();
        QTRY_VERIFY(!w.notesPane()->hasFocus());
        QTimer::singleShot(300, [] {   // (a box that opens is answered OK, so that a command that sends something is seen to)
            if (auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget())) d->accept();
        });
        w.act(QStringLiteral("review.translate"))->trigger();
        QCOMPARE(opened.size(), 2);
        QVERIFY(w.statusBar()->currentMessage().contains(QLatin1String("no text")));
    }

    // Grammar as you type, as JeffWrite has it (the same rules and the same setting): a slide's text gets its issues, the setting takes them away, and the renderer is told
    // where to draw the blue dotted line.
    void grammarIssuesAreFlaggedInSlideText()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        ed->setGrammarDelayMs(0);
        editTitle(w, kAnTest, 0, 0);
        const QString storyId = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId;
        QTRY_VERIFY(!ed->grammarIssuesIn(storyId).isEmpty());
        const auto issue = ed->grammarIssuesIn(storyId).first();
        QTRY_VERIFY(ed->renderOptions().grammar.contains(storyId));
        QCOMPARE(ed->renderOptions().grammar.value(storyId).first(), qMakePair(issue.start, issue.end));
        jp::setGrammarAsYouType(false);
        QVERIFY(ed->grammarIssuesIn(storyId).isEmpty());
        QVERIFY(!ed->renderOptions().grammar.contains(storyId));
        jp::setGrammarAsYouType(true);
        QTRY_VERIFY(!ed->grammarIssuesIn(storyId).isEmpty());
    }

    // Words in a language without rules are not checked, and a box marked "do not check" is not either.
    void grammarIsNotCheckedInALanguageWithoutRulesOrMarkedNoProof()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        ed->setGrammarDelayMs(0);
        QTextDocument *doc = editTitle(w, kAnTest, 0, 0);
        const QString storyId = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId;
        QTRY_VERIFY(!ed->grammarIssuesIn(storyId).isEmpty());
        QTextCursor c(doc);
        c.select(QTextCursor::Document);
        QTextCharFormat french;
        french.setProperty(tp::Language, QStringLiteral("fr-FR"));
        c.mergeCharFormat(french);
        QVERIFY(ed->grammarIssuesIn(storyId).isEmpty());
        QVERIFY(!ed->renderOptions().grammar.contains(storyId));
        QTextCharFormat english, noProof;
        english.setProperty(tp::Language, QStringLiteral("en-US"));
        c.mergeCharFormat(english);
        QTRY_VERIFY(!ed->grammarIssuesIn(storyId).isEmpty());
        noProof.setProperty(tp::NoProof, true);
        c.mergeCharFormat(noProof);
        QVERIFY(ed->grammarIssuesIn(storyId).isEmpty());
    }

    // The right-click menu on an issue in a slide: the message, the suggestion, and Ignore This Issue, which takes the mark away.
    void aGrammarIssueInASlideOffersItsSuggestionAndIgnore()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        ed->setGrammarDelayMs(0);
        QTextDocument *doc = editTitle(w, kAnTest, 0, 0);
        const QString storyId = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId;
        QTRY_VERIFY(!ed->grammarIssuesIn(storyId).isEmpty());
        const auto issue = ed->grammarIssuesIn(storyId).first();
        auto inIssue = [&] {
            QTextCursor c = ed->cursor();
            c.setPosition(issue.start + 1);
            ed->setCursor(c);
        };
        inIssue();
        {
            QMenu menu;
            QVERIFY(jp::addGrammarSuggestions(menu, ed));
            QAction *accept = nullptr;
            for (QAction *a : menu.actions())
                if (!issue.suggestions.isEmpty() && a->text() == issue.suggestions.first()) accept = a;
            QVERIFY(accept);
            accept->trigger();
        }
        QCOMPARE(doc->toPlainText(), QStringLiteral("This is a test of the checker."));
        doc->setPlainText(kAnTest);
        QTRY_VERIFY(!ed->grammarIssuesIn(storyId).isEmpty());
        inIssue();
        QMenu menu;
        QVERIFY(jp::addGrammarSuggestions(menu, ed));
        for (QAction *a : menu.actions())
            if (a->text() == QStringLiteral("Ignore This Issue")) a->trigger();
        QVERIFY(ed->grammarIssuesIn(storyId).isEmpty());
    }

    // The spelling check (F7) stops at a grammar issue in a slide, with the issue's message.
    void theSpellingCheckStopsAtAGrammarIssueInASlide()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        DeckWindow w;
        w.show();
        editTitle(w, kAnTest, 0, 0);
        const QString message = jp::grammarCheck(kAnTest).first().message;
        QString shown;
        withDialog([&] { w.act(QStringLiteral("review.spelling"))->trigger(); }, QStringLiteral("pageSpellingDialog"), [&](QDialog *d) {
            shown = d->findChild<QLabel *>(QStringLiteral("spellingNotIn"))->text();
        }, false);
        QVERIFY2(shown.contains(message.toHtmlEscaped()), qPrintable(shown));
    }

    // Speaker notes get the same: the issue is found in what is typed, and the menu offers its suggestion.
    void grammarIssuesAreFlaggedInSpeakerNotes()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        DeckWindow w;
        w.show();
        w.notesPane()->setGrammarDelayMs(0);
        w.notesPane()->setPlainText(kAnTest);
        QTRY_VERIFY(!w.notesPane()->grammarIssues().isEmpty());
        const auto issue = w.notesPane()->grammarIssues().first();
        QMenu menu;
        QVERIFY(w.notesPane()->addGrammarSuggestions(menu, issue.start + 1));
        for (QAction *a : menu.actions())
            if (!issue.suggestions.isEmpty() && a->text() == issue.suggestions.first()) a->trigger();
        QCOMPARE(w.notesPane()->toPlainText(), QStringLiteral("This is a test of the checker."));
        jp::setGrammarAsYouType(false);
        QTRY_VERIFY(w.notesPane()->grammarIssues().isEmpty());
    }
};

JP_TEST_CLASS(DeckLanguageTests)
#include "tst_deck_language.moc"
