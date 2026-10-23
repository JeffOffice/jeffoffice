#pragma once
// Number formats: what turns a cell's value and its format code into the text a person sees (ECMA-376 18.8.30 and 18.8.31, and the behavior workbooks
// depend on). No GUI: the grid, the text of a field, a chart axis, and TEXT() all call this.
//
// A code has up to four sections, split at ";": positive, negative, zero, text. A section may begin with a condition ("[>100]", "[<=-5]", "[=0]",
// "[<>3]") and a color ("[Red]", "[Color12]"). It is made of
//   - text: "quoted", \x for one character, the characters that need no quotes ($ - + / ( ) : ! ^ & ' ~ { } < > = and a space), "_x" for a space as wide
//     as x, "*x" for x repeated to fill the column (see Formatted::fillAt), and "@" for the text of a text cell;
//   - digits: 0 (a digit or 0), # (a digit or nothing), ? (a digit or a space), "." for the decimal point, "," between digits for groups of three and
//     after the last digit for each division by 1,000, "%" for each multiplication by 100, "E+" "E-" "e+" "e-" for scientific notation (and an exponent
//     of a multiple of the integer digits' number, so ##0.0E+0 is engineering notation), fractions ("# ?/?" with up to nine places on either side of the slash, "# ?/8" with a fixed denominator, "?/?" with no whole part), and "General";
//   - dates and times: d dd ddd dddd, m mm mmm mmmm mmmmm (m and mm are minutes after h and before s), yy yyyy, h hh, s ss with ".0" ".00" ".000", AM/PM,
//     am/pm, A/P, a/p, and the elapsed forms [h] [hh] [m] [mm] [s] [ss].
//   - "[$text-locale]": a currency sign or any text, and a locale, which is a number in hex ("409"), a language tag ("en-US", "de-DE"), or the system's long
//     date and time ("F800" or "x-sysdate", "F400" or "x-systime"). The system's long date and time stand for the pattern after them if the code has one
//     (a file stores the style with the pattern it had when it was saved, and that is shown, once), else for the United States' ("dddd, mmmm d, yyyy" and
//     "h:mm:ss AM/PM").
// Locales: a code's locale chooses the names of months and days. English is the default and the names of every English locale; German, French, Spanish,
// Italian, Portuguese, and Dutch are known by name. Any other locale gets English names. The separators of numbers are never changed by a locale: a code
// is always written with "." and ",", and the text it makes follows them (the reading of TEXT() in a workbook of another language is that program's).
//
// A value is rounded as the original does: from its 15 significant digits as decimal text, not from the binary value, so 2.675 is "2.68" with "0.00" and
// a value of 15 digits or more shows zeros after the 15th. Dates and times are rounded to the least unit the code shows (a second, or the digits of
// seconds it has; a code with no time shows the day and drops the time).
//
// Everything a file controls is bounded: the code's length, its sections, the digit places of a section, the brackets, and the text made (a cell holds
// 32,767 characters). A code past a bound is not valid, and a value is then shown as General.

#include "calc/dates.h"

#include <QChar>
#include <QString>
#include <QVector>

#include <memory>

namespace jp::calc {

// Characters in a code (the format allows 255), sections (a fifth makes a code invalid), 0 # ? places in one section, brackets in one section, and
// characters in the text made.
constexpr int kMaxFormatCodeChars = 4096;
constexpr int kMaxFormatSections = 4;
constexpr int kMaxFormatDigits = 255;
constexpr int kMaxFormatBrackets = 16;
constexpr int kMaxFormatOutputChars = 32767;
// FIXED and DOLLAR take at most this many decimals (in the other direction, the same number of places to the left).
constexpr int kMaxFixedDecimals = 127;

// "_x": the text has a space at `at`, as wide as the character `like`.
struct FormatPad {
    int at = 0;
    QChar like;
};

struct Formatted {
    QString text;
    // The color the section asks for, as the number of the 56 colors of the palette ([Color1] to [Color56]; [Black] is 1, [White] 2, [Red] 3, [Green] 4,
    // [Blue] 5, [Yellow] 6, [Magenta] 7, [Cyan] 8); 0 for none. formatColorRgb() has its RGB.
    int color = 0;
    // "*x": where the text of x is to be repeated to fill the width (an index into `text`, a character is not in the text), and x. -1 for none.
    int fillAt = -1;
    QChar fillChar;
    QVector<FormatPad> pads;
    // The value can't be shown in the code: a date before day 0 or past December 31, 9999, no section that takes the value, a number that is not
    // finite, or a text past the limit. A cell then shows "#" to fill its width, and TEXT() gives #VALUE!. `text` is empty.
    bool failed = false;
};

// What the first section that takes a number makes of it; for the grid (a number or a date aligns right, a text left).
enum class FormatKind : quint8 { General, Number, Date, Text };

// A parsed code. Cheap to copy. A code that is not valid (too long, too many sections, a quote never closed) shows as General.
class NumberFormat {
public:
    NumberFormat();                                   // General
    explicit NumberFormat(const QString &code);
    // The built-in format of an id (0 to 49, and the East Asian ones 27 to 36 and 50 to 58); General for any other id.
    static NumberFormat builtin(int id);

    bool isValid() const;
    const QString &code() const;
    FormatKind kind() const;
    // The code is "General" (any case), or not valid.
    bool isGeneral() const;
    // The value as the first section that takes it shows it. `generalChars` is the width, in characters, that a General number may take (what the
    // cell's column holds): the most digits of 15 that fit are shown, and scientific notation when none do; 0 for no limit, which is the 15 digits that
    // TEXT() and the joining of text give.
    Formatted format(double value, DateSystem system = DateSystem::Date1900, int generalChars = 0) const;
    // A text as the text section shows it (or as it is when the code has none).
    Formatted formatText(const QString &text) const;

private:
    struct Data;
    std::shared_ptr<const Data> d;
};

// The code of a built-in format id ("" for an id with none: 23 to 26, and 59 and up). Id 14 is the short date and 22 the short date and time, as the
// system shows them in the United States ("m/d/yyyy", "m/d/yyyy h:mm").
QString builtinFormatCode(int id);

// The RGB (0xRRGGBB) of a palette color 1 to 56 as the number formats name them; 0 for any other number.
quint32 formatColorRgb(int color);

// TEXT(value, code): the value in the code, as the spreadsheet's function gives it: a fill ("*x") is dropped (there is no column to fill), a pad ("_x")
// is a space, the empty code gives the empty text, and False (#VALUE!) is for a code that is not valid or has no section that can show the value.
bool formatTextFunction(double value, const QString &code, DateSystem system, QString *out);

// FIXED(value, decimals, no_commas) and DOLLAR(value, decimals) as the spreadsheet's functions give them: the value rounded to the decimals (to tens,
// hundreds, ... for negative ones) from its 15 digits, with commas for groups of three, and for DOLLAR a "$", negative ones in parentheses. False if
// the value is not finite or the decimals are past kMaxFixedDecimals either way.
bool formatFixed(double value, int decimals, bool noCommas, QString *out);
bool formatDollar(double value, int decimals, QString *out);

} // namespace jp::calc
