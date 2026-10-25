#pragma once
// The sheet shown as the pages it prints on, one below the other, each as paper with its margins, a gap between pages, rulers along the top and left, and the header and footer in the
// margins (the three parts of each can be clicked to be typed in). The view draws what a source tells it about the pages and tells what was clicked; it holds no cells. A page is drawn when it
// is in view, one page a turn of the event loop (a pixmap kept for the pages near the view, within a budget), so that a sheet of thousands of pages costs the few in view.
//
// Lengths are in points on the paper and in pixels on the screen: a point is `zoom * 96 / 72` pixels.

#include "grid/selection.h"

#include <QAbstractScrollArea>
#include <QHash>
#include <QPixmap>
#include <QSet>
#include <QTimer>

#include <list>
#include <vector>

class QPainter;

namespace jp::grid {

// What the view shows: the pages of a sheet. The pages are numbered from 0; every point is in points from the paper's corner.
class PageLayoutSource {
public:
    virtual ~PageLayoutSource() = default;
    virtual int pageCount() const = 0;
    virtual QSizeF pageSize(int page) const = 0;
    // The strip of the paper the three parts of the header (footer) go in: between the edges of the margins, from the top (to the bottom) of the paper to the cells.
    virtual QRectF headerBand(int page) const = 0;
    virtual QRectF footerBand(int page) const = 0;
    // The page drawn in points on the painter (its origin the paper's corner).
    virtual void paintPage(QPainter *painter, int page) = 0;
    virtual bool cellRect(int page, qint64 row, qint64 col, QRectF *rect) = 0;
    virtual bool cellAt(int page, const QPointF &point, qint64 *row, qint64 *col) = 0;
    virtual bool rangeRect(int page, const GridRange &range, QRectF *rect) = 0;
    // The first page that shows the cell, or -1.
    virtual int pageOfCell(qint64 row, qint64 col) = 0;
    // Whether pictures on the pages are still being made (what is drawn lacks them; the view draws the pages again a little later).
    virtual bool picturesPending() const { return false; }
};

class PageLayoutView : public QAbstractScrollArea {
    Q_OBJECT
public:
    // The gap between pages, the space around all of them, and the thickness of a ruler, in pixels.
    static constexpr int kGap = 14;
    static constexpr int kMargin = 18;
    static constexpr int kRuler = 20;
    // The most pixels of pages kept drawn (a page larger than this is drawn on the screen each time it is painted).
    static constexpr qint64 kCacheBudgetPixels = 48000000;

    explicit PageLayoutView(QWidget *parent = nullptr);
    ~PageLayoutView() override;

    // The pages come from `source` (not owned; null for none). sourceChanged() says the pages (their number, sizes, or cuts) changed: they are laid out again and drawn again; contentsChanged()
    // says only what is drawn on them did.
    void setSource(PageLayoutSource *source);
    PageLayoutSource *source() const { return m_source; }
    void sourceChanged();
    void contentsChanged();

    double zoom() const { return m_zoom; }
    void setZoom(double zoom);

    // The cells that are selected, and the cell with the cursor (drawn over the pages that show them).
    void setSelection(const GridRange &range, qint64 activeRow, qint64 activeCol);
    // Scrolls so that the cell is in view (its page, and the cell in it); true if it moved.
    bool ensureCellVisible(qint64 row, qint64 col);
    // Scrolls to the top of a page.
    void scrollToPage(int page);

    // Where things are, in the viewport's pixels (empty when there is no such thing in view).
    QRect pageRect(int page) const;
    QRect cellRect(qint64 row, qint64 col) const;   // the cell on the first page that shows it
    QRect zoneRect(int page, bool header, int zone) const;   // zone 0 left, 1 center, 2 right
    int pageAt(const QPoint &viewportPoint) const;   // the page under the point (also in the gap below it); -1 for none
    int firstVisiblePage() const;
    int lastVisiblePage() const;
    // Whether a page is drawn and kept, and how many pages have been drawn since the view was made (counted, for tests).
    bool pageIsDrawn(int page) const { return m_cache.contains(page); }
    // Whether the page's picture on the screen is made from what the source says now (a page that was drawn before a change stays until it is drawn again).
    bool pageIsCurrent(int page) const { return m_cache.contains(page) && !m_stale.contains(page); }
    // The pixels of pages kept drawn that make the view keep none of a page larger than this (it draws the exposed part of such a page each time; tests make it small).
    void setCacheBudgetPixels(qint64 pixels);
    int pagesDrawn() const { return m_drawn; }
    int pagesKept() const { return int(m_cache.size()); }
    // Draws the pages in view that are not drawn yet, now (the timer does it a page a turn).
    void drawPagesNow();
    // What the rulers count in: inches, or centimeters.
    void setMetric(bool metric);
    bool metric() const { return m_metric; }

Q_SIGNALS:
    void zoomChanged(double zoom);
    // A cell was pressed (with these keys held), dragged over, and let go; a double click on one.
    void cellPressed(int page, qint64 row, qint64 col, Qt::KeyboardModifiers modifiers);
    void cellDragged(int page, qint64 row, qint64 col);
    void cellReleased();
    void cellDoubleClicked(int page, qint64 row, qint64 col);
    // A part of the header (footer) of a page was clicked: zone 0 left, 1 center, 2 right.
    void zoneClicked(int page, bool header, int zone);
    // A key was pressed with the view in focus (the window gives it to the cells).
    void keyPressed(QKeyEvent *event);
    void viewChanged();

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void leaveEvent(QEvent *e) override;
    bool focusNextPrevChild(bool) override { return false; }
    void scrollContentsBy(int, int) override { viewport()->update(); Q_EMIT viewChanged(); }

private:
    double pixelsPerPoint() const { return m_zoom * 96.0 / 72.0; }
    void layoutPages();
    void updateBars();
    qint64 pageTop(int page) const;       // in the content's pixels, from the top
    int pageWidthPx(int page) const;
    int pageHeightPx(int page) const;
    int pageLeft(int page) const;         // in the viewport's pixels
    QPointF toPagePoint(int page, const QPoint &viewportPoint) const;
    void drawPage(int page);
    void trim();
    void scheduleDraw();
    void renderStep();
    void paintRulers(QPainter *p);
    bool zoneAt(const QPoint &p, int *page, bool *header, int *zone) const;

    PageLayoutSource *m_source = nullptr;
    double m_zoom = 1.0;
    bool m_metric = false;
    std::vector<qint64> m_tops;                // the content's pixel at the top of each page
    std::vector<QSize> m_sizes;                // each page's pixels
    qint64 m_total = 0;
    int m_widest = 0;
    QHash<int, QPixmap> m_cache;
    QSet<int> m_stale;                         // pages in the cache that what they show has changed since
    std::list<int> m_order;                    // pages in the cache, the one used last at the front
    qint64 m_cachedPixels = 0;
    int m_drawn = 0;
    QTimer m_renderTimer, m_pictureTimer;
    QSet<int> m_drawnWithoutPictures;
    qint64 m_budgetPixels = kCacheBudgetPixels;
    GridRange m_selection{0, 0, 0, 0};
    qint64 m_activeRow = -1, m_activeCol = -1;
    bool m_dragging = false;
    int m_hoverPage = -1;
    bool m_hoverHeader = false;
    int m_hoverZone = -1;
};

} // namespace jp::grid
