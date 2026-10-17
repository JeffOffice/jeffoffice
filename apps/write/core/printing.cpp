#include "core/printing.h"

#include <QCoreApplication>
#include <QLocale>
#include <QPageSize>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <algorithm>

namespace jp::write {

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
        bool okFrom = true, okTo = true;
        const int from = a.isEmpty() ? 1 : a.toInt(&okFrom), to = b.isEmpty() ? pageCount : b.toInt(&okTo);
        if (!okFrom || !okTo) continue;
        for (int p = std::max(1, std::min(from, to)); p <= std::min(pageCount, std::max(from, to)); ++p)
            if (!seen.contains(p)) {
                seen.insert(p);
                out << p - 1;
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
