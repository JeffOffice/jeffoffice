#pragma once
// The order texts are put in by Sort, by the list of a filter's drop-down, and by a filter's "greater than" and "less than" on text: one order for all of them, so that what a filter lists
// as above a text is what a sort puts above it. Letters by the usual rules of English (a before B, accents with their letters), with no regard to the person's language (a file sorted in one
// language is sorted the same in another).

#include <QCollator>
#include <QLocale>

namespace jp::crunch {

inline QCollator sortCollator(bool matchCase = false, bool natural = false)
{
    QCollator collator(QLocale(QLocale::English, QLocale::UnitedStates));
    collator.setCaseSensitivity(matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
    collator.setNumericMode(natural);   // (digits in text are read by their value: 2 before 10)
    return collator;
}

} // namespace jp::crunch
