#include "tst_flow.h"


// A paragraph that goes below a table set at a place (the table is as wide as the column, so there is no room beside it) has the table's room, and its space before under it:
// the paragraph's first line is the space before below the table's foot, as it is below the paragraph before it.
void FlowTests::aParagraphBelowAFloatingTableKeepsItsSpaceBefore()
{
    auto fd = std::make_unique<FlowDocument>();
    fd->env.wordLines = true;
    fillStory(fd->story(), 4, 17, [](int n, QTextBlockFormat &bf) { if (n == 1) bf.setTopMargin(20); }, 3, 6);
    TestTable spec;
    spec.rows = 2;
    spec.cols = 2;
    spec.columnWidth = 234;   // (the text area is 468 wide)
    spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.y = 0; t.place = p; };
    addTableAfter(*fd, 0, spec);
    Paginator pg(fd.get());
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QRectF table;
    for (const auto &t : page->tables)
        if (t->floating) table = t->rect();
    QVERIFY(!table.isNull());
    int paragraph = -1;
    for (QTextBlock b = fd->story()->begin(); b.isValid(); b = b.next())
        if (b.blockFormat().topMargin() == 20) paragraph = b.position();
    double top = -1;
    for (const auto &l : pg.lines(0))
        if (l.line == 0 && l.block == paragraph) top = pg.page(0).content.top() + l.rect.top();
    QVERIFY(top > 0);
    QVERIFY2(std::abs(top - (table.bottom() + 20)) < 0.01, qPrintable(QStringLiteral("the paragraph's first line is at %1, the table ends at %2").arg(top).arg(table.bottom())));
    // (The page layer, which sets the text again from the walls the page has, keeps that room under the table too.)
    const auto walls = audit::wallsOf(*page);
    QVERIFY(!walls.isEmpty());
    QVERIFY2(std::abs(walls.first().rect.bottom() - (table.bottom() + 20)) < 0.01, qPrintable(audit::rectText(walls.first().rect)));
}


// An object set from its paragraph (down from the paragraph's top) is set from the paragraph's top above the space before it, as a table is: the room the paragraph leaves above
// its first line is not in the offset.
void FlowTests::anObjectSetFromAParagraphIsSetAboveTheSpaceBeforeIt()
{
    auto fd = std::make_unique<FlowDocument>();
    fd->env.wordLines = true;
    // (The paragraph before it has 8 points after it, which the space before takes in: of the 20 before, 12 are above the first line.)
    fillStory(fd->story(), 6, 41, [](int n, QTextBlockFormat &bf) { if (n == 2) bf.setTopMargin(20); if (n == 1) bf.setBottomMargin(8); }, 3, 6);
    int paragraph = -1;
    for (QTextBlock b = fd->story()->begin(); b.isValid(); b = b.next())
        if (b.blockFormat().topMargin() == 20) paragraph = b.position();
    QVERIFY(paragraph > 0);
    // (Behind the text: no wall, so no line is moved.)
    const int id = addWrapped(fd.get(), 40, 30, paragraph + 1, FloatingObject::Wrap::Square, 0, 5, QMarginsF(0, 0, 0, 0), FloatingObject::Side::Both, FloatingObject::Rel::Column, FloatingObject::Rel::Paragraph);
    fd->objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
    Paginator pg(fd.get());
    const auto page = pg.materialize(0);
    QVERIFY(page);
    const Item *shape = objectOfSize(*page, 40, 30);
    QVERIFY(shape);
    double natural = -1;
    for (const auto &l : pg.lines(0))
        if (l.line == 0 && l.block == paragraph) natural = l.rect.top() - l.pushed;
    QVERIFY(natural > 0);
    const double top = pg.page(0).content.top() + natural - 12 + 5;
    QVERIFY2(std::abs(shape->rect.top() - top) < 0.01, qPrintable(QStringLiteral("the object is at %1, the paragraph's top and the offset say %2").arg(shape->rect.top()).arg(top)));
}


// An object is set from the top of its paragraph above the space before it that was kept: at the top of a page that the paragraph opens by the text's own flow the space before
// is not kept, and the paragraph's top is where its first line is, not the room above it that is not there.
void FlowTests::anObjectSetFromAParagraphAtTheTopOfAPageIsSetFromItsFirstLine()
{
    auto layout = [&](int firstOfSecondPage, QRectF *shape, double *firstLine, int *paragraph) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fillStory(fd->story(), 80, 23, [&](int n, QTextBlockFormat &bf) { if (n == firstOfSecondPage) bf.setTopMargin(20); }, 10, 30);
        Paginator pg(fd.get());
        if (pg.pageCount() < 2) return -1;
        const int at = pg.page(1).start.pos;
        QTextBlock b = fd->story()->findBlock(at);
        int number = 0;
        for (QTextBlock x = fd->story()->begin(); x.isValid() && x != b; x = x.next()) ++number;
        if (!shape) return number;
        const int id = addWrapped(fd.get(), 40, 30, b.position() + 1, FloatingObject::Wrap::Square, 0, 5, QMarginsF(0, 0, 0, 0), FloatingObject::Side::Both, FloatingObject::Rel::Column, FloatingObject::Rel::Paragraph);
        fd->objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
        Paginator laid(fd.get());
        const auto page = laid.materialize(1);
        if (!page || !laid.page(1).start.pos == at) return -2;
        const Item *item = objectOfSize(*page, 40, 30);
        if (!item) return -3;
        *shape = item->rect;
        *paragraph = number;
        for (const auto &l : laid.lines(1))
            if (l.block == b.position() && l.line == 0) *firstLine = laid.page(1).content.top() + l.rect.top() - l.pushed;
        return number;
    };
    // (The paragraph that opens the second page is found first; it is the one given the space before then.)
    const int opens = layout(-1, nullptr, nullptr, nullptr);
    QVERIFY(opens > 2);
    QRectF shape;
    double firstLine = -1;
    int paragraph = -1;
    QVERIFY(layout(opens, &shape, &firstLine, &paragraph) >= 0);
    QVERIFY(firstLine > 0);
    QVERIFY2(std::abs(shape.top() - (firstLine + 5)) < 0.01, qPrintable(QStringLiteral("the object is at %1, the first line is at %2 and the offset 5").arg(shape.top()).arg(firstLine)));
}


// And in a paragraph that goes on from the page before: its top there is its first line there, which has no space before.
void FlowTests::anObjectInTheRestOfAParagraphIsSetFromItsFirstLineOnThePage()
{
    auto fd = std::make_unique<FlowDocument>();
    fd->env.wordLines = true;
    QTextCursor c(fd->story());
    QTextBlockFormat bf;
    bf.setTopMargin(20);
    c.setBlockFormat(bf);
    c.insertText(QString(12000, QLatin1Char('x')).replace(QRegularExpression(QStringLiteral("(x{7})")), QStringLiteral("\\1 ")));
    Paginator probe(fd.get());
    QVERIFY(probe.pageCount() > 2);
    const int where = probe.page(1).start.pos + 40;   // (on the second page, in the rest of the paragraph)
    const int id = addWrapped(fd.get(), 40, 30, where, FloatingObject::Wrap::Square, 0, 5, QMarginsF(0, 0, 0, 0), FloatingObject::Side::Both, FloatingObject::Rel::Column, FloatingObject::Rel::Paragraph);
    fd->objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
    Paginator pg(fd.get());
    const int pageOf = pg.pageOfPosition(where);
    QCOMPARE(pageOf, 1);
    const auto page = pg.materialize(pageOf);
    QVERIFY(page);
    const Item *item = objectOfSize(*page, 40, 30);
    QVERIFY(item);
    const auto lines = pg.lines(pageOf);
    QVERIFY(!lines.isEmpty());
    const double firstLine = pg.page(pageOf).content.top() + lines.first().rect.top() - lines.first().pushed;
    QVERIFY2(std::abs(item->rect.top() - (firstLine + 5)) < 0.01, qPrintable(QStringLiteral("the object is at %1, the first line is at %2 and the offset 5").arg(item->rect.top()).arg(firstLine)));
}


// A table set at a place from its paragraph is set from the top of that paragraph, above the space before it: the offset does not count the room the paragraph leaves
// above its first line.
void FlowTests::aTableSetFromItsParagraphIsSetAboveTheSpaceBeforeIt()
{
    auto fd = std::make_unique<FlowDocument>();
    fd->env.wordLines = true;
    fillStory(fd->story(), 4, 17, [](int n, QTextBlockFormat &bf) { if (n == 1) bf.setTopMargin(20); }, 3, 6);
    TestTable spec;
    spec.rows = 2;
    spec.cols = 2;
    spec.columnWidth = 60;
    spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.y = 5; t.place = p; };
    addTableAfter(*fd, 0, spec);
    Paginator pg(fd.get());
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QRectF table;
    for (const auto &t : page->tables)
        if (t->floating) table = t->rect();
    QVERIFY(!table.isNull());
    // (The first line of the paragraph after the table, where the text would set it: the table is beside the text, which has room.)
    int paragraph = -1;
    for (QTextBlock b = fd->story()->begin(); b.isValid(); b = b.next())
        if (b.blockFormat().topMargin() == 20) paragraph = b.position();
    double natural = -1;
    for (const auto &l : pg.lines(0))
        if (l.line == 0 && l.block == paragraph) natural = l.rect.top() - l.pushed;
    QVERIFY(natural > 0);
    const double top = pg.page(0).content.top() + natural - 20 + 5;
    QVERIFY2(std::abs(table.top() - top) < 0.01, qPrintable(QStringLiteral("the table is at %1, the paragraph's top and the offset say %2").arg(table.top()).arg(top)));
}


// The same for a table set at a place: its pictures are in the page's document as well, and drawn with the table where it is.
void FlowTests::aPictureInACellOfAFloatingTableIsDrawnWithTheTable()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.images.push_back({tinyPng(6, 6, Qt::blue), QStringLiteral("png")});
    fillStory(fd.story(), 3, 12, {}, 3, 6);
    TestTable spec;
    spec.rows = 2;
    spec.cols = 2;
    spec.columnWidth = 150;
    spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.down = TableData::Place::From::Margin; p.y = 200; t.place = p; };
    const TestTable table = addTableAfter(fd, 0, spec);
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
    QVERIFY(page && page->tables.size() == 1 && page->tables[0]->floating);
    QCOMPARE(page->cellImages.size(), 1);
    const QString id = page->cellImages.value(QStringLiteral("flow-img:0"));
    QVERIFY(!id.isEmpty() && !id.startsWith(QStringLiteral("flow-img:")));
    QVERIFY(page->doc->images.contains(id));
}


// A picture set in the text of a cell is in the page's document and is drawn with the table, where its line is.
void FlowTests::aPictureInTheTextOfACellIsDrawnWithTheTable()
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
void FlowTests::repeatedHeaderRowsAreDecidedByTheRoomOfTheColumnTheTableIsIn()
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
void FlowTests::aTableIsIndentedToTheTextOfItsFirstCellOrCenteredOrRight()
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
void FlowTests::aTableFlowsFromColumnToColumn()
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


// A cell whose text is turned (from the top down or from the bottom up) lays its lines out along the row: the row is as tall as the text is long (up to a limit), the lines stack
// across the cell from the side the direction says, and the caret and a click in it are found through the turn.
void FlowTests::aCellWithTurnedTextHasItsRowAsTallAsTheTextIsLong()
{
    for (const CellDirection direction : {CellDirection::BottomToTop, CellDirection::TopToBottom}) {
        FlowDocument fd;
        fd.sections[0].paper = QSizeF(400, 400);
        fd.sections[0].margins = QMarginsF(36, 36, 36, 36);
        TestTable spec;
        spec.rows = 1;
        spec.cols = 2;
        spec.columnWidth = 60;
        spec.text = [](int, int c) { return c == 0 ? QStringLiteral("Turned text of some length") : QStringLiteral("x"); };
        spec.setCell = [&](int, int c, CellData &cell) {
            if (c == 0) cell.direction = direction;
        };
        const TestTable table = addTable(fd, spec);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page && !page->tables.isEmpty());
        const TableFragment &fragment = *page->tables[0];
        // (Across, the text would wrap in 60 points and make a row of about 60: turned, it is on one line along the row.)
        QVERIFY2(fragment.layout->rowHeight(0) > 100 && fragment.layout->rowHeight(0) < 260, qPrintable(QString::number(fragment.layout->rowHeight(0))));
        const int first = table.rowStarts[0];
        QLineF line;
        QVERIFY(fragment.caretLine(first, &line));
        QVERIFY2(std::abs(line.dx()) > std::abs(line.dy()) * 3, "the caret of turned text lies across the line");
        const QRectF all = fragment.rect();
        if (direction == CellDirection::BottomToTop) QVERIFY2(line.y1() > all.center().y(), "the text begins at the foot");
        else QVERIFY2(line.y1() < all.center().y(), "the text begins at the top");
        // A click on the caret finds the character.
        QLineF later;
        QVERIFY(fragment.caretLine(first + 5, &later));
        QVERIFY(std::abs(later.y1() - line.y1()) > 10);
        const int hit = fragment.positionAt(QPointF(later.center().x(), later.center().y()));
        QVERIFY2(hit >= first + 3 && hit <= first + 7, qPrintable(QString::number(hit - first)));
    }
}


// A file that says more than it can mean: cells that span further than the table has columns, a table that is a single row with a header, a header row taller than the
// page, and cells joined down that have nothing to be joined to. Every one lays out and ends.
void FlowTests::hostileTablesLayOutAndEnd()
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
void FlowTests::paginatorFollowsEditsInTablesAndMeasuresOnlyTheRowsTheyTouch()
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
void FlowTests::typingInALongTableIsFast()
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
    fresh();
    const int pages = pg->pageCount();
    QVERIFY(pages > 40);
    const qint64 wholePages = pg->pagesLaidOut(), wholeRows = pg->tableRowsMeasured();
    QTextCursor c(fd->story());
    c.setPosition(fd->story()->findBlock(table.rowStarts[2500]).position() + 1);
    const qint64 pagesBefore = pg->pagesLaidOut(), rowsBefore = pg->tableRowsMeasured();
    c.insertText(QStringLiteral("x"));
    pg->paginate(pg->pageOfPosition(c.position()) + 1);
    const qint64 typedPages = pg->pagesLaidOut() - pagesBefore, typedRows = pg->tableRowsMeasured() - rowsBefore;
    qInfo("a table of 3,000 rows: all of it laid out in %lld pages and %lld rows measured, a letter typed in row 2,500 in %lld pages and %lld rows", (long long)wholePages, (long long)wholeRows, (long long)typedPages, (long long)typedRows);
    QVERIFY2(wholeRows >= 3000, qPrintable(QString::number(wholeRows)));
    QVERIFY2(typedPages <= 8 && typedPages * 8 < wholePages, qPrintable(QStringLiteral("typing laid out %1 pages, the whole %2").arg(typedPages).arg(wholePages)));
    QVERIFY2(typedRows <= 5, qPrintable(QStringLiteral("typing measured %1 rows, the whole %2").arg(typedRows).arg(wholeRows)));
}


// ---- the pages that are looked at ----

// A page made into a document of its own is laid out by the page layer in the lines the paginator found: whole paragraphs, the rest of
// one that began on the page before, and the beginning of one that goes on, in one column and in two.
void FlowTests::materializedPagesHaveTheLinesThePaginatorFound()
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
void FlowTests::materializedPagesKeepListNumbers()
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
void FlowTests::materializedPagesHaveStableIds()
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
void FlowTests::materializedPagesPaint()
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
void FlowTests::aPageWithATableDrawsItAndKeepsThePositionsOfTheTextAroundIt()
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
void FlowTests::paginatorFollowsEditsAndLaysOutOnlyNearby()
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
void FlowTests::paginatorKeepsLinesTogether()
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
void FlowTests::paginatorMeetsTheSpeedTargets()
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
void FlowTests::paginatorSettlesAnEditAfterAPartialSettle()
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
void FlowTests::paginatorHandlesSpaceBeforeTallerThanAPage()
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
void FlowTests::paginatorKeepsLongChainsWithTheNext()
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
void FlowTests::paginatorMakesAPageAfterABreakAtTheEnd()
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
void FlowTests::paginatorLaysOutAHugeParagraph()
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
    const EngineWork before = EngineWork::now();
    auto made = std::make_unique<Paginator>(&fd);
    const int pages = made->pageCount();
    const EngineWork did = EngineWork::now().since(before);
    Paginator &pg = *made;
    qInfo("a paragraph of 900 KB: %d pages: %s", pages, qPrintable(did.text()));
    QVERIFY(pages > 150);
    // (A page costs a page: about one range of format for each, and none of the paragraph's characters read again for it.)
    QVERIFY2(did.ranges <= 2 * pages + 8 && did.chars <= pages && did.runs <= pages, qPrintable(did.text()));
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
void FlowTests::paginatorLaysOutHugeParagraphsInTime()
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
        const EngineWork before = EngineWork::now();
        auto made = std::make_unique<Paginator>(&fd);
        const int pages = made->pageCount();
        const EngineWork did = EngineWork::now().since(before);
        Paginator &pg = *made;
        qInfo("%s: %d pages: %s", what ? "a word of 200 KB" : "a paragraph of 2 MB", pages, qPrintable(did.text()));
        QVERIFY2(did.ranges <= 2 * pages + 8 && did.chars <= pages && did.runs <= pages, qPrintable(did.text()));
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
void FlowTests::paginatorRestOfAParagraphIsLaidOutAsTheWholeIs()
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
void FlowTests::paginatorGrowsTheWindowForTinyText()
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
void FlowTests::changingALaterSectionLaysOutOnlyItsPagesAgain()
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
void FlowTests::paginatorNumbersPagesBySection()
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
void FlowTests::aSectionCanStartItsPageNumbersAtZero()
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


// A continuous section that begins among the lines of a page counts that page as its first: the page keeps the number it had, and the next is one more than the number
// the section restarts at. One that begins at the top of a page (after a page break) restarts them there.
void FlowTests::aContinuousSectionRestartsTheNumbersFromThePageItBeginsOn()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 60, 5, [](int i, QTextBlockFormat &bf) {
        if (i == 0 || i == 20 || i == 40) bf.setProperty(flow::fp::SectionBreakAfter, true);
        if (i == 20) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
    }, 14, 20);
    fd.sections[0].paper = QSizeF(300, 300);
    fd.sections[0].margins = QMarginsF(50, 50, 50, 50);
    Section second = fd.sections[0], third = fd.sections[0], fourth = fd.sections[0];
    second.continuous = true;
    second.firstPageNumber = 5;   // (it begins below the first paragraph, in the middle of the first page: that page is 1, the next 6)
    third.continuous = true;
    third.firstPageNumber = 7;    // (it begins at the top of the page after a page break)
    fd.sections << second << third << fourth;
    Paginator pg(&fd);
    int firstOfThird = -1;
    for (int k = 0; k < pg.pageCount() && firstOfThird < 0; ++k)
        if (pg.page(k).start.section == 2) firstOfThird = k;
    QVERIFY(firstOfThird > 1);
    QCOMPARE(pg.page(0).number, 1);
    QCOMPARE(pg.page(1).number, 6);
    QCOMPARE(pg.page(2).number, 7);
    QCOMPARE(pg.page(firstOfThird - 1).number, 5 + firstOfThird - 1);
    QCOMPARE(pg.page(firstOfThird).number, 7);
    QCOMPARE(pg.page(firstOfThird + 1).number, 8);
    QCOMPARE(pg.materialize(1)->number, 6);
    QCOMPARE(pg.materialize(firstOfThird)->number, 7);
}


// A word processing document keeps pieces of its file as text that shows what the field names after "raw:", or nothing.
void FlowTests::aRawFieldShowsItsOwnText()
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
void FlowTests::paginatorSettlesSeveralEditsAtOnce()
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
void FlowTests::restartedNotesAreInTheOrderOfTheTextAndNumberedFromTheirSection()
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
void FlowTests::notesInCellsAreNumberedAgainWhenASectionBreakIsPutInAndTakenOut()
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
void FlowTests::aSectionThatBeginsMidPageRestartsTheNumbersOnceNotAtEachPage()
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
void FlowTests::aSectionWithOtherColumnsBeginsAboveTheNotesAtThePageFoot()
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


// The empty paragraph that ends a section before a continuous one with other sides or more columns takes no room, and the paragraph after it is drawn in the box of its own
// section, where the paginator put it (not in the box above, which has the room of the empty paragraph's line).
void FlowTests::theParagraphAfterAnEmptyContinuousBreakIsDrawnInItsSectionsBox()
{
    for (const int columns : {1, 2}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.sections[0].paper = QSizeF(400, 500);
        fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
        fillStory(fd.story(), 3, 5, {}, 6, 10);
        appendParagraph(fd, QString(), [](QTextBlockFormat &bf) {
            bf.setProperty(fp::SectionBreakAfter, true);
            bf.setProperty(tp::OutOfFlow, true);
        });
        for (int i = 0; i < 6; ++i) appendParagraph(fd, QStringLiteral("Paragraph %1 of the second section, with words enough to fill a few lines of a narrow column on the page.").arg(i));
        Section second = fd.sections[0];
        second.continuous = true;
        second.margins.setLeft(80);
        second.columns = columns;
        second.columnGap = 20;
        fd.sections << second;
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const QString bad = audit::problems(pg, fd.story());
        QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1 columns: %2").arg(columns).arg(bad)));
        // The section below begins where the text above ends: the empty paragraph took no room.
        const int firstOfSecond = fd.story()->findBlockByNumber(4).position(), lastOfFirst = fd.story()->findBlockByNumber(2).position();
        const auto lines = pg.lines(0);
        const auto placed = audit::placedLines(pg, fd.story(), 0, *page);
        QCOMPARE(placed.size(), lines.size());
        double above = -1, below = -1;
        for (int i = 0; i < lines.size(); ++i) {
            if (lines[i].block == lastOfFirst) above = std::max(above, placed[i].rect.bottom() - placed[i].below);
            if (lines[i].block == firstOfSecond && below < 0) below = placed[i].rect.top();
        }
        QVERIFY2(above > 0 && below >= above - 0.01 && below - above < 12, qPrintable(QStringLiteral("%1 columns: the text above ends at %2, the section below begins at %3").arg(columns).arg(above).arg(below)));
    }
}


// A wall across the columns of the section it is in does not reach the section above or below it: a box that keeps text off by its top and bottom, in the second column of a section
// that begins below one column of text, has the first column's text, and the page layer's of it, where they are without it.
void FlowTests::aTopAndBottomWallAcrossTheColumnsItIsInDoesNotReachTheSectionAbove()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.sections[0].paper = QSizeF(400, 500);
    fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
    fillStory(fd.story(), 2, 5, {}, 4, 6);
    appendParagraph(fd, QString(), [](QTextBlockFormat &bf) {
        bf.setProperty(fp::SectionBreakAfter, true);
        bf.setProperty(tp::OutOfFlow, true);
    });
    for (int i = 0; i < 12; ++i) appendParagraph(fd, QStringLiteral("Paragraph %1 of the second section, with words enough to fill a few lines of a narrow column on the page.").arg(i));
    Section second = fd.sections[0];
    second.continuous = true;
    second.columns = 2;
    second.columnGap = 20;
    fd.sections << second;
    // Anchored in the first paragraph of the second section: as wide as the second column, from the page's top by 90 (below the first section's text), a hundred high.
    const int at = fd.story()->findBlockByNumber(3).position();
    const double columnWidth = second.columnWidth();
    const int id = addWrapped(&fd, columnWidth, 100, at, FloatingObject::Wrap::TopBottom, columnWidth + 20, 90, QMarginsF(0, 0, 0, 0), FloatingObject::Side::Both, FloatingObject::Rel::Margin, FloatingObject::Rel::Page);
    QVERIFY(id > 0);
    Paginator pg(&fd);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    // The page layer's wall is as wide as the second column and no wider: the first part's column (as tall as the page, though its text ends above) is not one the wall is in.
    const auto walls = audit::wallsOf(*page);
    QCOMPARE(int(walls.size()), 1);
    QVERIFY2(walls[0].rect.left() > 200 && walls[0].rect.right() < 362, qPrintable(QStringLiteral("the wall is at %1").arg(audit::rectText(walls[0].rect))));
    const QString bad = audit::problems(pg, fd.story());
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    // The first column's lines are not below the wall: the text of the first column runs on from where the section begins.
    const auto lines = pageLines(pg, fd, 0);
    const double wallBottom = 40 + 90 + 100;
    int inFirstColumnBelowFirstLine = 0;
    for (const PageLine &l : lines)
        if (l.block >= at && l.column == 1 && l.rect.top() < wallBottom - 0.05) ++inFirstColumnBelowFirstLine;
    QVERIFY2(inFirstColumnBelowFirstLine >= 3, qPrintable(QStringLiteral("%1 lines of the first column are beside the wall").arg(inFirstColumnBelowFirstLine)));
}


// Two notes cited on the last line of a page (the text goes on after it, with a table before or none): both are drawn in all their height, wherever the line lands.
void FlowTests::twoNotesOnThePagesLastLineAreBothDrawn()
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
void FlowTests::aNoteCitedInACellThatGoesOverAPageBreakOverlapsNothing()
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
void FlowTests::theEndOfALongNoteIsDrawnWhereverItIsCited()
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
void FlowTests::aNoteCitedInTheLastParagraphIsDrawnWholeWhateverThePageFills()
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
void FlowTests::thePartsOfARowWithNotesFillThePages()
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
void FlowTests::rowsHoldTheirSetHeightAtThePageFoot()
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


// A row whose cell ends with a paragraph kept with the next is kept with the next row: where the two do not fit what is left of the page, it goes to the next page whole, and with
// the row after it (a row that is not kept is where it fits, as rows are).
void FlowTests::aRowKeptWithTheNextGoesToTheNextPageWithIt()
{
    auto rowsOnFirstPage = [](bool keep, int *firstOnSecond) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.sections[0].paper = QSizeF(300, 185);
        fd.sections[0].margins = QMarginsF(30, 30, 30, 30);
        TestTable spec;
        spec.rows = 8;
        spec.cols = 1;
        spec.columnWidth = 200;
        spec.text = [](int r, int) { return r == 3 ? QStringLiteral("row three\nsecond\nthird") : QStringLiteral("row %1").arg(r); };
        spec = addTable(fd, spec);
        if (keep) {
            // The last paragraph of row three's cell: the third block from where the row begins.
            QTextBlock b = fd.story()->findBlock(spec.rowStarts[3]);
            b = b.next().next();
            QTextCursor c(b);
            QTextBlockFormat bf;
            bf.setProperty(tp::KeepWithNext, true);
            c.mergeBlockFormat(bf);
        }
        Paginator pg(&fd);
        const auto first = pg.materialize(0);
        if (firstOnSecond) {
            const auto second = pg.materialize(1);
            *firstOnSecond = second && !second->tables.isEmpty() ? second->tables[0]->firstRow : -1;
        }
        return first && !first->tables.isEmpty() ? first->tables[0]->endRow : -1;
    };
    int secondStart = -1;
    QCOMPARE(rowsOnFirstPage(false, &secondStart), 4);
    QCOMPARE(secondStart, 4);
    QCOMPARE(rowsOnFirstPage(true, &secondStart), 3);
    QCOMPARE(secondStart, 3);
}


// The space between a table's cells (where it says it has some) is above and below each row's cells: a row is as tall as its text and its cells' margins and that space twice.
void FlowTests::spaceBetweenCellsIsAboveAndBelowEachRow()
{
    auto height = [](double spacing) {
        FlowDocument fd;
        fd.env.wordLines = true;
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 100;
        spec.setTable = [&](TableData &t) {
            t.spaced = true;
            t.spacing = spacing;
        };
        addTable(fd, spec);
        Paginator pg(&fd);
        const auto m = pg.materialize(0);
        return m && !m->tables.isEmpty() ? m->tables[0]->heightOf(1) : -1.0;
    };
    const double plain = height(0), spaced = height(3);
    QVERIFY(plain > 10);
    QVERIFY2(std::abs(spaced - plain - 6) < 0.01, qPrintable(QStringLiteral("a row is %1 high with space between the cells, %2 without").arg(spaced).arg(plain)));
}
