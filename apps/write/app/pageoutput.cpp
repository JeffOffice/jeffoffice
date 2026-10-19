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
        // The page's color is in a PDF (a reference PDF of such a file has it), and is not sent to a printer: paper is left white unless it is asked for.
        if (printer) {
            auto &items = page->doc->pages[0]->items;
            items.erase(std::remove_if(items.begin(), items.end(), [](const std::shared_ptr<Item> &item) { return item->id.startsWith(QLatin1String("flow-page-color-")); }), items.end());
        }
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
