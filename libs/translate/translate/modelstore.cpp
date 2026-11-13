#include "modelstore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace jp {
namespace translate {

QString ModelStore::defaultRoot() { return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/JeffOffice/Translation/models"); }

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
    bool ok = !QFileInfo::exists(folder(from, to)) || QDir(folder(from, to)).removeRecursively();
    // (and every half-made one of the pair, whoever made it)
    for (const QString &name : QDir(m_root).entryList({QStringLiteral(".part-") + from + QLatin1Char('-') + to + QStringLiteral("-*")}, QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot))
        ok = QDir(m_root + QLatin1Char('/') + name).removeRecursively() && ok;
    return ok;
}

void ModelStore::removeStaleParts() const
{
    const QDateTime old = QDateTime::currentDateTime().addDays(-1);
    for (const QFileInfo &info : QDir(m_root).entryInfoList({QStringLiteral(".part-*")}, QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot))
        if (info.lastModified() < old) QDir(info.absoluteFilePath()).removeRecursively();
}

bool ModelStore::commit(const ModelPair &pair, const QString &token) const
{
    const QString part = partFolder(pair.from, pair.to, token);
    for (const ModelFile &f : pair.files) {
        const QFileInfo info(part + QLatin1Char('/') + f.fileName);
        if (!info.isFile() || info.size() != f.size) return false;
    }
    // (the same files, put in place by another download: nothing to do)
    if (isInstalled(pair)) {
        QDir(part).removeRecursively();
        return true;
    }
    QFile rec(part + QStringLiteral("/installed.json"));
    if (!rec.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    rec.write("{\"complete\": true}\n");
    rec.close();
    QDir().mkpath(m_root);
    if (QFileInfo::exists(folder(pair.from, pair.to)) && !QDir(folder(pair.from, pair.to)).removeRecursively()) return false;
    if (QDir().rename(part, folder(pair.from, pair.to))) return true;
    // (another download renamed its folder in meanwhile: that one stands)
    QDir(part).removeRecursively();
    return isInstalled(pair);
}

} // namespace translate
} // namespace jp
