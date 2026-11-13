// Developer tool: the time to load each spelling dictionary and the thesaurus at first use, and the memory the process holds after each
// (resident set, Linux only). Prints one line per language, then the thesaurus. Usage: jo_dictbench [lang ...]  (default: every language)
// jo_dictbench --golden <plain dictionary folder> prints tests/data/dict_golden.txt (see tests/pub/dictgolden.h).
#include "core/appinfo.h"
#include "../tests/pub/dictgolden.h"
#include "app/proofingdialogs.h"
#include "text/dictionaries.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <cstdio>

static long residentKb()
{
    QFile f(QStringLiteral("/proc/self/statm"));
    if (!f.open(QIODevice::ReadOnly)) return -1;
    return f.readAll().split(' ').value(1).toLong() * 4;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    jp::AppInfo info;
    info.name = QStringLiteral("JeffPub");
    info.fileName = QStringLiteral("jeffpub");
    info.organization = QStringLiteral("JeffOffice");
    jp::setAppInfo(info);
    if (argc == 3 && QByteArray(argv[1]) == "--golden") {
        for (const jp::dictgolden::Source &src : jp::dictgolden::kSources) std::printf("%s\n", qPrintable(jp::dictgolden::digest(QString::fromLocal8Bit(argv[2]), src)));
        std::printf("%s\n", qPrintable(jp::dictgolden::thesaurusDigest(QString::fromLocal8Bit(argv[2]))));
        return 0;
    }
    QStringList codes;
    for (int i = 1; i < argc; ++i) codes << QString::fromLocal8Bit(argv[i]);
    const bool all = codes.isEmpty();
    if (all) for (const jp::dict::Language &l : jp::dict::languages()) codes << l.code;
    std::printf("dictionary root: %s\n", qPrintable(jp::dict::root()));
    const long base = residentKb();
    std::printf("start rss %ld KB\n", base);
    for (const QString &c : codes) {
        if (c == QLatin1String("thesaurus")) continue;
        QElapsedTimer t;
        t.start();
        const bool ok = jp::dict::hasSpelling(c) && jp::dict::spell(c, QStringLiteral("a"));
        const qint64 ms = t.elapsed();
        const long before = residentKb();
        t.restart();
        jp::dict::spell(c, QStringLiteral("zzqxv"));
        jp::dict::suggest(c, QStringLiteral("zzqxv"));
        std::printf("spell %-6s load %5lld ms  first suggest %4lld ms  rss %ld KB (+%ld)\n", qPrintable(c), ms, t.elapsed(), residentKb(), residentKb() - base);
        (void)ok; (void)before;
    }
    if (all || codes.contains(QStringLiteral("thesaurus"))) {
        const long before = residentKb();
        QElapsedTimer t;
        t.start();
        const int n = int(jp::thesaurusLookup(QStringLiteral("happy")).size());
        const qint64 first = t.elapsed();
        t.restart();
        for (const char *w : {"running", "houses", "quickly", "bigger", "walk", "dog", "table", "angry"}) jp::thesaurusLookup(QString::fromLatin1(w));
        std::printf("thesaurus first lookup %lld ms (%d results), 8 more %lld ms, rss %ld KB (+%ld)\n", first, n, t.elapsed(), residentKb(), residentKb() - before);
    }
    return 0;
}
