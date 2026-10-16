// What the paginator does for tables (see tableflow.h): the rows of a table are laid out as lines, one each, as tall as the row, set down the page between
// the text engine's runs over the paragraphs before and after, so a page break falls between rows as it does between paragraphs, and the rules of the
// page (what stays with what) see lines as they always did. A page that has rows of a table draws them as a TableFragment, and has in its story, in
// the place of the rows, one line as tall as they are.

#include "flow/paginator.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::flow {


void paintTables(QPainter *p, const MaterializedPage &page, const TablePaint &options)
{
    // The objects set in the text of the cells are drawn as the page layer draws those of its text boxes: the pictures by the ids they have in the page's document.
    std::unique_ptr<LayoutCache> cache;
    TablePaint own = options;
    if (!own.objects && page.doc)
        own.objects = [&](QPainter *painter, const StoryLayout &text) {
            if (!cache) cache = std::make_unique<LayoutCache>();
            PaintContext ctx;
            ctx.doc = page.doc.get();
            ctx.cache = cache.get();
            ctx.pageNumber = page.number;
            for (const StoryLayout::InlineObject &ob : text.inlineObjects()) {
                if (ob.frame != 0) continue;
                QJsonObject json = QJsonDocument::fromJson(ob.json.toUtf8()).object();
                const QString ref = json.value(QStringLiteral("image")).toString();
                if (ref.startsWith(kFlowImagePrefix)) json.insert(QStringLiteral("image"), page.cellImages.value(ref));
                const ItemPtr item = Item::fromJsonAny(json);
                // (The layout keeps sizes sane; an object it had to change is not drawn.)
                if (!item || std::abs(item->rect.width() - ob.rect.width()) > 0.01 || std::abs(item->rect.height() - ob.rect.height()) > 0.01) continue;
                item->moveBy(ob.rect.left() - item->rect.left(), ob.rect.top() - item->rect.top());
                Renderer::paintItem(painter, ctx, *item);
            }
        };
    for (const std::shared_ptr<TableFragment> &f : page.tables) f->paint(p, own);
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

Paginator::NoteCut Paginator::cutNote(const QString &id, double width, double room)
{
    NoteCut cut;
    if (!m_doc->fields || room <= 0) return cut;
    // As the text engine cuts a note that a line cites at the foot of a frame: a story that cites it, laid out in a frame as tall as the line and the room, and a frame after it.
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
    double line = 0;
    {
        StoryLayout lay;
        lay.build(&d, {fs}, m_doc->env);
        const auto briefs = lay.lineBriefs();
        if (!briefs.isEmpty()) line = briefs.first().rect.bottom();
    }
    FrameSpec first = fs;
    first.size.setHeight(line + room);
    StoryLayout lay;
    lay.build(&d, {first, fs}, m_doc->env);
    for (const StoryLayout::NotePlace &pl : lay.notePlaces()) {
        if (pl.frame == 0 && pl.from == 0) cut.height = pl.rect.height();
        else if (pl.frame >= 1 && pl.from > 0) cut.from = pl.from;
    }
    return cut;
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
    // The notes a row (or the part of one) cites go to the foot of the frame it is in, in the room given (the rule above them, where the frame has none yet, is a part
    // of it): one that does not fit it is cut there, and goes on at the foot of the next page with those after it.
    auto seatNotes = [&](int frame, const QVector<NoteRef> &refs, double width, double colH, double room) {
        if (refs.isEmpty()) return;
        knowGaps();
        double used = led->reserve[frame] > 0 ? 0 : std::max(0.0, led->ruleGap);
        bool cutDone = false;
        for (const NoteRef &nr : refs) {
            const double h = noteHeight(nr.id, width);
            if (!cutDone && used + h + std::max(0.0, led->noteGap) <= room + 0.05) {
                led->places << StoryLayout::NotePlace{nr.id, nr.number, frame, 0, QRectF(0, 0, width, h), 0};
                used += h + std::max(0.0, led->noteGap);
                continue;
            }
            if (cutDone) {
                led->carried << StoryLayout::CarriedNote{nr.id, nr.number, 0, nr.pos};
                continue;
            }
            // The first that does not fit is cut in what is left of the room, and goes on from there.
            const NoteCut cut = cutNote(nr.id, width, std::max(0.0, room - used));
            if (cut.height > 0) {
                led->places << StoryLayout::NotePlace{nr.id, nr.number, frame, 0, QRectF(0, 0, width, cut.height), 0};
                used += cut.height + std::max(0.0, led->noteGap);
                if (cut.from == 0) continue;   // (it fits after all)
            }
            led->carried << StoryLayout::CarriedNote{nr.id, nr.number, cut.height > 0 ? cut.from : 0, nr.pos};
            cutDone = true;
        }
        led->reserve[frame] = std::min(colH, led->reserve[frame] + std::min(used, room));
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
            const int first = table->rowOfPosition(pos);
            for (int r = first; r < table->rowCount(); ++r) {
                const TableLayout::Row &row = table->row(r);
                const double height = table->rowHeight(r);
                int state = r == first ? piece : 0;
                QVector<double> offs;   // where in the row's cells the part to be placed begins (none: at the top)
                if (state != 0) offs = st.pieceAt;
                if (offs.size() != row.cells) {
                    offs.clear();
                    state = 0;   // (a page start that is not of this row: the row from its top)
                }
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
                        m_offsOut = offs;
                        return out;
                    }
                    const double colH = frames[f].size.height(), colW = frames[f].size.width();
                    const bool atTop = y < 0.001 && led->reserve[f] < 0.001;   // (at the top of a column that has no notes either: a row is put there whether it fits or not)
                    // The header rows go above the first line of a page that the table goes on to, and take from the room of the parts of a row there.
                    const double headers = TableFlow::repeatsHeader(*table, r, colH) ? table->headerHeight() : 0;
                    // (A page that the table goes on to has the header rows above its first line.)
                    const double extra = out.isEmpty() ? headers : 0;
                    ln.rect = QRectF(0, y, colW, height + extra);
                    const double avail = colH - y - extra - led->reserve[f];
                    table->setNoteHeights([this, colW, led](const QString &id) { return noteHeight(id, colW) + std::max(0.0, led->noteGap); });
                    if (state == 0) {
                        const double nh = heightOfNotes(rowNotes, f, colW);
                        if (height + nh <= avail + 0.05 || (atTop && !table->splittable(r))) {
                            ln.frame = f;
                            ln.line = 0;
                            ln.lines = 1;
                            out << ln;
                            y += height + extra;
                            seatNotes(f, rowNotes, colW, colH, colH - y - led->reserve[f]);
                            placed = true;
                            continue;
                        }
                    }
                    if (state != 0 || table->splittable(r)) {
                        // The row, or what is left of it, over the foot of the column: the lines of its cells that fit, and the rest on the next.
                        if (cites) knowGaps();
                        const TableLayout::Piece part = table->pieceOf(r, state, offs, avail, led->reserve[f] <= 0, led->ruleGap > 0 ? led->ruleGap : 0, atTop || state != 0);
                        if (part.progress) {
                            ln.frame = f;
                            ln.line = state;
                            ln.lines = part.next ? state + 2 : state + 1;
                            ln.rect = QRectF(0, y, colW, part.height + extra);
                            out << ln;
                            m_pieces.push_back({row.from, state, part});
                            y += part.height + extra;
                            seatNotes(f, part.notes, colW, colH, part.noteHeight + 0.01);
                            if (!part.next) placed = true;
                            state = part.next;
                            if (part.next) {
                                offs = part.ends;
                                m_offsOut = offs;
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
            y = briefs[i].rect.bottom() + layout->trailingRoom();
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
        // A row that is on the page in part (it goes over a page break, or on from one), or that was put there with the room it had (the space under its last line let go at the foot).
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
    const int r = table->rowOfPosition(position), cell = table->cellOfPosition(position);
    const int from = table->row(r).from;
    // A row that no page starts in is whole on the page that has it.
    if (page(home).start.pos != from || cell < 0) return home;
    const double y = table->lineTopIn(cell, position);
    if (y < 0) return home;
    const int at = cell - table->row(r).cell0;
    // The row is in parts on the pages that start in it, and on the one before them (unless the row begins a page). The part that has the position's line is the one whose
    // cell begins above the line and ends below it: found among the pages, each of which says what parts of the row it has.
    int lo = home;
    while (lo > 0 && page(lo - 1).start.pos == from) --lo;
    if (page(lo).start.piece != 0) --lo;
    int hi = home;
    while (lo < hi) {
        const int mid = (lo + hi) / 2;
        double top = 1e18, bottom = -1;
        bool found = false;
        for (const TableLayout::Piece &piece : partsOnPage(mid, span, r)) {
            if (at < 0 || at >= piece.ends.size() || at >= piece.offsets.size()) continue;
            top = std::min(top, piece.offsets[at]);
            bottom = std::max(bottom, piece.ends[at]);
            found |= y >= piece.offsets[at] - 0.01 && y < piece.ends[at];
        }
        if (found) return mid;
        if (bottom < 0) return home;   // (no part of the row on the page: not found)
        if (y < top) hi = mid - 1; else lo = mid + 1;
    }
    return std::clamp(lo, 0, home);
}

void Paginator::addTableFragments(MaterializedPage *page, const QVector<QRectF> &cols, const QRectF &body, const std::vector<TableGroup> &groups)
{
    for (const TableGroup &g : groups) {
        const std::shared_ptr<TableLayout> table = m_tables->layout(g.span);
        if (!table || cols.isEmpty()) continue;
        const QRectF col = cols[std::clamp(g.frame, 0, int(cols.size()) - 1)];
        const std::shared_ptr<TableFragment> fragment = m_tables->fragment(table, g.firstRow, g.endRow, col.height(), g.parts);   // (the column's height decides whether the header rows repeat, as it did when the lines were found)
        if (!fragment) continue;
        fragment->origin = QPointF(col.left() + table->leftEdgeIn(col.width()), body.top() + col.top() + g.rect.top());
        page->tables.push_back(fragment);
    }
}

} // namespace jp::flow
