#pragma once
// A speech engine for the tests of Read Aloud (no sound): it keeps what it was told to say, and lets the test play the engine's part: the word being said and the end of the text, at once or
// later (the events carry the number of the utterance they are about, so a test can send one that is late: from an utterance already cut off).
#include "app/readaloud.h"

#include <QMetaObject>
#include <QStringList>

namespace jptest {

class FakeSpeech : public jp::SpeechEngine {
public:
    QList<jp::SpeechVoice> voiceList{{QStringLiteral("a"), QStringLiteral("Ann"), QStringLiteral("en_US")}, {QStringLiteral("b"), QStringLiteral("Ben"), QStringLiteral("en_GB")}};
    QStringList said;
    QString voice;
    double rate = 0;
    int pauses = 0, resumes = 0, stops = 0;
    int token = 0;   // the utterance being said now
    QList<jp::SpeechVoice> voices() const override { return voiceList; }
    void setVoice(const QString &id) override { voice = id; }
    void setRate(double r) override { rate = r; }
    int say(const QString &text) override
    {
        said << text;
        return ++token;
    }
    void pause() override { ++pauses; }
    void resume() override { ++resumes; }
    void stop() override { ++stops; }
    // The engine says a word of the utterance being said, or of an earlier one (late); the text ends; the voice fails.
    void word(int start, int length) { Q_EMIT wordSpoken(token, start, length); }
    void wordOf(int utterance, int start, int length) { Q_EMIT wordSpoken(utterance, start, length); }
    void end() { Q_EMIT finished(token); }
    void endOf(int utterance) { Q_EMIT finished(utterance); }
    void fail(const QString &message) { Q_EMIT errorOccurred(token, message); }
    // The same, but through the event loop, as a real engine's events arrive (for the utterance being said when it is called).
    void postWord(int start, int length)
    {
        const int t = token;
        QMetaObject::invokeMethod(this, [this, t, start, length] { Q_EMIT wordSpoken(t, start, length); }, Qt::QueuedConnection);
    }
    void postEnd()
    {
        const int t = token;
        QMetaObject::invokeMethod(this, [this, t] { Q_EMIT finished(t); }, Qt::QueuedConnection);
    }
};

} // namespace jptest
