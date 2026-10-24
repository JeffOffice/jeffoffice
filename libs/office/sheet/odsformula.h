#pragma once
// A formula as an OpenDocument spreadsheet writes it ("of:=SUM([.A1:.B2];[$Sheet2.C3])"), turned into the text the model keeps (SUM(A1:B2,Sheet2!C3), no "=" in front, the
// names of functions as the file format of the .xlsx writes them, with the prefix _xlfn. on those added after its first edition), so that the engine works it out.
//  - A reference is in square brackets, its sheet before a dot ([$'My Sheet'.A1]); the range of two sheets is a 3-D range; a reference to another file is not converted.
//  - Arguments are separated by a semicolon, the columns of an array constant by a semicolon and its rows by a vertical bar ({1;2|3;4}); the union of references is a tilde and
//    their intersection an exclamation mark.
//  - Names of functions the format spells with a prefix (COM.MICROSOFT.CEILING.MATH, ORG.OPENOFFICE.EASTERSUNDAY, LEGACY.NORMSDIST) lose it.
// A formula that cannot be converted (another file's reference, text that is not a formula) gives false: the cell keeps the value the file stored, as a constant.

#include <QHash>
#include <QString>

namespace jp::office::sheet {

// `formula` as the attribute holds it (with its "of:" or "oooc:" namespace prefix and "=", or without either). The converted text, or an empty one and `ok` false.
// `sheetNames`, if given, is the sheets whose names were changed (the document's name to the new one): a reference to one is written with the new name.
QString odfFormulaToText(const QString &formula, bool *ok = nullptr, const QHash<QString, QString> *sheetNames = nullptr);

// A cell range as the attributes of a named range write it: "$Sheet1.$A$1:.$B$2" or "$'My Sheet'.A1" gives "Sheet1!$A$1:$B$2" or "'My Sheet'!A1". `ok` false if it is no range.
QString odfRangeToText(const QString &address, bool *ok = nullptr, const QHash<QString, QString> *sheetNames = nullptr);

// A cell address without sheet ("B3", "$B$3") as a row and column (from 0); false if it is none.
bool odfCellOf(const QString &text, quint32 *row, quint32 *col);

// The name the standard gives a function that the model names `modelName` (ERROR.TYPE, FORMULATEXT as FORMULA, CEILING as COM.MICROSOFT.CEILING.XCL, a function the .xlsx form added
// later with its COM.MICROSOFT. prefix), and the model's name for one the standard names: the two tables that odfFormulaToText reads function names by, one each way.
QString odfFunctionName(const QString &modelName);
QString modelFunctionName(const QString &odfName);

} // namespace jp::office::sheet
