#include "fields/fieldhost.h"

namespace jp::fields {

Env makeEnv(const FieldHost &host)
{
    Env env;
    env.now = host.now();
    env.properties = host.properties();
    env.page = host.pageNumber();
    env.pages = host.pageCount();
    env.sectionPages = host.sectionPageCount();
    env.section = host.section();
    env.pageText = host.pageText();
    env.locale = host.locale();
    env.sequences = host.sequences();
    env.bookmark = [&host](const QString &name) { return host.bookmark(name); };
    return env;
}

std::optional<QString> evaluate(const FieldHost &host, const QString &instruction)
{
    Env env = makeEnv(host);
    return compute(parseCode(instruction), env);
}

std::optional<QString> dateFromQtFormat(const QDateTime &when, const QString &format, const QLocale &locale)
{
    if (format.size() > kMostPictureChars) return std::nullopt;
    // Qt shows hours on a 12-hour clock only where the format has AP or ap.
    bool twelve = false, quoted = false;
    for (int i = 0; i < format.size(); ++i) {
        if (format[i] == QLatin1Char('\'')) quoted = !quoted;
        else if (!quoted && (format.mid(i, 2) == QLatin1String("AP") || format.mid(i, 2) == QLatin1String("ap"))) twelve = true;
    }
    QString picture;
    quoted = false;
    for (int i = 0; i < format.size(); ++i) {
        const QChar c = format[i];
        if (c == QLatin1Char('\'')) {
            if (i + 1 < format.size() && format[i + 1] == QLatin1Char('\'')) return std::nullopt;   // (a quote mark in the text)
            quoted = !quoted;
            picture += c;
        } else if (quoted) {
            picture += c;
        } else if (format.mid(i, 2) == QLatin1String("AP") || format.mid(i, 2) == QLatin1String("ap")) {
            // (Qt shows a language's own words for the day's halves; the picture has English ones only.)
            if (locale.language() != QLocale::English && locale.language() != QLocale::C) return std::nullopt;
            picture += c == QLatin1Char('A') ? QStringLiteral("AM/PM") : QStringLiteral("am/pm");
            ++i;
        } else if (c == QLatin1Char('h') || c == QLatin1Char('H')) {
            picture += twelve ? QLatin1Char('h') : QLatin1Char('H');
        } else if (c == QLatin1Char('d') || c == QLatin1Char('M') || c == QLatin1Char('y') || c == QLatin1Char('m') || c == QLatin1Char('s')) {
            picture += c;
        } else if (c.isLetter() || c == QLatin1Char('\\')) {
            return std::nullopt;
        } else {
            picture += c;
        }
    }
    if (quoted) return std::nullopt;
    return dateInPicture(when, picture, locale);
}

} // namespace jp::fields
