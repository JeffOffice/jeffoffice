#include "core/autocomplete.h"

#include "core/editsession.h"

namespace jp::crunch {

namespace {

// A text that reads as a number (a digit string with signs, marks, or a percent or currency sign) is not offered: it was typed to be a number.
bool looksNumeric(const QString &s)
{
    bool digit = false;
    for (const QChar c : s) {
        if (c.isDigit()) digit = true;
        else if (!c.isSpace() && !QStringLiteral("+-.,%$/:()").contains(c)) return false;
    }
    return digit;
}

} // namespace

QString autoCompleted(const QStringList &column, const QString &typed)
{
    if (typed.isEmpty() || typed.startsWith(QLatin1Char('='))) return QString();
    QString found;
    for (const QString &entry : column) {
        if (entry.size() <= typed.size() || !entry.startsWith(typed, Qt::CaseInsensitive) || looksNumeric(entry)) continue;
        if (found.isEmpty()) found = entry;
        else if (found.compare(entry, Qt::CaseInsensitive) != 0) return QString();   // (two different entries start so: none is offered)
    }
    return found;
}

QStringList columnTextsAround(const EditSession &session, int sheet, quint32 row, quint32 col)
{
    QStringList texts;
    auto walk = [&](qint64 step) {
        qint64 r = qint64(row) + step;
        for (int n = 0; n < kAutoCompleteReach && r >= 0 && r < qint64(jp::office::sheet::kMaxRows); ++n, r += step) {
            const jp::calc::Value v = session.valueAt(sheet, quint32(r), col);
            if (v.isEmpty()) break;
            if (v.isText()) texts << v.text();
        }
    };
    walk(-1);
    walk(1);
    return texts;
}

} // namespace jp::crunch
