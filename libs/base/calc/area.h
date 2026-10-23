#pragma once
// A block of cells, as the engine names one: what a reference stands for, and what the dependency graph is told a formula reads.

#include <QtGlobal>

namespace jp::calc {

// A block of cells of one sheet (or of several, for a 3-D reference: sheets sheet1 through sheet2 by their place); rows and columns counted from 0.
struct CellArea {
    qint32 sheet1 = 0, sheet2 = 0;
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    qint64 rows() const { return qint64(row2) - qint64(row1) + 1; }
    qint64 cols() const { return qint64(col2) - qint64(col1) + 1; }
    qint64 cells() const { return rows() * cols() * (qint64(sheet2) - qint64(sheet1) + 1); }
    bool single() const { return row1 == row2 && col1 == col2 && sheet1 == sheet2; }
    bool operator==(const CellArea &o) const { return sheet1 == o.sheet1 && sheet2 == o.sheet2 && row1 == o.row1 && col1 == o.col1 && row2 == o.row2 && col2 == o.col2; }
    bool operator<(const CellArea &o) const
    {
        if (sheet1 != o.sheet1) return sheet1 < o.sheet1;
        if (sheet2 != o.sheet2) return sheet2 < o.sheet2;
        if (row1 != o.row1) return row1 < o.row1;
        if (col1 != o.col1) return col1 < o.col1;
        if (row2 != o.row2) return row2 < o.row2;
        return col2 < o.col2;
    }
};

} // namespace jp::calc
