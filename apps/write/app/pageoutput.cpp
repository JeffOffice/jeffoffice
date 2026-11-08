#include "app/pageoutput.h"

#include "app/pagepaint.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrinter>
#include <algorithm>

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

QSize sheetGrid(int perSheet)
{
    switch (perSheet) {
    case 2: return QSize(2, 1);
    case 4: return QSize(2, 2);
    case 6: return QSize(3, 2);
    case 9: return QSize(3, 3);
    case 16: return QSize(4, 4);
    default: return QSize(1, 1);
    }
}

bool paintSheets(WriteDoc &doc, QPagedPaintDevice *device, const QVector<int> &pages, int pageCount, bool grayscale, const SheetLayout &layout)
{
    auto *printer = dynamic_cast<QPrinter *>(device);
    if (printer) printer->setFullPage(true);
    QPainter p;
    bool started = false;
    const QSize grid = sheetGrid(layout.perSheet);
    const int perSheet = grid.width() * grid.height();
    const bool shared = perSheet > 1 || (layout.paper.width() > 0 && layout.paper.height() > 0);
    auto draw = [&](flow::MaterializedPage &page) {
        const double scale = double(device->logicalDpiX()) / 72.0;
        p.save();
        p.scale(scale, scale);
        LayoutCache cache;
        PageDrawing d;
        d.output = true;
        d.textOutput = printer ? TextOutput::Print : TextOutput::Pdf;
        d.pageCount = pageCount;
        d.grayscale = grayscale;
        paintPage(&p, page, &cache, d);
        p.restore();
    };
    auto begin = [&] {
        if (!started) {
            started = p.begin(device);
            return started;
        }
        device->newPage();
        return true;
    };
    for (qsizetype first = 0; first < pages.size(); first += perSheet) {
        // The sheet is the size of the first page on it (or the paper asked for, turned to the first page's way up), turned on its side for a grid that is wider than tall.
        QVector<std::shared_ptr<flow::MaterializedPage>> group;
        for (qsizetype i = first; i < pages.size() && i < first + perSheet; ++i) {
            std::shared_ptr<flow::MaterializedPage> page = pages[i] < 0 ? nullptr : doc.pager().materialize(pages[i]);
            if (!page) continue;
            // The page's color is in a PDF (a reference PDF of such a file has it), and is not sent to a printer: paper is left white unless it is asked for.
            if (printer) {
                auto &items = page->doc->pages[0]->items;
                items.erase(std::remove_if(items.begin(), items.end(), [](const std::shared_ptr<Item> &item) { return item->id.startsWith(QLatin1String("flow-page-color-")); }), items.end());
            }
            group.push_back(page);
        }
        // A chunk of only -1s is a blank sheet (both sides by hand, a front with no back), the size of the sheets around it.
        bool blank = group.isEmpty();
        for (qsizetype i = first; blank && i < pages.size() && i < first + perSheet; ++i) blank = pages[i] < 0;
        if (group.isEmpty() && !blank) continue;
        if (blank) {
            QSizeF size;
            for (int page : pages) {
                std::shared_ptr<flow::MaterializedPage> other = page >= 0 ? doc.pager().materialize(page) : nullptr;
                if (other) {
                    size = other->size;
                    break;
                }
            }
            if (size.isEmpty()) continue;
            if (shared) {
                if (layout.paper.width() > 0 && layout.paper.height() > 0) size = size.width() > size.height() ? QSizeF(layout.paper.height(), layout.paper.width()) : layout.paper;
                if (grid.width() > grid.height() && size.width() < size.height()) size.transpose();
            }
            setSheet(device, size);
            if (!begin()) return false;
            continue;
        }
        QSizeF sheet = group[0]->size;
        if (shared) {
            const bool wide = sheet.width() > sheet.height();
            if (layout.paper.width() > 0 && layout.paper.height() > 0) sheet = wide ? QSizeF(layout.paper.height(), layout.paper.width()) : layout.paper;
            if (grid.width() > grid.height() && sheet.width() < sheet.height()) sheet.transpose();
        }
        setSheet(device, sheet);
        if (!begin()) return false;
        if (!shared) {
            draw(*group[0]);
            continue;
        }
        const double inset = perSheet > 1 ? 8.0 : 0.0;
        const double cellW = sheet.width() / grid.width(), cellH = sheet.height() / grid.height();
        for (qsizetype k = 0; k < group.size(); ++k) {
            flow::MaterializedPage &page = *group[k];
            const double boxW = cellW - 2 * inset, boxH = cellH - 2 * inset;
            const double fit = std::min(boxW / page.size.width(), boxH / page.size.height());
            const double w = page.size.width() * fit, h = page.size.height() * fit;
            const double x = (k % grid.width()) * cellW + (cellW - w) / 2, y = (k / grid.width()) * cellH + (cellH - h) / 2;
            p.save();
            const double dpi = double(device->logicalDpiX()) / 72.0;
            p.scale(dpi, dpi);
            p.translate(x, y);
            p.scale(fit, fit);
            p.setClipRect(QRectF(0, 0, page.size.width(), page.size.height()));
            p.scale(1.0 / dpi, 1.0 / dpi);   // (draw() scales by the device's dpi itself)
            draw(page);
            p.restore();
            if (perSheet > 1) {
                p.save();
                p.scale(dpi, dpi);
                QPen pen(QColor(150, 150, 150));
                pen.setWidthF(0.5);
                p.setPen(pen);
                p.setBrush(Qt::NoBrush);
                p.drawRect(QRectF(x, y, w, h));
                p.restore();
            }
        }
    }
    if (started) p.end();
    return started;
}

namespace {
constexpr int kIsFileRole = Qt::UserRole + 1;   // set on the entry that makes a PDF file
}

void fillPrinterBox(QComboBox *box, const QStringList &printers, const QString &current)
{
    box->clear();
    for (const QString &name : printers) box->addItem(name, name);
    box->addItem(QCoreApplication::translate("Print", "Save as PDF"), QString());
    box->setItemData(box->count() - 1, true, kIsFileRole);
    const int at = printers.indexOf(current);
    if (at >= 0) box->setCurrentIndex(at);
    else if (printers.isEmpty()) box->setCurrentIndex(0);
}

PrinterChoice printerChoice(const QComboBox *box)
{
    PrinterChoice choice;
    if (box->currentIndex() < 0) return choice;
    choice.toFile = box->currentData(kIsFileRole).toBool();
    if (!choice.toFile) choice.printer = box->currentData(Qt::UserRole).toString();
    return choice;
}

} // namespace jp::write
