// Tests of libs/flow, and of the changes below it that the flowing document needs: the
// lookup of items by id in the document, the layout cache's key, and the additions to the
// text engine that the paginator uses.

#include "core/document.h"
#include "core/fonts.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "render/renderer.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QImage>
#include <QPainter>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <functional>
#include <random>

using namespace jp;
using jp::flow::FlowDocument;
using jp::flow::MaterializedPage;
using jp::flow::Paginator;
using jp::flow::Section;

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
    return a.frame == b.frame + frameShift && a.block == b.block + posShift && a.line == b.line && a.lines == b.lines && a.pos == b.pos + posShift &&
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
QString coverageProblem(Paginator &pg, FlowDocument &fd, bool allowBlankLast = false)
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

} // namespace

class FlowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        initCore();   // (the program's own fonts, as the other tests' programs have them)
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
        QVERIFY(lines.size() > 20 && lines.size() < 120);
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

    // A footnote cut at the foot of a page goes on at the foot of the next, whether the next page is laid out with the
    // first or by itself with the note given as carried over.
    void spanCarriesAFootnoteOverFromThePageBefore()
    {
        int compared = 0;
        for (double height : {110.0, 125.0, 140.0, 155.0, 170.0, 185.0})
            for (unsigned seed : {9u, 10u, 11u}) {
                bool cut = false;
                const QString bad = carriedFootnoteCheck(height, seed, &cut);
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
        QVERIFY2(firstMs < 3000, qPrintable(QString::number(firstMs)));
        QVERIFY2(allMs < 50000, qPrintable(QString::number(allMs)));
        QVERIFY2(worst < 500, qPrintable(QString::number(worst)));
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
        Paginator pg(&fd);
        QElapsedTimer t;
        t.start();
        const int pages = pg.pageCount();
        qInfo("a paragraph of 900 KB: %d pages in %.1f s", pages, t.elapsed() / 1000.0);
        QVERIFY(pages > 150);
        QVERIFY2(t.elapsed() < 60000, qPrintable(QString::number(t.elapsed())));
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
            Paginator pg(&fd);
            QElapsedTimer t;
            t.start();
            const int pages = pg.pageCount();
            qInfo("%s: %d pages in %.1f s", what ? "a word of 200 KB" : "a paragraph of 2 MB", pages, t.elapsed() / 1000.0);
            QVERIFY2(t.elapsed() < (what ? 60000 : 120000), qPrintable(QString::number(t.elapsed())));
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

    // Several edits before the pages are laid out again are settled together.
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
};

QTEST_MAIN(FlowTests)
#include "tst_flow.moc"
