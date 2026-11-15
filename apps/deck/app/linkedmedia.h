#pragma once
// A movie or sound a slide links to a file instead of holding it. What is done is Options > Trust Center's choice: never played (the default), played, or played after a question.
// Only a file on this computer is ever played, whatever the choice: a web address, a file on another computer, a program, and a file that is not a movie or sound are never read.

#include <QHash>
#include <QString>

class QWidget;

namespace jp {

class MediaPlayback;
class PictureItem;

// The most a linked movie or sound may be: it is read into memory when it plays.
constexpr qint64 kMostLinkedMediaBytes = qint64(256) << 20;

// The file a link names, when it may be played from: a movie or sound file on this computer (a relative name counts from `baseDir`, the presentation's folder, which may be empty). Empty for
// anything else, and nothing is read or touched but the name (a file on another computer is judged by its name).
QString linkedMediaFile(const QString &link, const QString &baseDir);

// Gives `playback` the picture's linked file to play, or says why it can't (for "never", for a link that may not be followed, and for a "no" to the question). `answers` keeps what was
// answered for each link, so a link is asked about once.
void giveLinkedMedia(MediaPlayback *playback, const PictureItem &pic, const QString &baseDir, QWidget *parent, QHash<QString, bool> *answers);

} // namespace jp
