#pragma once
// The sounds the effects of a slide start, in the slide show and in the Animation pane's preview. An effect's sound (core/animation.h
// SoundCue) plays from the file the presentation keeps; "Stop Previous Sound" stops the sounds effects before it started. What plays stops
// when the slide goes, and when the show or the preview ends. Nothing is decoded until a sound is asked to play.
//
// Where a sound goes is one function (setOutput): the program plays it through MediaPlayback (mediaplayer.h); a test puts its own in the place
// of it and counts what was asked, with no audio device.

#include "core/animation.h"

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QPointer>
#include <functional>

namespace jp {

class Document;
class MediaPlayback;

class EffectSounds : public QObject {
    Q_OBJECT
public:
    explicit EffectSounds(const Document *doc, QObject *parent = nullptr);

    // What one request to the output is.
    struct Request {
        enum class Kind { Play, StopAll };
        Kind kind = Kind::Play;
        QString media;             // Play: the sound's id among the document's media
        QByteArray bytes;          // Play: the file
        QString extension;         // Play: its extension for the decoder
    };
    void setOutput(const std::function<void(const Request &)> &output) { m_output = output; }

    // The presentation the sounds come from (it may be another one next).
    void rebind(const Document *doc);
    // A sound begins, or the sounds before stop.
    void cue(const deck::SoundCue &cue);
    // Everything the effects started stops.
    void stopAll();
    // The most sounds that play at the same time: a cue that comes while that many play is skipped (never kept for later).
    static constexpr int kMostTogether = 8;
    // The most sounds one step starts, however far apart in time (a file's bound).
    static constexpr int kMostInStep = 64;
    // How many sounds the program is playing now (not counting a test's output).
    int playing() const { return int(m_players.size()); }
    // The output says a sound it was asked to play has ended (the program's own players say it themselves).
    void outputEnded() { if (m_outputPlaying > 0) --m_outputPlaying; }
    // A step begins: the sounds it may start are counted from nothing.
    void beginStep() { m_startedInStep = 0; }
    // How many cues were skipped so far for that, and how many players were dropped because their sound could not be played.
    int skipped() const { return m_skipped; }
    int failedPlayers() const { return m_failed; }

private:
    void play(const QString &mediaId);
    void drop(MediaPlayback *player);
    int m_startedInStep = 0, m_skipped = 0, m_failed = 0, m_outputPlaying = 0;
    const Document *m_doc = nullptr;
    std::function<void(const Request &)> m_output;
    QList<QPointer<MediaPlayback>> m_players;
};

} // namespace jp
