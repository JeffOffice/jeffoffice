#pragma once
// What the readers of the three kinds of workbook share about files protected with a password: how a refusal is told, and a quick check of a password against a file.

#include "sheet/sheetreader.h"

#include "office/officecrypt.h"

#include <QString>

#include <functional>

namespace jp::office::sheet {

// Tries what opens a protected file with the password in `limits` (when none was given, with the one that files protected only against changes by accident have), and says why when it
// does not: in `error`, and in limits.passwordNeed. True if `attempt` came back Ok.
bool passwordGate(const Limits &limits, QString *error, const std::function<CryptResult(const QString &)> &attempt);

// A password tried for every protected file that is opened with none given in its Limits, for the command-line tools (which read it from the environment). Empty: none.
void setFallbackPassword(const QString &password);

// Whether the file at `path` is protected with a password, and whether `password` is it, without opening the workbook (only the password is tried: the cost of a few hundred
// thousand hashes). None: the file is not protected, or the password given (or, when none, the default one) opens it. Needed: it is protected and no password was given.
// Wrong: it is protected and the one given is not it.
PasswordNeed passwordCheck(const QString &path, const QString &password);

} // namespace jp::office::sheet
