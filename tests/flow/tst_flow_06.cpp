#include "tst_flow.h"


// The lines of a table take room in its rows, as a word processor sets them: a row is as tall as its cells (with their margins) and the line in its top edge (the table's
// top for the first row, the one between it and the row above for the others), and the last row has the table's foot line too.
void FlowTests::theLinesOfATableTakeRoomInItsRows()
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
void FlowTests::aLineOfATableIsDrawnInsideTheRowItIsTheTopOf()
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
void FlowTests::aCellThatSaysNoneAtALineDoesNotTakeItAwayFromACellThatSaysNothing()
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
void FlowTests::theLineUnderCellsJoinedDownIsThatOfTheCellThatContinuesThem()
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
void FlowTests::aLineAtTheTopOfACellThatContinuesTheOneAboveTakesRoomInItsRow()
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
void FlowTests::aTableThatGoesOnAfterAPageBreakHasItsOuterLinesAtTheBreak()
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
void FlowTests::aSpacedTableThatGoesOnAfterAPageBreakHasItsFrameAtTheBreak()
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
void FlowTests::aSpacedTablePaintsTheLineOfEveryCellsFoot()
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
void FlowTests::aCellAcrossColumnsHasTheLineOfEveryCellAboveIt()
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
void FlowTests::ofTwoLinesAtAnEdgeTheOneOfTheGreaterWeightIsTheLine()
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
void FlowTests::aRowOfASetHeightHasItsMarginsAndLinesBesidesWhenAtLeastAndInsideItWhenExact()
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
void FlowTests::aDoubleLineTakesRoomForBothLinesAndTheSpace()
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
void FlowTests::theEmptyParagraphAfterATableAtTheEndOfACellTakesNoRoom()
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
void FlowTests::aTableBreaksBetweenRowsAcrossPages()
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
void FlowTests::headerRowsRepeatOnTheNextPages()
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
void FlowTests::aPageBreakBeforeATableStartsItOnANewPage()
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
void FlowTests::aRowThatCantBeSplitAndIsTallerThanAPageStillLaysOut()
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
void FlowTests::aRowThatDoesNotFitIsSplitBetweenTheLinesOfItsCells()
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
void FlowTests::aRowSetToAHeightWithContentNotAtItsTopIsNotBegunInLessRoom()
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
void FlowTests::aRowOfATableInACellSetToAHeightIsNotBegunInLessRoomEither()
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
void FlowTests::aTableInACellHasItsMarginAgainWhereItsRowGoesOn()
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
void FlowTests::aPartOfARowDoesNotShowTheLinesOfThePartBeforeItInItsMargin()
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
void FlowTests::aRowWithATableInItIsSplitInsideTheTable()
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
void FlowTests::theLinesLeftForTheNextPageAreNotDrawnUnderTheOnesThatAreOnThisOne()
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
void FlowTests::aRowOfThousandsOfPartsContinuesToItsEnd()
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
void FlowTests::aCellOfThousandsOfParagraphsIsLaidOutWhole()
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
void FlowTests::editsGiveTheRowsTheHeightsAFreshLayoutHas()
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
void FlowTests::editingARowThatIsInPartsGivesThePagesAFreshLayoutHas()
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
void FlowTests::thePageOfAPositionInARowInPartsIsTheOneThatShowsItsLine()
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
void FlowTests::aHeaderThatTakesMoreThanAThirdOfAPageIsNotRepeated()
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


// A cell of eight thousand paragraphs is laid out once for the pages that show it: asking for the pages one after the other does not lay it out again for each. (Counted, not timed: the
// cells laid out for forty more pages are few, where laying the cell out for each would be as many as the pages.)
void FlowTests::aCellOfThousandsOfParagraphsIsNotLaidOutAgainForEachPage()
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
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() > 41);
    QVERIFY(pg.materialize(0));
    const qint64 first = pg.tableCellsLaidOut();
    for (int k = 1; k <= 40; ++k) QVERIFY(pg.materialize(k));
    const qint64 more = pg.tableCellsLaidOut() - first;
    QVERIFY2(more <= 10, qPrintable(QStringLiteral("%1 cells laid out for the first page, %2 more for forty pages after it").arg(first).arg(more)));
}


// A cell is laid out once to find the heights of the rows (in a pass over all of them) and once more for a page that shows it, and a header's cells, which every page of the
// table shows, are laid out once for all of them.
void FlowTests::eachCellIsLaidOutOnceToMeasureAndOnceMoreForAPageThatShowsIt()
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
void FlowTests::notesCitedBeforeAndInsideATableLandOnThePageThatCitesThem()
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
void FlowTests::headersAndFootersOfOtherHeightsBoundTheRowsOfATable()
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
void FlowTests::aTwoColumnSectionAfterATableHoldsATableInItsColumns()
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
void FlowTests::aNoteCitedInACutRowIsOnThePageOfItsLine()
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
void FlowTests::aLongNoteIsCarriedAcrossPagesThatHoldATable()
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


// The rest of a note that goes on from the page before has the file's own separator for it also on a page that holds a table (the page's notes are stacked again there): the
// rule is drawn in that paragraph, two fifths of it above the rest, and the rest is where the text engine's room for it ends.
void FlowTests::theContinuedPartOfANoteHasItsOwnSeparatorOnAPageThatHoldsATable()
{
    auto ruleToPart = [&](double continuation) {
        TestNotes notes;
        notes.separator = 12;
        notes.continuation = continuation;
        addNote(&notes, QStringLiteral("long"), 100, QStringLiteral("LONGNOTE"));
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
        Paginator pg(&fd);
        // The first page that holds a table and the rest of the note.
        for (int k = 1; k < pg.pageCount(); ++k) {
            const auto page = pg.materialize(k);
            const TextItem *part = page && !pg.page(k).start.carried.isEmpty() && !page->tables.isEmpty() ? boxHolding(*page, QStringLiteral("word")) : nullptr;
            if (!part) continue;
            double rule = -1;
            for (const auto &item : page->doc->pages[0]->items)
                if (const auto *line = dynamic_cast<const LineItem *>(item.get()))
                    if (line->p1.y() < part->rect.top() && line->p1.y() > rule) rule = line->p1.y();
            return rule > 0 ? part->rect.top() - rule : -1000.0;
        }
        return -1000.0;
    };
    const double none = ruleToPart(0), own = ruleToPart(40);
    QVERIFY2(std::abs(none - 0.4 * 12) < 0.05, qPrintable(QString::number(none)));
    QVERIFY2(std::abs(own - 0.4 * 40) < 0.05, qPrintable(QString::number(own)));
}


// The paragraph that stands for a table in the story takes no height of its own, set at a word processor's line heights or not: the table is where the text before it ends.
void FlowTests::theStandInForATableDoesNotShiftItUnderWordLineHeights()
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
void FlowTests::objectsAnchoredInTablesAreSetOnceWhereTheirLinesAre()
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
void FlowTests::thePageOfAPositionInARowInPartsIsTheOneThatShowsItsLineWithFurniture()
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
void FlowTests::theStoryOfAPageWithTablesIsLaidOutWhereThePaginatorPutItsLines()
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
void FlowTests::aNoteBesideATableIsSeatedAtTheFootOfItsPage()
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
