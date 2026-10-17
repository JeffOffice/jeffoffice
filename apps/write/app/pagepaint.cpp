#include "app/pagepaint.h"

#include "core/spellcheck.h"
#include "core/tableops.h"

#include <QPainter>
#include <QPainterPath>
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

// A red wave under [x1, x2] at the height y, as the text engine draws the waves under the words of its own text.
void paintWave(QPainter *p, double x1, double x2, double y)
{
    QPainterPath wave;
    wave.moveTo(x1, y);
    for (double x = x1; x < x2; x += 2) wave.lineTo(x + 1, y + ((int(x / 2) % 2) ? -1 : 1));
    p->setPen(QPen(QColor(220, 30, 30), 0.6));
    p->setBrush(Qt::NoBrush);
    p->drawPath(wave);
}

// The misspelled words in the cells of the page's tables: the rows are drawn apart from the page's text, so their words are underlined here, in the places the
// rows' own lines are.
void paintCellWaves(QPainter *p, const flow::MaterializedPage &page, const PageDrawing &d)
{
    for (const auto &fragment : page.tables) {
        p->save();
        p->setClipRect(fragment->rect());
        fragment->forTexts([&](const StoryLayout &text) {
            const int first = text.firstPosition(0), last = text.lastPosition(0);
            if (first < 0) return;
            QVector<QPair<int, int>> words;
            for (const auto &w : d.spelling->misspelledIn(first, last + 1))
                if (!(d.typingAt >= w.first && d.typingAt <= w.second)) words << w;
            if (words.isEmpty()) return;
            // Where the text's own coordinates are on the page: a caret in it is in both.
            QLineF onPage;
            int frame = 0;
            QRectF inText;
            if (!fragment->caretLine(first, &onPage) || !text.caretRect(first, &frame, &inText)) return;
            const QPointF origin = onPage.p1() - inText.topLeft();
            for (const StoryLayout::LineInfo &line : text.lineInfo(0))
                for (const auto &w : words) {
                    const int s = std::max(w.first, line.docPos), e = std::min(w.second, line.docEnd);
                    if (s >= e) continue;
                    double x1 = 1e18, x2 = -1e18;
                    for (const QRectF &r : text.rangeRects(0, s, e)) {
                        x1 = std::min(x1, r.left());
                        x2 = std::max(x2, r.right());
                    }
                    if (x2 > x1) paintWave(p, origin.x() + x1, origin.x() + x2, origin.y() + line.baseline + 2);
                }
        });
        p->restore();
    }
}

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
            if (inTable(d.spelling->story()->findBlock(w.first))) continue;   // (the rows are not in the page's text: see paintCellWaves)
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
    if (d.spelling && !d.output) paintCellWaves(p, page, d);
}

} // namespace jp::write
