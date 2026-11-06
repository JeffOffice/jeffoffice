#include "tst_flow.h"

void FlowTests::initTestCase()
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
void FlowTests::sectionsKnowTheirGeometry()
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
void FlowTests::documentFindAgreesWithScanAfterRandomEdits()
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
void FlowTests::documentFindDoesNotScanOnceIndexed()
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
void FlowTests::documentFindRepairsItemsThatShiftedWithoutScanning()
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
void FlowTests::documentFindReturnsTheFirstOfTwoItemsWithOneId()
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
void FlowTests::layoutDoesNotFollowTheClockWithoutAClockField()
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


void FlowTests::layoutFollowsTheClockWithADateOrTimeField()
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
void FlowTests::layoutFollowsTheClockWithADateInANote()
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
void FlowTests::layoutFollowsWhatItsFieldsShow()
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
void FlowTests::layoutIsKeptWhileADateShowsTheSameDay()
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
void FlowTests::lineInfoGivesDocumentPositions()
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
void FlowTests::spanFromAParagraphIsTheSameAsAStoryFromThere()
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
void FlowTests::spanContinuesAParagraphFromTheFrameBefore()
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
void FlowTests::spanStopsAtAFrame()
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
void FlowTests::spanContinuesAParagraphWithoutItsEarlierLines()
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
void FlowTests::spanWindowCutsAHugeParagraph()
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
void FlowTests::spanWithoutRulesIgnoresKeepsAndWidows()
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
void FlowTests::spanCanLeaveOutTheSpaceBeforeTheFirstParagraph()
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
void FlowTests::spanEndsBeforeTheParagraphAtItsEnd()
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
void FlowTests::spanCarriesAFootnoteOverFromThePageBefore()
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
void FlowTests::paginatorMatchesTheOneShotLayout()
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
void FlowTests::paginatorHandlesTinyDocuments()
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
void FlowTests::paginatorKeepsWidowsAndOrphans()
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
void FlowTests::paginatorKeepsMoreLinesWhenAsked()
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
void FlowTests::paginatorKeepsParagraphsWithTheNext()
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
void FlowTests::paginatorBreaksPagesWhereAsked()
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
void FlowTests::aPageBreakCharacterEndsThePageAfterItsLine()
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
void FlowTests::aBreakAtTheStartOfAParagraphIsNotAnOrphan()
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


// A word processor lays out in twentieths of a point: a line that goes over the foot of the page by less than one of them is on the page. Text that is not a word
// processor's (JeffPub's and JeffDeck's frames, and the fit of text to a box) keeps a hundredth.
void FlowTests::aLineOverTheFootByLessThanATwentiethOfAPointFits()
{
    auto lay = [&](double room, bool word) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        for (int i = 0; i < 5; ++i) {
            if (i == 0) c.setBlockFormat(bf); else c.insertBlock(bf);
            c.insertText(QStringLiteral("line %1").arg(i), run);
        }
        LayoutEnv env;
        env.wordLines = word;
        StoryLayout l;
        l.build(&doc, {plainFrame(300, room), plainFrame(300, 500)}, env);
        return l.lineBriefs();
    };
    for (const bool word : {true, false}) {
        // The foot the third line's text reaches (its bottom without the spacing under it), from a layout with room to spare; and the least room that has it on the first page.
        const auto whole = lay(1000, word);
        QVERIFY(whole.size() >= 3);
        const double bottom = whole[2].rect.bottom() - whole[2].below;
        auto onFirst = [&](double room) {
            int n = 0;
            for (const auto &line : lay(room, word)) n += line.frame == 0;
            return n;
        };
        QCOMPARE(onFirst(bottom + 0.1), 3);
        double low = bottom - 1, high = bottom + 0.1;   // (the least room that holds three lines is in between)
        for (int i = 0; i < 30; ++i) {
            const double mid = (low + high) / 2;
            (onFirst(mid) >= 3 ? high : low) = mid;
        }
        const double tolerance = bottom - high;
        const double want = word ? 0.05 : 0.01;
        QVERIFY2(std::abs(tolerance - want) < 0.002, qPrintable(QStringLiteral("%1 text goes over the foot by %2 and is on the page, not %3").arg(word ? "word processor's" : "other").arg(tolerance).arg(want)));
    }
}


// A paragraph that begins with a page break and goes on for more lines than a page's window past its foot is no page of its own for the break: the break's line
// ends the page it follows on (as for a paragraph that is short), and the text goes on at the top of the next.
void FlowTests::aLongParagraphThatBeginsWithABreakBreaksTheWholePageBefore()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    QTextCursor c(fd.story());
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    c.insertText(QStringLiteral("before the break"), run);
    c.insertBlock();
    QString words;
    for (int i = 0; i < 1200; ++i) words += QStringLiteral("word%1 ").arg(i % 97);
    c.insertText(QString(flow::kPageBreak) + words, run);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    const auto first = pg.lines(0);
    QCOMPARE(first.size(), 2);                  // the line before, and the break's
    QCOMPARE(first[1].end - first[1].pos, 1);
    QCOMPARE(pg.page(1).start.skip, 1);         // the text goes on at the paragraph's next line
    QVERIFY(pg.lines(1).size() > 5);
}


// A page break that ends a line is not made one of an orphan by what shows nothing between the line's last letter and it (an object that stands where it is anchored, and takes
// no room): the paragraph's first line stays at the foot of its page, and the rest goes on at the next.
void FlowTests::aBreakAfterWhatShowsNothingIsNotAnOrphan()
{
    const double single = (1825 + 443 + 87) / 2048.0 * 12;
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
    c.insertText(QStringLiteral("before"), run);
    QTextCharFormat nothing = run;
    nothing.setProperty(tp::Field, QStringLiteral("raw:"));
    c.insertText(QString(QChar::ObjectReplacementCharacter), nothing);
    c.insertText(QString(flow::kPageBreak) + QStringLiteral("after the break"), run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(300, 5 * single + 3), plainFrame(300, 500)}, env);
    const auto lines = lay.lineBriefs();
    QCOMPARE(lines.size(), 6);
    QCOMPARE(lines[4].frame, 0);   // the line with the break stays where it is
    QCOMPARE(lines[5].frame, 1);
}


// A file whose options say to suppress the space before after a page break (the 2003 program's) has none at the top of the page a break at the start of a paragraph opens:
// its text begins at the page's top.
void FlowTests::aFileThatSuppressesTheSpaceBeforeAfterAPageBreakHasNone()
{
    auto secondPageTop = [&](bool suppress) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.spaceBeforeAfterBreak = !suppress;
        QTextCursor c(fd.story());
        QTextBlockFormat bf;
        bf.setTopMargin(20);
        c.setBlockFormat(bf);
        c.insertText(QString(flow::kPageBreak) + QStringLiteral("second words"));
        c.insertBlock(bf);
        c.insertText(QStringLiteral("a paragraph after"));
        Paginator pg(&fd);
        return pg.pageCount() == 2 ? pg.lines(1)[0].rect.top() : -1.0;
    };
    QVERIFY2(std::abs(secondPageTop(false) - 20) < 0.01, qPrintable(QString::number(secondPageTop(false))));
    QVERIFY2(std::abs(secondPageTop(true)) < 0.01, qPrintable(QString::number(secondPageTop(true))));
}


// A file that suppresses the space before after a page break suppresses only that: the text after a break at the start of a paragraph is still the paragraph's first line, with
// its first-line indent, as the page the engine laid out and the one made for painting have it.
void FlowTests::aFileThatSuppressesTheSpaceBeforeAfterAPageBreakKeepsTheIndent()
{
    for (bool suppress : {false, true}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.spaceBeforeAfterBreak = !suppress;
        QTextCursor c(fd.story());
        QTextBlockFormat bf;
        bf.setTopMargin(20);
        bf.setTextIndent(36);
        c.setBlockFormat(bf);
        c.insertText(QString(flow::kPageBreak) + QStringLiteral("second words"));
        c.insertBlock(bf);
        c.insertText(QStringLiteral("a paragraph after"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 2);
        const auto lines = pg.lines(1);
        QVERIFY(!lines.isEmpty());
        QVERIFY2(std::abs(lines[0].rect.left() - 36) < 0.01, qPrintable(QStringLiteral("suppressing %1: the first line begins at %2").arg(suppress).arg(lines[0].rect.left())));
        QVERIFY2(std::abs(lines[0].rect.top() - (suppress ? 0 : 20)) < 0.01, qPrintable(QString::number(lines[0].rect.top())));
        LayoutCache cache;
        QVERIFY2(materializedDiffers(fd, pg, 1, cache).isEmpty(), qPrintable(materializedDiffers(fd, pg, 1, cache)));
    }
}


// The text after a page break that begins a paragraph is its first line, at the top of the next page with the paragraph's space before; what goes on after a break that
// has something before it on its line (the rest of the paragraph) has none there, as a paragraph that goes on onto a page for want of room has none.
void FlowTests::aBreakKeepsTheSpaceBeforeOnTheNextPageOnlyWhereItBeginsTheParagraph()
{
    auto secondPage = [&](const QString &text) {
        FlowDocument fd;
        fd.env.wordLines = true;
        QTextCursor c(fd.story());
        QTextBlockFormat bf;
        bf.setTopMargin(20);
        c.setBlockFormat(bf);
        c.insertText(text);
        c.insertBlock(bf);
        c.insertText(QStringLiteral("a paragraph after"));
        Paginator pg(&fd);
        return std::make_pair(pg.pageCount(), pg.lines(1));
    };
    const auto begins = secondPage(QString(flow::kPageBreak) + QStringLiteral("second words"));
    QCOMPARE(begins.first, 2);
    QCOMPARE(begins.second[0].line, 1);
    QVERIFY2(std::abs(begins.second[0].rect.top() - 20) < 0.01, qPrintable(QString::number(begins.second[0].rect.top())));
    QVERIFY(std::abs(begins.second[1].rect.top() - (begins.second[0].rect.bottom() + 20)) < 0.01);
    const auto inside = secondPage(QStringLiteral("first words ") + flow::kPageBreak + QStringLiteral("second words"));
    QCOMPARE(inside.first, 2);
    QCOMPARE(inside.second[0].line, 1);
    QVERIFY2(std::abs(inside.second[0].rect.top()) < 0.01, qPrintable(QString::number(inside.second[0].rect.top())));
    QVERIFY(std::abs(inside.second[1].rect.top() - (inside.second[0].rect.bottom() + 20)) < 0.01);
    // The page made for painting has the same lines.
    for (const QString &text : {QString(flow::kPageBreak) + QStringLiteral("second words"), QStringLiteral("first words ") + flow::kPageBreak + QStringLiteral("second words")}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        QTextCursor c(fd.story());
        QTextBlockFormat bf;
        bf.setTopMargin(20);
        c.setBlockFormat(bf);
        c.insertText(text);
        c.insertBlock(bf);
        c.insertText(QStringLiteral("a paragraph after"));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 2);
        LayoutCache cache;
        QVERIFY2(materializedDiffers(fd, pg, 1, cache).isEmpty(), qPrintable(materializedDiffers(fd, pg, 1, cache)));
    }
    // A page that a paragraph runs onto for want of room has no space before its rest.
    FlowDocument run;
    QTextCursor r(run.story());
    QTextBlockFormat bf;
    bf.setTopMargin(20);
    r.setBlockFormat(bf);
    r.insertText(QString(12000, QLatin1Char('x')).replace(QRegularExpression(QStringLiteral("(x{7})")), QStringLiteral("\\1 ")));
    Paginator rp(&run);
    QVERIFY(rp.pageCount() > 1);
    QVERIFY(!rp.page(1).start.hard);
    QVERIFY(std::abs(rp.lines(1)[0].rect.top()) < 0.01);
}


// A break at the end of a paragraph leaves the next paragraph the top of the next page, with no empty line before it; at the end of the
// last paragraph, it makes no page of its own.
void FlowTests::aPageBreakAtTheEndOfAParagraphMakesNoEmptyLine()
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
void FlowTests::theMarkAfterABreakGoesToTheNextPageWhenTheDocumentSaysSo()
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
void FlowTests::aPageBreakOnTheLastLineThatFitsStillOpensTheNextPage_data()
{
    QTest::addColumn<bool>("word");
    QTest::addColumn<bool>("markFollows");
    QTest::newRow("publication") << false << false;
    QTest::newRow("word") << true << false;
    QTest::newRow("publication, mark follows") << false << true;
    QTest::newRow("word, mark follows") << true << true;
}


void FlowTests::aPageBreakOnTheLastLineThatFitsStillOpensTheNextPage()
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
void FlowTests::aPageBreakInTheMiddleOfAParagraphContinuesItOnTheNextPage()
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
void FlowTests::pageBreakCharactersFollowOneAnotherAndOpenAParagraph()
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
void FlowTests::editingAPageBreakCharacterRepaginates()
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
void FlowTests::paginatorStartsSectionsOnNewPages()
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
void FlowTests::paginatorLeavesOutSpaceBeforeAtTheTopOfAPage()
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
void FlowTests::wordLinesAreTheFontsWindowsMetrics()
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
void FlowTests::wordLinesOfMeasuredFacesAreTheirOwn()
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
void FlowTests::wordLinesOfMissingFamiliesAreTheStandIns()
{
    const auto lines = wordLinesOf({{QStringLiteral("No Such Typeface Anywhere"), 12}, {QStringLiteral("Times"), 12}, {QStringLiteral("Helvetica"), 11}});
    QCOMPARE(lines.size(), 3);
    const double heights[] = {(1950 + 550) / 2048.0 * 12, (1825 + 443 + 87) / 2048.0 * 12, (1854 + 434 + 67) / 2048.0 * 11};
    for (int i = 0; i < 3; ++i)
        QVERIFY2(std::abs(lines[i].rect.height() - heights[i]) < 0.01, qPrintable(QStringLiteral("line %1 is %2 tall, not %3").arg(i).arg(lines[i].rect.height()).arg(heights[i])));
}


// The characters of the symbol fonts (a degree sign from Symbol, an arrow from Wingdings, a solid block from Webdings: codes 0xF020 to 0xF0FF) are drawn as
// the pictures they are where this machine lacks the font, each as wide as the font makes it (819, 1825, and 2048 units of 2048 for these three).
void FlowTests::symbolFontCharactersAreTheirPicturesAtTheirOwnWidths()
{
    struct Case { const char *family; char16_t code, picture; int units; };
    const Case cases[] = {{"Symbol", 0xF0B0, 0x00B0, 819}, {"Wingdings", 0xF0EA, 0x2B07, 1825}, {"Webdings", 0xF067, 0x2588, 2048}};
    int checked = 0;
    for (const Case &k : cases) {
        if (QFontDatabase::hasFamily(QString::fromLatin1(k.family))) continue;   // (installed here: drawn by the font itself)
        ++checked;
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
    // (Where all three fonts are installed nothing was checked: the fonts draw themselves.)
    if (checked == 0) QSKIP("Symbol, Wingdings, and Webdings are all installed here: their characters are drawn by the fonts");
}


// Courier New, where the machine lacks it, is drawn in TeX Gyre Cursor, a face of Courier's widths (600 units in 1000, Courier New's 1229 in 2048 to a ten-thousandth of an
// em) whose strokes are as thin as any bundled (its stand-in for a publisher's text, Cousine, is heavy), fully opaque in the text's own color, and its line stays Courier
// New's; a publisher's text keeps Cousine.
void FlowTests::courierNewIsDrawnInAThinFaceAndSetAsItself()
{
    if (QFontDatabase::hasFamily(QStringLiteral("Courier New"))) QSKIP("Courier New is installed");
    QVERIFY(QFontDatabase::hasFamily(QStringLiteral("TeX Gyre Cursor")));
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("Courier New")});
    cf.setFontPointSize(10);
    cf.setForeground(QColor(10, 20, 30));
    LayoutEnv word;
    word.wordLines = true;
    const QTextCharFormat drawn = resolveCharFormat(cf, word);
    QCOMPARE(drawn.fontFamilies().toStringList().value(0), QStringLiteral("TeX Gyre Cursor"));
    QCOMPARE(drawn.foreground().color(), QColor(10, 20, 30));   // (opaque: printed, it is as black as the text says)
    QCOMPARE(drawn.intProperty(tp::HandBold), 3);   // (and drawn with a light stroke in pictures: the reference letters are a little darker than the face is)
    QVERIFY(!resolveCharFormat(cf, LayoutEnv()).hasProperty(tp::HandBold));
    QCOMPARE(resolveCharFormat(cf, LayoutEnv()).fontFamilies().toStringList().value(0), QStringLiteral("Courier New"));
    // (The line is Courier New's, not the face it is drawn in: 1.1328 em.)
    const auto lines = wordLinesOf({{QStringLiteral("Courier New"), 20}});
    QVERIFY2(std::abs(lines.value(0).rect.height() - (1705 + 615) / 2048.0 * 20) < 0.01, qPrintable(QString::number(lines.value(0).rect.height())));
    // (The letters are Courier New's width, 0.6 em, to a ten-thousandth.)
    QFont font(QStringLiteral("TeX Gyre Cursor"));
    font.setPixelSize(1000);
    QVERIFY(std::abs(QFontMetricsF(font).horizontalAdvance(QLatin1Char('M')) - 1000 * 1229 / 2048.0) < 0.2);
}


// The light stroke of the Courier New stand-in (tp::HandBold kind 3) is a picture's alone: it takes no part in how the letters are set, so each letter of a run is where
// Courier New's 0.6 em puts it (as in a PDF and in print), with or without the stroke, in a run of words and of spaces, and a bold run is set the same, with the stroke.
void FlowTests::theCourierStrokeLeavesTheLettersWhereTheyAre()
{
    if (QFontDatabase::hasFamily(QStringLiteral("Courier New"))) QSKIP("Courier New is installed");
    const QString text = QStringLiteral("Time_Period_of_Content: Single Date/Time   Calendar_Date: 2005");
    const auto line = letterLineOf(QStringLiteral("Courier New"), text, 10, true, false);
    for (int i = 0; i <= text.size(); ++i)
        QVERIFY2(std::abs(line.x[i] - i * 6.0) < 0.15, qPrintable(QStringLiteral("letter %1 is at %2, not %3").arg(i).arg(line.x[i]).arg(i * 6.0)));
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("Courier New")});
    cf.setFontPointSize(10);
    LayoutEnv word;
    word.wordLines = true;
    for (const int weight : {int(QFont::Normal), int(QFont::Bold)}) {
        cf.setFontWeight(weight);
        QCOMPARE(resolveCharFormat(cf, word).intProperty(tp::HandBold), 3);   // (a bold run has a real bold face, and is not by hand: it keeps the light stroke)
    }
}


// The light stroke of the Courier New stand-in made a typewritten report of 85 pages take over five minutes to draw (it was 16 seconds before
// the stroke): every letter's outline was stroked anew each time it came. Each letter is now stroked once and then filled where it comes again.
void FlowTests::theCourierStrokeIsWorkedOutOncePerLetterAndRun()
{
    if (QFontDatabase::hasFamily(QStringLiteral("Courier New"))) QSKIP("Courier New is installed");
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat a, b;
    a.setFontFamilies({QStringLiteral("Courier New")});
    a.setFontPointSize(10);
    b = a;
    b.setForeground(QColor(40, 40, 40));   // (two formats one after the other, so that every word is a run of its own)
    const int words = 3000;
    for (int i = 0; i < words; ++i) c.insertText(QStringLiteral("report%1 ").arg(i % 10), i % 2 ? b : a);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(400, 40000)}, env);
    const int lines = lay.lineInfo(0).size();
    QVERIFY2(lines > 300, qPrintable(QStringLiteral("%1 lines").arg(lines)));
    QImage img(400, 4000, QImage::Format_RGB32);
    img.fill(Qt::white);
    handBoldWork() = HandBoldWork();
    QPainter p(&img);
    lay.paint(&p, 0, PaintOptions());
    p.end();
    const HandBoldWork w = handBoldWork();
    qInfo() << lines << "lines," << words << "runs:" << w.stroked << "letters stroked";
    QVERIFY2(w.stroked > 0 && w.stroked <= 40, qPrintable(QStringLiteral("%1 letters stroked").arg(w.stroked)));
}


// A word processor's missing face, measured, is drawn in a stand-in whose letters take the original's widths, the difference from the stand-in's going into the
// spacing after each letter. Where the stand-in's capital I is much narrower than the original's, the letters of a word shared that difference unevenly (the first
// letter of a run kept all of its own after it), and the gap beside the I was wide enough for a reader of the page's text to take "I llinois", "I ncludes", and
// "SECTION I I" for two words each (a gap of a tenth of an em parts a word there). The letters of a word now share the word's difference evenly, none of
// its gaps over a twentieth of an em, and the words keep their places on the line.
void FlowTests::wordLettersShareTheirWidthsEvenly()
{
    double diff = 0;
    const QString family = narrowStandInForI(&diff);
    if (family.isEmpty()) QSKIP("every face with a narrow I stand-in is installed here");
    qInfo() << family << "stand-in's I is narrower by" << diff << "em";
    const double size = 100;
    const QString text = QStringLiteral("Illinois Includes SECTION II 1999 end");
    for (bool kerned : {false, true}) {
        const LetterLine line = letterLineOf(family, text, size, true, kerned);
        QVERIFY(line.x.size() == text.size() + 1);
        // No gap between two letters of a word is wide enough to part it.
        double widest = 0;
        QString where;
        for (int i = 0; i + 1 < text.size(); ++i) {
            if (text[i].isSpace() || text[i + 1].isSpace()) continue;
            const double gap = (line.x[i + 1] - line.x[i]) / size - line.stand[i];
            if (gap > widest) {
                widest = gap;
                where = text.mid(i, 2);
            }
        }
        QVERIFY2(widest <= 0.07, qPrintable(QStringLiteral("a gap of %1 em after the %2 of \"%3\"").arg(widest).arg(where.left(1), where)));
        // Every word starts and ends where the original's widths put it, to a point.
        int checked = 0;
        for (int start = 0; start < text.size();) {
            if (text[start].isSpace()) { ++start; continue; }
            int stop = start;
            while (stop < text.size() && !text[stop].isSpace()) ++stop;
            if (stop < text.size()) {
                const QString word = text.mid(start, stop - start);
                QVERIFY2(std::abs(line.x[start] - line.x[0] - line.cell[start]) <= 1, qPrintable(QStringLiteral("\"%1\" starts at %2, not %3").arg(word).arg(line.x[start] - line.x[0]).arg(line.cell[start])));
                QVERIFY2(std::abs(line.x[stop] - line.x[0] - line.cell[stop]) <= 1, qPrintable(QStringLiteral("\"%1\" ends at %2, not %3").arg(word).arg(line.x[stop] - line.x[0]).arg(line.cell[stop])));
                ++checked;
            }
            start = stop;
        }
        QCOMPARE(checked, 5);
    }
}


// A word is not set as one stretch where a line may break inside it (after a hyphen or a dash; a word processor's line does not break after a slash): the part before the break
// is shared out by itself and ends where the original's does, so that a line breaks as it does there; the rest of the word is the same.
void FlowTests::wordLettersKeepTheOriginalsPlaceAtEveryBreakInsideTheWord()
{
    double diff = 0;
    const QString family = narrowStandInForI(&diff);
    if (family.isEmpty()) QSKIP("every face with a narrow I stand-in is installed here");
    const double size = 100;
    const QString text = QStringLiteral("well-known and/or Illinois-Includes Il\u2013In end");
    const LetterLine line = letterLineOf(family, text, size, true, false);
    QVERIFY(line.x.size() == text.size() + 1);
    int checked = 0;
    for (int p = 1; p < text.size(); ++p) {
        if (text[p - 1] != QLatin1Char('-') && text[p - 1] != QChar(0x2013)) continue;
        QVERIFY2(std::abs(line.x[p] - line.x[0] - line.cell[p]) <= 1, qPrintable(QStringLiteral("a line may break before \"%1\" (at %2), the original's place is %3, not %4").arg(text.mid(p, 4)).arg(p).arg(line.cell[p]).arg(line.x[p] - line.x[0])));
        ++checked;
    }
    QCOMPARE(checked, 3);
    for (int stop = 0; stop < text.size(); ++stop)   // (the words, "and/or" among them, end where the original's do)
        if (text[stop].isSpace()) QVERIFY2(std::abs(line.x[stop] - line.x[0] - line.cell[stop]) <= 1, qPrintable(QStringLiteral("the word before %1 ends at %2, not %3").arg(stop).arg(line.x[stop] - line.x[0]).arg(line.cell[stop])));
    double widest = 0;
    for (int i = 0; i + 1 < text.size(); ++i)
        if (!text[i].isSpace() && !text[i + 1].isSpace()) widest = std::max(widest, (line.x[i + 1] - line.x[i]) / size - line.stand[i]);
    QVERIFY2(widest <= 0.07, qPrintable(QString::number(widest)));
}


// A word with a soft hyphen or a zero-width space in it (or one that the layout hyphenates by its own rule) is shared out in the parts between the places a line may break, as
// a word with a hyphen is: the part before the place ends where the original's does.
void FlowTests::wordLettersKeepTheOriginalsPlaceAtASoftHyphenAndAHyphenationPoint()
{
    double diff = 0;
    const QString family = narrowStandInForI(&diff);
    if (family.isEmpty()) QSKIP("every face with a narrow I stand-in is installed here");
    const double size = 100;
    const QString text = QStringLiteral("Illi\u00ADnois Includ\u200Bes Information end");
    const QString hyphenated = QStringLiteral("Information");
    QVector<int> places;   // the places a line may break, as the character that follows
    for (int p = 0; p < text.size(); ++p)
        if (text[p] == QChar(0x00AD) || text[p] == QChar(0x200B)) places << p + 1;
    const int at = int(text.indexOf(hyphenated));
    for (int point : hyphenationPoints(hyphenated)) places << at + point;
    QCOMPARE(int(places.size()), 2 + int(hyphenationPoints(hyphenated).size()));
    QVERIFY(places.size() >= 4);
    const LetterLine line = letterLineOf(family, text, size, true, false, {}, true);
    QVERIFY(line.x.size() == text.size() + 1);
    for (int p : places)
        QVERIFY2(std::abs(line.x[p] - line.x[0] - line.cell[p]) <= 1, qPrintable(QStringLiteral("a line may break before \"%1\" (at %2), the original's place is %3, not %4").arg(text.mid(p, 4)).arg(p).arg(line.cell[p]).arg(line.x[p] - line.x[0])));
}


// A line of Chinese text in a stand-in run (each character a piece of its own, no word between spaces) is set as the publication's text is, as it was before words were shared
// out: its Latin letters keep what they had.
void FlowTests::aLineOfChineseTextIsSetAsItWas()
{
    double diff = 0;
    const QString family = narrowStandInForI(&diff);
    if (family.isEmpty()) QSKIP("every face with a narrow I stand-in is installed here");
    const double size = 100;
    for (const QString &text : {QStringLiteral("\u65E5\u672C\u8A9E\u306E\u30C6\u30AD\u30B9\u30C8"), QStringLiteral("\u65E5\u672CA\u8A9EI\u6587\u5B57I")}) {
        const LetterLine word = letterLineOf(family, text, size, true, false), publication = letterLineOf(family, text, size, false, false);
        QVERIFY(word.x.size() == text.size() + 1 && publication.x.size() == word.x.size());
        for (int i = 1; i <= text.size(); ++i)
            QVERIFY2(std::abs((word.x[i] - word.x[0]) - (publication.x[i] - publication.x[0])) <= 1, qPrintable(QStringLiteral("%1: character %2 is at %3, not %4").arg(text.left(3)).arg(i).arg(word.x[i] - word.x[0]).arg(publication.x[i] - publication.x[0])));
    }
}


// A word whose last character the pieces can't carry alone (a letter the table doesn't measure, or a combining accent) still ends where the original's does, in the middle of a
// run and at the end of one, and what is left of its difference doesn't turn up in the first gap of the word after it.
void FlowTests::aWordEndingInACharacterNoPieceCanCarryStillEndsWhereTheOriginalsDoes()
{
    double diff = 0;
    const QString family = narrowStandInForI(&diff);
    if (family.isEmpty()) QSKIP("every face with a narrow I stand-in is installed here");
    const double size = 100;
    const QString text = QStringLiteral("Includ\u017C Illinois\u0301 du\u017C\u0105 cafe\u0301 Illinois end");
    QVector<int> everyWordEnd;
    for (int i = 0; i < text.size(); ++i)
        if (text[i].isSpace()) everyWordEnd << i;
    for (const QVector<int> &splits : {QVector<int>(), everyWordEnd}) {
        const LetterLine line = letterLineOf(family, text, size, true, false, splits);
        QVERIFY(line.x.size() == text.size() + 1);
        const QString where = splits.isEmpty() ? QStringLiteral("in a run") : QStringLiteral("at the end of a run");
        for (int stop = 0; stop < text.size(); ++stop) {
            if (!text[stop].isSpace()) continue;
            QVERIFY2(std::abs(line.x[stop] - line.x[0] - line.cell[stop]) <= 1, qPrintable(QStringLiteral("%1: the word before %2 ends at %3, not %4").arg(where).arg(stop).arg(line.x[stop] - line.x[0]).arg(line.cell[stop])));
        }
        double widest = 0;
        int at = 0;
        for (int i = 0; i + 1 < text.size(); ++i) {
            if (text[i].isSpace() || text[i + 1].isSpace()) continue;
            const double gap = (line.x[i + 1] - line.x[i]) / size - line.stand[i];
            if (gap > widest) {
                widest = gap;
                at = i;
            }
        }
        QVERIFY2(widest <= 0.07, qPrintable(QStringLiteral("%1: a gap of %2 em after \"%3\"").arg(where).arg(widest).arg(text.mid(at, 2))));
    }
    // What is left of a word that the gaps can't take within the limit (its capitals much narrower) is taken by the gaps all the same, not dropped.
    const QString narrow = QStringLiteral("III\u017C IIIi\u0301 end");
    for (const QVector<int> &splits : {QVector<int>(), QVector<int>{4, 11}}) {
        const LetterLine line = letterLineOf(family, narrow, size, true, false, splits);
        QVERIFY(line.x.size() == narrow.size() + 1);
        for (int stop = 0; stop < narrow.size(); ++stop)
            if (narrow[stop].isSpace())
                QVERIFY2(std::abs(line.x[stop] - line.x[0] - line.cell[stop]) <= 1, qPrintable(QStringLiteral("\"%1\" ends at %2, not %3").arg(narrow.left(stop)).arg(line.x[stop] - line.x[0]).arg(line.cell[stop])));
    }
}


// A word that goes on in another run (a format change inside it: a bold I, a field's result) is one word all the same: what the letters of its first part
// can't take of the word's difference (a gap over a twentieth of an em) is taken by the letters of the next, not left after the first, where the "I" of
// "I llinois" parts the word.
void FlowTests::wordLettersShareTheirWidthsAcrossRuns()
{
    double diff = 0;
    const QString family = narrowStandInForI(&diff);
    if (family.isEmpty()) QSKIP("every face with a narrow I stand-in is installed here");
    const double size = 100;
    const QString text = QStringLiteral("Illinois Includes SECTION II 1999 end");
    for (const QVector<int> &splits : {QVector<int>{1}, QVector<int>{1, 2}, QVector<int>{10}, QVector<int>{1, 10, 11}}) {
        const LetterLine line = letterLineOf(family, text, size, true, true, splits);
        QVERIFY(line.x.size() == text.size() + 1);
        const QString where = QStringLiteral("split at %1").arg(splits.value(0));
        double widest = 0;
        int at = 0;
        for (int i = 0; i + 1 < text.size(); ++i) {
            if (text[i].isSpace() || text[i + 1].isSpace()) continue;
            const double gap = (line.x[i + 1] - line.x[i]) / size - line.stand[i];
            if (gap > widest) {
                widest = gap;
                at = i;
            }
        }
        QVERIFY2(widest <= 0.07, qPrintable(QStringLiteral("%1: a gap of %2 em after the %3 of \"%4\"").arg(where).arg(widest).arg(text[at]).arg(text.mid(at, 2))));
        for (int stop = 0; stop < text.size(); ++stop) {
            if (!text[stop].isSpace()) continue;
            QVERIFY2(std::abs(line.x[stop] - line.x[0] - line.cell[stop]) <= 1, qPrintable(QStringLiteral("%1: the word before %2 ends at %3, not %4").arg(where).arg(stop).arg(line.x[stop] - line.x[0]).arg(line.cell[stop])));
        }
    }
}


// A run that takes no part in it (another face, which no table measures) after the I: the I keeps the whole of its difference after it, as the original's
// letter fills its width, and the line is no shorter.
void FlowTests::aWordsFirstPartKeepsItsWidthWhereTheNextRunTakesNone()
{
    double diff = 0;
    const QString family = narrowStandInForI(&diff);
    if (family.isEmpty()) QSKIP("every face with a narrow I stand-in is installed here");
    const double size = 100;
    const QHash<char16_t, double> *widths = originalLetterWidths(family, false, false);
    QVERIFY(widths);
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat cf;
    cf.setFontFamilies({family});
    cf.setFontPointSize(size);
    c.insertText(QStringLiteral("I"), cf);
    cf.setFontFamilies({QStringLiteral("Arimo")});
    c.insertText(QStringLiteral("llinois"), cf);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(8000, 400)}, env);
    auto x = [&](int pos) {
        int frame = -1;
        QRectF r;
        lay.caretRect(pos, &frame, &r);
        return r.x();
    };
    const double want = widths->value(u'I') * size;
    QVERIFY2(std::abs(x(1) - x(0) - want) <= 1, qPrintable(QStringLiteral("the I is %1 wide, not %2").arg(x(1) - x(0)).arg(want)));
}


// The publication's text is set as it was: each letter of a missing, measured face centered in the original's width (the first letter of a run where the
// original's starts), whatever a word processor's words do.
void FlowTests::publicationLettersStayCenteredInTheirWidths()
{
    double diff = 0;
    const QString family = narrowStandInForI(&diff);
    if (family.isEmpty()) QSKIP("every face with a narrow I stand-in is installed here");
    const double size = 100;
    const QString text = QStringLiteral("Illinois Includes SECTION II 1999 end");
    const LetterLine line = letterLineOf(family, text, size, false);
    QVERIFY(line.x.size() == text.size() + 1);
    QVERIFY2(std::abs(line.x[0]) <= 1, qPrintable(QString::number(line.x[0])));
    for (int i = 1; i < text.size(); ++i) {
        const double shift = text[i].isSpace() ? 0 : (line.width[i] - line.stand[i]) * size / 2;
        QVERIFY2(std::abs(line.x[i] - line.x[0] - line.cell[i] - shift) <= 1, qPrintable(QStringLiteral("character %1 (%2) is at %3, not %4").arg(i).arg(text[i]).arg(line.x[i] - line.x[0]).arg(line.cell[i] + shift)));
    }
}


// A face the word processor's machine has and this one lacks, and no table of letter widths measures (News Gothic MT, whose bold is the masthead of every public notice),
// is drawn in the nearest bundled face of its kind (Libre Franklin, a gothic of the same bearing: "PUBLIC NOTICE" at 48 points bold is 495 pixels wide at 96 to the inch,
// 497 in Libre Franklin Bold, and 403 in the Calibri stand-in it had), and its line stays its own (2003 and 458 units).
void FlowTests::newsGothicIsDrawnInALikeGothicAndSetAsItself()
{
    if (QFontDatabase::hasFamily(QStringLiteral("News Gothic MT"))) QSKIP("News Gothic MT is installed");
    QVERIFY(QFontDatabase::hasFamily(QStringLiteral("Libre Franklin")));
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("News Gothic MT")});
    cf.setFontPointSize(48);
    cf.setFontWeight(QFont::Bold);
    LayoutEnv word;
    word.wordLines = true;
    QCOMPARE(resolveCharFormat(cf, word).fontFamilies().toStringList().value(0), QStringLiteral("Libre Franklin"));
    QCOMPARE(resolveCharFormat(cf, LayoutEnv()).fontFamilies().toStringList().value(0), QStringLiteral("News Gothic MT"));
    const auto lines = wordLinesOf({{QStringLiteral("News Gothic MT"), 48}});
    QVERIFY2(std::abs(lines.value(0).rect.height() - (2003 + 458) / 2048.0 * 48) < 0.01, qPrintable(QString::number(lines.value(0).rect.height())));
}


// Univers, the other face of the word processor's machine that no table measures, is drawn in Arimo, 8 percent wider in its regular face and no wider in its bold, and its
// line stays its own (2025 and 512 units).
void FlowTests::universIsDrawnInArimoWiderAndSetAsItself()
{
    if (QFontDatabase::hasFamily(QStringLiteral("Univers"))) QSKIP("Univers is installed");
    LayoutEnv word;
    word.wordLines = true;
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("Univers")});
    cf.setFontPointSize(11);
    const QTextCharFormat regular = resolveCharFormat(cf, word);
    QCOMPARE(regular.fontFamilies().toStringList().value(0), QStringLiteral("Arimo"));
    QCOMPARE(regular.fontStretch(), stretchToAsk(108));
    cf.setFontWeight(QFont::Bold);
    QCOMPARE(resolveCharFormat(cf, word).fontStretch(), 0);   // (the bold is as wide as Arimo's: no stretch is asked)
    const auto lines = wordLinesOf({{QStringLiteral("Univers"), 11}});
    QVERIFY2(std::abs(lines.value(0).rect.height() - (2025 + 512) / 2048.0 * 11) < 0.01, qPrintable(QString::number(lines.value(0).rect.height())));
}


// A font the document's font table says its machine set in Yu Gothic (V104's FreeSerif, charset 128, with that alternate) is set in it by a machine that lacks FreeSerif:
// the alternate is one of the faces the word processor's machine has, drawn in Open Sans (the same widths), and its line is 1.673 em (3426 units) as the
// reference pages have it.
void FlowTests::yuGothicIsDrawnInOpenSansAndSetAsItself()
{
    QCOMPARE(wordStandInFor(QStringLiteral("Yu Gothic"), QStringLiteral("roman"), QStringLiteral("variable"), QString(), QStringLiteral("80")), QStringLiteral("Yu Gothic"));
    if (QFontDatabase::hasFamily(QStringLiteral("Yu Gothic"))) QSKIP("Yu Gothic is installed");
    LayoutEnv word;
    word.wordLines = true;
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("Yu Gothic")});
    cf.setFontPointSize(11);
    const QTextCharFormat drawn = resolveCharFormat(cf, word);
    QCOMPARE(drawn.fontFamilies().toStringList().value(0), QStringLiteral("Open Sans"));
    QCOMPARE(drawn.foreground().color().alphaF(), 1.0);   // (the text is never drawn partly transparent: printed, it is black)
    const auto lines = wordLinesOf({{QStringLiteral("Yu Gothic"), 11}});
    QVERIFY2(std::abs(lines.value(0).rect.height() - 3426 / 2048.0 * 11) < 0.01, qPrintable(QString::number(lines.value(0).rect.height())));
}


// Text is never drawn partly transparent, nor with a stroke by hand in print or in a PDF, to make a stand-in as light or as heavy as the face it stands for: it is printed and
// exported as the text's color says (black is black). The one stroke there is is the thin stand-in for Courier New's, a light one in pictures only (kind 3 of tp::HandBold).
void FlowTests::aStandInIsDrawnInTheTextsOwnColorAndWithNoStroke()
{
    LayoutEnv word;
    word.wordLines = true;
    QTextCharFormat cf;
    cf.setFontPointSize(10);
    cf.setForeground(QColor(200, 0, 0));
    for (const QString &family : {QStringLiteral("Courier New"), QStringLiteral("Univers"), QStringLiteral("News Gothic MT"), QStringLiteral("Yu Gothic"), QStringLiteral("Century Gothic")}) {
        if (QFontDatabase::hasFamily(family)) continue;   // (a machine with the face draws that)
        cf.setFontFamilies({family});
        for (const int weight : {int(QFont::Normal), int(QFont::Bold)}) {
            cf.setFontWeight(weight);
            const QTextCharFormat drawn = resolveCharFormat(cf, word);
            QVERIFY2(drawn.foreground().color() == QColor(200, 0, 0), qPrintable(family + QStringLiteral(": ") + drawn.foreground().color().name(QColor::HexArgb)));
            const bool lightStroke = family == QLatin1String("Courier New") && drawn.intProperty(tp::HandBold) == 3;
            QVERIFY2(!drawn.hasProperty(tp::HandBold) || lightStroke, qPrintable(family + QStringLiteral(" has a stroke by hand")));
        }
    }
}


// A stand-in that is drawn with a stroke by hand (Bodoni MT Black, Rockwell Extra Bold) is drawn in two passes, and a PDF's text reader ends a word at each piece of text that is
// drawn so: a word that the letter widths cut into pieces reads as two ("Propert" and "y", in the heading of a practice document). Such a run is cut only where a word ends or a line
// may break in it, whatever the stand-in's widths are to the original's.
void FlowTests::aWordInAStandInDrawnByHandIsOnePiece()
{
    LayoutEnv word;
    word.wordLines = true;
    const QString text = QString::fromUtf8("Property Scrapbook—Success & Dedication of Ownership");
    int tested = 0;
    for (const QString &family : {QStringLiteral("Bodoni MT Black"), QStringLiteral("Rockwell Extra Bold")}) {
        if (QFontDatabase::hasFamily(family)) continue;   // (a machine with the face draws that)
        QTextCharFormat cf;
        cf.setFontFamilies({family});
        cf.setFontPointSize(12);
        const QTextCharFormat drawn = resolveCharFormat(cf, word);
        if (!drawn.hasProperty(tp::HandBold)) continue;
        ++tested;
        const auto pieces = applyWordLetterWidths({QTextLayout::FormatRange{0, int(text.size()), drawn}}, text);
        QVERIFY2(pieces.size() > 1, "the widths cut the run");
        for (const auto &r : pieces) {
            if (r.start == 0) continue;
            const QChar before = text[r.start - 1], at = text[r.start];
            QVERIFY2(before.isSpace() || at.isSpace() || before == QChar(0x2014) || at == QChar(0x2014), qPrintable(QStringLiteral("%1: a piece begins inside a word, at %2 (\"%3\")").arg(family).arg(r.start).arg(text.mid(std::max(0, r.start - 4), 8))));
        }
    }
    if (!tested) QSKIP("no face of the kind is missing here");
}


// East Asian characters that a run's font lacks are drawn in a face of the machine's own that has them (the system's fonts are what is relied on: none is bundled): in a font the
// machine lacks, "東京" is two ems wide, where a missing glyph is half an em of nothing. Skipped on a machine with no face for them (the macOS and Windows machines of CI have them;
// a Linux one has them only if its image installs a CJK font).
void FlowTests::aRunInAMissingFontDrawsEastAsianCharactersInTheMachinesOwnFace()
{
    if (installedFaceFor(0x6771).isEmpty() || installedFaceFor(0x4EAC).isEmpty() || installedFaceFor(0x3068).isEmpty() || installedFaceFor(0x304D).isEmpty()) QSKIP("this machine has no face for the Japanese characters of the test (the ideographs and the kana)");
    const QString family = QStringLiteral("Test Mincho That No Machine Has");
    QVERIFY(!QFontDatabase::hasFamily(family));
    // (As text, and as a field's result: a ruby's text, which the practice document's two characters are, is a piece kept as XML that shows text in its place.)
    auto widthOf = [&](const QString &text, bool asField) {
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        QTextCharFormat cf;
        cf.setFontFamilies({family});
        cf.setFontPointSize(20);
        if (asField) cf.setProperty(tp::Field, QStringLiteral("raw:") + text);
        c.insertText(asField ? QString(QChar::ObjectReplacementCharacter) : text, cf);
        FrameSpec fs;
        fs.size = QSizeF(600, 100);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        LayoutEnv word;
        word.wordLines = true;
        StoryLayout layout;
        layout.build(doc.get(), {fs}, word);
        double width = 0;
        for (const QRectF &r : layout.rangeRects(0, 0, asField ? 1 : int(text.size()))) width += r.width();
        return width;
    };
    const QString ideographs = QString::fromUtf8("東京"), kana = QString::fromUtf8("とうきょう");
    for (const bool asField : {false, true}) {
        const double wide = widthOf(ideographs, asField), small = widthOf(kana, asField);
        QVERIFY2(wide >= 2 * 20 * 0.9, qPrintable(QStringLiteral("two ideographs are %1 points wide%2").arg(wide).arg(asField ? QStringLiteral(" as a field") : QString())));
        QVERIFY2(small >= 5 * 20 * 0.9, qPrintable(QStringLiteral("five kana are %1 points wide%2").arg(small).arg(asField ? QStringLiteral(" as a field") : QString())));
    }
}


// A stretch of white space of 65,536 characters or more, in one run or in several, and the words after it, are laid out in a few lines, the words on the last: Qt keeps the length
// of a stretch in sixteen bits, and gave lines of no characters one after another, each taking the time of the whole paragraph (a paragraph of 70,000 spaces and a picture took
// more than two minutes). The stretch is cut by word joiners every 60,000 characters, and the lines made (the work counted) are one for each part and one for the words.
void FlowTests::aStretchOfWhiteSpaceLongerThanQtLaysOutIsLaidOutInAFewLines()
{
    LayoutEnv word;
    word.wordLines = true;
    for (const int n : {1000, 65535, 65536, 70000, 140000}) {
        for (const bool split : {false, true}) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Arimo")});
            cf.setFontPointSize(11);
            if (!split) {
                c.insertText(QString(n, QLatin1Char(' ')), cf);
            } else {
                QTextCharFormat blue = cf;
                blue.setForeground(QColor(Qt::blue));
                c.insertText(QString(n / 2, QLatin1Char(' ')), cf);
                c.insertText(QString(n - n / 2, QLatin1Char(' ')), blue);
            }
            c.insertText(QStringLiteral("tail words"), cf);
            StoryLayout layout;
            layout.build(&doc, {plainFrame(300, 400)}, word);
            const auto lines = layout.lineInfo(0);
            const QString what = QStringLiteral("%1 spaces%2: %3 lines").arg(n).arg(split ? QStringLiteral(" in two runs") : QString()).arg(lines.size());
            QVERIFY2(!lines.isEmpty() && lines.size() <= n / 60000 + 2, qPrintable(what));
            const QVector<QRectF> tail = layout.rangeRects(0, n, n + 4);
            QVERIFY2(!tail.isEmpty() && tail.first().top() >= lines.last().rect.top() - 0.01, qPrintable(what + QStringLiteral(": the words are not on the last line")));
        }
    }
}


QTEST_MAIN(FlowTests)
