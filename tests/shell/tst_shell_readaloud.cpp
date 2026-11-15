// Read Aloud's shared part (libs/shell/app/readaloud): the bar's play, pause, stop, and paragraph keys, the word highlighted as it is said, going on from the next paragraph, the speed and voice
// kept, and the plain words when there is no voice or nothing to read. A fake speech engine stands for the system's voices.
#include "app/readaloud.h"
#include "../fakespeech.h"
#include "app/settings.h"
#include "../testrunner.h"

#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QSignalSpy>
#include <QTextDocument>
#include <QToolButton>
#include <QtTest>

using namespace jp;

namespace {

// Says nothing; keeps what it was told, and lets the test play the engine's part.
using FakeEngine = jptest::FakeSpeech;

FakeEngine *g_engine = nullptr;
bool g_noVoices = false;

struct Marks {
    QList<QList<int>> shown;   // story, start in the story, length
    int cleared = 0;
};

ReadAloudHost hostFor(const QList<ReadAloudParagraph> &paras, Marks *marks)
{
    ReadAloudHost h;
    h.text = [paras] { return paras; };
    h.highlight = [marks](const ReadAloudParagraph &p, int start, int length) { marks->shown.push_back({p.story, p.offset + start, length}); };
    h.clear = [marks] { ++marks->cleared; };
    return h;
}

QToolButton *button(QWidget *bar, const char *name) { return bar->findChild<QToolButton *>(QLatin1String(name)); }

class ShellReadAloudTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        g_engine = nullptr;
        g_noVoices = false;
        setSpeechEngineFactory([] {
            auto *e = new FakeEngine;
            if (g_noVoices) e->voiceList.clear();
            g_engine = e;
            return e;
        });
        Settings::get().setValue(QStringLiteral("speech/speed"), 1.0);
        Settings::get().setValue(QStringLiteral("speech/voice"), QString());
    }
    void cleanup() { setSpeechEngineFactory({}); }

    void theFirstParagraphIsSaidAndEachWordIsShown()
    {
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("Hello brave world"), 3, 10}, {QStringLiteral("Second one"), 3, 40}}, &m));
        bar.start();
        QCOMPARE(bar.state(), ReadAloudBar::State::Playing);
        QCOMPARE(g_engine->said, QStringList{QStringLiteral("Hello brave world")});
        g_engine->word(6, 5);
        QCOMPARE(m.shown.size(), 1);
        QCOMPARE(m.shown.last(), (QList<int>{3, 16, 5}));   // (the story, the paragraph's offset 10 plus the word's start 6, and its length)
    }
    void whenOneParagraphEndsTheNextIsSaidAndTheLastEndsTheReading()
    {
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("One"), 0, 0}, {QStringLiteral("Two"), 0, 4}}, &m));
        bar.start();
        g_engine->end();
        QCOMPARE(bar.paragraphIndex(), 1);
        QCOMPARE(g_engine->said, (QStringList{QStringLiteral("One"), QStringLiteral("Two")}));
        const int before = m.cleared;
        g_engine->end();
        QCOMPARE(bar.state(), ReadAloudBar::State::Stopped);
        QVERIFY(m.cleared > before);   // (the highlight is taken away at the end)
    }
    void playPausesAndGoesOnAndStopStops()
    {
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("One"), 0, 0}}, &m));
        bar.start();
        QTest::mouseClick(button(&bar, "readAloudPlay"), Qt::LeftButton);   // (while playing, the Play button is Pause)
        QCOMPARE(bar.state(), ReadAloudBar::State::Paused);
        QCOMPARE(g_engine->pauses, 1);
        QTest::mouseClick(button(&bar, "readAloudPlay"), Qt::LeftButton);
        QCOMPARE(bar.state(), ReadAloudBar::State::Playing);
        QCOMPARE(g_engine->resumes, 1);
        const int cleared = m.cleared;
        QTest::mouseClick(button(&bar, "readAloudStop"), Qt::LeftButton);
        QCOMPARE(bar.state(), ReadAloudBar::State::Stopped);
        QVERIFY(m.cleared > cleared);
        // Play after Stop reads again from the start
        QTest::mouseClick(button(&bar, "readAloudPlay"), Qt::LeftButton);
        QCOMPARE(bar.state(), ReadAloudBar::State::Playing);
        QCOMPARE(g_engine->said.size(), 2);
    }
    void thePreviousAndNextParagraphKeysMoveBetweenParagraphs()
    {
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("A"), 0, 0}, {QStringLiteral("B"), 0, 2}, {QStringLiteral("C"), 0, 4}}, &m));
        bar.start();
        QTest::mouseClick(button(&bar, "readAloudNext"), Qt::LeftButton);
        QCOMPARE(bar.paragraphIndex(), 1);
        QCOMPARE(g_engine->said.last(), QStringLiteral("B"));
        QTest::mouseClick(button(&bar, "readAloudNext"), Qt::LeftButton);
        QCOMPARE(bar.paragraphIndex(), 2);
        QTest::mouseClick(button(&bar, "readAloudPrevious"), Qt::LeftButton);
        QCOMPARE(bar.paragraphIndex(), 1);
        QCOMPARE(g_engine->said.last(), QStringLiteral("B"));
        // Next past the last ends the reading
        bar.nextParagraph();
        bar.nextParagraph();
        QCOMPARE(bar.state(), ReadAloudBar::State::Stopped);
    }
    void aNewSpeedAndVoiceGoOnFromTheWordAndAreRemembered()
    {
        Marks m;
        {
            ReadAloudBar bar(hostFor({{QStringLiteral("Hello brave world"), 0, 0}}, &m));
            bar.start();
            g_engine->word(6, 5);
            bar.setSpeed(2.0);
            QCOMPARE(g_engine->said.last(), QStringLiteral("brave world"));   // (from the word being said)
            QVERIFY(g_engine->rate > 0);
            g_engine->word(6, 5);   // (the word "world", seen from the shorter text)
            QCOMPARE(m.shown.last(), (QList<int>{0, 12, 5}));
            bar.setVoice(QStringLiteral("b"));
            QCOMPARE(g_engine->voice, QStringLiteral("b"));
        }
        // a bar made later has both
        ReadAloudBar later(hostFor({}, &m));
        QCOMPARE(later.speed(), 2.0);
        QCOMPARE(later.voiceId(), QStringLiteral("b"));
        QCOMPARE(g_engine->voice, QStringLiteral("b"));
        QVERIFY(g_engine->rate > 0);
    }
    void withNoVoiceTheBarSaysSoAndSaysNothing()
    {
        g_noVoices = true;
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("One"), 0, 0}}, &m));
        bar.start();
        QCOMPARE(bar.state(), ReadAloudBar::State::Stopped);
        QVERIFY(bar.note().contains(QStringLiteral("No voices")));
        QVERIFY(g_engine->said.isEmpty());
    }
    void withNothingToReadTheBarSaysSo()
    {
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("  "), 0, 0}}, &m));
        bar.start();
        QCOMPARE(bar.state(), ReadAloudBar::State::Stopped);
        QVERIFY(bar.note().contains(QStringLiteral("no text")));
    }
    void aProgramBuiltWithoutSpeechHasNoVoice()
    {
        setSpeechEngineFactory({});
        if (speechIsBuiltIn()) QSKIP("this build has the system's voices");
        std::unique_ptr<SpeechEngine> e(makeSpeechEngine());
        QVERIFY(e->voices().isEmpty());
    }
    void paragraphsAreTheBlocksOfARangeWithTheirPlaces()
    {
        QTextDocument doc;
        doc.setPlainText(QStringLiteral("First one\nSecond\u2028line\nThird"));
        QList<ReadAloudParagraph> all = readAloudParagraphs(&doc, 7);
        QCOMPARE(all.size(), 3);
        QCOMPARE(all.at(0).text, QStringLiteral("First one"));
        QCOMPARE(all.at(1).text, QStringLiteral("Second line"));   // (a line break is said as a space: the places stay)
        QCOMPARE(all.at(1).offset, 10);
        QCOMPARE(all.at(1).story, 7);
        // from the middle of the first, to the middle of the last
        const QList<ReadAloudParagraph> part = readAloudParagraphs(&doc, 0, 6, doc.toPlainText().indexOf(QStringLiteral("Third")) + 2);
        QCOMPARE(part.size(), 3);
        QCOMPARE(part.at(0).text, QStringLiteral("one"));
        QCOMPARE(part.at(0).offset, 6);
        QCOMPARE(part.at(2).text, QStringLiteral("Th"));
    }
    void anEventOfAnUtteranceThatWasCutOffIsDropped()
    {
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("One two"), 0, 0}, {QStringLiteral("Three four"), 0, 10}}, &m));
        bar.start();
        const int first = g_engine->token;
        bar.nextParagraph();   // (the first is cut off: its end and its words may still be on their way)
        QCOMPARE(bar.paragraphIndex(), 1);
        const int shownBefore = m.shown.size();
        g_engine->wordOf(first, 0, 3);
        g_engine->endOf(first);
        QCOMPARE(bar.paragraphIndex(), 1);   // (the late end did not go on to a paragraph that is not there, nor stop the reading)
        QCOMPARE(bar.state(), ReadAloudBar::State::Playing);
        QCOMPARE(m.shown.size(), shownBefore);
        g_engine->word(0, 5);
        QCOMPARE(m.shown.size(), shownBefore + 1);   // (the new text's own words are shown)
        // after a change of speed, what was being said is said again from its word, and the old one's end does not count
        const int second = g_engine->token;
        bar.setSpeed(2.0);
        g_engine->endOf(second);
        QCOMPARE(bar.state(), ReadAloudBar::State::Playing);
    }
    void anErrorOfTheVoiceEndsTheReadingAndSaysWhy()
    {
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("One"), 0, 0}, {QStringLiteral("Two"), 0, 4}}, &m));
        bar.start();
        g_engine->word(0, 3);
        const int cleared = m.cleared;
        g_engine->fail(QStringLiteral("the speech service stopped"));
        QCOMPARE(bar.state(), ReadAloudBar::State::Stopped);
        QVERIFY(bar.note().contains(QStringLiteral("the speech service stopped")));
        QVERIFY(m.cleared > cleared);
        // an error of an utterance that is gone is not the new one's
        bar.start();
        const int old = g_engine->token;
        bar.nextParagraph();
        Q_EMIT g_engine->errorOccurred(old, QStringLiteral("late"));
        QCOMPARE(bar.state(), ReadAloudBar::State::Playing);
    }
    void eventsThroughTheEventLoopAreShownAndTheLateOnesDropped()
    {
        Marks m;
        ReadAloudBar bar(hostFor({{QStringLiteral("One two"), 0, 0}, {QStringLiteral("Three"), 0, 10}}, &m));
        bar.start();
        g_engine->postWord(4, 3);
        QTRY_COMPARE(m.shown.size(), 1);
        g_engine->postEnd();   // (queued for the first paragraph) ...
        bar.nextParagraph();   // ... and the reading has gone on to the second before it arrives
        QTest::qWait(30);
        QCOMPARE(bar.paragraphIndex(), 1);
        QCOMPARE(bar.state(), ReadAloudBar::State::Playing);
    }
    void aChangeOfTheDocumentStopsTheReadingAndClosesTheBar()
    {
        Marks m;
        std::function<void()> changed;
        QObject source;
        ReadAloudHost h = hostFor({{QStringLiteral("One"), 0, 0}}, &m);
        h.watch = [&](const std::function<void()> &call) {
            changed = call;
            return QList<QMetaObject::Connection>();
        };
        h.note = [] { return QStringLiteral("a limit"); };
        ReadAloudBar bar(h);
        QSignalSpy closed(&bar, &ReadAloudBar::closeRequested);
        bar.start();
        QCOMPARE(bar.note(), QStringLiteral("a limit"));   // (what the program says of its text is shown while reading)
        QVERIFY(changed);
        changed();
        QCOMPARE(bar.state(), ReadAloudBar::State::Stopped);
        QCOMPARE(closed.size(), 1);
        QVERIFY(m.cleared > 0);
    }
    void theRateRunsFromSlowToFast()
    {
        QCOMPARE(speechRateForSpeed(1.0), 0.0);
        QCOMPARE(speechRateForSpeed(0.5), -1.0);
        QCOMPARE(speechRateForSpeed(3.0), 1.0);
        QVERIFY(speechRateForSpeed(1.5) > 0 && speechRateForSpeed(1.5) < 1);
    }
    void aWindowGetsOneBarThatStartsReadingAndClosesWithItsButton()
    {
        Marks m;
        QMainWindow win;
        win.show();
        ReadAloudBar *bar = showReadAloud(&win, hostFor({{QStringLiteral("One"), 0, 0}}, &m));
        QVERIFY(bar);
        QCOMPARE(bar->state(), ReadAloudBar::State::Playing);
        QCOMPARE(readAloudBarOf(&win), bar);
        QCOMPARE(showReadAloud(&win, hostFor({{QStringLiteral("One"), 0, 0}}, &m)), bar);   // (a second call reads again; there is still one bar)
        QCOMPARE(win.findChildren<ReadAloudBar *>().size(), 1);
        QTRY_VERIFY(bar->isVisible());
        QTest::mouseClick(button(bar, "readAloudClose"), Qt::LeftButton);
        QCOMPARE(bar->state(), ReadAloudBar::State::Stopped);
        QTRY_VERIFY(!bar->isVisible());
    }
};

} // namespace

JP_TEST_CLASS(ShellReadAloudTests)
#include "tst_shell_readaloud.moc"
