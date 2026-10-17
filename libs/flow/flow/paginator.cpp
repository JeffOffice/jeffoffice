#include "flow/paginator.h"

#include "text/textprops.h"

#include <QElapsedTimer>
#include <QHash>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>
#include <limits>

namespace jp::flow {

namespace {

// How tall the frames for a paragraph's lines on the page before, and for what runs over the page, are: more than a paragraph takes.
constexpr double kTall = 100000;
// How many characters of a paragraph a page lays out to begin with (a page holds a few thousand; if the lines of a longer paragraph need more
// than that to fill the page and show what comes after it, the page is laid out again with four times as many).
constexpr int kWindow = 6000;
// The least of a paragraph a page that begins inside it lays out to begin with, and how many notes in the lines of a paragraph before the page make it
// laid out from the page on.
constexpr int kLeastWindow = 300;
constexpr int kDenseNotes = 64;
// How many times a page is laid out again to make the objects the text goes round and the lines agree about where each is, and the most the objects seen before are kept to
// know that they go in circles.
constexpr int kWrapPasses = 4;

// Whether the page shows the number of pages (in its text, its header, or its footer), and what number it was made with.
void markTotals(MaterializedPage *out, const Document &d)
{
    for (const auto &story : d.stories)
        for (QTextBlock b = story->doc->begin(); b.isValid() && !out->showsTotal; b = b.next())
            for (auto it = b.begin(); !it.atEnd() && !out->showsTotal; ++it) {
                const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                out->showsTotal = code == QLatin1String("pages") || code == QLatin1String("sectionpages");
            }
    out->pageTotal = d.flowPageTotal;
}

} // namespace

// What a paragraph asks of the pages.
struct Paginator::BlockFlags {
    bool breakBefore = false, breakAfter = false, sectionAfter = false, keepNext = false, widow = false, outOfFlow = false;
};

// A page laid out: the story's lines on it, and where it ended.
struct Paginator::Run {
    // The lines from the first on the page (those of a first paragraph that the pages before hold are left out), then those that run over.
    QVector<StoryLayout::LineBrief> lines;
    int firstFrame = 0;        // the frame of the first column
    int columns = 1;
    // Each column's place on the page: its left, its top below the text area's, its width, and its height. (A page where a section with other columns begins below
    // the one before has the first's column, then the second's.)
    QVector<QRectF> cols;
    int kept = 0;              // the lines on the page
    bool blank = false;        // the page the pages end with after a break at the end of the text: it has nothing on it
    bool firstSpace = true;    // space before the first paragraph is kept (it isn't if the page starts without it, or it was as tall as the page)
    bool hasNext = false;
    bool keepsSpace = false;   // the first paragraph is the rest of one after a break, and has its space before
    QVector<StoryLayout::NotePlace> notes;   // the footnotes as the page's layout put them
    QVector<StoryLayout::Rule> rules;        // and the rules above them
    PageStart next;            // where the page after starts
    std::vector<TableGroup> groups;   // the rows of tables among the lines kept
    QVector<Wrapped> wrapped;  // the objects the text of the page was laid out round
    bool cut = false;          // the page ends before the paragraph it was asked to end before
};

Paginator::Paginator(FlowDocument *doc) : m_doc(doc), m_tables(std::make_unique<TableFlow>(doc)), m_watch(std::make_unique<QObject>())
{
    m_starts << PageStart();
    QObject::connect(doc->story(), &QTextDocument::contentsChange, m_watch.get(), [this](int position, int removed, int added) { changed(position, removed, added); });
}

Paginator::~Paginator() = default;

int Paginator::numberOf(int index) const
{
    bool restarts = false;
    for (const Section &s : m_doc->sections) restarts |= s.firstPageNumber.has_value();
    if (!restarts) return index + 1;
    // The page after the one asked for last, in the same section: one more.
    if (index > 0 && index == m_numberOf + 1 && index < m_starts.size() && m_starts[index].section == m_starts[index - 1].section) {
        m_numberOf = index;
        return ++m_number;
    }
    int number = index + 1;
    for (int p = std::min(index, int(m_starts.size()) - 1); p >= 0; --p, ++m_numberSteps)
        if (p == 0 || m_starts[p].section != m_starts[p - 1].section) {
            const std::optional<int> first = m_doc->section(m_starts[p].section).firstPageNumber;
            if (first) {
                number = *first + (index - p);
                break;
            }
        }
    m_numberOf = index;
    m_number = number;
    return number;
}

// Where the text goes on after a page break that is the end of `last`, the last line laid out, for a page that has no line after it: the next line of the
// paragraph if it has text to show or (where the mark follows the break) the mark's own empty line, else the paragraph after it. (A row of a table has no
// break that ends it.) Says whether there is such a place.
bool Paginator::breakContinuesAt(const StoryLayout::LineBrief &last, PageStart *next) const
{
    if (m_tables->any() && m_tables->index().spanAt(last.pos) >= 0) return false;
    const QTextDocument *story = m_doc->story();
    const QTextBlock block = story->findBlock(last.pos);
    if (!block.isValid()) return false;
    const bool onlyTheMarkLeft = last.end >= block.position() + block.length() - 1;
    if (last.line + 1 < last.lines && (!onlyTheMarkLeft || m_doc->markFollowsBreak)) {
        next->pos = last.end;
        next->skip = last.line + 1;
        return true;
    }
    if (!block.next().isValid()) return false;
    next->pos = block.next().position();
    next->skip = 0;
    return true;
}

Paginator::Run Paginator::layoutPage(int index, const PageStart &st)
{
    ++m_laidOut;
    if (!hasFloaters()) return layoutPageWith(index, st, {});
    // A page laid out before, as it is now, ends with the walls it did.
    if (const auto memo = m_wallMemo.find(index); memo != m_wallMemo.end() && memo->second.start == st && memo->second.pages == int(m_starts.size()))
        return layoutPageWith(index, st, memo->second.walls, memo->second.cutAt);
    // The objects the lines anchor, and the text goes round, are set where the lines say; the text goes round them there, which moves the lines, and the page is laid out
    // again until they agree (the paragraph's top an object is set from is where the paragraph would be without it: that is what the engine's `pushed` is for). `moveBefore`:
    // the paragraph that a table set at a place goes over the foot from, as the page is when its walls are settled. `cycle`: where they do not settle, the walls of the layouts
    // that follow one another again and again.
    auto settle = [&](Run &run, QVector<Wrapped> &used, int cutAt, int *moveBefore, QVector<QVector<Wrapped>> *cycle) {
        QVector<QVector<Wrapped>> seen;
        bool agreed = true;
        for (int pass = 0; pass < kWrapPasses; ++pass) {
            int mb = -1;
            const QVector<Wrapped> want = wrappersOf(run, index, st, &mb);
            agreed = want == used;
            if (agreed) {
                if (moveBefore) *moveBefore = mb;
                break;
            }
            if (seen.contains(want)) {
                if (cycle) {
                    *cycle = seen.mid(int(seen.indexOf(want)));
                    cycle->push_back(used);
                }
                break;
            }
            seen << used;
            used = want;
            run = layoutPageWith(index, st, used, cutAt);
            if (pass + 1 == kWrapPasses && cycle) {
                *cycle = seen.mid(std::max<qsizetype>(0, seen.size() - 2));
                cycle->push_back(used);
            }
        }
        if (!used.isEmpty() && !agreed) agreed = wrappersOf(run, index, st) == used;
        return agreed;
    };
    Run run = layoutPageWith(index, st, {});
    QVector<Wrapped> used;
    int moveBefore = -1, cutAt = -1;
    QVector<QVector<Wrapped>> cycle;
    bool agreed = settle(run, used, -1, &moveBefore, &cycle);
    // Objects that keep moving the lines, and so themselves (an anchor that the wrap of another puts on the next page, which has the wrap of the first no more, and the anchor
    // is back): the one anchored last of them goes to the next page with its paragraph, where it is set from the paragraph's top as it asks (the page ends before the paragraph),
    // and the objects before it are walls of this page still.
    if (!agreed && !cycle.isEmpty()) {
        QVector<int> flipping;
        for (const QVector<Wrapped> &state : cycle)
            for (const Wrapped &w : state) {
                bool everywhere = true;
                for (const QVector<Wrapped> &other : cycle) everywhere = everywhere && other.contains(w);
                if (!everywhere && w.paragraph >= 0 && !flipping.contains(w.paragraph)) flipping << w.paragraph;
            }
        std::sort(flipping.begin(), flipping.end(), std::greater<int>());
        for (const int paragraph : flipping) {
            Run shorter = layoutPageWith(index, st, {}, paragraph);
            if (!shorter.cut) continue;
            QVector<Wrapped> walls;
            if (settle(shorter, walls, paragraph, &moveBefore, nullptr) && shorter.kept > 0) {
                run = std::move(shorter);
                used = walls;
                cutAt = paragraph;
                agreed = true;
                break;
            }
        }
    }
    // Objects that keep moving the lines and so themselves in some other way are not walls of this page: the lines are those of the page without them, and every wall there is
    // has its anchor on the page. And a page whose objects leave no room for a line would have none, and the pages would never end: it is laid out as it is without them too.
    if (cutAt < 0 && !used.isEmpty() && (!agreed || run.kept <= 0)) {
        used.clear();
        run = layoutPageWith(index, st, {});
        moveBefore = -1;
        wrappersOf(run, index, st, &moveBefore);
    }
    // A table set at a place that goes over the foot of the page from its paragraph, as the page is when its walls are settled, is moved to the next page with the paragraph: the
    // page ends before the paragraph, and its walls are those of the page without the table.
    if (moveBefore >= 0 && (cutAt < 0 || moveBefore < cutAt)) {
        Run cut = layoutPageWith(index, st, used, moveBefore);
        if (cut.cut) {
            cutAt = moveBefore;
            run = std::move(cut);
            if (!settle(run, used, cutAt, nullptr, nullptr) || run.kept <= 0) {
                used.clear();
                run = layoutPageWith(index, st, {}, cutAt);
            }
        }
    }
    if (m_wallMemo.size() >= 4096) m_wallMemo.clear();
    m_wallMemo[index] = WallMemo{st, int(m_starts.size()), run.wrapped, cutAt};
    return run;
}

Paginator::Run Paginator::layoutPageWith(int index, const PageStart &st, const QVector<Wrapped> &wrapped, int cutAt)
{
    ++m_passes;
    Run run;
    run.wrapped = wrapped;
    // The page a break at the end of the text makes: it starts past the text's last character.
    // (With notes that go on from the page before, it is a page of notes alone.)
    if (st.pos >= m_doc->story()->characterCount() && st.carried.isEmpty()) {
        run.blank = true;
        return run;
    }
    // The page an odd-page or even-page section break leaves blank, before the section's own.
    if (st.blank) {
        run.blank = true;
        run.hasNext = true;
        run.next = st;
        run.next.blank = false;
        run.next.section = std::min(st.section + 1, int(m_doc->sections.size()) - 1);
        run.next.firstOfSection = true;
        run.next.hard = true;
        return run;
    }
    const Section &sec = m_doc->section(st.section);
    // (The text after a page break that begins its paragraph is the paragraph's first line: it has the space before. The rest of a paragraph after a break that has
    // something before it on its line goes on, with none.)
    const QTextBlock atStart = m_doc->story()->findBlock(st.pos);
    const bool breakFirst = atStart.isValid() && m_doc->story()->characterAt(atStart.position()) == kPageBreak;
    run.keepsSpace = st.hard && st.skip > 0 && breakFirst;
    const double colW = sec.columnWidth();
    const int number = numberOf(index);
    const QRectF body = furnitureOf(index, st, number).body;
    const double colH = body.height();
    auto frame = [&](double width, double height) {
        FrameSpec fs;
        fs.size = QSizeF(width, height);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.columns = 1;
        fs.hyphenate = m_doc->hyphenate;
        fs.hyphenZone = m_doc->hyphenZone;
        fs.noteRoomLimit = colH / 4;   // (the text area of this page: a note separator or a notice takes at most a quarter of it)
        fs.ctx.doc = m_doc->fields;
        fs.ctx.pageNumber = number;
        fs.ctx.pageCount = std::max<int>(number, m_starts.size());
        return fs;
    };
    // A frame that is a column of the page (`column`: where, in page coordinates) has the objects the page's text goes round as places it keeps out of.
    auto walled = [&](FrameSpec fs, const QRectF &column) {
        if (!wrapped.isEmpty()) fs.obstacles = obstaclesFor(wrapped, column);
        return fs;
    };
    QHash<int, BlockFlags> cache;
    auto flags = [&](int blockStart) -> const BlockFlags & {
        auto it = cache.find(blockStart);
        if (it == cache.end()) {
            const QTextBlockFormat bf = m_doc->story()->findBlock(blockStart).blockFormat();
            BlockFlags f;
            if (bf.intProperty(fp::Cell)) {
                // A row of a table: of its paragraphs only a page break before the table's first asks anything of the page.
                const int span = m_tables->index().spanAt(blockStart);
                f.breakBefore = span >= 0 && m_tables->index().spans()[size_t(span)].from == blockStart && (bf.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore);
                it = cache.insert(blockStart, f);
                return it.value();
            }
            f.breakBefore = bf.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore;
            f.breakAfter = bf.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter;
            f.sectionAfter = bf.boolProperty(fp::SectionBreakAfter);
            f.keepNext = bf.boolProperty(tp::KeepWithNext);
            f.widow = bf.boolProperty(tp::WidowControl);
            f.outOfFlow = bf.boolProperty(tp::OutOfFlow);
            it = cache.insert(blockStart, f);
        }
        return it.value();
    };
    // The page is a frame for each column, and one more for the text that runs over. If the first paragraph began on the page before, either
    // a frame before them is where its lines that went there are put (the paragraph is laid out whole and so is as the text sets it), or, if
    // it has many lines, the page lays out the rest of it as a paragraph of its own.
    // A page that a break inside a paragraph opens has the rest of the paragraph with its space before again, as the paragraph of its own it is laid out as.
    const bool afterBreak = st.hard && st.skip > 0;
    // (A paragraph with many notes in the lines before the page is laid out from the page's first line: each of the lines before costs all of its notes
    // again, on every page after it.)
    bool denseNotes = false;
    if (st.skip > 0 && st.pos > 0) {
        int footnotes = 0, endnotes = 0;
        countNotes(m_doc->story()->findBlock(st.pos).position(), st.pos, &footnotes, &endnotes);
        denseNotes = footnotes + endnotes > kDenseNotes;
    }
    const bool tail = afterBreak || denseNotes || st.wrapped || st.skip > std::max(0, m_doc->exactLines);
    // How much of the paragraph the page lays out to begin with: a page that holds little of it (one with many notes) needs a few lines more than it
    // holds, not a page of them. (It is laid out again with more when that isn't enough.)
    const int heldBefore = index > 0 && index <= m_starts.size() - 1 ? std::max(1, m_starts[index].pos - m_starts[index - 1].pos) : kWindow;
    const int firstWindow = tail ? std::clamp(3 * heldBefore, kLeastWindow, kWindow) : kWindow + st.skip * 160;
    // Where a section with other columns begins on the page (below the text of the one before, in a column of the page's width): how far down the
    // text area, and the paragraph that ends the first section. (None: -1.)
    double split = -1;
    int splitBlock = -1;
    int over = 0, kept = 0;
    QStringList carriedOut;
    QVector<int> carriedAt;   // where the references of those are (-1: before the page)
    // A paragraph that runs on after the page is laid out as far as the lines wanted need (its window): the lines the rules look at past the page,
    // a line more, and the last, which is cut short.
    const int wanted = std::max({m_doc->widowLines, m_doc->orphanLines, 2}) + 2;
    // The text of the page laid out, by the engine or beside the tables: its lines, the notes it seats and the rules above them, those that go on past it, and the room
    // the notes take at the foot of each frame.
    struct Part {
        QVector<StoryLayout::LineBrief> lines;
        QVector<StoryLayout::NotePlace> notes;
        QVector<StoryLayout::Rule> rules;
        QVector<StoryLayout::CarriedNote> carried;
        QVector<double> reserve;
        std::vector<PieceRecord> pieces;
        bool windowShort = false;
    };
    auto layPart = [&](const QVector<FrameSpec> &fr, const StoryLayout::Span &given, const PageStart &from) {
        Part p;
        // (The paragraphs of frames that the paginator does not set apart are laid out as the text's own.)
        StoryLayout::Span sp = given;
        sp.inLayout = &floatIndex().inText;
        if (m_tables->any()) {
            // (Tables are laid out beside the text: paginator_tables.cpp.)
            TablesRun tr;
            tr.pageIndex = index;
            tr.pageNumber = number;
            p.lines = linesWithTables(fr, sp, 0, from, &tr);
            p.notes = tr.notes;
            p.rules = tr.rules;
            p.windowShort = tr.windowShort;
            p.carried = tr.carried;
            p.reserve = tr.reserve;
            p.pieces = m_pieces;
        } else {
            StoryLayout layout;
            layout.build(m_doc->story(), fr, m_doc->env, sp);
            p.lines = layout.lineBriefs();
            p.notes = layout.notePlaces();
            p.rules = layout.noteRules();
            p.windowShort = layout.windowShort();
            p.carried = layout.carriedNotes(int(fr.size()) - 1);
            p.reserve = layout.noteReserves();
        }
        return p;
    };
    bool splitFailed = false;
    int stopBefore = -1;   // where the text of the page stops when a break ends it before the engine's frames run out (the start of a paragraph; none: -1)
    auto lay = [&]() {
        QVector<FrameSpec> frames;
        if (st.skip > 0 && !tail) frames << frame(colW, std::max(kTall, st.skip * 5000.0));
        run.firstFrame = int(frames.size());
        run.cols.clear();
        // (A page where a section with other columns begins below the text of the one before is laid out in two parts: that text, in a column as tall as the page, with its notes at
        // the foot; and then the columns of the next section, from where the text ended to above those notes.)
        const bool splitting = split >= 0;
        const Section &nextSection = m_doc->section(std::min<int>(st.section + 1, int(m_doc->sections.size()) - 1));
        if (splitting) {
            frames << walled(frame(colW, colH), QRectF(sec.columnRect(0).left(), body.top(), colW, colH));
            run.cols << QRectF(sec.columnRect(0).left(), 0, colW, colH);
            for (int c = 0; c < nextSection.columnCount(); ++c) {
                frames << walled(frame(nextSection.columnWidth(), std::max(1.0, colH - split)), QRectF(nextSection.columnRect(c).left(), body.top() + split, nextSection.columnWidth(), std::max(1.0, colH - split)));
                run.cols << QRectF(nextSection.columnRect(c).left(), split, nextSection.columnWidth(), std::max(1.0, colH - split));
            }
        } else {
            for (int c = 0; c < sec.columnCount(); ++c) {
                frames << walled(frame(colW, colH), QRectF(sec.columnRect(c).left(), body.top(), colW, colH));
                run.cols << QRectF(sec.columnRect(c).left(), 0, colW, colH);
            }
        }
        run.columns = int(run.cols.size());
        frames << frame(colW, kTall);
        over = int(frames.size()) - 1;

        StoryLayout::Span span;
        span.from = st.pos;
        span.end = stopBefore;
        span.firstBreakAfter = st.skip > 0 && !tail ? st.skip : -1;
        span.continued = tail;
        span.spaceBeforeContinued = afterBreak && breakFirst;
        span.stopAtFrame = over;
        span.overLines = wanted + 2;
        span.restartNotesAt = notesRestartIn(st.pos, kWindow * 64);
        // The notes before the page's first paragraph (before its first line, if the paragraph is laid out from there) and the footnotes cut on the page before.
        span.footnotesBefore = st.foot;
        span.endnotesBefore = st.end;
        if (tail && st.pos > 0) {
            int footnotes = 0, endnotes = 0;
            countNotes(m_doc->story()->findBlock(st.pos).position(), st.pos, &footnotes, &endnotes);
            span.footnotesBefore += footnotes;
            span.endnotesBefore += endnotes;
        }
        for (const QString &c : st.carried) {
            const QStringList f = c.split(QLatin1Char('|'));
            if (f.size() == 3) span.carried << StoryLayout::CarriedNote{f[0], f[1].toInt(), f[2].toInt()};
        }
        kept = 0;
        for (bool space = st.hard;; space = false) {
            span.spaceBeforeFirst = space;
            QVector<StoryLayout::LineBrief> all;
            // (A paragraph that is laid out whole from its start, with the lines before the page, needs room for those too.)
            for (int window = firstWindow;; window *= 4) {
                span.window = window;
                Part part;
                if (!splitting) {
                    part = layPart(frames, span, st);
                } else {
                    // The first section's text, in the frame as tall as the page, up to the paragraph that ends it.
                    const QTextBlock endBlock = m_doc->story()->findBlock(splitBlock);
                    const int splitEnd = endBlock.position() + endBlock.length();
                    QVector<FrameSpec> f1;
                    if (run.firstFrame > 0) f1 << frames[0];
                    f1 << walled(frame(colW, colH), QRectF(sec.columnRect(0).left(), body.top(), colW, colH)) << frame(colW, kTall);
                    StoryLayout::Span s1 = span;
                    s1.end = splitEnd;
                    s1.stopAtFrame = int(f1.size()) - 1;
                    Part p1 = layPart(f1, s1, st);
                    double splitY = -1;
                    bool fits = !p1.windowShort;
                    for (const auto &ln : p1.lines) {
                        if (ln.frame >= run.firstFrame + 1) fits = false;
                        if (ln.block == splitBlock && ln.line == ln.lines - 1 && ln.frame == run.firstFrame) splitY = ln.rect.bottom();
                    }
                    if (!fits || splitY < 0) {
                        splitFailed = true;
                        return;
                    }
                    const double reserved = p1.reserve.value(run.firstFrame, 0.0);
                    const double columnH = std::max(1.0, colH - splitY - reserved);
                    // The second section's columns.
                    QVector<FrameSpec> f2;
                    for (int c = 0; c < nextSection.columnCount(); ++c)
                        f2 << walled(frame(nextSection.columnWidth(), columnH), QRectF(nextSection.columnRect(c).left(), body.top() + splitY, nextSection.columnWidth(), columnH));
                    f2 << frame(colW, kTall);
                    PageStart st2 = st;
                    st2.pos = splitEnd;
                    st2.skip = 0;
                    st2.piece = 0;
                    st2.pieceAt.clear();
                    st2.carried.clear();
                    st2.hard = true;
                    {
                        int footnotes = 0, endnotes = 0;
                        bool restarted = false;
                        const int from = m_doc->story()->findBlock(st.pos).position();
                        countNotes(from, std::max(from, splitEnd), &footnotes, &endnotes, &restarted);
                        st2.foot = (restarted ? 0 : st.foot) + footnotes;
                        st2.end = (restarted ? 0 : st.end) + endnotes;
                        const QTextBlock first2 = m_doc->story()->findBlock(splitEnd);
                        if (first2.isValid() && restartsNotesAt(first2)) st2.foot = st2.end = 0;
                    }
                    StoryLayout::Span s2;
                    s2.from = splitEnd;
                    s2.stopAtFrame = int(f2.size()) - 1;
                    s2.overLines = span.overLines;
                    s2.window = span.window;
                    s2.keepRules = span.keepRules;
                    s2.spaceBeforeFirst = true;
                    s2.restartNotesAt = notesRestartIn(splitEnd, kWindow * 64);
                    s2.footnotesBefore = st2.foot;
                    s2.endnotesBefore = st2.end;
                    Part p2 = layPart(f2, s2, st2);
                    // Where the columns have no room for a line (the notes and the text above them leave less than a line), the engine puts the first line of each in all the same, over
                    // the notes: the section's text goes to the next page, in the frame past the page.
                    int toOver = -1;
                    for (const auto &ln : p2.lines)
                        if (ln.frame >= 0 && ln.frame < nextSection.columnCount() && ln.rect.bottom() - ln.below > columnH + 0.05) toOver = int(nextSection.columnCount());
                    if (toOver >= 0) {
                        QVector<FrameSpec> f3;
                        f3 << f2.last();
                        StoryLayout::Span s3 = s2;
                        s3.stopAtFrame = 0;
                        p2 = layPart(f3, s3, st2);
                        for (auto &ln : p2.lines)
                            if (ln.frame >= 0) ln.frame += toOver;
                        for (auto &n : p2.notes)
                            if (n.frame >= 0) n.frame += toOver;
                        for (auto &r : p2.rules) r.frame += toOver;
                    }
                    // Together: the frames of the second part come after the first's.
                    const int shift = run.firstFrame + 1;
                    part = p1;
                    // (What the first part set in its frame past the page is carried on, and is not on this page.)
                    part.notes.erase(std::remove_if(part.notes.begin(), part.notes.end(), [&](const StoryLayout::NotePlace &n) { return n.frame > run.firstFrame; }), part.notes.end());
                    part.rules.erase(std::remove_if(part.rules.begin(), part.rules.end(), [&](const StoryLayout::Rule &r) { return r.frame > run.firstFrame; }), part.rules.end());
                    for (StoryLayout::LineBrief l : p2.lines) {
                        if (l.frame >= 0) l.frame += shift;
                        part.lines << l;
                    }
                    for (StoryLayout::NotePlace n : p2.notes) {
                        if (n.frame >= 0) n.frame += shift;
                        part.notes << n;
                    }
                    for (StoryLayout::Rule r : p2.rules) {
                        r.frame += shift;
                        part.rules << r;
                    }
                    part.carried += p2.carried;
                    part.windowShort = p2.windowShort;
                    part.pieces.insert(part.pieces.end(), p2.pieces.begin(), p2.pieces.end());
                    run.cols.clear();
                    run.cols << QRectF(sec.columnRect(0).left(), 0, colW, colH);
                    for (int c = 0; c < nextSection.columnCount(); ++c) run.cols << QRectF(nextSection.columnRect(c).left(), splitY, nextSection.columnWidth(), columnH);
                }
                all = part.lines;
                run.notes = part.notes;
                run.rules = part.rules;
                if (m_tables->any()) m_pieces = part.pieces;
                const bool windowShort = part.windowShort;
                carriedOut.clear();
                carriedAt.clear();
                for (const auto &cn : part.carried) {
                    carriedOut << QStringLiteral("%1|%2|%3").arg(cn.storyId).arg(cn.number).arg(cn.from);
                    carriedAt << cn.refPos;
                }
                int past = 0;
                bool cut = false;
                for (const auto &l : all) {
                    cut |= l.cut;
                    past += l.cut && (l.frame < 0 || l.frame >= over);
                }
                if ((!windowShort && (!cut || past >= wanted)) || window > std::numeric_limits<int>::max() / 8) break;
            }
            int first = 0;
            while (first < all.size() && all[first].frame >= 0 && all[first].frame < run.firstFrame) ++first;
            const int firstBlock = all.isEmpty() ? 0 : all.first().block;
            run.lines = all.mid(first);
            for (auto &l : run.lines) {
                if (tail && l.block == firstBlock) {
                    l.line += st.skip;   // (the lines of the paragraph before are not laid out, but they are numbered)
                    l.lines += st.skip;
                }
                if (l.cut) l.lines = 1 << 28;   // (how many it has isn't known)
            }
            kept = 0;
            while (kept < run.lines.size() && run.lines[kept].frame >= run.firstFrame && run.lines[kept].frame < over) ++kept;
            // Lines that take no room are on the page the text before them ends on, though the engine, finding no room for a line in the frame (as it first reckons
            // the line), put them in the next: the empty paragraph that ends a section (before a continuous one it takes none; before a section on a page of its own it is
            // only where the section breaks), and the first line of a paragraph that holds only the page break it begins with.
            while (kept < run.lines.size() && run.lines[kept].frame >= over && run.lines[kept].line == 0) {
                const StoryLayout::LineBrief &ln = run.lines[kept];
                const BlockFlags &bf = flags(ln.block);
                const bool noRoom = (bf.sectionAfter && (bf.outOfFlow || (ln.pos == ln.end && ln.lines == 1))) || (ln.rect.height() < 0.001 && ln.end == ln.pos + 1 && m_doc->story()->characterAt(ln.pos) == kPageBreak);
                if (!noRoom) break;
                ++kept;
            }
            run.firstSpace = space;
            // Space before the first paragraph as tall as the page, or nearly, leaves no room for a line: it is cut to what the page holds (none).
            if (kept > 0 || !space || run.lines.isEmpty()) break;
        }
    };
    lay();
    // A continuous section with other columns or side margins, when the text of the one before ends on the page, begins below it: the page is laid out again in two parts.
    if (st.section + 1 < m_doc->sections.size() && sec.columnCount() == 1) {
        const Section &next = m_doc->section(st.section + 1);
        if (next.continuous && sec.sameSheetAs(next) && !(sec.sameColumnsAs(next) && sec.sameSidesAs(next))) {
            for (int i = 0; i < kept; ++i) {
                const StoryLayout::LineBrief &ln = run.lines[i];
                const BlockFlags &bf = flags(ln.block);
                if ((ln.line == 0 && i > 0 && bf.breakBefore) || (ln.line == ln.lines - 1 && bf.breakAfter)) break;
                if (ln.line == ln.lines - 1 && bf.sectionAfter) {
                    split = ln.rect.bottom();
                    splitBlock = ln.block;
                    break;
                }
            }
            if (split >= 0) {
                lay();
                if (splitFailed) {   // (the text did not end where the page showed: the page as it was)
                    split = -1;
                    splitBlock = -1;
                    lay();
                }
            }
        }
    }
    const QVector<StoryLayout::LineBrief> &L = run.lines;
    // Where the page ends: where the text ran out of room, unless a break asks for less.
    int c = kept;
    bool hard = false, sectionEnds = false, forced = false, byCharacter = false;
    // A section that begins on the page of the one before, with the same page (a continuous section break), is no end of the page; nor is one with
    // other columns after the paragraph the page was laid out in two parts at.
    auto continuesOn = [&](int section, int block) {
        if (section + 1 >= m_doc->sections.size() || !m_doc->sections[section + 1].continuous) return false;
        const Section &a = m_doc->sections[section], &b = m_doc->sections[section + 1];
        return a.samePageAs(b) || (a.sameSheetAs(b) && section == st.section && block == splitBlock);
    };
    int current = st.section;
    for (int i = 0; i < c; ++i) {
        const StoryLayout::LineBrief &ln = L[i];
        const BlockFlags &bf = flags(ln.block);
        if (ln.line == 0 && i > 0 && bf.breakBefore) {
            c = i;
            hard = forced = true;
            break;
        }
        // The page ends before a paragraph that a table set from it would go over the foot with.
        if (cutAt >= 0 && ln.line == 0 && i > 0 && ln.block >= cutAt) {
            c = i;
            forced = run.cut = true;
            break;
        }
        if (ln.line == ln.lines - 1 && (bf.breakAfter || bf.sectionAfter)) {
            if (!bf.breakAfter && continuesOn(current, ln.block)) {
                ++current;
            } else {
                c = i + 1;
                hard = forced = true;
                sectionEnds = bf.sectionAfter;
                break;
            }
        }
        // A page break character: the page ends after the line that holds it.
        if (!ln.cut && ln.end > ln.pos && m_doc->story()->characterAt(ln.end - 1) == kPageBreak) {
            c = i + 1;
            hard = forced = byCharacter = true;
            // (The empty line the text engine puts after a break that ends its paragraph stays with the page: the next paragraph is at the top
            // of the next.)
            if (!m_doc->markFollowsBreak && c < L.size() && L[c].block == L[i].block && L[c].line == L[i].line + 1 && L[c].pos == L[c].end && L[c].line == L[c].lines - 1) ++c;
            break;
        }
    }
    if (!forced && c < L.size()) {
        // Widow and orphan control and keeping with the next paragraph, which only ever take lines off the page. (The text engine
        // has done what it does of this; this is for the paragraph that began pages before, and for more lines than it keeps.)
        // (Each time round the page ends earlier, so it ends: at most once for each line.)
        const int natural = c;
        while (c >= 1 && c < L.size()) {
            const StoryLayout::LineBrief &last = L[c - 1], &after = L[c];
            int begin = c - 1;
            while (begin > 0 && L[begin - 1].block == last.block) --begin;
            const bool began = L[begin].line == 0;   // the paragraph began on this page
            if (last.block == after.block) {
                if (!flags(last.block).widow) break;
                const int onPage = c - begin, left = last.lines - (last.line + 1);
                if (began && onPage < m_doc->orphanLines && begin > 0) {
                    c = begin;
                    continue;
                }
                if (left < m_doc->widowLines) {
                    const int want = c - (m_doc->widowLines - left);
                    if (want - begin >= (began ? m_doc->orphanLines : 1)) {
                        c = want;
                        continue;
                    }
                    if (began && begin > 0) {
                        c = begin;   // not room for both: the paragraph goes whole
                        continue;
                    }
                }
                break;
            }
            if (flags(last.block).keepNext && began) {
                if (begin > 0) {
                    c = begin;
                    continue;
                }
                c = natural;   // the paragraphs kept with the next begin the page and a page can't hold them: it's cut where the text ran out of room
            }
            break;
        }
        // The page after starts for a reason of its own if its first paragraph asks for a page.
        if (c < L.size() && L[c].line == 0 && flags(L[c].block).breakBefore) hard = true;
    }
    // A break that ends the page takes the lines after it off the page, but the text engine laid them out in the page's frame, and the room it kept at the foot for
    // the notes of those lines stays under the notes that are the page's: the page is laid out again as far as the break, for where its notes go.
    if (forced && c > 0 && c < kept && stopBefore < 0) {
        const QTextBlock lastBlock = m_doc->story()->findBlock(L[c - 1].block);
        const int blockEnd = lastBlock.position() + lastBlock.length();
        bool cites = false;   // (a footnote's reference, whether or not it takes a number)
        for (QTextBlock b = m_doc->story()->findBlock(blockEnd); b.isValid() && b.position() < L[kept - 1].end && !cites; b = b.next())
            for (auto it = b.begin(); !it.atEnd() && !cites; ++it)
                cites = it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("footnote:"));
        if (L[c].block >= blockEnd && cites) {
            const QVector<StoryLayout::LineBrief> lines = run.lines;
            const int keptBefore = kept;
            stopBefore = blockEnd;
            lay();
            run.lines = lines;
            kept = keptBefore;
        }
    }
    run.kept = c;
    if (m_tables->any()) run.groups = tableGroups(L, c, run.firstFrame);
    // A note goes on from the page when its reference is in a line the page keeps: the text engine laid out the lines that a break takes off the page too, and
    // cut the notes of those that didn't fit, which belong to the page they end up on.
    if (c < kept && !carriedOut.isEmpty()) {
        const int end = c > 0 ? L[c - 1].end : st.pos;
        QStringList stays;
        for (int i = 0; i < carriedOut.size(); ++i)
            if (carriedAt.value(i, -1) < end) stays << carriedOut[i];
        carriedOut = stays;
    }
    // The section the page after starts in: those that began on this page (continuous breaks among its lines) and, if a break ends it, the next.
    current = st.section;
    for (int i = 0; i < c; ++i) {
        const StoryLayout::LineBrief &ln = L[i];
        if (ln.line == ln.lines - 1 && flags(ln.block).sectionAfter && !flags(ln.block).breakAfter && continuesOn(current, ln.block) && !(sectionEnds && i == c - 1)) ++current;
    }
    const int nextSection = sectionEnds ? std::min(current + 1, int(m_doc->sections.size()) - 1) : current;
    const QTextBlock goesOn = c > 0 && c == L.size() && flags(L[c - 1].block).sectionAfter ? m_doc->story()->findBlock(L[c - 1].pos).next() : QTextBlock();
    if (c < L.size()) {
        run.hasNext = true;
        run.next.pos = L[c].pos;
        run.next.skip = L[c].line;
        // (Lines of the paragraph that a wall narrowed are on this page, or on a page before it when this page began inside the same paragraph: the page after sets the rest
        // of it from its character, as this one did, and not by counting lines that a full-width layout of the whole paragraph breaks elsewhere.)
        run.next.wrapped = L[c].line > 0 && (!wrapped.isEmpty() || (st.wrapped && L[0].block == L[c].block));
        if (m_tables->any() && m_tables->index().spanAt(L[c].pos) >= 0) {   // (a row of a table: the line's number is the part of it)
            run.next.piece = L[c].line;
            run.next.pieceAt = L[c].line != 0 ? m_offsOut : QVector<double>();
            run.next.skip = 0;
            run.next.wrapped = false;
        }
        run.next.hard = hard;
        run.next.section = nextSection;
        // A section of odd or even pages whose first page would be of the other kind, or one that restarts the numbers (where the pages are of odd and even kinds) at a
        // number of the other kind than the page it begins, has a blank page of the section before it.
        const Section &entered = m_doc->section(nextSection);
        const bool oddPhysical = (index + 2) % 2 != 0;   // (the page the section would begin, by its place in the document)
        bool wrong = false;
        if (entered.startParity != Section::Parity::Any) wrong = oddPhysical != (entered.startParity == Section::Parity::Odd);
        else if (entered.differentOddEven && entered.firstPageNumber) wrong = oddPhysical != (std::abs(*entered.firstPageNumber) % 2 != 0);
        if (sectionEnds && nextSection != current && !entered.continuous && wrong) {
            run.next.blank = true;
            run.next.section = current;
            run.next.firstOfSection = false;
        }
    } else if (byCharacter && c > 0 && breakContinuesAt(L[c - 1], &run.next)) {
        // A page break that ends the last line the page laid out (the empty line after it went past the frame, where laying out stops): the page
        // after it begins where the text goes on.
        run.hasNext = true;
        run.next.hard = true;
        run.next.section = nextSection;
    } else if ((forced && !byCharacter && c > 0) || !carriedOut.isEmpty() || goesOn.isValid()) {
        // A break after the text's last line: the pages go on with one that has nothing on it. Or notes go on past the text's last line: with pages of their own.
        // (Or the text goes on after a section's last, empty paragraph that took the page's last place, though no line's room was left: the lines laid out end with it.)
        run.hasNext = true;
        run.next.pos = goesOn.isValid() ? goesOn.position() : m_doc->story()->characterCount();
        run.next.skip = 0;
        run.next.hard = forced || !goesOn.isValid();
        run.next.section = nextSection;
    }
    if (run.hasNext) {
        // What the page after starts from: the notes through this page's paragraphs (those before the next page's first paragraph), and what was cut.
        const QTextDocument *story = m_doc->story();
        const int from = story->findBlock(st.pos).position();
        const QTextBlock nb = story->findBlock(run.next.pos);
        const int to = nb.isValid() ? nb.position() : story->characterCount();
        int footnotes = 0, endnotes = 0;
        bool restarted = false;
        countNotes(from, std::max(from, to), &footnotes, &endnotes, &restarted);
        run.next.foot = (restarted ? 0 : st.foot) + footnotes;
        run.next.end = (restarted ? 0 : st.end) + endnotes;
        // (The numbers begin again at a paragraph that begins a section: the page that starts with it, not the page after one that a section began in the middle of.)
        if (nb.isValid() && restartsNotesAt(nb)) run.next.foot = run.next.end = 0;
        run.next.carried = carriedOut;
        run.next.firstOfSection = !run.next.blank && run.next.section != st.section;
    }
    return run;
}

std::vector<TableLayout::Piece> Paginator::partsOnPage(int index, int span, int row)
{
    std::vector<TableLayout::Piece> out;
    if (index < 0 || index >= m_starts.size()) return out;
    const Run run = layoutPage(index, m_starts[index]);
    for (const TableGroup &g : run.groups)
        for (const TableFragment::Part &part : g.parts)
            if (g.span == span && part.row == row) out.push_back(part.piece);
    return out;
}

bool Paginator::restartsNotesAt(const QTextBlock &block) const
{
    if (!m_doc->restartNotesInSections) return false;
    const QTextBlock before = block.previous();
    return before.isValid() && before.blockFormat().boolProperty(fp::SectionBreakAfter);
}

QVector<int> Paginator::notesRestartIn(int from, int characters) const
{
    QVector<int> places;
    if (!m_doc->restartNotesInSections) return places;
    const QTextDocument *story = m_doc->story();
    int looked = 0;
    for (QTextBlock b = story->findBlock(from).next(); b.isValid() && b.position() <= from + characters && places.size() < 1000 && ++looked < 50000; b = b.next())
        if (restartsNotesAt(b)) places << b.position();
    return places;
}

void Paginator::countNotes(int from, int to, int *footnotes, int *endnotes, bool *restarted) const
{
    *footnotes = *endnotes = 0;
    if (restarted) *restarted = false;
    const QTextDocument *story = m_doc->story();
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        if (b.position() > from && restartsNotesAt(b)) {
            *footnotes = *endnotes = 0;
            if (restarted) *restarted = true;
        }
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const QString code = frag.charFormat().stringProperty(tp::Field);
            if (code.isEmpty()) continue;
            const int lo = std::max(frag.position(), from), hi = std::min(frag.position() + frag.length(), to);
            if (hi <= lo) continue;
            if (code.startsWith(QLatin1String("footnote:"))) *footnotes += frag.charFormat().boolProperty(tp::NoteMarkCustom) ? 0 : hi - lo;   // (a custom mark takes no number)
            else if (code.startsWith(QLatin1String("endnote:"))) *endnotes += hi - lo;
        }
    }
}

void Paginator::extend(int index)
{
    while (!m_pending && !m_complete && (index < 0 || m_starts.size() <= index)) {
        const Run run = layoutPage(int(m_starts.size()) - 1, m_starts.back());
        if (!run.hasNext || run.next == m_starts.back()) {
            m_complete = true;
            break;
        }
        m_starts << run.next;
    }
}

int Paginator::paginate(int index)
{
    settle(index);
    extend(index);
    return int(m_starts.size());
}

bool Paginator::paginateFor(double milliseconds)
{
    QElapsedTimer t;
    t.start();
    while ((m_pending || !m_complete) && t.nsecsElapsed() < milliseconds * 1e6) {
        if (m_pending) settleStep();
        else extend(int(m_starts.size()));
    }
    return !m_pending && m_complete;
}

int Paginator::pagesKnown()
{
    return int(m_starts.size());
}

bool Paginator::complete()
{
    return !m_pending && m_complete;
}

void Paginator::invalidate()
{
    forgetFloats();
    m_tables->reset();
    m_noteHeights.clear();
    m_numberOf = -1;
    m_starts.clear();
    m_starts << PageStart();
    m_complete = false;
    m_pending = false;
}

void Paginator::invalidateFrom(int section)
{
    forgetFloats();
    int keep = 0;
    while (keep < m_starts.size() && m_starts[keep].section < section) ++keep;
    keep = std::max(keep, 1);
    if (keep >= m_starts.size()) return;
    m_numberOf = -1;
    m_starts.resize(keep);
    m_complete = false;
    // (Pages that edits had not been settled to are still waiting; none are beyond the last.)
    m_high = std::min(m_high, keep - 1);
    m_valid = std::min(m_valid, keep - 1);
    if (m_pending && m_valid >= keep - 1) m_pending = false;
}

Paginator::Page Paginator::page(int index)
{
    settle(index);
    Page p;
    index = std::clamp(index, 0, int(m_starts.size()) - 1);
    p.start = m_starts[index];
    const Section &sec = m_doc->section(p.start.section);
    p.number = numberOf(index);
    p.size = sec.pageSize();
    p.content = furnitureOf(index, p.start, p.number).body;
    p.columns = sec.columnCount();
    p.columnGap = sec.columnGap;
    return p;
}

int Paginator::pageOfPosition(int position)
{
    // (A frame, or a table set at a place, is on the page of the paragraph it is set from.)
    position = anchorOfPosition(position);
    for (;;) {
        const auto it = std::upper_bound(m_starts.begin(), m_starts.end(), position, [](int pos, const PageStart &s) { return pos < s.pos; });
        const int found = std::max(0, int(it - m_starts.begin()) - 1);
        // (A page after an edit is only where it was until it's laid out again: the place may be another page's then.)
        if (m_pending && found > m_valid) {
            settle(found);
            continue;
        }
        if (it != m_starts.end() || m_complete) return found;
        // More pages are needed: those the edits touched first, then new ones.
        if (m_pending) settleStep();
        else extend(int(m_starts.size()));
    }
}

// ---- edits ----

void Paginator::forgetFloats()
{
    m_floats = FloatIndex();
    m_frames.clear();
    m_tableObjects.clear();
    m_anchorChars.clear();
    m_wallMemo.clear();
}

void Paginator::changed(int position, int removed, int added)
{
    forgetFloats();
    m_tables->edited(position, removed, added);
    const int delta = added - removed;
    m_numberOf = -1;
    // The pages' places move with the text; one that began inside what was taken out is gone.
    int w = 0, goneBefore = 0;
    const int startsBefore = int(m_starts.size());
    for (int r = 0; r < m_starts.size(); ++r) {
        PageStart s = m_starts[r];
        if (s.pos > position && s.pos < position + removed) {
            goneBefore += m_pending && r <= m_high;
            continue;
        }
        if (s.pos >= position + removed && s.pos > position) s.pos += delta;
        m_starts[w++] = s;
    }
    m_starts.resize(w);
    const int goneByTheEdit = startsBefore - w;
    m_high -= goneBefore;
    // The paragraphs the edit touches, and the page that holds the first (and the one before, for what the page before did to make
    // room for it: a heading kept with its paragraph, a widow).
    const QTextDocument *story = m_doc->story();
    const int last = std::max(0, story->characterCount() - 1);
    const QTextBlock b0 = story->findBlock(std::min(position, last)), b1 = story->findBlock(std::min(position + added, last));
    int from = b0.isValid() ? b0.position() : 0;
    int to = b1.isValid() ? b1.position() + b1.length() : last + 1;
    // The paragraphs of a table that is set at a place are too: its cells' (and the table's own place is where it is set from the paragraph after it, which the text round the
    // table is, and the pages from that on are laid out again). That is so for an edit that begins in the table, and for one that begins before it and goes into it (the cells of
    // a row put in at its top, opened at the end of the paragraph before the table) or through it.
    if (m_tables->any() && b0.isValid()) {
        auto afterPlaced = [&](const QTextBlock &x) {
            if (!x.blockFormat().intProperty(fp::Cell)) return QTextBlock();
            const CellRecord *rec = m_doc->tables.get(x.blockFormat().intProperty(fp::Cell));
            while (rec && rec->parent) rec = m_doc->tables.get(rec->parent);
            if (!rec || !rec->table || !rec->table->place) return QTextBlock();
            QTextBlock after = x;
            while (after.isValid() && after.blockFormat().intProperty(fp::Cell)) after = after.next();
            return after;
        };
        if (const QTextBlock after = afterPlaced(b0); after.isValid()) {
            from = after.position();
            to = std::max(to, after.position() + after.length());
        }
        int looked = 0;
        for (QTextBlock x = b0.next(); x.isValid() && b1.isValid() && x.position() <= b1.position() && ++looked <= 20000; x = x.next())
            if (const QTextBlock after = afterPlaced(x); after.isValid()) to = std::max(to, after.position() + after.length());
    }
    // The paragraphs of a frame are set where the paragraph after them is (they have no lines of their own): an edit in them touches the page of that paragraph.
    if (!m_doc->frames.isEmpty()) {
        QTextBlock after = b0;
        while (after.isValid() && after.blockFormat().intProperty(fp::Frame) > 0) after = after.next();
        if (after.isValid() && after != b0) {
            from = after.position();
            to = std::max(to, after.position() + after.length());
        }
        QTextBlock end = b1;
        while (end.isValid() && end.blockFormat().intProperty(fp::Frame) > 0) end = end.next();
        if (end.isValid() && end != b1) to = std::max(to, end.position() + end.length());
    }
    // A frame next to the edit may have another paragraph to be set from, or be cut in two or joined to another by it: the paragraphs it was set from and is set from now, the
    // first two of the text that come after the edit (those frames' paragraphs are no text), are touched, and the pages that have them are laid out again.
    if (!m_doc->frames.isEmpty() && b0.isValid() && b1.isValid()) {
        auto isFrame = [](const QTextBlock &x) { return x.isValid() && x.blockFormat().intProperty(fp::Frame) > 0 && x.blockFormat().boolProperty(tp::OutOfLayout); };
        if (isFrame(b0) || isFrame(b1) || isFrame(b0.previous()) || isFrame(b1.next())) {
            QTextBlock n = b1.next();
            for (int text = 0; n.isValid() && text < 2; n = n.next()) {
                if (isFrame(n)) continue;
                to = std::max(to, n.position() + n.length());
                ++text;
            }
        }
    }
    // The rows of tables that the edit touched, with those that cells joined down tie them to: what a page in them is laid out as depends on all of them.
    if (m_tables->any()) {
        m_tables->settleEdits();
        int dirtyFrom = 0, dirtyTo = 0;
        if (m_tables->takeDirty(&dirtyFrom, &dirtyTo)) {
            const QTextBlock d0 = story->findBlock(std::clamp(dirtyFrom, 0, last)), d1 = story->findBlock(std::clamp(dirtyTo - 1, 0, last));
            if (d0.isValid()) from = std::min(from, d0.position());
            if (d1.isValid()) to = std::max(to, d1.position() + d1.length());
        }
    }
    // The frames and the tables set at places that come right before the paragraph an edit is in are set from it, and which of them are set from it and which are in the text
    // depends on how many there are: the page is the one that holds the first of them. (They have no lines of their own; the paragraph they come before does.)
    if (!m_doc->frames.isEmpty() || m_tables->any()) {
        QTextBlock first = story->findBlock(std::clamp(from, 0, last));
        for (QTextBlock prev = first.previous(); prev.isValid(); prev = prev.previous()) {
            const QTextBlockFormat pf = prev.blockFormat();
            if (pf.intProperty(fp::Frame) > 0 && pf.boolProperty(tp::OutOfLayout)) {
                first = prev;
                continue;
            }
            if (m_tables->any() && pf.intProperty(fp::Cell)) {
                const TableIndex &tix = m_tables->index();
                const int span = tix.spanAt(prev.position());
                if (span >= 0 && tix.spans()[size_t(span)].table && tix.spans()[size_t(span)].table->place.has_value()) {
                    first = story->findBlock(tix.spans()[size_t(span)].from);
                    prev = first;
                    continue;
                }
            }
            break;
        }
        from = std::min(from, first.position());
    }
    // (Pages of notes alone begin where the one after does: the page the edit is in is the first that begins at or after it, less one, and then the one before.)
    const auto at = std::lower_bound(m_starts.begin(), m_starts.end(), from, [](const PageStart &s, int pos) { return s.pos < pos; });
    const int first = int(at - m_starts.begin());
    int holding = first < m_starts.size() && m_starts[first].pos == from ? first : first - 1;
    holding = std::max(0, holding);
    // An edit in a cell of a row that goes over page breaks may change every part of the row: the pages that start in it all begin at its first paragraph, so the page that
    // holds the edit is the last of them. The row is laid out again from the page it began on.
    if (m_tables->any() && b0.isValid() && b0.blockFormat().intProperty(fp::Cell) != 0)
        while (holding > 0 && m_starts[holding].piece != 0) --holding;
    const int page = std::max(0, holding - 1);
    if (!m_pending) {
        m_pending = true;
        m_shift = 0;
        m_valid = m_high = page;
        m_dirtyEnd = to;
    } else {
        // (An earlier edit's end, in today's positions.)
        int before = m_dirtyEnd;
        if (before >= position + removed && before > position) before += delta;
        else if (before > position) before = position + added;
        m_valid = std::min(m_valid, page);
        m_high = std::clamp(m_high, m_valid, int(m_starts.size()) - 1);
        m_dirtyEnd = std::max(before, to);
    }
    m_shift -= goneByTheEdit;   // (the pages that began in what was taken out are before the ones after it, which are one fewer pages along)
}

void Paginator::settle(int index)
{
    while (m_pending && (index < 0 || m_valid < index)) settleStep();
}

// Lays out the page after the last that is right, and sees whether the rest are the same as they were.
void Paginator::settleStep()
{
    const int k = m_valid;
    const Run run = layoutPage(k, m_starts[k]);
    if (m_numberOf > k) m_numberOf = -1;   // (the pages after this one are about to change)
    if (!run.hasNext || run.next == m_starts[k]) {
        m_starts.resize(k + 1);
        m_high = std::min(m_high, k);
        m_complete = true;
        m_pending = false;
        return;
    }
    const PageStart next = run.next;
    int i = k + 1;
    // Pages that began where the text no longer has a page begin: before the place the next begins at, and at it, those that start in a row that is not set as it was (a row of a
    // table that goes over page breaks is on all the pages that start in it, at the place of its first paragraph: the parts after the one that is next, when the room
    // the first part had is not what it was, are not the parts there are now).
    auto goneAt = [&](const PageStart &e) {
        if (e.pos != next.pos) return e.pos < next.pos;
        if (e == next) return false;
        return next.piece == 0 ? e.piece != 0 : (e.piece == 0 || e.pieceAt != next.pieceAt || e.piece < next.piece);
    };
    while (i < m_starts.size() && goneAt(m_starts[i])) ++i;
    const int gone = i - (k + 1);
    m_starts.remove(k + 1, gone);
    m_shift -= gone;
    if (m_high > k) m_high -= std::min(gone, m_high - k);
    i = k + 1;
    // The pages after are as they were only if they are where they were: one more or one less before them, and odd pages are even and even pages odd, which
    // a section with other headers or footers for each is laid out differently by.
    bool oddAndEven = false;
    for (const Section &s : m_doc->sections) oddAndEven |= s.differentOddEven;
    if (i < m_starts.size() && m_starts[i] == next && !(oddAndEven && (m_shift & 1))) {
        m_valid = i;
        if (next.pos >= m_dirtyEnd) {
            // Past what the edits touched. An entry from before them: the pages from there on are as they were. One laid out afresh after the
            // first edit (up to m_high) is as good as this page: the ones to m_high stay, and those after them, still as they were before the
            // first edit, are laid out next.
            if (i > m_high) m_pending = false;
            else m_valid = m_high;
        }
        m_high = std::max(m_high, m_valid);
    } else if (i < m_starts.size() && m_starts[i].pos == next.pos) {
        // (The same place with other particulars is the same page, made otherwise: not one more, with the old kept after it.)
        m_starts[i] = next;
        m_high = std::max(m_high, i);
        m_valid = i;
    } else {
        m_starts.insert(i, next);
        ++m_shift;
        m_high = m_high >= i ? m_high + 1 : i;
        m_valid = i;
    }
    // (An unfinished pagination goes no further than it was.)
    if (m_pending && !m_complete && m_valid + 1 >= m_starts.size()) m_pending = false;
}

// ---- the pages that are looked at ----

// The text on a page, as a story of its own: a copy of the paragraphs from the page's first line to the end of the one its last line is in.
// (If the page begins in the middle of a paragraph, that is the rest of it: with no space before it or first-line indent, and no bullet
// or number.) The paragraph the page ends in is whole, so that its last line on the page is set as it is in the document, the lines that
// don't fit being left out of the frame.
std::unique_ptr<QTextDocument> Paginator::sliceOf(const Run &run, const PageStart &st, int *storyFrom, bool *fromBefore)
{
    const QTextDocument *main = m_doc->story();
    const QVector<StoryLayout::LineBrief> &L = run.lines;
    const QVector<int> &inText = floatIndex().inText;
    const int from = L.first().pos;
    *storyFrom = from;
    const QTextBlock firstBlock = main->findBlock(L.first().block);
    QTextBlock lastBlock = main->findBlock(L[run.kept - 1].block);
    if (!run.groups.empty()) lastBlock = lastBlockWithTables(L, run.kept, run.groups);
    // The copy ends with the paragraph the page's last line is in, but where that goes on past the page, with the line after (so that the last line
    // on the page is not the paragraph's last, which isn't set as the others: and a paragraph of thousands of lines isn't copied whole).
    int to = lastBlock.position() + lastBlock.length() - 1;
    if (run.kept < L.size() && L[run.kept].block == L[run.kept - 1].block) to = std::min(to, L[run.kept].end);
    *fromBefore = L.first().line > 0;
    auto sd = std::make_unique<QTextDocument>();
    sd->setDocumentMargin(main->documentMargin());
    sd->setDefaultFont(main->defaultFont());
    sd->setDefaultTextOption(main->defaultTextOption());
    sd->setUndoRedoEnabled(false);
    QTextCursor dst(sd.get());
    QHash<QTextList *, QTextList *> lists;
    // The notes' references show their numbers as they are counted through the document (the notes themselves are boxes of their own, below): the
    // number of the next one, from the notes before the page's first paragraph and those in it before its first line.
    int footnote = st.foot + 1, endnote = st.end + 1;
    {
        int before = 0, endBefore = 0;
        countNotes(firstBlock.position(), from, &before, &endBefore);
        footnote += before;
        endnote += endBefore;
    }
    auto roman = [](int n) {
        static const std::pair<int, const char *> table[] = {{1000, "m"}, {900, "cm"}, {500, "d"}, {400, "cd"}, {100, "c"}, {90, "xc"}, {50, "l"}, {40, "xl"}, {10, "x"}, {9, "ix"}, {5, "v"}, {4, "iv"}, {1, "i"}};
        QString r;
        n = std::clamp(n, 1, 3999);
        for (const auto &t : table)
            while (n >= t.first) {
                r += QLatin1String(t.second);
                n -= t.first;
            }
        return r;
    };
    for (QTextBlock b = firstBlock; b.isValid(); b = b.next()) {
        if (b != firstBlock && restartsNotesAt(b)) footnote = endnote = 1;
        // A table set at a place has no lines in the text: it is a paragraph of as many characters as it takes that shows nothing and takes no room.
        if (m_tables->any() && b.blockFormat().intProperty(fp::Cell)) {
            const int span = m_tables->index().spanAt(b.position());
            if (span >= 0 && b != lastBlock && m_tables->index().spans()[size_t(span)].from == b.position() && floatsTable(span)) {
                const TableIndex::Span &sp = m_tables->index().spans()[size_t(span)];
                QTextBlockFormat hidden;
                hidden.setProperty(tp::OutOfLayout, true);
                hidden.setTopMargin(0);
                hidden.setBottomMargin(0);
                QTextCharFormat cf;
                cf.setFontPointSize(1);
                if (b == firstBlock) {
                    dst.setBlockFormat(hidden);
                    dst.setBlockCharFormat(cf);
                } else {
                    dst.insertBlock(hidden, cf);
                }
                dst.insertText(QString(std::max(0, sp.to - sp.from - 1), QChar(0x200B)), cf);
                int foots = 0, ends = 0;
                QTextBlock last = b;
                for (QTextBlock t = b; t.isValid() && t.position() < sp.to; t = t.next()) {
                    int f2 = 0, e2 = 0;
                    countNotes(t.position(), t.position() + t.length(), &f2, &e2);
                    foots += f2;
                    ends += e2;
                    last = t;
                }
                footnote += foots;
                endnote += ends;
                b = last;
                if (b.position() >= lastBlock.position()) break;
                continue;
            }
        }
        // The rows of a table are one line as tall as they are, of as many characters as they take (the notes cited in them are counted: the numbers after go on from them).
        if (!run.groups.empty()) {
            const QTextBlock before = b;
            if (tablePlaceholder(dst, b, before == firstBlock, run.groups)) {
                int foots = 0, ends = 0;
                for (QTextBlock t = before; t.isValid() && t.position() <= b.position(); t = t.next()) {
                    int f2 = 0, e2 = 0;
                    countNotes(t.position(), t.position() + t.length(), &f2, &e2);
                    foots += f2;
                    ends += e2;
                }
                footnote += foots;
                endnote += ends;
                if (b.position() >= lastBlock.position()) break;
                continue;
            }
        }
        const bool tail = b == firstBlock && L.first().line > 0;
        QTextBlockFormat bf = b.blockFormat();
        bf.clearProperty(QTextFormat::ObjectIndex);   // (the list is the document's, not this story's: it's made again below)
        QTextList *list = tail ? nullptr : b.textList();
        // What the paginator decided is in the frames' heights; widow control and keeping paragraphs together are not asked of the text again.
        bf.clearProperty(tp::WidowControl);
        bf.clearProperty(tp::KeepTogether);
        bf.clearProperty(tp::KeepWithNext);
        // A frame that is lines of the text is paragraphs of it, as they would be without the frame's properties.
        if (std::binary_search(inText.cbegin(), inText.cend(), b.position())) {
            bf.clearProperty(tp::OutOfLayout);
            bf.clearProperty(fp::Frame);
        }
        if (b == firstBlock && ((tail && !run.keepsSpace) || !run.firstSpace)) bf.setTopMargin(0);
        // A paragraph's box that began on the page before, or goes on to the next, is open at that end.
        auto sameBox = [](const QTextBlock &x, const QTextBlock &y) {
            if (!x.isValid() || !y.isValid()) return false;
            const QTextBlockFormat xf = x.blockFormat(), yf = y.blockFormat();
            return xf.hasProperty(tp::Borders) && xf.stringProperty(tp::Borders) == yf.stringProperty(tp::Borders) &&
                   std::abs(xf.leftMargin() - yf.leftMargin()) < 0.01 && std::abs(xf.rightMargin() - yf.rightMargin()) < 0.01;
        };
        if (b == firstBlock && (tail || sameBox(b.previous(), b))) bf.setProperty(tp::BorderTopOff, true);
        if (b == lastBlock && ((run.kept < L.size() && L[run.kept].block == L[run.kept - 1].block) || sameBox(b, b.next()))) bf.setProperty(tp::BorderBottomOff, true);
        if (tail) {
            // (As the text after a break that begins the paragraph is its first line, with the indent.)
            if (!(run.keepsSpace && main->characterAt(b.position()) == kPageBreak && b.length() > 1)) bf.setTextIndent(0);
            bf.clearProperty(tp::DropCapLines);
            bf.clearProperty(tp::DropCapChars);
            bf.clearProperty(tp::StartInNextBox);
        }
        if (b == firstBlock) {
            dst.setBlockFormat(bf);
            dst.setBlockCharFormat(b.charFormat());
        } else {
            dst.insertBlock(bf, b.charFormat());
        }
        const int skip = b == firstBlock ? from - b.position() : 0;
        const int upto = b == lastBlock ? to - b.position() : std::numeric_limits<int>::max();
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const int rel = frag.position() - b.position();
            QString text = frag.text();
            if (rel + text.size() <= skip) continue;
            if (rel >= upto) break;
            if (rel < skip) text = text.mid(skip - rel);
            if (rel + int(text.size()) > upto) text = text.left(upto - std::max(rel, skip));
            const QString code = frag.charFormat().stringProperty(tp::Field);
            const bool foot = code.startsWith(QLatin1String("footnote:")), end = code.startsWith(QLatin1String("endnote:"));
            if (foot || end) {
                for (const QChar ch : text) {
                    QTextCharFormat cf = frag.charFormat();
                    const bool custom = foot && cf.boolProperty(tp::NoteMarkCustom);   // (shows nothing: its mark is the text after it)
                    cf.setProperty(tp::Field, QLatin1String("raw:") + (custom ? QString() : foot ? QString::number(footnote++) : roman(endnote++)));
                    dst.insertText(QString(ch), cf);
                }
                continue;
            }
            dst.insertText(text, frag.charFormat());
        }
        if (list) {
            QTextList *made = lists.value(list);
            if (!made) {
                QTextListFormat lf = list->format();
                // A list that began on an earlier page goes on from the number its item here has.
                if (list->itemNumber(b) > 0) lf.setStart(lf.start() + list->itemNumber(b));
                made = dst.createList(lf);
                lists.insert(list, made);
            } else {
                made->add(dst.block());
            }
        }
        if (b == lastBlock) break;
    }
    return sd;
}

QVector<Paginator::Wrapped> Paginator::wrappersOf(const Run &run, int index, const PageStart &start, int *moveBefore)
{
    QVector<Wrapped> out;
    if (!hasFloaters() || run.kept <= 0) return out;
    const Section &sec = m_doc->section(start.section);
    const QRectF body = furnitureOf(index, start, numberOf(index)).body;
    const QTextDocument *story = m_doc->story();
    // The text's own lines (the rows of tables are not: what a cell anchors does not move the text round it), and the first rows of the tables in the text (what is set from a
    // table in the text is set from its first row).
    QVector<AnchorLine> held, firstLines;
    for (int i = 0; i < run.kept; ++i) {
        const StoryLayout::LineBrief &ln = run.lines[i];
        const int column = ln.frame - run.firstFrame;
        if (column < 0 || column >= run.columns) continue;
        const bool inCell = !run.groups.empty() && story->findBlock(ln.block).blockFormat().intProperty(fp::Cell);
        const QRectF col = run.cols[column];
        const AnchorLine line = anchorLineOf(ln, ln.rect.translated(col.left(), body.top() + col.top()), col.left());
        if (!inCell) held << line;
        if (!inCell || ln.line == 0) firstLines << line;
    }
    const QVector<Anchor> anchors = anchorsOfLines(held, firstLines, sec.columnWidth());
    int made = 0;
    for (const Anchor &a : anchors)
        for (int id = a.object; (a.frame || (id > 0 && id <= m_doc->objects.size())) && id > 0 && made < kMostObjects; id = objectAt(a, id).next, ++made) {
            const FloatingObject &o = objectAt(a, id);
            using W = FloatingObject::Wrap;
            // A table set from its paragraph (down from the paragraph or the line) that goes over the foot of the text area is not on this page: it goes to the next with its paragraph.
            if (moveBefore && a.table >= 0 && o.item && (o.vRel == FloatingObject::Rel::Paragraph || o.vRel == FloatingObject::Rel::Line) && a.paragraph >= 0 &&
                placeOf(o, a, sec, false, body.bottom()).bottom() > body.bottom() + 0.01 && (*moveBefore < 0 || a.paragraph < *moveBefore))
                *moveBefore = a.paragraph;
            if (!o.item || o.behind || o.item->type() == ItemType::Line || (o.wrap != W::Square && o.wrap != W::Tight && o.wrap != W::TopBottom)) continue;
            QRectF r = placeOf(o, a, sec, true, body.bottom());
            if (o.item->rotation != 0) {
                ItemPtr turned = o.item->clone();
                turned->rect = r;
                r = turned->bounds();
            }
            out << Wrapped{id, a.pos, r, a.paragraph, a.frame};
            if (out.size() >= kMostWrapped) return out;
        }
    return out;
}

QVector<StoryLayout::LineBrief> Paginator::lines(int index)
{
    paginate(index);
    if (index < 0 || index >= m_starts.size()) return {};
    Run run = layoutPage(index, m_starts[index]);
    QVector<StoryLayout::LineBrief> out = run.lines.mid(0, run.kept);
    for (auto &l : out) l.frame -= run.firstFrame;
    return out;
}

std::unique_ptr<MaterializedPage> Paginator::materialize(int index)
{
    paginate(index);
    if (index < 0 || index >= m_starts.size()) return nullptr;
    const PageStart st = m_starts[index];
    const Run run = layoutPage(index, st);
    // (A page with none of the body's lines is a page of notes that go on from the page before, when it has them.)
    if (run.kept <= 0 && !run.blank && st.carried.isEmpty()) return nullptr;
    const bool bodyless = run.kept <= 0 && !run.blank;
    const Section &sec = m_doc->section(st.section);
    auto out = std::make_unique<MaterializedPage>();
    m_imageIds.clear();
    out->index = index;
    out->section = st.section;
    out->number = numberOf(index);
    out->size = sec.pageSize();

    auto d = Document::blank(out->size, QString(), 1);
    d->pages[0]->masterId.clear();
    d->pages[0]->id = QStringLiteral("flow-page-%1").arg(index);
    d->setup.margins = sec.margins;
    d->setup.firstPageNumber = out->number;
    d->setup.pageNumberFormat = sec.pageNumberFormat;
    d->colors = m_doc->env.colors;
    d->fonts = m_doc->env.fonts;
    d->defaultTabStop = m_doc->env.defaultTab;
    d->wordText = m_doc->env.wordLines;
    d->wordNoHang = m_doc->env.wordNoHang;
    d->wordSpaceSums = m_doc->env.wordSpaceSums;
    d->wordNoHangStop = m_doc->env.wordNoHangStop;
    d->wordNoNumberIndentStop = m_doc->env.wordNoNumberIndentStop;
    d->wordStandIns = m_doc->env.standIns;
    const Furniture furniture = furnitureOf(index, st, out->number);
    d->fontAliases = m_doc->env.familyAliases;
    if (run.blank) {
        out->storyFrom = m_doc->story()->characterCount();
        addFurniture(d.get(), furniture, sec, st.section, out->number);
        markTotals(out.get(), *d);
        out->doc = std::move(d);
        return out;
    }

    if (bodyless) {
        out->storyFrom = st.pos;
        addFurniture(d.get(), furniture, sec, st.section, out->number);
    } else {
        // The header and the footer are the page's lowest layer: before the body's text and objects, which are drawn over them.
        addFurniture(d.get(), furniture, sec, st.section, out->number);
        const size_t front = d->pages[0]->items.size();
        auto story = std::make_shared<Story>();
        story->id = QStringLiteral("flow-story-%1").arg(index);
        story->doc = sliceOf(run, st, &out->storyFrom, &out->continuesFromBefore);
        resolveImages(story->doc.get(), d.get());
        d->stories.insert(story->id, story);
        if (!run.groups.empty()) {
            addTableFragments(out.get(), run.cols, furniture.body, run.groups);
            if (!m_doc->images.isEmpty()) resolveCellImages(out.get(), d.get());
        }
        const StoryLayout::LineBrief &lastLine = run.lines[run.kept - 1];
        out->continuesOnAfter = lastLine.line < lastLine.lines - 1;

        // A text box for each column that has lines, as tall as its lines are: the lines that follow, which went to the next page, don't fit
        // in it (a frame a line short of the text's bottom is how the page is cut where the paginator cut it).
        QVector<double> bottom(run.columns, -1), top(run.columns, -1);
        for (int i = 0; i < run.kept; ++i) {
            const StoryLayout::LineBrief &ln = run.lines[i];
            const int col = ln.frame - run.firstFrame;
            if (col < 0 || col >= run.columns) continue;
            bottom[col] = std::max(bottom[col], ln.rect.bottom() - ln.below);
            if (top[col] < 0) top[col] = ln.rect.top();
        }
        QVector<std::shared_ptr<TextItem>> boxes;
        for (int c = 0; c < run.columns && bottom[c] >= 0; ++c) {
            auto t = std::make_shared<TextItem>();
            t->id = QStringLiteral("flow-box-%1-%2").arg(index).arg(c);
            const QRectF col = run.cols[c];
            t->rect = QRectF(col.left(), furniture.body.top() + col.top(), col.width(), bottom[c] + 0.005);
            // A paragraph that comes to the top of a column (not the page's) for want of room has no space before it there, but one that the
            // text engine moved there to keep it with the next, or its lines together, has it. Whatever is above the first line is the top inset.
            t->insets = QMarginsF(0, c > 0 ? top[c] : 0, 0, 0);
            t->columns = 1;
            t->hyphenate = m_doc->hyphenate;
            t->hyphenZone = m_doc->hyphenZone;
            t->storyId = story->id;
            t->wrap.mode = Wrap::None;
            if (!boxes.isEmpty()) boxes.last()->nextId = t->id;
            boxes << t;
            d->pages[0]->items.push_back(t);
            out->boxes << t->id;
        }
        // The objects set at places that the page's lines anchor.
        if (hasFloaters()) {
            // (The line that stands for a row of a table is not a line of text: the cells' own lines, set from the fragment, are. It is where the first row of a table in the text is, which
            // is what the frames and tables before the table are set from.)
            QVector<AnchorLine> held, firstLines;
            for (int i = 0; i < run.kept; ++i) {
                const StoryLayout::LineBrief &ln = run.lines[i];
                const int column = ln.frame - run.firstFrame;
                if (column < 0 || column >= run.columns) continue;
                const bool inCell = !run.groups.empty() && m_doc->story()->findBlock(ln.block).blockFormat().intProperty(fp::Cell);
                const QRectF col = run.cols[column];
                const AnchorLine line = anchorLineOf(ln, ln.rect.translated(col.left(), furniture.body.top() + col.top()), col.left());
                if (!inCell) held << line;
                if (!inCell || ln.line == 0) firstLines << line;
            }
            for (const auto &fragment : out->tables) {
                // (The column a table is in: the one its middle is in, else the nearest.)
                double columnLeft = run.cols.isEmpty() ? 0 : run.cols[0].left(), nearest = 1e18;
                for (const QRectF &col : run.cols) {
                    const double away = std::abs(col.center().x() - fragment->rect().center().x());
                    if (away < nearest) { nearest = away; columnLeft = col.left(); }
                }
                fragment->forLines([&](const StoryLayout::LineBrief &ln, const QRectF &rect) {
                    if (held.size() < 20000) held << anchorLineOf(ln, rect, columnLeft);
                });
            }
            const QVector<Anchor> anchors = anchorsOfLines(held, firstLines, sec.columnWidth());
            QVector<QRectF> columns;
            for (const QRectF &col : run.cols) columns << QRectF(col.left(), furniture.body.top() + col.top(), col.width(), col.height());
            if (!anchors.isEmpty()) addObjects(d.get(), anchors, sec, out->number, run.wrapped, columns, out.get(), front);
        }
    }
    // The notes the page's lines cite and those that go on from the page before.
    {
        QStringList ids;
        for (const QString &c : st.carried) ids << c.section(QLatin1Char('|'), 0, 0);
        for (int i = 0; i < run.kept; ++i) {
            const StoryLayout::LineBrief &ln = run.lines[i];
            for (QTextBlock b = m_doc->story()->findBlock(ln.pos); b.isValid() && b.position() < ln.end; b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    const QTextFragment frag = it.fragment();
                    if (frag.position() >= ln.end || frag.position() + frag.length() <= ln.pos) continue;
                    const QString code = frag.charFormat().stringProperty(tp::Field);
                    if (!code.startsWith(QLatin1String("footnote:"))) continue;
                    const int lo = std::max(frag.position(), ln.pos), hi = std::min(frag.position() + frag.length(), ln.end);
                    for (int k = lo; k < hi; ++k) ids << code.mid(9);
                }
        }
        if (!ids.isEmpty()) addNotes(d.get(), run.notes, run.rules, run.firstFrame, ids, run.cols, furniture.body);
    }
    markTotals(out.get(), *d);
    out->doc = std::move(d);
    return out;
}

} // namespace jp::flow
