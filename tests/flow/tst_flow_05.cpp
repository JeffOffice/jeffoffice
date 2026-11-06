#include "tst_flow.h"


// Rows put in at the top of a table set at a place, by an edit that begins in the paragraph before the table (the end of its text, where the new cells' paragraphs are
// opened): the table is taller, the text that goes round it has more lines, and the pages from its paragraph on are laid out again (they are those of a new paginator).
void FlowTests::rowsPutInAtTheTopOfAFloatingTableLayOutThePagesAfterIt()
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


// A table set at a place may be wider than the text area (it goes over a margin, as a word processor's does): it still floats, at the place it says. Only one wider than the
// page is in the text.
void FlowTests::aFloatingTableWiderThanTheTextAreaStillFloats()
{
    auto make = [](double columnWidth) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fillStory(fd->story(), 4, 33, {}, 3, 6);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = columnWidth;
        spec.text = [](int r, int c) { return QStringLiteral("r%1c%2").arg(r).arg(c); };
        spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.down = TableData::Place::From::Margin; p.y = -30; t.place = p; };
        addTableAfter(*fd, 0, spec);
        return fd;
    };
    auto floating = [&](double columnWidth, QRectF *content = nullptr) {
        auto fd = make(columnWidth);
        Paginator pg(fd.get());
        const auto page = pg.materialize(0);
        QVector<QRectF> out;
        if (page)
            for (const auto &t : page->tables)
                if (t->floating) out << t->rect();
        if (content) *content = pg.page(0).content;
        return out;
    };
    QCOMPARE(int(floating(100).size()), 1);   // (the setup floats a table that fits)
    QRectF content;
    const auto wide = floating(250, &content);   // (500 points across, 468 of text)
    QVERIFY2(content.width() < 499 && content.width() > 400, qPrintable(QString::number(content.width())));
    QCOMPARE(int(wide.size()), 1);
    QVERIFY2(std::abs(wide[0].top() - (content.top() - 30)) < 0.01, qPrintable(audit::rectText(wide[0]) + QStringLiteral(" for the text area ") + audit::rectText(content)));
    // It begins at the margin's edge and goes over the right margin: it is not cut to the text, nor moved in.
    QVERIFY2(std::abs(wide[0].left() - content.left()) < 0.01 && wide[0].right() > content.right() + 20, qPrintable(audit::rectText(wide[0]) + QStringLiteral(" for the text area ") + audit::rectText(content)));
    QCOMPARE(int(floating(320).size()), 0);   // (a table wider than the page is in the text)
}


// A table set at a place a distance down from its paragraph that would go over the foot of the text area, but that has room between the paragraph's top and the foot, is
// slid up to end at the foot: it is on the page of its paragraph, in the text area, and not above its paragraph.
void FlowTests::aFloatingTableOffsetOverTheFootIsSlidUpToTheTextArea()
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
void FlowTests::anObjectWhoseWallPushesItsAnchorOffThePageGoesToTheNextWithIt()
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
void FlowTests::squareWrapKeepsLinesBesideAnObjectClearOfItsRectangle()
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


// An object is set from where its anchor would be without the wall the text goes round it by: the lines that its wall moved down, to the next column, are back where they were,
// and the object is where the paragraph is without it (a negative offset puts it above, in the column before).
void FlowTests::anObjectIsSetFromItsAnchorWhereItWouldBeWithoutItsOwnWall()
{
    auto make = [](bool wrapped, int *anchorBlock) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fd->sections[0].paper = QSizeF(300, 300);
        fd->sections[0].margins = QMarginsF(30, 30, 30, 30);
        fd->sections[0].columns = 2;
        fd->sections[0].columnGap = 20;
        fillStory(fd->story(), 14, 9, {}, 10, 20);
        // The paragraph whose first line is the lowest in the first column without the object.
        if (anchorBlock) {
            Paginator probe(fd.get());
            double lowest = -1;
            *anchorBlock = -1;
            for (const PageLine &l : pageLines(probe, *fd, 0))
                if (l.column == 0 && l.pos == l.block && l.rect.top() > lowest && l.rect.top() < 30 + 235 * 0.8) {
                    lowest = l.rect.top();
                    *anchorBlock = l.block;
                }
        }
        if (wrapped && anchorBlock && *anchorBlock >= 0) {
            const double columnWidth = fd->sections[0].columnWidth();
            const int id = addWrapped(fd.get(), columnWidth - 20, 120, *anchorBlock, FloatingObject::Wrap::TopBottom, 10, -110, QMarginsF(0, 0, 0, 0));
            Q_UNUSED(id);
        }
        return fd;
    };
    int anchorBlock = -1;
    auto plain = make(false, &anchorBlock);
    QVERIFY(anchorBlock >= 0);
    Paginator pp(plain.get());
    const double natural = paragraphTopOn(pp, *plain, 0, anchorBlock);
    QVERIFY(natural > 0);
    auto wrapped = make(true, &anchorBlock);
    Paginator pg(wrapped.get());
    const auto page = pg.materialize(0);
    QVERIFY(page);
    const Item *obj = objectOfSize(*page, wrapped->sections[0].columnWidth() - 20, 120);
    QVERIFY(obj);
    const QRectF firstColumn = wrapped->sections[0].columnRect(0);
    QVERIFY2(std::abs(obj->rect.top() - (natural - 110)) < 0.05, qPrintable(QStringLiteral("the object is at %1, its paragraph is at %2 without it").arg(obj->rect.top()).arg(natural)));
    QVERIFY2(std::abs(obj->rect.left() - (firstColumn.left() + 10)) < 0.05, qPrintable(QStringLiteral("the object is at %1 across, the first column is at %2").arg(obj->rect.left()).arg(firstColumn.left())));
    QVERIFY2(audit::problems(pg, wrapped->story()).isEmpty(), qPrintable(audit::problems(pg, wrapped->story())));
}


// The same where the lines that the wall moves are those of the paragraph before the anchor's, on the page where they go on: the object is set from where the anchor would be below
// them without it, and the lines are below the object, not a page on.
void FlowTests::anObjectAboveItsAnchorStaysOnThePageOfTheLinesItMoves()
{
    auto make = [](unsigned seed, bool wrapped, int *anchorBlock, int *page) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fd->sections[0].paper = QSizeF(300, 300);
        fd->sections[0].margins = QMarginsF(30, 30, 30, 30);
        fillStory(fd->story(), 12, seed, {}, 8, 40);
        // The first page that begins with the end of a paragraph that has few lines left, a paragraph after it on the page.
        Paginator probe(fd.get());
        *anchorBlock = -1;
        *page = -1;
        for (int k = 1; k < probe.pageCount() && *anchorBlock < 0; ++k) {
            const auto lines = pageLines(probe, *fd, k);
            if (lines.size() < 4 || lines[0].pos == lines[0].block) continue;
            int tail = 0;
            while (tail < lines.size() && lines[size_t(tail)].block == lines[0].block) ++tail;
            if (tail > 3 || tail >= lines.size()) continue;
            *anchorBlock = lines[size_t(tail)].block;
            *page = k;
        }
        if (wrapped && *anchorBlock >= 0) addWrapped(fd.get(), 200, 90, *anchorBlock, FloatingObject::Wrap::TopBottom, 0, -20, QMarginsF(0, 0, 0, 0));
        return fd;
    };
    unsigned seed = 1;
    int anchorBlock = -1, page = -1;
    std::unique_ptr<FlowDocument> plain;
    for (; seed < 200 && anchorBlock < 0; ++seed) plain = make(seed, false, &anchorBlock, &page);
    --seed;
    QVERIFY2(anchorBlock >= 0, "no story has a page that begins with the end of a paragraph and has another after it");
    Paginator pp(plain.get());
    const double natural = paragraphTopOn(pp, *plain, page, anchorBlock);
    QVERIFY(natural > 0);
    int again = -1, againPage = -1;
    auto wrapped = make(seed, true, &again, &againPage);
    Paginator pg(wrapped.get());
    const auto made = pg.materialize(page);
    QVERIFY(made);
    const Item *obj = objectOfSize(*made, 200, 90);
    QVERIFY2(obj, qPrintable(QStringLiteral("the object is not on page %1, where the lines it moves are").arg(page)));
    QVERIFY2(std::abs(obj->rect.top() - (natural - 20)) < 0.05, qPrintable(QStringLiteral("the object is at %1, its paragraph is at %2 without it").arg(obj->rect.top()).arg(natural)));
    QVERIFY2(audit::problems(pg, wrapped->story()).isEmpty(), qPrintable(audit::problems(pg, wrapped->story())));
}


// An empty paragraph is only its mark, and goes in any gap between objects that is a few points wide: a column of empty paragraphs between two pictures that are a little apart
// stays a column there (each a line below the one before), where a paragraph of words, which has no room there, is below the pictures.
void FlowTests::anEmptyParagraphGoesInANarrowGapBetweenObjects()
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
void FlowTests::objectsAreDrawnInTheOrderOfTheirHeights()
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
void FlowTests::anObjectAnchoredAtTheEndOfAParagraphIsOnItsLastLine()
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
void FlowTests::topAndBottomWrapLeavesNoTextBesideTheObject()
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
void FlowTests::wrapSidesPutTheTextOnTheSidesAsked()
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
void FlowTests::objectsBehindAndOverTheTextMoveNoLine()
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
void FlowTests::tightWrapIsSquareWrapForNow()
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
void FlowTests::objectsInTheMarginStripsAreWhereTheStripPutsThem()
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
void FlowTests::aParagraphThatRunsOnPastTheObjectsPageIsWhole()
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
void FlowTests::aParagraphOfSeveralPagesWithAWallLosesNoText()
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
void FlowTests::randomWallsAmongLongParagraphsLoseNoText()
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
void FlowTests::anEditAmongObjectsLaysOutOnlyThePagesItTouches()
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
void FlowTests::anObjectThatFillsThePageDoesNotStopThePagination()
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
void FlowTests::aPageGivesAtMostSomeObjectsAWrap()
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
void FlowTests::anObjectInOneColumnLeavesTheOtherColumnAlone()
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
void FlowTests::aTopAndBottomObjectIsAWallOnlyInTheColumnsItIsIn()
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
void FlowTests::textAfterATableGoesRoundAnObjectOnThePage()
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
void FlowTests::anObjectAboveItsAnchorLineWrapsTheLinesAbove()
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
void FlowTests::anObjectInATableCellIsNoWall()
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


// The objects in a header or footer are not walls for the body's text, which is another layer: it has no wall of theirs. A shape of the header that the text goes round
// and that reaches down past the margin does hold the page's text below it, where it begins (a shape that is not wrapped, or behind the text, does not).
void FlowTests::aWrappedObjectInTheHeaderHoldsTheBodyBelowItAndIsNoWall()
{
        struct Result { double top = 0; int walls = -1; int lines = 0; QVector<StoryLayout::LineBrief> briefs; };
        auto layout = [](FloatingObject::Wrap wrap) {
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
            Result r;
            r.top = pg.page(0).content.top();
            r.briefs = pg.lines(0);
            r.lines = int(r.briefs.size());
            if (const auto page = pg.materialize(0)) r.walls = int(audit::wallsOf(*page).size());
            return r;
        };
        const Result round = layout(FloatingObject::Wrap::Square), free = layout(FloatingObject::Wrap::None);
        QCOMPARE(free.walls, 0);
        QCOMPARE(round.walls, 0);   // (no wall: the text is held below it, not set round it)
        QVERIFY2(free.top < 100, qPrintable(QString::number(free.top)));
        QVERIFY2(std::abs(round.top - 180) < 0.01, qPrintable(QString::number(round.top)));
        QVERIFY(round.lines > 0 && round.lines < free.lines);
        // The text is set as it is without the shape, line for line, and moved down by what the shape holds it: none of its lines goes round it.
        for (int i = 0; i < round.lines; ++i) {
            const auto &r = round.briefs[i], &f = free.briefs[i];
            QVERIFY2(r.pos == f.pos && r.end == f.end && std::abs(r.rect.left() - f.rect.left()) < 0.01 && std::abs(r.rect.width() - f.rect.width()) < 0.01 &&
                         std::abs(r.rect.top() - f.rect.top()) < 0.01 && std::abs(r.rect.height() - f.rect.height()) < 0.01,
                     qPrintable(QStringLiteral("line %1: %2 and %3").arg(i).arg(audit::rectText(r.rect), audit::rectText(f.rect))));
        }
    }


// The room above an object that the text goes round, set from its paragraph, does not reach up into the lines before the paragraph (text is laid out in order: they were
// there first, and the object is set from the paragraph's top): a frame as wide as the column with room above it leaves the line right above its paragraph where it is.
void FlowTests::theRoomAboveAnObjectSetFromItsParagraphLeavesTheLinesBeforeIt()
{
    auto firstLines = [&](bool withObject, double *objectTop, int *pages = nullptr) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 4, 91, {}, 3, 4);
        if (withObject) addWrapped(&fd, 466, 100, fd.story()->findBlockByNumber(2).position(), FloatingObject::Wrap::Square, 1, 1, QMarginsF(12, 12, 12, 12));
        Paginator pg(&fd);
        if (pages) *pages = pg.pageCount();
        const auto lines = pageLines(pg, fd, 0);
        if (objectTop && withObject)
            if (const auto page = pg.materialize(0))
                for (const auto &it : page->doc->pages[0]->items)
                    if (it->id.contains(QLatin1String("flow-object"))) *objectTop = it->rect.top();
        QVector<double> tops;
        for (const PageLine &l : lines)
            if (fd.story()->findBlock(l.pos).blockNumber() < 2) tops << l.rect.top();
        return tops;
    };
    double objectTop = -1;
    int pages = 0;
    const auto free = firstLines(false, nullptr), held = firstLines(true, &objectTop, &pages);
    QVERIFY(!free.isEmpty());
    QVERIFY2(pages == 1, qPrintable(QStringLiteral("the page and the object fit one page, and the document has %1").arg(pages)));
    QCOMPARE(held.size(), free.size());
    for (int i = 0; i < free.size(); ++i) QVERIFY2(std::abs(held[i] - free[i]) < 0.01, qPrintable(QStringLiteral("line %1 of the paragraphs before the object's: %2 with it, %3 without").arg(i).arg(held[i]).arg(free[i])));
    // (The object is where its paragraph begins, a point down.)
    QVERIFY2(objectTop > free.last() && objectTop < free.last() + 60, qPrintable(QString::number(objectTop)));
}


// An object is anchored by a character of the story it is in: a header has the objects its own characters anchor, and the body's first paragraph (at the same place in its
// story) has its own, and each is drawn once.
void FlowTests::anObjectIsDrawnOnceWhenThePageHasAHeaderToo()
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
void FlowTests::theHeaderAndFooterAreDrawnBeforeTheBodysObjects()
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
void FlowTests::aTextBoxIsAWallAsAPictureIs()
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
void FlowTests::lineBriefsSayHowFarAWrapMovedALine()
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


// An entry of a table of contents with its number ("1.1<tab>title<tab>12", a left stop for the title and a right one past the indent for the number): the number may go past the
// right indent to its stop, but the title wraps at the indent as any text does.
void FlowTests::theTitleOfAnEntryWithANumberPastTheRightIndentWrapsAtTheIndent()
{
    auto layout = [&](const QString &title, double *titleRight, int *lines) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        bf.setRightMargin(36);
        bf.setTabPositions({QTextOption::Tab(36, QTextOption::LeftTab), QTextOption::Tab(300, QTextOption::RightTab)});
        c.setBlockFormat(bf);
        const QString text = QStringLiteral("1.1\t") + title + QStringLiteral("\t12");
        c.insertText(text, run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(300, 600)}, env);
        *lines = int(lay.lineBriefs().size());
        double right = 0;
        for (const QRectF &r : lay.rangeRects(0, 4, 4 + int(title.size()))) right = std::max(right, r.right());
        *titleRight = right;
        return lay.lineBriefs();
    };
    // A title that, after the first tab, ends between the right indent (264) and the stop (300) when it is not wrapped.
    QString title;
    double wide = 0;
    int lines = 0;
    for (int words = 4; words < 30; ++words) {
        title += QStringLiteral("word ");
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        c.insertText(title.trimmed(), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(2000, 100)}, env);
        const auto rects = lay.rangeRects(0, 0, int(title.trimmed().size()));
        wide = rects.isEmpty() ? 0 : rects.last().right();
        if (36 + wide > 272) break;
    }
    QVERIFY2(36 + wide > 272 && 36 + wide < 296, qPrintable(QString::number(36 + wide)));
    double right = 0;
    layout(title.trimmed(), &right, &lines);
    QVERIFY2(right <= 264.5, qPrintable(QStringLiteral("the title reaches %1, the right indent is at 264").arg(right)));
    QVERIFY(lines >= 2);
    // A title that fits keeps the number on its line, at the stop.
    double shortRight = 0;
    int shortLines = 0;
    const auto briefs = layout(QStringLiteral("a short title"), &shortRight, &shortLines);
    QCOMPARE(shortLines, 1);
    QCOMPARE(briefs[0].end - briefs[0].pos, int(QStringLiteral("1.1\ta short title\t12").size()));
}


// A picture beside an entry is not the right indent: a stop between the picture and the indent takes no text over the picture.
void FlowTests::aRightStopBeyondAPictureTakesNoTextOverIt()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setRightMargin(36);
    bf.setTabPositions({QTextOption::Tab(250, QTextOption::RightTab)});
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("Entry\t12"), run);
    LayoutEnv env;
    env.wordLines = true;
    FrameSpec fs = plainFrame(300, 600);
    const QRectF picture(200, 0, 100, 40);
    fs.obstacles << QPolygonF(picture);
    StoryLayout lay;
    lay.build(&doc, {fs}, env);
    const auto rects = lay.rangeRects(0, 0, int(QStringLiteral("Entry\t12").size()));
    QVERIFY(!rects.isEmpty());
    for (const QRectF &r : rects) QVERIFY2(!r.intersects(picture.adjusted(0.5, 0.5, -0.5, -0.5)), qPrintable(audit::rectText(r)));
}


// A word processor's hanging first line goes out into the margin as far as the indents say: a paragraph with no left indent that hangs by 36 points begins 36 left of the
// text area.
void FlowTests::aHangingFirstLineGoesOutIntoTheMargin()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setLeftMargin(0);
    bf.setTextIndent(-36);
    c.setBlockFormat(bf);
    QString words;
    for (int i = 0; i < 30; ++i) words += QStringLiteral("word ");
    c.insertText(words, run);
    StoryLayout lay;
    LayoutEnv env;
    env.wordLines = true;
    FrameSpec fs = plainFrame(300, 600);
    lay.build(&doc, {fs}, env);
    const auto lines = lay.lineBriefs();
    QVERIFY(lines.size() >= 3);
    QVERIFY2(std::abs(lines[0].rect.left() + 36) < 0.01, qPrintable(QString::number(lines[0].rect.left())));
    QVERIFY2(std::abs(lines[1].rect.left()) < 0.01, qPrintable(QString::number(lines[1].rect.left())));
}


// A hanging first line goes out into the margin as far as the indents say, and no farther than the page is wide: an indent a file states as a million points is not a line a million
// points from the text.
void FlowTests::aHangingFirstLineGoesOutNoFartherThanThePageIsWide()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    QTextCursor c(fd.story());
    QTextBlockFormat bf;
    bf.setLeftMargin(0);
    bf.setTextIndent(-1e6);
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("words that hang a great way out into the margin and then are set on lines of the paragraph, which has several of them and goes on and on"));
    Paginator pg(&fd);
    const auto lines = pg.lines(0);
    QVERIFY(!lines.isEmpty());
    const double page = fd.section(0).pageSize().width();
    QVERIFY2(lines[0].rect.left() >= -page - 0.01, qPrintable(QString::number(lines[0].rect.left())));
    QVERIFY2(lines[0].rect.left() < -page + 1, "the line is as far out as the page is wide");
    LayoutCache cache;
    QVERIFY2(materializedDiffers(fd, pg, 0, cache).isEmpty(), qPrintable(materializedDiffers(fd, pg, 0, cache)));
}


// A right tab stop that is past the right indent and inside the column still takes the text after its tab (the page number of an entry in a table of contents): the line is
// not broken before the tab.
void FlowTests::aRightTabPastTheRightIndentKeepsItsText()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setRightMargin(36);
    bf.setTabPositions({QTextOption::Tab(300, QTextOption::RightTab)});
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("An entry of the table\t12"), run);
    StoryLayout lay;
    LayoutEnv env;
    env.wordLines = true;
    lay.build(&doc, {plainFrame(300, 600)}, env);
    const auto lines = lay.lineBriefs();
    QCOMPARE(lines.size(), 1);   // (the number is on the line of its entry)
    QCOMPARE(lines[0].end - lines[0].pos, int(doc.characterCount() - 1));
}


// A multiple of single spacing adds its room under the line's text: the last line of a column needs the text's height there, not the whole, and a line whose text fits
// is the column's though the room under it hangs past its bottom.
void FlowTests::aLineOfDoubleSpacingFitsByItsTextNotItsRoomUnderIt()
{
    const double single = (1825 + 443 + 87) / 2048.0 * 12;
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("one") + QChar(QChar::LineSeparator) + QStringLiteral("two") + QChar(QChar::LineSeparator) + QStringLiteral("three"), run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    // Room for the first line whole and the second line's text, not its room under.
    lay.build(&doc, {plainFrame(300, 2 * single + single + 0.5), plainFrame(300, 500)}, env);
    const auto lines = lay.lineBriefs();
    QCOMPARE(lines.size(), 3);
    QCOMPARE(lines[0].frame, 0);
    QCOMPARE(lines[1].frame, 0);   // (its text fits)
    QCOMPARE(lines[2].frame, 1);
    QVERIFY2(std::abs(lines[1].rect.height() - 2 * single) < 0.01, qPrintable(QString::number(lines[1].rect.height())));   // (it is still a line of double spacing)
}


// An empty paragraph after a frame whose paragraph mark is hidden keeps its line: the frame is set apart from the text, so the mark joins nothing (a tracked deletion's mark joins
// the paragraph that follows it into one: that is another paragraph, one of the text).
void FlowTests::anEmptyParagraphAfterAFramedParagraphWithAHiddenMarkKeepsItsLine()
{
    auto lines = [](bool framed) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        if (framed) bf.setProperty(tp::OutOfLayout, true);
        c.setBlockFormat(bf);
        QTextCharFormat mark = run;
        mark.setProperty(tp::HiddenText, true);
        c.setBlockCharFormat(mark);
        c.insertText(QStringLiteral("a paragraph"), run);
        c.insertBlock(QTextBlockFormat(), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(300, 600)}, env);
        return lay.lineBriefs();
    };
    const auto text = lines(false), frame = lines(true);
    QCOMPARE(text.size(), 1);    // (the paragraph's text, and the empty one that is the rest of it: no line of its own)
    QCOMPARE(frame.size(), 1);   // (the frame has no line in the text, and the empty paragraph after it has its own)
    QVERIFY(frame[0].rect.height() > 10);
    QCOMPARE(frame[0].pos, int(QString(QStringLiteral("a paragraph")).size()) + 1);
}


// A word processor's note that does not fit whole under its line at the foot of a page is cut there when room for three of its lines is left under the line (a page is
// not cut for want of that): the line stays and the rest of the note goes on at the next page. With less room the line goes on to the next page with its note whole. A note
// whose first paragraph has no widow and orphan control is cut where one of its lines fits, and goes on whole to the next page with its line only when not even one does.
void FlowTests::aNoteThatDoesNotFitIsCutWhenThreeOfItsLinesDo()
{
    for (const bool controlled : {true, false}) {
    TestNotes notes;
    auto note = std::make_unique<QTextDocument>();
    {
        QTextCursor nc(note.get());
        QTextBlockFormat noteFormat;
        if (controlled) noteFormat.setProperty(tp::WidowControl, true);
        nc.setBlockFormat(noteFormat);
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Times New Roman")});
        cf.setFontPointSize(10);
        nc.insertText(QStringLiteral("one") + QChar(QChar::LineSeparator) + QStringLiteral("two") + QChar(QChar::LineSeparator) + QStringLiteral("three") + QChar(QChar::LineSeparator) +
                          QStringLiteral("four") + QChar(QChar::LineSeparator) + QStringLiteral("five") + QChar(QChar::LineSeparator) + QStringLiteral("six"),
                      cf);
    }
    notes.docs[QStringLiteral("n1")] = std::move(note);
    auto layout = [&](double height) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("one") + QChar(QChar::LineSeparator) + QStringLiteral("two") + QChar(QChar::LineSeparator) + QStringLiteral("three") + QChar(QChar::LineSeparator) + QStringLiteral("four"), run);
        QTextCharFormat ref = run;
        ref.setProperty(tp::Field, QStringLiteral("footnote:n1"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), ref);
        LayoutEnv env;
        env.wordLines = true;
        auto frame = [&](double h) { FrameSpec fs = plainFrame(300, h); fs.ctx.doc = &notes; return fs; };
        StoryLayout lay;
        lay.build(&doc, {frame(height), frame(300)}, env);
        const auto lines = lay.lineBriefs();
        struct Out { int lastLineFrame = -9, noteFrame = -9; bool cut = false; };
        Out out;
        if (lines.size() == 4) out.lastLineFrame = lines.last().frame;
        if (lay.notes().size() == 1) {
            out.noteFrame = lay.notes().first().frame;
            out.cut = !lay.notes().first().more.isEmpty();
        }
        return out;
    };
    const auto room = layout(100), tight = layout(93), least = layout(60);
    QCOMPARE(room.lastLineFrame, 0);
    QCOMPARE(room.noteFrame, 0);
    QVERIFY2(room.cut, "the note is cut at the foot of the page");
    QCOMPARE(tight.lastLineFrame, controlled ? 1 : 0);
    QCOMPARE(tight.noteFrame, controlled ? 1 : 0);
    QCOMPARE(tight.cut, !controlled);
    QCOMPARE(least.lastLineFrame, 1);
    QVERIFY(!least.cut);
    }
}


// A line that cites a note whole at the foot of a page, in a file that asks for the older word processor's spacing (w:suppressTopSpacingWP, LayoutEnv::wordWholePointLines), is on
// the page when the note's text fits, though the space after the note's last paragraph does not: that space hangs below the foot. (The least room for the line and its note is the
// same with and without that space; a note before it that is not the last has its space. A file that does not ask has the space counted.)
void FlowTests::theSpaceAfterTheLastNoteOnAPageMayHangBelowTheFoot()
{
    // The least height of a frame that has the paragraph's last line on it, when the line cites a note of one line with `after` points after it (and, with `before`, an earlier
    // line cites a note of the same kind).
    auto leastHeight = [&](double after, bool before, bool older = true) {
        TestNotes notes;
        auto note = std::make_unique<QTextDocument>();
        {
            QTextCursor nc(note.get());
            QTextBlockFormat nf;
            nf.setBottomMargin(after);
            nc.setBlockFormat(nf);
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Times New Roman")});
            cf.setFontPointSize(10);
            nc.insertText(QStringLiteral("one"), cf);
        }
        notes.docs[QStringLiteral("n1")] = std::move(note);
        auto onFirst = [&](double height) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            QTextCharFormat ref = run;
            ref.setProperty(tp::Field, QStringLiteral("footnote:n1"));
            QTextBlockFormat bf;
            c.setBlockFormat(bf);
            c.insertText(QStringLiteral("one"), run);
            if (before) c.insertText(QString(QChar::ObjectReplacementCharacter), ref);
            c.insertText(QString(QChar::LineSeparator) + QStringLiteral("two") + QChar(QChar::LineSeparator) + QStringLiteral("three"), run);
            c.insertText(QString(QChar::ObjectReplacementCharacter), ref);
            LayoutEnv env;
            env.wordLines = true;
            env.wordWholePointLines = older;
            FrameSpec fs = plainFrame(300, height);
            fs.ctx.doc = &notes;
            FrameSpec rest = plainFrame(300, 300);
            rest.ctx.doc = &notes;
            StoryLayout lay;
            lay.build(&doc, {fs, rest}, env);
            const auto lines = lay.lineBriefs();
            return !lines.isEmpty() && lines.last().frame == 0;
        };
        double low = 20, high = 400;
        for (int i = 0; i < 40; ++i) {
            const double mid = (low + high) / 2;
            (onFirst(mid) ? high : low) = mid;
        }
        return high;
    };
    QVERIFY2(std::abs(leastHeight(10, false) - leastHeight(0, false)) < 0.5, qPrintable(QStringLiteral("%1 with space after the note, %2 without").arg(leastHeight(10, false)).arg(leastHeight(0, false))));
    QVERIFY2(leastHeight(10, false, false) - leastHeight(0, false, false) > 9.5, qPrintable(QStringLiteral("%1 with space after the note, %2 without").arg(leastHeight(10, false, false)).arg(leastHeight(0, false, false))));
    QVERIFY2(leastHeight(10, true) - leastHeight(0, true) > 9.5, qPrintable(QStringLiteral("%1 with space after the notes, %2 without").arg(leastHeight(10, true)).arg(leastHeight(0, true))));
}


// A note of a file with a notice that it goes on (the room for the notice is kept under the notes) and the older word processor's spacing (w:suppressTopSpacingWP) is cut where two
// of its lines fit under the line that cites it, not three, when the file has widow and orphan control: the notice is the third. A note of fewer than four lines is not cut at all then (a line of it would be left alone on a page), and goes
// whole to the next page with its line.
void FlowTests::aNoteWithAWidowControlIsCutAfterTwoLinesAndNeverLeavesOne()
{
    // The least height of a frame with the paragraph's last line on it, the line citing a note of `lines` lines, the file's notice `notice` high; and whether that note was cut.
    auto least = [&](int lines, double notice, bool *cut) {
        TestNotes notes;
        notes.notice = notice;
        auto note = std::make_unique<QTextDocument>();
        {
            QTextCursor nc(note.get());
            QTextBlockFormat nf;
            nf.setProperty(tp::WidowControl, true);
            nc.setBlockFormat(nf);
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Times New Roman")});
            cf.setFontPointSize(10);
            QString text = QStringLiteral("one");
            for (int i = 1; i < lines; ++i) text += QChar(QChar::LineSeparator) + QStringLiteral("line");
            nc.insertText(text, cf);
        }
        notes.docs[QStringLiteral("n1")] = std::move(note);
        auto onFirst = [&](double height, bool *wasCut) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(12);
            QTextBlockFormat bf;
            c.setBlockFormat(bf);
            c.insertText(QStringLiteral("one") + QChar(QChar::LineSeparator) + QStringLiteral("two") + QChar(QChar::LineSeparator) + QStringLiteral("three"), run);
            QTextCharFormat ref = run;
            ref.setProperty(tp::Field, QStringLiteral("footnote:n1"));
            c.insertText(QString(QChar::ObjectReplacementCharacter), ref);
            LayoutEnv env;
            env.wordLines = true;
            env.wordWholePointLines = true;
            FrameSpec fs = plainFrame(300, height);
            fs.ctx.doc = &notes;
            FrameSpec rest = plainFrame(300, 300);
            rest.ctx.doc = &notes;
            StoryLayout lay;
            lay.build(&doc, {fs, rest}, env);
            const auto briefs = lay.lineBriefs();
            if (wasCut) *wasCut = lay.notes().size() == 1 && !lay.notes().first().more.isEmpty();
            return !briefs.isEmpty() && briefs.last().frame == 0;
        };
        double low = 20, high = 400;
        for (int i = 0; i < 40; ++i) {
            const double mid = (low + high) / 2;
            (onFirst(mid, nullptr) ? high : low) = mid;
        }
        onFirst(high, cut);
        return high;
    };
    bool cutWith = false, cutWithout = false, cutShort = false, cutShortWhole = false;
    const double with = least(6, 10, &cutWith), without = least(6, 0, &cutWithout);
    QVERIFY2(cutWith, "a note of six lines is cut where the room is the least that the line and two lines of the note ask for");
    QVERIFY2(with < without, qPrintable(QStringLiteral("%1 high with the notice, %2 without").arg(with).arg(without)));
    // A note of three lines is whole where it is on the page, whatever the room.
    least(3, 10, &cutShort);
    QVERIFY(!cutShort);
    Q_UNUSED(cutShortWhole);
}


// The widow and orphan control that counts in cutting a note is that of the paragraph the cut is in, not the note's first: a note whose first paragraph has none and whose second has it
// is never cut so as to leave one line of the second on a page (in a frame of any height).
void FlowTests::aNoteIsCutByTheControlOfTheParagraphItIsCutIn()
{
    TestNotes notes;
    auto note = std::make_unique<QTextDocument>();
    {
        QTextCursor nc(note.get());
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Times New Roman")});
        cf.setFontPointSize(10);
        QTextBlockFormat first;   // (no widow and orphan control)
        nc.setBlockFormat(first);
        nc.insertText(QStringLiteral("first"), cf);
        QTextBlockFormat second;
        second.setProperty(tp::WidowControl, true);
        nc.insertBlock(second);
        nc.insertText(QStringLiteral("a") + (QChar(QChar::LineSeparator) + QStringLiteral("b")).repeated(5), cf);
    }
    notes.docs[QStringLiteral("n1")] = std::move(note);
    int cuts = 0;
    for (double height = 70; height < 130; height += 0.5) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("one") + QChar(QChar::LineSeparator) + QStringLiteral("two") + QChar(QChar::LineSeparator) + QStringLiteral("three"), run);
        QTextCharFormat ref = run;
        ref.setProperty(tp::Field, QStringLiteral("footnote:n1"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), ref);
        LayoutEnv env;
        env.wordLines = true;
        FrameSpec fs = plainFrame(300, height);
        fs.ctx.doc = &notes;
        FrameSpec rest = plainFrame(300, 300);
        rest.ctx.doc = &notes;
        StoryLayout lay;
        lay.build(&doc, {fs, rest}, env);
        if (lay.notes().size() != 1 || lay.notes().first().more.isEmpty() || !lay.notes().first().layout) continue;
        const auto lines = lay.notes().first().layout->lineInfo(0);
        int ofSecond = 0;
        for (const auto &l : lines) ofSecond += l.docStart > 0;
        if (ofSecond == 0) continue;   // (cut at the end of the first paragraph)
        ++cuts;
        QVERIFY2(ofSecond >= 2, qPrintable(QStringLiteral("a frame %1 high has %2 line of the second paragraph at its foot").arg(height).arg(ofSecond)));
    }
    QVERIFY2(cuts > 0, "the note is cut inside its second paragraph in some frame");
}


// The part of a note that goes on from the page before is set under the file's own separator for it, when the file has one: the room above it is that paragraph's height (and the
// rule is drawn in it), not the separator's of the first part.
void FlowTests::theContinuedPartOfANoteHasTheRoomOfTheFilesOwnSeparatorForIt()
{
    auto layout = [&](double continuation) {
        TestNotes notes;
        notes.separator = 12;
        notes.continuation = continuation;
        auto note = std::make_unique<QTextDocument>();
        {
            QTextCursor nc(note.get());
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Times New Roman")});
            cf.setFontPointSize(10);
            nc.insertText(QStringLiteral("one") + QChar(QChar::LineSeparator) + QStringLiteral("two") + QChar(QChar::LineSeparator) + QStringLiteral("three") + QChar(QChar::LineSeparator) +
                              QStringLiteral("four") + QChar(QChar::LineSeparator) + QStringLiteral("five") + QChar(QChar::LineSeparator) + QStringLiteral("six"),
                          cf);
        }
        notes.docs[QStringLiteral("n1")] = std::move(note);
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        c.setBlockFormat(QTextBlockFormat());
        c.insertText(QStringLiteral("one") + QChar(QChar::LineSeparator) + QStringLiteral("two") + QChar(QChar::LineSeparator) + QStringLiteral("three") + QChar(QChar::LineSeparator) + QStringLiteral("four"), run);
        QTextCharFormat ref = run;
        ref.setProperty(tp::Field, QStringLiteral("footnote:n1"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), ref);
        LayoutEnv env;
        env.wordLines = true;
        auto frame = [&](double h) { FrameSpec fs = plainFrame(300, h); fs.ctx.doc = &notes; return fs; };
        StoryLayout lay;
        lay.build(&doc, {frame(108), frame(300)}, env);
        struct Out { bool ok = false; double ruleToPart = 0, reserve = 0; } out;
        if (lay.notes().size() != 1 || lay.notes().first().more.isEmpty()) return out;
        const auto &part = lay.notes().first().more.first();
        for (const auto &rule : lay.noteRules())
            if (rule.frame == part.frame) {
                out.ok = true;
                out.ruleToPart = part.rect.top() - rule.line.y1();
                out.reserve = lay.noteReserves().value(part.frame);
            }
        return out;
    };
    const auto none = layout(0), own = layout(40);
    QVERIFY2(none.ok && own.ok, "the note is cut, and its rest has a rule");
    // The rule is three fifths down the paragraph above the part: two fifths of it above the part. Without the file's own, that paragraph is the separator's (twelve points).
    QVERIFY2(std::abs(none.ruleToPart - 0.4 * 12) < 0.05, qPrintable(QString::number(none.ruleToPart)));
    QVERIFY2(std::abs(own.ruleToPart - 0.4 * 40) < 0.05, qPrintable(QString::number(own.ruleToPart)));
    QVERIFY2(std::abs((own.reserve - none.reserve) - (40 - 12)) < 0.05, qPrintable(QString::number(own.reserve - none.reserve)));
}


// A line beside an object that the engine moves down in steps (one that is not a rectangle) is measured once, not at each step: the work for a paragraph of many runs does not grow
// with the height of the object. (Counted: how many times a line's own height was measured, which a line gone round at once and one moved down by steps need alike.)
void FlowTests::aLineBelowATallObjectCostsNothingForEachStep()
{
    auto build = [&](bool plain) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat a, b;
        a.setFontFamilies({QStringLiteral("Times New Roman")});
        a.setFontPointSize(12);
        b = a;
        b.setFontItalic(true);
        for (int i = 0; i < 600; ++i) c.insertText(QStringLiteral("w "), i % 2 ? a : b);
        FrameSpec fs = plainFrame(300, 100000);
        // (A tall triangle across the column: a line meets it from the top to the foot, by steps of two points; a rectangle of the same height is gone round at once.)
        fs.obstacles << (plain ? QPolygonF(QRectF(-1e5, 0, 2e5, 300)) : QPolygonF({QPointF(-1e5, 0), QPointF(1e5, 0), QPointF(1e5, 300), QPointF(-1e5, 301)}));
        LayoutEnv env;
        env.wordLines = true;
        const qint64 before = StoryLayout::lineHeightsMeasuredForTest();
        StoryLayout lay;
        lay.build(&doc, {fs}, env);
        return StoryLayout::lineHeightsMeasuredForTest() - before;
    };
    const qint64 rectangle = build(true), stepped = build(false);
    QVERIFY2(rectangle > 0, qPrintable(QString::number(rectangle)));
    QVERIFY2(stepped <= 2 * rectangle + 10, qPrintable(QStringLiteral("a line below a tall object that is not a rectangle was measured %1 times, below a rectangle %2").arg(stepped).arg(rectangle)));
}


// A line that a paragraph goes on with, in smaller type than its first run, is checked against the objects at the height its own text has, not the first run's: an object
// that begins below the line (and above where the big type's line would end) is no wall for it.
void FlowTests::aLineInSmallTypeIsNotMovedByAnObjectBelowIt()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat big, small;
    big.setFontFamilies({QStringLiteral("Times New Roman")});
    big.setFontPointSize(30);
    small = big;
    small.setFontPointSize(10);
    c.insertText(QStringLiteral("Big "), big);
    QString words;
    for (int i = 0; i < 60; ++i) words += QStringLiteral("small ");
    c.insertText(words, small);
    auto lines = [&](double wallTop) {
        FrameSpec fs = plainFrame(300, 600);
        if (wallTop > 0) fs.obstacles << QPolygonF(QRectF(-1e5, wallTop, 2e5, 50));
        StoryLayout lay;
        LayoutEnv env;
        env.wordLines = true;
        lay.build(&doc, {fs}, env);
        return lay.lineBriefs();
    };
    const auto free = lines(0);
    QVERIFY(free.size() >= 3);
    QVERIFY(free[1].rect.height() < 14);   // (the second line is in small type: a line of its own height)
    // The object begins just below that line's own bottom.
    const double wallTop = free[1].rect.bottom() - free[1].below + 1;
    QVERIFY(wallTop < free[1].rect.top() + free[0].rect.height());   // (the big type's line would reach it)
    const auto walled = lines(wallTop);
    QCOMPARE(walled.size(), free.size());
    QCOMPARE(walled[1].rect.top(), free[1].rect.top());
    QCOMPARE(walled[1].pushed, 0.0);
}


// A section that is continuous and has the same page as the one before goes on the page the one before ends on; one with another page, or that is not
// continuous, begins a page of its own.
void FlowTests::continuousSectionsShareThePage()
{
    auto pagesOf = [&](bool continuous, bool samePage, bool otherMargins = false) {
        FlowDocument fd;
        fillStory(fd.story(), 6, 3, [](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(6);
            if (i == 2) bf.setProperty(flow::fp::SectionBreakAfter, true);
        });
        Section second = fd.sections[0];
        second.continuous = continuous;
        if (!samePage) second.paper = QSizeF(595, 842);
        if (otherMargins) second.margins = QMarginsF(100, 72, 72, 72);
        fd.sections << second;
        Paginator pg(&fd);
        // (The section of the page each paragraph is on is the section the page starts in, and the first page's is the first.)
        QVector<int> sections;
        for (int k = 0; k < pg.pageCount(); ++k) sections << pg.page(k).start.section;
        return std::make_pair(pg.pageCount(), sections);
    };
    const auto apart = pagesOf(false, true), joined = pagesOf(true, true), other = pagesOf(true, false), margins = pagesOf(true, true, true);
    QCOMPARE(apart.first, 2);
    QCOMPARE(joined.first, 1);
    QCOMPARE(other.first, 2);   // (a page of another size can't be shared)
    QCOMPARE(margins.first, 1);   // (one with other margins can)
    QCOMPARE(apart.second.last(), 1);
}


// A continuous section after a section of columns begins below the columns, which are as even as the lines let them be (the first column is not filled to the foot of the
// page): the second section's text is below the lowest of them.
void FlowTests::aContinuousSectionAfterColumnsBeginsBelowTheBalancedColumns()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 4, 9, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(6);
        if (i == 2) bf.setProperty(fp::SectionBreakAfter, true);
    }, 30, 40);
    Section first = fd.sections[0];
    first.columns = 2;
    first.columnGap = 20;
    fd.sections[0] = first;
    Section second = fd.sections[0];
    second.continuous = true;
    second.columns = 1;
    fd.sections << second;
    Paginator pg(&fd);
    QCOMPARE(pg.pageCount(), 1);
    const auto lines = pg.lines(0);
    double bottom[3] = {0, 0, 0}, tallest = 0;
    int count[3] = {0, 0, 0};
    for (const auto &l : lines) {
        QVERIFY(l.frame >= 0 && l.frame < 3);
        ++count[l.frame];
        if (l.frame < 2) bottom[l.frame] = std::max(bottom[l.frame], l.rect.bottom());
        tallest = std::max(tallest, l.rect.height());
    }
    QVERIFY2(count[0] > 0 && count[1] > 0 && count[2] > 0, qPrintable(QStringLiteral("lines in the columns: %1, %2, then %3").arg(count[0]).arg(count[1]).arg(count[2])));
    QVERIFY2(std::abs(bottom[0] - bottom[1]) <= 2 * tallest, qPrintable(QStringLiteral("the columns end at %1 and %2").arg(bottom[0]).arg(bottom[1])));
    // (The second section's box, in page coordinates, begins below the text area's top by where the columns end.)
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QCOMPARE(int(page->boxes.size()), 3);
    const auto *box = dynamic_cast<const TextItem *>(page->doc->item(page->boxes[2]));
    QVERIFY(box);
    const double areaTop = pg.page(0).content.top();
    QVERIFY2(box->rect.top() >= areaTop + std::max(bottom[0], bottom[1]) - 0.01, qPrintable(QStringLiteral("the second section begins at %1, the columns end at %2").arg(box->rect.top() - areaTop).arg(std::max(bottom[0], bottom[1]))));
    QVERIFY2(box->rect.top() - areaTop - std::max(bottom[0], bottom[1]) < 40, "the second section follows the columns closely");
    // Columns with no lines have no box, and the boxes after them are the page's still: a section of two columns of a line, then the second section.
    FlowDocument few;
    few.env.wordLines = true;
    fillStory(few.story(), 3, 9, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(6);
        if (i == 0) bf.setProperty(fp::SectionBreakAfter, true);
    }, 3, 4);
    few.sections[0].columns = 2;
    few.sections[0].columnGap = 20;
    Section after = few.sections[0];
    after.continuous = true;
    after.columns = 1;
    few.sections << after;
    Paginator pf(&few);
    QCOMPARE(pf.pageCount(), 1);
    const auto lonely = pf.materialize(0);
    QVERIFY(lonely);
    QCOMPARE(int(lonely->boxes.size()), 2);   // (the first column of the first section, and the second section's)
    const auto *lastBox = dynamic_cast<const TextItem *>(lonely->doc->item(lonely->boxes[1]));
    QVERIFY(lastBox && lastBox->rect.height() > 10);
}


// A continuous section with more columns than the one before begins below it on the same page, with its own columns.
void FlowTests::continuousSectionWithOtherColumnsBeginsBelow()
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
void FlowTests::paginatorFindsThePageOfAPosition()
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
void FlowTests::paginatorLaysOutMoreThanTwoThousandPages()
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
void FlowTests::aTableIsLaidOutInRowsBetweenTheTextAroundIt()
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
