#pragma once
// The movies and sounds on a slide, and how each one plays. A picture that
// plays one is a PictureItem with a mediaKind (the file's p:pic with an
// a:videoFile or a:audioFile); how it plays is in two places the file keeps
// and this reads and writes: the slide's timing (page props "timingXml": the
// p:video or p:audio that says volume, looping, and whether it shows while
// stopped, and the calls that start it) and the picture's trim and fade
// (p14:trim and p14:fade of its p14:media).
//
// Nothing here decodes a movie or sound. That is app/mediaplayer.h, and it
// does so only when a person or the show asks for the media to play.

#include "core/document.h"

#include <QHash>
#include <QImage>
#include <QIODevice>
#include <QPointF>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>

namespace jp::deck {

// What starts it: a click in the slide's click sequence, the slide itself
// (after what comes before it), or a click on a shape (its own, usually).
enum class MediaStart { ClickSequence, Automatically, WhenClickedOn };

// The volumes the Playback tab offers, in thousandths of a percent as the file writes them.
constexpr int kVolumeLow = 20000, kVolumeMedium = 50000, kVolumeHigh = 80000;

struct MediaPlay {
    int shape = 0;                        // the picture's number in the file (Item::sourceId); 0 for one the timing can't name
    bool video = false;                   // else a sound
    MediaStart start = MediaStart::ClickSequence;
    int volume = kVolumeHigh;
    bool mute = false;
    bool loop = false;                    // until stopped
    bool rewind = false;                  // to the start after it plays
    bool fullScreen = false;              // a movie fills the screen in the show
    bool hideWhenStopped = false;         // in the show, the picture shows only while it plays
    int acrossSlides = 1;                 // slides it plays across; more than 1 and it goes on when the slide changes
    int trimStart = 0, trimEnd = 0;       // ms cut off the start of the file and off its end (as p14:trim says); it plays what is left
    int fadeIn = 0, fadeOut = 0;          // ms its volume rises at the start and falls at the end
    int lengthHint = 0;                   // ms it plays for, as the timing's call says (0 if it doesn't); for a bar to show before the file is played. Not compared
    bool operator==(const MediaPlay &o) const;
    bool operator!=(const MediaPlay &o) const { return !(*this == o); }
};

// The pictures of the slide that play a movie or sound (in groups too), in order.
QVector<const PictureItem *> mediaPicturesOf(const Page &slide);

// How the picture plays, as the slide's timing and the picture say.
MediaPlay mediaPlayOf(const Page &slide, const PictureItem &pic);

// How long the whole file lasts, as the timing's call says what is left after the trim: that and what is cut off each end; 0 when the call
// says nothing (a movie that waits for a click has a call that lasts a millisecond).
qint64 knownFileLength(const MediaPlay &play);

// Makes the slide's timing and the picture say it plays as `play` does: what
// the timing said of this picture is replaced (and the calls of others, and
// the rest of the animations, stay as they are), the picture gets a number
// the timing can name if it has none, and its trim and fade are set. `lengthMs`
// is how long the file plays, when known; the file's click step lasts as long.
void setMediaPlay(Page &slide, PictureItem &pic, const MediaPlay &play, qint64 lengthMs = 0);

// Takes out of the slide's timing everything it does for that shape's media (its node and its calls); the timing goes if nothing else is in it.
void clearMediaPlay(Page &slide, int shape);

// The timing as it is, but for the media of shapes that are not among `shapes` (the shapes a slide is written with): a
// picture that played a movie and was deleted takes its node and its calls with it, and the rest of the timing stays.
// The same string when nothing goes; empty when nothing is left.
QString mediaTimingForShapes(const QString &timingXml, const QSet<qint64> &shapes);

// What the slide's timing says to do with a movie or sound, as the show plays
// it. A cue is a call at a moment: ms after its step starts.
struct MediaCue {
    int shape = 0;
    QString command;             // "playFrom(0.0)", "togglePause", "play", "pause", "stop", "resume", "playFrom(12.5)"
    double at = 0;
};
struct MediaStep {
    bool automatic = false;      // begins by itself when the slide is shown
    bool visual = false;         // something is shown or moved: the show counts it as a click step of its own
    QVector<MediaCue> cues;
};
struct MediaPlan {
    QHash<int, MediaPlay> plays;                   // by the picture's number
    QVector<MediaStep> steps;                      // the main sequence, every step as written
    QHash<int, QVector<MediaCue>> triggers;        // a click on this shape (by number) does these
    bool isEmpty() const { return plays.isEmpty(); }
};
// Nothing is read for a slide with no movie or sound.
MediaPlan mediaPlanOf(const Page &slide);

// The hidden-while-stopped pictures of the slide: their ids, for a show that paints them only while they play.
QSet<QString> mediaHiddenWhenStopped(const Page &slide);

// ---- a movie or sound put on a slide ----

// What a file of this kind is: "video" or "audio" from its extension, empty when it is neither.
QString mediaKindOfFile(const QString &fileName);
// Its media type for the package ("video/mp4"), and a general one when unknown.
QString mediaTypeOfFile(const QString &fileName, const QString &kind);
// The name's end that a movie or sound's file may be given for the decoder to go by: one of those below, in lower case, else "bin". The
// name a stranger's file gave its part (m3u8, sdp, a path) never goes to the decoder.
QString mediaExtensionFor(const QString &extension);
// Whether the bytes could be a movie or sound's file. A text (a playlist, a manifest, a list of files to join, a session
// description) is a list of places to fetch, which a decoder follows: none of that is media, and nothing here follows it.
bool looksLikeMediaFile(const QByteArray &bytes);
// The file types Insert > Video and Insert > Audio offer, as a file dialog's filter.
QStringList mediaExtensions(const QString &kind);

// A movie or sound put on a slide as the presentation program does: the file
// kept in the document, a picture that plays it (the poster, with the
// speaker's picture for a sound), the picture's number, and the timing that
// plays it in the click sequence. The picture is centered on the slide
// unless `at` says where its corner goes; its size is the poster's pixels
// (a sound's is fixed), no larger than most of the slide. Returns the
// picture, not yet on the slide's items.
std::shared_ptr<PictureItem> newMediaPicture(Document &doc, Page &slide, const QByteArray &bytes, const QString &fileName, const QImage &poster, qint64 lengthMs,
                                             const QPointF &at = QPointF(-1, -1));

// The speaker picture a sound shows (a dark circle with a speaker).
QImage soundPoster();

// What a file can be written and read with: all the presentation's pictures, movies, and sounds together may come to this much (a file
// is read only up to Package::kDefaultMaxInflated inflated, and the rest of it takes room too). A file is never put in past it.
qint64 mediaByteLimit();
void setMediaByteLimit(qint64 limit);   // for the tests; 0 puts the usual one back
// Whether `bytes` can go among the document's media and the whole still be read again: bytes it already keeps add nothing.
bool mediaWouldFit(const Document &doc, const QByteArray &bytes);
// What a file holds, at most `most` bytes of it: false, with nothing read, when there is more. A file's size can't be trusted (a pipe or a
// device says 0 and goes on), so what is read is counted, and one byte past the most is as far as it goes.
bool readMediaBytes(QIODevice &device, qint64 most, QByteArray *out);

} // namespace jp::deck
