#pragma once
// The names of months and days in the languages a number format's locale can name (calc/numfmt.h).

#include <QString>

namespace jp::calc {

// The language of a locale id ([$-LCID]): 0 English (and every locale with no names here), 1 German, 2 French, 3 Spanish, 4 Italian, 5 Portuguese, 6 Dutch.
int formatLanguageOf(quint32 lcid);
// Month 1 to 12 in full (form 0) or abbreviated (form 1); a month out of range gives the empty string.
const QString &formatMonthName(int lang, int month, int form);
// Day 0 (Sunday) to 6 in full (form 0) or abbreviated (form 1).
const QString &formatDayName(int lang, int weekday, int form);

} // namespace jp::calc
