// Run from an installed copy (tools/check_packaged_layout.sh): finds the data folder the way the apps do, loads a Help topic, and loads a
// translation. Exit status 0 when all three work; what failed is on stderr.
#include "app/help.h"
#include "app/i18n.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/datafolder.h"

#include <QCoreApplication>
#include <cstdio>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    jp::AppInfo info;
    info.name = QStringLiteral("JeffPub");
    info.fileName = QStringLiteral("jeffpub");
    info.organization = QStringLiteral("JeffOffice");
    jp::setAppInfo(info);
    int bad = 0;
    auto fail = [&bad](const char *what) { std::fprintf(stderr, "datacheck: %s\n", what); ++bad; };
    if (jp::dataFolder(QStringLiteral("help")).isEmpty()) fail("no help folder");
    if (jp::dataFolder(QStringLiteral("i18n")).isEmpty()) fail("no i18n folder");
    const QStringList ids = jp::help::topicIds();
    if (ids.size() < 7) fail("Help lists too few topics");
    if (ids.isEmpty() || jp::help::topic(ids.first()).title.isEmpty()) fail("a Help topic does not load");
    if (!jp::availableUiLanguages().contains(QStringLiteral("de"))) fail("German is not listed");
    jp::Settings::get().setValue(QStringLiteral("ui/language"), QStringLiteral("de"));
    if (jp::installTranslations() != QLatin1String("de")) fail("German did not install");
    if (QCoreApplication::translate("QPlatformTheme", "Cancel") == QLatin1String("Cancel")) fail("Qt's own German text did not load");
    std::printf("datacheck: %d topics, data folder %s\n", int(ids.size()), qPrintable(jp::dataFolder(QStringLiteral("help"))));
    return bad ? 1 : 0;
}
