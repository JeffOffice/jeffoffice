#include "core/validationedit.h"

#include "calc/dates.h"

#include <QDate>
#include <QLocale>
#include <QTime>

namespace jp::crunch {

using namespace jp::office::sheet;

QVector<CellRange> subtractRange(const CellRange &from, const CellRange &taken)
{
    QVector<CellRange> out;
    if (!from.valid) return out;
    if (!taken.valid || taken.row1 > from.row2 || taken.row2 < from.row1 || taken.col1 > from.col2 || taken.col2 < from.col1) {
        out.append(from);
        return out;
    }
    auto block = [&](quint32 r1, quint32 c1, quint32 r2, quint32 c2) {
        CellRange r;
        r.valid = true;
        r.row1 = r1;
        r.col1 = c1;
        r.row2 = r2;
        r.col2 = c2;
        out.append(r);
    };
    if (taken.row1 > from.row1) block(from.row1, from.col1, taken.row1 - 1, from.col2);   // above
    if (taken.row2 < from.row2) block(taken.row2 + 1, from.col1, from.row2, from.col2);   // below
    const quint32 midTop = std::max(from.row1, taken.row1), midBottom = std::min(from.row2, taken.row2);
    if (taken.col1 > from.col1) block(midTop, from.col1, midBottom, taken.col1 - 1);      // left
    if (taken.col2 < from.col2) block(midTop, taken.col2 + 1, midBottom, from.col2);      // right
    return out;
}

QVector<Validation> withRuleOn(const QVector<Validation> &rules, const CellRange &where, const std::optional<Validation> &rule)
{
    QVector<Validation> out;
    for (const Validation &v : rules) {
        Validation kept = v;
        kept.ranges.clear();
        for (const CellRange &r : v.ranges) kept.ranges += subtractRange(r, where);
        if (!kept.ranges.isEmpty()) out.append(kept);
    }
    if (rule && !(rule->type == Validation::Type::Any && !rule->showInput && !rule->showError)) {
        Validation added = *rule;
        added.ranges = {where};
        out.append(added);
    }
    return out;
}

QString validationLimitText(Validation::Type type, const QString &typedIn, bool *ok)
{
    const QString typed = typedIn.trimmed();
    *ok = true;
    if (typed.isEmpty()) {
        *ok = false;
        return QString();
    }
    if (typed.startsWith(QLatin1Char('='))) return typed.mid(1);
    switch (type) {
    case Validation::Type::List:
        if (typed.size() > 255) {
            *ok = false;
            return QString();
        }
        return QLatin1Char('"') + QString(typed).replace(QLatin1Char('"'), QStringLiteral("\"\"")) + QLatin1Char('"');
    case Validation::Type::Date: {
        QDate d = QDate::fromString(typed, Qt::ISODate);
        if (!d.isValid()) d = QLocale().toDate(typed, QLocale::ShortFormat);
        if (d.isValid()) {
            double serial = 0;
            if (jp::calc::dateToSerial(d.year(), d.month(), d.day(), jp::calc::DateSystem::Date1900, &serial)) return QString::number(qint64(serial));
        }
        break;
    }
    case Validation::Type::Time: {
        QTime t = QTime::fromString(typed, QStringLiteral("hh:mm"));
        if (!t.isValid()) t = QTime::fromString(typed, QStringLiteral("hh:mm:ss"));
        if (!t.isValid()) t = QLocale().toTime(typed, QLocale::ShortFormat);
        if (t.isValid()) return QString::number(double(t.msecsSinceStartOfDay()) / 86400000.0, 'g', 15);
        break;
    }
    default: break;
    }
    bool number = false;
    double n = QLocale().toDouble(typed, &number);
    if (!number) n = QLocale::c().toDouble(typed, &number);
    if (!number) {
        *ok = false;
        return QString();
    }
    return QString::number(n, 'g', 15);
}

QString validationLimitShown(Validation::Type type, const QString &stored)
{
    if (type == Validation::Type::List && stored.size() >= 2 && stored.startsWith(QLatin1Char('"')) && stored.endsWith(QLatin1Char('"')))
        return stored.mid(1, stored.size() - 2).replace(QStringLiteral("\"\""), QStringLiteral("\""));
    bool number = false;
    stored.toDouble(&number);
    if (number && type != Validation::Type::Date && type != Validation::Type::Time) return QLocale().toString(stored.toDouble(), 'g', 15);
    if (number) return stored;
    return stored.isEmpty() ? stored : QLatin1Char('=') + stored;
}

} // namespace jp::crunch
