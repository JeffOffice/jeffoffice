#pragma once
// A movie or sound a slide links to a file instead of holding it. What is done is Options > Security's choice: never played (the default), played, or played after a question.
// Only a movie or sound file whose name is on this computer is ever played, whatever the choice: a web address, a file on another computer, a program, a stream or device name, a link
// or special file, a file that is not a movie or sound (by its name and by its bytes), and one that is too large are never read. The file is read on a thread of its own, with a time limit,
// so a mount that does not answer cannot hold the window or the show.

#include <QByteArray>
#include <QHash>
#include <QString>
#include <functional>

class QWidget;

namespace jp {

class MediaPlayback;
class PictureItem;

// The most a linked movie or sound may be: it is read into memory when it plays.
constexpr qint64 kMostLinkedMediaBytes = qint64(256) << 20;
// The longest the file may take to be read before the link is given up for the session.
constexpr int kLinkedMediaReadMillis = 20000;

// What a linked file's reading came to.
struct LinkedRead {
    QByteArray bytes;
    QString problem;   // empty when the bytes are a movie or sound
};

// What was answered and refused for each link, kept by the window or show for as long as it lives: a link is asked about once, and one that failed is not read again.
struct LinkedMediaMemory {
    QHash<QString, bool> answers;
    QHash<QString, QString> refused;   // by link: why it was refused
};

// The file a link names, when its name allows it: a movie or sound file on this computer (a relative name counts from `baseDir`, the presentation's folder, and may not leave it; it may be
// empty). Empty for anything else. Only the name is looked at: no file is touched here (a file on another computer, or a mount that does not answer, must not hold anything up).
QString linkedMediaFile(const QString &link, const QString &baseDir);

// Reads the file (a regular file, not a link, within the cap, whose bytes look like a movie or sound). The work is done by whatever thread calls it.
LinkedRead readLinkedMediaFile(const QString &path, qint64 most = kMostLinkedMediaBytes);

// Gives `playback` the picture's linked file to play, or says why it can't (for "never", for a link that may not be followed, a refusal, a "no" to the question, and a read that failed or
// took too long). The read waits on the event loop (the window stays alive) for at most `waitMillis`.
void giveLinkedMedia(MediaPlayback *playback, const PictureItem &pic, const QString &baseDir, QWidget *parent, LinkedMediaMemory *memory, int waitMillis = kLinkedMediaReadMillis);

// For the media of a slide show that has a linked file: asks once, when the show starts, and true when there is nothing to ask or the answer is yes (so the show never asks in the middle of
// a full screen). `memory` keeps the answer for every link of the show.
bool askAboutLinkedMedia(const QStringList &links, QWidget *parent, LinkedMediaMemory *memory);

// A test stands in for the read (set to an empty function: the file is read).
void setLinkedMediaReadStandIn(const std::function<LinkedRead(const QString &path)> &standIn);

} // namespace jp
