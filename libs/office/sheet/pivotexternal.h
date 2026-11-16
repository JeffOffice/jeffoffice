#pragma once
// The data of a pivot cache read from a range of another workbook file (a sheet's cells, or a table or a defined name of that file). The file is read once, read only, with the limits an opened
// file has; what it links to is not followed (its own links stay as it stores them). The cells are given to buildPivotCache as any source is.

#include "sheet/pivot.h"

#include <QString>

namespace jp::office::sheet {

// `path` is the file; `where` is "Sheet1!A1:D100", "A1:D100" (the first sheet of the file), or the name of a table or a defined name. On success `source` has the cells, and `sheet`, `ref`, and `name`
// are what a cache stores of the source (`name` set: `sheet` and `ref` empty). False, with a reason in `why`, for a file that can't be opened (missing, too big, damaged, protected), a sheet or
// name it does not have, or a range past the bounds a pivot table is made within.
bool readPivotExternalSource(const QString &path, const QString &where, PivotSource *source, QString *sheet, QString *ref, QString *name, QString *why);

// The address a file is stored under in a cache definition for a person's choice of file: absolute, as a file: address.
QString pivotExternalAddress(const QString &path);

} // namespace jp::office::sheet
