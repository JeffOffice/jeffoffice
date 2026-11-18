#include "app/embeddededit.h"

#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUrl>

#include <atomic>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace jp {

using namespace office::ole;

namespace {
std::atomic<int> g_nextId{1};   // one count for the whole program, so the files' names differ even between editors

QByteArray hashOf(const QByteArray &bytes) { return QCryptographicHash::hash(bytes, QCryptographicHash::Sha1); }
} // namespace

EmbeddedEdit::EmbeddedEdit(QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, &EmbeddedEdit::poll);
    m_open = [](const QString &path, Kind) { return QDesktopServices::openUrl(QUrl::fromLocalFile(path)); };
}

EmbeddedEdit::~EmbeddedEdit() { closeAll(); }

void EmbeddedEdit::setPicture(Kind kind, PictureFn draw)
{
    if (kind == Kind::Sheet) m_draw[0] = std::move(draw);
    else if (kind == Kind::Page) m_draw[1] = std::move(draw);
}

void EmbeddedEdit::setPollInterval(int ms) { m_timer.setInterval(qMax(1, ms)); }

QString EmbeddedEdit::pathOf(int id) const
{
    auto it = m_sessions.find(id);
    return it == m_sessions.end() ? QString() : it->second.path;
}

int EmbeddedEdit::sweepStale(const QString &root, const QDateTime &olderThan)
{
    QDateTime cutoff = olderThan.isValid() ? olderThan : QDateTime::currentDateTime().addDays(-1);
    int removed = 0;
    const QFileInfoList dirs = QDir(root).entryInfoList({QStringLiteral("jo-embed-*")}, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QFileInfo &d : dirs) {
#ifdef Q_OS_UNIX
        if (d.ownerId() != geteuid()) continue;
#endif
        if (d.lastModified() >= cutoff) continue;
        if (QDir(d.absoluteFilePath()).removeRecursively()) ++removed;
    }
    return removed;
}

void EmbeddedEdit::removeFolder(const QString &dir)
{
    if (QFileInfo::exists(dir) && !QDir(dir).removeRecursively()) m_leftover << dir;
}

void EmbeddedEdit::retryFolders()
{
    QStringList still;
    for (const QString &d : m_leftover)
        if (QFileInfo::exists(d) && !QDir(d).removeRecursively()) still << d;
    m_leftover = still;
}

int EmbeddedEdit::start(QObject *owner, const QString &key, const EmbeddedObject &object)
{
    Kind kind = object.kind();   // Other unless it is ours
    if (!owner || kind == Kind::Other || object.partBytes.isEmpty() || object.partBytes.size() > limits().objectBytes || !packageWithinLimits(object.partBytes)) return 0;
    for (const auto &kv : m_sessions)
        if (kv.second.owner == owner && kv.second.key == key) return kv.first;
    if (int(m_sessions.size()) >= kMaxOpen) return 0;
    retryFolders();
    QString root = m_root.isEmpty() ? QDir::tempPath() : m_root;
    if (!QDir().mkpath(root)) return 0;
    if (!m_swept.contains(root)) {
        m_swept << root;
        sweepStale(root);
    }
    QTemporaryDir made(QDir(root).filePath(QStringLiteral("jo-embed-XXXXXX")));
    if (!made.isValid()) return 0;
    made.setAutoRemove(false);
    Session s;
    s.dir = made.path();
    int id = g_nextId++;
    s.path = QDir(s.dir).filePath(QStringLiteral("Object %1.%2").arg(id).arg(extensionOf(kind)));
    {
        QFile f(s.path);
        if (!f.open(QIODevice::WriteOnly) || f.write(object.partBytes) != object.partBytes.size()) {
            f.close();
            removeFolder(s.dir);
            return 0;
        }
    }
    QFileInfo fi(s.path);
    s.deliveredSize = fi.size();
    s.deliveredTime = fi.lastModified().toMSecsSinceEpoch();
    s.hash = hashOf(object.partBytes);
    s.owner = owner;
    s.key = key;
    s.kind = kind;
    s.widthEmu = object.widthEmu;
    s.heightEmu = object.heightEmu;
    if (!m_open || !m_open(s.path, kind)) {
        removeFolder(s.dir);
        return 0;
    }
    connect(owner, &QObject::destroyed, this, [this](QObject *o) { closeFor(o); });
    m_sessions.emplace(id, std::move(s));
    m_timer.start();
    return id;
}

void EmbeddedEdit::erase(std::map<int, Session>::iterator it)
{
    QString dir = it->second.dir;
    m_sessions.erase(it);
    removeFolder(dir);
    if (m_sessions.empty()) m_timer.stop();
}

void EmbeddedEdit::close(int id)
{
    auto it = m_sessions.find(id);
    if (it != m_sessions.end()) erase(it);
}

void EmbeddedEdit::closeFor(QObject *owner)
{
    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        auto cur = it++;
        if (cur->second.owner == owner) erase(cur);
    }
}

void EmbeddedEdit::closeAll()
{
    while (!m_sessions.empty()) erase(m_sessions.begin());
    retryFolders();
    m_timer.stop();
}

void EmbeddedEdit::end(int id, EndReason reason)
{
    close(id);
    Q_EMIT watchEnded(id, reason);
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
    const qint64 size = fi.size(), time = fi.lastModified().toMSecsSinceEpoch(), cap = limits().objectBytes;
    if (size > cap) return end(id, EndReason::TooLarge);
    const bool statChanged = size != s.deliveredSize || time != s.deliveredTime;
    if (!statChanged && size > kHashEveryLook && s.pendingLooks == 0) return;
    QFile f(s.path);
    if (!f.open(QIODevice::ReadOnly)) {   // the other program may hold it for a moment
        if (++s.unreadable >= kUnreadableLooks) end(id, EndReason::Unreadable);
        return;
    }
    s.unreadable = 0;
    QByteArray bytes = f.read(cap + 1);
    f.close();
    if (bytes.size() > cap) return end(id, EndReason::TooLarge);
    const QByteArray h = hashOf(bytes);
    if (h == s.hash) {   // the same bytes saved again: nothing to hand back
        s.deliveredSize = size;
        s.deliveredTime = time;
        s.pendingLooks = 0;
        return;
    }
    if (s.pendingLooks > 0 && h == s.pendingHash && size == s.pendingSize && time == s.pendingTime) {
        ++s.pendingLooks;
    } else {
        s.pendingSize = size;
        s.pendingTime = time;
        s.pendingHash = h;
        s.pendingLooks = 1;
    }
    if (s.pendingLooks < 2) return;
    s.hash = h;
    s.deliveredSize = size;
    s.deliveredTime = time;
    s.pendingLooks = 0;
    const Kind kind = s.kind;
    auto saved = readSave(kind, bytes, s.widthEmu, s.heightEmu, m_draw[kind == Kind::Sheet ? 0 : 1]);
    if (!saved) {
        Q_EMIT saveRefused(id);
        return;
    }
    Q_EMIT objectSaved(id, saved->partBytes, saved->previewPng, saved->pictureStale);
}

} // namespace jp
