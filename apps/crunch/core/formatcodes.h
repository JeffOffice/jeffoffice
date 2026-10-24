#pragma once
// Number format codes as a person chooses them: the categories of the Format Cells dialog (General, Number, Currency, Accounting, Date, Time, Percentage, Fraction, Scientific, Text,
// Special, Custom) and the options of each (the places after the decimal mark, the grouping of digits, how a negative number shows, the currency symbol), turned into the code the file
// holds and back, the lists of ready-made formats a category offers, and the change the Increase Decimal and Decrease Decimal buttons make to a code. The codes are the file's: a "."
// for the decimal mark and a "," for grouping whatever the person's language writes.

#include <QLocale>
#include <QString>
#include <QVector>

namespace jp::crunch {

enum class NumberCategory { General, Number, Currency, Accounting, Date, Time, Percentage, Fraction, Scientific, Text, Special, Custom };

struct NumberOptions {
    int decimals = 2;
    bool thousands = false;     // Number, Currency, Accounting
    int negative = 0;           // Number, Currency: 0 -1234.10, 1 -1234.10 in red, 2 (1234.10), 3 (1234.10) in red
    QString symbol;             // Currency, Accounting: "$", "EUR", ...; empty for none
    bool symbolFirst = true;    // the symbol comes before the number (else after, with a space)
    bool operator==(const NumberOptions &o) const { return decimals == o.decimals && thousands == o.thousands && negative == o.negative && symbol == o.symbol && symbolFirst == o.symbolFirst; }
};

// A ready-made format of a category: its code, and what a person is told it is.
struct FormatType {
    QString code, label;
};

// The name of a category as the dialog shows it.
QString categoryName(NumberCategory category);
// The category a code is in (Custom for one that is none of the others).
NumberCategory categoryOf(const QString &code);
// The code a category makes of its options (the code of General and Text have no options; the lists of Date, Time, Fraction, and Special are `typesFor`).
QString codeFor(NumberCategory category, const NumberOptions &options);
// The options of a code of Number, Currency, Accounting, Percentage, or Scientific, as the dialog would have made it (false, and the options as far as they are known, for a code it would
// not: the person is shown it as Custom).
bool optionsOf(const QString &code, NumberCategory *category, NumberOptions *options);
// The ready-made formats of Date, Time, Fraction, and Special in the language (and, for Custom, the usual codes); empty for the others.
QVector<FormatType> typesFor(NumberCategory category, const QLocale &locale);
// The negative styles of Number and Currency for a number of places, each as the code makes it show -1234.1 (a sample in the dialog's list is formatted by the caller).
int negativeStyles();
// Increase Decimal (delta 1) and Decrease Decimal (-1): the code with a place more or less after the decimal mark in its first part (General becomes 0.0 for an increase). A code that
// shows no number (text, a date) is returned as it is.
QString withDecimals(const QString &code, int delta);
// The currency of the Currency button in the language: its code, with two places, the symbol before or after the number as the language writes it.
QString currencyCode(const QLocale &locale);
// The symbol a language writes for its currency ("$", "€", "£", "¥"), and whether it comes before the number.
QString currencySymbolOf(const QLocale &locale, bool *first = nullptr);
// The code of the format the Comma Style button gives (the accounting format with no symbol and two places) and the Percent Style button.
QString commaCode();
QString percentCode();

} // namespace jp::crunch
