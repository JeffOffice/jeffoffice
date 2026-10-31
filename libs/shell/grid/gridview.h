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
    // Rows or columns of the sheet were put in or taken out (its cells already are): the picture follows in place. The sizes move with their lines; the selection follows the cells it was
    // on (rows put in at the selection are selected, as the lines taken out of it are replaced by the lines that move up); the part that scrolls stays at the row (column) it began at; and
    // only what the edit changed is drawn again (the lines from the one before the first changed to the end of the window; nothing for an edit below the window). False, with nothing
    // changed, when the sheet has to be shown again with setData.
    bool followEdit(const jp::office::sheet::StructEdit &edit);
    // What the paints have drawn, for a test: the rectangle the last asked for, and the pixels of all of them.
    QRect lastPaintRect() const { return m_lastPaintRect; }
    qint64 paintedPixels() const { return m_paintedPixels; }
    // Everything drawn since the log was cleared, as a region.
    // The rectangle the last followEdit asked to have drawn (empty for none).
    QRect lastEditBand() const { return m_lastEditBand; }
    QRegion paintedRegion() const { return m_paintedRegion; }
    void clearPaintedRegion() { m_paintedRegion = QRegion(); }

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
    bool borderAt(const QPoint &p, bool *column, qint64 *index) const;   // (a point of the viewport's pixels)
    // The cell with the cursor offers a list to choose from (a rule of validation): a small button at the right edge of it opens the list (dropDownRequested), as Alt+Down does. -1 for none.
    void setDropDownCell(qint64 row, qint64 col);
    QRect dropDownRect() const;       // the button, in the viewport's pixels (empty when the cell is out of sight or too narrow)
    QRect dropDownCellRect() const;   // and the cell it is on
    // Cells marked as what a search found (Go To Special): tinted over the cells, apart from the selection. An empty list takes the marks away.
    // ---- notes (gridview_notes.cpp)
    // A note that is shown all the time: the cell it is on, and its words (drawn in a box beside the cell, for those in view).
    struct ShownNote {
        qint64 row = 0, col = 0;
        QString text;
    };
    // The cells that have notes (sorted by row, then column; each is `(row << 32) | col`): a small mark is drawn in the corner of each that is in view.
    void setNoteCells(std::vector<quint64> sortedKeys);
    size_t noteCellCount() const { return m_noteCells.size(); }
    // How many boxes of shown notes have been laid out so far (for tests: a box is laid out once for a zoom, not at every paint).
    qint64 shownNoteLayouts() const { return m_shownLayouts; }
    void setShownNotes(std::vector<ShownNote> notes);
    // The cell the pointer is on (-1 for none), and the place a note about it is to appear (the cell's right edge, in the viewport's pixels).
    qint64 hoverRow() const { return m_hoverRow; }
    qint64 hoverCol() const { return m_hoverCol; }
    QRect cellRectAt(qint64 row, qint64 col) const { return cellRect(row, col); }

    void setMarks(const std::vector<GridRange> &ranges);
    const std::vector<GridRange> &marks() const { return m_marks; }
    // Notes over the cells while a person checks formulas: cells outlined as circular, and arrows from the cells a formula reads to the formula (or from a cell to the formulas that read
    // it). An arrow starts at a cell or a block (a dot where it starts) and ends at a cell (the head); an end that is on another sheet is a small sheet icon beside the other end. The
    // notes are not the sheet's. At most kMaxCheckArrows arrows and kMaxCheckCircles outlined cells are kept.
    struct CheckArrow {
        GridRange from;                   // the cell or block it starts at (ignored with fromOtherSheet)
        qint64 toRow = 0, toCol = 0;      // the cell it ends at (ignored with toOtherSheet)
        bool fromOtherSheet = false;      // it starts on another sheet: an icon stands for the start, beside the end
        bool toOtherSheet = false;        // it ends on another sheet: an icon stands for the end, beside the start
    };
    static constexpr size_t kMaxCheckArrows = 2000;
    static constexpr size_t kMaxCheckCircles = 10000;
    void setCheckMarks(std::vector<GridRange> circles, std::vector<CheckArrow> arrows);
    const std::vector<CheckArrow> &checkArrows() const { return m_checkArrows; }
    const std::vector<GridRange> &checkCircles() const { return m_checkCircles; }
    // Buttons on cells (the drop-downs of a filter's header row): one at the bottom right of each cell, drawn as a funnel when `filtered`, with an arrow up or down when the cells are sorted
    // by it (`sort` 1 or -1). A click on one is told by headerButtonClicked instead of selecting the cell. At most kMaxHeaderButtons.
    struct HeaderButton {
        qint64 row = 0, col = 0;
        bool filtered = false;
        int sort = 0;
    };
    static constexpr size_t kMaxHeaderButtons = 16384;
    void setHeaderButtons(std::vector<HeaderButton> buttons);
    size_t headerButtonCount() const { return m_headerButtons.size(); }
    QRect headerButtonRect(qint64 row, qint64 col) const;   // in the viewport's pixels; empty when the cell is out of sight or too small
    // Lines where the pages of a printed sheet begin: a horizontal line above row `before` across the columns `from` to `to` (a row break), or a vertical line left of column `before`
    // across the rows `from` to `to`; a break a person set is drawn solid and one the program worked out dashed. Each list is in order of `before`. Drawn over the cells, clipped to
    // what shows; none of it is the sheet's.
    struct PageBreakLine {
        qint64 before = 0, from = 0, to = 0;
        bool manual = false;
    };
    void setPageBreaks(std::vector<PageBreakLine> rows, std::vector<PageBreakLine> cols);
    const std::vector<PageBreakLine> &pageBreakRows() const { return m_breakRows; }
    const std::vector<PageBreakLine> &pageBreakCols() const { return m_breakCols; }
    bool hasPageBreaks() const { return !m_breakRows.empty() || !m_breakCols.empty(); }
    // The sheet shown as the pages it prints on: the cells that print as they are and the rest shaded, each page numbered over its cells, the lines where pages begin and the edges of the
    // printed blocks (`areas`) grabbed and dragged. A page is the block of cells it prints (its number is what its fields show). Nothing here is the sheet's; the window gives the lists
    // and does what a drag asks for. With `editable` off (breaks set by hand are not used when the sheet is fitted to pages) the lines and edges cannot be dragged.
    struct BreakPage {
        qint64 row1 = 0, row2 = -1, col1 = 0, col2 = -1;
        int number = 1;
    };
    void setBreakView(bool on);
    bool breakView() const { return m_breakView; }
    void setBreakPages(std::vector<GridRange> areas, std::vector<BreakPage> pages, bool editable);
    const std::vector<GridRange> &breakAreas() const { return m_breakAreas; }
    // What is under a point of the viewport in the break view: a line where a page begins, or an edge of a printed block (0 top, 1 bottom, 2 left, 3 right).
    struct BreakGrab {
        enum class Kind { None, Line, Edge } kind = Kind::None;
        bool column = false;     // a line between columns
        qint64 before = 0;       // the row (column) the page begins at
        int area = -1, side = 0;
    };
    BreakGrab breakGrabAt(const QPoint &p) const;
    // Something drawn over the cells (the objects of a sheet) that also takes the clicks that land on it and the keys while it has something picked. Not owned: the owner takes it away
    // (setOverlay(nullptr)) before it is destroyed.
    void setOverlay(GridOverlay *overlay);
    GridOverlay *overlay() const { return m_overlay; }
    // The cells can be acted on (the usual): when not (a chart sheet, which shows a chart and no cells) there is no cell cursor, and a click, a key, a double click, and an input method
    // do nothing to the cells.
    void setCellsEnabled(bool enabled);
    bool cellsEnabled() const { return m_cellsEnabled; }
    // A hook asked before a click changes the selection: false, and the click is ignored (the window finishes an entry first, and does not go on when it can't).
    void setClickGuard(std::function<bool()> guard) { m_clickGuard = std::move(guard); }
    // Asked first of a click on a cell (its row and column): true when it was used for something else (a cell pointed at while a formula is typed), and the click then does nothing more.
    void setPointGuard(std::function<bool(qint64, qint64)> guard) { m_pointGuard = std::move(guard); }

Q_SIGNALS:
    void selectionChanged();
    // The selection is where the person left it: a key moved it, or the button was let go after a click or a drag (during a drag it is only changed).
    void selectionSettled();
    void zoomChanged(double zoom);
    void statsChanged(const QString &text);
    void contextMenuRequested(const QPoint &globalPos);
    // The pointer moved onto another cell (row and column -1 when it left the cells), and a cell was clicked (pressed and let go, without moving to another).
    void hoverCellChanged(qint64 row, qint64 col);
    void cellClicked(qint64 row, qint64 col, Qt::KeyboardModifiers modifiers);
    void viewChanged();   // the scroll, frozen panes, or what is shown changed
    // Something the rules of conditional formatting could not do for want of room (a limit): a sentence for the status bar, said once.
    void noticeRaised(const QString &text);
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
    // A button put on a cell (setHeaderButtons) was clicked.
    void headerButtonClicked(qint64 row, qint64 col);
    // The border of a column (row) was dragged and let go: which, and the size it was given, in the grid's units at 100% (0 hides it).
    void resizeRequested(bool column, qint64 index, int size);
    // A double click on the border at the end of a column (row): fit it to what it holds.
    void fitRequested(bool column, qint64 index);
    // In the break view, a line where a page begins was dragged and let go: the row (column) it began at, and the row (column) it was let go at (-1: let go away from the printed cells, so it is
    // to be taken away).
    void breakMoveRequested(bool column, qint64 from, qint64 to);
    // An edge of a printed block was dragged and let go: which block, which side (0 top, 1 bottom, 2 left, 3 right), and the row (column) the edge is now at (the first row of the block for the
    // top, the last for the bottom).
    void printEdgeRequested(int area, int side, qint64 to);

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
    // A sheet that reads right to left has its columns laid out as ever and drawn mirrored: everything inside the view is in the layout's places, and what comes in from the screen (the mouse, the
    // area to draw) and goes out to it (a rectangle to draw again, a cell's place) is mirrored at the edge. `mirror` is its own inverse.
    bool mirrored() const { return m_sheet.rightToLeft(); }
    QRect mirror(const QRect &r) const { return mirrored() && !r.isEmpty() ? QRect(viewport()->width() - r.right() - 1, r.top(), r.width(), r.height()) : r; }
    QPoint mirror(const QPoint &p) const { return mirrored() ? QPoint(viewport()->width() - 1 - p.x(), p.y()) : p; }
    QRegion mirror(const QRegion &region) const
    {
        if (!mirrored()) return region;
        QRegion out;
        for (const QRect &r : region) out += mirror(r);
        return out;
    }
    QRect cellRectInLayout(qint64 row, qint64 col) const;
    QRect fillHandleRectInLayout() const;
    QRect dropDownRectInLayout() const;
    QRect headerButtonRectInLayout(qint64 row, qint64 col) const;
    void paintMarks(QPainter *painter, const QRect &clip);
    void paintNotes(QPainter *painter, const QRect &clip);
    void leaveEvent(QEvent *e) override;
    void paintChecks(QPainter *painter, const QRect &clip);
    bool headerButtonPress(const QPoint &layoutPoint);
    bool borderAtInLayout(const QPoint &p, bool *column, qint64 *index) const;
    void paintOverlay(QPainter *painter, const QRect &clip);
    void paintPageBreaks(QPainter *painter, const QRect &clip);
    void paintBreakView(QPainter *painter, const QRect &clip);
    bool breakPress(const QPoint &lp);
    bool breakMove(const QPoint &lp);
    bool breakRelease();
    qint64 breakTargetAt(const QPoint &lp, bool column, bool *inside) const;
    bool overlayPress(QMouseEvent *e);
    bool overlayMove(QMouseEvent *e);
    bool overlayRelease(QMouseEvent *e);
    bool overlayContext(QMouseEvent *e);
    bool overlayDoubleClick(QMouseEvent *e);
    QRect bodyScrollRect(bool vertical) const;

    GridSheet m_sheet;
    std::unique_ptr<GridPainter> m_painter;
    std::unique_ptr<Nav> m_nav;
    Selection m_selection;
    bool m_gridlines = true, m_headings = true;
    // the mouse
    enum class Drag { None, Cells, Rows, Columns, Fill, Resize, BreakLine, BreakEdge } m_drag = Drag::None;
    bool m_resizeColumn = false;
    qint64 m_resizeIndex = -1;
    int m_resizeFrom = 0, m_resizeEdge = 0, m_resizeSize0 = 0, m_resizeSize = 0;
    qint64 m_dropRow = -1, m_dropCol = -1;
    std::vector<GridRange> m_marks;
    std::vector<quint64> m_noteCells;
    std::vector<ShownNote> m_shownNotes;
    mutable std::vector<QSize> m_shownSizes;
    mutable double m_shownZoom = 0;
    mutable qint64 m_shownLayouts = 0;
    qint64 m_hoverRow = -1, m_hoverCol = -1, m_pressRow = -1, m_pressCol = -1;
    std::vector<GridRange> m_checkCircles;
    std::vector<CheckArrow> m_checkArrows;
    std::vector<HeaderButton> m_headerButtons;
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
    QTimer m_conditionalTimer;   // the work of the rules of conditional formatting that a paint left (figures to make, formulas to work out for more cells), done a slice at a time
    void conditionalWork();
    void afterPaint();
    qint64 m_blits = 0, m_fullScrolls = 0;
    QRect m_lastPaintRect;
    QRegion m_paintedRegion;
    QRect m_lastEditBand;
    // The edits of rows and columns followed, with the selection before and after each, so that Undo gives the selection back where it was and Redo where it went (an edit that is the inverse of the last followed, with the selection still where that left it).
    struct FollowedEdit {
        jp::office::sheet::StructEdit edit;
        Selection before, after;
    };
    std::vector<FollowedEdit> m_followed, m_unfollowed;
    qint64 m_paintedPixels = 0;
    std::function<bool()> m_clickGuard;
    std::function<bool(qint64, qint64)> m_pointGuard;
    GridOverlay *m_overlay = nullptr;
    bool m_cellsEnabled = true;
    std::vector<PageBreakLine> m_breakRows, m_breakCols;
    bool m_breakView = false, m_breakEditable = true;
    std::vector<GridRange> m_breakAreas;
    std::vector<BreakPage> m_breakPageList;
    BreakGrab m_breakGrab;
    QPoint m_breakAt;   // where the dragged line or edge is now
};

} // namespace jp::grid
