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

std::optional<QString> evaluate(const FieldHost &host, const QString &instruction, Work *work)
{
    Env env = makeEnv(host);
    qint64 formulaSteps = kMostFormulaSteps;
    env.formulaBudget = &formulaSteps;
    const auto result = compute(parseCode(instruction), env);
    if (work) work->steps += 1 + instruction.size() + (kMostFormulaSteps - formulaSteps) + env.globSteps;
    return result;
}

std::optional<QString> dateFromQtFormat(const QDateTime &when, const QString &format, const QLocale &locale)
{
    // The engine answers only where its text is the text Qt gives by construction: the digits are 0-9, the year has four digits, and every letter run is one the picture works out
    // as Qt does (a letter or a run length it has no equal for is left to the caller's old way).
    if (!when.isValid() || when.date().year() < 1000 || when.date().year() > 9999) return std::nullopt;
    if (format.size() > kMostPictureChars || locale.zeroDigit() != QLatin1Char('0')) return std::nullopt;
    // Qt shows hours on a 12-hour clock only where the format has AP or ap.
    bool twelve = false, quoted = false;
    for (int i = 0; i < format.size(); ++i) {
        if (format[i] == QLatin1Char('\'')) quoted = !quoted;
        else if (!quoted && (format.mid(i, 2) == QLatin1String("AP") || format.mid(i, 2) == QLatin1String("ap"))) twelve = true;
    }
    // (Qt shows a language's own words for the day's halves; the picture has English ones only.)
    if (twelve && (locale.amText() != QLatin1String("AM") || locale.pmText() != QLatin1String("PM"))) return std::nullopt;
    QString picture;
    quoted = false;
    for (int i = 0; i < format.size(); ++i) {
        const QChar c = format[i];
        int run = 1;
        while (i + run < format.size() && format[i + run] == c) ++run;
        if (c == QLatin1Char('\'')) {
            if (i + 1 < format.size() && format[i + 1] == QLatin1Char('\'')) return std::nullopt;   // (a quote mark in the text)
            quoted = !quoted;
            picture += c;
        } else if (quoted) {
            picture += c;
        } else if (format.mid(i, 2) == QLatin1String("AP") || format.mid(i, 2) == QLatin1String("ap")) {
            picture += c == QLatin1Char('A') ? QStringLiteral("AM/PM") : QStringLiteral("am/pm");
            ++i;
        } else if (c == QLatin1Char('h')) {
            if (run > 2) return std::nullopt;
            picture += QString(run, twelve ? QLatin1Char('h') : QLatin1Char('H'));   // (Qt's h is on a 24-hour clock without AP)
            i += run - 1;
        } else if (c == QLatin1Char('H') || c == QLatin1Char('m') || c == QLatin1Char('s')) {
            if (run > 2) return std::nullopt;
            picture += QString(run, c);
            i += run - 1;
        } else if (c == QLatin1Char('d') || c == QLatin1Char('M')) {
            if (run > 4) return std::nullopt;
            picture += QString(run, c);
            i += run - 1;
        } else if (c == QLatin1Char('y')) {
            if (run != 2 && run != 4) return std::nullopt;
            picture += QString(run, c);
            i += run - 1;
        } else if (c.isLetter() || c == QLatin1Char('\\')) {
            return std::nullopt;
        } else {
            picture += c;
        }
    }
    if (quoted) return std::nullopt;
    return dateInPicture(when, picture, locale);
}

bool dateFieldKindFits(const QString &kind, const QString &format)
{
    bool date = false, time = false, quoted = false;
    for (const QChar c : format) {
        if (c == QLatin1Char('\'')) { quoted = !quoted; continue; }
        if (quoted) continue;
        if (c == QLatin1Char('d') || c == QLatin1Char('M') || c == QLatin1Char('y')) date = true;
        else if (c.isLetter()) time = true;   // (h, H, m, s, AP, and any other letter: the old way has no date-only reading of them)
    }
    if (kind == QLatin1String("date")) return !time;
    if (kind == QLatin1String("time")) return !date;
    return true;
}

} // namespace jp::fields
