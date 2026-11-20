#pragma once
// Where a note's box is, as the file's anchor has it (cells and pixels from the note's own cell: sheet/notes.h NoteBox) and as the grid draws it (a rectangle from the note's cell's top left, in
// pixels at 100%), worked out from the sheet's rows and columns. The two are the same box; a hidden row or column has no size, so a corner in it is at its edge.

#include "grid/axis.h"
#include "sheet/notes.h"

#include <QRect>

#include <algorithm>

namespace jp::crunch {

inline QRect noteBoxPixels(const jp::office::sheet::NoteBox &b, const jp::grid::Axis &liveRows, const jp::grid::Axis &liveCols, quint32 row, quint32 col)
{
    // (a view whose frozen rows begin below the first has the rows above them skipped, which its axes do not find: the box is worked out at the places of all of them)
    jp::grid::Axis rows = liveRows, cols = liveCols;
    rows.setSkipped(0);
    cols.setSkipped(0);
    auto at = [](const jp::grid::Axis &axis, qint64 cell, int offset) {
        if (axis.count() <= 0) return qint64(offset);
        cell = std::clamp<qint64>(cell, 0, axis.count() - 1);
        return axis.position(cell) + offset;
    };
    const qint64 originX = cols.count() > 0 ? cols.position(std::min<qint64>(col, cols.count() - 1)) : 0;
    const qint64 originY = rows.count() > 0 ? rows.position(std::min<qint64>(row, rows.count() - 1)) : 0;
    const qint64 x1 = at(cols, qint64(col) + b.col1, b.x1) - originX, y1 = at(rows, qint64(row) + b.row1, b.y1) - originY;
    const qint64 x2 = at(cols, qint64(col) + b.col2, b.x2) - originX, y2 = at(rows, qint64(row) + b.row2, b.y2) - originY;
    return QRect(int(std::clamp<qint64>(x1, -10000000, 10000000)), int(std::clamp<qint64>(y1, -10000000, 10000000)), int(std::clamp<qint64>(std::max<qint64>(1, x2 - x1), 1, 10000000)),
                 int(std::clamp<qint64>(std::max<qint64>(1, y2 - y1), 1, 10000000)));
}

inline jp::office::sheet::NoteBox noteBoxFromPixels(const QRect &rel, const jp::grid::Axis &liveRows, const jp::grid::Axis &liveCols, quint32 row, quint32 col)
{
    jp::grid::Axis rows = liveRows, cols = liveCols;
    rows.setSkipped(0);
    cols.setSkipped(0);
    // (a place as a cell, counted from the note's, and the pixels into it; before the sheet is the first cell's edge, past it is the last cell's, as far as that goes)
    auto cellAt = [](const jp::grid::Axis &axis, qint64 place, qint64 own, int *cell, int *offset) {
        if (axis.count() <= 0) {
            *cell = 0;
            *offset = int(std::max<qint64>(0, place));
            return;
        }
        place = std::max<qint64>(0, place);
        qint64 c = axis.indexAt(place);
        if (c < 0) c = axis.count() - 1;
        *cell = int(std::clamp<qint64>(c - own, -jp::office::sheet::kMaxNoteBoxCells, jp::office::sheet::kMaxNoteBoxCells));
        *offset = int(std::clamp<qint64>(place - axis.position(c), 0, jp::office::sheet::kMaxNoteBoxPixels));
    };
    jp::office::sheet::NoteBox b;
    b.set = true;
    const qint64 originX = cols.count() > 0 ? cols.position(std::min<qint64>(col, cols.count() - 1)) : 0;
    const qint64 originY = rows.count() > 0 ? rows.position(std::min<qint64>(row, rows.count() - 1)) : 0;
    cellAt(cols, originX + rel.left(), col, &b.col1, &b.x1);
    cellAt(rows, originY + rel.top(), row, &b.row1, &b.y1);
    cellAt(cols, originX + rel.left() + std::max(1, rel.width()), col, &b.col2, &b.x2);
    cellAt(rows, originY + rel.top() + std::max(1, rel.height()), row, &b.row2, &b.y2);
    return b;
}

} // namespace jp::crunch
