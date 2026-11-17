#include "core/datafolder.h"

#include "core/version.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QResource>

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
int s_searches = 0;
QStringList s_bundles;      // the .rcc files registered, to take out again when the search is made again
QStringList s_bundled;      // the subfolders ("help") that a registered bundle holds

// Help is one compressed Qt resource file (help.rcc) in the data folder, not thousands of small files; registered, it is the resource folder ":/help". A missing or corrupt file registers nothing.
const char *const kBundles[] = {"help"};

void unregisterBundles()
{
    for (const QString &file : s_bundles) QResource::unregisterResource(file);
    s_bundles.clear();
    s_bundled.clear();
}

// For writing Help (a developer's edit-and-reload loop): the folder named here is Help's folder, and not the bundle's.
QString helpOverride()
{
    const QString dir = QString::fromLocal8Bit(qgetenv("JO_HELP_DIR"));
    return !dir.isEmpty() && QFileInfo(dir).isDir() ? QDir::cleanPath(dir) : QString();
}

bool isDataFolder(const QString &root)
{
    QFile stamp(root + QStringLiteral("/data.stamp"));
    return stamp.open(QIODevice::ReadOnly) && QString::fromUtf8(stamp.readAll()).trimmed() == QLatin1String(versionString());
}

void find()   // (the mutex is held)
{
    if (s_found) return;
    s_found = true;
    ++s_searches;
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
            for (const char *name : kBundles) {
                const QString file = s_root + QLatin1Char('/') + QLatin1String(name) + QStringLiteral(".rcc");
                if (QFileInfo(file).isFile() && QResource::registerResource(file)) {
                    // (a damaged file can pass Qt's header check and hold nothing: it is let go at once, or Windows keeps it mapped and in use)
                    if (!QFileInfo(QStringLiteral(":/") + QLatin1String(name)).isDir()) {
                        QResource::unregisterResource(file);
                        continue;
                    }
                    s_bundles << file;
                    s_bundled << QLatin1String(name);
                }
            }
            QFile manifest(s_root + QStringLiteral("/manifest.txt"));
            if (manifest.open(QIODevice::ReadOnly))
                for (const QString &line : QString::fromUtf8(manifest.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts))
                    if (const QString path = line.trimmed(); !path.isEmpty()) s_manifest << path;   // (CMake writes CRLF on Windows)
            return;
        }
}
}

int dataFolderSearches()
{
    QMutexLocker lock(&mutex());
    find();
    return s_searches;
}

void setDataFolderOverride(const QString &root)
{
    QMutexLocker lock(&mutex());
    s_override = root;
    s_found = false;
    unregisterBundles();
    s_root.clear();
    s_manifest.clear();
}

QString dataFolder(const QString &subfolder)
{
    QMutexLocker lock(&mutex());
    find();
    if (s_root.isEmpty()) return QString();
    if (subfolder == QLatin1String("help"))
        if (const QString dir = helpOverride(); !dir.isEmpty()) return dir;
    if (s_bundled.contains(subfolder)) return QStringLiteral(":/") + subfolder;   // (a bundle's folder, and never a plain one beside it: an older version's copy does not win)
    if (subfolder == QLatin1String("help")) return QString();
    const QString dir = s_root + QLatin1Char('/') + subfolder;
    return QFileInfo(dir).isDir() ? dir : QString();
}

QStringList dataManifest(const QString &subfolder)
{
    QMutexLocker lock(&mutex());
    find();
    QStringList out;
    if (subfolder == QLatin1String("help") && !s_root.isEmpty())
        if (const QString dir = helpOverride(); !dir.isEmpty()) {
            for (QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories); it.hasNext();) out << QDir(dir).relativeFilePath(it.next());
            return out;
        }
    const QString prefix = subfolder + QLatin1Char('/');
    for (const QString &line : s_manifest)
        if (line.startsWith(prefix)) out << line.mid(prefix.size());
    return out;
}

} // namespace jp
