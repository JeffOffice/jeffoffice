// Tests of the small pieces that finish JeffWrite's first release: the status bar's place and proofing count, the navigation pane's section moves, the paper source, and the style area.
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/grammarcheck.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/spelling.h"
#include "app/theme.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/spellcheck.h"
#include "core/writedoc.h"
#include "office/package.h"

#include <QLabel>
#include <QSignalSpy>
#include <QStatusBar>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QToolButton>
#include <QTemporaryDir>
#include <memory>
#include <vector>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

QByteArray makeDocx(const QByteArray &body)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

std::unique_ptr<WriteDoc> textDoc(const QStringList &paragraphs)
{
    QByteArray body;
    for (const QString &p : paragraphs) body += "<w:p><w:r><w:t xml:space=\"preserve\">" + p.toUtf8() + "</w:t></w:r></w:p>";
    return open(makeDocx(body));
}

QStringList misspelledParagraphs(int n)
{
    QStringList texts;
    for (int i = 0; i < n; ++i) texts << QStringLiteral("Paragraph number %1 has a misspeled word in it.").arg(i);
    return texts;
}

// ---- the probe: a seeded run of edits that compares the kept total with a fresh count after each one
struct Dice {
    quint64 s;
    explicit Dice(quint64 seed) : s(seed) {}
    int below(int n)
    {
        s ^= s << 13;
        s ^= s >> 7;
        s ^= s << 17;
        return int((s >> 11) % quint64(n));
    }
};

// What a fresh check of a copy of the story finds: the misspelled words, all of them, nothing kept from before.
int freshCount(QTextDocument *story, const QString &language)
{
    QTextDocument copy;
    copy.setPlainText(story->toPlainText());
    SpellCheck fresh(&copy);
    fresh.setDefaultLanguage(language);
    return int(fresh.misspelledIn(0, copy.characterCount()).size());
}

// Lets the idle checking run until it says it has settled (counted work: a number of turns of the event loop, no waiting).
bool settle(const std::vector<SpellCheck *> &checks)
{
    for (int turns = 0; turns < 20000; ++turns) {
        bool all = true;
        for (SpellCheck *c : checks) all = all && c->counted() && !c->pending();
        if (all) return true;
        QCoreApplication::processEvents();
    }
    return false;
}

const char *const kProbeWords[] = {"the", "cat", "teh", "house", "misspeled", "wrold", "color", "colour", "flurbix", "zorgle", "Paris", "NASA", "r2d2", "went", "recieve", "and"};

QString probeText(Dice &d, int words)
{
    QString t;
    for (int i = 0; i < words; ++i) t += QString::fromLatin1(kProbeWords[d.below(int(sizeof(kProbeWords) / sizeof(kProbeWords[0])))]) + QLatin1Char(' ');
    return t;
}

} // namespace

class TailTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }

    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    // ---------------------------------------------------------------- status bar: the proofing count
    void theSpellingTotalIsKeptAsParagraphsAreCheckedAndNeverScansTheText()
    {
        auto doc = textDoc(misspelledParagraphs(100));
        QTextDocument *story = doc->story();
        SpellCheck sc(story);
        sc.setCounting(true);
        QCOMPARE(sc.knownMisspellings(), 0);   // (nothing is checked until idle time)
        QTRY_VERIFY(sc.counted());
        QCOMPARE(sc.knownMisspellings(), 100);
        QCOMPARE(sc.paragraphsChecked(), qint64(100));
        // Asking for the total costs no checking at all.
        const qint64 words = sc.wordsChecked();
        for (int i = 0; i < 1000; ++i) QCOMPARE(sc.knownMisspellings(), 100);
        QCOMPARE(sc.paragraphsChecked(), qint64(100));
        QCOMPARE(sc.wordsChecked(), words);
        // An edit makes only its own paragraph checked again, and the total follows: exactly one more paragraph, and only its words.
        const QTextBlock third = story->findBlockByNumber(3);
        QTextCursor c(story);
        c.setPosition(third.position());
        insertText(*doc, c, QStringLiteral("teh "));
        QTRY_COMPARE(sc.knownMisspellings(), 101);
        QCOMPARE(sc.paragraphsChecked(), qint64(101));
        const qint64 spent = sc.wordsChecked() - words;
        QVERIFY2(spent > 0 && spent <= words / 100 + 2, qPrintable(QString::number(spent)));   // (one paragraph's words, not the document's)
        // A paragraph taken out takes its errors out of the total, and checks at most the one that joins.
        const QTextBlock fifth = story->findBlockByNumber(5);
        QTextCursor cut(story);
        cut.setPosition(fifth.position());
        cut.setPosition(fifth.next().position(), QTextCursor::KeepAnchor);
        cut.removeSelectedText();
        QTRY_COMPARE(sc.knownMisspellings(), 100);
        QVERIFY(sc.paragraphsChecked() <= 102);
        // Undo brings it back.
        story->undo();
        QTRY_COMPARE(sc.knownMisspellings(), 101);
        QVERIFY(sc.paragraphsChecked() <= 104);
    }

    // Whatever is done to the text or the words accepted, once the checking has settled the total kept is what a fresh count of the same text gives.
    void probeTheKeptTotalAgainstAFreshCountAfterEveryStep()
    {
        QTemporaryDir dictionary;
        QVERIFY(dictionary.isValid());
        struct Restore {
            ~Restore()
            {
                Speller::setUserDictionaryPath(QString());
                Settings::get().setValue(QStringLiteral("proof/ignoreUpper"), true);
            }
        } restore;
        Speller::setUserDictionaryPath(dictionary.path() + QStringLiteral("/custom.dic"));
        for (const quint64 seed : {quint64(88172645463325252ull), quint64(1234567891011ull)}) {
            Dice d(seed);
            QStringList paragraphs;
            for (int i = 0; i < 40; ++i) paragraphs << probeText(d, 3 + d.below(9));
            auto doc = textDoc(paragraphs);
            QTextDocument *story = doc->story();
            SpellCheck sc(story);
            sc.setCounting(true);
            QString language = QStringLiteral("en-US");
            sc.setDefaultLanguage(language);
            std::unique_ptr<SpellCheck> split;
            bool upperIgnored = true;
            int ignores = 0, adds = 0;
            QVERIFY(settle({&sc}));
            const auto where = [&] { return d.below(std::max(1, story->characterCount() - 1)); };
            const int steps = 450;
            for (int step = 0; step < steps; ++step) {
                const int kind = step == 0 ? 1 : d.below(15);   // (the first step splits a paragraph: the second half is the one that was missed)
                QTextCursor c(story);
                switch (kind) {
                case 0:
                case 1:
                    c.setPosition(where());
                    c.insertBlock();
                    break;
                case 2:
                case 3:
                    c.setPosition(where());
                    c.insertText(probeText(d, 1 + d.below(4)));
                    break;
                case 4: {   // a range deleted, across paragraphs sometimes
                    const int from = where();
                    c.setPosition(from);
                    c.setPosition(std::min(story->characterCount() - 1, from + d.below(40)), QTextCursor::KeepAnchor);
                    c.removeSelectedText();
                    break;
                }
                case 5: {   // two paragraphs made one
                    const QTextBlock b = story->findBlock(where());
                    if (b.next().isValid()) {
                        c.setPosition(b.position() + b.length() - 1);
                        c.deleteChar();
                    }
                    break;
                }
                case 6: {   // many paragraphs pasted, the last of them sometimes ended
                    QString text;
                    const int n = 5 + d.below(30);
                    for (int i = 0; i < n; ++i) text += probeText(d, 1 + d.below(6)) + QLatin1Char('\n');
                    if (d.below(2)) text.chop(1);
                    if (story->blockCount() < 250) {
                        c.setPosition(where());
                        c.insertText(text);
                    }
                    break;
                }
                case 7: {   // cut, and pasted somewhere else, before the checking has had a turn
                    const int from = where();
                    c.setPosition(from);
                    c.setPosition(std::min(story->characterCount() - 1, from + d.below(150)), QTextCursor::KeepAnchor);
                    QString text = c.selectedText();
                    text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
                    c.removeSelectedText();
                    c.setPosition(where());
                    c.insertText(text);
                    break;
                }
                case 8:
                    for (int i = 1 + d.below(3); i > 0 && story->isUndoAvailable(); --i) story->undo();
                    break;
                case 9:
                    for (int i = 1 + d.below(3); i > 0 && story->isRedoAvailable(); --i) story->redo();
                    break;
                case 10: {   // Replace All
                    const QString from = d.below(2) ? QStringLiteral("misspeled") : QStringLiteral("and");
                    const QString to = d.below(2) ? QStringLiteral("wrold") : QStringLiteral("the");
                    QTextCursor found(story);
                    for (int i = 0; i < 60; ++i) {
                        found = story->find(from, found);
                        if (found.isNull()) break;
                        const int start = found.selectionStart();
                        replaceWord(story, start, found.selectionEnd(), to);
                        found.setPosition(start + int(to.size()));
                    }
                    break;
                }
                case 11:   // Ignore All
                    if (ignores < 2) {
                        Speller::get().ignore(ignores++ == 0 ? QStringLiteral("flurbix") : QStringLiteral("zorgle"));
                        sc.refresh();
                        if (split) split->refresh();
                    }
                    break;
                case 12:   // Add to Dictionary
                    if (adds < 2) {
                        QString failure;
                        QVERIFY2(spellingAdd(adds++ == 0 ? QStringLiteral("wrold") : QStringLiteral("recieve"), &failure), qPrintable(failure));
                        sc.refresh();
                        if (split) split->refresh();
                    }
                    break;
                case 13:   // the language, or a rule of Options
                    if (d.below(2)) {
                        language = language == QLatin1String("en-US") ? QStringLiteral("en-GB") : QStringLiteral("en-US");
                        sc.setDefaultLanguage(language);
                        if (split) split->setDefaultLanguage(language);
                    } else {
                        upperIgnored = !upperIgnored;
                        Settings::get().setValue(QStringLiteral("proof/ignoreUpper"), upperIgnored);
                        sc.refresh();
                        if (split) split->refresh();
                    }
                    break;
                case 14:   // the split view opened (it draws the pages, so checks them) or closed
                    if (split) {
                        split.reset();
                    } else {
                        split = std::make_unique<SpellCheck>(story);
                        split->setDefaultLanguage(language);
                        split->setCounting(d.below(2));
                        split->misspelledIn(0, story->characterCount());
                    }
                    break;
                }
                if (split && d.below(3) == 0) split->misspelledIn(0, std::min(story->characterCount(), 400));   // (the split view draws what it shows)
                std::vector<SpellCheck *> checks{&sc};
                if (split && split->counting()) checks.push_back(split.get());
                const QString what = QStringLiteral("seed %1, step %2, kind %3").arg(seed).arg(step).arg(kind);
                QVERIFY2(settle(checks), qPrintable(what + QStringLiteral(": the checking did not settle")));
                const int real = freshCount(story, language);
                QVERIFY2(sc.knownMisspellings() == real, qPrintable(what + QStringLiteral(": shown %1, real %2").arg(sc.knownMisspellings()).arg(real)));
                if (split && split->counting())
                    QVERIFY2(split->knownMisspellings() == real, qPrintable(what + QStringLiteral(": the split view's check shows %1, real %2").arg(split->knownMisspellings()).arg(real)));
            }
        }
    }

    // An edit while the counting pass is part way through: where it goes on from moves with the text, so no paragraph after the edit is skipped.
    void anEditBeforeThePlaceTheCountingPassHasReachedSkipsNothing()
    {
        auto doc = textDoc(misspelledParagraphs(700));
        QTextDocument *story = doc->story();
        SpellCheck sc(story);
        sc.setCounting(true);
        for (int turns = 0; turns < 5000 && sc.paragraphsWalked() < 130; ++turns) QCoreApplication::processEvents();
        QVERIFY(sc.paragraphsWalked() >= 130);   // (the pass is past the paragraphs the edit takes out)
        QVERIFY(!sc.counted());   // (a slice goes by 256 paragraphs at the most, so 700 are not done in one or two)
        QVERIFY(sc.paragraphsWalked() <= 130 + 256);
        QTextCursor c(story);
        c.setPosition(story->findBlockByNumber(5).position());
        c.setPosition(story->findBlockByNumber(120).position(), QTextCursor::KeepAnchor);
        c.removeSelectedText();
        QVERIFY(settle({&sc}));
        QCOMPARE(sc.knownMisspellings(), freshCount(story, QString()));
        QCOMPARE(sc.knownMisspellings(), 700 - 115);
    }

    // The counting pass goes by a bounded number of paragraphs in a slice, even over ones that are checked already, and does not begin again at the top each slice.
    void theCountingPassGoesByAtMostSomeParagraphsInASliceAndNeverRestartsFromTheTop()
    {
        auto doc = textDoc(misspelledParagraphs(2000));
        SpellCheck sc(doc->story());
        sc.setCounting(true);
        QVERIFY(settle({&sc}));
        QCOMPARE(sc.knownMisspellings(), 2000);
        const qint64 walked = sc.paragraphsWalked(), checked = sc.paragraphsChecked();
        sc.refresh();   // (the whole story is gone through again; what is kept holds)
        for (int turns = 0; turns < 1000 && sc.paragraphsWalked() == walked; ++turns) QCoreApplication::processEvents();
        QVERIFY(sc.paragraphsWalked() > walked);
        QVERIFY2(sc.paragraphsWalked() - walked <= 512, qPrintable(QString::number(sc.paragraphsWalked() - walked)));   // (a slice or two, 256 at most each)
        QVERIFY(settle({&sc}));
        QVERIFY2(sc.paragraphsWalked() - walked <= 2000 + 256, qPrintable(QString::number(sc.paragraphsWalked() - walked)));   // (once through, not again from the top)
        QCOMPARE(sc.paragraphsChecked(), checked);
        QCOMPARE(sc.knownMisspellings(), 2000);
    }

    // So many ranges waiting for idle time that they are not each looked through for every new one: they are a few, whatever is asked for.
    void theRangesWaitingForIdleTimeAreBounded()
    {
        auto doc = textDoc(misspelledParagraphs(300));
        QTextDocument *story = doc->story();
        SpellCheck sc(story);
        {
            SpellCheck::Pass pass(sc, 0);   // (no time: the first paragraph is checked and each other one is left for idle time)
            for (int i = 0; i < 300; i += 2) {
                const QTextBlock b = story->findBlockByNumber(i);
                sc.misspelledIn(b.position(), b.position() + b.length());
            }
        }
        QVERIFY2(sc.wantedRanges() <= 32 && sc.wantedRanges() > 0, qPrintable(QString::number(sc.wantedRanges())));
    }

    // Replace All over thousands of paragraphs marks each paragraph once and checks each once.
    void aReplaceAllOverThousandsOfParagraphsChecksEachOnce()
    {
        auto doc = textDoc(misspelledParagraphs(5000));
        QTextDocument *story = doc->story();
        SpellCheck sc(story);
        sc.setCounting(true);
        QVERIFY(settle({&sc}));
        QCOMPARE(sc.knownMisspellings(), 5000);
        const qint64 checked = sc.paragraphsChecked();
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            const int at = b.position() + int(b.text().indexOf(QLatin1String("misspeled")));
            replaceWord(story, at, at + 9, QStringLiteral("spelled"));
        }
        QVERIFY(settle({&sc}));
        QCOMPARE(sc.knownMisspellings(), 0);
        QCOMPARE(sc.paragraphsChecked() - checked, qint64(5000));
    }

    // The count in the status bar is updated at a steady pace while the checking goes on, not only after it is over.
    void theStatusBarCountShowsWhileTheCheckingGoesOn()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 600);
        w.show();
        w.view()->setGrammarEnabled(false);
        w.setDocument(textDoc(misspelledParagraphs(40)), QString());
        auto *label = w.findChild<QToolButton *>(QStringLiteral("proofingStatus"));
        QVERIFY(label);
        QTRY_VERIFY(label->text().startsWith(QStringLiteral("40 proofing error")));
        const QString before = label->text();
        QTextDocument *story = w.document()->story();
        // (an edit in every turn, so the count changes all the time: the label must still show it)
        QTRY_VERIFY_WITH_TIMEOUT(([&] {
            QTextCursor c(story);
            c.setPosition(0);
            c.insertText(QStringLiteral("teh "));
            return label->text() != before;
        }()), 30000);
    }

    // A hidden window counts nothing; it goes on when the window is shown.
    void aHiddenWindowCountsNothingUntilItIsShown()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 600);
        w.view()->setGrammarEnabled(false);
        w.setDocument(textDoc(misspelledParagraphs(50)), QString());
        SpellCheck *sc = w.view()->spelling();
        QVERIFY(sc);
        auto *label = w.findChild<QToolButton *>(QStringLiteral("proofingStatus"));
        QVERIFY(label);
        for (int turns = 0; turns < 500; ++turns) QCoreApplication::processEvents();
        QVERIFY(sc->paused());
        QVERIFY(!sc->counted());
        QCOMPARE(sc->knownMisspellings(), 0);
        QCOMPARE(sc->paragraphsWalked(), qint64(0));
        QTRY_VERIFY2(label->text().startsWith(QStringLiteral("0+ proofing error")), qPrintable(label->text()));
        w.show();
        QVERIFY(!sc->paused());
        QTRY_VERIFY(sc->counted());
        QCOMPARE(sc->knownMisspellings(), 50);
        // Hidden again: an edit is marked but not checked until it is shown.
        w.hide();
        QVERIFY(sc->paused());
        QTextCursor c(w.document()->story());
        c.setPosition(0);
        c.insertText(QStringLiteral("teh "));
        for (int turns = 0; turns < 500; ++turns) QCoreApplication::processEvents();
        QCOMPARE(sc->knownMisspellings(), 50);
        w.show();
        QTRY_COMPARE(sc->knownMisspellings(), 51);
    }

    void withCountingOffNothingIsCheckedInIdleTime()
    {
        auto doc = textDoc(misspelledParagraphs(20));
        SpellCheck sc(doc->story());
        for (int turns = 0; turns < 500; ++turns) QCoreApplication::processEvents();   // (counted turns of the event loop, no waiting)
        QCOMPARE(sc.paragraphsChecked(), qint64(0));
        QVERIFY(!sc.counted());
        QCOMPARE(sc.knownMisspellings(), 0);
    }

    void aChangeOfTheLanguageOrTheCheckBeingOffCountsAgain()
    {
        auto doc = textDoc(misspelledParagraphs(10));
        SpellCheck sc(doc->story());
        sc.setCounting(true);
        QTRY_VERIFY(sc.counted());
        QCOMPARE(sc.knownMisspellings(), 10);
        QSignalSpy counts(&sc, &SpellCheck::countChanged);
        sc.setEnabled(false);
        QVERIFY(counts.count() > 0);
        sc.setEnabled(true);
        QVERIFY(!sc.counted());
        QTRY_VERIFY(sc.counted());
        QCOMPARE(sc.knownMisspellings(), 10);
    }

    void theGrammarCountReadsWhatIsKeptAndSendsNothingToTheChecker()
    {
        auto doc = textDoc({QStringLiteral("This is an test of the count."), QStringLiteral("Nothing is wrong here."), QStringLiteral("There is an apple and an test too.")});
        GrammarCheck check(doc->story());
        check.setDelayMs(0);
        QCOMPARE(check.knownIssues(), 0);
        const int end = doc->story()->characterCount();
        check.issuesIn(0, end);
        QTRY_VERIFY(!check.pending());
        const int found = int(check.issuesIn(0, end).size());
        QVERIFY(found >= 2);
        const int checked = check.paragraphsChecked(), looks = check.looksMade();
        for (int i = 0; i < 100; ++i) QCOMPARE(check.knownIssues(), found);
        QCOMPARE(check.paragraphsChecked(), checked);
        QCOMPARE(check.looksMade(), looks);
        check.setEnabled(false);
        QCOMPARE(check.knownIssues(), 0);
    }

    void theStatusBarCountsErrorsAndAClickGoesToTheNextOneAndRoundTheEnd()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 600);
        w.show();
        w.view()->setGrammarEnabled(false);   // (the count here is of the spelling)
        w.setDocument(textDoc({QStringLiteral("First brwon fox."), QStringLiteral("A fine line."), QStringLiteral("Then jumpd high and teh end.")}), QString());
        auto *label = w.findChild<QToolButton *>(QStringLiteral("proofingStatus"));
        QVERIFY(label);
        QTRY_VERIFY2(label->text().startsWith(QStringLiteral("3 proofing error")), qPrintable(label->text()));
        QTRY_VERIFY(label->isVisible());
        auto selected = [&] { return w.view()->cursor().selectedText(); };
        w.view()->setCursor(QTextCursor(w.document()->story()));   // (the caret at the start)
        QTest::mouseClick(label, Qt::LeftButton);
        QCOMPARE(selected(), QStringLiteral("brwon"));
        QTest::mouseClick(label, Qt::LeftButton);
        QCOMPARE(selected(), QStringLiteral("jumpd"));
        QTest::mouseClick(label, Qt::LeftButton);
        QCOMPARE(selected(), QStringLiteral("teh"));
        QTest::mouseClick(label, Qt::LeftButton);
        QCOMPARE(selected(), QStringLiteral("brwon"));   // (round the end)
        // Fixing one lowers the count, to the singular at one.
        QTextCursor c = w.view()->cursor();
        c.insertText(QStringLiteral("brown"));
        w.view()->setCursor(c);
        QTRY_VERIFY2(label->text().startsWith(QStringLiteral("2 proofing error")), qPrintable(label->text()));
        QTextCursor d(w.document()->story());
        d.setPosition(w.document()->story()->toPlainText().indexOf(QStringLiteral("jumpd")));
        d.setPosition(d.position() + 5, QTextCursor::KeepAnchor);
        d.insertText(QStringLiteral("jumped"));
        QTRY_VERIFY2(label->text().startsWith(QStringLiteral("1 proofing error")), qPrintable(label->text()));
    }

    void theCountIsHiddenWhenProofingIsOffAndTellsWhenThereIsNoError()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.view()->setGrammarEnabled(false);
        w.setDocument(textDoc({QStringLiteral("All is well here.")}), QString());
        auto *label = w.findChild<QToolButton *>(QStringLiteral("proofingStatus"));
        QTRY_VERIFY2(label->text().startsWith(QStringLiteral("0 proofing error")), qPrintable(label->text()));
        QTest::mouseClick(label, Qt::LeftButton);
        QTRY_VERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("no spelling or grammar errors")));
        w.view()->setSpellingEnabled(false);
        QTRY_VERIFY(!label->isVisible());
    }

    // ---------------------------------------------------------------- status bar: line and column
    void theStatusBarGivesTheCaretsLineAndColumnOnItsPage()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 600);
        w.show();
        w.setDocument(textDoc({QStringLiteral("First line."), QStringLiteral("Second paragraph here."), QStringLiteral("Third.")}), QString());
        auto *label = w.findChild<QLabel *>(QStringLiteral("caretPlace"));
        QVERIFY(label);
        QTextCursor c(w.document()->story());
        c.setPosition(w.document()->story()->findBlockByNumber(1).position() + 3);
        w.view()->setCursor(c);
        QTRY_COMPARE(label->text(), QStringLiteral("Line 2, Column 4"));
        c.setPosition(0);
        w.view()->setCursor(c);
        QTRY_COMPARE(label->text(), QStringLiteral("Line 1, Column 1"));
        // A column is a character you can see: a letter that takes two places in the text is one.
        w.setDocument(textDoc({QString::fromUtf8("A\xF0\x9F\x98\x80" "B")}), QString());
        QTextCursor e(w.document()->story());
        e.setPosition(3);   // (after the A and the two places of the other letter)
        w.view()->setCursor(e);
        QTRY_COMPARE(label->text(), QStringLiteral("Line 1, Column 3"));
    }

    void aLongParagraphCountsItsWrappedLinesAndHomeGoesBackToColumnOne()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 600);
        w.show();
        QString text;
        for (int i = 0; i < 120; ++i) text += QStringLiteral("word%1 ").arg(i);
        w.setDocument(textDoc({QStringLiteral("Heading"), text.trimmed()}), QString());
        auto *label = w.findChild<QLabel *>(QStringLiteral("caretPlace"));
        QTextCursor c(w.document()->story());
        c.movePosition(QTextCursor::End);
        w.view()->setCursor(c);
        int line = 0, column = 0;
        QTRY_VERIFY(w.view()->caretLineColumn(&line, &column));
        QVERIFY2(line > 3, qPrintable(QString::number(line)));   // (a heading and a paragraph of several lines)
        QVERIFY(column > 1);
        QTRY_COMPARE(label->text(), QStringLiteral("Line %1, Column %2").arg(line).arg(column));
        QTest::keyClick(w.view(), Qt::Key_Home);
        int line2 = 0, column2 = 0;
        QTRY_VERIFY(w.view()->caretLineColumn(&line2, &column2) && column2 == 1);
        QCOMPARE(line2, line);
    }

    // With JO_SHOT_DIR set: the status bar with the place and the count; skipped otherwise.
    void pictureOfTheStatusBarPlaceAndProofingCount()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1000, 520);
        w.show();
        w.view()->setGrammarEnabled(false);
        w.setDocument(textDoc({QStringLiteral("Minutes of the garden committee"), QStringLiteral("The commitee met in March to plan the spring plantting."),
                               QStringLiteral("Volunteers are asked to bring gloves, a trowel, and a watter bottle.")}), QString());
        auto *label = w.findChild<QToolButton *>(QStringLiteral("proofingStatus"));
        QTRY_VERIFY2(label->text().startsWith(QStringLiteral("3 proofing error")), qPrintable(label->text()));
        QTest::mouseClick(label, Qt::LeftButton);
        QTRY_VERIFY(w.findChild<QLabel *>(QStringLiteral("caretPlace"))->text().startsWith(QStringLiteral("Line 2,")));
        QCoreApplication::processEvents();
        w.grab().save(dir + QStringLiteral("/status-bar-place-and-errors.png"));
    }
};

QTEST_MAIN(TailTests)
#include "tst_write_tail.moc"
