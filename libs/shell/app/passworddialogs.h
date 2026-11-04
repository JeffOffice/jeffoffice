#pragma once
// The two questions about a file's password that every app asks the same way: the password to protect a file with (typed twice, and removable), and the password a protected file
// asks for when it is opened. A password is a string the caller wipes after use; a test stands in for each question with a function.

#include <QString>
#include <atomic>
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

// Runs `work` on a thread of its own while the window goes on (its events are handled, its input is not), with a note and a Cancel button that show if it takes longer than a moment. `work` is
// given a flag it looks at now and then and stops when it is set. True if the work ran to its end, false if the person cancelled. A key made from a password can take many seconds: this is how
// the window is not frozen for them.
bool runWithCancel(QWidget *parent, const QString &text, const std::function<void(const std::atomic<bool> *cancel)> &work);
// For tests: presses the Cancel of the work that is running.
void cancelRunningForTest();

} // namespace jp
