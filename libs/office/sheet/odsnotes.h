#pragma once
// The box of a note in an OpenDocument spreadsheet and in the model: the file gives an annotation a width, a height, and a position (svg:width, svg:height, svg:x, svg:y: lengths, from the
// sheet's top left), the model a corner as a cell counted from the note's own and a distance in pixels (see notebox.h). Both ways go through the sheet's grid of column widths and row heights.

#include "sheet/notebox.h"
#include "sheet/odsdraw.h"

#include <QString>

namespace jp::office::sheet {

// A length as the standard writes it ("2.5cm", "10pt", "0.5in", "12mm", "1pc", "16px") in points. False for a text that is not one: no unit or an unknown one ("em", "%"), not a number (nan, inf),
// or too large to be a place on a sheet.
bool odsLengthPoints(const QString &text, double *points);

// The box of a note from the four lengths of its annotation (in points), for the note on cell (row, col). Not set when the file gives no size, a size that is not a positive one, or a place
// above or to the left of the sheet.
NoteBox noteBoxFromOds(double x, double y, double width, double height, quint32 row, quint32 col, const odsdraw::Grid &grid);
// The four lengths (in points) of a box for the note on cell (row, col). False for a box that is not set or has no size.
bool noteBoxToOds(const NoteBox &box, quint32 row, quint32 col, const odsdraw::Grid &grid, double *x, double *y, double *width, double *height);

} // namespace jp::office::sheet
