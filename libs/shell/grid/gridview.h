#pragma once
// The grid as a widget: a scrolling view of the sheet (GridSheet) drawn by GridPainter, with the selection and the keys and mouse that change it (Navigator), the numbers of the
// selection for the status bar (StatsWalk), and zoom. It holds no cells; the window gives it the data. Scrolling by a row or a column at 100% moves the pixels already on the
// screen and draws only the strip that is left, so a frame costs the cells in a row, not those on the screen. Work that may be long (the edge of the data a Ctrl and an arrow
// look for, the numbers of a whole column) is done a few milliseconds at a time between events.

#include "grid/gridsheet.h"
#include "grid/painter.h"
#include "grid/selection.h"
#include "grid/stats.h"
#include "grid/textwalk.h"

#include <QAbstractScrollArea>
#include <QElapsedTimer>
#include <QTimer>

#include <memory>
#include <vector>

namespace jp::grid {

class GridView : public QAbstractScrollArea {
    Q_OBJECT
public:
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
        qint64 frozenRows = 0, frozenCols = 0;
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
    void invalidateValues();   // values (or formats) changed: draw again

    // For measuring: the time of each paint, in milliseconds, since the last clear; and the work the last paint did.
    const std::vector<double> &paintTimes() const { return m_paintTimes; }
    void clearPaintTimes() { m_paintTimes.clear(); }
    const PaintStats &lastPaintStats() const { return m_painter->stats(); }
    // Scrolls done by moving the pixels already on the screen (and drawing the strip they left), and by drawing the window again: counted, so that a test can see which a scroll was
    // on a window system that may not move pixels (the strip is then drawn as part of a larger piece).
    qint64 blitScrolls() const { return m_blits; }
    qint64 fullScrolls() const { return m_fullScrolls; }
    // True while a walk for the edge of the data or the numbers of a selection is going on.
    bool busy() const { return m_jump || m_statsWalk; }

Q_SIGNALS:
    void selectionChanged();
    void zoomChanged(double zoom);
    void statsChanged(const QString &text);
    void contextMenuRequested(const QPoint &globalPos);
    void viewChanged();   // the scroll, frozen panes, or what is shown changed

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    bool focusNextPrevChild(bool) override { return false; }
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
    QRect bodyScrollRect(bool vertical) const;

    GridSheet m_sheet;
    std::unique_ptr<GridPainter> m_painter;
    std::unique_ptr<Nav> m_nav;
    Selection m_selection;
    bool m_gridlines = true, m_headings = true;
    // the mouse
    enum class Drag { None, Cells, Rows, Columns } m_drag = Drag::None;
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
    std::vector<double> m_paintTimes;
    qint64 m_blits = 0, m_fullScrolls = 0;
};

} // namespace jp::grid
