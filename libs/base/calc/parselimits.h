#pragma once
// How far the rows and columns of a formula's references reach (see ParseOptions in calc/formula.h). No GUI.

#include <QtGlobal>

namespace jp::calc {

// The grid's size: 16,384 columns (A to XFD) and 1,048,576 rows.
constexpr quint32 kGridCols = 16384;
constexpr quint32 kGridRows = 1048576;

// How far a row and a column may reach in a reference's text. A person types within the grid; the text a formula in the id frame is held as has the ids of rows and columns in it
// (docs/jeffcrunch-rowids.md, 4.4), which an inserted row or column makes larger than the grid: rows to 2^24 and columns to 65,536 (letters to CRXP).
struct ParseLimits {
    quint32 rows = kGridRows, cols = kGridCols;   // how many: the last row is rows - 1
    static ParseLimits grid() { return ParseLimits(); }
    static ParseLimits ids() { return ParseLimits{quint32(1) << 24, quint32(1) << 16}; }
    bool wide() const { return rows > kGridRows || cols > kGridCols; }
    bool operator==(const ParseLimits &o) const { return rows == o.rows && cols == o.cols; }
};

} // namespace jp::calc
