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
    // The header and footer alone (the margins of the paper).
    void paintHeaderFooter(QPainter *painter, const PaperBox &paper, const PageSetup &setup, const jp::office::sheet::HfContext &labels, bool firstOfSheet, double scale);

    GridPainter &gridPainter() { return *m_painter; }
    GridSheet &grid() { return *m_sheet; }
    // The grid's paint of the last page (counts of what it did).
    const PaintStats &stats() const { return m_painter->stats(); }

private:
    void paintHeadings(QPainter *p, const GridLayout &layout, const PageCut &page, const PageMetrics &m, double scale);
    void paintHeaderFooterText(QPainter *p, const QString &code, const PaperBox &paper, const PageSetup &setup, const jp::office::sheet::HfContext &labels, bool header, double scale, bool first, bool even);
    QImage headerPicture(const jp::office::sheet::HeaderPicture &pic);

    GridSheet *m_sheet;
    std::unique_ptr<GridPainter> m_painter;
    QFont m_base;
    QColor m_gridline = QColor(0x40, 0x40, 0x40);
    std::map<std::pair<const void *, qsizetype>, QImage> m_pictures;
};

} // namespace jp::grid
