#pragma once
// The models on this computer: one folder for each language pair in the folder every JeffOffice app shares (the user's data folder, JeffOffice/Translation/models/<from>-<to>), so one download serves them all, with the pair's files and a file
// "installed.json" that is written last, when every file has been checked. A pair is there when that file is, and every file in it is the size it names.

#include "catalog.h"

#include <QDir>
#include <QString>
#include <QStringList>

namespace jp {
namespace translate {

class ModelStore {
public:
    explicit ModelStore(const QString &root) : m_root(root) {}
    // JeffOffice/Translation/models in the user's data folder (not the program's own: every app uses the one).
    static QString defaultRoot();

    QString root() const { return m_root; }
    QString folder(const QString &from, const QString &to) const { return m_root + QLatin1Char('/') + from + QLatin1Char('-') + to; }
    // The folder where a pair is put together before it is complete (nothing in it is used); `token` is the downloader's own (a number of the program, and one of the download), so two\n    // downloads of one pair, in one program or two, never touch each other's files.
    QString partFolder(const QString &from, const QString &to, const QString &token) const { return m_root + QStringLiteral("/.part-") + from + QLatin1Char('-') + to + QLatin1Char('-') + token; }
    // Takes away the half-made folders that are a day old or more (a download that was cut off by the program or the computer ending).
    void removeStaleParts() const;
    bool isInstalled(const ModelPair &pair) const;
    // The path of one of a pair's files (the pair has to be installed).
    QString filePath(const ModelPair &pair, const QString &role) const;
    // The pairs that are not installed, of those the route uses.
    QVector<ModelPair> missing(const QVector<ModelPair> &route) const;
    // Removes a pair (and any half-made one); whether it is gone.
    bool remove(const QString &from, const QString &to) const;
    // Takes away one downloader's half-made folder.
    void removePart(const QString &from, const QString &to, const QString &token) const { QDir(partFolder(from, to, token)).removeRecursively(); }
    // Marks a pair done: its folder is made from the part folder by renaming, with the record written. False when the files are not all there at their sizes. A pair that is installed
    // already (another download got there first) is left as it is, and the part folder is thrown away.
    bool commit(const ModelPair &pair, const QString &token) const;

private:
    QString m_root;
};

} // namespace translate
} // namespace jp
