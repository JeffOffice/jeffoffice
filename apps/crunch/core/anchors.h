#pragma once
// The notes and the links of an open workbook, as the person has them now: every sheet's notes (by cell) and links (by range), read from the file when it was opened, changed by the note and
// link commands (one step of Undo each), moved and dropped with the rows, columns, and cells they are on (and brought back with them by Undo), and moved with cells put in another
// order. Saving asks what to put in the file: the cells and ranges that were touched since the file was read or saved, as they are now; the file's other notes and links are left alone.

#include "sheet/notes.h"
#include "sheet/refshift.h"
#include "sheet/xlsxsave.h"

#include <QStringList>

#include <map>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

namespace jp::crunch {

// One step's changes to the notes and the links of a sheet. A note: the cell, and what was there and what is there (none for no note). Links: those taken out and those put in.
struct SideEdit {
    struct Note {
        quint32 row = 0, col = 0;
        std::optional<jp::office::sheet::NoteInfo> before, after;
    };
    int sheet = 0;
    std::vector<Note> notes;
    std::vector<jp::office::sheet::LinkSpec> linksRemoved, linksAdded;
    bool empty() const { return notes.empty() && linksRemoved.empty() && linksAdded.empty(); }
};

class CellAnchors {
public:
    // The most notes a workbook's sheet and links are kept for (a sheet read with more has the first of them: see the readers' limits).
    void open(std::vector<std::map<quint64, jp::office::sheet::NoteInfo>> notes, std::vector<std::vector<jp::office::sheet::LinkSpec>> links);
    int sheetCount() const { return int(m_sheets.size()); }
    // A sheet that has more notes than are kept: its notes are shown and left as they are, and none is put on it or taken off it.
    void setTruncated(int sheet, bool on) { sheetData(sheet).truncated = on; }
    bool truncated(int sheet) const { return sheet >= 0 && sheet < int(m_sheets.size()) && m_sheets[size_t(sheet)].truncated; }

    const std::map<quint64, jp::office::sheet::NoteInfo> &notes(int sheet) const;
    const jp::office::sheet::NoteInfo *noteAt(int sheet, quint32 row, quint32 col) const;
    const std::vector<jp::office::sheet::LinkSpec> &links(int sheet) const;
    // The link on the cell (the one put in last, if more than one covers it), or null.
    const jp::office::sheet::LinkSpec *linkAt(int sheet, quint32 row, quint32 col) const;
    // The cells of the sheet that have notes, sorted: what the grid marks.
    std::vector<quint64> noteCells(int sheet) const;

    // A step's changes done (forward) or taken back.
    void apply(const SideEdit &edit, bool forward);

    // Rows, columns, or cells were inserted or deleted (`token` names the step): the notes and links follow, and what they were is kept for undoEdit. `sheetNames` are the sheets' names by
    // place, for the places that links name.
    void followEdit(const jp::office::sheet::StructEdit &edit, const QStringList &sheetNames, const void *token);
    // The step is undone: the notes and links are as they were before it (nothing if the step is not known).
    void undoEdit(const void *token, const jp::office::sheet::StructEdit &edit);
    // Cells put in another order (a sort), done or taken back: the notes and the links on one cell go with it.
    void permute(const jp::office::sheet::CellPermute &p, bool forward);
    // A sheet added at the end, and taken away again.
    // (a copy has the links of the sheet it is a copy of; the file's copy takes them from the same place)
    void addSheet(int copyOf = -1);
    void removeLastSheet();

    // What saving needs. `names` as for followEdit.
    void addToSave(jp::office::sheet::SaveChanges *changes) const;
    // The file has been written with them.
    void saved();
    // Whether a save has something to write for notes or links.
    bool touched() const;
    // The cells and ranges (the number of each) the next save writes, for tests.
    size_t touchedNotes() const;
    size_t touchedLinks() const;

private:
    using RangeKey = std::tuple<quint32, quint32, quint32, quint32>;
    struct SheetData {
        std::map<quint64, jp::office::sheet::NoteInfo> notes;
        std::vector<jp::office::sheet::LinkSpec> links;
        std::set<quint64> noteTouched;
        std::set<RangeKey> linkTouched;
        bool truncated = false;
    };
    struct Kept {
        const void *token = nullptr;
        int saves = 0;
        std::map<int, SheetData> before;
    };
    SheetData &sheetData(int sheet);
    static RangeKey keyOf(const jp::office::sheet::CellRange &r) { return {r.row1, r.col1, r.row2, r.col2}; }
    void touch(SheetData &d, const jp::office::sheet::LinkSpec &l) { d.linkTouched.insert(keyOf(l.range)); }

    std::vector<SheetData> m_sheets;
    std::vector<Kept> m_kept;
    int m_saves = 0;
};

} // namespace jp::crunch
