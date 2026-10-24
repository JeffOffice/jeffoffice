#pragma once
// Workbooks of the older binary file format (.xls, BIFF5 and BIFF8). JeffCrunch opens them and shows what it reads of them; it saves workbooks as .xlsx only, so a workbook
// that came from an .xls file is saved under a new name with that ending, after the person is told so in plain words, and the file it came from is never written to.

#include <QString>
#include <QStringList>

namespace jp::crunch {

// Whether the file's name ends as the older binary form's do (.xls, any case).
bool isOlderFormatName(const QString &path);

// The name offered when a workbook of the older form is saved: the same folder and name with the ending .xlsx, or if there is a file of that name already, the name with " (2)",
// " (3)", and so on before the ending. Never the path it came from.
QString suggestedSaveName(const QString &openedPath);

// The name the person chose, with the ending a workbook of the new form has (what is saved from an old workbook has no macros, so it is never .xlsm): .xlsx stays, .xls and .xlsm
// become .xlsx, and a name with any other ending (or none) has .xlsx added.
QString withWorkbookEnding(const QString &chosen);

// What the person is told before being asked for a name, in plain words: that the workbook is in an older file format, that it is saved as .xlsx under a new name, and that the file is
// not changed; and, if the file has parts this program does not read (charts, macros, ...), that they will not be in the copy.
QString saveAsExplanation(const QString &fileName, const QStringList &notRead = QStringList());

// What the person is told when the name given for the copy is the file the workbook was read from, which is never written to.
QString originalNotWrittenNotice(const QString &fileName);

// What the person is told when a workbook is opened that has parts JeffCrunch does not read yet (charts, pictures, ...); empty if there are none.
QString notReadNotice(const QStringList &notRead);

} // namespace jp::crunch
