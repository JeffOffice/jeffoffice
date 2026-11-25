#pragma once
// The rows of a sheet, a table, or a named range of another workbook file, for Data > Get Data (see datasource.h). The file is opened as for any workbook (every form the program opens), read for its
// values only (a formula gives the value the file stored), and never changed. A file larger than the limit, past the limits of the readers, or protected with a password is refused in words.

#include "sheet/datasource.h"

namespace jp::office::sheet {

// What the file offers: "sheet:Name", "table:Name", and "name:Name" (a named range of one block of cells). Empty with *why set when the file can't be read or offers nothing.
QStringList dataBookLists(const QString &path, const DataLimits &limits, const std::function<bool()> &cancelled, QString *why);

// Reads the list: the first row is the header (a table without a header row, and a named range, are given columns named Column1, Column2, ...: a named range's first row is its header). maxRows <= 0 reads all.
bool dataReadBook(const QString &path, const QString &list, int maxRows, const DataLimits &limits, const std::function<bool()> &cancelled, DataRows *out, QString *why);

}   // namespace jp::office::sheet
