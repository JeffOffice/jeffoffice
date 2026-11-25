#pragma once
// The order texts are put in by Sort, by the list of a filter's drop-down, and by a filter's "greater than" and "less than" on text: one order for all of them, so that what a filter lists
// as above a text is what a sort puts above it. Letters by the usual rules of English (a before B, accents with their letters), with no regard to the person's language (a file sorted in one
// language is sorted the same in another).

#include <QCollator>
#include <QLocale>

namespace jp::crunch {

inline QCollator sortCollator(bool matchCase = false)
{
    QCollator collator(QLocale(QLocale::English, QLocale::UnitedStates));
    collator.setCaseSensitivity(matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
    return collator;
}

// The text with each run of digits written so that the plain order puts runs by value (2 before 10): without its leading zeros, behind the count of its digits, and that count behind the
// number of digits it has. Made once for each value, so that comparing two of them is the plain comparison. (The collator's own numeric mode is slower, and cuts a run of more than 254
// digits in parts.)
inline QString naturalSortKey(const QString &text)
{
    QString out;
    out.reserve(text.size() + 8);
    const int n = text.size();
    int i = 0;
    while (i < n) {
        if (text[i] < QLatin1Char('0') || text[i] > QLatin1Char('9')) {
            out += text[i++];
            continue;
        }
        int end = i;
        while (end < n && text[end] >= QLatin1Char('0') && text[end] <= QLatin1Char('9')) ++end;
        int first = i;
        while (first < end - 1 && text[first] == QLatin1Char('0')) ++first;
        const QString count = QString::number(end - first);
        out += QChar(char16_t(u'0' + count.size()));
        out += count;
        out += QStringView(text).mid(first, end - first);
        i = end;
    }
    return out;
}

} // namespace jp::crunch
