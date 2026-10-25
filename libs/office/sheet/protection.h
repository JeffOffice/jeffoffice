#pragma once
// Protecting a sheet (its locked cells can't be changed, and a list of things is allowed or not) and a workbook's structure (sheets can't be added, deleted, renamed, moved, hidden, or
// shown), with or without a password. A password is never kept as text: what the file holds is a salted hash of it (the strong form: the algorithm, the hash, the salt, and how many
// times the hash was repeated), or the older 16-bit number, which is read and checked but never written for a new password.

#include <QByteArray>
#include <QString>

#include <atomic>

namespace jp::office::sheet {

// The most repeats of the hash a file may ask to be computed to check a password. A file that asks for more is refused with a message (it would hold the program for minutes).
constexpr qint64 kMaxSpinCount = 10000000;
// The repeats a new password is hashed with.
constexpr int kNewSpinCount = 100000;

struct PasswordHash {
    QString algorithm;       // "SHA-512" and the like, for the strong form
    QString hash, salt;      // base64, as the file has them
    qint64 spin = 0;
    QString legacy;          // the 16-bit form, four hex digits, as the file has it
    bool isSet() const { return !hash.isEmpty() || !legacy.isEmpty(); }
    bool operator==(const PasswordHash &o) const { return algorithm == o.algorithm && hash == o.hash && salt == o.salt && spin == o.spin && legacy == o.legacy; }
};

enum class PasswordResult : quint8 {
    Match,
    Wrong,
    Unsupported,   // the file's algorithm is one this program doesn't have
    TooSlow,       // the file asks for more repeats than kMaxSpinCount
    Stopped,       // `stop` was set
};

// Whether `password` is the one the hash was made from. Work counted in repeats of the hash: up to `spin`. Takes about a tenth of a second for the default; run it off the window's
// thread. `stop`, if given, is looked at every few thousand repeats.
PasswordResult checkPassword(const PasswordHash &hash, const QString &password, const std::atomic<bool> *stop = nullptr);
// A new hash for the password: the strong form, SHA-512, a random salt, kNewSpinCount repeats. False if `stop` was set.
bool makePasswordHash(const QString &password, PasswordHash *out, const std::atomic<bool> *stop = nullptr);
// The older 16-bit form of a password, as four upper-case hex digits (for the tests and for checking).
QString legacyPasswordHash(const QString &password);

struct SheetProtection {
    bool enabled = false;
    bool objects = true;     // drawing objects are protected
    bool scenarios = true;
    // What stays allowed on the protected sheet. (In the file each is "locked" when set, and all but the two selections are set unless the file says otherwise.)
    bool allowSelectLocked = true, allowSelectUnlocked = true;
    bool allowFormatCells = false, allowFormatColumns = false, allowFormatRows = false;
    bool allowInsertColumns = false, allowInsertRows = false, allowInsertLinks = false;
    bool allowDeleteColumns = false, allowDeleteRows = false;
    bool allowSort = false, allowFilter = false, allowPivot = false;
    PasswordHash password;
    bool operator==(const SheetProtection &o) const;
    bool operator!=(const SheetProtection &o) const { return !(*this == o); }
};

// The element `sheetProtection` made from the settings (empty when not enabled); `prefix` is the namespace prefix of the sheet's elements ("" or "x:").
QByteArray sheetProtectionElement(const SheetProtection &p, const QByteArray &prefix);

// A workbook's protection of its structure.
struct WorkbookProtection {
    bool lockStructure = false;
    bool lockWindows = false;
    PasswordHash password;
    bool any() const { return lockStructure || lockWindows || password.isSet(); }
    bool operator==(const WorkbookProtection &o) const { return lockStructure == o.lockStructure && lockWindows == o.lockWindows && password == o.password; }
    bool operator!=(const WorkbookProtection &o) const { return !(*this == o); }
};
QByteArray workbookProtectionElement(const WorkbookProtection &p, const QByteArray &prefix);

} // namespace jp::office::sheet
