#include "app/pageoutput.h"
#include "core/items.h"

#include "app/pagepaint.h"
#include "app/writetags.h"
#include "core/commentops.h"
#include "core/linkops.h"
#include "core/reviewops.h"
#include "text/textprops.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrinter>
#include <algorithm>
#include <limits>

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

std::function<void(int, const MarkupCounts &)> g_markupObserver;

} // namespace

void setMarkupObserver(const std::function<void(int, const MarkupCounts &)> &observer) { g_markupObserver = observer; }

MarkupCounts paintMarkupMarks(QPainter *p, flow::MaterializedPage &page, LayoutCache *layouts, WriteDoc &doc, const MarkupMarks &marks)
{
    MarkupCounts counts;
    if (!marks.any()) return counts;
    QVector<TextItem *> boxes;
    for (const QString &id : page.boxes)
        if (Item *item = page.doc->item(id))
            if (item->type() == ItemType::Text) boxes.push_back(static_cast<TextItem *>(item));
    if (boxes.isEmpty()) return counts;
    const LayoutCache::FrameLayout fl = layouts->textFrame(*page.doc, *boxes.first(), page.number, RenderOptions());
    if (!fl.layout) return counts;
    const double pageWidth = page.size.width();
    struct Line { const TextItem *box; QRectF rect; int a, b; };
    QVector<Line> lines;
    int lo = std::numeric_limits<int>::max(), hi = 0;
    for (int f = 0; f < boxes.size(); ++f)
        for (const auto &line : fl.layout->lineInfo(f)) {
            const int a = line.docPos + page.storyFrom, b = std::max(line.docEnd, line.docPos + 1) + page.storyFrom;
            lines.push_back({boxes[f], line.rect, a, b});
            lo = std::min(lo, a);
            hi = std::max(hi, b);
        }
    if (lines.isEmpty()) return counts;
    // What the marks are made from: the changes and the comments of the stretch of the story the page shows.
    struct Mark { int at; QString initials, text; QColor color; bool done; };
    QVector<std::pair<int, int>> spans, formatSpans;
    QVector<Mark> comments, deletions;
    QVector<int> carets;
    for (const Revision &r : revisionsIn(doc.story(), lo, hi, 4000)) {
        const bool formatting = r.kind == Revision::Format || r.kind == Revision::ParagraphFormat;
        (formatting ? formatSpans : spans).push_back({r.from, std::max(r.to, r.from + 1)});
        if (r.kind != Revision::Deletion && r.kind != Revision::MovedFrom) continue;
        if (marks.deletions) {
            const QString words = revisionText(doc.story(), r, 60);
            if (!words.isEmpty()) deletions.push_back({r.from, QString(), words, tp::revisionColor(doc.reviewerIndex(r.author)), false});
        }
        if (marks.carets) carets.push_back(r.from);
    }
    auto joined = [](QVector<std::pair<int, int>> &in) {
        std::sort(in.begin(), in.end());
        QVector<std::pair<int, int>> out;
        for (const auto &sp : std::as_const(in)) {
            if (!out.isEmpty() && sp.first <= out.last().second) out.last().second = std::max(out.last().second, sp.second);
            else out.push_back(sp);
        }
        in = out;
    };
    joined(spans);
    joined(formatSpans);
    if (marks.comments)
        for (const Comment &c : commentsIn(doc, lo, hi)) {
            const QString initials = c.initials.isEmpty() ? c.author.left(2) : c.initials;
            comments.push_back({c.reference, initials, c.text, tp::revisionColor(doc.reviewerIndex(c.author)), c.done});
        }
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    const tp::RevisionMarkup &look = tp::revisionMarkup();
    const int placing = look.bars;
    const bool rightBar = placing == tp::RevisionMarkup::BarsRight || (placing == tp::RevisionMarkup::BarsOutside && page.number % 2 == 1);
    const QColor barColor = look.barColor.isValid() ? look.barColor : QColor(0xC0, 0x39, 0x2B);
    double lastBottom = -1e9;
    const double floor = page.size.height() - 18;   // (the balloons end here; the note of the ones left out is under it)
    int more = 0;
    for (int lineAt = 0; lineAt < lines.size(); ++lineAt) {
        const Line &line = lines[lineAt];
        const TextItem *box = line.box;
        const double marginRight = pageWidth - box->rect.right();
        const int a = line.a, b = lineAt + 1 < lines.size() ? std::max(line.b, lines[lineAt + 1].a) : line.b;
        const double top = box->rect.top() + line.rect.top(), height = std::max(1.0, line.rect.height());
        auto bar = [&](const QVector<std::pair<int, int>> &which) {
            auto it = std::lower_bound(which.begin(), which.end(), a, [](const std::pair<int, int> &sp, int v) { return sp.second <= v; });
            if (it == which.end() || it->first >= b) return;
            p->fillRect(QRectF(rightBar ? box->rect.right() + 5.4 : box->rect.left() - 7, top, 1.6, height), barColor);
            ++counts.bars;
        };
        if (placing != tp::RevisionMarkup::BarsNone) {
            if (marks.changeBars) bar(spans);
            if (marks.formatBars) bar(formatSpans);
        }
        for (const int at : carets) {
            if (at < a || at >= b) continue;
            int frame = 0;
            QRectF caret;
            if (!fl.layout->caretRect(at - page.storyFrom, &frame, &caret) || frame < 0 || frame >= boxes.size() || boxes[frame] != box) continue;
            const double x = box->rect.left() + caret.x(), y = top + height;
            QPainterPath mark;
            mark.moveTo(x - 2.5, y + 2.5);
            mark.lineTo(x, y - 0.5);
            mark.lineTo(x + 2.5, y + 2.5);
            p->setPen(QPen(tp::revisionColor(0), 0.9));
            p->setBrush(Qt::NoBrush);
            p->drawPath(mark);
            ++counts.carets;
        }
        auto balloon = [&](const Mark &m, bool deletion) {
            if (m.at < a || m.at >= b) return;
            if (marginRight < 30 || lastBottom >= floor) {
                ++more;
                return;
            }
            const double w = std::min(marginRight - 8, 120.0);
            QFont font = p->font();
            font.setPointSizeF(6.5);
            p->setFont(font);
            const QString shown = deletion ? QCoreApplication::translate("jp::write::PageView", "Deleted: %1").arg(m.text) : m.initials + QStringLiteral(": ") + m.text;
            const QRectF want(box->rect.right() + 6, std::max(top, lastBottom + 2), w, 0);
            const QRectF text = p->boundingRect(want.adjusted(3, 2, -3, 0).adjusted(0, 0, 0, 80), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, shown.left(120));
            const QRectF frame(want.left(), want.top(), w, std::min(78.0, text.height() + 5));
            if (frame.bottom() > floor) {
                ++more;
                lastBottom = floor;
                return;
            }
            QColor fill = m.color;
            fill.setAlpha(m.done ? 28 : 55);
            p->setPen(QPen(m.color, 0.7));
            p->setBrush(fill);
            p->drawRoundedRect(frame, 3, 3);
            p->setPen(m.done ? QColor(0x70, 0x70, 0x70) : QColor(0x20, 0x20, 0x20));
            p->drawText(frame.adjusted(3, 2, -3, -1), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, shown.left(120));
            lastBottom = frame.bottom();
            counts.lowest = std::max(counts.lowest, frame.bottom());
            ++(deletion ? counts.deletions : counts.comments);
        };
        for (const auto &c : comments) balloon(c, false);
        for (const auto &d : deletions) balloon(d, true);
    }
    if (more > 0) {
        QFont font = p->font();
        font.setPointSizeF(6.5);
        p->setFont(font);
        p->setPen(QColor(0x40, 0x40, 0x40));
        const QRectF note(page.size.width() - 64, page.size.height() - 16, 60, 12);
        p->drawText(note, Qt::AlignRight | Qt::AlignVCenter, QCoreApplication::translate("jp::write::PageView", "+%1 more").arg(more));
        counts.lowest = std::max(counts.lowest, note.bottom());
    }
    counts.more = more;
    p->restore();
    return counts;
}

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

QVector<int> sheetOfPages(WriteDoc &doc, const QVector<int> &pages, const SheetLayout &layout)
{
    QVector<int> out;
    for (const PagePlace &place : placesOfPages(doc, pages, layout)) out << place.sheet;
    return out;
}

bool paintSheets(WriteDoc &doc, QPagedPaintDevice *device, const QVector<int> &pages, int pageCount, bool grayscale, const SheetLayout &layout, double maxImageDpi, WriteTagger *tagger, const PrintLook *look)
{
    auto *printer = dynamic_cast<QPrinter *>(device);
    if (printer) printer->setFullPage(true);
    QPainter p;
    bool started = false;
    const QSize grid = sheetGrid(layout.perSheet);
    const int perSheet = grid.width() * grid.height();
    const bool shared = perSheet > 1 || (layout.paper.width() > 0 && layout.paper.height() > 0) || layout.turn != PrintTurn::AsSet;
    const int frameTag = tagger ? tagger->tags().add({QStringLiteral("Artifact"), QString(), -1, false}) : -1;
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
        if (look && look->marks.any()) {
            if (tagger) tagger->tags().begin(&p, frameTag);   // (the marks are decoration)
            const MarkupCounts drawn = paintMarkupMarks(&p, page, &cache, doc, look->marks);
            if (tagger) tagger->tags().end(&p, frameTag);
            if (g_markupObserver) g_markupObserver(page.number, drawn);
        }
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
            // The page's color is in a PDF (a reference PDF of such a file has it), and is not sent to a printer: paper is left white unless it is asked for (a job says which it wants).
            if (look && look->background.has_value() ? !*look->background : printer != nullptr) {
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
                // (the frame is decoration)
                if (tagger) tagger->tags().begin(&p, frameTag);
                p.drawRect(QRectF(x, y, w, h));
                if (tagger) tagger->tags().end(&p, frameTag);
                p.restore();
            }
        }
    }
    if (started) p.end();
    return started;
}

} // namespace jp::write
