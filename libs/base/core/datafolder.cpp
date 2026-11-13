#include "core/datafolder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>

namespace jp {

namespace {
QMutex &mutex()
{
    static QMutex m;
    return m;
}
QString s_override;
}

void setDataFolderOverride(const QString &root)
{
    QMutexLocker lock(&mutex());
    s_override = root;
}

QString dataFolder(const QString &subfolder)
{
    QStringList roots;
    {
        QMutexLocker lock(&mutex());
        if (!s_override.isEmpty()) roots << s_override;
    }
    if (!roots.isEmpty()) {
        const QString dir = roots.first() + QLatin1Char('/') + subfolder;
        return QFileInfo(dir).isDir() ? QDir::cleanPath(dir) : QString();
    }
    const QString app = QCoreApplication::applicationDirPath();
    roots << app << app + QStringLiteral("/../share/jeffoffice") << app + QStringLiteral("/../Resources");
#ifdef JP_SOURCE_DIR
    roots << QStringLiteral(JP_SOURCE_DIR "/resources");   // (help's topics are here as they are)
#endif
#ifdef JO_BUILD_DATA_DIR
    roots << QStringLiteral(JO_BUILD_DATA_DIR);   // (the translations, made by the build)
#endif
    for (const QString &root : roots) {
        const QString dir = root + QLatin1Char('/') + subfolder;
        if (QFileInfo(dir).isDir()) return QDir::cleanPath(dir);
    }
    return QString();
}

} // namespace jp
