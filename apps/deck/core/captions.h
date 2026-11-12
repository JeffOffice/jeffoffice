#pragma once
// Captions for a video: WebVTT files a video holds as tracks (one for each language), and what they say at a moment of the video.
//
// A track's file is kept as it came (PictureItem::mediaCaptions names it in the document's media store), so a format this does not read
// (anything but WebVTT) is written back as it was and shows nothing. Nothing here decodes a video.

#include "core/document.h"

#include <QByteArray>
#include <QString>
#include <QVector>

namespace jp::deck {

// What a caption file is held to. Real ones (a two-hour film: about 150 KB, 2,000 cues, none over 200 characters) are far under these: more
// than ten times the size, the cues, and the length of a cue. A cue's text past its limit is cut; cues past the limit are left out.
constexpr qint64 kMaxCaptionFileBytes = kMaxCaptionBytes;
constexpr int kMaxCaptionCues = 20000;
constexpr int kMaxCueChars = 2000;
constexpr int kMaxActiveCues = 4;   // cues shown at one moment, at most

struct CaptionCue {
    qint64 start = 0, end = 0;   // milliseconds from the start of the video's file
    QString text;                // lines joined by newlines, markup taken out
    bool operator==(const CaptionCue &o) const { return start == o.start && end == o.end && text == o.text; }
};

struct CaptionFile {
    QVector<CaptionCue> cues;    // by start
    QString error;               // why nothing can be shown, in words for a person; empty when there is a cue
    int leftOut = 0;             // cues (or damaged blocks) that were not kept
};

// Whether the bytes begin as a WebVTT file does.
bool looksLikeWebVtt(const QByteArray &bytes);

// Reads a WebVTT file. Never fails hard: a file that is not one, is too large, or holds no cue comes back with `error` and no cues; a damaged
// cue is left out and counted.
CaptionFile parseWebVtt(const QByteArray &bytes);

// What the cues say at `ms` (cues that overlap are joined, one under the other); empty when none shows then.
QString captionTextAt(const QVector<CaptionCue> &cues, qint64 ms);

// Whether this is a language tag a track may be given (letters, digits, and hyphens, 1 to 35 of them).
bool validCaptionLanguage(const QString &language);

// Puts a track on the video, for the language (the video's track for it, if it has one, is replaced). Returns empty when it is done, else why it is not
// (the file is not WebVTT or holds nothing to show, or it is too large, or the video has all the tracks it may).
QString addCaptions(Document &doc, PictureItem &video, const QByteArray &bytes, const QString &language);

// Takes out the track for exactly that language (an empty one names a track whose language is empty, not all of them), or every track. The
// track's file goes out of the document's media when nothing else (another video, another track) keeps it. Whether any track went.
bool removeCaptions(Document &doc, PictureItem &video, const QString &language);
bool removeAllCaptions(Document &doc, PictureItem &video);

// The cues of the track a show of the video displays: the one for `preferred` (a language tag) if there is one that is WebVTT, else the first that is.
// A file is read once while its bytes are the same: asking again for the same one (as every change to the document does) is no work.
QVector<CaptionCue> captionCuesOf(const Document &doc, const PictureItem &video, const QString &preferred = QString());

// How many times a file has been read by parseWebVtt in this program (for the tests of what is read once).
int captionParseCount();

// Whether the video has a caption track (of any format).
bool hasCaptions(const PictureItem &video);

} // namespace jp::deck
