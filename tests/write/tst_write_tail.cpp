// Tests of the small pieces that finish JeffWrite's first release: the status bar's place and proofing count, the navigation pane's section moves, the paper source, and the style area.
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/grammarcheck.h"
#include "app/pageview.h"
#include "app/settings.h"
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

    void withCountingOffNothingIsCheckedInIdleTime()
    {
        auto doc = textDoc(misspelledParagraphs(20));
        SpellCheck sc(doc->story());
        QTest::qWait(50);
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
