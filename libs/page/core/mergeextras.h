#pragma once
// What a mail merge remembers and makes beyond the merged document: the recipients left out of the list (kept with the document and found again when the list is read again), and one file
// for each recipient (the names of the files, made safe for every system).

#include "core/mergesource.h"

#include <QByteArray>
#include <QStringList>
#include <QVector>
#include <functional>

namespace jp {

// ---- recipients left out
// The most recipients a document remembers as left out, and the most characters of a column's name it keeps. A list of the size a merge allows (100,000) with every recipient left out is still
// bounded by this, and what a file says is bounded before anything is allocated.
constexpr int kMostSavedExclusions = 200000;
constexpr int kMostKeyColumnChars = 200;
constexpr qint64 kMostExclusionBytes = qint64(8) << 20;

// The recipients left out of a list, as the document keeps them: by a key (the value of a chosen column, kept as a short fingerprint so that the document does not hold the recipients' own
// names or addresses), or, where the list has no key column, by their place in the list.
struct MergeExclusions {
    QString keyColumn;           // empty: by place
    qint64 rows = 0;             // how many recipients the list had when this was made
    QString salt;                // 64 hexadecimal digits, made once for the document: the fingerprints are keyed by it, so a list of guesses made elsewhere finds none of them
    QStringList keys;            // by key: the fingerprint of the key of each recipient left out
    QVector<int> places;         // by place: the place in the list as it is read (never as shown) of each recipient left out
    bool isEmpty() const { return keys.isEmpty() && places.isEmpty(); }
    bool operator==(const MergeExclusions &o) const { return keyColumn == o.keyColumn && rows == o.rows && salt == o.salt && keys == o.keys && places == o.places; }
};

// The most recipients a list can have where a document remembers its size, and the length of the salt in digits.
constexpr qint64 kMostSavedListRows = 10000000;
constexpr int kSaltDigits = 64;

// The fingerprint of a key value (the same for "Ann@X.com " and "ann@x.com"): a keyed hash of the trimmed, case-folded value, 16 hexadecimal digits. It is not a secret from a person who has the
// document and guesses an address: that guess can still be tested.
QString mergeKeyFingerprint(const QString &value, const QString &salt);
// A new random salt.
QString newExclusionSalt();
// The recipients `m` leaves out (include is false), by `m.keyColumn` where the list has that column, else by place (`m.place`, where the rows were put in another order). `salt` is the document's
// own where it has one; a new one is made where there is none and somebody is left out.
MergeExclusions exclusionsOf(const MergeSource &m, const QString &salt = QString());
// The bytes a document keeps, and what is read from them (false, nothing read, where they are not what this writes or are over the limits: a size that is not a whole number in range, a place
// outside the list, a key with no salt).
QByteArray exclusionsToBytes(const MergeExclusions &e);
bool exclusionsFromBytes(const QByteArray &bytes, MergeExclusions *out);

struct ExclusionResult {
    int applied = 0;             // recipients left out again
    QString problem;             // not empty: what no longer fits, as a sentence for the person (every recipient the saved list could not place is included)
};
// Leaves out of `m` the recipients `saved` says, matching by the key where there is one and by place otherwise. Whatever does not fit is not guessed: the recipient stays included and
// `problem` says so. `m.keyColumn` becomes the saved one.
ExclusionResult applyExclusions(MergeSource &m, const MergeExclusions &saved);

// ---- one file for each recipient
// The most files one merge makes, and the number over which the person is asked first.
constexpr int kMostMergeFiles = 100000;
constexpr int kAskAboveMergeFiles = 200;
// The most characters of a file's name (without its ending), and the most bytes the whole name is (every system takes 255).
constexpr int kMostMergeNameChars = 80;
constexpr int kMostMergeNameBytes = 200;

// A file name from text: no folder separators or characters a system refuses, no control characters, no leading or trailing dots and spaces, not a name Windows keeps for itself, at most
// kMostMergeNameChars characters. Text with nothing usable gives `fallback`.
QString safeMergeFileName(const QString &text, const QString &fallback = QStringLiteral("Recipient"));
// The pattern an app offers first: Last name, First name where the list has them, else its first column. A pattern is text with {Column} in it.
QString defaultFileNamePattern(const MergeSource &m);
// The name of the file of each row in `rows` (the ending `extension` with its dot, included): the pattern filled from the row with each value made safe, a name that is empty becomes the
// row's number, and a name that came before is numbered ("Smith (2).pdf"; compared without regard to case or to how an accented letter is composed, as the systems that fold case or normalize need).
QStringList mergeFileNames(const MergeSource &m, const QVector<int> &rows, const QString &pattern, const QString &extension);
// Whether `name` is a plain file name that stays in `folder` (no separator, not "." or "..").
bool nameStaysInFolder(const QString &folder, const QString &name);

// How many of the files `names` would make are already in `folder` (a link of that name counts, whether it leads anywhere or not: it is replaced).
int mergeFilesThatExist(const QString &folder, const QStringList &names);
// Writes `bytes` as the file `path`. Where `path` is a symbolic link (leading anywhere or not) the LINK is taken away first and a real file is made in its place; the file it led to is never
// touched. The look and the write follow each other with nothing between them (no turn of the event loop).
bool writeMergeFile(const QString &path, const QByteArray &bytes, QString *error);
// What a run of files came to: how many are written, whether the person stopped it (the files already written stay), and why it stopped where it could not go on.
struct MergeFilesResult {
    int written = 0;
    bool stopped = false;
    QString error;
};
// `make` writes the file of a row at the path it is given (false, with the reason, where it can't). `progress` is called before each file, and stops the run where it gives false (between files:
// a file is never left half made by it).
using MakeMergeFile = std::function<bool(int row, const QString &path, QString *error)>;
MergeFilesResult makeMergeFiles(const QString &folder, const QVector<int> &rows, const QStringList &names, const MakeMergeFile &make, const std::function<bool(int done, int total)> &progress = {});

} // namespace jp
