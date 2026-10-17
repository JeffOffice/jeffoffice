#include "app/pagepaint.h"

#include "core/spellcheck.h"

#include <QPainter>
#include <QTextDocument>
#include <algorithm>

namespace jp::write {

namespace {

// The story that the page's text boxes show: the text of the document (a header, a footer, or a note is a story of its own).
QString bodyStoryId(const flow::MaterializedPage &page)
{
    for (const QString &id : page.boxes)
        if (const Item *item = page.doc->item(id))
            if (item->type() == ItemType::Text) return static_cast<const TextItem *>(item)->storyId;
    return QString();
}

std::function<void(const flow::MaterializedPage &, const PageDrawing &)> s_observer;

} // namespace

void setPaintObserver(const std::function<void(const flow::MaterializedPage &, const PageDrawing &)> &observer) { s_observer = observer; }

void paintPage(QPainter *p, flow::MaterializedPage &page, LayoutCache *layouts, const PageDrawing &d)
{
    if (s_observer) s_observer(page, d);
    PaintContext ctx;
    ctx.doc = page.doc.get();
    ctx.cache = layouts;
    ctx.pageNumber = page.number;
    ctx.pageCount = d.pageCount;
    ctx.opt.output = d.output;
    ctx.opt.textOutput = d.textOutput;
    ctx.opt.showSpecial = d.marks;
    ctx.opt.grayscale = d.grayscale;
    const QString bodyId = bodyStoryId(page);
    const Story *body = bodyId.isEmpty() ? nullptr : page.doc->story(bodyId);
    // The selection, in the positions of the page's own story.
    if (d.selectionFrom >= 0 && d.selectionTo > d.selectionFrom && body && body->doc) {
        const int from = std::max(0, d.selectionFrom - page.storyFrom), to = std::min(body->doc->characterCount() - 1, d.selectionTo - page.storyFrom);
        if (to > from) {
            ctx.opt.editStory = bodyId;
            ctx.opt.selFrom = from;
            ctx.opt.selTo = to;
        }
    }
    // The misspelled words, in the same positions.
    if (d.spelling && !d.output && body && body->doc) {
        const int length = body->doc->characterCount();
        QVector<QPair<int, int>> words;
        for (const auto &w : d.spelling->misspelledIn(page.storyFrom, page.storyFrom + length)) {
            if (d.typingAt >= w.first && d.typingAt <= w.second) continue;
            words << qMakePair(std::max(0, w.first - page.storyFrom), std::min(length, w.second - page.storyFrom));
        }
        if (!words.isEmpty()) ctx.opt.misspelled.insert(bodyId, words);
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
