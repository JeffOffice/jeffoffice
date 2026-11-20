#pragma once
// Where a note's box sits (shared by the notes read from a package and the notes an OpenDocument sheet is read with).

#include <QtGlobal>

#include <algorithm>

namespace jp::office::sheet {

// Where a note's box sits when the file says so: the corners as the file's anchor has them, a cell (counted from the note's own cell) and a distance from its top left in pixels, for the
// box's top left and for its bottom right. A note with none (`set` false) is drawn where the program puts it.
struct NoteBox {
    bool set = false;
    int row1 = 0, col1 = 0, y1 = 0, x1 = 0;
    int row2 = 0, col2 = 0, y2 = 0, x2 = 0;
    bool operator==(const NoteBox &o) const
    {
        return set == o.set && (!set || (row1 == o.row1 && col1 == o.col1 && y1 == o.y1 && x1 == o.x1 && row2 == o.row2 && col2 == o.col2 && y2 == o.y2 && x2 == o.x2));
    }
};
// The most cells and pixels a corner of the box may be from the note's cell.
constexpr int kMaxNoteBoxCells = 100000;
constexpr int kMaxNoteBoxPixels = 100000;

// The box moved down and across, whole, by as many cells as it reaches above or to the left of the sheet's first row and column (its cells are counted from the note's own cell (row, col)):
// its size in cells and its distances into them are kept, so the box is the same size and a place that exists.
inline NoteBox noteBoxInsideSheet(const NoteBox &box, quint32 row, quint32 col)
{
    NoteBox b = box;
    if (!b.set) return b;
    const qint64 down = std::max<qint64>(0, -(qint64(row) + std::min(b.row1, b.row2))), across = std::max<qint64>(0, -(qint64(col) + std::min(b.col1, b.col2)));
    b.row1 += int(down);
    b.row2 += int(down);
    b.col1 += int(across);
    b.col2 += int(across);
    return b;
}

} // namespace jp::office::sheet
