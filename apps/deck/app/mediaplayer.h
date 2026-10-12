#pragma once
// Playing a movie or sound a slide holds, and the bar that controls it.
//
// MediaPlayback plays one file through Qt Multimedia (its FFmpeg backend),
// from the bytes the document keeps, as the settings of core/media.h say:
// from where the trim starts to where it ends, at its volume, looping
// until stopped, rewinding after it plays. A movie's picture comes as
// frames (frame()) for the editor and the show to paint over the slide's
// poster. **Nothing of a file is read or decoded until play() is called**,
// so opening, saving, printing, and drawing thumbnails of a presentation
// never touch what a stranger's file holds. A file that can't be played
// reports it (failed(), error()) and the program goes on.
//
// MediaBar is the control bar under a movie or sound: play and pause, a seek
// bar with the time, mute, and volume. It is drawn and read by both the
// editor (editormedia.h) and the show (showmedia.h).

#include "core/media.h"

#include <QBuffer>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QPointer>

class QAudioOutput;
class QMediaPlayer;
class QVideoSink;

namespace jp {

class MediaPlayback : public QObject {
    Q_OBJECT
public:
    enum class State { Stopped, Playing, Paused };

    explicit MediaPlayback(QObject *parent = nullptr);
    ~MediaPlayback() override;

    // The file to play (its bytes, and its extension to tell the decoder what it is) and how. Nothing is read yet.
    void setFile(const QByteArray &bytes, const QString &extension);
    void setPlay(const deck::MediaPlay &play);
    const deck::MediaPlay &settings() const { return m_play; }

    // Says it can't be played here (a movie linked from outside the presentation, say), with what to tell the person.
    void setUnavailable(const QString &message);

    State state() const { return m_state; }
    // It has played, or is playing, or has a picture to show.
    bool started() const { return m_started; }
    bool failed() const { return !m_error.isEmpty(); }
    // What to tell the person when it can't be played; empty when it can.
    QString error() const { return m_error; }
    // Where it is in what plays (the part the trim leaves), and how long that is, in milliseconds; 0 until the file says.
    qint64 played() const;
    qint64 length() const;
    // The movie's picture now, null for a sound or before the first frame, and what it was when the movie ended unless it rewinds.
    QImage frame() const { return m_frame; }
    // The volume the bar sets (0 to 1; the settings' volume is the start), and mute.
    double volume() const { return m_volume; }
    bool muted() const { return m_muted; }

    // How many times a decoder has been started in this program; stays 0 while nothing has been played.
    static int decoders();

public Q_SLOTS:
    // From where it is; from the start when it is stopped.
    void play();
    // From a moment: seconds into what plays.
    void playFrom(double seconds);
    void pause();
    void resume();
    void togglePause();
    // Stops it where it is no longer playing: back to its start, and (when it rewinds) to its poster.
    void stop();
    void seek(qint64 playedMs);
    void setVolume(double volume);
    void setMuted(bool muted);

Q_SIGNALS:
    // The state, the position, or the picture changed.
    void changed();
    // It reached its end (not when it loops, and not when it was stopped).
    void ended();

private:
    bool ensurePlayer();
    void fail(const QString &message);
    void applyVolume();
    void reachedEnd();
    void seekFile(qint64 fileMs);
    qint64 trimStart() const { return m_play.trimStart; }
    qint64 trimEnd() const;
    QByteArray m_bytes;
    QString m_extension;
    QBuffer m_buffer;
    deck::MediaPlay m_play;
    QPointer<QMediaPlayer> m_player;
    QPointer<QAudioOutput> m_audio;
    QPointer<QVideoSink> m_sink;
    State m_state = State::Stopped;
    bool m_started = false;
    bool m_ending = false;
    QString m_error;
    QImage m_frame;
    double m_volume = 0.8;
    bool m_muted = false;
    qint64 m_seekOnLoad = -1;
    qint64 m_fileDuration = 0;
};

// What a file is made of: a picture of its first frame (null for a sound or when none came) and how long it lasts.
struct MediaProbe {
    QImage frame;
    qint64 duration = 0;     // ms; 0 when it couldn't be told
    QString error;           // empty when the file was read
};
// Reads a file as it is put on a slide: waits (the program stays awake) for the first frame of a movie and the length of
// either, for at most `timeoutMs`. Decodes, so only for a file a person chose to put on a slide.
MediaProbe probeMedia(const QByteArray &bytes, const QString &extension, bool wantFrame, int timeoutMs = 8000);

// The numbers a control bar shows.
struct MediaBarState {
    MediaPlayback::State state = MediaPlayback::State::Stopped;
    qint64 played = 0, length = 0;
    double volume = 1;
    bool muted = false;
    bool failed = false;
};

class MediaBar {
public:
    enum class Part { None, Play, Seek, Mute, Volume };
    static constexpr double kHeight = 30;
    static constexpr double kMinWidth = 320;
    // Where the bar goes for a movie or sound that shows in `picture`: under it, as wide as it is (at least kMinWidth),
    // inside `area`; over its bottom when there is no room below.
    static QRectF placement(const QRectF &picture, const QRectF &area);
    static Part partAt(const QRectF &bar, const QPointF &pos);
    // The moment in what plays that a point on the seek bar means, and the volume a point on the volume slider means.
    static qint64 seekAt(const QRectF &bar, const QPointF &pos, qint64 length);
    static double volumeAt(const QRectF &bar, const QPointF &pos);
    static void paint(QPainter *p, const QRectF &bar, const MediaBarState &state, Part hover = Part::None);
    // "0:07", "12:34", "1:02:03".
    static QString timeText(qint64 ms);
};

// A movie's picture over the slide's poster: `frame` fills the picture's cropped place, in the picture's shape, with the
// painter set to page points (or `toDevice` times the page's).
void paintMediaFrame(QPainter *p, const PictureItem &pic, const QImage &frame, const QTransform &toDevice);
// A message in place of a picture that can't play.
void paintMediaMessage(QPainter *p, const PictureItem &pic, const QString &message, const QTransform &toDevice);

} // namespace jp
