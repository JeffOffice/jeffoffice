#pragma once
// Where everything is on the screen: the rows and columns (their sizes, the hidden ones), the merged ranges, the first rows and columns held in place, how far the rest has
// been scrolled, the zoom, and the size of the window. From these the grid finds the rectangle of a cell, the cell under a point, and the block of rows and columns each of
// its four parts shows. It draws nothing and holds no cells: the work of a search is that of the axis and merge index (the logarithm of what differs), never the number of
// rows, so a sheet of a million rows scrolls as a sheet of ten.
//
// Sizes are in the grid's units (pixels at 100%) and the zoom turns them to what is on the screen; the headers and the viewport are in screen pixels. A place on the screen is
// the rounded place of the grid's, the same for the end of one cell and the start of the next, so cells never leave a gap or overlap by a pixel at any zoom.
//
// The four parts: the frozen rows and columns (the corner), the frozen rows over the columns that scroll (the top), the frozen columns beside the rows that scroll (the left),
// and the rest (the body).

#include "grid/axis.h"
#include "grid/merges.h"

#include <QPoint>
#include <QRect>
#include <QSize>

namespace jp::grid {

enum class Pane { Corner, Top, Left, Body };

struct Hit {
    enum class Kind { None, Cell, RowHeader, ColumnHeader, Corner };
    Kind kind = Kind::None;
    qint64 row = -1, col = -1;   // the cell, or the row (of a row header) or column (of a column header); a cell in a merged range is the first cell of the range
    int merge = -1;              // the merge the cell is in (MergeIndex), else -1
    Pane pane = Pane::Body;
};

// What one part of the grid shows: its place in the window, and the rows and columns (the shown ones, whole or in part) that are in it. `lastRow < firstRow` for none.
struct PaneView {
    QRect rect;
    qint64 firstRow = 0, lastRow = -1;
    qint64 firstCol = 0, lastCol = -1;
    bool isEmpty() const { return lastRow < firstRow || lastCol < firstCol; }
};

class GridLayout {
public:
    static constexpr double kMinZoom = 0.1, kMaxZoom = 4.0;

    GridLayout();

    Axis rows, cols;
    MergeIndex merges;

    double zoom() const { return m_zoom; }
    void setZoom(double zoom);
    // The headers (the numbers of the rows down the left, and the letters of the columns along the top), in screen pixels; none when not shown.
    void setHeaders(bool shown, int rowHeaderWidth, int columnHeaderHeight);
    int headerWidth() const { return m_headerW; }
    int headerHeight() const { return m_headerH; }
    void setViewport(const QSize &size) { m_viewport = size; }
    QSize viewport() const { return m_viewport; }
    // The first rows and columns held in place (counted from the first of the sheet; a count past the sheet's is the sheet's whole).
    void setFrozen(qint64 rows, qint64 cols);
    qint64 frozenRows() const { return m_frozenRows; }
    qint64 frozenCols() const { return m_frozenCols; }

    // How far the part that scrolls has gone, in the grid's units, from where it begins (the end of the frozen rows or columns); kept between 0 and what lets the last row
    // (or column) be at the top (left) of the part.
    void setScroll(qint64 x, qint64 y);
    qint64 scrollX() const { return m_scrollX; }
    qint64 scrollY() const { return m_scrollY; }
    qint64 maxScrollX() const;
    qint64 maxScrollY() const;
    // Scrolls as little as it takes for the cell to be whole in the part that scrolls (a cell in a frozen row or column needs none in that direction; one too large to fit
    // has its start in view); true if it moved.
    bool ensureVisible(qint64 row, qint64 col);

    // The rectangle of the cell on the screen (it may be partly or wholly outside the window), and of a block of cells. The cell of a merged range is the range's.
    QRect cellRect(qint64 row, qint64 col) const;
    QRect rangeRect(qint64 row1, qint64 col1, qint64 row2, qint64 col2) const;
    // The place of a row's top (columns' left) edge on the screen, in the part the row (column) is in.
    int yOf(qint64 row) const;
    int xOf(qint64 col) const;
    // The cell, or header, at a point of the window; a point past the last row or column (or on a hidden one's edge) is none.
    Hit hit(const QPoint &point) const;
    PaneView pane(Pane which) const;
    // The whole of what the grid shows of its rows and columns, left to right and top to bottom, including the frozen ones: where the cells are.
    QRect gridRect() const;

private:
    // The place on the screen, in one direction, of a place of the grid's.
    int screenX(qint64 logical, bool frozen) const;
    int screenY(qint64 logical, bool frozen) const;
    qint64 bodyOriginX() const;   // the grid's place at the left of the part that scrolls
    qint64 bodyOriginY() const;
    int frozenWidth() const;      // screen pixels
    int frozenHeight() const;

    double m_zoom = 1.0;
    int m_headerW = 0, m_headerH = 0;
    QSize m_viewport;
    qint64 m_frozenRows = 0, m_frozenCols = 0;
    qint64 m_scrollX = 0, m_scrollY = 0;
};

} // namespace jp::grid
