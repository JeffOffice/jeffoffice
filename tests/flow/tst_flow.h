#pragma once
// Tests of libs/flow, and of the changes below it that the flowing document needs: the
// lookup of items by id in the document, the layout cache's key, and the additions to the
// text engine that the paginator uses.

#include "core/document.h"
#include "core/fonts.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "flow/tableflow.h"
#include "pageaudit.h"
#include "render/renderer.h"
#include "text/hyphenation.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QApplication>
#include <QBuffer>
#include <QElapsedTimer>
#include <QImage>
#include <QTextBoundaryFinder>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QRegularExpression>
#include <QSet>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QFontDatabase>
#include <QRawFont>
#include <QTextDocument>
#include <QTextList>
#include <functional>
#include <numeric>
#include <map>
#include <memory>
#include <tuple>
#include <random>

using namespace jp;
using jp::flow::FlowDocument;
using jp::flow::FloatingObject;
namespace fp = jp::flow::fp;
namespace audit = jp::flowaudit;
using jp::flow::MaterializedPage;
using jp::flow::Paginator;
using jp::flow::Section;
using namespace jp::flow;

namespace detail_tst_flow {

// Where an item is by looking through everything, as Document::find did before it kept an index.
inline Document::Loc scanFor(const Document &d, const QString &id)
{
    Document::Loc out;
    std::function<bool(ItemList &, GroupItem *)> scan = [&](ItemList &l, GroupItem *parent) {
        for (int i = 0; i < int(l.size()); ++i) {
            if (l[i]->id == id) {
                out.item = l[i].get(); out.list = &l; out.index = i; out.parent = parent;
                return true;
            }
            if (l[i]->type() == ItemType::Group && scan(static_cast<GroupItem *>(l[i].get())->children, static_cast<GroupItem *>(l[i].get())))
                return true;
        }
        return false;
    };
    for (int p = 0; p < d.pages.size(); ++p)
        if (scan(d.pages[p]->items, nullptr)) { out.page = p; return out; }
    for (const auto &m : d.masters)
        if (scan(m->items, nullptr)) { out.masterId = m->id; return out; }
    if (scan(const_cast<ItemList &>(d.scratch), nullptr)) { out.scratch = true; return out; }
    return Document::Loc();
}

inline bool sameLoc(const Document::Loc &a, const Document::Loc &b)
{
    return a.item == b.item && a.list == b.list && a.index == b.index && a.page == b.page && a.masterId == b.masterId && a.scratch == b.scratch &&
           a.parent == b.parent;
}

// Every list of items in the document (pages, masters, scratch, and the children of groups).
inline void allLists(Document &d, const std::function<void(ItemList &, GroupItem *)> &fn)
{
    std::function<void(ItemList &, GroupItem *)> visit = [&](ItemList &l, GroupItem *parent) {
        fn(l, parent);
        for (size_t i = 0; i < l.size(); ++i)
            if (l[i]->type() == ItemType::Group) visit(static_cast<GroupItem *>(l[i].get())->children, static_cast<GroupItem *>(l[i].get()));
    };
    for (auto &p : d.pages) visit(p->items, nullptr);
    for (auto &m : d.masters) visit(m->items, nullptr);
    visit(d.scratch, nullptr);
}

// A story of `paragraphs` paragraphs of plain words, the same for the same seed; `format` may change a paragraph's format.
inline void fillStory(QTextDocument *doc, int paragraphs, unsigned seed, const std::function<void(int, QTextBlockFormat &)> &format = {}, int fewest = 14, int most = 60)
{
    static const char *const words[] = {"the", "court", "held", "that", "section", "applies", "to", "every", "defendant", "who", "appeals", "from", "a", "judgment",
                                        "entered", "after", "trial", "unless", "the", "statute", "provides", "otherwise", "and", "reasonable", "notice", "was", "given"};
    std::mt19937 rng(seed);
    QTextCursor c(doc);
    QTextCharFormat body;
    body.setFontPointSize(11);
    for (int i = 0; i < paragraphs; ++i) {
        QTextBlockFormat bf;
        bf.setBottomMargin(6);
        if (format) format(i, bf);
        if (i == 0) c.setBlockFormat(bf); else c.insertBlock(bf);
        QString text;
        const int n = fewest + int(rng() % unsigned(most - fewest + 1));
        for (int k = 0; k < n; ++k) text += (k ? " " : "") + QString::fromLatin1(words[rng() % (sizeof words / sizeof *words)]);
        c.insertText(text + QLatin1Char('.'), body);
    }
}

inline std::unique_ptr<QTextDocument> makeStory(int paragraphs, unsigned seed, const std::function<void(int, QTextBlockFormat &)> &format = {}, int fewest = 14, int most = 60)
{
    auto doc = std::make_unique<QTextDocument>();
    fillStory(doc.get(), paragraphs, seed, format, fewest, most);
    return doc;
}

inline FrameSpec plainFrame(double w, double h)
{
    FrameSpec fs;
    fs.size = QSizeF(w, h);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    return fs;
}

// The text of a publication's notes, for the layouts that have footnotes.
struct TestNotes : FieldSource {
    int firstPageNumber() const override { return 1; }
    QString pageNumberFormat() const override { return QString(); }
    QString businessField(const QString &) const override { return QString(); }
    QStringList mergeFields() const override { return {}; }
    QString mergeValue(int, const QString &) const override { return QString(); }
    const QTextDocument *storyDoc(const QString &id) const override { auto it = docs.find(id); return it == docs.end() ? nullptr : it->second.get(); }
    QList<const QTextDocument *> storyDocs() const override { QList<const QTextDocument *> l; for (auto &p : docs) l << p.second.get(); return l; }
    double noteSeparatorHeight() const override { return separator; }
    double noteContinuationHeight() const override { return continuation; }
    double noteNoticeHeight() const override { return notice; }
    std::map<QString, std::unique_ptr<QTextDocument>> docs;
    double separator = 0, continuation = 0, notice = 0;   // (the heights of a file's separator, and of its own for the part of a note that goes on)
};

inline bool sameBrief(const StoryLayout::LineBrief &a, const StoryLayout::LineBrief &b, int frameShift = 0, int posShift = 0)
{
    // (A paragraph that goes on past the page by more lines than the page asks for is cut there: how many lines it has isn't known.)
    const bool sameCount = a.lines == b.lines || a.lines == (1 << 28) || b.lines == (1 << 28);
    return a.frame == b.frame + frameShift && a.block == b.block + posShift && a.line == b.line && sameCount && a.pos == b.pos + posShift &&
           a.end == b.end + posShift && std::abs(a.rect.top() - b.rect.top()) < 0.001 && std::abs(a.rect.left() - b.rect.left()) < 0.001 &&
           std::abs(a.rect.width() - b.rect.width()) < 0.001 && std::abs(a.rect.height() - b.rect.height()) < 0.001 && std::abs(a.below - b.below) < 0.001;
}

// Lays out three pages with a footnote that may be cut at the foot of the first, then the second and third by themselves with the
// note carried over; empty if they agree. `cut` tells whether the note was cut (if it wasn't, there is nothing to compare).
inline QString carriedFootnoteCheck(double height, unsigned seed, bool *cut)
{
    TestNotes notes;
    auto note = std::make_unique<QTextDocument>();
    {
        QTextCursor nc(note.get());
        QTextCharFormat cf;
        cf.setFontPointSize(11);
        QString text;
        for (int i = 0; i < 70; ++i) text += QStringLiteral("note%1 words ").arg(i);
        nc.insertText(text, cf);
    }
    notes.docs[QStringLiteral("n1")] = std::move(note);
    auto doc = makeStory(12, seed);
    {
        // A reference on the first line of the first paragraph.
        QTextCursor c(doc.get());
        c.setPosition(6);
        QTextCharFormat ref;
        ref.setFontPointSize(11);
        ref.setProperty(tp::Field, QStringLiteral("footnote:n1"));
        c.insertText(QString(QChar(0xFFFC)), ref);
    }
    auto frame = [&](double h) { FrameSpec fs = plainFrame(300, h); fs.ctx.doc = &notes; return fs; };
    const QVector<FrameSpec> pages{frame(height), frame(260), frame(260)};
    StoryLayout whole;
    whole.build(doc.get(), pages, LayoutEnv());
    *cut = false;
    if (whole.notes().size() != 1) return QStringLiteral("%1 notes").arg(whole.notes().size());
    const auto &n = whole.notes().first();
    if (n.frame != 0 || n.more.isEmpty() || n.more.first().frame != 1) return QString();   // not cut at the foot of the first page
    *cut = true;
    const auto carried = whole.carriedNotes(1);
    if (carried.size() != 1 || carried.first().storyId != QLatin1String("n1") || carried.first().number != 1 || carried.first().from <= 0)
        return QStringLiteral("the note was not carried out of the first page");
    if (whole.carriedNotes(2).size() != (n.more.size() > 1 ? 1 : 0)) return QStringLiteral("carried out of the second page");
    // The second page by itself, from where it starts.
    const auto lines = whole.lineBriefs();
    int taken = 0, firstPos = -1;
    for (const auto &l : lines)
        if (l.frame == 1) { firstPos = l.pos; taken = l.line; break; }
    if (firstPos <= 0) return QStringLiteral("no second page");
    QVector<FrameSpec> rest{frame(100000), pages[1], pages[2]};
    StoryLayout run;
    StoryLayout::Span span;
    span.from = firstPos;
    span.firstBreakAfter = taken;
    span.carried = carried;
    if (taken == 0) { rest.removeFirst(); span.firstBreakAfter = -1; }
    run.build(doc.get(), rest, LayoutEnv(), span);
    const int shift = taken == 0 ? 1 : 0;   // (without lines in a frame for the page before, the second page is the first frame)
    // (A reference in a line that stood in for the page before has a note of its own there, drawn nowhere.)
    const StoryLayout::Note *continued = nullptr;
    for (const auto &r : run.notes())
        if (r.frame == -2) continued = &r;
    if (!continued) return QStringLiteral("no note goes on");
    const auto &m = *continued;
    if (m.storyId != QLatin1String("n1") || m.number != 1 || m.more.isEmpty()) return QStringLiteral("the note that goes on is wrong");
    if (m.more.first().frame != 1 - shift) return QStringLiteral("the note goes on in frame %1").arg(m.more.first().frame);
    if (m.more.first().from != n.more.first().from) return QStringLiteral("the note goes on from %1, not %2").arg(m.more.first().from).arg(n.more.first().from);
    if (std::abs(m.more.first().height - n.more.first().height) > 0.01 || std::abs(m.more.first().rect.top() - n.more.first().rect.top()) > 0.01 ||
        m.more.first().rect.width() != n.more.first().rect.width() || m.more.size() != n.more.size())
        return QStringLiteral("the note goes on in a different place");
    // The lines on the page are placed above the note, as they were.
    const auto got = run.lineBriefs();
    int g = 0;
    for (const auto &l : lines) {
        if (l.frame < 1) continue;
        while (g < got.size() && got[g].frame < l.frame - shift) ++g;
        if (g >= got.size() || !sameBrief(got[g], l, -shift)) return QStringLiteral("a line of the second or third page differs");
        ++g;
    }
    return QString();
}

// The pages of a story laid out whole, in frames the size of the section's columns, for the lines a paginator should find on each page.
inline QVector<QVector<StoryLayout::LineBrief>> oneShotPages(const FlowDocument &fd, int pages)
{
    const Section &sec = fd.section(0);
    const int columns = std::max(1, sec.columns);
    QVector<FrameSpec> frames;
    for (int p = 0; p < pages; ++p)
        for (int c = 0; c < columns; ++c) frames << plainFrame(sec.columnWidth(), sec.content().height());
    StoryLayout lay;
    lay.build(fd.story(), frames, fd.env);
    QVector<QVector<StoryLayout::LineBrief>> out(pages);
    for (const auto &l : lay.lineBriefs())
        if (l.frame >= 0) {
            StoryLayout::LineBrief b = l;
            b.frame = l.frame % columns;
            out[l.frame / columns] << b;
        }
    return out;
}

// The first difference between a paginator's pages and the one-shot layout's, or nothing.
inline QString differenceFromOneShot(FlowDocument &fd, Paginator &pg)
{
    const int pages = pg.pageCount();
    const auto whole = oneShotPages(fd, pages + 1);
    for (int k = 0; k < pages; ++k) {
        const auto mine = pg.lines(k);
        const auto &want = whole[k];
        if (mine.size() != want.size()) return QStringLiteral("page %1 has %2 lines, the one-shot layout's has %3").arg(k).arg(mine.size()).arg(want.size());
        for (int i = 0; i < mine.size(); ++i)
            if (!sameBrief(mine[i], want[i])) return QStringLiteral("line %1 of page %2 differs").arg(i).arg(k);
    }
    if (!whole[pages].isEmpty()) return QStringLiteral("the one-shot layout has more pages than %1").arg(pages);
    return QString();
}

// A page's starts, lines, and (made) items as text, to say whether two paginators of the same document agree.
inline QString pageSignature(Paginator &pg, int k, bool made)
{
    auto r2 = [](double v) { return QString::number(std::round(v * 100) / 100); };
    QString s;
    const auto page = pg.page(k);
    s += QStringLiteral("P%1 %2/%3/%4/%5/%6/%7/[%8] number %9 area %10,%11,%12,%13\n").arg(k).arg(page.start.pos).arg(page.start.skip).arg(page.start.hard).arg(page.start.section).arg(page.start.foot)
             .arg(page.start.end).arg(page.start.carried.join(QLatin1Char(','))).arg(page.number).arg(r2(page.content.left())).arg(r2(page.content.top())).arg(r2(page.content.width())).arg(r2(page.content.height()));
    for (const auto &l : pg.lines(k))
        s += QStringLiteral(" L%1/%2 b%3 %4-%5 f%6 %7,%8,%9,%10\n").arg(l.line).arg(l.lines).arg(l.block).arg(l.pos).arg(l.end).arg(l.frame).arg(r2(l.rect.left())).arg(r2(l.rect.top())).arg(r2(l.rect.width())).arg(r2(l.rect.height()));
    if (!made) return s;
    if (const auto m = pg.materialize(k))
        for (const auto &item : m->doc->pages[0]->items) {
            QString what;
            if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                if (const Story *st = m->doc->story(t->storyId)) what = st->doc->toPlainText().left(24);
            s += QStringLiteral(" I%1 %2,%3,%4,%5 %6\n").arg(int(item->type())).arg(r2(item->rect.left())).arg(r2(item->rect.top())).arg(r2(item->rect.width())).arg(r2(item->rect.height())).arg(what);
        }
    else s += QStringLiteral(" (not made)\n");
    return s;
}

// The places the pages begin at, for a message.
inline QString startsText(Paginator &p)
{
    QStringList l;
    const int n = p.pageCount();
    for (int k = 0; k < n; ++k) {
        const auto s = p.page(k).start;
        l << QStringLiteral("%1/%2/%3/%4/%5/%6[%7]").arg(s.pos).arg(s.skip).arg(s.hard).arg(s.section).arg(s.foot).arg(s.end).arg(s.carried.join(QLatin1Char(',')));
    }
    return l.join(QLatin1Char(' '));
}

// The first page where `incremental` (a paginator that was laid out before the story was edited) differs from a paginator made after, or nothing.
inline QString differenceFromFresh(FlowDocument &fd, Paginator &incremental, bool made = true)
{
    Paginator fresh(&fd);
    const int pages = incremental.pageCount(), want = fresh.pageCount();
    if (pages != want) {
        return QStringLiteral("%1 pages after the edits, %2 when made afresh; starts [%3] and [%4]").arg(pages).arg(want).arg(startsText(incremental), startsText(fresh));
    }
    for (int k = 0; k < pages; ++k) {
        const QString a = pageSignature(incremental, k, made), b = pageSignature(fresh, k, made);
        if (a != b) {
            const QStringList la = a.split(QLatin1Char('\n')), lb = b.split(QLatin1Char('\n'));
            for (int i = 0; i < std::max(la.size(), lb.size()); ++i)
                if (la.value(i) != lb.value(i)) return QStringLiteral("page %1 differs at its line %2: [%3] and [%4] afresh").arg(k).arg(i).arg(la.value(i), lb.value(i));
        }
    }
    return QString();
}

inline QTextBlockFormat formatAt(const FlowDocument &fd, int position) { return fd.story()->findBlock(position).blockFormat(); }

// A paragraph split between two pages leaves at least `lines` of its lines on each side of the break (if it began on the page: of the first
// page's, however many it has); empty if every one does.
inline QString splitsLeaveLines(FlowDocument &fd, Paginator &pg, int lines)
{
    const int pages = pg.pageCount();
    for (int k = 0; k + 1 < pages; ++k) {
        const auto a = pg.lines(k), b = pg.lines(k + 1);
        if (a.isEmpty() || b.isEmpty() || a.last().block != b.first().block) continue;
        if (!formatAt(fd, a.last().block).boolProperty(tp::WidowControl)) continue;
        int onPage = 0;
        for (const auto &l : a) onPage += l.block == a.last().block;
        const bool began = a.last().line - onPage + 1 == 0;
        const int left = a.last().lines - (a.last().line + 1);
        if (left < lines) return QStringLiteral("page %1 leaves %2 line(s) of a paragraph for the next").arg(k).arg(left);
        if (began && onPage < lines) return QStringLiteral("page %1 holds %2 line(s) of a paragraph that goes on").arg(k).arg(onPage);
    }
    return QString();
}

// Every character of the story is on exactly one page, in order, and a paragraph's lines follow one another across the pages; empty if so.
inline QString coverageProblem(Paginator &pg, FlowDocument &fd, bool allowBlankLast = false, bool allowNoteOnly = false)
{
    const QString text = fd.story()->toPlainText();
    const int pages = pg.pageCount();
    int previousEnd = 0, block = -1, line = -1, lines = 0;
    auto blank = [&](int from, int to) {
        for (int p = from; p < to && p < text.size(); ++p)
            if (!text[p].isSpace() && text[p] != QChar::ParagraphSeparator) return false;
        return true;
    };
    for (int k = 0; k < pages; ++k) {
        const auto page = pg.lines(k);
        if (page.isEmpty() && k == pages - 1 && k > 0 && allowBlankLast) continue;   // (a page break after the last paragraph makes one)
        if (page.isEmpty() && allowNoteOnly && !pg.page(k).start.carried.isEmpty()) continue;   // (a note that goes on over pages of its own)
        if (page.isEmpty()) return QStringLiteral("page %1 of %2 has no lines").arg(k).arg(pages);
        for (const auto &l : page) {
            if (l.pos < previousEnd) return QStringLiteral("page %1: a line at %2 begins before the one before it ends, at %3 (a line twice)").arg(k).arg(l.pos).arg(previousEnd);
            if (!blank(previousEnd, l.pos)) return QStringLiteral("page %1: text from %2 to %3 is on no page").arg(k).arg(previousEnd).arg(l.pos);
            if (block >= 0) {
                if (l.block == block && l.line != line + 1) return QStringLiteral("page %1: line %2 of a paragraph follows line %3").arg(k).arg(l.line).arg(line);
                if (l.block != block && (line != lines - 1 || l.line != 0)) return QStringLiteral("page %1: a paragraph is left before its last line, or begins after its first").arg(k);
            }
            previousEnd = std::max(previousEnd, l.end);
            block = l.block;
            line = l.line;
            lines = l.lines;
        }
    }
    if (!blank(previousEnd, text.size())) return QStringLiteral("the text after %1 is on no page").arg(previousEnd);
    if (block >= 0 && line != lines - 1) return QStringLiteral("the last paragraph's last line is on no page");
    return QString();
}

// The pages' starts of a paginator, all of them.
inline QVector<Paginator::PageStart> startsOf(Paginator &pg)
{
    QVector<Paginator::PageStart> v;
    const int n = pg.pageCount();
    for (int k = 0; k < n; ++k) v << pg.page(k).start;
    return v;
}

// What differs between a page made into a document and the lines the paginator found for it, or nothing.
inline QString materializedDiffers(FlowDocument &fd, Paginator &pg, int k, LayoutCache &cache)
{
    const auto mine = pg.lines(k);
    const std::unique_ptr<MaterializedPage> page = pg.materialize(k);
    if (!page) return QStringLiteral("page %1 not made").arg(k);
    if (page->doc->pages.size() != 1) return QStringLiteral("page %1 has %2 pages").arg(k).arg(page->doc->pages.size());
    const QString text = fd.story()->toPlainText();
    QVector<int> perColumn;
    for (const auto &l : mine) {
        while (perColumn.size() <= l.frame) perColumn << 0;
        ++perColumn[l.frame];
    }
    if (page->boxes.size() != perColumn.size()) return QStringLiteral("page %1: %2 boxes for %3 columns with lines").arg(k).arg(page->boxes.size()).arg(perColumn.size());
    int at = 0;
    for (int c = 0; c < page->boxes.size(); ++c) {
        auto *box = dynamic_cast<TextItem *>(page->doc->item(page->boxes[c]));
        if (!box) return QStringLiteral("page %1: no box %2").arg(k).arg(c);
        const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, 1, RenderOptions());
        if (!fl.layout) return QStringLiteral("page %1: no layout").arg(k);
        const auto got = fl.layout->lineInfo(fl.frame);
        if (got.size() != perColumn[c]) return QStringLiteral("page %1 column %2: %3 lines, the paginator found %4").arg(k).arg(c).arg(got.size()).arg(perColumn[c]);
        for (int i = 0; i < got.size(); ++i, ++at) {
            const auto &want = mine[at];
            if (std::abs(got[i].rect.top() - want.rect.top()) > 0.01 || std::abs(got[i].rect.left() - want.rect.left()) > 0.01 ||
                std::abs(got[i].rect.width() - want.rect.width()) > 0.01 || std::abs(got[i].rect.height() - want.rect.height()) > 0.01)
                return QStringLiteral("page %1 column %2 line %3 is in another place").arg(k).arg(c).arg(i);
            if (got[i].docPos + page->storyFrom != want.pos || got[i].docEnd + page->storyFrom != want.end)
                return QStringLiteral("page %1 column %2 line %3 is another part of the text").arg(k).arg(c).arg(i);
            // (The page layer shows a page break as a line break.)
            if (got[i].text != text.mid(want.pos, want.end - want.pos).replace(QChar(0x000C), QChar(0x2028))) return QStringLiteral("page %1 column %2 line %3 has other words").arg(k).arg(c).arg(i);
        }
    }
    return QString();
}


// ---- tables

// Appends a table to the end of the story: `rows` rows of `cols` cells of `columnWidth` points, each cell a paragraph (or several, in the text's lines) of
// `text(row, col)`, with the cell and row properties that `setCell` and `setRow` give. Returns the first position of each row.
struct TestTable {
    int rows = 0, cols = 0;
    double columnWidth = 100;
    QMarginsF margins{4, 3, 4, 3};
    std::function<QString(int, int)> text = [](int r, int c) { return QStringLiteral("r%1c%2").arg(r).arg(c); };
    std::function<void(int, int, CellData &)> setCell;
    std::function<void(int, RowData &)> setRow;
    std::function<void(TableData &)> setTable;
    QVector<int> rowStarts;
    std::vector<int> keys;   // the cells' keys, row after row
};

inline TestTable addTable(FlowDocument &fd, TestTable t)
{
    QTextCursor c(fd.story());
    c.movePosition(QTextCursor::End);
    QTextCharFormat body;
    body.setFontPointSize(11);
    auto table = std::make_shared<TableData>();
    for (int i = 0; i < t.cols; ++i) table->columns << t.columnWidth;
    table->margins = t.margins;
    if (t.setTable) t.setTable(*table);
    for (int r = 0; r < t.rows; ++r) {
        auto row = std::make_shared<RowData>();
        if (t.setRow) t.setRow(r, *row);
        for (int col = 0; col < t.cols; ++col) {
            CellRecord rec;
            rec.table = table;
            rec.row = row;
            if (t.setCell) t.setCell(r, col, rec.cell);
            if (rec.cell.vmerge == CellData::VMerge::Continue && t.setCell == nullptr) rec.cell.vmerge = CellData::VMerge::None;
            const int key = fd.tables.add(rec);
            t.keys.push_back(key);
            const QStringList paragraphs = t.text(r, col).split(QLatin1Char('\n'));
            for (int k = 0; k < paragraphs.size(); ++k) {
                QTextBlockFormat bf;
                bf.setProperty(fp::Cell, key);
                c.insertBlock(bf, body);
                if (k == 0 && col == 0) t.rowStarts << c.block().position();
                c.insertText(paragraphs[k], body);
            }
        }
    }
    return t;
}

// The lines of page `index` that are rows of tables, in order.
inline QVector<StoryLayout::LineBrief> rowLines(Paginator &pg, FlowDocument &fd, int index)
{
    QVector<StoryLayout::LineBrief> out;
    for (const auto &l : pg.lines(index))
        if (fd.story()->findBlock(l.block).blockFormat().intProperty(fp::Cell)) out << l;
    return out;
}

// Like addTable, but the rows go in after the paragraph that has block number `after` (the story goes on after them), not at the end of the story.
inline TestTable addTableAfter(FlowDocument &fd, int after, TestTable t)
{
    QTextCursor c(fd.story());
    const QTextBlock b = fd.story()->findBlockByNumber(after);
    c.setPosition(b.position() + b.length() - 1);
    QTextCharFormat body;
    body.setFontPointSize(11);
    auto table = std::make_shared<TableData>();
    for (int i = 0; i < t.cols; ++i) table->columns << t.columnWidth;
    table->margins = t.margins;
    if (t.setTable) t.setTable(*table);
    for (int r = 0; r < t.rows; ++r) {
        auto row = std::make_shared<RowData>();
        if (t.setRow) t.setRow(r, *row);
        for (int col = 0; col < t.cols; ++col) {
            CellRecord rec;
            rec.table = table;
            rec.row = row;
            if (t.setCell) t.setCell(r, col, rec.cell);
            const int key = fd.tables.add(rec);
            t.keys.push_back(key);
            const QStringList paragraphs = t.text(r, col).split(QLatin1Char('\n'));
            for (int k = 0; k < paragraphs.size(); ++k) {
                QTextBlockFormat bf;
                bf.setProperty(fp::Cell, key);
                c.insertBlock(bf, body);
                if (k == 0 && col == 0) t.rowStarts << c.block().position();
                c.insertText(paragraphs[k], body);
            }
        }
    }
    return t;
}

// The rows of the tables in the story, each as the stretch of the story it takes (from its first paragraph, to just past its last) and the table it is in.
struct RowRange {
    int from = 0, to = 0;
    const TableData *table = nullptr;
};

inline QVector<RowRange> rowRangesOf(FlowDocument &fd)
{
    QVector<RowRange> out;
    const RowData *row = nullptr;
    for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next()) {
        const CellRecord *rec = fd.tables.get(b.blockFormat().intProperty(fp::Cell));
        if (!rec) {
            row = nullptr;
            continue;
        }
        if (row == rec->row.get() && !out.isEmpty()) out.last().to = b.position() + b.length();
        else out << RowRange{b.position(), b.position() + b.length(), rec->table.get()};
        row = rec->row.get();
    }
    return out;
}

// A row like the one given, put in after it (one paragraph in each cell); false if the story has no such row.
inline bool copyRowOf(FlowDocument &fd, const RowRange &r)
{
    QVector<int> keys;
    for (QTextBlock b = fd.story()->findBlock(r.from); b.isValid() && b.position() < r.to; b = b.next()) {
        const int key = b.blockFormat().intProperty(fp::Cell);
        if (keys.isEmpty() || keys.last() != key) keys << key;
    }
    if (keys.isEmpty() || !fd.tables.get(keys[0])) return false;
    auto row = std::make_shared<RowData>(*fd.tables.get(keys[0])->row);
    QTextCursor c(fd.story());
    c.setPosition(r.to - 1);
    QTextCharFormat body;
    body.setFontPointSize(11);
    for (const int key : keys) {
        CellRecord rec = *fd.tables.get(key);
        rec.row = row;
        QTextBlockFormat bf;
        bf.setProperty(fp::Cell, fd.tables.add(rec));
        c.insertBlock(bf, body);
        c.insertText(QStringLiteral("added cell with a few words"), body);
    }
    return true;
}

// A paragraph of 11 point text put in at the end of the story.
inline void appendParagraph(FlowDocument &fd, const QString &text, const std::function<void(QTextBlockFormat &)> &format = {})
{
    QTextCursor c(fd.story());
    c.movePosition(QTextCursor::End);
    QTextBlockFormat bf;
    bf.setBottomMargin(6);
    if (format) format(bf);
    QTextCharFormat body;
    body.setFontPointSize(11);
    c.insertBlock(bf, body);
    c.insertText(text, body);
}

// Numbers as text, the first few of them.
inline QString intsText(const QVector<int> &v, int most = 24)
{
    QStringList l;
    for (int i = 0; i < std::min<int>(int(v.size()), most); ++i) l << QString::number(v[i]);
    return l.join(QLatin1Char(' ')) + (v.size() > most ? QStringLiteral(" ...") : QString());
}

// A footer story of a line with the page's number and the document's page count in it, and then the lines given.
inline std::shared_ptr<QTextDocument> footerWithPageFields(const QStringList &more)
{
    auto doc = std::make_shared<QTextDocument>();
    doc->setDocumentMargin(0);
    QTextCursor c(doc.get());
    QTextCharFormat cf, page, pages;
    cf.setFontPointSize(10);
    page = cf;
    pages = cf;
    page.setProperty(tp::Field, QStringLiteral("page"));
    pages.setProperty(tp::Field, QStringLiteral("pages"));
    c.insertText(QStringLiteral("Page "), cf);
    c.insertText(QString(QChar::ObjectReplacementCharacter), page);
    c.insertText(QStringLiteral(" of "), cf);
    c.insertText(QString(QChar::ObjectReplacementCharacter), pages);
    for (const QString &line : more) {
        c.insertBlock();
        c.insertText(line, cf);
    }
    return doc;
}

// The height of one line of 11 point text as the engine sets it, and the width of a cell's text area (for expectations that do not depend on the fonts).
inline double lineHeightOf(FlowDocument &fd)
{
    auto doc = makeStory(1, 1, [](int, QTextBlockFormat &bf) { bf.setBottomMargin(0); }, 1, 1);
    StoryLayout l;
    l.build(doc.get(), {plainFrame(200, 400)}, fd.env);
    return l.lineBriefs().first().rect.height();
}

// The rows of a table that goes on over a page break, painted, with the lines at the break: what the lines of the page's first and last rows take, and the painted
// fragments of the first page and the next.
struct BreakPages {
    std::shared_ptr<MaterializedPage> first, next;
    QImage firstImage, nextImage;
    double firstTop = 0, firstBottom = 0, nextTop = 0, nextBottom = 0;   // where the text of each is: from the fragment's top, the first line's top and the last line's bottom
};
inline BreakPages breakPagesOf(FlowDocument &fd, Paginator &pg)
{
    BreakPages out;
    out.first = pg.materialize(0);
    out.next = pg.materialize(1);
    if (!out.first || !out.next || out.first->tables.isEmpty() || out.next->tables.isEmpty()) return out;
    auto paint = [&](const TableFragment &f) {
        QImage image(int(fd.section(0).pageSize().width()) * 4, int(fd.section(0).pageSize().height()) * 4, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        QPainter painter(&image);
        painter.scale(4, 4);
        f.paint(&painter, TablePaint());
        return image;
    };
    auto extent = [&](const TableFragment &f, double *top, double *bottom) {
        *top = 1e9;
        *bottom = -1e9;
        f.forLines([&](const StoryLayout::LineBrief &, const QRectF &rect) {
            *top = std::min(*top, rect.top() - f.origin.y());
            *bottom = std::max(*bottom, rect.bottom() - f.origin.y());
        });
    };
    out.firstImage = paint(*out.first->tables[0]);
    out.nextImage = paint(*out.next->tables[0]);
    extent(*out.first->tables[0], &out.firstTop, &out.firstBottom);
    extent(*out.next->tables[0], &out.nextTop, &out.nextBottom);
    return out;
}
// Whether a pixel of the painted fragment is the red of the lines of these tests, and the number of dark pixels (the text's) in a band of it, from `y0` to `y1` below its top.
inline bool isRed(const QImage &image, const TableFragment &f, double x, double y)
{
    const QColor c = image.pixelColor(int((f.origin.x() + x) * 4), int((f.origin.y() + y) * 4));
    return c.red() > 200 && c.green() < 80 && c.blue() < 80;
}
inline int darkIn(const QImage &image, const TableFragment &f, double y0, double y1)
{
    int dark = 0;
    for (int y = int((f.origin.y() + y0) * 4); y < int((f.origin.y() + y1) * 4); ++y)
        for (int x = int((f.origin.x() + 4) * 4); x < int((f.origin.x() + 290) * 4); ++x) dark += image.pixelColor(x, y).red() < 150;
    return dark;
}


// A table inside a cell: `rows` by `cols` cells of `colW` points, each a paragraph, the cell of row 0 and column 0 holding another (until `maxLevel` is reached); `parent` is
// the cell the table is in. Appended at the cursor.
inline void addNestedTable(FlowDocument &fd, QTextCursor &c, int level, int maxLevel, int parent, int rows, int cols, double colW)
{
    QTextCharFormat body;
    body.setFontPointSize(11);
    auto table = std::make_shared<TableData>();
    for (int i = 0; i < cols; ++i) table->columns << colW;
    table->margins = QMarginsF(4, 3, 4, 3);
    for (int r = 0; r < rows; ++r) {
        auto row = std::make_shared<RowData>();
        for (int col = 0; col < cols; ++col) {
            CellRecord rec;
            rec.table = table;
            rec.row = row;
            rec.parent = parent;
            const int key = fd.tables.add(rec);
            if (r == 0 && col == 0 && level < maxLevel) addNestedTable(fd, c, level + 1, maxLevel, key, rows, cols, std::max(30.0, colW - 6));
            QTextBlockFormat bf;
            bf.setProperty(fp::Cell, key);
            c.insertBlock(bf, body);
            c.insertText(QStringLiteral("L%1 r%2c%3").arg(level).arg(r).arg(col), body);
        }
    }
}

// What a paginator that was told of edits has that a new one on the same story does not: the pages (their starts, lines, and tables with the heights of their rows).
// Empty when there is nothing.
inline QString differsFromFresh(FlowDocument &fd, Paginator &pg)
{
    Paginator fresh(&fd);
    const int want = fresh.pageCount(), got = pg.pageCount();
    if (want != got) return QStringLiteral("%1 pages, a new paginator finds %2").arg(got).arg(want);
    for (int k = 0; k < want; ++k) {
        if (fresh.page(k).start != pg.page(k).start)
            return QStringLiteral("page %1 starts at (%2, %3, part %4), a new paginator has (%5, %6, part %7)").arg(k).arg(pg.page(k).start.pos).arg(pg.page(k).start.skip).arg(pg.page(k).start.piece)
                .arg(fresh.page(k).start.pos).arg(fresh.page(k).start.skip).arg(fresh.page(k).start.piece);
        const auto a = fresh.lines(k), b = pg.lines(k);
        if (a.size() != b.size()) return QStringLiteral("page %1 has %2 lines, a new paginator's has %3").arg(k).arg(b.size()).arg(a.size());
        for (int i = 0; i < a.size(); ++i)
            if (a[i].pos != b[i].pos || std::abs(a[i].rect.top() - b[i].rect.top()) > 0.01 || std::abs(a[i].rect.height() - b[i].rect.height()) > 0.01)
                return QStringLiteral("page %1, line %2: at %3 and %4 high, a new paginator has %5 and %6").arg(k).arg(i).arg(b[i].rect.top()).arg(b[i].rect.height()).arg(a[i].rect.top()).arg(a[i].rect.height());
        const auto fa = fresh.materialize(k), fb = pg.materialize(k);
        if (!fa || !fb) return QStringLiteral("page %1 is not made (%2, a new paginator's %3)").arg(k).arg(fb ? 1 : 0).arg(fa ? 1 : 0);
        if (fa->tables.size() != fb->tables.size()) return QStringLiteral("page %1 has %2 tables, a new paginator's has %3").arg(k).arg(fb->tables.size()).arg(fa->tables.size());
        for (int i = 0; i < fa->tables.size(); ++i) {
            const TableFragment &x = *fa->tables[i], &y = *fb->tables[i];
            if (x.firstRow != y.firstRow || x.endRow != y.endRow || std::abs(x.height - y.height) > 0.01)
                return QStringLiteral("page %1, table %2: rows %3 to %4, %5 high; a new paginator has %6 to %7, %8 high").arg(k).arg(i).arg(y.firstRow).arg(y.endRow).arg(y.height).arg(x.firstRow).arg(x.endRow).arg(x.height);
            for (int r = x.firstRow; r < x.endRow; ++r)
                if (std::abs(x.heightOf(r) - y.heightOf(r)) > 0.01) return QStringLiteral("page %1, table %2, row %3: %4 high, a new paginator has %5").arg(k).arg(i).arg(r).arg(y.heightOf(r)).arg(x.heightOf(r));
        }
    }
    return QString();
}

// Where the text of page `k`, as the page's own story is laid out, puts its lines, against where the paginator put them (the rows of a table are one item in the page's story).
inline QString tableAwareDiffers(FlowDocument &fd, Paginator &pg, int k, LayoutCache &cache)
{
    const auto mine = pg.lines(k);
    const std::unique_ptr<MaterializedPage> page = pg.materialize(k);
    if (!page) return QStringLiteral("page %1 not made").arg(k);
    struct W { int frame; double top, bottom; bool cell; };
    QVector<W> want;
    for (const auto &l : mine) {
        const bool cell = fd.story()->findBlock(l.block).blockFormat().intProperty(fp::Cell) != 0;
        if (cell && !want.isEmpty() && want.last().cell && want.last().frame == l.frame) want.last().bottom = l.rect.bottom();
        else want.push_back({l.frame, l.rect.top(), l.rect.bottom(), cell});
    }
    QVector<W> got;
    for (int c = 0; c < page->boxes.size(); ++c) {
        auto *box = dynamic_cast<TextItem *>(page->doc->item(page->boxes[c]));
        if (!box) return QStringLiteral("no box");
        const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, 1, RenderOptions());
        if (!fl.layout) return QStringLiteral("no layout");
        for (const auto &li : fl.layout->lineInfo(fl.frame)) got.push_back({c, li.rect.top(), li.rect.bottom(), false});
    }
    if (got.size() != want.size()) return QStringLiteral("page %1: %2 lines (with a row group as one), the paginator has %3").arg(k).arg(got.size()).arg(want.size());
    for (int i = 0; i < got.size(); ++i)
        if (std::abs(got[i].top - want[i].top) > 0.02 || std::abs(got[i].bottom - want[i].bottom) > 0.02)
            return QStringLiteral("page %1 item %2 (%3): materialized %4..%5, paginator %6..%7").arg(k).arg(i).arg(want[i].cell ? "table rows" : "text").arg(got[i].top).arg(got[i].bottom).arg(want[i].top).arg(want[i].bottom);
    return QString();
}

// Paragraphs set the way a word processor sets lines (LayoutEnv::wordLines), each a line of text in the family and size given: the lines of
// the story, in a frame of one page. `shape` sets a paragraph's format (spacing, line height, style).
inline QVector<StoryLayout::LineInfo> wordLinesOf(const QVector<std::pair<QString, double>> &fonts, const std::function<void(int, QTextBlockFormat &)> &shape = {}, bool word = true,
                                           const std::function<void(LayoutEnv &)> &setup = {})
{
    auto doc = std::make_unique<QTextDocument>();
    QTextCursor c(doc.get());
    for (int i = 0; i < fonts.size(); ++i) {
        QTextBlockFormat bf;
        if (shape) shape(i, bf);
        QTextCharFormat cf;
        cf.setFontFamilies({fonts[i].first});
        cf.setFontPointSize(fonts[i].second);
        if (i == 0) c.setBlockFormat(bf); else c.insertBlock(bf);
        c.setBlockCharFormat(cf);
        c.insertText(QStringLiteral("Line %1").arg(i), cf);
    }
    LayoutEnv env;
    env.wordLines = word;
    if (setup) setup(env);
    StoryLayout lay;
    lay.build(doc.get(), {plainFrame(400, 700)}, env);
    return lay.lineInfo(0);
}


// A line of text in a missing face the original's letter widths are measured for, as the layout sets it (size in points, on the word processor's path, kerned
// or not, or the publication's): `x` is each caret's place (one more than there are characters), `cell` where the original's widths put each character (kerning
// pairs included where the text is kerned), `stand` the advance of each character in the stand-in the layout draws (in em), and `width` the original's (in em).
// `splits` are the places where the text goes on in a run of its own; `hyphenate` lets the layout hyphenate words by its own rule. A character that no table measures keeps its stand-in's width in `width` (as in the layout).
struct LetterLine {
    QVector<double> x, cell, stand, width;
};

// A family of the machine's own fonts that has the character (looked for among those the system lists for the East Asian writing systems), or none.
inline QString installedFaceFor(uint ch)
{
    // (The candidates are the layout's own: the first 64 families of each East Asian writing system, in the order Japanese, Simplified and Traditional Chinese, and Korean.)
    for (const QString &family : eastAsianCandidateFamilies())
        if (QRawFont::fromFont(QFont(family)).supportsCharacter(ch)) return family;
    return QString();
}

inline LetterLine letterLineOf(const QString &family, const QString &text, double size, bool word, bool kerned = true, const QVector<int> &splits = {}, bool hyphenate = false)
{
    LetterLine line;
    QTextCharFormat cf;
    cf.setFontFamilies({family});
    // (Where the machine has a face for the East Asian characters of the text, both layouts are given it, as a word processor's layout gives it itself: they are then drawn alike in both.)
    for (const QChar ch : text)
        if (ch.unicode() >= 0x2E80 && ch.unicode() <= 0xD7FF)
            if (const QString face = installedFaceFor(ch.unicode()); !face.isEmpty()) {
                cf.setFontFamilies({family, face});
                break;
            }
    cf.setFontPointSize(size);
    if (word && kerned) cf.setProperty(tp::KernAbove, 1.0);   // (a word processor's text is kerned only where its file asks)
    LayoutEnv env;
    env.wordLines = word;
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        // (A run of its own where the text is split: the same face in another color.)
        int from = 0;
        bool other = false;
        for (int cut : splits + QVector<int>{int(text.size())}) {
            QTextCharFormat part = cf;
            if (other) part.setForeground(QColor(Qt::blue));
            c.insertText(text.mid(from, cut - from), part);
            from = cut;
            other = !other;
        }
        StoryLayout lay;
        FrameSpec frame = plainFrame(8000, 400);
        frame.hyphenate = hyphenate;
        lay.build(&doc, {frame}, env);
        for (int i = 0; i <= text.size(); ++i) {
            int frame = -1;
            QRectF r;
            lay.caretRect(i, &frame, &r);
            line.x << r.x();
        }
    }
    QFont standIn = resolveCharFormat(cf, env).font();
    standIn.setPixelSize(1000);
    standIn.setLetterSpacing(QFont::AbsoluteSpacing, 0);
    standIn.setWordSpacing(0);
    const QFontMetricsF metrics(standIn);
    const QHash<char16_t, double> *widths = originalLetterWidths(family, false, false);
    const QHash<quint32, double> *pairs = !word || kerned ? originalKerning(family, false, false) : nullptr;
    double at = 0;
    for (int i = 0; i < text.size(); ++i) {
        line.cell << at;
        line.stand << metrics.horizontalAdvance(text[i]) / 1000;
        line.width << (widths && widths->contains(text[i].unicode()) ? widths->value(text[i].unicode()) : line.stand.last());   // (a character the table doesn't measure keeps its stand-in's width)
        if (text[i] == QChar(0x00AD) || text[i] == QChar(0x200B)) line.stand.last() = line.width.last() = 0;   // (shown only where a line breaks)
        at += line.width.last() * size;
        if (pairs && i + 1 < text.size()) at += pairs->value(quint32(text[i].unicode()) << 16 | text[i + 1].unicode()) * size;
    }
    line.cell << at;
    return line;
}

// A face this machine lacks, measured, whose stand-in's capital I is much narrower than the original's (by `diff` of an em).
inline QString narrowStandInForI(double *diff)
{
    for (const char *name : {"Elephant", "Bodoni MT", "Verdana", "Tahoma"}) {
        const QString family = QString::fromLatin1(name);
        if (QFontDatabase::hasFamily(family) || !originalLetterWidths(family, false, false)) continue;
        const LetterLine line = letterLineOf(family, QStringLiteral("I"), 10, true);
        *diff = line.width.value(0) - line.stand.value(0);
        if (*diff > 0.08) return family;
    }
    return QString();
}

// The line numbers drawn on page `k`, in order, as text.
inline QStringList lineNumbersOnPage(Paginator &pg, int k)
{
    QStringList shown;
    const auto page = pg.materialize(k);
    if (!page) return shown;
    for (const auto &item : page->doc->pages[0]->items)
        if (item->id.startsWith(QLatin1String("flow-lineno-box"))) shown << page->doc->stories.value(dynamic_cast<const TextItem *>(item.get())->storyId)->doc->toPlainText();
    return shown;
}

// A story of short paragraphs on small pages (about ten lines each), its first section numbering its lines.
inline void numberedStory(FlowDocument &fd, int paragraphs, int countBy, Section::LineNumbers::Restart restart)
{
    fd.env.wordLines = true;
    fillStory(fd.story(), paragraphs, 4, [](int, QTextBlockFormat &bf) { bf.setBottomMargin(0); }, 6, 12);
    fd.sections[0].paper = QSizeF(240, 200);
    fd.sections[0].margins = QMarginsF(60, 30, 30, 30);
    fd.sections[0].lineNumbers.countBy = countBy;
    fd.sections[0].lineNumbers.start = 1;
    fd.sections[0].lineNumbers.distance = 10;
    fd.sections[0].lineNumbers.restart = restart;
    fd.sections[0].lineNumbers.format.setFontPointSize(7);
    // (A bundled face, so that the lines to a page are the same on every machine.)
    QTextCursor all(fd.story());
    all.select(QTextCursor::Document);
    QTextCharFormat face;
    face.setFontFamilies({QStringLiteral("Arimo")});
    all.mergeCharFormat(face);
}

// A header or footer story of the lines given, a paragraph each.
inline std::shared_ptr<QTextDocument> furnitureStory(const QStringList &lines)
{
    auto doc = std::make_shared<QTextDocument>();
    doc->setDocumentMargin(0);
    QTextCursor c(doc.get());
    QTextCharFormat cf;
    cf.setFontPointSize(10);
    for (int i = 0; i < lines.size(); ++i) {
        if (i) c.insertBlock();
        c.insertText(lines[i], cf);
    }
    return doc;
}

// How tall a story is where the paginator lays it out (its lines from the top to the bottom of the last), measured on its own.
inline double storyHeightOf(const QTextDocument *story, double width, const FlowDocument &fd)
{
    StoryLayout lay;
    lay.build(story, {plainFrame(width, 100000)}, fd.env);
    return lay.usedHeight(0);
}

// The text boxes of a materialized page by the text of their stories: the header's, the footer's, the body's (the story longest).
inline QMap<QString, TextItem *> boxesByText(const MaterializedPage &page)
{
    QMap<QString, TextItem *> out;
    for (const auto &item : page.doc->pages[0]->items)
        if (auto *t = dynamic_cast<TextItem *>(item.get()))
            if (const Story *story = page.doc->story(t->storyId)) out.insert(story->doc->toPlainText(), t);
    return out;
}


// Notes for a flowing document: each a paragraph that begins with its mark and says `words` more.
inline void addNote(TestNotes *notes, const QString &id, int words, const QString &name)
{
    auto doc = std::make_unique<QTextDocument>();
    doc->setDocumentMargin(0);
    QTextCursor c(doc.get());
    QTextCharFormat mark, text;
    mark.setFontPointSize(10);
    mark.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    mark.setProperty(tp::Field, QStringLiteral("notemark"));
    text.setFontPointSize(10);
    c.insertText(QString(QChar(0xFFFC)), mark);
    QString t = QStringLiteral(" ") + name;
    for (int i = 0; i < words; ++i) t += QStringLiteral(" word%1").arg(i);
    c.insertText(t, text);
    notes->docs[id] = std::move(doc);
}

// A reference to a note, put into the story at a position.
inline void addReference(QTextDocument *story, int position, const QString &id)
{
    QTextCursor c(story);
    c.setPosition(position);
    QTextCharFormat ref;
    ref.setFontPointSize(11);
    ref.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    ref.setProperty(tp::Field, QStringLiteral("footnote:") + id);
    c.insertText(QString(QChar(0xFFFC)), ref);
}

// The story of a materialized page's text box that holds `text` (its plain text).
inline const TextItem *boxHolding(const MaterializedPage &page, const QString &text, QString *story = nullptr)
{
    for (const auto &item : page.doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (const Story *st = page.doc->story(t->storyId))
                if (st->doc->toPlainText().contains(text)) {
                    if (story) *story = st->doc->toPlainText();
                    return t;
                }
    return nullptr;
}


// A paragraph of four spaces with a border round them (the spec as tp::RunBorder has it), after an empty line, at 12 point type and a line height of `percent` of single,
// drawn at 8 points to the point: the picture, the rectangle of the four spaces, and the rectangle of their line. (Spaces have no ink: every dark point is the border.)
struct BoxedSpaces {
    QImage image;
    QRectF run, line;
    double scale = 8;
    bool dark(double x, double y) const { return x >= 0 && y >= 0 && x < image.width() / scale && y < image.height() / scale && QColor(image.pixel(int(x * scale), int(y * scale))).lightness() < 60; }
};

inline BoxedSpaces boxedSpaces(const QString &border, double percent = 100)
{
    BoxedSpaces out;
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat plain;
    plain.setFontFamilies({QStringLiteral("Times New Roman")});
    plain.setFontPointSize(12);
    QTextCharFormat boxed = plain;
    boxed.setProperty(tp::RunBorder, border);
    QTextBlockFormat bf;   // (an empty line above, to have room over the box)
    bf.setLineHeight(percent, QTextBlockFormat::ProportionalHeight);
    c.insertBlock(bf);
    c.insertText(QStringLiteral("  "), plain);
    c.insertText(QStringLiteral("    "), boxed);
    c.insertText(QStringLiteral("  "), plain);
    const int start = doc.findBlockByNumber(1).position();
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(300, 100)}, env);
    const auto lines = lay.lineInfo(0);
    if (lines.size() != 2) return out;
    out.image = QImage(int(300 * out.scale), int(100 * out.scale), QImage::Format_RGB32);
    out.image.fill(Qt::white);
    QPainter p(&out.image);
    p.scale(out.scale, out.scale);
    lay.paint(&p, 0, PaintOptions());
    p.end();
    const auto rects = lay.rangeRects(0, start + 2, start + 6);
    if (!rects.isEmpty()) out.run = rects.first().united(rects.last());
    out.line = lines[1].rect;
    return out;
}

// A small picture's bytes, as a file.
inline QByteArray tinyPng(int w, int h, const QColor &color)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(color);
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return bytes;
}

// An object to set at a place, with the item given, anchored by a character put into the story at `position`.
inline int addObject(FlowDocument *fd, ItemPtr item, int position, FloatingObject::Rel hRel, FloatingObject::Rel vRel, double dx = 0, double dy = 0)
{
    FloatingObject o;
    o.item = std::move(item);
    o.hRel = hRel;
    o.vRel = vRel;
    o.dx = dx;
    o.dy = dy;
    fd->objects.push_back(o);
    const int id = int(fd->objects.size());
    QTextCursor c(fd->story());
    c.setPosition(position);
    QTextCharFormat anchor;
    anchor.setFontPointSize(11);
    anchor.setProperty(fp::AnchorId, id);
    c.insertText(QString(QChar(0xFFFC)), anchor);
    return id;
}


// An object that the text goes round: a box `w` by `h`, anchored by a character put in at `position`, set from the column across and the paragraph down unless
// `hRel` and `vRel` say otherwise, with room beside, above, and below it as `gap` says.
inline int addWrapped(FlowDocument *fd, double w, double h, int position, FloatingObject::Wrap wrap, double dx, double dy, const QMarginsF &gap = QMarginsF(8, 4, 8, 4),
               FloatingObject::Side side = FloatingObject::Side::Both, FloatingObject::Rel hRel = FloatingObject::Rel::Column, FloatingObject::Rel vRel = FloatingObject::Rel::Paragraph)
{
    auto item = std::make_shared<ShapeItem>();
    item->rect = QRectF(0, 0, w, h);
    const int id = addObject(fd, item, position, hRel, vRel, dx, dy);
    FloatingObject &o = fd->objects[size_t(id) - 1];
    o.wrap = wrap;
    o.gap = gap;
    o.side = side;
    return id;
}

// The lines of the text of page `k` where they are on the page: each without the space under its text, with the column it is in and the stretch of the story it holds.
struct PageLine {
    QRectF rect;
    int column = 0, pos = 0, end = 0, block = 0;
};

inline QVector<PageLine> pageLines(Paginator &pg, FlowDocument &fd, int k)
{
    QVector<PageLine> out;
    const Paginator::Page page = pg.page(k);
    const Section &sec = fd.section(page.start.section);
    for (const auto &l : pg.lines(k)) {
        if (fd.story()->findBlock(l.pos).blockFormat().intProperty(fp::Cell)) continue;
        const QRectF col = sec.columnRect(l.frame);
        out.push_back({QRectF(col.left() + l.rect.left(), page.content.top() + l.rect.top(), l.rect.width(), l.rect.height() - l.below), l.frame, l.pos, l.end, l.block});
    }
    return out;
}

// The top of the first line of the paragraph that begins at `block` on page `k`, as the page has it (page coordinates); -1 if the page has none of it.
inline double paragraphTopOn(Paginator &pg, FlowDocument &fd, int k, int block)
{
    for (const PageLine &l : pageLines(pg, fd, k))
        if (l.block == block) return l.rect.top();
    return -1;
}

// The objects of the made page that are boxes of the size given (width and height), as items.
inline const Item *objectOfSize(const MaterializedPage &page, double w, double h)
{
    for (const auto &it : page.doc->pages[0]->items)
        if (it->type() == ItemType::Shape && std::abs(it->rect.width() - w) < 0.01 && std::abs(it->rect.height() - h) < 0.01) return it.get();
    return nullptr;
}

// Whether every character of the story is on exactly one line of the pages, in order: each line begins where the one before ends (a paragraph's first line at the paragraph's start).
inline QString textIsWholeOnThePages(Paginator &pg, FlowDocument &fd)
{
    int at = 0;
    for (int k = 0; k < pg.pageCount(); ++k)
        for (const PageLine &l : pageLines(pg, fd, k)) {
            const bool paragraphStart = l.pos == fd.story()->findBlock(l.pos).position();
            if (l.pos != at && !(paragraphStart && l.pos == at + 1))
                return QStringLiteral("page %1: the line at %2 does not begin where the one before ended (%3)").arg(k).arg(l.pos).arg(at);
            at = l.end;
        }
    // (And the last line reaches the end of the story's text, and every paragraph is whole: a paragraph cut short on a later page is no gap between two lines.)
    return audit::coverageProblems(pg, fd.story());
}


// A frame: the paragraphs of the text given put into the story before block number `before` (after the one before it), each with the frame's key and set out of the layout,
// and an object that is set as `proto` says (where, and how the text goes round it), `width` wide (0: as wide as its longest line), `height` high at least (or exactly, if
// `exact`; 0: as its text is). Returns its key.
inline int addFrame(FlowDocument &fd, int before, const QStringList &paragraphs, FloatingObject proto, double width, double height = 0, bool exact = false)
{
    proto.item = std::make_shared<TextItem>();
    proto.frameWidth = width;
    proto.frameHeight = height;
    proto.frameExactHeight = exact;
    fd.objects.push_back(proto);
    const int id = int(fd.objects.size());
    fd.frames.push_back(id);
    const int key = int(fd.frames.size());
    fd.objects[size_t(id) - 1].frame = key;
    QTextCursor c(fd.story());
    const QTextBlock b = fd.story()->findBlockByNumber(before - 1);
    c.setPosition(b.position() + b.length() - 1);
    QTextCharFormat body;
    body.setFontPointSize(11);
    for (const QString &text : paragraphs) {
        QTextBlockFormat bf;
        bf.setBottomMargin(4);
        bf.setProperty(fp::Frame, key);
        bf.setProperty(tp::OutOfLayout, true);
        c.insertBlock(bf, body);
        c.insertText(text, body);
    }
    return key;
}

// The positions of the paragraphs of the story that are in a frame.
inline QSet<int> frameBlocksOf(const FlowDocument &fd)
{
    QSet<int> out;
    for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next())
        if (b.blockFormat().intProperty(fp::Frame) > 0) out.insert(b.position());
    return out;
}

// The text boxes of a made page that are frames' (their stories have the words given), as items.
inline const TextItem *frameBoxWith(const MaterializedPage &page, const QString &words)
{
    for (const auto &it : page.doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(it.get()))
            if (t->id.startsWith(QLatin1String("flow-object-")))
                if (const Story *st = page.doc->story(t->storyId))
                    if (st->doc->toPlainText().contains(words)) return t;
    return nullptr;
}


// ---- notes beside tables, sections, and pages: what a first-principles audit asks

// References put into the story at the places given (from the last to the first, so that the places hold).
inline void insertRefsDescending(FlowDocument &fd, QVector<QPair<int, QString>> refs)
{
    std::sort(refs.begin(), refs.end(), [](const auto &a, const auto &b) { return a.first > b.first; });
    for (const auto &r : refs) addReference(fd.story(), r.first, r.second);
}

// The work it takes to find the pages of a document (and to make them all): the pages laid out, the notes set in type, the characters of paragraphs the text engine read, and
// the runs it passed. Counted, so that what a busy machine does to the clock does not change it.
struct PaginateWork {
    qint64 pages = 0, notes = 0, chars = 0, runs = 0;
};

inline PaginateWork workToPaginate(const std::function<void(FlowDocument &, TestNotes &)> &build, int make = 0)
{
    TestNotes notes;
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.fields = &notes;
    build(fd, notes);
    const qint64 chars = StoryLayout::paragraphCharsReadForTest(), runs = StoryLayout::runsPassedForTest(), notesSet = StoryLayout::notesSetInTypeForTest();
    Paginator pg(&fd);
    const int count = pg.pageCount();
    for (int k = 0; k < std::min(count, make); ++k) pg.materialize(k);
    return {pg.pagesLaidOut(), StoryLayout::notesSetInTypeForTest() - notesSet, StoryLayout::paragraphCharsReadForTest() - chars, StoryLayout::runsPassedForTest() - runs};
}

// What the text engine did since a snapshot: the characters of paragraphs it read, the runs it passed, the heights of lines it measured against objects, and the ranges of format it
// made. Counted work, which does not change with the load the machine is under.
struct EngineWork {
    qint64 chars = 0, runs = 0, heights = 0, ranges = 0;
    static EngineWork now()
    {
        return {StoryLayout::paragraphCharsReadForTest(), StoryLayout::runsPassedForTest(), StoryLayout::lineHeightsMeasuredForTest(), StoryLayout::formatRangesMadeForTest()};
    }
    EngineWork since(const EngineWork &before) const { return {chars - before.chars, runs - before.runs, heights - before.heights, ranges - before.ranges}; }
    QString text() const { return QStringLiteral("%1 characters read, %2 runs passed, %3 heights measured, %4 ranges made").arg(chars).arg(runs).arg(heights).arg(ranges); }
};

// Four times the input costs about four times the work when work is in step with it, and sixteen times when it is the square: the counted work of the larger may be at most
// `most` times the smaller's, in each of the counts that matter to the test (a count of nothing in the smaller is not compared).
inline void verifyInStep(const PaginateWork &small, const PaginateWork &big, double most, bool notes, bool chars = true)
{
    const auto what = [&] { return QStringLiteral("%1 characters read and %2 notes set for the smaller, %3 and %4 for the larger").arg(small.chars).arg(small.notes).arg(big.chars).arg(big.notes); };
    if (chars) QVERIFY2(big.chars < most * std::max<qint64>(small.chars, 1), qPrintable(what()));
    if (notes) QVERIFY2(big.notes < most * std::max<qint64>(small.notes, 1), qPrintable(what()));
}

// The tests of where a word processor's tabs go lay a story out twice: once as it is, and once with the default stops (past a paragraph's own) listed as its own, which is where the
// list of a line's stops puts a tab. The two are to be the same in every line and at every letter. (A story is made of its seed alone, so a failure names it.)
struct TabPara {
    QList<QTextOption::Tab> stops;
    double left = 0, indent = 0;
    bool rtl = false;   // set right to left
    QList<std::pair<QString, QTextCharFormat>> runs;
};

// `script`: 0 Latin letters; 1 Hebrew first, and Hebrew and Latin words after; 2 the same in Arabic; 3 a Latin word first, and Hebrew and Latin words after. `rtl`: the paragraphs are set right to left.
inline QList<TabPara> randomTabParas(unsigned seed, int script = 0, bool rtl = false)
{
    std::mt19937 rng(seed * 2654435761u + 17u);
    auto pick = [&](int n) { return int(rng() % unsigned(n)); };
    static const char *const words[] = {"a", "ii", "lil", "tab", "stop", "ledger", "iiiiii", "mm", "of", "the", "text", "illicit", "w", "..."};
    QList<TabPara> paras;
    const int count = 1 + pick(3);
    for (int n = 0; n < count; ++n) {
        TabPara p;
        p.rtl = rtl;
        const int stops = 1 + pick(3);
        double at = 6 + pick(40) + pick(10) / 10.0;
        for (int k = 0; k < stops; ++k) {
            const int t = pick(6);
            QTextOption::Tab stop(at, t < 3 ? QTextOption::LeftTab : t == 3 ? QTextOption::RightTab : t == 4 ? QTextOption::CenterTab : QTextOption::DelimiterTab);
            if (stop.type == QTextOption::DelimiterTab) stop.delimiter = QLatin1Char('.');
            p.stops << stop;
            at += 12 + pick(90) + pick(10) / 10.0;
        }
        static const double lefts[] = {0, 0, 0, 18, 36, 50.5};
        p.left = lefts[pick(6)];
        const int shape = pick(4);
        p.indent = shape == 0 ? 14 : shape == 1 && p.left >= 20 ? -20 : 0;
        const int pieces = 8 + pick(40);
        QTextCharFormat cf;
        for (int k = 0; k < pieces; ++k) {
            if (k == 0 || pick(5) == 0) {
                cf = QTextCharFormat();
                cf.setFontFamilies({pick(2) ? QStringLiteral("Arimo") : QStringLiteral("Tinos")});
                if (script == 2) cf.setFontFamilies({QStringLiteral("DejaVu Sans")});   // (the program's own face that has the Arabic letters)
                cf.setFontPointSize(9 + pick(17) * 0.25);
                if (pick(6) == 0) cf.setFontWeight(QFont::Bold);
            }
            QString text = QString::fromLatin1(words[pick(int(sizeof words / sizeof *words))]);
            if (script != 0) {
                static const QString hebrew[] = {QStringLiteral("\u05E9\u05DC\u05D5\u05DD"), QStringLiteral("\u05E2\u05D5\u05DC\u05DD"), QStringLiteral("\u05DE\u05E1\u05DE\u05DA"),
                                                 QStringLiteral("\u05D8\u05E7\u05E1\u05D8"), QStringLiteral("\u05D0\u05D1")};
                static const QString arabic[] = {QStringLiteral("\u0645\u0631\u062D\u0628\u0627"), QStringLiteral("\u0627\u0644\u0639\u0627\u0644\u0645"), QStringLiteral("\u0643\u062A\u0627\u0628"),
                                                 QStringLiteral("\u0646\u0635")};
                const bool inScript = k == 0 ? script != 3 : pick(5) != 0;
                if (inScript) text = script == 2 ? arabic[pick(4)] : hebrew[pick(5)];
            }
            if (pick(4) != 0) text += QLatin1Char('\t');
            else if (pick(2)) text += QLatin1Char(' ');
            p.runs << std::make_pair(text, cf);
        }
        paras << p;
    }
    return paras;
}

struct TabPlaces {
    QVector<StoryLayout::LineInfo> lines;
    QVector<double> carets;   // where each character of the story starts: its frame's, and its x
};

// `listed`: the default stops past each paragraph's own are the paragraph's own stops (every one the frame can reach).
inline TabPlaces tabPlaces(const QList<TabPara> &paras, double step, double width, bool listed)
{
    QTextDocument doc;
    QTextCursor c(&doc);
    for (int n = 0; n < paras.size(); ++n) {
        const TabPara &p = paras[n];
        QTextBlockFormat bf;
        QList<QTextOption::Tab> stops = p.stops;
        if (listed) {
            double last = p.left;
            for (const auto &t : p.stops) last = std::max(last, t.position);
            for (int j = int(std::floor((last + 0.01) / step)) + 1; j * step < width + 3 * step; ++j) stops << QTextOption::Tab(j * step, QTextOption::LeftTab);   // (each at a multiple of the distance, not a sum of them)
        }
        bf.setTabPositions(stops);
        bf.setLeftMargin(p.left);
        bf.setTextIndent(p.indent);
        if (p.rtl) bf.setLayoutDirection(Qt::RightToLeft);
        if (n == 0) c.setBlockFormat(bf); else c.insertBlock(bf);
        for (const auto &run : p.runs) c.insertText(run.first, run.second);
    }
    LayoutEnv word;
    word.wordLines = true;
    word.defaultTab = step;
    StoryLayout layout;
    layout.build(&doc, {plainFrame(width, 20000)}, word);
    TabPlaces out;
    out.lines = layout.lineInfo(0);
    for (int pos = 0; pos < doc.characterCount(); ++pos) {
        int frame = -1;
        QRectF r;
        layout.caretRect(pos, &frame, &r);
        out.carets << r.x() << r.y();
    }
    return out;
}

// Whether two layouts of one story are the same: a line for a line, and a letter for a letter. The first difference is told.
inline QString tabPlacesDiffer(const TabPlaces &a, const TabPlaces &b)
{
    if (a.lines.size() != b.lines.size()) return QStringLiteral("%1 lines against %2").arg(a.lines.size()).arg(b.lines.size());
    for (int i = 0; i < a.lines.size(); ++i)
        if (a.lines[i].docPos != b.lines[i].docPos || a.lines[i].docEnd != b.lines[i].docEnd)
            return QStringLiteral("line %1 is %2-%3 against %4-%5").arg(i).arg(a.lines[i].docPos).arg(a.lines[i].docEnd).arg(b.lines[i].docPos).arg(b.lines[i].docEnd);
    for (int i = 0; i + 1 < a.carets.size() && i + 1 < b.carets.size(); i += 2)
        if (std::abs(a.carets[i] - b.carets[i]) > 1e-4 || std::abs(a.carets[i + 1] - b.carets[i + 1]) > 1e-4)
            return QStringLiteral("letter %1 is at x %2 against %3").arg(i / 2).arg(a.carets[i]).arg(b.carets[i]);
    return QString();
}

// The first of `stories` stories (made of the seeds 0 on) that is laid out differently with the default stops `step` points apart left to the engine than with them listed, told by its
// seed and what differs; none: an empty string.
inline QString firstTabStoryThatDiffers(double step, unsigned stories, int script = 0)
{
    for (unsigned seed = 0; seed < stories; ++seed) {
        const QList<TabPara> paras = randomTabParas(seed, script);
        const QString why = tabPlacesDiffer(tabPlaces(paras, step, 468, false), tabPlaces(paras, step, 468, true));
        if (!why.isEmpty()) return QStringLiteral("story %1 (script %2) with stops %3 apart: %4").arg(seed).arg(script).arg(step).arg(why);
    }
    return QString();
}

} // namespace
using namespace detail_tst_flow;


class FlowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void sectionsKnowTheirGeometry();
    void documentFindAgreesWithScanAfterRandomEdits();
    void documentFindDoesNotScanOnceIndexed();
    void documentFindRepairsItemsThatShiftedWithoutScanning();
    void documentFindReturnsTheFirstOfTwoItemsWithOneId();
    void layoutDoesNotFollowTheClockWithoutAClockField();
    void layoutFollowsTheClockWithADateOrTimeField();
    void layoutFollowsTheClockWithADateInANote();
    void layoutFollowsWhatItsFieldsShow();
    void layoutIsKeptWhileADateShowsTheSameDay();
    void lineInfoGivesDocumentPositions();
    void spanFromAParagraphIsTheSameAsAStoryFromThere();
    void spanContinuesAParagraphFromTheFrameBefore();
    void spanStopsAtAFrame();
    void spanContinuesAParagraphWithoutItsEarlierLines();
    void spanWindowCutsAHugeParagraph();
    void spanWithoutRulesIgnoresKeepsAndWidows();
    void spanCanLeaveOutTheSpaceBeforeTheFirstParagraph();
    void spanEndsBeforeTheParagraphAtItsEnd();
    void spanCarriesAFootnoteOverFromThePageBefore();
    void paginatorMatchesTheOneShotLayout();
    void paginatorHandlesTinyDocuments();
    void paginatorKeepsWidowsAndOrphans();
    void paginatorKeepsMoreLinesWhenAsked();
    void paginatorKeepsParagraphsWithTheNext();
    void paginatorBreaksPagesWhereAsked();
    void aPageBreakCharacterEndsThePageAfterItsLine();
    void aBreakAtTheStartOfAParagraphIsNotAnOrphan();
    void aLineOverTheFootByLessThanATwentiethOfAPointFits();
    void aLongParagraphThatBeginsWithABreakBreaksTheWholePageBefore();
    void aBreakAfterWhatShowsNothingIsNotAnOrphan();
    void aFileThatSuppressesTheSpaceBeforeAfterAPageBreakHasNone();
    void aFileThatSuppressesTheSpaceBeforeAfterAPageBreakKeepsTheIndent();
    void aBreakKeepsTheSpaceBeforeOnTheNextPageOnlyWhereItBeginsTheParagraph();
    void aPageBreakAtTheEndOfAParagraphMakesNoEmptyLine();
    void theMarkAfterABreakGoesToTheNextPageWhenTheDocumentSaysSo();
    void aPageBreakOnTheLastLineThatFitsStillOpensTheNextPage_data();
    void aPageBreakOnTheLastLineThatFitsStillOpensTheNextPage();
    void aPageBreakInTheMiddleOfAParagraphContinuesItOnTheNextPage();
    void pageBreakCharactersFollowOneAnotherAndOpenAParagraph();
    void editingAPageBreakCharacterRepaginates();
    void paginatorStartsSectionsOnNewPages();
    void paginatorLeavesOutSpaceBeforeAtTheTopOfAPage();
    void wordLinesAreTheFontsWindowsMetrics();
    void wordLinesOfMeasuredFacesAreTheirOwn();
    void wordLinesOfMissingFamiliesAreTheStandIns();
    void symbolFontCharactersAreTheirPicturesAtTheirOwnWidths();
    void courierNewIsDrawnInAThinFaceAndSetAsItself();
    void theCourierStrokeLeavesTheLettersWhereTheyAre();
    void theCourierStrokeIsWorkedOutOncePerLetterAndRun();
    void wordLettersShareTheirWidthsEvenly();
    void wordLettersKeepTheOriginalsPlaceAtEveryBreakInsideTheWord();
    void wordLettersKeepTheOriginalsPlaceAtASoftHyphenAndAHyphenationPoint();
    void aLineOfChineseTextIsSetAsItWas();
    void aWordEndingInACharacterNoPieceCanCarryStillEndsWhereTheOriginalsDoes();
    void wordLettersShareTheirWidthsAcrossRuns();
    void aWordsFirstPartKeepsItsWidthWhereTheNextRunTakesNone();
    void publicationLettersStayCenteredInTheirWidths();
    void newsGothicIsDrawnInALikeGothicAndSetAsItself();
    void universIsDrawnInArimoWiderAndSetAsItself();
    void yuGothicIsDrawnInOpenSansAndSetAsItself();
    void aStandInIsDrawnInTheTextsOwnColorAndWithNoStroke();
    void aWordInAStandInDrawnByHandIsOnePiece();
    void aRunInAMissingFontDrawsEastAsianCharactersInTheMachinesOwnFace();
    void aStretchOfWhiteSpaceLongerThanQtLaysOutIsLaidOutInAFewLines();
    void aParagraphOfOneRunOverHundredsOfPagesIsReadInProportionToItsText();
    void theLinesPastTheEndOfAFrameAskOnceWhetherTheTextIsBlank();
    void runsThatShowNothingBetweenThemAreOneRangeOfFormat();
    void aParagraphWithNoPlaceToBreakIsLaidOutAsBreakingAnywhereWithTheSameLines();
    void aTabThatStartsJustBeforeADefaultStopGoesToItAsTheListOfStopsPutsIt();
    void tabsOfStoriesWithTheirOwnStopsGoWhereTheListOfDefaultStopsPutsThem();
    void tabsToDefaultStopsOfAnUnevenDistanceGoWhereTheListPutsThem();
    void tabsOfParagraphsThatReadRightToLeftGoWhereTheListOfDefaultStopsPutsThem();
    void aParagraphSetRightToLeftIsGivenNoDefaultStops();
    void defaultStopsPastAParagraphsOwnAreNotListedAsStopsThatMakeQtReadTheParagraph();
    void thePagesOfAParagraphOfManyRunsPassTheRunsBeforeThemOnceInAll();
    void thePagesOfAParagraphOfThousandsOfRunsCountItsNotesFromWhereTheLastLeftOff();
    void theFontDatabaseIsScannedForEastAsianFacesOnce();
    void onlyEastAsianCharactersTakeTheirFaceFromTheMachine();
    void aRunInAMissingFontGainsTheMachinesFamilyThatHasItsEastAsianCharacters();
    void aNumberedCourierNewItemKeepsCourierNewsLine();
    void keepWithNextMeasuresTheNextParagraphInCourierNewsLine();
    void aStretchedStandInsStretchStaysWithItsRun();
    void wholePointLinesFollowMultipleAtLeastAndExactSpacing();
    void aFontsAlternateIsKeptWhereTheWordProcessorHasIt();
    void wordTextKernsOnlyWhereTheFileAsks();
    void blanksAtTheEndOfAWordLineTakeNoPartInItsHeight();
    void aTabTakesNoPartInALineHeight();
    void aFileCanAskForWholePointLinesOrNoLeading();
    void aFileCanAskForTrailingSpacesToWrap();
    void aLineDoesNotBreakAfterASlashInALinksAddress();
    void missingFamilyStandsInAsTheFileSays();
    void pageNumbersInDashes();
    void numberIsFollowedByATab();
    void wordLineHeightsFollowTheirRules();
    void aMarkThatShowsNothingIsNoRunOfTheLine();
    void aBulletInItsOwnFontHasRoomInTheFirstLine();
    void aNumberFollowedByASpaceOrNothingIsFollowedByTheText();
    void aNumberInARoomTooNarrowStaysAtItsIndent();
    void paragraphBordersTakeRoom();
    void theFirstLineOfABorderedParagraphGoesInAFrameThatIsTooShort();
    void paragraphBordersAndShadingAreDrawn();
    void anUnderscoreLeaderIsOneLineFromTheTabToItsStop();
    void anAbsolutePositionTabGoesToItsPlaceWhateverTheParagraphsStops();
    void anAbsolutePositionTabLeavesTheParagraphsStopsToItsOrdinaryTabs();
    void aJustifiedLineOfAScriptWithoutSpacesStillSpreadsItsCharacters();
    void anotherProgramsLeaderIsNotGivenAHundredthOfAPointOver();
    void theShadingOfAHangingParagraphJoinsTheShadedParagraphAfterIt();
    void aBorderRoundARunIsDrawnRoundIt();
    void theShadingOfAParagraphThatGoesOnBeginsWhereItDidOnThePageBefore();
    void theBorderRoundARunIsAsHighAsItsTextNotItsLine();
    void aDoubleAndADashedBorderRoundARunAreDrawnAsSuch();
    void aRightTabPastAColumnTakesNoTextOverTheNextColumnAsDrawn();
    void aTextBoxIsGivenNoRoomForARightTabBeyondItAndASingleColumnTheMargin();
    void aLineAsWideAsItsRoomFits();
    void aLineFitsAsMuchWithPunctuationThatHangsAsWithoutIt();
    void aMarkThatHangsLeavesTheTextBeforeItTheRoomExactly();
    void aClosingBracketDoesNotHangPastTheMargin();
    void aRowOfUnderscoresAsWideAsTheRoomIsOneLine();
    void aLineBreaksNeitherBetweenBracketsThatTouchNorAfterAQuestionMark();
    void theRuleAfterAQuestionMarkLeavesEastAsianTextAndSpansRuns();
    void theRulesAgainstABreakAreBoundedByTheLengthOfTheStretch();
    void aLineThatEndsAtADashIsFittedLikeTheOthers();
    void hiddenTextIsLeftOut();
    void automaticSpaceIsFourteenPoints();
    void tabStopsAreFromTheEdgeAndTheDefaultOnesFollowTheLast();
    void aHangingIndentIsATabStopUnlessTheFileSaysNot();
    void aHangingIndentIsATabStopBeforeTheParagraphsOwnStops();
    void justifiedWordLinesReachTheMargin();
    void wordTextHasNoLigatures();
    void wordLigaturesFollowTheRunAndTheScript();
    void punctuationAtTheEndOfALineHangsPastTheMargin();
    void aParagraphThatTurnsHangingOffHasNone();
    void textAfterABreakThatBeginsAParagraphHasItsFirstLineIndent();
    void aParagraphThatIsAllHiddenTakesNoRoom();
    void wordSpaceBetweenParagraphsIsTheLarger();
    void wordSpaceBetweenParagraphsAddsWhenTheFileAsksForIt();
    void aParagraphOutOfTheFlowTakesNoRoom();
    void pagesDrawTheirHeaderAndFooter();
    void theSpaceAfterAHeadersLastParagraphOfLineBreaksIsNotKept();
    void anEmptyFirstParagraphOfAHeaderKeepsItsMark();
    void mirroredMarginsSwapTheSidesOfEvenPages();
    void mirroredMarginsHoldOnAPageLaidOutInParts();
    void insideAndOutsideAreTheSidesOfTheBindingByThePageNumber();
    void aParagraphSetOutsideOrInsideFollowsThePageNumber();
    void evenAndOddHeadersOfOtherHeightsSettleAfterAnEdit();
    void incrementalPaginationMatchesAFreshOneAfterRandomEdits();
    void notesCitedAroundATableAreOnThePagesThatCiteThem_data();
    void notesCitedAroundATableAreOnThePagesThatCiteThem();
    void tableRowsStayInTheBodyThatHeadersAndFootersLeave();
    void aNumberFollowsAPageBreakThatBeginsItsParagraph();
    void aColumnBreakGoesOnInTheNextColumn();
    void aJustifiedLineWithoutASpaceKeepsItsLettersTogether();
    void aDocumentWhoseFirstSectionBeginsOnAnEvenPageNumbersItsFirstPageTwo();
    void aSectionOfOddOrEvenPagesLeavesABlankPageBeforeItWhereTheNumberIsWrong();
    void thePageColorFillsEveryPageUnderTheText();
    void aFramesLinesAreTheRoomItTakesInAHeaderAndAFooter();
    void aSectionsPageBordersAreLinesRoundItsPages();
    void aSectionThatBeganInTheMiddleOfAPageIsNotOnItsFirstPageAfterIt();
    void aParagraphAskedToBeLowerOnThePageBeginsAtThePlaceAsked();
    void aBlankPageForAnOddPageSectionFollowsEditsThatChangeThePagesBefore();
    void theSpaceBeforeIsAfterTheBreakThatBeginsTheParagraphOnly();
    void theColumnsAboveAContinuousSectionAreAsShortAsTheirLinesLetThemBe();
    void aContinuousSectionWithOtherMarginsBeginsOnThePageBelowTheTextAbove();
    void aPageOfThreeSectionsHasAPartForEach();
    void aPageOfSeventeenSectionsIsBoundedAndShowsEveryParagraph();
    void aPagesCopyCutsARunOfJoinedParagraphsWhereTheDocumentDoes();
    void aSectionsLineNumbersAreDrawnInTheMarginAtEachLine();
    void continuousLineNumbersRunOnThroughThePages();
    void newSectionLineNumbersBeginAgainAtEachSection();
    void lineNumbersByFiveGoOnAcrossAPage();
    void aSuppressedParagraphIsNeitherNumberedNorCountedAcrossAPage();
    void anEditOnTheFirstPageMovesTheLineNumbersOnTheThird();
    void lineNumberingWorkIsLinearInTheLinesOfTheDocument();
    void anEditThatOnlyMovesLineNumbersLaysOutNoLaterPage();
    void sectionsOfDifferentLineNumberingGoOnFromOneAnother();
    void aTableInAContinuousTwoColumnSectionIsInsideItsColumnBelowTheTextAbove();
    void aNoteLongerThanThePageCitedFromACellHasPagesOfItsOwn();
    void aTableIsNotShiftedByItsPlaceholderUnderProcessorLineHeights();
    void pageFieldsInAFooterKeepRightForATableThatSpansPages();
    void positionsInARowThatGoesOverPageBreaksAreOnThePagesThatShowThem();
    void marginsLargerThanThePageLeaveTheLeastTextArea();
    void aTallHeaderAndFooterTakeRoomFromTheText();
    void firstAndEvenPagesUseTheirOwnHeaders();
    void pageFieldsKnowTheTotals();
    void aNoteLongerThanTheBodyHasPagesOfItsOwn();
    void denseReferencesTakeTimeInStepWithTheirNumber();
    void noteNumbersRestartAtAContinuousSectionBreak();
    void footnotesAreAtTheFootOfThePageThatCitesThem();
    void aLongFootnoteGoesOnOnTheNextPage();
    void objectsAreSetWherePageParagraphAndLinePutThem();
    void headerObjectsAndInlinePicturesAreOnTheirPages();
    void aParagraphOutOfLayoutHasNoLinesAndNoRoom();
    void aStretchIsNotEndedByAParagraphWithNoLines();
    void aFrameIsSetAtTheParagraphAfterItAndTheTextGoesRoundIt();
    void aFramesHeightIsItsTextsOrWhatItAsks();
    void aFrameWithNoWidthIsAsWideAsTheColumnOrAsItsLongestLineIfItShrinks();
    void aFrameIsOnThePageOfItsParagraph();
    void editsInAFrameLayOutThePageItIsOn();
    void aFrameAtTheStartOfAContinuousSectionLosesNoText();
    void aFrameBeforeTheEmptyParagraphThatEndsASectionIsDrawnWhateverThePageFoot();
    void framesInUseSurviveTheirCacheTurningOver();
    void aFrameOfThousandsOfParagraphsIsInTheText();
    void framesTogetherAndAtTheEndOfTheStory();
    void aParagraphPutInsideAFrameLaysOutThePageOfTheParagraphItWasSetFrom();
    void aFrameRightBeforeATableIsDrawnAtTheTable();
    void aFloatingTableAndAFrameNextToOneAnotherAreBothDrawn();
    void anObjectAnchoredInAFramesTextIsDrawn();
    void thousandsOfFramesInARowLoseNoText();
    void positionsInAFrameOrAFloatingTableAreOnThePageOfTheirParagraph();
    void aTableSetAtAPlaceIsDrawnThereAndTheTextGoesRoundIt();
    void aTableSetAtAPlaceThatIsTooTallIsInTheText();
    void aFloatingTableIsOnThePageOfItsParagraphAndEditsInItLayThatPageOut();
    void floatingTablesInUseSurviveTheirCacheTurningOver();
    void floatingTablesTogetherAndAtTheEndOfTheStory();
    void theNumbersOfTheStoredPropertiesAreNotMovedByNewOnes();
    void randomWallsAreWhereThePageLayerPutsTheText();
    void aParagraphOfManyRunsCostsNoMoreWithAnObjectOnThePage();
    void manyObjectsInOneParagraphAreCutAsTheyAreFound();
    void aPageWithWallsIsLaidOutOnceWhenAskedAgain();
    void aTableSetAtAPlaceFloatsOnlyIfItFitsItsOwnSection();
    void aFloatingTableThatWouldGoOverTheFootGoesToThePageAfter();
    void rowsPutInAtTheTopOfAFloatingTableLayOutThePagesAfterIt();
    void aFloatingTableWiderThanTheTextAreaStillFloats();
    void aFloatingTableOffsetOverTheFootIsSlidUpToTheTextArea();
    void anObjectWhoseWallPushesItsAnchorOffThePageGoesToTheNextWithIt();
    void squareWrapKeepsLinesBesideAnObjectClearOfItsRectangle();
    void anObjectIsSetFromItsAnchorWhereItWouldBeWithoutItsOwnWall();
    void anObjectAboveItsAnchorStaysOnThePageOfTheLinesItMoves();
    void anEmptyParagraphGoesInANarrowGapBetweenObjects();
    void objectsAreDrawnInTheOrderOfTheirHeights();
    void anObjectAnchoredAtTheEndOfAParagraphIsOnItsLastLine();
    void topAndBottomWrapLeavesNoTextBesideTheObject();
    void wrapSidesPutTheTextOnTheSidesAsked();
    void objectsBehindAndOverTheTextMoveNoLine();
    void tightWrapIsSquareWrapForNow();
    void objectsInTheMarginStripsAreWhereTheStripPutsThem();
    void aParagraphThatRunsOnPastTheObjectsPageIsWhole();
    void aParagraphOfSeveralPagesWithAWallLosesNoText();
    void randomWallsAmongLongParagraphsLoseNoText();
    void anEditAmongObjectsLaysOutOnlyThePagesItTouches();
    void anObjectThatFillsThePageDoesNotStopThePagination();
    void aPageGivesAtMostSomeObjectsAWrap();
    void anObjectInOneColumnLeavesTheOtherColumnAlone();
    void aTopAndBottomObjectIsAWallOnlyInTheColumnsItIsIn();
    void textAfterATableGoesRoundAnObjectOnThePage();
    void anObjectAboveItsAnchorLineWrapsTheLinesAbove();
    void anObjectInATableCellIsNoWall();
    void aWrappedObjectInTheHeaderHoldsTheBodyBelowItAndIsNoWall();
    void theRoomAboveAnObjectSetFromItsParagraphLeavesTheLinesBeforeIt();
    void anObjectIsDrawnOnceWhenThePageHasAHeaderToo();
    void theHeaderAndFooterAreDrawnBeforeTheBodysObjects();
    void aTextBoxIsAWallAsAPictureIs();
    void lineBriefsSayHowFarAWrapMovedALine();
    void theTitleOfAnEntryWithANumberPastTheRightIndentWrapsAtTheIndent();
    void aRightStopBeyondAPictureTakesNoTextOverIt();
    void aHangingFirstLineGoesOutIntoTheMargin();
    void aHangingFirstLineGoesOutNoFartherThanThePageIsWide();
    void aRightTabPastTheRightIndentKeepsItsText();
    void aLineOfDoubleSpacingFitsByItsTextNotItsRoomUnderIt();
    void anEmptyParagraphAfterAFramedParagraphWithAHiddenMarkKeepsItsLine();
    void aNoteThatDoesNotFitIsCutWhenThreeOfItsLinesDo();
    void theSpaceAfterTheLastNoteOnAPageMayHangBelowTheFoot();
    void aNoteWithAWidowControlIsCutAfterTwoLinesAndNeverLeavesOne();
    void aNoteIsCutByTheControlOfTheParagraphItIsCutIn();
    void theContinuedPartOfANoteHasTheRoomOfTheFilesOwnSeparatorForIt();
    void aLineBelowATallObjectCostsNothingForEachStep();
    void aLineInSmallTypeIsNotMovedByAnObjectBelowIt();
    void continuousSectionsShareThePage();
    void aContinuousSectionAfterColumnsBeginsBelowTheBalancedColumns();
    void continuousSectionWithOtherColumnsBeginsBelow();
    void paginatorFindsThePageOfAPosition();
    void paginatorLaysOutMoreThanTwoThousandPages();
    void aTableIsLaidOutInRowsBetweenTheTextAroundIt();
    void theLinesOfATableTakeRoomInItsRows();
    void aLineOfATableIsDrawnInsideTheRowItIsTheTopOf();
    void aCellThatSaysNoneAtALineDoesNotTakeItAwayFromACellThatSaysNothing();
    void theLineUnderCellsJoinedDownIsThatOfTheCellThatContinuesThem();
    void aLineAtTheTopOfACellThatContinuesTheOneAboveTakesRoomInItsRow();
    void aTableThatGoesOnAfterAPageBreakHasItsOuterLinesAtTheBreak();
    void aSpacedTableThatGoesOnAfterAPageBreakHasItsFrameAtTheBreak();
    void aSpacedTablePaintsTheLineOfEveryCellsFoot();
    void aCellAcrossColumnsHasTheLineOfEveryCellAboveIt();
    void ofTwoLinesAtAnEdgeTheOneOfTheGreaterWeightIsTheLine();
    void aRowOfASetHeightHasItsMarginsAndLinesBesidesWhenAtLeastAndInsideItWhenExact();
    void aDoubleLineTakesRoomForBothLinesAndTheSpace();
    void theEmptyParagraphAfterATableAtTheEndOfACellTakesNoRoom();
    void aTableBreaksBetweenRowsAcrossPages();
    void headerRowsRepeatOnTheNextPages();
    void aPageBreakBeforeATableStartsItOnANewPage();
    void aRowThatCantBeSplitAndIsTallerThanAPageStillLaysOut();
    void aRowThatDoesNotFitIsSplitBetweenTheLinesOfItsCells();
    void aRowSetToAHeightWithContentNotAtItsTopIsNotBegunInLessRoom();
    void aRowOfATableInACellSetToAHeightIsNotBegunInLessRoomEither();
    void aTableInACellHasItsMarginAgainWhereItsRowGoesOn();
    void aPartOfARowDoesNotShowTheLinesOfThePartBeforeItInItsMargin();
    void aRowWithATableInItIsSplitInsideTheTable();
    void theLinesLeftForTheNextPageAreNotDrawnUnderTheOnesThatAreOnThisOne();
    void aRowOfThousandsOfPartsContinuesToItsEnd();
    void aCellOfThousandsOfParagraphsIsLaidOutWhole();
    void editsGiveTheRowsTheHeightsAFreshLayoutHas();
    void editingARowThatIsInPartsGivesThePagesAFreshLayoutHas();
    void thePageOfAPositionInARowInPartsIsTheOneThatShowsItsLine();
    void aHeaderThatTakesMoreThanAThirdOfAPageIsNotRepeated();
    void aCellOfThousandsOfParagraphsIsNotLaidOutAgainForEachPage();
    void eachCellIsLaidOutOnceToMeasureAndOnceMoreForAPageThatShowsIt();
    void notesCitedBeforeAndInsideATableLandOnThePageThatCitesThem();
    void headersAndFootersOfOtherHeightsBoundTheRowsOfATable();
    void aTwoColumnSectionAfterATableHoldsATableInItsColumns();
    void aNoteCitedInACutRowIsOnThePageOfItsLine();
    void aLongNoteIsCarriedAcrossPagesThatHoldATable();
    void theContinuedPartOfANoteHasItsOwnSeparatorOnAPageThatHoldsATable();
    void theStandInForATableDoesNotShiftItUnderWordLineHeights();
    void objectsAnchoredInTablesAreSetOnceWhereTheirLinesAre();
    void thePageOfAPositionInARowInPartsIsTheOneThatShowsItsLineWithFurniture();
    void theStoryOfAPageWithTablesIsLaidOutWhereThePaginatorPutItsLines();
    void aNoteBesideATableIsSeatedAtTheFootOfItsPage();
    void aParagraphBelowAFloatingTableKeepsItsSpaceBefore();
    void anObjectSetFromAParagraphIsSetAboveTheSpaceBeforeIt();
    void anObjectSetFromAParagraphAtTheTopOfAPageIsSetFromItsFirstLine();
    void anObjectInTheRestOfAParagraphIsSetFromItsFirstLineOnThePage();
    void aTableSetFromItsParagraphIsSetAboveTheSpaceBeforeIt();
    void aPictureInACellOfAFloatingTableIsDrawnWithTheTable();
    void aPictureInTheTextOfACellIsDrawnWithTheTable();
    void repeatedHeaderRowsAreDecidedByTheRoomOfTheColumnTheTableIsIn();
    void aTableIsIndentedToTheTextOfItsFirstCellOrCenteredOrRight();
    void aTableFlowsFromColumnToColumn();
    void aCellWithTurnedTextHasItsRowAsTallAsTheTextIsLong();
    void hostileTablesLayOutAndEnd();
    void paginatorFollowsEditsInTablesAndMeasuresOnlyTheRowsTheyTouch();
    void typingInALongTableIsFast();
    void materializedPagesHaveTheLinesThePaginatorFound();
    void materializedPagesKeepListNumbers();
    void materializedPagesHaveStableIds();
    void materializedPagesPaint();
    void aPageWithATableDrawsItAndKeepsThePositionsOfTheTextAroundIt();
    void paginatorFollowsEditsAndLaysOutOnlyNearby();
    void paginatorKeepsLinesTogether();
    void paginatorMeetsTheSpeedTargets();
    void paginatorSettlesAnEditAfterAPartialSettle();
    void paginatorHandlesSpaceBeforeTallerThanAPage();
    void paginatorKeepsLongChainsWithTheNext();
    void paginatorMakesAPageAfterABreakAtTheEnd();
    void paginatorLaysOutAHugeParagraph();
    void paginatorLaysOutHugeParagraphsInTime();
    void paginatorRestOfAParagraphIsLaidOutAsTheWholeIs();
    void paginatorGrowsTheWindowForTinyText();
    void changingALaterSectionLaysOutOnlyItsPagesAgain();
    void paginatorNumbersPagesBySection();
    void aSectionCanStartItsPageNumbersAtZero();
    void aContinuousSectionRestartsTheNumbersFromThePageItBeginsOn();
    void aRawFieldShowsItsOwnText();
    void paginatorSettlesSeveralEditsAtOnce();
    void restartedNotesAreInTheOrderOfTheTextAndNumberedFromTheirSection();
    void notesInCellsAreNumberedAgainWhenASectionBreakIsPutInAndTakenOut();
    void aSectionThatBeginsMidPageRestartsTheNumbersOnceNotAtEachPage();
    void aSectionWithOtherColumnsBeginsAboveTheNotesAtThePageFoot();
    void theParagraphAfterAnEmptyContinuousBreakIsDrawnInItsSectionsBox();
    void aTopAndBottomWallAcrossTheColumnsItIsInDoesNotReachTheSectionAbove();
    void twoNotesOnThePagesLastLineAreBothDrawn();
    void aNoteCitedInACellThatGoesOverAPageBreakOverlapsNothing();
    void theEndOfALongNoteIsDrawnWhereverItIsCited();
    void aNoteCitedInTheLastParagraphIsDrawnWholeWhateverThePageFills();
    void thePartsOfARowWithNotesFillThePages();
    void rowsHoldTheirSetHeightAtThePageFoot();
    void aRowKeptWithTheNextGoesToTheNextPageWithIt();
    void spaceBetweenCellsIsAboveAndBelowEachRow();
    void thePartsOfARowSetToAHeightAreThatTallAndHoldTheirShortCellsAsARowDoes();
    void aTableOfManyRowsKeptWithTheNextLaysOutInStepWithItsRows();
    void spaceBetweenCellsIsRoundEachCellItsFillAndLinesToo();
    void aRowThatCannotBeSplitCitingManyNotesTakesTimeInStepWithThem();
    void manyRowsCitingManyNotesTakeTimeInStepWithTheirNumber();
    void aLongNoteInANestedCellTakesTimeInStepWithItsLength();
    void aNewDocumentIsNeverGivenADeletedOnesKeptAnswers();
    void aLongNoteInAParagraphOrACellTakesTimeInStepWithItsLength();
    void anchorsInTheCellsOfATableCostInStepWithTheirNumber();
    void aChangeOfTheTextSizeChangesTheHeightsOfNotesBesideTables();
    void deletingACharacterCostsLikeTypingOne();
};
