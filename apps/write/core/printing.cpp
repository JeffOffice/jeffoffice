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
    return parsePageRange(text, pageCount, {});
}

QVector<int> parsePageRange(const QString &text, int pageCount, const QVector<int> &sectionOfPage)
{
    // The pages of each section, by its number from 1 (the sections in the order of their values).
    QVector<int> order = sectionOfPage;
    std::sort(order.begin(), order.end());
    order.erase(std::unique(order.begin(), order.end()), order.end());
    auto pagesOf = [&](qint64 number) {
        QVector<int> pages;
        if (number < 1 || number > order.size()) return pages;
        for (int i = 0; i < sectionOfPage.size() && i < pageCount; ++i)
            if (sectionOfPage[i] == order[int(number) - 1]) pages << i + 1;
        return pages;
    };
    // An end of a range: a page number, the page of a section, or a whole section (its first and last pages).
    static const QRegularExpression pageOfSection(QStringLiteral("^p(\\d{1,9})s(\\d{1,9})$"), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression wholeSection(QStringLiteral("^s(\\d{1,9})$"), QRegularExpression::CaseInsensitiveOption);
    auto parseEnd = [&](const QString &t, qint64 *first, qint64 *last) {
        if (pageNumber(t, first)) {
            *last = *first;
            return true;
        }
        if (const auto m = pageOfSection.match(t); m.hasMatch()) {
            const QVector<int> pages = pagesOf(m.captured(2).toLongLong());
            const qint64 n = m.captured(1).toLongLong();
            if (n < 1 || n > pages.size()) return false;
            *first = *last = pages[int(n) - 1];
            return true;
        }
        if (const auto m = wholeSection.match(t); m.hasMatch()) {
            const QVector<int> pages = pagesOf(m.captured(1).toLongLong());
            if (pages.isEmpty()) return false;
            *first = pages.first();
            *last = pages.last();
            return true;
        }
        return false;
    };
    QVector<int> out;
    QSet<int> seen;
    static const QRegularExpression separators(QStringLiteral("[,;]"));
    for (const QString &part : text.split(separators, Qt::SkipEmptyParts)) {
        const QStringList ends = part.trimmed().split(QLatin1Char('-'));
        if (ends.size() > 2) continue;
        const QString a = ends.value(0).trimmed(), b = ends.size() > 1 ? ends.value(1).trimmed() : a;
        if (a.isEmpty() && b.isEmpty()) continue;
        // A range with an open end runs to the last page (or from the first) and no further; one with both ends may be given backwards.
        qint64 from = 1, to = pageCount, ignored = 0;
        if (!a.isEmpty() && !parseEnd(a, &from, ends.size() > 1 ? &ignored : &to)) continue;
        if (!b.isEmpty() && ends.size() > 1 && !parseEnd(b, &ignored, &to)) continue;
        if (!a.isEmpty() && !b.isEmpty() && from > to) std::swap(from, to);
        for (qint64 p = from; p <= std::min<qint64>(to, pageCount); ++p)
            if (!seen.contains(int(p))) {
                seen.insert(int(p));
                out << int(p) - 1;
            }
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
