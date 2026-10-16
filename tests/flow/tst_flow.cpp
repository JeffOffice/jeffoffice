// Tests of libs/flow, and of the changes below it that the flowing document needs: the
// lookup of items by id in the document, the layout cache's key, and the additions to the
// text engine that the paginator uses.

#include "core/document.h"
#include "flow/flowdocument.h"
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

} // namespace

class FlowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
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
};

QTEST_MAIN(FlowTests)
#include "tst_flow.moc"
