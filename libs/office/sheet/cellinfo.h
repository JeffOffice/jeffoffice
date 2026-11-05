#pragma once
// What CELL("format"), CELL("color"), and CELL("parentheses") say about a number format code.

#include <QString>

namespace jp::office::sheet {

struct FormatSummary {
    QString code;          // "G", "F2", ",0", "C2", "P0", "S2", "D1" to "D9": the kind of format and its decimal places
    bool negativeColor = false;   // the section for negative numbers has a color
    bool parentheses = false;     // the section for positive numbers is in parentheses
};

// The summary of a number format code (as a file writes it, "General" and the others). `code` of the result has "-" added when negatives have a color and "()" when positives are in parentheses.
FormatSummary summarizeFormat(const QString &code);

} // namespace jp::office::sheet
