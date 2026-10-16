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

struct Paginator::Ledger {
    QVector<double> reserve;                     // by frame: what the notes there take at its foot, the rule's gap included
    QVector<StoryLayout::NotePlace> places;      // the notes, as the text engine seated them (those of the text) and as the rows' were set (their rects are made again)
    QVector<StoryLayout::CarriedNote> carried;   // the notes that go on past the page
    double ruleGap = -1, noteGap = 0;            // the gaps the text engine leaves above and among notes (-1: not known yet)
    explicit Ledger(int frames) : reserve(frames, 0.0) {}
};

double Paginator::noteHeight(const QString &id, double width)
{
    if (!m_doc->fields) return 0;
    const QTextDocument *source = m_doc->fields->storyDoc(id);
    const auto key = std::make_tuple(id, int(std::lround(width * 100)), source ? source->revision() : -1);
    const auto found = m_noteHeights.find(key);
    if (found != m_noteHeights.end()) return found->second;
    // A note is as tall as the text engine sets it in a column of that width: a story that cites it, laid out, says.
    QTextDocument d;
    d.setDocumentMargin(0);
    {
        QTextCursor c(&d);
        QTextCharFormat ref;
        ref.setFontPointSize(11);
        ref.setProperty(tp::Field, QStringLiteral("footnote:") + id);
        c.insertText(QString(QChar(0xFFFC)), ref);
    }
    FrameSpec fs;
    fs.size = QSizeF(std::max(1.0, width), 100000);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.columns = 1;
    fs.hyphenate = m_doc->hyphenate;
    fs.hyphenZone = m_doc->hyphenZone;
    fs.ctx.doc = m_doc->fields;
    StoryLayout lay;
    lay.build(&d, {fs}, m_doc->env);
    const auto places = lay.notePlaces();
    const double h = places.isEmpty() ? 0 : places.first().rect.height();
    if (m_noteHeights.size() > 20000) m_noteHeights.clear();
    m_noteHeights[key] = h;
    return h;
}

int Paginator::sectionAt(const PageStart &st, int position) const
{
    // The sections end at the paragraphs that say so: those between the page's first paragraph and the position.
    int section = st.section;
    const QTextDocument *story = m_doc->story();
    for (QTextBlock b = story->findBlock(st.pos); b.isValid() && b.position() < position; b = b.next())
        if (b.blockFormat().boolProperty(fp::SectionBreakAfter) && section + 1 < int(m_doc->sections.size())) ++section;
    return section;
}

TableLayout::Rooms Paginator::partRooms(TableLayout &table, int row, int section, int numberOfFirst, double first, bool ruleAbove, double ruleGap)
{
    TableLayout::Rooms rooms;
    rooms.first = first;
    rooms.firstParity = ((numberOfFirst % 2) + 2) % 2;
    rooms.ruleAbove = ruleAbove;
    rooms.ruleGap = ruleGap;
    // The pages after the first: not the first of their section, and odd or even as the numbers go.
    for (int parity = 0; parity < 2; ++parity) {
        PageStart q;
        q.section = section;
        q.firstOfSection = false;
        const double body = furnitureOf(1 << 20, q, parity == 0 ? 2 : 1).body.height();
        rooms.full[parity] = body - (TableFlow::repeatsHeader(table, row, body) ? table.headerHeight() : 0);
    }
    return rooms;
}

QVector<StoryLayout::LineBrief> Paginator::linesWithTables(const QVector<FrameSpec> &frames, const StoryLayout::Span &span, int firstFrame, const PageStart &st, TablesRun *tr)
{
    Ledger ledger(int(frames.size()));
    const QVector<StoryLayout::LineBrief> out = tableLines(frames, span, firstFrame, st, tr, &ledger);
    restackNotes(frames, &ledger, tr);
    tr->carried = ledger.carried;
    return out;
}

void Paginator::restackNotes(const QVector<FrameSpec> &frames, Ledger *led, TablesRun *tr)
{
    tr->notes.clear();
    tr->rules.clear();
    std::stable_sort(led->places.begin(), led->places.end(), [](const StoryLayout::NotePlace &a, const StoryLayout::NotePlace &b) {
        if (a.frame != b.frame) return a.frame < b.frame;
        if (a.number != b.number) return a.number < b.number;
        return a.from < b.from;
    });
    const bool word = m_doc->env.wordLines;
    for (int i = 0; i < led->places.size();) {
        const int f = led->places[i].frame;
        int j = i;
        while (j < led->places.size() && led->places[j].frame == f) ++j;
        if (f < 0 || f >= frames.size()) {
            i = j;
            continue;
        }
        const double width = frames[f].size.width(), height = frames[f].size.height();
        double total = 0;
        for (int k = i; k < j; ++k) total += led->places[k].rect.height() + led->noteGap;
        const double areaTop = std::max(0.0, height - std::max(led->reserve[f], total + std::max(0.0, led->ruleGap)));
        double ny = areaTop + std::max(0.0, led->ruleGap);
        for (int k = i; k < j; ++k) {
            StoryLayout::NotePlace pl = led->places[k];
            pl.rect = QRectF(0, ny, width, pl.rect.height());
            ny += pl.rect.height() + led->noteGap;
            tr->notes << pl;
        }
        const double ruleY = areaTop + std::max(0.0, led->ruleGap) * (word ? 0.2 : 0.45);
        const double ruleW = led->places[i].from > 0 ? width : word ? std::min(width, 144.0) : width / 3;
        tr->rules << StoryLayout::Rule{f, QLineF(0, ruleY, ruleW, ruleY)};
        i = j;
    }
}

QVector<StoryLayout::LineBrief> Paginator::tableLines(const QVector<FrameSpec> &frames, const StoryLayout::Span &span, int firstFrame, const PageStart &st, TablesRun *tr, Ledger *led)
{
    int piece = st.piece;
    const int pieceRoom = st.pieceRoom;
    bool *windowShort = &tr->windowShort;
    m_pieces.clear();
    const TableIndex &ix = m_tables->index();
    const QTextDocument *story = m_doc->story();
    const int over = int(frames.size()) - 1;
    const bool notes = m_doc->fields != nullptr;
    QVector<StoryLayout::LineBrief> out;
    int pos = span.from, f = 0;      // where the next lines begin in the story, and the frame and the height used in it by then
    double y = 0;
    bool firstRun = true, atTop = false, firstSegment = true;
    // What a stretch of text laid out gives the page's notes: the room they take in each frame, where they are, and those that go on past the page.
    auto absorb = [&](const StoryLayout &lay, int base) {
        const QVector<double> res = lay.noteReserves();
        for (int k = 0; k < res.size() && base + k < led->reserve.size(); ++k) led->reserve[base + k] = std::max(led->reserve[base + k], res[k]);
        if (led->ruleGap < 0) {
            led->ruleGap = lay.noteRuleGap();
            led->noteGap = lay.noteGap();
            m_ruleGap = led->ruleGap;
        }
        for (StoryLayout::NotePlace pl : lay.notePlaces()) {
            pl.frame += base;
            led->places << pl;
        }
        for (const StoryLayout::CarriedNote &cn : lay.carriedNotes(lay.frameCount() - 1)) {
            bool known = false;
            for (const StoryLayout::CarriedNote &c : std::as_const(led->carried)) known |= c.storyId == cn.storyId && c.number == cn.number;
            if (!known) led->carried << cn;
        }
    };
    // The gaps the text engine leaves among notes, found by laying out no text at all, for a page that has notes of rows and no text before them.
    auto knowGaps = [&]() {
        if (led->ruleGap >= 0) return;
        StoryLayout::Span t;
        t.from = pos;
        t.end = pos;
        StoryLayout lay;
        lay.build(story, frames.mid(0, 1), m_doc->env, t);
        led->ruleGap = lay.noteRuleGap();
        led->noteGap = lay.noteGap();
        m_ruleGap = led->ruleGap;
    };
    // The notes a row (or the part of one) cites go to the foot of the frame it is in.
    auto addNotes = [&](int frame, const QVector<NoteRef> &refs, double height, double width, double colH) {
        for (const NoteRef &nr : refs) led->places << StoryLayout::NotePlace{nr.id, nr.number, frame, 0, QRectF(0, 0, width, noteHeight(nr.id, width)), 0};
        led->reserve[frame] = std::min(colH, led->reserve[frame] + height);
    };
    auto heightOfNotes = [&](const QVector<NoteRef> &refs, int frame, double width) {
        if (refs.isEmpty()) return 0.0;
        knowGaps();
        double h = led->reserve[frame] > 0 ? 0 : led->ruleGap;
        for (const NoteRef &nr : refs) h += noteHeight(nr.id, width) + led->noteGap;
        return h;
    };
    // The notes that went on from the page before come first at the foot of the first frame: with the text before the first table, the text engine seats them, and with
    // none, a stretch of no text does.
    if (notes && !st.carried.isEmpty() && ix.spanAt(pos) >= 0) {
        StoryLayout::Span t = span;
        t.end = pos;
        StoryLayout lay;
        lay.build(story, frames, m_doc->env, t);
        absorb(lay, 0);
        firstSegment = false;
    }
    for (;;) {
        // A table: its rows, from the one at the position, each as one line.
        if (const int sp = ix.spanAt(pos); sp >= 0) {
            const std::shared_ptr<TableLayout> table = m_tables->layout(sp);
            if (!table) return out;
            const int tableSection = sectionAt(st, ix.spans()[size_t(sp)].from);
            const int first = table->rowOfPosition(pos);
            for (int r = first; r < table->rowCount(); ++r) {
                const TableLayout::Row &row = table->row(r);
                const double height = table->rowHeight(r);
                int state = r == first ? piece : 0;
                double rowFirst = state != 0 ? firstOfRoom(pieceRoom) : 0;   // (the room the row's first part had)
                bool ruleAbove0 = state != 0 && ruleAboveOfRoom(pieceRoom);
                const bool cites = notes && table->rowHasNotes(r);
                const QVector<NoteRef> rowNotes = cites ? table->rowNotes(r) : QVector<NoteRef>();
                for (bool placed = false; !placed;) {
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
                        ln.rect = QRectF(0, 0, frames[over].size.width(), height);
                        out << ln;
                        m_roomOut = roomOfPart(rowFirst, ruleAbove0);
                        return out;
                    }
                    const double colH = frames[f].size.height(), colW = frames[f].size.width();
                    const bool atTop = y < 0.001;
                    // The header rows go above the first line of a page that the table goes on to, and take from the room of the parts of a row there.
                    const double headers = TableFlow::repeatsHeader(*table, r, colH) ? table->headerHeight() : 0;
                    // (A page that the table goes on to has the header rows above its first line.)
                    const double extra = out.isEmpty() ? headers : 0;
                    ln.rect = QRectF(0, y, colW, height + extra);
                    const double avail = colH - y - extra - led->reserve[f];
                    table->setNoteHeights([this, colW](const QString &id) { return noteHeight(id, colW); });
                    if (state == 0) {
                        const double nh = heightOfNotes(rowNotes, f, colW);
                        if (height + nh <= avail + 0.05 || (atTop && !table->splittable(r))) {
                            ln.frame = f;
                            ln.line = 0;
                            ln.lines = 1;
                            out << ln;
                            y += height + extra;
                            addNotes(f, rowNotes, nh, colW, colH);
                            placed = true;
                            continue;
                        }
                    }
                    if (state != 0 || table->splittable(r)) {
                        // The row, or what is left of it, over the foot of the column: the lines of its cells that fit, and the rest on the next.
                        if (state == 0) {
                            rowFirst = avail;
                            ruleAbove0 = led->reserve[f] > 0;
                        }
                        if (cites) knowGaps();
                        const TableLayout::Rooms rooms = partRooms(*table, r, tableSection, tr->pageNumber - state, rowFirst, ruleAbove0, led->ruleGap > 0 ? led->ruleGap : 0);
                        const TableLayout::Piece part = table->pieceOf(r, state, rooms, avail, atTop || state != 0);
                        if (part.progress) {
                            ln.frame = f;
                            ln.line = state;
                            ln.lines = part.next ? state + 2 : state + 1;
                            ln.rect = QRectF(0, y, colW, part.height + extra);
                            out << ln;
                            m_pieces.push_back({row.from, state, part});
                            y += part.height + extra;
                            addNotes(f, part.notes, part.noteHeight, colW, colH);
                            if (!part.next) placed = true;
                            state = part.next;
                            if (part.next) {
                                m_roomOut = roomOfPart(rowFirst, ruleAbove0);
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
        int base = 0;
        for (;;) {
            QVector<FrameSpec> fr = frames;
            StoryLayout::Span t = span;
            base = 0;
            if (!(firstRun && f == 0 && y == 0 && firstSegment)) {
                // (Not the whole page's text: what is left of this column and the ones after it.)
                fr = frames.mid(f);
                fr[0].size.setHeight(f < over ? std::max(0.0, frames[f].size.height() - y) : frames[f].size.height());
                base = f;
                t = StoryLayout::Span();
                t.keepRules = span.keepRules;
                t.window = span.window;
                t.overLines = span.overLines;
                t.restartNotesAt = span.restartNotesAt;
                t.spaceBeforeFirst = !atTop;
                if (notes) {
                    // The notes before it, counted from the page's first paragraph; and what the notes before already take at the foot of each frame.
                    int footnotes = 0, endnotes = 0;
                    bool restarted = false;
                    const int from = story->findBlock(st.pos).position();
                    countNotes(from, std::max(from, pos), &footnotes, &endnotes, &restarted);
                    t.footnotesBefore = (restarted ? 0 : st.foot) + footnotes;
                    t.endnotesBefore = (restarted ? 0 : st.end) + endnotes;
                    for (int k = f; k < led->reserve.size(); ++k) t.noteReserves << led->reserve[k];
                }
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
        if (notes) absorb(*layout, base);
        firstSegment = false;
        if (layout->windowShort()) {
            *windowShort = true;
            return out;
        }
        if (full || end < 0) return out;
        // The text ended before a table: how far down it went, with the space after its last paragraph.
        for (int i = int(briefs.size()) - 1; i >= 0; --i) {
            if (briefs[i].frame < 0) continue;
            f = briefs[i].frame;
            y = briefs[i].rect.bottom() + (m_doc->env.wordLines ? layout->trailingSpace() : std::max(0.0, story->findBlock(briefs[i].block).blockFormat().bottomMargin()));
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
    const Page continuing = page(first + 1);
    const int stored = continuing.start.pieceRoom;
    const double colWidth = continuing.content.width();
    table->setNoteHeights([this, colWidth](const QString &id) { return noteHeight(id, colWidth); });
    const TableLayout::Rooms rooms = partRooms(*table, r, continuing.start.section, page(home).number - parts, firstOfRoom(stored), ruleAboveOfRoom(stored), m_ruleGap > 0 ? m_ruleGap : 0);
    const int at = cell - table->row(r).cell0;
    int lo = 0, hi = parts;
    while (lo < hi) {
        const int mid = (lo + hi) / 2;
        const TableLayout::Piece p = table->pieceOf(r, mid, rooms, page(first + mid).content.height(), true);
        if (at >= 0 && at < p.ends.size() && p.ends[at] > y) hi = mid;
        else lo = mid + 1;
    }
    return first + lo;
}

void Paginator::addTableFragments(MaterializedPage *page, const QVector<QRectF> &cols, const QRectF &body, const std::vector<TableGroup> &groups)
{
    for (const TableGroup &g : groups) {
        const std::shared_ptr<TableLayout> table = m_tables->layout(g.span);
        if (!table || cols.isEmpty()) continue;
        const QRectF col = cols[std::clamp(g.frame, 0, int(cols.size()) - 1)];
        const std::shared_ptr<TableFragment> fragment = m_tables->fragment(table, g.firstRow, g.endRow, body.height(), g.parts);
        if (!fragment) continue;
        fragment->origin = QPointF(col.left() + table->leftEdgeIn(col.width()), body.top() + col.top() + g.rect.top());
        page->tables.push_back(fragment);
    }
}

} // namespace jp::flow
