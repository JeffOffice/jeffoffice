#pragma once
// Read Aloud (Review > Read Aloud, the same in all four programs): the text read with this computer's own voices, from the cursor or the selection, with the word being read
// highlighted, and a small bar of Play/Pause, Stop, the paragraph before and after, a speed, and a voice. The speed and the voice are remembered. A program supplies only its text, in
// reading order, and a way to show a range (a ReadAloudHost); the speech itself is a SpeechEngine (the system's, through Qt TextToSpeech where the program was built with it).

#include "core/export.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QWidget>
#include <functional>

class QComboBox;
class QLabel;
class QMainWindow;
class QTextDocument;
class QToolButton;

namespace jp {

struct SpeechVoice {
    QString id, name, language;
};

// What speaks. The system's is made by makeSpeechEngine; a test gives its own.
class JO_COMMON_EXPORT SpeechEngine : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual QList<SpeechVoice> voices() const = 0;
    virtual void setVoice(const QString &id) = 0;
    // -1 (slowest) to 1 (fastest), 0 is the voice's normal speed.
    virtual void setRate(double rate) = 0;
    // Starts saying the text (what was being said is cut off) and returns the number of this utterance: every signal below carries the number of the utterance it is about, so that
    // an event that was on its way when the speech was cut off (a late word, the end of what was stopped) can be told from the new text's and dropped.
    virtual int say(const QString &text) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    // Stops at once; finished() is not sent for what was cut off.
    virtual void stop() = 0;
Q_SIGNALS:
    // The word being spoken: its place in the text given to say().
    void wordSpoken(int utterance, int start, int length);
    // The text given to say() was spoken to its end.
    void finished(int utterance);
    // The voice failed (the speech service is gone, a voice can't be loaded): speech has ended.
    void errorOccurred(int utterance, const QString &message);
};

using SpeechEngineFactory = std::function<SpeechEngine *()>;
// A test's own engine for the bars made after this (an empty function puts the system's back).
JO_COMMON_EXPORT void setSpeechEngineFactory(const SpeechEngineFactory &factory);
// A new engine (the caller owns it): the system's voices, or, in a program built without Qt TextToSpeech, one with no voices.
JO_COMMON_EXPORT SpeechEngine *makeSpeechEngine();
// Whether this build speaks with the system's voices (Qt TextToSpeech is in it).
JO_COMMON_EXPORT bool speechIsBuiltIn();
// The engine's rate for a speed as people say it (1 is normal, 0.5 half, 3 three times as fast).
JO_COMMON_EXPORT double speechRateForSpeed(double speed);

// A paragraph to read. `story` and `offset` are the program's own: which text it is in, and where this paragraph's first character is in it, so that a range of the paragraph can be shown.
struct ReadAloudParagraph {
    QString text;
    int story = 0;
    int offset = 0;
};

// The paragraphs of a text document from character `from` to `to` (to the end when negative), for the story number `story`: each block's text, the first and the last cut to the range, with
// the characters a voice can't say (a picture, a line break) as spaces so that the places stay the same.
JO_COMMON_EXPORT QList<ReadAloudParagraph> readAloudParagraphs(const QTextDocument *doc, int story, int from = 0, int to = -1);

struct ReadAloudHost {
    // The text to read now, in reading order: from the cursor to the end, or the selection alone. Asked each time Play starts from the beginning.
    std::function<QList<ReadAloudParagraph>()> text;
    // Shows [start, start + length) of the paragraph's own text (add paragraph.offset for its place in the story): the word being read.
    std::function<void(const ReadAloudParagraph &paragraph, int start, int length)> highlight;
    // Takes the highlight away.
    std::function<void()> clear;
    // Said in the bar while reading, after the text is gathered (a limit that cut it short); empty for none.
    std::function<QString()> note;
    // Called when reading starts: connects to what changes the document (an edit, a replaced or closed document, a switch of sheet) so that `changed` is called then, and returns the
    // connections (they are let go when the reading ends). The bar stops reading and closes when `changed` is called.
    std::function<QList<QMetaObject::Connection>(const std::function<void()> &changed)> watch;
};

class JO_COMMON_EXPORT ReadAloudBar : public QWidget {
    Q_OBJECT
public:
    explicit ReadAloudBar(const ReadAloudHost &host, QWidget *parent = nullptr);
    ~ReadAloudBar() override;

    // Gets the text from the program and reads it from the first paragraph. Says so, in the bar, when there is no voice or nothing to read.
    void start();
    void stop();
    // Pauses what is read, or goes on.
    void togglePause();
    void previousParagraph();
    void nextParagraph();
    // The document was changed, replaced, or closed: reading stops, and the bar closes.
    void documentChanged();
    // Changes the speed (1 is normal) or the voice (by id), and remembers it; what is being read goes on from its word.
    void setSpeed(double speed);
    void setVoice(const QString &id);

    enum class State { Stopped, Playing, Paused };
    State state() const { return m_state; }
    int paragraphIndex() const { return m_index; }
    // What the bar says in its note: no voices, nothing to read, or empty.
    QString note() const;
    double speed() const;
    QString voiceId() const;
    SpeechEngine *engine() const { return m_engine; }

Q_SIGNALS:
    // The close button was used: whoever holds the bar hides it.
    void closeRequested();

private:
    void speak(int index, int from);
    void onWord(int utterance, int start, int length);
    void onFinished(int utterance);
    void onError(int utterance, const QString &message);
    void finish(const QString &note);
    void updateControls();

    ReadAloudHost m_host;
    SpeechEngine *m_engine;
    QList<ReadAloudParagraph> m_paras;
    State m_state = State::Stopped;
    QList<QMetaObject::Connection> m_watching;
    int m_token = 0;   // (the utterance being said: events of any other are dropped)
    int m_index = 0, m_base = 0, m_word = 0;   // (m_base: where in the paragraph the engine's text begins; m_word: where the word last said began)
    QToolButton *m_play, *m_stop, *m_prev, *m_next, *m_close;
    QComboBox *m_speed, *m_voice;
    QLabel *m_note;
};

// The bar of a window: made at the bottom of `win` the first time (a dock with no title), shown, and told to start. A second call reads again from the cursor.
JO_COMMON_EXPORT ReadAloudBar *showReadAloud(QMainWindow *win, const ReadAloudHost &host);
// The window's bar, or null when none was made.
JO_COMMON_EXPORT ReadAloudBar *readAloudBarOf(QMainWindow *win);

} // namespace jp
