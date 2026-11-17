#include "app/pageoutput.h"
#include "core/items.h"

#include "app/pagepaint.h"
#include "app/writetags.h"
#include "core/linkops.h"

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

// The size of the sheet that pages share, from the first of them: its own, or the paper asked for (put the first page's way up), on its side for a grid that is wider than tall, and
// turned as the layout says. A page alone on a sheet of its own size is that size.
QSizeF sharedSheetSize(QSizeF sheet, const QSize &grid, const SheetLayout &layout)
{
    const bool wide = sheet.width() > sheet.height();
    if (layout.paper.width() > 0 && layout.paper.height() > 0) sheet = wide ? QSizeF(layout.paper.height(), layout.paper.width()) : layout.paper;
    if (grid.width() > grid.height() && sheet.width() < sheet.height()) sheet.transpose();
    return turnedSheet(sheet, layout.turn);
}

// Where page `k` of a sheet's group is put on the sheet (from its top left, in points), and how much it is scaled to fit its place of the grid.
struct SheetSlot {
    double x = 0, y = 0, fit = 1;
};
SheetSlot slotOnSheet(const QSizeF &sheet, const QSize &grid, int k, const QSizeF &page, int perSheet)
{
    const double inset = perSheet > 1 ? 8.0 : 0.0;
    const double cellW = sheet.width() / grid.width(), cellH = sheet.height() / grid.height();
    const double boxW = cellW - 2 * inset, boxH = cellH - 2 * inset;
    SheetSlot slot;
    slot.fit = std::min(boxW / page.width(), boxH / page.height());
    const double w = page.width() * slot.fit, h = page.height() * slot.fit;
    slot.x = (k % grid.width()) * cellW + (cellW - w) / 2;
    slot.y = (k / grid.width()) * cellH + (cellH - h) / 2;
    return slot;
}

// Where each of the pages is in the PDF paintSheets makes of them: the sheet (from 0, in the order the sheets are begun), its size, and the page's place on it.
struct PagePlace {
    int sheet = -1;
    QSizeF sheetSize;
    SheetSlot slot;
};
QVector<PagePlace> placesOfPages(WriteDoc &doc, const QVector<int> &pages, const SheetLayout &layout)
{
    QVector<PagePlace> out(pages.size());
    const QSize grid = sheetGrid(layout.perSheet);
    const int perSheet = grid.width() * grid.height();
    const bool shared = perSheet > 1 || (layout.paper.width() > 0 && layout.paper.height() > 0) || layout.turn != PrintTurn::AsSet;
    // (a blank sheet is made only when some page gives it its size)
    bool some = false;
    for (int i = 0; !some && i < pages.size(); ++i) some = pages[i] >= 0 && doc.pager().materialize(pages[i]);
    int sheetNumber = 0;
    for (qsizetype first = 0; first < pages.size(); first += perSheet) {
        QVector<int> at;
        QVector<QSizeF> sizes;
        bool blank = true;
        for (qsizetype i = first; i < pages.size() && i < first + perSheet; ++i) {
            blank = blank && pages[i] < 0;
            const std::shared_ptr<flow::MaterializedPage> page = pages[i] < 0 ? nullptr : doc.pager().materialize(pages[i]);
            if (!page) continue;
            at << int(i);
            sizes << page->size;
        }
        if (at.isEmpty() && !blank) continue;
        if (blank) {
            if (some) ++sheetNumber;
            continue;
        }
        const QSizeF sheet = shared ? sharedSheetSize(sizes[0], grid, layout) : sizes[0];
        for (int k = 0; k < at.size(); ++k) {
            PagePlace &place = out[at[k]];
            place.sheet = sheetNumber;
            place.sheetSize = sheet;
            if (shared) place.slot = slotOnSheet(sheet, grid, k, sizes[k], perSheet);
        }
        ++sheetNumber;
    }
    return out;
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

QVector<PdfLink> sheetLinks(WriteDoc &doc, const QVector<int> &sheets, const SheetLayout &layout)
{
    QVector<PdfLink> out;
    const QVector<PagePlace> places = placesOfPages(doc, sheets, layout);
    const QVector<LinkSpan> spans = linkSpans(doc);
    LayoutCache cache;
    PaintContext ctx;
    ctx.opt.output = true;
    ctx.opt.textOutput = TextOutput::Pdf;
    const StoryScan scan = scanStory(doc.story());
    // Where a place of the text is on its page: the spot, from the page's bottom left (false where the page is not one the PDF has, or the place is not on a line of it).
    auto spotOf = [&](int position, QPointF *spot, double *pageHeight) {
        const std::shared_ptr<flow::MaterializedPage> page = doc.pager().materialize(doc.pager().pageOfPosition(position));
        if (!page) return false;
        const double height = page->size.height();
        *pageHeight = height;
        for (const auto &fragment : page->tables) {
            QLineF line;
            if (fragment->contains(position) && fragment->caretLine(position, &line)) {
                *spot = QPointF(line.x1(), height - std::min(line.y1(), line.y2()));
                return true;
            }
        }
        for (const QString &id : page->boxes) {
            const Item *item = page->doc->item(id);
            if (!item || item->type() != ItemType::Text) continue;
            const auto *box = static_cast<const TextItem *>(item);
            const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, page->number, ctx.opt);
            int frame = -1;
            QRectF caret;
            if (!fl.layout || fl.frame < 0 || !fl.layout->caretRect(position - page->storyFrom, &frame, &caret) || frame != fl.frame) continue;
            const QRectF m = box->transform().mapRect(caret);
            *spot = QPointF(m.left(), height - m.top());
            return true;
        }
        return false;
    };
    // A point of a page (from its top left) as a PDF counts it on the sheet the page is on (from the sheet's bottom left).
    auto onSheet = [](const PagePlace &pl, double x, double yFromTop) { return QPointF(pl.slot.x + x * pl.slot.fit, pl.sheetSize.height() - (pl.slot.y + yFromTop * pl.slot.fit)); };
    auto link = [&](int index, const flow::MaterializedPage &page, const LinkSpan &s, const QRectF &r) {
        const PagePlace &pl = places[index];
        if (pl.sheet < 0) return;
        PdfLink k;
        k.page = pl.sheet;
        k.rect = QRectF(onSheet(pl, r.left(), r.bottom()), onSheet(pl, r.right(), r.top())).normalized();
        k.uri = s.uri;
        if (s.uri.isEmpty()) {
            const int target = int(sheets.indexOf(doc.pager().pageOfPosition(s.target)));
            if (target < 0 || places[target].sheet < 0) return;
            k.toPage = places[target].sheet;
            QPointF spot;
            double height = 0;
            k.hasSpot = spotOf(s.target, &spot, &height);
            if (k.hasSpot) k.spot = onSheet(places[target], spot.x(), height - spot.y());
        }
        out.push_back(k);
    };
    for (int sheet = 0; sheet < sheets.size() && out.size() < kMostPdfLinks; ++sheet) {
        const std::shared_ptr<flow::MaterializedPage> page = sheets[sheet] < 0 ? nullptr : doc.pager().materialize(sheets[sheet]);
        if (!page) continue;
        QString bodyId;
        for (const QString &id : page->boxes)
            if (const Item *item = page->doc->item(id))
                if (item->type() == ItemType::Text) {
                    bodyId = static_cast<const TextItem *>(item)->storyId;
                    break;
                }
        const Story *body = bodyId.isEmpty() ? nullptr : page->doc->story(bodyId);
        const int length = body && body->doc ? body->doc->characterCount() : 0;
        // The text of the page (a link in a table is looked at below, where its words are).
        for (const LinkSpan &s : spans) {
            bool inTable = false;
            for (const auto &fragment : page->tables) inTable = inTable || fragment->contains(s.from);
            if (inTable) continue;
            const int from = std::max(0, s.from - page->storyFrom), to = std::min(length, s.to - page->storyFrom);
            if (to <= from) continue;
            for (const QString &id : page->boxes) {
                const Item *item = page->doc->item(id);
                if (!item || item->type() != ItemType::Text) continue;
                const auto *box = static_cast<const TextItem *>(item);
                const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, page->number, ctx.opt);
                if (!fl.layout || fl.frame < 0) continue;
                for (const QRectF &r : fl.layout->rangeRects(fl.frame, from, to)) {
                    link(sheet, *page, s, box->transform().mapRect(r));
                    if (out.size() >= kMostPdfLinks) return out;
                }
            }
        }
        // The tables: the words of a link on a line of a cell, from where it begins to where it ends; across lines, the cell.
        for (const auto &fragment : page->tables) {
            for (const LinkSpan &s : spans) {
                if (!fragment->contains(s.from)) continue;
                QLineF a, b;
                QRectF box;
                if (fragment->caretLine(s.from, &a) && fragment->caretLine(s.to, &b) && std::abs(a.y1() - b.y1()) < 0.5) box = QRectF(QPointF(a.x1(), std::min(a.y1(), a.y2())), QPointF(b.x1(), std::max(a.y1(), a.y2())));
                else box = fragment->cellRect(s.from);
                if (box.isEmpty()) continue;
                link(sheet, *page, s, box);
                if (out.size() >= kMostPdfLinks) return out;
            }
        }
        // The other stories the page shows: a header, a footer, a note, a text box.
        QHash<QString, const QTextDocument *> sourceOf;
        for (const auto &fb : page->furniture) sourceOf.insert(fb.itemId, fb.source);
        for (const auto &fb : page->noteBoxes) sourceOf.insert(fb.itemId, fb.source);
        for (const auto &item : page->doc->pages[0]->items) {
            if (item->type() != ItemType::Text || item->id.startsWith(QLatin1String("flow-box-"))) continue;
            const auto *box = static_cast<const TextItem *>(item.get());
            const Story *own = page->doc->story(box->storyId);
            if (!own || !own->doc) continue;
            const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, page->number, ctx.opt);
            if (!fl.layout || fl.frame < 0) continue;
            for (const LinkSpan &s : linkSpansIn(doc, own->doc.get(), sourceOf.value(box->id), scan))
                for (const QRectF &r : fl.layout->rangeRects(fl.frame, s.from, s.to)) {
                    link(sheet, *page, s, box->transform().mapRect(r));
                    if (out.size() >= kMostPdfLinks) return out;
                }
        }
    }
    return out;
}

bool paintSheets(WriteDoc &doc, QPagedPaintDevice *device, const QVector<int> &pages, int pageCount, bool grayscale, const SheetLayout &layout, double maxImageDpi, WriteTagger *tagger)
{
    auto *printer = dynamic_cast<QPrinter *>(device);
    if (printer) printer->setFullPage(true);
    QPainter p;
    bool started = false;
    const QSize grid = sheetGrid(layout.perSheet);
    const int perSheet = grid.width() * grid.height();
    const bool shared = perSheet > 1 || (layout.paper.width() > 0 && layout.paper.height() > 0) || layout.turn != PrintTurn::AsSet;
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
        d.maxImageDpi = maxImageDpi;
        d.tagger = tagger;
        if (tagger) tagger->beginPage();
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
            if (shared) size = sharedSheetSize(size, grid, layout);
            setSheet(device, size);
            if (!begin()) return false;
            continue;
        }
        const QSizeF sheet = shared ? sharedSheetSize(group[0]->size, grid, layout) : group[0]->size;
        setSheet(device, sheet);
        if (!begin()) return false;
        if (!shared) {
            draw(*group[0]);
            continue;
        }
        for (qsizetype k = 0; k < group.size(); ++k) {
            flow::MaterializedPage &page = *group[k];
            const SheetSlot slot = slotOnSheet(sheet, grid, int(k), page.size, perSheet);
            const double fit = slot.fit, x = slot.x, y = slot.y;
            const double w = page.size.width() * fit, h = page.size.height() * fit;
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

} // namespace jp::write
