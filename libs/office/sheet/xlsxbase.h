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

// The XML of one font, fill, border, cell format (`xfId` is the style format it follows; `cellFormat` writes the apply attributes of a cell format) as the styles part has it, without a
// prefix, for putting in a styles part that is changed (see xlsxsave.h); and the alignment element of a format, empty if it has none.
QByteArray fontXml(const FontSpec &font);
QByteArray fillXml(const FillSpec &fill);
QByteArray borderXml(const BorderSpec &border);
QByteArray xfXml(const CellFormat &format, int xfId, bool cellFormat);
QByteArray alignmentXml(const CellFormat &format);
// A color as an element of the given name ("tabColor"): rgb, theme, indexed, or auto; empty for none.
QByteArray colorXml(const char *name, const ColorSpec &color);
// The XML a sheet that a person added begins as: its view, default sizes, and tab color, an empty sheetData, and the rules of conditional formatting and validation it has (its cells, rows,
// columns, and merged ranges are put in by the changes: see SheetChanges::skeleton).
QByteArray newSheetSkeleton(const Sheet &sheet);
// The conditionalFormatting and dataValidations elements of the rules, as the file writes them. With `extensions`, the data bars that look other than the plain one are written with their
// extension too: the list of extensions (which ends the sheet) is put there, empty if there is none.
QByteArray conditionalFormatsXml(const QVector<CondRule> &rules, QByteArray *extensions = nullptr);
QByteArray validationsXml(const QVector<Validation> &rules);

} // namespace jp::office::sheet
