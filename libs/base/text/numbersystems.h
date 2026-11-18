#pragma once
// A number in the number formats of a document: the w:numFmt values of a list level, a page number, a note, or a field switch (Arabic-Indic and other digits, the letters
// of other alphabets, Hebrew, Thai, Chinese, Japanese, and Korean counting, ordinal and cardinal words), and the style:num-format values of the same in a text document.
// Every number that a program numbers goes through here, so a format added here is in every place. A number that a format cannot show (zero or a negative one where the
// system counts from one, one past the end of a short alphabet, one with too many digits) shows as decimal digits.

#include <QString>
#include <QStringList>
#include <QtGlobal>

namespace jp::numsys {

// The most a counting system (Chinese, Japanese, Korean, Thai, English words) writes out; a larger number shows as digits.
constexpr qint64 kMostCounted = 999999999999LL;

// Whether this file uses the format name (a w:numFmt value) and this code writes numbers in it ("decimal" and the formats the list has).
bool knows(const QString &format);

// The number in the format: "decimal", "upperRoman", "hindiNumbers", "chineseCounting", "ordinalText"... (decimal digits where the format is unknown or cannot show n).
QString numberIn(const QString &format, qint64 n);

// Whether the format shows n in its own way (not as the decimal digits a number outside its range falls back to).
bool shows(const QString &format, qint64 n);

// A Roman numeral (1 to 3999: a number outside is clamped into it).
QString romanNumeral(qint64 n, bool upper);

// English words: 21 is "twenty-one" and "twenty-first"; empty outside 0 to kMostCounted.
QString cardinalWords(qint64 n);
QString ordinalWords(qint64 n);

// The formats offered to a person who picks one besides decimal digits, letters, and Roman numerals, in the order to show them: the w:numFmt value of each, and a label ("Thai numerals (๑, ๒, ๓, …)", translated).
QStringList offeredFormats();
QString formatLabel(const QString &format);

// The text of ODF's style:num-format for a format ("" where it has none), and the format an ODF value names ("" for one this does not know).
QString odfFormatOf(const QString &format);
QString formatOfOdf(const QString &odf);

} // namespace jp::numsys
