// Run from an installed copy (tools/check_packaged_layout.sh): finds the data folder the way the apps do, loads a Help topic, and loads a
// translation, and checks a spelling dictionary, the hyphenation patterns, and the thesaurus load from the compressed files of the data
// folder. Exit status 0 when all of it works; what failed is on stderr.
#include "app/help.h"
#include "app/i18n.h"
#include "app/proofingdialogs.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/datafolder.h"
#include "text/dictionaries.h"
#include "text/hyphenation.h"

#include <QCoreApplication>
#include <QFile>
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
    // The dictionaries: compressed files in the data folder's dict, and no plain ones beside them.
    const QString dictFolder = jp::dataFolder(QStringLiteral("dict"));
    if (dictFolder.isEmpty()) fail("no dict folder");
    if (jp::dict::root() != dictFolder) fail("the dictionaries are not read from the data folder");
    if (!QFile::exists(dictFolder + QStringLiteral("/en/en_US.dic.jz")) || QFile::exists(dictFolder + QStringLiteral("/en/en_US.dic"))) fail("the English dictionary is not the compressed file alone");
    for (const char *code : {"en-US", "de-DE", "uk-UA"}) {
        const QString c = QLatin1String(code);
        if (!jp::dict::hasSpelling(c)) fail(code);
    }
    if (!jp::dict::spell(QStringLiteral("en-US"), QStringLiteral("house")) || jp::dict::spell(QStringLiteral("en-US"), QStringLiteral("hosue"))) fail("English spelling does not answer");
    if (!jp::dict::spell(QStringLiteral("de-DE"), QString::fromUtf8("Häuser")) || jp::dict::spell(QStringLiteral("de-DE"), QStringLiteral("Hasuer"))) fail("German spelling does not answer");
    if (jp::hyphenationPoints(QStringLiteral("hyphenation"), QStringLiteral("en-US")).isEmpty()) fail("hyphenation patterns do not load");
    if (jp::thesaurusLookup(QStringLiteral("happy")).isEmpty()) fail("the thesaurus does not answer");
    std::printf("datacheck: dictionaries %s\n", qPrintable(dictFolder));
    std::printf("datacheck: %d topics, data folder %s\n", int(ids.size()), qPrintable(jp::dataFolder(QStringLiteral("help"))));
    return bad ? 1 : 0;
}
