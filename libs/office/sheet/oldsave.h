#pragma once
// A copy of a workbook in another file format: the old binary form (.xls, BIFF8, the form of 1997 to 2003) or OpenDocument (.ods). The copy is made from a workbook read from
// a package (the .xlsx that a save makes, with the person's edits in it), sheet by sheet, so the copy has what the model holds of cells (values, formulas with their results,
// array formulas), formats (number formats, fonts, fills, borders, alignment, protection), merged cells, the sizes of rows and columns and which are hidden, frozen panes,
// defined names, and links. What it cannot hold, or does not yet write, is named in the report (`notKept`), so the person is told before it is written and after.
//
// A model can be damaged or hostile: nothing here writes more than the limits below say, however large a count in the model is. What is cut is named in `trimmed`.

#include "sheet/sheetreader.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <atomic>
#include <functional>

namespace jp::office::sheet {

// The most cells a copy holds in all, characters in a text, and sheets (the old form holds 255 sheets; OpenDocument has no limit, and this one is the reader's own).
constexpr qint64 kMaxCopyCells = 20000000;
constexpr int kMaxCopyTextChars = 32767;
constexpr qint64 kMaxCopyTextTotal = 200000000;
constexpr int kMaxXlsSheets = 255;
// The most cell formats, fonts, number formats, names, and merged ranges a copy in the old form holds (the old form's own bounds, a little under what it allows).
constexpr int kMaxXlsFormats = 4000;
constexpr int kMaxXlsFonts = 480;
constexpr int kMaxXlsNumberFormats = 240;
constexpr int kMaxXlsNames = 20000;
constexpr int kMaxXlsMerges = 65000;
constexpr int kMaxCopyHyperlinks = 65000;

struct CopyReport {
    QStringList notKept;     // what the workbook has that the copy does not ("charts", "pictures", ...), in words
    QStringList trimmed;     // what was cut to fit ("rows past the 65,536th on Sheet1 (12 cells)", ...)
    qint64 cells = 0;        // cells written
    qint64 formulas = 0;     // formulas written as formulas
    qint64 formulasAsValues = 0;   // formulas the format could not hold, written as the value they came to
    qint64 sheets = 0;
    QStringList formulaReasons;   // why formulas were written as values, the commonest first ("123 x: reason")
};

enum class CopyKind { Xls, Ods };

// What the workbook has that a copy in that format would not have, in words, for the person to be told before the copy is written. It reads the package and the sheets' models.
QStringList featuresNotInCopy(const Workbook &book, CopyKind kind);

// Writes the workbook as an .xls (BIFF8) or an .ods. False, with `error`, if it could not be made (or `cancelled` said stop). `report` may be null.
bool saveXls(const Workbook &book, QByteArray *out, CopyReport *report, QString *error, const std::function<bool()> &cancelled = nullptr);
bool saveOds(const Workbook &book, QByteArray *out, CopyReport *report, QString *error, const std::function<bool()> &cancelled = nullptr);

} // namespace jp::office::sheet
