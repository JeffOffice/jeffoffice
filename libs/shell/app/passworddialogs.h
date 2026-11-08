#pragma once
// The two questions about a file's password that every app asks the same way: the password to protect a file with (typed twice, and removable), and the password a protected file
// asks for when it is opened. A password is a string the caller wipes after use; a test stands in for each question with a function.

#include "io/pdfcrypt.h"

#include <QByteArray>
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

// Asks how to protect a PDF that is being made: a password to open it, and, if wanted, a second to lift limits on printing, copying, and changing. `hasPassword` says one is set now (the box
// then offers to remove it, and shows what is set). With `Set`, `protection` holds what was typed. The PDF is made once; nothing here is kept.
NewPasswordChoice askPdfProtection(QWidget *parent, bool hasPassword, PdfProtection *protection);

// How a PDF that may have a password was opened.
enum class PdfUnlock { Opened, Cancelled, Unreadable };
// Opens `bytes` for the file named `fileName`: with no password if it needs none, else asking (again, when the one typed was wrong) until it opens or the person cancels. `password` holds
// the one that worked, for the caller to open the file again with, and to wipe.
PdfUnlock unlockPdf(QWidget *parent, const QString &fileName, const QByteArray &bytes, QString *password);

// A test's answers (set to an empty function: the questions are asked as usual).
void setNewPasswordStandIn(const std::function<NewPasswordChoice(bool hasPassword, QString *password)> &standIn);
void setPdfProtectionStandIn(const std::function<NewPasswordChoice(bool hasPassword, PdfProtection *protection)> &standIn);
void setOpenPasswordStandIn(const std::function<bool(const QString &fileName, bool wrongBefore, QString *password)> &standIn);

// Runs `work` on a thread of its own while the window goes on (its events are handled, its input is not), with a note and a Cancel button that show if it takes longer than a moment. `work` is
// given a flag it looks at now and then and stops when it is set. True if the work ran to its end, false if the person cancelled. A key made from a password can take many seconds: this is how
// the window is not frozen for them.
bool runWithCancel(QWidget *parent, const QString &text, const std::function<void(const std::atomic<bool> *cancel)> &work);
// For tests: presses the Cancel of the work that is running.
void cancelRunningForTest();

// A file protected with a password becomes the package inside it: the password that files protected only against changes by accident have is tried first, then the person is asked (above),
// as often as it takes, with the key made on a thread of its own (runWithCancel). `bytes` is the package afterwards; `kept` the password that opened it and `wasProtected` true; a file that is not
// protected is left as it is (Opened). A reason in `error` goes with Failed (a plain sentence); Cancelled asks for no message. WindowGone: `parent` was deleted meanwhile, and nothing of it may be touched.
enum class PackageUnlock { Opened, Cancelled, Failed, WindowGone };
PackageUnlock unlockProtectedPackage(QWidget *parent, const QString &fileName, QByteArray *bytes, QString *kept, bool *wasProtected, QString *error);

} // namespace jp
