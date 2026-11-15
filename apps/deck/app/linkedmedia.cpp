#include "app/linkedmedia.h"

#include "app/mediaplayer.h"
#include "app/trust.h"
#include "core/media.h"
#include "core/remotepath.h"
#include "office/hyperlink.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrl>

namespace jp {

QString linkedMediaFile(const QString &linkIn, const QString &baseDir)
{
    QString path = linkIn.trimmed();
    if (path.isEmpty() || path.size() > 4096) return QString();
    // (a file on another computer, and a program or script: never, whatever is chosen)
    if (isOnAnotherComputer(path) || office::isProgramFile(path)) return QString();
    if (path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        path = QUrl(path).toLocalFile();
        if (path.isEmpty() || isOnAnotherComputer(path)) return QString();
    } else if (QRegularExpression(QStringLiteral("^[A-Za-z][A-Za-z0-9+.-]+:")).match(path).hasMatch() && !QRegularExpression(QStringLiteral("^[A-Za-z]:[\\\\/]")).match(path).hasMatch()) {
        return QString();   // (a web address or any other kind of address)
    }
    if (!QDir::isAbsolutePath(path)) {
        if (baseDir.isEmpty() || isOnAnotherComputer(baseDir)) return QString();
        path = QDir(baseDir).filePath(path);
    }
    path = QDir::cleanPath(path);
    if (isOnAnotherComputer(path) || office::isProgramFile(path)) return QString();
    if (deck::mediaKindOfFile(path).isEmpty()) return QString();
    return QFileInfo(path).isFile() ? path : QString();
}

void giveLinkedMedia(MediaPlayback *pb, const PictureItem &pic, const QString &baseDir, QWidget *parent, QHash<QString, bool> *answers)
{
    const bool video = pic.mediaKind == QLatin1String("video");
    const trust::Choice choice = trust::linkedMedia();
    if (choice == trust::Choice::Never) {
        pb->setUnavailable(video ? QCoreApplication::translate("LinkedMedia", "This movie is linked from outside the presentation, so it can't be played here.")
                                 : QCoreApplication::translate("LinkedMedia", "This sound is linked from outside the presentation, so it can't be played here."));
        return;
    }
    const QString file = linkedMediaFile(pic.mediaLink, baseDir);
    if (file.isEmpty()) {
        pb->setUnavailable(video ? QCoreApplication::translate("LinkedMedia", "This movie is linked to a file that is not on this computer, is missing, or is not a movie file, so it isn't played.")
                                 : QCoreApplication::translate("LinkedMedia", "This sound is linked to a file that is not on this computer, is missing, or is not a sound file, so it isn't played."));
        return;
    }
    if (choice == trust::Choice::Ask) {
        bool yes = false;
        const auto known = answers ? answers->constFind(pic.mediaLink) : QHash<QString, bool>::const_iterator();
        if (answers && known != answers->cend()) yes = known.value();
        else {
            yes = trust::ask(parent, QCoreApplication::translate("LinkedMedia", "Linked Media"),
                             video ? QCoreApplication::translate("LinkedMedia", "This slide has a movie that is linked to a file on this computer. Play it?")
                                   : QCoreApplication::translate("LinkedMedia", "This slide has a sound that is linked to a file on this computer. Play it?"),
                             QDir::toNativeSeparators(file));
            if (answers) answers->insert(pic.mediaLink, yes);
        }
        if (!yes) {
            pb->setUnavailable(video ? QCoreApplication::translate("LinkedMedia", "This linked movie isn't played, because you chose not to play it.")
                                     : QCoreApplication::translate("LinkedMedia", "This linked sound isn't played, because you chose not to play it."));
            return;
        }
    }
    QFile f(file);
    QByteArray bytes;
    // (what the file says its size is can't be trusted, so what is read is counted)
    if (!f.open(QIODevice::ReadOnly) || !deck::readMediaBytes(f, kMostLinkedMediaBytes, &bytes) || bytes.isEmpty()) {
        pb->setUnavailable(video ? QCoreApplication::translate("LinkedMedia", "This linked movie can't be read, or is too large to play from a link (the most is 256 MB).")
                                 : QCoreApplication::translate("LinkedMedia", "This linked sound can't be read, or is too large to play from a link (the most is 256 MB)."));
        return;
    }
    pb->setFile(bytes, deck::mediaExtensionFor(QFileInfo(file).suffix()));
}

} // namespace jp
