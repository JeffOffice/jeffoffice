#include "flow/paginator.h"

#include "text/textprops.h"

#include <QElapsedTimer>
#include <QHash>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>

namespace jp::flow {

namespace {

// How tall the frames for a paragraph's lines on the page before, and for what runs over the page, are: more than a paragraph takes.
constexpr double kTall = 100000;

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
    for (const Section &s : m_doc->sections) restarts |= s.firstPageNumber > 0;
    if (!restarts) return index + 1;
    for (int p = std::min(index, int(m_starts.size()) - 1); p >= 0; --p)
        if (p == 0 || m_starts[p].section != m_starts[p - 1].section) {
            const int first = m_doc->section(m_starts[p].section).firstPageNumber;
            if (first > 0) return first + (index - p);
        }
    return index + 1;
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
    const double colW = sec.columnWidth(), colH = sec.content().height();
    const int number = numberOf(index);
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
    // The page is a frame for each column, and one more for the text that runs over; if the first paragraph began on the page before, a
    // frame before them is where its lines that went there are put.
    QVector<FrameSpec> frames;
    if (st.skip > 0) frames << frame(kTall);
    run.firstFrame = int(frames.size());
    for (int c = 0; c < run.columns; ++c) frames << frame(colH);
    frames << frame(kTall);
    const int over = int(frames.size()) - 1;

    StoryLayout::Span span;
    span.from = st.pos;
    span.firstBreakAfter = st.skip > 0 ? st.skip : -1;
    span.stopAtFrame = over;
    int kept = 0;
    for (bool space = st.hard;; space = false) {
        span.spaceBeforeFirst = space;
        StoryLayout layout;
        layout.build(m_doc->story(), frames, m_doc->env, span);
        const QVector<StoryLayout::LineBrief> all = layout.lineBriefs();
        int first = 0;
        while (first < all.size() && all[first].frame >= 0 && all[first].frame < run.firstFrame) ++first;
        run.lines = all.mid(first);
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
    bool hard = false, sectionEnds = false, forced = false;
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
    } else if (forced && c > 0) {
        // A break after the text's last line: the pages go on with one that has nothing on it.
        run.hasNext = true;
        run.next.pos = m_doc->story()->characterCount();
        run.next.skip = 0;
        run.next.hard = true;
        run.next.section = nextSection;
    }
    return run;
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
    m_starts.clear();
    m_starts << PageStart();
    m_complete = false;
    m_pending = false;
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
    p.content = sec.content();
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
std::unique_ptr<QTextDocument> Paginator::sliceOf(const Run &run, int *storyFrom, bool *fromBefore) const
{
    const QTextDocument *main = m_doc->story();
    const QVector<StoryLayout::LineBrief> &L = run.lines;
    const int from = L.first().pos;
    *storyFrom = from;
    const QTextBlock firstBlock = main->findBlock(L.first().block);
    const QTextBlock lastBlock = main->findBlock(L[run.kept - 1].block);
    *fromBefore = L.first().line > 0;
    auto sd = std::make_unique<QTextDocument>();
    sd->setDocumentMargin(main->documentMargin());
    sd->setDefaultFont(main->defaultFont());
    sd->setDefaultTextOption(main->defaultTextOption());
    sd->setUndoRedoEnabled(false);
    QTextCursor dst(sd.get());
    QHash<QTextList *, QTextList *> lists;
    for (QTextBlock b = firstBlock; b.isValid(); b = b.next()) {
        const bool tail = b == firstBlock && L.first().line > 0;
        QTextBlockFormat bf = b.blockFormat();
        bf.clearProperty(QTextFormat::ObjectIndex);   // (the list is the document's, not this story's: it's made again below)
        QTextList *list = tail ? nullptr : b.textList();
        // What the paginator decided is in the frames' heights; widow control and keeping paragraphs together are not asked of the text again.
        bf.clearProperty(tp::WidowControl);
        bf.clearProperty(tp::KeepTogether);
        bf.clearProperty(tp::KeepWithNext);
        if (b == firstBlock && (tail || !run.firstSpace)) bf.setTopMargin(0);
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
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const int rel = frag.position() - b.position();
            QString text = frag.text();
            if (rel + text.size() <= skip) continue;
            if (rel < skip) text = text.mid(skip - rel);
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
    d->fontAliases = m_doc->env.familyAliases;
    if (run.blank) {
        out->storyFrom = m_doc->story()->characterCount();
        out->doc = std::move(d);
        return out;
    }

    auto story = std::make_shared<Story>();
    story->id = QStringLiteral("flow-story-%1").arg(index);
    story->doc = sliceOf(run, &out->storyFrom, &out->continuesFromBefore);
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
        t->rect = QRectF(col.left(), col.top(), col.width(), bottom[c] + 0.005);
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
    out->doc = std::move(d);
    return out;
}

} // namespace jp::flow
