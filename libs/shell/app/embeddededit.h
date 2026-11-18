#pragma once
// Editing an embedded object in its own app: the object's package is written to a temp file, opened in the program that made it, and watched; each time that program saves, the new bytes
// and a new picture come back as a whole object, for the window to put in as one undo step. The watch ends, and the temp file goes, when the object is closed, the file is deleted or
// replaced by something that is not a file, or it grows past the cap. Everything is cleared when the editor is destroyed.

#include "core/export.h"
#include "ole/embedobj.h"

#include <QObject>
#include <QString>
#include <QTimer>

#include <functional>
#include <map>
#include <memory>

class QTemporaryDir;

namespace jp {

class JO_COMMON_EXPORT EmbeddedEdit : public QObject {
    Q_OBJECT
public:
    enum class EndReason { Closed, Deleted, Replaced, TooLarge, Unreadable };
    static constexpr int kMaxOpen = 16;   // objects being edited at once

    explicit EmbeddedEdit(QObject *parent = nullptr);
    ~EmbeddedEdit() override;

    // How a window draws the picture of an object of ours again after a save (the apps' own renderers, through here).
    void setPicture(office::ole::PictureFn draw) { m_draw = std::move(draw); }
    // How the file is opened: the path and the object's kind. The default hands it to the system; an app may give one that finds a running instance of the program first. Return false if it could not.
    void setOpener(std::function<bool(const QString &path, office::ole::Kind kind)> open) { m_open = std::move(open); }
    // Where the temp files go (a folder made for the purpose; default: the system's temp folder) and how often the file is looked at.
    void setTempRoot(const QString &folder) { m_root = folder; }
    void setPollInterval(int ms);

    // Writes the object to a temp file and opens it. Returns the edit's number, or 0 when the object is not ours, is over the cap, too many are open, or the file could not be written or opened.
    int start(const office::ole::EmbeddedObject &object);
    // Ends the watch and deletes the temp file; the number is not used again.
    void close(int id);
    void closeAll();
    bool isOpen(int id) const { return m_sessions.count(id) != 0; }
    int openCount() const { return int(m_sessions.size()); }
    QString pathOf(int id) const;

Q_SIGNALS:
    // The program saved: the whole object, with the new bytes and picture.
    void objectUpdated(int id, const jp::office::ole::EmbeddedObject &object);
    // A save that is not a package (half written, or another kind of file) was left out; the watch goes on.
    void saveRefused(int id);
    // The watch ended on its own (not by close()); the temp file is gone.
    void watchEnded(int id, int reason);

private:
    struct Session {
        std::unique_ptr<QTemporaryDir> dir;
        QString path;
        office::ole::EmbeddedObject object;
        QByteArray lastBytes;
        qint64 deliveredSize = -1, deliveredTime = -1;   // what the file looked like when last taken in
        int missing = 0;                                 // looks in a row at which the file was not there
        qint64 pendingSize = -1, pendingTime = -1;       // what it looked like at the last look, if that differed: it must hold still for one more look
    };
    void poll();
    void look(int id);
    void end(int id, EndReason reason);

    std::map<int, Session> m_sessions;
    int m_next = 1;
    QTimer m_timer;
    QString m_root;
    office::ole::PictureFn m_draw;
    std::function<bool(const QString &, office::ole::Kind)> m_open;
};

} // namespace jp
