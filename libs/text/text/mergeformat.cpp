#include "text/mergeformat.h"

#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QRegularExpression>
#include <QTime>
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

// A number as written in a list: with a currency sign or a percent sign, with a comma or a period between thousands, or with the comma as the decimal mark.
std::optional<double> numberOf(const QString &text)
{
    QString t = text.trimmed();
    t.remove(QRegularExpression(QStringLiteral("[\\s\\p{Sc}%]")));
    if (t.isEmpty() || t.size() > 40) return std::nullopt;
    const bool comma = t.contains(QLatin1Char(',')), period = t.contains(QLatin1Char('.'));
    if (comma && period) {
        // (the mark that comes last is the decimal mark)
        if (t.lastIndexOf(QLatin1Char(',')) > t.lastIndexOf(QLatin1Char('.'))) {
            t.remove(QLatin1Char('.'));
            t.replace(QLatin1Char(','), QLatin1Char('.'));
        } else t.remove(QLatin1Char(','));
    } else if (comma) {
        static const QRegularExpression thousands(QStringLiteral("^[+-]?\\d{1,3}(,\\d{3})+$"));
        if (thousands.match(t).hasMatch()) t.remove(QLatin1Char(','));
        else t.replace(QLatin1Char(','), QLatin1Char('.'));
    }
    bool ok = false;
    const double v = QLocale::c().toDouble(t, &ok);
    if (!ok) return std::nullopt;
    return v;
}

std::optional<QDateTime> dateTimeOf(const QString &text)
{
    const QString t = text.trimmed();
    if (t.isEmpty() || t.size() > 60) return std::nullopt;
    QDateTime iso = QDateTime::fromString(t, Qt::ISODate);
    if (iso.isValid()) return iso;
    for (const char *pattern : {"M/d/yyyy", "MM/dd/yyyy", "M/d/yy", "d.M.yyyy", "MMMM d, yyyy", "MMM d, yyyy", "d MMMM yyyy", "d MMM yyyy", "yyyy/M/d", "M/d/yyyy h:mm AP", "M/d/yyyy H:mm", "yyyy-MM-dd H:mm"}) {
        const QDateTime d = QDateTime::fromString(t, QLatin1String(pattern));
        if (d.isValid() && d.date().year() >= 1000) return d;
    }
    for (QLocale::FormatType type : {QLocale::ShortFormat, QLocale::LongFormat}) {
        const QDate d = QLocale().toDate(t, type);
        if (d.isValid()) return QDateTime(d, QTime(0, 0));
    }
    for (const char *pattern : {"H:mm", "HH:mm", "H:mm:ss", "h:mm AP", "h:mm:ss AP", "h AP"}) {
        const QTime tm = QTime::fromString(t, QLatin1String(pattern));
        if (tm.isValid()) return QDateTime(QDate(2000, 1, 1), tm);
    }
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

QString formatMergeValue(const QString &value, const QString &format)
{
    if (format.isEmpty() || value.isEmpty() || !isMergeFormat(format)) return value;
    QString kind, detail;
    parts(format, &kind, &detail);
    const QLocale loc;
    if (kind == QLatin1String("date") || kind == QLatin1String("time")) {
        const auto when = dateTimeOf(value);
        if (!when) return value;
        if (kind == QLatin1String("date")) {
            if (detail == QLatin1String("short")) return loc.toString(when->date(), QLocale::ShortFormat);
            if (detail == QLatin1String("long")) return loc.toString(when->date(), QLocale::LongFormat);
            if (detail == QLatin1String("iso")) return when->date().toString(Qt::ISODate);
            return loc.toString(when->date(), detail == QLatin1String("monthday") ? QStringLiteral("MMMM d") : QStringLiteral("MMMM yyyy"));
        }
        if (detail == QLatin1String("short")) return loc.toString(when->time(), QLocale::ShortFormat);
        return loc.toString(when->time(), detail == QLatin1String("12") ? QStringLiteral("h:mm AP") : QStringLiteral("HH:mm"));
    }
    const auto number = numberOf(value);
    if (!number) return value;
    int places = 2;
    if (!detail.isEmpty()) decimalsOk(detail, &places);
    if (kind == QLatin1String("currency")) return loc.toCurrencyString(*number, loc.currencySymbol(), places);
    if (kind == QLatin1String("percent")) return loc.toString(*number * 100, 'f', places) + loc.percent();
    return loc.toString(*number, 'f', places);
}

} // namespace jp
