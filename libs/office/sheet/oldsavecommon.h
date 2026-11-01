#pragma once
// What the two writers of copies (.xls and .ods) share: which sheets are written and under what names, a sheet's cells in the order they are written, a text cut to what a cell may
// hold, and the account of what the workbook has that the copy does not.

#include "sheet/oldsave.h"

#include <QMap>
#include <QSet>

#include <vector>

namespace jp::office::sheet::oldsave {

// A sheet that is written: its place in the workbook, the name it has in the copy (made valid and unique), and the name it has in formulas.
struct Written {
    int place = 0;
    QString name;
    QString formulaName;
};

// The sheets of the workbook that are written, in the order of the tabs: worksheets only (a chart sheet, a dialog sheet, and a macro sheet are not cells), at most `limit`, with the
// names made valid (at most 31 characters, none of []:*?/\, none twice in capitals or small letters). What is left out or changed is told in `report`. A workbook with no sheet to
// write gets one empty sheet.
std::vector<Written> sheetsToWrite(const Workbook &book, int limit, int maxName, CopyReport *report);

// The cells of a sheet in the order of rows, then columns.
std::vector<PlacedCell> sortedCells(const Sheet &sheet);

// The text cut to `kMaxCopyTextChars` characters (not between the two halves of a pair); `cut` is set when it was.
QString cutText(const QString &text, bool *cut);

// The text of a cell as a string of the copy.
QString textOf(const Sheet &sheet, const SharedStrings &shared, const Cell &cell);

// What the sheets the copy is made of have that the copy has no place for. Each sheet is noted as it is read; `result` adds what the package holds.
class Features {
public:
    Features(const Workbook &book, CopyKind kind);
    void note(const Sheet &sheet);
    QStringList result() const;

private:
    const Workbook &m_book;
    CopyKind m_kind;
    QSet<QString> m_found;
};

// The commonest of the reasons (each with how many times it came up), as lines for a report.
QStringList topReasons(const QMap<QString, qint64> &reasons, int most = 6);

// The place in the copy's text of a message that names a sheet, in words: "Sheet1: 3 cells past row 65,536".
QString sheetNote(const QString &sheet, const QString &what);

} // namespace jp::office::sheet::oldsave
