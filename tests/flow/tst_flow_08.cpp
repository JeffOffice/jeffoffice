#include "tst_flow.h"


// A row that goes over a page break is shown in parts: the part after the break is as tall as the height the row was set to, as the row is, and a cell with all of its text in the
// part before is set in it as a cell is in a row (one that says bottom has its text at the foot of the part).
void FlowTests::thePartsOfARowSetToAHeightAreThatTallAndHoldTheirShortCellsAsARowDoes()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.sections[0].paper = QSizeF(300, 300);
    fd.sections[0].margins = QMarginsF(30, 30, 30, 30);
    TestTable spec;
    spec.rows = 1;
    spec.cols = 2;
    spec.columnWidth = 100;
    spec.text = [](int, int c) {
        if (c == 1) return QStringLiteral("n");
        QStringList lines;
        for (int i = 0; i < 20; ++i) lines << QStringLiteral("line %1").arg(i);
        return lines.join(QLatin1Char('\n'));
    };
    spec.setRow = [](int, RowData &rd) { rd.height = 150; };
    spec.setCell = [](int, int c, CellData &cd) {
        if (c == 1) cd.valign = CellAlign::Bottom;
    };
    addTable(fd, spec);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 2);
    // The part on the first page: the short cell's line is in its lower half.
    const auto first = pg.materialize(0);
    QVERIFY(first && !first->tables.isEmpty());
    const int shortBlock = fd.story()->lastBlock().position();
    QRectF numberAt;
    first->tables[0]->forLines([&](const StoryLayout::LineBrief &ln, const QRectF &rect) {
        if (ln.block == shortBlock) numberAt = rect;
    });
    const QRectF whole = first->tables[0]->rect();
    QVERIFY2(numberAt.isValid() && numberAt.top() > whole.top() + whole.height() * 0.6, qPrintable(QStringLiteral("the cell's line is at %1 in a part at %2").arg(audit::rectText(numberAt), audit::rectText(whole))));
    // The part on the last page is as tall as the row was set to be.
    const auto last = pg.materialize(pg.pageCount() - 1);
    QVERIFY(last && !last->tables.isEmpty());
    QVERIFY2(last->tables[0]->heightOf(0) > 150 - 0.01, qPrintable(QStringLiteral("the last part is %1 high").arg(last->tables[0]->heightOf(0))));
}


// A table whose rows all ask to be kept with the next, and are many on a page, is not kept together past a few rows: it is laid out in all its rows, and each row looks at no
// more than a few of those after it (the work stays in step with the rows, not with the rows that a page holds).
void FlowTests::aTableOfManyRowsKeptWithTheNextLaysOutInStepWithItsRows()
{
    auto run = [](int rows, qint64 *checks) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.sections[0].paper = QSizeF(300, 300);
        fd.sections[0].margins = QMarginsF(30, 30, 30, 30);
        TestTable spec;
        spec.rows = rows;
        spec.cols = 1;
        spec.columnWidth = 200;
        spec.margins = QMarginsF(0, 0, 0, 0);
        spec.setRow = [](int, RowData &rd) {
            rd.height = 1;
            rd.exact = true;
        };
        addTable(fd, spec);
        for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next())
            if (b.blockFormat().intProperty(fp::Cell)) {
                QTextCursor c(b);
                QTextBlockFormat bf;
                bf.setProperty(tp::KeepWithNext, true);
                c.mergeBlockFormat(bf);
            }
        Paginator pg(&fd);
        int shown = 0;
        for (int k = 0; k < pg.pageCount(); ++k) {
            const auto page = pg.materialize(k);
            for (const auto &t : page->tables) shown += t->endRow - t->firstRow;
        }
        *checks = pg.keepChecks();
        return shown;
    };
    qint64 checks = 0;
    const int rows = 1500;
    QVERIFY2(run(rows, &checks) >= rows, "every row is on a page");
    QVERIFY2(checks > 0 && checks < qint64(rows) * 100, qPrintable(QStringLiteral("%1 rows looked at for %2 rows").arg(checks).arg(rows)));
}


// The space between a table's cells (where it says it has some) is round each cell, at the table's edges too: the text, the fill, and the lines of a cell are in from the edges of
// its place by that much on every side, so that the fills do not cover the space, the first row is below the table's top by it, and the last row is above its foot by it.
void FlowTests::spaceBetweenCellsIsRoundEachCellItsFillAndLinesToo()
{
    struct Made {
        std::unique_ptr<FlowDocument> fd;
        std::unique_ptr<Paginator> pg;
        std::unique_ptr<MaterializedPage> page;
    };
    auto build = [](double spacing) {
        Made m;
        m.fd = std::make_unique<FlowDocument>();
        m.fd->env.wordLines = true;
        m.fd->sections[0].paper = QSizeF(300, 300);
        m.fd->sections[0].margins = QMarginsF(30, 30, 30, 30);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 100;
        spec.setTable = [spacing](TableData &t) {
            t.spaced = true;
            t.spacing = spacing;
        };
        spec.setCell = [](int, int, CellData &cd) {
            cd.fill = QColor(255, 0, 0);
            TableLine line;
            line.style = TableLine::Style::Single;
            line.width = 1;
            line.color = QColor(0, 0, 255);
            cd.top = cd.left = cd.bottom = cd.right = line;
        };
        addTable(*m.fd, spec);
        m.pg = std::make_unique<Paginator>(m.fd.get());
        m.page = m.pg->materialize(0);
        return m;
    };
    const Made plain = build(0), spaced = build(6);
    QVERIFY(plain.page && spaced.page && plain.page->tables.size() == 1 && spaced.page->tables.size() == 1);
    const TableFragment &a = *plain.page->tables[0], &b = *spaced.page->tables[0];
    // The lines of the cells: the first row's top and the last row's foot, and the first cell's text across.
    auto extent = [](const TableFragment &t, double *top, double *bottom, double *left, double *width) {
        *top = 1e9;
        *bottom = -1e9;
        *left = -1;
        t.forLines([&](const StoryLayout::LineBrief &, const QRectF &rect) {
            *top = std::min(*top, rect.top());
            *bottom = std::max(*bottom, rect.bottom());
            if (*left < 0) {
                *left = rect.left();
                *width = rect.width();
            }
        });
    };
    double topA, bottomA, leftA, widthA, topB, bottomB, leftB, widthB;
    extent(a, &topA, &bottomA, &leftA, &widthA);
    extent(b, &topB, &bottomB, &leftB, &widthB);
    QVERIFY2(std::abs((topB - b.rect().top()) - (topA - a.rect().top()) - 6) < 0.01, qPrintable(QStringLiteral("the first row's text is %1 below the table's top, %2 without space").arg(topB - b.rect().top()).arg(topA - a.rect().top())));
    QVERIFY2(std::abs((b.rect().bottom() - bottomB) - (a.rect().bottom() - bottomA) - 6) < 0.01, qPrintable(QStringLiteral("the last row's text is %1 above the table's foot, %2 without space").arg(b.rect().bottom() - bottomB).arg(a.rect().bottom() - bottomA)));
    QVERIFY2(std::abs((leftB - b.rect().left()) - (leftA - a.rect().left()) - 6) < 0.01 && std::abs(widthA - widthB - 12) < 0.01, qPrintable(QStringLiteral("a cell's text is at %1 across and %2 wide, %3 and %4 without space").arg(leftB - b.rect().left()).arg(widthB).arg(leftA - a.rect().left()).arg(widthA)));
    // Drawn: the fill is in the cell and not in the space between cells or at the table's edge, and the cell's lines are at its own edges.
    const int scale = 4;
    QImage image(300 * scale, 300 * scale, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    {
        QPainter painter(&image);
        painter.scale(scale, scale);
        paintTables(&painter, *spaced.page, TablePaint());
    }
    const QPointF origin = b.origin;
    auto count = [&](double x0, double y0, double x1, double y1, bool blue) {
        int n = 0;
        for (int y = int(y0 * scale); y < int(y1 * scale); ++y)
            for (int x = int(x0 * scale); x < int(x1 * scale); ++x) {
                const QRgb px = image.pixel(x, y);
                n += blue ? (qBlue(px) > 200 && qRed(px) < 80 && qGreen(px) < 80) : (qRed(px) > 200 && qGreen(px) < 80 && qBlue(px) < 80);
            }
        return n;
    };
    const double y1 = origin.y() + b.heightOf(0) / 2;
    QVERIFY2(count(origin.x() + 70, y1, origin.x() + 90, y1 + 4, false) > 100, "the cell is filled");
    QCOMPARE(count(origin.x() + 96, y1, origin.x() + 104, y1 + 4, false), 0);   // between the cells (the one's edge is at 94 across, the other's at 106)
    QCOMPARE(count(origin.x() + 20, origin.y(), origin.x() + 80, origin.y() + 5, false), 0);   // above the first row (the space at the table's top)
    QCOMPARE(count(origin.x(), y1, origin.x() + 5, y1 + 4, false), 0);   // beside the first column (the space at the table's left)
    QVERIFY2(count(origin.x() + 5, y1, origin.x() + 7.5, y1 + 4, true) > 20, "the first cell's left line is at the cell's edge, 6 in from the table's");
    QCOMPARE(count(origin.x(), y1, origin.x() + 4.5, y1 + 4, true), 0);
    QVERIFY2(count(origin.x() + 20, origin.y() + 5, origin.x() + 80, origin.y() + 7.5, true) > 100, "the first cell's top line is at the cell's edge, 6 down from the table's top");
    // The last row and the last column are in from the table's foot and right edge by the same, and rows are apart by twice that.
    const double foot = b.rect().bottom(), right = origin.x() + 200, between = origin.y() + b.heightOf(0);
    QCOMPARE(count(origin.x() + 20, foot - 5, origin.x() + 80, foot, false), 0);
    QVERIFY2(count(origin.x() + 20, foot - 7.5, origin.x() + 80, foot - 5, true) > 100, "the last row's foot line is at the cell's edge, 6 up from the table's foot");
    QCOMPARE(count(origin.x() + 20, foot - 4.5, origin.x() + 80, foot, true), 0);
    QCOMPARE(count(right - 5, y1, right, y1 + 4, false), 0);
    QVERIFY2(count(right - 7.5, y1, right - 5, y1 + 4, true) > 20, "the last cell's right line is at the cell's edge, 6 in from the table's");
    QCOMPARE(count(right - 4.5, y1, right, y1 + 4, true), 0);
    QCOMPARE(count(origin.x() + 20, between - 5, origin.x() + 80, between + 5, false), 0);   // (no fill in the space between the rows, 12 high)
    QVERIFY2(count(origin.x() + 20, between - 8, origin.x() + 80, between - 5, true) > 100 && count(origin.x() + 20, between + 5, origin.x() + 80, between + 8, true) > 100, "each row has its own line at its foot and at its top");
}


// A row that cannot be split, citing N notes, costs N, not N squared: twice the notes take about twice as long.
void FlowTests::aRowThatCannotBeSplitCitingManyNotesTakesTimeInStepWithThem()
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
    const PaginateWork small = workToPaginate(build(500), 1 << 30), big = workToPaginate(build(2000), 1 << 30);
    // Four times the notes: linear work is about 4x, the quadratic carry this guards against about 16x.
    verifyInStep(small, big, 8, true);
    // Four times the notes: linear work takes about 4x, the quadratic carry this guards against about 16x; 12x leaves room for a slow CI machine.
    //         QVERIFY2(big < 12 * std::max(small, 20.0), qPrintable(QStringLiteral("500 notes took %1 ms, 2000 notes %2 ms").arg(small).arg(big)));
}


// Tables of rows that each cite many notes: the work is bounded for the document, not for each row (twice the rows take about twice as long, and a hostile file is not
// made to take minutes).
void FlowTests::manyRowsCitingManyNotesTakeTimeInStepWithTheirNumber()
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
    const PaginateWork small = workToPaginate(build(4), 1 << 30), big = workToPaginate(build(16), 1 << 30);
    verifyInStep(small, big, 8, true);
    //         QVERIFY2(big < 8 * std::max(small, 50.0), qPrintable(QStringLiteral("4 rows of 1000 notes took %1 ms, 16 rows %2 ms").arg(small).arg(big)));
}


// A long note cited in a cell of a table in a cell: the time is in step with its length.
void FlowTests::aLongNoteInANestedCellTakesTimeInStepWithItsLength()
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
    const PaginateWork small = workToPaginate(build(5000), 1 << 30), big = workToPaginate(build(20000), 1 << 30);
    verifyInStep(small, big, 8, true);
    QVERIFY2(big.chars < 4.75 * std::max<qint64>(small.chars, 1), qPrintable(QStringLiteral("%1 characters read for the smaller, %2 for the larger").arg(small.chars).arg(big.chars)));
    //         QVERIFY2(big < 8 * std::max(small, 100.0), qPrintable(QStringLiteral("5000 words took %1 ms, 20000 words %2 ms").arg(small).arg(big)));
}


// What is kept about a long paragraph belongs to its document: a new document made where a deleted one was, with a paragraph of the same place and
// length, is read again, not given the deleted one's answer (a kept answer is held by a guarded pointer, which a deletion empties).
void FlowTests::aNewDocumentIsNeverGivenADeletedOnesKeptAnswers()
{
    const QString text = QString(5000, QLatin1Char('a'));
    for (int round = 0; round < 20; ++round) {
        const PaginateWork work = workToPaginate([&](FlowDocument &fd, TestNotes &) { appendParagraph(fd, text); });
        QVERIFY2(work.chars >= text.size(), qPrintable(QStringLiteral("round %1 read %2 characters of %3").arg(round).arg(work.chars).arg(text.size())));
    }
}


// The same note cited in a paragraph of the text, and in a cell of a table that is not in another: its pages read the note in step with its length too.
void FlowTests::aLongNoteInAParagraphOrACellTakesTimeInStepWithItsLength()
{
    for (const bool inTable : {false, true}) {
        auto build = [inTable](int words) {
            return [words, inTable](FlowDocument &fd, TestNotes &notes) {
                addNote(&notes, QStringLiteral("long"), words, QStringLiteral("LONGNOTE"));
                fillStory(fd.story(), 2, 3, {}, 10, 14);
                int refAt = fd.story()->findBlockByNumber(1).position() + 3;
                if (inTable) {
                    TestTable spec;
                    spec.rows = 2;
                    spec.cols = 2;
                    spec.columnWidth = 140;
                    addTable(fd, spec);
                    for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next())
                        if (b.text() == QLatin1String("r1c1")) refAt = b.position() + 2;
                }
                appendParagraph(fd, QStringLiteral("zebra after"));
                addReference(fd.story(), refAt, QStringLiteral("long"));
            };
        };
        const PaginateWork small = workToPaginate(build(5000), 1 << 30), big = workToPaginate(build(20000), 1 << 30);
        verifyInStep(small, big, 8, true);
        QVERIFY2(big.chars < 4.75 * std::max<qint64>(small.chars, 1),
                 qPrintable(QStringLiteral("%1: %2 characters read for the smaller, %3 for the larger").arg(inTable ? "table" : "paragraph").arg(small.chars).arg(big.chars)));
    }
}


// Objects anchored in the paragraphs of cells: making a page costs the paragraphs and the anchors, not their product.
void FlowTests::anchorsInTheCellsOfATableCostInStepWithTheirNumber()
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
    const PaginateWork small = workToPaginate(build(5), 1 << 30), big = workToPaginate(build(20), 1 << 30);
    verifyInStep(small, big, 8, true);
    //         QVERIFY2(big < 8 * std::max(small, 100.0), qPrintable(QStringLiteral("5 cells took %1 ms, 20 cells %2 ms").arg(small).arg(big)));
}


// The heights of notes belong to the environment they were measured in: after the text is made another size (and the pages are laid out again), the notes are the heights a
// paginator made afresh finds.
void FlowTests::aChangeOfTheTextSizeChangesTheHeightsOfNotesBesideTables()
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
void FlowTests::deletingACharacterCostsLikeTypingOne()
{
    struct Cost {
        EngineWork engine;
        qint64 pages = 0, rows = 0, cells = 0, indexed = 0;
        QString text() const { return engine.text() + QStringLiteral(", %1 pages laid out, %2 rows and %3 cells measured, %4 paragraphs searched for tables").arg(pages).arg(rows).arg(cells).arg(indexed); }
    };
    Cost perDelete[2], perType[2];
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
        // What twenty letters typed (or deleted) in a row cost in all.
        const auto counted = [&](const std::function<void()> &edit) {
            const EngineWork before = EngineWork::now();
            const qint64 pages = pg.pagesLaidOut(), rows = pg.tableRowsMeasured(), cells = pg.tableCellsLaidOut(), indexed = pg.tableBlocksIndexed();
            for (int i = 0; i < reps; ++i) {
                edit();
                pg.paginate(paras / 100);
            }
            return Cost{EngineWork::now().since(before), pg.pagesLaidOut() - pages, pg.tableRowsMeasured() - rows, pg.tableCellsLaidOut() - cells, pg.tableBlocksIndexed() - indexed};
        };
        perType[n] = counted([&] {
            cur.setPosition(at);
            cur.insertText(QStringLiteral("x"));
        });
        perDelete[n] = counted([&] {
            cur.setPosition(at + 1);
            cur.deleteChar();
        });
        qInfo("%d paragraphs: typing %s; deleting %s", paras, qPrintable(perType[n].text()), qPrintable(perDelete[n].text()));
        ++n;
    }
    // (Counted: a letter deleted does the work of a letter typed, and the longer story costs no more.)
    const auto cheaper = [](const Cost &a, const Cost &b) {
        return a.engine.chars <= 3 * b.engine.chars + 100 && a.engine.runs <= 3 * b.engine.runs + 100 && a.engine.ranges <= 3 * b.engine.ranges + 100 && a.pages <= 3 * b.pages + 2 && a.rows <= 3 * b.rows + 2
               && a.cells <= 3 * b.cells + 2 && a.indexed <= 3 * b.indexed;
    };
    QVERIFY2(cheaper(perDelete[1], perType[1]), qPrintable(QStringLiteral("typing: %1; deleting: %2 (32000 paragraphs)").arg(perType[1].text(), perDelete[1].text())));
    QVERIFY2(cheaper(perDelete[1], perDelete[0]) && cheaper(perType[1], perType[0]), qPrintable(QStringLiteral("deleting at 8000 paragraphs: %1; at 32000: %2").arg(perDelete[0].text(), perDelete[1].text())));
    QCOMPARE(perDelete[1].indexed, qint64(0));
}


// ---- Underlines drawn by drawUnderline (Qt 6.11 moved the ones it draws itself) ----
namespace {
QImage paintedStory(QTextDocument &doc, double frameWidth, const QColor &pen = Qt::black, bool wordLines = false)
{
    LayoutEnv env;
    env.wordLines = wordLines;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(frameWidth, 600)}, env);
    QImage img(2000, 400, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.translate(600, 10);   // (room to the left, for a right to left line)
    p.setPen(pen);
    lay.paint(&p, 0, PaintOptions());
    p.end();
    return img;
}
QRect inkBox(const QImage &img)
{
    int x0 = img.width(), x1 = -1, y0 = img.height(), y1 = -1;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            if (qGray(img.pixel(x, y)) < 200) {
                x0 = std::min(x0, x); x1 = std::max(x1, x); y0 = std::min(y0, y); y1 = std::max(y1, y);
            }
    return x1 < 0 ? QRect() : QRect(QPoint(x0, y0), QPoint(x1, y1));
}
} // namespace

// Spaces that end an underlined line hang past its width and are underlined as far as they go (Qt's own line was drawn under them): the same words and spaces make a line as long in
// a frame too narrow for them as in one wide enough, left to right or right to left, after a tab, and with letters spaced.
void FlowTests::underlinedSpacesPastTheLineEndAreUnderlinedToo()
{
    struct Case { QString text; bool rtl; double spacing; };
    const QString spaces(40, QLatin1Char(' '));
    const Case cases[] = {{QStringLiteral("hello world") + spaces, false, 0}, {QStringLiteral("hello world") + spaces, true, 0},
                          {QStringLiteral("a\tb") + spaces, false, 0}, {QStringLiteral("hello world") + spaces, false, 2.0}};
    for (const Case &k : cases) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextBlockFormat bf;
        if (k.rtl) bf.setLayoutDirection(Qt::RightToLeft);
        c.setBlockFormat(bf);
        QTextCharFormat f;
        f.setFontPointSize(12);
        f.setFontUnderline(true);
        if (k.spacing) {
            f.setFontLetterSpacingType(QFont::AbsoluteSpacing);
            f.setFontLetterSpacing(k.spacing);
        }
        c.insertText(k.text, f);
        QTextDocument bareDoc;
        QTextCursor bc(&bareDoc);
        bc.setBlockFormat(bf);
        bc.insertText(k.text.trimmed(), f);
        const QRect bare = inkBox(paintedStory(bareDoc, 1000));
        const QRect wide = inkBox(paintedStory(doc, 1000)), narrow = inkBox(paintedStory(doc, 100));
        const QString what = QStringLiteral("rtl %1, spacing %2: %3 wide in a wide frame, %4 in a narrow one").arg(k.rtl).arg(k.spacing).arg(wide.width()).arg(narrow.width());
        QVERIFY2(wide.width() > bare.width() + 40, qPrintable(what + QStringLiteral(" (%1 without the spaces)").arg(bare.width())));
        QVERIFY2(std::abs(wide.width() - narrow.width()) <= 3, qPrintable(what));
    }
}

// A tab that ends an underlined line is underlined to the stop it goes to, however far that is past the text.
void FlowTests::anUnderlinedTabThatEndsALineIsUnderlinedAsFarAsItGoes()
{
    for (const bool wordLines : {false, true}) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextBlockFormat bf;
        bf.setTabPositions({QTextOption::Tab(300, QTextOption::LeftTab)});
        c.setBlockFormat(bf);
        QTextCharFormat f;
        f.setFontPointSize(12);
        f.setFontUnderline(true);
        c.insertText(QStringLiteral("Signature:\t"), f);
        const QRect box = inkBox(paintedStory(doc, 468, Qt::black, wordLines));
        QVERIFY2(box.width() >= 295, qPrintable(QStringLiteral("the line is %1 wide, the tab goes to 300").arg(box.width())));
    }
}

// The underline is in the color of the letters: black where a run sets none (the engine draws such letters black whatever the painter's pen is), the run's own where it sets one.
void FlowTests::underlinesTakeTheTextsColorFromThePenWhenItSetsNone()
{
    for (const bool colored : {false, true}) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat f;
        f.setFontPointSize(24);
        f.setFontUnderline(true);
        if (colored) f.setForeground(QColor(200, 0, 0));
        c.insertText(QStringLiteral("HHHH"), f);
        const QImage img = paintedStory(doc, 400, QColor(0, 160, 0));   // (a pen that is neither)
        const QRect box = inkBox(img);
        QVERIFY(!box.isNull());
        // the letters' darkest pixel and the underline's
        QColor glyph = Qt::white, line = Qt::white;
        for (int x = box.left(); x <= box.right(); ++x) {
            for (int y = box.top(); y < box.bottom() - 2; ++y)
                if (qGray(img.pixel(x, y)) < qGray(glyph.rgb())) glyph = img.pixelColor(x, y);
            for (int y = box.bottom() - 1; y <= box.bottom(); ++y)
                if (qGray(img.pixel(x, y)) < qGray(line.rgb())) line = img.pixelColor(x, y);
        }
        QCOMPARE(line.name(), glyph.name());
        QCOMPARE(line.red() > 150 && line.green() < 60, colored);
    }
}

// Only the plain single underline is drawn by the engine; wave, dotted, dashed, and the spelling underlines are Qt's, as they were.
void FlowTests::onlyPlainSingleUnderlinesAreDrawnHere()
{
    auto drawn = [&](QTextCharFormat::UnderlineStyle style, bool plainSingle) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat f;
        f.setFontPointSize(14);
        if (plainSingle) f.setFontUnderline(true);
        else f.setUnderlineStyle(style);
        c.insertText(QStringLiteral("some underlined words"), f);
        const qint64 before = StoryLayout::ownUnderlinesDrawnForTest();
        paintedStory(doc, 400);
        return StoryLayout::ownUnderlinesDrawnForTest() - before;
    };
    QVERIFY(drawn(QTextCharFormat::SingleUnderline, true) > 0);
    for (const auto style : {QTextCharFormat::WaveUnderline, QTextCharFormat::DotLine, QTextCharFormat::DashUnderline, QTextCharFormat::DashDotLine, QTextCharFormat::SpellCheckUnderline})
        QCOMPARE(drawn(style, false), qint64(0));
}

// Painting the underlines of a paragraph of many runs and lines visits each line and each run a few times, not each run from every line: counted, not timed (the visits are
// in proportion to the runs plus the lines, whatever the machine).
void FlowTests::underlinesCostInProportionToTheirRunsAndLines()
{
    auto visits = [&](int runs, int *lines) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat a, b;
        a.setFontPointSize(10);
        a.setFontUnderline(true);
        b = a;
        b.setForeground(QColor(60, 60, 200));
        for (int i = 0; i < runs; ++i) c.insertText(QStringLiteral("word%1 ").arg(i % 10), i % 2 ? b : a);
        LayoutEnv env;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(300, 1e6)}, env);
        *lines = int(lay.lineInfo(0).size());
        QImage img(400, 60, QImage::Format_RGB32);
        QPainter p(&img);
        const qint64 before = StoryLayout::underlineVisitsForTest();
        lay.paint(&p, 0, PaintOptions());
        return StoryLayout::underlineVisitsForTest() - before;
    };
    for (const int runs : {2000, 8000}) {
        int lines = 0;
        const qint64 n = visits(runs, &lines);
        QVERIFY2(lines > 50, qPrintable(QString::number(lines)));
        QVERIFY2(n > 0 && n <= 4 * (qint64(runs) + lines), qPrintable(QStringLiteral("%1 visits for %2 runs and %3 lines").arg(n).arg(runs).arg(lines)));
    }
}
