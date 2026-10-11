#pragma once
// AutoRecover: copies of unsaved work, kept per run of JeffPub in
// AutoRecover/<session>/ under the app's data folder, with a lock file held
// while that run lives. A run that ends normally removes its folder; one
// that crashes leaves it, and the next run offers its copies back.

#include <QDateTime>
#include <QString>
#include <QVector>
#include <functional>

namespace jp {
namespace recovery {

// The file ending of the app's copies (".jpub" for JeffPub's documents);
// the app sets it before using anything below.
void setCopySuffix(const QString &suffix);
QString copySuffix();

// Where every run keeps its folder (tests point it elsewhere).
QString root();
void setRoot(const QString &dir);

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
