#include "app/linkedmedia.h"
#include "core/workthread.h"

#include "app/mediaplayer.h"
#include "app/trust.h"
#include "core/media.h"
#include "core/remotepath.h"
#include "office/hyperlink.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include <condition_variable>
#include <memory>
#include <mutex>

namespace jp {

namespace {

std::function<LinkedRead(const QString &)> g_readStandIn;

const QString kAllLinks = QStringLiteral("*");

QString videoOrSound(const PictureItem &pic, const QString &video, const QString &sound) { return pic.mediaKind == QLatin1String("video") ? video : sound; }

} // namespace

void setLinkedMediaReadStandIn(const std::function<LinkedRead(const QString &)> &standIn) { g_readStandIn = standIn; }

QString linkedMediaFile(const QString &linkIn, const QString &baseDir)
{
    QString path = linkIn.trimmed();
    if (path.isEmpty() || path.size() > 4096) return QString();
    // (a file on another computer, and a program or script: never, whatever is chosen)
    if (isOnAnotherComputer(path) || office::isProgramFile(path)) return QString();
    if (path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        path = QUrl(path).toLocalFile();
        if (path.isEmpty() || isOnAnotherComputer(path)) return QString();
    } else if (QRegularExpression(QStringLiteral("^[A-Za-z][A-Za-z0-9+.-]+:")).match(path).hasMatch()) {
        return QString();   // (a web address or any other kind of address)
    }
    // (a stream of a file, or a device: no file to play, whichever system the presentation is opened on)
    if (isWindowsSpecialPath(path)) return QString();
    const bool relative = !QDir::isAbsolutePath(path);
    if (relative) {
        if (baseDir.isEmpty() || isOnAnotherComputer(baseDir)) return QString();
        path = QDir(baseDir).filePath(path);
    }
    path = QDir::cleanPath(path);
    if (relative) {
        // (a relative name stays in the presentation's folder: it may not go up out of it)
        const QString root = QDir::cleanPath(baseDir) + QLatin1Char('/');
        if (!path.startsWith(root)) return QString();
    }
    if (isOnAnotherComputer(path) || isWindowsSpecialPath(path) || office::isProgramFile(path)) return QString();
    if (deck::mediaKindOfFile(path).isEmpty()) return QString();
    return path;
}

LinkedRead readLinkedMediaFile(const QString &path, qint64 most)
{
    LinkedRead out;
    const QFileInfo info(path);
    // (the size is asked before anything is read, and what is read is counted too: a pipe or a device says 0)
    if (info.isSymLink() || !info.isFile()) out.problem = QStringLiteral("notFile");
    else if (info.size() > most) out.problem = QStringLiteral("tooLarge");
    else {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) out.problem = QStringLiteral("unreadable");
        else if (!deck::readMediaBytes(f, most, &out.bytes)) out.problem = QStringLiteral("tooLarge");
        else if (out.bytes.isEmpty()) out.problem = QStringLiteral("unreadable");
        else if (!deck::looksLikeMediaFile(out.bytes)) out.problem = QStringLiteral("notMedia");
    }
    if (!out.problem.isEmpty()) out.bytes.clear();
    return out;
}

namespace {

// The read on a thread of its own: it is waited for on the event loop for at most `waitMillis`, and a mount that never answers leaves the thread behind (it ends when the system lets it) and
// the link given up.
bool readOnThread(const QString &path, int waitMillis, LinkedRead *out)
{
    struct State {
        std::mutex lock;
        bool done = false;
        LinkedRead result;
    };
    auto state = std::make_shared<State>();
    const auto standIn = g_readStandIn;
    jp::runDetached([state, path, standIn] {
        const LinkedRead r = standIn ? standIn(path) : readLinkedMediaFile(path);
        const std::lock_guard<std::mutex> guard(state->lock);
        state->result = r;
        state->done = true;
    });
    QElapsedTimer clock;
    clock.start();
    for (;;) {
        {
            const std::lock_guard<std::mutex> guard(state->lock);
            if (state->done) {
                *out = state->result;
                return true;
            }
        }
        if (clock.elapsed() > waitMillis) return false;
        QEventLoop loop;
        QTimer::singleShot(15, &loop, &QEventLoop::quit);
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }
}

QString reasonText(const QString &problem, const PictureItem &pic)
{
    if (problem == QLatin1String("tooLarge"))
        return videoOrSound(pic, QCoreApplication::translate("LinkedMedia", "This linked movie is too large to play from a link (the most is 256 MB)."),
                            QCoreApplication::translate("LinkedMedia", "This linked sound is too large to play from a link (the most is 256 MB)."));
    if (problem == QLatin1String("notMedia"))
        return videoOrSound(pic, QCoreApplication::translate("LinkedMedia", "This linked file is not a movie, so it isn't played."), QCoreApplication::translate("LinkedMedia", "This linked file is not a sound, so it isn't played."));
    if (problem == QLatin1String("timeout"))
        return videoOrSound(pic, QCoreApplication::translate("LinkedMedia", "This linked movie took too long to read, so it isn't played."),
                            QCoreApplication::translate("LinkedMedia", "This linked sound took too long to read, so it isn't played."));
    return videoOrSound(pic, QCoreApplication::translate("LinkedMedia", "This linked movie can't be read (it is missing, a link or special file, or not allowed to be read), so it isn't played."),
                        QCoreApplication::translate("LinkedMedia", "This linked sound can't be read (it is missing, a link or special file, or not allowed to be read), so it isn't played."));
}

} // namespace

void giveLinkedMedia(MediaPlayback *pb, const PictureItem &pic, const QString &baseDir, QWidget *parent, LinkedMediaMemory *memory, int waitMillis)
{
    const trust::Choice choice = trust::linkedMedia();
    if (choice == trust::Choice::Never) {
        pb->setUnavailable(videoOrSound(pic, QCoreApplication::translate("LinkedMedia", "This movie is linked from outside the presentation, so it can't be played here."),
                                        QCoreApplication::translate("LinkedMedia", "This sound is linked from outside the presentation, so it can't be played here.")));
        return;
    }
    if (memory) {
        if (const auto known = memory->refused.constFind(pic.mediaLink); known != memory->refused.cend()) {
            pb->setUnavailable(known.value());
            return;
        }
    }
    auto refuse = [&](const QString &why) {
        if (memory) memory->refused.insert(pic.mediaLink, why);
        pb->setUnavailable(why);
    };
    const QString file = linkedMediaFile(pic.mediaLink, baseDir);
    if (file.isEmpty()) {
        refuse(videoOrSound(pic, QCoreApplication::translate("LinkedMedia", "This movie is linked to a file that is not on this computer, is not allowed, or is not a movie file, so it isn't played."),
                            QCoreApplication::translate("LinkedMedia", "This sound is linked to a file that is not on this computer, is not allowed, or is not a sound file, so it isn't played.")));
        return;
    }
    if (choice == trust::Choice::Ask) {
        bool yes = false;
        const auto all = memory ? memory->answers.constFind(kAllLinks) : QHash<QString, bool>::const_iterator();
        const auto known = memory ? memory->answers.constFind(pic.mediaLink) : QHash<QString, bool>::const_iterator();
        if (memory && all != memory->answers.cend()) yes = all.value();
        else if (memory && known != memory->answers.cend()) yes = known.value();
        else {
            yes = trust::ask(parent, QCoreApplication::translate("LinkedMedia", "Linked Media"),
                             videoOrSound(pic, QCoreApplication::translate("LinkedMedia", "This slide has a movie that is linked to a file on this computer. Play it?"),
                                          QCoreApplication::translate("LinkedMedia", "This slide has a sound that is linked to a file on this computer. Play it?")),
                             QDir::toNativeSeparators(file));
            if (memory) memory->answers.insert(pic.mediaLink, yes);
        }
        if (!yes) {
            pb->setUnavailable(videoOrSound(pic, QCoreApplication::translate("LinkedMedia", "This linked movie isn't played, because you chose not to play it."),
                                            QCoreApplication::translate("LinkedMedia", "This linked sound isn't played, because you chose not to play it.")));
            return;
        }
    }
    LinkedRead read;
    if (!readOnThread(file, waitMillis, &read)) {
        refuse(reasonText(QStringLiteral("timeout"), pic));
        return;
    }
    if (!read.problem.isEmpty()) {
        refuse(reasonText(read.problem, pic));
        return;
    }
    pb->setFile(read.bytes, deck::mediaExtensionFor(QFileInfo(file).suffix()));
}

bool askAboutLinkedMedia(const QStringList &links, QWidget *parent, LinkedMediaMemory *memory)
{
    if (links.isEmpty() || trust::linkedMedia() != trust::Choice::Ask) return true;
    if (!memory) return true;
    if (const auto known = memory->answers.constFind(kAllLinks); known != memory->answers.cend()) return known.value();
    const bool yes = trust::ask(parent, QCoreApplication::translate("LinkedMedia", "Linked Media"),
                                QCoreApplication::translate("LinkedMedia", "This show has movies or sounds that are linked to files on this computer. Play them?"), links.join(QLatin1Char('\n')));
    memory->answers.insert(kAllLinks, yes);
    return yes;
}

} // namespace jp
