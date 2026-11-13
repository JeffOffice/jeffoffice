#pragma once
// The models on this computer: one folder for each language pair under the program's data folder (Translation/models/<from>-<to>), with the pair's files and a file
// "installed.json" that is written last, when every file has been checked. A pair is there when that file is, and every file in it is the size it names.

#include "catalog.h"

#include <QString>
#include <QStringList>

namespace jp {
namespace translate {

class ModelStore {
public:
    explicit ModelStore(const QString &root) : m_root(root) {}
    // Translation/models in the program's data folder.
    static QString defaultRoot();

    QString root() const { return m_root; }
    QString folder(const QString &from, const QString &to) const { return m_root + QLatin1Char('/') + from + QLatin1Char('-') + to; }
    // The folder where a pair is put together before it is complete (nothing in it is used).
    QString partFolder(const QString &from, const QString &to) const { return m_root + QStringLiteral("/.part-") + from + QLatin1Char('-') + to; }
    bool isInstalled(const ModelPair &pair) const;
    // The path of one of a pair's files (the pair has to be installed).
    QString filePath(const ModelPair &pair, const QString &role) const;
    // The pairs that are not installed, of those the route uses.
    QVector<ModelPair> missing(const QVector<ModelPair> &route) const;
    // Removes a pair (and any half-made one); whether it is gone.
    bool remove(const QString &from, const QString &to) const;
    // Marks a pair done: its folder is made from `partFolder` by renaming, with the record written. False when the files are not all there at their sizes.
    bool commit(const ModelPair &pair) const;

private:
    QString m_root;
};

} // namespace translate
} // namespace jp
