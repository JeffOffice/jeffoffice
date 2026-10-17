#include "core/printing.h"

#include <QCoreApplication>
#include <QLocale>
#include <QPageSize>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <algorithm>
#include <utility>

namespace jp::write {

namespace {

// A page number as typed: the digits alone, and at least 1. Too many digits is a number past any page. False for anything else (and for 0).
bool pageNumber(const QString &text, qint64 *number)
{
    if (text.isEmpty() || !std::all_of(text.cbegin(), text.cend(), [](QChar c) { return c >= QLatin1Char('0') && c <= QLatin1Char('9'); })) return false;
    bool ok = false;
    const qint64 n = text.size() > 9 ? qint64(1) << 40 : text.toLongLong(&ok);
    if (text.size() > 9) ok = true;
    if (!ok || n < 1) return false;
    *number = n;
    return true;
}

} // namespace

QVector<int> parsePageRange(const QString &text, int pageCount)
{
    QVector<int> out;
    QSet<int> seen;
    static const QRegularExpression separators(QStringLiteral("[,;]"));
    for (const QString &part : text.split(separators, Qt::SkipEmptyParts)) {
        const QStringList ends = part.trimmed().split(QLatin1Char('-'));
        if (ends.size() > 2) continue;
        const QString a = ends.value(0).trimmed(), b = ends.size() > 1 ? ends.value(1).trimmed() : a;
        if (a.isEmpty() && b.isEmpty()) continue;
        // A range with an open end runs to the last page (or from the first) and no further; one with both ends may be given backwards.
        qint64 from = 1, to = pageCount;
        if (!a.isEmpty() && !pageNumber(a, &from)) continue;
        if (!b.isEmpty() && !pageNumber(b, &to)) continue;
        if (!a.isEmpty() && !b.isEmpty() && from > to) std::swap(from, to);
        for (qint64 p = from; p <= std::min<qint64>(to, pageCount); ++p)
            if (!seen.contains(int(p))) {
                seen.insert(int(p));
                out << int(p) - 1;
            }
    }
    return out;
}

QVector<int> sheetOrder(const QVector<int> &pages, int copies, bool collate)
{
    copies = std::max(1, copies);
    QVector<int> out;
    if (collate) {
        for (int c = 0; c < copies; ++c) out += pages;
    } else {
        for (int page : pages)
            for (int c = 0; c < copies; ++c) out << page;
    }
    return out;
}

QString paperDescription(const QSizeF &points)
{
    const bool wide = points.width() > points.height();
    const QSizeF upright = wide ? QSizeF(points.height(), points.width()) : points;
    const QPageSize size(upright, QPageSize::Point, QString(), QPageSize::FuzzyMatch);
    QString name = size.id() == QPageSize::Custom ? QString() : size.name().section(QStringLiteral(" / "), 0, 0);   // ("Letter / ANSI A": the first name)
    if (name.isEmpty()) {
        const bool inches = QLocale().measurementSystem() == QLocale::ImperialUSSystem || QLocale().measurementSystem() == QLocale::ImperialUKSystem;
        const double unit = inches ? 72.0 : 72.0 / 2.54;
        name = QStringLiteral("%1 x %2 %3").arg(points.width() / unit, 0, 'f', 1).arg(points.height() / unit, 0, 'f', 1).arg(inches ? QStringLiteral("in") : QStringLiteral("cm"));
    }
    return wide ? QCoreApplication::translate("Printing", "%1, landscape").arg(name) : QCoreApplication::translate("Printing", "%1, portrait").arg(name);
}

} // namespace jp::write
