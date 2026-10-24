#pragma once
// A workbook read from a file that is neither a package of XML parts nor the older binary form: an OpenDocument spreadsheet (.ods, and the flat .fods) or a text file of
// delimited values (.csv, .tsv, .txt). Such a file is read into the same model the other readers fill, and is saved as an .xlsx written from that model, as a workbook of the
// older binary form is (see xlsxbase.h). A reader of one kind gives Workbook its sheets, strings, and styles when the file is opened, and the cells of a sheet when asked.

#include "sheet/sheetreader.h"

#include <QStringList>

#include <functional>

namespace jp::office::sheet {

class ForeignBook {
public:
    virtual ~ForeignBook() = default;
    // Reads sheet `index` into `out` (cleared first). True if all of it was read; else `out` has what came before the problem and out->error says what it was.
    // `stopAfterRow` and `screenRows` are as in Workbook::readSheet; a reader that cannot stop early reads the whole sheet.
    virtual bool readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows, Sheet *out) const = 0;
    // What the file holds that is not read, in words ("charts", "pictures and other drawing objects", ...), for the person to be told.
    virtual QStringList skipped() const = 0;
    // The name of the kind of file, as the person is told it (".ods", ".csv").
    virtual QString kind() const = 0;
};

} // namespace jp::office::sheet
