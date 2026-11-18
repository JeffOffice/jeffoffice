#include "app/embeddededit.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUrl>

namespace jp {

using namespace office::ole;

EmbeddedEdit::EmbeddedEdit(QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, &EmbeddedEdit::poll);
    m_open = [](const QString &path, Kind) { return QDesktopServices::openUrl(QUrl::fromLocalFile(path)); };
}

EmbeddedEdit::~EmbeddedEdit() { closeAll(); }

void EmbeddedEdit::setPollInterval(int ms) { m_timer.setInterval(qMax(1, ms)); }

QString EmbeddedEdit::pathOf(int id) const
{
    auto it = m_sessions.find(id);
    return it == m_sessions.end() ? QString() : it->second.path;
}

int EmbeddedEdit::start(const EmbeddedObject &object)
{
    Kind kind = object.kind();
    if (!object.isOurs || kind == Kind::Other || object.partBytes.isEmpty() || object.partBytes.size() > kMaxObjectBytes) return 0;
    if (m_sessions.size() >= kMaxOpen) return 0;
    QString root = m_root.isEmpty() ? QDir::tempPath() : m_root;
    if (!QDir().mkpath(root)) return 0;
    Session s;
    s.dir = std::make_unique<QTemporaryDir>(QDir(root).filePath(QStringLiteral("jo-embed-XXXXXX")));
    if (!s.dir->isValid()) return 0;
    s.path = s.dir->filePath(QStringLiteral("Object.") + extensionOf(kind));
    {
        QFile f(s.path);
        if (!f.open(QIODevice::WriteOnly) || f.write(object.partBytes) != object.partBytes.size()) return 0;
    }
    QFileInfo fi(s.path);
    s.deliveredSize = fi.size();
    s.deliveredTime = fi.lastModified().toMSecsSinceEpoch();
    s.object = object;
    s.lastBytes = object.partBytes;
    if (!m_open || !m_open(s.path, kind)) return 0;   // the temp folder goes with s
    int id = m_next++;
    m_sessions.emplace(id, std::move(s));
    m_timer.start();
    return id;
}

void EmbeddedEdit::close(int id)
{
    if (m_sessions.erase(id) && m_sessions.empty()) m_timer.stop();
}

void EmbeddedEdit::closeAll()
{
    m_sessions.clear();
    m_timer.stop();
}

void EmbeddedEdit::end(int id, EndReason reason)
{
    close(id);
    Q_EMIT watchEnded(id, int(reason));
}

void EmbeddedEdit::poll()
{
    QList<int> ids;
    for (const auto &kv : m_sessions) ids << kv.first;
    for (int id : ids)
        if (m_sessions.count(id)) look(id);
}

void EmbeddedEdit::look(int id)
{
    Session &s = m_sessions.at(id);
    QFileInfo fi(s.path);
    if (!fi.exists() && !fi.isSymLink()) {   // a program may delete and put back in one save: gone at two looks in a row is deleted
        if (++s.missing >= 2) end(id, EndReason::Deleted);
        return;
    }
    s.missing = 0;
    if (fi.isSymLink() || !fi.isFile()) return end(id, EndReason::Replaced);
    qint64 size = fi.size(), time = fi.lastModified().toMSecsSinceEpoch();
    if (size > kMaxObjectBytes) return end(id, EndReason::TooLarge);
    if (size == s.deliveredSize && time == s.deliveredTime) { s.pendingSize = -1; return; }
    if (size != s.pendingSize || time != s.pendingTime) {   // still changing: look again before reading
        s.pendingSize = size;
        s.pendingTime = time;
        return;
    }
    QFile f(s.path);
    if (!f.open(QIODevice::ReadOnly)) return end(id, EndReason::Unreadable);
    QByteArray bytes = f.read(kMaxObjectBytes + 1);
    f.close();
    if (bytes.size() > kMaxObjectBytes) return end(id, EndReason::TooLarge);
    s.deliveredSize = size;
    s.deliveredTime = time;
    s.pendingSize = -1;
    if (bytes == s.lastBytes) return;
    auto updated = withNewBytes(s.object, bytes, m_draw);
    if (!updated) {
        Q_EMIT saveRefused(id);
        return;
    }
    s.object = *updated;
    s.lastBytes = bytes;
    Q_EMIT objectUpdated(id, *updated);
}

} // namespace jp
