#pragma once
// The two questions about a file's password that every app asks the same way: the password to protect a file with (typed twice, and removable), and the password a protected file
// asks for when it is opened. A password is a string the caller wipes after use; a test stands in for each question with a function.

#include <QString>
#include <functional>

class QWidget;

namespace jp {

enum class NewPasswordChoice { Cancelled, Set, Removed };

// Asks for the password to protect the file with. `hasPassword` says the file has one now (the box then offers to remove it). With `Set`, `password` holds what was typed.
NewPasswordChoice askNewFilePassword(QWidget *parent, bool hasPassword, QString *password);
// Asks for the password to open `fileName`; `wrongBefore` says the last one tried was wrong. False if cancelled.
bool askOpenFilePassword(QWidget *parent, const QString &fileName, bool wrongBefore, QString *password);

// A test's answers (set to an empty function: the questions are asked as usual).
void setNewPasswordStandIn(const std::function<NewPasswordChoice(bool hasPassword, QString *password)> &standIn);
void setOpenPasswordStandIn(const std::function<bool(const QString &fileName, bool wrongBefore, QString *password)> &standIn);

} // namespace jp
