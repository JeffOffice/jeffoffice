#pragma once
// Drawing one printed page of a sheet: the cells (the grid's own painter, at the page's scale), the repeated title rows and columns, the headings and gridlines when the setup asks for them,
// the objects over the cells (through a function the caller gives), and the header and footer in the margins. A page is drawn in points on any painter whose origin is the paper's corner,
// so the preview, a PDF, and a printer show the same thing.
//
// The grid paints in pixels at a zoom; a page paints it at the zoom that makes a pixel of the sheet 0.75 point times the scale, and the painter is scaled down to points, so the cells are
// where the page layout (printlayout.h) says and text and lines are vector shapes at the printer's own resolution.

#include "grid/gridsheet.h"
#include "grid/painter.h"
#include "grid/printlayout.h"
#include "sheet/headerfooter.h"

#include <QFont>
#include <QImage>
#include <QPainter>

#include <functional>
#include <map>
#include <unordered_map>
#include <memory>

namespace jp::grid {

// What is drawn over the cells of a page: called with the painter set up as the cells were drawn in (the grid's pixels at the zoom the layout has), the layout, and the rectangle of the cells
// in the painter's units. The objects that print are drawn by it, clipped to the page's cells.
using PageObjects = std::function<void(QPainter *painter, const GridLayout &layout, const QRect &cells)>;

// Where the parts of a page are on the paper, in points.
struct PageMetrics {
    QRectF cells;              // the title rows and columns and the body: all the cells the page shows
    QRectF headingColumn;      // the row numbers' strip (empty when headings are not printed)
    QRectF headingRow;         // and the column letters'
    double zoom = 1;           // the zoom the grid is painted at
    double unit = 1;           // points in one unit of the painter the grid is painted in
    qint64 widthUnits = 0, heightUnits = 0;
};
PageMetrics pageMetrics(const SheetPrint &print, const PageCut &page, const PageSetup &setup, const Axis &rows, const Axis &cols);

class PagePainter {
public:
    // `sheet` is the grid model of the sheet being printed: its own, not a window's (its layout is changed to each page in turn).
    explicit PagePainter(GridSheet *sheet);
    ~PagePainter();

    // The font the header and footer begin in: the workbook's own (its family and size in points).
    void setBaseFont(const QString &family, double points);
    // The ink and paper of the page (the grid's colors; a printed gridline is dark, not the screen's pale).
    void setGridlineColor(const QColor &color) { m_gridline = color; }

    // Draws the page. `labels` has the page's number and the totals, the date, and the names its fields come to; `firstOfSheet` and the number's parity pick the header and footer.
    void paint(QPainter *painter, const SheetPrint &print, const PageCut &page, const PageSetup &setup, const jp::office::sheet::HfContext &labels, bool firstOfSheet, const PageObjects &objects = nullptr);
    // Where a cell is on the page, in points from the paper's corner (false if the cell is not among those the page shows), and which cell is at a point of the paper (false if none): the page is
    // laid out as paint() lays it out, without drawing it.
    bool cellRect(const SheetPrint &print, const PageCut &page, const PageSetup &setup, qint64 row, qint64 col, QRectF *out);
    bool cellAt(const SheetPrint &print, const PageCut &page, const PageSetup &setup, const QPointF &point, qint64 *row, qint64 *col);
    // The header and footer alone (the margins of the paper).
    void paintHeaderFooter(QPainter *painter, const PaperBox &paper, const PageSetup &setup, const jp::office::sheet::HfContext &labels, bool firstOfSheet, double scale);

    // Pictures in the header and footer are made on a worker while this is on (a page is drawn without one that is not ready yet, and picturesPending() says so); off, a picture not ready is made
    // at once (a print or an export waits for its pictures).
    void setAsyncPictures(bool on) { m_asyncPictures = on; }
    // Whether the page drawn last lacked a picture that was not ready (it is to be drawn again when the picture is: decided by what the drawing met, not by what is being made afterwards).
    bool missedPictures() const { return m_missedPictures; }
    // What the screen shows and the paper does not: the gridlines and the row and column headings (the headings in the margins, so that the cells keep their places).
    void setScreenMarks(bool gridlines, bool headings)
    {
        m_screenGrid = gridlines;
        m_screenHeadings = headings;
    }
    // How many times a page's place among the sheet's rows and columns was worked out (a page is worked out once, and again only for another page's turn between): counted, for tests.
    qint64 layoutsWorkedOut() const { return m_layoutRuns; }
    GridPainter &gridPainter() { return *m_painter; }
    GridSheet &grid() { return *m_sheet; }
    // The grid's paint of the last page (counts of what it did).
    const PaintStats &stats() const { return m_painter->stats(); }

private:
    PageMetrics layoutPage(const SheetPrint &print, const PageCut &page, const PageSetup &setup);
    void paintScreenGridlines(QPainter *p, const GridLayout &layout, const QRect &area);
    void paintHeadings(QPainter *p, const GridLayout &layout, const PageCut &page, const PageMetrics &m, double scale);
    void paintHeaderFooterText(QPainter *p, const QString &code, const PaperBox &paper, const PageSetup &setup, const jp::office::sheet::HfContext &labels, bool header, double scale, bool first, bool even);
    QImage headerPicture(const jp::office::sheet::HeaderPicture &pic);

    GridSheet *m_sheet;
    std::unique_ptr<GridPainter> m_painter;
    QFont m_base;
    QColor m_gridline = QColor(0x40, 0x40, 0x40);
    bool m_missedPictures = false;
    bool m_asyncPictures = false, m_screenGrid = false, m_screenHeadings = false;
    // the metrics of the pages worked out (by the page's cut, which is as long-lived as the job), and the page the grid's layout is set for now
    std::unordered_map<const PageCut *, PageMetrics> m_metrics;
    const PageCut *m_applied = nullptr;
    const SheetPrint *m_appliedPrint = nullptr;
    qint64 m_layoutRuns = 0;
};

} // namespace jp::grid
