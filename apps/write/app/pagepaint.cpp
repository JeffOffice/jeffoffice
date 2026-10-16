#include "app/pagepaint.h"

#include <QPainter>
#include <QTextDocument>
#include <algorithm>

namespace jp::write {

void paintPage(QPainter *p, flow::MaterializedPage &page, LayoutCache *layouts, const PageDrawing &d)
{
    PaintContext ctx;
    ctx.doc = page.doc.get();
    ctx.cache = layouts;
    ctx.pageNumber = page.number;
    ctx.pageCount = d.pageCount;
    ctx.opt.output = d.output;
    ctx.opt.textOutput = d.textOutput;
    ctx.opt.showSpecial = d.marks;
    ctx.opt.grayscale = d.grayscale;
    // The selection, in the positions of the page's own story.
    if (d.selectionFrom >= 0 && d.selectionTo > d.selectionFrom && !page.doc->stories.isEmpty()) {
        const QString storyId = page.doc->stories.begin().key();
        const QTextDocument *story = page.doc->stories.begin().value()->doc.get();
        const int from = std::max(0, d.selectionFrom - page.storyFrom), to = std::min(story->characterCount() - 1, d.selectionTo - page.storyFrom);
        if (to > from) {
            ctx.opt.editStory = storyId;
            ctx.opt.selFrom = from;
            ctx.opt.selTo = to;
        }
    }
    Renderer::paintPage(p, ctx, 0);
    flow::TablePaint tables;
    tables.output = d.textOutput;
    tables.marks = d.marks;
    tables.selFrom = d.selectionFrom;
    tables.selTo = d.selectionTo;
    flow::paintTables(p, page, tables);
}

} // namespace jp::write
