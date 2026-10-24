#pragma once
// What a person types into a cell, read as a spreadsheet reads it: a leading "=" starts a formula (checked here, with a plain message when it is not one); a number, a percent, an
// amount of money, a date, or a time is read in the person's own locale (its decimal mark, its separator of thousands, its currency sign, the order of day and month in its dates)
// and comes with the number format that shows it as it was typed (5% is 0.05 shown as 5%); TRUE, FALSE, and the errors are those; anything else is text. A cell holds 32,767
// characters, and a formula 8,192. No GUI.

#include "calc/dates.h"
#include "calc/value.h"

#include <QDate>
#include <QLocale>
#include <QString>

namespace jp::office::sheet {

// The most characters a cell's text, and a formula, may have.
constexpr int kMaxCellChars = 32767;
constexpr int kMaxFormulaTypedChars = 8192;

struct Entry {
    enum class Kind : quint8 { Empty, Number, Text, Bool, Error, Formula } kind = Kind::Empty;
    double number = 0;                  // Number; 1 or 0 for Bool
    QString text;                       // Text; the formula for Formula, with no "=" in front
    jp::calc::ErrorKind error = jp::calc::ErrorKind::Null;
    // The number format the entry asks for, as a code ("0%", "$#,##0.00", "m/d/yyyy"); empty for none (General). The window gives it to a cell whose format is General.
    QString formatCode;
};

struct EntryOptions {
    QLocale locale;                                                 // the person's
    jp::calc::DateSystem dateSystem = jp::calc::DateSystem::Date1900;
    QDate today;                                                    // the year of a date written without one; the current date when not valid
    bool textFormat = false;                                        // the cell's format is Text: everything but a formula stays as typed
};

struct EntryResult {
    bool ok = true;
    Entry entry;
    QString problem;      // when not ok: what is wrong, in a sentence the person can read
    int problemAt = -1;   // and where in what was typed (a character), or -1
};

// Reads what was typed.
EntryResult readEntry(const QString &typed, const EntryOptions &options);

// A formula as a person types it, written as a file writes it: the names of functions in capitals, and the prefix "_xlfn." put before those that were added after the first release of the
// file format (without it the other programs take the name for a function of someone's own: STDEV.S is _xlfn.STDEV.S in a file). Text in quotes, names of sheets, and names a function
// does not have are left as they are. Reading a formula back for a person (displayFormula in grid/text.h) takes the prefixes away.
QString toFileFormula(const QString &typed);

// Formulas in a locale that writes a comma for the decimal point (German, French, Spanish, and many more): there the arguments of a function are separated by a semicolon, and a number
// has a comma in it ("=SUM(A1;A2)*0,5"). The file always has the format's own (commas between arguments, dots in numbers). `formulaFromLocale` turns what was typed into the file's,
// `formulaToLocale` the file's into what is typed; text in quotes, names of sheets, brackets, and braces are left as they are, and so is a formula in any other locale.
// (Names of functions are English for now.)
QString formulaFromLocale(const QString &typed, const QLocale &locale);
QString formulaToLocale(const QString &formula, const QLocale &locale);

// What a cell is shown as when it is edited: the value as it would be typed to give the same value (a percent with its "%", a date in the locale's short form), not cut to the
// width of the cell. A text that would be read as something else (a number, a date, TRUE, a formula) has an apostrophe put in front.
// `formatCode` is the cell's number format.
QString editTextOfValue(const jp::calc::Value &value, const QString &formatCode, const EntryOptions &options);

} // namespace jp::office::sheet
