#pragma once
// "Remove personal information from this file when saving": the one setting all four apps share, its words, and the box that shows it (on the Info page and in Options). What it takes out of a
// file is office/privacy.h's; the apps call that as the file is written.

#include <QString>
#include <QStringList>

class QCheckBox;
class QWidget;

namespace jp::privacy {

// Whether saving takes personal information out of the file (the settings key privacy/removeOnSave; JeffCrunch's older crunch/removePersonalOnSave is read when the new key was never set).
bool removeOnSave();
void setRemoveOnSave(bool on);

// The words of the choice, and of the line that says what it takes out.
QString optionText();
QString optionDetail();

// What a person is told when a file was written (or sent) but the names could not all be taken out of it: the reasons `office::removePersonalFromPackage` gave, in plain sentences.
QString notTakenOutMessage(const QStringList &reasons);

// A box with the words, checked as the setting is. It changes nothing until the one who made it applies it (an Options page does at OK); `objectName` is for tests.
QCheckBox *makeOption(QWidget *parent, const QString &objectName);

} // namespace jp::privacy
