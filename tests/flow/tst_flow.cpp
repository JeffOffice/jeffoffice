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
};

QTEST_MAIN(FlowTests)
#include "tst_flow.moc"
