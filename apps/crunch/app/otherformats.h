#pragma once
// The other kinds of files JeffCrunch opens besides the .xlsx and .xlsm it saves (.ods, .fods, and the text files of values .csv, .tsv, and .txt, as well as the older .xls: see
// oldformat.h), and the one it writes besides: the values of one sheet as a .csv. A workbook read from any of them is saved as a new .xlsx and the file it came from is never written.

#include <QLocale>
#include <QString>
#include <QStringList>

namespace jp::crunch {

// Whether the file's name ends in .csv (any case): the name of the one other kind of file the program writes.
bool isCsvName(const QString &path);

// The filter of the Open dialog (the kinds of workbook the program opens, together), and of the Save As dialog for .csv.
QString openFilter();
QString csvFilter();

// What the program writes between the values of a .csv: the semicolon where the person's language writes a comma in its numbers, else the comma (the list separator of the system).
QChar csvSeparatorFor(const QLocale &locale);

// What the person is told before being asked for a name, when the workbook was read from a file of `kind` (".xls", ".ods", ".fods", ".csv", ".tsv", ".txt"): what the file is, that the
// workbook is saved as .xlsx under a new name, and that the file is not changed; and, if the file has parts the program does not read, that they will not be in the copy.
QString saveAsExplanation(const QString &fileName, const QString &kind, const QStringList &notRead = QStringList());

// What the person is told when a sheet is saved as a .csv: that it is the values of this sheet only, as the cells show them.
QString csvExportNote(const QString &sheetName, QChar separator);

} // namespace jp::crunch
