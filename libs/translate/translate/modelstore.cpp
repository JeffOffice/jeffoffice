#include "modelstore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace jp {
namespace translate {

QString ModelStore::defaultRoot() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/Translation/models"); }

bool ModelStore::isInstalled(const ModelPair &pair) const
{
    const QString dir = folder(pair.from, pair.to);
    if (!QFileInfo::exists(dir + QStringLiteral("/installed.json"))) return false;
    for (const ModelFile &f : pair.files) {
        const QFileInfo info(dir + QLatin1Char('/') + f.fileName);
        if (!info.isFile() || info.size() != f.size) return false;
    }
    return true;
}

QString ModelStore::filePath(const ModelPair &pair, const QString &role) const
{
    for (const ModelFile &f : pair.files)
        if (f.role == role) return folder(pair.from, pair.to) + QLatin1Char('/') + f.fileName;
    return QString();
}

QVector<ModelPair> ModelStore::missing(const QVector<ModelPair> &route) const
{
    QVector<ModelPair> out;
    for (const ModelPair &p : route)
        if (!isInstalled(p)) out.push_back(p);
    return out;
}

bool ModelStore::remove(const QString &from, const QString &to) const
{
    const bool a = !QFileInfo::exists(folder(from, to)) || QDir(folder(from, to)).removeRecursively();
    const bool b = !QFileInfo::exists(partFolder(from, to)) || QDir(partFolder(from, to)).removeRecursively();
    return a && b;
}

bool ModelStore::commit(const ModelPair &pair) const
{
    const QString part = partFolder(pair.from, pair.to);
    for (const ModelFile &f : pair.files) {
        const QFileInfo info(part + QLatin1Char('/') + f.fileName);
        if (!info.isFile() || info.size() != f.size) return false;
    }
    QFile rec(part + QStringLiteral("/installed.json"));
    if (!rec.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    rec.write("{\"complete\": true}\n");
    rec.close();
    QDir().mkpath(m_root);
    if (QFileInfo::exists(folder(pair.from, pair.to)) && !QDir(folder(pair.from, pair.to)).removeRecursively()) return false;
    return QDir().rename(part, folder(pair.from, pair.to));
}

} // namespace translate
} // namespace jp
