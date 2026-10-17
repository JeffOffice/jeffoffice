#include "app/pageoutput.h"

#include "app/pagepaint.h"

#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrinter>

namespace jp::write {

namespace {

// Sets the sheet to the page's size. A printer is given a standard size where the paper is one, and the way up; a PDF has the size as it is.
void setSheet(QPagedPaintDevice *device, const QSizeF &points)
{
    if (dynamic_cast<QPrinter *>(device)) {
        const bool wide = points.width() > points.height();
        const QSizeF upright = wide ? QSizeF(points.height(), points.width()) : points;
        device->setPageSize(QPageSize(upright, QPageSize::Point, QString(), QPageSize::FuzzyMatch));
        device->setPageOrientation(wide ? QPageLayout::Landscape : QPageLayout::Portrait);
    } else {
        device->setPageSize(QPageSize(points, QPageSize::Point, QString(), QPageSize::ExactMatch));
    }
    device->setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Point);
}

} // namespace

bool paintSheets(WriteDoc &doc, QPagedPaintDevice *device, const QVector<int> &pages, int pageCount, bool grayscale)
{
    auto *printer = dynamic_cast<QPrinter *>(device);
    if (printer) printer->setFullPage(true);
    QPainter p;
    bool started = false;
    for (int index : pages) {
        const auto page = doc.pager().materialize(index);
        if (!page) continue;
        setSheet(device, page->size);
        if (!started) {
            started = p.begin(device);
            if (!started) return false;
        } else {
            device->newPage();
        }
        const double scale = double(device->logicalDpiX()) / 72.0;
        p.save();
        p.scale(scale, scale);
        LayoutCache cache;
        PageDrawing d;
        d.output = true;
        d.textOutput = printer ? TextOutput::Print : TextOutput::Pdf;
        d.pageCount = pageCount;
        d.grayscale = grayscale;
        paintPage(&p, *page, &cache, d);
        p.restore();
    }
    if (started) p.end();
    return started;
}

} // namespace jp::write
