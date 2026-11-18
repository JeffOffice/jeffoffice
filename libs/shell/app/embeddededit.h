#pragma once
// Editing an embedded object in its own app: the object's package is written to a temp file, opened in the program that made it, and watched; each time that program saves, the new
// bytes and a new picture come back (not a whole object: the window puts them into its current object with office::ole::applySave, as one undo step, so a size changed during the edit
// stays). The watch ends, and the temp folder goes, when the edit is closed, its owner (the document) closes or is destroyed, the file is deleted or replaced by something that is not a
// file, it cannot be read for several looks, or it grows past the cap. Everything is cleared when the editor is destroyed; a folder that cannot be deleted (the other program holds it)
// is tried again at the next start and at closeAll, and folders a crash left behind are removed when they are a day old.

#include "core/export.h"
#include "ole/embedobj.h"

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <functional>
#include <map>
#include <memory>

class QLockFile;

namespace jp {

class JO_COMMON_EXPORT EmbeddedEdit : public QObject {
    Q_OBJECT
public:
    enum class EndReason { Closed, Deleted, Replaced, TooLarge, Unreadable };
    Q_ENUM(EndReason)
    static constexpr int kMaxOpen = 16;           // objects being edited at once
    static constexpr int kUnreadableLooks = 5;    // looks in a row at which the file cannot be opened before the watch ends
    static constexpr qint64 kHashEveryLook = 4 * 1024 * 1024;   // a file up to this size is read and hashed at every look; a larger one only when its size or time changed

    explicit EmbeddedEdit(QObject *parent = nullptr);
    ~EmbeddedEdit() override;

    // How a window draws the picture of an object of a kind again after a save (the apps' own renderers, through here).
    void setPicture(office::ole::Kind kind, office::ole::PictureFn draw);
    // How the file is opened: the path and the object's kind. The default hands it to the system; an app may give one that finds a running instance of the program first. Return false if it could not.
    void setOpener(std::function<bool(const QString &path, office::ole::Kind kind)> open) { m_open = std::move(open); }
    // Where the temp files go (a folder made for the purpose; default: the system's temp folder) and how often the file is looked at.
    void setTempRoot(const QString &folder) { m_root = folder; }
    void setPollInterval(int ms);

    // Writes the object to a temp file and opens it. `owner` is the document the object is in and `key` says which object of it; starting the same owner and key again returns the open
    // edit. Returns the edit's number, or 0 when the owner is missing, the object is not ours (or its bytes are not a safe package), is over the cap, too many are open, or the file
    // could not be written or opened.
    int start(QObject *owner, const QString &key, const office::ole::EmbeddedObject &object);
    // Ends the watch and deletes the temp folder; the number is not used again. closeFor ends every edit of a document (also done when the owner is destroyed).
    void close(int id);
    void closeFor(QObject *owner);
    void closeAll();
    bool isOpen(int id) const { return m_sessions.count(id) != 0; }
    int openCount() const { return int(m_sessions.size()); }
    QString pathOf(int id) const;

    // Looks at every watched file now, as the timer does (what a test calls, with a long interval, to run the looks one at a time).
    void pollNow() { poll(); }
    // Removes the jo-embed-* folders in `root` that this user owns, that were last changed before `olderThan` (default: a day ago), and whose lock no running copy holds. Returns how many.
    static int sweepStale(const QString &root, const QDateTime &olderThan = QDateTime());

Q_SIGNALS:
    // The program saved: the new bytes, and a picture for them (empty, with `pictureStale` set, when it could not be drawn: keep the old one).
    void objectSaved(int id, const QByteArray &partBytes, const QByteArray &previewPng, bool pictureStale);
    // A save that is not a safe package (half written, another kind of file, or one that unpacks to too much) was left out; the watch goes on.
    void saveRefused(int id);
    // The watch ended on its own (not by close()); the temp folder is gone or will be tried again.
    void watchEnded(int id, jp::EmbeddedEdit::EndReason reason);

private:
    struct Session {
        QString dir, path, key;
        std::shared_ptr<QLockFile> lock;                  // held in the folder while the edit lives, so another copy of the program does not sweep it
        QObject *owner = nullptr;
        office::ole::Kind kind = office::ole::Kind::Other;
        qint64 widthEmu = 0, heightEmu = 0;
        QByteArray hash;                                  // of the bytes last taken in
        qint64 deliveredSize = -1, deliveredTime = -1;
        qint64 pendingSize = -1, pendingTime = -1;        // a change must hold (size, time, and hash) for two looks before it is read
        QByteArray pendingHash;
        int pendingLooks = 0;
        int missing = 0;                                  // looks in a row at which the file was not there
        int unreadable = 0;                               // looks in a row at which it could not be opened
    };
    void poll();
    void look(int id);
    void end(int id, EndReason reason);
    void erase(std::map<int, Session>::iterator it);
    void removeFolder(const QString &dir);
    void retryFolders();

    std::map<int, Session> m_sessions;
    QStringList m_leftover;   // folders that could not be deleted yet
    QStringList m_swept;      // roots already swept
    QTimer m_timer;
    QString m_root;
    office::ole::PictureFn m_draw[2];
    std::function<bool(const QString &, office::ole::Kind)> m_open;
};

} // namespace jp
