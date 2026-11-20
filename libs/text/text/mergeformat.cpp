#include "text/mergeformat.h"

#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QRegularExpression>
#include <QTime>
#include <cmath>
#include <optional>

namespace jp {

namespace {

const QStringList &dateStyles()
{
    static const QStringList styles{QStringLiteral("short"), QStringLiteral("long"), QStringLiteral("iso"), QStringLiteral("monthday"), QStringLiteral("monthyear")};
    return styles;
}

const QStringList &timeStyles()
{
    static const QStringList styles{QStringLiteral("short"), QStringLiteral("12"), QStringLiteral("24")};
    return styles;
}

// A format's kind and its detail: "number:2" is "number" and "2".
void parts(const QString &format, QString *kind, QString *detail)
{
    *kind = format.section(QLatin1Char(':'), 0, 0);
    *detail = format.section(QLatin1Char(':'), 1);
}

bool decimalsOk(const QString &detail, int *out)
{
    bool ok = false;
    const int n = detail.toInt(&ok);
    if (!ok || n < 0 || n > 4) return false;
    if (out) *out = n;
    return true;
}

QLocale localeOf(const QString &language)
{
    if (language.trimmed().isEmpty()) return QLocale(QLocale::English, QLocale::UnitedStates);   // (no language says US English, as everywhere in the layout)
    const QLocale loc(QString(language).replace(QLatin1Char('-'), QLatin1Char('_')));
    return loc.language() == QLocale::C ? QLocale(QLocale::English, QLocale::UnitedStates) : loc;
}

// The one character a locale uses for a mark, with the spaces that look alike taken as one.
QChar markOf(const QString &text)
{
    const QChar c = text.isEmpty() ? QChar() : text.at(0);
    return c == QChar(0x00A0) || c == QChar(0x202F) || c == QChar(0x2009) ? QChar(' ') : (c == QChar(0x2019) ? QChar('\'') : c);
}

// A number as written in a list, in the document's language: with a currency sign or a percent sign, with that language's group sign between thousands (in groups of three) and its
// decimal sign, or a lone sign that can't be a group sign taken as the decimal one; "(500)" is -500. Words such as nan and inf, exponents, and anything else are no number.
std::optional<double> numberOf(const QString &text, const QLocale &loc)
{
    static const QRegularExpression signs(QStringLiteral("[\\p{Sc}%]"));
    QString t = text.trimmed();
    t.remove(signs);
    t = t.trimmed();
    bool negative = false;
    if (t.size() > 2 && t.startsWith(QLatin1Char('(')) && t.endsWith(QLatin1Char(')'))) {
        negative = true;
        t = t.mid(1, t.size() - 2).trimmed();
    }
    t.replace(QChar(0x00A0), QLatin1Char(' ')).replace(QChar(0x202F), QLatin1Char(' ')).replace(QChar(0x2019), QLatin1Char('\''));
    t.remove(QRegularExpression(QStringLiteral("(?<=[+-])\\s+")));
    if (t.isEmpty() || t.size() > 40) return std::nullopt;
    static const QRegularExpression shape(QStringLiteral("^[+-]?\\d[\\d.,' ]*$"));
    if (!shape.match(t).hasMatch()) return std::nullopt;
    const QChar dec = markOf(loc.decimalPoint()), grp = markOf(loc.groupSeparator());
    QString digits = t;
    const QString g = QRegularExpression::escape(QString(grp)), d = QRegularExpression::escape(QString(dec));
    const QRegularExpression grouped(QStringLiteral("^[+-]?\\d{1,3}(%1\\d{3})+(%2\\d+)?$").arg(g, d));
    if (grouped.match(t).hasMatch()) {
        digits.remove(grp);
        digits.replace(dec, QLatin1Char('.'));
    } else {
        // (not grouped: at most one sign, and it is a decimal one)
        int marks = 0;
        for (QChar c : t)
            if (c == QLatin1Char('.') || c == QLatin1Char(',') || c == QLatin1Char('\'') || c == QLatin1Char(' ')) ++marks;
        if (marks > 1) return std::nullopt;
        if (marks == 1) {
            const int at = t.indexOf(QRegularExpression(QStringLiteral("[.,' ]")));
            if (t.at(at) == QLatin1Char('\'') || t.at(at) == QLatin1Char(' ') || at == t.size() - 1) return std::nullopt;
            digits.replace(at, 1, QLatin1Char('.'));
        }
    }
    bool ok = false;
    const double v = QLocale::c().toDouble(digits, &ok);
    if (!ok || !std::isfinite(v) || std::abs(v) > 1e15) return std::nullopt;
    return negative ? -v : v;
}

// What a value holds of a moment: a date, a time, or both.
struct Moment {
    QDate date;
    QTime time;
};

// A year of two digits: 00 to 29 is the 2000s, 30 to 99 the 1900s.
int yearOf(int y, int digits) { return digits <= 2 ? (y < 30 ? 2000 + y : 1900 + y) : y; }

std::optional<QDate> dateOnly(const QString &text, const QLocale &loc)
{
    const QString t = text.trimmed();
    if (t.isEmpty() || t.size() > 40) return std::nullopt;
    static const QRegularExpression iso(QStringLiteral("^(\\d{4})-(\\d{1,2})-(\\d{1,2})$")), numeric(QStringLiteral("^(\\d{1,2})[/.\\-](\\d{1,2})[/.\\-](\\d{2}|\\d{4})$"));
    if (const auto m = iso.match(t); m.hasMatch()) {
        const QDate d(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt());
        return d.isValid() ? std::optional<QDate>(d) : std::nullopt;
    }
    if (const auto m = numeric.match(t); m.hasMatch()) {
        // (the order of day and month is the language's: the first of "d" and "M" in its short date)
        const QString pattern = loc.dateFormat(QLocale::ShortFormat);
        const int dayAt = pattern.indexOf(QLatin1Char('d')), monthAt = pattern.indexOf(QLatin1Char('M'));
        const bool dayFirst = dayAt >= 0 && monthAt >= 0 ? dayAt < monthAt : false;
        const int a = m.captured(1).toInt(), b = m.captured(2).toInt();
        const QDate d(yearOf(m.captured(3).toInt(), int(m.captured(3).size())), dayFirst ? b : a, dayFirst ? a : b);
        return d.isValid() ? std::optional<QDate>(d) : std::nullopt;
    }
    for (const char *pattern : {"MMMM d, yyyy", "MMM d, yyyy", "d MMMM yyyy", "d MMM yyyy", "d. MMMM yyyy"}) {
        for (const QLocale &where : {loc, QLocale(QLocale::English, QLocale::UnitedStates)}) {
            const QDate d = where.toDate(t, QLatin1String(pattern));
            if (d.isValid() && d.year() >= 1000) return d;
        }
    }
    return std::nullopt;
}

std::optional<QTime> timeOnly(const QString &text)
{
    const QString t = text.trimmed();
    if (t.isEmpty() || t.size() > 20) return std::nullopt;
    for (const char *pattern : {"H:mm", "HH:mm", "H:mm:ss", "h:mm AP", "h:mm:ss AP", "h AP"}) {
        const QTime tm = QTime::fromString(t, QLatin1String(pattern));
        if (tm.isValid()) return tm;
    }
    return std::nullopt;
}

std::optional<Moment> momentOf(const QString &text, const QLocale &loc)
{
    const QString t = text.trimmed();
    if (t.isEmpty() || t.size() > 60) return std::nullopt;
    // A date and a time together: the date, then a space or T, then the time.
    static const QRegularExpression both(QStringLiteral("^(.+?)[ T](\\d{1,2}(?::\\d{2}).*)$"));
    if (const auto m = both.match(t); m.hasMatch()) {
        const auto d = dateOnly(m.captured(1), loc);
        const auto tm = timeOnly(m.captured(2).left(12).remove(QRegularExpression(QStringLiteral("(Z|[+-]\\d\\d:?\\d\\d)$"))));
        if (d && tm) return Moment{*d, *tm};
        if (d) return Moment{*d, QTime()};
    }
    if (const auto d = dateOnly(t, loc)) return Moment{*d, QTime()};
    if (const auto tm = timeOnly(t)) return Moment{QDate(), *tm};
    return std::nullopt;
}

} // namespace

MergeFieldCode splitMergeField(const QString &arg)
{
    const int bar = arg.lastIndexOf(QLatin1Char('|'));
    if (bar > 0 && isMergeFormat(arg.mid(bar + 1))) return {arg.left(bar), arg.mid(bar + 1)};
    return {arg, QString()};
}

QString joinMergeField(const QString &name, const QString &format)
{
    return format.isEmpty() ? name : name + QLatin1Char('|') + format;
}

bool isMergeFormat(const QString &format)
{
    QString kind, detail;
    parts(format, &kind, &detail);
    if (kind == QLatin1String("number") || kind == QLatin1String("percent")) return decimalsOk(detail, nullptr);
    if (kind == QLatin1String("currency")) return detail.isEmpty() || decimalsOk(detail, nullptr);
    if (kind == QLatin1String("date")) return dateStyles().contains(detail);
    if (kind == QLatin1String("time")) return timeStyles().contains(detail);
    return false;
}

QString formatMergeValue(const QString &value, const QString &format, const QString &language)
{
    if (format.isEmpty() || value.isEmpty() || !isMergeFormat(format)) return value;
    QString kind, detail;
    parts(format, &kind, &detail);
    const QLocale loc = localeOf(language);
    if (kind == QLatin1String("date") || kind == QLatin1String("time")) {
        const auto when = momentOf(value, loc);
        // (a value with no date is not given one, and a value with no time is not given a time of midnight: it is left as the list holds it)
        if (!when || (kind == QLatin1String("date") ? when->date.isNull() : !when->time.isValid())) return value;
        if (kind == QLatin1String("date")) {
            if (detail == QLatin1String("short")) return loc.toString(when->date, QLocale::ShortFormat);
            if (detail == QLatin1String("long")) return loc.toString(when->date, QLocale::LongFormat);
            if (detail == QLatin1String("iso")) return when->date.toString(Qt::ISODate);
            return loc.toString(when->date, detail == QLatin1String("monthday") ? QStringLiteral("MMMM d") : QStringLiteral("MMMM yyyy"));
        }
        if (detail == QLatin1String("short")) return loc.toString(when->time, QLocale::ShortFormat);
        return loc.toString(when->time, detail == QLatin1String("12") ? QStringLiteral("h:mm AP") : QStringLiteral("HH:mm"));
    }
    const auto number = numberOf(value, loc);
    if (!number) return value;
    int places = 2;
    if (!detail.isEmpty()) decimalsOk(detail, &places);
    if (kind == QLatin1String("currency")) return loc.toCurrencyString(*number, loc.currencySymbol(), places);
    if (kind == QLatin1String("percent")) return loc.toString(*number * 100, 'f', places) + loc.percent();
    return loc.toString(*number, 'f', places);
}

} // namespace jp
