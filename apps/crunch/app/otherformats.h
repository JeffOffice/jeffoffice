#pragma once
// The other kinds of files JeffCrunch opens besides the .xlsx and .xlsm it saves (.ods, .fods, and the text files of values .csv, .tsv, and .txt, as well as the older .xls: see
// oldformat.h), and the one it writes besides: the values of one sheet as a .csv. A workbook read from any of them is saved as a new .xlsx and the file it came from is never written.

#include <QLocale>
#include <QString>
#include <QStringList>

namespace jp::office::sheet {
class Workbook;
}

namespace jp::crunch {

// Whether the file's name ends in .csv (any case): the name of the one other kind of file the program writes.
bool isCsvName(const QString &path);

// The filter of the Open dialog (the kinds of workbook the program opens, together), and of the Save As dialog for .csv.
QString openFilter();
QString csvFilter();

// The kind of workbook file a main part's content type makes: "xlsx" (a workbook), "xlsm" (with macros), "xltx" (a template), "xltm" (a template with macros), or "xlam" (an add-in).
// A workbook is saved as the kind it came from, because the kind is in the file.
QString workbookKindOfType(const QString &mainContentType);
// The filter the Save As dialog shows for a kind, and what the person is told when a name ends in another kind's ending.
QString workbookKindFilter(const QString &kind);
QString workbookKindNotice(const QString &kind, const QString &programName);

// What the program writes between the values of a .csv: the semicolon where the person's language writes a comma in its numbers, else the comma (the list separator of the system).
QChar csvSeparatorFor(const QLocale &locale);

// What the person is told before being asked for a name, when the workbook was read from a file of `kind` (".xls", ".ods", ".fods", ".csv", ".tsv", ".txt"): what the file is, that the
// workbook is saved as .xlsx under a new name, and that the file is not changed; and, if the file has parts the program does not read, that they will not be in the copy.
QString saveAsExplanation(const QString &fileName, const QString &kind, const QStringList &notRead = QStringList(), const QStringList &changed = QStringList());

// What the person is told when a sheet is saved as a .csv: that it is the values of this sheet only, as the cells show them.
QString csvExportNote(const QString &sheetName, QChar separator, bool partial = false);

// What a workbook has that a .csv file cannot hold, in words for the person, for the sheet in view: the other sheets, the formulas (the file has their results), the pictures, charts, notes,
// PivotTables, form controls, links to other workbooks, and macros that its parts show, and always the formats. `formulas` is how many formula cells the workbook has.
QStringList leftOutOfCsv(const jp::office::sheet::Workbook &book, qint64 formulas);

// The note of csvExportNote and, if `leftOut` is not empty, what this workbook has that the file will not.
QString csvExportWarning(const QString &sheetName, QChar separator, bool partial, const QStringList &leftOut);

// Whether two paths are one file: the same place whatever the spelling (a link, a "." or a ".." in the name), also when one of them is not there yet (its folder is looked at). Empty is none.
bool sameFile(const QString &a, const QString &b);

// The name the person chose with the kind of file picked in the dialog: with the .csv kind and no ending typed, the name ends in .csv (a dialog that adds the ending itself leaves it as it is).
QString withPickedEnding(const QString &chosen, const QString &pickedFilter);

} // namespace jp::crunch
