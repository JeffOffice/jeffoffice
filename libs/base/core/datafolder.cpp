#include "core/datafolder.h"

#include "core/version.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

namespace jp {

namespace {
QMutex &mutex()
{
    static QMutex m;
    return m;
}
QString s_override;
bool s_found = false;   // the search is made once
QString s_root;
QStringList s_manifest;

bool isDataFolder(const QString &root)
{
    QFile stamp(root + QStringLiteral("/data.stamp"));
    return stamp.open(QIODevice::ReadOnly) && QString::fromUtf8(stamp.readAll()).trimmed() == QLatin1String(versionString());
}

void find()   // (the mutex is held)
{
    if (s_found) return;
    s_found = true;
    QStringList roots;
    if (!s_override.isEmpty()) roots << s_override;
    else {
        const QString app = QCoreApplication::applicationDirPath();
        roots << app << app + QStringLiteral("/data") << app + QStringLiteral("/../share/jeffoffice") << app + QStringLiteral("/../Resources");
#ifdef JO_BUILD_DATA_DIR
        roots << QStringLiteral(JO_BUILD_DATA_DIR);   // (a development build: its own build folder)
#endif
    }
    for (const QString &root : roots)
        if (isDataFolder(root)) {
            s_root = QDir::cleanPath(root);
            QFile manifest(s_root + QStringLiteral("/manifest.txt"));
            if (manifest.open(QIODevice::ReadOnly))
                s_manifest = QString::fromUtf8(manifest.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            return;
        }
}
}

void setDataFolderOverride(const QString &root)
{
    QMutexLocker lock(&mutex());
    s_override = root;
    s_found = false;
    s_root.clear();
    s_manifest.clear();
}

QString dataFolder(const QString &subfolder)
{
    QMutexLocker lock(&mutex());
    find();
    if (s_root.isEmpty()) return QString();
    const QString dir = s_root + QLatin1Char('/') + subfolder;
    return QFileInfo(dir).isDir() ? dir : QString();
}

QStringList dataManifest(const QString &subfolder)
{
    QMutexLocker lock(&mutex());
    find();
    QStringList out;
    const QString prefix = subfolder + QLatin1Char('/');
    for (const QString &line : s_manifest)
        if (line.startsWith(prefix)) out << line.mid(prefix.size());
    return out;
}

} // namespace jp
