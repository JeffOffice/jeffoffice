#pragma once
// Workbooks of the older binary file format (.xls, BIFF5 and BIFF8). JeffCrunch opens them and shows what it reads of them; it saves workbooks as .xlsx only, so a workbook
// that came from an .xls file is saved under a new name with that ending, after the person is told so in plain words, and the file it came from is never written to.

#include <QString>
#include <QStringList>

class QWidget;

namespace jp::crunch {

// Whether the file's name ends as the older binary form's do (.xls, any case).
bool isOlderFormatName(const QString &path);

// The name offered when a workbook of the older form is saved: the same folder and name with the ending .xlsx, or if there is a file of that name already, the name with " (2)",
// " (3)", and so on before the ending. Never the path it came from.
QString suggestedSaveName(const QString &openedPath);

// The name the person chose, with the ending a workbook of the new form has: .xlsx and .xlsm stay, .xls becomes .xlsx, and a name with any other ending (or none) has .xlsx added.
QString withWorkbookEnding(const QString &chosen);

// What the person is told before being asked for a name, in plain words.
QString saveAsExplanation(const QString &fileName);

// What the person is told when a workbook is opened that has parts JeffCrunch does not read yet (charts, pictures, ...); empty if there are none.
QString notReadNotice(const QStringList &notRead);

// Tells the person the explanation and asks for a name (a dialog for files, the .xlsx ending chosen, the name offered); the name with its ending, or an empty string if the person
// cancels. `parent` is the window.
QString askForSaveName(QWidget *parent, const QString &openedPath);

} // namespace jp::crunch
