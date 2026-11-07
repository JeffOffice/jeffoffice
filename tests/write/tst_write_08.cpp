#include "tst_write.h"


// ---- tables

// A grid of `rows` by `cols` cells of "r<row>c<col>", with `before` and `after` paragraphs around it (a file of its own making).
QByteArray WriteTests::tableDocx(int rows, int cols, const QByteArray &cellExtra)
{
    QByteArray t = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders></w:tblPr><w:tblGrid>";
    for (int c = 0; c < cols; ++c) t += "<w:gridCol w:w=\"2000\"/>";
    t += "</w:tblGrid>";
    for (int r = 0; r < rows; ++r) {
        t += "<w:tr>";
        for (int c = 0; c < cols; ++c) t += "<w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/>" + cellExtra + "</w:tcPr><w:p><w:r><w:t>r" + QByteArray::number(r) + "c" + QByteArray::number(c) + "</w:t></w:r></w:p></w:tc>";
        t += "</w:tr>";
    }
    t += "</w:tbl>";
    return makeDocx("<w:p><w:r><w:t>before</w:t></w:r></w:p>" + t + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles);
}


// The heights of the rows of the first table on the first page.
QVector<double> WriteTests::rowHeightsOf(const QByteArray &body)
{
    auto doc = open(makeDocx(body, kStyles));
    QVector<double> out;
    const auto page = doc->pager().materialize(0);
    if (!page || page->tables.isEmpty()) return out;
    const flow::TableFragment &f = *page->tables[0];
    for (int r = f.firstRow; r < f.endRow; ++r) out << f.heightOf(r);
    return out;
}

QByteArray WriteTests::lineX(const char *side, const char *val, int sz) { return "<w:" + QByteArray(side) + " w:val=\"" + val + "\" w:sz=\"" + QByteArray::number(sz) + "\" w:space=\"0\" w:color=\"auto\"/>"; }


// The lines a file gives a table take room in its rows (a row is as tall as its cells and the line at its top; the last, the table's foot line too): the heights with lines
// are those without them and the lines' widths (a size of 8 is a point).
void WriteTests::theLinesOfATableTakeRoomInItsRowsAsTheFileGivesThem()
{
    auto table = [](const QByteArray &borders, const QByteArray &firstCellPr = QByteArray(), const QByteArray &otherCellPr = QByteArray()) {
        return tableX(rowX(cellX(cellP("a"), firstCellPr)) + rowX(cellX(cellP("b"), otherCellPr)) + rowX(cellX(cellP("c"), otherCellPr)), 1, 2400, "<w:tblW w:w=\"0\" w:type=\"auto\"/>" + borders);
    };
    const QVector<double> bare = rowHeightsOf(table(QByteArray()));
    QCOMPARE(bare.size(), 3);
    const QByteArray lines = "<w:tblBorders>" + lineX("top", "single", 8) + lineX("bottom", "single", 12) + lineX("insideH", "single", 8) + "</w:tblBorders>";
    const QVector<double> got = rowHeightsOf(table(lines));
    QCOMPARE(got.size(), 3);
    QVERIFY2(std::abs(got[0] - (bare[0] + 1)) < 0.01, qPrintable(QString::number(got[0] - bare[0])));
    QVERIFY2(std::abs(got[1] - (bare[1] + 1)) < 0.01, qPrintable(QString::number(got[1] - bare[1])));
    QVERIFY2(std::abs(got[2] - (bare[2] + 1 + 1.5)) < 0.01, qPrintable(QString::number(got[2] - bare[2])));
    // A cell that says none for a line, with the cell beyond it saying nothing, leaves the table's line; both saying none take it out.
    const QVector<double> oneSays = rowHeightsOf(table(lines, "<w:tcBorders>" + lineX("bottom", "nil", 0) + "</w:tcBorders>"));
    QVERIFY2(std::abs(oneSays[1] - (bare[1] + 1)) < 0.01, qPrintable(QString::number(oneSays[1] - bare[1])));
    const QVector<double> bothSay = rowHeightsOf(table(lines, "<w:tcBorders>" + lineX("bottom", "nil", 0) + "</w:tcBorders>", "<w:tcBorders>" + lineX("top", "nil", 0) + "</w:tcBorders>"));
    QVERIFY2(std::abs(bothSay[1] - bare[1]) < 0.01, qPrintable(QString::number(bothSay[1] - bare[1])));
}


// A table that says it has space between its cells, even none, has its lines round the lot of them and not the cells' own (so a cell that says none of its lines does not
// take the table's away): the first row has the top line and the last the foot line.
void WriteTests::aTableWithSpaceBetweenItsCellsHasItsLinesRoundThem()
{
    const QByteArray none = "<w:tcBorders>" + lineX("top", "nil", 0) + lineX("left", "nil", 0) + lineX("bottom", "nil", 0) + lineX("right", "nil", 0) + "</w:tcBorders>";
    auto table = [&](const QByteArray &spacing) {
        const QByteArray pr = "<w:tblW w:w=\"0\" w:type=\"auto\"/>" + spacing + "<w:tblBorders>" + lineX("top", "single", 12) + lineX("left", "single", 12) + lineX("bottom", "single", 16) + lineX("right", "single", 12) + "</w:tblBorders>";
        return tableX(rowX(cellX(cellP("a"), none)) + rowX(cellX(cellP("b"), none)) + rowX(cellX(cellP("c"), none)), 1, 2400, pr);
    };
    const QVector<double> apart = rowHeightsOf(table("<w:tblCellSpacing w:w=\"0\" w:type=\"dxa\"/>")), together = rowHeightsOf(table(QByteArray()));
    QCOMPARE(apart.size(), 3);
    QCOMPARE(together.size(), 3);
    // (Without the space, the cells say none of the lines they have at the edge of the table, and none is left.)
    QVERIFY2(std::abs(apart[0] - (together[0] + 1.5)) < 0.01, qPrintable(QString::number(apart[0] - together[0])));
    QVERIFY2(std::abs(apart[1] - together[1]) < 0.01, qPrintable(QString::number(apart[1] - together[1])));
    QVERIFY2(std::abs(apart[2] - (together[2] + 2)) < 0.01, qPrintable(QString::number(apart[2] - together[2])));
}


// A triple line takes the room of its three lines and the two spaces between them (five times its width), and is read as such, not as a double one.
void WriteTests::aTripleLineTakesRoomForItsThreeLinesAndTheSpaces()
{
    auto table = [](const QByteArray &borders) {
        return tableX(rowX(cellX(cellP("a"))) + rowX(cellX(cellP("b"))), 1, 2400, "<w:tblW w:w=\"0\" w:type=\"auto\"/>" + borders);
    };
    const QVector<double> bare = rowHeightsOf(table(QByteArray()));
    const QVector<double> triple = rowHeightsOf(table("<w:tblBorders>" + lineX("insideH", "triple", 8) + "</w:tblBorders>")), twice = rowHeightsOf(table("<w:tblBorders>" + lineX("insideH", "double", 8) + "</w:tblBorders>"));
    QCOMPARE(bare.size(), 2);
    QCOMPARE(triple.size(), 2);
    QVERIFY2(std::abs(triple[1] - (bare[1] + 5)) < 0.01, qPrintable(QString::number(triple[1] - bare[1])));
    QVERIFY2(std::abs(twice[1] - (bare[1] + 3)) < 0.01, qPrintable(QString::number(twice[1] - bare[1])));
}


// The saved document is well formed and opens again with the same text.
void WriteTests::checkWhole(WriteDoc &doc)
{
    const QByteArray zip = doc.toDocx();
    QVERIFY2(!zip.isEmpty(), "the document saves");
    QVERIFY2(rt::strictProblems(zip).isEmpty(), qPrintable(rt::strictProblems(zip).join(QLatin1Char('\n'))));
    auto back = open(zip);
    QVERIFY(back);
    QString a = doc.story()->toPlainText(), b = back->story()->toPlainText();
    a.remove(QChar::ObjectReplacementCharacter);
    b.remove(QChar::ObjectReplacementCharacter);
    QCOMPARE(b, a);
    QCOMPARE(documentXml(back->toDocx()), documentXml(zip));
    QVERIFY(doc.pager().pageCount() >= 1);
    QVERIFY(doc.pager().materialize(0));
}


// The cell the cursor is in is a cell of the table: Tab goes to the next with its text selected, Shift+Tab to the one before, and Tab in the last cell adds a
// row; Enter in a cell is another paragraph of it; and Backspace and Delete at a cell's edge do not reach into the next cell.
void WriteTests::tabEnterAndDeleteWorkInCells()
{
    auto doc = open(tableDocx(2, 2));
    QTextDocument *s = doc->story();
    auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
    QTextCursor c(s);
    c.setPosition(block(QStringLiteral("r0c0")).position() + 2);
    insertTab(*doc, c, false);
    QCOMPARE(c.selectedText(), QStringLiteral("r0c1"));
    typeText(*doc, c, QStringLiteral("X"));   // (replaces it)
    QVERIFY(block(QStringLiteral("X")).isValid());
    insertTab(*doc, c, true);
    QCOMPARE(c.selectedText(), QStringLiteral("r0c0"));
    QVERIFY(!c.hasSelection() || c.position() != c.anchor());
    // Tab on from the last cell: a row is added, the cursor is in its first cell.
    c.setPosition(block(QStringLiteral("r1c1")).position());
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tr>")), 2);
    insertTab(*doc, c, false);
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tr>")), 3);
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 6);
    QCOMPARE(cellKeyOf(c.block()) != 0, true);
    QVERIFY(c.block().text().isEmpty());
    typeText(*doc, c, QStringLiteral("new"));
    checkWhole(*doc);
    // Enter in a cell: the cell has another paragraph.
    c.setPosition(block(QStringLiteral("r1c0")).position() + 1);
    insertParagraph(*doc, c);
    QVERIFY(cellKeyOf(block(QStringLiteral("r"))) != 0);
    QCOMPARE(cellKeyOf(block(QStringLiteral("r"))), cellKeyOf(block(QStringLiteral("1c0"))));
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 6);
    // Backspace at the start of a cell, Delete at the end of one: nothing joins.
    const int blocks = s->blockCount();
    c.setPosition(block(QStringLiteral("r1c1")).position());
    deleteBack(*doc, c, false);
    QCOMPARE(s->blockCount(), blocks);
    c.setPosition(block(QStringLiteral("1c0")).position() + 3);
    deleteForward(*doc, c, false);
    QCOMPARE(s->blockCount(), blocks);
    checkWhole(*doc);
}


// A file that multiplies what it holds past the limits (more cells than a table may have, tables nested deeper than they are read, more columns than there are) is
// kept whole as the file had it and shown as a stand-in; one at the limits opens, is laid out, and saves.
void WriteTests::tablesPastTheLimitsAreKeptWholeAndOnesAtThemWork()
{
    auto tableOf = [](int rows, int cols) {
        QByteArray t = "<w:tbl><w:tblGrid>";
        for (int c = 0; c < cols; ++c) t += "<w:gridCol w:w=\"500\"/>";
        t += "</w:tblGrid>";
        for (int r = 0; r < rows; ++r) {
            t += "<w:tr>";
            for (int c = 0; c < cols; ++c) t += "<w:tc><w:p/></w:tc>";
            t += "</w:tr>";
        }
        return t + "</w:tbl>";
    };
    auto standIns = [](WriteDoc &doc) {
        int n = 0;
        for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) n += isLocked(b);
        return n;
    };
    // 1,001 rows of 100 columns: more cells than one table may have.
    const QByteArray many = tableOf(1001, 100);
    auto big = open(makeDocx(many + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles));
    QVERIFY(big);
    QCOMPARE(standIns(*big), 1);
    QVERIFY(documentXml(big->toDocx()).contains(many));
    // 40 rows of 256 columns, and 3,000 rows of 3: at the limits, they work.
    auto wide = open(makeDocx(tableOf(40, 256) + "<w:p/>", kStyles));
    QVERIFY(wide);
    QCOMPARE(standIns(*wide), 0);
    QVERIFY(wide->pager().pageCount() >= 1);
    QVERIFY(wide->pager().materialize(0));
    checkWhole(*wide);
    auto tall = open(makeDocx(tableOf(3000, 3) + "<w:p/>", kStyles));
    QVERIFY(tall);
    QCOMPARE(standIns(*tall), 0);
    QVERIFY(tall->pager().pageCount() > 10);
    // More columns than the limit in one row: a stand-in.
    auto toowide = open(makeDocx(tableOf(2, 257) + "<w:p/>", kStyles));
    QVERIFY(toowide);
    QCOMPARE(standIns(*toowide), 1);
    // Nested nine deep: eight are read, and what is in the ninth is a stand-in.
    QByteArray deep;
    for (int i = 0; i < 9; ++i) deep += "<w:tbl><w:tblGrid><w:gridCol w:w=\"1000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>level " + QByteArray::number(i) + "</w:t></w:r></w:p>";
    for (int i = 0; i < 9; ++i) deep += "</w:tc></w:tr></w:tbl><w:p/>";
    auto nested = open(makeDocx(deep, kStyles));
    QVERIFY(nested);
    QCOMPARE(standIns(*nested), 1);
    QVERIFY(documentXml(nested->toDocx()).contains("level 8"));
    QVERIFY(nested->pager().pageCount() >= 1);
    checkWhole(*nested);
}


// A section does not end in a table's cell: the command there changes nothing.
void WriteTests::aSectionBreakIsNotMadeInACell()
{
    auto doc = open(tableDocx(1, 2));
    QTextDocument *s = doc->story();
    QTextCursor c(s);
    for (QTextBlock b = s->begin(); b.isValid(); b = b.next())
        if (b.text() == QLatin1String("r0c1")) c.setPosition(b.position() + 1);
    const int blocks = s->blockCount();
    insertSectionBreak(*doc, c);
    QCOMPARE(s->blockCount(), blocks);
    QCOMPARE(doc->sectionCount(), 1);
    QVERIFY(!s->isUndoAvailable());
}


// Rows and columns are added beside the cell's and taken away, as one step each that undoes to the very file it began as.
void WriteTests::rowsAndColumnsAreAddedAndTakenAway()
{
    const QByteArray original = tableDocx(3, 3);
    auto doc = open(original);
    QTextDocument *s = doc->story();
    const QByteArray was = documentXml(doc->toDocx());
    auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
    auto count = [&](const char *what) { return int(documentXml(doc->toDocx()).count(what)); };
    QTextCursor c(s);
    c.setPosition(block(QStringLiteral("r1c1")).position());
    insertTableRow(*doc, c, true);
    QCOMPARE(count("<w:tr>"), 4);
    QCOMPARE(count("<w:tc>"), 12);
    checkWhole(*doc);
    s->undo();
    QCOMPARE(documentXml(doc->toDocx()), was);   // (as it was, to the byte: the table that nothing changed)
    s->redo();
    QCOMPARE(count("<w:tr>"), 4);
    insertTableRow(*doc, c, false);
    QCOMPARE(count("<w:tr>"), 5);
    // The new rows are empty paragraphs in cells, in the places: the row after r1 has r1's cells' widths.
    QStringList texts;
    for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) texts << b.text();
    QCOMPARE(texts, (QStringList{"before", "r0c0", "r0c1", "r0c2", "", "", "", "r1c0", "r1c1", "r1c2", "", "", "", "r2c0", "r2c1", "r2c2", "after"}));
    checkWhole(*doc);
    // A column: three in each row become four, the grid has four columns, and the new cells are empty.
    c.setPosition(block(QStringLiteral("r1c1")).position());
    insertTableColumn(*doc, c, true);
    QCOMPARE(count("<w:tc>"), 5 * 4);
    QCOMPARE(count("<w:gridCol "), 4);
    checkWhole(*doc);
    s->undo();
    QCOMPARE(count("<w:gridCol "), 3);
    s->redo();
    insertTableColumn(*doc, c, false);
    QCOMPARE(count("<w:gridCol "), 5);
    QCOMPARE(count("<w:tc>"), 5 * 5);
    checkWhole(*doc);
    // Taking away: the row and the column of the cursor, then the whole table when no row or no column is left.
    c.setPosition(block(QStringLiteral("r1c1")).position());
    deleteTableRows(*doc, c);
    QCOMPARE(count("<w:tr>"), 4);
    QVERIFY(!block(QStringLiteral("r1c1")).isValid());
    QVERIFY(block(QStringLiteral("r1c0")).isValid() == false);
    c.setPosition(block(QStringLiteral("r2c1")).position());
    deleteTableColumns(*doc, c);
    QCOMPARE(count("<w:gridCol "), 4);
    QCOMPARE(count("<w:tc>"), 4 * 4);
    QVERIFY(!block(QStringLiteral("r2c1")).isValid());
    QVERIFY(block(QStringLiteral("r2c0")).isValid() && block(QStringLiteral("r2c2")).isValid());
    checkWhole(*doc);
    c.setPosition(block(QStringLiteral("r2c0")).position());
    deleteTable(*doc, c);
    QCOMPARE(count("<w:tbl>"), 0);
    QCOMPARE(s->toPlainText(), QStringLiteral("before\nafter"));
    checkWhole(*doc);
    while (s->isUndoAvailable()) s->undo();
    QCOMPARE(documentXml(doc->toDocx()), was);
}


// A column through cells that are joined across makes them one wider; a row through cells joined down is a row of the same joined cells; and deleting the row a joined
// cell begins in gives the cell below it what it was.
void WriteTests::rowsAndColumnsKeepJoinedCellsWhole()
{
    QByteArray t = "<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
                   "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"4000\" w:type=\"dxa\"/><w:gridSpan w:val=\"2\"/></w:tcPr><w:p><w:r><w:t>wide</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:vMerge w:val=\"restart\"/></w:tcPr><w:p><w:r><w:t>tall</w:t></w:r></w:p></w:tc></w:tr>"
                   "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>a</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>b</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:vMerge/></w:tcPr><w:p/></w:tc></w:tr>"
                   "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>c</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>d</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:vMerge/></w:tcPr><w:p/></w:tc></w:tr></w:tbl>";
    auto doc = open(makeDocx(t + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles));
    QTextDocument *s = doc->story();
    auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
    auto xmlNow = [&] { return documentXml(doc->toDocx()); };
    QTextCursor c(s);
    // A column to the right of the first (it is inside "wide"): that cell is wider, the rows below have a new cell beside "a".
    c.setPosition(block(QStringLiteral("a")).position());
    insertTableColumn(*doc, c, true);
    QVERIFY(xmlNow().contains("<w:gridSpan w:val=\"3\"/>"));
    QCOMPARE(int(xmlNow().count("<w:gridCol ")), 4);
    QCOMPARE(int(xmlNow().count("<w:vMerge/>")), 2);
    checkWhole(*doc);
    s->undo();
    // A row below the first: it is inside "tall".
    c.setPosition(block(QStringLiteral("wide")).position());
    insertTableRow(*doc, c, true);
    QCOMPARE(int(xmlNow().count("<w:vMerge/>")), 3);
    checkWhole(*doc);
    s->undo();
    // The row "tall" begins in goes: "tall" is now at the next row, with the rest of its cells (one row of it left, so not joined).
    c.setPosition(block(QStringLiteral("wide")).position());
    deleteTableRows(*doc, c);
    QVERIFY(!xmlNow().contains("tall"));
    QCOMPARE(int(xmlNow().count("<w:tr>")), 2);
    QVERIFY(xmlNow().contains("<w:vMerge w:val=\"restart\"/>"));
    checkWhole(*doc);
}


// ---- tables with gaps in their rows, and commands that take columns out

QByteArray WriteTests::cellP(const QByteArray &text) { return "<w:p><w:r><w:t>" + text + "</w:t></w:r></w:p>"; }

QByteArray WriteTests::cellX(const QByteArray &content, const QByteArray &pr, int width)
{
    return "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(width) + "\" w:type=\"dxa\"/>" + pr + "</w:tcPr>" + content + "</w:tc>";
}

QByteArray WriteTests::rowX(const QByteArray &cells, const QByteArray &pr) { return "<w:tr>" + (pr.isEmpty() ? QByteArray() : "<w:trPr>" + pr + "</w:trPr>") + cells + "</w:tr>"; }

QByteArray WriteTests::tableX(const QByteArray &rows, int columns, int columnWidth, const QByteArray &width)
{
    QByteArray t = "<w:tbl><w:tblPr>" + width + "</w:tblPr><w:tblGrid>";
    for (int i = 0; i < columns; ++i) t += "<w:gridCol w:w=\"" + QByteArray::number(columnWidth) + "\"/>";
    return t + "</w:tblGrid>" + rows + "</w:tbl>";
}

// The story's paragraphs, as one text.
QString WriteTests::storyText(WriteDoc &doc) { return doc.story()->toPlainText().replace(QLatin1Char('\n'), QLatin1Char('|')); }

QTextCursor WriteTests::cursorAt(WriteDoc &doc, const QString &text)
{
    QTextCursor c(doc.story());
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        if (b.text() == text) c.setPosition(b.position());
    return c;
}

// Taking away columns takes the paragraphs of the cells in them and no others, whatever rows there are between: a row that has only a gap and a cell in the columns
// goes with them, and what is outside the table is as it was.
void WriteTests::deletingColumnsTakesOnlyTheCellsInThem()
{
    const QByteArray body = cellP("top") +
                            tableX(rowX(cellX(cellP("A0")) + cellX(cellP("A1")) + cellX(cellP("A2"))) +
                                       rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 4800), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"2400\" w:type=\"dxa\"/>") +
                                       rowX(cellX(cellP("R0")) + cellX(cellP("R1")) + cellX(cellP("R2"))),
                                   3) +
                            cellP("tail1") + cellP("tail2") + cellP("tail3");
    auto doc = open(makeDocx(body, kStyles));
    QTextCursor c = cursorAt(*doc, QStringLiteral("A1"));
    c.setPosition(cursorAt(*doc, QStringLiteral("A2")).position() + 1, QTextCursor::KeepAnchor);
    const QString was = storyText(*doc);
    deleteTableColumns(*doc, c);
    QCOMPARE(storyText(*doc), QStringLiteral("top|A0|R0|tail1|tail2|tail3"));
    checkWhole(*doc);
    doc->story()->undo();
    QCOMPARE(storyText(*doc), was);
    // Columns 1 and 2 of a table where the row with the gap is the first, the gap's cell is on the edge of the columns, and one more row follows.
    const QByteArray body2 = cellP("top") +
                             tableX(rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 4800), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"2400\" w:type=\"dxa\"/>") +
                                        rowX(cellX(cellP("B0")) + cellX(cellP("B1")) + cellX(cellP("B2"))) + rowX(cellX(cellP("C0")) + cellX(cellP("C1")) + cellX(cellP("C2"))),
                                    3) +
                             cellP("tail1");
    auto second = open(makeDocx(body2, kStyles));
    QTextCursor d = cursorAt(*second, QStringLiteral("B1"));
    d.setPosition(cursorAt(*second, QStringLiteral("B2")).position() + 1, QTextCursor::KeepAnchor);
    deleteTableColumns(*second, d);
    QCOMPARE(storyText(*second), QStringLiteral("top|B0|C0|tail1"));
    checkWhole(*second);
}


// A row that has a gap before its first cell or after its last keeps it where it is the gap's, in every command: a row added beside it has the same, a column taken
// away takes from the gap what is of it, a column added in the gap makes it wider, and the table's width goes with its columns.
void WriteTests::gapsInRowsFollowTheRowAndColumnCommands()
{
    const QByteArray gapRow = rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 4800), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"2400\" w:type=\"dxa\"/>");
    const QByteArray body = cellP("top") +
                            tableX(rowX(cellX(cellP("A0")) + cellX(cellP("A1")) + cellX(cellP("A2"))) + gapRow + rowX(cellX(cellP("D0")) + cellX(cellP("D1")) + cellX(cellP("D2"))), 3, 2400,
                                   "<w:tblW w:w=\"7200\" w:type=\"dxa\"/>") +
                            cellP("tail");
    auto count = [](WriteDoc &d, const char *what) { return int(documentXml(d.toDocx()).count(what)); };
    {   // a row beside the row with a gap has the gap
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c = cursorAt(*doc, QStringLiteral("G"));
        insertTableRow(*doc, c, true);
        QCOMPARE(count(*doc, "<w:gridBefore "), 2);
        QCOMPARE(count(*doc, "<w:wBefore "), 2);
        QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
        checkWhole(*doc);
    }
    {   // a column taken from the gap makes it narrower, and the grid's width less: the row still has as many columns as the grid
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c = cursorAt(*doc, QStringLiteral("D0"));
        deleteTableColumns(*doc, c);
        QCOMPARE(count(*doc, "<w:gridBefore "), 0);   // (the gap was that column)
        QCOMPARE(count(*doc, "<w:wBefore "), 0);
        QCOMPARE(count(*doc, "<w:gridCol "), 2);
        QVERIFY2(documentXml(doc->toDocx()).contains("<w:tblW w:w=\"4800\" w:type=\"dxa\"/>"), documentXml(doc->toDocx()).left(900).constData());
        QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
        checkWhole(*doc);
    }
    for (const bool right : {false, true}) {   // a column added left of the first: the gap is a column wider (the new one is in it)
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c = cursorAt(*doc, QStringLiteral("A0"));
        insertTableColumn(*doc, c, right);
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(count(*doc, "<w:gridCol "), 4);
        QVERIFY2(x.contains("<w:tblW w:w=\"9600\" w:type=\"dxa\"/>"), x.left(900).constData());
        QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
        // The new cell of the gap's row is in the gap when it is left of the gap's end, and a cell of its own when it is right of it.
        const QByteArray gap = x.mid(x.indexOf("<w:gridBefore "), 40);
        QVERIFY2(right ? gap.contains("w:val=\"1\"") : gap.contains("w:val=\"2\""), gap.constData());
        checkWhole(*doc);
        doc->story()->undo();
        QCOMPARE(count(*doc, "<w:gridCol "), 3);
    }
    {   // the gap after the last cell grows when a column is added in it, and loses what a deleted column held of it
        const QByteArray b2 = cellP("top") +
                              tableX(rowX(cellX(cellP("A0")) + cellX(cellP("A1")) + cellX(cellP("A2"))) + rowX(cellX(cellP("E0")) + cellX(cellP("E1")), "<w:gridAfter w:val=\"1\"/><w:wAfter w:w=\"2400\" w:type=\"dxa\"/>"), 3) +
                              cellP("tail");
        auto doc = open(makeDocx(b2, kStyles));
        QTextCursor c = cursorAt(*doc, QStringLiteral("A2"));
        deleteTableColumns(*doc, c);   // (the last column: the row with the gap loses it)
        QCOMPARE(count(*doc, "<w:gridAfter "), 0);
        QCOMPARE(count(*doc, "<w:gridCol "), 2);
        QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
        checkWhole(*doc);
        QTextCursor e = cursorAt(*doc, QStringLiteral("A1"));
        insertTableColumn(*doc, e, true);   // (right of the last column)
        QVERIFY2(rt::strictProblems(doc->toDocx()).isEmpty(), qPrintable(rt::strictProblems(doc->toDocx()).join(QLatin1Char('\n'))));
        checkWhole(*doc);
    }
}


// A row or a column added beside others is like them: the row says what the row beside it says of its height and of not being split, and each new cell says what
// the cell beside it says of its shading and its place of its text (the properties of a start that a file leaves its root's declarations to be read by).
void WriteTests::newRowsAndCellsAreLikeTheOnesBesideThem()
{
    const QByteArray pr = "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"FF0000\"/><w:vAlign w:val=\"center\"/>";
    const QByteArray body = cellP("top") +
                            tableX(rowX(cellX(cellP("A0"), pr) + cellX(cellP("A1"), pr), "<w:cantSplit/><w:trHeight w:val=\"500\" w:hRule=\"exact\"/>") + rowX(cellX(cellP("B0"), pr) + cellX(cellP("B1"), pr)), 2) + cellP("tail");
    auto doc = open(makeDocx(body, kStyles));
    auto count = [&](const char *what) { return int(documentXml(doc->toDocx()).count(what)); };
    QTextCursor c = cursorAt(*doc, QStringLiteral("A0"));
    insertTableRow(*doc, c, true);
    QCOMPARE(count("<w:cantSplit/>"), 2);
    QCOMPARE(count("<w:trHeight "), 2);
    QCOMPARE(count("w:fill=\"FF0000\""), 6);
    QCOMPARE(count("<w:vAlign w:val=\"center\"/>"), 6);
    c = cursorAt(*doc, QStringLiteral("B0"));
    insertTableColumn(*doc, c, true);
    QCOMPARE(count("w:fill=\"FF0000\""), 9);
    QCOMPARE(count("<w:vAlign w:val=\"center\"/>"), 9);
    checkWhole(*doc);
}


// A paragraph in a cell stays in its cell when it is given a style, and so does the paragraph that Enter makes after it in the style that follows (a heading is
// followed by body text): a paragraph's format is made over in both, and the cell is not what the style says.
void WriteTests::aParagraphInACellStaysThereWhateverIsDoneToItsFormat()
{
    auto doc = open(tableDocx(2, 2));
    QTextDocument *s = doc->story();
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c1"));
    const int key = cellKeyOf(c.block());
    QVERIFY(key != 0);
    applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
    QCOMPARE(cellKeyOf(cursorAt(*doc, QStringLiteral("r0c1")).block()), key);
    c.movePosition(QTextCursor::EndOfBlock);
    insertParagraph(*doc, c);   // (at the end of a heading: the next paragraph is body text)
    QCOMPARE(cellKeyOf(c.block()), key);
    QCOMPARE(c.block().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Normal"));
    typeText(*doc, c, QStringLiteral("more"));
    QVERIFY(plain(*doc).contains(QStringLiteral("r0c1\nmore\nr1c0")));
    checkWhole(*doc);
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:tc>")), 4);   // (all four cells are cells still)
    while (s->isUndoAvailable()) s->undo();
    QCOMPARE(plain(*doc), QStringLiteral("before\nr0c0\nr0c1\nr1c0\nr1c1\nafter"));
}


// Rows added beside a row whose cells are all empty (so that no paragraph says which is which but its place) have their cells in the same order: the cell joined to the
// one above is the first, as the row beside it has it, for a row put above and for one put below.
void WriteTests::rowsAddedBesideEmptyCellsKeepTheOrderOfTheirCells()
{
    const QByteArray empty = "<w:p/>";
    const QByteArray body = cellP("top") +
                            tableX(rowX(cellX(empty, "<w:vMerge w:val=\"restart\"/>") + cellX(empty) + cellX(empty)) + rowX(cellX(empty, "<w:vMerge/>") + cellX(empty) + cellX(empty)) +
                                       rowX(cellX(empty) + cellX(empty) + cellX(empty)),
                                   3) +
                            cellP("tail");
    for (const bool below : {false, true}) {
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c(doc->story());
        // (The second row's first cell: the second table paragraph in the story, after the first row's three.)
        QVector<QTextBlock> cells;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (cellKeyOf(b)) cells << b;
        QCOMPARE(int(cells.size()), 9);
        c.setPosition(cells[3].position());
        insertTableRow(*doc, c, below);
        const QByteArray saved = doc->toDocx();
        QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
        // The joined cells are the first of their rows: three rows of three become four, and three of the four have a joined first cell where the row before has one.
        const QByteArray x = documentXml(saved);
        QCOMPARE(int(x.count("<w:tr>")), 4);
        const qsizetype firstRowEnd = x.indexOf("</w:tr>");
        QVERIFY(x.left(firstRowEnd).contains("w:val=\"restart\""));
        // Each row's joined cell is its first: no row has one after a cell that is not.
        qsizetype from = 0;
        for (int row = 0; row < 4; ++row) {
            const qsizetype end = x.indexOf("</w:tr>", from);
            const QByteArray text = x.mid(from, end - from);
            const qsizetype joined = std::max(text.indexOf("<w:vMerge/>"), text.indexOf("<w:vMerge w:val"));
            if (joined >= 0) QCOMPARE(int(text.left(joined).count("<w:tc>")), 1);
            from = end + 7;
        }
        checkWhole(*doc);
    }
}


// Tab in the last cell of a table whose last row is joined to the one above adds a row after the table's true last row, with the cursor in its first cell, and its
// cells are not joined.
void WriteTests::tabInTheLastCellOfAJoinedLastRowAddsARowAfterTheTable()
{
    const QByteArray body = cellP("x") +
                            tableX(rowX(cellX(cellP("r0a"), "<w:vMerge w:val=\"restart\"/>") + cellX(cellP("r0b"), "<w:vMerge w:val=\"restart\"/>")) +
                                       rowX(cellX("<w:p/>", "<w:vMerge/>") + cellX("<w:p/>", "<w:vMerge/>")),
                                   2) +
                            cellP("y");
    auto doc = open(makeDocx(body, kStyles));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0b"));
    insertTab(*doc, c, false);
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:tr>")), 3);
    QCOMPARE(int(x.count("<w:vMerge/>")), 2);   // (the new row's cells are not joined to the ones above)
    const CellPlace place = cellPlace(*doc, c);
    QVERIFY(place.inTable);
    QCOMPARE(place.row, 2);
    QCOMPARE(place.column, 0);
    checkWhole(*doc);
}


// ---- making and changing tables (beta wave B)

// The tables of a document, cell by cell, as text: the grid, the table's own settings, each row's, and each cell's span, merge, lines, fill, and alignment, and the words of its
// paragraphs. A table that is saved and opened comes to the same text.
QString WriteTests::tablesSignature(WriteDoc &doc)
{
    auto line = [](const std::optional<flow::TableLine> &l) {
        return l ? QStringLiteral("%1/%2/%3").arg(int(l->style)).arg(l->width).arg(l->color.isValid() ? l->color.name() : QStringLiteral("auto")) : QStringLiteral("-");
    };
    auto edge = [](const flow::TableLine &l) { return QStringLiteral("%1/%2/%3").arg(int(l.style)).arg(l.width).arg(l.color.isValid() ? l.color.name() : QStringLiteral("auto")); };
    QString out;
    const flow::TableStore &store = doc.flow().tables;
    const flow::TableData *table = nullptr;
    const flow::RowData *row = nullptr;
    int lastKey = -1;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        const int key = cellKeyOf(b);
        if (!key) {
            out += QStringLiteral("P:") + b.text() + QLatin1Char('\n');
            table = nullptr;
            row = nullptr;
            lastKey = -1;
            continue;
        }
        const flow::CellRecord *r = store.get(key);
        if (r->table.get() != table) {
            table = r->table.get();
            QStringList cols;
            for (double w : table->columns) cols << QString::number(w, 'f', 1);
            out += QStringLiteral("TABLE cols=%1 align=%2 indent=%3 margins=%4,%5,%6,%7 spacing=%8 borders=%9 %10 %11 %12 %13 %14 fill=%15\n")
                       .arg(cols.join(QLatin1Char(',')))
                       .arg(int(table->align))
                       .arg(table->indent)
                       .arg(table->margins.left())
                       .arg(table->margins.top())
                       .arg(table->margins.right())
                       .arg(table->margins.bottom())
                       .arg(table->spacing)
                       .arg(edge(table->top), edge(table->left), edge(table->bottom), edge(table->right), edge(table->insideH), edge(table->insideV))
                       .arg(table->fill ? table->fill->name() : QStringLiteral("-"));
            row = nullptr;
        }
        if (r->row.get() != row) {
            row = r->row.get();
            out += QStringLiteral(" ROW height=%1 exact=%2 cantSplit=%3 header=%4\n").arg(row->height).arg(row->exact).arg(row->cantSplit).arg(row->header);
            lastKey = -1;
        }
        if (key != lastKey) {
            const flow::CellData &c = r->cell;
            out += QStringLiteral("  CELL span=%1 vmerge=%2 lines=%3 %4 %5 %6 fill=%7 valign=%8 nowrap=%9 margins=%10\n")
                       .arg(c.span)
                       .arg(int(c.vmerge))
                       .arg(line(c.top), line(c.left), line(c.bottom), line(c.right))
                       .arg(c.fill ? (c.fill->alpha() ? c.fill->name() : QStringLiteral("none")) : QStringLiteral("-"))
                       .arg(int(c.valign))
                       .arg(c.noWrap)
                       .arg(c.margins ? QStringLiteral("%1,%2,%3,%4").arg(c.margins->left()).arg(c.margins->top()).arg(c.margins->right()).arg(c.margins->bottom()) : QStringLiteral("-"));
            lastKey = key;
        }
        out += QStringLiteral("    ") + b.text() + QStringLiteral(" align=%1 after=%2\n").arg(int(b.blockFormat().alignment())).arg(b.blockFormat().bottomMargin());
    }
    return out;
}


// The document saved and opened again has the same tables, and the file passes the strict checks.
void WriteTests::checkTablesReopenTheSame(WriteDoc &doc)
{
    const QByteArray zip = doc.toDocx();
    QVERIFY2(!zip.isEmpty(), "the document saves");
    QVERIFY2(rt::strictProblems(zip).isEmpty(), qPrintable(rt::strictProblems(zip).join(QLatin1Char('\n'))));
    auto back = open(zip);
    QVERIFY(back);
    QCOMPARE(tablesSignature(*back), tablesSignature(doc));
    // (and it is written the same the second time)
    QCOMPARE(documentXml(back->toDocx()), documentXml(zip));
}


int WriteTests::selectCells(WriteDoc &doc, QTextCursor &c, const QString &from, const QString &to)
{
    QTextCursor a = cursorAt(doc, from), b = cursorAt(doc, to);
    c = QTextCursor(doc.story());
    c.setPosition(a.position());
    c.setPosition(b.position() + b.block().length() - 1, QTextCursor::KeepAnchor);
    return c.position();
}


// Insert Table makes a table of the size asked at the cursor, in the table style, in cells as wide as the text area shares, and each cell has a paragraph in the style
// that table style gives: the file has it, and opens the same. Undo takes it away.
void WriteTests::insertTableMakesATableThatSavesAndOpensTheSame()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("before"));
    insertParagraph(*doc, c);
    typeText(*doc, c, QStringLiteral("after"));
    c.setPosition(doc->story()->findBlock(0).next().position());
    const QString was = plain(*doc);
    NewTable t;
    t.rows = 3;
    t.columns = 4;
    QVERIFY(insertTable(*doc, c, t));
    QCOMPARE(int(doc->story()->blockCount()), 2 + 12);
    const QString signature = tablesSignature(*doc);
    QVERIFY2(signature.count(QStringLiteral(" ROW ")) == 3 && signature.count(QStringLiteral("  CELL ")) == 12, qPrintable(signature));
    // The grid shares the text area (the paper's width less its margins), and the table style's lines are the table's.
    const QTextBlock firstCell = doc->story()->findBlock(c.position());
    QCOMPARE(cellKeyOf(firstCell) != 0, true);
    const flow::CellRecord *rec = doc->flow().tables.get(cellKeyOf(firstCell));
    QCOMPARE(rec->table->columns.size(), 4);
    double sum = 0;
    for (double w : rec->table->columns) sum += w;
    QVERIFY2(std::abs(sum - textWidthAt(*doc, 0)) < 0.6, qPrintable(QString::number(sum)));
    QVERIFY(rec->table->top.visible() && rec->table->insideV.visible());
    QCOMPARE(firstCell.blockFormat().bottomMargin(), 0.0);   // (Table Grid has no space after)
    // The cursor is in the first cell; the paragraph after the table is the one the cursor was in.
    QCOMPARE(c.position(), firstCell.position());
    QCOMPARE(doc->story()->lastBlock().text(), QStringLiteral("after"));
    checkTablesReopenTheSame(*doc);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY(x.contains("<w:tblStyle w:val=\"TableGrid\"/>"));
    QCOMPARE(int(x.count("<w:tr>")), 3);
    QCOMPARE(int(x.count("<w:tc>")), 12);
    QVERIFY(styleXml(doc->toDocx()).contains("w:styleId=\"TableGrid\""));
    // One step takes it away, and one puts it back.
    doc->story()->undo();
    QCOMPARE(plain(*doc), was);
    QVERIFY(tablesSignature(*doc).count(QStringLiteral("CELL")) == 0);
    doc->story()->redo();
    QCOMPARE(int(doc->story()->blockCount()), 2 + 12);
    QCOMPARE(tablesSignature(*doc), signature);
    // Typing goes into the cell, and Tab goes on to the next.
    c.setPosition(firstCell.position());
    typeText(*doc, c, QStringLiteral("one"));
    QVERIFY(tabThroughCells(*doc, c, false));
    typeText(*doc, c, QStringLiteral("two"));
    QVERIFY(plain(*doc).contains(QStringLiteral("one\ntwo")));
    checkTablesReopenTheSame(*doc);
}


// Insert Table in the middle of a paragraph cuts it in two with the table between; where the cursor is at the start of the document, the table is the first thing in it; a
// table is not made in a table; and the fit options are written (the window is the whole width as a percent, the contents' are automatic).
void WriteTests::insertTableCutsAParagraphAtTheCursorAndKeepsTheFitItWasGiven()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("abcdef"));
    c.setPosition(3);
    NewTable t;
    t.rows = 1;
    t.columns = 2;
    t.fit = TableFit::Window;
    QVERIFY(insertTable(*doc, c, t));
    QCOMPARE(plain(*doc), QStringLiteral("abc\n\n\ndef"));
    QVERIFY(documentXml(doc->toDocx()).contains("<w:tblW w:w=\"5000\" w:type=\"pct\"/>"));
    QVERIFY(!insertTable(*doc, c, t));   // (the cursor is in a cell)
    checkTablesReopenTheSame(*doc);
    // At the very start.
    auto first = WriteDoc::blank();
    QTextCursor f(first->story());
    typeText(*first, f, QStringLiteral("text"));
    f.setPosition(0);
    t.fit = TableFit::Contents;
    QVERIFY(insertTable(*first, f, t));
    QCOMPARE(cellKeyOf(first->story()->begin()) != 0, true);
    QCOMPARE(first->story()->lastBlock().text(), QStringLiteral("text"));
    QVERIFY(documentXml(first->toDocx()).contains("<w:tblW w:w=\"0\" w:type=\"auto\"/>"));
    checkTablesReopenTheSame(*first);
    QVERIFY(!insertTable(*first, f, NewTable{0, 3}));
}


// Merge Cells across joins the cells of a row into one with the span of them, with the words of each in turn; empty cells add no paragraph; Split Cells makes cells of
// it again; each is one step to undo, and the file opens the same.
void WriteTests::mergeCellsAcrossAndSplitThemAgain()
{
    auto doc = open(tableDocx(3, 3));
    QTextCursor c(doc->story());
    selectCells(*doc, c, QStringLiteral("r0c0"), QStringLiteral("r0c1"));
    const QString before = tablesSignature(*doc);
    QVERIFY(canMergeCells(*doc, c));
    QVERIFY(mergeCells(*doc, c));
    QString now = tablesSignature(*doc);
    QVERIFY2(now.contains(QStringLiteral("CELL span=2 vmerge=0")), qPrintable(now));
    QCOMPARE(plain(*doc), QStringLiteral("before\nr0c0\nr0c1\nr0c2\nr1c0\nr1c1\nr1c2\nr2c0\nr2c1\nr2c2\nafter"));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY(x.contains("<w:gridSpan w:val=\"2\"/>"));
    QCOMPARE(int(x.count("<w:tc>")), 8);
    checkTablesReopenTheSame(*doc);
    // The merged cell is split into two again (the words stay in the first), and a third row's cell into three.
    QTextCursor in = cursorAt(*doc, QStringLiteral("r0c0"));
    QVERIFY(canSplitCell(*doc, in));
    QVERIFY(splitCells(*doc, in, 2, 1));
    now = tablesSignature(*doc);
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 9);
    QVERIFY(!now.contains(QStringLiteral("span=2")));
    checkTablesReopenTheSame(*doc);
    // Undo each step.
    doc->story()->undo();
    QVERIFY(tablesSignature(*doc).contains(QStringLiteral("span=2")));
    doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
    doc->story()->redo();
    QVERIFY(tablesSignature(*doc).contains(QStringLiteral("span=2")));
    // Empty cells add nothing.
    auto empties = open(makeDocx(cellP("x") + tableX(rowX(cellX("<w:p/>") + cellX("<w:p/>") + cellX(cellP("end"))), 3) + cellP("y"), kStyles));
    QTextCursor e = cursorAt(*empties, QStringLiteral("end"));
    const QTextBlock endBlock = e.block();
    e.setPosition(endBlock.previous().previous().position());
    e.setPosition(endBlock.position() + endBlock.length() - 1, QTextCursor::KeepAnchor);
    QVERIFY(mergeCells(*empties, e));
    QCOMPARE(plain(*empties), QStringLiteral("x\nend\ny"));
    checkTablesReopenTheSame(*empties);
}


// Merge Cells over rows and columns joins a block of cells: the first row's cell begins the join and the rows under have a cell that continues it; what the cells held goes in
// the first cell, and splitting it into as many rows as it joins makes cells of them again.
void WriteTests::mergeCellsOverRowsAndColumnsAndSplitThem()
{
    auto doc = open(tableDocx(3, 3));
    QTextCursor c(doc->story());
    selectCells(*doc, c, QStringLiteral("r0c1"), QStringLiteral("r1c2"));
    const QString before = tablesSignature(*doc);
    QVERIFY(mergeCells(*doc, c));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY(x.contains("<w:vMerge w:val=\"restart\"/>") && x.contains("<w:vMerge/>"));
    QCOMPARE(plain(*doc), QStringLiteral("before\nr0c0\nr0c1\nr0c2\nr1c1\nr1c2\nr1c0\n\nr2c0\nr2c1\nr2c2\nafter"));
    checkTablesReopenTheSame(*doc);
    QTextCursor in = cursorAt(*doc, QStringLiteral("r0c1"));
    QVERIFY(splitCells(*doc, in, 2, 2));
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 9);
    QVERIFY(!documentXml(doc->toDocx()).contains("vMerge"));
    QCOMPARE(plain(*doc), QStringLiteral("before\nr0c0\nr0c1\nr0c2\nr1c1\nr1c2\n\nr1c0\n\n\nr2c0\nr2c1\nr2c2\nafter"));
    checkTablesReopenTheSame(*doc);
    doc->story()->undo();
    doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
}


// A cell split into more columns than it takes makes columns of the grid: the cells of the other rows that cross them are wider, and the whole is as wide as it was. A cell
// split into rows adds rows, in which the cells beside it go on.
void WriteTests::splitCellsMakeColumnsAndRows()
{
    auto doc = open(tableDocx(2, 2));
    QTextCursor in = cursorAt(*doc, QStringLiteral("r0c0"));
    QVERIFY(splitCells(*doc, in, 3, 1));
    QString now = tablesSignature(*doc);
    {
        const QVector<double> columns = doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("r1c1")).block()))->table->columns;
        QCOMPARE(columns.size(), 4);
        QVERIFY(std::abs(columns[0] + columns[1] + columns[2] - 100) < 0.01 && std::abs(columns[0] - 33.3) < 0.1);
        QCOMPARE(columns[3], 100.0);
    }
    QVERIFY2(now.contains(QStringLiteral("CELL span=3")), qPrintable(now));
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 6);
    checkTablesReopenTheSame(*doc);
    doc->story()->undo();
    auto rows = open(tableDocx(2, 2));
    in = cursorAt(*rows, QStringLiteral("r0c0"));
    QVERIFY(splitCells(*rows, in, 1, 2));
    now = tablesSignature(*rows);
    QCOMPARE(int(now.count(QStringLiteral(" ROW "))), 3);
    QCOMPARE(plain(*rows), QStringLiteral("before\nr0c0\nr0c1\n\n\nr1c0\nr1c1\nafter"));
    const QByteArray x = documentXml(rows->toDocx());
    QCOMPARE(int(x.count("<w:vMerge w:val=\"restart\"/>")), 1);
    QCOMPARE(int(x.count("<w:vMerge/>")), 1);
    checkTablesReopenTheSame(*rows);
    // The row added is a row of the table: the cursor is in the new cell, and typing goes there.
    QTextCursor lower(rows->story());
    lower.setPosition(rows->story()->findBlock(cursorAt(*rows, QStringLiteral("r0c1")).position()).next().position());
    typeText(*rows, lower, QStringLiteral("below"));
    checkTablesReopenTheSame(*rows);
    QVERIFY(!splitCells(*rows, in, 1, 1));
}


// Convert Text to Table: each selected paragraph is a row cut at the separator, the table has as many columns as the longest row (the short rows get empty cells), the words stay
// in order, one step undoes it, and the file opens the same; Table to Text puts the cells' words between the separator again.
// Text too big for a table is refused before any words are taken: one huge paragraph of commas, and a hundred thousand short paragraphs.
void WriteTests::convertTextToTableRefusesHostileTextBeforeBuildingAnything()
{
    {
        auto doc = open(makeDocx(cellP("x") + cellP(QString(3000000, QLatin1Char(',')).toUtf8()) + cellP("y"), kStyles));
        QTextCursor c = cursorAt(*doc, QString(3000000, QLatin1Char(',')));
        const QString was = plain(*doc);
        const qint64 made = textToTableFragmentsMadeForTest();
        QVERIFY(!convertTextToTable(*doc, c, TextSeparator::Commas, QChar(), 1));
        QCOMPARE(textToTableFragmentsMadeForTest(), made);
        QVERIFY2(doc->tableNotice.contains(QStringLiteral("500 rows and 63 columns")), qPrintable(doc->tableNotice));
        QCOMPARE(plain(*doc), was);
    }
    {
        QByteArray body;
        for (int i = 0; i < 100000; ++i) body += "<w:p><w:r><w:t>p</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c(doc->story());
        c.select(QTextCursor::Document);
        const qint64 made = textToTableFragmentsMadeForTest();
        QVERIFY(!convertTextToTable(*doc, c, TextSeparator::Paragraphs, QChar(), 1));
        QCOMPARE(textToTableFragmentsMadeForTest(), made);
        QVERIFY2(doc->tableNotice.contains(QStringLiteral("500 rows and 63 columns")), qPrintable(doc->tableNotice));
        QTextCursor all(doc->story());
        all.select(QTextCursor::Document);
        QVERIFY(!convertTextToTable(*doc, all, TextSeparator::Paragraphs, QChar(), 64));   // (too many columns)
        QCOMPARE(textToTableFragmentsMadeForTest(), made);
    }
}


// The note, comment, and bookmark in text that becomes a table are the same ones in the cells (they are moved, not copied and taken out).
QString WriteTests::rangeText(WriteDoc &doc, int from, int to)
{
    QTextCursor r(doc.story());
    r.setPosition(from);
    r.setPosition(to, QTextCursor::KeepAnchor);
    return r.selectedText().remove(QChar(0xFFFC));
}

void WriteTests::convertTextToTableKeepsNotesCommentsAndBookmarks()
{
    auto doc = open(makeDocx(cellP("before") + "<w:p><w:r><w:t>aa</w:t></w:r><w:r><w:t xml:space=\"preserve\">\t</w:t></w:r><w:bookmarkStart w:id=\"7\" w:name=\"mark\"/><w:r><w:t>bb</w:t></w:r><w:bookmarkEnd w:id=\"7\"/>"
                                              "<w:r><w:t xml:space=\"preserve\">\t</w:t></w:r><w:r><w:t>cc</w:t></w:r></w:p>" + cellP("after"), kStyles));
    QTextCursor n = cursorAt(*doc, QStringLiteral("aa\tbb\tcc"));
    n.setPosition(n.block().position() + 2);
    QTextDocument *note = insertFootnote(*doc, n);
    QVERIFY(note);
    n.insertText(QStringLiteral("note words"));
    QTextCursor sel(doc->story());
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (b.text().startsWith(QStringLiteral("aa"))) sel.setPosition(b.position());
    {
        const QString text = sel.block().text();
        const int cc = int(text.indexOf(QStringLiteral("cc")));
        sel.setPosition(sel.block().position() + cc);
        sel.setPosition(sel.block().position() + cc + 2, QTextCursor::KeepAnchor);
        QVERIFY(addComment(*doc, sel, QStringLiteral("remark")) >= 0);
    }
    auto count = [](const QByteArray &x, const char *needle) { return int(x.count(needle)); };
    const QByteArray before = documentXml(doc->toDocx());
    QCOMPARE(count(before, "<w:footnoteReference"), 1);
    QCOMPARE(count(before, "<w:commentRangeStart"), 1);
    QCOMPARE(count(before, "w:name=\"mark\""), 1);
    {
        office::Package pkg;
        pkg.read(doc->toDocx());
        QVERIFY(pkg.part(QStringLiteral("/word/footnotes.xml")).contains("note words"));
    }
    {
        const QVector<Comment> was = comments(*doc);
        QCOMPARE(rangeText(*doc, was[0].from, was[0].to), QStringLiteral("cc"));
    }
    QTextCursor c(doc->story());
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (b.text().startsWith(QStringLiteral("aa"))) c.setPosition(b.position());
    QVERIFY(convertTextToTable(*doc, c, TextSeparator::Tabs, QChar(), 1));
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(count(x, "<w:footnoteReference"), 1);
    QCOMPARE(count(x, "<w:commentRangeStart"), 1);
    QCOMPARE(count(x, "<w:commentRangeEnd"), 1);
    QCOMPARE(count(x, "w:name=\"mark\""), 1);
    QCOMPARE(count(x, "<w:bookmarkEnd"), 1);
    {
        office::Package pkg;
        pkg.read(doc->toDocx());
        QVERIFY(pkg.part(QStringLiteral("/word/footnotes.xml")).contains("note words"));
        const StoryScan scan = scanStory(doc->story());
        QCOMPARE(int(scan.bookmarks.size()), 1);
        QCOMPARE(rangeText(*doc, scan.bookmarks[0].start, scan.bookmarks[0].end), QStringLiteral("bb"));
    }
    QCOMPARE(int(comments(*doc).size()), 1);
    QCOMPARE(int(doc->footnoteReferences(1).size()), 1);
    checkTablesReopenTheSame(*doc);
    const QVector<Comment> all = comments(*doc);
    QVERIFY(all[0].from >= 0 && all[0].to > all[0].from);
    QCOMPARE(rangeText(*doc, all[0].from, all[0].to), QStringLiteral("cc"));
    doc->story()->undo();
    QCOMPARE(count(documentXml(doc->toDocx()), "<w:footnoteReference"), 1);
    QCOMPARE(int(comments(*doc).size()), 1);
}


void WriteTests::convertTextToTableAndBack()
{
    auto doc = open(makeDocx(cellP("before") + cellP("a\tb\tc") + cellP("d\te") + cellP("after"), kStyles));
    QTextCursor c(doc->story());
    selectCells(*doc, c, QStringLiteral("a\tb\tc"), QStringLiteral("d\te"));
    const QString was = plain(*doc);
    QVERIFY(canConvertTextToTable(*doc, c));
    QVERIFY(convertTextToTable(*doc, c, TextSeparator::Tabs, QChar(), 1));
    QCOMPARE(plain(*doc), QStringLiteral("before\na\nb\nc\nd\ne\n\nafter"));
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:tr>")), 2);
    QCOMPARE(int(x.count("<w:tc>")), 6);
    checkTablesReopenTheSame(*doc);
    QVERIFY(!canConvertTextToTable(*doc, c));
    // And back, with commas.
    QTextCursor in = cursorAt(*doc, QStringLiteral("b"));
    QVERIFY(convertTableToText(*doc, in, TextSeparator::Commas, QChar()));
    QCOMPARE(plain(*doc), QStringLiteral("before\na,b,c\nd,e,\nafter"));
    QVERIFY(!documentXml(doc->toDocx()).contains("<w:tbl>"));
    checkTablesReopenTheSame(*doc);
    doc->story()->undo();
    QCOMPARE(plain(*doc), QStringLiteral("before\na\nb\nc\nd\ne\n\nafter"));
    doc->story()->undo();
    QCOMPARE(plain(*doc), was);
    QVERIFY(!documentXml(doc->toDocx()).contains("<w:tbl>"));
}


// Paragraphs as cells make rows of as many cells as asked; another character cuts the text too; a table made of the story's last paragraphs leaves a paragraph after it; words
// keep their bold.
void WriteTests::convertTextToTableByParagraphsOtherCharacterAndAtTheEnd()
{
    auto doc = open(makeDocx(cellP("x") + cellP("y") + "<w:p><w:r><w:rPr><w:b/></w:rPr><w:t>z;w</w:t></w:r></w:p>", kStyles));
    QTextCursor c(doc->story());
    c.select(QTextCursor::Document);
    QVERIFY(convertTextToTable(*doc, c, TextSeparator::Paragraphs, QChar(), 2));
    QCOMPARE(plain(*doc), QStringLiteral("x\ny\nz;w\n\n"));
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tr>")), 2);
    checkTablesReopenTheSame(*doc);
    QVERIFY(!cellKeyOf(doc->story()->lastBlock()));
    QTextCursor in = cursorAt(*doc, QStringLiteral("z;w"));
    QVERIFY(convertTableToText(*doc, in, TextSeparator::Paragraphs, QChar()));
    QCOMPARE(plain(*doc), QStringLiteral("x\ny\nz;w\n\n"));
    QVERIFY(!documentXml(doc->toDocx()).contains("<w:tbl>"));
    auto semi = open(makeDocx(cellP("p;q") + "<w:p><w:r><w:rPr><w:b/></w:rPr><w:t>r;s</w:t></w:r></w:p>" + cellP("end"), kStyles));
    QTextCursor s(semi->story());
    selectCells(*semi, s, QStringLiteral("p;q"), QStringLiteral("r;s"));
    QVERIFY(convertTextToTable(*semi, s, TextSeparator::Other, QChar(';'), 1));
    QCOMPARE(plain(*semi), QStringLiteral("p\nq\nr\ns\nend"));
    QTextCursor bold = cursorAt(*semi, QStringLiteral("s"));
    bold.setPosition(bold.position() + 1);
    QVERIFY(bold.charFormat().fontWeight() > QFont::Normal);
    checkTablesReopenTheSame(*semi);
    QTextCursor none = cursorAt(*semi, QStringLiteral("end"));
    QVERIFY(!convertTextToTable(*semi, s, TextSeparator::Other, QChar(), 1));   // (in a table now)
    Q_UNUSED(none);
}


// Split Table cuts a table above the row the cursor is in into two with an empty paragraph between (two tables in the file), one step undoes it; the first row and a cut through a
// cell that is joined down are refused.
void WriteTests::splitTableCutsAboveTheCursorsRow()
{
    auto doc = open(tableDocx(3, 2));
    const QString before = tablesSignature(*doc);
    QTextCursor in = cursorAt(*doc, QStringLiteral("r1c1"));
    QVERIFY(canSplitTable(*doc, in));
    QVERIFY(splitTable(*doc, in));
    QCOMPARE(plain(*doc), QStringLiteral("before\nr0c0\nr0c1\n\nr1c0\nr1c1\nr2c0\nr2c1\nafter"));
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:tbl>")), 2);
    QCOMPARE(int(x.count("<w:tr>")), 3);
    checkTablesReopenTheSame(*doc);
    QTextCursor top = cursorAt(*doc, QStringLiteral("r0c0"));
    QVERIFY(!canSplitTable(*doc, top));
    QVERIFY(!splitTable(*doc, top));
    doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tbl>")), 1);
    // A cell joined down.
    auto joined = open(tableDocx(3, 2));
    QTextCursor c(joined->story());
    selectCells(*joined, c, QStringLiteral("r0c0"), QStringLiteral("r1c0"));
    QVERIFY(mergeCells(*joined, c));
    QTextCursor low = cursorAt(*joined, QStringLiteral("r1c1"));
    QVERIFY(!splitTable(*joined, low));
    QVERIFY(!joined->tableNotice.isEmpty());
}


// A table with a header row and rows that have a cell across two columns is written so that it opens the same after Split Table and after Sort.
void WriteTests::splitAndSortKeepHeaderRowsAndSpannedCells()
{
    auto wide = [&](const char *a, const char *b) { return rowX(cellX(cellP(a), "<w:gridSpan w:val=\"2\"/>", 4800) + cellX(cellP(b))); };
    const QByteArray body = cellP("top") + tableX(rowX(cellX(cellP("H1")) + cellX(cellP("H2")) + cellX(cellP("H3")), "<w:tblHeader/>") + wide("pear", "3") + wide("apple", "2") + wide("fig", "1"), 3) + cellP("tail");
    auto doc = open(makeDocx(body, kStyles));
    const QString before = tablesSignature(*doc);
    checkTablesReopenTheSame(*doc);
    QTextCursor in = cursorAt(*doc, QStringLiteral("apple"));
    TableSortKey text;
    QVERIFY(sortTableRows(*doc, in, {text}, true));
    QVERIFY2(plain(*doc).contains(QStringLiteral("apple")), qPrintable(plain(*doc)));
    QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:tblHeader/>")), 1);
    QCOMPARE(int(x.count("<w:gridSpan w:val=\"2\"/>")), 3);
    QVERIFY(x.indexOf("<w:tblHeader/>") < x.indexOf("apple"));   // (the header row stays first)
    checkTablesReopenTheSame(*doc);
    QTextCursor low = cursorAt(*doc, QStringLiteral("pear"));
    QVERIFY(splitTable(*doc, low));
    x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:tbl>")), 2);
    QCOMPARE(int(x.count("<w:tblHeader/>")), 1);
    QCOMPARE(int(x.count("<w:gridSpan w:val=\"2\"/>")), 3);
    checkTablesReopenTheSame(*doc);
    doc->story()->undo();
    doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
}


// Sort puts the rows in order of a column (text, numbers, or dates, either way, up to three keys), keeping the header row where it is; the words of a row move together, one step
// undoes it, and the file opens the same.
void WriteTests::sortTableRowsByTextNumbersAndDates()
{
    auto row = [&](const char *a, const char *b, const char *d) { return rowX(cellX(cellP(a)) + cellX(cellP(b)) + cellX(cellP(d))); };
    const QByteArray body = cellP("top") + tableX(row("Name", "Qty", "Day") + row("pear", "10", "3/5/2026") + row("Apple", "9", "1/20/2026") + row("fig", "$1,200", "12/1/2025") + row("apple", "9", "2/2/2026"), 3) + cellP("tail");
    auto doc = open(makeDocx(body, kStyles));
    QTextCursor in = cursorAt(*doc, QStringLiteral("pear"));
    const QString before = tablesSignature(*doc);
    TableSortKey text;
    QVERIFY(sortTableRows(*doc, in, {text}, true));
    QCOMPARE(plain(*doc), QStringLiteral("top\nName\nQty\nDay\nApple\n9\n1/20/2026\napple\n9\n2/2/2026\nfig\n$1,200\n12/1/2025\npear\n10\n3/5/2026\ntail"));
    checkTablesReopenTheSame(*doc);
    TableSortKey number;
    number.kind = TableSortKey::Kind::Number;
    number.column = 1;
    number.ascending = false;
    TableSortKey byDay;
    byDay.kind = TableSortKey::Kind::Date;
    byDay.column = 2;
    byDay.ascending = false;
    QVERIFY(sortTableRows(*doc, in, {number, byDay}, true));
    QCOMPARE(plain(*doc), QStringLiteral("top\nName\nQty\nDay\nfig\n$1,200\n12/1/2025\npear\n10\n3/5/2026\napple\n9\n2/2/2026\nApple\n9\n1/20/2026\ntail"));
    checkTablesReopenTheSame(*doc);
    TableSortKey dates;
    dates.kind = TableSortKey::Kind::Date;
    dates.column = 2;
    QVERIFY(sortTableRows(*doc, in, {dates}, true));
    QVERIFY(plain(*doc).contains(QStringLiteral("fig\n$1,200\n12/1/2025\nApple")));
    doc->story()->undo();
    doc->story()->undo();
    doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
    // Rows that cells are joined down in can't be sorted.
    auto joined = open(tableDocx(3, 2));
    QTextCursor c(joined->story());
    selectCells(*joined, c, QStringLiteral("r0c0"), QStringLiteral("r1c0"));
    QVERIFY(mergeCells(*joined, c));
    QTextCursor whole = cursorAt(*joined, QStringLiteral("r2c1"));
    QVERIFY(!sortTableRows(*joined, whole, {text}, false));
    QVERIFY(!joined->tableNotice.isEmpty());
}


// What a formula's cell names stand for: A1 and A1:B2 by a column's letters and a row's number, and the cells with numbers above, below, left, and right up to an empty cell.
// A bookmark named like a cell is the bookmark in a formula (the document's own names come first), alone and in a function.
void WriteTests::aBookmarkNamedLikeACellWinsInAFormula()
{
    const QByteArray mark = "<w:p><w:bookmarkStart w:id=\"9\" w:name=\"A1\"/><w:r><w:t>100</w:t></w:r><w:bookmarkEnd w:id=\"9\"/></w:p>";
    auto field = [](const char *code) { return QByteArray("<w:p><w:fldSimple w:instr=\" ") + code + " \"><w:r><w:t>0</w:t></w:r></w:fldSimple></w:p>"; };
    auto doc = open(makeDocx(mark + tableX(rowX(cellX(cellP("1")) + cellX(field("=A1*2")) + cellX(field("=SUM(A1)")) + cellX(field("=B2*2"))), 4) + cellP("after"), kStyles));
    updateFields(*doc);
    const QString text = doc->story()->toPlainText();
    QVERIFY2(text.contains(QStringLiteral("200")) && text.contains(QStringLiteral("100")), qPrintable(text));
    QVERIFY2(!text.contains(QStringLiteral("\n2\n")), qPrintable(text));
}


// Many formulas over a huge range look at no more cells than the pass has steps for.
void WriteTests::manyFormulasOverAHugeRangeFinishInBoundedTime()
{
    QByteArray rows;
    for (int r = 0; r < 200; ++r) {
        QByteArray cells = cellX(QByteArray("<w:p><w:fldSimple w:instr=\" =SUM(A1:ZZ99999) \"><w:r><w:t>0</w:t></w:r></w:fldSimple></w:p>"));
        for (int c = 1; c < 10; ++c) cells += cellX(cellP("1"));
        rows += rowX(cells);
    }
    auto doc = open(makeDocx(tableX(rows, 10) + cellP("after"), kStyles));
    const qint64 before = formulaRangeCellsScannedForTest();
    updateFields(*doc);
    const qint64 scanned = formulaRangeCellsScannedForTest() - before;
    QVERIFY2(scanned <= 200000 + 2000, qPrintable(QString::number(scanned)));
    QVERIFY(scanned > 0);
}


// Updating many formula fields makes the table's index once for the pass, not once for each field.
void WriteTests::updatingManyFormulaFieldsBuildsTheIndexOnce()
{
    QByteArray rows;
    for (int r = 0; r < 500; ++r) rows += rowX(cellX(cellP("1")) + cellX(QByteArray("<w:p><w:fldSimple w:instr=\" =SUM(LEFT)*100 \"><w:r><w:t>0</w:t></w:r></w:fldSimple></w:p>")));
    auto doc = open(makeDocx(paragraphs(300) + tableX(rows, 2) + cellP("after"), kStyles));
    const qint64 before = tableIndexRebuildsForTest();
    QElapsedTimer timer;
    timer.start();
    updateFields(*doc);
    qWarning("index rebuilds %lld, %lld ms", tableIndexRebuildsForTest() - before, timer.elapsed());
    QVERIFY2(tableIndexRebuildsForTest() - before <= 5, qPrintable(QString::number(tableIndexRebuildsForTest() - before)));
    QVERIFY(doc->story()->toPlainText().remove(QChar(0xFFFC)).contains(QStringLiteral("\n1\n100\n1\n100\n")));
}


void WriteTests::tableCellValuesAreFoundByNameAndDirection()
{
    auto row = [&](const char *a, const char *b) { return rowX(cellX(cellP(a)) + cellX(cellP(b))); };
    auto doc = open(makeDocx(tableX(row("Item", "Cost") + row("a", "$1,200.50") + row("b", "(3)") + row("c", "20%") + row("", "") + row("d", "99") + row("Total", "x"), 2), kStyles));
    const int total = cursorAt(*doc, QStringLiteral("x")).position();
    QCOMPARE(*tableCellValues(*doc, total, QStringLiteral("ABOVE")), (QVector<double>{99}));   // (stops at the empty cell)
    const int third = cursorAt(*doc, QStringLiteral("20%")).position();
    QCOMPARE(*tableCellValues(*doc, third, QStringLiteral("ABOVE")), (QVector<double>{-3, 1200.5}));
    QCOMPARE(*tableCellValues(*doc, third, QStringLiteral("LEFT")), QVector<double>());   // ("c" is not a number)
    QCOMPARE(*tableCellValues(*doc, third, QStringLiteral("b2")), (QVector<double>{1200.5}));
    QCOMPARE(*tableCellValues(*doc, third, QStringLiteral("B2:B4")), (QVector<double>{1200.5, -3}));   // (its own cell is left out)
    QVERIFY(!tableCellValues(*doc, third, QStringLiteral("A2")).has_value());   // (words)
    QVERIFY(!tableCellValues(*doc, third, QStringLiteral("C1")).has_value());   // (no such cell)
    QCOMPARE(*tableCellValues(*doc, cursorAt(*doc, QStringLiteral("a")).position(), QStringLiteral("RIGHT")), (QVector<double>{1200.5}));
    QCOMPARE(*tableCellValues(*doc, cursorAt(*doc, QStringLiteral("Item")).position(), QStringLiteral("BELOW")), QVector<double>());   // ("a" is words, then the next is a cell of the first column: words again)
}



// Shading and borders are set on the cells selected (and the lines also on the sides of the cells beside them that touch), written to the file, and undone as one step.
void WriteTests::shadingAndBordersOfTheSelectedCellsAreWrittenAndUndone()
{
    auto doc = open(tableDocx(2, 2));
    QTextCursor c(doc->story());
    const QString before = tablesSignature(*doc);
    selectCells(*doc, c, QStringLiteral("r0c0"), QStringLiteral("r0c1"));
    setShading(*doc, c, QColor(0xFF, 0xFF, 0x00));
    QString now = tablesSignature(*doc);
    QCOMPARE(int(now.count(QStringLiteral("fill=#ffff00"))), 2);
    QVERIFY(documentXml(doc->toDocx()).contains("w:fill=\"FFFF00\""));
    checkTablesReopenTheSame(*doc);
    setShading(*doc, c, QColor());
    QCOMPARE(int(tablesSignature(*doc).count(QStringLiteral("fill=none"))), 2);
    checkTablesReopenTheSame(*doc);
    // A red double line round the second row.
    selectCells(*doc, c, QStringLiteral("r1c0"), QStringLiteral("r1c1"));
    flow::TableLine red;
    red.style = flow::TableLine::Style::Double;
    red.width = 1.5;
    red.color = QColor(0xFF, 0, 0);
    setBorders(*doc, c, BorderSide::Outside, red);
    const flow::CellRecord *a = doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("r1c0")).block()));
    QVERIFY(a->cell.top && a->cell.top->style == flow::TableLine::Style::Double && a->cell.top->color == QColor(0xFF, 0, 0));
    QVERIFY(a->cell.left && a->cell.left->style == flow::TableLine::Style::Double);
    QVERIFY(!a->cell.right || a->cell.right->style != flow::TableLine::Style::Double);   // (between the two cells: not an outside line)
    const flow::CellRecord *above = doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("r0c0")).block()));
    QVERIFY(above->cell.bottom && above->cell.bottom->style == flow::TableLine::Style::Double);   // (the cell above has the same line on its foot)
    QVERIFY(documentXml(doc->toDocx()).contains("<w:top w:val=\"double\" w:sz=\"12\" w:space=\"0\" w:color=\"FF0000\"/>"));
    checkTablesReopenTheSame(*doc);
    // No lines at all between the cells of the table.
    selectCells(*doc, c, QStringLiteral("r0c0"), QStringLiteral("r1c1"));
    setBorders(*doc, c, BorderSide::All, flow::TableLine());
    const flow::CellRecord *none = doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("r1c1")).block()));
    QVERIFY(none->cell.top && !none->cell.top->visible() && none->cell.right && !none->cell.right->visible());
    checkTablesReopenTheSame(*doc);
    while (doc->story()->isUndoAvailable()) doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
}


// The nine places of text in a cell: across is the paragraphs' alignment, down is the cell's.
void WriteTests::cellAlignmentIsAcrossInTheParagraphsAndDownInTheCell()
{
    auto doc = open(tableDocx(2, 2));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r1c1"));
    setCellAlignment(*doc, c, Qt::AlignHCenter, flow::CellAlign::Bottom);
    const QTextBlock b = cursorAt(*doc, QStringLiteral("r1c1")).block();
    QCOMPARE(b.blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
    QCOMPARE(doc->flow().tables.get(cellKeyOf(b))->cell.valign, flow::CellAlign::Bottom);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY(x.contains("<w:vAlign w:val=\"bottom\"/>") && x.contains("<w:jc w:val=\"center\"/>"));
    checkTablesReopenTheSame(*doc);
    QCOMPARE(doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("r0c0")).block()))->cell.valign, flow::CellAlign::Top);
    doc->story()->undo();
    doc->story()->undo();
    QCOMPARE(doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("r1c1")).block()))->cell.valign, flow::CellAlign::Top);
}


// The direction of a cell's text: from the top down or from the bottom up is written as w:textDirection in the place the schema gives it, read again, and one step to undo.
void WriteTests::cellTextDirectionIsKeptInTheCellAndInTheFile()
{
    auto doc = open(tableDocx(2, 2));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r1c1"));
    setCellAlignment(*doc, c, Qt::AlignLeft, flow::CellAlign::Bottom);
    c = cursorAt(*doc, QStringLiteral("r1c1"));
    setCellTextDirection(*doc, c, flow::CellDirection::BottomToTop);
    QCOMPARE(doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("r1c1")).block()))->cell.direction, flow::CellDirection::BottomToTop);
    QCOMPARE(doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("r0c0")).block()))->cell.direction, flow::CellDirection::Horizontal);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY(x.contains("<w:textDirection w:val=\"btLr\"/>"));
    // (In the order of the schema: the direction comes before the vertical alignment.)
    QVERIFY(x.indexOf("<w:textDirection") < x.indexOf("<w:vAlign"));
    checkTablesReopenTheSame(*doc);
    auto back = open(doc->toDocx());
    QCOMPARE(back->flow().tables.get(cellKeyOf(cursorAt(*back, QStringLiteral("r1c1")).block()))->cell.direction, flow::CellDirection::BottomToTop);
    c = cursorAt(*doc, QStringLiteral("r1c1"));
    setCellTextDirection(*doc, c, flow::CellDirection::TopToBottom);
    QVERIFY(documentXml(doc->toDocx()).contains("<w:textDirection w:val=\"tbRl\"/>"));
    c = cursorAt(*doc, QStringLiteral("r1c1"));
    setCellTextDirection(*doc, c, flow::CellDirection::Horizontal);
    QVERIFY(!documentXml(doc->toDocx()).contains("textDirection"));
    // The same direction again changes nothing (no step to undo).
    const QByteArray same = documentXml(doc->toDocx());
    c = cursorAt(*doc, QStringLiteral("r1c1"));
    setCellTextDirection(*doc, c, flow::CellDirection::Horizontal);
    QCOMPARE(documentXml(doc->toDocx()), same);
    // Table Properties reads and sets it.
    c = cursorAt(*doc, QStringLiteral("r0c1"));
    const TableProps was = tablePropsAt(*doc, c);
    TableProps now = was;
    now.direction = flow::CellDirection::TopToBottom;
    setTableProps(*doc, c, was, now);
    QCOMPARE(tablePropsAt(*doc, cursorAt(*doc, QStringLiteral("r0c1"))).direction, flow::CellDirection::TopToBottom);
}


// Column width, row height, equal columns and rows, and the fit of the table: the grid and the cells' widths are written together.
void WriteTests::cellSizeAndFitAreWrittenWithTheGrid()
{
    auto doc = open(tableDocx(2, 3));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    setColumnWidth(*doc, c, 150);
    QString now = tablesSignature(*doc);
    QVERIFY2(now.contains(QStringLiteral("cols=150.0,100.0,100.0")), qPrintable(now));
    QByteArray x = documentXml(doc->toDocx());
    QVERIFY(x.contains("<w:gridCol w:w=\"3000\"/>"));
    QVERIFY(x.contains("<w:tcW w:w=\"3000\" w:type=\"dxa\"/>"));
    checkTablesReopenTheSame(*doc);
    distributeColumns(*doc, c);
    QVERIFY2(tablesSignature(*doc).contains(QStringLiteral("cols=116.7,116.7,116.7")), qPrintable(tablesSignature(*doc)));
    checkTablesReopenTheSame(*doc);
    c = cursorAt(*doc, QStringLiteral("r1c1"));
    setRowHeight(*doc, c, 40, true);
    now = tablesSignature(*doc);
    QVERIFY2(now.contains(QStringLiteral("ROW height=40 exact=1")), qPrintable(now));
    QVERIFY(documentXml(doc->toDocx()).contains("<w:trHeight w:val=\"800\" w:hRule=\"exact\"/>"));
    checkTablesReopenTheSame(*doc);
    setRowHeight(*doc, c, 0, false);
    QVERIFY(!documentXml(doc->toDocx()).contains("trHeight"));
    QTextCursor all(doc->story());
    selectCells(*doc, all, QStringLiteral("r0c0"), QStringLiteral("r1c2"));
    distributeRows(*doc, all);
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:trHeight")), 2);
    checkTablesReopenTheSame(*doc);
    fitTable(*doc, c, TableFit::Window);
    x = documentXml(doc->toDocx());
    QVERIFY(x.contains("<w:tblW w:w=\"5000\" w:type=\"pct\"/>") && x.contains("<w:tblLayout w:type=\"autofit\"/>"));
    double sum = 0;
    for (double w : doc->flow().tables.get(cellKeyOf(c.block()))->table->columns) sum += w;
    QVERIFY2(std::abs(sum - (textWidthAt(*doc, 0) - 10.8)) < 0.6, qPrintable(QString::number(sum)));   // (the text area less the cells' side margins)
    checkTablesReopenTheSame(*doc);
    fitTable(*doc, c, TableFit::Contents);
    QVERIFY(documentXml(doc->toDocx()).contains("<w:tblW w:w=\"0\" w:type=\"auto\"/>"));
    QVERIFY(doc->flow().tables.get(cellKeyOf(c.block()))->table->columns[0] < 100);   // ("r0c0" is narrower than 100 points)
    checkTablesReopenTheSame(*doc);
    fitTable(*doc, c, TableFit::Fixed);
    QVERIFY(documentXml(doc->toDocx()).contains("<w:tblLayout w:type=\"fixed\"/>"));
    checkTablesReopenTheSame(*doc);
}


// Table Properties: the table's alignment, indent, width, margins, and spacing, the row's height, whether it splits across pages and repeats at the top, and the cell's place
// for its text are read from the table and set on it, and the file has them.
void WriteTests::tablePropertiesAreReadSetAndWritten()
{
    auto doc = open(tableDocx(3, 2));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r1c1"));
    const TableProps was = tablePropsAt(*doc, c);
    QCOMPARE(was.widthKind, TableProps::Width::Auto);
    QVERIFY(!was.header && !was.cantSplit);
    TableProps now = was;
    now.align = flow::TableData::Align::Center;
    now.indent = 10;
    now.widthKind = TableProps::Width::Percent;
    now.width = 80;
    now.margins = QMarginsF(7, 1, 7, 2);
    now.spacing = 2;
    now.rowHeight = 30;
    now.rowExact = false;
    now.cantSplit = true;
    now.header = true;
    now.valign = flow::CellAlign::Center;
    now.altTitle = QStringLiteral("Totals");
    now.altText = QStringLiteral("The totals & the rest");
    setTableProps(*doc, c, was, now);
    const TableProps again = tablePropsAt(*doc, c);
    QVERIFY(again == now);
    const QByteArray x = documentXml(doc->toDocx());
    for (const char *want : {"<w:jc w:val=\"center\"/>", "<w:tblInd w:w=\"200\" w:type=\"dxa\"/>", "<w:tblW w:w=\"4000\" w:type=\"pct\"/>", "<w:cantSplit/>", "<w:tblHeader/>",
                             "<w:trHeight w:val=\"600\" w:hRule=\"atLeast\"/>", "<w:tblCellSpacing w:w=\"40\" w:type=\"dxa\"/>", "<w:tblCaption w:val=\"Totals\"/>",
                             "<w:tblDescription w:val=\"The totals &amp; the rest\"/>", "<w:vAlign w:val=\"center\"/>"})
        QVERIFY2(x.contains(want), want);
    checkTablesReopenTheSame(*doc);
    auto back = open(doc->toDocx());
    QTextCursor in = cursorAt(*back, QStringLiteral("r1c1"));
    QVERIFY(tablePropsAt(*back, in) == now);
    // The row is a header row: it is laid out to repeat.
    const flow::CellRecord *rec = doc->flow().tables.get(cellKeyOf(c.block()));
    QVERIFY(rec->row->cantSplit);
    while (doc->story()->isUndoAvailable()) doc->story()->undo();
    QVERIFY(tablePropsAt(*doc, c) == was);
}


// The table style (a built-in one is added to the document), its options, and Clear: the cells take the look the style gives them, and the file says the style and the options.
void WriteTests::aTableStyleAndItsOptionsGiveTheCellsTheirLook()
{
    auto doc = open(tableDocx(3, 2));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    setTableStyle(*doc, c, QStringLiteral("GridTable4-Accent1"));
    QVERIFY(styleXml(doc->toDocx()).contains("w:styleId=\"GridTable4-Accent1\""));
    QVERIFY(documentXml(doc->toDocx()).contains("<w:tblStyle w:val=\"GridTable4-Accent1\"/>"));
    auto fillOfCell = [&](const QString &text) {
        const flow::CellRecord *r = doc->flow().tables.get(cellKeyOf(cursorAt(*doc, text).block()));
        return r->cell.fill ? r->cell.fill->name() : QString();
    };
    QCOMPARE(fillOfCell(QStringLiteral("r0c0")), QStringLiteral("#4472c4"));     // (the header row)
    QCOMPARE(fillOfCell(QStringLiteral("r1c0")), QStringLiteral("#d9e2f3"));     // (the first banded row)
    QCOMPARE(fillOfCell(QStringLiteral("r2c0")), QString());                      // (the second is plain)
    const office::wml::TableLayer *layer = doc->tableLayerOf(cellKeyOf(cursorAt(*doc, QStringLiteral("r0c0")).block()));
    QVERIFY(layer && layer->r.bold.value_or(false));
    checkTablesReopenTheSame(*doc);
    setTableLook(*doc, c, 0x20, false);   // no header row
    QCOMPARE(fillOfCell(QStringLiteral("r0c0")), QStringLiteral("#d9e2f3"));
    QVERIFY(tablePropsAt(*doc, c).look == (0x04A0 & ~0x20));
    checkTablesReopenTheSame(*doc);
    setTableLook(*doc, c, 0x20, true);
    setTableStyle(*doc, c, QStringLiteral("TableNormal"));
    QCOMPARE(fillOfCell(QStringLiteral("r0c0")), QString());
    checkTablesReopenTheSame(*doc);
    // Every built-in style gives a table that opens the same.
    for (const QString &id : office::wml::builtinTableStyleIds()) {
        setTableStyle(*doc, c, id);
        checkTablesReopenTheSame(*doc);
    }
}


// Rows and columns added to or taken from a table with a style that bands its rows, or dresses its first and last ones, are dressed as the table is when it is opened again: what
// the style gives a cell goes by the cell's place, which the new row or column changes for those after it.
void WriteTests::rowsAndColumnsAddedToAStyledTableAreDressedAsTheFileOpensThem()
{
    auto doc = open(tableDocx(5, 3));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r1c1"));
    setTableStyle(*doc, c, QStringLiteral("GridTable4-Accent1"));
    setTableLook(*doc, c, 0x40 | 0x100 | 0x400, true);   // (a total row, a last column, and no banded columns)
    checkTablesReopenTheSame(*doc);
    insertTableRow(*doc, c, true);
    checkTablesReopenTheSame(*doc);
    insertTableRow(*doc, c, false);
    checkTablesReopenTheSame(*doc);
    insertTableColumn(*doc, c, true);
    checkTablesReopenTheSame(*doc);
    deleteTableRows(*doc, c);
    checkTablesReopenTheSame(*doc);
    deleteTableColumns(*doc, c);
    checkTablesReopenTheSame(*doc);
    c = cursorAt(*doc, QStringLiteral("r3c0"));
    c.movePosition(QTextCursor::EndOfBlock);
    tabThroughCells(*doc, c, false);
    checkTablesReopenTheSame(*doc);
    const QString full = tablesSignature(*doc);
    while (doc->story()->isUndoAvailable()) doc->story()->undo();
    QVERIFY(tablesSignature(*doc) != full);
}


// ---- review fixes of beta wave B

// Every command of a table changes the records of the table's cells; the ones no paragraph holds and no step of the history can bring back are let go, so that many commands on
// a long table keep the records within (the cells it has) and (those the history can reach), and a new edit after Undo, which ends the steps Redo would have made, lets go what
// they held. The history is never cut for the sake of room.
void WriteTests::aTablesRecordsAreFreedAsItIsChanged()
{
    auto doc = open(tableDocx(2000, 3));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    const QByteArray started = documentXml(doc->toDocx());
    for (int i = 0; i < 50; ++i) {
        if (i % 3 == 0) setShading(*doc, c, QColor((i * 4) % 256, 0, 0));
        else if (i % 3 == 1) setColumnWidth(*doc, c, 90 + i);
        else setRowHeight(*doc, c, 20 + i, false);
        const int reached = tableRecordsReachableForTest(*doc);
        QVERIFY2(doc->flow().tables.liveCount() <= std::max(reached, 30000 + 6000), qPrintable(QStringLiteral("%1 held, %2 reachable").arg(doc->flow().tables.liveCount()).arg(reached)));
    }
    // All of it undone: the document is as it was opened, and every step was there to undo.
    int undone = 0;
    while (doc->canUndo()) {
        doc->undo();
        ++undone;
    }
    QCOMPARE(undone, 50);
    QCOMPARE(documentXml(doc->toDocx()), started);
    QVERIFY(doc->canRedo());
    // A new edit ends what Redo would have made, and the records of those steps are let go.
    const int before = doc->flow().tables.liveCount();
    setShading(*doc, c, QColor(1, 2, 3));
    QVERIFY2(doc->flow().tables.liveCount() < before / 4 && doc->flow().tables.liveCount() <= 3 * 6000 + 100, qPrintable(QString::number(doc->flow().tables.liveCount())));
    QVERIFY(!doc->canRedo());
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tr>")), 2000);
    auto back = open(doc->toDocx());
    QVERIFY(back);
    QCOMPARE(back->flow().tables.liveCount(), 6000);
    doc->undo();
    QCOMPARE(documentXml(doc->toDocx()), started);
}


// A command that would make a table the reader's limits cut (256 columns, 20,000 rows) is refused with a reason, whatever the command.
void WriteTests::commandsThatWouldPassTheReadersLimitsAreRefused()
{
    auto wide = open(tableDocx(2, 256));
    QTextCursor c = cursorAt(*wide, QStringLiteral("r0c0"));
    const QByteArray was = wide->toDocx();
    wide->tableNotice.clear();
    insertTableColumn(*wide, c, true);
    QVERIFY(!wide->tableNotice.isEmpty());
    QCOMPARE(documentXml(wide->toDocx()), documentXml(was));
    wide->tableNotice.clear();
    QVERIFY(!splitCells(*wide, c, 2, 1));   // (a cell of one column into two: 257)
    QVERIFY(!wide->tableNotice.isEmpty());
    QCOMPARE(documentXml(wide->toDocx()), documentXml(was));
    // One column short of the limit: a column and a split are fine, up to it.
    auto nearly = open(tableDocx(2, 255));
    QTextCursor n = cursorAt(*nearly, QStringLiteral("r0c0"));
    nearly->tableNotice.clear();
    QVERIFY(splitCells(*nearly, n, 2, 1));
    QVERIFY(nearly->tableNotice.isEmpty());
    checkTablesReopenTheSame(*nearly);
    // Rows.
    auto tall = open(tableDocx(20000, 1));
    QTextCursor t = cursorAt(*tall, QStringLiteral("r19999c0"));
    const QString plainWas = plain(*tall);
    tall->tableNotice.clear();
    insertTableRow(*tall, t, true);
    QVERIFY(!tall->tableNotice.isEmpty());
    tall->tableNotice.clear();
    QVERIFY(tabThroughCells(*tall, t, false));
    QVERIFY(!tall->tableNotice.isEmpty());
    tall->tableNotice.clear();
    QVERIFY(!splitCells(*tall, t, 1, 2));
    QVERIFY(!tall->tableNotice.isEmpty());
    QCOMPARE(plain(*tall), plainWas);
    QCOMPARE(int(documentXml(tall->toDocx()).count("<w:tr>")), 20000);
}


// A cell split into more columns than it takes leaves the gaps of the rows that begin or end later as they are: the gap grows when the new columns are in it, and the width of
// the gap with it. Setting a column's width does the same.
void WriteTests::splittingAndSizingKeepTheGapsOfTheOtherRowsRight()
{
    // Four columns of 2000: row A has a cell on columns 0, 1 to 2, and 3; row B has its one cell in the last column, three columns empty before it, and row C has one cell
    // with three empty after it.
    const QByteArray body = cellP("top") +
                            tableX(rowX(cellX(cellP("A0")) + cellX(cellP("A12"), "<w:gridSpan w:val=\"2\"/>", 4000) + cellX(cellP("A3"))) +
                                       rowX(cellX(cellP("B3")), "<w:gridBefore w:val=\"3\"/><w:wBefore w:w=\"6000\" w:type=\"dxa\"/>") +
                                       rowX(cellX(cellP("C0")), "<w:gridAfter w:val=\"3\"/><w:wAfter w:w=\"6000\" w:type=\"dxa\"/>"),
                                   4, 2000) +
                            cellP("tail");
    auto doc = open(makeDocx(body, kStyles));
    QTextCursor in = cursorAt(*doc, QStringLiteral("A12"));
    QVERIFY(splitCells(*doc, in, 3, 1));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:gridBefore w:val=\"4\"/>"), x.constData());
    // (the grid is 2000, 1000, 1000, 2000, 2000 now: the four columns before B's cell are 6000 wide, the four after C's are the last four, also 6000)
    QVERIFY2(x.contains("<w:wBefore w:w=\"6000\"") && x.contains("<w:gridAfter w:val=\"4\"/>") && x.contains("<w:wAfter w:w=\"6000\""), x.constData());
    checkTablesReopenTheSame(*doc);
    // A column wider moves the width of the gap that has it, and not that of the gap that has not.
    QTextCursor sized = cursorAt(*doc, QStringLiteral("A0"));
    setColumnWidth(*doc, sized, 150);
    const QByteArray y = documentXml(doc->toDocx());
    QVERIFY2(y.contains("<w:wBefore w:w=\"7000\"") && y.contains("<w:wAfter w:w=\"6000\""), y.constData());
    checkTablesReopenTheSame(*doc);
    // The after gap: the cell split is the last row's, and the gap is at the end of row C.
    auto after = open(makeDocx(body, kStyles));
    QTextCursor cc = cursorAt(*after, QStringLiteral("A3"));
    QVERIFY(splitCells(*after, cc, 2, 1));
    checkTablesReopenTheSame(*after);
    const QByteArray z = documentXml(after->toDocx());
    QVERIFY2(z.contains("<w:gridBefore w:val=\"3\"/>") && z.contains("<w:gridAfter w:val=\"4\"/>"), z.constData());   // (the new column is in the end gap, not the first)
}


// A table style that is added to a document is the document's default only when the document has none under any id, so a file never has two.
void WriteTests::aBuiltInTableStyleIsNotAddedAsASecondDefault()
{
    const QByteArray styles = kStyles + "<w:style w:type=\"table\" w:default=\"1\" w:styleId=\"NormalTable\"><w:name w:val=\"Normal Table\"/><w:tblPr><w:tblInd w:w=\"0\" w:type=\"dxa\"/></w:tblPr></w:style>";
    auto doc = open(makeDocx(cellP("x") + tableX(rowX(cellX(cellP("a")) + cellX(cellP("b"))), 2) + cellP("y"), styles));
    QTextCursor c = cursorAt(*doc, QStringLiteral("a"));
    setTableStyle(*doc, c, QStringLiteral("GridTable4-Accent1"));
    const QByteArray s = styleXml(doc->toDocx());
    QCOMPARE(int(s.count("w:type=\"table\" w:default=\"1\"")) + int(s.count("w:default=\"1\" w:styleId=\"TableNormal\"")), 1);
    QVERIFY(s.contains("w:styleId=\"GridTable4-Accent1\""));
    QVERIFY(s.contains("w:styleId=\"NormalTable\""));
    // In a document with none, the first one added has it, and a second built-in table style does not add another.
    auto plainDoc = open(tableDocx(2, 2));
    QTextCursor p = cursorAt(*plainDoc, QStringLiteral("r0c0"));
    setTableStyle(*plainDoc, p, QStringLiteral("TableGrid"));
    setTableStyle(*plainDoc, p, QStringLiteral("PlainTable1"));
    const QByteArray t = styleXml(plainDoc->toDocx());
    QCOMPARE(int(t.count("w:default=\"1\"")) - int(t.count("w:type=\"paragraph\" w:default=\"1\"")), 1);
}


// A command that adds a style to the document is one step: Undo gives back the document as it was, the styles part included, and Redo makes it again.
void WriteTests::undoOfATableStyleTakesTheAddedStyleBackToo()
{
    auto doc = open(tableDocx(2, 2));
    const QByteArray stylesWas = styleXml(doc->toDocx()), bodyWas = documentXml(doc->toDocx());
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    setTableStyle(*doc, c, QStringLiteral("GridTable4-Accent2"));
    const QByteArray stylesNow = styleXml(doc->toDocx());
    QVERIFY(stylesNow.contains("GridTable4-Accent2"));
    doc->story()->undo();
    QCOMPARE(styleXml(doc->toDocx()), stylesWas);
    QCOMPARE(documentXml(doc->toDocx()), bodyWas);
    doc->story()->redo();
    QCOMPARE(styleXml(doc->toDocx()), stylesNow);
    // The same for a table made, and for Table Properties that name a style.
    auto blank = WriteDoc::blank();
    QTextCursor b(blank->story());
    const QByteArray blankWas = styleXml(blank->toDocx());
    NewTable t;
    QVERIFY(insertTable(*blank, b, t));
    QVERIFY(styleXml(blank->toDocx()).contains("w:styleId=\"TableGrid\""));
    blank->story()->undo();
    QCOMPARE(styleXml(blank->toDocx()), blankWas);
    blank->story()->redo();
    QVERIFY(styleXml(blank->toDocx()).contains("w:styleId=\"TableGrid\""));
    b.setPosition(0);
    const TableProps was = tablePropsAt(*blank, b);
    TableProps now = was;
    now.style = QStringLiteral("PlainTable2");
    setTableProps(*blank, b, was, now);
    QVERIFY(styleXml(blank->toDocx()).contains("w:styleId=\"PlainTable2\""));
    blank->story()->undo();
    QVERIFY(!styleXml(blank->toDocx()).contains("PlainTable2"));
}


// The ids of the built-in table styles are the conventional ones, so a file's own style of the id is the gallery's, and a style a build wrote with the id without its hyphen is
// still read.
void WriteTests::builtInTableStylesHaveTheConventionalIds()
{
    const QStringList ids = office::wml::builtinTableStyleIds();
    QVERIFY(ids.contains(QStringLiteral("GridTable4-Accent1")) && ids.contains(QStringLiteral("ListTable3-Accent3")));
    for (const QString &id : ids) QVERIFY2(office::wml::builtinStyle(id).id == id, qPrintable(id));
    const office::wml::Style old = office::wml::builtinStyle(QStringLiteral("GridTable4Accent1"));
    QCOMPARE(old.id, QStringLiteral("GridTable4Accent1"));
    QCOMPARE(old.tail, office::wml::builtinStyle(QStringLiteral("GridTable4-Accent1")).tail);
    // A file that has the style is not given another.
    auto doc = open(tableDocx(2, 2));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    setTableStyle(*doc, c, QStringLiteral("GridTable4-Accent1"));
    auto again = open(doc->toDocx());
    const int before = int(again->styleSheet().styles.size());
    QTextCursor d = cursorAt(*again, QStringLiteral("r0c0"));
    setTableStyle(*again, d, QStringLiteral("GridTable4-Accent1"));
    QCOMPARE(int(again->styleSheet().styles.size()), before);
}


// Table Properties writes what the person changed and nothing else: a table indent that is negative, or past what the box held, stays as the file has it when OK is pressed.
void WriteTests::tablePropertiesKeepWhatThePersonDidNotChange()
{
    for (const int twips : {-200, 40000}) {
        const QByteArray body = cellP("x") +
                                tableX(rowX(cellX(cellP("a")) + cellX(cellP("b"))), 2, 2400, "<w:tblW w:w=\"0\" w:type=\"auto\"/><w:tblInd w:w=\"" + QByteArray::number(twips) + "\" w:type=\"dxa\"/>") + cellP("y");
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c = cursorAt(*doc, QStringLiteral("a"));
        // OK with nothing changed: the file is as it was.
        answerDialog([](QDialog *) {});
        QVERIFY(tablePropertiesDialog(nullptr, doc.get(), &c));
        QVERIFY2(documentXml(doc->toDocx()).contains("<w:tblInd w:w=\"" + QByteArray::number(twips) + "\""), qPrintable(QString::number(twips)));
        // One thing changed: the rest of it as it was.
        answerDialog([](QDialog *d) { d->findChild<QCheckBox *>(QStringLiteral("headerRow"))->setChecked(true); });
        QVERIFY(tablePropertiesDialog(nullptr, doc.get(), &c));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains("<w:tblHeader/>"));
        QVERIFY2(x.contains("<w:tblInd w:w=\"" + QByteArray::number(twips) + "\""), qPrintable(QString::number(twips)));
        checkTablesReopenTheSame(*doc);
    }
}


// The Column tab takes the width as a percent of the table's (the points follow it).
void WriteTests::aColumnsWidthCanBeGivenAsAPercent()
{
    auto doc = open(tableDocx(2, 3));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    answerDialog([](QDialog *d) {
        auto *percent = d->findChild<QDoubleSpinBox *>(QStringLiteral("columnPercent"));
        QVERIFY(percent && percent->isEnabled());
        QCOMPARE(percent->value(), 33.3);
        percent->setValue(50);
        QCOMPARE(d->findChild<QDoubleSpinBox *>(QStringLiteral("columnWidth"))->value(), 150.0);
    });
    QVERIFY(tablePropertiesDialog(nullptr, doc.get(), &c));
    QVERIFY2(tablesSignature(*doc).contains(QStringLiteral("cols=150.0,100.0,100.0")), qPrintable(tablesSignature(*doc)));
    QVERIFY2(documentXml(doc->toDocx()).contains("<w:tcW w:w=\"2500\" w:type=\"pct\"/>"), documentXml(doc->toDocx()).left(1500).constData());   // (50 percent, in fiftieths)
    checkTablesReopenTheSame(*doc);
    // The table made wider keeps the percent; a width in points puts the cells back to points.
    setTableWidth(*doc, c, 600);
    QVERIFY(documentXml(doc->toDocx()).contains("<w:tcW w:w=\"2500\" w:type=\"pct\"/>"));
    setColumnWidth(*doc, c, 80);
    QVERIFY(!documentXml(doc->toDocx()).contains("w:type=\"pct\"/>"));
    checkTablesReopenTheSame(*doc);
}


void WriteTests::answerDialog(const std::function<void(QDialog *)> &fill)
{
    QTimer::singleShot(0, qApp, [fill] {
        auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dlg) return;
        fill(dlg);
        dlg->accept();
    });
}


// A table in a cell of cells that are merged stays a table: the cell it was in is the merged cell, and its own cells are its. A cell that holds only a table whose cells are
// empty is not an empty cell.
void WriteTests::mergingKeepsATableInACellIntact()
{
    const QByteArray inner = tableX(rowX(cellX(cellP("n0")) + cellX(cellP("n1"))), 2);
    const QByteArray innerEmpty = tableX(rowX(cellX("<w:p/>") + cellX("<w:p/>")), 2);
    {
        auto doc = open(makeDocx(cellP("top") + tableX(rowX(cellX(cellP("A")) + cellX(cellP("B0") + inner + cellP("B1"))), 2) + cellP("tail"), kStyles));
        QTextCursor c(doc->story());
        selectCells(*doc, c, QStringLiteral("A"), QStringLiteral("B1"));
        QVERIFY(mergeCells(*doc, c));
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("<w:tbl>")), 2);   // (the nested table is there as a table)
        QVERIFY(x.contains("n0") && x.contains("n1"));
        checkTablesReopenTheSame(*doc);
        const flow::CellRecord *inside = doc->flow().tables.get(cellKeyOf(cursorAt(*doc, QStringLiteral("n1")).block()));
        QVERIFY(inside && inside->parent != 0);
        const flow::CellRecord *parent = doc->flow().tables.get(inside->parent);
        QVERIFY(parent && parent->parent == 0);
        QCOMPARE(parent->cell.span, 2);
    }
    {
        auto doc = open(makeDocx(cellP("top") + tableX(rowX(cellX(cellP("A")) + cellX(innerEmpty + "<w:p/>")), 2) + cellP("tail"), kStyles));
        QTextBlock lastCell = doc->story()->lastBlock().previous();   // (the empty paragraph that ends the second cell)
        QTextCursor c(doc->story());
        c.setPosition(cursorAt(*doc, QStringLiteral("A")).position());
        c.setPosition(lastCell.position(), QTextCursor::KeepAnchor);
        QVERIFY(mergeCells(*doc, c));
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tbl>")), 2);
        int nested = 0;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (const flow::CellRecord *r = doc->flow().tables.get(cellKeyOf(b)); r && r->parent) ++nested;
        QCOMPARE(nested, 2);
        checkTablesReopenTheSame(*doc);
    }
}


// A merge puts the kept pieces of the cells it joins (a bookmark between cells) in the cell it makes.
void WriteTests::mergingKeepsThePiecesKeptBetweenCells()
{
    const QByteArray body = cellP("top") +
                            tableX(rowX(cellX(cellP("A")) + "<w:bookmarkStart w:id=\"7\" w:name=\"between\"/>" + cellX(cellP("B")) + "<w:bookmarkEnd w:id=\"7\"/>" + cellX(cellP("C"))), 3) +
                            cellP("tail");
    auto doc = open(makeDocx(body, kStyles));
    QCOMPARE(int(documentXml(doc->toDocx()).count("w:name=\"between\"")), 1);
    QTextCursor c(doc->story());
    selectCells(*doc, c, QStringLiteral("A"), QStringLiteral("B"));
    QVERIFY(mergeCells(*doc, c));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("w:name=\"between\"") && x.contains("<w:bookmarkEnd w:id=\"7\"/>"), x.constData());
    checkTablesReopenTheSame(*doc);
}


// A bookmark kept between the rows or the cells of a table stays when the table is changed by any command, the older ones too.
void WriteTests::aBookmarkBetweenRowsStaysWhenATableIsChanged()
{
    const QByteArray body = cellP("top") +
                            tableX("<w:bookmarkStart w:id=\"7\" w:name=\"keep\"/>" + rowX(cellX(cellP("A0")) + cellX(cellP("A1"))) + "<w:bookmarkEnd w:id=\"7\"/>" +
                                       rowX(cellX(cellP("B0")) + cellX(cellP("B1"))),
                                   2) +
                            cellP("tail");
    for (int command = 0; command < 3; ++command) {
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c = cursorAt(*doc, QStringLiteral("B0"));
        if (command == 0) insertTableRow(*doc, c, true);
        else if (command == 1) setShading(*doc, c, QColor(0, 255, 0));
        else deleteTableColumns(*doc, c);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:bookmarkStart w:id=\"7\" w:name=\"keep\"/>") && x.contains("<w:bookmarkEnd w:id=\"7\"/>"), qPrintable(QString::number(command)));
        checkTablesReopenTheSame(*doc);
    }
}


// The commands of the Table tabs on tables made to be hard on them: one of 20,000 rows, one of 70 columns, and one whose merges and spans make no sense. Each runs in bounded time
// and leaves the document whole and the records bounded.
void WriteTests::everyTableCommandRunsOnHostileTables()
{
    QByteArray odd;
    odd += rowX(cellX(cellP("h0"), "<w:gridSpan w:val=\"9\"/><w:vMerge/>") + cellX(cellP("h1")));
    odd += rowX(cellX(cellP("h2"), "<w:vMerge w:val=\"restart\"/>") + cellX(cellP("h3"), "<w:gridSpan w:val=\"0\"/>") + cellX(cellP("h4"), "<w:gridSpan w:val=\"7\"/>"));
    odd += rowX(cellX(cellP("h5"), "<w:vMerge/>") + cellX(cellP("h6"), "<w:vMerge/>") + cellX(cellP("h7"), "<w:vMerge/>") + cellX(cellP("h8"), "<w:vMerge/>"));
    odd += rowX(cellX(cellP("h9"), "<w:vMerge w:val=\"restart\"/><w:gridSpan w:val=\"2\"/>") + cellX(cellP("ha"), "<w:vMerge/><w:gridSpan w:val=\"3\"/>"));
    odd += rowX(cellX(cellP("hb"), "<w:vMerge/>"), "<w:gridBefore w:val=\"5\"/><w:gridAfter w:val=\"9\"/>");
    struct Doc {
        const char *name;
        QByteArray zip;
        int bound;
    };
    const Doc docs[] = {{"tall", tableDocx(20000, 1), 60}, {"wide", tableDocx(3, 70), 20}, {"odd", makeDocx(cellP("top") + tableX(odd, 3) + cellP("tail"), kStyles), 20}};
    for (const Doc &d : docs) {
        for (int command = 0; command < 24; ++command) {
            auto doc = open(d.zip);
            QVERIFY(doc);
            QTextCursor c = cursorAt(*doc, QStringLiteral("top"));
            // (a place in the table: its first cell, and a block of cells for the ones that take a selection)
            QTextBlock first = doc->story()->begin();
            while (first.isValid() && !cellKeyOf(first)) first = first.next();
            QVERIFY(first.isValid());
            c.setPosition(first.position());
            QTextBlock last = doc->story()->lastBlock();
            while (last.isValid() && !cellKeyOf(last)) last = last.previous();
            if (command % 2) c.setPosition(last.position(), QTextCursor::KeepAnchor);
            QElapsedTimer clock;
            clock.start();
            switch (command / 2) {
            case 0: insertTableRow(*doc, c, true); break;
            case 1: insertTableColumn(*doc, c, false); break;
            case 2: deleteTableRows(*doc, c); break;
            case 3: deleteTableColumns(*doc, c); break;
            case 4: mergeCells(*doc, c); break;
            case 5: splitCells(*doc, c, 2, 2); break;
            case 6: setShading(*doc, c, QColor(9, 9, 9)); setBorders(*doc, c, BorderSide::All, flow::TableLine{flow::TableLine::Style::Single, 1, QColor()}); break;
            case 7: setCellAlignment(*doc, c, Qt::AlignHCenter, flow::CellAlign::Bottom); setColumnWidth(*doc, c, 33); setRowHeight(*doc, c, 25, true); break;
            case 8: distributeColumns(*doc, c); distributeRows(*doc, c); break;
            case 9: fitTable(*doc, c, TableFit::Contents); fitTable(*doc, c, TableFit::Window); fitTable(*doc, c, TableFit::Fixed); break;
            case 10: {
                const TableProps was = tablePropsAt(*doc, c);
                TableProps now = was;
                now.header = true;
                now.indent = -5;
                now.style = QStringLiteral("GridTable4-Accent1");
                setTableProps(*doc, c, was, now);
                setTableLook(*doc, c, 0x20, false);
                break;
            }
            default: selectInTable(*doc, c, TableUnit::Column); tabThroughCells(*doc, c, false); deleteTable(*doc, c); break;
            }
            const double seconds = clock.elapsed() / 1000.0;
            const QString why = QStringLiteral("%1 command %2 took %3 s").arg(QLatin1String(d.name)).arg(command).arg(seconds);
            // (A checked build, with the memory and undefined-behavior checkers, runs slower and sets JP_TEST_TIME_SCALE.)
            QVERIFY2(seconds < d.bound * std::max(1, qEnvironmentVariableIntValue("JP_TEST_TIME_SCALE")), qPrintable(why));
            QVERIFY2(doc->flow().tables.liveCount() < 200000, qPrintable(why));
            const QByteArray zip = doc->toDocx();
            QVERIFY2(!zip.isEmpty(), qPrintable(why));
            auto back = open(zip);
            QVERIFY2(back, qPrintable(why));
            QVERIFY2(back->story()->toPlainText().size() > 0, qPrintable(why));
        }
    }
}


// What the commands and the controls cost on a long table is counted, not timed: the caret moving about a table looks at the story's tables once, a join of two cells looks at the
// cells next to them and not at all of them, and fitting columns to their cells makes a font's measures once.
void WriteTests::theCaretMovingAboutALongTableLooksAtItsTablesOnce()
{
    auto doc = open(tableDocx(4000, 3));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r2000c1"));
    const qint64 built = tableViewsBuiltForTest();
    for (int i = 0; i < 100; ++i) {
        c.setPosition(cursorAt(*doc, QStringLiteral("r%1c%2").arg(1000 + i * 7).arg(i % 3)).position());
        QVERIFY(tablePropsAt(*doc, c).columnWidth > 0);
        QVERIFY(cellPlace(*doc, c).inTable);
    }
    QVERIFY2(tableViewsBuiltForTest() - built <= 4, qPrintable(QString::number(tableViewsBuiltForTest() - built)));
}


void WriteTests::aJoinOfTwoCellsLooksAtTheCellsNextToThem()
{
    auto doc = open(tableDocx(4000, 3));
    QTextCursor sel = cursorAt(*doc, QStringLiteral("r2000c0"));
    sel.setPosition(cursorAt(*doc, QStringLiteral("r2001c1")).position() + 2, QTextCursor::KeepAnchor);
    const qint64 scanned = tableCellsScannedForTest();
    for (int i = 0; i < 20; ++i) QVERIFY(canMergeCells(*doc, sel));
    QVERIFY2(tableCellsScannedForTest() - scanned < 20 * 30, qPrintable(QString::number(tableCellsScannedForTest() - scanned)));
}


void WriteTests::fittingColumnsToTheirCellsMeasuresAFontOnce()
{
    auto doc = open(tableDocx(4000, 3));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r2000c1"));
    const qint64 measured = tableFontMeasuresMadeForTest();
    fitTable(*doc, c, TableFit::Contents);
    QVERIFY2(tableFontMeasuresMadeForTest() - measured <= 3, qPrintable(QString::number(tableFontMeasuresMadeForTest() - measured)));
}


// The text goes round a frame and a square-wrapped box on a small page: the band a line was kept clear of objects by is the height that was checked for it (that of the text on it,
// which is smaller than the estimate from its paragraph's first run when the line is in smaller type), not the estimate (found by the random editing of tables, seed 19 of it).
void WriteTests::aLineInSmallerTypeIsKeptClearOfAFrameByTheHeightThatWasChecked()
{
    const QByteArray tail = "<w:pPr><w:pStyle w:val=\"Heading2\"/></w:pPr>";
    const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
                                        "<w:pPr><w:spacing w:before=\"120\" w:after=\"60\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Arimo\" w:hAnsi=\"Arimo\"/><w:b/><w:sz w:val=\"26\"/></w:rPr></w:style>";
    const QByteArray body =
        "<w:p>" + tail + "<w:r><w:t>2 tail</w:t></w:r></w:p><w:p>" + tail + "</w:p>"
        "<w:p><w:r><w:rPr><w:rFonts w:ascii=\"Arimo\" w:hAnsi=\"Arimo\"/><w:b/><w:sz w:val=\"26\"/></w:rPr><w:t xml:space=\"preserve\">bword </w:t></w:r><w:r><w:rPr><w:b/></w:rPr><w:t xml:space=\"preserve\"> that a line goes round the box</w:t></w:r><w:r><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\" "
        "xmlns:w10=\"urn:schemas-microsoft-com:office:word\"><v:rect id=\"wall3414\" style=\"position:absolute;margin-left:110pt;margin-top:4pt;width:50pt;height:40pt\" fillcolor=\"red\" stroked=\"f\">"
        "<w10:wrap type=\"square\"/></v:rect></w:pict></w:r></w:p>"
        "<w:p><w:pPr><w:framePr w:w=\"1800\" w:hSpace=\"120\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\" w:y=\"60\"/></w:pPr><w:r><w:t>FRAME one of wo paragraphword s</w:t></w:r></w:p>"
        "<w:p><w:pPr><w:framePr w:w=\"1800\" w:hSpace=\"120\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\" w:y=\"60\"/></w:pPr>"
        "<w:r><w:t>and the second of them, whichis longer than a line of the frame</w:t></w:r></w:p>"
        "<w:p><w:r><w:t>ough words that more than a line or two are beside the box that is set from the page</w:t></w:r><w:r><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\" "
        "xmlns:w10=\"urn:schemas-microsoft-com:office:word\"><v:rect id=\"wall4710\" style=\"position:absolute;margin-left:150pt;margin-top:60pt;width:40pt;height:50pt;mso-position-horizontal-relative:page;"
        "mso-position-vertical-relative:page\" fillcolor=\"red\" stroked=\"f\"><w10:wrap type=\"square\"/></v:rect></w:pict></w:r></w:p>";
    auto doc = open(makeDocx(body, styles, QByteArray(),
                             "<w:sectPr><w:pgSz w:w=\"5200\" w:h=\"4400\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>"));
    QVERIFY(doc);
    for (int k = 0; k < doc->pager().pageCount(); ++k) doc->pager().materialize(k);
    const QString problem = audit::wallProblems(doc->pager(), doc->story());
    QVERIFY2(problem.isEmpty(), qPrintable(problem));
}


// The records a delete across cells takes out of the story go with its step: the table commands after it, enough to pass the number of records that are let go from, do not let
// them go, and Undo past it gives the document as it was.
void WriteTests::undoPastADeleteAcrossCellsFindsItsRecordsAfterManyTableCommands()
{
    QByteArray big;
    for (int r = 0; r < 2000; ++r) big += rowX(cellX(cellP("g" + QByteArray::number(r))) + cellX(cellP("h")) + cellX(cellP("i")));
    const QByteArray body = cellP("top") + tableX(rowX(cellX(cellP("s0")) + cellX(cellP("s1"))), 2) + cellP("mid") + tableX(big, 3) + cellP("tail");
    auto doc = open(makeDocx(body, kStyles));
    const QByteArray started = documentXml(doc->toDocx());
    QTextCursor sel = cursorAt(*doc, QStringLiteral("top"));
    sel.movePosition(QTextCursor::EndOfBlock);
    sel.setPosition(cursorAt(*doc, QStringLiteral("mid")).position() + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, sel);
    QVERIFY(!plain(*doc).contains(QStringLiteral("s0")));
    QTextCursor c = cursorAt(*doc, QStringLiteral("g0"));
    for (int i = 0; i < 12; ++i) setShading(*doc, c, QColor(i * 9, 0, 0));
    QVERIFY(doc->flow().tables.liveCount() >= 30000);
    while (doc->canUndo()) doc->undo();
    QVERIFY(plain(*doc).contains(QStringLiteral("s0")));
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (cellKeyOf(b)) QVERIFY2(doc->flow().tables.get(cellKeyOf(b)), qPrintable(b.text()));
    QCOMPARE(documentXml(doc->toDocx()), started);
}


// A command that would make a table of more than 50,000 cells is refused with a reason.
void WriteTests::aCommandThatWouldPassFiftyThousandCellsIsRefused()
{
    auto doc = open(tableDocx(16000, 3));
    QTextCursor sel = cursorAt(*doc, QStringLiteral("r0c0"));
    sel.setPosition(cursorAt(*doc, QStringLiteral("r999c2")).position() + 1, QTextCursor::KeepAnchor);
    const QByteArray was = documentXml(doc->toDocx());
    doc->tableNotice.clear();
    insertTableRow(*doc, sel, true);   // (1,000 rows of 3: 51,000 cells)
    QVERIFY2(doc->tableNotice.contains(QStringLiteral("50,000")), qPrintable(doc->tableNotice));
    QCOMPARE(documentXml(doc->toDocx()), was);
}


// A split that can't be made says why, whichever it is.
void WriteTests::splittingWhereItCanNotBeDoneSaysWhy()
{
    auto doc = open(tableDocx(2, 2));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    for (const QPair<int, int> &asked : {QPair<int, int>{64, 1}, QPair<int, int>{1, 501}, QPair<int, int>{1, 1}}) {
        doc->tableNotice.clear();
        QVERIFY(!splitCells(*doc, c, asked.first, asked.second));
        QVERIFY2(!doc->tableNotice.isEmpty(), qPrintable(QString::number(asked.first)));
    }
    QTextCursor outside = cursorAt(*doc, QStringLiteral("before"));
    doc->tableNotice.clear();
    QVERIFY(!splitCells(*doc, outside, 2, 1));
    QVERIFY(!doc->tableNotice.isEmpty());
}


// A command that does nothing says why when the cell can't be split as asked.
void WriteTests::splittingACellJoinedDownAsksForRowsAsItIsJoined()
{
    auto doc = open(makeDocx(cellP("top") +
                                 tableX(rowX(cellX(cellP("t"), "<w:vMerge w:val=\"restart\"/>") + cellX(cellP("a"))) + rowX(cellX(cellP(""), "<w:vMerge/>") + cellX(cellP("b"))) +
                                            rowX(cellX(cellP(""), "<w:vMerge/>") + cellX(cellP("c"))),
                                        2) +
                                 cellP("tail"),
                             kStyles));
    QTextCursor c = cursorAt(*doc, QStringLiteral("t"));
    doc->tableNotice.clear();
    const QByteArray was = documentXml(doc->toDocx());
    QVERIFY(!splitCells(*doc, c, 1, 2));
    QVERIFY(!doc->tableNotice.isEmpty());
    QCOMPARE(documentXml(doc->toDocx()), was);
    doc->tableNotice.clear();
    QVERIFY(splitCells(*doc, c, 1, 3));
    QVERIFY(doc->tableNotice.isEmpty());
}


// A table in a table's cell is changed with the table it is in: its cells keep their place in the cell, and the cell's other paragraphs.
void WriteTests::changesKeepATableInACellAndWhatIsAroundIt()
{
    const QByteArray inner = tableX(rowX(cellX(cellP("n0")) + cellX(cellP("n1"))), 2);
    const QByteArray body = cellP("top") +
                            tableX(rowX(cellX(cellP("A0") + inner + cellP("A0b")) + cellX(cellP("A1"))) + rowX(cellX(cellP("B0")) + cellX(cellP("B1"))),
                                   2) +
                            cellP("tail");
    auto doc = open(makeDocx(body, kStyles));
    QTextCursor c = cursorAt(*doc, QStringLiteral("B0"));
    setShading(*doc, c, QColor(0, 0xFF, 0));
    setColumnWidth(*doc, c, 90);
    QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:tbl>")), 2);
    checkTablesReopenTheSame(*doc);
    // The cell with the table in it keeps it; a change inside the table in the cell works.
    QTextCursor in = cursorAt(*doc, QStringLiteral("n1"));
    setShading(*doc, in, QColor(0, 0, 0xFF));
    checkTablesReopenTheSame(*doc);
    QTextCursor outer = cursorAt(*doc, QStringLiteral("A1"));
    QVERIFY(mergeCells(*doc, outer) == false);   // (one cell)
    QTextCursor row;
    selectCells(*doc, row, QStringLiteral("B0"), QStringLiteral("B1"));
    QVERIFY(mergeCells(*doc, row));
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tbl>")), 2);
    checkTablesReopenTheSame(*doc);
    QVERIFY(plain(*doc).contains(QStringLiteral("n0\nn1\nA0b")));
}


// A selection across cells empties them without joining what is in different cells, and one that holds a whole table takes it out.
void WriteTests::deletingAcrossCellsEmptiesThemAndTakesWholeTablesOut()
{
    auto doc = open(tableDocx(2, 2));
    QTextDocument *s = doc->story();
    auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
    const QByteArray was = documentXml(doc->toDocx());
    QTextCursor c(s);
    // From the middle of r0c0 to the middle of r1c0: r0c0 loses its end, r0c1 is emptied, r1c0 loses its start.
    c.setPosition(block(QStringLiteral("r0c0")).position() + 1);
    c.setPosition(block(QStringLiteral("r1c0")).position() + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QStringList texts;
    for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) texts << b.text();
    QCOMPARE(texts, (QStringList{"before", "r", "", "1c0", "r1c1", "after"}));
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:tc>")), 4);
    checkWhole(*doc);
    s->undo();
    QCOMPARE(documentXml(doc->toDocx()), was);
    // From inside the paragraph before to inside the one after: the table goes, and what is left of the two is one paragraph.
    c.setPosition(block(QStringLiteral("before")).position() + 3);
    c.setPosition(block(QStringLiteral("after")).position() + 2, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QCOMPARE(s->toPlainText(), QStringLiteral("befter"));
    QVERIFY(!documentXml(doc->toDocx()).contains("<w:tbl>"));
    checkWhole(*doc);
    s->undo();
    QCOMPARE(documentXml(doc->toDocx()), was);
}
