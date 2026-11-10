#pragma once
// Automatic recovery: copies of unsaved work, kept per run of JeffPub in
// Recovery/<session>/ under the app's data folder, with a lock file held
// while that run lives. A run that ends normally removes its folder; one
// that crashes leaves it, and the next run offers its copies back.

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QVector>
#include <functional>
#include <memory>

class QObject;

namespace jp {
namespace recovery {

// The file ending of the app's copies (".jpub" for JeffPub's documents);
// the app sets it before using anything below.
void setCopySuffix(const QString &suffix);
QString copySuffix();

// Where every run keeps its folder (tests point it elsewhere).
QString root();
void setRoot(const QString &dir);
// The folder older versions kept their copies in, by default the old-named folder beside the root (recovery.cpp) (none after setRoot): orphans() moves what is there into the root, and still reads what it
// could not move.
void setLegacyRoot(const QString &dir);

// This run's folder, created and locked on first use.
QString sessionDir();

// The copy a window keeps of its document: the document's name and a
// short hash of its full path (or, unsaved, of the window), so two
// "Cover.pub" files in different folders keep separate copies.
QString copyPath(const QString &docPath, const QString &displayName, quint64 windowSerial);

// Writes the copy with `save` (the app's own way of writing its document to
// a file) and, beside it, where the work came from.
bool write(const std::function<bool(const QString &copy, QString *error)> &save, const QString &copy, const QString &docPath, const QString &title,
           QString *error = nullptr);
void remove(const QString &copy);

// One copy at a time made off the window's thread: the caller takes what it needs from its document where nothing can change meanwhile and gives `make` that, `make` (on a worker thread) turns it into
// the file's bytes, and the bytes are written to the copy (and the note beside it) there. The copy appears whole or not at all (written beside it and renamed). `finished` is called on the window's
// thread. cancel() (a save, a close) is safe at any time: a copy not yet written is never written, and one that was is removed, so no copy is left behind by a job that was cancelled.
class AsyncWriter {
public:
    using Make = std::function<QByteArray(QString *error)>;
    using Finished = std::function<void(bool ok, const QString &copy, const QString &error)>;
    // `window` is the object `finished` is called through (its thread).
    explicit AsyncWriter(QObject *window);
    ~AsyncWriter();
    AsyncWriter(const AsyncWriter &) = delete;
    AsyncWriter &operator=(const AsyncWriter &) = delete;

    // A job is begun and its end not yet reported (or cancelled).
    bool busy() const { return bool(m_job); }
    // False, doing nothing, while a job is running.
    bool start(Make make, const QString &copy, const QString &docPath, const QString &title, Finished finished);
    // The same on this thread, done when it returns (a copy a caller needs at once).
    bool writeNow(const Make &make, const QString &copy, const QString &docPath, const QString &title, QString *error);
    void cancel();
    // Waits for every worker to end (the program is leaving).
    static void waitForAll();

private:
    struct Job;
    QObject *m_window;
    std::shared_ptr<Job> m_job;
};

struct Recovered {
    QString file;       // the copy (ending in copySuffix())
    QString source;     // the publication it was made from (empty if never saved)
    QString title;
    QDateTime saved;
};
// Copies left by runs that didn't end normally (their lock is free), and
// copies from versions that kept them loose in the folder, newest first.
QVector<Recovered> orphans();
// Deletes a recovered copy and its folder once that is empty.
void discard(const Recovered &r);

// A normal exit: this run's folder goes.
void endSession();

} // namespace recovery
} // namespace jp
