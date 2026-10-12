#include "app/pubinfo.h"

#include "app/settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace jp {

void moveFromJeffPub79()
{
    const QString marker = QStringLiteral("migration/jeffpub79");
    QSettings now(QStringLiteral("JeffOffice"), QStringLiteral("JeffPub"));
    if (now.value(marker).toBool()) return;
    {
        const QSettings old(QStringLiteral("JeffPub79"), QStringLiteral("JeffPub79"));
        copyMissingSettings(old, now);
    }
    // The old folder is where QStandardPaths put it under the old names.
    const QString org = QCoreApplication::organizationName(), app = QCoreApplication::applicationName();
    QCoreApplication::setOrganizationName(QStringLiteral("JeffPub"));
    QCoreApplication::setApplicationName(QStringLiteral("JeffPub 79"));
    const QString oldData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QCoreApplication::setOrganizationName(org);
    QCoreApplication::setApplicationName(app);
    const QString newData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!oldData.isEmpty() && !newData.isEmpty() && oldData != newData && moveDataFolder(oldData, newData))
        QDir().rmdir(QFileInfo(oldData).path());   // the old "JeffPub" folder, if now empty
    now.setValue(marker, true);
}

} // namespace jp
