#pragma once
// The workbooks a workbook's formulas read through links (`[1]Prices!A1`, `SUM('[2]Q1'!B2:B9)`, `[1]!Rate`): what each link holds in the file (where it points, and the values of the cells the
// formulas read, its cache), and the source file itself, read only, in the background, when the link is live. A source is read once and kept as long as the file does not change (its size and
// time are looked at now and then); it is only read, never written, and nothing in it runs. What a source gives is the values its file stored: a formula in the source that reads a third
// workbook keeps its stored value, so a loop of links ends where it begins. A source that is not there, is too big, or takes too long gives the values the link's cache holds, and a cell the
// cache does not hold is not answered (the formula keeps what the file stored).
//
// All of it may be asked from the thread that works formulas out while a window asks for the state of the links.

#include "calc/value.h"
#include "sheet/xlsxsave.h"

#include <QDateTime>
#include <QString>
#include <QStringList>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace jp::office::sheet {

class Workbook;

// The most a source file may be, and the most time reading it may take.
constexpr qint64 kMaxExternalFileBytes = qint64(512) << 20;
constexpr qint64 kMaxExternalReadMillis = 30000;
// The most cells one reference to a source gives (a whole column of a sheet is as many rows as the sheet holds).
constexpr qint64 kMaxExternalRefCells = 2000000;

class ExternalBooks {
public:
    ExternalBooks();
    ~ExternalBooks();
    ExternalBooks(const ExternalBooks &) = delete;
    ExternalBooks &operator=(const ExternalBooks &) = delete;

    // The links of an XML-form workbook, in the order of the numbers its formulas use (null when it has none). `baseDir` is the folder the workbook's file is in (what a relative address is
    // from); `fileTime` the time of the file (a source newer than this, never read, is "newer").
    static std::shared_ptr<ExternalBooks> read(const Workbook &book, const QString &baseDir, const QDateTime &fileTime = QDateTime());
    // A link made by hand (a test, or a link to a workbook that is open): the number it has.
    int addLink(const QString &target, const QStringList &sheetNames = QStringList());

    int count() const;
    // The address the link points to, as the file (or Change Source) has it; and the file on this computer it names (empty for a web address, or none).
    QString target(int index) const;
    QString localPath(int index) const;
    void setBaseDir(const QString &dir);
    // Change Source: the link points at another address from now (the source is read again, and the cache stays as it is).
    void setTarget(int index, const QString &target);

    enum class State { Ok, Missing, Newer, Unreadable };
    // Ok: the file is there and has not changed since its values were read (or the workbook was last saved); Missing: no such file (not a local one, or none); Newer: the file changed after;
    // Unreadable: it is there and could not be read (too big, damaged, too slow).
    State state(int index) const;

    // Live: the formulas read the source file (else the link's cache). A link becomes live when its values are updated.
    bool live(int index) const;
    void setLive(int index, bool on);
    // How often the source file's size and time are looked at while it is asked for (milliseconds; 500 unless a test says 0).
    void setCheckInterval(int millis);
    // Forgets what was read of the source (it is read again when next asked).
    void invalidate(int index);
    // Reads the source now (when live or not) to see that it can be: true, or false with the reason.
    bool probe(int index, QString *why = nullptr);

    // What a reference as a formula writes it is (`[1]Sheet1!A1:B9`, `'[1]My Sheet'!$A:$A`, `[1]!Name`): a value for a cell, an array for a range (a whole column or row is the used part
    // of the sheet; a reference through several sheets stacks them), false when it can't be told (the link has no such source, no cache for it, no such sheet).
    // `fromFile` is set to true when the answer came from the source file, false when from the cache.
    bool resolve(const QString &text, jp::calc::Value *out, bool *fromFile = nullptr) const;

    // The number of the link a text of a reference names (0 for none).
    static int linkNumber(const QString &text);

    // The cells to put in the link's part: those the formulas read from the live source since it was read, with what the source has now (an empty list for a link that was not read).
    std::vector<SaveChanges::ExternalCachedCell> readCells(int index) const;
    // The names of the sheets of the live source that formulas read (for the part of a link that is new).
    QStringList sheetsRead(int index) const;
    // The number of times a source file was read (a test counts them: a source is read once, and again when the file changes).
    int fileReads() const;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace jp::office::sheet
