#pragma once
// The saver's view of the rows and columns (docs/jeffcrunch-rowids.md, 4.8 and round 7). The edits made to a workbook since its file was read are replayed once on a map of lines per sheet
// (calc/linemap.h), the ids of each being the places of the file's rows and columns; a row or column of the file is then found by one lookup, and the text of a formula of the file, which
// names rows and columns by the places they had there, is turned into the text it has now by the same rules as a formula held in ids (sheet/formulaframe.h). No cell and no model is needed,
// so the saver can do it on its own thread, from the list of edits alone. No GUI.

#include "sheet/refshift.h"

#include <QString>
#include <QStringList>

#include <memory>
#include <vector>

namespace jp::office::sheet {

class LineReplay {
public:
    // `edits` are the edits to rows and columns in the order they were made (the others are passed over); `sheetNames` are the file's sheet names in order (the sheet of an edit is its place there).
    LineReplay(const std::vector<StructEdit> &edits, const QStringList &sheetNames);
    ~LineReplay();
    LineReplay(const LineReplay &) = delete;
    LineReplay &operator=(const LineReplay &) = delete;

    // Whether every edit was made on the maps. A map that cannot take one (it holds as many runs as it may, or the ids are used up: the window makes the order again then) says no, and the
    // caller does not rewrite from it.
    bool ok() const;
    // Whether the rows or columns of any sheet are now in another order than the file's (rows put in or taken out and not put back).
    bool movesAnything() const;
    // The same for a sheet.
    bool moves(int sheet) const;
    // The names (as the file has them) of the sheets that moves() says yes for.
    QStringList movedSheetNames() const;

    // The text of a formula of the file (no "=" in front; its cell's own, not a shared formula's master) written on `ownSheet`, as the edits leave it. The text is returned as it is when no
    // reference in it names a row or column that moved; a reference to cells that are gone is #REF!, a range that lost an end takes the line next to it, and a reference through several sheets
    // is left alone.
    QString rewrite(const QString &text, int ownSheet) const;

    // Where a row (column) of the file is now; false if it is deleted.
    bool rowNow(int sheet, quint32 row, quint32 *now) const;
    bool columnNow(int sheet, quint32 col, quint32 *now) const;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace jp::office::sheet
