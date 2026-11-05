#include "core/fixeddecimals.h"

#include <algorithm>

namespace jp::crunch {

QString withFixedDecimals(const QString &typed, int places, const QString &point)
{
    if (places == 0 || places > kMaxFixedDecimals || places < -kMaxFixedDecimals) return typed;
    const QString t = typed.trimmed();
    int at = 0;
    QString sign;
    if (!t.isEmpty() && (t.at(0) == QLatin1Char('-') || t.at(0) == QLatin1Char('+'))) {
        if (t.at(0) == QLatin1Char('-')) sign = QStringLiteral("-");
        at = 1;
    }
    QString digits = t.mid(at);
    if (digits.isEmpty() || digits.size() > 30) return typed;
    for (const QChar c : digits)
        if (c < QLatin1Char('0') || c > QLatin1Char('9')) return typed;
    if (places < 0) return sign + digits + QString(-places, QLatin1Char('0'));
    if (digits.size() <= places) digits = QString(places + 1 - digits.size(), QLatin1Char('0')) + digits;
    return sign + digits.left(digits.size() - places) + point + digits.right(places);
}

} // namespace jp::crunch
