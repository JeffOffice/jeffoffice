#include "languages.h"

#include <QCoreApplication>
#include <algorithm>

namespace jp {
namespace translate {

namespace {

struct Entry {
    const char *code;
    const char *name;
};

// (the names are the ones the old Translate dialog had, with the context their translations are filed under)
const Entry kLanguages[] = {
    {"en", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "English")}, {"es", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Spanish")}, {"fr", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "French")},
    {"de", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "German")}, {"it", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Italian")}, {"pt", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Portuguese")},
    {"nl", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Dutch")}, {"pl", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Polish")}, {"ru", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Russian")},
    {"uk", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Ukrainian")}, {"id", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Indonesian")}, {"ar", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Arabic")},
    {"hi", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Hindi")}, {"zh", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Chinese (Simplified)")}, {"ja", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Japanese")},
    {"ko", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Korean")}, {"vi", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Vietnamese")}, {"tr", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Turkish")},
    {"el", QT_TRANSLATE_NOOP("jp::write::HomeDialogs", "Greek")}};

} // namespace

QString languageName(const QString &code)
{
    for (const Entry &e : kLanguages)
        if (code == QLatin1String(e.code)) return QCoreApplication::translate("jp::write::HomeDialogs", e.name);
    return code;
}

QVector<Language> languages()
{
    QVector<Language> out;
    for (const Entry &e : kLanguages) out.push_back({QString::fromLatin1(e.code), languageName(QString::fromLatin1(e.code))});
    std::stable_sort(out.begin() + 1, out.end(), [](const Language &a, const Language &b) { return a.name.localeAwareCompare(b.name) < 0; });
    return out;
}

QString languageOfTag(const QString &tag)
{
    const QString lower = tag.toLower();
    const QString first = lower.section(QLatin1Char('-'), 0, 0).section(QLatin1Char('_'), 0, 0);
    for (const Entry &e : kLanguages)
        if (first == QLatin1String(e.code)) return first;
    return QString();
}

QString websiteCode(const QString &code) { return code == QLatin1String("zh") ? QStringLiteral("zh-Hans") : code; }

} // namespace translate
} // namespace jp
