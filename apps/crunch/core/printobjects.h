#pragma once
// The charts, pictures, and shapes of a sheet on its printed pages: for each page, the objects that reach the page's rows and columns, drawn at the size the page's scale gives them,
// clipped to the cells of the page (an object across two pages is on both, cut at the edge). The objects come from SheetObjects::printObjects (each draws itself when asked, on a
// worker for a chart); the pictures are made here, one at a time, at the resolution the page is painted at (kMaxPixels at the most for one), and kept for the pages that follow.

#include "grid/axis.h"
#include "grid/objectlayer.h"
#include "grid/printlayout.h"

#include <QImage>
#include <QPainter>

#include <map>
#include <vector>

namespace jp::crunch {

class PageObjectPainter {
public:
    // The most pixels of a picture, and the most bytes of pictures held for the pages that follow.
    static constexpr qint64 kMaxPixels = 6000000;
    static constexpr qint64 kMaxHeldBytes = qint64(96) << 20;

    // The objects, and the sheet's row and column sizes (copied; nothing skipped), which place them.
    PageObjectPainter(std::vector<jp::grid::LayerObject> objects, const jp::grid::Axis &rows, const jp::grid::Axis &cols);

    // Where each object lies in the grid's pixels from the sheet's first row and column (the ones that are empty are not in it): what extends the printed block.
    std::vector<jp::grid::ObjectBox> boxes() const;
    int count() const { return int(m_items.size()); }

    // Draws the objects that reach the page, in the painter set up as the cells were drawn (its origin the page's cells, a unit one pixel of the sheet at `zoom`).
    void paint(QPainter *painter, const jp::grid::PageCut &page, double zoom);
    // Pictures made so far (a test counts them to see that a picture is made once for the pages an object is on).
    int picturesMade() const { return m_made; }

private:
    struct Item {
        jp::grid::LayerObject object;
        jp::grid::ObjectBox box;
        bool settled = false;
        QImage image;
        QSize requested;
        double zoom = 0;
        quint64 used = 0;
    };
    QImage pictureOf(Item &item, const QSize &wanted, double zoom, const QRectF &inner);
    void trim();

    std::vector<Item> m_items;
    jp::grid::Axis m_rows, m_cols;
    quint64 m_tick = 0;
    int m_made = 0;
};

} // namespace jp::crunch
