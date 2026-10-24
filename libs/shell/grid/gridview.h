#pragma once
// The grid as a widget: a scrolling view of the sheet (GridSheet) drawn by GridPainter, with the selection and the keys and mouse that change it (Navigator), the numbers of the
// selection for the status bar (StatsWalk), and zoom. It holds no cells; the window gives it the data. Scrolling by a row or a column at 100% moves the pixels already on the
// screen and draws only the strip that is left, so a frame costs the cells in a row, not those on the screen. Work that may be long (the edge of the data a Ctrl and an arrow
// look for, the numbers of a whole column) is done a few milliseconds at a time between events.

#include "grid/gridsheet.h"
#include "grid/overlay.h"
#include "grid/painter.h"
#include "grid/selection.h"
#include "grid/stats.h"
#include "grid/textwalk.h"

#include <QAbstractScrollArea>
#include <QInputMethodEvent>
#include <QElapsedTimer>
#include <QTimer>

#include <functional>
#include <memory>
#include <vector>

namespace jp::grid {

class GridView : public QAbstractScrollArea {
    Q_OBJECT
public:
    // Whether a key with these modifiers and this text begins an entry in the cell: one that prints, and is not a shortcut. Ctrl (and Cmd on a Mac) with a character is a shortcut. AltGr,
    // which gives the characters some keyboards have on their letters, arrives as Ctrl and Alt together (Windows); on a Mac, Option gives them, and Cmd with Option is a shortcut.
    // `macKeyboard` says which of the two the keys are of (the platform's, as the grid asks it).
    static bool keyBeginsAnEntry(Qt::KeyboardModifiers modifiers, const QString &text, bool macKeyboard);
    explicit GridView(QWidget *parent = nullptr);
    ~GridView() override;

    GridSheet &sheet() { return m_sheet; }
    const GridSheet &sheet() const { return m_sheet; }
    GridPainter &painter() { return *m_painter; }
    // Gives the view the sheet to show; its geometry (sizes, frozen panes, zoom, where it was scrolled to) is the file's. With `keepView`, what the person has done (the scroll, the
    // zoom, the selection) is kept instead, for the same sheet read whole after its first rows were shown.
    void setData(const GridData &data, bool keepView = false);

    // What the person has done to the view: where it is scrolled, the zoom, the panes held, the gridlines and headings, and the selection.
    struct ViewState {
        Selection selection;
        qint64 scrollX = 0, scrollY = 0;
        qint64 frozenRows = 0, frozenCols = 0;   // where the frozen part ends, from the first row and column of the sheet
        qint64 skipRows = 0, skipCols = 0;       // and the rows and columns before where it begins, which are out of sight
        double zoom = 1.0;
        bool gridlines = true, headings = true;
        bool valid = false;
    };
    ViewState viewState() const;
    void setViewState(const ViewState &state);

    double zoom() const { return m_sheet.layout().zoom(); }
    void setZoom(double zoom);
    bool showGridlines() const { return m_gridlines; }
    void setShowGridlines(bool on);
    bool showHeadings() const { return m_headings; }
    void setShowHeadings(bool on);
    // The first rows and columns held in place, now (a view change, not saved).
    void setFrozen(qint64 rows, qint64 cols);
    // Freezes as people expect: the rows from the first one in view down to (but not including) `row` stay in place, and the columns from the first in view to `col`; rows and columns
    // scrolled out of sight before them stay out of sight, and the part that scrolls begins at `row` and `col`. A row at or above the first in view freezes no rows (and the sheet is
    // left scrolled as it was); likewise columns.
    void freezeAt(qint64 row, qint64 col);
    // Freezes the first row in view (and keeps the columns frozen), the first column in view (and keeps the rows), or lets everything go (the sheet stays scrolled as it was).
    void freezeTopRow();
    void freezeFirstColumn();
    void unfreeze();
    // The first row and column that are shown in the grid's top left: the frozen part's, or if there is none the part that scrolls's.
    qint64 firstRowInView() const;
    qint64 firstColInView() const;

    const Selection &selection() const { return m_selection; }
    void setSelection(const Selection &s);
    void selectCell(qint64 row, qint64 col);
    // A range, from its first cell to its last (a merged range at an edge taken whole); whole rows, whole columns, and everything.
    void selectRange(qint64 row1, qint64 col1, qint64 row2, qint64 col2);
    void selectRows(qint64 from, qint64 to);
    void selectColumns(qint64 from, qint64 to);
    void selectAll();
    // The address of the cell with the cursor ("B7"; a merged range's first cell), and its text as the formula bar shows it: its formula, or what it holds. rangeName() is what
    // is selected ("A1:C5", "A:C", "2:4", or the cell's address).
    QString selectionName() const;
    QString rangeName() const;
    QString activeCellText() const;
    // The cells of the selection as tab-separated text (what is shown in them), for the clipboard; empty, with `tooMany` set, past a million cells. All at once: for a large
    // selection copyWalk() is the way (a walk to be stepped a little at a time; null when the selection holds nothing).
    QString selectionAsText(bool *tooMany = nullptr) const;
    std::unique_ptr<TextWalk> copyWalk() const;
    // The text of the numbers of the selection for the status bar ("" when there are none to show); empty while they are being added up.
    QString statsText() const { return m_statsText; }

    // Scrolls so that the cell is in view; true if it moved.
    bool ensureVisible(qint64 row, qint64 col);
    // The shown rows that are whole in the part of the grid that scrolls (at least one): what Page Down moves by.
    int wholeRowsInView() const;
    void invalidateValues();
    // Values the calculation worked out for cells of a sheet were put in the store the grid reads: when the sheet is the one in view, the texts of those cells are made again and the
    // part of the screen they are in is drawn again (the whole of it, if there are many), and the numbers of the selection are worked out again if one of them is in it. True if
    // the cell with the cursor is one of them (what the formula bar says of it is new). Cost is in proportion to the items.
    bool valuesChanged(int sheetIndex, const std::vector<ValueStore::Item> &items);   // values (or formats) changed: draw again

    // For measuring: the time of each paint, in milliseconds, since the last clear; and the work the last paint did.
    // How long each paint took (milliseconds, by the wall and by the thread's own processor time, which another program using the machine does not change).
    const std::vector<double> &paintTimes() const { return m_paintTimes; }
    const std::vector<double> &paintCpuTimes() const { return m_paintCpuTimes; }
    void clearPaintTimes()
    {
        m_paintTimes.clear();
        m_paintCpuTimes.clear();
    }
    const PaintStats &lastPaintStats() const { return m_painter->stats(); }
    // Scrolls done by moving the pixels already on the screen (and drawing the strip they left), and by drawing the window again: counted, so that a test can see which a scroll was
    // on a window system that may not move pixels (the strip is then drawn as part of a larger piece).
    qint64 blitScrolls() const { return m_blits; }
    qint64 fullScrolls() const { return m_fullScrolls; }
    // True while a walk for the edge of the data or the numbers of a selection is going on.
    bool busy() const { return m_jump || m_statsWalk; }

    // ---- for a window that edits
    // Where a cell is in the view, in the viewport's pixels (a merged range's whole rectangle for a cell in one); empty if the cell is scrolled out of sight.
    QRect cellRect(qint64 row, qint64 col) const;
    // The moves that follow an entry, as the keys make them: Enter and Shift+Enter (down and up, through the cells of a selection), Tab and Shift+Tab (right and left), and an arrow.
    void moveAfterEnter(bool up);
    void moveAfterTab(bool back);
    void moveAfterArrow(int dRow, int dCol);
    // The fill handle: the small square at the corner of a selection of cells, in the viewport's pixels (empty when there is none to drag). Dragging it over cells (along one row or one
    // column) and letting go asks for a fill: the block it began from, and the block with the cells it reached.
    QRect fillHandleRect() const;
    // The cells that were copied or cut have a dashed border that moves; none to show with an empty range (clearCopyBorder).
    void setCopyBorder(const GridRange &range);
    void clearCopyBorder();
    bool hasCopyBorder() const { return m_copyBorder.row1 >= 0; }
    GridRange copyBorder() const { return m_copyBorder; }
    // The border between two rows or columns under a point of the headers (a few pixels either side of it): whether it is a column's (its right edge), and which row or column it ends.
    bool borderAt(const QPoint &p, bool *column, qint64 *index) const;
    // The cell with the cursor offers a list to choose from (a rule of validation): a small button at the right edge of it opens the list (dropDownRequested), as Alt+Down does. -1 for none.
    void setDropDownCell(qint64 row, qint64 col);
    QRect dropDownRect() const;       // the button, in the viewport's pixels (empty when the cell is out of sight or too narrow)
    QRect dropDownCellRect() const;   // and the cell it is on
    // Something drawn over the cells (the objects of a sheet) that also takes the clicks that land on it and the keys while it has something picked. Not owned: the owner takes it away
    // (setOverlay(nullptr)) before it is destroyed.
    void setOverlay(GridOverlay *overlay);
    GridOverlay *overlay() const { return m_overlay; }
    // A hook asked before a click changes the selection: false, and the click is ignored (the window finishes an entry first, and does not go on when it can't).
    void setClickGuard(std::function<bool()> guard) { m_clickGuard = std::move(guard); }

Q_SIGNALS:
    void selectionChanged();
    void zoomChanged(double zoom);
    void statsChanged(const QString &text);
    void contextMenuRequested(const QPoint &globalPos);
    void viewChanged();   // the scroll, frozen panes, or what is shown changed
    // Keys that start an entry: a character that prints (with no Ctrl or Alt) is typed into the cell with the cursor in place of what it holds, and Backspace starts an entry with
    // nothing; F2 and a double click on a cell edit what it holds; Delete clears the selection.
    void typingStarted(const QString &text);
    void editRequested();
    void clearRequested();
    // The fill handle was dragged and let go: the block it began from, the block with the cells it reached, and whether Ctrl was held (a copy, not a series).
    void fillRequested(const jp::grid::GridRange &source, const jp::grid::GridRange &extent, bool ctrl);
    // The button of a list was clicked, or Alt+Down was pressed on a cell that has one.
    void dropDownRequested();
    // Esc took away the dashed border of the cells that were copied or cut.
    void copyBorderCleared();
    // The border of a column (row) was dragged and let go: which, and the size it was given, in the grid's units at 100% (0 hides it).
    void resizeRequested(bool column, qint64 index, int size);
    // A double click on the border at the end of a column (row): fit it to what it holds.
    void fitRequested(bool column, qint64 index);

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    bool focusNextPrevChild(bool) override { return false; }
    void inputMethodEvent(QInputMethodEvent *e) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void scrollContentsBy(int, int) override {}

private:
    class Nav;
    void applySheetView();
    void layoutChanged();
    void updateScrollBars();
    void scrollTo(qint64 x, qint64 y);
    void scrollRows(int rows);
    void scrollColumns(int columns);
    void setSelectionInternal(const Selection &s, bool keepActiveInView = true);
    void showActive();
    void startJump(int dRow, int dCol, bool extend);
    void stepJump();
    void startStats();
    void stepStats();
    void updateHeaderWidth();
    void dragTo(const QPoint &p);
    PaintOptions paintOptions() const;
    void paintOverlay(QPainter *painter, const QRect &clip);
    bool overlayPress(QMouseEvent *e);
    QRect bodyScrollRect(bool vertical) const;

    GridSheet m_sheet;
    std::unique_ptr<GridPainter> m_painter;
    std::unique_ptr<Nav> m_nav;
    Selection m_selection;
    bool m_gridlines = true, m_headings = true;
    // the mouse
    enum class Drag { None, Cells, Rows, Columns, Fill, Resize } m_drag = Drag::None;
    bool m_resizeColumn = false;
    qint64 m_resizeIndex = -1;
    int m_resizeFrom = 0, m_resizeEdge = 0, m_resizeSize0 = 0, m_resizeSize = 0;
    qint64 m_dropRow = -1, m_dropCol = -1;
    GridRange m_fillSource, m_fillExtent;
    bool m_fillCtrl = false;
    GridRange m_copyBorder{-1, -1, -1, -1};
    int m_copyPhase = 0;
    QTimer m_copyTimer;
    QTimer m_autoScroll;
    QPoint m_lastMouse;
    // walks done between events
    std::unique_ptr<JumpWalk> m_jump;
    bool m_jumpExtend = false;
    Selection m_jumpFrom;
    QTimer m_walkTimer;
    std::unique_ptr<StatsWalk> m_statsWalk;
    QString m_statsText;
    int m_headerDigits = 0;
    bool m_syncingBars = false;
    qint64 m_lastScrollX = 0, m_lastScrollY = 0;
    std::vector<double> m_paintTimes, m_paintCpuTimes;
    qint64 m_blits = 0, m_fullScrolls = 0;
    std::function<bool()> m_clickGuard;
    GridOverlay *m_overlay = nullptr;
};

} // namespace jp::grid
