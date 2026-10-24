#pragma once
// The file a workbook read from the older binary form is saved as. Such a workbook has no package of XML parts to put changes in, so the first save writes one from what was read
// (every sheet with its cells, formulas, and results, the strings, the styles, the names, the widths, merged ranges, and panes), with the cell formats, the strings, and the sheets in
// the places the workbook has them, and then puts the changes in that file as it does in any other (see xlsxsave.h). Nothing here changes the workbook.

#include "sheet/sheetreader.h"

#include <QByteArray>
#include <QString>

#include <atomic>

namespace jp::office::sheet {

// The names of the parts the file is written with. A workbook of the older form has them from the start (its sheets, strings, and styles are said to be there), so that the changes of
// the window find their places; they become real parts when the file is written.
QString baseMainPart();
QString baseSharedStringsPart();
QString baseStylesPart();
QString baseSheetPart(int index);

// The package, as the bytes of its ZIP, for a workbook that was read from the older binary form. Each sheet is read again from the workbook's records and written as it is read, one at
// a time. False, with a message, if a sheet cannot be read whole or `cancel` is set (looked at as the cells are written).
bool writeBaseXlsx(const Workbook &book, QByteArray *out, QString *error = nullptr, const std::atomic<bool> *cancel = nullptr);

} // namespace jp::office::sheet
