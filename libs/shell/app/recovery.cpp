#include "app/recovery.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>
#include <algorithm>
#include <QCoreApplication>
#include <QMetaObject>
#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <atomic>
#include <memory>

namespace jp::recovery {

namespace {
// Main thread only, like the windows that keep copies.
QString g_root;
QString g_legacy;      // the folder older versions kept their copies in (read once more, and emptied into the root)
QString g_session;
std::unique_ptr<QLockFile> g_lock;

const char *kLockName = "session.lock";

// Characters some file systems refuse, and leading dots, become "_".
QString safeName(QString s)
{
    for (QChar &c : s)
        if (c < QChar(0x20) || QStringLiteral("<>:\"/\\|?*").contains(c)) c = QLatin1Char('_');
    s = s.trimmed();
    while (s.startsWith(QLatin1Char('.'))) s[0] = QLatin1Char('_');
    if (s.isEmpty()) s = QStringLiteral("Publication");
    if (s.size() > 60) {
        s.truncate(60);
        if (s.back().isHighSurrogate()) s.chop(1);   // not half a character
    }
    return s;
}

QString s_suffix = QStringLiteral(".jpub");

QString sidecar(const QString &copy) { return copy.left(copy.size() - s_suffix.size()) + QStringLiteral(".json"); }

Recovered describe(const QString &file)
{
    Recovered r;
    r.file = file;
    const QFileInfo fi(file);
    r.saved = fi.lastModified();
    QFile meta(sidecar(file));
    if (meta.open(QIODevice::ReadOnly)) {
        const QJsonObject o = QJsonDocument::fromJson(meta.readAll()).object();
        r.source = o.value(QStringLiteral("source")).toString();
        r.title = o.value(QStringLiteral("title")).toString();
        const QDateTime t = QDateTime::fromString(o.value(QStringLiteral("saved")).toString(), Qt::ISODate);
        if (t.isValid()) r.saved = t;
    }
    if (r.title.isEmpty()) {
        // "Cover.autorecover.jpub" (older versions), "Cover 1a2b3c4d.jpub".
        QString t = fi.completeBaseName();
        if (t.endsWith(QLatin1String(".autorecover"))) t.chop(12);
        else if (t.size() > 9 && t.at(t.size() - 9) == QLatin1Char(' ')) t.chop(9);
        r.title = t;
    }
    return r;
}
} // namespace

void setCopySuffix(const QString &suffix) { s_suffix = suffix; }
QString copySuffix() { return s_suffix; }

QString root()
{
    if (g_root.isEmpty()) {
        const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        g_root = base + QStringLiteral("/Recovery");
        g_legacy = base + QStringLiteral("/AutoRecover");   // (the name older versions gave the folder: kept so that their copies are found)
    }
    return g_root;
}

void setRoot(const QString &dir)
{
    endSession();
    g_root = dir;
    g_legacy.clear();
}

void setLegacyRoot(const QString &dir) { g_legacy = dir; }

QString sessionDir()
{
    if (!g_session.isEmpty()) return g_session;
    const QString dir = root() + QLatin1Char('/') + QUuid::createUuid().toString(QUuid::Id128).left(12);
    QDir().mkpath(dir);
    auto lock = std::make_unique<QLockFile>(dir + QLatin1Char('/') + QLatin1String(kLockName));
    lock->setStaleLockTime(0);   // held for the whole run; only a dead run's lock is stale
    if (!lock->tryLock(1000)) {   // (another run's scan of the folder may hold the lock for an instant)
        lock.reset();
        QDir().rmdir(dir);   // (not left behind empty on every try; a folder with anything in it stays)
        return QString();
    }
    g_lock = std::move(lock);
    g_session = dir;
    return g_session;
}

QString copyPath(const QString &docPath, const QString &displayName, quint64 windowSerial)
{
    const QString dir = sessionDir();
    if (dir.isEmpty()) return QString();
    const QString key = docPath.isEmpty() ? QStringLiteral("window:%1").arg(windowSerial) : QFileInfo(docPath).absoluteFilePath();
    const QByteArray h = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex().left(8);
    return dir + QLatin1Char('/') + safeName(displayName) + QLatin1Char(' ') + QString::fromLatin1(h) + s_suffix;
}

bool write(const std::function<bool(const QString &copy, QString *error)> &save, const QString &copy, const QString &docPath, const QString &title,
           QString *error)
{
    if (copy.isEmpty()) return false;
    if (!save(copy, error)) return false;
    QSaveFile meta(sidecar(copy));
    if (!meta.open(QIODevice::WriteOnly)) return true;   // the copy alone still recovers
    const QJsonObject o{{QStringLiteral("source"), docPath}, {QStringLiteral("title"), title},
                        {QStringLiteral("saved"), QDateTime::currentDateTime().toString(Qt::ISODate)}};
    meta.write(QJsonDocument(o).toJson(QJsonDocument::Compact));
    meta.commit();
    return true;
}

void remove(const QString &copy)
{
    if (copy.isEmpty()) return;
    QFile::remove(copy);
    QFile::remove(sidecar(copy));
}

namespace {
QThreadPool &workers()
{
    static QThreadPool *pool = [] {
        auto *p = new QThreadPool;   // (one worker: copies of several windows are made one after another)
        p->setMaxThreadCount(1);
        return p;
    }();
    return *pool;
}

bool writeBytes(const QByteArray &bytes, const QString &copy, const QString &docPath, const QString &title, QString *error)
{
    return write(
        [&](const QString &file, QString *err) {
            QSaveFile out(file);
            if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size() || !out.commit()) {
                if (err) *err = out.errorString();
                return false;
            }
            return true;
        },
        copy, docPath, title, error);
}
} // namespace

struct AsyncWriter::Job {
    std::atomic<bool> cancelled{false}, wrote{false};
    QString copy;
};

namespace {
std::atomic<bool> g_leaving{false};   // (the program is leaving: every job stops)
}

AsyncWriter::AsyncWriter(QObject *window) : m_window(window) {}

AsyncWriter::~AsyncWriter() { cancel(); }

bool AsyncWriter::start(Make make, const QString &copy, const QString &docPath, const QString &title, Finished finished)
{
    if (m_job || copy.isEmpty()) return false;
    auto job = std::make_shared<Job>();
    job->copy = copy;
    m_job = job;
    QObject *window = m_window ? m_window : QCoreApplication::instance();
    // (The result is posted to an object that outlives every window; the post itself finds out whether the window cancelled meanwhile.)
    workers().start(QRunnable::create([this, job, make = std::move(make), copy, docPath, title, finished = std::move(finished), window] {
        auto cancelled = [&job] { return job->cancelled.load() || g_leaving.load(); };
        if (cancelled()) return;
        QString error;
        const QByteArray bytes = make(&error, cancelled);
        if (cancelled()) return;   // (nobody is waiting for it)
        const bool ok = !bytes.isEmpty() && writeBytes(bytes, copy, docPath, title, &error);
        job->wrote = ok;
        // Cancelled while it was being written: the copy is taken away here (a cancel that comes after this check sees `wrote`, and does it).
        if (ok && cancelled()) {
            remove(copy);
            return;
        }
        if (!window) return;
        QMetaObject::invokeMethod(window, [this, job, ok, copy, error, finished] {
            if (job->cancelled) return;   // (the writer may be gone then)
            m_job.reset();
            finished(ok, copy, error);
        }, Qt::QueuedConnection);
    }));
    return true;
}

bool AsyncWriter::writeNow(const Make &make, const QString &copy, const QString &docPath, const QString &title, QString *error)
{
    cancel();
    const QByteArray bytes = make(error, [] { return false; });
    return !bytes.isEmpty() && writeBytes(bytes, copy, docPath, title, error);
}

// Never waits: the job is marked, and what it has written by now is removed (it removes what it writes after this itself).
void AsyncWriter::cancel()
{
    if (!m_job) return;
    m_job->cancelled = true;
    if (m_job->wrote) remove(m_job->copy);
    m_job.reset();
}

void AsyncWriter::waitForAll() { workers().waitForDone(); }

void AsyncWriter::stopAll()
{
    g_leaving = true;   // (what is running stops at its next step, and what is waiting does not begin)
    workers().waitForDone();
    g_leaving = false;
}

namespace {

// Moves what an older version kept in its folder into the root: loose copies, and the folders of runs that have ended (a run still going keeps its own). What cannot be moved stays,
// and is still read where it is.
void adoptLegacy()
{
    const QString legacy = g_legacy;
    if (legacy.isEmpty() || QDir::cleanPath(legacy) == QDir::cleanPath(root()) || !QDir(legacy).exists()) return;
    QDir().mkpath(root());
    const QDir from(legacy), to(root());
    for (const QFileInfo &f : from.entryInfoList(QDir::Files)) {
        if (!f.fileName().endsWith(s_suffix) && !f.fileName().endsWith(QLatin1String(".json"))) continue;
        if (!to.exists(f.fileName())) QFile::rename(f.absoluteFilePath(), to.filePath(f.fileName()));
    }
    for (const QFileInfo &d : from.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QLockFile probe(d.absoluteFilePath() + QLatin1Char('/') + QLatin1String(kLockName));
        probe.setStaleLockTime(0);
        if (!probe.tryLock(0)) continue;
        probe.unlock();
        if (!to.exists(d.fileName())) QDir().rename(d.absoluteFilePath(), to.filePath(d.fileName()));
    }
    QDir().rmdir(legacy);   // (only if it is empty now)
}

// The copies in one folder of runs: loose ones, and those of runs that ended without removing their folder.
void scanRoot(const QString &rootDir, QVector<Recovered> *out)
{
    const QDir top(rootDir);
    if (!top.exists()) return;
    // Copies older versions kept loose in the folder.
    for (const QFileInfo &f : top.entryInfoList({QLatin1Char('*') + s_suffix}, QDir::Files)) *out << describe(f.absoluteFilePath());
    for (const QFileInfo &d : top.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString dir = d.absoluteFilePath();
        if (dir == g_session) continue;
        // A run that's still going holds its lock; a crashed run's lock is
        // stale (its process is gone) and can be taken.
        QLockFile probe(dir + QLatin1Char('/') + QLatin1String(kLockName));
        probe.setStaleLockTime(0);
        if (!probe.tryLock(0)) continue;
        probe.unlock();
        const QFileInfoList copies = QDir(dir).entryInfoList({QLatin1Char('*') + s_suffix}, QDir::Files);
        // (nothing left to offer; but a folder made a moment ago may be one a run is about to lock)
        if (copies.isEmpty() && d.lastModified().secsTo(QDateTime::currentDateTime()) > 60) QDir(dir).removeRecursively();
        for (const QFileInfo &f : copies) *out << describe(f.absoluteFilePath());
    }
}

} // namespace

QVector<Recovered> orphans()
{
    QVector<Recovered> out;
    root();
    adoptLegacy();
    scanRoot(root(), &out);
    if (!g_legacy.isEmpty() && QDir::cleanPath(g_legacy) != QDir::cleanPath(root())) scanRoot(g_legacy, &out);   // (what could not be moved)
    std::sort(out.begin(), out.end(), [](const Recovered &a, const Recovered &b) { return a.saved > b.saved; });
    return out;
}

void discard(const Recovered &r)
{
    remove(r.file);
    const QFileInfo fi(r.file);
    QDir dir = fi.absoluteDir();
    if (QDir::cleanPath(dir.absolutePath()) == QDir::cleanPath(root())) return;
    if (!g_legacy.isEmpty() && QDir::cleanPath(dir.absolutePath()) == QDir::cleanPath(g_legacy)) return;
    if (dir.entryInfoList({QLatin1Char('*') + s_suffix}, QDir::Files).isEmpty()) dir.removeRecursively();
}

void endSession()
{
    AsyncWriter::stopAll();
    if (g_session.isEmpty()) return;
    g_lock.reset();   // unlocks
    QDir(g_session).removeRecursively();
    g_session.clear();
}

} // namespace jp::recovery
