#pragma once
// "Automatic decimal places" (Options > Editing): a whole number typed in a cell has its decimal point put in that many places from the right (with 2, typing 1234 enters 12.34); a negative number
// of places adds zeros instead (with -2, typing 5 enters 500). A text that is not a whole number (it has a point, an exponent, a letter, a sign inside, or is a formula) is left as it is.

#include <QString>

namespace jp::crunch {

constexpr int kMaxFixedDecimals = 15;

// The text to enter for `typed`, with `places` decimal places (0 is off) and `point` as the decimal mark.
QString withFixedDecimals(const QString &typed, int places, const QString &point);

} // namespace jp::crunch
