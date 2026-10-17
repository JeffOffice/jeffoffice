// Tests of libs/flow, and of the changes below it that the flowing document needs: the
// lookup of items by id in the document, the layout cache's key, and the additions to the
// text engine that the paginator uses.

#include "core/document.h"
#include "core/fonts.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "flow/tableflow.h"
#include "../timing.h"
#include "pageaudit.h"
#include "render/renderer.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QApplication>
#include <QBuffer>
#include <QElapsedTimer>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QRegularExpression>
#include <QSet>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QFontDatabase>
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

namespace {

// Where an item is by looking through everything, as Document::find did before it kept an index.
Document::Loc scanFor(const Document &d, const QString &id)
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

bool sameLoc(const Document::Loc &a, const Document::Loc &b)
{
    return a.item == b.item && a.list == b.list && a.index == b.index && a.page == b.page && a.masterId == b.masterId && a.scratch == b.scratch &&
           a.parent == b.parent;
}

// Every list of items in the document (pages, masters, scratch, and the children of groups).
void allLists(Document &d, const std::function<void(ItemList &, GroupItem *)> &fn)
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
void fillStory(QTextDocument *doc, int paragraphs, unsigned seed, const std::function<void(int, QTextBlockFormat &)> &format = {}, int fewest = 14, int most = 60)
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

std::unique_ptr<QTextDocument> makeStory(int paragraphs, unsigned seed, const std::function<void(int, QTextBlockFormat &)> &format = {}, int fewest = 14, int most = 60)
{
    auto doc = std::make_unique<QTextDocument>();
    fillStory(doc.get(), paragraphs, seed, format, fewest, most);
    return doc;
}

FrameSpec plainFrame(double w, double h)
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
    std::map<QString, std::unique_ptr<QTextDocument>> docs;
};

bool sameBrief(const StoryLayout::LineBrief &a, const StoryLayout::LineBrief &b, int frameShift = 0, int posShift = 0)
{
    // (A paragraph that goes on past the page by more lines than the page asks for is cut there: how many lines it has isn't known.)
    const bool sameCount = a.lines == b.lines || a.lines == (1 << 28) || b.lines == (1 << 28);
    return a.frame == b.frame + frameShift && a.block == b.block + posShift && a.line == b.line && sameCount && a.pos == b.pos + posShift &&
           a.end == b.end + posShift && std::abs(a.rect.top() - b.rect.top()) < 0.001 && std::abs(a.rect.left() - b.rect.left()) < 0.001 &&
           std::abs(a.rect.width() - b.rect.width()) < 0.001 && std::abs(a.rect.height() - b.rect.height()) < 0.001 && std::abs(a.below - b.below) < 0.001;
}

// Lays out three pages with a footnote that may be cut at the foot of the first, then the second and third by themselves with the
// note carried over; empty if they agree. `cut` tells whether the note was cut (if it wasn't, there is nothing to compare).
QString carriedFootnoteCheck(double height, unsigned seed, bool *cut)
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
QVector<QVector<StoryLayout::LineBrief>> oneShotPages(const FlowDocument &fd, int pages)
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
QString differenceFromOneShot(FlowDocument &fd, Paginator &pg)
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
QString pageSignature(Paginator &pg, int k, bool made)
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
QString startsText(Paginator &p)
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
QString differenceFromFresh(FlowDocument &fd, Paginator &incremental, bool made = true)
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

QTextBlockFormat formatAt(const FlowDocument &fd, int position) { return fd.story()->findBlock(position).blockFormat(); }

// A paragraph split between two pages leaves at least `lines` of its lines on each side of the break (if it began on the page: of the first
// page's, however many it has); empty if every one does.
QString splitsLeaveLines(FlowDocument &fd, Paginator &pg, int lines)
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
QString coverageProblem(Paginator &pg, FlowDocument &fd, bool allowBlankLast = false, bool allowNoteOnly = false)
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
QVector<Paginator::PageStart> startsOf(Paginator &pg)
{
    QVector<Paginator::PageStart> v;
    const int n = pg.pageCount();
    for (int k = 0; k < n; ++k) v << pg.page(k).start;
    return v;
}

// What differs between a page made into a document and the lines the paginator found for it, or nothing.
QString materializedDiffers(FlowDocument &fd, Paginator &pg, int k, LayoutCache &cache)
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
            if (got[i].text != text.mid(want.pos, want.end - want.pos)) return QStringLiteral("page %1 column %2 line %3 has other words").arg(k).arg(c).arg(i);
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

TestTable addTable(FlowDocument &fd, TestTable t)
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
QVector<StoryLayout::LineBrief> rowLines(Paginator &pg, FlowDocument &fd, int index)
{
    QVector<StoryLayout::LineBrief> out;
    for (const auto &l : pg.lines(index))
        if (fd.story()->findBlock(l.block).blockFormat().intProperty(fp::Cell)) out << l;
    return out;
}

// Like addTable, but the rows go in after the paragraph that has block number `after` (the story goes on after them), not at the end of the story.
TestTable addTableAfter(FlowDocument &fd, int after, TestTable t)
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

QVector<RowRange> rowRangesOf(FlowDocument &fd)
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
bool copyRowOf(FlowDocument &fd, const RowRange &r)
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
void appendParagraph(FlowDocument &fd, const QString &text, const std::function<void(QTextBlockFormat &)> &format = {})
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
QString intsText(const QVector<int> &v, int most = 24)
{
    QStringList l;
    for (int i = 0; i < std::min<int>(int(v.size()), most); ++i) l << QString::number(v[i]);
    return l.join(QLatin1Char(' ')) + (v.size() > most ? QStringLiteral(" ...") : QString());
}

// A footer story of a line with the page's number and the document's page count in it, and then the lines given.
std::shared_ptr<QTextDocument> footerWithPageFields(const QStringList &more)
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
double lineHeightOf(FlowDocument &fd)
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
BreakPages breakPagesOf(FlowDocument &fd, Paginator &pg)
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
bool isRed(const QImage &image, const TableFragment &f, double x, double y)
{
    const QColor c = image.pixelColor(int((f.origin.x() + x) * 4), int((f.origin.y() + y) * 4));
    return c.red() > 200 && c.green() < 80 && c.blue() < 80;
}
int darkIn(const QImage &image, const TableFragment &f, double y0, double y1)
{
    int dark = 0;
    for (int y = int((f.origin.y() + y0) * 4); y < int((f.origin.y() + y1) * 4); ++y)
        for (int x = int((f.origin.x() + 4) * 4); x < int((f.origin.x() + 290) * 4); ++x) dark += image.pixelColor(x, y).red() < 150;
    return dark;
}


// A table inside a cell: `rows` by `cols` cells of `colW` points, each a paragraph, the cell of row 0 and column 0 holding another (until `maxLevel` is reached); `parent` is
// the cell the table is in. Appended at the cursor.
void addNestedTable(FlowDocument &fd, QTextCursor &c, int level, int maxLevel, int parent, int rows, int cols, double colW)
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
QString differsFromFresh(FlowDocument &fd, Paginator &pg)
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
QString tableAwareDiffers(FlowDocument &fd, Paginator &pg, int k, LayoutCache &cache)
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
QVector<StoryLayout::LineInfo> wordLinesOf(const QVector<std::pair<QString, double>> &fonts, const std::function<void(int, QTextBlockFormat &)> &shape = {}, bool word = true,
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


// A header or footer story of the lines given, a paragraph each.
std::shared_ptr<QTextDocument> furnitureStory(const QStringList &lines)
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
double storyHeightOf(const QTextDocument *story, double width, const FlowDocument &fd)
{
    StoryLayout lay;
    lay.build(story, {plainFrame(width, 100000)}, fd.env);
    return lay.usedHeight(0);
}

// The text boxes of a materialized page by the text of their stories: the header's, the footer's, the body's (the story longest).
QMap<QString, TextItem *> boxesByText(const MaterializedPage &page)
{
    QMap<QString, TextItem *> out;
    for (const auto &item : page.doc->pages[0]->items)
        if (auto *t = dynamic_cast<TextItem *>(item.get()))
            if (const Story *story = page.doc->story(t->storyId)) out.insert(story->doc->toPlainText(), t);
    return out;
}


// Notes for a flowing document: each a paragraph that begins with its mark and says `words` more.
void addNote(TestNotes *notes, const QString &id, int words, const QString &name)
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
void addReference(QTextDocument *story, int position, const QString &id)
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
const TextItem *boxHolding(const MaterializedPage &page, const QString &text, QString *story = nullptr)
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


// A small picture's bytes, as a file.
QByteArray tinyPng(int w, int h, const QColor &color)
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
int addObject(FlowDocument *fd, ItemPtr item, int position, FloatingObject::Rel hRel, FloatingObject::Rel vRel, double dx = 0, double dy = 0)
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
int addWrapped(FlowDocument *fd, double w, double h, int position, FloatingObject::Wrap wrap, double dx, double dy, const QMarginsF &gap = QMarginsF(8, 4, 8, 4),
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

QVector<PageLine> pageLines(Paginator &pg, FlowDocument &fd, int k)
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
double paragraphTopOn(Paginator &pg, FlowDocument &fd, int k, int block)
{
    for (const PageLine &l : pageLines(pg, fd, k))
        if (l.block == block) return l.rect.top();
    return -1;
}

// The objects of the made page that are boxes of the size given (width and height), as items.
const Item *objectOfSize(const MaterializedPage &page, double w, double h)
{
    for (const auto &it : page.doc->pages[0]->items)
        if (it->type() == ItemType::Shape && std::abs(it->rect.width() - w) < 0.01 && std::abs(it->rect.height() - h) < 0.01) return it.get();
    return nullptr;
}

// Whether every character of the story is on exactly one line of the pages, in order: each line begins where the one before ends (a paragraph's first line at the paragraph's start).
QString textIsWholeOnThePages(Paginator &pg, FlowDocument &fd)
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
int addFrame(FlowDocument &fd, int before, const QStringList &paragraphs, FloatingObject proto, double width, double height = 0, bool exact = false)
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
QSet<int> frameBlocksOf(const FlowDocument &fd)
{
    QSet<int> out;
    for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next())
        if (b.blockFormat().intProperty(fp::Frame) > 0) out.insert(b.position());
    return out;
}

// The text boxes of a made page that are frames' (their stories have the words given), as items.
const TextItem *frameBoxWith(const MaterializedPage &page, const QString &words)
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
void insertRefsDescending(FlowDocument &fd, QVector<QPair<int, QString>> refs)
{
    std::sort(refs.begin(), refs.end(), [](const auto &a, const auto &b) { return a.first > b.first; });
    for (const auto &r : refs) addReference(fd.story(), r.first, r.second);
}

// How long it takes to find the pages of a document (and to make them all), in milliseconds; the best of `times` runs.
double millisecondsToPaginate(const std::function<void(FlowDocument &, TestNotes &)> &build, int times = 5, bool make = false)
{
    std::unique_ptr<TestNotes> notes;
    std::unique_ptr<FlowDocument> fd;
    std::unique_ptr<Paginator> pg;
    return bestNanoseconds([&] {
        pg = std::make_unique<Paginator>(fd.get());
        const int pages = pg->pageCount();
        if (make)
            for (int k = 0; k < pages; ++k) pg->materialize(k);
    }, times, [&] {
        pg.reset();
        fd.reset();
        notes = std::make_unique<TestNotes>();
        fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fd->fields = notes.get();
        build(*fd, *notes);
    }) / 1e6;
}

} // namespace

class FlowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        initCore();   // (the program's own fonts, as the other tests' programs have them)
        // Stories made here without a family use the application's font: one the program brings (Arimo), so line counts and
        // pages are the same on every system (the system's own default differs on macOS, and two tests failed there).
        QFont font(QStringLiteral("Arimo"));
        font.setPointSizeF(11);
        QGuiApplication::setFont(font);
        QVERIFY(QFontInfo(font).family().contains(QStringLiteral("Arimo")));
    }

    // A section's page is the paper as it is turned, with the text area inside the margins; its columns share that width.
    void sectionsKnowTheirGeometry()
    {
        Section s;
        QCOMPARE(s.pageSize(), QSizeF(612, 792));
        QCOMPARE(s.content(), QRectF(72, 72, 468, 648));
        s.orientation = Section::Orientation::Landscape;
        QCOMPARE(s.pageSize(), QSizeF(792, 612));
        QCOMPARE(s.content(), QRectF(72, 72, 648, 468));
        s.columns = 3;
        s.columnGap = 18;
        QVERIFY(std::abs(s.columnWidth() - (648 - 2 * 18) / 3.0) < 1e-9);
        QVERIFY(std::abs(s.columnRect(0).left() - 72) < 1e-9);
        QVERIFY(std::abs(s.columnRect(2).right() - 720) < 1e-9);
        QVERIFY(std::abs(s.columnRect(1).left() - (72 + s.columnWidth() + 18)) < 1e-9);
        s.columns = 100;
        QCOMPARE(s.columnCount(), jp::flow::kMostColumns);
        s.columns = 0;
        QCOMPARE(s.columnCount(), 1);
        // A document is laid out as if it had a section even when its sections were all taken out.
        FlowDocument fd;
        QCOMPARE(fd.sections.size(), 1);
        fd.sections.clear();
        QCOMPARE(fd.section(3).pageSize(), QSizeF(612, 792));
    }

    // Document::find keeps where each item is, so a lookup doesn't look through every item. After
    // any series of edits (adding, removing, moving, grouping, replacing pages, and loading
    // again) it answers as looking through everything does.
    void documentFindAgreesWithScanAfterRandomEdits()
    {
        std::mt19937 rng(20261016);
        auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
        auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 4);
        auto newItem = [&]() -> ItemPtr { return pick(2) ? Item::create(ItemType::Shape) : Item::create(ItemType::Text); };
        auto randomList = [&]() -> ItemList * {
            std::vector<ItemList *> all;
            allLists(*d, [&](ItemList &l, GroupItem *) { all.push_back(&l); });
            return all[pick(int(all.size()))];
        };
        QStringList seen;
        // Empty when every lookup agrees, else what differed.
        auto check = [&](const QString &when) -> QString {
            QStringList ids;
            allLists(*d, [&](ItemList &l, GroupItem *) { for (const auto &it : l) ids << it->id; });
            seen += ids;
            for (const QString &id : ids) {
                const Document::Loc want = scanFor(*d, id), got = d->find(id);
                if (!sameLoc(want, got)) return QStringLiteral("find(%1) differs from looking through everything %2").arg(id, when);
            }
            // What was here once and no longer is, and what never was.
            for (int k = 0; k < 6 && !seen.isEmpty(); ++k) {
                const QString id = seen[pick(int(seen.size()))];
                if (!sameLoc(scanFor(*d, id), d->find(id))) return QStringLiteral("find(%1) differs for an item gone or moved %2").arg(id, when);
            }
            if (d->find(QStringLiteral("nothing-has-this-id")).item) return QStringLiteral("found an item no one has %1").arg(when);
            return QString();
        };
        for (int step = 0; step < 600; ++step) {
            const int op = pick(13);
            const QString when = QStringLiteral("after step %1 (operation %2)").arg(step).arg(op);
            switch (op) {
            case 0: case 1: case 2: randomList()->push_back(newItem()); break;                       // add
            case 3: { ItemList *l = randomList(); l->insert(l->begin(), newItem()); break; }          // add in front, which moves the rest
            case 4: { ItemList *l = randomList(); if (!l->empty()) l->erase(l->begin() + pick(int(l->size()))); break; }   // remove
            case 5: { ItemList *a = randomList(), *b = randomList(); if (!a->empty() && a != b) { const int i = pick(int(a->size())); ItemPtr it = (*a)[i]; a->erase(a->begin() + i); b->push_back(it); } break; }   // move
            case 6: { ItemList *l = randomList(); if (l->size() > 1) std::swap((*l)[pick(int(l->size()))], (*l)[pick(int(l->size()))]); break; }
            case 7: {   // group a few items of one list
                ItemList *l = randomList();
                if (l->size() >= 2) {
                    auto g = std::make_shared<GroupItem>();
                    const int n = 2 + pick(2);
                    for (int k = 0; k < n && l->size() > 1; ++k) { const int i = pick(int(l->size())); g->children.push_back((*l)[i]); l->erase(l->begin() + i); }
                    l->push_back(g);
                }
                break;
            }
            case 8: {   // ungroup
                ItemList *l = randomList();
                for (size_t i = 0; i < l->size(); ++i)
                    if ((*l)[i]->type() == ItemType::Group) {
                        ItemPtr g = (*l)[i];
                        l->erase(l->begin() + i);
                        const auto kids = static_cast<GroupItem *>(g.get())->children;
                        l->insert(l->begin() + i, kids.begin(), kids.end());
                        break;
                    }
                break;
            }
            case 9: { ItemList *l = randomList(); if (!l->empty()) (*l)[pick(int(l->size()))] = newItem(); break; }   // replaced by another
            case 10:
                if (pick(2) || d->pages.size() < 3) d->addPage(pick(int(d->pages.size()) + 1));
                else d->pages.removeAt(pick(int(d->pages.size())));
                break;
            case 11: if (d->pages.size() > 1) d->pages.swapItemsAt(pick(int(d->pages.size())), pick(int(d->pages.size()))); break;
            default: { const QJsonObject snapshot = d->toJson(); d->fromJson(snapshot); break; }   // undo and open both load again
            }
            const QString bad = check(when);
            QVERIFY2(bad.isEmpty(), qPrintable(bad));
        }
    }

    // Once the index has been made, finding an item that is there costs no scan.
    void documentFindDoesNotScanOnceIndexed()
    {
        auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 40);
        QStringList ids;
        for (auto &p : d->pages)
            for (int k = 0; k < 20; ++k) {
                p->items.push_back(Item::create(ItemType::Shape));
                ids << p->items.back()->id;
            }
        QVERIFY(d->find(ids.first()).item);
        const int before = d->findScansForTest();
        for (int round = 0; round < 5; ++round)
            for (const QString &id : ids) QVERIFY(d->find(id).item);
        QCOMPARE(d->findScansForTest(), before);
        // An item that comes later is found, and the ones before it still don't need a scan.
        d->pages[3]->items.push_back(Item::create(ItemType::Text));
        QVERIFY(d->find(d->pages[3]->items.back()->id).item);
        const int mid = d->findScansForTest();
        for (const QString &id : ids) QVERIFY(d->find(id).item);
        QCOMPARE(d->findScansForTest(), mid);
    }

    // After an edit that moves items along their page (one put in front of the rest), finding them costs a look at their page, not at every item,
    // and the index is not made again.
    void documentFindRepairsItemsThatShiftedWithoutScanning()
    {
        auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 30);
        QStringList ids;
        for (auto &p : d->pages)
            for (int k = 0; k < 40; ++k) {
                p->items.push_back(Item::create(ItemType::Shape));
                ids << p->items.back()->id;
            }
        QVERIFY(d->find(ids.first()).item);
        const int scans = d->findScansForTest(), indexed = d->findIndexingsForTest();
        for (int round = 0; round < 20; ++round) {
            auto &items = d->pages[round % 30]->items;
            items.insert(items.begin(), Item::create(ItemType::Shape));   // (everything on the page moves one place)
            for (const QString &id : ids) {
                const Document::Loc got = d->find(id);
                QVERIFY(got.item);
                QVERIFY(sameLoc(got, scanFor(*d, id)));
            }
        }
        QCOMPARE(d->findScansForTest(), scans);
        QCOMPARE(d->findIndexingsForTest(), indexed);
        // An item that went to another page is found by looking at every item, once, and the index isn't made again for it.
        ItemPtr moved = d->pages[3]->items.back();
        d->pages[3]->items.pop_back();
        d->pages[20]->items.push_back(moved);
        QVERIFY(sameLoc(d->find(moved->id), scanFor(*d, moved->id)));
        QCOMPARE(d->findScansForTest(), scans + 1);
        QCOMPARE(d->findIndexingsForTest(), indexed);
        QCOMPARE(d->find(moved->id).page, 20);
        QCOMPARE(d->findScansForTest(), scans + 1);
        // Groups: an item in a group that went to another page with it.
        auto group = std::make_shared<GroupItem>();
        auto inner = Item::create(ItemType::Text);
        group->children.push_back(inner);
        d->pages[5]->items.push_back(group);
        QCOMPARE(d->find(inner->id).page, 5);
        d->pages[5]->items.pop_back();
        d->pages[9]->items.insert(d->pages[9]->items.begin(), group);
        QVERIFY(sameLoc(d->find(inner->id), scanFor(*d, inner->id)));
        QCOMPARE(d->find(inner->id).page, 9);
        QVERIFY(sameLoc(d->find(group->id), scanFor(*d, group->id)));
    }

    // Two items with one id (a damaged file): the first, in the order of the pages, masters, and scratch area, and of the items on a page, a group's
    // within it, as looking at every item finds it.
    void documentFindReturnsTheFirstOfTwoItemsWithOneId()
    {
        auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);
        auto group = std::make_shared<GroupItem>();
        auto inside = Item::create(ItemType::Shape);
        group->children.push_back(inside);
        auto later = Item::create(ItemType::Shape);
        later->id = inside->id;
        d->pages[0]->items.push_back(group);
        d->pages[0]->items.push_back(later);
        auto onThePageAfter = Item::create(ItemType::Shape);
        onThePageAfter->id = inside->id;
        d->pages[1]->items.push_back(onThePageAfter);
        QVERIFY(sameLoc(d->find(inside->id), scanFor(*d, inside->id)));
        QCOMPARE(d->find(inside->id).item, inside.get());
        // (Again, with the index made.)
        QCOMPARE(d->find(inside->id).item, inside.get());
        const QJsonObject json = d->toJson();
        d->fromJson(json);
        QVERIFY(sameLoc(d->find(inside->id), scanFor(*d, inside->id)));
        QVERIFY(d->find(inside->id).parent != nullptr);
    }

    // The layout cache's key held the minute, so a long story was laid out again every minute it was
    // on screen. It holds it only for a story with a date or time field in it.
    void layoutDoesNotFollowTheClockWithoutAClockField()
    {
        auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 1);
        const QString sid = d->createStory(QStringLiteral("Just words, no field at all."));
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(72, 72, 300, 200);
        box->storyId = sid;
        d->pages[0]->items.push_back(box);
        LayoutCache cache;
        RenderOptions opt;
        opt.now = QDateTime(QDate(2026, 10, 16), QTime(12, 0));
        const StoryLayout *first = cache.textFrame(*d, *box, 1, opt).layout;
        QVERIFY(first);
        opt.now = QDateTime(QDate(2026, 10, 16), QTime(12, 1));
        QCOMPARE(cache.textFrame(*d, *box, 1, opt).layout, first);   // the same layout: not made again
        // Still made again for what does change it.
        d->storyDoc(sid)->setPlainText(QStringLiteral("Other words."));
        const StoryLayout *edited = cache.textFrame(*d, *box, 1, opt).layout;
        opt.now = QDateTime(QDate(2026, 10, 16), QTime(12, 2));
        QCOMPARE(cache.textFrame(*d, *box, 1, opt).layout, edited);
    }

    void layoutFollowsTheClockWithADateOrTimeField()
    {
        for (const char *kind : {"time", "date", "datetime", "time:HH:mm"}) {
            auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 1);
            const QString sid = d->createStory(QStringLiteral("Printed at "));
            QTextCursor c(d->storyDoc(sid));
            c.movePosition(QTextCursor::End);
            QTextCharFormat field;
            field.setProperty(tp::Field, QString::fromLatin1(kind));
            c.insertText(QString(QChar(0xFFFC)), field);
            auto box = std::make_shared<TextItem>();
            box->rect = QRectF(72, 72, 300, 200);
            box->storyId = sid;
            d->pages[0]->items.push_back(box);
            LayoutCache cache;
            RenderOptions opt;
            opt.now = QDateTime(QDate(2026, 10, 16), QTime(12, 0));
            const StoryLayout *a = cache.textFrame(*d, *box, 1, opt).layout;
            QVERIFY(a);
            QCOMPARE(cache.textFrame(*d, *box, 1, opt).layout, a);   // the same minute: kept
            opt.now = QDateTime(QDate(2026, 10, 17), QTime(13, 5));
            const StoryLayout *b = cache.textFrame(*d, *box, 1, opt).layout;
            QVERIFY2(b != a, kind);
            QCOMPARE(cache.textFrame(*d, *box, 1, opt).layout, b);
        }
    }

    // A note's own text is laid out with the story's clock, so a date in a note counts too.
    void layoutFollowsTheClockWithADateInANote()
    {
        auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 1);
        const QString note = d->createStory(QStringLiteral("As of "));
        QTextCursor nc(d->storyDoc(note));
        nc.movePosition(QTextCursor::End);
        QTextCharFormat date;
        date.setProperty(tp::Field, QStringLiteral("date"));
        nc.insertText(QString(QChar(0xFFFC)), date);
        const QString sid = d->createStory(QStringLiteral("Body text"));
        QTextCursor c(d->storyDoc(sid));
        c.movePosition(QTextCursor::End);
        QTextCharFormat ref;
        ref.setProperty(tp::Field, QStringLiteral("footnote:") + note);
        c.insertText(QString(QChar(0xFFFC)), ref);
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(72, 72, 300, 200);
        box->storyId = sid;
        d->pages[0]->items.push_back(box);
        LayoutCache cache;
        RenderOptions opt;
        opt.now = QDateTime(QDate(2026, 10, 16), QTime(12, 0));
        const StoryLayout *a = cache.textFrame(*d, *box, 1, opt).layout;
        opt.now = QDateTime(QDate(2026, 10, 17), QTime(13, 5));
        QVERIFY(cache.textFrame(*d, *box, 1, opt).layout != a);
    }

    // A layout shows more than the story's own text: business information, mail merge values, and the text of its notes, which no edit of the
    // story touches. (The minute in the layout's key used to bring these up to date, by laying everything out again every minute.) What the
    // fields show, and the notes' text, are in the key now: a change to one lays the story out again, and the minute does not.
    void layoutFollowsWhatItsFieldsShow()
    {
        auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 1);
        d->business().name = QStringLiteral("Old Name LLC");
        d->merge.fields = {QStringLiteral("Name")};
        d->merge.rows = {{QStringLiteral("Ann")}};
        d->merge.include = {true};
        const QString note = d->createStory(QStringLiteral("old footnote text"));
        const QString sid = d->createStory(QStringLiteral("Body:"));
        QTextCursor c(d->storyDoc(sid));
        c.movePosition(QTextCursor::End);
        for (const QString &code : {QStringLiteral("biz:name"), QStringLiteral("merge:Name"), QStringLiteral("footnote:") + note}) {
            QTextCharFormat field;
            field.setProperty(tp::Field, code);
            c.insertText(QString(QChar(0xFFFC)), field);
            c.insertText(QStringLiteral(" "));
        }
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(72, 72, 300, 200);
        box->storyId = sid;
        box->hyphenate = false;
        d->pages[0]->items.push_back(box);
        LayoutCache cache;
        RenderOptions opt;
        opt.mergeRecord = 0;
        opt.now = QDateTime(QDate(2026, 10, 16), QTime(12, 0));
        auto shown = [&] {
            const StoryLayout *lay = cache.textFrame(*d, *box, 1, opt).layout;
            QString all;
            for (const auto &li : lay->lineInfo(0)) all += li.text + QLatin1Char('|');
            for (const auto &n : lay->notes())
                if (n.layout)
                    for (const auto &li : n.layout->lineInfo(0)) all += QStringLiteral("[note: ") + li.text + QLatin1Char(']');
            return all;
        };
        auto layout = [&] { return cache.textFrame(*d, *box, 1, opt).layout; };
        const StoryLayout *first = layout();
        QVERIFY(shown().contains(QStringLiteral("Old Name LLC")) && shown().contains(QStringLiteral("Ann")) && shown().contains(QStringLiteral("old footnote")));
        // A minute later: the same layout (none of it is the time).
        opt.now = QDateTime(QDate(2026, 10, 16), QTime(12, 1));
        QCOMPARE(layout(), first);
        // Business Information edited.
        d->business().name = QStringLiteral("New Name Inc");
        const StoryLayout *second = layout();
        QVERIFY(second != first);
        QVERIFY2(shown().contains(QStringLiteral("New Name Inc")), qPrintable(shown()));
        QCOMPARE(layout(), second);
        // A value of the merge list edited.
        d->merge.rows[0][0] = QStringLiteral("Bob");
        const StoryLayout *third = layout();
        QVERIFY(third != second);
        QVERIFY2(shown().contains(QStringLiteral("Bob")), qPrintable(shown()));
        QCOMPARE(layout(), third);
        // The text of a note edited (as Edit Note does: the story that refers to it is not touched).
        d->storyDoc(note)->setPlainText(QStringLiteral("new footnote text"));
        const StoryLayout *fourth = layout();
        QVERIFY(fourth != third);
        QVERIFY2(shown().contains(QStringLiteral("new footnote")), qPrintable(shown()));
        QCOMPARE(layout(), fourth);
        // Another record of the list.
        d->merge.rows << QStringList{QStringLiteral("Cy")};
        d->merge.include << true;
        opt.mergeRecord = 1;
        QVERIFY(layout() != fourth);
        QVERIFY2(shown().contains(QStringLiteral("Cy")), qPrintable(shown()));
    }

    // A date is laid out again when the day changes, not every minute of it; a time, when the minute does.
    void layoutIsKeptWhileADateShowsTheSameDay()
    {
        auto d = Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 1);
        const QString sid = d->createStory(QStringLiteral("Today is "));
        QTextCursor c(d->storyDoc(sid));
        c.movePosition(QTextCursor::End);
        QTextCharFormat date;
        date.setProperty(tp::Field, QStringLiteral("date"));
        c.insertText(QString(QChar(0xFFFC)), date);
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(72, 72, 300, 200);
        box->storyId = sid;
        d->pages[0]->items.push_back(box);
        LayoutCache cache;
        RenderOptions opt;
        opt.now = QDateTime(QDate(2026, 10, 16), QTime(12, 0));
        const StoryLayout *a = cache.textFrame(*d, *box, 1, opt).layout;
        opt.now = QDateTime(QDate(2026, 10, 16), QTime(23, 59));
        QCOMPARE(cache.textFrame(*d, *box, 1, opt).layout, a);
        opt.now = QDateTime(QDate(2026, 10, 17), QTime(0, 0));
        QVERIFY(cache.textFrame(*d, *box, 1, opt).layout != a);
    }

    // ---- the text engine, for laying out a page at a time ----

    // Each line says where its text is in the document, and the lines of a paragraph follow on from one another.
    void lineInfoGivesDocumentPositions()
    {
        auto doc = makeStory(5, 11);
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(220, 160), plainFrame(220, 400)}, LayoutEnv());
        const QString text = doc->toPlainText();
        int lines = 0, previousEnd = -1, previousBlock = -1;
        for (int f = 0; f < lay.frameCount(); ++f)
            for (const auto &li : lay.lineInfo(f)) {
                ++lines;
                QVERIFY(li.docEnd > li.docPos);
                QCOMPARE(text.mid(li.docPos, li.docEnd - li.docPos), li.text);
                // The next line of a paragraph starts where the one before ended; a new paragraph, where it starts.
                if (li.docStart == previousBlock) QCOMPARE(li.docPos, previousEnd);
                else QCOMPARE(li.docPos, li.docStart);
                previousEnd = li.docEnd;
                previousBlock = li.docStart;
            }
        QVERIFY(lines > 10);
        // The brief list has the same lines, in the same order, past the last frame too.
        const auto briefs = lay.lineBriefs();
        QCOMPARE(int(briefs.size()), lines);
        int at = 0;
        for (int f = 0; f < lay.frameCount(); ++f)
            for (const auto &li : lay.lineInfo(f)) {
                QCOMPARE(briefs[at].frame, f);
                QCOMPARE(briefs[at].pos, li.docPos);
                QCOMPARE(briefs[at].end, li.docEnd);
                QCOMPARE(briefs[at].block, li.docStart);
                QCOMPARE(briefs[at].rect, li.rect);
                ++at;
            }
    }

    // A stretch from a paragraph on is laid out as that paragraph and the ones after it would be as a story of their own.
    void spanFromAParagraphIsTheSameAsAStoryFromThere()
    {
        auto whole = makeStory(40, 5);
        const QTextBlock start = whole->findBlockByNumber(25);
        auto tail = makeStory(40, 5);
        QTextCursor cut(tail.get());
        cut.setPosition(start.position(), QTextCursor::KeepAnchor);
        cut.setPosition(0, QTextCursor::MoveAnchor);
        cut.setPosition(start.position(), QTextCursor::KeepAnchor);
        cut.removeSelectedText();
        const QVector<FrameSpec> frames{plainFrame(260, 180), plainFrame(260, 180), plainFrame(260, 600)};
        StoryLayout own, span;
        own.build(tail.get(), frames, LayoutEnv());
        StoryLayout::Span s;
        s.from = start.position() + 7;   // inside the paragraph: it is laid out whole
        span.build(whole.get(), frames, LayoutEnv(), s);
        const auto a = span.lineBriefs(), b = own.lineBriefs();
        QCOMPARE(a.size(), b.size());
        QVERIFY(a.size() > 30);
        for (int i = 0; i < a.size(); ++i) QVERIFY2(sameBrief(a[i], b[i], 0, start.position()), qPrintable(QStringLiteral("line %1").arg(i)));
    }

    // A paragraph that began on the page before goes on at the top of the next page with the lines it has left,
    // set as in a layout that runs on from the first page into the next.
    void spanContinuesAParagraphFromTheFrameBefore()
    {
        auto doc = makeStory(30, 21, [](int i, QTextBlockFormat &bf) { if (i % 4 == 0) bf.setTextIndent(18); });
        const QVector<FrameSpec> pages{plainFrame(300, 210), plainFrame(300, 210), plainFrame(300, 210), plainFrame(300, 210)};
        StoryLayout whole;
        whole.build(doc.get(), pages, LayoutEnv());
        const auto lines = whole.lineBriefs();
        // A page that starts in the middle of a paragraph.
        int frame = -1, taken = 0;
        for (int i = 1; i < lines.size() && frame < 0; ++i)
            if (lines[i].frame == lines[i - 1].frame + 1 && lines[i].line > 0 && lines[i].frame < 3) {
                frame = lines[i].frame;
                taken = lines[i].line;   // the lines of the paragraph before it
            }
        QVERIFY2(frame >= 1, "no paragraph runs from one page to the next");
        int firstPos = -1;
        for (const auto &l : lines)
            if (l.frame == frame) { firstPos = l.pos; break; }
        QVERIFY(firstPos > 0);
        // The page it left is stood in for by a frame as wide, and tall enough.
        QVector<FrameSpec> rest{plainFrame(300, 100000)};
        for (int f = frame; f < pages.size(); ++f) rest << pages[f];
        StoryLayout run;
        StoryLayout::Span span;
        span.from = firstPos;
        span.firstBreakAfter = taken;
        run.build(doc.get(), rest, LayoutEnv(), span);
        const auto got = run.lineBriefs();
        int compared = 0;
        QCOMPARE(got.first().frame, 0);
        int g = 0;
        while (g < got.size() && got[g].frame == 0) ++g;   // the lines that stood in the page before
        QCOMPARE(g, taken);
        for (const auto &l : lines) {
            if (l.frame < frame) continue;
            QVERIFY2(g < got.size(), "the run has fewer lines");
            QVERIFY2(sameBrief(got[g], l, 1 - frame), qPrintable(QStringLiteral("line %1 of the page").arg(compared)));
            ++g;
            ++compared;
        }
        QVERIFY(compared > 20);
    }

    // Laying out ends after the paragraph that reaches the last frame, and what is laid out before it is the same.
    void spanStopsAtAFrame()
    {
        auto doc = makeStory(200, 8);
        const QVector<FrameSpec> two{plainFrame(280, 230), plainFrame(280, 100000)};
        StoryLayout whole, run;
        whole.build(doc.get(), {plainFrame(280, 230), plainFrame(280, 230), plainFrame(280, 230)}, LayoutEnv());
        StoryLayout::Span span;
        span.stopAtFrame = 1;
        run.build(doc.get(), two, LayoutEnv(), span);
        const auto lines = run.lineBriefs();
        QVERIFY(!lines.isEmpty());
        // Only the paragraphs on the page and the one that runs over: far fewer than the story's.
        QVERIFY2(lines.size() < 120, qPrintable(QString::number(lines.size())));
        int over = -1;
        for (const auto &l : lines)
            if (l.frame == 1) { if (over < 0) over = l.block; QCOMPARE(l.block, over); }
        QVERIFY(over >= 0);
        QCOMPARE(lines.last().block, over);
        // The page is the one the whole layout has.
        const auto all = whole.lineBriefs();
        int n = 0;
        for (const auto &l : all) {
            if (l.frame != 0) break;
            QVERIFY(sameBrief(lines[n], l));
            ++n;
        }
        QVERIFY(n > 10);
        // Where the next page starts is where the whole layout starts its second page.
        int second = -1;
        for (const auto &l : all) if (l.frame == 1) { second = l.pos; break; }
        for (const auto &l : lines) if (l.frame == 1) { QCOMPARE(l.pos, second); break; }
    }

    // The rest of a paragraph that began on the frame before, laid out from where it goes on, has the lines it has there: without a bullet or a first-line
    // indent, and its lines before are not laid out again.
    void spanContinuesAParagraphWithoutItsEarlierLines()
    {
        auto doc = makeStory(10, 91, [](int i, QTextBlockFormat &bf) { bf.setTextIndent(i % 2 ? 20 : 0); bf.setAlignment(Qt::AlignJustify); }, 30, 140);
        {
            QTextCursor c(doc.get());
            c.setPosition(0);
            c.setPosition(doc->findBlockByNumber(2).position() - 1, QTextCursor::KeepAnchor);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            lf.setIndent(1);
            c.createList(lf);
        }
        const QVector<FrameSpec> pages{plainFrame(300, 200), plainFrame(300, 200), plainFrame(300, 200), plainFrame(300, 2000)};
        StoryLayout whole;
        whole.build(doc.get(), pages, LayoutEnv());
        const auto lines = whole.lineBriefs();
        int checked = 0;
        for (int frame = 1; frame < 3; ++frame) {
            int at = -1;
            for (int i = 0; i < lines.size() && at < 0; ++i)
                if (lines[i].frame == frame) at = i;
            if (at <= 0 || lines[at].line == 0) continue;   // (a frame that begins with a paragraph)
            QVector<FrameSpec> rest;
            for (int f = frame; f < pages.size(); ++f) rest << pages[f];
            StoryLayout run;
            StoryLayout::Span span;
            span.from = lines[at].pos;
            span.continued = true;
            run.build(doc.get(), rest, LayoutEnv(), span);
            const auto got = run.lineBriefs();
            const int taken = lines[at].line;
            for (int k = 0; at + k < lines.size() && k < got.size(); ++k) {
                const auto &want = lines[at + k];
                const bool same = want.block == lines[at].block;
                QVERIFY2(got[k].frame == want.frame - frame && got[k].pos == want.pos && got[k].end == want.end && got[k].line == (same ? want.line - taken : want.line) &&
                             std::abs(got[k].rect.top() - want.rect.top()) < 0.01 && std::abs(got[k].rect.left() - want.rect.left()) < 0.01 && std::abs(got[k].rect.width() - want.rect.width()) < 0.01,
                         qPrintable(QStringLiteral("frame %1, line %2 of the rest").arg(frame).arg(k)));
            }
            ++checked;
        }
        QVERIFY2(checked > 0, "no paragraph went from one frame to the next");
    }

    // A window cuts a paragraph that is longer than the lines wanted need: its lines to the cut are the lines it has, the last is marked, and laying out
    // stops; if the frames aren't full by then the window was too small.
    void spanWindowCutsAHugeParagraph()
    {
        QTextDocument doc;
        {
            QTextCursor c(&doc);
            QString text;
            std::mt19937 rng(5);
            while (text.size() < 300000) text += QStringLiteral("word%1 ").arg(rng() % 1000);
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            c.insertText(text, cf);
            c.insertBlock();
            c.insertText(QStringLiteral("a paragraph after"), cf);
        }
        const QVector<FrameSpec> two{plainFrame(300, 200), plainFrame(300, 5000)};
        StoryLayout whole;
        whole.build(&doc, {plainFrame(300, 200), plainFrame(300, 100000)}, LayoutEnv());
        const auto all = whole.lineBriefs();
        StoryLayout run;
        StoryLayout::Span span;
        span.window = 4000;
        span.stopAtFrame = 1;
        run.build(&doc, two, LayoutEnv(), span);
        QVERIFY(!run.windowShort());
        const auto lines = run.lineBriefs();
        // (How many lines a window holds depends on the system's letter sizes; what matters is that they equal the whole layout's.)
        QVERIFY2(lines.size() > 5 && lines.size() < 400, qPrintable(QString::number(lines.size())));
        for (const auto &l : lines) QVERIFY(l.cut);
        // Every line but the last is the line the whole paragraph has, and nothing after the paragraph is laid out.
        for (int i = 0; i + 1 < lines.size(); ++i)
            QVERIFY2(lines[i].frame == all[i].frame && lines[i].pos == all[i].pos && lines[i].end == all[i].end && std::abs(lines[i].rect.top() - all[i].rect.top()) < 0.01,
                     qPrintable(QString::number(i)));
        QVERIFY2(lines.last().end <= 4000 + 1, qPrintable(QStringLiteral("last end %1, pos %2, lines %3").arg(lines.last().end).arg(lines.last().pos).arg(lines.size())));
        // A frame the window can't fill.
        StoryLayout big;
        big.build(&doc, {plainFrame(300, 100000), plainFrame(300, 100000)}, LayoutEnv(), span);
        QVERIFY(big.windowShort());
        // No window: whole paragraphs.
        StoryLayout none;
        StoryLayout::Span uncut;
        uncut.stopAtFrame = 1;
        none.build(&doc, two, LayoutEnv(), uncut);
        QVERIFY(!none.windowShort());
        QVERIFY(none.lineBriefs().size() > 1000);
        QVERIFY(!none.lineBriefs().first().cut);
    }

    // Without the rules, widow and orphan control and keeping paragraphs together leave the lines where they fall.
    void spanWithoutRulesIgnoresKeepsAndWidows()
    {
        auto asked = makeStory(40, 14, [](int, QTextBlockFormat &bf) {
            bf.setProperty(tp::WidowControl, true);
            bf.setProperty(tp::KeepTogether, true);
            bf.setProperty(tp::KeepWithNext, true);
        });
        auto plain = makeStory(40, 14);
        bool differs = false;
        for (double h = 150; h < 240; h += 7) {
            const QVector<FrameSpec> frames{plainFrame(260, h), plainFrame(260, h), plainFrame(260, h), plainFrame(260, 100000)};
            StoryLayout with, without, natural;
            with.build(asked.get(), frames, LayoutEnv());
            StoryLayout::Span off;
            off.keepRules = false;
            without.build(asked.get(), frames, LayoutEnv(), off);
            natural.build(plain.get(), frames, LayoutEnv());
            const auto a = without.lineBriefs(), b = natural.lineBriefs(), c = with.lineBriefs();
            QCOMPARE(a.size(), b.size());
            for (int i = 0; i < a.size(); ++i) QVERIFY(sameBrief(a[i], b[i]));
            for (int i = 0; i < std::min(a.size(), c.size()) && !differs; ++i) differs |= !sameBrief(a[i], c[i]);
        }
        QVERIFY2(differs, "the rules never moved a line, so this proves nothing");
    }

    // The space before the first paragraph is kept at the top, or left out when the page only came to it.
    void spanCanLeaveOutTheSpaceBeforeTheFirstParagraph()
    {
        auto doc = makeStory(3, 3, [](int, QTextBlockFormat &bf) { bf.setTopMargin(20); });
        StoryLayout kept, left;
        kept.build(doc.get(), {plainFrame(260, 400)}, LayoutEnv());
        StoryLayout::Span span;
        span.spaceBeforeFirst = false;
        left.build(doc.get(), {plainFrame(260, 400)}, LayoutEnv(), span);
        const auto a = kept.lineBriefs(), b = left.lineBriefs();
        QCOMPARE(a.size(), b.size());
        QVERIFY(std::abs(a[0].rect.top() - 20) < 0.01);
        QVERIFY(std::abs(b[0].rect.top()) < 0.01);
        // (Only the first paragraph's: the lines after it keep their space before.)
        const auto second = std::find_if(a.begin(), a.end(), [](const auto &l) { return l.block > 0; }) - a.begin();
        QVERIFY(std::abs((b[second].rect.top() - b[second - 1].rect.bottom()) - (a[second].rect.top() - a[second - 1].rect.bottom())) < 0.01);
    }

    // A span can end before a paragraph: the paragraphs from `end` on are not laid out (a table's cells are laid out apart from the text around it).
    void spanEndsBeforeTheParagraphAtItsEnd()
    {
        auto doc = makeStory(12, 7);
        const QTextBlock stop = doc->findBlockByNumber(8), from = doc->findBlockByNumber(3);
        StoryLayout whole, part, one;
        const QVector<FrameSpec> frames{plainFrame(260, 2000)};
        StoryLayout::Span s;
        s.from = from.position();
        whole.build(doc.get(), frames, LayoutEnv(), s);
        s.end = stop.position();
        part.build(doc.get(), frames, LayoutEnv(), s);
        const auto all = whole.lineBriefs(), some = part.lineBriefs();
        QVERIFY(some.size() > 10 && some.size() < all.size());
        for (int i = 0; i < some.size(); ++i) {
            QVERIFY2(sameBrief(some[i], all[i], 0, 0), qPrintable(QStringLiteral("line %1").arg(i)));
            QVERIFY(some[i].block < stop.position());
        }
        QCOMPARE(all[some.size()].block, stop.position());   // (the next line of the whole is the first of the paragraph it stopped before)
        // A span that ends in the paragraph it starts in has that paragraph's lines, and one that ends at its start has none.
        s.end = doc->findBlockByNumber(4).position();
        one.build(doc.get(), frames, LayoutEnv(), s);
        for (const auto &l : one.lineBriefs()) QCOMPARE(l.block, from.position());
        QVERIFY(!one.lineBriefs().isEmpty());
        s.end = from.position();
        one.build(doc.get(), frames, LayoutEnv(), s);
        QVERIFY(one.lineBriefs().isEmpty());
        // (It lays out no endnotes, as one that stops at a frame does not: the rest of the story is not in it.)
        QVERIFY(!one.overflow());
    }

    // A footnote cut at the foot of a page goes on at the foot of the next, whether the next page is laid out with the
    // first or by itself with the note given as carried over.
    void spanCarriesAFootnoteOverFromThePageBefore()
    {
        int compared = 0;
        for (double height : {110.0, 125.0, 140.0, 155.0, 170.0, 185.0})
            for (unsigned seed : {9u, 10u, 11u}) {
                bool cut = false;
                const QString bad = carriedFootnoteCheck(height, seed, &cut);
                // A case whose text happens to fit one page tests nothing (it depends on how large this system sets the
                // letters: smaller on macOS); it's skipped, and enough cases must still carry a note over (below).
                if (bad == QLatin1String("no second page")) continue;
                QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("height %1, seed %2: %3").arg(height).arg(seed).arg(bad)));
                compared += cut;
            }
        QVERIFY2(compared >= 6, qPrintable(QString::number(compared)));
    }

    // ---- the paginator ----

    // The pages are the lines the whole story gets when it is laid out in one run of frames (where that is right: the story is short, and nothing
    // asks more of a page than the text engine gives).
    void paginatorMatchesTheOneShotLayout()
    {
        for (int columns : {1, 2})
            for (unsigned seed : {1u, 2u, 3u, 4u}) {
                FlowDocument fd;
                fd.sections[0].columns = columns;
                fd.widowLines = fd.orphanLines = 1;
                // (Keeping with the next paragraph, and widow control, each on its own: asking both of a heading and the paragraph after it
                // lets the text engine leave a heading at the foot of a page, where the paginator moves it on.)
                fillStory(fd.story(), 150, seed, [seed](int i, QTextBlockFormat &bf) {
                    if (i % 9 == 0) {
                        if (seed % 2) { bf.setProperty(tp::KeepWithNext, true); bf.setProperty(tp::KeepTogether, true); }
                    } else if (seed % 2 == 0) {
                        bf.setProperty(tp::WidowControl, true);
                    }
                });
                Paginator pg(&fd);
                QVERIFY(pg.pageCount() > 6);
                const QString bad = differenceFromOneShot(fd, pg);
                QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1 columns, seed %2: %3").arg(columns).arg(seed).arg(bad)));
            }
    }

    // A document with one page is one page, and an empty one is too.
    void paginatorHandlesTinyDocuments()
    {
        FlowDocument fd;
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 1);
        QVERIFY(pg.complete());
        fillStory(fd.story(), 2, 4);
        QCOMPARE(pg.pageCount(), 1);
        QCOMPARE(pg.lines(0).first().pos, 0);
    }

    // The pages that cross a paragraph leave two lines of it on each side (the text engine keeps two lines but for a paragraph of three).
    void paginatorKeepsWidowsAndOrphans()
    {
        int splits = 0;
        for (unsigned seed : {5u, 6u, 7u, 8u, 9u, 10u})
            for (double margin : {72.0, 79.0, 85.0}) {
                FlowDocument fd;
                fd.sections[0].margins = QMarginsF(72, margin, 72, margin);
                fillStory(fd.story(), 120, seed, [](int, QTextBlockFormat &bf) { bf.setProperty(tp::WidowControl, true); }, 24, 140);
                Paginator pg(&fd);
                const QString bad = splitsLeaveLines(fd, pg, 2);
                QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("seed %1, margin %2: %3").arg(seed).arg(margin).arg(bad)));
                for (int k = 0; k + 1 < pg.pageCount(); ++k) splits += pg.lines(k).last().block == pg.lines(k + 1).first().block;
            }
        QVERIFY2(splits > 10, "the stories hardly split a paragraph, so the test proves little");
    }

    // More lines kept than the text engine does.
    void paginatorKeepsMoreLinesWhenAsked()
    {
        for (unsigned seed : {5u, 6u, 7u}) {
            FlowDocument fd;
            fd.widowLines = fd.orphanLines = 3;
            fillStory(fd.story(), 120, seed, [](int, QTextBlockFormat &bf) { bf.setProperty(tp::WidowControl, true); }, 24, 140);
            Paginator pg(&fd);
            const QString bad = splitsLeaveLines(fd, pg, 3);
            QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("seed %1: %2").arg(seed).arg(bad)));
        }
    }

    // A paragraph that is kept with the next one is not the last on a page, unless it is all the page holds.
    void paginatorKeepsParagraphsWithTheNext()
    {
        int moved = 0;
        for (unsigned seed : {1u, 2u, 3u, 4u}) {
            FlowDocument fd;
            fillStory(fd.story(), 160, seed, [](int i, QTextBlockFormat &bf) {
                bf.setProperty(tp::WidowControl, true);
                if (i % 5 == 4) { bf.setProperty(tp::KeepWithNext, true); bf.setProperty(tp::KeepTogether, true); }
            }, 8, 70);
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            for (int k = 0; k + 1 < pages; ++k) {
                const auto lines = pg.lines(k);
                if (!formatAt(fd, lines.last().block).boolProperty(tp::KeepWithNext)) continue;
                QVERIFY2(lines.first().block == lines.last().block, qPrintable(QStringLiteral("page %1 ends with a paragraph kept with the next").arg(k)));
            }
            // (A page that ends just before a paragraph kept with the next that was moved to the next page.)
            for (int k = 1; k < pages; ++k) moved += formatAt(fd, pg.lines(k).first().block).boolProperty(tp::KeepWithNext) && pg.lines(k).first().line == 0;
        }
        QVERIFY2(moved > 3, "no paragraph was ever moved to keep it with the next");
    }

    // A page break before a paragraph starts a page with it, and one after ends a page; no page comes out empty.
    void paginatorBreaksPagesWhereAsked()
    {
        FlowDocument fd;
        fillStory(fd.story(), 60, 12, [](int i, QTextBlockFormat &bf) {
            if (i == 3 || i == 20 || i == 21) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
            if (i == 30 || i == 45) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
        });
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        auto blockNumber = [&](int position) { return fd.story()->findBlock(position).blockNumber(); };
        QSet<int> startsWith, endsWith;
        for (int k = 0; k < pages; ++k) {
            const auto lines = pg.lines(k);
            QVERIFY2(!lines.isEmpty(), "an empty page");
            if (lines.first().line == 0) startsWith.insert(blockNumber(lines.first().block));
            if (lines.last().line == lines.last().lines - 1) endsWith.insert(blockNumber(lines.last().block));
        }
        for (int block : {3, 20, 21}) QVERIFY2(startsWith.contains(block), qPrintable(QStringLiteral("a page begins with paragraph %1").arg(block)));
        for (int block : {30, 45}) QVERIFY2(endsWith.contains(block), qPrintable(QStringLiteral("a page ends with paragraph %1").arg(block)));
        QVERIFY(startsWith.contains(31) && startsWith.contains(46));
        // Paragraphs 20 and 21 each have a page; 21's does not follow an empty one.
        QVERIFY(pg.page(pg.pageOfPosition(fd.story()->findBlockByNumber(21).position())).start.pos == fd.story()->findBlockByNumber(21).position());
        QVERIFY(pg.page(pg.pageOfPosition(fd.story()->findBlockByNumber(21).position())).start.hard);
    }

    // The page break character (flow::kPageBreak) in a paragraph ends the page after the line that holds it; the rest of the paragraph goes on
    // the next page. The paragraph stays one block: that it goes on is only that its next line is its line 1.
    void aPageBreakCharacterEndsThePageAfterItsLine()
    {
        FlowDocument fd;
        QTextCursor c(fd.story());
        c.insertText(QStringLiteral("first words ") + flow::kPageBreak + QStringLiteral("second words"));
        c.insertBlock();
        c.insertText(QStringLiteral("a paragraph after"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 2);
        const auto first = pg.lines(0), second = pg.lines(1);
        QCOMPARE(first.size(), 1);
        QCOMPARE(first[0].pos, 0);
        QCOMPARE(first[0].end, 13);                 // (the line ends with the break)
        QCOMPARE(second.size(), 2);
        QCOMPARE(second[0].pos, 13);
        QCOMPARE(second[0].line, 1);                // the same paragraph's next line
        QCOMPARE(second[0].block, 0);
        QCOMPARE(second[1].block, fd.story()->findBlockByNumber(1).position());
        QVERIFY(pg.page(1).start.hard);
        QCOMPARE(pg.page(1).start.skip, 1);
        auto before = pg.materialize(0), after = pg.materialize(1);
        QVERIFY(before && after);
        QVERIFY(before->continuesOnAfter && !before->continuesFromBefore);
        QVERIFY(after->continuesFromBefore);
        QCOMPARE(after->storyFrom, 13);
    }

    // A paragraph that begins with a page break goes on at the next page because of the break, not for want of room: no rule of widows or orphans moves
    // its first line (the break's) to the next frame, though it is left alone there.
    void aBreakAtTheStartOfAParagraphIsNotAnOrphan()
    {
        const double single = (1825 + 443 + 87) / 2048.0 * 12;
        auto lines = [&](bool withBreak) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            QTextBlockFormat bf;
            bf.setProperty(tp::WidowControl, true);
            for (int i = 0; i < 4; ++i) {
                if (i == 0) c.setBlockFormat(bf); else c.insertBlock(bf);
                c.insertText(QStringLiteral("filler %1").arg(i), run);
            }
            c.insertBlock(bf);
            c.insertText(QString(withBreak ? flow::kPageBreak : QChar(0x2028)) + QStringLiteral("after the break"), run);
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout lay;
            // Room for the four lines and a few points: none for a line of the paragraph, but the break's takes none.
            lay.build(&doc, {plainFrame(300, 4 * single + 3), plainFrame(300, 500)}, env);
            return lay.lineBriefs();
        };
        const auto broken = lines(true), line = lines(false);
        QCOMPARE(broken.size(), 6);
        QCOMPARE(broken[4].frame, 0);   // the break's line stays where it is
        QCOMPARE(broken[5].frame, 1);
        // (A line break that is only a line break leaves a paragraph's one line at the foot of a page as an orphan.)
        QCOMPARE(line[4].frame, 1);
        QCOMPARE(line[5].frame, 1);
        // The break's line takes no room, so that it is at the foot of the page even where a line would not fit there.
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        bf.setTopMargin(6);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("filler"), run);
        c.insertBlock(bf);
        c.insertText(QString(flow::kPageBreak) + QStringLiteral("after the break"), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(300, single + 9), plainFrame(300, 500)}, env);
        const auto tight = lay.lineBriefs();
        QCOMPARE(tight.size(), 3);
        QCOMPARE(tight[1].frame, 0);
        QVERIFY(tight[1].rect.height() < 0.01);
        QCOMPARE(tight[2].frame, 1);
    }

    // The text after a page break inside a paragraph is at the top of the next page with the paragraph's space before again (as a word processor sets it),
    // and a paragraph that goes on onto a page for want of room has none there.
    void aBreakInsideAParagraphKeepsItsSpaceBeforeOnTheNextPage()
    {
        FlowDocument fd;
        QTextCursor c(fd.story());
        QTextBlockFormat bf;
        bf.setTopMargin(20);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("first words ") + flow::kPageBreak + QStringLiteral("second words"));
        c.insertBlock(bf);
        c.insertText(QStringLiteral("a paragraph after"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 2);
        QVERIFY(std::abs(pg.lines(0)[0].rect.top() - 20) < 0.01);
        const auto second = pg.lines(1);
        QCOMPARE(second[0].line, 1);
        QVERIFY2(std::abs(second[0].rect.top() - 20) < 0.01, qPrintable(QString::number(second[0].rect.top())));
        QVERIFY(std::abs(second[1].rect.top() - (second[0].rect.bottom() + 20)) < 0.01);
        // The page made for painting has the same lines.
        LayoutCache cache;
        QVERIFY2(materializedDiffers(fd, pg, 1, cache).isEmpty(), qPrintable(materializedDiffers(fd, pg, 1, cache)));
        // A page that a paragraph runs onto for want of room has no space before its rest.
        FlowDocument run;
        QTextCursor r(run.story());
        r.setBlockFormat(bf);
        r.insertText(QString(12000, QLatin1Char('x')).replace(QRegularExpression(QStringLiteral("(x{7})")), QStringLiteral("\\1 ")));
        Paginator rp(&run);
        QVERIFY(rp.pageCount() > 1);
        QVERIFY(!rp.page(1).start.hard);
        QVERIFY(std::abs(rp.lines(1)[0].rect.top()) < 0.01);
    }

    // A break at the end of a paragraph leaves the next paragraph the top of the next page, with no empty line before it; at the end of the
    // last paragraph, it makes no page of its own.
    void aPageBreakAtTheEndOfAParagraphMakesNoEmptyLine()
    {
        FlowDocument fd;
        QTextCursor c(fd.story());
        c.insertText(QStringLiteral("ends with a break") + flow::kPageBreak);
        c.insertBlock();
        c.insertText(QStringLiteral("next"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 2);
        for (const auto &l : pg.lines(0)) QCOMPARE(l.block, 0);
        QCOMPARE(pg.lines(1).size(), 1);
        QCOMPARE(pg.lines(1)[0].block, fd.story()->findBlockByNumber(1).position());
        QCOMPARE(pg.lines(1)[0].line, 0);
        // The last paragraph ending in a break: the text has no more pages.
        FlowDocument last;
        QTextCursor lc(last.story());
        lc.insertText(QStringLiteral("only a break") + flow::kPageBreak);
        Paginator lp(&last);
        QCOMPARE(lp.pageCount(), 1);
    }

    // Where the paragraph mark follows a break to the next page (FlowDocument::markFollowsBreak), a paragraph that ends with a break begins the page after
    // it with an empty line, its mark's, and the next paragraph comes after that; the break at the end of the last paragraph makes that page too.
    void theMarkAfterABreakGoesToTheNextPageWhenTheDocumentSaysSo()
    {
        FlowDocument fd;
        fd.markFollowsBreak = true;
        QTextCursor c(fd.story());
        c.insertText(QStringLiteral("ends with a break") + flow::kPageBreak);
        c.insertBlock();
        c.insertText(QStringLiteral("next"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 2);
        for (const auto &l : pg.lines(0)) QCOMPARE(l.block, 0);
        const auto second = pg.lines(1);
        QCOMPARE(second.size(), 2);
        QCOMPARE(second[0].block, 0);               // (the empty rest of the first paragraph)
        QCOMPARE(second[0].line, 1);
        QCOMPARE(second[0].pos, second[0].end);
        QCOMPARE(second[1].block, fd.story()->findBlockByNumber(1).position());
        QVERIFY(std::abs(second[0].rect.top()) < 0.01);
        QVERIFY(second[1].rect.top() >= second[0].rect.bottom() - 0.01);
        QVERIFY(pg.page(1).start.hard);
        // The last paragraph ending in a break: the page after it has the mark.
        FlowDocument last;
        last.markFollowsBreak = true;
        QTextCursor lc(last.story());
        lc.insertText(QStringLiteral("only a break") + flow::kPageBreak);
        Paginator lp(&last);
        QCOMPARE(lp.pageCount(), 2);
        QCOMPARE(lp.lines(1).size(), 1);
    }

    // A break that is on the last line the page has room for, with no room left for the empty line after it, still opens the page after it with the
    // paragraph that follows (the text after the break is not lost), in a publication's text and a word processor's, and where the mark follows the break the
    // page begins with that empty line, which is the paragraph's own next line.
    void aPageBreakOnTheLastLineThatFitsStillOpensTheNextPage_data()
    {
        QTest::addColumn<bool>("word");
        QTest::addColumn<bool>("markFollows");
        QTest::newRow("publication") << false << false;
        QTest::newRow("word") << true << false;
        QTest::newRow("publication, mark follows") << false << true;
        QTest::newRow("word, mark follows") << true << true;
    }

    void aPageBreakOnTheLastLineThatFitsStillOpensTheNextPage()
    {
        QFETCH(bool, word);
        QFETCH(bool, markFollows);
        FlowDocument fd;
        fd.env.wordLines = word;
        fd.markFollowsBreak = markFollows;
        const double line = 24, body = fd.section(0).content().height();
        const int filler = int(body / line) - 1;
        QTextBlockFormat bf;
        bf.setLineHeight(line, QTextBlockFormat::FixedHeight);
        QTextCursor c(fd.story());
        for (int i = 0; i < filler; ++i) {
            if (i) c.insertBlock(bf); else c.setBlockFormat(bf);
            c.insertText(QStringLiteral("filler"));
        }
        c.insertBlock(bf);
        c.insertText(QStringLiteral("ends with a break") + flow::kPageBreak);
        c.insertBlock(bf);
        c.insertText(QStringLiteral("after the break"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 2);
        const auto second = pg.lines(1);
        const int after = fd.story()->findBlockByNumber(filler + 1).position();
        if (markFollows) {
            QCOMPARE(second.size(), 2);
            QCOMPARE(second[0].block, fd.story()->findBlockByNumber(filler).position());   // (the break paragraph's empty line)
            QCOMPARE(second[0].line, 1);
            QCOMPARE(second[1].block, after);
        } else {
            QCOMPARE(second.size(), 1);
            QCOMPARE(second[0].block, after);
        }
        QVERIFY(pg.page(1).start.hard);
    }

    // A break in the middle of a paragraph, on the last line that fits, begins the next page with the rest of the same paragraph, then the paragraphs after it.
    void aPageBreakInTheMiddleOfAParagraphContinuesItOnTheNextPage()
    {
        for (const bool word : {false, true}) {
            FlowDocument fd;
            fd.env.wordLines = word;
            const double line = 24, body = fd.section(0).content().height();
            const int filler = int(body / line) - 1;
            QTextBlockFormat bf;
            bf.setLineHeight(line, QTextBlockFormat::FixedHeight);
            QTextCursor c(fd.story());
            for (int i = 0; i < filler; ++i) {
                if (i) c.insertBlock(bf); else c.setBlockFormat(bf);
                c.insertText(QStringLiteral("filler"));
            }
            c.insertBlock(bf);
            c.insertText(QStringLiteral("first words") + flow::kPageBreak + QStringLiteral("second words"));
            c.insertBlock(bf);
            c.insertText(QStringLiteral("after"));
            Paginator pg(&fd);
            QCOMPARE(pg.pageCount(), 2);
            const auto second = pg.lines(1);
            QCOMPARE(second.size(), 2);
            QCOMPARE(second[0].block, fd.story()->findBlockByNumber(filler).position());   // (the rest of the paragraph the break is in)
            QCOMPARE(second[0].line, 1);
            QCOMPARE(second[1].block, fd.story()->findBlockByNumber(filler + 1).position());
        }
    }

    // Two breaks in a row make the page between them; one at the start of a paragraph moves its text to the next page.
    void pageBreakCharactersFollowOneAnotherAndOpenAParagraph()
    {
        FlowDocument fd;
        QTextCursor c(fd.story());
        c.insertText(QStringLiteral("a") + flow::kPageBreak + flow::kPageBreak + QStringLiteral("b"));
        c.insertBlock();
        c.insertText(flow::kPageBreak + QStringLiteral("moved"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 4);
        QCOMPARE(pg.lines(1).size(), 1);            // (the page that only the second break is on)
        QCOMPARE(pg.lines(1)[0].pos, 2);
        QCOMPARE(pg.lines(1)[0].end, 3);
        QCOMPARE(pg.lines(2)[0].pos, 3);            // b
        const int second = fd.story()->findBlockByNumber(1).position();
        QCOMPARE(pg.lines(2).size(), 2);            // b, and the line the next paragraph's break is on, which ends the page
        QCOMPARE(pg.lines(2)[0].end, second - 1);
        QCOMPARE(pg.lines(2)[1].block, second);
        QCOMPARE(pg.lines(3)[0].pos, second + 1);   // moved
    }

    // Editing the break: taking it out joins the pages, typing it in splits them, and the pages after an edit are as the text makes them.
    void editingAPageBreakCharacterRepaginates()
    {
        FlowDocument fd;
        fillStory(fd.story(), 30, 8, {}, 20, 30);
        Paginator pg(&fd);
        const int plain = pg.pageCount();
        QTextCursor c(fd.story());
        c.setPosition(fd.story()->findBlockByNumber(12).position() + 10);
        const int was = pg.pageOfPosition(c.position());
        c.insertText(flow::kPageBreak);
        QVERIFY(pg.pageCount() >= plain);
        const int at = pg.pageOfPosition(c.position());
        QVERIFY(at == was + 1);                      // (what comes after the break is on the page after)
        const auto lines = pg.lines(was);
        QCOMPARE(lines.last().end, c.position());    // the page before ends where the break is
        c.setPosition(c.position() - 1);
        c.deleteChar();
        QCOMPARE(pg.pageCount(), plain);
        // The same pages as a document that never had it.
        FlowDocument other;
        fillStory(other.story(), 30, 8, {}, 20, 30);
        Paginator po(&other);
        QCOMPARE(po.pageCount(), plain);
        for (int k = 0; k < plain; ++k) QCOMPARE(pg.page(k).start.pos, po.page(k).start.pos);
    }

    // A section break starts the next section on a new page with its own size, margins, and columns.
    void paginatorStartsSectionsOnNewPages()
    {
        FlowDocument fd;
        fillStory(fd.story(), 90, 14, [](int i, QTextBlockFormat &bf) { if (i == 24 || i == 50) bf.setProperty(flow::fp::SectionBreakAfter, true); });
        Section second;
        second.orientation = Section::Orientation::Landscape;
        second.margins = QMarginsF(54, 36, 54, 36);
        second.columns = 2;
        second.columnGap = 24;
        Section third;
        third.paper = QSizeF(420, 595);
        third.margins = QMarginsF(40, 40, 40, 40);
        fd.sections << second << third;
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QSet<int> sections;
        int firstOfSecond = -1, firstOfThird = -1;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.page(k);
            sections.insert(page.start.section);
            const Section &sec = fd.section(page.start.section);
            QCOMPARE(page.size, sec.pageSize());
            QCOMPARE(page.content, sec.content());
            if (page.start.section == 1 && firstOfSecond < 0) firstOfSecond = k;
            if (page.start.section == 2 && firstOfThird < 0) firstOfThird = k;
            // The lines are in the columns the section has, and no wider.
            for (const auto &l : pg.lines(k)) {
                QVERIFY(l.frame >= 0 && l.frame < sec.columns);
                QVERIFY(l.rect.right() <= sec.columnWidth() + 0.01);
                QVERIFY(l.rect.bottom() - l.below <= sec.content().height() + 0.02);
            }
        }
        QCOMPARE(sections.size(), 3);
        QVERIFY(firstOfSecond > 0 && firstOfThird > firstOfSecond);
        // The section's first page starts with its first paragraph, and the one before ends with the break's.
        QCOMPARE(fd.story()->findBlock(pg.page(firstOfSecond).start.pos).blockNumber(), 25);
        QCOMPARE(fd.story()->findBlock(pg.page(firstOfThird).start.pos).blockNumber(), 51);
        QCOMPARE(fd.story()->findBlock(pg.lines(firstOfSecond - 1).last().block).blockNumber(), 24);
        QVERIFY(pg.page(firstOfSecond).start.hard);
        QCOMPARE(pg.page(firstOfSecond).size, QSizeF(792, 612));
        QCOMPARE(pg.page(firstOfThird).size, QSizeF(420, 595));
    }

    // The space before a paragraph is kept at the top of the first page and after a break asked for, and left out where the text came
    // to the top of a page for want of room.
    void paginatorLeavesOutSpaceBeforeAtTheTopOfAPage()
    {
        FlowDocument fd;
        fillStory(fd.story(), 80, 15, [](int i, QTextBlockFormat &bf) {
            bf.setTopMargin(20);
            if (i == 33) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
        });
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages > 5);
        bool natural = false, asked = false;
        for (int k = 0; k < pages; ++k) {
            const auto lines = pg.lines(k);
            const double top = lines.first().rect.top();
            const bool begins = lines.first().line == 0;
            if (k == 0) { QVERIFY2(std::abs(top - 20) < 0.01, qPrintable(QString::number(top))); continue; }
            if (!begins) { QVERIFY2(std::abs(top) < 0.01, "the rest of a paragraph has nothing above it"); continue; }
            const bool hard = pg.page(k).start.hard;
            QVERIFY2(std::abs(top - (hard ? 20 : 0)) < 0.01, qPrintable(QStringLiteral("page %1: first line at %2, hard %3").arg(k).arg(top).arg(hard)));
            natural |= !hard;
            asked |= hard;
        }
        QVERIFY(natural && asked);
    }

    // A word processor's single line is its font's Windows ascent and descent plus the leading the font's tables ask for, whatever font stands in
    // for it: 1.15 em for Times New Roman and Arial, 1.22 em for Calibri, with the baseline one ascent below the line's top.
    void wordLinesAreTheFontsWindowsMetrics()
    {
        const auto lines = wordLinesOf({{QStringLiteral("Times New Roman"), 12}, {QStringLiteral("Arial"), 11}, {QStringLiteral("Calibri"), 11}, {QStringLiteral("Courier New"), 10}});
        QCOMPARE(lines.size(), 4);
        const double heights[] = {(1825 + 443 + 87) / 2048.0 * 12, (1854 + 434 + 67) / 2048.0 * 11, (1950 + 550) / 2048.0 * 11, (1705 + 615) / 2048.0 * 10};
        const double ascents[] = {1825 / 2048.0 * 12, 1854 / 2048.0 * 11, 1950 / 2048.0 * 11, 1705 / 2048.0 * 10};
        double top = 0;
        for (int i = 0; i < 4; ++i) {
            QVERIFY2(std::abs(lines[i].rect.top() - top) < 0.01, qPrintable(QStringLiteral("line %1 starts at %2, not %3").arg(i).arg(lines[i].rect.top()).arg(top)));
            QVERIFY2(std::abs(lines[i].rect.height() - heights[i]) < 0.01, qPrintable(QStringLiteral("line %1 is %2 tall, not %3").arg(i).arg(lines[i].rect.height()).arg(heights[i])));
            QVERIFY2(std::abs(lines[i].baseline - (top + ascents[i])) < 0.02, qPrintable(QStringLiteral("line %1's baseline is at %2, not %3").arg(i).arg(lines[i].baseline).arg(top + ascents[i])));
            top += heights[i];
        }
        // (Without it, a text's lines are as before: a Publisher's, 1.06 em for Times New Roman.)
        const auto old = wordLinesOf({{QStringLiteral("Times New Roman"), 12}}, {}, false);
        QVERIFY(std::abs(old[0].rect.height() - heights[0]) > 0.5);
    }

    // Each face of the fonts a word processor measures has its own Windows ascent and descent, from the font's tables: Comic Sans MS and Palatino Linotype
    // are not set in Calibri's line, Book Antiqua Bold's line is shorter than its regular face's, and Bookman Old Style Italic's baseline is its own ascent
    // below the line's top. A line is the Windows ascent and descent plus the header's gap less what they already hold beyond the header's own.
    void wordLinesOfMeasuredFacesAreTheirOwn()
    {
        struct Face { const char *family; double size; bool bold, italic; int winAscent, winDescent, ascent, descent, gap; };
        const Face faces[] = {
            {"Comic Sans MS", 11, false, false, 2257, 597, 2257, 597, 0},
            {"Palatino Linotype", 10, true, false, 2150, 613, 1499, 582, 682},
            {"Book Antiqua", 12, false, false, 1967, 578, 1891, 578, 0},
            {"Book Antiqua", 12, true, false, 1891, 543, 1891, 578, 0},
            {"Bookman Old Style", 10, false, true, 1791, 475, 1929, 475, 0},
            {"Bookman Old Style", 10, false, false, 1831, 473, 1929, 475, 0},
            // (The math font's Windows ascent and descent, 6383 and 5045 units, are for stacking formulas: a line of its text is the header's, 1595 and 455.)
            {"Cambria Math", 10, false, false, 1595, 455, 1595, 455, 353},
        };
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        for (int i = 0; i < int(std::size(faces)); ++i) {
            QTextCharFormat cf;
            cf.setFontFamilies({QString::fromLatin1(faces[i].family)});
            cf.setFontPointSize(faces[i].size);
            if (faces[i].bold) cf.setFontWeight(QFont::Bold);
            cf.setFontItalic(faces[i].italic);
            if (i == 0) c.setBlockCharFormat(cf); else c.insertBlock(QTextBlockFormat(), cf);
            c.insertText(QStringLiteral("Line %1").arg(i), cf);
        }
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(400, 700)}, env);
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), int(std::size(faces)));
        double top = 0;
        for (int i = 0; i < int(std::size(faces)); ++i) {
            const Face &f = faces[i];
            const double leading = std::max(0, f.gap - ((f.winAscent + f.winDescent) - (f.ascent + f.descent)));
            const double height = (f.winAscent + f.winDescent + leading) / 2048.0 * f.size;
            QVERIFY2(std::abs(lines[i].rect.height() - height) < 0.01, qPrintable(QStringLiteral("%1 (line %2) is %3 tall, not %4").arg(f.family).arg(i).arg(lines[i].rect.height()).arg(height)));
            QVERIFY2(std::abs(lines[i].baseline - (top + f.winAscent / 2048.0 * f.size)) < 0.02, qPrintable(QStringLiteral("%1 (line %2): baseline at %3, not %4").arg(f.family).arg(i).arg(lines[i].baseline).arg(top + f.winAscent / 2048.0 * f.size)));
            top += height;
        }
    }

    // A family that isn't on the machine is set as its stand-in is, Calibri's line; Times and Helvetica are the names of Times New Roman and Arial.
    void wordLinesOfMissingFamiliesAreTheStandIns()
    {
        const auto lines = wordLinesOf({{QStringLiteral("No Such Typeface Anywhere"), 12}, {QStringLiteral("Times"), 12}, {QStringLiteral("Helvetica"), 11}});
        QCOMPARE(lines.size(), 3);
        const double heights[] = {(1950 + 550) / 2048.0 * 12, (1825 + 443 + 87) / 2048.0 * 12, (1854 + 434 + 67) / 2048.0 * 11};
        for (int i = 0; i < 3; ++i)
            QVERIFY2(std::abs(lines[i].rect.height() - heights[i]) < 0.01, qPrintable(QStringLiteral("line %1 is %2 tall, not %3").arg(i).arg(lines[i].rect.height()).arg(heights[i])));
    }

    // The characters of the symbol fonts (a degree sign from Symbol, an arrow from Wingdings, a solid block from Webdings: codes 0xF020 to 0xF0FF) are drawn as
    // the pictures they are where this machine lacks the font, each as wide as the font makes it (819, 1825, and 2048 units of 2048 for these three).
    void symbolFontCharactersAreTheirPicturesAtTheirOwnWidths()
    {
        struct Case { const char *family; char16_t code, picture; int units; };
        const Case cases[] = {{"Symbol", 0xF0B0, 0x00B0, 819}, {"Wingdings", 0xF0EA, 0x2B07, 1825}, {"Webdings", 0xF067, 0x2588, 2048}};
        for (const Case &k : cases) {
            if (QFontDatabase::hasFamily(QString::fromLatin1(k.family))) continue;   // (installed here: drawn by the font itself)
            auto doc = std::make_unique<QTextDocument>();
            QTextCursor c(doc.get());
            QTextCharFormat cf;
            cf.setFontFamilies({QString::fromLatin1(k.family)});
            cf.setFontPointSize(10);
            c.setCharFormat(cf);
            c.insertText(QString(80, QChar(k.code)), cf);
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout lay;
            lay.build(doc.get(), {plainFrame(105, 700)}, env);
            const auto lines = lay.lineInfo(0);
            QVERIFY(!lines.isEmpty());
            const int fit = int((105 + 0.13) / (k.units / 2048.0 * 10));
            QVERIFY2(lines[0].text.size() == fit && lines[0].text == QString(fit, QChar(k.picture)),
                     qPrintable(QStringLiteral("%1: the first line has %2 characters, %3 fit as the font sets them; its first is U+%4").arg(k.family).arg(lines[0].text.size()).arg(fit).arg(lines[0].text.isEmpty() ? 0 : lines[0].text[0].unicode(), 0, 16)));
        }
    }

    // A font the file's font table gives an alternate for is set in that alternate when the word processor has it, whether or not this machine has it (a
    // bundled look-alike stands in for it): an unknown serif face with Bookman Old Style as its alternate is not set in Calibri.
    void aFontsAlternateIsKeptWhereTheWordProcessorHasIt()
    {
        LayoutEnv env;
        env.wordLines = true;
        env.standIns.insert(QStringLiteral("itc bookman light"), QStringLiteral("Bookman Old Style"));
        env.standIns.insert(QStringLiteral("futura md bt"), QStringLiteral("Century Gothic"));
        env.standIns.insert(QStringLiteral("no such face"), QStringLiteral("No Such Alternate"));
        auto familyOf = [&](const QString &family) {
            QTextCharFormat cf;
            cf.setFontFamilies({family});
            cf.setFontPointSize(10);
            return resolveCharFormat(cf, env).fontFamilies().toStringList().value(0);
        };
        QCOMPARE(familyOf(QStringLiteral("ITC Bookman Light")), QStringLiteral("Bookman Old Style"));
        QCOMPARE(familyOf(QStringLiteral("Futura Md BT")), QStringLiteral("Century Gothic"));
        QCOMPARE(familyOf(QStringLiteral("No Such Face")), QStringLiteral("Calibri"));
    }

    // A word processor kerns the letters of a run only where the file asks for it (tp::KernAbove, from w:kern): a run that doesn't ask isn't kerned at any
    // size, where a publisher's text is kerned from 14 points. Kerned text of letter pairs that kern (AV, VA) is narrower, so more of it fits on a line.
    void wordTextKernsOnlyWhereTheFileAsks()
    {
        auto charsOnFirstLine = [&](bool word, double size, std::optional<double> kernAbove) {
            auto doc = std::make_unique<QTextDocument>();
            QTextCursor c(doc.get());
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Arial")});
            cf.setFontPointSize(size);
            if (kernAbove) cf.setProperty(tp::KernAbove, *kernAbove);
            c.setCharFormat(cf);
            QString text;
            for (int i = 0; i < 60; ++i) text += QStringLiteral("AVAVAVAV ");
            c.insertText(text, cf);
            LayoutEnv env;
            env.wordLines = word;
            StoryLayout lay;
            lay.build(doc.get(), {plainFrame(300, 700)}, env);
            return lay.lineInfo(0).value(0).text.size();
        };
        const auto plain = charsOnFirstLine(true, 20, std::nullopt);       // asks nothing: not kerned, though 20 points is past 14
        const auto never = charsOnFirstLine(true, 20, 200.0);              // asks from 200 points: not kerned
        const auto asked = charsOnFirstLine(true, 20, 10.0);               // asks from 10 points: kerned
        QCOMPARE(plain, never);
        QVERIFY2(asked > plain, qPrintable(QStringLiteral("kerned %1 characters, plain %2").arg(asked).arg(plain)));
        // (A publisher's text keeps its own rule: kerned from 14 points whether it says so or not.)
        QVERIFY(charsOnFirstLine(false, 20, std::nullopt) > charsOnFirstLine(false, 20, 200.0));
    }

    // The spaces and tabs that end a word processor's line take no part in its height, and a line that is only such blanks is as tall as the paragraph
    // mark's font makes it; blanks among the words count.
    void blanksAtTheEndOfAWordLineTakeNoPartInItsHeight()
    {
        const QString tnr = QStringLiteral("Times New Roman");
        auto single = [&](double size) { return (1825 + 443 + 87) / 2048.0 * size; };
        // One paragraph of runs (text, size in points), its mark `mark` points; the height of its first line.
        auto height = [&](const QVector<std::pair<QString, double>> &runs, double mark) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat markFormat;
            markFormat.setFontFamilies({tnr});
            markFormat.setFontPointSize(mark);
            c.setBlockCharFormat(markFormat);
            for (const auto &r : runs) {
                QTextCharFormat cf;
                cf.setFontFamilies({tnr});
                cf.setFontPointSize(r.second);
                c.insertText(r.first, cf);
            }
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(400, 700)}, env);
            const auto lines = lay.lineInfo(0);
            return lines.isEmpty() ? -1.0 : lines[0].rect.height();
        };
        const QString text = QStringLiteral("some text");
        QVERIFY(std::abs(height({{text, 10}, {QStringLiteral("   "), 24}}, 10) - single(10)) < 0.01);
        QVERIFY(std::abs(height({{text, 10}, {QStringLiteral("\t\t"), 24}}, 10) - single(10)) < 0.01);
        QVERIFY(std::abs(height({{text, 10}, {QStringLiteral(" \t "), 24}}, 10) - single(10)) < 0.01);
        // Only blanks: the mark's font.
        QVERIFY(std::abs(height({{QStringLiteral("\t\t\t"), 24}}, 9) - single(9)) < 0.01);
        QVERIFY(std::abs(height({{QStringLiteral("    "), 24}}, 14) - single(14)) < 0.01);
        // Blanks among the words are part of the line.
        QVERIFY(std::abs(height({{QStringLiteral("a"), 10}, {QStringLiteral("   "), 24}, {QStringLiteral("b"), 10}}, 10) - single(24)) < 0.01);
        // And the mark doesn't make a line of text taller.
        QVERIFY(std::abs(height({{text, 10}}, 24) - single(10)) < 0.01);
    }

    // The stand-in a file's font table names for a family (the environment's stand-ins, LayoutEnv::standIns) is the line a missing family is set in.
    void missingFamilyStandsInAsTheFileSays()
    {
        auto height = [&](const QString &family, const QHash<QString, QString> &standIns) {
            auto doc = std::make_unique<QTextDocument>();
            QTextCursor c(doc.get());
            QTextCharFormat cf;
            cf.setFontFamilies({family});
            cf.setFontPointSize(12);
            c.setBlockCharFormat(cf);
            c.insertText(QStringLiteral("Line"), cf);
            LayoutEnv env;
            env.wordLines = true;
            env.standIns = standIns;
            StoryLayout lay;
            lay.build(doc.get(), {plainFrame(400, 700)}, env);
            return lay.lineInfo(0).first().rect.height();
        };
        const QString missing = QStringLiteral("No Such Typeface Anywhere");
        const double calibri = (1950 + 550) / 2048.0 * 12, times = (1825 + 443 + 87) / 2048.0 * 12, arial = (1854 + 434 + 67) / 2048.0 * 12, courier = (1705 + 615) / 2048.0 * 12;
        // Each stand-in the table names is the line it is set in (none of them is the Calibri that a family with no entry gets).
        QVERIFY(std::abs(height(missing, {{missing.toLower(), QStringLiteral("Times New Roman")}}) - times) < 0.01);
        QVERIFY(std::abs(height(missing, {{missing.toLower(), QStringLiteral("Arial")}}) - arial) < 0.01);
        QVERIFY(std::abs(height(missing, {{missing.toLower(), QStringLiteral("Courier New")}}) - courier) < 0.01);
        QVERIFY(std::abs(times - calibri) > 0.5 && std::abs(arial - calibri) > 0.1 && std::abs(courier - calibri) > 0.5);
        // A family with no entry, or an entry that names another family that isn't there, is set in Calibri.
        QVERIFY(std::abs(height(missing, {}) - calibri) < 0.01);
        QVERIFY(std::abs(height(missing, {{missing.toLower(), QStringLiteral("Another Missing Typeface")}}) - calibri) < 0.01);
    }

    // A section's page numbers in dashes are the number between two hyphens, a space each side.
    void pageNumbersInDashes()
    {
        struct Dashed : TestNotes {
            QString pageNumberFormat() const override { return QStringLiteral("dash"); }
        } source;
        FieldContext ctx;
        ctx.doc = &source;
        ctx.pageNumber = 3;
        QCOMPARE(ctx.resolve(QStringLiteral("page")), QStringLiteral("- 3 -"));
    }

    // A number that isn't hanging is followed by a tab: the text begins at the next stop past the number, the paragraph's own or the default ones.
    void numberIsFollowedByATab()
    {
        auto textStart = [&](double indent, double left, const QList<QTextOption::Tab> &tabs) {
            auto doc = std::make_unique<QTextDocument>();
            QTextCursor c(doc.get());
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDecimal);
            lf.setNumberSuffix(QStringLiteral("."));
            QTextBlockFormat bf;
            bf.setTextIndent(indent);
            bf.setLeftMargin(left);
            if (!tabs.isEmpty()) bf.setTabPositions(tabs);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Times New Roman")});
            cf.setFontPointSize(12);
            c.setBlockCharFormat(cf);
            c.insertText(QStringLiteral("Text of the item"), cf);
            c.createList(lf);
            LayoutEnv env;
            env.wordLines = true;
            env.defaultTab = 36;
            StoryLayout lay;
            lay.build(doc.get(), {plainFrame(400, 700)}, env);
            return lay.lineInfo(0).first().rect.left();
        };
        // A number set in at 36 points ends before the stop at 72; one that ends past a stop goes to the next.
        QVERIFY2(std::abs(textStart(36, 0, {}) - 72) < 0.01, qPrintable(QString::number(textStart(36, 0, {}))));
        QVERIFY2(std::abs(textStart(60, 0, {}) - 72) < 0.01, qPrintable(QString::number(textStart(60, 0, {}))));
        // A stop of the paragraph's own is the one it goes to, and the default stops begin past the last of them.
        QVERIFY2(std::abs(textStart(36, 0, {QTextOption::Tab(100, QTextOption::LeftTab)}) - 100) < 0.01, qPrintable(QString::number(textStart(36, 0, {QTextOption::Tab(100, QTextOption::LeftTab)}))));
        QVERIFY2(std::abs(textStart(36, 0, {QTextOption::Tab(40, QTextOption::LeftTab)}) - 72) < 0.01, "a stop before the number's end is passed");
        // A hanging number leaves the text at the paragraph's indent.
        QVERIFY2(std::abs(textStart(-36, 36, {}) - 36) < 0.01, qPrintable(QString::number(textStart(-36, 36, {}))));
    }

    // A multiple of single spacing makes the line that many times as tall with the text at its top; an exact height sets the baseline a descent
    // above its bottom (the text's top is cut off where there is less room than the font's ascent); an at-least height does the same when it is
    // more than a single line, and is a single line when it is less.
    void wordLineHeightsFollowTheirRules()
    {
        const QString tnr = QStringLiteral("Times New Roman");
        const double single = (1825 + 443 + 87) / 2048.0 * 12, ascent = 1825 / 2048.0 * 12, descent = 443 / 2048.0 * 12;
        auto one = [&](QTextBlockFormat::LineHeightTypes type, double value) {
            return wordLinesOf({{tnr, 12}}, [&](int, QTextBlockFormat &bf) { bf.setLineHeight(value, type); }).first();
        };
        auto twice = one(QTextBlockFormat::ProportionalHeight, 200);
        QVERIFY(std::abs(twice.rect.height() - 2 * single) < 0.01);
        QVERIFY(std::abs(twice.baseline - ascent) < 0.02);
        auto exact = one(QTextBlockFormat::FixedHeight, 20);
        QVERIFY(std::abs(exact.rect.height() - 20) < 0.01);
        QVERIFY(std::abs(exact.baseline - (20 - descent)) < 0.02);
        auto tight = one(QTextBlockFormat::FixedHeight, 9);
        QVERIFY(std::abs(tight.rect.height() - 9) < 0.01);
        QVERIFY(std::abs(tight.baseline - (9 - descent)) < 0.02);
        auto most = one(QTextBlockFormat::MinimumHeight, 24);
        QVERIFY(std::abs(most.rect.height() - 24) < 0.01);
        QVERIFY(std::abs(most.baseline - (24 - descent)) < 0.02);
        auto least = one(QTextBlockFormat::MinimumHeight, 10);
        QVERIFY(std::abs(least.rect.height() - single) < 0.01);
        QVERIFY(std::abs(least.baseline - ascent) < 0.02);
    }

    // A mark that shows nothing in a line (a tag's or a bookmark's, kept as a character that shows no text) is no run of it: it doesn't make the line taller.
    void aMarkThatShowsNothingIsNoRunOfTheLine()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat plainRun;
        plainRun.setFontFamilies({QStringLiteral("Times New Roman")});
        plainRun.setFontPointSize(12);
        QTextCharFormat mark = plainRun;
        mark.setFontPointSize(40);
        mark.setProperty(tp::Field, QStringLiteral("raw:"));
        c.insertText(QStringLiteral("some words "), plainRun);
        c.insertText(QString(QChar::ObjectReplacementCharacter), mark);
        c.insertText(QStringLiteral(" and more"), plainRun);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(400, 700)}, env);
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 1);
        QVERIFY2(std::abs(lines[0].rect.height() - (1825 + 443 + 87) / 2048.0 * 12) < 0.01, qPrintable(QString::number(lines[0].rect.height())));
    }

    // A list item's number or bullet is a run of its own font on the item's first line, which has room for it above the baseline: a Symbol bullet's line is Symbol's
    // ascent over the text's descent.
    void aBulletInItsOwnFontHasRoomInTheFirstLine()
    {
        auto first = [](const QString &bulletFont) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            c.insertText(QStringLiteral("an item of the list that is long enough to take two lines of the frame when it is set in it, at twelve points"), run);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            lf.setIndent(1);
            if (!bulletFont.isEmpty()) lf.setProperty(tp::BulletFont, bulletFont);
            c.createList(lf);
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(300, 700)}, env);
            return lay.lineInfo(0);
        };
        const auto plain = first(QString()), symbol = first(QStringLiteral("Symbol"));
        QVERIFY(plain.size() >= 2 && symbol.size() >= 2);
        const double single = (1825 + 443 + 87) / 2048.0 * 12, symbolLine = (2059 + 443) / 2048.0 * 12;
        QVERIFY2(std::abs(plain[0].rect.height() - single) < 0.01, qPrintable(QString::number(plain[0].rect.height())));
        QVERIFY2(std::abs(symbol[0].rect.height() - symbolLine) < 0.01, qPrintable(QString::number(symbol[0].rect.height())));
        QVERIFY(std::abs(symbol[1].rect.height() - single) < 0.01);   // (the lines after it are the text's)
        // A bullet whose own line is shorter than the text's (Courier's) makes it no taller.
        const auto courier = first(QStringLiteral("Courier New"));
        QVERIFY2(std::abs(courier[0].rect.height() - single) < 0.01, qPrintable(QString::number(courier[0].rect.height())));
    }

    // A paragraph's borders take room: the top one and its distance from the text above the first line, the bottom one below the last, and none between
    // paragraphs with the same borders but the line between them; the side borders take none. The room is added to the space before and after.
    void paragraphBordersTakeRoom()
    {
        const QString tnr = QStringLiteral("Times New Roman");
        tp::ParaBox box;
        box.top = {QStringLiteral("single"), 8, 3, QColor()};
        box.bottom = {QStringLiteral("double"), 4, 2, QColor()};
        box.left = {QStringLiteral("single"), 4, 4, QColor()};
        const double single = (1825 + 443 + 87) / 2048.0 * 12;
        auto lines = [&](const std::function<void(int, QTextBlockFormat &)> &shape) { return wordLinesOf({{tnr, 12}, {tnr, 12}, {tnr, 12}, {tnr, 12}}, shape); };
        // The second paragraph alone has the borders.
        const auto one = lines([&](int i, QTextBlockFormat &bf) {
            bf.setTopMargin(2);
            bf.setBottomMargin(2);
            if (i == 1) bf.setProperty(tp::Borders, box.spec());
        });
        const auto none = lines([&](int, QTextBlockFormat &bf) {
            bf.setTopMargin(2);
            bf.setBottomMargin(2);
        });
        const double topRoom = 1 + 3, bottomRoom = 4 / 8.0 * 3 + 2;   // (a double border of half point lines is three times that wide)
        QVERIFY(std::abs(one[1].rect.top() - (none[1].rect.top() + topRoom)) < 0.01);
        QVERIFY(std::abs(one[2].rect.top() - (none[2].rect.top() + topRoom + bottomRoom)) < 0.01);
        QVERIFY(std::abs(one[0].rect.top() - none[0].rect.top()) < 0.01);
        QVERIFY(std::abs(one[1].rect.height() - single) < 0.01);
        // Two paragraphs with the same borders are one box: the top is above the first, the bottom below the second, and nothing between them
        // but their space (the larger of the first's after and the second's before).
        const auto pair = lines([&](int i, QTextBlockFormat &bf) {
            bf.setTopMargin(2);
            bf.setBottomMargin(2);
            if (i == 1 || i == 2) bf.setProperty(tp::Borders, box.spec());
        });
        QVERIFY(std::abs(pair[1].rect.top() - (none[1].rect.top() + topRoom)) < 0.01);
        QVERIFY(std::abs((pair[2].rect.top() - pair[1].rect.bottom()) - 2) < 0.01);
        QVERIFY(std::abs(pair[3].rect.top() - (none[3].rect.top() + topRoom + bottomRoom)) < 0.01);
        // (A line between them takes the room its width and distance ask.)
        tp::ParaBox ruled = box;
        ruled.between = {QStringLiteral("single"), 8, 1, QColor()};
        const auto between = lines([&](int i, QTextBlockFormat &bf) {
            bf.setTopMargin(2);
            bf.setBottomMargin(2);
            if (i == 1 || i == 2) bf.setProperty(tp::Borders, ruled.spec());
        });
        QVERIFY(std::abs((between[2].rect.top() - between[1].rect.bottom()) - (2 + 2)) < 0.01);
        // The spec reads back as it was written.
        QVERIFY(tp::ParaBox::parse(ruled.spec()) == ruled);
    }

    // The borders are drawn at the paragraph's edges, a distance from the text and outside its indents, and the shading fills the box.
    void paragraphBordersAndShadingAreDrawn()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        bf.setLeftMargin(30);
        bf.setRightMargin(30);
        bf.setTopMargin(20);
        tp::ParaBox box;
        box.top = {QStringLiteral("single"), 16, 3, QColor(Qt::black)};
        box.bottom = {QStringLiteral("single"), 16, 3, QColor(Qt::black)};
        box.left = {QStringLiteral("single"), 16, 4, QColor(Qt::black)};
        box.right = {QStringLiteral("single"), 16, 4, QColor(Qt::black)};
        box.fill = QColor(200, 200, 255);
        bf.setProperty(tp::Borders, box.spec());
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("boxed"), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(200, 100)}, env);
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 1);
        const double scale = 8;
        QImage img(int(200 * scale), int(100 * scale), QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.scale(scale, scale);
        lay.paint(&p, 0, PaintOptions());
        p.end();
        auto at = [&](double x, double y) { return QColor(img.pixel(int(x * scale), int(y * scale))); };
        const QRectF text = lines[0].rect;
        const double top = text.top(), bottom = text.bottom();
        // The box's edges: the borders 2 points wide, 3 and 4 points from the text, outside the indents (30 points).
        QVERIFY(at(100, top - 3 - 1).lightness() < 40);                    // the top line, 3 points above the text
        QVERIFY(at(100, bottom + 3 + 1).lightness() < 40);                  // the bottom line
        QVERIFY(at(30 - 4 - 1, (top + bottom) / 2).lightness() < 40);      // the left line, 4 points left of the indent
        QVERIFY(at(200 - 30 + 4 + 1, (top + bottom) / 2).lightness() < 40);   // the right line
        QCOMPARE(at(100, top - 3 - 2 - 6).lightness(), 255);               // outside it, nothing
        QCOMPARE(at(15, (top + bottom) / 2).lightness(), 255);
        const QColor inside = at(31 + 100, bottom + 1);                     // between the text and the bottom line, filled
        QVERIFY2(std::abs(inside.red() - 200) < 4 && std::abs(inside.blue() - 255) < 4, qPrintable(inside.name()));
        const QColor gap = at(30 - 2, (top + bottom) / 2);                 // between the left line and the indent, filled
        QVERIFY2(std::abs(gap.red() - 200) < 4, qPrintable(gap.name()));
    }

    // A line of text that is as wide as the room it has, to what rounding leaves, fits on one line.
    void aLineAsWideAsItsRoomFits()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        c.insertText(QString(36, QLatin1Char('_')), run);   // (half an em each: 216 points)
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(216, 200)}, env);
        QCOMPARE(lay.lineInfo(0).size(), 1);
        StoryLayout narrower;
        narrower.build(&doc, {plainFrame(215, 200)}, env);
        QCOMPARE(narrower.lineInfo(0).size(), 2);
    }

    // Hidden text takes no room and shows nothing: a line's text is what is left, and a paragraph of nothing else is an empty line.
    void hiddenTextIsLeftOut()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextCharFormat hidden = run;
        hidden.setProperty(tp::HiddenText, true);
        c.insertText(QStringLiteral("seen "), run);
        c.insertText(QStringLiteral("<@33><i>hidden codes "), hidden);
        c.insertText(QStringLiteral("and seen again"), run);
        c.insertBlock();
        c.insertText(QStringLiteral("only hidden"), hidden);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(400, 700)}, env);
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 2);
        QCOMPARE(lines[0].text, QStringLiteral("seen and seen again"));
        QCOMPARE(lines[1].text, QString());
        // (As before, for a text that isn't a word processor's: the characters are shown.)
        StoryLayout plain;
        plain.build(&doc, {plainFrame(400, 700)}, LayoutEnv());
        QVERIFY(plain.lineInfo(0)[0].text.contains(QStringLiteral("hidden codes")));
    }

    // Automatic space before and after (a paragraph from a web page) is 14 points, whatever the margins say.
    void automaticSpaceIsFourteenPoints()
    {
        const QString tnr = QStringLiteral("Times New Roman");
        const auto lines = wordLinesOf({{tnr, 12}, {tnr, 12}, {tnr, 12}}, [](int i, QTextBlockFormat &bf) {
            bf.setTopMargin(5);
            bf.setBottomMargin(5);
            if (i == 1) {
                bf.setProperty(tp::SpaceBeforeAuto, true);
                bf.setProperty(tp::SpaceAfterAuto, true);
            }
        });
        // The first's space after (5) and the second's automatic before (14): the larger; then its automatic after and the third's before (5).
        QVERIFY(std::abs((lines[1].rect.top() - lines[0].rect.bottom()) - 14) < 0.01);
        QVERIFY(std::abs((lines[2].rect.top() - lines[1].rect.bottom()) - 14) < 0.01);
        // Between two items of a list, none; before the first and after the last, the automatic space.
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({tnr});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        bf.setProperty(tp::SpaceBeforeAuto, true);
        bf.setProperty(tp::SpaceAfterAuto, true);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("intro"), run);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        QTextList *list = nullptr;
        for (int i = 0; i < 3; ++i) {
            c.insertBlock(bf);
            c.insertText(QStringLiteral("item"), run);
            if (i == 0) list = c.createList(lf); else list->add(c.block());
        }
        c.insertBlock(bf);
        c.insertText(QStringLiteral("outro"), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(400, 700)}, env);
        const auto all = lay.lineInfo(0);
        QCOMPARE(all.size(), 5);
        QVERIFY(std::abs((all[1].rect.top() - all[0].rect.bottom()) - 14) < 0.01);
        QVERIFY(std::abs((all[2].rect.top() - all[1].rect.bottom())) < 0.01);
        QVERIFY(std::abs((all[3].rect.top() - all[2].rect.bottom())) < 0.01);
        QVERIFY(std::abs((all[4].rect.top() - all[3].rect.bottom()) - 14) < 0.01);
    }

    // A word processor's tab stops are measured from the text area's edge, not from the paragraph's indent, and the default stops go on after the last
    // of the paragraph's own.
    void tabStopsAreFromTheEdgeAndTheDefaultOnesFollowTheLast()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        bf.setLeftMargin(20);
        QTextOption::Tab stop;
        stop.position = 100;
        stop.type = QTextOption::LeftTab;
        bf.setTabPositions({stop});
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("ab\tcd\tef"), run);
        LayoutEnv env;
        env.wordLines = true;
        env.defaultTab = 36;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(400, 100)}, env);
        auto xOf = [&](int from) { return lay.rangeRects(0, from, from + 1).first().left(); };
        QVERIFY2(std::abs(xOf(3) - 100) < 0.05, qPrintable(QString::number(xOf(3))));   // "cd": at the stop, from the edge
        QVERIFY2(std::abs(xOf(6) - 144) < 0.05, qPrintable(QString::number(xOf(6))));   // "ef": the next default stop past "cd" (4 times 36)
        QVERIFY(xOf(0) > 19.9 && xOf(0) < 20.1);                                         // (the text begins at the indent)
    }

    // The hanging indent is a tab stop for the first line (a tab goes to it), unless the file says its hanging indents are not (LayoutEnv::wordNoHangStop):
    // then the default stops go on from the edge, and two tabs after a short word end at the second of them.
    void aHangingIndentIsATabStopUnlessTheFileSaysNot()
    {
        auto textAt = [&](bool noStop) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            QTextBlockFormat bf;
            bf.setLeftMargin(72);
            bf.setTextIndent(-72);
            c.setBlockFormat(bf);
            c.insertText(QStringLiteral("!\t\ttext"), run);
            LayoutEnv env;
            env.wordLines = true;
            env.defaultTab = 36;
            env.wordNoHangStop = noStop;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(400, 100)}, env);
            return lay.rangeRects(0, 3, 4).first().left();
        };
        QVERIFY2(std::abs(textAt(false) - 108) < 0.05, qPrintable(QString::number(textAt(false))));   // the first tab to the indent (72), the second to the stop after it
        QVERIFY2(std::abs(textAt(true) - 72) < 0.05, qPrintable(QString::number(textAt(true))));      // the first to 36, the second to 72
        LayoutEnv a, b;
        b.wordNoHangStop = true;
        QVERIFY(a.key() != b.key());
    }

    // A justified line of a word processor's text reaches the margin with its last letter, not a space short of it (the space that ends the line is not
    // part of the width the words are spread over); the paragraph's last line is as long as its words make it.
    void justifiedWordLinesReachTheMargin()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        bf.setAlignment(Qt::AlignJustify);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("Alvin developed from a poorly-defined tropical wave that crossed the Atlantic Ocean and the Caribbean Sea during the second half of May and then moved "
                                    "westward across the eastern North Pacific with little associated thunderstorm activity until the wave continued westward across the "
                                    "eastern North Pacific waters."), run);
        for (bool noHang : {false, true}) {
            LayoutEnv env;
            env.wordLines = true;
            env.wordNoHang = noHang;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(468, 300)}, env);
            const auto lines = lay.lineInfo(0);
            QVERIFY(lines.size() >= 3);
            for (int i = 0; i + 1 < lines.size(); ++i) {
                const auto rects = lay.rangeRects(0, lines[i].docEnd - 2, lines[i].docEnd - 1);
                QVERIFY(!rects.isEmpty());
                QVERIFY2(std::abs(rects.last().right() - 468) < 0.3, qPrintable(QStringLiteral("line %1 ends at %2 (hang %3)").arg(i).arg(rects.last().right()).arg(!noHang)));
            }
            QVERIFY(lay.rangeRects(0, lines.last().docEnd - 2, lines.last().docEnd - 1).last().right() < 400);
        }
    }

    // A word processor's text has no ligatures: the letters of "ti" in Carlito (a face the program carries, with a ligature for them) take what they take
    // apart, where a publication's text sets them as one glyph.
    void wordTextHasNoLigatures()
    {
        QVERIFY2(QFontDatabase::hasFamily(QStringLiteral("Carlito")), "the face the program carries is not registered");
        auto width = [&](const QString &text, bool word) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Carlito")});
            run.setFontPointSize(24);
            c.insertText(text, run);
            LayoutEnv env;
            env.wordLines = word;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(1000, 100)}, env);
            const auto rects = lay.rangeRects(0, 0, text.size());
            return rects.isEmpty() ? -1.0 : rects.last().right() - rects.first().left();
        };
        const double apart = width(QStringLiteral("t"), true) + width(QStringLiteral("i"), true);
        QVERIFY2(std::abs(width(QStringLiteral("ti"), true) - apart) < 0.01, qPrintable(QStringLiteral("%1 against %2").arg(width(QStringLiteral("ti"), true)).arg(apart)));
        QVERIFY2(std::abs(width(QStringLiteral("ti"), false) - apart) > 0.05, "the face has no ligature for ti");   // (a publication's text keeps it)
    }

    // The ligatures of a word processor's text are the file's to ask for (tp::Ligatures on a run that has them: the standard ones, or none), and its letters of
    // the scripts that are shaped with ligatures (Arabic) are not touched by the default of none: a paragraph that has one keeps its font's.
    void wordLigaturesFollowTheRunAndTheScript()
    {
        QVERIFY2(QFontDatabase::hasFamily(QStringLiteral("Carlito")), "the face the program carries is not registered");
        auto width = [&](const QString &text, bool word, int ligatures) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Carlito")});
            run.setFontPointSize(24);
            if (ligatures >= 0) run.setProperty(tp::Ligatures, ligatures);
            c.insertText(text, run);
            LayoutEnv env;
            env.wordLines = word;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(1000, 100)}, env);
            const auto rects = lay.rangeRects(0, 0, 2);
            return rects.isEmpty() ? -1.0 : rects.last().right() - rects.first().left();
        };
        const QString ti = QStringLiteral("ti");
        const double apart = width(ti, true, -1), joined = width(ti, false, -1);
        QVERIFY2(std::abs(apart - joined) > 0.05, "the face has no ligature for ti");
        QVERIFY(std::abs(width(ti, true, 0) - joined) < 0.01);      // (the file asks for the standard ligatures)
        QVERIFY(std::abs(width(ti, true, 1) - apart) < 0.01);       // (and for none)
        QVERIFY(std::abs(width(ti + QChar(0x0628), true, -1) - joined) < 0.01);   // (an Arabic letter in the paragraph: the font's own)
        QVERIFY(std::abs(width(ti + QStringLiteral(" \u0393\u0431"), true, -1) - apart) < 0.01);   // (Greek and Cyrillic are Latin's kind)
    }

    // A mark of punctuation at the end of a line may hang past the margin in a word processor's text: a word that fits but for its period is the last
    // of the line, unless the file says no punctuation hangs; a word that is itself too long, or a mark that isn't one that hangs, is not let.
    void punctuationAtTheEndOfALineHangsPastTheMargin()
    {
        auto lineCount = [&](const QString &text, double room, bool noHang, bool word = true) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            c.insertText(text, run);
            LayoutEnv env;
            env.wordLines = word;
            env.wordNoHang = noHang;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(room, 300)}, env);
            return int(lay.lineInfo(0).size());
        };
        // The room: the width the text takes up to its last mark.
        const QString text = QStringLiteral("alpha beta gamma delta epsilon.");
        QTextDocument measure;
        QTextCursor mc(&measure);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        mc.insertText(text, run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout wide;
        wide.build(&measure, {plainFrame(1000, 100)}, env);
        const double upToMark = wide.rangeRects(0, text.size() - 1, text.size()).first().left();
        const double room = upToMark + 0.5;   // (half a point past the word, not the room the period needs)
        QCOMPARE(lineCount(text, room, false), 1);                // it hangs
        QCOMPARE(lineCount(text, room, true), 2);                 // (not where the file says no punctuation hangs)
        QCOMPARE(lineCount(text, room, false, false), 2);         // (nor in a publication's text)
        QCOMPARE(lineCount(QStringLiteral("alpha beta gamma delta epsilon-"), room, false), 2);   // (a hyphen is no mark that hangs)
        QCOMPARE(lineCount(text, upToMark - 5, false), 2);        // (the word itself doesn't fit)
    }

    // A paragraph that does not let punctuation extend past the margin (tp::NoHangPunct, a word processor's overflowPunct turned off) has none hang, though the
    // text does elsewhere.
    void aParagraphThatTurnsHangingOffHasNone()
    {
        auto lineCount = [&](double room, bool paragraphOff) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            QTextBlockFormat bf;
            if (paragraphOff) bf.setProperty(tp::NoHangPunct, true);
            c.setBlockFormat(bf);
            c.insertText(QStringLiteral("alpha beta gamma delta epsilon."), run);
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(room, 300)}, env);
            return int(lay.lineInfo(0).size());
        };
        QTextDocument measure;
        QTextCursor mc(&measure);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        const QString text = QStringLiteral("alpha beta gamma delta epsilon.");
        mc.insertText(text, run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout wide;
        wide.build(&measure, {plainFrame(1000, 100)}, env);
        const double room = wide.rangeRects(0, text.size() - 1, text.size()).first().left() + 0.5;
        QCOMPARE(lineCount(room, false), 1);
        QCOMPARE(lineCount(room, true), 2);
    }

    // The text after a page break that begins a paragraph is its first line, with the paragraph's first-line indent (here a hanging one); after a break in
    // the middle of a paragraph the text is a line that follows.
    void textAfterABreakThatBeginsAParagraphHasItsFirstLineIndent()
    {
        auto lefts = [&](const QString &text, bool paginated) {
            QVector<double> out;
            QTextBlockFormat bf;
            bf.setLeftMargin(40);
            bf.setTextIndent(-40);
            const QString words = QStringLiteral("alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu nu xi omicron pi rho sigma tau upsilon phi chi psi omega alpha beta gamma delta");
            if (paginated) {
                FlowDocument fd;
                fd.env.wordLines = true;
                QTextCursor c(fd.story());
                c.setBlockFormat(bf);
                c.insertText(text + words);
                Paginator pg(&fd);
                for (int k = 0; k < pg.pageCount(); ++k)
                    for (const auto &l : pg.lines(k)) out << l.rect.left();
                return out;
            }
            QTextDocument doc;
            QTextCursor c(&doc);
            c.setBlockFormat(bf);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            c.insertText(text + words, run);
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(200, 300)}, env);
            for (const auto &l : lay.lineInfo(0)) out << l.rect.left();
            return out;
        };
        for (bool paginated : {false, true}) {
            const auto begin = lefts(QString(flow::kPageBreak), paginated), middle = lefts(QStringLiteral("some words ") + flow::kPageBreak, paginated);
            QVERIFY(begin.size() >= 3 && middle.size() >= 3);
            // The paragraph that begins with the break: its break's line, then the first line (at the margin), then the others (at the indent).
            QVERIFY2(std::abs(begin[1]) < 0.01 && std::abs(begin[2] - 40) < 0.01, qPrintable(QStringLiteral("%1 %2 %3").arg(begin[0]).arg(begin[1]).arg(begin[2])));
            // The break in the middle: the text after it is at the indent.
            QVERIFY2(std::abs(middle[1] - 40) < 0.01, qPrintable(QString::number(middle[1])));
        }
    }

    // A paragraph whose text and mark are both hidden takes no room (not a line, not its space); one whose mark is not hidden is an empty line.
    void aParagraphThatIsAllHiddenTakesNoRoom()
    {
        auto tops = [&](bool markHidden) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            QTextCharFormat hidden = run;
            hidden.setProperty(tp::HiddenText, true);
            QTextBlockFormat bf;
            bf.setTopMargin(10);
            bf.setBottomMargin(10);
            c.setBlockFormat(bf);
            c.insertText(QStringLiteral("before"), run);
            c.insertBlock(bf, markHidden ? hidden : run);
            c.insertText(QStringLiteral("Top of Form"), hidden);
            c.insertBlock(bf, run);
            c.insertText(QStringLiteral("after"), run);
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout lay;
            lay.build(&doc, {plainFrame(300, 300)}, env);
            const auto lines = lay.lineInfo(0);
            return std::make_pair(int(lines.size()), lines.last().rect.top() - lines.first().rect.bottom());
        };
        const auto gone = tops(true), line = tops(false);
        QCOMPARE(gone.first, 2);
        QVERIFY(std::abs(gone.second - 10) < 0.01);
        QCOMPARE(line.first, 3);
        QVERIFY(line.second > 20);
    }

    // The space between two paragraphs is the larger of the first's space after and the second's space before, not their sum; a paragraph that asks
    // for contextual spacing has none between it and a paragraph of its own style.
    void wordSpaceBetweenParagraphsIsTheLarger()
    {
        const QString tnr = QStringLiteral("Times New Roman");
        const double single = (1825 + 443 + 87) / 2048.0 * 12;
        auto gap = [&](double after, double before, bool word = true) {
            const auto lines = wordLinesOf({{tnr, 12}, {tnr, 12}}, [&](int i, QTextBlockFormat &bf) {
                if (i == 0) bf.setBottomMargin(after);
                else bf.setTopMargin(before);
            }, word);
            return lines[1].rect.top() - lines[0].rect.bottom();
        };
        QVERIFY(std::abs(gap(6, 12) - 12) < 0.01);
        QVERIFY(std::abs(gap(12, 6) - 12) < 0.01);
        QVERIFY(std::abs(gap(6, 6) - 6) < 0.01);
        QVERIFY(std::abs(gap(0, 9) - 9) < 0.01);
        QVERIFY(std::abs(gap(6, 12, false) - 18) < 0.01);   // (as a publication's are added)
        auto contextual = [&](const QString &second) {
            const auto lines = wordLinesOf({{tnr, 12}, {tnr, 12}}, [&](int i, QTextBlockFormat &bf) {
                bf.setTopMargin(10);
                bf.setBottomMargin(10);
                bf.setProperty(tp::StyleName, i == 0 ? QStringLiteral("Body") : second);
                bf.setProperty(tp::ContextualSpacing, true);
            });
            return lines[1].rect.top() - lines[0].rect.bottom();
        };
        QVERIFY(std::abs(contextual(QStringLiteral("Body"))) < 0.01);
        QVERIFY(std::abs(contextual(QStringLiteral("Other")) - 10) < 0.01);
        Q_UNUSED(single);
    }

    // A file that turns the web page's way of spacing paragraphs off (LayoutEnv::wordSpaceSums) has the space after a paragraph and the space before
    // the next both: their sum, as a publication's are.
    void wordSpaceBetweenParagraphsAddsWhenTheFileAsksForIt()
    {
        const QString tnr = QStringLiteral("Times New Roman");
        auto gap = [&](double after, double before, bool sums) {
            const auto lines = wordLinesOf({{tnr, 12}, {tnr, 12}}, [&](int i, QTextBlockFormat &bf) {
                if (i == 0) bf.setBottomMargin(after);
                else bf.setTopMargin(before);
            }, true, [&](LayoutEnv &env) { env.wordSpaceSums = sums; });
            return lines[1].rect.top() - lines[0].rect.bottom();
        };
        QVERIFY(std::abs(gap(12, 6, true) - 18) < 0.01);
        QVERIFY(std::abs(gap(6, 12, true) - 18) < 0.01);
        QVERIFY(std::abs(gap(6, 6, true) - 12) < 0.01);
        QVERIFY(std::abs(gap(12, 6, false) - 12) < 0.01);
        QVERIFY(std::abs(gap(6, 6, false) - 6) < 0.01);
        // (The layouts are kept apart: the same story is not found under the other's key.)
        LayoutEnv a, b;
        b.wordSpaceSums = true;
        QVERIFY(a.key() != b.key());
    }

    // A paragraph out of the flow (a frame's, in a word processor) is set where the next paragraph begins and takes none of the room.
    void aParagraphOutOfTheFlowTakesNoRoom()
    {
        const QString tnr = QStringLiteral("Times New Roman");
        auto lines = [&](bool out) {
            return wordLinesOf({{tnr, 12}, {tnr, 12}, {tnr, 12}}, [&](int i, QTextBlockFormat &bf) {
                bf.setBottomMargin(5);
                if (i == 1 && out) bf.setProperty(tp::OutOfFlow, true);
            });
        };
        const auto flowing = lines(false), floating = lines(true);
        QCOMPARE(floating.size(), 3);
        QVERIFY(std::abs(floating[1].rect.top() - floating[2].rect.top()) < 0.01);
        QVERIFY(std::abs(floating[2].rect.top() - flowing[1].rect.top()) < 0.01);
        QVERIFY(std::abs(floating[0].rect.top() - flowing[0].rect.top()) < 0.01);
        QVERIFY(floating[2].rect.top() < flowing[2].rect.top() - 10);
    }

    // A section's header and footer are drawn on its pages at the header and footer distances from the page's edge, the width of the text.
    void pagesDrawTheirHeaderAndFooter()
    {
        FlowDocument fd;
        fillStory(fd.story(), 60, 4);
        Section &s = fd.sections[0];
        s.headerDistance = 30;
        s.footerDistance = 20;
        const auto header = furnitureStory({QStringLiteral("HEADER ONE")}), footer = furnitureStory({QStringLiteral("FOOTER ONE"), QStringLiteral("FOOTER TWO")});
        s.header.odd = header;
        s.footer.odd = footer;
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() > 1);
        for (int k = 0; k < 2; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            const auto boxes = boxesByText(*page);
            QVERIFY(boxes.contains(QStringLiteral("HEADER ONE")));
            QVERIFY(boxes.contains(QStringLiteral("FOOTER ONE\nFOOTER TWO")));
            const TextItem *h = boxes[QStringLiteral("HEADER ONE")];
            QVERIFY(std::abs(h->rect.top() - 30) < 0.01);
            QVERIFY(std::abs(h->rect.left() - s.content().left()) < 0.01 && std::abs(h->rect.width() - s.content().width()) < 0.01);
            const double footerHeight = storyHeightOf(footer.get(), s.content().width(), fd);
            const TextItem *f = boxes[QStringLiteral("FOOTER ONE\nFOOTER TWO")];
            QVERIFY2(std::abs(f->rect.bottom() - (s.pageSize().height() - 20)) < 0.02, qPrintable(QString::number(f->rect.bottom())));
            QVERIFY2(std::abs(f->rect.height() - footerHeight) < 0.02, qPrintable(QStringLiteral("%1 vs %2").arg(f->rect.height()).arg(footerHeight)));
        }
        // (A page of a section with neither has no such boxes.)
        FlowDocument bare;
        fillStory(bare.story(), 5, 4);
        Paginator bp(&bare);
        const auto plain = bp.materialize(0);
        QCOMPARE(int(plain->doc->pages[0]->items.size()), 1);
    }

    // A header's first paragraph, when it is empty, keeps its line height and the size of its mark on the page: the empty line is as tall as the paginator measured it.
    void anEmptyFirstParagraphOfAHeaderKeepsItsMark()
    {
        FlowDocument fd;
        fillStory(fd.story(), 20, 4);
        auto header = std::make_shared<QTextDocument>();
        header->setDocumentMargin(0);
        {
            QTextCursor c(header.get());
            QTextCharFormat big, small;
            big.setFontPointSize(30);
            small.setFontPointSize(10);
            QTextBlockFormat tight;
            tight.setLineHeight(80, QTextBlockFormat::ProportionalHeight);
            c.setBlockFormat(tight);
            c.setBlockCharFormat(big);
            c.insertBlock(QTextBlockFormat(), small);
            c.insertText(QStringLiteral("HEADER TEXT"), small);
        }
        fd.sections[0].header.odd = header;
        fd.sections[0].headerDistance = 10;
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const TextItem *box = nullptr;
        for (const auto &item : page->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                if (const Story *st = page->doc->story(t->storyId))
                    if (st->doc->toPlainText().contains(QStringLiteral("HEADER TEXT"))) {
                        box = t;
                        QCOMPARE(st->doc->begin().charFormat().fontPointSize(), 30.0);
                        QCOMPARE(st->doc->begin().blockFormat().lineHeight(), 80.0);
                    }
        QVERIFY(box);
    }

    // Pages that an edit moves down by a page change from odd to even, and where odd and even pages have headers of other heights they are laid out again:
    // the pages after the edit are the ones a fresh paginator makes.
    void evenAndOddHeadersOfOtherHeightsSettleAfterAnEdit()
    {
        for (bool sameHeights : {false, true}) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 6 * 25, 7, [](int i, QTextBlockFormat &bf) { if (i % 25 == 0 && i > 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 40, 40);
            Section &s = fd.sections[0];
            s.differentOddEven = true;
            s.header.odd = furnitureStory({QStringLiteral("ODD")});
            s.header.even = sameHeights ? furnitureStory({QStringLiteral("EVEN")})
                                        : furnitureStory({QStringLiteral("EVEN 1"), QStringLiteral("EVEN 2"), QStringLiteral("EVEN 3"), QStringLiteral("EVEN 4"), QStringLiteral("EVEN 5"), QStringLiteral("EVEN 6")});
            Paginator pg(&fd);
            const int before = pg.pageCount();
            QTextCursor c(fd.story());
            c.setPosition(fd.story()->findBlockByNumber(2).position());
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            for (int i = 0; i < 14; ++i) {
                c.insertText(QStringLiteral("An added paragraph that is long enough to take two or three lines of the page when it is set at eleven points in this frame. "), cf);
                c.insertBlock();
            }
            QVERIFY(pg.pageCount() > before);
            const QString bad = differenceFromFresh(fd, pg, false);
            QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1 headers: %2").arg(sameHeights ? "same-height" : "different-height", bad)));
        }
    }

    // Edits of every kind to a document with headers for the first, odd, and even pages of other heights, notes (one of them longer than a page), sections that
    // are continuous and that are on pages of their own (with other columns), and page breaks: after each, the paginator that laid the document out before is
    // the paginator a fresh one is. Half of the documents have tables as well, one in each kind of section, with notes cited in their cells and rows that go over
    // page breaks; their edits also type in cells, put rows in and take them out, put section breaks in and take them out, replace headers and footers, and undo
    // and redo. The two paginators have the same pages: the page count, and for each page its start, lines, table rows, notes, headers and footers, and boxes.
    // (JP_FUZZ=n: seeds 1 to n, for a longer run; JP_FUZZ_TRACE=1: what each run did.)
    void incrementalPaginationMatchesAFreshOneAfterRandomEdits()
    {
        int compared = 0, runs = 0;
        QStringList failures;   // (a run that fails stops, and the next goes on: the report says which seeds fail)
        const unsigned seeds = qEnvironmentVariableIsSet("JP_FUZZ") ? qEnvironmentVariable("JP_FUZZ").toUInt() : 2u;
        for (int variant = 0; variant < 4; ++variant) {
            const bool restart = variant & 1, tables = variant & 2;
            for (unsigned seed = 1; seed <= seeds; ++seed) {
                std::mt19937 rng(seed * 7919u + (restart ? 1u : 0u) + (tables ? 1000003u : 0u));
                TestNotes notes;
                const int words[] = {3, 12, 60, 900, 4, 30};
                QStringList ids;
                for (int i = 0; i < 6; ++i) {
                    ids << QStringLiteral("n%1").arg(i);
                    addNote(&notes, ids.last(), words[i], QStringLiteral("NOTE%1").arg(i));
                }
                FlowDocument fd;
                fd.env.wordLines = true;
                fd.fields = &notes;
                fd.restartNotesInSections = restart;
                fillStory(fd.story(), 44, seed, [](int i, QTextBlockFormat &bf) {
                    bf.setBottomMargin(6);
                    if (i == 7 || i == 24 || i == 38) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
                    if (i == 15 || i == 30) bf.setProperty(flow::fp::SectionBreakAfter, true);
                    if (i % 3 == 0) bf.setProperty(tp::WidowControl, true);
                }, 20, 70);
                Section &a = fd.sections[0];
                a.differentFirstPage = a.differentOddEven = true;
                a.header.first = furnitureStory({QStringLiteral("FIRST")});
                a.header.odd = furnitureStory({QStringLiteral("ODD")});
                a.header.even = furnitureStory({QStringLiteral("EVEN 1"), QStringLiteral("EVEN 2"), QStringLiteral("EVEN 3"), QStringLiteral("EVEN 4")});
                a.footer.odd = furnitureStory({QStringLiteral("FOOT ODD")});
                a.footer.even = furnitureStory({QStringLiteral("FOOT EVEN 1"), QStringLiteral("FOOT EVEN 2")});
                Section b = a;
                b.continuous = true;
                b.columns = 2;
                b.columnGap = 20;
                Section c = a;
                c.columns = 1;
                c.differentFirstPage = false;
                c.margins = QMarginsF(90, 60, 54, 80);
                c.header.even = furnitureStory({QStringLiteral("C EVEN 1"), QStringLiteral("C EVEN 2"), QStringLiteral("C EVEN 3"), QStringLiteral("C EVEN 4"), QStringLiteral("C EVEN 5")});
                fd.sections << b << c;
                for (const int block : {40, 22, 5, 33, 12}) addReference(fd.story(), fd.story()->findBlockByNumber(block).position() + 4, ids[rng() % 6]);
                // Objects the text goes round, in each section and of each kind: a picture against the column's edge, one with no text beside it, one with text on its left only,
                // one from the page with the text on its larger side, one from a line above it, and a wide one across a column (the story's first paragraphs hold them: edits that
                // take their anchors away, and move them across pages, change which pages have walls).
                {
                    using Wr = FloatingObject::Wrap;
                    using Sd = FloatingObject::Side;
                    using Rl = FloatingObject::Rel;
                    struct Wall { int block; double w, h; Wr wrap; Sd side; Rl h0, v0; double dx, dy; FloatingObject::Align align; };
                    for (const Wall &w : {Wall{2, 120, 80, Wr::Square, Sd::Both, Rl::Column, Rl::Paragraph, 0, 0, FloatingObject::Align::End},
                                          Wall{9, 100, 40, Wr::TopBottom, Sd::Both, Rl::Column, Rl::Paragraph, 60, 0, FloatingObject::Align::None},
                                          Wall{14, 90, 70, Wr::Square, Sd::Left, Rl::Margin, Rl::Paragraph, 150, 10, FloatingObject::Align::None},
                                          Wall{20, 110, 100, Wr::Square, Sd::Largest, Rl::Page, Rl::Page, 300, 200, FloatingObject::Align::None},
                                          Wall{26, 80, 60, Wr::Square, Sd::Both, Rl::Column, Rl::Line, 0, -20, FloatingObject::Align::End},
                                          Wall{33, 70, 90, Wr::Square, Sd::Right, Rl::Column, Rl::Paragraph, 0, 5, FloatingObject::Align::Center},
                                          Wall{41, 300, 30, Wr::TopBottom, Sd::Both, Rl::Column, Rl::Paragraph, 20, 0, FloatingObject::Align::None}}) {
                        const int id = addWrapped(&fd, w.w, w.h, fd.story()->findBlockByNumber(w.block).position() + 1, w.wrap, w.dx, w.dy, QMarginsF(8, 4, 8, 4), w.side, w.h0, w.v0);
                        fd.objects[size_t(id) - 1].hAlign = w.align;
                    }
                }
                if (tables) {
                    // A table in each section (the last first, so that the blocks the others are after stay where they are), each with a note cited in a cell and a row taller than the rest.
                    struct Put { int after, rows, cols; double width; const char *note; int row; };
                    for (const Put &p : {Put{36, 18, 3, 140, "n2", 9}, Put{20, 7, 2, 100, "n0", 3}, Put{3, 6, 2, 110, "n1", 2}}) {
                        TestTable spec;
                        spec.rows = p.rows;
                        spec.cols = p.cols;
                        spec.columnWidth = p.width;
                        spec.setRow = [](int r, RowData &row) { row.header = r == 0; };
                        spec.text = [](int r, int col) { return r % 4 == 2 && col == 1 ? QStringLiteral("a taller\ncell of\nthree lines") : QStringLiteral("r%1c%2 words").arg(r).arg(col); };
                        const TestTable made = addTableAfter(fd, p.after, spec);
                        addReference(fd.story(), made.rowStarts[p.row] + 2, QString::fromLatin1(p.note));
                    }
                }
                // Frames, in each section: set out of the text, at the paragraph after them (the first and last of the block numbers asked for that are not in a table: a frame
                // between the paragraphs of a cell would be no frame of the text's). Edits that go into them, cut them, or move their paragraph from one page to another change
                // which pages have them.
                {
                    FloatingObject proto;
                    proto.hRel = FloatingObject::Rel::Margin;
                    proto.vRel = FloatingObject::Rel::Paragraph;
                    proto.wrap = FloatingObject::Wrap::Square;
                    proto.gap = QMarginsF(6, 3, 6, 3);
                    int made = 0;
                    for (const int wanted : {50, 30, 12, 4}) {
                        int block = std::min(wanted, fd.story()->blockCount() - 1);   // (a story of 44 paragraphs has no block 50: the frame would go in at the story's start)
                        while (block > 1 && (fd.story()->findBlockByNumber(block).blockFormat().intProperty(fp::Cell) || fd.story()->findBlockByNumber(block - 1).blockFormat().intProperty(fp::Cell) ||
                                             fd.story()->findBlockByNumber(block - 1).blockFormat().intProperty(fp::Frame)))
                            --block;
                        if (block <= 1) continue;
                        FloatingObject o = proto;
                        o.hAlign = made % 2 ? FloatingObject::Align::Start : FloatingObject::Align::End;
                        o.dy = 2 * made;
                        addFrame(fd, block, {QStringLiteral("FRAME %1 first words of it").arg(made), QStringLiteral("and a second paragraph of the frame, longer, so that it takes more than a line of it")}, o, made == 2 ? 0 : 110 + 10 * made, made == 3 ? 90 : 0, made == 1);
                        ++made;
                    }
                    QVERIFY(made >= 3);
                }
                if (tables) {
                    // Tables set at places, with the text going round them: between ordinary paragraphs, one at the start of a column's text and one nearer the end. Edits in their
                    // cells, and around them, change which pages draw them and how tall each is.
                    int made = 0;
                    auto plainBlock = [&](int i) { const QTextBlockFormat f = fd.story()->findBlockByNumber(i).blockFormat(); return !f.intProperty(fp::Cell) && !f.intProperty(fp::Frame); };
                    QVector<int> candidates;
                    for (int i = 1; i + 1 < fd.story()->blockCount(); ++i)
                        if (plainBlock(i - 1) && plainBlock(i) && plainBlock(i + 1)) candidates << i;
                    QVERIFY(candidates.size() >= 6);
                    // (From the last to the first, so that the numbers of the blocks before stay.)
                    for (const int k : {3, 2, 1}) {
                        const int n = candidates[candidates.size() * k / 4];
                        TestTable spec;
                        spec.rows = 2 + made;
                        spec.cols = 2;
                        spec.columnWidth = 50 + 10 * made;
                        spec.setTable = [made](TableData &t) {
                            TableData::Place p;
                            p.across = made == 1 ? TableData::Place::From::Page : TableData::Place::From::Margin;
                            p.acrossSpec = made == 1 ? TableData::Place::Spec::None : (made == 0 ? TableData::Place::Spec::End : TableData::Place::Spec::Start);
                            p.x = made == 1 ? 120 : 0;
                            p.y = 3 * made;
                            p.room = QMarginsF(6, 2, 6, 2);
                            t.place = p;
                        };
                        addTableAfter(fd, n, spec);
                        ++made;
                    }
                    QVERIFY(made >= 2);
                }
                if (tables) fd.story()->setUndoRedoEnabled(true);   // (undo and redo are of the edits below)
                Paginator pg(&fd);
                QVERIFY(pg.pageCount() > 4);
                QString trace;
                std::vector<std::shared_ptr<QTextDocument>> kept;   // (replaced headers stay alive: a new story at the address of a freed one has the paginator's measures of the old)
                const int steps = tables ? 14 : 12;
                for (int step = 0; step < steps; ++step) {
                    QTextCursor cur(fd.story());
                    const QString was = startsText(pg);
                    const int last = std::max(1, fd.story()->characterCount() - 1);
                    const int at = int(rng() % unsigned(last));
                    const QTextBlock blk = fd.story()->findBlock(at);
                    const bool flagged = blk.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
                    const bool inCell = blk.blockFormat().intProperty(fp::Cell) != 0;
                    QTextCharFormat cf;
                    cf.setFontPointSize(11);
                    const unsigned what = rng() % (tables ? 13u : 6u);
                    cur.setPosition(at);
                    if (what == 0 || what == 1) {
                        QString text;
                        for (unsigned w = 0, n = 3 + rng() % 60; w < n; ++w) text += QStringLiteral("word%1 ").arg(rng() % 100);
                        cur.insertText(text, cf);
                        trace += QStringLiteral("text@%1 ").arg(at);
                    } else if (what == 2) {
                        const int len = std::min<int>(int(rng() % 400), blk.position() + blk.length() - 1 - at);
                        if (len > 0) {
                            cur.setPosition(at + len, QTextCursor::KeepAnchor);
                            cur.removeSelectedText();
                        }
                        trace += QStringLiteral("cut@%1+%2 ").arg(at).arg(len);
                    } else if (what == 3 && !flagged) {
                        cur.setPosition(blk.position() + blk.length() - 1);
                        QTextBlockFormat nf;
                        nf.setBottomMargin(6);
                        if (inCell) nf.setProperty(fp::Cell, blk.blockFormat().intProperty(fp::Cell));   // (Enter in a cell: another paragraph of the cell)
                        cur.insertBlock(nf, cf);
                        QString text;
                        for (unsigned w = 0, n = 10 + rng() % 70; w < n; ++w) text += QStringLiteral("para%1 ").arg(rng() % 100);
                        cur.insertText(text, cf);
                        trace += QStringLiteral("para@%1 ").arg(blk.position());
                    } else if (what == 4 && blk.position() > 0 && !flagged && !inCell) {
                        QTextBlockFormat bf = blk.blockFormat();
                        bf.setPageBreakPolicy(bf.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore ? QTextFormat::PageBreak_Auto : QTextFormat::PageBreak_AlwaysBefore);
                        cur.setPosition(blk.position());
                        cur.setBlockFormat(bf);
                        trace += QStringLiteral("break@%1 ").arg(blk.position());
                    } else if (what == 6) {
                        // Typing in a cell.
                        QVector<QTextBlock> cells;
                        for (QTextBlock x = fd.story()->begin(); x.isValid(); x = x.next())
                            if (x.blockFormat().intProperty(fp::Cell)) cells << x;
                        if (!cells.isEmpty()) {
                            const QTextBlock cell = cells[int(rng() % unsigned(cells.size()))];
                            const int where = cell.position() + int(rng() % unsigned(cell.length()));
                            cur.setPosition(where);
                            cur.insertText(QStringLiteral(" typed in a cell with several words "), cf);
                            trace += QStringLiteral("cell@%1 ").arg(where);
                        }
                    } else if (what == 7 || what == 8) {
                        // A row put in, or taken out (of a table that keeps two).
                        const QVector<RowRange> rows = rowRangesOf(fd);
                        if (!rows.isEmpty()) {
                            const RowRange r = rows[int(rng() % unsigned(rows.size()))];
                            int inTable = 0;
                            for (const RowRange &o : rows) inTable += o.table == r.table;
                            if (what == 7) {
                                copyRowOf(fd, r);
                                trace += QStringLiteral("row+@%1 ").arg(r.from);
                            } else if (inTable > 2) {
                                cur.setPosition(r.from - 1);
                                cur.setPosition(r.to - 1, QTextCursor::KeepAnchor);
                                cur.removeSelectedText();
                                trace += QStringLiteral("row-@%1 ").arg(r.from);
                            }
                        }
                    } else if (what == 9) {
                        // A section break put in after a paragraph, or the one it has taken out; the sections are told as a word processor tells them.
                        if (!inCell && blk.next().isValid()) {
                            int s = 0;
                            for (QTextBlock x = fd.story()->begin(); x != blk; x = x.next()) s += x.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
                            QTextBlockFormat bf = blk.blockFormat();
                            bool changedSections = true;
                            if (flagged) {
                                bf.clearProperty(flow::fp::SectionBreakAfter);
                                cur.setPosition(blk.position());
                                cur.setBlockFormat(bf);
                                if (s + 1 < fd.sections.size()) fd.sections.remove(s + 1);
                                trace += QStringLiteral("unsection@%1 ").arg(blk.position());
                            } else if (fd.sections.size() < 6) {
                                bf.setProperty(flow::fp::SectionBreakAfter, true);
                                cur.setPosition(blk.position());
                                cur.setBlockFormat(bf);
                                Section made = fd.sections[std::min<int>(s, fd.sections.size() - 1)];
                                made.continuous = false;
                                fd.sections.insert(s + 1, made);
                                trace += QStringLiteral("section@%1 ").arg(blk.position());
                            } else {
                                changedSections = false;
                            }
                            if (changedSections) {
                                fd.story()->clearUndoRedoStacks();   // (the sections are not in it)
                                pg.invalidateFrom(s);
                            }
                        }
                    } else if (what == 10) {
                        // A header or a footer replaced.
                        const int s = int(rng() % unsigned(fd.sections.size())), slot = int(rng() % 6u);
                        QStringList lines;
                        for (unsigned i = 0, n = 1 + rng() % 6; i < n; ++i) lines << QStringLiteral("NEW %1 LINE %2").arg(slot).arg(i);
                        auto story = furnitureStory(lines);
                        HeaderFooter &hf = slot < 3 ? fd.sections[s].header : fd.sections[s].footer;
                        std::shared_ptr<QTextDocument> &target = slot % 3 == 0 ? hf.first : slot % 3 == 1 ? hf.odd : hf.even;
                        kept.push_back(target);
                        kept.push_back(story);
                        target = story;
                        pg.invalidateFrom(s);
                        trace += QStringLiteral("header%1/%2 ").arg(s).arg(slot);
                    } else if (what == 11 || what == 12) {
                        if (what == 11 && fd.story()->isUndoAvailable()) {
                            fd.story()->undo();
                            trace += QStringLiteral("undo ");
                        } else if (what == 12 && fd.story()->isRedoAvailable()) {
                            fd.story()->redo();
                            trace += QStringLiteral("redo ");
                        }
                    } else {
                        addReference(fd.story(), at, ids[rng() % 6]);
                        trace += QStringLiteral("note@%1 ").arg(at);
                    }
                    if (rng() % 3 == 0) pg.paginate(int(rng() % 4));   // (some of it, now)
                    QString refsText;
                    for (QTextBlock rb = fd.story()->begin(); rb.isValid(); rb = rb.next())
                        for (auto rit = rb.begin(); !rit.atEnd(); ++rit)
                            if (rit.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("footnote:"))) refsText += QStringLiteral("%1:%2 ").arg(rit.fragment().position()).arg(rit.fragment().charFormat().stringProperty(tp::Field).mid(9));
                    Paginator fresh(&fd);
                    const QString bad0 = audit::differenceFromFresh(pg, fresh, step % 2 == 1 || step == steps - 1);   // (the pages made, every other edit: that costs more than their lines)
                    const QString bad = bad0.isEmpty() ? bad0 : bad0 + QStringLiteral(" refs [") + refsText + QLatin1Char(']');
                    ++compared;
                    // The text goes round the objects as the page layer draws it, on every page that has them (every third edit: the pages are made for it).
                    const QString walls = bad.isEmpty() && step % 3 == 2 ? audit::wallProblems(pg, fd.story()) : QString();
                    // The text on the pages is the story's text, whole: the walls, frames, and floating tables take room from lines and move them, and keep none from the pages.
                    if (const QString lost = bad.isEmpty() ? audit::coverageProblems(pg, fd.story()) : QString(); !lost.isEmpty()) {
                        failures << QStringLiteral("%1 restart %2, seed %3, step %4 (%5): text: %6").arg(tables ? "tables," : "plain,").arg(restart).arg(seed).arg(step).arg(trace, lost);
                        break;
                    }
                    if (!walls.isEmpty()) {
                        failures << QStringLiteral("%1 restart %2, seed %3, step %4 (%5): walls: %6").arg(tables ? "tables," : "plain,").arg(restart).arg(seed).arg(step).arg(trace, walls);
                        break;
                    }
                    // Every frame and every table set at a place is drawn once, or is in the text.
                    if (const QString floats = bad.isEmpty() && step % 3 == 1 ? audit::floatProblems(pg, fd) : QString(); !floats.isEmpty()) {
                        failures << QStringLiteral("%1 restart %2, seed %3, step %4 (%5): floats: %6").arg(tables ? "tables," : "plain,").arg(restart).arg(seed).arg(step).arg(trace, floats);
                        break;
                    }
                    if (!bad.isEmpty()) {
                        failures << QStringLiteral("%1 restart %2, seed %3, step %4 (%5): %6 (before the last edit: %7)").arg(tables ? "tables," : "plain,").arg(restart).arg(seed).arg(step).arg(trace, bad, was.left(160));
                        break;   // (the next run)
                    }
                }
                ++runs;
                if (qEnvironmentVariableIsSet("JP_FUZZ_TRACE")) qInfo("%s restart %d seed %u: %d pages, %d table rows; %s", tables ? "tables" : "plain", int(restart), seed, pg.pageCount(), int(rowRangesOf(fd).size()), qPrintable(trace));
            }
        }
        QVERIFY2(failures.isEmpty(), qPrintable(QStringLiteral("%1 of %2 runs differ from a fresh paginator:\n%3").arg(failures.size()).arg(runs).arg(failures.join(QLatin1Char('\n')))));
        QVERIFY(compared >= 8 * 12);
    }

    // ---- tables with the headers, footers, notes, and sections of a word processor's document (the two waves of JeffWrite together)

    // A note cited in the paragraph before a table, one cited in a cell of it, and one in the text after it (each alone, then the three together): each is drawn on the page
    // whose lines cite it, below the rows, at the number its reference has.
    void notesCitedAroundATableAreOnThePagesThatCiteThem_data()
    {
        QTest::addColumn<bool>("before");
        QTest::addColumn<bool>("cell");
        QTest::addColumn<bool>("after");
        QTest::newRow("in the paragraph before the table") << true << false << false;
        QTest::newRow("in a cell") << false << true << false;
        QTest::newRow("in the text after the table") << false << false << true;
        QTest::newRow("all three") << true << true << true;
    }

    void notesCitedAroundATableAreOnThePagesThatCiteThem()
    {
        QFETCH(bool, before);
        QFETCH(bool, cell);
        QFETCH(bool, after);
        TestNotes notes;
        addNote(&notes, QStringLiteral("before"), 14, QStringLiteral("NOTEBEFORE"));
        addNote(&notes, QStringLiteral("cell"), 30, QStringLiteral("NOTECELL"));
        addNote(&notes, QStringLiteral("after"), 9, QStringLiteral("NOTEAFTER"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fillStory(fd.story(), 5, 3, {}, 30, 50);
        TestTable spec;
        spec.rows = 110;
        spec.cols = 2;
        spec.columnWidth = 150;
        const TestTable table = addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("The paragraph after the table, which may cite a note."));
        appendParagraph(fd, QStringLiteral("The last paragraph."));
        if (after) addReference(fd.story(), fd.story()->findBlockByNumber(fd.story()->blockCount() - 2).position() + 20, QStringLiteral("after"));
        if (cell) addReference(fd.story(), table.rowStarts[45] + 2, QStringLiteral("cell"));
        if (before) addReference(fd.story(), fd.story()->findBlockByNumber(4).position() + 25, QStringLiteral("before"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 3);
        QMap<QString, int> pageOf;
        for (int k = 0; k < pages; ++k)
            for (const QString &id : audit::citedNotes(pg, fd.story(), k)) pageOf[id] = k;
        QCOMPARE(int(pageOf.size()), int(before) + int(cell) + int(after));
        // (All three are on three pages: the paragraph before the table on the first, the cell far down the table, the paragraph after it on the last.)
        if (before && cell && after)
            QVERIFY2(pageOf["before"] < pageOf["cell"] && pageOf["cell"] < pageOf["after"], qPrintable(QStringLiteral("the notes are on the pages %1 %2 %3").arg(pageOf["before"]).arg(pageOf["cell"]).arg(pageOf["after"])));
        const QMap<QString, QString> marks{{"before", "NOTEBEFORE"}, {"cell", "NOTECELL"}, {"after", "NOTEAFTER"}};
        const QString bad = audit::problems(pg, fd.story(), marks);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // Headers and footers of the first, odd, and even pages are of other heights, and a table runs across several pages: its rows stay in the text area that the header and
    // footer leave on each, none over the footer, and each page holds as many rows as its own area has room for.
    void tableRowsStayInTheBodyThatHeadersAndFootersLeave()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 2, 5, {}, 10, 20);
        TestTable spec;
        spec.rows = 150;
        spec.cols = 2;
        spec.columnWidth = 140;
        spec.text = [](int r, int c) { return r % 9 == 4 ? QStringLiteral("r%1c%2\nand a second\nand a third line").arg(r).arg(c) : QStringLiteral("r%1c%2").arg(r).arg(c); };
        const TestTable table = addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("After the table."));
        Section &s = fd.sections[0];
        s.differentFirstPage = s.differentOddEven = true;
        s.header.first = furnitureStory({QStringLiteral("FIRST HEAD")});
        s.header.odd = furnitureStory({QStringLiteral("ODD HEAD")});
        s.header.even = furnitureStory({QStringLiteral("EVEN HEAD 1"), QStringLiteral("EVEN HEAD 2"), QStringLiteral("EVEN HEAD 3"), QStringLiteral("EVEN HEAD 4"), QStringLiteral("EVEN HEAD 5"), QStringLiteral("EVEN HEAD 6")});
        s.footer.first = furnitureStory({QStringLiteral("FIRST FOOT 1"), QStringLiteral("FIRST FOOT 2"), QStringLiteral("FIRST FOOT 3")});
        s.footer.odd = furnitureStory({QStringLiteral("ODD FOOT")});
        s.footer.even = furnitureStory({QStringLiteral("EVEN FOOT 1"), QStringLiteral("EVEN FOOT 2"), QStringLiteral("EVEN FOOT 3"), QStringLiteral("EVEN FOOT 4"), QStringLiteral("EVEN FOOT 5")});
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 6);
        QSet<int> heights;
        for (int k = 0; k < pages; ++k) heights << int(std::lround(pg.page(k).content.height() * 10));
        QVERIFY2(heights.size() >= 3, "the pages' text areas are of three heights (the first page's, the odd pages', the even pages')");
        const QString bad = audit::problems(pg, fd.story());
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        // Each page's furniture is the one its number and place say, and its rows end above the footer.
        QVector<int> seen;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            const QString want = k == 0 ? QStringLiteral("FIRST") : (k + 1) % 2 == 0 ? QStringLiteral("EVEN") : QStringLiteral("ODD");
            const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box")), foots = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
            QVERIFY2(heads.size() == 1 && foots.size() == 1, qPrintable(QString::number(k)));
            QVERIFY2(audit::storyOf(*page, heads[0]).startsWith(want + QStringLiteral(" HEAD")), qPrintable(QStringLiteral("page %1: %2").arg(k).arg(audit::storyOf(*page, heads[0]))));
            QVERIFY2(audit::storyOf(*page, foots[0]).startsWith(want + QStringLiteral(" FOOT")), qPrintable(QStringLiteral("page %1: %2").arg(k).arg(audit::storyOf(*page, foots[0]))));
            const double room = pg.page(k).content.height();
            const auto rows = rowLines(pg, fd, k);
            for (const auto &r : rows) seen << r.pos;
            if (!rows.isEmpty()) {
                QVERIFY2(rows.last().rect.bottom() <= room + 0.1, qPrintable(QStringLiteral("page %1: rows go down to %2 of %3").arg(k).arg(rows.last().rect.bottom()).arg(room)));
                const auto next = k + 1 < pages ? rowLines(pg, fd, k + 1) : QVector<StoryLayout::LineBrief>();
                if (!next.isEmpty() && next.first().pos == pg.lines(k + 1).first().pos)
                    QVERIFY2(rows.last().rect.bottom() + next.first().rect.height() > room + 0.05, qPrintable(QStringLiteral("page %1 leaves room for the next row").arg(k)));
            }
        }
        QCOMPARE(seen, table.rowStarts);
    }

    // A continuous section of two columns that holds a table: it begins below the text of the section before, the table is inside a column and flows into the next, and
    // the page's note (cited in the text above the table, in the first column) and furniture keep to their places.
    void aTableInAContinuousTwoColumnSectionIsInsideItsColumnBelowTheTextAbove()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("n"), 20, QStringLiteral("NOTECOL"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fillStory(fd.story(), 6, 7, [](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(6);
            if (i == 2) bf.setProperty(fp::SectionBreakAfter, true);
        }, 8, 12);
        TestTable spec;
        spec.rows = 90;
        spec.cols = 2;
        spec.columnWidth = 100;
        const TestTable table = addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("After the table, in the second section."));
        addReference(fd.story(), fd.story()->findBlockByNumber(4).position() + 5, QStringLiteral("n"));   // (in the second section, in the text before the table)
        Section second = fd.sections[0];
        second.continuous = true;
        second.columns = 2;
        second.columnGap = 20;
        fd.sections << second;
        fd.sections[0].header.odd = furnitureStory({QStringLiteral("HEADER")});
        fd.sections[0].footer.odd = furnitureStory({QStringLiteral("FOOTER A"), QStringLiteral("FOOTER B")});
        fd.sections[1].header.odd = fd.sections[0].header.odd;
        fd.sections[1].footer.odd = fd.sections[0].footer.odd;
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 2);
        const Section &sec = fd.section(1);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        // The first section's lines end where the second begins (the lines of its frame), and the table's first rows are in its first column, below them.
        const auto lines = pg.lines(0);
        double split = 0;
        for (const auto &l : lines)
            if (l.frame == 0) split = std::max(split, l.rect.bottom());
        QVERIFY(split > 0);
        QVERIFY(!page->tables.isEmpty());
        const double top = pg.page(0).content.top();
        int tablesChecked = 0;
        for (int k = 0; k < pages; ++k) {
            const auto m = pg.materialize(k);
            QVERIFY(m);
            for (const auto &t : m->tables) {
                const QRectF r = t->rect();
                bool inAColumn = false;
                for (int col = 0; col < sec.columnCount(); ++col) {
                    const QRectF c = sec.columnRect(col);
                    inAColumn |= r.left() >= c.left() - 0.1 && r.right() <= c.right() + 0.1;
                }
                QVERIFY2(inAColumn, qPrintable(QStringLiteral("page %1: the table at %2 is not inside a column of the section").arg(k).arg(audit::rectText(r))));
                if (k == 0) QVERIFY2(r.top() >= top + split - 0.1, qPrintable(QStringLiteral("page 0: the table begins at %1, the section before it ends at %2").arg(r.top()).arg(top + split)));
                ++tablesChecked;
            }
        }
        QVERIFY(tablesChecked >= 2);
        // The table goes from the first column to the second, which begins where the section's columns do (below the text of the section before).
        const auto &fragments = page->tables;
        QVERIFY2(fragments.size() == 2, qPrintable(QString::number(fragments.size())));
        QVERIFY(fragments[1]->rect().left() > fragments[0]->rect().right() - 0.1);
        QVERIFY2(std::abs(fragments[1]->rect().top() - (top + split)) < 0.1, qPrintable(QStringLiteral("the table's first part is at %1, its second at %2, the columns begin at %3")
                                                                                         .arg(audit::rectText(fragments[0]->rect()), audit::rectText(fragments[1]->rect())).arg(top + split)));
        QVector<int> seen, wanted;
        for (const RowRange &r : rowRangesOf(fd)) wanted << r.from;   // (the references put into cells moved the rows on)
        for (int k = 0; k < pages; ++k)
            for (const auto &r : rowLines(pg, fd, k)) seen << r.pos;
        QVERIFY2(seen == wanted, qPrintable(QStringLiteral("the rows on the pages begin at [%1], the table's rows at [%2]").arg(intsText(seen), intsText(wanted))));
        const QString bad = audit::problems(pg, fd.story(), {{"n", "NOTECOL"}});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A note longer than the pages, cited from a cell of a table: it goes on over pages of its own, which are made and drawn, and the end of it is on the last of them.
    void aNoteLongerThanThePageCitedFromACellHasPagesOfItsOwn()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("long"), 1500, QStringLiteral("LONGNOTE"));
        addNote(&notes, QStringLiteral("short"), 10, QStringLiteral("SHORTNOTE"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fillStory(fd.story(), 2, 4, {}, 20, 30);
        TestTable spec;
        spec.rows = 8;
        spec.cols = 2;
        spec.columnWidth = 150;
        const TestTable table = addTable(fd, spec);
        for (int i = 0; i < 5; ++i) appendParagraph(fd, QStringLiteral("A paragraph after the table, number %1, with a few words in it.").arg(i));
        addReference(fd.story(), fd.story()->findBlockByNumber(fd.story()->blockCount() - 2).position() + 10, QStringLiteral("short"));
        addReference(fd.story(), table.rowStarts[3] + 2, QStringLiteral("long"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        int noteOnly = 0;
        QString drawn;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY2(page, qPrintable(QStringLiteral("page %1 of %2 isn't made").arg(k).arg(pages)));
            if (pg.lines(k).isEmpty()) {
                ++noteOnly;
                QVERIFY2(!page->doc->pages[0]->items.empty(), qPrintable(QStringLiteral("page %1 of notes alone has nothing drawn").arg(k)));
            }
            for (const auto &item : page->doc->pages[0]->items) drawn += audit::storyOf(*page, item.get()) + QLatin1Char('\n');
        }
        QVERIFY2(noteOnly > 0, qPrintable(QStringLiteral("%1 pages, none of notes alone").arg(pages)));
        // (A box's story holds the whole rest of its note, whatever the box shows of it: what is drawn is the height of the boxes.)
        const QString lost = audit::lostNotes(pg, audit::fullNoteHeight(notes.docs["long"].get(), pg.page(0).content.width(), fd.env) + audit::fullNoteHeight(notes.docs["short"].get(), pg.page(0).content.width(), fd.env));
        QVERIFY2(lost.isEmpty(), qPrintable(lost));
        QVERIFY2(drawn.contains(QStringLiteral("SHORTNOTE")), "the short note is drawn");
        QVector<int> seen, wanted;
        for (const RowRange &r : rowRangesOf(fd)) wanted << r.from;   // (the references put into cells moved the rows on)
        for (int k = 0; k < pages; ++k)
            for (const auto &r : rowLines(pg, fd, k)) seen << r.pos;
        QCOMPARE(seen, wanted);
        const QString bad = audit::problems(pg, fd.story(), {{"long", "LONGNOTE"}, {"short", "SHORTNOTE"}});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // Under a word processor's line heights (several kinds of spacing and line height before the table), the stand-in for the table in the page's text, a paragraph of a
    // fixed height, does not move the table or the text after it: the table begins where the text before it ends (and its space after), and the page's text has every line
    // where the paginator put it.
    void aTableIsNotShiftedByItsPlaceholderUnderProcessorLineHeights()
    {
        struct Case { const char *name; QTextBlockFormat::LineHeightTypes type; double height, after, before; bool bordered = false; };
        const Case cases[] = {{"single, 8 after", QTextBlockFormat::SingleHeight, 0, 8, 0},
                              {"115 percent, 10 after", QTextBlockFormat::ProportionalHeight, 115, 10, 0},
                              {"exactly 20, none after", QTextBlockFormat::FixedHeight, 20, 0, 0},
                              {"150 percent, 4 after, 12 before", QTextBlockFormat::ProportionalHeight, 150, 4, 12},
                              {"at least 30, 6 after", QTextBlockFormat::MinimumHeight, 30, 6, 0},
                              {"a border below, 6 after", QTextBlockFormat::SingleHeight, 0, 6, 0, true}};
        tp::ParaBox box;
        box.top = {QStringLiteral("single"), 4, 2, QColor()};
        box.bottom = {QStringLiteral("single"), 8, 3, QColor()};
        for (const bool withTable : {false, true})
            for (const Case &cs : cases) {
                FlowDocument fd;
                fd.env.wordLines = true;
                fillStory(fd.story(), 3, 9, [&](int i, QTextBlockFormat &bf) {
                    bf.setBottomMargin(i == 2 ? cs.after : 6);
                    if (i == 2) {
                        bf.setTopMargin(cs.before);
                        if (cs.type != QTextBlockFormat::SingleHeight) bf.setLineHeight(cs.height, cs.type);
                        if (cs.bordered) bf.setProperty(tp::Borders, box.spec());
                    }
                }, 8, 12);
                TestTable spec;
                spec.rows = 3;
                spec.cols = 2;
                spec.columnWidth = 150;
                if (withTable) addTable(fd, spec);
                appendParagraph(fd, QStringLiteral("After the table."), [](QTextBlockFormat &bf) { bf.setLineHeight(130, QTextBlockFormat::ProportionalHeight); });
                appendParagraph(fd, QStringLiteral("And one more paragraph, the last."));
                Paginator pg(&fd);
                const QString what = QStringLiteral("%1, %2").arg(QLatin1String(cs.name), withTable ? "with the table" : "without it");
                QCOMPARE(pg.pageCount(), 1);
                const auto lines = pg.lines(0);
                if (withTable) {
                    const auto rows = rowLines(pg, fd, 0);
                    QCOMPARE(rows.size(), 3);
                    int firstRow = 0;
                    while (lines[firstRow].pos != rows[0].pos) ++firstRow;
                    // The text before the table ends, with the space after its last paragraph, where the table begins.
                    // (A paragraph with a border below has the room of the border's lower edge under its last line, before the space after it.)
                    const double after = formatAt(fd, lines[firstRow - 1].pos).bottomMargin() + (cs.bordered ? box.bottom.reserve() : 0.0);
                    QVERIFY2(std::abs(lines[firstRow - 1].rect.bottom() + std::max(0.0, after) - rows[0].rect.top()) < 0.05, qPrintable(what + ": the table's top and the end of the text before it"));
                    const auto page = pg.materialize(0);
                    QVERIFY(page && page->tables.size() == 1);
                    QVERIFY2(std::abs(page->tables[0]->rect().top() - pg.page(0).content.top() - rows[0].rect.top()) < 0.05, qPrintable(what + ": the table is drawn where its rows are"));
                    QVERIFY2(std::abs(page->tables[0]->height - (rows.last().rect.bottom() - rows[0].rect.top())) < 0.05, qPrintable(what + ": the table is as tall as its rows"));
                }
                const QString bad = audit::placementProblems(pg, fd, 0);
                QVERIFY2(bad.isEmpty(), qPrintable(what + QStringLiteral(": ") + bad));
                if (cs.before > 0 && !withTable) {
                    // (The check is able to tell: the lines laid out as the page layer would with other settings are not where the paginator put them.)
                    LayoutEnv other = fd.env;
                    other.wordLines = false;
                    QVERIFY2(!audit::placementProblems(pg, fd, 0, &other).isEmpty(), qPrintable(what + ": the check sees no difference"));
                }
            }
    }

    // A footer with the page's number and the document's page count, on every page of a document whose table spans pages, shows them right, and the pages made after an
    // edit that adds rows show the new count.
    void pageFieldsInAFooterKeepRightForATableThatSpansPages()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 2, 6, {}, 10, 14);
        TestTable spec;
        spec.rows = 140;
        spec.cols = 2;
        spec.columnWidth = 120;
        spec.setRow = [](int r, RowData &row) { row.header = r == 0; };
        addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("After the table."));
        fd.sections[0].footer.odd = footerWithPageFields({QStringLiteral("A second line of the footer")});
        Paginator pg(&fd);
        auto checkPages = [&](const QString &when) -> QString {
            const int pages = pg.pageCount();
            for (int k = 0; k < pages; ++k) {
                const auto page = pg.materialize(k);
                if (!page) return QStringLiteral("%1: page %2 is not made").arg(when).arg(k);
                const auto foots = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
                if (foots.size() != 1) return QStringLiteral("%1: page %2 has %3 footers").arg(when).arg(k).arg(foots.size());
                FieldContext ctx;
                ctx.doc = page->doc.get();
                if (ctx.resolve(QStringLiteral("page")) != QString::number(k + 1)) return QStringLiteral("%1: page %2 shows the number %3").arg(when).arg(k).arg(ctx.resolve(QStringLiteral("page")));
                if (ctx.resolve(QStringLiteral("pages")) != QString::number(pages)) return QStringLiteral("%1: page %2 shows %3 pages, the document has %4").arg(when).arg(k).arg(ctx.resolve(QStringLiteral("pages"))).arg(pages);
                if (!page->showsTotal || page->pageTotal != pages) return QStringLiteral("%1: page %2 is made with the total %3 (it shows it: %4), the document has %5").arg(when).arg(k).arg(page->pageTotal).arg(page->showsTotal).arg(pages);
            }
            return audit::problems(pg, fd.story());
        };
        const int before = pg.pageCount();
        QVERIFY(before >= 4);
        QString bad = checkPages(QStringLiteral("as made"));
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        // Rows put in near the top of the table: it has more pages, and every page that was made before shows the count it had.
        const auto old = pg.materialize(before - 1);
        QVERIFY(old);
        for (int i = 0; i < 25; ++i) QVERIFY(copyRowOf(fd, rowRangesOf(fd)[3]));
        QVERIFY(pg.pageCount() > before);
        bad = checkPages(QStringLiteral("after the rows"));
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        Paginator fresh(&fd);
        bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A row that goes over page breaks, under headers and footers that take room from the text: every paragraph of its cells is found on the page that shows the part of the
    // row it is in.
    void positionsInARowThatGoesOverPageBreaksAreOnThePagesThatShowThem()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 2, 5, {}, 10, 20);
        TestTable spec;
        spec.rows = 4;
        spec.cols = 2;
        spec.columnWidth = 200;
        spec.text = [](int r, int c) {
            QStringList lines;
            for (int i = 0; i < (r == 1 && c == 1 ? 140 : 3); ++i) lines << QStringLiteral("line %1 of cell %2 and %3").arg(i).arg(r).arg(c);
            return lines.join(QLatin1Char('\n'));
        };
        addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("After the table."));
        fd.sections[0].header.odd = furnitureStory({QStringLiteral("HEAD 1"), QStringLiteral("HEAD 2"), QStringLiteral("HEAD 3"), QStringLiteral("HEAD 4"), QStringLiteral("HEAD 5")});
        fd.sections[0].footer.odd = furnitureStory({QStringLiteral("FOOT 1"), QStringLiteral("FOOT 2"), QStringLiteral("FOOT 3"), QStringLiteral("FOOT 4")});
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 3);
        QVERIFY2(fd.section(0).content().height() - pg.page(1).content.height() > 20, "the headers and the footer take room from the text");
        const QString bad = audit::splitRowProblems(pg, fd.story());
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        const QString rest = audit::problems(pg, fd.story());
        QVERIFY2(rest.isEmpty(), qPrintable(rest));
    }

    // Margins that leave a page no text area (and a header and footer beside them) give it the least text area every page has, not a negative one.
    void marginsLargerThanThePageLeaveTheLeastTextArea()
    {
        FlowDocument fd;
        fillStory(fd.story(), 30, 3);
        Section &s = fd.sections[0];
        s.margins = QMarginsF(72, 5000, 72, 5000);
        s.header.odd = furnitureStory({QStringLiteral("HEADER")});
        s.footer.odd = furnitureStory({QStringLiteral("FOOTER")});
        Paginator pg(&fd);
        const auto page = pg.page(0);
        const QRectF plain = s.content();
        QVERIFY2(page.content.height() >= 35.9, qPrintable(QString::number(page.content.height())));
        QVERIFY2(std::abs(page.content.height() - plain.height()) < 0.01, qPrintable(QStringLiteral("%1 vs %2").arg(page.content.height()).arg(plain.height())));
        QVERIFY(pg.pageCount() >= 1 && pg.pageCount() < 100000);
    }

    // A header or footer taller than the margin leaves room for moves the text away from it, so the page holds fewer lines.
    void aTallHeaderAndFooterTakeRoomFromTheText()
    {
        auto lines = [](bool tall) {
            FlowDocument fd;
            fillStory(fd.story(), 80, 9);
            Section &s = fd.sections[0];
            s.headerDistance = 36;
            s.footerDistance = 36;
            if (tall) {
                s.header.odd = furnitureStory({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c"), QStringLiteral("d"), QStringLiteral("e"), QStringLiteral("f"), QStringLiteral("g")});
                s.footer.odd = furnitureStory({QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z"), QStringLiteral("w"), QStringLiteral("v"), QStringLiteral("u")});
            }
            Paginator pg(&fd);
            const auto first = pg.lines(0);
            return std::make_tuple(int(first.size()), first.last().rect.bottom(), pg.page(0).content, pg.pageCount());
        };
        const auto bare = lines(false), tall = lines(true);
        QVERIFY(std::get<0>(tall) < std::get<0>(bare));
        QVERIFY(std::get<3>(tall) > std::get<3>(bare));
        // The text area starts below the header and ends above the footer, each at its distance from the edge plus its own height.
        FlowDocument probe;
        const double width = probe.sections[0].content().width();
        const double headerH = storyHeightOf(furnitureStory({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c"), QStringLiteral("d"), QStringLiteral("e"), QStringLiteral("f"), QStringLiteral("g")}).get(), width, probe);
        const double footerH = storyHeightOf(furnitureStory({QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z"), QStringLiteral("w"), QStringLiteral("v"), QStringLiteral("u")}).get(), width, probe);
        const QRectF body = std::get<2>(tall);
        QVERIFY(std::abs(body.top() - (36 + headerH)) < 0.02);
        QVERIFY(std::abs(body.bottom() - (792 - 36 - footerH)) < 0.02);
        QVERIFY(headerH + 36 > 72 && footerH + 36 > 72);
        QVERIFY(std::get<1>(tall) <= body.height() + 0.01);
    }

    // The first page of a section, the even pages, and the rest each have their own story, as the section asks.
    void firstAndEvenPagesUseTheirOwnHeaders()
    {
        FlowDocument fd;
        fillStory(fd.story(), 120, 6, [](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(10);
            if (i == 59) bf.setProperty(flow::fp::SectionBreakAfter, true);
        });
        Section &s = fd.sections[0];
        s.differentFirstPage = true;
        s.differentOddEven = true;
        s.header.first = furnitureStory({QStringLiteral("first")});
        s.header.odd = furnitureStory({QStringLiteral("odd")});
        s.header.even = furnitureStory({QStringLiteral("even")});
        Section second = s;
        second.header.first = furnitureStory({QStringLiteral("second first")});
        second.differentOddEven = false;
        fd.sections << second;
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 6);
        QStringList seen;
        int firstOfSecond = -1;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QString header;
            const auto boxes = boxesByText(*page);
            for (auto it = boxes.cbegin(); it != boxes.cend(); ++it)
                if (std::abs(it.value()->rect.top() - 36) < 0.01) header = it.key();
            seen << header;
            if (page->section == 1 && firstOfSecond < 0) firstOfSecond = k;
        }
        QVERIFY(firstOfSecond >= 4 && pages >= firstOfSecond + 3);
        QCOMPARE(seen[0], QStringLiteral("first"));
        QCOMPARE(seen[1], QStringLiteral("even"));
        QCOMPARE(seen[2], QStringLiteral("odd"));
        QCOMPARE(seen[3], QStringLiteral("even"));
        QCOMPARE(seen[firstOfSecond], QStringLiteral("second first"));
        QCOMPARE(seen[firstOfSecond + 1], QStringLiteral("odd"));   // (the second section asks for no even pages' headers: its ordinary one)
        QCOMPARE(seen[firstOfSecond + 2], QStringLiteral("odd"));
    }

    // NUMPAGES and SECTIONPAGES show how many pages the document and the page's section have, which the page's fields read.
    void pageFieldsKnowTheTotals()
    {
        FlowDocument fd;
        fillStory(fd.story(), 70, 6, [](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(10);
            if (i == 39) bf.setProperty(flow::fp::SectionBreakAfter, true);
        });
        fd.sections << fd.sections[0];
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        int inFirst = 0;
        for (int k = 0; k < pages; ++k) inFirst += pg.page(k).start.section == 0;
        QVERIFY(inFirst > 1 && inFirst < pages - 1);
        for (int k : {0, pages - 1}) {
            const auto page = pg.materialize(k);
            FieldContext ctx;
            ctx.doc = page->doc.get();
            QCOMPARE(ctx.resolve(QStringLiteral("pages")), QString::number(pages));
            QCOMPARE(ctx.resolve(QStringLiteral("sectionpages")), QString::number(k == 0 ? inFirst : pages - inFirst));
        }
        // (Without a flowing document's counts, a context's own page count is shown.)
        FieldContext own;
        own.pageCount = 7;
        QCOMPARE(own.resolve(QStringLiteral("pages")), QStringLiteral("7"));
        QCOMPARE(own.resolve(QStringLiteral("sectionpages")), QStringLiteral("7"));
    }

    // A note longer than the pages the body has goes on over pages of its own, which have no text of the body: every page is made, and the end of the note is
    // drawn on the last of them.
    void aNoteLongerThanTheBodyHasPagesOfItsOwn()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("long"), 2000, QStringLiteral("LONGNOTE"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fillStory(fd.story(), 20, 4, {}, 30, 40);
        addReference(fd.story(), fd.story()->findBlockByNumber(1).position() + 5, QStringLiteral("long"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        int noteOnly = 0;
        QString drawn;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY2(page, qPrintable(QStringLiteral("page %1 of %2 isn't made").arg(k).arg(pages)));
            if (pg.lines(k).isEmpty()) {
                ++noteOnly;
                QVERIFY2(!page->doc->pages[0]->items.empty(), "a page of notes alone has them drawn");
            }
            for (const auto &item : page->doc->pages[0]->items)
                if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                    if (const Story *st = page->doc->story(t->storyId)) drawn += st->doc->toPlainText() + QLatin1Char('\n');
        }
        QVERIFY2(noteOnly > 0, "the note is longer than the body's pages");
        QVERIFY(drawn.contains(QStringLiteral("word1999")));
    }

    // References packed in one paragraph cost in step with their number, not its square: the page lays out only what it holds of the paragraph's notes.
    void denseReferencesTakeTimeInStepWithTheirNumber()
    {
        auto seconds = [&](int count) {
            std::unique_ptr<TestNotes> notes;
            std::unique_ptr<FlowDocument> fd;
            std::unique_ptr<Paginator> pg;
            return bestNanoseconds([&] {
                pg = std::make_unique<Paginator>(fd.get());
                const int pages = pg->pageCount();
                for (int k = 0; k < std::min(pages, 12); ++k) pg->materialize(k);
            }, 3, [&] {
                pg.reset();
                fd.reset();
                notes = std::make_unique<TestNotes>();
                fd = std::make_unique<FlowDocument>();
                fd->env.wordLines = true;
                fd->fields = notes.get();
                QTextCursor c(fd->story());
                QTextCharFormat cf;
                cf.setFontPointSize(11);
                for (int i = 0; i < count; ++i) {
                    const QString id = QStringLiteral("n%1").arg(i);
                    addNote(notes.get(), id, 3, QStringLiteral("N%1").arg(i));
                    QTextCharFormat ref = cf;
                    ref.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                    ref.setProperty(tp::Field, QStringLiteral("footnote:") + id);
                    c.insertText(QString(QChar(0xFFFC)), ref);
                }
            }) / 1e9;
        };
        const double small = seconds(400), large = seconds(1600);
        // (Four times the references: four times the time when it is in step, sixteen when it is the square; it was twenty before.)
        QVERIFY2(large < small * 8, qPrintable(QStringLiteral("400 references %1 s, 1600 references %2 s").arg(small).arg(large)));
    }

    // Note numbers that restart in each section restart where a continuous section begins in the middle of a page: on the page, in its references and in
    // its notes, and in the numbers the pages after it go on from.
    void noteNumbersRestartAtAContinuousSectionBreak()
    {
        auto numbersOf = [](Paginator &pg, int k, QStringList *references, QStringList *notes) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            QStringList ids = page->doc->stories.keys();
            ids.sort();
            for (const QString &id : ids)
                for (QTextBlock b = page->doc->story(id)->doc->begin(); b.isValid(); b = b.next())
                    for (auto it = b.begin(); !it.atEnd(); ++it) {
                        const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                        if (code.startsWith(QLatin1String("raw:"))) (id.startsWith(QLatin1String("flow-note-")) ? notes : references)->push_back(code.mid(4));
                    }
        };
        for (bool restart : {false, true}) {
            TestNotes notes;
            for (const char *id : {"a", "b", "c", "d"}) addNote(&notes, QString::fromLatin1(id), 3, QString::fromLatin1(id).toUpper());
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.fields = &notes;
            fd.restartNotesInSections = restart;
            fillStory(fd.story(), 4, 3, [](int i, QTextBlockFormat &bf) {
                bf.setBottomMargin(6);
                if (i == 1) bf.setProperty(flow::fp::SectionBreakAfter, true);
            }, 8, 12);
            Section second = fd.sections[0];
            second.continuous = true;
            fd.sections << second;
            const char *ids[] = {"a", "b", "c", "d"};
            for (int i = 3; i >= 0; --i) addReference(fd.story(), fd.story()->findBlockByNumber(i).position() + 3, QString::fromLatin1(ids[i]));
            Paginator pg(&fd);
            QCOMPARE(pg.pageCount(), 1);
            QStringList references, shown;
            numbersOf(pg, 0, &references, &shown);
            const QStringList want = restart ? QStringList{"1", "2", "1", "2"} : QStringList{"1", "2", "3", "4"};
            QCOMPARE(references, want);
            QCOMPARE(shown, want);
        }
    }

    // A footnote is at the foot of the page that cites it, in a text box of its own at the bottom of the text area with a rule above it, and its
    // number is where the reference is counted: the references go on through the pages, and the page's text has less room for the notes.
    void footnotesAreAtTheFootOfThePageThatCitesThem()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("a"), 12, QStringLiteral("NOTEA"));
        addNote(&notes, QStringLiteral("b"), 12, QStringLiteral("NOTEB"));
        auto build = [&](bool withNotes) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 70, 12);
            if (withNotes) {
                fd->fields = &notes;
                addReference(fd->story(), fd->story()->findBlockByNumber(2).position() + 5, QStringLiteral("a"));
                addReference(fd->story(), fd->story()->findBlockByNumber(30).position() + 5, QStringLiteral("b"));
            }
            return fd;
        };
        auto with = build(true), without = build(false);
        Paginator pg(with.get()), plain(without.get());
        QVERIFY(pg.pageCount() >= 3);
        // The first page: the note, its rule, and fewer lines than the page without notes holds.
        const auto first = pg.materialize(0);
        QString noteStory;
        const TextItem *note = boxHolding(*first, QStringLiteral("NOTEA"), &noteStory);
        QVERIFY(note);
        QVERIFY(!boxHolding(*first, QStringLiteral("NOTEB")));
        const double bottom = with->sections[0].content().bottom();
        QVERIFY2(std::abs(note->rect.bottom() - bottom) < 0.05, qPrintable(QString::number(note->rect.bottom())));
        QVERIFY(noteStory.contains(QStringLiteral("raw")) == false);   // (the mark is its number)
        QVERIFY(pg.lines(0).size() < plain.lines(0).size());
        const LineItem *rule = nullptr;
        for (const auto &item : first->doc->pages[0]->items)
            if (const auto *l = dynamic_cast<const LineItem *>(item.get())) rule = l;
        QVERIFY(rule);
        QVERIFY(std::abs((rule->p2.x() - rule->p1.x()) - 144) < 0.01);
        QVERIFY(rule->p1.y() < note->rect.top() && rule->p1.y() > note->rect.top() - 20);
        // The reference in the text shows its number; the number goes on through the pages (the second note is 2 wherever it is).
        const TextItem *body = boxHolding(*first, QStringLiteral("court"));
        QVERIFY(body);
        const Story *bodyStory = first->doc->story(body->storyId);
        bool numbered = false;
        for (QTextBlock b = bodyStory->doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) numbered |= it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("raw:1");
        QVERIFY(numbered);
        int notePage = -1;
        for (int k = 1; k < pg.pageCount() && notePage < 0; ++k)
            if (boxHolding(*pg.materialize(k), QStringLiteral("NOTEB"))) notePage = k;
        QVERIFY(notePage > 0);
        const auto later = pg.materialize(notePage);
        const TextItem *bNote = boxHolding(*later, QStringLiteral("NOTEB"));
        bool two = false;
        for (QTextBlock b = later->doc->story(bNote->storyId)->doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) two |= it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("raw:2");
        QVERIFY(two);
        QCOMPARE(pg.page(notePage).start.foot, 1);   // (one note came before the page's first paragraph)
        QVERIFY(std::abs(bNote->rect.bottom() - bottom) < 0.05);
        // A page that cites none has no note and no rule.
        const auto bare = pg.materialize(1 == notePage ? 2 : 1);
        QVERIFY(!boxHolding(*bare, QStringLiteral("NOTE")));
        for (const auto &item : bare->doc->pages[0]->items) QVERIFY(!dynamic_cast<const LineItem *>(item.get()));
    }

    // A footnote too long for the room at the foot of the page is cut there, and the rest of it begins the notes of the next page.
    void aLongFootnoteGoesOnOnTheNextPage()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("long"), 900, QStringLiteral("LONGNOTE"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fillStory(fd.story(), 40, 14);
        addReference(fd.story(), fd.story()->findBlockByNumber(1).position() + 5, QStringLiteral("long"));
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 3);
        QVERIFY(!pg.page(1).start.carried.isEmpty());
        const auto first = pg.materialize(0), second = pg.materialize(1);
        QVERIFY(boxHolding(*first, QStringLiteral("LONGNOTE")));
        QString rest;
        const TextItem *tail = boxHolding(*second, QStringLiteral("word"), &rest);
        QVERIFY(tail);
        QVERIFY(!rest.contains(QStringLiteral("LONGNOTE")));   // (the second part begins in the middle of the note's text)
        // The note's text is all on the pages it is on: every word once.
        QSet<QString> seen;
        int count = 0;
        for (int k = 0; k < pg.pageCount(); ++k) {
            const auto page = pg.materialize(k);
            for (const auto &item : page->doc->pages[0]->items)
                if (const auto *t = dynamic_cast<const TextItem *>(item.get())) {
                    const Story *st = page->doc->story(t->storyId);
                    if (!st->doc->toPlainText().contains(QStringLiteral("word"))) continue;
                    if (st->doc->toPlainText().contains(QStringLiteral("court"))) continue;   // (the text, not a note)
                    LayoutCache cache;
                    const auto fl = cache.textFrame(*page->doc, *t, 1, RenderOptions());
                    if (!fl.layout) continue;
                    for (const auto &li : fl.layout->lineInfo(fl.frame))
                        for (const QString &w : li.text.split(QLatin1Char(' '), Qt::SkipEmptyParts))
                            if (w.startsWith(QStringLiteral("word"))) { seen.insert(w); ++count; }
                }
        }
        QCOMPARE(int(seen.size()), 900);
        QCOMPARE(count, 900);
    }

    // An object set at a place goes where its anchor's page, paragraph, or line puts it: from the margin or the page, in the middle or at the end of one,
    // behind the text or over it; a rule's ends are from the place too; a picture's data is in the page's document.
    void objectsAreSetWherePageParagraphAndLinePutThem()
    {
        FlowDocument fd;
        fillStory(fd.story(), 60, 12);
        const QTextDocument *story = fd.story();
        const int third = story->findBlockByNumber(3).position() + 4;
        auto box = [](double w, double h) {
            auto item = std::make_shared<ShapeItem>();
            item->rect = QRectF(0, 0, w, h);
            return item;
        };
        using Rel = FloatingObject::Rel;
        // From the margin across and the paragraph down.
        const int a = addObject(&fd, box(100, 20), third, Rel::Margin, Rel::Paragraph, 10, 5);
        // In the middle across the page, at the end of the margin down, behind the text.
        const int b = addObject(&fd, box(50, 30), third, Rel::Page, Rel::Margin);
        fd.objects[b - 1].hAlign = FloatingObject::Align::Center;
        fd.objects[b - 1].vAlign = FloatingObject::Align::End;
        fd.objects[b - 1].behind = true;
        // A rule from the column's edge, a line down.
        auto line = std::make_shared<LineItem>();
        line->p1 = QPointF(0, 0);
        line->p2 = QPointF(120, 0);
        const int c = addObject(&fd, line, third, Rel::Column, Rel::Line, 3, 2);
        // A picture from the page's corner.
        auto picture = std::make_shared<PictureItem>();
        picture->rect = QRectF(0, 0, 40, 20);
        fd.images.push_back({tinyPng(8, 4, Qt::red), QStringLiteral("png")});
        const int p = addObject(&fd, picture, third, Rel::Page, Rel::Page, 20, 30);
        fd.objects[p - 1].image = 0;
        Q_UNUSED(a); Q_UNUSED(c);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const auto &items = page->doc->pages[0]->items;
        // The paragraph and the line that hold the anchor, as the page laid them out.
        const auto lines = pg.lines(0);
        double paragraphTop = -1, lineTop = -1;
        const int blockStart = story->findBlockByNumber(3).position();
        for (const auto &l : lines)
            if (l.block == blockStart) {
                if (paragraphTop < 0) paragraphTop = l.rect.top();
                if (third >= l.pos && third < l.end) lineTop = l.rect.top();
            }
        QVERIFY(paragraphTop > 0 && lineTop >= paragraphTop);
        const double top = fd.sections[0].margins.top(), left = fd.sections[0].margins.left();
        const ShapeItem *first = nullptr, *second = nullptr;
        const LineItem *rule = nullptr;
        const PictureItem *pic = nullptr;
        for (const auto &it : items) {
            if (it->type() == ItemType::Line) rule = static_cast<const LineItem *>(it.get());
            if (it->type() == ItemType::Picture) pic = static_cast<const PictureItem *>(it.get());
            if (it->type() == ItemType::Shape) {
                const auto *sh = static_cast<const ShapeItem *>(it.get());
                if (sh->rect.width() == 100) first = sh;
                else second = sh;
            }
        }
        QVERIFY(first && second && rule && pic);
        QVERIFY2(std::abs(first->rect.left() - (left + 10)) < 0.01 && std::abs(first->rect.top() - (top + paragraphTop + 5)) < 0.01, qPrintable(QStringLiteral("%1 %2 (paragraph at %3)").arg(first->rect.left()).arg(first->rect.top()).arg(paragraphTop)));
        QVERIFY(std::abs(second->rect.left() - (612 - 50) / 2.0) < 0.01);
        QVERIFY(std::abs(second->rect.bottom() - (792 - fd.sections[0].margins.bottom())) < 0.01);
        QVERIFY(items.front().get() == second);   // (behind the text: before it)
        QVERIFY(std::abs(rule->p1.x() - (left + 3)) < 0.01 && std::abs(rule->p2.x() - (left + 3 + 120)) < 0.01);
        QVERIFY(std::abs(rule->p1.y() - (top + lineTop + 2)) < 0.01 && std::abs(rule->p1.y() - rule->p2.y()) < 0.01);
        QVERIFY(std::abs(pic->rect.left() - 20) < 0.01 && std::abs(pic->rect.top() - 30) < 0.01);
        QVERIFY(!pic->imageId.isEmpty());
        QVERIFY(page->doc->images.contains(pic->imageId));
        // A page whose lines hold no anchor has none of them.
        const auto other = pg.materialize(pg.pageCount() - 1);
        for (const auto &it : other->doc->pages[0]->items) QVERIFY(it->type() == ItemType::Text);
    }

    // The objects set in a section's header are on every page of it, from the header's own lines; an inline picture in a story has the Id its picture
    // has in the page's document.
    void headerObjectsAndInlinePicturesAreOnTheirPages()
    {
        FlowDocument fd;
        fillStory(fd.story(), 80, 12);
        auto header = furnitureStory({QStringLiteral("HEADER"), QStringLiteral("second line")});
        {
            // A rule anchored to the header's second paragraph, from the margin across and the paragraph down.
            auto line = std::make_shared<LineItem>();
            line->p1 = QPointF(0, 0);
            line->p2 = QPointF(200, 0);
            FloatingObject o;
            o.item = line;
            o.hRel = FloatingObject::Rel::Margin;
            o.vRel = FloatingObject::Rel::Paragraph;
            o.dy = 1;
            fd.objects.push_back(o);
            QTextCursor c(header.get());
            c.setPosition(header->findBlockByNumber(1).position());
            QTextCharFormat anchor;
            anchor.setProperty(fp::AnchorId, 1);
            c.insertText(QString(QChar(0xFFFC)), anchor);
        }
        fd.sections[0].header.odd = header;
        fd.sections[0].headerDistance = 30;
        // An inline picture in the body.
        fd.images.push_back({tinyPng(6, 6, Qt::blue), QStringLiteral("png")});
        {
            auto item = std::make_shared<PictureItem>();
            item->rect = QRectF(0, 0, 30, 20);
            item->imageId = QStringLiteral("flow-img:0");
            QTextCursor c(fd.story());
            c.setPosition(fd.story()->findBlockByNumber(1).position() + 3);
            QTextCharFormat inl;
            inl.setFontPointSize(11);
            inl.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(item->toJson()).toJson(QJsonDocument::Compact)));
            c.insertText(QString(QChar(0xFFFC)), inl);
        }
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        for (int k : {0, 1}) {
            const auto page = pg.materialize(k);
            const LineItem *rule = nullptr;
            for (const auto &it : page->doc->pages[0]->items)
                if (it->type() == ItemType::Line) rule = static_cast<const LineItem *>(it.get());
            QVERIFY(rule);
            // The header's second line: below its first by a line, from the header's top.
            StoryLayout own;
            own.build(header.get(), {plainFrame(fd.sections[0].content().width(), 100000)}, fd.env);
            const double second = own.lineInfo(0).at(1).rect.top();
            QVERIFY2(std::abs(rule->p1.y() - (30 + second + 1)) < 0.01, qPrintable(QString::number(rule->p1.y())));
            QVERIFY(std::abs(rule->p1.x() - fd.sections[0].margins.left()) < 0.01);
        }
        const auto first = pg.materialize(0);
        const TextItem *body = boxHolding(*first, QStringLiteral("court"));
        QVERIFY(body);
        QString id;
        for (QTextBlock b = first->doc->story(body->storyId)->doc->begin(); b.isValid() && id.isEmpty(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString json = it.fragment().charFormat().stringProperty(tp::InlineObject);
                if (!json.isEmpty()) id = QJsonDocument::fromJson(json.toUtf8()).object().value(QStringLiteral("image")).toString();
            }
        QVERIFY(!id.isEmpty() && !id.startsWith(QStringLiteral("flow-img:")));
        QVERIFY(first->doc->images.contains(id));
    }

    // ---- frames

    // A word processor's text does not lay out a paragraph that is set apart from it: it has no lines and no room, and the text around it is as if it were not there; the other
    // texts (a publication's) take no notice of the property.
    void aParagraphOutOfLayoutHasNoLinesAndNoRoom()
    {
        auto lay = [](bool outOfLayout, bool word) {
            auto story = makeStory(5, 9, {}, 14, 25);
            if (outOfLayout) {
                QTextCursor c(story.get());
                c.setPosition(story->findBlockByNumber(2).position());
                QTextBlockFormat bf = c.blockFormat();
                bf.setProperty(tp::OutOfLayout, true);
                c.setBlockFormat(bf);
            }
            LayoutEnv env;
            env.wordLines = word;
            StoryLayout l;
            l.build(story.get(), {plainFrame(300, 600)}, env);
            QStringList out;
            const int second = story->findBlockByNumber(2).position();
            for (const auto &b : l.lineBriefs()) out << QStringLiteral("%1 %2%3").arg(b.block == second ? QStringLiteral("hidden") : QStringLiteral("body"), audit::rectText(b.rect), b.block == second ? QStringLiteral("!") : QString());
            return out;
        };
        const QStringList word = lay(true, true), plain = lay(false, true), other = lay(true, false), otherPlain = lay(false, false);
        for (const QString &line : word) QVERIFY2(!line.endsWith(QLatin1Char('!')), qPrintable(line));
        QCOMPARE(word.size() + int(std::count_if(plain.begin(), plain.end(), [](const QString &l) { return l.endsWith(QLatin1Char('!')); })), int(plain.size()));
        // The lines of the others are where they are without the paragraph in the text at all (the same story with the paragraph taken out).
        {
            auto story = makeStory(5, 9, {}, 14, 25);
            QTextCursor c(story.get());
            const QTextBlock b = story->findBlockByNumber(2);
            c.setPosition(b.position());
            c.setPosition(b.position() + b.length(), QTextCursor::KeepAnchor);
            c.removeSelectedText();
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout l;
            l.build(story.get(), {plainFrame(300, 600)}, env);
            QStringList without;
            for (const auto &x : l.lineBriefs()) without << QStringLiteral("body %1").arg(audit::rectText(x.rect));
            QCOMPARE(word, without);
        }
        // (In a publication's text it is a paragraph like any other.)
        QCOMPARE(other, otherPlain);
        // A picture in the paragraph (a frame with only a logo in it) is not laid out either: no line, no room, nothing drawn in the text.
        {
            auto story = makeStory(3, 9, {}, 14, 25);
            auto item = std::make_shared<PictureItem>();
            item->rect = QRectF(0, 0, 40, 30);
            item->imageId = QStringLiteral("flow-img:0");
            QTextCursor c(story.get());
            c.setPosition(story->findBlockByNumber(1).position());
            QTextBlockFormat bf = c.blockFormat();
            bf.setProperty(tp::OutOfLayout, true);
            c.setBlockFormat(bf);
            QTextCharFormat inl;
            inl.setFontPointSize(11);
            inl.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(item->toJson()).toJson(QJsonDocument::Compact)));
            c.insertText(QString(QChar(0xFFFC)), inl);
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout l;
            l.build(story.get(), {plainFrame(300, 600)}, env);
            QVERIFY(l.inlineObjects().isEmpty());
            const int second = story->findBlockByNumber(1).position();
            for (const auto &b : l.lineBriefs()) QVERIFY(b.block != second);
        }
    }

    // A stretch of text that stops at a frame (a page's, laid out to see what goes past it) does not stop at a paragraph with no lines, which has put nothing in the frame: the
    // text after it is laid out, and is what lands in the frame.
    void aStretchIsNotEndedByAParagraphWithNoLines()
    {
        auto story = makeStory(4, 9, {}, 14, 25);
        QTextCursor c(story.get());
        c.setPosition(0);
        QTextBlockFormat bf = c.blockFormat();
        bf.setProperty(tp::OutOfLayout, true);
        c.setBlockFormat(bf);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        StoryLayout::Span span;
        span.stopAtFrame = 0;   // (the only frame is the one that runs over)
        lay.build(story.get(), {plainFrame(300, 100000)}, env, span);
        const int second = story->findBlockByNumber(1).position();
        QVERIFY(!lay.lineBriefs().isEmpty());
        QCOMPARE(lay.lineBriefs().first().block, second);
        // (All of the paragraph that lands there is laid out, and the stretch ends after it.)
        int lastBlock = -1;
        for (const auto &l : lay.lineBriefs()) lastBlock = l.block;
        QCOMPARE(lastBlock, second);
    }

    // A frame is set where the paragraph after its paragraphs is: its paragraphs have no lines and no room, it is a text box the page draws at its place, with the text of its
    // paragraphs and as wide as it says, and the text goes round it.
    void aFrameIsSetAtTheParagraphAfterItAndTheTextGoesRoundIt()
    {
        auto make = [](bool withFrame) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 24, 41, {}, 25, 40);
            if (withFrame) {
                FloatingObject proto;
                proto.hRel = FloatingObject::Rel::Column;
                proto.vRel = FloatingObject::Rel::Paragraph;
                proto.hAlign = FloatingObject::Align::End;
                proto.dy = 6;
                proto.wrap = FloatingObject::Wrap::Square;
                proto.gap = QMarginsF(9, 0, 9, 0);
                addFrame(*fd, 6, {QStringLiteral("First paragraph of the frame"), QStringLiteral("and a second paragraph of it, which is longer, so that it takes two lines of the frame")}, proto, 130);
            }
            return fd;
        };
        auto plain = make(false), framed = make(true);
        Paginator pp(plain.get()), pg(framed.get());
        const QSet<int> frameBlocks = frameBlocksOf(*framed);
        QCOMPARE(int(frameBlocks.size()), 2);
        const int anchorBlock = framed->story()->findBlockByNumber(8).position();   // (the paragraph that was block 6)
        QVERIFY(!frameBlocks.contains(anchorBlock));
        for (const auto &l : pg.lines(0)) QVERIFY2(!frameBlocks.contains(l.block), "a line of a frame's paragraph");
        // The paragraphs above the frame are as they are without it, and so is the paragraph it is anchored to (its top).
        const auto a = pageLines(pg, *framed, 0), b = pageLines(pp, *plain, 0);
        int above = 0;
        for (const PageLine &l : a) {
            if (l.block >= anchorBlock) break;
            QVERIFY2(above < b.size() && std::abs(b[above].rect.top() - l.rect.top()) < 0.01 && std::abs(b[above].rect.width() - l.rect.width()) < 0.01, qPrintable(audit::rectText(l.rect)));
            ++above;
        }
        QVERIFY(above > 10);
        const double top = paragraphTopOn(pg, *framed, 0, anchorBlock);
        QVERIFY(top > 0 && std::abs(top - paragraphTopOn(pp, *plain, 0, plain->story()->findBlockByNumber(6).position())) < 0.01);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const TextItem *box = frameBoxWith(*page, QStringLiteral("First paragraph of the frame"));
        QVERIFY(box);
        const QRectF column = framed->sections[0].columnRect(0);
        QVERIFY2(std::abs(box->rect.width() - 130) < 0.01 && std::abs(box->rect.right() - column.right()) < 0.01 && std::abs(box->rect.top() - (top + 6)) < 0.02, qPrintable(audit::rectText(box->rect)));
        const Story *story = page->doc->story(box->storyId);
        QVERIFY(story && story->doc->toPlainText().contains(QStringLiteral("two lines of the frame")));
        // (Its paragraphs are laid out in it: not set out of the layout there.)
        for (QTextBlock x = story->doc->begin(); x.isValid(); x = x.next()) QVERIFY(!x.blockFormat().boolProperty(tp::OutOfLayout) && x.blockFormat().intProperty(fp::Frame) == 0);
        // As tall as its text is in a box 130 wide.
        StoryLayout own;
        own.build(story->doc.get(), {plainFrame(130, 100000)}, framed->env);
        QVERIFY2(std::abs(box->rect.height() - own.usedHeight(0)) < 0.02 && own.lineInfo(0).size() >= 4, qPrintable(QStringLiteral("%1 high; its text %2 in a box 130 wide").arg(box->rect.height()).arg(own.usedHeight(0))));
        // The text goes round it, as the page layer draws it; and the page is whole.
        const QRectF wall = box->rect.adjusted(-9, 0, 9, 0);
        int beside = 0;
        for (const PageLine &l : a)
            if (l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05) {
                ++beside;
                QVERIFY2(l.rect.right() <= wall.left() + 0.05, qPrintable(audit::rectText(l.rect)));
            }
        QVERIFY(beside >= 4);
        QVERIFY2(audit::problems(pg, framed->story()).isEmpty(), qPrintable(audit::problems(pg, framed->story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, *framed, k).isEmpty(), qPrintable(audit::placementProblems(pg, *framed, k)));
    }

    // How tall a frame is: as its text is when it asks nothing, at least what it asks, or exactly that (then the text is cut by it).
    void aFramesHeightIsItsTextsOrWhatItAsks()
    {
        auto heightOf = [](double asked, bool exact) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 10, 42, {}, 25, 40);
            FloatingObject proto;
            proto.hRel = FloatingObject::Rel::Column;
            proto.vRel = FloatingObject::Rel::Paragraph;
            proto.wrap = FloatingObject::Wrap::Square;
            addFrame(fd, 3, {QStringLiteral("One line of the frame"), QStringLiteral("a second paragraph of the frame, which is long enough to take a second line")}, proto, 140, asked, exact);
            Paginator pg(&fd);
            const auto page = pg.materialize(0);
            const TextItem *box = page ? frameBoxWith(*page, QStringLiteral("One line of the frame")) : nullptr;
            return box ? box->rect.height() : -1.0;
        };
        const double natural = heightOf(0, false);
        QVERIFY(natural > 30 && natural < 100);
        QVERIFY(std::abs(heightOf(natural / 2, false) - natural) < 0.01);    // (at least: a text taller than that is as tall as it is)
        QVERIFY(std::abs(heightOf(natural + 50, false) - (natural + 50)) < 0.01);
        QVERIFY(std::abs(heightOf(20, true) - 20) < 0.01);                   // (exactly)
        QVERIFY(std::abs(heightOf(natural + 50, true) - (natural + 50)) < 0.01);
    }

    // A frame with no width of its own is as wide as the column; one that shrinks (a drop cap) is as wide as its longest line: no narrower (a narrower box wraps a line) and no
    // wider than needed (in it no line is wrapped, and a few points of room at most), and no wider than the column.
    void aFrameWithNoWidthIsAsWideAsTheColumnOrAsItsLongestLineIfItShrinks()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 10, 43, {}, 25, 40);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        const int a = addFrame(fd, 3, {QStringLiteral("Some words of a frame"), QStringLiteral("and more of them")}, proto, 0);
        fd.objects[size_t(fd.frames[a - 1]) - 1].frameShrinks = true;
        addFrame(fd, 6, {QStringLiteral("A frame that does not shrink, with a few words")}, proto, 0);
        addFrame(fd, 9, {QStringLiteral("a very long line of a frame that shrinks, longer than the column of the page it is on, so that it has to wrap there and then, at the column's width, "
                                        "which is what the column has")}, proto, 0);
        fd.objects[size_t(fd.frames[2]) - 1].frameShrinks = true;
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const TextItem *narrow = frameBoxWith(*page, QStringLiteral("Some words of a frame")), *plain = frameBoxWith(*page, QStringLiteral("does not shrink")),
                       *wide = frameBoxWith(*page, QStringLiteral("a very long line"));
        QVERIFY(narrow && plain && wide);
        const double column = fd.sections[0].columnWidth();
        QVERIFY2(narrow->rect.width() > 40 && narrow->rect.width() < column - 100, qPrintable(QString::number(narrow->rect.width())));
        QVERIFY2(std::abs(plain->rect.width() - column) < 0.01, qPrintable(QString::number(plain->rect.width())));
        QVERIFY2(std::abs(wide->rect.width() - column) < 0.01, qPrintable(QString::number(wide->rect.width())));
        const Story *story = page->doc->story(narrow->storyId);
        QVERIFY(story);
        auto lines = [&](double width) {
            StoryLayout lay;
            lay.build(story->doc.get(), {plainFrame(width, 100000)}, fd.env);
            return int(lay.lineInfo(0).size());
        };
        QCOMPARE(lines(narrow->rect.width()), 2);          // (a line to a paragraph: nothing is wrapped in it)
        QVERIFY(lines(narrow->rect.width() - 8) > 2);      // (and it is no wider than it must be: a few points of room at most)
    }

    // A frame belongs to the page of the paragraph after it: where that paragraph begins a page, the frame is on that page and not the one before.
    void aFrameIsOnThePageOfItsParagraph()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 20, 44, [](int i, QTextBlockFormat &bf) { if (i == 7) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 25, 40);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        proto.hAlign = FloatingObject::Align::End;
        addFrame(fd, 7, {QStringLiteral("FRAMEWORDS of the frame")}, proto, 120);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        const auto first = pg.materialize(0), second = pg.materialize(1);
        QVERIFY(first && second);
        QVERIFY(!frameBoxWith(*first, QStringLiteral("FRAMEWORDS")));
        QVERIFY(frameBoxWith(*second, QStringLiteral("FRAMEWORDS")));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
    }

    // Edits in a frame's paragraphs, and before and after them, lay out the page of the paragraph after them: the pages are those a new paginator makes, and the frame shows
    // what was typed.
    void editsInAFrameLayOutThePageItIsOn()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 120, 45, {}, 25, 45);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        proto.hAlign = FloatingObject::Align::End;
        addFrame(fd, 50, {QStringLiteral("FRAMEWORDS in the middle")}, proto, 120);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 8);
        const QSet<int> frame = frameBlocksOf(fd);
        QCOMPARE(int(frame.size()), 1);
        QTextCursor c(fd.story());
        c.setPosition(*frame.begin() + 5);
        c.insertText(QStringLiteral(" typed into the frame's own paragraph, which takes more of the frame's height, "));
        Paginator fresh0(&fd);
        QString bad = audit::differenceFromFresh(pg, fresh0);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        int shown = 0;
        for (int k = 0; k < pg.pageCount(); ++k)
            if (const auto m = pg.materialize(k))
                if (const TextItem *box = frameBoxWith(*m, QStringLiteral("typed into the frame"))) {
                    ++shown;
                    QVERIFY(box->rect.height() > 40);
                }
        QCOMPARE(shown, 1);
        // Taking the paragraph after it away (text before the frame too): the same.
        c.setPosition(fd.story()->findBlockByNumber(40).position());
        c.insertText(QStringLiteral("more words before the frame, enough to move it to the next page or not, as the text goes. "));
        Paginator fresh(&fd);
        bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A frame at the start of a continuous section with other columns (the page is laid out in two parts there) loses no text, whether the page has room for the section's first
    // lines or not, with a table in the section or without: every paragraph of the section is on the pages.
    void aFrameAtTheStartOfAContinuousSectionLosesNoText()
    {
        for (const bool table : {false, true})
            for (int before = 3; before <= 14; ++before) {
                FlowDocument fd;
                fd.env.wordLines = true;
                fillStory(fd.story(), before + 12, 48, [&](int i, QTextBlockFormat &bf) { if (i == before) bf.setProperty(fp::SectionBreakAfter, true); }, 25, 40);
                Section second = fd.sections[0];
                second.continuous = true;
                second.columns = 2;
                second.columnGap = 20;
                fd.sections << second;
                FloatingObject proto;
                proto.hRel = FloatingObject::Rel::Column;
                proto.vRel = FloatingObject::Rel::Paragraph;
                proto.wrap = FloatingObject::Wrap::Square;
                addFrame(fd, before + 1, {QStringLiteral("FRAMEWORDS of the second section")}, proto, 100);
                if (table) {
                    TestTable spec;
                    spec.rows = 4;
                    spec.cols = 2;
                    spec.columnWidth = 60;
                    addTableAfter(fd, before + 3, spec);
                }
                Paginator pg(&fd);
                const QString what = QStringLiteral("%1 paragraphs in the first section, table %2: ").arg(before).arg(table);
                QVERIFY2(pg.pageCount() >= 1, qPrintable(what));
                const QSet<int> frame = frameBlocksOf(fd);
                QSet<int> seen;
                for (int k = 0; k < pg.pageCount(); ++k)
                    for (const auto &l : pg.lines(k)) seen.insert(l.block);
                // Every paragraph of the story (but a frame's, and the cells' of a table, which are lines as rows) has a line on a page.
                for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next()) {
                    if (frame.contains(b.position()) || b.blockFormat().intProperty(fp::Cell)) continue;
                    QVERIFY2(seen.contains(b.position()), qPrintable(what + QStringLiteral("the paragraph at %1 (\"%2\") has no line on any page").arg(b.position()).arg(b.text().left(20))));
                }
            }
    }

    // The paginator keeps the frames it has made for a few hundred edits, then lets them go; the page being laid out holds the ones it uses (a pointer to a frame that was let
    // go is a use of freed memory, which the address sanitizer's build of this test finds). Typing past the cache's size with frames on the page, the pages stay right.
    void framesInUseSurviveTheirCacheTurningOver()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 30, 31, {}, 30, 60);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.hAlign = FloatingObject::Align::End;
        proto.wrap = FloatingObject::Wrap::Square;
        proto.gap = QMarginsF(9, 0, 9, 0);
        addFrame(fd, 3, {QStringLiteral("frame one text")}, proto, 100);
        addFrame(fd, 6, {QStringLiteral("frame two text")}, proto, 100);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        QTextCursor cur(fd.story());
        for (int i = 0; i < 260; ++i) {
            cur.setPosition(fd.story()->findBlockByNumber(20).position() + 3);
            cur.insertText(QStringLiteral("x"));
            QVERIFY(pg.pageCount() >= 2);
            const auto page = pg.materialize(0);
            QVERIFY(page);
            if (i % 100 == 0) {
                QVERIFY(frameBoxWith(*page, QStringLiteral("frame one text")));
                QVERIFY(frameBoxWith(*page, QStringLiteral("frame two text")));
            }
        }
        Paginator fresh(&fd);
        const QString bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A file can have a frame of thousands of paragraphs: one of no more than a frame has (kMostFrameParagraphs) is a box of its own, and one of more is paragraphs in the text, all
    // of them, as they would be without the frame's properties.
    void aFrameOfThousandsOfParagraphsIsInTheText()
    {
        for (const int count : {kMostFrameParagraphs, kMostFrameParagraphs + 1, 3000}) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 12, 55, {}, 25, 40);
            FloatingObject proto;
            proto.hRel = FloatingObject::Rel::Column;
            proto.vRel = FloatingObject::Rel::Paragraph;
            proto.wrap = FloatingObject::Wrap::Square;
            QStringList paragraphs;
            for (int i = 0; i < count; ++i) paragraphs << QStringLiteral("frame paragraph %1").arg(i);
            addFrame(fd, 4, paragraphs, proto, 120);
            QElapsedTimer timer;
            timer.start();
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            QVERIFY(pages >= 1);
            QVERIFY2(timer.elapsed() < 20000, qPrintable(QString::number(timer.elapsed())));
            const auto page = pg.materialize(0);
            QVERIFY(page);
            const TextItem *box = frameBoxWith(*page, QStringLiteral("frame paragraph 0"));
            if (count <= kMostFrameParagraphs) {
                QVERIFY(box);
                QVERIFY(page->doc->story(box->storyId)->doc->blockCount() == count);
            } else {
                QVERIFY2(!box, "a frame of more paragraphs than a frame has is in the text");
            }
            QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd.story())));
            QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
            QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
            // A frame that is paragraphs of the text is no box, and no boxes of its pieces are stacked (a box as tall as the frame's pieces together is taller than a page).
            if (count > kMostFrameParagraphs)
                for (int k = 0; k < pg.pageCount(); ++k) {
                    const auto m = pg.materialize(k);
                    QVERIFY(m);
                    for (const auto &it : m->doc->pages[0]->items) QVERIFY2(it->rect.height() <= m->size.height() + 0.01, qPrintable(audit::rectText(it->rect)));
                }
        }
    }

    // Frames side by side before a paragraph, and one at the end of the story (nothing after it to be set from, so it is in the text): all are made and drawn where they can
    // be, and nothing is lost or stuck.
    void framesTogetherAndAtTheEndOfTheStory()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 12, 46, {}, 25, 40);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        FloatingObject left = proto, right = proto;
        left.hAlign = FloatingObject::Align::Start;
        right.hAlign = FloatingObject::Align::End;
        addFrame(fd, 4, {QStringLiteral("LEFTFRAME text")}, left, 90);
        addFrame(fd, 5, {QStringLiteral("RIGHTFRAME text")}, right, 90);   // (after the first frame's paragraph: the two are one after the other before the paragraph that follows)
        // A frame at the very end.
        {
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            FloatingObject o = proto;
            o.item = std::make_shared<TextItem>();
            o.frameWidth = 80;
            fd.objects.push_back(o);
            fd.frames.push_back(int(fd.objects.size()));
            fd.objects.last().frame = int(fd.frames.size());
            QTextBlockFormat bf;
            bf.setProperty(fp::Frame, int(fd.frames.size()));
            bf.setProperty(tp::OutOfLayout, true);
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            c.insertBlock(bf, cf);
            c.insertText(QStringLiteral("ENDFRAME text"), cf);
        }
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 1);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QVERIFY(frameBoxWith(*page, QStringLiteral("LEFTFRAME")));
        QVERIFY(frameBoxWith(*page, QStringLiteral("RIGHTFRAME")));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY(pg.materialize(k));
        // The frame at the very end has no paragraph after it to be set from: its text is lines of the text, with the rest of it.
        QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
        QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd.story())));
        bool endLine = false;
        for (int k = 0; k < pg.pageCount(); ++k)
            for (const auto &l : pg.lines(k)) endLine |= fd.story()->findBlock(l.pos).text().contains(QStringLiteral("ENDFRAME"));
        QVERIFY(endLine);
    }

    // A paragraph put in between the paragraphs of a frame cuts it in two: the paragraph that the frame was set from now has the second half only (a shorter box, and a wall of
    // another height); one put in after the frame is the paragraph it is set from, and the paragraph that it was set from has no frame: the pages from there on are laid out
    // again (they are those of a new paginator).
    void aParagraphPutInsideAFrameLaysOutThePageOfTheParagraphItWasSetFrom()
    {
        QStringList failures;
        for (unsigned seed = 1; seed <= 24; ++seed) {
            const bool after = seed > 12;   // (the paragraph put in after the frame, which it is set from now, and not the paragraph that followed the frame)
            FlowDocument fd;
            fd.env.wordLines = true;
            // (The paragraph the frame is set from begins the second page, so that the page before ends where the edit's end is.)
            fillStory(fd.story(), 40, seed, [](int i, QTextBlockFormat &bf) { if (i == 10) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 30, 60);
            FloatingObject proto;
            proto.hRel = FloatingObject::Rel::Column;
            proto.vRel = FloatingObject::Rel::Paragraph;
            proto.hAlign = FloatingObject::Align::End;
            proto.wrap = FloatingObject::Wrap::Square;
            proto.gap = QMarginsF(8, 2, 8, 2);
            addFrame(fd, 10, {QStringLiteral("first paragraph of the frame, a few words that fill lines of the frame"), QStringLiteral("second paragraph of the frame with more words, so that its box is a good deal taller than one line of the text")}, proto, 110);
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() >= 3);
            const QSet<int> frame = frameBlocksOf(fd);
            QCOMPARE(int(frame.size()), 2);
            const int first = after ? *std::max_element(frame.begin(), frame.end()) : *std::min_element(frame.begin(), frame.end());
            const QTextBlock firstBlock = fd.story()->findBlock(first);
            QTextCursor c(fd.story());
            c.setPosition(firstBlock.position() + firstBlock.length() - 1);
            QTextBlockFormat plain;
            plain.setBottomMargin(6);
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            c.insertBlock(plain, cf);
            c.insertText(QStringLiteral("a paragraph of the text between the two halves of the frame, with a few words."), cf);
            Paginator fresh(&fd);
            const QString bad = audit::differenceFromFresh(pg, fresh);
            if (!bad.isEmpty()) failures << QStringLiteral("seed %1: %2").arg(seed).arg(bad.left(160));
            const QString floats = audit::floatProblems(pg, fd);
            if (!floats.isEmpty()) failures << QStringLiteral("seed %1: %2").arg(seed).arg(floats.left(160));
        }
        QVERIFY2(failures.isEmpty(), qPrintable(QStringLiteral("%1:\n%2").arg(failures.size()).arg(failures.mid(0, 4).join(QLatin1Char('\n')))));
    }

    // A frame comes right before a table that is in the text: it is set from the table, at its first row, and is drawn.
    void aFrameRightBeforeATableIsDrawnAtTheTable()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 8, 41, {}, 25, 40);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 70;
        addTableAfter(fd, 4, spec);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::None;
        proto.hAlign = FloatingObject::Align::End;
        addFrame(fd, 5, {QStringLiteral("FRAMEBEFORETABLE text")}, proto, 90);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 1);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const TextItem *box = frameBoxWith(*page, QStringLiteral("FRAMEBEFORETABLE"));
        QVERIFY(box);
        QVERIFY(!page->tables.isEmpty());
        QVERIFY2(std::abs(box->rect.top() - page->tables[0]->rect().top()) < 0.5, qPrintable(audit::rectText(box->rect) + QStringLiteral(" and the table at ") + audit::rectText(page->tables[0]->rect())));
        QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
        QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd.story())));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
    }

    // A table set at a place and a frame next to one another, in either order, before a paragraph: both are drawn, and every paragraph of the text has its lines (the text
    // after them is not lost for the frame's being between the table and it).
    void aFloatingTableAndAFrameNextToOneAnotherAreBothDrawn()
    {
        for (const bool tableFirst : {true, false}) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 8, 41, {}, 25, 40);
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            spec.columnWidth = 70;
            spec.setTable = [](TableData &t) {
                TableData::Place p;
                p.across = TableData::Place::From::Margin;
                p.acrossSpec = TableData::Place::Spec::End;
                p.room = QMarginsF(9, 0, 9, 0);
                t.place = p;
            };
            addTableAfter(fd, 4, spec);
            FloatingObject proto;
            proto.hRel = FloatingObject::Rel::Column;
            proto.vRel = FloatingObject::Rel::Paragraph;
            proto.wrap = FloatingObject::Wrap::Square;
            proto.hAlign = FloatingObject::Align::Start;
            if (tableFirst) {
                int n = 0, after = -1;
                for (QTextBlock b = fd.story()->begin(); b.isValid() && after < 0; b = b.next(), ++n)
                    if (n > 4 && !b.blockFormat().intProperty(fp::Cell)) after = n;
                addFrame(fd, after, {QStringLiteral("FRAMEAFTERTABLE text")}, proto, 90);   // (after the table's last cell, before the paragraph that follows it)
            } else {
                addFrame(fd, 5, {QStringLiteral("FRAMEBEFORETABLE text")}, proto, 90);   // (after block 4, before the table)
            }
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() >= 1);
            QString where = tableFirst ? QStringLiteral("table, then frame") : QStringLiteral("frame, then table");
            const auto page = pg.materialize(0);
            QVERIFY(page);
            QVERIFY2(frameBoxWith(*page, QStringLiteral("FRAME")), qPrintable(where));
            QVERIFY2(!page->tables.isEmpty() && page->tables[0]->floating, qPrintable(where));
            QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(where + QStringLiteral(": ") + audit::floatProblems(pg, fd)));
            QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(where + QStringLiteral(": ") + audit::coverageProblems(pg, fd.story())));
            QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(where + QStringLiteral(": ") + audit::problems(pg, fd.story())));
        }
    }

    // An object anchored in a frame's text (a picture in a frame's paragraph) is set where the frame is, and is drawn.
    void anObjectAnchoredInAFramesTextIsDrawn()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 10, 61, {}, 25, 40);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        proto.hAlign = FloatingObject::Align::End;
        addFrame(fd, 4, {QStringLiteral("FRAMEWORDS text")}, proto, 100);
        const QSet<int> frame = frameBlocksOf(fd);
        QCOMPARE(int(frame.size()), 1);
        auto item = std::make_shared<ShapeItem>();
        item->rect = QRectF(0, 0, 33, 22);
        addObject(&fd, item, *frame.begin() + 11, FloatingObject::Rel::Column, FloatingObject::Rel::Paragraph, 0, 0);   // (after "FRAMEWORDS ")
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QVERIFY(frameBoxWith(*page, QStringLiteral("FRAMEWORDS")));
        QVERIFY2(objectOfSize(*page, 33, 22), "the picture anchored in the frame's text is on the page");
        int count = 0;
        for (const auto &it : page->doc->pages[0]->items) count += it->type() == ItemType::Shape && std::abs(it->rect.width() - 33) < 0.01 && std::abs(it->rect.height() - 22) < 0.01;
        QCOMPARE(count, 1);
    }

    // Thousands of frames one after another, each its own: the nearest few of them are boxes at the paragraph after them, and the rest are paragraphs in the text, so that no
    // word is lost and the pages are as many as the text takes.
    void thousandsOfFramesInARowLoseNoText()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 12, 55, {}, 25, 40);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        const int count = 3000;
        for (int i = 0; i < count; ++i) addFrame(fd, 4 + i, {QStringLiteral("FRAMETEXT %1").arg(i)}, proto, 50);
        QElapsedTimer t;
        t.start();
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY2(t.elapsed() < 30000, qPrintable(QString::number(t.elapsed())));
        // (Each frame paragraph in the text is a line of its own, 3000 lines and more than the text's own: dozens of pages.)
        QVERIFY2(pages >= 40, qPrintable(QString::number(pages)));
        QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd.story())));
        QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
        int boxes = 0;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            int here = 0;
            for (const auto &it : page->doc->pages[0]->items) here += it->type() == ItemType::Text && it->id.startsWith(QLatin1String("flow-object-"));
            QVERIFY2(here <= kMostFloatsAtAParagraph, qPrintable(QStringLiteral("page %1 has %2 frame boxes").arg(k).arg(here)));
            boxes += here;
        }
        QVERIFY(boxes >= 1 && boxes <= kMostFloatsAtAParagraph);
    }

    // The paragraphs of a frame, and the cells of a table set at a place, have no lines of their own: they are on the page of the paragraph they are set from, which may be the
    // page after the one the text before them ends on.
    void positionsInAFrameOrAFloatingTableAreOnThePageOfTheirParagraph()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 80, 62, [](int i, QTextBlockFormat &bf) { if (i == 30 || i == 60) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 25, 45);
        // A frame right before the paragraph that begins a page, and a table set at a place right before the one that begins another.
        TestTable spec;
        spec.rows = 2;
        spec.cols = 2;
        spec.columnWidth = 60;
        spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        proto.hAlign = FloatingObject::Align::End;
        addFrame(fd, 30, {QStringLiteral("FRAMEWORDS first paragraph"), QStringLiteral("and a second of them")}, proto, 100);
        const TestTable table = addTableAfter(fd, 61, spec);   // (before the paragraph that was block 60, with its break: the frame's two paragraphs are before it now)
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 4);
        QSet<int> frame = frameBlocksOf(fd);
        QCOMPARE(int(frame.size()), 2);
        int frameFirst = *std::min_element(frame.begin(), frame.end());
        int frameLast = *std::max_element(frame.begin(), frame.end());
        const QTextBlock afterFrame = fd.story()->findBlock(frameLast).next();
        QVERIFY(afterFrame.isValid());
        const int home = pg.pageOfPosition(afterFrame.position());
        QVERIFY(home >= 1);
        QCOMPARE(pg.pageOfPosition(frameFirst), home);
        QCOMPARE(pg.pageOfPosition(frameFirst + 4), home);
        QCOMPARE(pg.pageOfPosition(frameLast + 3), home);
        QVERIFY(pg.pageOfPosition(fd.story()->findBlockByNumber(28).position() + 5) < home);   // (the text before the frame is on the page before)
        // The table's cells.
        const QTextBlock paragraphAfterTable = fd.story()->findBlock(table.rowStarts.last()).next();
        QTextBlock past = fd.story()->findBlock(table.rowStarts.first());
        while (past.isValid() && past.blockFormat().intProperty(fp::Cell)) past = past.next();
        QVERIFY(past.isValid());
        Q_UNUSED(paragraphAfterTable);
        const int tableHome = pg.pageOfPosition(past.position());
        QVERIFY(tableHome > home);
        for (const int rowStart : table.rowStarts) QCOMPARE(pg.pageOfPosition(rowStart + 2), tableHome);
        // And the page does draw them (so that the caret has a box to be in).
        const auto page = pg.materialize(home);
        QVERIFY(page);
        QVERIFY(frameBoxWith(*page, QStringLiteral("FRAMEWORDS")));
        QVERIFY(!page->frames.isEmpty() && page->frames[0].from == frameFirst);
        const auto tablePage = pg.materialize(tableHome);
        QVERIFY(tablePage && !tablePage->tables.isEmpty() && tablePage->tables[0]->floating);
    }

    // ---- floating tables

    // A table that is set at a place has no lines among the text: the text goes round it, and the page draws its rows where it is set, at the paragraph after the table
    // (the place is from that paragraph's top down, and from the margin's right edge across).
    void aTableSetAtAPlaceIsDrawnThereAndTheTextGoesRoundIt()
    {
        auto make = [](bool withTable) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 24, 51, {}, 25, 40);
            if (withTable) {
                TestTable spec;
                spec.rows = 3;
                spec.cols = 2;
                spec.columnWidth = 70;
                spec.setTable = [](TableData &t) {
                    TableData::Place p;
                    p.across = TableData::Place::From::Margin;
                    p.acrossSpec = TableData::Place::Spec::End;
                    p.y = 8;
                    p.room = QMarginsF(9, 0, 9, 0);
                    t.place = p;
                };
                addTableAfter(*fd, 5, spec);
            }
            return fd;
        };
        auto plain = make(false), floating = make(true);
        Paginator pp(plain.get()), pg(floating.get());
        QVERIFY(rowLines(pg, *floating, 0).isEmpty());
        // The first paragraph after the table.
        int anchorBlock = -1;
        for (QTextBlock b = floating->story()->findBlockByNumber(6); b.isValid() && anchorBlock < 0; b = b.next())
            if (!b.blockFormat().intProperty(fp::Cell)) anchorBlock = b.position();
        const double top = paragraphTopOn(pg, *floating, 0, anchorBlock);
        QVERIFY(top > 0);
        QVERIFY2(std::abs(top - paragraphTopOn(pp, *plain, 0, plain->story()->findBlockByNumber(6).position())) < 0.01, "the paragraph after the table is where it is without the table");
        const auto a = pageLines(pg, *floating, 0), b = pageLines(pp, *plain, 0);
        int above = 0;
        for (const PageLine &l : a) {
            if (l.block >= anchorBlock) break;
            QVERIFY2(above < b.size() && std::abs(b[above].rect.top() - l.rect.top()) < 0.01, qPrintable(audit::rectText(l.rect)));
            ++above;
        }
        QVERIFY(above > 10);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QCOMPARE(int(page->tables.size()), 1);
        const TableFragment &t = *page->tables[0];
        QVERIFY(t.floating);
        const flow::Section &sec = floating->sections[0];
        QVERIFY2(std::abs(t.rect().right() - (sec.pageSize().width() - sec.margins.right())) < 0.01 && std::abs(t.rect().width() - 140) < 0.01 && std::abs(t.rect().top() - (top + 8)) < 0.02,
                 qPrintable(audit::rectText(t.rect())));
        QVERIFY(t.height > 30 && t.endRow - t.firstRow == 3);
        // The text beside it stands clear of its place and the room (9 points at the sides): left of it.
        const QRectF wall = t.rect().adjusted(-9, 0, 9, 0);
        int beside = 0;
        for (const PageLine &l : a)
            if (l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05) {
                ++beside;
                QVERIFY2(l.rect.right() <= wall.left() + 0.05, qPrintable(audit::rectText(l.rect)));
            }
        QVERIFY2(beside >= 2, qPrintable(QString::number(beside)));
        QVERIFY2(audit::problems(pg, floating->story()).isEmpty(), qPrintable(audit::problems(pg, floating->story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, *floating, k).isEmpty(), qPrintable(audit::placementProblems(pg, *floating, k)));
        // Every paragraph that is not in the table has a line on a page.
        QSet<int> seen;
        for (int k = 0; k < pg.pageCount(); ++k)
            for (const auto &l : pg.lines(k)) seen.insert(l.block);
        for (QTextBlock x = floating->story()->begin(); x.isValid(); x = x.next())
            if (!x.blockFormat().intProperty(fp::Cell)) QVERIFY2(seen.contains(x.position()), qPrintable(QString::number(x.position())));
    }

    // A table that says it is set at a place but is too tall for one (more rows than a page's text area holds) is in the text, with its rows as lines.
    void aTableSetAtAPlaceThatIsTooTallIsInTheText()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 6, 52, {}, 25, 40);
        TestTable spec;
        spec.rows = 80;
        spec.cols = 2;
        spec.columnWidth = 70;
        spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; t.place = p; };
        addTableAfter(fd, 2, spec);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        int rows = 0;
        for (int k = 0; k < pg.pageCount(); ++k) {
            rows += int(rowLines(pg, fd, k).size());
            const auto page = pg.materialize(k);
            QVERIFY(page);
            for (const auto &t : page->tables) QVERIFY(!t->floating);
        }
        QVERIFY(rows >= 80);
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
    }

    // A floating table is on the page of the paragraph after it (the one that begins a page: the page it begins), and an edit in its cells lays that page out again: the pages
    // are those a new paginator makes, and the table is as tall as its cells need.
    void aFloatingTableIsOnThePageOfItsParagraphAndEditsInItLayThatPageOut()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 120, 53, [](int i, QTextBlockFormat &bf) { if (i == 60) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 25, 45);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 70;
        spec.text = [](int r, int c) { return QStringLiteral("cell %1 %2").arg(r).arg(c); };
        spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
        const TestTable table = addTableAfter(fd, 59, spec);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 6);
        int onPage = -1;
        double height = 0;
        for (int k = 0; k < pages; ++k) {
            const auto m = pg.materialize(k);
            QVERIFY(m);
            if (!m->tables.isEmpty()) {
                QCOMPARE(onPage, -1);
                onPage = k;
                height = m->tables[0]->height;
            }
        }
        QVERIFY(onPage > 0);
        // (The paragraph after the table begins that page: a break comes before it.)
        QCOMPARE(pg.page(onPage).start.pos, fd.story()->findBlockByNumber(59 + 1 + 6).position());
        // Typing in a cell: the page is laid out again, the table is taller, and the pages are a new paginator's.
        QTextCursor c(fd.story());
        c.setPosition(table.rowStarts[1] + 3);
        c.insertText(QStringLiteral(" many more words typed into the cell so that it takes several lines of it and the row grows"));
        Paginator fresh(&fd);
        const QString bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        const auto m = pg.materialize(onPage);
        QVERIFY(m && !m->tables.isEmpty());
        QVERIFY2(m->tables[0]->height > height + 10, qPrintable(QStringLiteral("%1 then %2").arg(height).arg(m->tables[0]->height)));
    }

    // The same for the tables set at places: the objects made of them are let go from their cache while pages hold them.
    void floatingTablesInUseSurviveTheirCacheTurningOver()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 30, 56, {}, 30, 60);
        for (const int after : {8, 4}) {   // (both on the first page: the second is made while the first is held)
            TestTable spec;
            spec.rows = 2;
            spec.cols = 2;
            spec.columnWidth = 70;
            spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
            addTableAfter(fd, after, spec);
        }
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        QTextCursor cur(fd.story());
        for (int i = 0; i < 260; ++i) {
            cur.setPosition(fd.story()->findBlockByNumber(26).position() + 3);
            cur.insertText(QStringLiteral("x"));
            QVERIFY(pg.pageCount() >= 2);
            const auto page = pg.materialize(0);
            QVERIFY(page);
            if (i % 100 == 0) QVERIFY(!page->tables.isEmpty());
        }
        Paginator fresh(&fd);
        const QString bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // Tables set at places one after another, before one paragraph, are all drawn, side by side where they ask; and one at the end of the story (no paragraph after it to be set
    // from) is rows in the text.
    void floatingTablesTogetherAndAtTheEndOfTheStory()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 14, 54, {}, 25, 40);
        auto add = [&](int after, TableData::Place::Spec side) {
            TestTable spec;
            spec.rows = 2;
            spec.cols = 1;
            spec.columnWidth = 60;
            spec.setTable = [side](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = side; p.room = QMarginsF(6, 0, 6, 0); t.place = p; };
            return addTableAfter(fd, after, spec);
        };
        add(5, TableData::Place::Spec::Start);
        // (After the first table's paragraphs: its last cell is a block before the paragraph that follows.)
        {
            QTextBlock last = fd.story()->findBlockByNumber(5);
            while (last.next().isValid() && last.next().blockFormat().intProperty(fp::Cell)) last = last.next();
            int n = 0;
            for (QTextBlock x = fd.story()->begin(); x != last; x = x.next()) ++n;
            add(n, TableData::Place::Spec::End);
        }
        TestTable end;
        end.rows = 1;
        end.cols = 1;
        end.columnWidth = 50;
        end.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; t.place = p; };
        addTable(fd, end);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 1);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QVector<std::shared_ptr<TableFragment>> floating;
        for (const auto &t : page->tables)
            if (t->floating) floating << t;
        QCOMPARE(int(floating.size()), 2);   // (the one at the end of the story is rows in the text, on this page or a later one)
        QVERIFY(floating[0]->rect().left() < floating[1]->rect().left());
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY(pg.materialize(k));
        // The one at the end of the story has no paragraph to be set from: it is rows in the text, among the lines, where it is.
        QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
        bool inText = false;
        for (int k = 0; k < pg.pageCount(); ++k) {
            const auto made = pg.materialize(k);
            QVERIFY(made);
            for (const auto &t : made->tables) inText |= !t->floating;
        }
        QVERIFY(inText);
    }

    // A file stores a paragraph property by its number: the numbers of the properties that files already have are not moved by a new one (it goes at the end).
    void theNumbersOfTheStoredPropertiesAreNotMovedByNewOnes()
    {
        QCOMPARE(int(tp::NoHangPunct), int(tp::BorderBottomOff) + 1);
        QVERIFY2(int(tp::OutOfLayout) > int(tp::NoHangPunct), "a property added after the files that store the others goes after them");
    }

    // Objects of every kind put among paragraphs of a few lines to a few pages, in one column or two: on every page the paginator's lines are where the page layer sets the text
    // round the objects (to a hundredth of a point), and clear of the walls. (JP_FUZZ=n: n documents for a longer run.)
    void randomWallsAreWhereThePageLayerPutsTheText()
    {
        using Wr = FloatingObject::Wrap;
        using Rl = FloatingObject::Rel;
        const unsigned docs = qEnvironmentVariableIsSet("JP_FUZZ") ? std::max(150u, qEnvironmentVariable("JP_FUZZ").toUInt() * 10) : 150u;
        QStringList reports;
        for (unsigned seed = 1; seed <= docs; ++seed) {
            std::mt19937 rng(seed * 104729u);
            FlowDocument fd;
            fd.env.wordLines = true;
            const int paras = 20 + int(rng() % 60);
            fillStory(fd.story(), paras, seed, [&](int, QTextBlockFormat &bf) { bf.setBottomMargin(6); if (rng() % 17 == 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 5, 90);
            if (rng() % 3 == 0) {
                Section b = fd.sections[0];
                b.columns = 2;
                b.columnGap = 20;
                fd.sections[0] = b;
            }
            for (unsigned i = 0, n = 3 + rng() % 40; i < n; ++i) {
                const Rl hs[] = {Rl::Column, Rl::Margin, Rl::Page, Rl::Character, Rl::LeftMargin, Rl::RightMargin};
                const Rl vs[] = {Rl::Paragraph, Rl::Line, Rl::Margin, Rl::Page, Rl::TopMargin, Rl::BottomMargin};
                const Wr ws[] = {Wr::Square, Wr::Square, Wr::TopBottom, Wr::Tight, Wr::None};
                const FloatingObject::Side sides[] = {FloatingObject::Side::Both, FloatingObject::Side::Left, FloatingObject::Side::Right, FloatingObject::Side::Largest};
                const double w = 10 + double(rng() % 500), h = 10 + double(rng() % (rng() % 4 == 0 ? 900 : 200));
                const int block = int(rng() % unsigned(paras));
                const int id = addWrapped(&fd, w, h, fd.story()->findBlockByNumber(block).position() + int(rng() % 4), ws[rng() % 5], double(int(rng() % 600) - 100), double(int(rng() % 700) - 350),
                                          QMarginsF(double(rng() % 20), double(rng() % 20), double(rng() % 20), double(rng() % 20)), sides[rng() % 4], hs[rng() % 6], vs[rng() % 6]);
                if (rng() % 4 == 0) fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align(1 + rng() % 3);
                if (rng() % 8 == 0) fd.objects[size_t(id) - 1].behind = true;
            }
            Paginator pg(&fd);
            QString bad = audit::wallProblems(pg, fd.story());
            if (bad.isEmpty()) bad = audit::coverageProblems(pg, fd.story());
            if (!bad.isEmpty()) reports << QStringLiteral("seed %1: %2").arg(seed).arg(bad.left(300));
        }
        QVERIFY2(reports.isEmpty(), qPrintable(QStringLiteral("%1 of %2 documents:\n%3").arg(reports.size()).arg(docs).arg(reports.mid(0, 8).join(QLatin1Char('\n')))));
    }

    // ---- what the objects cost

    // A paragraph of many runs (every other word in bold: 20,000 runs) costs no more with an object anchored in another paragraph than without: what a page looks for in the
    // runs of the paragraphs it holds is found once for the paragraph, not for each of its lines.
    void aParagraphOfManyRunsCostsNoMoreWithAnObjectOnThePage()
    {
        qint64 took[2] = {0, 0};
        for (int variant = 0; variant < 2; ++variant) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 3, 5, {}, 30, 40);
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            c.insertBlock();
            for (int i = 0; i < 20000; ++i) {
                QTextCharFormat f;
                f.setFontPointSize(11);
                f.setFontWeight(i % 2 ? QFont::Bold : QFont::Normal);
                c.insertText(QStringLiteral("word "), f);
            }
            if (variant == 1) {
                auto item = std::make_shared<ShapeItem>();
                item->rect = QRectF(0, 0, 50, 40);
                const int id = addObject(&fd, item, fd.story()->findBlockByNumber(0).position() + 2, FloatingObject::Rel::Page, FloatingObject::Rel::Page, 500, 700);
                fd.objects[size_t(id) - 1].wrap = FloatingObject::Wrap::None;
            }
            QElapsedTimer t;
            t.start();
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() > 20);
            took[variant] = t.elapsed();
        }
        qInfo("20,000 runs: %lld ms with an object on the page, %lld ms without", took[1], took[0]);
        QVERIFY2(took[1] < 2 * took[0] + 300, qPrintable(QStringLiteral("%1 ms with the object, %2 ms without").arg(took[1]).arg(took[0])));
    }

    // Ten thousand objects anchored in one paragraph cost little more than the paragraph does without them (anchor marks that anchor nothing): the anchors are cut at the most a
    // page is given as they are found, character by character, not paragraph by paragraph, and each is found by its place in the paragraph, not by looking at all its lines.
    void manyObjectsInOneParagraphAreCutAsTheyAreFound()
    {
        qint64 took[2] = {0, 0};
        for (int variant = 0; variant < 2; ++variant) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 6, 5, {}, 80, 120);
            const int n = 10000;
            for (int i = 0; i < n; ++i) {
                auto item = std::make_shared<ShapeItem>();
                item->rect = QRectF(0, 0, 10, 10);
                if (variant == 1) {
                    const int id = addObject(&fd, item, fd.story()->findBlockByNumber(2).position() + 2, FloatingObject::Rel::Column, FloatingObject::Rel::Paragraph, (i % 40) * 11, (i / 40) * 11);
                    fd.objects[size_t(id) - 1].wrap = FloatingObject::Wrap::Square;
                    fd.objects[size_t(id) - 1].gap = QMarginsF(2, 2, 2, 2);
                } else {
                    // (The same marks in the text, and no object for any: the document has none.)
                    QTextCursor c(fd.story());
                    c.setPosition(fd.story()->findBlockByNumber(2).position() + 2);
                    QTextCharFormat anchor;
                    anchor.setFontPointSize(11);
                    anchor.setProperty(fp::AnchorId, i + 1);
                    c.insertText(QString(QChar(0xFFFC)), anchor);
                }
            }
            QElapsedTimer t;
            t.start();
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            for (int k = 0; k < pages; ++k) QVERIFY(pg.materialize(k));
            took[variant] = t.elapsed();
        }
        qInfo("10,000 anchor marks: %lld ms with the objects, %lld ms without", took[1], took[0]);
        QVERIFY2(took[1] < 2 * took[0] + 300, qPrintable(QStringLiteral("%1 ms with 10,000 objects, %2 ms with none").arg(took[1]).arg(took[0])));
    }

    // A page with walls is laid out again to find where they are, once; the walls it ended with are kept, and the page asked for again (its lines, the page made) is laid out
    // once, not found again.
    void aPageWithWallsIsLaidOutOnceWhenAskedAgain()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 40, 63, {}, 25, 45);
        const int id = addWrapped(&fd, 150, 90, fd.story()->findBlockByNumber(3).position() + 2, FloatingObject::Wrap::Square, 0, 0);
        fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 3);
        QVERIFY(pg.materialize(0));   // (the first time the page is made: its walls are found)
        qint64 before = pg.pagePassesForTest();
        pg.lines(0);
        QCOMPARE(pg.pagePassesForTest() - before, qint64(1));
        before = pg.pagePassesForTest();
        QVERIFY(pg.materialize(0));
        QCOMPARE(pg.pagePassesForTest() - before, qint64(1));
        before = pg.pagePassesForTest();
        pg.lines(0);
        QVERIFY(pg.materialize(0));
        QCOMPARE(pg.pagePassesForTest() - before, qint64(2));
        // After an edit it is found again (and the pages are those of a new paginator).
        QTextCursor c(fd.story());
        c.setPosition(fd.story()->findBlockByNumber(1).position() + 4);
        c.insertText(QStringLiteral("some more words typed before the object, to move it a little. "));
        Paginator fresh(&fd);
        const QString bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        before = pg.pagePassesForTest();
        pg.lines(0);
        pg.lines(0);
        QVERIFY(pg.pagePassesForTest() - before <= 4);   // (the first after the edit finds them, the second does not)
    }

    // A table set at a place floats only if it fits the text area of the section it is in: one that is taller than a page of its own section's is in the text, though it would
    // fit a page of another section's.
    void aTableSetAtAPlaceFloatsOnlyIfItFitsItsOwnSection()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 30, 64, [](int i, QTextBlockFormat &bf) { if (i == 14) bf.setProperty(fp::SectionBreakAfter, true); }, 20, 35);
        Section small = fd.sections[0];
        small.paper = QSizeF(300, 230);
        small.margins = QMarginsF(36, 36, 36, 36);
        small.continuous = false;
        fd.sections << small;
        TestTable spec;
        spec.rows = 12;
        spec.cols = 2;
        spec.columnWidth = 50;
        spec.text = [](int r, int c) { return QStringLiteral("r%1c%2").arg(r).arg(c); };
        spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
        addTableAfter(fd, 22, spec);   // (in the second section, whose pages are 158 points of text tall: the table is taller than 0.9 of that)
        addTableAfter(fd, 6, spec);    // (in the first, whose pages are 648 tall)
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 3);
        int floating = 0, inText = 0, secondSectionFloating = 0;
        for (int k = 0; k < pg.pageCount(); ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            for (const auto &t : page->tables) {
                if (t->floating) {
                    ++floating;
                    secondSectionFloating += page->section == 1;
                } else {
                    ++inText;
                }
            }
        }
        QCOMPARE(floating, 1);
        QCOMPARE(secondSectionFloating, 0);
        QVERIFY(inText >= 1);
        QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
    }

    // A table set at a place from its paragraph, near the foot of the text area, does not go over the foot (or the margin below it, to the page's edge): it is moved, with its
    // paragraph, to the top of the next page.
    void aFloatingTableThatWouldGoOverTheFootGoesToThePageAfter()
    {
        auto make = [](int tableBefore) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 150, 71, {}, 3, 6);
            if (tableBefore > 0) {
                TestTable spec;
                spec.rows = 6;
                spec.cols = 2;
                spec.columnWidth = 60;
                spec.text = [](int r, int c) { return QStringLiteral("r%1c%2").arg(r).arg(c); };
                spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
                addTableAfter(*fd, tableBefore - 1, spec);
            }
            return fd;
        };
        // A paragraph whose line is a little above the foot of the first page (less than a table's height above it).
        int paragraph = -1;
        {
            auto plain = make(0);
            Paginator pg(plain.get());
            const double bottom = pg.page(0).content.bottom() - pg.page(0).content.top();
            for (const auto &l : pg.lines(0))
                if (l.line == 0 && l.rect.top() > bottom - 70 && l.rect.top() < bottom - 25 && paragraph < 0) {
                    int n = 0;
                    for (QTextBlock b = plain->story()->begin(); b.isValid() && b.position() < l.block; b = b.next()) ++n;
                    paragraph = n;
                }
        }
        QVERIFY(paragraph > 3);
        auto fd = make(paragraph);
        Paginator pg(fd.get());
        QVERIFY(pg.pageCount() >= 4);
        const QTextBlock after = [&]() { QTextBlock b = fd->story()->begin(); while (b.isValid() && !b.blockFormat().intProperty(fp::Cell)) b = b.next(); while (b.isValid() && b.blockFormat().intProperty(fp::Cell)) b = b.next(); return b; }();
        QVERIFY(after.isValid());
        // The paragraph the table is set from is the first on the second page; the table is on that page, whole inside the text area.
        QCOMPARE(pg.page(1).start.pos, after.position());
        QCOMPARE(pg.page(1).start.skip, 0);
        for (int k = 0; k < pg.pageCount(); ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            const flow::Paginator::Page info = pg.page(k);
            for (const auto &t : page->tables) {
                if (!t->floating) continue;
                QCOMPARE(k, 1);
                QVERIFY2(t->rect().bottom() <= info.content.bottom() + 0.05, qPrintable(audit::rectText(t->rect()) + QStringLiteral(" in ") + audit::rectText(info.content)));
                QVERIFY(t->rect().top() >= info.content.top() - 0.05);
            }
        }
        QVERIFY2(audit::floatProblems(*&pg, *fd).isEmpty(), qPrintable(audit::floatProblems(pg, *fd)));
        QVERIFY2(audit::coverageProblems(pg, fd->story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd->story())));
        QVERIFY2(audit::problems(pg, fd->story()).isEmpty(), qPrintable(audit::problems(pg, fd->story())));
        // An edit before it: the pages are those of a new paginator (the move is found again on the page it was on, and the table stays with its paragraph).
        QTextCursor c(fd->story());
        c.setPosition(fd->story()->findBlockByNumber(2).position() + 2);
        c.insertText(QStringLiteral("a few words typed on the first page. "));
        Paginator fresh(fd.get());
        const QString bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        // The same table with room above it is on the first page, where it goes.
        auto roomy = make(8);
        Paginator pr(roomy.get());
        const auto first = pr.materialize(0);
        QVERIFY(first);
        int floating = 0;
        for (const auto &t : first->tables) floating += t->floating;
        QCOMPARE(floating, 1);
    }

    // Rows put in at the top of a table set at a place, by an edit that begins in the paragraph before the table (the end of its text, where the new cells' paragraphs are
    // opened): the table is taller, the text that goes round it has more lines, and the pages from its paragraph on are laid out again (they are those of a new paginator).
    void rowsPutInAtTheTopOfAFloatingTableLayOutThePagesAfterIt()
    {
        QStringList failures;
        int moved = 0;
        for (unsigned seed = 1; seed <= 40; ++seed) {
            FlowDocument fd;
            fd.env.wordLines = true;
            // (The paragraph after the table, which it is set from, begins the second page: the first page ends at the table, and the edit's end is where it ends.)
            fillStory(fd.story(), 40, seed, [](int i, QTextBlockFormat &bf) { if (i == 6) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 10, 80);
            TestTable spec;
            spec.rows = 2;
            spec.cols = 2;
            spec.columnWidth = 60;
            spec.text = [](int r, int c) { return QStringLiteral("r%1c%2").arg(r).arg(c); };
            spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
            const TestTable table = addTableAfter(fd, 5, spec);
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() >= 3);
            const int linesBefore = int(pg.lines(1).size());
            const QTextBlock prev = fd.story()->findBlock(table.rowStarts[0]).previous();
            QVERIFY(prev.isValid() && !prev.blockFormat().intProperty(fp::Cell));
            auto row = std::make_shared<RowData>(*fd.tables.get(table.keys[0])->row);
            QTextCursor c(fd.story());
            c.setPosition(prev.position() + prev.length() - 1);
            QTextCharFormat body;
            body.setFontPointSize(11);
            c.beginEditBlock();
            for (int col = 0; col < 2; ++col) {
                CellRecord rec = *fd.tables.get(table.keys[size_t(col)]);
                rec.row = row;
                QTextBlockFormat bf;
                bf.setProperty(fp::Cell, fd.tables.add(rec));
                for (int line = 0; line < 4; ++line) {
                    c.insertBlock(bf, body);
                    c.insertText(QStringLiteral("a new cell's paragraph %1").arg(line), body);
                }
            }
            c.endEditBlock();
            Paginator fresh(&fd);
            const QString bad = audit::differenceFromFresh(pg, fresh);
            if (!bad.isEmpty()) failures << QStringLiteral("seed %1: %2").arg(seed).arg(bad.left(200));
            moved += int(pg.lines(1).size()) != linesBefore;
        }
        QVERIFY2(failures.isEmpty(), qPrintable(QStringLiteral("%1 of 40:\n%2").arg(failures.size()).arg(failures.mid(0, 4).join(QLatin1Char('\n')))));
        QVERIFY2(moved >= 5, qPrintable(QString::number(moved)));   // (the text round the taller table has other lines)
    }

    // A table set at a place a distance down from its paragraph that would go over the foot of the text area, but that has room between the paragraph's top and the foot, is
    // slid up to end at the foot: it is on the page of its paragraph, in the text area, and not above its paragraph.
    void aFloatingTableOffsetOverTheFootIsSlidUpToTheTextArea()
    {
        auto make = [](int tableBefore) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 150, 71, {}, 3, 6);
            if (tableBefore > 0) {
                TestTable spec;
                spec.rows = 6;
                spec.cols = 2;
                spec.columnWidth = 60;
                spec.text = [](int r, int c) { return QStringLiteral("r%1c%2").arg(r).arg(c); };
                spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.y = 70; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
                addTableAfter(*fd, tableBefore - 1, spec);
            }
            return fd;
        };
        int paragraph = -1;
        double paragraphTop = 0;
        {
            auto plain = make(0);
            Paginator pg(plain.get());
            const double bottom = pg.page(0).content.bottom() - pg.page(0).content.top();
            for (const auto &l : pg.lines(0))
                if (l.line == 0 && l.rect.top() > bottom - 150 && l.rect.top() < bottom - 130 && paragraph < 0) {
                    int n = 0;
                    for (QTextBlock b = plain->story()->begin(); b.isValid() && b.position() < l.block; b = b.next()) ++n;
                    paragraph = n;
                    paragraphTop = pg.page(0).content.top() + l.rect.top();
                }
        }
        QVERIFY(paragraph > 3);
        auto fd = make(paragraph);
        Paginator pg(fd.get());
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const flow::Paginator::Page info = pg.page(0);
        QVector<std::shared_ptr<TableFragment>> floating;
        for (const auto &t : page->tables)
            if (t->floating) floating << t;
        QCOMPARE(int(floating.size()), 1);
        QVERIFY2(floating[0]->rect().bottom() <= info.content.bottom() + 0.05, qPrintable(audit::rectText(floating[0]->rect()) + QStringLiteral(" in ") + audit::rectText(info.content)));
        QVERIFY2(floating[0]->rect().bottom() > info.content.bottom() - 0.5, "slid up to end at the foot, not higher");
        QVERIFY2(floating[0]->rect().top() >= paragraphTop - 0.05, "not above its paragraph");
        QVERIFY2(audit::wallProblems(pg, fd->story()).isEmpty(), qPrintable(audit::wallProblems(pg, fd->story())));
        QVERIFY2(audit::floatProblems(pg, *fd).isEmpty(), qPrintable(audit::floatProblems(pg, *fd)));
    }

    // An object that is set from the page, tall, whose wall (the text beside it narrowed) pushes its own anchor paragraph off the page, and without it the paragraph is back: the
    // page has no state that agrees. The paragraph goes to the next page with the object, which wraps the text there; on the page before, no line goes through it.
    void anObjectWhoseWallPushesItsAnchorOffThePageGoesToTheNextWithIt()
    {
        auto make = [](int anchorBlock) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 60, 72, {}, 20, 40);
            if (anchorBlock > 0) {
                const int id = addWrapped(fd.get(), 260, 420, fd->story()->findBlockByNumber(anchorBlock).position() + 2, FloatingObject::Wrap::Square, 280, 200, QMarginsF(8, 4, 8, 4), FloatingObject::Side::Both,
                                          FloatingObject::Rel::Page, FloatingObject::Rel::Page);
                Q_UNUSED(id);
            }
            return fd;
        };
        int overlaps = 0, tried = 0, cycles = 0;
        for (int block = 6; block < 16; ++block) {
            auto fd = make(block);
            Paginator pg(fd.get());
            QVERIFY(pg.pageCount() >= 2);
            ++tried;
            QVERIFY2(audit::coverageProblems(pg, fd->story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd->story())));
            for (int k = 0; k < pg.pageCount(); ++k) {
                const auto page = pg.materialize(k);
                QVERIFY(page);
                const Item *picture = objectOfSize(*page, 260, 420);
                if (!picture) continue;
                const flow::Paginator::Page info = pg.page(k);
                for (const auto &l : pg.lines(k)) {
                    const QRectF r(info.content.left() + l.rect.left(), info.content.top() + l.rect.top(), l.rect.width(), l.rect.height() - l.below);
                    if (r.intersects(picture->rect.adjusted(0.1, 0.1, -0.1, -0.1))) ++overlaps;
                }
                cycles += k > 0 && pg.page(k).start.pos == fd->story()->findBlockByNumber(block).position();
            }
            QVERIFY2(audit::wallProblems(pg, fd->story()).isEmpty(), qPrintable(audit::wallProblems(pg, fd->story())));
        }
        QCOMPARE(tried, 10);
        QVERIFY2(overlaps == 0, qPrintable(QStringLiteral("%1 lines go through the object").arg(overlaps)));
        qInfo("%d of %d anchors begin the page their object is on", cycles, tried);
    }

    // ---- text round objects

    // Text goes round an object that has a square wrap: the object is at the top of its paragraph (as the text sets that, not moved by the object), the lines beside it
    // stand clear of it and the room round it, and the lines below have the column's width again.
    void squareWrapKeepsLinesBesideAnObjectClearOfItsRectangle()
    {
        auto make = [](FloatingObject::Wrap wrap) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 30, 21, {}, 20, 40);
            const int at = fd->story()->findBlockByNumber(3).position() + 2;
            const int id = addWrapped(fd.get(), 150, 100, at, wrap, 0, 0);
            fd->objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
            return fd;
        };
        auto plain = make(FloatingObject::Wrap::None), wrapped = make(FloatingObject::Wrap::Square);
        Paginator pp(plain.get()), pg(wrapped.get());
        const int block = wrapped->story()->findBlockByNumber(3).position();
        const double top = paragraphTopOn(pp, *plain, 0, block);
        QVERIFY(top > 0);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const Item *obj = objectOfSize(*page, 150, 100);
        QVERIFY(obj);
        const QRectF column = wrapped->sections[0].columnRect(0);
        QVERIFY2(std::abs(obj->rect.top() - top) < 0.01, qPrintable(QStringLiteral("the object is at %1, its paragraph's top is %2 without it").arg(obj->rect.top()).arg(top)));
        QVERIFY(std::abs(obj->rect.right() - column.right()) < 0.01);
        const QRectF wall = obj->rect.adjusted(-8, -4, 8, 4);
        int beside = 0, above = 0, below = 0;
        for (const PageLine &l : pageLines(pg, *wrapped, 0)) {
            const bool inBand = l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05;
            if (inBand) {
                ++beside;
                QVERIFY2(l.rect.right() <= wall.left() + 0.05 && l.rect.left() >= column.left() - 0.05, qPrintable(QStringLiteral("a line beside the object is at %1").arg(audit::rectText(l.rect))));
            } else {
                (l.rect.top() < wall.top() ? above : below) += 1;
                QVERIFY2(std::abs(l.rect.width() - column.width()) < 0.5, qPrintable(QStringLiteral("a line outside the object's band is %1 wide (the column is %2)").arg(l.rect.width()).arg(column.width())));
            }
        }
        QVERIFY2(beside >= 5 && above >= 3 && below >= 3, qPrintable(QStringLiteral("%1 lines beside, %2 above, %3 below").arg(beside).arg(above).arg(below)));
        QVERIFY2(audit::problems(pg, wrapped->story()).isEmpty(), qPrintable(audit::problems(pg, wrapped->story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, *wrapped, k).isEmpty(), qPrintable(audit::placementProblems(pg, *wrapped, k)));
        QVERIFY2(textIsWholeOnThePages(pg, *wrapped).isEmpty(), qPrintable(textIsWholeOnThePages(pg, *wrapped)));
        // The room the object takes is room the pages lack.
        QVERIFY(pg.pageCount() >= pp.pageCount());
    }

    // An empty paragraph is only its mark, and goes in any gap between objects that is a few points wide: a column of empty paragraphs between two pictures that are a little apart
    // stays a column there (each a line below the one before), where a paragraph of words, which has no room there, is below the pictures.
    void anEmptyParagraphGoesInANarrowGapBetweenObjects()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        // Twelve empty paragraphs, and then one of words. The pictures are 180 wide each, with 9 points round them, and 15 points between: the column is 468 wide.
        {
            QTextCursor c(fd.story());
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            QTextBlockFormat bf;
            c.setBlockFormat(bf);
            for (int i = 0; i < 12; ++i) c.insertBlock(bf, cf);
            c.insertText(QStringLiteral("The words of a paragraph that has no room between the pictures, so that it goes on below them, as words do."), cf);
        }
        // Anchored in the first paragraph: one against the left edge, one at 213 (15 + 18 between: the walls are 9 round each, so the gap is 15 to the right of the first).
        const int left = addWrapped(&fd, 180, 150, 0, FloatingObject::Wrap::Square, 0, 0);
        const int right = addWrapped(&fd, 180, 150, 0, FloatingObject::Wrap::Square, 213, 0);
        Q_UNUSED(left);
        Q_UNUSED(right);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const auto walls = audit::wallsOf(*page);
        QCOMPARE(int(walls.size()), 2);
        const double bottom = std::max(walls[0].rect.bottom(), walls[1].rect.bottom());
        const auto lines = pageLines(pg, fd, 0);
        QVERIFY(lines.size() >= 13);
        // The empty paragraphs are one under another, each a line (the pitch of the first two), in the gap between the pictures; the gap is 15 wide, as the walls leave it.
        const double pitch = lines[2].rect.top() - lines[1].rect.top();
        QVERIFY(pitch > 8 && pitch < 30);
        int inGap = 0;
        for (int i = 1; i < 12; ++i) {
            QVERIFY2(std::abs(lines[i].rect.top() - lines[1].rect.top() - (i - 1) * pitch) < 0.05, qPrintable(QStringLiteral("line %1 at %2").arg(i).arg(audit::rectText(lines[i].rect))));
            if (lines[i].rect.width() < 20) ++inGap;
        }
        QVERIFY2(inGap >= 6, qPrintable(QStringLiteral("%1 lines in the gap").arg(inGap)));
        // The words go below the pictures.
        QVERIFY2(lines.last().rect.top() >= bottom - 0.05, qPrintable(QStringLiteral("the words at %1, the pictures end at %2").arg(lines.last().rect.top()).arg(bottom)));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, fd, k).isEmpty(), qPrintable(audit::placementProblems(pg, fd, k)));
    }

    // The objects of a page are drawn in the order of their heights, whatever the order of their anchors: those behind the text first, then the text, then the rest from the lowest.
    void objectsAreDrawnInTheOrderOfTheirHeights()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 10, 47, {}, 20, 30);
        const int at = fd.story()->findBlockByNumber(2).position() + 2;
        auto box = [&](double w, int z, bool behind) {
            const int id = addWrapped(&fd, w, 10, at, FloatingObject::Wrap::None, 0, 0);
            fd.objects[size_t(id) - 1].z = z;
            fd.objects[size_t(id) - 1].behind = behind;
        };
        box(11, 5, false);
        box(12, 1, false);
        box(13, 3, false);
        box(14, -1, true);
        box(15, -2, true);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QVector<int> widths;
        for (const auto &it : page->doc->pages[0]->items) widths << (it->type() == ItemType::Shape ? int(std::lround(it->rect.width())) : 0);
        // (The two behind first, the lower of them first; the text box; the others from the lowest up.)
        QCOMPARE(widths, (QVector<int>{15, 14, 0, 12, 13, 11}));
    }

    // An object anchored by a character that shows nothing (a file's picture is one) at the end of a paragraph is on the paragraph's last line, and it is set from there; one at
    // the start of the paragraph is on its first line.
    void anObjectAnchoredAtTheEndOfAParagraphIsOnItsLastLine()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 12, 38, {}, 25, 40);
        const QTextBlock block = fd.story()->findBlockByNumber(4);
        const int end = block.position() + block.length() - 1;
        auto anchorAt = [&](int position, double dy) {
            auto item = std::make_shared<ShapeItem>();
            item->rect = QRectF(0, 0, 30, 20);
            FloatingObject o;
            o.item = item;
            o.hRel = FloatingObject::Rel::Column;
            o.vRel = FloatingObject::Rel::Line;
            o.dy = dy;
            fd.objects.push_back(o);
            QTextCursor c(fd.story());
            c.setPosition(position);
            QTextCharFormat anchor;
            anchor.setFontPointSize(11);
            anchor.setProperty(fp::AnchorId, int(fd.objects.size()));
            anchor.setProperty(tp::Field, QStringLiteral("raw:"));   // (it shows nothing)
            c.insertText(QString(QChar(0xFFFC)), anchor);
        };
        anchorAt(end, 1);
        anchorAt(block.position(), 2);   // (the start of the paragraph)
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const auto lines = pageLines(pg, fd, 0);
        double first = -1, last = -1;
        for (const PageLine &l : lines)
            if (l.block == block.position()) {
                if (first < 0) first = l.rect.top();
                last = l.rect.top();
            }
        QVERIFY(last > first);
        int found = 0;
        for (const auto &it : page->doc->pages[0]->items)
            if (it->type() == ItemType::Shape) {
                ++found;
                const double y = it->rect.top();
                QVERIFY2(std::abs(y - (last + 1)) < 0.02 || std::abs(y - (first + 2)) < 0.02, qPrintable(QStringLiteral("an object is at %1 (the paragraph's lines begin at %2, the last at %3)").arg(y).arg(first).arg(last)));
            }
        QCOMPARE(found, 2);
    }

    // A wrap that leaves no text beside the object: no line is in the object's band at all, the text above ends before it, the text below begins where it ends, and the
    // text does not move the object (it is at the top of its paragraph, where that is without it).
    void topAndBottomWrapLeavesNoTextBesideTheObject()
    {
        auto make = [](FloatingObject::Wrap wrap) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 30, 22, {}, 20, 40);
            const int at = fd->story()->findBlockByNumber(4).position() + 2;
            addWrapped(fd.get(), 120, 90, at, wrap, 100, 0);
            return fd;
        };
        auto plain = make(FloatingObject::Wrap::None), wrapped = make(FloatingObject::Wrap::TopBottom);
        Paginator pp(plain.get()), pg(wrapped.get());
        const int block = wrapped->story()->findBlockByNumber(4).position();
        const double top = paragraphTopOn(pp, *plain, 0, block);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const Item *obj = objectOfSize(*page, 120, 90);
        QVERIFY(obj);
        QVERIFY2(std::abs(obj->rect.top() - top) < 0.01, qPrintable(QStringLiteral("the object is at %1, its paragraph's top is %2 without it").arg(obj->rect.top()).arg(top)));
        const QRectF wall = obj->rect.adjusted(-8, -4, 8, 4);
        const QRectF column = wrapped->sections[0].columnRect(0);
        double lastAbove = -1, firstBelow = 1e9;
        for (const PageLine &l : pageLines(pg, *wrapped, 0)) {
            QVERIFY2(!(l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05), qPrintable(QStringLiteral("a line at %1 is in the band of %2").arg(audit::rectText(l.rect), audit::rectText(wall))));
            if (l.rect.bottom() <= wall.top() + 0.05) lastAbove = std::max(lastAbove, l.rect.bottom());
            else firstBelow = std::min(firstBelow, l.rect.top());
            QVERIFY(std::abs(l.rect.width() - column.width()) < 0.5);
        }
        QVERIFY(lastAbove > 0 && firstBelow < 1e8);
        QVERIFY2(firstBelow >= wall.bottom() - 0.05 && firstBelow - wall.bottom() < 0.05, qPrintable(QStringLiteral("the text below begins at %1, the room ends at %2").arg(firstBelow).arg(wall.bottom())));
        QVERIFY(firstBelow - lastAbove >= wall.height() - 0.05);
        QVERIFY2(audit::problems(pg, wrapped->story()).isEmpty(), qPrintable(audit::problems(pg, wrapped->story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, *wrapped, k).isEmpty(), qPrintable(audit::placementProblems(pg, *wrapped, k)));
        QVERIFY2(textIsWholeOnThePages(pg, *wrapped).isEmpty(), qPrintable(textIsWholeOnThePages(pg, *wrapped)));
    }

    // The side of an object that the text goes on: both (the lines beside it are in two pieces), only the left, only the right, or the side with more room.
    void wrapSidesPutTheTextOnTheSidesAsked()
    {
        using Side = FloatingObject::Side;
        for (const Side side : {Side::Both, Side::Left, Side::Right, Side::Largest}) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 24, 23, {}, 25, 45);
            const int at = fd.story()->findBlockByNumber(3).position() + 2;
            // 100 wide, 150 points in from the column's left: 142 of room to its left (with the 8 points), and 468 - 258 = 210 to its right.
            addWrapped(&fd, 100, 120, at, FloatingObject::Wrap::Square, 150, 0, QMarginsF(8, 4, 8, 4), side);
            Paginator pg(&fd);
            const auto page = pg.materialize(0);
            QVERIFY(page);
            const Item *obj = objectOfSize(*page, 100, 120);
            QVERIFY(obj);
            const QRectF wall = obj->rect.adjusted(-8, -4, 8, 4);
            int left = 0, right = 0;
            for (const PageLine &l : pageLines(pg, fd, 0)) {
                if (!(l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05)) continue;
                if (l.rect.right() <= wall.left() + 0.05) ++left;
                else if (l.rect.left() >= wall.right() - 0.05) ++right;
                else QFAIL(qPrintable(QStringLiteral("a line beside the object is at %1, over it (%2)").arg(audit::rectText(l.rect), audit::rectText(wall))));
            }
            const QString what = QStringLiteral("side %1: %2 lines on the left and %3 on the right").arg(int(side)).arg(left).arg(right);
            if (side == Side::Both) QVERIFY2(left > 3 && right > 3, qPrintable(what));
            else if (side == Side::Left) QVERIFY2(left > 3 && right == 0, qPrintable(what));
            else QVERIFY2(right > 3 && left == 0, qPrintable(what));   // (the right: the larger)
            QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(what + audit::problems(pg, fd.story())));
            for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, fd, k).isEmpty(), qPrintable(audit::placementProblems(pg, fd, k)));
        }
    }

    // An object behind the text or over it, and one with no wrap, move no line: the pages' lines are those of a document without them.
    void objectsBehindAndOverTheTextMoveNoLine()
    {
        auto linesOf = [](int variant) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 30, 24, {}, 20, 40);
            const int at = fd.story()->findBlockByNumber(3).position() + 2;
            if (variant > 0) {
                const int id = addWrapped(&fd, 150, 100, at, variant == 1 ? FloatingObject::Wrap::None : FloatingObject::Wrap::Square, 100, 0);
                fd.objects[size_t(id) - 1].behind = variant == 2;
            } else {
                QTextCursor c(fd.story());
                c.setPosition(at);
                QTextCharFormat anchor;
                anchor.setFontPointSize(11);
                c.insertText(QString(QChar(0xFFFC)), anchor);
            }
            Paginator pg(&fd);
            QStringList out;
            for (int k = 0; k < pg.pageCount(); ++k)
                for (const auto &l : pg.lines(k)) out << QStringLiteral("%1-%2 %3").arg(l.pos).arg(l.end).arg(audit::rectText(l.rect));
            return out;
        };
        const QStringList none = linesOf(0), over = linesOf(1), behind = linesOf(2);
        QVERIFY(none.size() > 40);
        QCOMPARE(over, none);
        QCOMPARE(behind, none);   // (a square wrap behind the text is no wrap)
    }

    // Square and tight wrap are the same until the contours are read: the pages' lines are the same.
    void tightWrapIsSquareWrapForNow()
    {
        auto linesOf = [](FloatingObject::Wrap wrap) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 30, 25, {}, 20, 40);
            addWrapped(&fd, 130, 110, fd.story()->findBlockByNumber(2).position() + 2, wrap, 40, 0);
            Paginator pg(&fd);
            QStringList out;
            for (int k = 0; k < pg.pageCount(); ++k)
                for (const auto &l : pg.lines(k)) out << QStringLiteral("%1-%2 %3").arg(l.pos).arg(l.end).arg(audit::rectText(l.rect));
            return out;
        };
        QCOMPARE(linesOf(FloatingObject::Wrap::Tight), linesOf(FloatingObject::Wrap::Square));
    }

    // Objects set in the strips between the page's edge and the margins: from the left strip's edge (the page's), the right strip's (the margin's), the top's and foot's;
    // aligned to the start, middle, or end of the strip. Inside and outside are the left and right strips across, and the top and foot down.
    void objectsInTheMarginStripsAreWhereTheStripPutsThem()
    {
        using Rel = FloatingObject::Rel;
        using Align = FloatingObject::Align;
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 10, 26, {}, 20, 40);
        fd.sections[0].margins = QMarginsF(90, 60, 54, 80);   // left, top, right, bottom
        const int at = fd.story()->findBlockByNumber(2).position() + 2;
        auto box = [&](double w, double h, Rel hRel, Rel vRel, double dx, double dy) { return addWrapped(&fd, w, h, at, FloatingObject::Wrap::None, dx, dy, QMarginsF(), FloatingObject::Side::Both, hRel, vRel); };
        const int a = box(20, 10, Rel::LeftMargin, Rel::TopMargin, 5, 3);
        const int b = box(21, 11, Rel::RightMargin, Rel::BottomMargin, 4, 2);
        const int c = box(22, 12, Rel::LeftMargin, Rel::TopMargin, 0, 0);
        fd.objects[size_t(c) - 1].hAlign = Align::Center;
        fd.objects[size_t(c) - 1].vAlign = Align::End;
        const int d = box(23, 13, Rel::InsideMargin, Rel::InsideMargin, 0, 0);
        fd.objects[size_t(d) - 1].hAlign = Align::End;
        fd.objects[size_t(d) - 1].vAlign = Align::Center;
        const int e = box(24, 14, Rel::OutsideMargin, Rel::OutsideMargin, 0, 0);
        fd.objects[size_t(e) - 1].hAlign = Align::Center;
        fd.objects[size_t(e) - 1].vAlign = Align::Start;
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const double W = 612, H = 792;
        auto at2 = [&](double w, double h) { const Item *it = objectOfSize(*page, w, h); return it ? it->rect : QRectF(); };
        QVERIFY(a > 0 && b > 0 && c > 0 && d > 0 && e > 0);
        // From the page's left edge (the strip begins there), and from the page's top.
        QCOMPARE(at2(20, 10).topLeft(), QPointF(5, 3));
        // From the margin's right edge (the right strip begins there), and from the margin's foot.
        QCOMPARE(at2(21, 11).topLeft(), QPointF(W - 54 + 4, H - 80 + 2));
        // The middle of the left strip (90 wide) across, the end of the top strip (60 high) down.
        QCOMPARE(at2(22, 12).topLeft(), QPointF((90 - 22) / 2.0, 60 - 12));
        // Inside: the left strip across (its end) and the top strip down (its middle).
        QCOMPARE(at2(23, 13).topLeft(), QPointF(90 - 23, (60 - 13) / 2.0));
        // Outside: the right strip across (its middle) and the foot strip down (its start).
        QCOMPARE(at2(24, 14).topLeft(), QPointF(W - 54 + (54 - 24) / 2.0, H - 80));
    }

    // A paragraph that goes on to the page after the one with the object has its lines there as the text sets them: whole, full width, none lost or said twice; and the
    // pages are those a new paginator makes after edits above and below the object.
    void aParagraphThatRunsOnPastTheObjectsPageIsWhole()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 8, 27, [](int, QTextBlockFormat &bf) { bf.setBottomMargin(8); }, 380, 520);
        // Right-aligned, 200 by 260, at the top of the second paragraph, which has the lines of two pages.
        const int id = addWrapped(&fd, 200, 260, fd.story()->findBlockByNumber(1).position() + 3, FloatingObject::Wrap::Square, 0, 0);
        fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 5);
        QVERIFY2(textIsWholeOnThePages(pg, fd).isEmpty(), qPrintable(textIsWholeOnThePages(pg, fd)));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, fd, k).isEmpty(), qPrintable(audit::placementProblems(pg, fd, k)));
        // The pages after the one with the object are full width throughout (the paragraph's lines on the next page are not set round an object that is not there).
        const double width = fd.sections[0].columnWidth();
        int fullLines = 0;
        for (int k = 1; k < pg.pageCount(); ++k)
            for (const PageLine &l : pageLines(pg, fd, k)) {
                QVERIFY2(std::abs(l.rect.width() - width) < 0.5, qPrintable(QStringLiteral("page %1: a line is %2 wide").arg(k).arg(l.rect.width())));
                ++fullLines;
            }
        QVERIFY(fullLines > 80);
        // Edits above and below the object; the pages are those of a new paginator.
        QTextCursor cur(fd.story());
        cur.setPosition(5);
        cur.insertText(QStringLiteral("typed above the object, which moves what follows it by some words that fill a line or two of the page, the object going with its paragraph. "));
        cur.setPosition(fd.story()->findBlockByNumber(6).position() + 10);
        cur.insertText(QStringLiteral("and below it "));
        Paginator fresh(&fd);
        const QString bad = audit::differenceFromFresh(pg, fresh);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        QVERIFY2(textIsWholeOnThePages(pg, fd).isEmpty(), qPrintable(textIsWholeOnThePages(pg, fd)));
    }

    // A paragraph of more than two pages with a wall in its first lines: the page after the one with the wall holds on from where that page ended, and so do those
    // after it. (Each page keeps only where its text begins and whether the paragraph's lines before it were set round a wall: the third page of the paragraph must
    // know that the second one was, or it lays the paragraph out again from its start and skips the lines it takes for the second page's.)
    void aParagraphOfSeveralPagesWithAWallLosesNoText()
    {
        int bad = 0;
        QStringList reports;
        for (unsigned seed = 1; seed <= 24; ++seed) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 3, seed, {}, 700 + int(seed) * 60, 900 + int(seed) * 60);
            const int id = addWrapped(&fd, 120 + double(seed % 5) * 40, 160 + double(seed % 7) * 30, fd.story()->findBlockByNumber(seed % 2).position() + 3, FloatingObject::Wrap::Square, 0, 0);
            fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() >= 3);
            const QString whole = textIsWholeOnThePages(pg, fd);
            if (!whole.isEmpty()) {
                ++bad;
                reports << QStringLiteral("seed %1 (%2 pages): %3").arg(seed).arg(pg.pageCount()).arg(whole);
            }
            for (int k = 0; k < pg.pageCount(); ++k)
                if (!pg.materialize(k)) {
                    ++bad;
                    reports << QStringLiteral("seed %1: page %2 has nothing to make").arg(seed).arg(k);
                }
        }
        QVERIFY2(bad == 0, qPrintable(QStringLiteral("%1 of 24 documents lose or repeat text:\n%2").arg(bad).arg(reports.join(QLatin1Char('\n')))));
    }

    // Objects of every kind put among paragraphs of hundreds of words (many pages each, one or two columns): the text on the pages is the story's text, whole and in order,
    // and no page is empty. (JP_FUZZ=n: n documents, for a longer run.)
    void randomWallsAmongLongParagraphsLoseNoText()
    {
        using Wr = FloatingObject::Wrap;
        using Rl = FloatingObject::Rel;
        const unsigned docs = qEnvironmentVariableIsSet("JP_FUZZ") ? std::max(60u, qEnvironmentVariable("JP_FUZZ").toUInt() * 4) : 60u;
        QStringList reports;
        for (unsigned seed = 1; seed <= docs; ++seed) {
            std::mt19937 rng(seed * 104729u);
            FlowDocument fd;
            fd.env.wordLines = true;
            const int paras = 3 + int(rng() % 8);
            fillStory(fd.story(), paras, seed, [&](int, QTextBlockFormat &bf) { bf.setBottomMargin(6); }, 300, 900);
            if (rng() % 3 == 0) {
                Section b = fd.sections[0];
                b.columns = 2;
                b.columnGap = 20;
                fd.sections[0] = b;
            }
            for (unsigned i = 0, n = 2 + rng() % 12; i < n; ++i) {
                const Rl hs[] = {Rl::Column, Rl::Margin, Rl::Page, Rl::Character, Rl::LeftMargin, Rl::RightMargin};
                const Rl vs[] = {Rl::Paragraph, Rl::Line, Rl::Margin, Rl::Page, Rl::TopMargin, Rl::BottomMargin};
                const Wr ws[] = {Wr::Square, Wr::Square, Wr::TopBottom, Wr::Tight, Wr::None};
                const FloatingObject::Side sides[] = {FloatingObject::Side::Both, FloatingObject::Side::Left, FloatingObject::Side::Right, FloatingObject::Side::Largest};
                const double w = 10 + double(rng() % 400), h = 10 + double(rng() % (rng() % 4 == 0 ? 700 : 200));
                const int block = int(rng() % unsigned(paras));
                const QTextBlock b = fd.story()->findBlockByNumber(block);
                const int at = b.position() + int(rng() % unsigned(std::min(b.length(), 400)));
                const int id = addWrapped(&fd, w, h, at, ws[rng() % 5], double(int(rng() % 500) - 100), double(int(rng() % 500) - 200),
                                          QMarginsF(double(rng() % 20), double(rng() % 20), double(rng() % 20), double(rng() % 20)), sides[rng() % 4], hs[rng() % 6], vs[rng() % 6]);
                if (rng() % 4 == 0) fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align(1 + rng() % 3);
            }
            Paginator pg(&fd);
            QString bad = textIsWholeOnThePages(pg, fd);
            for (int k = 0; bad.isEmpty() && k < pg.pageCount(); ++k)
                if (pg.lines(k).isEmpty()) bad = QStringLiteral("page %1 has no lines").arg(k);
            if (!bad.isEmpty()) reports << QStringLiteral("seed %1 (%2 pages, %3 words in all): %4").arg(seed).arg(pg.pageCount()).arg(fd.story()->characterCount() / 6).arg(bad);
        }
        QVERIFY2(reports.isEmpty(), qPrintable(QStringLiteral("%1 of %2 documents lose text or have an empty page:\n%3").arg(reports.size()).arg(docs).arg(reports.join(QLatin1Char('\n')))));
    }

    // An edit lays out again the pages from the one it is on until a page starts where it did: the objects on the pages before it, and on those after, cost nothing.
    void anEditAmongObjectsLaysOutOnlyThePagesItTouches()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 240, 28, {}, 30, 60);
        for (const int block : {5, 40, 80, 170, 220}) {
            const int id = addWrapped(&fd, 120, 80, fd.story()->findBlockByNumber(block).position() + 2, FloatingObject::Wrap::Square, 0, 0);
            fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
        }
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 12);
        const qint64 before = pg.pagesLaidOut();
        QTextCursor cur(fd.story());
        cur.setPosition(fd.story()->findBlockByNumber(120).position() + 5);
        cur.insertText(QStringLiteral("a few typed words"));
        QCOMPARE(pg.pageCount(), pages);
        const qint64 cost = pg.pagesLaidOut() - before;
        QVERIFY2(cost <= 3, qPrintable(QStringLiteral("%1 pages laid out again for one typed word").arg(cost)));
        Paginator fresh(&fd);
        const QString bad = audit::differenceFromFresh(pg, fresh, false);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // An object as large as the page, or larger, leaves a line no room, and a page that kept none would never end: the page is laid out as it would be without that wrap, so the
    // pagination ends, every page has lines, and the text is whole.
    void anObjectThatFillsThePageDoesNotStopThePagination()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 20, 29, {}, 30, 60);
        addWrapped(&fd, 700, 900, fd.story()->findBlockByNumber(0).position() + 1, FloatingObject::Wrap::Square, -50, -50);
        addWrapped(&fd, 700, 900, fd.story()->findBlockByNumber(5).position() + 1, FloatingObject::Wrap::TopBottom, -50, -50);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 2 && pages < 40);
        for (int k = 0; k < pages; ++k) QVERIFY2(!pg.lines(k).isEmpty(), qPrintable(QStringLiteral("page %1 has no lines").arg(k)));
        QVERIFY2(textIsWholeOnThePages(pg, fd).isEmpty(), qPrintable(textIsWholeOnThePages(pg, fd)));
        const auto first = pg.materialize(0);
        QVERIFY(first);
        QCOMPARE(audit::wallsOf(*first).size(), qsizetype(0));   // (the page as it is laid out without the walls: they are drawn, and the text goes through them)
        QCOMPARE(int(first->doc->pages[0]->items.size()) - int(first->boxes.size()), 2);
    }

    // A file can anchor an object at every paragraph: the page takes the first ones as walls, as many as it ever takes, and the pagination is as it is for any text.
    void aPageGivesAtMostSomeObjectsAWrap()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 600, 30, {}, 4, 10);
        for (int block = 0; block < 600; ++block) addWrapped(&fd, 10, 10, fd.story()->findBlockByNumber(block).position() + 1, FloatingObject::Wrap::Square, 400, 0);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 5 && pages < 60);
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            QVERIFY2(audit::wallsOf(*page).size() <= 24, qPrintable(QStringLiteral("page %1 has %2 walls").arg(k).arg(audit::wallsOf(*page).size())));
        }
        QVERIFY2(textIsWholeOnThePages(pg, fd).isEmpty(), qPrintable(textIsWholeOnThePages(pg, fd)));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
    }

    // In columns an object is a wall in the column of its anchor: the lines of the other column are those of a document without the object.
    void anObjectInOneColumnLeavesTheOtherColumnAlone()
    {
        auto make = [](FloatingObject::Wrap wrap, int anchor) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fd->sections[0].columns = 2;
            fd->sections[0].columnGap = 36;
            fillStory(fd->story(), 40, 31, {}, 10, 25);
            if (anchor > 0) {
                const int id = addWrapped(fd.get(), 90, 70, anchor + 1, wrap, 0, 0);
                fd->objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
            }
            return fd;
        };
        // A paragraph that is in the second column of the first page, as the text sets them out without the object.
        int anchor = -1;
        {
            auto probe = make(FloatingObject::Wrap::None, -1);
            Paginator pp(probe.get());
            for (const auto &l : pp.lines(0))
                if (l.frame == 1 && l.line == 0 && anchor < 0) anchor = l.block;
        }
        QVERIFY(anchor > 0);
        auto with = make(FloatingObject::Wrap::Square, anchor), without = make(FloatingObject::Wrap::None, anchor);
        Paginator pw(with.get()), po(without.get());
        const auto page = pw.materialize(0);
        QVERIFY(page);
        const Item *obj = objectOfSize(*page, 90, 70);
        QVERIFY(obj);
        QVERIFY2(with->sections[0].columnRect(1).contains(obj->rect), qPrintable(audit::rectText(obj->rect)));
        // The first column is as it is without the object, line for line; the second has the lines beside the object short.
        const auto a = pageLines(pw, *with, 0), b = pageLines(po, *without, 0);
        int firstColumn = 0, matched = 0, shortened = 0;
        for (const PageLine &l : a)
            if (l.column == 0) {
                ++firstColumn;
                for (const PageLine &o : b)
                    if (o.column == 0 && o.pos == l.pos && o.end == l.end && std::abs(o.rect.top() - l.rect.top()) < 0.01 && std::abs(o.rect.width() - l.rect.width()) < 0.01) {
                        ++matched;
                        break;
                    }
            }
        QVERIFY(firstColumn > 20);
        QCOMPARE(matched, firstColumn);
        const QRectF wall = obj->rect.adjusted(-8, -4, 8, 4);
        for (const PageLine &l : a)
            if (l.column == 1 && l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05) {
                ++shortened;
                QVERIFY(l.rect.right() <= wall.left() + 0.05);
            }
        QVERIFY(shortened >= 3);
        QVERIFY2(audit::problems(pw, with->story()).isEmpty(), qPrintable(audit::problems(pw, with->story())));
        QVERIFY2(textIsWholeOnThePages(pw, *with).isEmpty(), qPrintable(textIsWholeOnThePages(pw, *with)));
    }

    // A wrap that leaves no text beside an object is a wall across the columns the object is in, and not across the others: in three columns, an object in the third leaves
    // the first two as they are, and the page layer (which draws the page, and lays the text out again) has the lines where the paginator put them.
    void aTopAndBottomObjectIsAWallOnlyInTheColumnsItIsIn()
    {
        auto make = [](FloatingObject::Wrap wrap, int anchor) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fd->sections[0].columns = 3;
            fd->sections[0].columnGap = 24;
            fd->sections[0].margins = QMarginsF(54, 54, 54, 54);
            fillStory(fd->story(), 70, 37, {}, 10, 25);
            if (anchor > 0) {
                const int id = addWrapped(fd.get(), 60, 50, anchor + 1, wrap, 0, 20);
                fd->objects[size_t(id) - 1].hAlign = FloatingObject::Align::Center;
            }
            return fd;
        };
        int anchor = -1;
        {
            auto probe = make(FloatingObject::Wrap::None, -1);
            Paginator pp(probe.get());
            for (const auto &l : pp.lines(0))
                if (l.frame == 2 && l.line == 0 && anchor < 0) anchor = l.block;
        }
        QVERIFY(anchor > 0);
        auto with = make(FloatingObject::Wrap::TopBottom, anchor), without = make(FloatingObject::Wrap::None, anchor);
        Paginator pw(with.get()), po(without.get());
        const auto page = pw.materialize(0);
        QVERIFY(page);
        const Item *obj = objectOfSize(*page, 60, 50);
        QVERIFY(obj);
        const Section &sec = with->sections[0];
        QVERIFY2(sec.columnRect(2).contains(obj->rect), qPrintable(audit::rectText(obj->rect)));
        const auto a = pageLines(pw, *with, 0), b = pageLines(po, *without, 0);
        int others = 0, matched = 0, third = 0;
        const QRectF wall = obj->rect.adjusted(-8, -4, 8, 4);
        for (const PageLine &l : a) {
            if (l.column < 2) {
                ++others;
                for (const PageLine &o : b)
                    if (o.column == l.column && o.pos == l.pos && o.end == l.end && std::abs(o.rect.top() - l.rect.top()) < 0.01) {
                        ++matched;
                        break;
                    }
                continue;
            }
            ++third;
            QVERIFY2(!(l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05), qPrintable(audit::rectText(l.rect)));
        }
        QVERIFY(others > 40 && third > 10);
        QCOMPARE(matched, others);
        QVERIFY2(audit::problems(pw, with->story()).isEmpty(), qPrintable(audit::problems(pw, with->story())));
        for (int k = 0; k < pw.pageCount(); ++k) {
            const auto m = pw.materialize(k);
            QVERIFY(m);
            const QString bad = audit::pageLayerProblems(pw, with->story(), k, *m);
            QVERIFY2(bad.isEmpty(), qPrintable(bad));
        }
    }

    // Text before a table and after it goes round an object that is on the page; the table's rows are not moved by it.
    void textAfterATableGoesRoundAnObjectOnThePage()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 6, 32, {}, 20, 40);
        TestTable spec;
        spec.rows = 5;
        spec.cols = 2;
        spec.columnWidth = 150;
        const TestTable table = addTable(fd, spec);
        Q_UNUSED(table);
        appendParagraph(fd, QStringLiteral("The paragraph after the table, which says a good many words so that it takes more than a line or two of the page when it is set beside an object that stands in its way, and says a few more."));
        for (int i = 0; i < 6; ++i) appendParagraph(fd, QStringLiteral("Another paragraph of the text after the table, with enough words in it to take several lines of the page: the court held that section applies to every defendant who appeals."));
        // A page-relative object across the lower half of the text area, to the right.
        const int id = addWrapped(&fd, 170, 200, fd.story()->findBlockByNumber(0).position() + 1, FloatingObject::Wrap::Square, 300, 330, QMarginsF(8, 4, 8, 4), FloatingObject::Side::Both, FloatingObject::Rel::Page, FloatingObject::Rel::Page);
        Q_UNUSED(id);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 1);
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, fd, k).isEmpty(), qPrintable(audit::placementProblems(pg, fd, k)));
        const auto page = pg.materialize(0);
        QVERIFY(page && !page->tables.isEmpty());
        // Some of the text that comes after the table is beside the object.
        const QRectF wall = QRectF(300, 330, 170, 200).adjusted(-8, -4, 8, 4);
        int beside = 0;
        for (const PageLine &l : pageLines(pg, fd, 0))
            if (l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05) ++beside;
        QVERIFY2(beside >= 4, qPrintable(QStringLiteral("%1 lines beside the object").arg(beside)));
    }

    // An object placed from a line, above that line, is a wall for the lines above it too; where the page is laid out until the objects and the lines agree, they agree.
    void anObjectAboveItsAnchorLineWrapsTheLinesAbove()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 30, 33, {}, 20, 40);
        const int at = fd.story()->findBlockByNumber(8).position() + 20;
        const int id = addWrapped(&fd, 140, 90, at, FloatingObject::Wrap::Square, 0, -60, QMarginsF(8, 4, 8, 4), FloatingObject::Side::Both, FloatingObject::Rel::Column, FloatingObject::Rel::Line);
        fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
        Paginator pg(&fd);
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, fd, k).isEmpty(), qPrintable(audit::placementProblems(pg, fd, k)));
        QVERIFY2(textIsWholeOnThePages(pg, fd).isEmpty(), qPrintable(textIsWholeOnThePages(pg, fd)));
        const auto page = pg.materialize(0);
        const Item *obj = page ? objectOfSize(*page, 140, 90) : nullptr;
        QVERIFY(obj);
        // It is above the line that holds its anchor.
        double anchorTop = -1;
        for (const PageLine &l : pageLines(pg, fd, 0))
            if (at >= l.pos && at < l.end) anchorTop = l.rect.top();
        QVERIFY(anchorTop > 0);
        QVERIFY2(obj->rect.top() < anchorTop - 20, qPrintable(QStringLiteral("the object is at %1, the line at %2").arg(obj->rect.top()).arg(anchorTop)));
    }

    // An object anchored in the text of a table's cell is drawn where its line is and wraps nothing: the pages' lines are those of a document without a wrap.
    void anObjectInATableCellIsNoWall()
    {
        auto linesOf = [](FloatingObject::Wrap wrap, int *walls) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 4, 34, {}, 20, 40);
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            spec.columnWidth = 120;
            const TestTable table = addTable(fd, spec);
            for (int i = 0; i < 8; ++i) appendParagraph(fd, QStringLiteral("A paragraph after the table with some words in it, which the court held applies to every defendant who appeals from a judgment entered after trial."));
            addWrapped(&fd, 100, 60, table.rowStarts[1] + 1, wrap, 130, 0);
            Paginator pg(&fd);
            QStringList out;
            for (int k = 0; k < pg.pageCount(); ++k)
                for (const auto &l : pg.lines(k)) out << QStringLiteral("%1-%2 %3").arg(l.pos).arg(l.end).arg(audit::rectText(l.rect));
            const auto page = pg.materialize(0);
            *walls = page ? audit::wallsOf(*page).size() : -1;
            return out;
        };
        int a = 0, b = 0;
        const QStringList square = linesOf(FloatingObject::Wrap::Square, &a), none = linesOf(FloatingObject::Wrap::None, &b);
        QCOMPARE(square, none);
        QCOMPARE(a, 0);
    }

    // The objects in a header or footer are not walls for the body's text, which is another layer: the body's lines are those of a document without them.
    void anObjectInTheHeaderIsNoWallForTheBody()
    {
        auto linesOf = [](FloatingObject::Wrap wrap) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 30, 35, {}, 20, 40);
            auto header = furnitureStory({QStringLiteral("HEADER"), QStringLiteral("second line")});
            auto item = std::make_shared<ShapeItem>();
            item->rect = QRectF(0, 0, 200, 150);
            FloatingObject o;
            o.item = item;
            o.hRel = FloatingObject::Rel::Margin;
            o.vRel = FloatingObject::Rel::Page;
            o.dy = 30;
            o.wrap = wrap;
            fd.objects.push_back(o);
            QTextCursor c(header.get());
            c.setPosition(header->findBlockByNumber(1).position());
            QTextCharFormat anchor;
            anchor.setProperty(fp::AnchorId, 1);
            c.insertText(QString(QChar(0xFFFC)), anchor);
            fd.sections[0].header.odd = header;
            fd.sections[0].headerDistance = 30;
            Paginator pg(&fd);
            QStringList out;
            for (int k = 0; k < pg.pageCount(); ++k)
                for (const auto &l : pg.lines(k)) out << QStringLiteral("%1-%2 %3").arg(l.pos).arg(l.end).arg(audit::rectText(l.rect));
            const auto page = pg.materialize(0);
            if (page) out << QStringLiteral("walls %1").arg(audit::wallsOf(*page).size());
            return out;
        };
        QStringList a = linesOf(FloatingObject::Wrap::Square), b = linesOf(FloatingObject::Wrap::None);
        QVERIFY(a.size() > 30);
        QCOMPARE(a, b);   // (and no wall: the last entry says how many)
        QCOMPARE(a.last(), QStringLiteral("walls 0"));
    }

    // An object is anchored by a character of the story it is in: a header has the objects its own characters anchor, and the body's first paragraph (at the same place in its
    // story) has its own, and each is drawn once.
    void anObjectIsDrawnOnceWhenThePageHasAHeaderToo()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 20, 36, {}, 20, 40);
        fd.sections[0].header.odd = furnitureStory({QStringLiteral("HEADER TEXT at the top of the page")});
        addObject(&fd, [] { auto it = std::make_shared<ShapeItem>(); it->rect = QRectF(0, 0, 77, 33); return it; }(), fd.story()->findBlockByNumber(0).position() + 2, FloatingObject::Rel::Page, FloatingObject::Rel::Page, 300, 500);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        int count = 0;
        for (const auto &it : page->doc->pages[0]->items) count += it->type() == ItemType::Shape && std::abs(it->rect.width() - 77) < 0.01 && std::abs(it->rect.height() - 33) < 0.01;
        QCOMPARE(count, 1);
    }

    // The header and the footer are the lowest layer of the page: drawn before the body's objects (an object in front of the text is over them), and the text of the header is not
    // pushed aside by the body's objects, whose walls keep to the text area.
    void theHeaderAndFooterAreDrawnBeforeTheBodysObjects()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 30, 35, {}, 20, 40);
        fd.sections[0].header.odd = furnitureStory({QStringLiteral("HEADER TEXT at the top of the page")});
        fd.sections[0].footer.odd = furnitureStory({QStringLiteral("FOOTER TEXT at the foot of the page")});
        fd.sections[0].headerDistance = 30;
        fd.sections[0].footerDistance = 30;
        // An object over the page's top, taller than the margin (it reaches into the text area), and one over its foot; both ask the text to go round them, and one is behind the text.
        const int up = addWrapped(&fd, 300, 110, fd.story()->findBlockByNumber(0).position() + 2, FloatingObject::Wrap::Square, 100, 10, QMarginsF(6, 4, 6, 4), FloatingObject::Side::Both, FloatingObject::Rel::Page, FloatingObject::Rel::Page);
        const int down = addWrapped(&fd, 200, 120, fd.story()->findBlockByNumber(0).position() + 3, FloatingObject::Wrap::Square, 200, 700, QMarginsF(6, 4, 6, 4), FloatingObject::Side::Both, FloatingObject::Rel::Page, FloatingObject::Rel::Page);
        fd.objects[size_t(down) - 1].behind = true;
        Q_UNUSED(up);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const auto &items = page->doc->pages[0]->items;
        auto indexOf = [&](const QString &prefix) {
            for (size_t i = 0; i < items.size(); ++i)
                if (items[i]->id.startsWith(prefix)) return int(i);
            return -1;
        };
        auto indexOfSize = [&](double w, double h) {
            for (size_t i = 0; i < items.size(); ++i)
                if (items[i]->type() == ItemType::Shape && items[i]->wrap.mode == Wrap::None && std::abs(items[i]->rect.width() - w) < 0.01 && std::abs(items[i]->rect.height() - h) < 0.01) return int(i);
            return -1;
        };
        // (Each object is drawn once, the header's and footer's own anchors not taken for the body's.)
        int topCount = 0, footCount = 0;
        for (const auto &it : items) {
            topCount += it->type() == ItemType::Shape && std::abs(it->rect.width() - 300) < 0.01 && std::abs(it->rect.height() - 110) < 0.01 && it->wrap.mode == Wrap::None;
            footCount += it->type() == ItemType::Shape && std::abs(it->rect.width() - 200) < 0.01 && std::abs(it->rect.height() - 120) < 0.01 && it->wrap.mode == Wrap::None;
        }
        QCOMPARE(topCount, 1);
        QCOMPARE(footCount, 1);
        const int header = indexOf(QStringLiteral("flow-header-box")), footer = indexOf(QStringLiteral("flow-footer-box")), top = indexOfSize(300, 110), foot = indexOfSize(200, 120);
        QVERIFY(header >= 0 && footer >= 0 && top >= 0 && foot >= 0);
        QVERIFY2(header < top && footer < top, "an object in front of the text is over the header and the footer");
        QVERIFY2(header < foot && footer < foot, "an object behind the text is over them too: they are the lowest layer");
        // The body's text is round the objects, as before, and the walls keep to the text area.
        const auto walls = audit::wallsOf(*page);
        QVERIFY(walls.size() >= 1);
        const flow::Paginator::Page info = pg.page(0);
        for (const auto &w : walls) QVERIFY2(w.rect.top() >= info.content.top() - 0.01 && w.rect.bottom() <= info.content.bottom() + 0.01, qPrintable(audit::rectText(w.rect)));
        QVERIFY2(audit::wallProblems(pg, fd.story()).isEmpty(), qPrintable(audit::wallProblems(pg, fd.story())));
        // The header's own lines are not moved by them: the page layer sets them across the whole of the box.
        LayoutCache cache;
        const auto *box = dynamic_cast<const TextItem *>(items[size_t(header)].get());
        QVERIFY(box);
        const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, page->number, RenderOptions());
        QVERIFY(fl.layout);
        QVERIFY(!fl.layout->lineBriefs().isEmpty());
        for (const auto &l : fl.layout->lineBriefs()) QVERIFY2(std::abs(l.rect.left()) < 0.01 && std::abs(l.rect.width() - box->rect.width()) < 0.01, qPrintable(audit::rectText(l.rect)));
    }

    // A text box that the text goes round is a wall as a picture is: the lines stand clear of it.
    void aTextBoxIsAWallAsAPictureIs()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 30, 36, {}, 20, 40);
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(0, 0, 160, 110);
        FloatingObject o;
        o.item = box;
        o.story = makeStory(2, 1, {}, 5, 9);
        o.hRel = FloatingObject::Rel::Column;
        o.vRel = FloatingObject::Rel::Paragraph;
        o.hAlign = FloatingObject::Align::End;
        o.wrap = FloatingObject::Wrap::Square;
        o.gap = QMarginsF(9, 0, 9, 0);
        fd.objects.push_back(o);
        QTextCursor c(fd.story());
        c.setPosition(fd.story()->findBlockByNumber(3).position() + 2);
        QTextCharFormat anchor;
        anchor.setFontPointSize(11);
        anchor.setProperty(fp::AnchorId, 1);
        c.insertText(QString(QChar(0xFFFC)), anchor);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QCOMPARE(audit::wallsOf(*page).size(), qsizetype(1));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
        for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, fd, k).isEmpty(), qPrintable(audit::placementProblems(pg, fd, k)));
        int beside = 0;
        const QRectF wall = audit::wallsOf(*page).first().rect;
        for (const PageLine &l : pageLines(pg, fd, 0))
            if (l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05) ++beside;
        QVERIFY(beside >= 5);
    }

    // The engine says how far wrapping moved a line down from where it would have gone, so that a paginator can find where a paragraph would have begun without it.
    void lineBriefsSayHowFarAWrapMovedALine()
    {
        auto story = makeStory(3, 5, {}, 15, 25);
        FrameSpec fs = plainFrame(300, 600);
        fs.obstacles << QPolygonF(QRectF(-1e5, 30, 2e5, 50));   // across the column, from 30 to 80
        StoryLayout lay;
        LayoutEnv env;
        env.wordLines = true;
        lay.build(story.get(), {fs}, env);
        double moved = 0;
        int pushed = 0;
        for (const auto &l : lay.lineBriefs()) {
            QVERIFY(l.pushed >= 0);
            if (l.pushed > 0) {
                ++pushed;
                moved = l.pushed;
                // It stands where the room ends (a word processor's text goes there at once).
                QVERIFY2(l.rect.top() >= 80 - 0.05 && l.rect.top() < 80.05, qPrintable(QString::number(l.rect.top())));
                QVERIFY(moved > 20);
            } else {
                QVERIFY2(l.rect.bottom() - l.below <= 30 + 0.05 || l.rect.top() >= 80 - 0.05, qPrintable(audit::rectText(l.rect)));
            }
        }
        QCOMPARE(pushed, 1);   // (only the first line that met it: the lines after it follow from there)
        // Without the wrap, no line is moved.
        FrameSpec free = plainFrame(300, 600);
        StoryLayout lay2;
        lay2.build(story.get(), {free}, env);
        for (const auto &l : lay2.lineBriefs()) QCOMPARE(l.pushed, 0.0);
    }

    // A section that is continuous and has the same page as the one before goes on the page the one before ends on; one with another page, or that is not
    // continuous, begins a page of its own.
    void continuousSectionsShareThePage()
    {
        auto pagesOf = [&](bool continuous, bool samePage) {
            FlowDocument fd;
            fillStory(fd.story(), 6, 3, [](int i, QTextBlockFormat &bf) {
                bf.setBottomMargin(6);
                if (i == 2) bf.setProperty(flow::fp::SectionBreakAfter, true);
            });
            Section second = fd.sections[0];
            second.continuous = continuous;
            if (!samePage) second.margins = QMarginsF(100, 72, 72, 72);
            fd.sections << second;
            Paginator pg(&fd);
            // (The section of the page each paragraph is on is the section the page starts in, and the first page's is the first.)
            QVector<int> sections;
            for (int k = 0; k < pg.pageCount(); ++k) sections << pg.page(k).start.section;
            return std::make_pair(pg.pageCount(), sections);
        };
        const auto apart = pagesOf(false, true), joined = pagesOf(true, true), other = pagesOf(true, false);
        QCOMPARE(apart.first, 2);
        QCOMPARE(joined.first, 1);
        QCOMPARE(other.first, 2);   // (a page of another size of margins can't be shared)
        QCOMPARE(apart.second.last(), 1);
    }

    // A continuous section with more columns than the one before begins below it on the same page, with its own columns.
    void continuousSectionWithOtherColumnsBeginsBelow()
    {
        FlowDocument fd;
        fillStory(fd.story(), 24, 5, [](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(6);
            if (i == 1) bf.setProperty(flow::fp::SectionBreakAfter, true);
        });
        Section second = fd.sections[0];
        second.continuous = true;
        second.columns = 2;
        second.columnGap = 20;
        fd.sections << second;
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        const auto page = pg.materialize(0);
        QCOMPARE(page->boxes.size(), 3);
        QVector<const TextItem *> box;
        for (const QString &id : page->boxes)
            for (const auto &it : page->doc->pages[0]->items)
                if (it->id == id) box << static_cast<const TextItem *>(it.get());
        QCOMPARE(box.size(), 3);
        const Section &one = fd.sections[0], &two = fd.sections[1];
        // The first section's text is in one column of the page's width; the second's below it, in two.
        QVERIFY(std::abs(box[0]->rect.width() - one.columnRect(0).width()) < 0.01);
        QVERIFY(std::abs(box[1]->rect.width() - two.columnRect(0).width()) < 0.01);
        QVERIFY(std::abs(box[2]->rect.left() - two.columnRect(1).left()) < 0.01);
        QVERIFY2(box[1]->rect.top() >= box[0]->rect.bottom() - 0.01, qPrintable(QString::number(box[1]->rect.top()) + " " + QString::number(box[0]->rect.bottom())));
        QVERIFY2(box[1]->rect.top() - box[0]->rect.bottom() < 40, qPrintable(QString::number(box[1]->rect.top() - box[0]->rect.bottom())));
        QCOMPARE(std::round(box[1]->rect.top() * 100), std::round(box[2]->rect.top() * 100));
        // The columns go to the foot of the page, and the next page starts in the second section, with its columns.
        const double foot = page->size.height() - one.margins.bottom();
        QVERIFY(std::abs(box[2]->rect.bottom() - foot) < 30);
        QCOMPARE(pg.page(1).start.section, 1);
        const auto next = pg.materialize(1);
        QVERIFY(next->boxes.size() >= 2);
        QVERIFY(std::abs(static_cast<const TextItem *>(next->doc->pages[0]->items.front().get())->rect.width() - two.columnRect(0).width()) < 0.01);
    }

    // Where a position is on the pages.
    void paginatorFindsThePageOfAPosition()
    {
        FlowDocument fd;
        fillStory(fd.story(), 100, 16);
        Paginator pg(&fd);
        QCOMPARE(pg.pageOfPosition(0), 0);
        // Asking for a place far on lays out as far as it needs, not more.
        const int far = fd.story()->findBlockByNumber(60).position() + 5;
        const int k = pg.pageOfPosition(far);
        QVERIFY(k > 3);
        QVERIFY(!pg.complete());
        QVERIFY(pg.page(k).start.pos <= far);
        QVERIFY(k + 1 >= pg.pagesKnown() || pg.page(k + 1).start.pos > far);
        QVERIFY(pg.pageCount() > k);
        QCOMPARE(pg.pageOfPosition(fd.story()->characterCount() - 1), pg.pageCount() - 1);
    }

    // 2,600 pages of text: past the 2,340 the one-shot layout can place, every page is whole.
    void paginatorLaysOutMoreThanTwoThousandPages()
    {
        FlowDocument fd;
        Section &sec = fd.sections[0];
        sec.paper = QSizeF(300, 300);
        sec.margins = QMarginsF(50, 50, 50, 50);
        fillStory(fd.story(), 34000, 17, {}, 2, 4);   // a line each
        Paginator pg(&fd);
        QElapsedTimer t;
        t.start();
        const int pages = pg.pageCount();
        QVERIFY2(pages > 2400, qPrintable(QString::number(pages)));
        qInfo("%d pages in %.1f s", pages, t.elapsed() / 1000.0);
        // Every page from the middle to the last has the lines of an ordinary page, and each line is inside it.
        const int perPage = pg.lines(100).size();
        QVERIFY(perPage > 8);
        for (int k : {1000, 2339, 2340, 2341, 2500, pages - 2}) {
            const auto lines = pg.lines(k);
            QVERIFY2(std::abs(lines.size() - perPage) <= 2, qPrintable(QStringLiteral("page %1 has %2 lines, not about %3").arg(k).arg(lines.size()).arg(perPage)));
            for (const auto &l : lines) {
                QVERIFY(l.rect.top() >= -0.01 && l.rect.bottom() <= sec.content().height() + 0.5);
                QVERIFY(l.rect.width() > 10);
            }
        }
        // Pages go on in order through the text.
        QVERIFY(pg.page(pages - 1).start.pos > pg.page(pages - 2).start.pos);
        QVERIFY(pg.page(pages - 1).start.pos < fd.story()->characterCount());
    }


    // ---- tables

    // A table's rows are set down the page between the paragraphs around it, each as tall as its cells need (a cell's own margins counted), and the
    // text after it begins where the last row ends.
    void aTableIsLaidOutInRowsBetweenTheTextAroundIt()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, [](int, QTextBlockFormat &bf) { bf.setBottomMargin(6); }, 4, 6);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.text = [](int r, int c) { return r == 1 && c == 1 ? QStringLiteral("a\nb\nc") : QStringLiteral("r%1c%2").arg(r).arg(c); };
        const TestTable table = addTable(fd, spec);
        QTextCursor c(fd.story());
        c.movePosition(QTextCursor::End);
        QTextBlockFormat after;
        after.setBottomMargin(6);
        c.insertBlock(after);
        c.insertText(QStringLiteral("after the table"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 1);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 3);
        QCOMPARE(rows[0].pos, table.rowStarts[0]);
        QCOMPARE(rows[2].pos, table.rowStarts[2]);
        // (A cell is as tall as its lines and its margins; a row as its tallest cell.)
        QVERIFY(std::abs(rows[0].rect.height() - (line + 6)) < 0.05);
        QVERIFY(std::abs(rows[1].rect.height() - (3 * line + 6)) < 0.05);
        QVERIFY(std::abs(rows[2].rect.height() - (line + 6)) < 0.05);
        QVERIFY(std::abs(rows[1].rect.top() - rows[0].rect.bottom()) < 0.01 && std::abs(rows[2].rect.top() - rows[1].rect.bottom()) < 0.01);
        QCOMPARE(rows[0].rect.width(), fd.section(0).columnWidth());
        // The text before it ends 6 points above it (its space after), and the text after it begins at its foot.
        const auto lines = pg.lines(0);
        const auto firstRow = std::find_if(lines.begin(), lines.end(), [&](const auto &l) { return l.pos == rows[0].pos; }) - lines.begin();
        QVERIFY(firstRow > 0);
        QVERIFY(std::abs(lines[firstRow - 1].rect.bottom() + 6 - rows[0].rect.top()) < 0.05);
        QCOMPARE(lines.size() - 1, firstRow + 3);
        QVERIFY(std::abs(lines.last().rect.top() - rows[2].rect.bottom()) < 0.05);
        QCOMPARE(lines.last().pos, fd.story()->characterCount() - 1 - QStringLiteral("after the table").size());
    }

    // The lines of a table take room in its rows, as a word processor sets them: a row is as tall as its cells (with their margins) and the line in its top edge (the table's
    // top for the first row, the one between it and the row above for the others), and the last row has the table's foot line too.
    void theLinesOfATableTakeRoomInItsRows()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.setTable = [](TableData &t) {
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = 1.5;
            t.top = t.bottom = t.left = t.right = t.insideH = t.insideV = line;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 3);
        // (The margins are 3 above and below; a line is 1.5 points wide.)
        QVERIFY2(std::abs(rows[0].rect.height() - (line + 6 + 1.5)) < 0.05, qPrintable(QString::number(rows[0].rect.height())));
        QVERIFY2(std::abs(rows[1].rect.height() - (line + 6 + 1.5)) < 0.05, qPrintable(QString::number(rows[1].rect.height())));
        QVERIFY2(std::abs(rows[2].rect.height() - (line + 6 + 1.5 + 1.5)) < 0.05, qPrintable(QString::number(rows[2].rect.height())));
        const auto page = pg.materialize(0);
        QVERIFY(page && page->tables.size() == 1);
        QVERIFY(std::abs(page->tables[0]->height - (3 * (line + 6 + 1.5) + 1.5)) < 0.1);
    }

    // A line is where the row that it is the top of begins: drawn inside that row, from its top edge down; the foot line of the table, up from its foot.
    void aLineOfATableIsDrawnInsideTheRowItIsTheTopOf()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 8, {}, 5, 8);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.setTable = [](TableData &t) {
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = 3;
            line.color = QColor(255, 0, 0);
            t.top = t.bottom = t.insideH = line;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page && page->tables.size() == 1);
        const TableFragment &fragment = *page->tables[0];
        QImage image(int(fd.section(0).pageSize().width()) * 4, int(fd.section(0).pageSize().height()) * 4, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        {
            QPainter painter(&image);
            painter.scale(4, 4);
            fragment.paint(&painter, TablePaint());
        }
        const auto red = [&](double y) { const QColor c = image.pixelColor(int((fragment.origin.x() + 140) * 4), int(y * 4)); return c.red() > 200 && c.green() < 60; };
        const double top = fragment.origin.y();
        const double second = top + fragment.heightOf(0), third = second + fragment.heightOf(1), foot = top + fragment.height;
        for (double at : {top, second, third}) {
            QVERIFY2(red(at + 0.5) && red(at + 2.5), qPrintable(QStringLiteral("no line down from %1").arg(at)));
            QVERIFY2(!red(at - 0.5) && !red(at + 3.5), qPrintable(QStringLiteral("a line is not inside the row from %1").arg(at)));
        }
        QVERIFY(red(foot - 0.5) && red(foot - 2.5));
        QVERIFY(!red(foot - 3.5) && !red(foot + 0.5));
    }

    // Of the two cells at a line, the one that says nothing has the table's line, and the heavier wins: a cell that says none against one that says nothing leaves the table's
    // line; both saying none take it out.
    void aCellThatSaysNoneAtALineDoesNotTakeItAwayFromACellThatSaysNothing()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        TestTable spec;
        spec.rows = 4;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.setTable = [](TableData &t) {
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = 2;
            t.insideH = line;
        };
        spec.setCell = [](int r, int, CellData &cell) {
            if (r == 0) cell.bottom = TableLine();   // says none; the cells of row 1 say nothing
            if (r == 1) cell.bottom = TableLine();   // says none; so do the cells of row 2 at their tops
            if (r == 2) cell.top = TableLine();
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 4);
        QVERIFY2(std::abs(rows[0].rect.height() - (line + 6)) < 0.05, qPrintable(QString::number(rows[0].rect.height())));
        QVERIFY2(std::abs(rows[1].rect.height() - (line + 6 + 2)) < 0.05, qPrintable(QString::number(rows[1].rect.height())));
        QVERIFY2(std::abs(rows[2].rect.height() - (line + 6)) < 0.05, qPrintable(QString::number(rows[2].rect.height())));
        QVERIFY2(std::abs(rows[3].rect.height() - (line + 6 + 2)) < 0.05, qPrintable(QString::number(rows[3].rect.height())));
    }

    // The line under cells joined down is the last of them: the cell that continues the one above has the bottom of the whole.
    void theLineUnderCellsJoinedDownIsThatOfTheCellThatContinuesThem()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.setCell = [](int r, int c, CellData &cell) {
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = 2;
            if (c != 0) return;
            if (r == 0) {
                cell.vmerge = CellData::VMerge::Restart;
                cell.bottom = TableLine();
            } else if (r == 1) {
                cell.vmerge = CellData::VMerge::Continue;
                cell.bottom = line;
            }
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 3);
        QVERIFY2(std::abs(rows[2].rect.height() - (line + 6 + 2)) < 0.05, qPrintable(QString::number(rows[2].rect.height())));
    }

    // The line at the top of a cell that continues the one above takes room in its row, though the cells are one.
    void aLineAtTheTopOfACellThatContinuesTheOneAboveTakesRoomInItsRow()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        TestTable spec;
        spec.rows = 2;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.setCell = [](int r, int c, CellData &cell) {
            if (c != 0) return;
            cell.vmerge = r == 0 ? CellData::VMerge::Restart : CellData::VMerge::Continue;
            if (r == 1) {
                TableLine line;
                line.style = TableLine::Style::Double;
                line.width = 1.5;
                cell.top = line;
            }
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 2);
        QVERIFY2(std::abs(rows[1].rect.height() - (line + 6 + 4.5)) < 0.05, qPrintable(QString::number(rows[1].rect.height())));
    }

    // A table that goes on after a page break has the table's outer lines there, each in room of its own: the top line at the first row of the next page, and the foot line at
    // the last row of the page. The text of those rows is clear of them (with no margins in the cells, a line that took no room would be painted over it).
    void aTableThatGoesOnAfterAPageBreakHasItsOuterLinesAtTheBreak()
    {
        FlowDocument fd;
        TestTable spec;
        spec.rows = 90;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.margins = QMarginsF(4, 0, 4, 0);
        spec.setTable = [](TableData &t) {
            TableLine outer, inner;
            outer.style = inner.style = TableLine::Style::Single;
            outer.width = 6;
            outer.color = QColor(255, 0, 0);
            inner.width = 0.5;
            inner.color = QColor(0, 0, 255);
            t.top = t.bottom = outer;
            t.insideH = inner;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        const BreakPages b = breakPagesOf(fd, pg);
        QVERIFY(b.first && b.next && !b.first->tables.isEmpty() && !b.next->tables.isEmpty());
        const TableFragment &a = *b.first->tables[0], &n = *b.next->tables[0];
        // The lines are in the room the fragments have, and the rows' text is above and below them.
        QVERIFY2(b.firstBottom <= a.height - 6 + 0.05, qPrintable(QStringLiteral("%1 of %2").arg(b.firstBottom).arg(a.height)));
        QVERIFY2(b.nextTop >= 6 - 0.05, qPrintable(QString::number(b.nextTop)));
        QVERIFY(isRed(b.firstImage, a, 140, a.height - 1) && isRed(b.firstImage, a, 140, a.height - 5));
        QCOMPARE(darkIn(b.firstImage, a, a.height - 5.5, a.height - 0.5), 0);
        QVERIFY(isRed(b.nextImage, n, 140, 1) && isRed(b.nextImage, n, 140, 5));
        QCOMPARE(darkIn(b.nextImage, n, 0.5, 5.5), 0);
        // (The lines the paginator set the fragments down by are as tall as the fragments.)
        const auto rows = rowLines(pg, fd, 0);
        QVERIFY(std::abs(a.height - (rows.last().rect.bottom() - rows.first().rect.top())) < 0.01);
    }

    // The same with a table whose cells are apart: its lines are a frame round the lot, at the break too, in room of their own.
    void aSpacedTableThatGoesOnAfterAPageBreakHasItsFrameAtTheBreak()
    {
        FlowDocument fd;
        TestTable spec;
        spec.rows = 90;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.margins = QMarginsF(4, 0, 4, 0);
        spec.setTable = [](TableData &t) {
            TableLine outer;
            outer.style = TableLine::Style::Single;
            outer.width = 4;
            outer.color = QColor(255, 0, 0);
            t.top = t.bottom = outer;
            t.spaced = true;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        const BreakPages b = breakPagesOf(fd, pg);
        QVERIFY(b.first && b.next && !b.first->tables.isEmpty() && !b.next->tables.isEmpty());
        const TableFragment &a = *b.first->tables[0], &n = *b.next->tables[0];
        QVERIFY2(b.firstBottom <= a.height - 4 + 0.05, qPrintable(QStringLiteral("%1 of %2").arg(b.firstBottom).arg(a.height)));
        QVERIFY2(b.nextTop >= 4 - 0.05, qPrintable(QString::number(b.nextTop)));
        QVERIFY(isRed(b.firstImage, a, 140, a.height - 1) && isRed(b.firstImage, a, 140, a.height - 3));
        QCOMPARE(darkIn(b.firstImage, a, a.height - 3.5, a.height - 0.5), 0);
        QVERIFY(isRed(b.nextImage, n, 140, 1) && isRed(b.nextImage, n, 140, 3));
        QCOMPARE(darkIn(b.nextImage, n, 0.5, 3.5), 0);
    }

    // The cells of a table with space between them each have the lines they say: all that the rows take room for are painted (the foot of every row's cell, not only the last's).
    void aSpacedTablePaintsTheLineOfEveryCellsFoot()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        TestTable spec;
        spec.rows = 5;
        spec.cols = 1;
        spec.columnWidth = 200;
        spec.setTable = [](TableData &t) { t.spaced = true; };
        spec.setCell = [](int, int, CellData &cell) {
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = 3;
            line.color = QColor(255, 0, 0);
            cell.bottom = line;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page && page->tables.size() == 1);
        const TableFragment &f = *page->tables[0];
        const double line = lineHeightOf(fd);
        QImage image(int(fd.section(0).pageSize().width()) * 4, int(fd.section(0).pageSize().height()) * 4, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        {
            QPainter painter(&image);
            painter.scale(4, 4);
            f.paint(&painter, TablePaint());
        }
        double bottom = 0;
        for (int r = 0; r < 5; ++r) {
            QVERIFY2(std::abs(f.heightOf(r) - (line + 6 + 3)) < 0.05, qPrintable(QString::number(f.heightOf(r))));
            bottom += f.heightOf(r);
            QVERIFY2(isRed(image, f, 100, bottom - 1.5) && isRed(image, f, 100, bottom - 0.5) && !isRed(image, f, 100, bottom - 3.5), qPrintable(QStringLiteral("row %1").arg(r)));
        }
    }

    // A cell across columns has the line between it and each cell above it: the heaviest of them is the room it takes, and each part of the cell has its own.
    void aCellAcrossColumnsHasTheLineOfEveryCellAboveIt()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        QTextCursor c(fd.story());
        c.movePosition(QTextCursor::End);
        QTextCharFormat body;
        body.setFontPointSize(11);
        auto table = std::make_shared<TableData>();
        table->columns << 100 << 100;
        table->margins = QMarginsF(4, 3, 4, 3);
        auto paragraph = [&](int key, const QString &text) {
            QTextBlockFormat bf;
            bf.setProperty(fp::Cell, key);
            c.insertBlock(bf, body);
            c.insertText(text, body);
        };
        auto lineOf = [](double width, QColor color) {
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = width;
            line.color = color;
            return line;
        };
        auto first = std::make_shared<RowData>(), second = std::make_shared<RowData>();
        for (int col = 0; col < 2; ++col) {
            CellRecord rec;
            rec.table = table;
            rec.row = first;
            rec.cell.bottom = col == 0 ? lineOf(1, QColor(0, 0, 0)) : lineOf(5, QColor(0, 0, 255));
            paragraph(fd.tables.add(rec), QStringLiteral("a%1").arg(col));
        }
        CellRecord wide;
        wide.table = table;
        wide.row = second;
        wide.cell.span = 2;
        paragraph(fd.tables.add(wide), QStringLiteral("wide"));
        Paginator pg(&fd);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 2);
        QVERIFY2(std::abs(rows[1].rect.height() - (line + 6 + 5)) < 0.05, qPrintable(QString::number(rows[1].rect.height())));
        const auto page = pg.materialize(0);
        QVERIFY(page && page->tables.size() == 1);
        const TableFragment &f = *page->tables[0];
        QImage image(int(fd.section(0).pageSize().width()) * 4, int(fd.section(0).pageSize().height()) * 4, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        {
            QPainter painter(&image);
            painter.scale(4, 4);
            f.paint(&painter, TablePaint());
        }
        const double top = f.heightOf(0);
        auto at = [&](double x, double dy) { return image.pixelColor(int((f.origin.x() + x) * 4), int((f.origin.y() + top + dy) * 4)); };
        // Over the first column the line is the black one of a point, over the second the blue one of five.
        QVERIFY(at(90, 0.25).red() < 100 && at(90, 0.25).blue() < 100);
        QVERIFY(qGray(at(90, 3).rgb()) > 240);
        QVERIFY(at(190, 2.5).blue() > 200 && at(190, 2.5).red() < 100);
        QVERIFY(at(190, 4.5).blue() > 200 && at(190, 4.5).red() < 100);
    }

    // Of two lines at a cell edge the one of the greater weight is the line, a double line weighing twice its width: a double line of a point is lighter than a single one of two
    // and a half, though it takes more room.
    void ofTwoLinesAtAnEdgeTheOneOfTheGreaterWeightIsTheLine()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        TestTable spec;
        spec.rows = 2;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.setCell = [](int r, int, CellData &cell) {
            TableLine line;
            if (r == 0) {
                line.style = TableLine::Style::Double;
                line.width = 1;
                cell.bottom = line;
            } else {
                line.style = TableLine::Style::Single;
                line.width = 2.5;
                cell.top = line;
            }
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 2);
        QVERIFY2(std::abs(rows[1].rect.height() - (line + 6 + 2.5)) < 0.05, qPrintable(QString::number(rows[1].rect.height())));
    }

    // A row set to a height of at least so much has that for the room inside its cells' margins, and its lines (the margins and the lines come in addition); one set to exactly
    // so much is that tall, margins, lines, and all.
    void aRowOfASetHeightHasItsMarginsAndLinesBesidesWhenAtLeastAndInsideItWhenExact()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        TestTable spec;
        spec.rows = 4;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.setTable = [](TableData &t) {
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = 2;
            t.top = t.bottom = t.insideH = line;
        };
        spec.setRow = [](int r, RowData &row) {
            row.height = r == 3 ? 5 : 40;
            row.exact = r == 1;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 4);
        // (The margins are 3 above and below; a line is 2 points wide.)
        QVERIFY2(std::abs(rows[0].rect.height() - (40 + 6 + 2)) < 0.05, qPrintable(QString::number(rows[0].rect.height())));
        QVERIFY2(std::abs(rows[1].rect.height() - 40) < 0.05, qPrintable(QString::number(rows[1].rect.height())));
        QVERIFY2(std::abs(rows[2].rect.height() - (40 + 6 + 2)) < 0.05, qPrintable(QString::number(rows[2].rect.height())));
        // (The last: taller than set, so as its cells and lines need.)
        QVERIFY2(std::abs(rows[3].rect.height() - (line + 6 + 2 + 2)) < 0.05, qPrintable(QString::number(rows[3].rect.height())));
    }

    // A double line takes the room of its two lines and the space between them.
    void aDoubleLineTakesRoomForBothLinesAndTheSpace()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 3, {}, 4, 6);
        TestTable spec;
        spec.rows = 2;
        spec.cols = 2;
        spec.columnWidth = 150;
        spec.setTable = [](TableData &t) {
            TableLine line;
            line.style = TableLine::Style::Double;
            line.width = 1;
            t.insideH = line;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const double line = lineHeightOf(fd);
        const auto rows = rowLines(pg, fd, 0);
        QCOMPARE(rows.size(), 2);
        QVERIFY2(std::abs(rows[0].rect.height() - (line + 6)) < 0.05, qPrintable(QString::number(rows[0].rect.height())));
        QVERIFY2(std::abs(rows[1].rect.height() - (line + 6 + 3)) < 0.05, qPrintable(QString::number(rows[1].rect.height())));
    }

    // The last paragraph of a cell, after a table in it, is the cell's end mark, and takes no room when nothing is in it (what it asks of space above does not either); a
    // paragraph before it, or one with words in it, does. A caret can be put in it, and has the height of a line there.
    void theEmptyParagraphAfterATableAtTheEndOfACellTakesNoRoom()
    {
        double heights[4] = {0, 0, 0, 0};
        for (int variant = 0; variant < 4; ++variant) {
            FlowDocument fd;
            fillStory(fd.story(), 2, 3, {}, 4, 6);
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            QTextCharFormat body;
            body.setFontPointSize(11);
            auto outer = std::make_shared<TableData>(), inner = std::make_shared<TableData>();
            outer->columns << 300;
            inner->columns << 100;
            outer->margins = inner->margins = QMarginsF(4, 3, 4, 3);
            auto outerRow = std::make_shared<RowData>();
            CellRecord o;
            o.table = outer;
            o.row = outerRow;
            const int outerKey = fd.tables.add(o);
            auto paragraph = [&](int key, const QString &text, double above = 0) {
                QTextBlockFormat bf;
                bf.setProperty(fp::Cell, key);
                bf.setTopMargin(above);
                c.insertBlock(bf, body);
                c.insertText(text, body);
            };
            paragraph(outerKey, QStringLiteral("before"));
            for (int r = 0; r < 2; ++r) {
                CellRecord i;
                i.table = inner;
                i.row = std::make_shared<RowData>();
                i.parent = outerKey;
                paragraph(fd.tables.add(i), QStringLiteral("inner %1").arg(r));
            }
            switch (variant) {
            case 0: paragraph(outerKey, QString()); break;
            case 1: paragraph(outerKey, QStringLiteral("after")); break;
            case 2: paragraph(outerKey, QString()); paragraph(outerKey, QString()); break;   // (the first of them is a paragraph like any)
            default: paragraph(outerKey, QString(), 30); break;
            }
            Paginator pg(&fd);
            const auto rows = rowLines(pg, fd, 0);
            QCOMPARE(rows.size(), 1);
            heights[variant] = rows[0].rect.height();
            if (variant == 0) {
                const auto page = pg.materialize(0);
                QVERIFY(page && !page->tables.isEmpty());
                QLineF caret;
                QVERIFY(page->tables[0]->caretLine(fd.story()->lastBlock().position(), &caret));
                QVERIFY2(caret.length() > 8, qPrintable(QString::number(caret.length())));
            }
        }
        FlowDocument fdForLine;
        const double line = lineHeightOf(fdForLine);
        // (The outer cell: its margins, a line, and the two rows of the table in it, each a line and its margins.)
        const double base = 6 + line + 2 * (line + 6);
        QVERIFY2(std::abs(heights[0] - base) < 0.1, qPrintable(QString::number(heights[0])));
        QVERIFY2(std::abs(heights[1] - (base + line)) < 0.1, qPrintable(QString::number(heights[1])));
        QVERIFY2(std::abs(heights[2] - (base + line)) < 0.1, qPrintable(QString::number(heights[2])));
        QVERIFY2(std::abs(heights[3] - base) < 0.1, qPrintable(QString::number(heights[3])));
    }

    // A table that is taller than a page goes on onto the next, between rows, each row once and whole; the pages start at a row's first paragraph.
    void aTableBreaksBetweenRowsAcrossPages()
    {
        FlowDocument fd;
        fillStory(fd.story(), 3, 4, {}, 20, 30);
        TestTable spec;
        spec.rows = 90;
        spec.cols = 3;
        spec.columnWidth = 120;
        spec.text = [](int r, int c) { return c == 2 && r % 7 == 3 ? QStringLiteral("long row %1 with enough words to take a few lines in a narrow cell of the table").arg(r) : QStringLiteral("r%1c%2").arg(r).arg(c); };
        const TestTable table = addTable(fd, spec);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 3);
        QVector<int> seen;
        const double room = fd.section(0).content().height();
        for (int k = 0; k < pages; ++k) {
            const auto rows = rowLines(pg, fd, k);
            double bottom = 0;
            for (const auto &r : rows) {
                seen << r.pos;
                bottom = std::max(bottom, r.rect.bottom());
            }
            QVERIFY2(bottom <= room + 0.1, qPrintable(QStringLiteral("page %1 holds rows down to %2 of %3").arg(k).arg(bottom).arg(room)));
            // A page that goes on from the one before starts with a row.
            if (k > 0 && !rows.isEmpty() && pg.lines(k).first().pos == rows.first().pos) QVERIFY(table.rowStarts.contains(pg.page(k).start.pos));
        }
        QCOMPARE(seen, table.rowStarts);
        // Pages are as full as rows allow: no page has room left for the first row of the next.
        for (int k = 0; k + 1 < pages; ++k) {
            const auto here = rowLines(pg, fd, k), next = rowLines(pg, fd, k + 1);
            if (here.isEmpty() || next.isEmpty() || next.first().pos != pg.lines(k + 1).first().pos) continue;
            QVERIFY2(here.last().rect.bottom() + next.first().rect.height() > room + 0.05, qPrintable(QString::number(k)));
        }
    }

    // The header rows of a table are drawn again at the top of each page it goes on to, and the first row there is that much lower.
    void headerRowsRepeatOnTheNextPages()
    {
        FlowDocument fd;
        TestTable spec;
        spec.rows = 80;
        spec.cols = 2;
        spec.setRow = [](int r, RowData &row) { row.header = r < 2; };
        spec.text = [](int r, int c) { return r < 2 ? QStringLiteral("head %1 %2").arg(r).arg(c) : QStringLiteral("r%1c%2").arg(r).arg(c); };
        const TestTable table = addTable(fd, spec);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 2);
        const double line = lineHeightOf(fd);
        const double headers = 2 * (line + 6);
        const auto first = pg.materialize(0), second = pg.materialize(1);
        QVERIFY(first && second);
        QCOMPARE(first->tables.size(), 1);
        QCOMPARE(second->tables.size(), 1);
        QVERIFY(!first->tables[0]->header);
        QVERIFY(second->tables[0]->header);
        QVERIFY(second->tables[0]->firstRow >= 2);
        // (On the page, the header rows are above its first row: the line it takes is as tall as the two together and the row.)
        const auto rows = rowLines(pg, fd, 1);
        QVERIFY(std::abs(rows.first().rect.height() - (headers + line + 6)) < 0.05);
        QVERIFY(std::abs(second->tables[0]->height - std::accumulate(rows.begin(), rows.end(), 0.0, [](double a, const auto &l) { return a + l.rect.height(); })) < 0.05);
        // Every row is on some page once, the header rows on the first.
        QVector<int> seen;
        for (int k = 0; k < pages; ++k)
            for (const auto &r : rowLines(pg, fd, k)) seen << r.pos;
        QCOMPARE(seen, table.rowStarts);
    }

    // A page break before the first paragraph of a table's first cell starts the table on a page of its own; the same on a later row does nothing.
    void aPageBreakBeforeATableStartsItOnANewPage()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 6, {}, 6, 8);
        TestTable spec;
        spec.rows = 4;
        spec.cols = 2;
        const TestTable table = addTable(fd, spec);
        for (const int row : {0, 2}) {
            for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next()) {
                QTextBlockFormat bf = b.blockFormat();
                if (bf.intProperty(fp::Cell) && b.position() == table.rowStarts[row]) {
                    bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
                    QTextCursor(b).setBlockFormat(bf);
                }
            }
        }
        Paginator pg(&fd);
        // The first row is on the second page, with the rows after it; the break before the third row changes nothing.
        QCOMPARE(pg.pageCount(), 2);
        QCOMPARE(rowLines(pg, fd, 0).size(), 0);
        QCOMPARE(rowLines(pg, fd, 1).size(), 4);
        QCOMPARE(pg.page(1).start.pos, table.rowStarts[0]);
        QVERIFY(pg.page(1).start.hard);
    }

    // A row that says it can't be split, and is taller than a page, is placed at the top of a page by itself and the layout goes on after it; it makes no
    // endless pages.
    void aRowThatCantBeSplitAndIsTallerThanAPageStillLaysOut()
    {
        FlowDocument fd;
        fd.sections[0].paper = QSizeF(300, 300);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        fillStory(fd.story(), 2, 5, {}, 6, 8);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 90;
        spec.setRow = [](int r, RowData &row) { row.cantSplit = r == 1; };
        spec.text = [](int r, int c) {
            QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
            if (r == 1 && c == 0)
                for (int i = 0; i < 90; ++i) t += QStringLiteral("\nline %1").arg(i);
            return t;
        };
        const TestTable table = addTable(fd, spec);
        QTextCursor c(fd.story());
        c.movePosition(QTextCursor::End);
        c.insertBlock(QTextBlockFormat());
        c.insertText(QStringLiteral("after"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 3 && pages < 12);
        QVector<int> seen;
        for (int k = 0; k < pages; ++k)
            for (const auto &r : rowLines(pg, fd, k)) seen << r.pos;
        QCOMPARE(seen, table.rowStarts);
        // The tall row is the first thing on its page, and the row after it is not on that page.
        int tallPage = -1;
        for (int k = 0; k < pages; ++k)
            for (const auto &r : rowLines(pg, fd, k))
                if (r.pos == table.rowStarts[1]) tallPage = k;
        QVERIFY(tallPage >= 0);
        QCOMPARE(pg.lines(tallPage).first().pos, table.rowStarts[1]);
        QVERIFY(rowLines(pg, fd, tallPage).first().rect.height() > fd.section(0).content().height());
        QCOMPARE(rowLines(pg, fd, tallPage).size(), 1);
        QVERIFY(pg.page(pages - 1).start.pos < fd.story()->characterCount());
    }

    // A row that can be split and does not fit the page goes over the break a line at a time: each cell's lines that fit on this page, the rest on the next, to
    // as many pages as it takes, with every line once; and a row taller than a page is no exception. The pages after the first start in the row, in a part of it.
    void aRowThatDoesNotFitIsSplitBetweenTheLinesOfItsCells()
    {
        for (const int tall : {55, 400}) {   // (a few lines too many for the page it begins on, and many pages of them)
            FlowDocument fd;
            fd.sections[0].paper = QSizeF(300, 300);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            fillStory(fd.story(), 2, 5, {}, 6, 8);
            TestTable spec;
            spec.rows = 3;
            spec.cols = 3;
            spec.columnWidth = 70;
            spec.text = [&](int r, int c) {
                QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
                const int lines = r != 1 ? 0 : c == 0 ? tall : c == 1 ? tall / 3 : 2;
                for (int i = 0; i < lines; ++i) t += QStringLiteral("\nr%1c%2 line %3").arg(r).arg(c).arg(i);
                return t;
            };
            const TestTable table = addTable(fd, spec);
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            c.insertBlock(QTextBlockFormat());
            c.insertText(QStringLiteral("zebra after"));
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            QVERIFY2(pages >= 3 && pages < 60, qPrintable(QString::number(pages)));
            // The row is on consecutive pages, as parts; every line of its cells is shown once.
            QVector<int> pagesWithRow;
            std::vector<int> shown(3, 0);
            const double room = fd.section(0).content().height();
            for (int k = 0; k < pages; ++k) {
                const auto rows = rowLines(pg, fd, k);
                for (const auto &r : rows)
                    if (r.pos == table.rowStarts[1]) pagesWithRow << k;
                if (std::none_of(rows.begin(), rows.end(), [&](const auto &r) { return r.pos == table.rowStarts[1]; })) continue;
                const auto page = pg.materialize(k);
                QVERIFY(page && !page->tables.isEmpty());
                for (const auto &fragment : page->tables) {
                    const auto *part = fragment->partOf(1);
                    if (!part && !(fragment->firstRow <= 1 && fragment->endRow > 1)) continue;
                    // (A row not in parts is whole on the page: all its lines are shown.)
                    for (const auto &item : fragment->items) {
                        const auto &cell = fragment->layout->cell(item.cell);
                        if (cell.row != 1) continue;
                        const int at = item.cell - fragment->layout->row(1).cell0;
                        const double offset = part ? part->piece.offsets[at] : 0;
                        const double height = part ? part->piece.height : fragment->layout->rowHeight(1);
                        for (const auto &l : item.content->parts[0].text->lineBriefs())
                            if (l.rect.top() >= offset - 0.01 && l.rect.bottom() - offset <= height - cell.margins.top() - cell.margins.bottom() + 0.01) ++shown[size_t(at)];
                    }
                    QVERIFY2(fragment->height <= room + 0.1, qPrintable(QStringLiteral("page %1: %2 of %3").arg(k).arg(fragment->height).arg(room)));
                }
            }
            QVERIFY2(pagesWithRow.size() >= 2, qPrintable(QString::number(pagesWithRow.size())));
            for (int i = 1; i < pagesWithRow.size(); ++i) QCOMPARE(pagesWithRow[i], pagesWithRow[i - 1] + 1);
            QCOMPARE(shown[0], tall + 1);
            QCOMPARE(shown[1], tall / 3 + 1);
            QCOMPARE(shown[2], 3);
            // The pages after the first start in the row, in a part of it, and the paragraph after the table is after the last part.
            for (int i = 1; i < pagesWithRow.size(); ++i) {
                QCOMPARE(pg.page(pagesWithRow[i]).start.pos, table.rowStarts[1]);
                QVERIFY(pg.page(pagesWithRow[i]).start.piece != 0);
            }
            QCOMPARE(pg.page(pagesWithRow[0]).start.piece, 0);
            const int zebra = fd.story()->toPlainText().indexOf(QLatin1String("zebra"));
            QVERIFY(pg.pageOfPosition(zebra) >= pagesWithRow.last());
            // (A row after the split one begins below its last part.)
            const auto last = rowLines(pg, fd, pagesWithRow.last());
            QVERIFY(last.size() >= 1);
            QCOMPARE(rowLines(pg, fd, pg.pageOfPosition(table.rowStarts[2])).last().pos, table.rowStarts[2]);
        }
    }

    // A row set to a height that has a cell with its content in the middle or at the foot is not begun where the room is less than that height: it goes to the next page whole.
    // A row with its content at the top is cut as any row is, whatever its height.
    void aRowSetToAHeightWithContentNotAtItsTopIsNotBegunInLessRoom()
    {
        FlowDocument probe;
        const double line = lineHeightOf(probe);
        const double room = probe.section(0).content().height();
        const int filler = int(std::floor((room - 12) / line)) - 3;
        const double left = room - (filler * line + 6) - line;   // (what the page has for the second row, after the story's own line)
        struct Case { double height; CellAlign align; bool whole; };
        for (const Case &k : {Case{0.0, CellAlign::Bottom, false}, Case{10.0, CellAlign::Bottom, false}, Case{left + 20, CellAlign::Bottom, true}, Case{left + 20, CellAlign::Center, true},
                              Case{left + 20, CellAlign::Top, false}}) {
            FlowDocument fd;
            TestTable spec;
            spec.rows = 2;
            spec.cols = 2;
            spec.columnWidth = 150;
            spec.text = [&](int r, int c) {
                QStringList t;
                const int lines = r == 0 ? (c == 0 ? filler : 1) : 8;
                for (int i = 0; i < lines; ++i) t << QStringLiteral("line %1 of %2.%3").arg(i).arg(r).arg(c);
                return t.join(QLatin1Char('\n'));
            };
            spec.setRow = [&](int r, RowData &row) {
                if (r == 1) row.height = k.height;
            };
            spec.setCell = [&](int r, int, CellData &cell) {
                if (r == 1) cell.valign = k.align;
            };
            addTable(fd, spec);
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() >= 2);
            const int piece = pg.page(1).start.piece;
            QVERIFY2((piece == 0) == k.whole, qPrintable(QStringLiteral("height %1, align %2: the next page starts at part %3").arg(k.height).arg(int(k.align)).arg(piece)));
        }
    }

    // A table in a cell follows the same rule: its row, set to a height and with content at its foot, is not begun in less room than that.
    void aRowOfATableInACellSetToAHeightIsNotBegunInLessRoomEither()
    {
        for (const CellAlign align : {CellAlign::Bottom, CellAlign::Top}) {
            FlowDocument fd;
            fd.sections[0].paper = QSizeF(300, 300);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            QTextCharFormat body;
            body.setFontPointSize(11);
            const double line = lineHeightOf(fd);
            const double room = fd.section(0).content().height();
            const int filler = int(std::floor((room - 12 - 3) / line)) - 3;
            auto outer = std::make_shared<TableData>(), inner = std::make_shared<TableData>();
            outer->columns << 220;
            inner->columns << 150;
            outer->margins = inner->margins = QMarginsF(4, 3, 4, 3);
            CellRecord o;
            o.table = outer;
            o.row = std::make_shared<RowData>();
            const int outerKey = fd.tables.add(o);
            auto paragraph = [&](int key, const QString &text) {
                QTextBlockFormat bf;
                bf.setProperty(fp::Cell, key);
                c.insertBlock(bf, body);
                c.insertText(text, body);
            };
            for (int k = 0; k < filler; ++k) paragraph(outerKey, QStringLiteral("fill %1").arg(k));
            auto innerRow = std::make_shared<RowData>();
            innerRow->height = 5 * line;   // (more than the room the page has left for it, less than what its cell holds, so that it can be cut)
            CellRecord i;
            i.table = inner;
            i.row = innerRow;
            i.parent = outerKey;
            i.cell.valign = align;
            const int innerKey = fd.tables.add(i);
            for (int k = 0; k < 8; ++k) paragraph(innerKey, QStringLiteral("inner %1").arg(k));
            paragraph(outerKey, QString());
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() >= 2);
            // The next page goes on in the row, from where the lines of its cell were left: the lines before that are on this page. Not begun, the nested row is where it goes on
            // from (the filler's lines are before it); begun, some of its lines are before.
            const auto next = pg.page(1).start;
            QCOMPARE(next.pos, fd.story()->findBlock(next.pos).position());
            QVERIFY(next.piece != 0 && !next.pieceAt.isEmpty());
            const double rowTop = filler * line;
            if (align == CellAlign::Bottom) QVERIFY2(next.pieceAt[0] <= rowTop + 0.5, qPrintable(QStringLiteral("%1 for a row at %2").arg(next.pieceAt[0]).arg(rowTop)));
            else QVERIFY2(next.pieceAt[0] > rowTop + 3 + line / 2, qPrintable(QStringLiteral("%1 for a row at %2").arg(next.pieceAt[0]).arg(rowTop)));
        }
    }

    // A table in a cell has its cell's margin again where its row goes on over a page break: the part of the row on the next page has the nested cell's top margin above its
    // first line, besides the outer cell's own.
    void aTableInACellHasItsMarginAgainWhereItsRowGoesOn()
    {
        FlowDocument fd;
        fd.sections[0].paper = QSizeF(300, 300);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        QTextCursor c(fd.story());
        c.movePosition(QTextCursor::End);
        QTextCharFormat body;
        body.setFontPointSize(11);
        auto outer = std::make_shared<TableData>(), inner = std::make_shared<TableData>();
        outer->columns << 220;
        inner->columns << 150;
        outer->margins = QMarginsF(4, 3, 4, 3);
        inner->margins = QMarginsF(4, 6, 4, 3);
        CellRecord o;
        o.table = outer;
        o.row = std::make_shared<RowData>();
        const int outerKey = fd.tables.add(o);
        auto paragraph = [&](int key, const QString &text) {
            QTextBlockFormat bf;
            bf.setProperty(fp::Cell, key);
            c.insertBlock(bf, body);
            c.insertText(text, body);
        };
        CellRecord i;
        i.table = inner;
        i.row = std::make_shared<RowData>();
        i.parent = outerKey;
        const int innerKey = fd.tables.add(i);
        for (int k = 0; k < 60; ++k) paragraph(innerKey, QStringLiteral("line %1").arg(k));
        paragraph(outerKey, QString());
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        const auto page = pg.materialize(1);
        QVERIFY(page && !page->tables.isEmpty());
        const TableFragment &f = *page->tables[0];
        QVERIFY(f.partOf(0) != nullptr);
        double first = -1;
        f.forLines([&](const StoryLayout::LineBrief &, const QRectF &rect) {
            if (first < 0) first = rect.top() - f.origin.y();
        });
        // (The outer cell's margin is 3, the table in it has 6 at the top of its cell.)
        QVERIFY2(std::abs(first - 9) < 0.05, qPrintable(QString::number(first)));
    }

    // The part of a row that goes on from the page before shows only its own lines: nothing of the lines that the page before showed is drawn in the margin above them.
    void aPartOfARowDoesNotShowTheLinesOfThePartBeforeItInItsMargin()
    {
        FlowDocument fd;
        fd.sections[0].paper = QSizeF(300, 300);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        TestTable spec;
        spec.rows = 1;
        spec.cols = 1;
        spec.columnWidth = 200;
        spec.margins = QMarginsF(4, 12, 4, 3);
        spec.text = [](int, int) {
            QStringList t;
            for (int i = 0; i < 60; ++i) t << QStringLiteral("MMMMMMMMMMMMMMMM %1").arg(i);
            return t.join(QLatin1Char('\n'));
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        const auto page = pg.materialize(1);
        QVERIFY(page && !page->tables.isEmpty());
        const TableFragment &f = *page->tables[0];
        QVERIFY(f.partOf(0) != nullptr);
        QImage image(int(fd.section(0).pageSize().width()) * 4, int(fd.section(0).pageSize().height()) * 4, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        {
            QPainter painter(&image);
            painter.scale(4, 4);
            f.paint(&painter, TablePaint());
        }
        // (The margin is 12 points above the first line of the part: it is clear, and the line is there.)
        int inMargin = 0, inLines = 0;
        for (int y = int((f.origin.y() + 0.5) * 4); y < int((f.origin.y() + 11) * 4); ++y)
            for (int x = int((f.origin.x() + 4) * 4); x < int((f.origin.x() + 190) * 4); ++x) inMargin += qGray(image.pixel(x, y)) < 200;
        for (int y = int((f.origin.y() + 13) * 4); y < int((f.origin.y() + 24) * 4); ++y)
            for (int x = int((f.origin.x() + 4) * 4); x < int((f.origin.x() + 190) * 4); ++x) inLines += qGray(image.pixel(x, y)) < 200;
        QCOMPARE(inMargin, 0);
        QVERIFY2(inLines > 50, qPrintable(QString::number(inLines)));
    }

    // A row whose cell holds a table goes over the page break inside that table: between its rows, and between the lines of a row of it that may be split. A page
    // starts where the one before left off, so every line of the table in the cell is shown once and no page is taller than its room.
    void aRowWithATableInItIsSplitInsideTheTable()
    {
        for (const int tall : {30, 300}) {   // (a row of the table in the cell that does not fit the page, and one that is many pages)
            FlowDocument fd;
            fd.sections[0].paper = QSizeF(300, 300);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            QTextCursor c(fd.story());
            QTextCharFormat body;
            body.setFontPointSize(11);
            auto outerTable = std::make_shared<TableData>();
            outerTable->columns << 200;
            outerTable->margins = QMarginsF(4, 3, 4, 3);
            auto outerRow = std::make_shared<RowData>();
            CellRecord outer;
            outer.table = outerTable;
            outer.row = outerRow;
            const int outerKey = fd.tables.add(outer);
            auto paragraph = [&](int key, const QString &text) {
                QTextBlockFormat bf;
                bf.setProperty(fp::Cell, key);
                c.insertBlock(bf, body);
                c.insertText(text, body);
            };
            paragraph(outerKey, QStringLiteral("before the table"));
            auto innerTable = std::make_shared<TableData>();
            innerTable->columns << 90 << 90;
            innerTable->margins = QMarginsF(2, 2, 2, 2);
            for (int r = 0; r < 3; ++r) {
                auto row = std::make_shared<RowData>();
                for (int col = 0; col < 2; ++col) {
                    CellRecord rec;
                    rec.table = innerTable;
                    rec.row = row;
                    rec.parent = outerKey;
                    const int key = fd.tables.add(rec);
                    paragraph(key, QStringLiteral("r%1c%2").arg(r).arg(col));
                    const int lines = r == 1 ? (col == 0 ? tall : tall / 2) : 0;
                    for (int i = 0; i < lines; ++i) paragraph(key, QStringLiteral("r%1c%2 line %3").arg(r).arg(col).arg(i));
                }
            }
            paragraph(outerKey, QStringLiteral("after the table"));
            c.insertBlock(QTextBlockFormat());
            c.insertText(QStringLiteral("zebra after"));
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            QVERIFY2(pages >= 2 && pages < 60, qPrintable(QString::number(pages)));
            const double room = fd.section(0).content().height();
            // The lines of the table in the cell that each page shows (a line is shown by the page whose part of the row has it from its top to its bottom).
            std::map<QString, int> shown;
            int partsSeen = 0;
            for (int k = 0; k < pages; ++k) {
                const auto page = pg.materialize(k);
                QVERIFY(page);
                for (const auto &fragment : page->tables) {
                    if (fragment->layout->depth() != 0) continue;
                    QVERIFY2(fragment->height <= room + 0.1, qPrintable(QStringLiteral("page %1: %2 of %3").arg(k).arg(fragment->height).arg(room)));
                    const auto *part = fragment->partOf(0);
                    if (part) ++partsSeen;
                    const double offset = part ? part->piece.offsets[0] : 0;
                    const double height = part ? part->piece.height : fragment->layout->rowHeight(0);
                    const double inner = height - 6;
                    for (const auto &item : fragment->items) {
                        for (const auto &top : item.content->parts) {
                            if (!top.table) continue;
                            const auto &nested = *top.table;
                            double rowTop = 0;
                            for (int r = nested.firstRow; r < nested.endRow; ++r) {
                                const auto &row = nested.layout->row(r);
                                for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
                                    const auto &cell = nested.layout->cell(i);
                                    const auto cellContent = nested.layout->contentOf(cell);
                                    for (const auto &text : cellContent->parts) {
                                        if (!text.text) continue;
                                        for (const auto &l : text.text->lineBriefs()) {
                                            if (l.frame < 0) continue;
                                            const double y = top.y + rowTop + cell.margins.top() + text.y + l.rect.top();
                                            if (y >= offset - 0.01 && y + l.rect.height() - offset <= inner + 0.01) ++shown[QStringLiteral("%1/%2/%3").arg(r).arg(i).arg(int(l.pos))];
                                        }
                                    }
                                }
                                rowTop += nested.heightOf(r);
                            }
                        }
                    }
                }
            }
            QVERIFY2(partsSeen >= 2, qPrintable(QString::number(partsSeen)));
            // Every line of the table in the cell (a paragraph of each cell and those of the tall row) is shown, once.
            const int expected = 6 + tall + tall / 2;
            QCOMPARE(int(shown.size()), expected);
            for (const auto &entry : shown) QVERIFY2(entry.second == 1, qPrintable(entry.first));
            const int zebra = fd.story()->toPlainText().indexOf(QLatin1String("zebra"));
            QVERIFY(pg.pageOfPosition(zebra) >= pages - 1);
        }
    }

    // A row that goes over a page break is drawn to the page's foot, but the lines that are left for the next page are not drawn in the space under the last line
    // that is on this one (a line's top is the part of it that would show).
    void theLinesLeftForTheNextPageAreNotDrawnUnderTheOnesThatAreOnThisOne()
    {
        int gaps = 0;
        for (int extra = 0; extra < 14; ++extra) {
            FlowDocument fd;
            fd.sections[0].paper = QSizeF(300, 300 + extra);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            TestTable spec;
            spec.rows = 1;
            spec.cols = 1;
            spec.columnWidth = 120;
            spec.text = [](int, int) {
                QString t = QStringLiteral("first");
                for (int i = 0; i < 60; ++i) t += QStringLiteral("\nLINE %1").arg(i);
                return t;
            };
            addTable(fd, spec);
            Paginator pg(&fd);
            const auto page = pg.materialize(0);
            QVERIFY(page && page->tables.size() == 1);
            const TableFragment &fragment = *page->tables[0];
            const auto *part = fragment.partOf(0);
            QVERIFY(part && part->piece.next != 0);
            const auto &cell = fragment.layout->cell(fragment.items[0].cell);
            const double inner = part->piece.height - cell.margins.top() - cell.margins.bottom();
            double cut = 0;
            for (const auto &l : fragment.items[0].content->parts[0].text->lineBriefs())
                if (l.frame >= 0 && l.rect.bottom() <= inner + 0.01) cut = std::max(cut, l.rect.bottom());
            if (inner - cut > 3.5) ++gaps;
            const int scale = 4;
            QImage image(int(fd.section(0).pageSize().width()) * scale, int(fd.section(0).pageSize().height()) * scale, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::white);
            {
                QPainter painter(&image);
                painter.scale(scale, scale);
                paintTables(&painter, *page, TablePaint());
            }
            // Under the last line of the part, to the foot of the row: nothing is drawn.
            const int x0 = int((fragment.origin.x() + cell.x) * scale), x1 = int((fragment.origin.x() + cell.x + cell.w) * scale);
            const int y0 = int((fragment.origin.y() + cell.margins.top() + cut + 0.5) * scale), y1 = int((fragment.origin.y() + part->piece.height - 1.5) * scale);
            int ink = 0;
            for (int y = y0; y < y1; ++y)
                for (int x = x0; x < x1; ++x)
                    if (qGray(image.pixel(x, y)) < 200) ++ink;
            QVERIFY2(ink == 0, qPrintable(QStringLiteral("extra %1: %2 dark pixels under the last line").arg(extra).arg(ink)));
            // (And the lines before it are drawn.)
            int above = 0;
            for (int y = int((fragment.origin.y() + cell.margins.top()) * scale); y < y0 - scale; ++y)
                for (int x = x0; x < x1; ++x) above += qGray(image.pixel(x, y)) < 200;
            QVERIFY(above > 100);
        }
        QVERIFY2(gaps >= 3, qPrintable(QString::number(gaps)));
    }

    // A row that is shown in a thousand parts and more: the number of the part is not held in a few bits, and the parts go on to the end of the row, each starting where
    // the one before stopped.
    void aRowOfThousandsOfPartsContinuesToItsEnd()
    {
        FlowDocument fd;
        fd.sections[0].paper = QSizeF(300, 120);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);   // (40 points of room: two lines of a cell a page)
        TestTable spec;
        spec.rows = 1;
        spec.cols = 1;
        spec.columnWidth = 200;
        spec.text = [](int, int) {
            QString t = QStringLiteral("line 0");
            for (int i = 1; i < 3000; ++i) t += QStringLiteral("\nline %1").arg(i);
            return t;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        // (Asked for page by page, so that a paginator that never ends does not hold the test.)
        double offset = -1;
        for (int k = 1; k < 1500; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY2(page, qPrintable(QStringLiteral("page %1 is not made").arg(k)));
            QVERIFY(page->tables.size() == 1);
            const auto *part = page->tables[0]->partOf(0);
            QVERIFY2(part, qPrintable(QStringLiteral("page %1 has no part of the row").arg(k)));
            QVERIFY2(part->piece.offsets[0] > offset, qPrintable(QStringLiteral("page %1 starts the row's part at %2, the page before at %3").arg(k).arg(part->piece.offsets[0]).arg(offset)));
            offset = part->piece.offsets[0];
            if (!part->piece.next) break;
        }
        const int pages = pg.pageCount();
        QVERIFY2(pages > 1100 && pages < 3200, qPrintable(QString::number(pages)));
        const auto last = pg.materialize(pages - 1);
        QVERIFY(last && last->tables.size() == 1);
        QVERIFY(last->tables[0]->partOf(0) && last->tables[0]->partOf(0)->piece.next == 0);
    }

    // A cell that takes more than a hundred thousand points is laid out whole: nothing of its text is cut off at some height.
    void aCellOfThousandsOfParagraphsIsLaidOutWhole()
    {
        for (const int lines : {8000, 20000}) {
            FlowDocument fd;
            fd.sections[0].paper = QSizeF(300, 300);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            TestTable spec;
            spec.rows = 1;
            spec.cols = 1;
            spec.columnWidth = 200;
            spec.text = [&](int, int) {
                QString t = QStringLiteral("line 0");
                for (int i = 1; i < lines; ++i) t += QStringLiteral("\nline %1").arg(i);
                return t;
            };
            addTable(fd, spec);
            const double line = lineHeightOf(fd);
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            const double room = fd.section(0).content().height() - 6;
            QVERIFY2(pages * room >= lines * line * 0.98, qPrintable(QStringLiteral("%1 lines of %2 points in %3 pages of %4").arg(lines).arg(line).arg(pages).arg(room)));
            const auto last = pg.materialize(pages - 1);
            QVERIFY(last && last->tables.size() == 1);
            const auto *part = last->tables[0]->partOf(0);
            QVERIFY(part);
            QVERIFY(part->piece.offsets[0] + room >= lines * line * 0.98);
            QVERIFY(last->tables[0]->items[0].content->height >= lines * line * 0.98);
        }
    }

    // The heights of rows after edits are those a new layout finds: an edit in a table in a cell is one in the cell's row, an edit in the last row of cells joined down is
    // in all of their rows (the cell they are joined into is as tall as its text), and edits made one after the other before the pages are made again all count.
    void editsGiveTheRowsTheHeightsAFreshLayoutHas()
    {
        static const QString words = QStringLiteral(" and many more words that go on for several lines in this narrow cell of the table so it wraps again and again");
        {   // in a table in a cell
            FlowDocument fd;
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            addNestedTable(fd, c, 1, 3, 0, 3, 2, 120);
            c.insertBlock(QTextBlockFormat());
            c.insertText(QStringLiteral("after the table"));
            Paginator pg(&fd);
            pg.pageCount();
            QTextBlock b;
            for (QTextBlock x = fd.story()->begin(); x.isValid(); x = x.next())
                if (x.text() == QLatin1String("L3 r1c1")) b = x;
            QVERIFY(b.isValid());
            QTextCursor e(fd.story());
            e.setPosition(b.position() + b.length() - 1);
            e.insertText(words);
            const QString d = differsFromFresh(fd, pg);
            QVERIFY2(d.isEmpty(), qPrintable(QStringLiteral("nested: ") + d));
        }
        for (const int which : {2, 3, 4}) {   // in a cell joined down (rows 2 to 4 of column 1), or beside it
            FlowDocument fd;
            TestTable spec;
            spec.rows = 8;
            spec.cols = 2;
            spec.columnWidth = 150;
            spec.text = [](int r, int c) {
                QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
                if (r == 2 && c == 1)
                    for (int i = 0; i < 14; ++i) t += QStringLiteral("\nmerged line %1").arg(i);
                return t;
            };
            spec.setCell = [](int r, int c, CellData &cell) {
                if (c == 1 && r == 2) cell.vmerge = CellData::VMerge::Restart;
                if (c == 1 && (r == 3 || r == 4)) cell.vmerge = CellData::VMerge::Continue;
            };
            const TestTable table = addTable(fd, spec);
            QTextCursor t(fd.story());
            t.movePosition(QTextCursor::End);
            t.insertBlock(QTextBlockFormat());
            t.insertText(QStringLiteral("after the table"));
            Paginator pg(&fd);
            pg.pageCount();
            QTextBlock b = fd.story()->findBlock(table.rowStarts[which]);
            QTextCursor e(fd.story());
            e.setPosition(b.position() + b.length() - 1);
            e.insertText(QStringLiteral("x"));
            const QString d = differsFromFresh(fd, pg);
            QVERIFY2(d.isEmpty(), qPrintable(QStringLiteral("joined down, a letter in row %1: ").arg(which) + d));
        }
        {   // two edits, and the pages made again after both
            FlowDocument fd;
            TestTable spec;
            spec.rows = 120;
            spec.cols = 2;
            spec.columnWidth = 120;
            const TestTable table = addTable(fd, spec);
            QTextCursor t(fd.story());
            t.movePosition(QTextCursor::End);
            t.insertBlock(QTextBlockFormat());
            t.insertText(QStringLiteral("after the table"));
            Paginator pg(&fd);
            pg.pageCount();
            for (const int row : {10, 60}) {
                QTextBlock b = fd.story()->findBlock(table.rowStarts[row]);
                QTextCursor e(fd.story());
                e.setPosition(b.position() + b.length() - 1);
                e.insertText(words);
            }
            const QString d = differsFromFresh(fd, pg);
            QVERIFY2(d.isEmpty(), qPrintable(QStringLiteral("two edits: ") + d));
        }
    }

    // Edits in a row that goes over many page breaks: the pages that start in the row follow it, as they are in a new paginator (the row is the one it was laid out as
    // from the first page it is on, and fewer or more pages hold it).
    void editingARowThatIsInPartsGivesThePagesAFreshLayoutHas()
    {
        for (int variant = 0; variant < 5; ++variant) {
            FlowDocument fd;
            fd.sections[0].paper = QSizeF(300, 300);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            spec.columnWidth = 100;
            spec.text = [](int r, int c) {
                QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
                if (r == 1)
                    for (int i = 0; i < (c == 0 ? 300 : 120); ++i) t += QStringLiteral("\nr%1c%2 ln %3").arg(r).arg(c).arg(i);
                return t;
            };
            const TestTable table = addTable(fd, spec);
            QTextCursor t(fd.story());
            t.movePosition(QTextCursor::End);
            t.insertBlock(QTextBlockFormat());
            t.insertText(QStringLiteral("after the table"));
            Paginator pg(&fd);
            const int before = pg.pageCount();
            QVERIFY(before > 10);
            const QTextBlock first = fd.story()->findBlock(table.rowStarts[1]);
            QTextCursor e(fd.story());
            QString what;
            if (variant == 0) {   // 250 of the 300 lines of the first cell taken out, near its start
                QTextBlock a = first.next(), z = a;
                for (int i = 0; i < 250; ++i) z = z.next();
                e.setPosition(a.position());
                e.setPosition(z.position(), QTextCursor::KeepAnchor);
                e.removeSelectedText();
                what = QStringLiteral("250 lines taken out");
            } else if (variant == 1) {   // a hundred lines put in
                e.setPosition(first.position() + first.length() - 1);
                for (int i = 0; i < 100; ++i) {
                    e.insertBlock(first.blockFormat());
                    e.insertText(QStringLiteral("added line %1").arg(i));
                }
                what = QStringLiteral("100 lines put in");
            } else if (variant == 2) {   // the lines of the second cell (a shorter one) taken out
                QTextBlock c1 = first;
                while (c1.blockFormat().intProperty(fp::Cell) == first.blockFormat().intProperty(fp::Cell)) c1 = c1.next();
                QTextBlock a = c1.next(), z = a;
                for (int i = 0; i < 100; ++i) z = z.next();
                e.setPosition(a.position());
                e.setPosition(z.position(), QTextCursor::KeepAnchor);
                e.removeSelectedText();
                what = QStringLiteral("100 lines of the other cell taken out");
            } else if (variant == 3) {   // a letter in the row's first line
                e.setPosition(first.position() + 1);
                e.insertText(QStringLiteral("x"));
                what = QStringLiteral("a letter");
            } else {   // a letter in a line far down the row
                QTextBlock b = first;
                for (int i = 0; i < 200; ++i) b = b.next();
                e.setPosition(b.position() + 1);
                e.insertText(QStringLiteral("x"));
                what = QStringLiteral("a letter far down the row");
            }
            const QString d = differsFromFresh(fd, pg);
            QVERIFY2(d.isEmpty(), qPrintable(what + ": " + d));
        }
    }

    // The page that shows a position in a cell of a row that is in parts is the one that has the part with the position's line, found without looking at the pages before it.
    void thePageOfAPositionInARowInPartsIsTheOneThatShowsItsLine()
    {
        FlowDocument fd;
        fd.sections[0].paper = QSizeF(300, 300);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 100;
        spec.text = [](int r, int c) {
            QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
            if (r == 1)
                for (int i = 0; i < (c == 0 ? 300 : 120); ++i) t += QStringLiteral("\nr%1c%2 ln %3").arg(r).arg(c).arg(i);
            return t;
        };
        const TestTable table = addTable(fd, spec);
        QTextCursor t(fd.story());
        t.movePosition(QTextCursor::End);
        t.insertBlock(QTextBlockFormat());
        t.insertText(QStringLiteral("after the table"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages > 12);
        // The pages that show a line: those whose part of the row goes from above the line to below it, in the cell the line is in.
        auto shownOn = [&](int position) {
            QVector<int> on;
            for (int k = 0; k < pages; ++k) {
                const auto page = pg.materialize(k);
                if (!page) continue;
                for (const auto &fragment : page->tables) {
                    const int cell = fragment->layout->cellOfPosition(position);
                    if (cell < 0) continue;
                    const TableLayout::Cell &c = fragment->layout->cell(cell);
                    const auto *part = fragment->partOf(c.row);
                    if (!part) continue;
                    const double y = fragment->layout->lineTopIn(cell, position);
                    const int at = cell - fragment->layout->row(c.row).cell0;
                    if (y >= part->piece.offsets[at] - 0.01 && y < part->piece.ends[at]) on << k;
                }
            }
            return on;
        };
        QTextBlock b = fd.story()->findBlock(table.rowStarts[1]);
        int index = 0;
        for (; b.isValid() && index < 330; b = b.next(), ++index) {
            if (index % 29 != 3) continue;
            const int position = b.position() + 1;
            const QVector<int> on = shownOn(position);
            const int direct = pg.pageOfTablePosition(position);
            QVERIFY2(on.size() == 1 && on[0] == direct, qPrintable(QStringLiteral("paragraph %1 of the row: shown on %2 page(s) (first %3), found on %4").arg(index).arg(on.size()).arg(on.isEmpty() ? -1 : on[0]).arg(direct)));
        }
        QCOMPARE(pg.pageOfTablePosition(0), -1);   // (the paragraph before the table is not in one)
    }

    // A header row that is more than a third of a page is not set at the top of the pages after the first (a word processor leaves out what leaves too little room for the
    // rest), and a short one is: the pages that have the first are each as tall as the room they have, and the rows go on from page to page.
    void aHeaderThatTakesMoreThanAThirdOfAPageIsNotRepeated()
    {
        for (const int headerLines : {2, 12}) {
            FlowDocument fd;
            fd.sections[0].paper = QSizeF(300, 300);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            TestTable spec;
            spec.rows = 60;
            spec.cols = 2;
            spec.columnWidth = 100;
            spec.setRow = [](int r, RowData &row) { row.header = r == 0; };
            spec.text = [&](int r, int c) {
                QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
                if (r == 0) for (int i = 1; i < headerLines; ++i) t += QStringLiteral("\nheader line %1").arg(i);
                return t;
            };
            addTable(fd, spec);
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            QVERIFY(pages > 3);
            const double line = lineHeightOf(fd), room = fd.section(0).content().height();
            const bool repeats = headerLines * line + 6 <= room / 3;
            for (int k = 1; k < pages - 1; ++k) {
                const auto page = pg.materialize(k);
                QVERIFY(page && page->tables.size() == 1);
                QCOMPARE(page->tables[0]->header, repeats);
                QVERIFY2(page->tables[0]->height <= room + 0.1, qPrintable(QString::number(page->tables[0]->height)));
            }
            QCOMPARE(pg.materialize(0)->tables[0]->header, false);
        }
    }

    // A cell of eight thousand paragraphs is laid out once for the pages that show it: asking for the pages one after the other does not lay it out again for each.
    void aCellOfThousandsOfParagraphsIsNotLaidOutAgainForEachPage()
    {
        FlowDocument fd;
        fd.sections[0].paper = QSizeF(300, 300);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        TestTable spec;
        spec.rows = 1;
        spec.cols = 2;
        spec.columnWidth = 100;
        spec.text = [](int, int c) {
            QString t = QStringLiteral("c%1").arg(c);
            if (c == 0) for (int i = 1; i < 8000; ++i) t += QStringLiteral("\nline %1").arg(i);
            return t;
        };
        addTable(fd, spec);
        std::unique_ptr<Paginator> pg;
        const auto fresh = [&] {
            pg.reset();
            pg = std::make_unique<Paginator>(&fd);
            pg->pageCount();
        };
        const qint64 first = bestNanoseconds([&] { pg->materialize(0); }, 3, fresh) / 1000000;
        const qint64 forty = bestNanoseconds([&] { for (int k = 1; k <= 40; ++k) QVERIFY(pg->materialize(k)); }, 3, [&] {
            fresh();
            pg->materialize(0);
        }) / 1000000;
        // (A cell of this size takes a good part of a second to lay out: forty more pages that each laid it out would take forty times that.)
        QVERIFY2(forty < 8 * std::max<qint64>(first, 50) + 500, qPrintable(QStringLiteral("the first page %1 ms, forty more %2 ms").arg(first).arg(forty)));
    }

    // A cell is laid out once to find the heights of the rows (in a pass over all of them) and once more for a page that shows it, and a header's cells, which every page of the
    // table shows, are laid out once for all of them.
    void eachCellIsLaidOutOnceToMeasureAndOnceMoreForAPageThatShowsIt()
    {
        FlowDocument fd;
        TestTable spec;
        spec.rows = 1500;
        spec.cols = 3;
        spec.columnWidth = 120;
        spec.setRow = [](int r, RowData &row) { row.header = r == 0; };
        addTable(fd, spec);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages > 20);
        const qint64 cells = 1500 * 3;
        QVERIFY2(pg.tableCellsLaidOut() <= cells + 10, qPrintable(QString::number(pg.tableCellsLaidOut())));
        const qint64 before = pg.tableCellsLaidOut();
        for (int k = 0; k < 6; ++k) QVERIFY(pg.materialize(k));
        for (int k = 0; k < 6; ++k) QVERIFY(pg.materialize(k));   // (the same pages again: the cells are the ones laid out lately)
        const int perPage = int(cells / pages) + 3 + 3;
        QVERIFY2(pg.tableCellsLaidOut() - before <= 6 * perPage, qPrintable(QStringLiteral("%1 cells laid out for six pages of about %2").arg(pg.tableCellsLaidOut() - before).arg(perPage)));
    }

    // ---- tables with what the other waves of the word processor added: notes, headers and footers, sections, line heights

    // A footnote cited before a table, one cited in a cell of it (on a later page), and one cited after it are each set at the foot of the page that cites them, in the room the
    // table's rows leave, numbered as the document counts them (the cell's is the second, the one after the table the third).
    void notesCitedBeforeAndInsideATableLandOnThePageThatCitesThem()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("a"), 8, QStringLiteral("NOTEA"));
        addNote(&notes, QStringLiteral("b"), 8, QStringLiteral("NOTEB"));
        addNote(&notes, QStringLiteral("c"), 8, QStringLiteral("NOTEC"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fd.sections[0].paper = QSizeF(300, 330);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        fillStory(fd.story(), 3, 5, {}, 12, 18);
        addReference(fd.story(), fd.story()->findBlockByNumber(1).position() + 4, QStringLiteral("a"));
        TestTable spec;
        spec.rows = 40;
        spec.cols = 2;
        spec.columnWidth = 100;
        const TestTable table = addTable(fd, spec);
        addReference(fd.story(), table.rowStarts[25] + 1, QStringLiteral("b"));
        {
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            QTextBlockFormat bf;
            bf.setBottomMargin(6);
            c.insertBlock(bf);
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            c.insertText(QStringLiteral("After the table comes this paragraph, which cites a note."), cf);
        }
        addReference(fd.story(), fd.story()->characterCount() - 5, QStringLiteral("c"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 4);
        int onA = -1, onB = -1, onC = -1, rowPage = -1;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            const QRectF body = pg.page(k).content;
            if (boxHolding(*page, QStringLiteral("NOTEA"))) onA = onA < 0 ? k : -2;
            if (boxHolding(*page, QStringLiteral("NOTEB"))) onB = onB < 0 ? k : -2;
            if (boxHolding(*page, QStringLiteral("NOTEC"))) onC = onC < 0 ? k : -2;
            // Whatever is at the foot is inside the text area, and the rows and the text end above the notes.
            double notesTop = body.bottom();
            for (const QString &name : {QStringLiteral("NOTEA"), QStringLiteral("NOTEB"), QStringLiteral("NOTEC")})
                if (const TextItem *box = boxHolding(*page, name)) {
                    notesTop = std::min(notesTop, box->rect.top());
                    QVERIFY2(box->rect.bottom() <= body.bottom() + 0.5, qPrintable(QStringLiteral("page %1: %2 ends at %3 of %4").arg(k).arg(name).arg(box->rect.bottom()).arg(body.bottom())));
                }
            for (const auto &fragment : page->tables) {
                QVERIFY2(fragment->origin.y() + fragment->height <= notesTop + 0.01, qPrintable(QStringLiteral("page %1: the table ends at %2, the notes begin at %3").arg(k).arg(fragment->origin.y() + fragment->height).arg(notesTop)));
                if (rowPage < 0 && fragment->firstRow <= 25 && fragment->endRow > 25) rowPage = k;
            }
        }
        QCOMPARE(onA, 0);
        QVERIFY2(rowPage > 0, "the row with the note is on a page after the first");
        QCOMPARE(onB, rowPage);
        QCOMPARE(onC, pages - 1);
        // The numbers: the cell's note is the second, the one after the table the third.
        {
            const auto page = pg.materialize(rowPage);
            const auto &fragment = page->tables[0];
            int number = 0;
            for (const auto &item : fragment->items)
                for (const auto &part : item.content->parts)
                    for (const NoteRef &n : part.notes) number = n.number;
            QCOMPARE(number, 2);
            const auto last = pg.materialize(pages - 1);
            bool third = false;
            for (const auto &story : last->doc->stories)
                for (QTextBlock b = story->doc->begin(); b.isValid(); b = b.next())
                    for (auto it = b.begin(); !it.atEnd(); ++it) third |= it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("raw:3");
            QVERIFY(third);
        }
        const QString bad = differenceFromFresh(fd, pg, true) + differsFromFresh(fd, pg);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // Odd and even pages with headers and footers of other heights each leave a table the room that is left: its rows, and the parts of rows cut at the foot, stay between
    // the header and the footer of their page (the table does not run to the edge of the page), also after an edit moves the table by one page.
    void headersAndFootersOfOtherHeightsBoundTheRowsOfATable()
    {
        for (const bool tall : {false, true}) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 2, 4);
            Section &s = fd.sections[0];
            s.differentOddEven = true;
            s.header.odd = furnitureStory({QStringLiteral("ODDHEAD")});
            s.header.even = furnitureStory({QStringLiteral("EVEN 1"), QStringLiteral("EVEN 2"), QStringLiteral("EVEN 3"), QStringLiteral("EVEN 4"), QStringLiteral("EVEN 5"), QStringLiteral("EVEN 6")});
            s.footer.odd = furnitureStory({QStringLiteral("ODDFOOT")});
            s.footer.even = furnitureStory({QStringLiteral("EFOOT 1"), QStringLiteral("EFOOT 2"), QStringLiteral("EFOOT 3"), QStringLiteral("EFOOT 4"), QStringLiteral("EFOOT 5")});
            TestTable spec;
            spec.rows = tall ? 14 : 110;
            spec.cols = 2;
            spec.columnWidth = 120;
            if (tall) spec.text = [](int r, int c) {
                QStringList l;
                for (int i = 0; i < (c == 0 ? 30 : 3); ++i) l << QStringLiteral("r%1c%2 line %3").arg(r).arg(c).arg(i);
                return l.join(QLatin1Char('\n'));
            };
            addTable(fd, spec);
            {
                QTextCursor c(fd.story());
                c.movePosition(QTextCursor::End);
                c.insertBlock(QTextBlockFormat());
                c.insertText(QStringLiteral("After the table."));
            }
            Paginator pg(&fd);
            auto check = [&](const char *when) {
                const int pages = pg.pageCount();
                QVERIFY2(pages >= 4, when);
                double top[2] = {0, 0};
                int rows[2] = {0, 0}, counted[2] = {0, 0};
                for (int k = 0; k < pages; ++k) {
                    const auto page = pg.materialize(k);
                    QVERIFY(page);
                    const QRectF body = pg.page(k).content;
                    const bool even = pg.page(k).number % 2 == 0;
                    const TextItem *head = boxHolding(*page, even ? QStringLiteral("EVEN 6") : QStringLiteral("ODDHEAD"));
                    const TextItem *foot = boxHolding(*page, even ? QStringLiteral("EFOOT 5") : QStringLiteral("ODDFOOT"));
                    QVERIFY2(head && foot, qPrintable(QStringLiteral("%1: page %2 has no header or footer of its parity").arg(QString::fromLatin1(when)).arg(k)));
                    QVERIFY2(!boxHolding(*page, even ? QStringLiteral("ODDHEAD") : QStringLiteral("EVEN 1")), when);
                    QVERIFY2(head->rect.bottom() <= body.top() + 0.01 && foot->rect.top() >= body.bottom() - 0.01, qPrintable(QStringLiteral("%1: page %2 body %3 to %4, header ends %5, footer begins %6").arg(QString::fromLatin1(when)).arg(k).arg(body.top()).arg(body.bottom()).arg(head->rect.bottom()).arg(foot->rect.top())));
                    for (const auto &fragment : page->tables) {
                        QVERIFY2(fragment->origin.y() >= body.top() - 0.01 && fragment->origin.y() + fragment->height <= body.bottom() + 0.01,
                                 qPrintable(QStringLiteral("%1: page %2 (%3): the table is %4 to %5 in a body %6 to %7").arg(QString::fromLatin1(when)).arg(k).arg(even ? "even" : "odd").arg(fragment->origin.y()).arg(fragment->origin.y() + fragment->height).arg(body.top()).arg(body.bottom())));
                        if (fragment->firstRow > 0 && fragment->endRow < fragment->layout->rowCount()) { rows[even] += fragment->endRow - fragment->firstRow; ++counted[even]; }
                    }
                    if (k > 0 && k + 1 < pages) top[even] = body.top();
                }
                QVERIFY2(top[1] > top[0] + 20, when);
                if (!tall && counted[0] && counted[1]) QVERIFY2(double(rows[1]) / counted[1] < double(rows[0]) / counted[0], when);
                const QString bad = differsFromFresh(fd, pg);
                QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1: %2").arg(QString::fromLatin1(when), bad)));
            };
            check("as made");
            // Paragraphs added above push the table down by one page, and so every page after it changes its parity.
            QTextCursor c(fd.story());
            c.setPosition(fd.story()->findBlockByNumber(1).position());
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            for (int i = 0; i < 40; ++i) {
                c.insertText(QStringLiteral("An added paragraph that is long enough to take two or three lines of the page when it is set at eleven points in this frame."), cf);
                c.insertBlock();
            }
            check("after paragraphs above");
            c.setPosition(fd.story()->findBlockByNumber(1).position());
            c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, 25);
            c.removeSelectedText();
            check("after paragraphs removed");
        }
    }

    // A continuous section with two columns begins under a table on the page where the table ends, and a table in it is set in its columns, cut at the foot of each.
    void aTwoColumnSectionAfterATableHoldsATableInItsColumns()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        Section &one = fd.sections[0];
        one.differentOddEven = true;
        one.header.odd = furnitureStory({QStringLiteral("ODDHEAD")});
        one.header.even = furnitureStory({QStringLiteral("EVEN 1"), QStringLiteral("EVEN 2"), QStringLiteral("EVEN 3"), QStringLiteral("EVEN 4"), QStringLiteral("EVEN 5")});
        Section two = one;
        two.continuous = true;
        two.columns = 2;
        two.columnGap = 20;
        fd.sections << two;
        fillStory(fd.story(), 2, 4);
        TestTable first;
        first.rows = 5;
        first.cols = 2;
        first.columnWidth = 150;
        addTable(fd, first);
        {
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            QTextBlockFormat bf;
            bf.setProperty(flow::fp::SectionBreakAfter, true);
            c.insertBlock(bf);
            c.insertText(QStringLiteral("END OF ONE"));
        }
        QTextCursor tail(fd.story());
        tail.movePosition(QTextCursor::End);
        for (int i = 0; i < 3; ++i) {
            tail.insertBlock(QTextBlockFormat());
            tail.insertText(QStringLiteral("A paragraph in the columns that takes some lines of the narrow column, so that the table comes below text. "), QTextCharFormat());
        }
        TestTable second;
        second.rows = 200;
        second.cols = 2;
        second.columnWidth = 100;
        second.text = [](int r, int c) { return QStringLiteral("B%1c%2").arg(r).arg(c); };
        addTable(fd, second);
        tail.movePosition(QTextCursor::End);
        tail.insertBlock(QTextBlockFormat());
        tail.insertText(QStringLiteral("Last paragraph."));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 3);
        const Section &sec = fd.sections[1];
        int next = 0;   // the next row of the second table that must be seen
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            const QRectF body = pg.page(k).content;
            double firstTableBottom = -1;
            QVector<const TableFragment *> inColumns;
            for (const auto &fragment : page->tables) {
                QVERIFY(fragment->origin.y() >= body.top() - 0.01 && fragment->origin.y() + fragment->height <= body.bottom() + 0.01);
                if (fragment->layout->rowCount() == 5) firstTableBottom = fragment->origin.y() + fragment->height;
                else inColumns << fragment.get();
            }
            if (k == 0) QVERIFY(firstTableBottom > 0);
            for (const TableFragment *f : inColumns) {
                QCOMPARE(f->firstRow, next);
                next = f->endRow;
                const bool left = std::abs(f->origin.x() - sec.columnRect(0).left()) < 12, right = std::abs(f->origin.x() - sec.columnRect(1).left()) < 12;
                QVERIFY2(left || right, qPrintable(QStringLiteral("page %1: the table is at x %2 (columns at %3 and %4)").arg(k).arg(f->origin.x()).arg(sec.columnRect(0).left()).arg(sec.columnRect(1).left())));
                QVERIFY2(f->rect().right() <= (left ? sec.columnRect(0) : sec.columnRect(1)).right() + 12, "inside its column");
                if (k == 0) QVERIFY2(f->origin.y() >= firstTableBottom - 0.01, "under the first table");
            }
        }
        QCOMPARE(next, 200);
        QVERIFY(pg.page(0).start.section == 0);
        const QString bad = differenceFromFresh(fd, pg, true) + differsFromFresh(fd, pg);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A note cited in a cell of a row that is cut between two pages is set on the page that holds that cell's text, and the next page does not repeat it.
    void aNoteCitedInACutRowIsOnThePageOfItsLine()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("a"), 6, QStringLiteral("NOTEA"));
        addNote(&notes, QStringLiteral("b"), 6, QStringLiteral("NOTEB"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fd.sections[0].paper = QSizeF(300, 330);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        fillStory(fd.story(), 1, 3, {}, 10, 12);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 100;
        spec.text = [](int r, int c) {
            QStringList l;
            for (int i = 0; i < (c == 0 ? 40 : 2); ++i) l << QStringLiteral("r%1c%2 line %3").arg(r).arg(c).arg(i);
            return l.join(QLatin1Char('\n'));
        };
        const TestTable table = addTable(fd, spec);
        // `a` is cited on an early line of row 1's first cell, `b` on a late one.
        addReference(fd.story(), table.rowStarts[1] + 3, QStringLiteral("a"));
        QTextBlock late = fd.story()->findBlock(table.rowStarts[1]);
        for (int i = 0; i < 36; ++i) late = late.next();
        addReference(fd.story(), late.position() + 2, QStringLiteral("b"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 3);
        int onA = -1, onB = -1;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            if (boxHolding(*page, QStringLiteral("NOTEA"))) onA = onA < 0 ? k : -2;
            if (boxHolding(*page, QStringLiteral("NOTEB"))) onB = onB < 0 ? k : -2;
            const QRectF body = pg.page(k).content;
            for (const auto &fragment : page->tables) {
                double notesTop = body.bottom();
                for (const QString &name : {QStringLiteral("NOTEA"), QStringLiteral("NOTEB")})
                    if (const TextItem *box = boxHolding(*page, name)) notesTop = std::min(notesTop, box->rect.top());
                QVERIFY2(fragment->origin.y() + fragment->height <= notesTop + 0.01, qPrintable(QStringLiteral("page %1: table ends %2, notes begin %3").arg(k).arg(fragment->origin.y() + fragment->height).arg(notesTop)));
            }
        }
        // The page that shows a cut row's part with the reference to a note holds the note.
        auto pageOfLine = [&](int position) {
            for (int k = 0; k < pages; ++k) {
                const auto page = pg.materialize(k);   // (kept: the tables are the page's)
                if (!page) continue;
                for (const auto &fragment : page->tables)
                    if (fragment->contains(position)) {
                        QRectF r = fragment->cellRect(position);
                        if (!r.isEmpty()) return k;
                    }
            }
            return -1;
        };
        QVERIFY(onA >= 0 && onB >= 0);
        QCOMPARE(onA, pg.pageOfTablePosition(table.rowStarts[1] + 3));
        QCOMPARE(onB, pg.pageOfTablePosition(late.position() + 2));
        QVERIFY(onB > onA);
        (void)pageOfLine;
        const QString bad = differenceFromFresh(fd, pg, true) + differsFromFresh(fd, pg);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // A note longer than what is left of the page is cut at the foot and goes on at the foot of the next, also where a table is on those pages.
    void aLongNoteIsCarriedAcrossPagesThatHoldATable()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("long"), 400, QStringLiteral("LONGNOTE"));
        addNote(&notes, QStringLiteral("short"), 5, QStringLiteral("SHORTNOTE"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fd.sections[0].paper = QSizeF(300, 330);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        fillStory(fd.story(), 3, 5, {}, 12, 18);
        addReference(fd.story(), fd.story()->findBlockByNumber(2).position() + 3, QStringLiteral("long"));
        TestTable spec;
        spec.rows = 30;
        spec.cols = 2;
        spec.columnWidth = 100;
        addTable(fd, spec);
        addReference(fd.story(), fd.story()->characterCount() - 4, QStringLiteral("short"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 4);
        int withLong = 0, firstLong = -1, lastLong = -1;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            const QRectF body = pg.page(k).content;
            if (boxHolding(*page, QStringLiteral("word"))) {
                ++withLong;
                if (firstLong < 0) firstLong = k;
                lastLong = k;
            }
            for (const auto &fragment : page->tables) {
                double notesTop = body.bottom();
                for (const auto &item : page->doc->pages[0]->items)
                    if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                        if (const Story *st = page->doc->story(t->storyId))
                            if (st->doc->toPlainText().contains(QStringLiteral("word"))) notesTop = std::min(notesTop, t->rect.top());
                QVERIFY2(fragment->origin.y() + fragment->height <= notesTop + 0.01, qPrintable(QStringLiteral("page %1: table ends %2, notes begin %3").arg(k).arg(fragment->origin.y() + fragment->height).arg(notesTop)));
            }
            QVERIFY(pg.page(k).start.carried.size() <= 2);
        }
        QVERIFY2(withLong >= 2 && firstLong == 0, qPrintable(QStringLiteral("the long note is on %1 page(s), from %2 to %3").arg(withLong).arg(firstLong).arg(lastLong)));
        const QString bad = differenceFromFresh(fd, pg, true) + differsFromFresh(fd, pg);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }

    // The paragraph that stands for a table in the story takes no height of its own, set at a word processor's line heights or not: the table is where the text before it ends.
    void theStandInForATableDoesNotShiftItUnderWordLineHeights()
    {
        double at[2] = {0, 0}, gap[2] = {0, 0};
        for (const bool word : {false, true}) {
            FlowDocument fd;
            fd.env.wordLines = word;
            fillStory(fd.story(), 2, 4, [](int, QTextBlockFormat &bf) { bf.setBottomMargin(6); });
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            spec.columnWidth = 100;
            addTable(fd, spec);
            Paginator pg(&fd);
            const auto lines = pg.lines(0);
            double lastText = 0;
            for (const auto &l : lines)
                if (!fd.story()->findBlock(l.block).blockFormat().intProperty(fp::Cell)) lastText = l.rect.bottom();
            const auto page = pg.materialize(0);
            QVERIFY(!page->tables.isEmpty());
            at[word] = page->tables[0]->origin.y();
            gap[word] = at[word] - pg.page(0).content.top() - lastText;
            const auto rows = rowLines(pg, fd, 0);
            QVERIFY(!rows.isEmpty());
            QVERIFY2(std::abs(rows.first().rect.top() + pg.page(0).content.top() - at[word]) < 0.01, qPrintable(QStringLiteral("the first row line is at %1, the table at %2").arg(rows.first().rect.top() + pg.page(0).content.top()).arg(at[word])));
        }
        QVERIFY2(gap[0] >= 0 && gap[0] < 20 && std::abs(gap[0] - gap[1]) < 0.01, qPrintable(QStringLiteral("gaps %1 and %2").arg(gap[0]).arg(gap[1])));
    }

    // An object anchored in a paragraph before a table keeps its place on a page that holds the table; one anchored in a cell goes where the paragraph of its anchor is in the cell,
    // on the page that shows that line (once, though the row is cut between pages).
    void objectsAnchoredInTablesAreSetOnceWhereTheirLinesAre()
    {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.sections[0].paper = QSizeF(300, 330);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        fillStory(fd.story(), 1, 3, {}, 10, 12);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 100;
        spec.text = [](int r, int c) {
            QStringList l;
            for (int i = 0; i < (c == 0 ? 40 : 2); ++i) l << QStringLiteral("r%1c%2 line %3").arg(r).arg(c).arg(i);
            return l.join(QLatin1Char('\n'));
        };
        const TestTable table = addTable(fd, spec);
        auto box = [](double w, double h) {
            auto item = std::make_shared<ShapeItem>();
            item->rect = QRectF(0, 0, w, h);
            return item;
        };
        using Rel = FloatingObject::Rel;
        QTextBlock late = fd.story()->findBlock(table.rowStarts[1]);
        for (int i = 0; i < 33; ++i) late = late.next();
        const int latePos = late.position() + 2;
        addObject(&fd, box(31, 7), latePos, Rel::Page, Rel::Paragraph, 5, 3);                   // in a cell, on a late line of a cut row
        addObject(&fd, box(29, 7), table.rowStarts[2] + 1, Rel::Page, Rel::Paragraph, 5, 3);   // in the first cell of a row that is whole
        addObject(&fd, box(23, 7), fd.story()->findBlockByNumber(0).position() + 2, Rel::Margin, Rel::Paragraph, 0, 0);
        // (Where the anchors are now: each inserted character moved the places after it.)
        int anchorAt[3] = {-1, -1, -1};
        for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const int id = it.fragment().charFormat().intProperty(fp::AnchorId);
                if (id >= 1 && id <= 3) anchorAt[id - 1] = it.fragment().position();
            }
        QVERIFY(anchorAt[0] > 0 && anchorAt[1] > 0 && anchorAt[2] >= 0);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 3);
        int count[3] = {0, 0, 0}, pageOf[3] = {-1, -1, -1};
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            for (const auto &it : page->doc->pages[0]->items)
                if (it->type() == ItemType::Shape) {
                    const int w = int(static_cast<const ShapeItem *>(it.get())->rect.width());
                    const int which = w == 31 ? 0 : w == 29 ? 1 : w == 23 ? 2 : -1;
                    if (which < 0) continue;
                    ++count[which];
                    pageOf[which] = k;
                    if (which == 0 || which == 1) {
                        // At the top of the paragraph that holds the anchor, 3 points down, 5 from the page's edge.
                        const int where = anchorAt[which];
                        QLineF caret;
                        bool found = false;
                        for (const auto &fragment : page->tables) found |= fragment->caretLine(where, &caret);
                        QVERIFY(found);
                        const QRectF r = static_cast<const ShapeItem *>(it.get())->rect;
                        QVERIFY2(std::abs(r.left() - 5) < 0.01 && std::abs(r.top() - (caret.y1() + 3)) < 0.5, qPrintable(QStringLiteral("page %1: the object is at (%2, %3), the line at y %4").arg(k).arg(r.left()).arg(r.top()).arg(caret.y1())));
                    }
                }
        }
        QCOMPARE(count[0], 1);
        QCOMPARE(count[1], 1);
        QCOMPARE(count[2], 1);
        QCOMPARE(pageOf[2], 0);
        QCOMPARE(pageOf[0], pg.pageOfTablePosition(anchorAt[0]));
        QCOMPARE(pageOf[1], pg.pageOfTablePosition(anchorAt[1]));
    }

    // The page that a position in a row cut between pages is on is the one whose part of the row shows its line, also where the furniture takes room from the text area.
    void thePageOfAPositionInARowInPartsIsTheOneThatShowsItsLineWithFurniture()
    {
        for (const bool header : {false, true}) {
            FlowDocument fd;
            fd.sections[0].paper = QSizeF(300, 300);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            if (header) fd.sections[0].header.odd = furnitureStory({QStringLiteral("H1"), QStringLiteral("H2"), QStringLiteral("H3"), QStringLiteral("H4"), QStringLiteral("H5")});
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            spec.columnWidth = 100;
            spec.text = [](int r, int c) {
                QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
                if (r == 1)
                    for (int i = 0; i < (c == 0 ? 300 : 120); ++i) t += QStringLiteral("\nr%1c%2 ln %3").arg(r).arg(c).arg(i);
                return t;
            };
            const TestTable table = addTable(fd, spec);
            QTextCursor t(fd.story());
            t.movePosition(QTextCursor::End);
            t.insertBlock(QTextBlockFormat());
            t.insertText(QStringLiteral("after the table"));
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            auto shownOn = [&](int position) {
                QVector<int> on;
                for (int k = 0; k < pages; ++k) {
                    const auto page = pg.materialize(k);
                    if (!page) continue;
                    for (const auto &fragment : page->tables) {
                        const int cell = fragment->layout->cellOfPosition(position);
                        if (cell < 0) continue;
                        const TableLayout::Cell &c = fragment->layout->cell(cell);
                        const auto *part = fragment->partOf(c.row);
                        if (!part) continue;
                        const double y = fragment->layout->lineTopIn(cell, position);
                        const int at = cell - fragment->layout->row(c.row).cell0;
                        if (y >= part->piece.offsets[at] - 0.01 && y < part->piece.ends[at]) on << k;
                    }
                }
                return on;
            };
            QTextBlock b = fd.story()->findBlock(table.rowStarts[1]);
            int index = 0, checked = 0;
            for (; b.isValid() && index < 330; b = b.next(), ++index) {
                if (index % 7 != 3) continue;
                const int position = b.position() + 1;
                const QVector<int> on = shownOn(position);
                const int direct = pg.pageOfTablePosition(position);
                ++checked;
                QVERIFY2(on.size() == 1 && on[0] == direct, qPrintable(QStringLiteral("%1: paragraph %2 of the row is shown on %3 page(s) (the first %4), found on %5").arg(header ? "with a header" : "bare").arg(index).arg(on.size()).arg(on.isEmpty() ? -1 : on[0]).arg(direct)));
            }
            QVERIFY(checked > 30 && pages > 8);
        }
    }

    // The page's own story, with a stand-in for the rows of a table, is laid out where the paginator put its lines: the stand-in is as tall as the rows are, also where the lines are set
    // at a word processor's heights and the furniture takes room.
    void theStoryOfAPageWithTablesIsLaidOutWhereThePaginatorPutItsLines()
    {
        for (int variant = 0; variant < 3; ++variant) {
            FlowDocument fd;
            fd.env.wordLines = true;
            if (variant >= 1) {
                fd.sections[0].header.odd = furnitureStory({QStringLiteral("H1"), QStringLiteral("H2"), QStringLiteral("H3"), QStringLiteral("H4")});
                fd.sections[0].footer.odd = furnitureStory({QStringLiteral("F1"), QStringLiteral("F2")});
            }
            fillStory(fd.story(), 20, 5, [variant](int i, QTextBlockFormat &bf) { if (variant == 2) { bf.setTopMargin(i % 3 * 3); bf.setBottomMargin(6); } }, 40, 40);
            TestTable spec;
            spec.rows = 12;
            spec.cols = 3;
            spec.columnWidth = 120;
            spec.text = [](int r, int c) {
                QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
                if (r % 4 == 1) for (int i = 0; i < 4 + c * 3; ++i) t += QStringLiteral("\nr%1c%2 ln %3").arg(r).arg(c).arg(i);
                return t;
            };
            addTable(fd, spec);
            QTextCursor t(fd.story());
            t.movePosition(QTextCursor::End);
            for (int i = 0; i < 25; ++i) {
                QTextBlockFormat bf;
                if (variant == 2) { bf.setTopMargin(i % 3 * 3); bf.setBottomMargin(6); }
                t.insertBlock(bf);
                QTextCharFormat cf;
                cf.setFontPointSize(11);
                t.insertText(QStringLiteral("after the table paragraph %1 with some more words to make it wrap onto a second line of the page text area here").arg(i), cf);
            }
            Paginator pg(&fd);
            LayoutCache cache;
            QVERIFY(pg.pageCount() > 3);
            for (int k = 0; k < pg.pageCount(); ++k) {
                const QString d = tableAwareDiffers(fd, pg, k, cache);
                QVERIFY2(d.isEmpty(), qPrintable(QStringLiteral("variant %1: %2").arg(variant).arg(d)));
            }
        }
    }

    // A note cited in text before a table, in text after one (with a table or none before the text on the page), or in a table's page, is seated at the foot of the page, under the
    // rows and the text, whatever comes before it on the page.
    void aNoteBesideATableIsSeatedAtTheFootOfItsPage()
    {
        for (int variant = 0; variant < 4; ++variant) {
            TestNotes notes;
            addNote(&notes, QStringLiteral("a"), 12, QStringLiteral("NOTEA"));
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.fields = &notes;
            // 0: text with the reference, then a table.  1: a table, then text with the reference.  2: text, a table, then text with the reference.  3: as 1, with nothing before.
            fillStory(fd.story(), variant == 1 ? 1 : 3, 3);
            if (variant == 0) addReference(fd.story(), fd.story()->findBlockByNumber(1).position() + 2, QStringLiteral("a"));
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            spec.columnWidth = 100;
            if (variant == 1 || variant == 3) {
                QTextCursor c(fd.story());
                c.select(QTextCursor::Document);
                c.removeSelectedText();
            }
            const TestTable table = addTable(fd, spec);
            if (variant == 3) {
                // (the story starts with the table: the paragraph that was left of the filler goes)
                const QTextBlockFormat bf1 = fd.story()->begin().next().blockFormat();
                QTextCursor rm(fd.story());
                rm.setPosition(0);
                rm.setPosition(fd.story()->begin().next().position(), QTextCursor::KeepAnchor);
                rm.removeSelectedText();
                QTextCursor fix(fd.story());
                fix.setPosition(0);
                fix.setBlockFormat(bf1);
            }
            QTextCursor t(fd.story());
            t.movePosition(QTextCursor::End);
            t.insertBlock(QTextBlockFormat());
            t.insertText(QStringLiteral("after the table with a reference"));
            if (variant >= 1) addReference(fd.story(), fd.story()->characterCount() - 2, QStringLiteral("a"));
            Paginator pg(&fd);
            const auto page = pg.materialize(0);
            QVERIFY(page);
            const double bottom = fd.sections[0].content().bottom();
            const TextItem *note = boxHolding(*page, QStringLiteral("NOTEA"));
            QVERIFY2(note, qPrintable(QStringLiteral("variant %1: the note is not on the page").arg(variant)));
            double tablesBottom = 0;
            for (const auto &f : page->tables) tablesBottom = std::max(tablesBottom, f->origin.y() + f->height);
            QVERIFY2(note->rect.bottom() <= bottom + 0.01 && note->rect.bottom() > bottom - 30, qPrintable(QStringLiteral("variant %1: the note ends at %2, the text area at %3").arg(variant).arg(note->rect.bottom()).arg(bottom)));
            QVERIFY2(note->rect.top() >= tablesBottom - 0.01, qPrintable(QStringLiteral("variant %1: the note begins at %2, the table ends at %3").arg(variant).arg(note->rect.top()).arg(tablesBottom)));
            Q_UNUSED(table);
        }
    }

    // A picture set in the text of a cell is in the page's document and is drawn with the table, where its line is.
    void aPictureInTheTextOfACellIsDrawnWithTheTable()
    {
        FlowDocument fd;
        fd.images.push_back({tinyPng(6, 6, Qt::blue), QStringLiteral("png")});
        TestTable spec;
        spec.rows = 2;
        spec.cols = 2;
        spec.columnWidth = 150;
        const TestTable table = addTable(fd, spec);
        auto item = std::make_shared<PictureItem>();
        item->rect = QRectF(0, 0, 30, 20);
        item->imgRect = QRectF(0, 0, 30, 20);
        item->imageId = QStringLiteral("flow-img:0");
        item->wrap.left = item->wrap.right = item->wrap.top = item->wrap.bottom = 0;
        const int at = table.rowStarts[1] + 2;
        {
            QTextCursor c(fd.story());
            c.setPosition(at);
            QTextCharFormat inl;
            inl.setFontPointSize(11);
            inl.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(item->toJson()).toJson(QJsonDocument::Compact)));
            c.insertText(QString(QChar(0xFFFC)), inl);
        }
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page && page->tables.size() == 1);
        QCOMPARE(page->cellImages.size(), 1);
        const QString id = page->cellImages.value(QStringLiteral("flow-img:0"));
        QVERIFY(!id.isEmpty() && !id.startsWith(QStringLiteral("flow-img:")));
        QVERIFY(page->doc->images.contains(id));
        QImage img(int(page->size.width()), int(page->size.height()), QImage::Format_RGB32);
        img.fill(Qt::white);
        {
            QPainter painter(&img);
            paintTables(&painter, *page, TablePaint());
        }
        QLineF caret;
        QVERIFY(page->tables[0]->caretLine(at, &caret));
        // The picture stands on the line's baseline, to the right of the caret's place, as tall as it is: its middle is blue, and the cell's other places are not.
        int blue = 0;
        for (int y = int(caret.y1()); y < int(caret.y2()); ++y)
            for (int x = int(caret.x1()); x < int(caret.x1()) + 30; ++x) blue += qBlue(img.pixel(x, y)) > 200 && qRed(img.pixel(x, y)) < 60 ? 1 : 0;
        QVERIFY2(blue > 300, qPrintable(QStringLiteral("%1 blue pixels near the caret at %2, %3").arg(blue).arg(caret.x1()).arg(caret.y1())));
        int elsewhere = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = int(page->tables[0]->origin.x() + 160); x < img.width(); ++x) elsewhere += qBlue(img.pixel(x, y)) > 200 && qRed(img.pixel(x, y)) < 60 ? 1 : 0;
        QCOMPARE(elsewhere, 0);
    }

    // Whether a table's header rows are repeated on a page that a section with other columns shares is decided by the room the table has there (the lines are found with it, and
    // the fragment is made with it): the text under the table begins where the table ends, whichever the room is.
    void repeatedHeaderRowsAreDecidedByTheRoomOfTheColumnTheTableIsIn()
    {
        int repeating = 0, notRepeating = 0;
        for (int rows = 28; rows < 50; ++rows) {
            FlowDocument fd;
            Section &one = fd.sections[0];
            Section two = one;
            two.continuous = true;
            two.columns = 2;
            two.columnGap = 20;
            fd.sections << two;
            TestTable spec;
            spec.rows = rows;
            spec.cols = 2;
            spec.columnWidth = 150;
            spec.setRow = [](int r, RowData &row) { row.header = r == 0; };
            spec.text = [](int r, int c) {
                QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
                if (r == 0) for (int i = 1; i < 5; ++i) t += QStringLiteral("\nheader line %1").arg(i);
                return t;
            };
            addTable(fd, spec);
            {
                QTextCursor c(fd.story());
                c.movePosition(QTextCursor::End);
                QTextBlockFormat bf;
                bf.setProperty(flow::fp::SectionBreakAfter, true);
                c.insertBlock(bf);
                c.insertText(QStringLiteral("END OF ONE"));
                QTextBlockFormat plain;
                plain.setBottomMargin(6);
                for (int i = 0; i < 6; ++i) {
                    c.insertBlock(plain);
                    c.insertText(QStringLiteral("Text in the two columns that takes a few lines of the narrow column, and goes on for some more words than that."));
                }
            }
            Paginator pg(&fd);
            for (int k = 0; k < pg.pageCount(); ++k) {
                const auto lines = pg.lines(k);
                const auto page = pg.materialize(k);
                QVERIFY(page);
                for (const auto &fragment : page->tables) {
                    // The first text line under the table's rows: it begins at or below the fragment's foot.
                    double textTop = -1;
                    bool seenRow = false;
                    for (const auto &l : lines) {
                        const bool cell = fd.story()->findBlock(l.block).blockFormat().intProperty(fp::Cell) != 0;
                        if (cell) seenRow = true;
                        else if (seenRow) { textTop = l.rect.top() + pg.page(k).content.top(); break; }
                    }
                    if (k > 0) (fragment->header ? repeating : notRepeating)++;
                    if (textTop < 0) continue;
                    QVERIFY2(textTop >= fragment->origin.y() + fragment->height - 0.01, qPrintable(QStringLiteral("%1 rows, page %2: the text under the table begins at %3, the table (%4 header) ends at %5").arg(rows).arg(k).arg(textTop).arg(fragment->header ? "with a" : "without a").arg(fragment->origin.y() + fragment->height)));
                }
            }
        }
        QVERIFY2(repeating + notRepeating > 0, qPrintable(QStringLiteral("%1 pages repeat the header, %2 do not").arg(repeating).arg(notRepeating)));
    }

    // A table whose indent is to the text of its first cell (the way tables were set before the program of 2013) has its left edge left of that by the cell's margin; a
    // centered one is in the middle of the text area, and a right one at its right edge.
    void aTableIsIndentedToTheTextOfItsFirstCellOrCenteredOrRight()
    {
        for (const int align : {0, 1, 2, 3}) {
            FlowDocument fd;
            TestTable spec;
            spec.rows = 2;
            spec.cols = 2;
            spec.columnWidth = 100;
            spec.margins = QMarginsF(7, 1, 3, 1);
            spec.setTable = [align](TableData &t) {
                t.indent = 20;
                t.indentToText = align == 1;
                t.align = align == 2 ? TableData::Align::Center : align == 3 ? TableData::Align::Right : TableData::Align::Left;
            };
            addTable(fd, spec);
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            c.insertBlock(QTextBlockFormat());
            Paginator pg(&fd);
            const auto page = pg.materialize(0);
            QVERIFY(page && page->tables.size() == 1);
            const QRectF col = fd.section(0).columnRect(0);
            const double x = page->tables[0]->origin.x() - col.left();
            const double want = align == 0 ? 20 : align == 1 ? 20 - 7 : align == 2 ? (col.width() - 200) / 2 : col.width() - 200;
            QVERIFY2(std::abs(x - want) < 0.01, qPrintable(QStringLiteral("alignment %1: the table's edge is at %2, not %3").arg(align).arg(x).arg(want)));
        }
    }

    // In a section of columns a table goes down a column and on into the next, row by row; a row that is split goes on from the foot of one column to the top of the next.
    void aTableFlowsFromColumnToColumn()
    {
        for (const bool tallRow : {false, true}) {
            FlowDocument fd;
            fd.sections[0].columns = 2;
            fd.sections[0].paper = QSizeF(400, 300);
            fd.sections[0].margins = QMarginsF(36, 36, 36, 36);
            fillStory(fd.story(), 2, 9, {}, 6, 8);
            TestTable spec;
            spec.rows = tallRow ? 3 : 120;
            spec.cols = 2;
            spec.columnWidth = 70;
            spec.text = [&](int r, int c) {
                QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
                if (tallRow && r == 1 && c == 0)
                    for (int i = 0; i < 120; ++i) t += QStringLiteral("\nline %1").arg(i);
                return t;
            };
            const TestTable table = addTable(fd, spec);
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            QVERIFY2(pages >= 2, qPrintable(QString::number(pages)));
            QVector<int> seen;
            bool second = false;
            int partsInTheSecondColumn = 0;
            for (int k = 0; k < pages; ++k) {
                for (const auto &l : pg.lines(k)) {
                    if (!fd.story()->findBlock(l.block).blockFormat().intProperty(fp::Cell)) continue;
                    if (l.frame == 1) second = true;
                    if (tallRow && l.pos == table.rowStarts[1] && l.frame == 1) ++partsInTheSecondColumn;
                    if (seen.isEmpty() || seen.last() != l.pos) seen << l.pos;
                    QVERIFY2(l.rect.bottom() <= fd.section(0).content().height() + 0.1 || (tallRow && l.pos == table.rowStarts[1]), qPrintable(QStringLiteral("page %1: a row ends at %2").arg(k).arg(l.rect.bottom())));
                }
                const auto page = pg.materialize(k);
                QVERIFY(page);
            }
            QVERIFY2(second, "some rows are in the second column");
            QCOMPARE(seen, table.rowStarts);
            if (tallRow) QVERIFY(partsInTheSecondColumn >= 1);
            // A page with rows in both columns has a fragment in each, the second to the right of the first.
            for (int k = 0; k < pages; ++k) {
                const auto page = pg.materialize(k);
                if (page->tables.size() < 2) continue;
                QVERIFY(page->tables[1]->origin.x() > page->tables[0]->origin.x() + 50);
            }
        }
    }

    // A file that says more than it can mean: cells that span further than the table has columns, a table that is a single row with a header, a header row taller than the
    // page, and cells joined down that have nothing to be joined to. Every one lays out and ends.
    void hostileTablesLayOutAndEnd()
    {
        FlowDocument fd;
        fd.sections[0].paper = QSizeF(300, 200);
        fd.sections[0].margins = QMarginsF(30, 30, 30, 30);
        TestTable spec;
        spec.rows = 12;
        spec.cols = 3;
        spec.columnWidth = 60;
        spec.text = [](int r, int c) {
            QString t = QStringLiteral("r%1c%2").arg(r).arg(c);
            if (r == 0)
                for (int i = 0; i < 40; ++i) t += QStringLiteral("\nheading line %1").arg(i);   // (a header row taller than the page)
            if (r == 3 && c == 1)
                for (int i = 0; i < 30; ++i) t += QStringLiteral("\nrow three line %1").arg(i);
            return t;
        };
        spec.setRow = [](int r, RowData &row) { row.header = r == 0; row.before = r == 5 ? 2 : 0; row.after = r == 6 ? 1000 : 0; };
        spec.setCell = [](int r, int c, CellData &cell) {
            if (r == 2 && c == 0) cell.span = 500;                                   // (more columns than the table has)
            if (r == 4) cell.vmerge = CellData::VMerge::Continue;                    // (continuing a cell that is not there)
            if (r == 7 && c == 0) cell.vmerge = CellData::VMerge::Restart;           // (joined to the rows below, one of which is missing a cell)
            if (r == 8 && c == 0) cell.vmerge = CellData::VMerge::Continue;
            if (r == 9 && c == 1) cell.span = 3;
        };
        const TestTable table = addTable(fd, spec);
        QTextCursor c(fd.story());
        c.movePosition(QTextCursor::End);
        c.insertBlock(QTextBlockFormat());
        c.insertText(QStringLiteral("after"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY2(pages >= 2 && pages < 400, qPrintable(QString::number(pages)));
        QVERIFY(pg.page(pages - 1).start.pos < fd.story()->characterCount());
        for (int k = 0; k < pages; ++k) QVERIFY(pg.materialize(k));
        // Every row begins on some page.
        QVector<int> seen;
        for (int k = 0; k < pages; ++k)
            for (const auto &l : pg.lines(k))
                if (fd.story()->findBlock(l.block).blockFormat().intProperty(fp::Cell) && (seen.isEmpty() || seen.last() != l.pos)) seen << l.pos;
        QCOMPARE(seen, table.rowStarts);
    }

    // After edits in and around a long table, the pages are the ones a new paginator finds, the pages near an edit are the only ones that are laid out for it, and the rows
    // that were not touched are not measured again.
    void paginatorFollowsEditsInTablesAndMeasuresOnlyTheRowsTheyTouch()
    {
        FlowDocument fd;
        fillStory(fd.story(), 6, 12, {}, 20, 40);
        TestTable spec;
        spec.rows = 400;
        spec.cols = 3;
        spec.columnWidth = 120;
        spec.setRow = [](int r, RowData &row) { row.header = r == 0; };
        spec.text = [](int r, int c) {
            return r % 11 == 5 && c == 1 ? QStringLiteral("a longer cell %1 with enough words to take a few lines in a column this narrow").arg(r) : QStringLiteral("r%1c%2").arg(r).arg(c);
        };
        spec.setCell = [](int r, int c, CellData &cell) {
            if (c == 2 && r % 20 == 3) cell.vmerge = CellData::VMerge::Restart;
            if (c == 2 && r % 20 == 4) cell.vmerge = CellData::VMerge::Continue;
        };
        const TestTable table = addTable(fd, spec);
        QTextCursor tail(fd.story());
        tail.movePosition(QTextCursor::End);
        tail.insertBlock(QTextBlockFormat());
        tail.insertText(QStringLiteral("after the table"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages > 8);
        auto sameAsFresh = [&](const QString &what) {
            Paginator fresh(&fd);
            const int want = fresh.pageCount(), got = pg.pageCount();
            if (want != got) return QStringLiteral("%1: %2 pages, a new paginator finds %3").arg(what).arg(got).arg(want);
            for (int k = 0; k < want; ++k) {
                if (fresh.page(k).start != pg.page(k).start) return QStringLiteral("%1: page %2 starts elsewhere than a new paginator has it").arg(what).arg(k);
                const auto a = fresh.lines(k), b = pg.lines(k);
                if (a.size() != b.size()) return QStringLiteral("%1: page %2 has %3 lines, a new paginator's has %4").arg(what).arg(k).arg(b.size()).arg(a.size());
                for (int i = 0; i < a.size(); ++i)
                    if (a[i].pos != b[i].pos || std::abs(a[i].rect.top() - b[i].rect.top()) > 0.01 || std::abs(a[i].rect.height() - b[i].rect.height()) > 0.01)
                        return QStringLiteral("%1: line %2 of page %3 is in another place").arg(what).arg(i).arg(k);
            }
            return QString();
        };
        auto cellBlock = [&](int row, int col) {
            QTextBlock b = fd.story()->findBlock(table.rowStarts[row]);
            for (int n = 0; n < col; ++n) {
                const int key = b.blockFormat().intProperty(fp::Cell);
                while (b.blockFormat().intProperty(fp::Cell) == key) b = b.next();
            }
            return b;
        };
        struct Edit { QString what; std::function<void(QTextCursor &)> make; std::function<int()> where; int rows = 5; };   // (`rows`: the most that are measured again)
        const std::vector<Edit> edits = {
            {"a letter in a cell far down", [](QTextCursor &c) { c.insertText(QStringLiteral("x")); }, [&] { return cellBlock(300, 1).position() + 1; }},
            {"words that make a row taller", [](QTextCursor &c) { c.insertText(QStringLiteral(" and many more words that go on for several lines in this cell of the table")); }, [&] { return cellBlock(200, 0).position() + 2; }},
            {"a paragraph break in a cell", [](QTextCursor &c) { c.insertBlock(); }, [&] { return cellBlock(250, 1).position() + 1; }},
            {"the text in a header cell", [](QTextCursor &c) { c.insertText(QStringLiteral(" with a long heading that wraps in two lines")); }, [&] { return cellBlock(0, 0).position() + 2; }},
            {"a paragraph before the table taken out", [](QTextCursor &c) { c.select(QTextCursor::BlockUnderCursor); c.removeSelectedText(); }, [&] { return fd.story()->findBlockByNumber(2).position() + 1; }, 0},
            {"words in a cell joined down", [](QTextCursor &c) { c.insertText(QStringLiteral(" and a line more\nand another here")); }, [&] { return cellBlock(63, 2).position() + 1; }},
            {"a row taken out", [&](QTextCursor &c) {
                 const QTextBlock a = cellBlock(120, 0), b = cellBlock(121, 0);
                 c.setPosition(a.position() - 1);
                 c.setPosition(b.position() - 1, QTextCursor::KeepAnchor);
                 c.removeSelectedText();
             }, [&] { return 0; }},
        };
        QVERIFY2(sameAsFresh("before editing").isEmpty(), "starts");
        for (const Edit &e : edits) {
            QTextCursor c(fd.story());
            c.setPosition(std::clamp(e.where(), 0, fd.story()->characterCount() - 1));
            const int at = c.position();
            const qint64 measuredBefore = pg.tableRowsMeasured();
            e.make(c);
            const qint64 before = pg.pagesLaidOut();
            pg.paginate(pg.pageOfPosition(std::min(at, fd.story()->characterCount() - 1)) + 1);
            const qint64 near = pg.pagesLaidOut() - before;
            QVERIFY2(near <= 8, qPrintable(QStringLiteral("%1: %2 pages near the edit").arg(e.what).arg(near)));
            pg.pageCount();
            QVERIFY(pg.settled());
            // Only the rows the edit touched (and those beside them, and those cells joined down tie them to) were measured again: the rest are as tall as they were.
            const qint64 measured = pg.tableRowsMeasured() - measuredBefore;
            QVERIFY2(measured <= e.rows, qPrintable(QStringLiteral("%1: %2 rows measured (at most %3)").arg(e.what).arg(measured).arg(e.rows)));
            const QString bad = sameAsFresh(e.what);
            QVERIFY2(bad.isEmpty(), qPrintable(bad));
        }
    }

    // Typing in a long table costs about what it does in text: the rows above the edit are not measured again.
    void typingInALongTableIsFast()
    {
        TestTable spec;
        spec.rows = 3000;
        spec.cols = 3;
        spec.columnWidth = 120;
        std::unique_ptr<FlowDocument> fd;
        std::unique_ptr<Paginator> pg;
        TestTable table;
        // A new document of the table, nothing laid out yet, for each run.
        const auto fresh = [&] {
            pg.reset();
            fd = std::make_unique<FlowDocument>();
            table = addTable(*fd, spec);
            pg = std::make_unique<Paginator>(fd.get());
        };
        int pages = 0;
        const double whole = bestNanoseconds([&] { pages = pg->pageCount(); }, 3, fresh) / 1e6;
        QVERIFY(pages > 40);
        QTextCursor c;
        const double typed = bestNanoseconds([&] {
            c.insertText(QStringLiteral("x"));
            pg->paginate(pg->pageOfPosition(c.position()) + 1);
        }, 3, [&] {
            c = QTextCursor();
            fresh();
            pg->pageCount();
            c = QTextCursor(fd->story());
            c.setPosition(fd->story()->findBlock(table.rowStarts[2500]).position() + 1);
        }) / 1e6;
        qInfo("a table of 3,000 rows: all of it laid out in %.0f ms, a letter typed in row 2,500 in %.0f ms", whole, typed);
        QVERIFY2(typed < whole / 8, qPrintable(QStringLiteral("typing took %1 ms, the whole %2 ms").arg(typed).arg(whole)));
    }

    // ---- the pages that are looked at ----

    // A page made into a document of its own is laid out by the page layer in the lines the paginator found: whole paragraphs, the rest of
    // one that began on the page before, and the beginning of one that goes on, in one column and in two.
    void materializedPagesHaveTheLinesThePaginatorFound()
    {
        LayoutCache cache;
        int tails = 0, heads = 0;
        for (int columns : {1, 2, 3})
            for (unsigned seed : {31u, 32u, 36u}) {
                FlowDocument fd;
                fd.sections[0].columns = columns;
                fd.sections[0].columnGap = 18;
                fd.sections[0].margins = QMarginsF(60, 66, 60, 70);
                fillStory(fd.story(), 70, seed, [](int i, QTextBlockFormat &bf) {
                    bf.setProperty(tp::WidowControl, true);
                    bf.setAlignment(i % 3 ? Qt::AlignJustify : Qt::AlignLeft);
                    if (i % 5 == 0) bf.setTextIndent(24);
                    if (i % 11 == 0) bf.setTopMargin(14);
                    if (i % 13 == 0) { bf.setProperty(tp::KeepWithNext, true); bf.setProperty(tp::KeepTogether, true); }
                }, 20, 120);
                Paginator pg(&fd);
                const int pages = pg.pageCount();
                QVERIFY(pages > 6);
                for (int k = 0; k < pages; ++k) {
                    const QString bad = materializedDiffers(fd, pg, k, cache);
                    QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1 columns, seed %2: %3").arg(columns).arg(seed).arg(bad)));
                    const auto page = pg.materialize(k);
                    tails += page->continuesFromBefore;
                    heads += page->continuesOnAfter;
                }
            }
        QVERIFY2(tails > 4 && heads > 4, "no page ever began or ended in the middle of a paragraph");
    }

    // Lists go on across pages with their numbers, and the rest of an item that goes on has no number of its own.
    void materializedPagesKeepListNumbers()
    {
        FlowDocument fd;
        fillStory(fd.story(), 60, 33, [](int, QTextBlockFormat &bf) { bf.setProperty(tp::WidowControl, true); }, 18, 90);
        {
            QTextCursor c(fd.story());
            c.setPosition(fd.story()->findBlockByNumber(8).position());
            c.setPosition(fd.story()->findBlockByNumber(50).position(), QTextCursor::KeepAnchor);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDecimal);
            lf.setIndent(1);
            c.createList(lf);
        }
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages > 5);
        LayoutCache cache;
        int checked = 0;
        for (int k = 0; k < pages; ++k) {
            const QString bad = materializedDiffers(fd, pg, k, cache);
            QVERIFY2(bad.isEmpty(), qPrintable(bad));
            const auto page = pg.materialize(k);
            // Each item's number is what it is in the document.
            QTextDocument *slice = page->doc->storyDoc(QStringLiteral("flow-story-%1").arg(k));
            QVERIFY(slice);
            for (QTextBlock b = slice->begin(); b.isValid(); b = b.next()) {
                const QTextBlock original = fd.story()->findBlock(b.position() + page->storyFrom);
                const bool tail = b == slice->begin() && page->continuesFromBefore;
                if (tail) { QVERIFY(!b.textList()); continue; }
                QCOMPARE(b.textList() != nullptr, original.textList() != nullptr);
                if (b.textList()) {
                    QCOMPARE(b.textList()->itemText(b), original.textList()->itemText(original));
                    ++checked;
                }
            }
        }
        QVERIFY(checked > 20);
    }

    // Making a page again gives the same ids; each page's are its own.
    void materializedPagesHaveStableIds()
    {
        FlowDocument fd;
        fd.sections[0].columns = 2;
        fillStory(fd.story(), 120, 34);
        Paginator pg(&fd);
        const auto a = pg.materialize(2), b = pg.materialize(2), c = pg.materialize(3);
        QVERIFY(a && b && c);
        QCOMPARE(a->boxes, b->boxes);
        QCOMPARE(a->doc->pages[0]->id, b->doc->pages[0]->id);
        QVERIFY(a->boxes != c->boxes);
        QVERIFY(a->doc->pages[0]->id != c->doc->pages[0]->id);
        QVERIFY(a->doc->item(a->boxes.first()));
        // The boxes are in the columns of the section, in a chain.
        QCOMPARE(a->boxes.size(), 2);
        auto *first = static_cast<TextItem *>(a->doc->item(a->boxes[0]));
        auto *second = static_cast<TextItem *>(a->doc->item(a->boxes[1]));
        QCOMPARE(first->nextId, second->id);
        QCOMPARE(first->rect.left(), fd.sections[0].columnRect(0).left());
        QCOMPARE(second->rect.left(), fd.sections[0].columnRect(1).left());
        QVERIFY(pg.materialize(9999) == nullptr);
    }

    // The page layer draws a page: the text is on the paper, inside the margins, and each section's page is its own size.
    void materializedPagesPaint()
    {
        FlowDocument fd;
        fillStory(fd.story(), 50, 35, [](int i, QTextBlockFormat &bf) { if (i == 19) bf.setProperty(flow::fp::SectionBreakAfter, true); });
        Section wide;
        wide.orientation = Section::Orientation::Landscape;
        wide.margins = QMarginsF(100, 80, 100, 80);
        fd.sections << wide;
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        LayoutCache cache;
        bool sawWide = false;
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            const QSizeF size = page->doc->pageSize();
            QCOMPARE(size, fd.section(page->section).pageSize());
            sawWide |= size.width() > size.height();
            QImage image(int(size.width()), int(size.height()), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::white);
            {
                QPainter painter(&image);
                PaintContext ctx;
                ctx.doc = page->doc.get();
                ctx.cache = &cache;
                Renderer::paintPage(&painter, ctx, 0);
            }
            // Dark pixels only inside the margins, and some there.
            const QMarginsF m = fd.section(page->section).margins;
            int inside = 0, outside = 0;
            for (int y = 0; y < image.height(); ++y)
                for (int x = 0; x < image.width(); ++x) {
                    if (qGray(image.pixel(x, y)) > 128) continue;
                    const bool in = x >= m.left() - 1 && x <= size.width() - m.right() + 1 && y >= m.top() - 1 && y <= size.height() - m.bottom() + 1;
                    (in ? inside : outside)++;
                }
            QVERIFY2(inside > 200, qPrintable(QStringLiteral("page %1: %2 dark pixels").arg(k).arg(inside)));
            QCOMPARE(outside, 0);
        }
        QVERIFY(sawWide);
    }


    // A page that has rows of a table has a fragment to draw them, at the place of their lines, and in its story one paragraph in their place that takes
    // as many positions as they do in the document's, so that every other position on the page is the same distance from the page's first.
    void aPageWithATableDrawsItAndKeepsThePositionsOfTheTextAroundIt()
    {
        FlowDocument fd;
        fillStory(fd.story(), 2, 8, {}, 5, 8);
        TestTable spec;
        spec.rows = 4;
        spec.cols = 3;
        spec.columnWidth = 120;
        spec.setCell = [](int r, int c, CellData &cell) {
            if (r == 1 && c == 1) cell.fill = QColor(255, 0, 0);
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = 1;
            line.color = QColor(0, 0, 255);
            cell.top = cell.left = cell.bottom = cell.right = line;
        };
        spec.setTable = [](TableData &t) { t.indent = 10; };
        const TestTable table = addTable(fd, spec);
        QTextCursor c(fd.story());
        c.movePosition(QTextCursor::End);
        c.insertBlock(QTextBlockFormat());
        c.insertText(QStringLiteral("zebra after the table"));
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QCOMPARE(page->tables.size(), 1);
        const TableFragment &fragment = *page->tables[0];
        const auto rows = rowLines(pg, fd, 0);
        const QRectF col = fd.section(0).columnRect(0);
        QCOMPARE(fragment.origin.x(), col.left() + 10);
        QVERIFY(std::abs(fragment.origin.y() - (col.top() + rows.first().rect.top())) < 0.01);
        QVERIFY(std::abs(fragment.height - (rows.last().rect.bottom() - rows.first().rect.top())) < 0.01);
        QCOMPARE(fragment.layout->width(), 360.0);
        // The page's story has the text after the table where the document has it.
        QTextDocument *slice = page->doc->stories.begin().value()->doc.get();
        const int at = fd.story()->toPlainText().indexOf(QLatin1String("zebra"));
        QVERIFY(at > 0);
        QCOMPARE(slice->toPlainText().mid(at - page->storyFrom, 5), QStringLiteral("zebra"));
        QCOMPARE(page->storyFrom, 0);
        // (The text box holds the line that stands for the rows, and the text after them is under it.)
        const auto *box = static_cast<TextItem *>(page->doc->item(page->boxes.first()));
        QVERIFY(box->rect.bottom() > fragment.rect().bottom() + 5);
        // Drawn: the cell's fill is its color, the lines are blue, the text is dark, and the text after the table is below it.
        QImage image(int(fd.section(0).pageSize().width()) * 2, int(fd.section(0).pageSize().height()) * 2, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        {
            QPainter painter(&image);
            painter.scale(2, 2);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = page->doc.get();
            ctx.cache = &cache;
            Renderer::paintPage(&painter, ctx, 0);
            for (const auto &f : page->tables) f->paint(&painter, TablePaint());
        }
        const auto pixel = [&](double x, double y) { return image.pixelColor(int(x * 2), int(y * 2)); };
        const double rowTop = fragment.origin.y();
        const double r1 = rowTop + rows[0].rect.height();
        // The fill: the cell is the second column of the second row (a corner of it, clear of the text).
        QVERIFY(std::abs(pixel(fragment.origin.x() + 120 + 118, r1 + rows[1].rect.height() - 2).red() - 255) < 3);
        QVERIFY(pixel(fragment.origin.x() + 120 + 118, r1 + rows[1].rect.height() - 2).green() < 3);
        // The line between the first two rows is blue (on the line, not beside it).
        bool blue = false;
        for (double dy = -1.5; dy <= 1.5; dy += 0.5) blue |= pixel(fragment.origin.x() + 5, r1 + dy).blue() > 200 && pixel(fragment.origin.x() + 5, r1 + dy).red() < 100;
        QVERIFY(blue);
        // Text inside the first cell, none outside the table to its left.
        int dark = 0, leftOf = 0;
        for (int y = int(rowTop * 2); y < int(r1 * 2); ++y)
            for (int x = int(fragment.origin.x() * 2) + 3; x < int((fragment.origin.x() + 100) * 2); ++x) dark += qGray(image.pixel(x, y)) < 100 && image.pixelColor(x, y).blue() < 150;
        for (int y = int(rowTop * 2); y < int(r1 * 2); ++y)
            for (int x = 0; x < int((fragment.origin.x() - 2) * 2); ++x) leftOf += qGray(image.pixel(x, y)) < 250;
        QVERIFY2(dark > 10, qPrintable(QString::number(dark)));
        QCOMPARE(leftOf, 0);
    }

    // ---- editing ----

    // After any edit, the pages are the ones a new paginator finds, and little was laid out again to get them.
    void paginatorFollowsEditsAndLaysOutOnlyNearby()
    {
        FlowDocument fd;
        fillStory(fd.story(), 3500, 41, [](int i, QTextBlockFormat &bf) {
            bf.setProperty(tp::WidowControl, true);
            if (i % 20 == 0) { bf.setProperty(tp::KeepWithNext, true); bf.setTopMargin(10); }
        }, 30, 90);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages > 300);
        std::mt19937 rng(3);
        auto sameAsFresh = [&](const QString &what) {
            Paginator fresh(&fd);
            const int want = fresh.pageCount(), got = pg.pageCount();
            if (want != got) return QStringLiteral("%1: %2 pages, a new paginator finds %3").arg(what).arg(got).arg(want);
            for (int k = 0; k < want; ++k)
                if (fresh.page(k).start != pg.page(k).start) {
                    const auto a = fresh.page(k).start, b = pg.page(k).start;
                    return QStringLiteral("%1: page %2 starts elsewhere than a new paginator has it (new: %3/%4/%5/%6, kept: %7/%8/%9/%10)").arg(what).arg(k)
                        .arg(a.pos).arg(a.skip).arg(a.hard).arg(a.section).arg(b.pos).arg(b.skip).arg(b.hard).arg(b.section);
                }
            return QString();
        };
        const int last = fd.story()->characterCount() - 1;
        struct Edit { QString what; std::function<void(QTextCursor &)> make; int where; };
        const std::vector<Edit> edits = {
            {"a letter in the middle", [](QTextCursor &c) { c.insertText(QStringLiteral("x")); }, last / 2},
            {"a letter at the start", [](QTextCursor &c) { c.insertText(QStringLiteral("x")); }, 0},
            {"a letter at the end", [](QTextCursor &c) { c.insertText(QStringLiteral("x")); }, last},
            {"a paragraph break", [](QTextCursor &c) { c.insertBlock(); }, last / 3},
            {"a line of words", [](QTextCursor &c) { c.insertText(QStringLiteral("An added sentence that makes the paragraph a line longer than it was before. ")); }, last / 4},
            {"a paragraph taken out", [](QTextCursor &c) { c.select(QTextCursor::BlockUnderCursor); c.removeSelectedText(); }, last * 3 / 5},
            {"three pages taken out", [last](QTextCursor &c) { c.setPosition(c.position() + 9000, QTextCursor::KeepAnchor); c.removeSelectedText(); Q_UNUSED(last); }, last / 5},
            {"a page break before", [](QTextCursor &c) { QTextBlockFormat bf = c.blockFormat(); bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); c.setBlockFormat(bf); }, last * 2 / 3},
            {"a section break", [](QTextCursor &c) { QTextBlockFormat bf = c.blockFormat(); bf.setProperty(flow::fp::SectionBreakAfter, true); c.setBlockFormat(bf); }, last / 2},
            {"a heading's format", [](QTextCursor &c) { QTextBlockFormat bf = c.blockFormat(); bf.setTopMargin(40); bf.setProperty(tp::KeepWithNext, true); c.setBlockFormat(bf); }, last / 7},
        };
        fd.sections << fd.sections[0];
        QVERIFY2(sameAsFresh("before editing").isEmpty(), "starts");
        for (const Edit &e : edits) {
            QTextCursor c(fd.story());
            c.setPosition(std::min(e.where, fd.story()->characterCount() - 1));
            const int at = c.position();
            e.make(c);
            // What the page on the screen needs: the pages from before the edit to the one after it.
            const qint64 before = pg.pagesLaidOut();
            QElapsedTimer timer;
            timer.start();
            pg.paginate(pg.pageOfPosition(std::min(at, fd.story()->characterCount() - 1)) + 1);
            const qint64 near = pg.pagesLaidOut() - before;
            qInfo("%s: %lld pages laid out for the pages near it, %.1f ms", qPrintable(e.what), (long long)near, timer.nsecsElapsed() / 1e6);
            QVERIFY2(near <= 6, qPrintable(QStringLiteral("%1: %2 pages near the edit").arg(e.what).arg(near)));
            // The rest, which goes on in the background.
            pg.pageCount();
            QVERIFY(pg.settled());
            const qint64 all = pg.pagesLaidOut() - before;
            const QString bad = sameAsFresh(e.what);
            QVERIFY2(bad.isEmpty(), qPrintable(bad));
            // (An edit that moves the pages after it, a new line or a break, lays them out again until the pages settle.)
            if (e.what.startsWith("a letter")) QVERIFY2(all <= 8, qPrintable(QStringLiteral("%1: %2 pages in all").arg(e.what).arg(all)));
        }
    }

    // A paragraph that is kept together is never split between pages (these are all shorter than a page).
    void paginatorKeepsLinesTogether()
    {
        int boundaries = 0;
        for (unsigned seed : {51u, 52u, 53u}) {
            FlowDocument fd;
            fillStory(fd.story(), 150, seed, [](int i, QTextBlockFormat &bf) { if (i % 3 == 1) bf.setProperty(tp::KeepTogether, true); }, 10, 90);
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            for (int k = 0; k + 1 < pages; ++k) {
                const auto a = pg.lines(k), b = pg.lines(k + 1);
                if (a.last().block != b.first().block) { ++boundaries; continue; }
                QVERIFY2(!formatAt(fd, a.last().block).boolProperty(tp::KeepTogether), qPrintable(QStringLiteral("seed %1: a paragraph kept together is split after page %2").arg(seed).arg(k)));
            }
        }
        QVERIFY(boundaries > 10);
    }

    // The long story the speed targets are set by: paragraphs of about 380 characters and a heading in every twenty, 26,443 of them. The bounds are
    // ten times the targets (a full pagination in 5 seconds, the first page in 0.3, a typed character in 50 milliseconds), for machines slower than
    // this one; `writetool bench` measures it.
    void paginatorMeetsTheSpeedTargets()
    {
        FlowDocument fd;
        std::mt19937 rng(42);
        {
            static const char *const words[] = {"the", "of", "and", "to", "in", "a", "is", "that", "for", "it", "as", "was", "with", "be", "by", "on", "not", "this", "are", "or", "court",
                                                "statute", "section", "rule", "appeal", "judgment", "evidence", "applicable", "jurisdiction", "pursuant", "therefore", "defendant"};
            QTextCursor c(fd.story());
            QTextCharFormat body, head;
            body.setFontPointSize(11);
            head.setFontPointSize(16);
            head.setFontWeight(QFont::Bold);
            c.beginEditBlock();
            for (int i = 0; i < 26443; ++i) {
                const bool isHead = i % 20 == 0;
                QTextBlockFormat bf;
                bf.setBottomMargin(6);
                if (isHead) { bf.setTopMargin(12); bf.setProperty(tp::KeepWithNext, true); bf.setProperty(tp::KeepTogether, true); }
                else bf.setProperty(tp::WidowControl, true);
                if (i == 0) c.setBlockFormat(bf); else c.insertBlock(bf);
                c.setBlockCharFormat(isHead ? head : body);
                QString text;
                const int n = isHead ? 4 : 40 + int(rng() % 41);
                for (int k = 0; k < n; ++k) text += (k ? " " : "") + QString::fromLatin1(words[rng() % (sizeof words / sizeof *words)]);
                c.insertText(text + QLatin1Char('.'), isHead ? head : body);
            }
            c.endEditBlock();
        }
        Paginator pg(&fd);
        QElapsedTimer t;
        t.start();
        const auto first = pg.materialize(0);
        const double firstMs = t.nsecsElapsed() / 1e6;
        QVERIFY(first && !first->boxes.isEmpty());
        QVERIFY2(pg.pagesLaidOut() <= 2, qPrintable(QString::number(pg.pagesLaidOut())));   // (the first page is shown without laying out the rest)
        t.restart();
        const int pages = pg.pageCount();
        const double allMs = t.nsecsElapsed() / 1e6;
        QVERIFY(pages > 1500);
        // A typed character in the middle, and what the page needs of it.
        const int at = fd.story()->characterCount() / 2;
        double worst = 0;
        for (int i = 0; i < 5; ++i) {
            QTextCursor c(fd.story());
            c.setPosition(at + i * 101);
            const qint64 before = pg.pagesLaidOut();
            t.restart();
            c.insertText(QStringLiteral("x"));
            pg.paginate(pg.pageOfPosition(at + i * 101) + 1);
            const auto page = pg.materialize(pg.pageOfPosition(at + i * 101));
            worst = std::max(worst, t.nsecsElapsed() / 1e6);
            QVERIFY(page);
            // Only the pages by the edit are laid out again (and the page that is drawn).
            QVERIFY2(pg.pagesLaidOut() - before <= 7, qPrintable(QString::number(pg.pagesLaidOut() - before)));
        }
        pg.pageCount();
        qInfo("%d pages: the first page in %.0f ms, all of them in %.1f s, a typed character in %.1f ms (at most)", pages, firstMs, allMs / 1000, worst);
    }

    // Edits with the pages only partly laid out again between them leave no page as it was: the first edit pushes a line over a page's end, the pages
    // by it are laid out, the second edit comes before them; the pages after, still as they were before the first, must not be taken as right.
    void paginatorSettlesAnEditAfterAPartialSettle()
    {
        int differ = 0;
        for (unsigned trial = 0; trial < 40; ++trial) {
            FlowDocument fd;
            fillStory(fd.story(), 300, 70 + trial, [](int i, QTextBlockFormat &bf) {
                bf.setProperty(tp::WidowControl, true);
                if (i % 9 == 0) bf.setProperty(tp::KeepWithNext, true);
            }, 10, 90);
            Paginator pg(&fd);
            pg.pageCount();
            const int len = fd.story()->characterCount() - 1;
            const int p1 = 2000 + int((trial * 7919u) % unsigned(len / 2));
            {
                QTextCursor c(fd.story());
                c.setPosition(p1);
                c.insertText(QStringLiteral("one more sentence that pushes a line over the page. "));
            }
            pg.paginate(pg.pageOfPosition(p1) + 1);   // (not all of it)
            {
                QTextCursor c(fd.story());
                c.setPosition(std::max(0, p1 - 1 - int((trial * 31u) % 200u)));
                c.insertText(QStringLiteral("x"));
            }
            const QVector<Paginator::PageStart> got = startsOf(pg);
            Paginator fresh(&fd);
            if (got != startsOf(fresh)) ++differ;
            const QString bad = coverageProblem(pg, fd);
            QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("trial %1: %2").arg(trial).arg(bad)));
        }
        QCOMPARE(differ, 0);
    }

    // Space before a paragraph as tall as a page, or taller, at the top of a page: the paragraph still has a page with a line on it (the
    // space is cut to what the page holds), and every page can be made and drawn.
    void paginatorHandlesSpaceBeforeTallerThanAPage()
    {
        for (double space : {700.0, 648.0, 640.0, 5000.0}) {
            FlowDocument fd;
            fillStory(fd.story(), 6, 81, [space](int i, QTextBlockFormat &bf) {
                if (i == 0 || i == 3) bf.setTopMargin(space);
                if (i == 3) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
            }, 5, 30);
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            QVERIFY2(pages >= 2, qPrintable(QString::number(pages)));
            const QString bad = coverageProblem(pg, fd);
            QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("space %1: %2").arg(space).arg(bad)));
            for (int k = 0; k < pages; ++k) {
                QVERIFY2(!pg.lines(k).isEmpty(), qPrintable(QStringLiteral("space %1: page %2 is empty").arg(space).arg(k)));
                QVERIFY2(pg.materialize(k), qPrintable(QStringLiteral("space %1: page %2 can't be made").arg(space).arg(k)));
            }
        }
    }

    // Paragraphs kept with the next one in a chain longer than sixteen: the chain moves to the next page whole if a page holds it, and where a
    // page does not, the pages are cut as they would be without the keeping, not at a line or two.
    void paginatorKeepsLongChainsWithTheNext()
    {
        auto story = [](FlowDocument &fd, int fillers, int chain) {
            fillStory(fd.story(), fillers + chain + 1, 82, [fillers, chain](int i, QTextBlockFormat &bf) {
                if (i >= fillers && i < fillers + chain) bf.setProperty(tp::KeepWithNext, true);
            }, 1, 2);   // (a line each)
        };
        {
            FlowDocument fd;
            story(fd, 10, 25);   // the chain and the paragraph after it fit a page
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            QVERIFY(pages >= 2);
            const auto first = pg.lines(0), second = pg.lines(1);
            QCOMPARE(fd.story()->findBlock(first.last().block).blockNumber(), 9);
            QCOMPARE(fd.story()->findBlock(second.first().block).blockNumber(), 10);
        }
        {
            FlowDocument fd;
            story(fd, 5, 70);   // more than a page can hold
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            QVERIFY(pages >= 3);
            QVERIFY(pg.lines(0).size() >= 3);
            for (int k = 1; k + 1 < pages; ++k) QVERIFY2(pg.lines(k).size() >= 20, qPrintable(QStringLiteral("page %1 has %2 lines").arg(k).arg(pg.lines(k).size())));
            QVERIFY(coverageProblem(pg, fd).isEmpty());
        }
    }

    // A page break after the last paragraph makes a last page with nothing on it, as a break after the last line of a document does; so does a section
    // break, and the page is the next section's.
    void paginatorMakesAPageAfterABreakAtTheEnd()
    {
        for (int what = 0; what < 2; ++what) {
            FlowDocument fd;
            fillStory(fd.story(), 4, 83, [what](int i, QTextBlockFormat &bf) {
                if (i != 3) return;
                if (what == 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
                else bf.setProperty(flow::fp::SectionBreakAfter, true);
            }, 5, 20);
            if (what) {
                Section next;
                next.orientation = Section::Orientation::Landscape;
                fd.sections << next;
            }
            Paginator pg(&fd);
            QCOMPARE(pg.pageCount(), 2);
            QVERIFY(pg.lines(1).isEmpty());
            QVERIFY(pg.page(1).start.hard);
            QVERIFY(coverageProblem(pg, fd, true).isEmpty());
            const auto blank = pg.materialize(1);
            QVERIFY(blank);
            QVERIFY(blank->boxes.isEmpty());
            QCOMPARE(blank->size, fd.section(what).pageSize());
            QCOMPARE(blank->doc->pages.size(), 1);
            // Text typed after it goes on that page.
            QTextCursor c(fd.story());
            c.movePosition(QTextCursor::End);
            c.insertBlock(QTextBlockFormat());   // (a new paragraph has the one before's format, break and all, unless it is given another)
            c.insertText(QStringLiteral("after the break"));
            QCOMPARE(pg.pageCount(), 2);
            QVERIFY(!pg.lines(1).isEmpty());
            QVERIFY(coverageProblem(pg, fd).isEmpty());
            Paginator fresh(&fd);
            QCOMPARE(startsOf(pg), startsOf(fresh));
        }
        // The same without a break at the end is a page less.
        FlowDocument fd;
        fillStory(fd.story(), 4, 83, {}, 5, 20);
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 1);
    }

    // One paragraph of 900 kilobytes, some ten thousand lines on two hundred pages: every character is on one page and only one, the pages are alike, and the
    // pages made to be drawn are what the paginator found. (A page that went on from the one before once laid out its paragraph's earlier lines in a
    // frame that held seven thousand of them.)
    void paginatorLaysOutAHugeParagraph()
    {
        FlowDocument fd;
        {
            QString text;
            std::mt19937 rng(7);
            while (text.size() < 900 * 1024) text += QStringLiteral("word%1 and ").arg(rng() % 5000);
            QTextCursor c(fd.story());
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            c.insertText(text, cf);
        }
        std::unique_ptr<Paginator> made;
        int pages = 0;
        const qint64 ms = bestNanoseconds([&] { pages = made->pageCount(); }, 3, [&] { made = std::make_unique<Paginator>(&fd); }) / 1000000;
        Paginator &pg = *made;
        qInfo("a paragraph of 900 KB: %d pages in %.1f s", pages, ms / 1000.0);
        QVERIFY(pages > 150);
        QVERIFY2(ms < 60000, qPrintable(QString::number(ms)));
        const QString bad = coverageProblem(pg, fd);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        const int perPage = pg.lines(10).size();
        for (int k : {1, 100, 155, 156, 157, 158, pages / 2, pages - 2}) QVERIFY2(std::abs(pg.lines(k).size() - perPage) <= 1, qPrintable(QString::number(k)));
        LayoutCache cache;
        for (int k : {0, 1, 155, 156, 157, pages / 2, pages - 1}) {
            const QString differs = materializedDiffers(fd, pg, k, cache);
            QVERIFY2(differs.isEmpty(), qPrintable(differs));
        }
    }

    // Paragraphs without end in sight: two megabytes of words, and a word of two hundred thousand letters that can be broken anywhere. A page costs what a
    // page of ordinary text does, whatever came before it.
    void paginatorLaysOutHugeParagraphsInTime()
    {
        for (int what = 0; what < 2; ++what) {
            FlowDocument fd;
            {
                QString text;
                std::mt19937 rng(8);
                if (what == 0) while (text.size() < 2 * 1024 * 1024) text += QStringLiteral("w%1 ").arg(rng() % 9000);
                else text = QString(200 * 1024, QLatin1Char('x'));
                QTextCursor c(fd.story());
                QTextCharFormat cf;
                cf.setFontPointSize(11);
                c.insertText(text, cf);
            }
            std::unique_ptr<Paginator> made;
            int pages = 0;
            const qint64 ms = bestNanoseconds([&] { pages = made->pageCount(); }, 3, [&] { made = std::make_unique<Paginator>(&fd); }) / 1000000;
            Paginator &pg = *made;
            qInfo("%s: %d pages in %.1f s", what ? "a word of 200 KB" : "a paragraph of 2 MB", pages, ms / 1000.0);
            QVERIFY2(ms < (what ? 60000 : 120000), qPrintable(QString::number(ms)));
            QVERIFY(pages > 40);
            const QString bad = coverageProblem(pg, fd);
            QVERIFY2(bad.isEmpty(), qPrintable(bad));
            LayoutCache cache;
            for (int k : {1, pages / 2, pages - 1}) {
                const QString differs = materializedDiffers(fd, pg, k, cache);
                QVERIFY2(differs.isEmpty(), qPrintable(differs));
            }
        }
    }

    // Laying out the rest of a paragraph from where it goes on, as a paragraph of its own, gives the pages that laying out the whole of it does: every page
    // starts where it does, and has the lines it has, whatever the paragraph's alignment, indent, and bullet.
    void paginatorRestOfAParagraphIsLaidOutAsTheWholeIs()
    {
        int tails = 0;
        for (int columns : {1, 2})
            for (unsigned seed : {101u, 102u, 103u, 104u, 105u, 106u}) {
                auto make = [&](FlowDocument &fd, int exact) {
                    fd.exactLines = exact;
                    fd.sections[0].columns = columns;
                    fd.sections[0].margins = QMarginsF(60, 66, 60, 70);
                    fillStory(fd.story(), 120, seed, [seed](int i, QTextBlockFormat &bf) {
                        bf.setAlignment(i % 3 ? Qt::AlignJustify : Qt::AlignLeft);
                        if (i % 4 == 0) bf.setTextIndent(24);
                        if (i % 5 == 0) bf.setTopMargin(12);
                        if (seed % 2) bf.setProperty(tp::WidowControl, true);
                        if (i % 11 == 0) bf.setProperty(tp::KeepWithNext, true);
                    }, 20, 200);
                    QTextCursor c(fd.story());
                    c.setPosition(fd.story()->findBlockByNumber(7).position());
                    c.setPosition(fd.story()->findBlockByNumber(30).position(), QTextCursor::KeepAnchor);
                    QTextListFormat lf;
                    lf.setStyle(QTextListFormat::ListDecimal);
                    lf.setIndent(1);
                    c.createList(lf);
                };
                FlowDocument whole, rest;
                make(whole, 100000);   // (the paragraph is laid out whole, always)
                make(rest, 0);         // (never)
                Paginator a(&whole), b(&rest);
                QVERIFY(a.pageCount() > 6);
                const auto sa = startsOf(a), sb = startsOf(b);
                QVERIFY2(sa == sb, qPrintable(QStringLiteral("%1 columns, seed %2: %3 pages and %4").arg(columns).arg(seed).arg(sa.size()).arg(sb.size())));
                for (int k = 0; k < sa.size(); ++k) {
                    const auto la = a.lines(k), lb = b.lines(k);
                    QVERIFY2(la.size() == lb.size(), qPrintable(QStringLiteral("page %1").arg(k)));
                    for (int i = 0; i < la.size(); ++i)
                        QVERIFY2(sameBrief(la[i], lb[i]), qPrintable(QStringLiteral("%1 columns, seed %2, page %3, line %4").arg(columns).arg(seed).arg(k).arg(i)));
                    tails += sa[k].skip > 0;
                }
            }
        QVERIFY(tails > 10);
    }

    // Text so small that a page holds far more than a window of a long paragraph: the window grows until the page, and what the rules need after it,
    // are in it.
    void paginatorGrowsTheWindowForTinyText()
    {
        FlowDocument fd;
        {
            QString text;
            std::mt19937 rng(9);
            while (text.size() < 300 * 1024) text += QStringLiteral("w%1 ").arg(rng() % 900);
            QTextCursor c(fd.story());
            QTextCharFormat cf;
            cf.setFontPointSize(2);
            c.insertText(text, cf);
            c.insertBlock();
            c.insertText(QStringLiteral("and a paragraph after it"), cf);
        }
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY2(pages >= 2 && pages < 30, qPrintable(QString::number(pages)));
        const QString bad = coverageProblem(pg, fd);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
        LayoutCache cache;
        for (int k = 0; k < std::min(pages, 3); ++k) {
            const QString differs = materializedDiffers(fd, pg, k, cache);
            QVERIFY2(differs.isEmpty(), qPrintable(differs));
        }
    }

    // When a later section changes (its paper, margins, columns), the pages of the sections before it are as they were: only the pages from where
    // that section begins are laid out again, and they are what a fresh paginator makes.
    void changingALaterSectionLaysOutOnlyItsPagesAgain()
    {
        FlowDocument fd;
        fillStory(fd.story(), 1600, 52, [](int i, QTextBlockFormat &bf) { if (i == 799) bf.setProperty(flow::fp::SectionBreakAfter, true); }, 3, 6);
        Section second = fd.sections[0];
        fd.sections << second;
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        int inFirst = 0;
        for (int k = 0; k < pages; ++k) inFirst += pg.page(k).start.section == 0;
        QVERIFY(pages > 30 && inFirst > 10 && pages - inFirst > 10);
        const qint64 laid = pg.pagesLaidOut();
        fd.sections[1].paper = QSizeF(300, 400);
        fd.sections[1].margins = QMarginsF(40, 40, 40, 40);
        pg.invalidateFrom(1);
        const int after = pg.pageCount();
        QVERIFY(after > pages);   // (smaller pages)
        const qint64 again = pg.pagesLaidOut() - laid;
        QVERIFY2(again <= after - inFirst + 1, qPrintable(QStringLiteral("%1 pages laid out again, the second section has %2").arg(again).arg(after - inFirst)));
        FlowDocument fresh;
        fillStory(fresh.story(), 1600, 52, [](int i, QTextBlockFormat &bf) { if (i == 799) bf.setProperty(flow::fp::SectionBreakAfter, true); }, 3, 6);
        fresh.sections << fd.sections[1];
        fresh.sections[0] = fd.sections[0];
        Paginator po(&fresh);
        QCOMPARE(po.pageCount(), after);
        for (int k = 0; k < after; ++k) QCOMPARE(pg.page(k).start.pos, po.page(k).start.pos);
    }

    // A section can restart the page numbers; a page's number is found without going back to the section's first page every time.
    void paginatorNumbersPagesBySection()
    {
        FlowDocument fd;
        fillStory(fd.story(), 6000, 93, [](int i, QTextBlockFormat &bf) {
            if (i == 1999 || i == 3999) bf.setProperty(flow::fp::SectionBreakAfter, true);
        }, 3, 6);   // (a line each: some two hundred pages to a section)
        for (Section *s : {&fd.sections[0]}) {
            s->paper = QSizeF(300, 300);
            s->margins = QMarginsF(50, 50, 50, 50);
        }
        Section second, third;
        second = third = fd.sections[0];
        second.firstPageNumber = 1;
        second.pageNumberFormat = QStringLiteral("roman");
        third.firstPageNumber.reset();   // goes on from the second
        fd.sections[0].firstPageNumber = 5;
        fd.sections << second << third;
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages > 12);
        int first2 = -1, first3 = -1;
        for (int k = 0; k < pages; ++k) {
            const int section = pg.page(k).start.section;
            if (section == 1 && first2 < 0) first2 = k;
            if (section == 2 && first3 < 0) first3 = k;
        }
        QVERIFY(first2 > 1 && first3 > first2);
        QCOMPARE(pg.page(0).number, 5);
        QCOMPARE(pg.page(first2 - 1).number, 5 + first2 - 1);
        QCOMPARE(pg.page(first2).number, 1);
        QCOMPARE(pg.page(first3 - 1).number, first3 - first2);
        QCOMPARE(pg.page(first3).number, first3 - first2 + 1);
        QCOMPARE(pg.page(pages - 1).number, pages - first2);
        QCOMPARE(pg.materialize(first2 + 1)->number, 2);
        QCOMPARE(pg.materialize(first2 + 1)->doc->setup.pageNumberFormat, QStringLiteral("roman"));
        // Not a walk back to the section's start for every page.
        QVERIFY2(pg.numberStepsForTest() < 20 * pages, qPrintable(QStringLiteral("%1 steps for %2 pages").arg(pg.numberStepsForTest()).arg(pages)));
        // (And after an edit, which makes some of them again.)
        QTextCursor c(fd.story());
        c.setPosition(10);
        c.insertText(QStringLiteral("a few more words in the first paragraph of the first section, enough to take a line"));
        pg.pageCount();
        QCOMPARE(pg.page(first2).number, 1);
        QVERIFY(pg.numberStepsForTest() < 40 * pages);
    }

    // A section may start its page numbers at 0 (a cover that is not counted); only a section that says nothing goes on from the one before.
    void aSectionCanStartItsPageNumbersAtZero()
    {
        FlowDocument fd;
        fillStory(fd.story(), 5, 14, [](int i, QTextBlockFormat &bf) {
            if (i == 0 || i == 2) bf.setProperty(flow::fp::SectionBreakAfter, true);
            if (i == 1) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
        }, 3, 6);
        Section second, third;
        second = third = fd.sections[0];
        second.firstPageNumber = 0;
        fd.sections << second << third;
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 4);
        QCOMPARE(pg.page(0).number, 1);
        QCOMPARE(pg.page(1).number, 0);
        QCOMPARE(pg.page(2).number, 1);
        QCOMPARE(pg.page(3).number, 2);   // (the third says nothing: it goes on)
        QCOMPARE(pg.materialize(1)->number, 0);
    }

    // A word processing document keeps pieces of its file as text that shows what the field names after "raw:", or nothing.
    void aRawFieldShowsItsOwnText()
    {
        // A word processing document keeps pieces of its file as text that shows what the field names after "raw:" (a footnote's mark, a
        // stand-in), or nothing; the page shows it in the place of the piece, as it does the date a date field has.
        FlowDocument fd;
        QTextCursor c(fd.story());
        QTextCharFormat plain, mark, none;
        plain.setFontPointSize(11);
        mark = plain;
        mark.setProperty(tp::Field, QStringLiteral("raw:[Table: 2 rows]"));
        none = plain;
        none.setProperty(tp::Field, QStringLiteral("raw:"));
        c.insertText(QStringLiteral("Before "), plain);
        c.insertText(QString(QChar::ObjectReplacementCharacter), mark);
        c.insertText(QStringLiteral(" and "), plain);
        c.insertText(QString(QChar::ObjectReplacementCharacter), none);
        c.insertText(QStringLiteral(" after."), plain);
        FrameSpec fs;
        fs.size = QSizeF(400, 100);
        fs.hyphenate = false;
        StoryLayout lay;
        lay.build(fd.story(), {fs}, fd.env);
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 1);
        QCOMPARE(lines[0].text, QStringLiteral("Before [Table: 2 rows] and  after."));
    }

    // Several edits before the pages are laid out again are settled together.
    void paginatorSettlesSeveralEditsAtOnce()
    {
        FlowDocument fd;
        fillStory(fd.story(), 1500, 61, [](int i, QTextBlockFormat &bf) {
            bf.setProperty(tp::WidowControl, true);
            if (i % 15 == 0) bf.setProperty(tp::KeepWithNext, true);
        }, 20, 90);
        Paginator pg(&fd);
        pg.pageCount();
        std::mt19937 rng(8);
        for (int round = 0; round < 6; ++round) {
            for (int e = 0; e < 4; ++e) {
                QTextCursor c(fd.story());
                c.setPosition(int(rng() % unsigned(fd.story()->characterCount() - 1)));
                switch (rng() % 4) {
                case 0: c.insertText(QStringLiteral("more words here")); break;
                case 1: c.insertBlock(); break;
                case 2: c.setPosition(std::min(c.position() + 700, fd.story()->characterCount() - 1), QTextCursor::KeepAnchor); c.removeSelectedText(); break;
                default: c.insertText(QStringLiteral("x")); break;
                }
            }
            if (round % 2) pg.paginate(3);   // (some of it settled, some not)
            Paginator fresh(&fd);
            const int want = fresh.pageCount(), got = pg.pageCount();
            QVERIFY2(want == got, qPrintable(QStringLiteral("round %1: %2 pages, a new paginator finds %3").arg(round).arg(got).arg(want)));
            for (int k = 0; k < want; ++k) QVERIFY2(fresh.page(k).start == pg.page(k).start, qPrintable(QStringLiteral("round %1: page %2").arg(round).arg(k)));
        }
    }

    // ---- the second review: expected answers from first principles (no fresh paginator to compare with: it would have the same faults)

    // Numbers that begin again in each section, and a continuous break in the middle of a page: the notes of one page are in the order of their references, the numbers
    // are those of the references, with a table in the document or none.
    void restartedNotesAreInTheOrderOfTheTextAndNumberedFromTheirSection()
    {
        for (const bool withTable : {false, true}) {
            TestNotes notes;
            addNote(&notes, QStringLiteral("a"), 6, QStringLiteral("NOTEA"));
            addNote(&notes, QStringLiteral("b"), 6, QStringLiteral("NOTEB"));
            addNote(&notes, QStringLiteral("c"), 6, QStringLiteral("NOTEC"));
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.fields = &notes;
            fd.restartNotesInSections = true;
            fillStory(fd.story(), 5, 3, [](int i, QTextBlockFormat &bf) { if (i == 2) bf.setProperty(fp::SectionBreakAfter, true); }, 10, 14);
            if (withTable) {
                TestTable spec;
                spec.rows = 3;
                spec.cols = 2;
                addTable(fd, spec);
                appendParagraph(fd, QStringLiteral("After the table."));
            }
            Section second = fd.sections[0];
            second.continuous = true;
            fd.sections << second;
            insertRefsDescending(fd, {{fd.story()->findBlockByNumber(1).position() + 5, "a"}, {fd.story()->findBlockByNumber(2).position() + 5, "b"}, {fd.story()->findBlockByNumber(3).position() + 5, "c"}});
            Paginator pg(&fd);
            const QString bad = audit::problems(pg, fd.story(), {{"a", "NOTEA"}, {"b", "NOTEB"}, {"c", "NOTEC"}}, true);
            QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("table %1: %2").arg(withTable).arg(bad)));
        }
    }

    // Notes in cells are numbered as the story numbers them: after a paragraph is made the end of a section (with numbers that begin again in each), the notes of the cells after
    // it show the numbers from 1 again, and after it is made an ordinary paragraph again, they go on from the notes before.
    void notesInCellsAreNumberedAgainWhenASectionBreakIsPutInAndTakenOut()
    {
        TestNotes notes;
        for (const char *id : {"a", "b", "c", "d"}) addNote(&notes, QString::fromLatin1(id), 4, QStringLiteral("NOTE%1").arg(QChar(id[0]).toUpper()));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fd.restartNotesInSections = true;
        fillStory(fd.story(), 4, 3, {}, 8, 12);
        TestTable spec;
        spec.rows = 4;
        spec.cols = 2;
        const TestTable table = addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("After the table."));
        insertRefsDescending(fd, {{fd.story()->findBlockByNumber(0).position() + 2, "a"}, {table.rowStarts[1] + 1, "b"}, {table.rowStarts[2] + 1, "c"}, {fd.story()->findBlockByNumber(fd.story()->blockCount() - 1).position() + 2, "d"}});
        Paginator pg(&fd);
        pg.pageCount();
        const QMap<QString, QString> marks{{"a", "NOTEA"}, {"b", "NOTEB"}, {"c", "NOTEC"}, {"d", "NOTED"}};
        QVERIFY2(audit::problems(pg, fd.story(), marks, true).isEmpty(), "as made");
        // A paragraph well before the table ends a section.
        Section second = fd.sections[0];
        second.continuous = true;
        fd.sections.insert(1, second);
        QTextCursor c(fd.story());
        c.setPosition(fd.story()->findBlockByNumber(1).position());
        QTextBlockFormat bf = c.blockFormat();
        bf.setProperty(fp::SectionBreakAfter, true);
        c.setBlockFormat(bf);
        pg.invalidateFrom(0);
        QString bad = audit::problems(pg, fd.story(), marks, true);
        QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("a section break put in: ") + bad));
        bf.clearProperty(fp::SectionBreakAfter);
        c.setBlockFormat(bf);
        fd.sections.remove(1);
        pg.invalidateFrom(0);
        bad = audit::problems(pg, fd.story(), marks, true);
        QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("and taken out: ") + bad));
    }

    // A section that begins in the middle of a page restarts the numbers there, and the page after it goes on from them: the notes of the second section show 1, 2 on every page
    // (the count is not cleared at the top of a page because the section is another than the page before began in).
    void aSectionThatBeginsMidPageRestartsTheNumbersOnceNotAtEachPage()
    {
        for (const bool withTable : {false, true}) {
            TestNotes notes;
            addNote(&notes, QStringLiteral("a"), 3, QStringLiteral("NOTEA"));
            addNote(&notes, QStringLiteral("b"), 3, QStringLiteral("NOTEB"));
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.fields = &notes;
            fd.restartNotesInSections = true;
            fillStory(fd.story(), 1, 3, [](int, QTextBlockFormat &bf) { bf.setProperty(fp::SectionBreakAfter, true); }, 6, 8);
            for (int i = 0; i < 60; ++i) appendParagraph(fd, QStringLiteral("Paragraph %1 of the second section.").arg(i));
            if (withTable) {
                TestTable spec;
                spec.rows = 1;
                spec.cols = 1;
                addTable(fd, spec);
                appendParagraph(fd, QStringLiteral("tail"));
            }
            Section second = fd.sections[0];
            second.continuous = true;
            fd.sections << second;
            insertRefsDescending(fd, {{fd.story()->findBlockByNumber(58).position() + 2, "b"}, {fd.story()->findBlockByNumber(2).position() + 2, "a"}});
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() >= 2);
            const QString bad = audit::problems(pg, fd.story(), {{"a", "NOTEA"}, {"b", "NOTEB"}}, true);
            QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("table %1: %2").arg(withTable).arg(bad)));
        }
    }

    // A continuous section with other columns, on a page with a note cited in the section before: the text of the first section stays in its width, the notes are at the
    // foot of the page and the columns end above them, nothing is on no page, and no page is empty.
    void aSectionWithOtherColumnsBeginsAboveTheNotesAtThePageFoot()
    {
        auto one = [](int noteWords, int citeIn, int sec1Paras, double pageH, bool table, QString *what) {
            TestNotes notes;
            addNote(&notes, QStringLiteral("a"), noteWords, QStringLiteral("NOTEA"));
            addNote(&notes, QStringLiteral("b"), 6, QStringLiteral("NOTEB"));
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.fields = &notes;
            fd.sections[0].paper = QSizeF(300, pageH);
            fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
            fillStory(fd.story(), sec1Paras, 3, [&](int i, QTextBlockFormat &bf) { if (i == sec1Paras - 1) bf.setProperty(fp::SectionBreakAfter, true); }, 6, 20);
            for (int i = 0; i < 14; ++i) appendParagraph(fd, QStringLiteral("Second section paragraph number %1 with several words in it to fill the columns for a while longer.").arg(i));
            if (table) {
                TestTable spec;
                spec.rows = 4;
                spec.cols = 2;
                spec.columnWidth = 40;
                addTable(fd, spec);
                appendParagraph(fd, QStringLiteral("After."));
            }
            Section second = fd.sections[0];
            second.continuous = true;
            second.columns = 2;
            second.columnGap = 20;
            fd.sections << second;
            // (A note cited in the first paragraph of the second section too: on the page that shows that line, which is the next when the columns have no room.)
            insertRefsDescending(fd, {{fd.story()->findBlockByNumber(std::min(citeIn, sec1Paras - 1)).position() + 3, "a"}, {fd.story()->findBlockByNumber(sec1Paras).position() + 3, "b"}});
            Paginator pg(&fd);
            QString res;
            const int firstSecEnd = fd.story()->findBlockByNumber(sec1Paras).position();
            for (int k = 0; k < pg.pageCount(); ++k) {
                const auto lines = pg.lines(k);
                if (lines.isEmpty() && !pg.materialize(k)) res += QStringLiteral(" page %1 is empty;").arg(k);
                for (const auto &l : lines)
                    if (l.pos < firstSecEnd && l.rect.width() < 200) {
                        res += QStringLiteral(" text of the first section in a narrow column on page %1 (%2 wide);").arg(k).arg(l.rect.width());
                        break;
                    }
            }
            if (const QString cov = coverageProblem(pg, fd, true, true); !cov.isEmpty()) res += QStringLiteral(" text lost: ") + cov;
            if (const QString bad = audit::problems(pg, fd.story(), {{"a", "NOTEA"}, {"b", "NOTEB"}}); !bad.isEmpty()) res += QStringLiteral(" ") + bad;
            if (what) *what = QStringLiteral("%1 words, cited in block %2, %3 paragraphs, page %4 high, table %5").arg(noteWords).arg(citeIn).arg(sec1Paras).arg(pageH).arg(table);
            return res;
        };
        int bad = 0, total = 0;
        QStringList samples;
        for (int words : {5, 20, 40, 80, 150})
            for (int cite : {0, 1, 2, 3})
                for (int paras : {2, 4})
                    for (double ph : {260.0, 330.0, 420.0})
                        for (bool tb : {false, true}) {
                            ++total;
                            QString what;
                            const QString r = one(words, cite, paras, ph, tb, &what);
                            if (!r.isEmpty()) {
                                ++bad;
                                if (samples.size() < 4) samples << what + QStringLiteral(": ") + r;
                            }
                        }
        QVERIFY2(bad == 0, qPrintable(QStringLiteral("%1 of %2 configurations are wrong; for example %3").arg(bad).arg(total).arg(samples.join(QLatin1Char('\n')))));
    }

    // Two notes cited on the last line of a page (the text goes on after it, with a table before or none): both are drawn in all their height, wherever the line lands.
    void twoNotesOnThePagesLastLineAreBothDrawn()
    {
        int checked = 0;
        for (const int words1 : {15, 30, 60})
            for (int paras = 10; paras <= 50; paras += 2)
                for (const bool withTable : {false, true}) {
                    TestNotes notes;
                    addNote(&notes, QStringLiteral("a"), words1, QStringLiteral("NOTEA"));
                    addNote(&notes, QStringLiteral("b"), 3, QStringLiteral("NOTEB"));
                    FlowDocument fd;
                    fd.env.wordLines = true;
                    fd.fields = &notes;
                    fillStory(fd.story(), paras, 3, {}, 3, 5);
                    if (withTable) {
                        TestTable spec;
                        spec.rows = 2;
                        spec.cols = 1;
                        addTable(fd, spec);
                    }
                    for (int i = 0; i < 40; ++i) appendParagraph(fd, QStringLiteral("Trailing paragraph %1.").arg(i));
                    int at = -1;
                    {
                        Paginator first(&fd);
                        const auto lines = first.lines(0);
                        if (lines.isEmpty() || fd.story()->findBlock(lines.last().block).blockFormat().intProperty(fp::Cell)) continue;   // (the page ends in a row)
                        at = lines.last().pos + 1;
                    }
                    insertRefsDescending(fd, {{at + 1, "b"}, {at, "a"}});
                    Paginator pg(&fd);
                    ++checked;
                    const double full = audit::fullNoteHeight(notes.docs["a"].get(), 468, fd.env) + audit::fullNoteHeight(notes.docs["b"].get(), 468, fd.env);
                    const QString lost = audit::lostNotes(pg, full);
                    QVERIFY2(lost.isEmpty(), qPrintable(QStringLiteral("%1 words, %2 paragraphs, table %3: %4").arg(words1).arg(paras).arg(withTable).arg(lost)));
                    const QString bad = audit::problems(pg, fd.story(), {{"a", "NOTEA"}, {"b", "NOTEB"}});
                    QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1 words, %2 paragraphs, table %3: %4").arg(words1).arg(paras).arg(withTable).arg(bad)));
                }
        QVERIFY(checked > 40);
    }

    // One cell of text that goes over a page break, a note cited in it (of every length), the line it is cited on at the top, the middle, or later in the cell: the note is below
    // its line, on its page, over no line of the text and no row.
    void aNoteCitedInACellThatGoesOverAPageBreakOverlapsNothing()
    {
        int checked = 0;
        for (const int words : {60, 100, 150, 200, 300, 400})
            for (const int cellLines : {4, 8, 12, 20, 30})
                for (const int refLine : {0, 1, 3}) {
                    TestNotes notes;
                    addNote(&notes, QStringLiteral("n"), words, QStringLiteral("NOTEN"));
                    FlowDocument fd;
                    fd.env.wordLines = true;
                    fd.fields = &notes;
                    fd.sections[0].paper = QSizeF(320, 330);
                    fd.sections[0].margins = QMarginsF(36, 36, 36, 36);
                    fillStory(fd.story(), 1, 3, {}, 6, 10);
                    TestTable spec;
                    spec.rows = 1;
                    spec.cols = 1;
                    spec.columnWidth = 200;
                    spec.text = [&](int, int) {
                        QStringList l;
                        for (int i = 0; i < cellLines; ++i) l << QStringLiteral("cell line number %1").arg(i);
                        return l.join(QLatin1Char('\n'));
                    };
                    const TestTable t = addTable(fd, spec);
                    appendParagraph(fd, QStringLiteral("after"));
                    QTextBlock b = fd.story()->findBlock(t.rowStarts[0]);
                    for (int i = 0; i < refLine && b.next().isValid(); ++i) b = b.next();
                    addReference(fd.story(), b.position() + 2, QStringLiteral("n"));
                    Paginator pg(&fd);
                    ++checked;
                    const QString bad = audit::problems(pg, fd.story(), {{"n", "NOTEN"}});
                    QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("a note of %1 words, a cell of %2 lines, cited on line %3: %4").arg(words).arg(cellLines).arg(refLine).arg(bad)));
                    const QString lost = audit::lostNotes(pg, audit::fullNoteHeight(notes.docs["n"].get(), pg.page(0).content.width(), fd.env));
                    QVERIFY2(lost.isEmpty(), qPrintable(QStringLiteral("a note of %1 words, a cell of %2 lines, cited on line %3: %4").arg(words).arg(cellLines).arg(refLine).arg(lost)));
                }
        QVERIFY(checked == 90);
    }

    // A note longer than the pages, cited in the last paragraph, in a cell of the last table, or before a table: all of it is drawn (the heights of its boxes make its height).
    void theEndOfALongNoteIsDrawnWhereverItIsCited()
    {
        for (const int where : {0, 1, 2, 3})
            for (const bool textAfter : {false, true}) {
                if (where != 1 && textAfter) continue;
                TestNotes notes;
                addNote(&notes, QStringLiteral("long"), 1500, QStringLiteral("LONGNOTE"));
                FlowDocument fd;
                fd.env.wordLines = true;
                fd.fields = &notes;
                fillStory(fd.story(), 3, 3, {}, 10, 14);
                int refAt = fd.story()->findBlockByNumber(2).position() + 3;
                if (where == 1) {
                    TestTable spec;
                    spec.rows = 3;
                    spec.cols = 1;
                    const TestTable t = addTable(fd, spec);
                    refAt = t.rowStarts[1] + 1;
                    if (textAfter) appendParagraph(fd, QStringLiteral("zebra after"));
                }
                if (where == 2) {
                    TestTable spec;
                    spec.rows = 3;
                    spec.cols = 1;
                    addTable(fd, spec);
                }
                if (where == 3) {
                    TestTable spec;
                    spec.rows = 2;
                    spec.cols = 1;
                    addTable(fd, spec);
                    for (int i = 0; i < 3; ++i) appendParagraph(fd, QStringLiteral("Paragraph after the table number %1.").arg(i));
                    refAt = fd.story()->findBlockByNumber(fd.story()->blockCount() - 1).position() + 3;
                }
                insertRefsDescending(fd, {{refAt, "long"}});
                Paginator pg(&fd);
                const QString lost = audit::lostNotes(pg, audit::fullNoteHeight(notes.docs["long"].get(), 468, fd.env));
                QVERIFY2(lost.isEmpty(), qPrintable(QStringLiteral("cited %1, text after %2: %3").arg(where).arg(textAfter).arg(lost)));
                const QString bad = audit::problems(pg, fd.story(), {{"long", "LONGNOTE"}});
                QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("cited %1, text after %2: %3").arg(where).arg(textAfter).arg(bad)));
            }
    }

    // A note cited on the last line of the last page (as long as a few lines, or a page), in a document with a table before it or none: nothing of it is lost.
    void aNoteCitedInTheLastParagraphIsDrawnWholeWhateverThePageFills()
    {
        int checked = 0;
        for (const int words : {10, 30, 60})
            for (int paras = 3; paras <= 80; paras += 1)
                for (const bool withTable : {false, true}) {
                    TestNotes notes;
                    addNote(&notes, QStringLiteral("n"), words, QStringLiteral("NOTEN"));
                    FlowDocument fd;
                    fd.env.wordLines = true;
                    fd.fields = &notes;
                    fillStory(fd.story(), paras, 3, {}, 3, 5);
                    if (withTable) {
                        TestTable spec;
                        spec.rows = 1;
                        spec.cols = 1;
                        addTable(fd, spec);
                        appendParagraph(fd, QStringLiteral("Last one."));
                    }
                    addReference(fd.story(), fd.story()->findBlockByNumber(fd.story()->blockCount() - 1).position() + 2, QStringLiteral("n"));
                    Paginator pg(&fd);
                    ++checked;
                    const QString lost = audit::lostNotes(pg, audit::fullNoteHeight(notes.docs["n"].get(), 468, fd.env));
                    QVERIFY2(lost.isEmpty(), qPrintable(QStringLiteral("%1 words, %2 paragraphs, table %3: %4").arg(words).arg(paras).arg(withTable).arg(lost)));
                    const QString bad = audit::problems(pg, fd.story(), {{"n", "NOTEN"}});
                    QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1 words, %2 paragraphs, table %3: %4").arg(words).arg(paras).arg(withTable).arg(bad)));
                }
        QVERIFY(checked > 400);
    }

    // A cell of many paragraphs that each cite notes: the parts of the row use the page (the lines and their notes fill it), not a line to a page.
    void thePartsOfARowWithNotesFillThePages()
    {
        for (const int perLine : {1, 3}) {
            TestNotes notes;
            for (int i = 0; i < 400; ++i) addNote(&notes, QStringLiteral("n%1").arg(i), 3, QStringLiteral("NOTE%1X").arg(i));
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.fields = &notes;
            fillStory(fd.story(), 1, 3, {}, 6, 10);
            TestTable spec;
            spec.rows = 1;
            spec.cols = 1;
            spec.columnWidth = 300;
            spec.text = [](int, int) {
                QStringList l;
                for (int i = 0; i < 120; ++i) l << QStringLiteral("cell paragraph number %1").arg(i);
                return l.join(QLatin1Char('\n'));
            };
            const TestTable t = addTable(fd, spec);
            appendParagraph(fd, QStringLiteral("after"));
            QVector<QPair<int, QString>> refs;
            int id = 0;
            for (QTextBlock b = fd.story()->findBlock(t.rowStarts[0]); b.isValid() && b.blockFormat().intProperty(fp::Cell); b = b.next())
                if (b.blockNumber() % 2 == 0)
                    for (int k = 0; k < perLine; ++k) refs.push_back({b.position() + 3, QStringLiteral("n%1").arg(id++ % 400)});
            insertRefsDescending(fd, refs);
            Paginator pg(&fd);
            const int pages = pg.pageCount();
            // The lines (13.43 high at 11 points) and the notes (a line each, 13.43) of the 60 paragraphs that cite: 60 * perLine notes, 120 lines.
            const double need = 120 * 13.43 + 60 * perLine * 13.43;
            const double area = pg.page(0).content.height();
            QVERIFY2(pages <= int(need / (0.7 * area)) + 2, qPrintable(QStringLiteral("%1 notes to a line: %2 pages for about %3 points of lines and notes in pages of %4").arg(perLine).arg(pages).arg(need).arg(area)));
        }
    }

    // Rows that are set to be at least 90 high are that high wherever they are on a page, also where one ends a page.
    void rowsHoldTheirSetHeightAtThePageFoot()
    {
        int checked = 0;
        for (int paras = 20; paras <= 40; ++paras) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.sections[0].paper = QSizeF(320, 330);
            fd.sections[0].margins = QMarginsF(36, 36, 36, 36);
            fillStory(fd.story(), paras, 3, {}, 3, 4);
            TestTable spec;
            spec.rows = 6;
            spec.cols = 1;
            spec.columnWidth = 200;
            spec.setRow = [](int, RowData &rd) { rd.height = 90; };
            addTable(fd, spec);
            appendParagraph(fd, QStringLiteral("after"));
            Paginator pg(&fd);
            for (int k = 0; k < pg.pageCount(); ++k) {
                const auto m = pg.materialize(k);
                for (const auto &t : m->tables)
                    for (int r = t->firstRow; r < t->endRow; ++r) {
                        ++checked;
                        // (A part of a row that goes on over the page break is as tall as the room it has.)
                        const bool cutHere = k + 1 < pg.pageCount() && pg.page(k + 1).start.piece != 0 && r + 1 == t->endRow;
                        if (!cutHere)
                            QVERIFY2(t->heightOf(r) > 90 - 0.5, qPrintable(QStringLiteral("%1 paragraphs, page %2: row %3 is %4 high, set at least 90").arg(paras).arg(k).arg(r).arg(t->heightOf(r))));
                    }
            }
        }
        QVERIFY(checked > 100);
    }

    // A row that cannot be split, citing N notes, costs N, not N squared: twice the notes take about twice as long.
    void aRowThatCannotBeSplitCitingManyNotesTakesTimeInStepWithThem()
    {
        auto build = [](int refs) {
            return [refs](FlowDocument &fd, TestNotes &notes) {
                for (int i = 0; i < 50; ++i) addNote(&notes, QStringLiteral("n%1").arg(i), 3, QStringLiteral("NOTE%1").arg(i));
                fillStory(fd.story(), 2, 3, {}, 10, 14);
                TestTable spec;
                spec.rows = 1;
                spec.cols = 1;
                spec.columnWidth = 200;
                spec.setRow = [](int, RowData &r) { r.cantSplit = true; };
                const TestTable table = addTable(fd, spec);
                appendParagraph(fd, QStringLiteral("After the table."));
                const int at = table.rowStarts[0] + 1;
                for (int k = refs - 1; k >= 0; --k) addReference(fd.story(), at, QStringLiteral("n%1").arg(k % 50));
            };
        };
        const double small = millisecondsToPaginate(build(500), 3, true), big = millisecondsToPaginate(build(2000), 3, true);
        // Four times the notes: linear work takes about 4x, the quadratic carry this guards against about 16x; 12x leaves room for a slow CI machine.
        QVERIFY2(big < 12 * std::max(small, 20.0), qPrintable(QStringLiteral("500 notes took %1 ms, 2000 notes %2 ms").arg(small).arg(big)));
    }

    // Tables of rows that each cite many notes: the work is bounded for the document, not for each row (twice the rows take about twice as long, and a hostile file is not
    // made to take minutes).
    void manyRowsCitingManyNotesTakeTimeInStepWithTheirNumber()
    {
        auto build = [](int rows) {
            return [rows](FlowDocument &fd, TestNotes &notes) {
                for (int i = 0; i < 50; ++i) addNote(&notes, QStringLiteral("n%1").arg(i), 3, QStringLiteral("NOTE%1").arg(i));
                fillStory(fd.story(), 2, 3, {}, 10, 14);
                TestTable spec;
                spec.rows = rows;
                spec.cols = 1;
                spec.columnWidth = 200;
                const TestTable table = addTable(fd, spec);
                appendParagraph(fd, QStringLiteral("After the table."));
                for (int r = rows - 1; r >= 0; --r)
                    for (int k = 999; k >= 0; --k) addReference(fd.story(), table.rowStarts[r] + 1, QStringLiteral("n%1").arg((r * 7 + k) % 50));
            };
        };
        const double small = millisecondsToPaginate(build(4), 3, true), big = millisecondsToPaginate(build(16), 3, true);
        QVERIFY2(big < 8 * std::max(small, 50.0), qPrintable(QStringLiteral("4 rows of 1000 notes took %1 ms, 16 rows %2 ms").arg(small).arg(big)));
    }

    // A long note cited in a cell of a table in a cell: the time is in step with its length.
    void aLongNoteInANestedCellTakesTimeInStepWithItsLength()
    {
        auto build = [](int words) {
            return [words](FlowDocument &fd, TestNotes &notes) {
                addNote(&notes, QStringLiteral("long"), words, QStringLiteral("LONGNOTE"));
                fillStory(fd.story(), 2, 3, {}, 10, 14);
                QTextCursor c(fd.story());
                c.movePosition(QTextCursor::End);
                QTextCharFormat body;
                body.setFontPointSize(11);
                auto outerTable = std::make_shared<TableData>();
                outerTable->columns << 300;
                outerTable->margins = QMarginsF(4, 3, 4, 3);
                auto outerRow = std::make_shared<RowData>();
                CellRecord outer;
                outer.table = outerTable;
                outer.row = outerRow;
                const int outerKey = fd.tables.add(outer);
                auto paragraph = [&](int key, const QString &text) {
                    QTextBlockFormat bf;
                    bf.setProperty(fp::Cell, key);
                    c.insertBlock(bf, body);
                    c.insertText(text, body);
                };
                paragraph(outerKey, QStringLiteral("before"));
                auto inner = std::make_shared<TableData>();
                inner->columns << 140 << 140;
                inner->margins = QMarginsF(2, 2, 2, 2);
                int refAt = 0;
                for (int r = 0; r < 2; ++r) {
                    auto row = std::make_shared<RowData>();
                    for (int col = 0; col < 2; ++col) {
                        CellRecord rec;
                        rec.table = inner;
                        rec.row = row;
                        rec.parent = outerKey;
                        const int key = fd.tables.add(rec);
                        paragraph(key, QStringLiteral("r%1c%2").arg(r).arg(col));
                        if (r == 1 && col == 1) refAt = c.block().position() + 2;
                    }
                }
                paragraph(outerKey, QStringLiteral("after"));
                appendParagraph(fd, QStringLiteral("zebra after"));
                addReference(fd.story(), refAt, QStringLiteral("long"));
            };
        };
        const double small = millisecondsToPaginate(build(5000), 3, true), big = millisecondsToPaginate(build(20000), 3, true);
        QVERIFY2(big < 8 * std::max(small, 100.0), qPrintable(QStringLiteral("5000 words took %1 ms, 20000 words %2 ms").arg(small).arg(big)));
    }

    // Objects anchored in the paragraphs of cells: making a page costs the paragraphs and the anchors, not their product.
    void anchorsInTheCellsOfATableCostInStepWithTheirNumber()
    {
        auto build = [](int cols) {
            return [cols](FlowDocument &fd, TestNotes &) {
                fillStory(fd.story(), 1, 3, {}, 6, 10);
                auto item = std::make_shared<ShapeItem>();
                item->rect = QRectF(0, 0, 3, 3);
                TestTable spec;
                spec.rows = 1;
                spec.cols = cols;
                spec.columnWidth = 400.0 / cols;
                spec.text = [](int, int) {
                    QStringList l;
                    for (int i = 0; i < 400; ++i) l << QStringLiteral("x");
                    return l.join(QLatin1Char('\n'));
                };
                const TestTable t = addTable(fd, spec);
                FloatingObject o;
                o.item = item;
                fd.objects.push_back(o);
                const int id = int(fd.objects.size());
                for (QTextBlock b = fd.story()->lastBlock(); b.isValid() && b.position() >= t.rowStarts[0]; b = b.previous()) {
                    QTextCursor cur(fd.story());
                    cur.setPosition(b.position());
                    QTextCharFormat anchor;
                    anchor.setProperty(fp::AnchorId, id);
                    cur.insertText(QString(QChar(0xFFFC)), anchor);
                }
            };
        };
        const double small = millisecondsToPaginate(build(5), 3, true), big = millisecondsToPaginate(build(20), 3, true);
        QVERIFY2(big < 8 * std::max(small, 100.0), qPrintable(QStringLiteral("5 cells took %1 ms, 20 cells %2 ms").arg(small).arg(big)));
    }

    // The heights of notes belong to the environment they were measured in: after the text is made another size (and the pages are laid out again), the notes are the heights a
    // paginator made afresh finds.
    void aChangeOfTheTextSizeChangesTheHeightsOfNotesBesideTables()
    {
        TestNotes notes;
        addNote(&notes, QStringLiteral("n"), 40, QStringLiteral("NOTEN"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fillStory(fd.story(), 2, 3, {}, 6, 10);
        TestTable spec;
        spec.rows = 4;
        spec.cols = 1;
        const TestTable t = addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("after"));
        addReference(fd.story(), t.rowStarts[1] + 1, QStringLiteral("n"));
        Paginator pg(&fd);
        const double before = audit::drawnNoteHeight(pg);
        fd.env.fontScale = 1.6;
        pg.invalidate();
        const double after = audit::drawnNoteHeight(pg);
        Paginator fresh(&fd);
        QVERIFY2(std::abs(after - audit::drawnNoteHeight(fresh)) < 0.01, qPrintable(QStringLiteral("the note is %1 high before, %2 after the change, %3 for a new paginator").arg(before).arg(after).arg(audit::drawnNoteHeight(fresh))));
        QVERIFY(after > before + 1);
    }

    // Deleting a character in a document of many paragraphs with a table in it costs about what typing one does: it does not find the tables again.
    void deletingACharacterCostsLikeTypingOne()
    {
        double perDelete[2] = {0, 0}, perType[2] = {0, 0};
        int n = 0;
        for (const int paras : {8000, 32000}) {
            TestNotes notes;
            addNote(&notes, QStringLiteral("n"), 5, QStringLiteral("NOTEN"));
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.fields = &notes;
            fillStory(fd.story(), paras, 5, {}, 5, 9);
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            addTableAfter(fd, paras / 2, spec);
            fd.story()->setUndoRedoEnabled(false);
            Paginator pg(&fd);
            pg.pageCount();
            QTextCursor cur(fd.story());
            const int at = fd.story()->findBlockByNumber(paras / 4).position() + 5;
            const int reps = 20;
            // Each run types (or deletes) the block of letters; the best run of the five is the time per letter.
            const auto typeBlock = [&] {
                for (int i = 0; i < reps; ++i) {
                    cur.setPosition(at);
                    cur.insertText(QStringLiteral("x"));
                    pg.paginate(paras / 100);
                }
            };
            const auto deleteBlock = [&] {
                for (int i = 0; i < reps; ++i) {
                    cur.setPosition(at + 1);
                    cur.deleteChar();
                    pg.paginate(paras / 100);
                }
            };
            perType[n] = double(bestNanoseconds(typeBlock)) / 1e6 / reps;
            perDelete[n] = double(bestNanoseconds(deleteBlock)) / 1e6 / reps;
            ++n;
        }
        QVERIFY2(perDelete[1] < 3 * perType[1] + 3, qPrintable(QStringLiteral("typing takes %1 ms, deleting %2 ms (32000 paragraphs)").arg(perType[1]).arg(perDelete[1])));
        QVERIFY2(perDelete[1] < 2.5 * perDelete[0] + 3, qPrintable(QStringLiteral("deleting takes %1 ms at 8000 paragraphs and %2 ms at 32000 (typing %3 and %4)").arg(perDelete[0]).arg(perDelete[1]).arg(perType[0]).arg(perType[1])));
    }
};

QTEST_MAIN(FlowTests)
#include "tst_flow.moc"
