// What the paginator does for tables (see tableflow.h): the rows of a table are laid out as lines, one each, as tall as the row, set down the page between
// the text engine's runs over the paragraphs before and after, so a page break falls between rows as it does between paragraphs, and the rules of the
// page (what stays with what) see lines as they always did. A page that has rows of a table draws them as a TableFragment, and has in its story, in
// the place of the rows, one line as tall as they are.

#include "flow/paginator.h"

#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::flow {


void paintTables(QPainter *p, const MaterializedPage &page, const TablePaint &options)
{
    for (const std::shared_ptr<TableFragment> &f : page.tables) f->paint(p, options);
}

QVector<StoryLayout::LineBrief> Paginator::linesWithTables(const QVector<FrameSpec> &frames, const StoryLayout::Span &span, int firstFrame, int piece, int pieceRoom, bool *windowShort)
{
    m_pieces.clear();
    const TableIndex &ix = m_tables->index();
    const QTextDocument *story = m_doc->story();
    const int over = int(frames.size()) - 1;
    const double colW = frames[firstFrame].size.width();
    const double bodyH = frames[std::max(0, over - 1)].size.height();   // (a column's height: the same in each)
    QVector<StoryLayout::LineBrief> out;
    int pos = span.from, f = 0;      // where the next lines begin in the story, and the frame and the height used in it by then
    double y = 0;
    bool firstRun = true, atTop = false;
    for (;;) {
        // A table: its rows, from the one at the position, each as one line.
        if (const int sp = ix.spanAt(pos); sp >= 0) {
            const std::shared_ptr<TableLayout> table = m_tables->layout(sp);
            if (!table) return out;
            const int first = table->rowOfPosition(pos);
            for (int r = first; r < table->rowCount(); ++r) {
                const TableLayout::Row &row = table->row(r);
                const double height = table->rowHeight(r);
                int state = r == first ? piece : 0;
                double rowFirst = state != 0 ? pieceRoom / 20.0 : 0;   // (the room the row's first part had)
                // The header rows go above the first line of a page that the table goes on to, and take from the room of the parts of a row there.
                const double headers = TableFlow::repeatsHeader(*table, r, bodyH) ? table->headerHeight() : 0;
                for (bool placed = false; !placed;) {
                    // (A page that the table goes on to has the header rows above its first line.)
                    const double extra = out.isEmpty() ? headers : 0;
                    StoryLayout::LineBrief ln;
                    ln.block = row.from;
                    ln.pos = row.from;
                    ln.end = row.to;
                    ln.below = 0;
                    if (f >= over) {
                        // The page is full: this is the line that goes on the next (a row, or the part of one that is to come).
                        ln.frame = over;
                        ln.line = state;
                        ln.lines = state + 1;
                        ln.rect = QRectF(0, 0, colW, height + extra);
                        out << ln;
                        m_roomOut = int(std::lround(rowFirst * 20));
                        return out;
                    }
                    const double colH = frames[f].size.height();
                    const bool atTop = y < 0.001;
                    const double fullRoom = colH - headers;
                    if (state == 0 && (y + height + extra <= colH + 0.05 || (atTop && !table->splittable(r)))) {
                        ln.frame = f;
                        ln.line = 0;
                        ln.lines = 1;
                        ln.rect = QRectF(0, y, colW, height + extra);
                        out << ln;
                        y += height + extra;
                        placed = true;
                        continue;
                    }
                    if (state != 0 || table->splittable(r)) {
                        // The row, or what is left of it, over the foot of the column: the lines of its cells that fit, and the rest on the next.
                        if (state == 0) rowFirst = colH - y - extra;
                        const TableLayout::Piece part = table->pieceOf(r, state, rowFirst, colH - y - extra, fullRoom, atTop || state != 0);
                        if (part.progress) {
                            ln.frame = f;
                            ln.line = state;
                            ln.lines = part.next ? state + 2 : state + 1;
                            ln.rect = QRectF(0, y, colW, part.height + extra);
                            out << ln;
                            m_pieces.push_back({row.from, state, part});
                            y += part.height + extra;
                            if (!part.next) placed = true;
                            state = part.next;
                            if (part.next) {
                                m_roomOut = int(std::lround(rowFirst * 20));
                                ++f;   // (the rest of the row is in the next column, or on the next page)
                                y = 0;
                            }
                            continue;
                        }
                    }
                    // It does not fit, and is not cut: the next column, or the next page.
                    ++f;
                    y = 0;
                }
            }
            piece = 0;
            pos = ix.spans()[size_t(sp)].to;
            if (!story->findBlock(pos).isValid()) return out;
            firstRun = false;
            atTop = false;
            continue;
        }
        // Text, up to the next table.
        const int next = ix.firstFrom(pos);
        const int end = next < int(ix.spans().size()) ? ix.spans()[size_t(next)].from : -1;
        QVector<StoryLayout::LineBrief> briefs;
        std::unique_ptr<StoryLayout> layout;
        double shift = 0;
        for (;;) {
            QVector<FrameSpec> fr = frames;
            StoryLayout::Span t = span;
            if (!(firstRun && f == 0 && y == 0)) {
                // (Not the whole page's text: what is left of this column and the ones after it.)
                fr = frames.mid(f);
                fr[0].size.setHeight(f < over ? std::max(0.0, frames[f].size.height() - y) : frames[f].size.height());
                t = StoryLayout::Span();
                t.keepRules = span.keepRules;
                t.window = span.window;
                t.spaceBeforeFirst = !atTop;
            }
            t.from = pos;
            t.end = end;
            t.stopAtFrame = int(fr.size()) - 1;
            layout = std::make_unique<StoryLayout>();
            layout->build(story, fr, m_doc->env, t);
            briefs = layout->lineBriefs();
            shift = f < over ? y : 0;
            // A first line that does not fit what is left of the column goes to the next one (the engine puts a line at the top of an empty frame whether it fits or not).
            if (!(firstRun && f == 0 && y == 0) && f < over && !briefs.isEmpty() && briefs.first().frame == 0 && briefs.first().rect.bottom() - briefs.first().below > fr[0].size.height() + 0.01) {
                ++f;
                y = 0;
                atTop = true;
                continue;
            }
            break;
        }
        for (StoryLayout::LineBrief &l : briefs) {
            if (l.frame == 0) l.rect.translate(0, shift);
            if (l.frame >= 0) l.frame += f;
        }
        const bool full = std::any_of(briefs.begin(), briefs.end(), [&](const StoryLayout::LineBrief &l) { return l.frame < 0 || l.frame >= over; });
        out += briefs;
        if (layout->windowShort()) {
            *windowShort = true;
            return out;
        }
        if (full || end < 0) return out;
        // The text ended before a table: how far down it went, with the space after its last paragraph.
        for (int i = int(briefs.size()) - 1; i >= 0; --i) {
            if (briefs[i].frame < 0) continue;
            f = briefs[i].frame;
            y = briefs[i].rect.bottom() + std::max(0.0, story->findBlock(briefs[i].block).blockFormat().bottomMargin());
            break;
        }
        pos = end;
        firstRun = false;
        atTop = false;
    }
}

std::vector<Paginator::TableGroup> Paginator::tableGroups(const QVector<StoryLayout::LineBrief> &lines, int kept, int firstFrame)
{
    std::vector<TableGroup> out;
    const TableIndex &ix = m_tables->index();
    for (int i = 0; i < kept && i < lines.size(); ++i) {
        const StoryLayout::LineBrief &ln = lines[i];
        const int sp = ix.spanAt(ln.pos);
        if (sp < 0) continue;
        const std::shared_ptr<TableLayout> table = m_tables->layout(sp);
        if (!table) continue;
        const int row = table->rowOfPosition(ln.pos), frame = ln.frame - firstFrame;
        if (!out.empty() && out.back().span == sp && out.back().frame == frame && out.back().endRow == row) {
            out.back().endRow = row + 1;
            out.back().rect.setBottom(ln.rect.bottom());
            out.back().to = table->row(row).to;
        } else {
            out.push_back({sp, row, row + 1, frame, ln.rect, table->row(row).from, table->row(row).to, {}});
        }
        // A row that is on the page in part (it goes over a page break, or on from one).
        if (ln.line != 0 || ln.lines > 1)
            for (const PieceRecord &rec : m_pieces)
                if (rec.pos == ln.pos && rec.state == ln.line) out.back().parts.push_back({row, rec.piece});
    }
    return out;
}

bool Paginator::tablePlaceholder(QTextCursor &dst, QTextBlock &b, bool first, const std::vector<TableGroup> &groups) const
{
    for (const TableGroup &g : groups) {
        if (g.from != b.position()) continue;
        QTextBlockFormat bf;
        bf.setLineHeight(g.rect.height(), QTextBlockFormat::FixedHeight);
        bf.setTopMargin(0);
        bf.setBottomMargin(0);
        QTextCharFormat cf;
        cf.setFontPointSize(1);
        if (first) {
            dst.setBlockFormat(bf);
            dst.setBlockCharFormat(cf);
        } else {
            dst.insertBlock(bf, cf);
        }
        // (As many zero-width characters as the rows take in the story, less the end of the paragraph: so what is after it is where it is in the story.)
        dst.insertText(QString(std::max(0, g.to - g.from - 1), QChar(0x200B)), cf);
        QTextBlock last = b;
        while (last.isValid() && last.position() + last.length() < g.to) last = last.next();
        if (last.isValid()) b = last;
        return true;
    }
    return false;
}

QTextBlock Paginator::lastBlockWithTables(const QVector<StoryLayout::LineBrief> &lines, int kept, const std::vector<TableGroup> &groups) const
{
    const StoryLayout::LineBrief &last = lines[kept - 1];
    for (const TableGroup &g : groups)
        if (last.pos >= g.from && last.pos < g.to) return m_doc->story()->findBlock(g.to - 1);
    return m_doc->story()->findBlock(last.block);
}

int Paginator::pageOfTablePosition(int position)
{
    if (!m_tables->any()) return -1;
    const TableIndex &ix = m_tables->index();
    const int span = ix.spanAt(position);
    if (span < 0) return -1;
    const std::shared_ptr<TableLayout> table = m_tables->layout(span);
    if (!table) return -1;
    const int home = pageOfPosition(position);
    const int parts = page(home).start.piece;
    if (parts == 0) return home;
    // The row is in parts on the pages before this one, which is its last: its first is `parts` pages back, and the part that has the position's line is the first whose cell ends below it.
    const int r = table->rowOfPosition(position), cell = table->cellOfPosition(position);
    if (cell < 0) return home;
    const double y = table->lineTopIn(cell, position);
    if (y < 0) return home;
    const int first = home - parts;
    const double colH = m_doc->section(page(home).start.section).content().height();
    const double fullRoom = colH - (TableFlow::repeatsHeader(*table, r, colH) ? table->headerHeight() : 0);
    const double firstRoom = page(first + 1).start.pieceRoom / 20.0;
    const int at = cell - table->row(r).cell0;
    int lo = 0, hi = parts;
    while (lo < hi) {
        const int mid = (lo + hi) / 2;
        const TableLayout::Piece p = table->pieceOf(r, mid, firstRoom, firstRoom, fullRoom, true);
        if (at >= 0 && at < p.ends.size() && p.ends[at] > y) hi = mid;
        else lo = mid + 1;
    }
    return first + lo;
}

void Paginator::addTableFragments(MaterializedPage *page, const Section &sec, const std::vector<TableGroup> &groups)
{
    for (const TableGroup &g : groups) {
        const std::shared_ptr<TableLayout> table = m_tables->layout(g.span);
        if (!table) continue;
        const QRectF col = sec.columnRect(std::clamp(g.frame, 0, sec.columnCount() - 1));
        const std::shared_ptr<TableFragment> fragment = m_tables->fragment(table, g.firstRow, g.endRow, col.height(), g.parts);
        if (!fragment) continue;
        fragment->origin = QPointF(col.left() + table->leftEdgeIn(col.width()), col.top() + g.rect.top());
        page->tables.push_back(fragment);
    }
}

} // namespace jp::flow
