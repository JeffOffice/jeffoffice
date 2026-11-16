#pragma once
// The data of a pivot cache read from a range of another workbook file (a sheet's cells, or a table or a defined name of that file). The file is read once, read only, with the limits an opened
// file has; what it links to is not followed (its own links stay as it stores them). The cells are given to buildPivotCache as any source is.

#include "sheet/pivot.h"

#include <QHash>
#include <QString>

#include <memory>

namespace jp::office::sheet {

// `path` is the file; `where` is "Sheet1!A1:D100", "A1:D100" (the first sheet of the file), or the name of a table or a defined name. On success `source` has the cells, and `sheet`, `ref`, and `name`
// are what a cache stores of the source (`name` set: `sheet` and `ref` empty). False, with a reason in `why`, for a file that can't be opened (missing, too big, damaged, protected), a sheet or
// name it does not have, or a range past the bounds a pivot table is made within.
bool readPivotExternalSource(const QString &path, const QString &where, PivotSource *source, QString *sheet, QString *ref, QString *name, QString *why);

// What the reads of one refresh may come to together: the sizes of the files opened, the time, and the number of different files (the rest keep their data).
struct PivotExternalLimits {
    qint64 bytes = qint64(1) << 30;
    qint64 millis = 30000;
    int files = 32;
};
// (a test lowers them; a default one puts them back)
void setPivotExternalLimits(const PivotExternalLimits &limits);
// How many files have been opened by these reads in all (for tests).
int pivotExternalFilesOpened();

struct PivotExternalHeld;

// The reads of one refresh: each file is opened once however many caches name it, and all of them are held to the limits above together.
class PivotExternalReads {
public:
    PivotExternalReads();
    ~PivotExternalReads();
    // As readPivotExternalSource; a file that would pass a limit is refused (`why` says so).
    bool read(const QString &path, const QString &where, PivotSource *source, QString *sheet, QString *ref, QString *name, QString *why);

private:
    struct File;
    QHash<QString, std::shared_ptr<File>> m_files;
    QHash<QString, std::shared_ptr<PivotExternalHeld>> m_sheets;
    qint64 m_used = 0, m_deadline = 0;
};

// The address a file is stored under in a cache definition for a person's choice of file: absolute, as a file: address.
QString pivotExternalAddress(const QString &path);

// A target a cache stores, as it is when the workbook is saved in folder `to` instead of `from`: a relative one is made relative to `to` (so that it names the same file); an absolute one, a web
// address, or a file on another computer is as it was.
QString pivotExternalRebased(const QString &target, const QString &from, const QString &to);

} // namespace jp::office::sheet
