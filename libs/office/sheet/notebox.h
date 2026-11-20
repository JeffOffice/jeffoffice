#pragma once
// Where a note's box sits (shared by the notes read from a package and the notes an OpenDocument sheet is read with).

#include <QtGlobal>

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

} // namespace jp::office::sheet
