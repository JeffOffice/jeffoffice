#pragma once
// The yardstick for number formats (docs/jeffcrunch.md): a workbook stores the last result of every formula, so a formula that is TEXT(x, "code"),
// FIXED(x, n, no_commas), or DOLLAR(x, n) -- alone, or joined with other text by & or CONCATENATE -- has the text a format should make of x stored beside it.
// x is a number in the formula or a single cell (of this sheet, or of another by name) that holds a number; the code, the decimals, and no_commas are in the
// formula as well. Each such formula is a case: the number is formatted here and compared with what the file stored.

#include "sheet/sheetreader.h"

#include <QString>
#include <QVector>

namespace jp::office::sheet {

struct FormatCase {
    QString sheet;       // the name of the sheet the formula is in
    QString cell;        // its address, "B3"
    QString function;    // TEXT, FIXED, or DOLLAR: the first call in the formula
    double value = 0;    // the number of that call
    QString code;        // TEXT: the format code; FIXED: the decimals, and ",TRUE" if commas are off; DOLLAR: the decimals
    QString stored;      // the text the file stored for the formula
    QString ours;        // the text made here
    bool match = false;
};

// Collects the cases of a workbook one sheet at a time (so no more than one sheet is held), then those whose number is in another sheet.
class FormatCaseFinder {
public:
    explicit FormatCaseFinder(const Workbook &book);
    ~FormatCaseFinder();
    FormatCaseFinder(const FormatCaseFinder &) = delete;
    FormatCaseFinder &operator=(const FormatCaseFinder &) = delete;
    // Looks at the formula cells of sheet `index` (as Workbook::readSheet read it).
    void addSheet(int index, const Sheet &sheet);
    // Reads the other sheets the cells refer to, and gives the cases in the order of the sheets and cells. Call once.
    QVector<FormatCase> finish();

private:
    struct Impl;
    Impl *d;
};

} // namespace jp::office::sheet
