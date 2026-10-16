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

} // namespace

// What a paragraph asks of the pages.
struct Paginator::BlockFlags {
    bool breakBefore = false, breakAfter = false, sectionAfter = false, keepNext = false, widow = false;
};

// A page laid out: the story's lines on it, and where it ended.
struct Paginator::Run {
    // The lines from the first on the page (those of a first paragraph that the pages before hold are left out), then those that run over.
    QVector<StoryLayout::LineBrief> lines;
    int firstFrame = 0;        // the frame of the first column
    int columns = 1;
    int kept = 0;              // the lines on the page
    bool blank = false;        // the page the pages end with after a break at the end of the text: it has nothing on it
    bool firstSpace = true;    // space before the first paragraph is kept (it isn't if the page starts without it, or it was as tall as the page)
    bool hasNext = false;
    bool keepsSpace = false;   // the first paragraph is the rest of one after a break, and has its space before
    QVector<StoryLayout::NotePlace> notes;   // the footnotes as the page's layout put them
    QVector<StoryLayout::Rule> rules;        // and the rules above them
    PageStart next;            // where the page after starts
};

Paginator::Paginator(FlowDocument *doc) : m_doc(doc), m_watch(std::make_unique<QObject>())
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

Paginator::Run Paginator::layoutPage(int index, const PageStart &st)
{
    ++m_laidOut;
    Run run;
    // The page a break at the end of the text makes: it starts past the text's last character.
    if (st.pos >= m_doc->story()->characterCount()) {
        run.blank = true;
        return run;
    }
    const Section &sec = m_doc->section(st.section);
    run.columns = sec.columnCount();
    run.keepsSpace = st.hard && st.skip > 0;
    const double colW = sec.columnWidth();
    const int number = numberOf(index);
    const double colH = furnitureOf(index, st, number).body.height();
    auto frame = [&](double height) {
        FrameSpec fs;
        fs.size = QSizeF(colW, height);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.columns = 1;
        fs.hyphenate = m_doc->hyphenate;
        fs.hyphenZone = m_doc->hyphenZone;
        fs.ctx.doc = m_doc->fields;
        fs.ctx.pageNumber = number;
        fs.ctx.pageCount = std::max<int>(number, m_starts.size());
        return fs;
    };
    // The page is a frame for each column, and one more for the text that runs over. If the first paragraph began on the page before, either
    // a frame before them is where its lines that went there are put (the paragraph is laid out whole and so is as the text sets it), or, if
    // it has many lines, the page lays out the rest of it as a paragraph of its own.
    // A page that a break inside a paragraph opens has the rest of the paragraph with its space before again, as the paragraph of its own it is laid out as.
    const bool afterBreak = st.hard && st.skip > 0;
    const bool tail = afterBreak || st.skip > std::max(0, m_doc->exactLines);
    QVector<FrameSpec> frames;
    if (st.skip > 0 && !tail) frames << frame(std::max(kTall, st.skip * 5000.0));
    run.firstFrame = int(frames.size());
    for (int c = 0; c < run.columns; ++c) frames << frame(colH);
    frames << frame(kTall);
    const int over = int(frames.size()) - 1;

    StoryLayout::Span span;
    span.from = st.pos;
    span.firstBreakAfter = st.skip > 0 && !tail ? st.skip : -1;
    span.continued = tail;
    span.spaceBeforeContinued = afterBreak;
    span.stopAtFrame = over;
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
    QStringList carriedOut;
    // A paragraph that runs on after the page is laid out as far as the lines wanted need (its window): the lines the rules look at past the page,
    // a line more, and the last, which is cut short.
    const int wanted = std::max({m_doc->widowLines, m_doc->orphanLines, 2}) + 2;
    int kept = 0;
    for (bool space = st.hard;; space = false) {
        span.spaceBeforeFirst = space;
        QVector<StoryLayout::LineBrief> all;
        // (A paragraph that is laid out whole from its start, with the lines before the page, needs room for those too.)
        for (int window = kWindow + (tail ? 0 : st.skip * 160);; window *= 4) {
            span.window = window;
            StoryLayout layout;
            layout.build(m_doc->story(), frames, m_doc->env, span);
            all = layout.lineBriefs();
            run.notes = layout.notePlaces();
            run.rules = layout.noteRules();
            carriedOut.clear();
            for (const auto &cn : layout.carriedNotes(over)) carriedOut << QStringLiteral("%1|%2|%3").arg(cn.storyId).arg(cn.number).arg(cn.from);
            int past = 0;
            bool cut = false;
            for (const auto &l : all) {
                cut |= l.cut;
                past += l.cut && (l.frame < 0 || l.frame >= over);
            }
            if ((!layout.windowShort() && (!cut || past >= wanted)) || window > std::numeric_limits<int>::max() / 8) break;
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
        run.firstSpace = space;
        // Space before the first paragraph as tall as the page, or nearly, leaves no room for a line: it is cut to what the page holds (none).
        if (kept > 0 || !space || run.lines.isEmpty()) break;
    }
    const QVector<StoryLayout::LineBrief> &L = run.lines;

    QHash<int, BlockFlags> cache;
    auto flags = [&](int blockStart) -> const BlockFlags & {
        auto it = cache.find(blockStart);
        if (it == cache.end()) {
            const QTextBlockFormat bf = m_doc->story()->findBlock(blockStart).blockFormat();
            BlockFlags f;
            f.breakBefore = bf.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore;
            f.breakAfter = bf.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter;
            f.sectionAfter = bf.boolProperty(fp::SectionBreakAfter);
            f.keepNext = bf.boolProperty(tp::KeepWithNext);
            f.widow = bf.boolProperty(tp::WidowControl);
            it = cache.insert(blockStart, f);
        }
        return it.value();
    };

    // Where the page ends: where the text ran out of room, unless a break asks for less.
    int c = kept;
    bool hard = false, sectionEnds = false, forced = false, byCharacter = false;
    for (int i = 0; i < c; ++i) {
        const StoryLayout::LineBrief &ln = L[i];
        const BlockFlags &bf = flags(ln.block);
        if (ln.line == 0 && i > 0 && bf.breakBefore) {
            c = i;
            hard = forced = true;
            break;
        }
        if (ln.line == ln.lines - 1 && (bf.breakAfter || bf.sectionAfter)) {
            c = i + 1;
            hard = forced = true;
            sectionEnds = bf.sectionAfter;
            break;
        }
        // A page break character: the page ends after the line that holds it.
        if (!ln.cut && ln.end > ln.pos && m_doc->story()->characterAt(ln.end - 1) == kPageBreak) {
            c = i + 1;
            hard = forced = byCharacter = true;
            // (The empty line the text engine puts after a break that ends its paragraph stays with the page: the next paragraph is at the top
            // of the next.)
            if (c < L.size() && L[c].block == L[i].block && L[c].line == L[i].line + 1 && L[c].pos == L[c].end && L[c].line == L[c].lines - 1) ++c;
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
    run.kept = c;
    const int nextSection = sectionEnds ? std::min(st.section + 1, int(m_doc->sections.size()) - 1) : st.section;
    if (c < L.size()) {
        run.hasNext = true;
        run.next.pos = L[c].pos;
        run.next.skip = L[c].line;
        run.next.hard = hard;
        run.next.section = nextSection;
    } else if (forced && !byCharacter && c > 0) {
        // A break after the text's last line: the pages go on with one that has nothing on it.
        run.hasNext = true;
        run.next.pos = m_doc->story()->characterCount();
        run.next.skip = 0;
        run.next.hard = true;
        run.next.section = nextSection;
    }
    if (run.hasNext) {
        // What the page after starts from: the notes through this page's paragraphs (those before the next page's first paragraph), and what was cut.
        const QTextDocument *story = m_doc->story();
        const int from = story->findBlock(st.pos).position();
        const QTextBlock nb = story->findBlock(run.next.pos);
        const int to = nb.isValid() ? nb.position() : story->characterCount();
        int footnotes = 0, endnotes = 0;
        countNotes(from, std::max(from, to), &footnotes, &endnotes);
        run.next.foot = st.foot + footnotes;
        run.next.end = st.end + endnotes;
        if (m_doc->restartNotesInSections && run.next.section != st.section) run.next.foot = run.next.end = 0;
        run.next.carried = carriedOut;
    }
    return run;
}

void Paginator::countNotes(int from, int to, int *footnotes, int *endnotes) const
{
    *footnotes = *endnotes = 0;
    const QTextDocument *story = m_doc->story();
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to; b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const QString code = frag.charFormat().stringProperty(tp::Field);
            if (code.isEmpty()) continue;
            const int lo = std::max(frag.position(), from), hi = std::min(frag.position() + frag.length(), to);
            if (hi <= lo) continue;
            if (code.startsWith(QLatin1String("footnote:"))) *footnotes += hi - lo;
            else if (code.startsWith(QLatin1String("endnote:"))) *endnotes += hi - lo;
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
    m_numberOf = -1;
    m_starts.clear();
    m_starts << PageStart();
    m_complete = false;
    m_pending = false;
}

void Paginator::invalidateFrom(int section)
{
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

void Paginator::changed(int position, int removed, int added)
{
    const int delta = added - removed;
    m_numberOf = -1;
    // The pages' places move with the text; one that began inside what was taken out is gone.
    int w = 0, goneBefore = 0;
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
    m_high -= goneBefore;
    // The paragraphs the edit touches, and the page that holds the first (and the one before, for what the page before did to make
    // room for it: a heading kept with its paragraph, a widow).
    const QTextDocument *story = m_doc->story();
    const int last = std::max(0, story->characterCount() - 1);
    const QTextBlock b0 = story->findBlock(std::min(position, last)), b1 = story->findBlock(std::min(position + added, last));
    const int from = b0.isValid() ? b0.position() : 0;
    const int to = b1.isValid() ? b1.position() + b1.length() : last + 1;
    const auto at = std::upper_bound(m_starts.begin(), m_starts.end(), from, [](int pos, const PageStart &s) { return pos < s.pos; });
    const int page = std::max(0, std::max(0, int(at - m_starts.begin()) - 1) - 1);
    if (!m_pending) {
        m_pending = true;
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
    while (i < m_starts.size() && m_starts[i].pos < next.pos) ++i;   // (pages that began where the text no longer has a page begin)
    const int gone = i - (k + 1);
    m_starts.remove(k + 1, gone);
    if (m_high > k) m_high -= std::min(gone, m_high - k);
    i = k + 1;
    if (i < m_starts.size() && m_starts[i] == next) {
        m_valid = i;
        if (next.pos >= m_dirtyEnd) {
            // Past what the edits touched. An entry from before them: the pages from there on are as they were. One laid out afresh after the
            // first edit (up to m_high) is as good as this page: the ones to m_high stay, and those after them, still as they were before the
            // first edit, are laid out next.
            if (i > m_high) m_pending = false;
            else m_valid = m_high;
        }
        m_high = std::max(m_high, m_valid);
    } else {
        m_starts.insert(i, next);
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
std::unique_ptr<QTextDocument> Paginator::sliceOf(const Run &run, const PageStart &st, int *storyFrom, bool *fromBefore) const
{
    const QTextDocument *main = m_doc->story();
    const QVector<StoryLayout::LineBrief> &L = run.lines;
    const int from = L.first().pos;
    *storyFrom = from;
    const QTextBlock firstBlock = main->findBlock(L.first().block);
    const QTextBlock lastBlock = main->findBlock(L[run.kept - 1].block);
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
        const bool tail = b == firstBlock && L.first().line > 0;
        QTextBlockFormat bf = b.blockFormat();
        bf.clearProperty(QTextFormat::ObjectIndex);   // (the list is the document's, not this story's: it's made again below)
        QTextList *list = tail ? nullptr : b.textList();
        // What the paginator decided is in the frames' heights; widow control and keeping paragraphs together are not asked of the text again.
        bf.clearProperty(tp::WidowControl);
        bf.clearProperty(tp::KeepTogether);
        bf.clearProperty(tp::KeepWithNext);
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
            bf.setTextIndent(0);
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
                    cf.setProperty(tp::Field, QLatin1String("raw:") + (foot ? QString::number(footnote++) : roman(endnote++)));
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
    if (run.kept <= 0 && !run.blank) return nullptr;
    const Section &sec = m_doc->section(st.section);
    auto out = std::make_unique<MaterializedPage>();
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
    const Furniture furniture = furnitureOf(index, st, out->number);
    d->fontAliases = m_doc->env.familyAliases;
    if (run.blank) {
        out->storyFrom = m_doc->story()->characterCount();
        addFurniture(d.get(), furniture, sec, st.section, out->number);
        out->doc = std::move(d);
        return out;
    }

    auto story = std::make_shared<Story>();
    story->id = QStringLiteral("flow-story-%1").arg(index);
    story->doc = sliceOf(run, st, &out->storyFrom, &out->continuesFromBefore);
    d->stories.insert(story->id, story);
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
        const QRectF col = sec.columnRect(c);
        t->rect = QRectF(col.left(), furniture.body.top(), col.width(), bottom[c] + 0.005);
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
    addFurniture(d.get(), furniture, sec, st.section, out->number);
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
        if (!ids.isEmpty()) addNotes(d.get(), run.notes, run.rules, run.firstFrame, ids, sec, furniture.body);
    }
    out->doc = std::move(d);
    return out;
}

} // namespace jp::flow
